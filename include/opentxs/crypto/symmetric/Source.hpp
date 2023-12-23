// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <type_traits>

#include "opentxs/crypto/symmetric/Types.hpp"  // IWYU pragma: keep

namespace opentxs::crypto::symmetric
{
enum class Source : std::underlying_type_t<Source> {
    Error = 0,
    Raw = 1,
    ECDH = 2,
    Argon2i = 3,
    Argon2id = 4,
};
}  // namespace opentxs::crypto::symmetric
