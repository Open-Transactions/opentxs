// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "internal/blockchain/p2p/P2P.hpp"  // IWYU pragma: associated

namespace opentxs::factory
{
auto BlockchainAddress(
    const api::Session&,
    const blockchain::p2p::Protocol,
    const blockchain::p2p::Network,
    const Data&,
    const std::uint16_t,
    const blockchain::Type,
    const Time,
    const UnallocatedSet<blockchain::p2p::Service>&,
    const bool) noexcept -> blockchain::p2p::Address
{
    return {};
}

auto BlockchainAddress(
    const api::Session&,
    const proto::BlockchainPeerAddress&) noexcept -> blockchain::p2p::Address
{
    return {};
}
}  // namespace opentxs::factory
