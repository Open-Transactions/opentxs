// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                   // IWYU pragma: associated
#include "1_Internal.hpp"                 // IWYU pragma: associated
#include "IncomingConnectionManager.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <tuple>
#include <utility>

#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/p2p/Address.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/network/asio/Socket.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameIterator.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/Router.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::implementation
{
class ZMQIncomingConnectionManager final
    : public PeerManager::IncomingConnectionManager
{
public:
    using Task = node::internal::PeerManager::Task;

    auto Disconnect(const int id) const noexcept -> void final
    {
        auto lock = Lock{lock_};
        auto it = peers_.find(id);

        if (peers_.end() == it) { return; }

        const auto& [externalID, internalID, registered, cached] = it->second;
        external_index_.erase(externalID);
        internal_index_.erase(internalID);
        peers_.erase(it);
    }
    auto Listen(const p2p::Address& address) const noexcept -> bool final
    {
        if (p2p::Network::zmq != address.Type()) {
            LogError()(OT_PRETTY_CLASS())("Invalid address").Flush();

            return false;
        }

        return external_->Start(
            address.Bytes()->str() + ':' + std::to_string(address.Port()));
    }

    auto LookupIncomingSocket(const int id) noexcept(false)
        -> opentxs::network::asio::Socket final
    {
        throw std::runtime_error{"ZMQ does not use asio sockets"};
    }
    auto Shutdown() noexcept -> void final {}

    ZMQIncomingConnectionManager(
        const api::Session& api,
        PeerManager::Peers& parent) noexcept
        : IncomingConnectionManager(parent)
        , api_(api)
        , lock_()
        , peers_()
        , external_index_()
        , internal_index_()
        , cb_ex_(zmq::ListenCallback::Factory(
              [=](auto&& in) { this->external(std::move(in)); }))
        , cb_int_(zmq::ListenCallback::Factory(
              [=](auto&& in) { this->internal(std::move(in)); }))
        , external_(api_.Network().ZeroMQ().RouterSocket(
              cb_ex_,
              zmq::socket::Direction::Bind))
        , internal_(api_.Network().ZeroMQ().RouterSocket(
              cb_int_,
              zmq::socket::Direction::Bind))
    {
    }

private:
    using ConnectionID = OTData;
    using CachedMessages = std::queue<network::zeromq::Message>;
    using PeerData =
        std::tuple<ConnectionID, ConnectionID, bool, CachedMessages>;
    using Peers = UnallocatedMap<int, PeerData>;
    using PeerIndex = UnallocatedMap<ConnectionID, int>;

    const api::Session& api_;
    mutable std::mutex lock_;
    mutable Peers peers_;
    mutable PeerIndex external_index_;
    mutable PeerIndex internal_index_;
    OTZMQListenCallback cb_ex_;
    OTZMQListenCallback cb_int_;
    OTZMQRouterSocket external_;
    OTZMQRouterSocket internal_;

    auto external(zmq::Message&& message) noexcept -> void
    {
        const auto header = message.Header();
        const auto body = message.Body();

        if (0 == header.size()) {
            LogError()(OT_PRETTY_CLASS())(
                "Rejecting message with missing header")
                .Flush();

            return;
        }

        if (0 == body.size()) {
            LogError()(OT_PRETTY_CLASS())("Rejecting message with missing body")
                .Flush();

            return;
        }

        if (Task::P2P != body.at(0).as<Task>()) {
            LogError()(OT_PRETTY_CLASS())("Rejecting invalid message type")
                .Flush();

            return;
        }

        const auto& idFrame = header.at(0);
        const auto id = api_.Factory().Data(idFrame);
        auto index = external_index_.find(id);

        if (external_index_.end() != index) {
            auto lock = Lock{lock_};
            auto& [externalID, internalID, registered, cached] =
                peers_.at(index->second);

            if (registered) {
                forward_message(internalID, std::move(message));
            } else {
                cached.emplace(std::move(message));
            }
        } else {
            const auto inproc = network::zeromq::MakeArbitraryInproc();
            const auto port =
                params::Data::Chains().at(parent_.chain_).default_port_;
            const auto zmq = inproc + ':' + std::to_string(port);
            auto address = factory::BlockchainAddress(
                api_,
                p2p::Protocol::bitcoin,
                p2p::Network::zmq,
                api_.Factory().Data(inproc, StringStyle::Raw),
                port,
                parent_.chain_,
                {},
                {},
                true);

            if (false == internal_->Start(zmq)) {
                LogError()(OT_PRETTY_CLASS())(
                    "Failed to listen to internal endpoint")
                    .Flush();

                return;
            }

            auto lock = Lock{lock_};
            const auto peerID = parent_.ConstructPeer(std::move(address));

            if (-1 == peerID) {
                LogError()(OT_PRETTY_CLASS())("Failed to instantiate peer")
                    .Flush();

                return;
            }

            external_index_.try_emplace(id, peerID);
            auto [it, added] = peers_.try_emplace(
                peerID, id, api_.Factory().Data(), false, CachedMessages{});

            OT_ASSERT(added);

            auto& [externalID, internalID, registered, cached] = it->second;
            cached.emplace(std::move(message));
        }
    }
    auto forward_message(
        const Data& internalID,
        zmq::Message&& message) noexcept -> void
    {
        internal_->Send([&] {
            auto out = network::zeromq::Message{};
            out.AddFrame(internalID);
            out.StartBody();

            for (auto& frame : message.Body()) {
                out.AddFrame(std::move(frame));
            }

            return out;
        }());
    }
    auto internal(zmq::Message&& message) noexcept -> void
    {
        const auto header = message.Header();

        OT_ASSERT(0 < header.size());

        auto incomingID = api_.Factory().Data(header.at(0));
        auto body = message.Body();

        OT_ASSERT(0 < body.size());

        switch (body.at(0).as<Task>()) {
            case Task::Register: {
                OT_ASSERT(1 < body.size());

                const auto peerID = body.at(1).as<int>();
                auto lock = Lock{lock_};

                try {
                    auto& [externalID, internalID, registered, cached] =
                        peers_.at(peerID);
                    internalID = incomingID;
                    registered = true;
                    internal_->Send(network::zeromq::tagged_reply_to_message(
                        message, Task::Register));

                    while (0u < cached.size()) {
                        forward_message(internalID, std::move(cached.front()));
                        cached.pop();
                    }

                    internal_index_.try_emplace(std::move(incomingID), peerID);
                } catch (...) {

                    return;
                }
            } break;
            case Task::P2P: {
                OT_ASSERT(2 < body.size());

                try {
                    const auto externalID = [&] {
                        auto lock = Lock{lock_};
                        auto& [eID, internalID, registered, cached] =
                            peers_.at(internal_index_.at(incomingID));

                        return eID;
                    }();
                    external_->Send([&] {
                        auto out = network::zeromq::Message{};
                        out.AddFrame(externalID);
                        out.StartBody();

                        for (auto& frame : body) {
                            out.AddFrame(std::move(frame));
                        }

                        return out;
                    }());
                } catch (...) {

                    return;
                }
            } break;
            default: {
                OT_FAIL;
            }
        }
    }

    ZMQIncomingConnectionManager() = delete;
    ZMQIncomingConnectionManager(const ZMQIncomingConnectionManager&) = delete;
    ZMQIncomingConnectionManager(ZMQIncomingConnectionManager&&) = delete;
    auto operator=(const ZMQIncomingConnectionManager&)
        -> ZMQIncomingConnectionManager& = delete;
    auto operator=(ZMQIncomingConnectionManager&&)
        -> ZMQIncomingConnectionManager& = delete;
};

auto PeerManager::IncomingConnectionManager::ZMQ(
    const api::Session& api,
    PeerManager::Peers& parent) noexcept
    -> std::unique_ptr<IncomingConnectionManager>
{
    return std::make_unique<ZMQIncomingConnectionManager>(api, parent);
}
}  // namespace opentxs::blockchain::node::implementation
