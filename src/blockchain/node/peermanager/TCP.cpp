// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                   // IWYU pragma: associated
#include "1_Internal.hpp"                 // IWYU pragma: associated
#include "IncomingConnectionManager.hpp"  // IWYU pragma: associated

#include <map>
#include <mutex>
#include <stdexcept>
#include <utility>
#include <vector>

#include "internal/blockchain/p2p/P2P.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/p2p/Address.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/asio/Endpoint.hpp"
#include "opentxs/network/asio/Socket.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace opentxs::blockchain::node::implementation
{
class TCPIncomingConnectionManager final
    : public PeerManager::IncomingConnectionManager
{
public:
    auto Disconnect(const int id) const noexcept -> void final
    {
        auto lock = Lock{lock_};
        sockets_.erase(id);
    }
    auto Listen(const p2p::Address& address) const noexcept -> bool final
    {
        const auto type = address.Type();
        const auto port = address.Port();
        const auto bytes = [&] {
            auto out = Space{};
            const auto data = address.Bytes();
            copy(data->Bytes(), writer(out));

            return out;
        }();

        try {
            auto lock = Lock{lock_};
            const auto& endpoint = [&]() -> auto&
            {
                using EndpointType = opentxs::network::asio::Endpoint;
                auto network = EndpointType::Type{};

                switch (type) {
                    case p2p::Network::ipv4: {
                        network = EndpointType::Type::ipv4;
                    } break;
                    case p2p::Network::ipv6: {
                        network = EndpointType::Type::ipv6;
                    } break;
                    default: {
                        throw std::runtime_error{"invalid address"};
                    }
                }

                return listeners_.emplace_back(network, reader(bytes), port);
            }
            ();
            const auto output =
                api_.Network().Asio().Accept(endpoint, [=](auto&& socket) {
                    accept(type, port, bytes, std::move(socket));
                });

            if (false == output) { listeners_.pop_back(); }

            return output;
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            return false;
        }
    }

    auto LookupIncomingSocket(const int id) noexcept(false)
        -> opentxs::network::asio::Socket final
    {
        auto lock = Lock{lock_};
        auto it = sockets_.find(id);

        if (sockets_.end() == it) {
            throw std::runtime_error{"socket not found"};
        }

        auto out{std::move(it->second)};

        sockets_.erase(it);

        return out;
    }
    auto Shutdown() noexcept -> void final
    {
        auto lock = Lock{lock_};

        for (const auto& endpoint : listeners_) {
            api_.Network().Asio().Close(endpoint);
        }

        listeners_.clear();
    }

    TCPIncomingConnectionManager(
        const api::Session& api,
        PeerManager::Peers& parent) noexcept
        : IncomingConnectionManager(parent)
        , api_(api)
        , lock_()
        , listeners_()
        , sockets_()
    {
    }

    ~TCPIncomingConnectionManager() final { Shutdown(); }

private:
    const api::Session& api_;
    mutable std::mutex lock_;
    mutable std::pmr::vector<opentxs::network::asio::Endpoint> listeners_;
    mutable std::pmr::map<int, opentxs::network::asio::Socket> sockets_;

    auto accept(
        p2p::Network type,
        std::uint16_t port,
        Space bytes,
        opentxs::network::asio::Socket&& socket) const noexcept -> void
    {
        auto lock = Lock{lock_};
        auto address = factory::BlockchainAddress(
            api_,
            p2p::Protocol::bitcoin,
            type,
            api_.Factory().Data(reader(bytes)),
            port,
            parent_.chain_,
            {},
            {},
            true);
        const auto peerID = parent_.ConstructPeer(std::move(address));

        if (-1 == peerID) {
            LogError()(OT_PRETTY_CLASS())("Failed to instantiate peer").Flush();

            return;
        }

        sockets_.try_emplace(peerID, std::move(socket));
    }

    TCPIncomingConnectionManager() = delete;
    TCPIncomingConnectionManager(const TCPIncomingConnectionManager&) = delete;
    TCPIncomingConnectionManager(TCPIncomingConnectionManager&&) = delete;
    auto operator=(const TCPIncomingConnectionManager&)
        -> TCPIncomingConnectionManager& = delete;
    auto operator=(TCPIncomingConnectionManager&&)
        -> TCPIncomingConnectionManager& = delete;
};

auto PeerManager::IncomingConnectionManager::TCP(
    const api::Session& api,
    PeerManager::Peers& parent) noexcept
    -> std::unique_ptr<IncomingConnectionManager>
{
    return std::make_unique<TCPIncomingConnectionManager>(api, parent);
}
}  // namespace opentxs::blockchain::node::implementation
