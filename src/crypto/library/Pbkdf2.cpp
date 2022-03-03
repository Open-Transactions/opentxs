// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"               // IWYU pragma: associated
#include "1_Internal.hpp"             // IWYU pragma: associated
#include "crypto/library/Pbkdf2.hpp"  // IWYU pragma: associated

extern "C" {
#include "trezor/pbkdf2.h"
}

#include <cstdint>
#include <limits>

#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/crypto/library/HashingProvider.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::crypto::implementation
{
Pbkdf2::Pbkdf2() noexcept
    : pbkdf_lock_()
{
}

auto Pbkdf2::PKCS5_PBKDF2_HMAC(
    const void* input,
    const std::size_t inputSize,
    const void* salt,
    const std::size_t saltSize,
    const std::size_t iterations,
    const crypto::HashType hashType,
    const std::size_t bytes,
    void* output) const noexcept -> bool
{
    static_assert(sizeof(int) <= sizeof(std::size_t));
    static constexpr auto limit =
        static_cast<std::size_t>(std::numeric_limits<int>::max());

    if (inputSize > limit) {
        LogError()(OT_PRETTY_CLASS())("Input too large").Flush();

        return false;
    }

    if (saltSize > limit) {
        LogError()(OT_PRETTY_CLASS())("Salt too large").Flush();

        return false;
    }

    if (bytes > limit) {
        LogError()(OT_PRETTY_CLASS())("Requested output too large").Flush();

        return false;
    }

    if (iterations > std::numeric_limits<std::uint32_t>::max()) {
        LogError()(OT_PRETTY_CLASS())("Too many iterations").Flush();

        return false;
    }

    auto lock = Lock{pbkdf_lock_};

    switch (hashType) {
        case crypto::HashType::Sha256: {
            pbkdf2_hmac_sha256(
                static_cast<const std::uint8_t*>(input),
                static_cast<int>(inputSize),
                static_cast<const std::uint8_t*>(salt),
                static_cast<int>(saltSize),
                static_cast<std::uint32_t>(iterations),
                static_cast<std::uint8_t*>(output),
                static_cast<int>(bytes));
        } break;
        case crypto::HashType::Sha512: {
            pbkdf2_hmac_sha512(
                static_cast<const std::uint8_t*>(input),
                static_cast<int>(inputSize),
                static_cast<const std::uint8_t*>(salt),
                static_cast<int>(saltSize),
                static_cast<std::uint32_t>(iterations),
                static_cast<std::uint8_t*>(output),
                static_cast<int>(bytes));
        } break;
        default: {
            LogError()(OT_PRETTY_CLASS())("Error: invalid hash type: ")(
                HashingProvider::HashTypeToString(hashType))
                .Flush();

            return false;
        }
    }

    return true;
}
}  // namespace opentxs::crypto::implementation
