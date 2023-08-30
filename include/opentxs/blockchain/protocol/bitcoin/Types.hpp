// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <optional>

#include "opentxs/Export.hpp"  // IWYU pragma: keep

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Amount;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::protocol::bitcoin
{
OPENTXS_EXPORT auto amount_to_native_signed(const Amount&) noexcept
    -> std::optional<std::int64_t>;
OPENTXS_EXPORT auto amount_to_native_unsigned(const Amount&) noexcept
    -> std::optional<std::uint64_t>;
OPENTXS_EXPORT auto native_to_amount(std::int64_t) noexcept -> Amount;
OPENTXS_EXPORT auto native_to_amount(std::uint64_t) noexcept -> Amount;
}  // namespace opentxs::blockchain::protocol::bitcoin
