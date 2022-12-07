// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/Export.hpp"
#include "opentxs/crypto/Types.hpp"

namespace opentxs::crypto
{
enum class SecretStyle : std::uint8_t {
    Default = 0,
    X_only = 1,
};
}  // namespace opentxs::crypto
