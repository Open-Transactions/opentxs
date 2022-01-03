// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"       // IWYU pragma: associated
#include "opentxs/crypto/Types.hpp"  // IWYU pragma: associated

#include <cstdint>

namespace opentxs::otx::blind
{
enum class TokenState : std::uint8_t {
    Error = 0,
    Blinded = 1,
    Signed = 2,
    Ready = 3,
    Spent = 4,
    Expired = 5,
};
}  // namespace opentxs::otx::blind