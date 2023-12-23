// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <type_traits>

#include "opentxs/identifier/Types.hpp"  // IWYU pragma: keep

namespace opentxs::identifier
{
enum class Algorithm : std::underlying_type_t<Algorithm> {
    invalid = 0,
    sha256 = 1,
    blake2b160 = 2,
    blake2b256 = 3,
};
}  // namespace opentxs::identifier
