// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

// IWYU pragma: no_include "opentxs/crypto/HashType.hpp"

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstdint>
#include <iosfwd>

#include "opentxs/core/String.hpp"
#include "opentxs/crypto/Types.hpp"

namespace opentxs::crypto
{
class OPENTXS_EXPORT HashingProvider
{
public:
    static auto StringToHashType(const String& inputString) -> crypto::HashType;
    static auto HashTypeToString(const crypto::HashType hashType) -> OTString;
    static auto HashSize(const crypto::HashType hashType) -> std::size_t;

    virtual auto Digest(
        const crypto::HashType hashType,
        const std::uint8_t* input,
        const std::size_t inputSize,
        std::uint8_t* output) const -> bool = 0;
    virtual auto HMAC(
        const crypto::HashType hashType,
        const std::uint8_t* input,
        const std::size_t inputSize,
        const std::uint8_t* key,
        const std::size_t keySize,
        std::uint8_t* output) const -> bool = 0;

    virtual ~HashingProvider() = default;

protected:
    HashingProvider() = default;

private:
    HashingProvider(const HashingProvider&) = delete;
    HashingProvider(HashingProvider&&) = delete;
    auto operator=(const HashingProvider&) -> HashingProvider& = delete;
    auto operator=(HashingProvider&&) -> HashingProvider& = delete;
};
}  // namespace opentxs::crypto
