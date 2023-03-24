// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>

#include "internal/core/String.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace crypto
{
class Hasher;
}  // namespace crypto

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto
{
class HashingProvider
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
        Writer&& output) const noexcept -> bool = 0;
    virtual auto HMAC(
        const crypto::HashType hashType,
        const ReadView key,
        const ReadView data,
        Writer&& output) const noexcept -> bool = 0;
    virtual auto Hasher(const crypto::HashType hashType) const noexcept
        -> crypto::Hasher;

    HashingProvider(const HashingProvider&) = delete;
    HashingProvider(HashingProvider&&) = delete;
    auto operator=(const HashingProvider&) -> HashingProvider& = delete;
    auto operator=(HashingProvider&&) -> HashingProvider& = delete;

    virtual ~HashingProvider() = default;

protected:
    HashingProvider() = default;
};
}  // namespace opentxs::crypto
