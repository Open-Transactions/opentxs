// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "blockchain/node/peermanager/PeerManager.hpp"  // IWYU pragma: associated

#include <BlockchainPeerAddress.pb.h>
#include <boost/asio.hpp>
#include <atomic>
#include <chrono>
#include <iterator>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "core/Worker.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/bitcoin/block/Transaction.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Factory.hpp"
#include "internal/blockchain/p2p/P2P.hpp"  // IWYU pragma: keep
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/bitcoin/block/Transaction.hpp"
#include "opentxs/blockchain/p2p/Address.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameIterator.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"

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
    const blockchain::node::Endpoints& endpoints) noexcept
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
        endpoints);
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
    const node::Endpoints& endpoints) noexcept
    : internal::PeerManager()
    , Worker(
          api,
          100ms,
          "blockchain::node::PeerManager",
          {},
          {},
          [&] {
              using Dir = opentxs::network::zeromq::socket::Direction;
              using Endpoints = api::session::internal::Endpoints;
              auto dealer = opentxs::network::zeromq::EndpointArgs{};
              dealer.emplace_back(Endpoints::Asio(), Dir::Connect);

              return dealer;
          }())
    , node_(node)
    , database_(database)
    , chain_(chain)
    , jobs_(api)
    , peers_(api, config, node_, database_, *this, endpoints, chain, seednode)
    , verified_lock_()
    , verified_peers_()
    , to_blockchain_api_([&] {
        using Type = opentxs::network::zeromq::socket::Type;
        auto out = api.Network().ZeroMQ().Internal().RawSocket(Type::Push);
        const auto endpoint = UnallocatedCString{
            api.Endpoints().Internal().Internal().BlockchainMessageRouter()};
        const auto rc = out.Connect(endpoint.c_str());

        OT_ASSERT(rc);

        return out;
    }())
    , init_promise_()
    , init_(init_promise_.get_future())
{
    init_executor(
        {api_.Endpoints().Internal().BlockchainReportStatus().data(),
         endpoints.shutdown_publish_.c_str()});
}

auto PeerManager::AddIncomingPeer(const int id, const p2p::Address& endpoint)
    const noexcept -> void
{
    try {
        const auto proto = [&] {
            auto out = proto::BlockchainPeerAddress{};

            if (false == endpoint.Internal().Serialize(out)) {
                throw std::runtime_error{
                    "failed to serialize address to protobuf"};
            }

            return out;
        }();
        auto work = MakeWork(Work::IncomingPeer);
        work.AddFrame(id);

        if (false == proto::write(proto, work.AppendBytes())) {
            throw std::runtime_error{"failed to serialize protobuf to bytes"};
        }

        pipeline_.Push(std::move(work));
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
    }
}

auto PeerManager::AddPeer(const p2p::Address& address) const noexcept -> bool
{
    if (false == running_.load()) { return false; }

    try {
        const auto proto = [&] {
            auto out = proto::BlockchainPeerAddress{};

            if (false == address.Internal().Serialize(out)) {
                throw std::runtime_error{
                    "failed to serialize address to protobuf"};
            }

            return out;
        }();
        auto work = MakeWork(Work::AddPeer);

        if (false == proto::write(proto, work.AppendBytes())) {
            throw std::runtime_error{"failed to serialize protobuf to bytes"};
        }

        return pipeline_.Push(std::move(work));
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
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
        case PeerManagerJobs::BroadcastTransaction:
        case PeerManagerJobs::Heartbeat:
        default: {
        }
    }
}

auto PeerManager::Listen(const p2p::Address& address) const noexcept -> bool
{
    if (false == running_.load()) { return false; }

    try {
        const auto proto = [&] {
            auto out = proto::BlockchainPeerAddress{};

            if (false == address.Internal().Serialize(out)) {
                throw std::runtime_error{
                    "failed to serialize address to protobuf"};
            }

            return out;
        }();
        auto work = MakeWork(Work::AddListener);

        if (false == proto::write(proto, work.AppendBytes())) {
            throw std::runtime_error{"failed to serialize protobuf to bytes"};
        }

        return pipeline_.Push(std::move(work));
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
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
        case Work::Shutdown: {
            shutdown(shutdown_promise_);
        } break;
        case Work::Resolve: {
            static constexpr auto success = std::byte{0x01};
            static constexpr auto ipv4 =
                sizeof(boost::asio::ip::address_v4::bytes_type);
            static constexpr auto ipv6 =
                sizeof(boost::asio::ip::address_v6::bytes_type);

            OT_ASSERT(2 < body.size());

            if (body.at(1).as<std::byte>() == success) {
                const auto port = body.at(2).as<std::uint16_t>();
                auto addresses = Vector<p2p::Address>{};

                for (auto i = std::next(body.begin(), 3), end = body.end();
                     i != end;
                     ++i) {
                    const auto& frame = *i;
                    const auto network = [&] {
                        switch (frame.size()) {
                            case ipv4: {

                                return p2p::Network::ipv4;
                            }
                            case ipv6: {

                                return p2p::Network::ipv6;
                            }
                            default: {
                                LogAbort()(OT_PRETTY_CLASS())(
                                    "invalid address size (")(frame.size())(")")
                                    .Abort();
                            }
                        }
                    }();
                    addresses.emplace_back(factory::BlockchainAddress(
                        api_,
                        params::get(chain_).P2PDefaultProtocol(),
                        network,
                        api_.Factory().DataFromBytes(frame.Bytes()),
                        port,
                        chain_,
                        Time{},
                        {},
                        false));
                }

                peers_.AddResolvedDNS(std::move(addresses));
                do_work();
            } else {
                LogError()(OT_PRETTY_CLASS())(body.at(2).Bytes()).Flush();
            }
        } break;
        case Work::Disconnect: {
            OT_ASSERT(1 < body.size());

            const auto id = body.at(1).as<int>();

            {
                auto lock = Lock{verified_lock_};
                verified_peers_.erase(id);
                report(lock);
            }

            peers_.Disconnect(id);
            do_work();
        } break;
        case Work::AddPeer: {
            OT_ASSERT(1 < body.size());

            const auto& bytes = body.at(1);
            auto address = api_.Factory().BlockchainAddress(
                proto::Factory<proto::BlockchainPeerAddress>(
                    bytes.data(), bytes.size()));

            if (address.IsValid()) {
                peers_.AddPeer(std::move(address));
            } else {
                LogError()(OT_PRETTY_CLASS())("invalid address").Flush();
            }

            do_work();
        } break;
        case Work::AddListener: {
            OT_ASSERT(1 < body.size());

            const auto& bytes = body.at(1);
            auto address = api_.Factory().BlockchainAddress(
                proto::Factory<proto::BlockchainPeerAddress>(
                    bytes.data(), bytes.size()));

            if (address.IsValid()) {
                peers_.AddListener(std::move(address));
            } else {
                LogError()(OT_PRETTY_CLASS())("invalid address").Flush();
            }

            do_work();
        } break;
        case Work::IncomingPeer: {
            OT_ASSERT(2 < body.size());

            const auto id = body.at(1).as<int>();

            OT_ASSERT(0 <= id);

            const auto& bytes = body.at(2);
            auto address = api_.Factory().BlockchainAddress(
                proto::Factory<proto::BlockchainPeerAddress>(
                    bytes.data(), bytes.size()));

            if (address.IsValid()) {
                peers_.AddIncoming(id, std::move(address));
            } else {
                LogError()(OT_PRETTY_CLASS())("invalid address").Flush();
            }

            do_work();
        } break;
        case Work::Report: {
            auto lock = Lock{verified_lock_};
            report(lock);
        } break;
        case Work::StateMachine: {
            do_work();
        } break;
        default: {
            OT_FAIL;
        }
    }
}

auto PeerManager::report(const Lock&, std::string_view address) const noexcept
    -> void
{
    to_blockchain_api_.SendDeferred(
        [&] {
            auto work = MakeWork(WorkType::BlockchainPeerAdded);
            work.AddFrame(chain_);
            work.AddFrame(address.data(), address.size());
            work.AddFrame(verified_peers_.size());

            return work;
        }(),
        __FILE__,
        __LINE__);
}

auto PeerManager::Resolve(std::string_view host, std::uint16_t post) noexcept
    -> void
{
    pipeline_.Internal().SendFromThread([&] {
        auto out = MakeWork(WorkType::AsioResolve);
        out.AddFrame(host.data(), host.size());
        out.AddFrame(post);

        return out;
    }());
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
    auto lock = Lock{verified_lock_};
    verified_peers_.emplace(id);
    report(lock, address);
}

PeerManager::~PeerManager() { signal_shutdown().get(); }
}  // namespace opentxs::blockchain::node::implementation
