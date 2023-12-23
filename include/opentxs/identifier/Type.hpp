// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <type_traits>

#include "opentxs/identifier/Types.hpp"  // IWYU pragma: keep

namespace opentxs::identifier
{
enum class Type : std::underlying_type_t<Type> {
    invalid = 0,
    generic = 1,
    nym = 2,
    notary = 3,
    unitdefinition = 4,
    account = 5,
    hdseed = 6,
};
}  // namespace opentxs::identifier
