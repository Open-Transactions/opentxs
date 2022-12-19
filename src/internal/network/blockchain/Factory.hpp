// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace network
{
namespace blockchain
{
class Address;
}  // namespace blockchain
}  // namespace network

namespace proto
{
class BlockchainPeerAddress;
}  // namespace proto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
auto BlockchainAddress(
    const api::Session& api,
    const network::blockchain::Protocol protocol,
    const network::blockchain::Transport network,
    const ReadView bytes,
    const std::uint16_t port,
    const blockchain::Type chain,
    const Time lastConnected,
    const Set<network::blockchain::bitcoin::Service>& services,
    const bool incoming) noexcept -> network::blockchain::Address;
auto BlockchainAddress(
    const api::Session& api,
    const proto::BlockchainPeerAddress& serialized) noexcept
    -> network::blockchain::Address;
}  // namespace opentxs::factory
