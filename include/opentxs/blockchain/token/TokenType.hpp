// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <limits>
#include <type_traits>

#include "opentxs/blockchain/token/Types.hpp"

namespace opentxs::blockchain::token
{
enum class Type : std::underlying_type<Type>::type {
    null = 0,
    slp = 1,
    cashtoken = 2,
    unknown = std::numeric_limits<std::underlying_type<Type>::type>::max(),
};
}  // namespace opentxs::blockchain::token
