// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <functional>

#include "opentxs/blockchain/Types.hpp"

namespace opentxs::blockchain::block
{
using Version = std::int32_t;
using Height = std::int64_t;
using Hash = blockchain::Hash;
using pHash = blockchain::pHash;
using Position = std::pair<Height, pHash>;
using Txid = blockchain::Hash;
using pTxid = blockchain::pHash;

OPENTXS_EXPORT auto BlankHash() noexcept -> pHash;
/// Use this to test for a chain reorg. lhs is the current position and rhs is
/// the new position.
OPENTXS_EXPORT auto operator>(const Position& lhs, const Position& rhs) noexcept
    -> bool;
}  // namespace opentxs::blockchain::block

namespace opentxs
{
OPENTXS_EXPORT auto print(const blockchain::block::Position&) noexcept
    -> UnallocatedCString;
}  // namespace opentxs

namespace std
{
template <>
struct OPENTXS_EXPORT hash<opentxs::blockchain::block::Position> {
    auto operator()(const opentxs::blockchain::block::Position& data)
        const noexcept -> std::size_t;
};
}  // namespace std
