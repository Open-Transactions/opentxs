// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "api/crypto/blockchain/BalanceOracle.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <chrono>
#include <span>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "internal/api/crypto/blockchain/Types.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/core/Factory.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/alloc/Logging.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/BlockchainHandle.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Envelope.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/Direction.hpp"   // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Policy.hpp"      // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/ScopeGuard.hpp"

namespace opentxs::api::crypto::blockchain
{
auto print(BalanceOracleJobs job) noexcept -> std::string_view
{
    try {
        using Type = BalanceOracleJobs;

        static const auto map = Map<Type, CString>{
            {Type::shutdown, "shutdown"},
            {Type::update_balance, "update_balance"},
            {Type::registration, "registration"},
            {Type::init, "init"},
            {Type::statemachine, "statemachine"},
        };

        return map.at(job);
    } catch (...) {
        LogError()(__FUNCTION__)("invalid BalanceOracleJobs: ")(
            static_cast<OTZMQWorkType>(job))
            .Flush();

        OT_FAIL;
    }
}
}  // namespace opentxs::api::crypto::blockchain

namespace opentxs::api::crypto::blockchain
{
using enum opentxs::network::zeromq::socket::Direction;
using enum opentxs::network::zeromq::socket::Policy;
using enum opentxs::network::zeromq::socket::Type;

BalanceOracle::Imp::Imp(
    std::shared_ptr<const api::Session> api,
    std::string_view endpoint,
    const opentxs::network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : Actor(
          *api,
          LogTrace(),
          CString{"Balance oracle", alloc},
          0ms,
          batch,
          alloc,
          {
              {api->Endpoints().Shutdown(), Connect},
          },
          {
              {endpoint, Bind},
          },
          {},
          {
              {Router,
               Internal,
               {
                   {api->Endpoints().BlockchainBalance(), Bind},
               }},
              {Publish,
               Internal,
               {
                   {api->Endpoints().BlockchainWalletUpdated(), Bind},
               }},
          })
    , api_(api)
    , router_(pipeline_.Internal().ExtraSocket(0))
    , publish_(pipeline_.Internal().ExtraSocket(1))
    , data_(alloc)
{
    OT_ASSERT(api_);
}

auto BalanceOracle::Imp::do_shutdown() noexcept -> void { api_.reset(); }

auto BalanceOracle::Imp::do_startup(allocator_type) noexcept -> bool
{
    return api_->Internal().ShuttingDown();
}

auto BalanceOracle::Imp::make_message(
    const opentxs::network::zeromq::Envelope& id,
    const identifier::Nym* owner,
    const Chain chain,
    const Balance& balance,
    const WorkType type) const noexcept -> Message
{
    auto out = [&] {
        if (id.IsValid()) {

            return reply_to_message(id, true);
        } else {
            auto m = Message{};
            m.StartBody();

            return m;
        }
    }();
    out.AddFrame(type);
    out.AddFrame(chain);
    out.AddFrame(balance.first);
    out.AddFrame(balance.second);

    if (nullptr != owner) { out.AddFrame(*owner); }

    return out;
}

auto BalanceOracle::Imp::notify_subscribers(
    const Subscribers& recipients,
    const Balance& balance,
    const Chain chain) noexcept -> void
{
    const auto& log = LogTrace();
    publish_.Send(
        make_message(
            {}, nullptr, chain, balance, WorkType::BlockchainWalletUpdated),
        __FILE__,
        __LINE__);

    for (const auto& id : recipients) {
        log(OT_PRETTY_CLASS())("notifying connection ")
            .asHex(id.get()[0].Bytes())(" for ")(print(chain))(
                " balance update");
        router_.Send(
            make_message(
                id, nullptr, chain, balance, WorkType::BlockchainBalance),
            __FILE__,
            __LINE__);
    }
}

auto BalanceOracle::Imp::notify_subscribers(
    const Subscribers& recipients,
    const identifier::Nym& owner,
    const Balance& balance,
    const Chain chain) noexcept -> void
{
    const auto& log = LogTrace();
    publish_.Send(
        make_message(
            {}, &owner, chain, balance, WorkType::BlockchainWalletUpdated),
        __FILE__,
        __LINE__);

    for (const auto& id : recipients) {
        log(OT_PRETTY_CLASS())("notifying connection ")
            .asHex(id.get()[0].Bytes())(" for ")(print(chain))(
                " balance update for nym ")(owner, api_->Crypto());
        router_.Send(
            make_message(
                id, &owner, chain, balance, WorkType::BlockchainBalance),
            __FILE__,
            __LINE__);
    }
}

auto BalanceOracle::Imp::pipeline(
    const Work work,
    Message&& msg,
    allocator_type) noexcept -> void
{
    switch (work) {
        case Work::update_balance: {
            process_update_balance(std::move(msg));
        } break;
        case Work::registration: {
            process_registration(std::move(msg));
        } break;
        case Work::shutdown:
        case Work::init:
        case Work::statemachine:
        default: {
            LogError()(OT_PRETTY_CLASS())(": unhandled type").Flush();

            OT_FAIL;
        }
    }
}

auto BalanceOracle::Imp::process_registration(Message&& in) noexcept -> void
{
    // NOTE pipeline inserts an extra frame at the front of the message
    connection_id(in);
    const auto connectionID = in.Envelope();

    OT_ASSERT(connectionID.IsValid());

    const auto body = in.Payload();

    OT_ASSERT(1_uz < body.size());

    const auto haveNym = (2_uz < body.size());
    auto output = opentxs::blockchain::Balance{};
    const auto& chainFrame = body[1_uz];
    const auto nym = [&] {
        if (haveNym) {

            return api_->Factory().NymIDFromHash(body[2].Bytes());
        } else {

            return identifier::Nym{};
        }
    }();
    const auto chain = chainFrame.as<Chain>();
    const auto postcondition = ScopeGuard{[&]() {
        router_.Send(
            [&] {
                auto work = opentxs::network::zeromq::tagged_reply_to_message(
                    in, WorkType::BlockchainBalance);
                work.AddFrame(chainFrame);
                output.first.Serialize(work.AppendBytes());
                output.second.Serialize(work.AppendBytes());

                if (haveNym) { work.AddFrame(nym); }

                return work;
            }(),
            __FILE__,
            __LINE__);
    }};
    const auto unsupported =
        (0 == opentxs::blockchain::SupportedChains().count(chain)) &&
        (Chain::UnitTest != chain);

    if (unsupported) { return; }

    auto& subscribers = [&]() -> auto& {
        auto& chainData = data_[chain];

        if (haveNym) {

            return chainData.second[nym].second;
        } else {

            return chainData.first.second;
        }
    }();
    const auto& id = *(subscribers.emplace(connectionID).first);
    const auto& log = LogTrace();
    log(OT_PRETTY_CLASS())
        .asHex(id.get()[0].Bytes())(" subscribed to ")(print(chain))(
            " balance updates");

    if (haveNym) { log(" for nym ")(nym, api_->Crypto()); }

    log.Flush();

    try {
        const auto handle = api_->Network().Blockchain().GetChain(chain);

        if (false == handle.IsValid()) {
            throw std::runtime_error{"invalid chain"};
        }

        const auto& network = handle.get();

        if (haveNym) {
            output = network.GetBalance(nym);
        } else {
            output = network.GetBalance();
        }
    } catch (...) {
    }
}

auto BalanceOracle::Imp::process_update_balance(Message&& in) noexcept -> void
{
    const auto body = in.Payload();

    OT_ASSERT(3_uz < body.size());

    const auto chain = body[1_uz].as<Chain>();
    auto balance = std::make_pair(
        factory::Amount(body[2_uz]), factory::Amount(body[3_uz]));

    if (4_uz < body.size()) {
        const auto owner = api_->Factory().NymIDFromHash(body[4_uz].Bytes());
        process_update_balance(owner, chain, std::move(balance));
    } else {
        process_update_balance(chain, std::move(balance));
    }
}

auto BalanceOracle::Imp::process_update_balance(
    const Chain chain,
    Balance input) noexcept -> void
{
    auto changed{false};
    auto& data = [&]() -> auto& {
        if (auto i = data_.find(chain); i != data_.end()) {
            auto& out = i->second.first;
            changed = (out.first != input);

            return out;
        } else {
            changed = true;

            return data_[chain].first;
        }
    }();
    auto& balance = data.first;
    const auto& subscribers = data.second;
    balance.swap(input);

    if (changed) { notify_subscribers(subscribers, balance, chain); }
}

auto BalanceOracle::Imp::process_update_balance(
    const identifier::Nym& owner,
    const Chain chain,
    Balance input) noexcept -> void
{
    auto changed{false};
    auto& data = [&]() -> auto& {
        if (auto i = data_.find(chain); i != data_.end()) {
            auto& nymData = i->second.second;

            if (auto j = nymData.find(owner); j != nymData.end()) {
                auto& out = j->second;
                changed = (out.first != input);

                return out;
            } else {
                changed = true;

                return nymData[owner];
            }
        } else {
            changed = true;

            return data_[chain].second[owner];
        }
    }();
    auto& balance = data.first;
    const auto& subscribers = data.second;
    balance.swap(input);

    if (changed) { notify_subscribers(subscribers, owner, balance, chain); }
}

auto BalanceOracle::Imp::work(allocator_type monotonic) noexcept -> bool
{
    return false;
}

BalanceOracle::Imp::~Imp() = default;
}  // namespace opentxs::api::crypto::blockchain

namespace opentxs::api::crypto::blockchain
{
BalanceOracle::BalanceOracle(
    std::shared_ptr<const api::Session> api,
    std::string_view endpoint) noexcept
    : imp_([&] {
        const auto& asio = api->Network().ZeroMQ().Internal();
        const auto batchID = asio.PreallocateBatch();
        // TODO the version of libc++ present in android ndk 23.0.7599858
        // has a broken std::allocate_shared function so we're using
        // boost::shared_ptr instead of std::shared_ptr

        return boost::allocate_shared<Imp>(
            alloc::PMR<Imp>{asio.Alloc(batchID)}, api, endpoint, batchID);
    }())
{
    OT_ASSERT(imp_);
}

auto BalanceOracle::Start() noexcept -> void { imp_->Init(imp_); }

BalanceOracle::~BalanceOracle() = default;
}  // namespace opentxs::api::crypto::blockchain
