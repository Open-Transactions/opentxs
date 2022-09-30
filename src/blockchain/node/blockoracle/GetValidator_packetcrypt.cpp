// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                            // IWYU pragma: associated
#include "blockchain/node/blockoracle/Shared.hpp"  // IWYU pragma: associated

#include "crypto/library/packetcrypt/PacketCrypt.hpp"
#include "internal/blockchain/block/Validator.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"

namespace opentxs::blockchain::node::internal
{
auto BlockOracle::Shared::get_validator(
    const blockchain::Type chain,
    const node::HeaderOracle& headers) noexcept
    -> std::unique_ptr<const block::Validator>
{
    if (Type::PKT != chain) { return std::make_unique<block::Validator>(); }

    return std::make_unique<opentxs::crypto::implementation::PacketCrypt>(
        headers);
}
}  // namespace opentxs::blockchain::node::internal
