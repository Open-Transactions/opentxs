// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/Amount.hpp"

#pragma once

#include <cstddef>
#include <cstdint>

#include "opentxs/Export.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Amount;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::blind
{
enum class CashType : std::uint8_t;    // IWYU pragma: export
enum class PurseType : std::uint8_t;   // IWYU pragma: export
enum class TokenState : std::uint8_t;  // IWYU pragma: export

using Denomination = Amount;
using MintSeries = std::uint64_t;
}  // namespace opentxs::otx::blind

namespace opentxs
{
auto print(otx::blind::CashType) noexcept -> UnallocatedCString;
auto supported_otx_token_types() noexcept
    -> UnallocatedSet<otx::blind::CashType>;
}  // namespace opentxs
