// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"           // IWYU pragma: associated
#include "opentxs/crypto/key/Types.hpp"  // IWYU pragma: associated

#include <cstdint>

namespace opentxs::crypto::key::asymmetric
{
enum class Role : std::uint8_t {
    Error = 0,
    Auth = 1,
    Encrypt = 2,
    Sign = 3,
};
}  // namespace opentxs::crypto::key::asymmetric
