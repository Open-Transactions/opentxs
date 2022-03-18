// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/peermanager/PeerManager.hpp"  // IWYU pragma: associated

#include <memory>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace network
{
namespace asio
{
class Socket;
}  // namespace asio
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)
namespace opentxs::blockchain::node::implementation
{
class PeerManager::IncomingConnectionManager
{
public:
    static auto TCP(
        const api::Session& api,
        PeerManager::Peers& parent) noexcept
        -> std::unique_ptr<IncomingConnectionManager>;
    static auto ZMQ(
        const api::Session& api,
        PeerManager::Peers& parent) noexcept
        -> std::unique_ptr<IncomingConnectionManager>;

    virtual auto Disconnect(const int peer) const noexcept -> void = 0;
    virtual auto Listen(const blockchain::p2p::Address& address) const noexcept
        -> bool = 0;

    virtual auto LookupIncomingSocket(const int id) noexcept(false)
        -> opentxs::network::asio::Socket = 0;
    virtual auto Shutdown() noexcept -> void = 0;

    virtual ~IncomingConnectionManager() = default;

protected:
    PeerManager::Peers& parent_;

    IncomingConnectionManager(PeerManager::Peers& parent) noexcept
        : parent_(parent)
    {
    }

private:
    IncomingConnectionManager() = delete;
    IncomingConnectionManager(const IncomingConnectionManager&) = delete;
    IncomingConnectionManager(IncomingConnectionManager&&) = delete;
    auto operator=(const IncomingConnectionManager&)
        -> IncomingConnectionManager& = delete;
    auto operator=(IncomingConnectionManager&&)
        -> IncomingConnectionManager& = delete;
};
}  // namespace opentxs::blockchain::node::implementation
