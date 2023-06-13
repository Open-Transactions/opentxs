// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/Factory.hpp"  // IWYU pragma: associated

#include "opentxs/network/blockchain/Address.hpp"

namespace opentxs::factory
{
auto BlockchainAddress(
    const api::Session&,
    const network::blockchain::Protocol,
    const network::blockchain::Transport,
    const network::blockchain::Transport,
    const ReadView,
    const std::uint16_t,
    const blockchain::Type,
    const Time,
    const Set<network::blockchain::bitcoin::Service>&,
    const bool,
    const ReadView) noexcept -> network::blockchain::Address
{
    return {};
}

auto BlockchainAddress(
    const api::Session&,
    const network::blockchain::Protocol,
    const network::blockchain::Transport,
    const network::blockchain::Transport,
    const ReadView,
    const ReadView,
    const std::uint16_t,
    const blockchain::Type,
    const Time,
    const Set<network::blockchain::bitcoin::Service>&,
    const bool,
    const ReadView) noexcept -> network::blockchain::Address
{
    return {};
}

auto BlockchainAddress(
    const api::Session&,
    const proto::BlockchainPeerAddress&) noexcept
    -> network::blockchain::Address
{
    return {};
}
}  // namespace opentxs::factory
