// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <future>

#include "BoostAsio.hpp"
#include "core/StateMachine.hpp"
#include "opentxs/blockchain/p2p/Address.hpp"
#include "opentxs/blockchain/p2p/Peer.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace p2p
{
class Address;
}  // namespace p2p
}  // namespace blockchain

namespace proto
{
class BlockchainPeerAddress;
}  // namespace proto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ba = boost::asio;
namespace ip = ba::ip;
using tcp = ip::tcp;

namespace opentxs::blockchain::p2p::internal
{
class Address
{
public:
    virtual auto Incoming() const noexcept -> bool = 0;
    virtual auto PreviousLastConnected() const noexcept -> Time = 0;
    virtual auto PreviousServices() const noexcept
        -> UnallocatedSet<Service> = 0;
    virtual auto Serialize(proto::BlockchainPeerAddress& out) const noexcept
        -> bool = 0;

    virtual auto AddService(const Service service) noexcept -> void = 0;
    virtual auto RemoveService(const Service service) noexcept -> void = 0;
    virtual auto SetIncoming(bool value) noexcept -> void = 0;
    virtual auto SetLastConnected(const Time& time) noexcept -> void = 0;
    virtual auto SetServices(const UnallocatedSet<Service>& services) noexcept
        -> void = 0;

    virtual ~Address() = default;
};
}  // namespace opentxs::blockchain::p2p::internal

namespace opentxs::factory
{
auto BlockchainAddress(
    const api::Session& api,
    const blockchain::p2p::Protocol protocol,
    const blockchain::p2p::Network network,
    const ReadView bytes,
    const std::uint16_t port,
    const blockchain::Type chain,
    const Time lastConnected,
    const UnallocatedSet<blockchain::p2p::Service>& services,
    const bool incoming) noexcept -> blockchain::p2p::Address;
auto BlockchainAddress(
    const api::Session& api,
    const proto::BlockchainPeerAddress& serialized) noexcept
    -> blockchain::p2p::Address;
}  // namespace opentxs::factory
