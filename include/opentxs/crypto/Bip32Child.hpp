// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/crypto/Types.hpp"

namespace opentxs
{
enum class Bip32Child : Bip32Index {
    AUTH_KEY = 0x41555448,
    ENCRYPT_KEY = 0x454e4352,
    SIGN_KEY = 0x5349474e,
    HARDENED = 0x80000000,
};
}  // namespace opentxs
