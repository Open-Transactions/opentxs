// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <type_traits>

#include "opentxs/crypto/symmetric/Types.hpp"  // IWYU pragma: keep

namespace opentxs::crypto::symmetric
{
enum class Algorithm : std::underlying_type_t<Algorithm> {
    Error = 0,
    ChaCha20Poly1305 = 1,
};
}  // namespace opentxs::crypto::symmetric
