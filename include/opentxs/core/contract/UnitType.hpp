// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/core/contract/Types.hpp"  // IWYU pragma: associated

#include <cstdint>

#include "opentxs/Export.hpp"

namespace opentxs::contract
{
enum class UnitType : std::uint32_t {
    Error = 0,
    Currency = 1,
    Security = 2,
    Basket = 3,
};
}  // namespace opentxs::contract
