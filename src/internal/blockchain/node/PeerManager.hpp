// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <future>

#include "internal/blockchain/node/Types.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace bitcoin
{
namespace block
{
class Transaction;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Block;
class Hash;
}  // namespace block

namespace p2p
{
class Address;
}  // namespace p2p
}  // namespace blockchain

namespace network
{
namespace asio
{
class Socket;
}  // namespace asio
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::internal
{
class PeerManager
{
public:
    virtual auto AddIncomingPeer(const int id, const p2p::Address& endpoint)
        const noexcept -> void = 0;
    virtual auto AddPeer(const p2p::Address& address) const noexcept
        -> bool = 0;
    virtual auto BroadcastTransaction(
        const bitcoin::block::Transaction& tx) const noexcept -> bool = 0;
    virtual auto Connect() noexcept -> bool = 0;
    virtual auto Disconnect(const int id) const noexcept -> void = 0;
    virtual auto Endpoint(const PeerManagerJobs type) const noexcept
        -> UnallocatedCString = 0;
    virtual auto GetPeerCount() const noexcept -> std::size_t = 0;
    virtual auto GetVerifiedPeerCount() const noexcept -> std::size_t = 0;
    virtual auto Heartbeat() const noexcept -> void = 0;
    virtual auto JobReady(const PeerManagerJobs type) const noexcept
        -> void = 0;
    virtual auto Listen(const p2p::Address& address) const noexcept -> bool = 0;
    virtual auto LookupIncomingSocket(const int id) const noexcept(false)
        -> opentxs::network::asio::Socket = 0;
    virtual auto Nonce() const noexcept -> const p2p::bitcoin::Nonce& = 0;
    virtual auto VerifyPeer(const int id, const UnallocatedCString& address)
        const noexcept -> void = 0;

    virtual auto Resolve(std::string_view host, std::uint16_t post) noexcept
        -> void = 0;
    virtual auto Start() noexcept -> void = 0;
    virtual auto Shutdown() noexcept -> std::shared_future<void> = 0;

    virtual ~PeerManager() = default;
};
}  // namespace opentxs::blockchain::node::internal
