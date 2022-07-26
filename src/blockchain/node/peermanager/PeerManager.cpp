// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/peermanager/PeerManager.hpp"  // IWYU pragma: associated

#include <atomic>
#include <chrono>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>

#include "core/Worker.hpp"
#include "internal/api/network/Blockchain.hpp"
#include "internal/blockchain/bitcoin/block/Transaction.hpp"
#include "internal/blockchain/node/Factory.hpp"
#include "internal/blockchain/p2p/P2P.hpp"  // IWYU pragma: keep
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/bitcoin/block/Transaction.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/p2p/Address.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace opentxs::factory
{
auto BlockchainPeerManager(
    const api::Session& api,
    const blockchain::node::internal::Config& config,
    const blockchain::node::internal::Mempool& mempool,
    const blockchain::node::Manager& node,
    const blockchain::node::HeaderOracle& headers,
    const blockchain::node::FilterOracle& filter,
    const blockchain::node::BlockOracle& block,
    blockchain::database::Peer& database,
    const blockchain::Type type,
    std::string_view seednode,
    std::string_view shutdown) noexcept
    -> std::unique_ptr<blockchain::node::internal::PeerManager>
{
    using ReturnType = blockchain::node::implementation::PeerManager;

    return std::make_unique<ReturnType>(
        api,
        config,
        mempool,
        node,
        headers,
        filter,
        block,
        database,
        type,
        seednode,
        shutdown);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::node::implementation
{
PeerManager::PeerManager(
    const api::Session& api,
    const internal::Config& config,
    const node::internal::Mempool& mempool,
    const node::Manager& node,
    const node::HeaderOracle& headers,
    const node::FilterOracle& filter,
    const node::BlockOracle& block,
    database::Peer& database,
    const Type chain,
    std::string_view seednode,
    std::string_view shutdown) noexcept
    : internal::PeerManager()
    , Worker(api, 100ms)
    , node_(node)
    , database_(database)
    , chain_(chain)
    , jobs_(api)
    , peers_(api, config, node_, database_, *this, shutdown, chain, seednode)
    , verified_lock_()
    , verified_peers_()
    , init_promise_()
    , init_(init_promise_.get_future())
{
    init_executor({UnallocatedCString{shutdown}});
}

auto PeerManager::AddIncomingPeer(const int id, std::uintptr_t endpoint)
    const noexcept -> void
{
    auto work = MakeWork(Work::IncomingPeer);
    work.AddFrame(id);
    work.AddFrame(endpoint);
    pipeline_.Push(std::move(work));
}

auto PeerManager::AddPeer(
    const blockchain::p2p::Address& address) const noexcept -> bool
{
    if (false == running_.load()) { return false; }

    auto address_p = std::make_unique<OTBlockchainAddress>(address);
    auto promise = std::make_unique<std::promise<bool>>();
    auto future = promise->get_future();
    auto work = MakeWork(Work::AddPeer);
    work.AddFrame(reinterpret_cast<std::uintptr_t>(address_p.release()));
    work.AddFrame(reinterpret_cast<std::uintptr_t>(promise.release()));
    pipeline_.Push(std::move(work));

    while (running_.load()) {
        if (std::future_status::ready == future.wait_for(5s)) {

            return future.get();
        }
    }

    return false;
}

auto PeerManager::BroadcastTransaction(
    const bitcoin::block::Transaction& tx) const noexcept -> bool
{
    if (false == running_.load()) { return false; }

    if (0 == peers_.Count()) { return false; }

    auto bytes = Space{};

    if (false == tx.Internal().Serialize(writer(bytes)).has_value()) {
        return false;
    }

    const auto view = reader(bytes);
    auto work = jobs_.Work(PeerManagerJobs::BroadcastTransaction);
    work.AddFrame(view.data(), view.size());
    jobs_.Dispatch(std::move(work));

    return true;
}

auto PeerManager::Connect() noexcept -> bool
{
    if (false == running_.load()) { return false; }

    trigger();

    return true;
}

auto PeerManager::Disconnect(const int id) const noexcept -> void
{
    auto work = MakeWork(Work::Disconnect);
    work.AddFrame(id);
    pipeline_.Push(std::move(work));
}

auto PeerManager::GetVerifiedPeerCount() const noexcept -> std::size_t
{
    auto lock = Lock{verified_lock_};

    return verified_peers_.size();
}

auto PeerManager::JobReady(const PeerManagerJobs type) const noexcept -> void
{
    switch (type) {
        case PeerManagerJobs::JobAvailableCfheaders: {
            jobs_.Dispatch(jobs_.Work(PeerManagerJobs::JobAvailableCfheaders));
        } break;
        case PeerManagerJobs::JobAvailableCfilters: {
            jobs_.Dispatch(jobs_.Work(PeerManagerJobs::JobAvailableCfilters));
        } break;
        case PeerManagerJobs::JobAvailableBlock: {
            jobs_.Dispatch(jobs_.Work(PeerManagerJobs::JobAvailableBlock));
        } break;
        default: {
        }
    }
}

auto PeerManager::Listen(const blockchain::p2p::Address& address) const noexcept
    -> bool
{
    if (false == running_.load()) { return false; }

    auto address_p = std::make_unique<OTBlockchainAddress>(address);
    auto promise = std::make_unique<std::promise<bool>>();
    auto future = promise->get_future();
    auto work = MakeWork(Work::AddListener);
    work.AddFrame(reinterpret_cast<std::uintptr_t>(address_p.release()));
    work.AddFrame(reinterpret_cast<std::uintptr_t>(promise.release()));
    pipeline_.Push(std::move(work));

    while (running_.load()) {
        if (std::future_status::ready == future.wait_for(10s)) {

            return future.get();
        } else {

            return false;
        }
    }

    return false;
}

auto PeerManager::LookupIncomingSocket(const int id) const noexcept(false)
    -> opentxs::network::asio::Socket
{
    return peers_.LookupIncomingSocket(id);
}

auto PeerManager::pipeline(zmq::Message&& message) noexcept -> void
{
    if (false == running_.load()) { return; }

    const auto body = message.Body();

    OT_ASSERT(0 < body.size());

    const auto work = [&] {
        try {

            return body.at(0).as<Work>();
        } catch (...) {

            OT_FAIL;
        }
    }();

    switch (work) {
        case Work::Disconnect: {
            OT_ASSERT(1 < body.size());

            const auto id = body.at(1).as<int>();

            {
                auto lock = Lock{verified_lock_};
                verified_peers_.erase(id);
            }

            peers_.Disconnect(id);
            api_.Network().Blockchain().Internal().UpdatePeer(chain_, "");
            do_work();
        } break;
        case Work::AddPeer: {
            OT_ASSERT(2 < body.size());

            using Promise = std::promise<bool>;

            auto address_p = std::unique_ptr<OTBlockchainAddress>{
                reinterpret_cast<OTBlockchainAddress*>(
                    body.at(1).as<std::uintptr_t>())};
            auto promise_p = std::unique_ptr<Promise>{
                reinterpret_cast<Promise*>(body.at(2).as<std::uintptr_t>())};

            OT_ASSERT(address_p);
            OT_ASSERT(promise_p);

            const auto& address = address_p->get();
            auto& promise = *promise_p;

            peers_.AddPeer(address, promise);
            do_work();
        } break;
        case Work::AddListener: {
            OT_ASSERT(2 < body.size());

            using Promise = std::promise<bool>;

            auto address_p = std::unique_ptr<OTBlockchainAddress>{
                reinterpret_cast<OTBlockchainAddress*>(
                    body.at(1).as<std::uintptr_t>())};
            auto promise_p = std::unique_ptr<Promise>{
                reinterpret_cast<Promise*>(body.at(2).as<std::uintptr_t>())};

            OT_ASSERT(address_p);
            OT_ASSERT(promise_p);

            const auto& address = address_p->get();
            auto& promise = *promise_p;

            peers_.AddListener(address, promise);
            do_work();
        } break;
        case Work::IncomingPeer: {
            OT_ASSERT(2 < body.size());

            const auto id = body.at(1).as<int>();
            auto endpoint = peermanager::Peers::Endpoint{
                reinterpret_cast<blockchain::p2p::internal::Address*>(
                    body.at(2).as<std::uintptr_t>())};

            OT_ASSERT(0 <= id);
            OT_ASSERT(endpoint);

            peers_.AddIncoming(id, std::move(endpoint));
            do_work();
        } break;
        case Work::StateMachine: {
            do_work();
        } break;
        case Work::Shutdown: {
            shutdown(shutdown_promise_);
        } break;
        default: {
            OT_FAIL;
        }
    }
}

auto PeerManager::RequestBlock(const block::Hash& block) const noexcept -> bool
{
    if (block.empty()) { return false; }

    return RequestBlocks({block.Bytes()});
}

auto PeerManager::RequestBlocks(
    const UnallocatedVector<ReadView>& hashes) const noexcept -> bool
{
    if (false == running_.load()) { return false; }

    if (0 == peers_.Count()) { return false; }

    if (0 == hashes.size()) { return false; }

    auto work = jobs_.Work(PeerManagerJobs::Getblock);
    static constexpr auto limit =
        1_uz;  // TODO peers don't respond well to larger values

    for (const auto& block : hashes) {
        work.AddFrame(block.data(), block.size());

        if (work.Body().size() > limit) {
            jobs_.Dispatch(std::move(work));
            work = jobs_.Work(PeerManagerJobs::Getblock);
        }
    }

    if (work.Body().size() > 1u) { jobs_.Dispatch(std::move(work)); }

    return true;
}

auto PeerManager::RequestHeaders() const noexcept -> bool
{
    if (false == running_.load()) { return false; }

    if (0 == peers_.Count()) { return false; }

    jobs_.Dispatch(PeerManagerJobs::Getheaders);

    return true;
}

auto PeerManager::shutdown(std::promise<void>& promise) noexcept -> void
{
    if (auto previous = running_.exchange(false); previous) {
        init_.get();
        pipeline_.Close();
        jobs_.Shutdown();
        peers_.Shutdown();
        promise.set_value();
    }
}

auto PeerManager::Start() noexcept -> void
{
    init_promise_.set_value();
    trigger();
}

auto PeerManager::state_machine() noexcept -> bool
{
    LogTrace()(OT_PRETTY_CLASS()).Flush();

    if (false == running_.load()) { return false; }

    return peers_.Run();
}

auto PeerManager::VerifyPeer(const int id, const UnallocatedCString& address)
    const noexcept -> void
{
    {
        auto lock = Lock{verified_lock_};
        verified_peers_.emplace(id);
    }

    api_.Network().Blockchain().Internal().UpdatePeer(chain_, address);
}

PeerManager::~PeerManager() { signal_shutdown().get(); }
}  // namespace opentxs::blockchain::node::implementation
