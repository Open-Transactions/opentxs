// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

// IWYU pragma: no_include "opentxs/crypto/HashType.hpp"

#include <cstddef>
#include <cstdint>
#include <iosfwd>

#include "opentxs/Export.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Bytes.hpp"

namespace opentxs::crypto
{
class OPENTXS_EXPORT HashingProvider
{
public:
    static auto StringToHashType(const String& inputString) noexcept
        -> crypto::HashType;
    static auto HashTypeToString(const crypto::HashType hashType) noexcept
        -> OTString;
    static auto HashSize(const crypto::HashType hashType) noexcept
        -> std::size_t;

    virtual auto Digest(
        const crypto::HashType hashType,
        const ReadView data,
        const AllocateOutput output) const noexcept -> bool = 0;
    virtual auto HMAC(
        const crypto::HashType hashType,
        const ReadView key,
        const ReadView data,
        const AllocateOutput output) const noexcept -> bool = 0;

    HashingProvider(const HashingProvider&) = delete;
    HashingProvider(HashingProvider&&) = delete;
    auto operator=(const HashingProvider&) -> HashingProvider& = delete;
    auto operator=(HashingProvider&&) -> HashingProvider& = delete;

    virtual ~HashingProvider() = default;

protected:
    HashingProvider() = default;
};
}  // namespace opentxs::crypto
