// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/Types.hpp"

namespace opentxs::blockchain::protocol::bitcoin::base::block::script
{
enum class OP : std::uint8_t;        // IWYU pragma: export
enum class Pattern : std::uint8_t;   // IWYU pragma: export
enum class Position : std::uint8_t;  // IWYU pragma: export

OPENTXS_EXPORT auto print(blockchain::Type chain, OP) noexcept
    -> std::string_view;

constexpr auto value(OP i) noexcept { return static_cast<std::uint8_t>(i); }
}  // namespace opentxs::blockchain::protocol::bitcoin::base::block::script
