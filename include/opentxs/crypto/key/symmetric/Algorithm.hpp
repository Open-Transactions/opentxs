// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/crypto/key/Types.hpp"  // IWYU pragma: associated

#include <cstdint>

#include "opentxs/Export.hpp"

namespace opentxs::crypto::key::symmetric
{
enum class Algorithm : std::uint8_t {
    Error = 0,
    ChaCha20Poly1305 = 1,
};

constexpr auto value(const Algorithm in) noexcept
{
    return static_cast<std::uint8_t>(in);
}
}  // namespace opentxs::crypto::key::symmetric
