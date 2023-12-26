// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <type_traits>

#include "opentxs/identity/Types.hpp"  // IWYU pragma: keep

namespace opentxs::identity
{
enum class SourceType : std::underlying_type_t<SourceType> {
    Error = 0,
    PubKey = 1,
    Bip47 = 2,
};
}  // namespace opentxs::identity
