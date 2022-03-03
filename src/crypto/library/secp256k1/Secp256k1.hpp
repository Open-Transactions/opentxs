// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

extern "C" {
#include <secp256k1.h>
}
#include <cstddef>
#include <iosfwd>
#include <optional>

#include "Proto.hpp"
#include "crypto/library/AsymmetricProvider.hpp"
#include "crypto/library/EcdsaProvider.hpp"
#include "internal/crypto/library/Secp256k1.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/crypto/SecretStyle.hpp"
#include "opentxs/crypto/key/asymmetric/Role.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Bytes.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace crypto
{
class Util;
}  // namespace crypto

class Crypto;
class Session;
}  // namespace api

namespace crypto
{
namespace key
{
class Asymmetric;
class Parameters;
}  // namespace key

class Parameters;
}  // namespace crypto

class OTPassword;
class PasswordPrompt;
class Secret;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::implementation
{
class Secp256k1 final : virtual public crypto::Secp256k1,
                        public AsymmetricProvider,
                        public EcdsaProvider
{
public:
    auto PubkeyAdd(
        const ReadView pubkey,
        const ReadView scalar,
        const AllocateOutput result) const noexcept -> bool final;
    using AsymmetricProvider::RandomKeypair;
    auto RandomKeypair(
        const AllocateOutput privateKey,
        const AllocateOutput publicKey,
        const opentxs::crypto::key::asymmetric::Role role,
        const Parameters& options,
        const AllocateOutput params) const noexcept -> bool final;
    auto ScalarAdd(
        const ReadView lhs,
        const ReadView rhs,
        const AllocateOutput result) const noexcept -> bool final;
    auto ScalarMultiplyBase(const ReadView scalar, const AllocateOutput result)
        const noexcept -> bool final;
    auto SharedSecret(
        const ReadView publicKey,
        const ReadView privateKey,
        const SecretStyle style,
        Secret& secret) const noexcept -> bool final;
    auto Sign(
        const ReadView plaintext,
        const ReadView key,
        const crypto::HashType hash,
        const AllocateOutput signature) const -> bool final;
    auto SignDER(
        const ReadView plaintext,
        const ReadView key,
        const crypto::HashType hash,
        Space& signature) const noexcept -> bool final;
    auto Verify(
        const ReadView plaintext,
        const ReadView theKey,
        const ReadView signature,
        const crypto::HashType hashType) const -> bool final;

    void Init() final;

    Secp256k1(const api::Crypto& crypto, const api::crypto::Util& ssl) noexcept;

    ~Secp256k1() final;

private:
    static const std::size_t PrivateKeySize{32};
    static const std::size_t PublicKeySize{33};
    static bool Initialized_;

    secp256k1_context* context_;
    const api::crypto::Util& ssl_;

    static auto blank_private() noexcept -> ReadView;

    auto hash(const crypto::HashType type, const ReadView data) const
        noexcept(false) -> OTData;
    auto parsed_public_key(const ReadView bytes) const noexcept(false)
        -> ::secp256k1_pubkey;
    auto parsed_signature(const ReadView bytes) const noexcept(false)
        -> ::secp256k1_ecdsa_signature;

    Secp256k1() = delete;
    Secp256k1(const Secp256k1&) = delete;
    Secp256k1(Secp256k1&&) = delete;
    auto operator=(const Secp256k1&) -> Secp256k1& = delete;
    auto operator=(Secp256k1&&) -> Secp256k1& = delete;
};
}  // namespace opentxs::crypto::implementation
