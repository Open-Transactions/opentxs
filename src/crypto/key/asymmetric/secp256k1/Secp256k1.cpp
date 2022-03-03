// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "crypto/key/asymmetric/secp256k1/Secp256k1.hpp"  // IWYU pragma: associated

#include <string_view>

#include "internal/util/LogMacros.hpp"
#include "opentxs/crypto/key/Secp256k1.hpp"
#include "opentxs/crypto/key/asymmetric/Algorithm.hpp"

namespace opentxs::crypto::key::implementation
{
Secp256k1::Secp256k1(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& serializedKey) noexcept(false)
    : ot_super(api, ecdsa, serializedKey)
{
}

Secp256k1::Secp256k1(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const crypto::key::asymmetric::Role role,
    const VersionNumber version,
    const PasswordPrompt& reason) noexcept(false)
    : ot_super(
          api,
          ecdsa,
          crypto::key::asymmetric::Algorithm::Secp256k1,
          role,
          version,
          reason)
{
    OT_ASSERT(blank_private() != plaintext_key_->Bytes());
}

Secp256k1::Secp256k1(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const opentxs::Secret& privateKey,
    const Data& publicKey,
    const crypto::key::asymmetric::Role role,
    const VersionNumber version,
    key::Symmetric& sessionKey,
    const opentxs::PasswordPrompt& reason) noexcept(false)
    : ot_super(
          api,
          ecdsa,
          crypto::key::asymmetric::Algorithm::Secp256k1,
          privateKey,
          publicKey,
          role,
          version,
          sessionKey,
          reason)
{
}

Secp256k1::Secp256k1(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const opentxs::Secret& privateKey,
    const opentxs::Secret& chainCode,
    const Data& publicKey,
    const proto::HDPath& path,
    const Bip32Fingerprint parent,
    const crypto::key::asymmetric::Role role,
    const VersionNumber version,
    key::Symmetric& sessionKey,
    const opentxs::PasswordPrompt& reason) noexcept(false)
    : ot_super(
          api,
          ecdsa,
          crypto::key::asymmetric::Algorithm::Secp256k1,
          privateKey,
          chainCode,
          publicKey,
          path,
          parent,
          role,
          version,
          sessionKey,
          reason)
{
}

Secp256k1::Secp256k1(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const opentxs::Secret& privateKey,
    const opentxs::Secret& chainCode,
    const Data& publicKey,
    const proto::HDPath& path,
    const Bip32Fingerprint parent,
    const crypto::key::asymmetric::Role role,
    const VersionNumber version) noexcept(false)
    : ot_super(
          api,
          ecdsa,
          crypto::key::asymmetric::Algorithm::Secp256k1,
          privateKey,
          chainCode,
          publicKey,
          path,
          parent,
          role,
          version)
{
}

Secp256k1::Secp256k1(const Secp256k1& rhs) noexcept
    : key::Secp256k1()
    , ot_super(rhs)
{
}

Secp256k1::Secp256k1(const Secp256k1& rhs, const ReadView newPublic) noexcept
    : key::Secp256k1()
    , ot_super(rhs, newPublic)
{
}

Secp256k1::Secp256k1(const Secp256k1& rhs, OTSecret&& newSecretKey) noexcept
    : key::Secp256k1()
    , ot_super(rhs, std::move(newSecretKey))
{
}

auto Secp256k1::blank_private() const noexcept -> ReadView
{
    static const auto blank = space(32);

    return reader(blank);
}
}  // namespace opentxs::crypto::key::implementation
