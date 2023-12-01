// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/manager/Actor.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
#include <chrono>
#include <iomanip>
#include <span>
#include <sstream>
#include <string_view>
#include <utility>

#include "BoostAsio.hpp"
#include "blockchain/node/manager/Shared.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/blockchain/node/Config.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/Mempool.hpp"
#include "internal/blockchain/node/Types.hpp"
#include "internal/blockchain/node/filteroracle/FilterOracle.hpp"
#include "internal/blockchain/node/headeroracle/HeaderOracle.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/network/blockchain/Types.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/api/Network.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/Session.internal.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/AddressStyle.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Subchain.hpp"      // IWYU pragma: keep
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/blockchain/node/SendResult.hpp"  // IWYU pragma: keep
#include "opentxs/network/otdht/Base.hpp"
#include "opentxs/network/otdht/Block.hpp"  // IWYU pragma: keep
#include "opentxs/network/otdht/Data.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/Direction.hpp"   // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Policy.hpp"      // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WorkType.hpp"
#include "opentxs/util/WorkType.internal.hpp"

namespace opentxs::blockchain::node
{
using namespace std::literals;

auto print(ManagerJobs in) noexcept -> std::string_view
{
    using enum ManagerJobs;
    static constexpr auto map =
        frozen::make_unordered_map<ManagerJobs, std::string_view>({
            {shutdown, "shutdown"sv},
            {sync_reply, "sync_reply"sv},
            {sync_new_block, "sync_new_block"sv},
            {heartbeat, "heartbeat"sv},
            {start_wallet, "start_wallet"sv},
            {init, "init"sv},
            {filter_update, "filter_update"sv},
            {statemachine, "statemachine"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {
        LogAbort()(__FUNCTION__)(": invalid node::ManagerJobs: ")(
            static_cast<OTZMQWorkType>(in))
            .Abort();
    }
}
}  // namespace opentxs::blockchain::node

namespace opentxs::blockchain::node::manager
{
using enum opentxs::network::zeromq::socket::Direction;
using opentxs::network::zeromq::socket::Policy;
using enum opentxs::network::zeromq::socket::Type;

Actor::Actor(
    std::shared_ptr<const api::internal::Session> api,
    std::shared_ptr<node::Manager> self,
    std::shared_ptr<Shared> shared,
    network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : ManagerActor(
          api->Self(),
          LogTrace(),
          [&] {
              auto out = CString{alloc};
              out.append(print(shared->Chain()));
              out.append(" node manager"sv);

              return out;
          }(),
          0ms,
          std::move(batch),
          alloc,
          {
              {api->Endpoints().Shutdown(), Connect},
              {shared->Endpoints().shutdown_publish_, Connect},
              {shared->Endpoints().new_filter_publish_, Connect},
          },
          {
              {shared->Endpoints().manager_pull_, Bind},
          },
          {},
          {
              {Push,
               Policy::Internal,
               {
                   {shared->Endpoints().peer_manager_pull_, Connect},
               }},
              {Push,
               Policy::Internal,
               {
                   {shared->Endpoints().wallet_pull_, Connect},
               }},
              {Push,
               Policy::Internal,
               {
                   {shared->Endpoints().otdht_pull_, Connect},
               }},
              {Push,
               Policy::Internal,
               {
                   {api->Endpoints().Internal().BlockchainMessageRouter(),
                    Connect},
               }},
          })
    , api_p_(std::move(api))
    , self_p_(std::move(self))
    , shared_p_(std::move(shared))
    , api_(api_p_->Self())
    , self_(*self_p_)
    , shared_(*shared_p_)
    , to_peer_manager_(pipeline_.Internal().ExtraSocket(0))
    , to_wallet_(pipeline_.Internal().ExtraSocket(1))
    , to_dht_(pipeline_.Internal().ExtraSocket(2))
    , to_blockchain_api_(pipeline_.Internal().ExtraSocket(3))
    , heartbeat_(api_.Network().Asio().Internal().GetTimer())
{
    log_(shared_.GetConfig().Print(get_allocator())).Flush();
}

auto Actor::do_shutdown() noexcept -> void
{
    shared_.Shutdown();
    heartbeat_.Cancel();
    shared_p_.reset();
    self_p_.reset();
    api_p_.reset();
}

auto Actor::do_startup(allocator_type monotonic) noexcept -> bool
{
    if ((api_.Internal().ShuttingDown()) || (self_.Internal().ShuttingDown())) {

        return true;
    }

    reset_heartbeat();

    return false;
}

auto Actor::Init(std::shared_ptr<Actor> me) noexcept -> void
{
    signal_startup(me);
}

auto Actor::notify_sync_client() const noexcept -> void
{
    to_dht_.SendDeferred([this] {
        const auto& filters = shared_.FilterOracle();
        const auto tip = filters.FilterTip(filters.DefaultType());
        using Job = network::blockchain::DHTJob;
        auto msg = MakeWork(Job::job_processed);
        msg.AddFrame(tip.height_);
        msg.AddFrame(tip.hash_);

        return msg;
    }());
}

auto Actor::pipeline(
    const Work work,
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    using enum ManagerJobs;

    switch (work) {
        case sync_reply:
        case sync_new_block: {
            process_sync_data(std::move(msg), monotonic);
        } break;
        case heartbeat: {
            process_heartbeat(std::move(msg), monotonic);
        } break;
        case start_wallet: {
            process_start_wallet(std::move(msg), monotonic);
        } break;
        case filter_update: {
            process_filter_update(std::move(msg), monotonic);
        } break;
        case shutdown:
        case init:
        case statemachine: {
            unhandled_type(work);
        }
        default: {
            unknown_type(work);
        }
    }
}

auto Actor::process_filter_update(Message&& in, allocator_type) noexcept -> void
{
    const auto body = in.Payload();

    assert_true(2 < body.size());

    const auto height = body[2].as<block::Height>();
    const auto target = shared_.HeaderOracle().Internal().Target();

    {
        const auto progress =
            (0 == target) ? double{0}
                          : ((double(height) / double(target)) * double{100});
        auto display = std::stringstream{};
        display << std::setprecision(3) << progress << "%";

        if (false == shared_.GetConfig().disable_wallet_) {
            log_(print(shared_.Chain()))(" chain sync progress: ")(
                height)(" of ")(target)(" (")(display.str())(")")
                .Flush();
        }
    }

    to_blockchain_api_.SendDeferred([&] {
        auto work = opentxs::network::zeromq::tagged_message(
            WorkType::BlockchainSyncProgress, true);
        work.AddFrame(shared_.Chain());
        work.AddFrame(height);
        work.AddFrame(target);

        return work;
    }());
}

auto Actor::process_heartbeat(Message&& in, allocator_type monotonic) noexcept
    -> void
{
    // TODO upgrade all the oracles to no longer require this
    shared_.Mempool().Heartbeat();
    shared_.FilterOracle().Internal().Heartbeat();
    reset_heartbeat();
}

auto Actor::process_start_wallet(Message&& in, allocator_type) noexcept -> void
{
    to_wallet_.SendDeferred(MakeWork(wallet::WalletJobs::start_wallet));
}

auto Actor::process_sync_data(Message&& in, allocator_type monotonic) noexcept
    -> void
{
    const auto start = Clock::now();
    const auto sync = api_.Factory().BlockchainSyncMessage(in);
    const auto& data = sync->asData();
    auto prior = block::Hash{};
    auto hashes = Vector<block::Hash>{};
    const auto accepted =
        shared_.HeaderOracle().Internal().ProcessSyncData(prior, hashes, data);

    if (0_uz < accepted) {
        const auto& blocks = data.Blocks();

        log_("Accepted ")(accepted)(" of ")(blocks.size())(" ")(
            print(shared_.Chain()))(" headers")
            .Flush();
        shared_.FilterOracle().Internal().ProcessSyncData(
            prior, hashes, data, monotonic);
        const auto elapsed =
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                Clock::now() - start);
        log_("Processed ")(blocks.size())(" ")(print(shared_.Chain()))(
            " sync packets in ")(elapsed)
            .Flush();
    } else {
        log_("Invalid ")(print(shared_.Chain()))(" sync data").Flush();
    }

    notify_sync_client();
}

auto Actor::reset_heartbeat() noexcept -> void
{
    heartbeat_.SetRelative(heartbeat_interval_);
    heartbeat_.Wait([this](const auto& ec) {
        if (ec) {
            if (unexpected_asio_error(ec)) {
                LogError()()("received asio error (")(ec.value())(") :")(ec)
                    .Flush();
            }
        } else {
            pipeline_.Push(MakeWork(ManagerJobs::heartbeat));
        }
    });
}

auto Actor::work(allocator_type) noexcept -> bool { return false; }

Actor::~Actor() = default;
}  // namespace opentxs::blockchain::node::manager
