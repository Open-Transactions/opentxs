// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "1_Internal.hpp"                // IWYU pragma: associated
#include "crypto/library/Ripemd160.hpp"  // IWYU pragma: associated

extern "C" {
#include "trezor/ripemd160.h"
}

#include <array>
#include <cstring>
#include <limits>

#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::crypto::implementation
{
Ripemd160::Ripemd160() noexcept
    : ripemd_lock_()
{
}

auto Ripemd160::RIPEMD160(
    const std::uint8_t* input,
    const std::size_t size,
    std::uint8_t* output) const -> bool
{
    using SizeType = std::uint32_t;

    if (size > std::numeric_limits<SizeType>::max()) {
        LogError()(OT_PRETTY_CLASS())("Input too large").Flush();

        return false;
    }

    auto lock = Lock{ripemd_lock_};
    auto hash = std::array<std::uint8_t, RIPEMD160_DIGEST_LENGTH>{};
    ::ripemd160(input, static_cast<SizeType>(size), hash.data());
    std::memcpy(output, hash.data(), hash.size());

    return true;
}
}  // namespace opentxs::crypto::implementation
