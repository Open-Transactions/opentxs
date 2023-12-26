// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace token
{
struct Descriptor;
}  // namespace token
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::token
{
enum class Type : std::uint32_t;  // IWYU pragma: export

OPENTXS_EXPORT auto print(Type) noexcept -> std::string_view;
OPENTXS_EXPORT auto token_to_unit(const Descriptor&) noexcept
    -> opentxs::UnitType;
OPENTXS_EXPORT auto unit_to_token(opentxs::UnitType) noexcept
    -> const Descriptor&;
}  // namespace opentxs::blockchain::token
