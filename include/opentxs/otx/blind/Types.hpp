// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/Amount.hpp"

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstdint>
#include <set>
#include <string>

namespace opentxs
{
class Amount;
}  // namespace opentxs

namespace opentxs::otx::blind
{
enum class CashType : std::uint8_t;
enum class PurseType : std::uint8_t;
enum class TokenState : std::uint8_t;

using Denomination = Amount;
using MintSeries = std::uint64_t;
}  // namespace opentxs::otx::blind

namespace opentxs
{
auto print(otx::blind::CashType) noexcept -> std::string;
auto supported_otx_token_types() noexcept
    -> std::pmr::set<otx::blind::CashType>;
}  // namespace opentxs
