// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

extern "C" {
#include <secp256k1.h>
}
#include <cstddef>

#include "crypto/library/AsymmetricProvider.hpp"
#include "crypto/library/EcdsaProvider.hpp"
#include "internal/crypto/library/Secp256k1.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace crypto
{
class Util;
}  // namespace crypto

class Crypto;
}  // namespace api

namespace crypto
{
class Parameters;
}  // namespace crypto

class ByteArray;
class Secret;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::implementation
{
class Secp256k1 final : virtual public crypto::Secp256k1,
                        public AsymmetricProvider,
                        public EcdsaProvider
{
public:
    auto PubkeyAdd(ReadView pubkey, ReadView scalar, Writer&& result)
        const noexcept -> bool final;
    using AsymmetricProvider::RandomKeypair;
    auto RandomKeypair(
        Writer&& privateKey,
        Writer&& publicKey,
        const opentxs::crypto::asymmetric::Role role,
        const Parameters& options,
        Writer&& params) const noexcept -> bool final;
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
    auto SignDER(
        ReadView plaintext,
        ReadView key,
        crypto::HashType hash,
        Writer&& signature) const noexcept -> bool final;
    auto Verify(
        const ReadView plaintext,
        const ReadView theKey,
        const ReadView signature,
        const crypto::HashType hashType) const -> bool final;

    using AsymmetricProvider::Init;
    void Init() final;

    Secp256k1(const api::Crypto& crypto, const api::crypto::Util& ssl) noexcept;
    Secp256k1() = delete;
    Secp256k1(const Secp256k1&) = delete;
    Secp256k1(Secp256k1&&) = delete;
    auto operator=(const Secp256k1&) -> Secp256k1& = delete;
    auto operator=(Secp256k1&&) -> Secp256k1& = delete;

    ~Secp256k1() final;

private:
    static const std::size_t PrivateKeySize{32};
    static const std::size_t PublicKeySize{33};
    static bool Initialized_;

    secp256k1_context* context_;
    const api::crypto::Util& ssl_;

    static auto blank_private() noexcept -> ReadView;

    auto hash(const crypto::HashType type, const ReadView data) const
        noexcept(false) -> ByteArray;
    auto parsed_public_key(const ReadView bytes) const noexcept(false)
        -> ::secp256k1_pubkey;
    auto parsed_signature(const ReadView bytes) const noexcept(false)
        -> ::secp256k1_ecdsa_signature;
};
}  // namespace opentxs::crypto::implementation
