// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace proto
{
class Ciphertext;
}  // namespace proto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto
{
class SymmetricProvider
{
public:
    virtual auto Decrypt(
        const proto::Ciphertext& ciphertext,
        const std::uint8_t* key,
        const std::size_t keySize,
        std::uint8_t* plaintext) const -> bool = 0;
    virtual auto DefaultMode() const
        -> opentxs::crypto::key::symmetric::Algorithm = 0;
    virtual auto Derive(
        const std::uint8_t* input,
        const std::size_t inputSize,
        const std::uint8_t* salt,
        const std::size_t saltSize,
        const std::uint64_t operations,
        const std::uint64_t difficulty,
        const std::uint64_t parallel,
        const crypto::key::symmetric::Source type,
        std::uint8_t* output,
        std::size_t outputSize) const -> bool = 0;
    virtual auto Encrypt(
        const std::uint8_t* input,
        const std::size_t inputSize,
        const std::uint8_t* key,
        const std::size_t keySize,
        proto::Ciphertext& ciphertext) const -> bool = 0;
    virtual auto IvSize(const opentxs::crypto::key::symmetric::Algorithm mode)
        const -> std::size_t = 0;
    virtual auto KeySize(const opentxs::crypto::key::symmetric::Algorithm mode)
        const -> std::size_t = 0;
    virtual auto SaltSize(const crypto::key::symmetric::Source type) const
        -> std::size_t = 0;
    virtual auto TagSize(const opentxs::crypto::key::symmetric::Algorithm mode)
        const -> std::size_t = 0;

    SymmetricProvider(const SymmetricProvider&) = delete;
    SymmetricProvider(SymmetricProvider&&) = delete;
    auto operator=(const SymmetricProvider&) -> SymmetricProvider& = delete;
    auto operator=(SymmetricProvider&&) -> SymmetricProvider& = delete;

    virtual ~SymmetricProvider() = default;

protected:
    SymmetricProvider() = default;
};
}  // namespace opentxs::crypto
