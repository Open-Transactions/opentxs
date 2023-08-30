// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/blockchain/protocol/bitcoin/Types.hpp"  // IWYU pragma: associated

#include "internal/core/Amount.hpp"
#include "opentxs/core/Amount.hpp"

namespace opentxs::blockchain::protocol::bitcoin
{
auto amount_to_native_signed(const Amount& in) noexcept
    -> std::optional<std::int64_t>
{
    try {

        return in.Internal().ExtractInt64();
    } catch (...) {

        return std::nullopt;
    }
}

auto amount_to_native_unsigned(const Amount& in) noexcept
    -> std::optional<std::uint64_t>
{
    try {

        return in.Internal().ExtractUInt64();
    } catch (...) {

        return std::nullopt;
    }
}

auto native_to_amount(std::int64_t in) noexcept -> Amount { return in; }

auto native_to_amount(std::uint64_t in) noexcept -> Amount { return in; }
}  // namespace opentxs::blockchain::protocol::bitcoin
