// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>

#include "crypto/library/AsymmetricProvider.hpp"
#include "crypto/library/EcdsaProvider.hpp"
#include "internal/api/crypto/Util.hpp"
#include "internal/crypto/library/Sodium.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/crypto/symmetric/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
}  // namespace api

namespace crypto
{
class Parameters;
}  // namespace crypto

namespace protobuf
{
class Ciphertext;
}  // namespace protobuf

class Secret;
class WriteBuffer;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::implementation
{
class Sodium final : virtual public crypto::Sodium,
                     public AsymmetricProvider,
                     public EcdsaProvider,
                     public api::crypto::internal::Util
{
public:
    auto Digest(
        const crypto::HashType hashType,
        const ReadView data,
        Writer&& output) const noexcept -> bool final;
    auto Generate(
        const ReadView input,
        const ReadView salt,
        const std::uint64_t N,
        const std::uint32_t r,
        const std::uint32_t p,
        const std::size_t bytes,
        Writer&& writer) const noexcept -> bool final;
    auto HMAC(
        const crypto::HashType hashType,
        const ReadView key,
        const ReadView data,
        Writer&& output) const noexcept -> bool final;
    auto PubkeyAdd(ReadView pubkey, ReadView scalar, Writer&& result)
        const noexcept -> bool final;
    using AsymmetricProvider::RandomKeypair;
    auto RandomKeypair(
        Writer&& privateKey,
        Writer&& publicKey,
        const opentxs::crypto::asymmetric::Role role,
        const Parameters& options,
        Writer&& params) const noexcept -> bool final;
    auto RandomizeMemory(void* destination, const std::size_t size) const
        -> bool final;
    auto ScalarAdd(ReadView lhs, ReadView rhs, Writer&& result) const noexcept
        -> bool final;
    auto ScalarMultiplyBase(ReadView scalar, Writer&& result) const noexcept
        -> bool final;
    auto SharedSecret(
        const ReadView publicKey,
        const ReadView privateKey,
        const SecretStyle style,
        Secret& secret) const noexcept -> bool final;
    auto Sign(
        const ReadView plaintext,
        const ReadView key,
        const crypto::HashType hash,
        Writer&& signature) const -> bool final;
    auto Verify(
        const ReadView plaintext,
        const ReadView theKey,
        const ReadView signature,
        const crypto::HashType hashType) const -> bool final;

    Sodium(const api::Crypto& crypto) noexcept;
    Sodium() = delete;
    Sodium(const Sodium&) = delete;
    Sodium(Sodium&&) = delete;
    auto operator=(const Sodium&) -> Sodium& = delete;
    auto operator=(Sodium&&) -> Sodium& = delete;

    ~Sodium() final = default;

private:
    static auto blank_private() noexcept -> ReadView;

    auto Decrypt(
        const protobuf::Ciphertext& ciphertext,
        const std::uint8_t* key,
        const std::size_t keySize,
        std::uint8_t* plaintext) const -> bool final;
    auto DefaultMode() const -> opentxs::crypto::symmetric::Algorithm final;
    auto Derive(
        const std::uint8_t* input,
        const std::size_t inputSize,
        const std::uint8_t* salt,
        const std::size_t saltSize,
        const std::uint64_t operations,
        const std::uint64_t difficulty,
        const std::uint64_t parallel,
        const crypto::symmetric::Source type,
        std::uint8_t* output,
        std::size_t outputSize) const -> bool final;
    auto Encrypt(
        const std::uint8_t* input,
        const std::size_t inputSize,
        const std::uint8_t* key,
        const std::size_t keySize,
        protobuf::Ciphertext& ciphertext) const -> bool final;
    auto IvSize(const opentxs::crypto::symmetric::Algorithm mode) const
        -> std::size_t final;
    auto KeySize(const opentxs::crypto::symmetric::Algorithm mode) const
        -> std::size_t final;
    auto SaltSize(const crypto::symmetric::Source type) const
        -> std::size_t final;
    auto sha1(const ReadView data, WriteBuffer& output) const -> bool;
    auto TagSize(const opentxs::crypto::symmetric::Algorithm mode) const
        -> std::size_t final;
};
}  // namespace opentxs::crypto::implementation
