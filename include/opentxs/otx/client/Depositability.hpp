// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <type_traits>

#include "opentxs/otx/client/Types.hpp"  // IWYU pragma: keep

namespace opentxs::otx::client
{
enum class Depositability : std::underlying_type_t<Depositability> {
    ACCOUNT_NOT_SPECIFIED = -4,
    WRONG_ACCOUNT = -3,
    WRONG_RECIPIENT = -2,
    INVALID_INSTRUMENT = -1,
    READY = 0,
    NOT_REGISTERED = 1,
    NO_ACCOUNT = 2,
    UNKNOWN = 127,
};
}  // namespace opentxs::otx::client
