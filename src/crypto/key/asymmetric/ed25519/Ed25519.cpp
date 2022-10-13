// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                               // IWYU pragma: associated
#include "crypto/key/asymmetric/ed25519/Ed25519.hpp"  // IWYU pragma: associated

#include <string_view>

#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/key/Ed25519.hpp"
#include "opentxs/crypto/key/asymmetric/Algorithm.hpp"
#include "util/Sodium.hpp"

namespace opentxs::crypto::key::implementation
{
Ed25519::Ed25519(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& serializedKey) noexcept(false)
    : ot_super(api, ecdsa, serializedKey)
{
}

Ed25519::Ed25519(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const crypto::key::asymmetric::Role role,
    const VersionNumber version,
    const PasswordPrompt& reason) noexcept(false)
    : ot_super(
          api,
          ecdsa,
          crypto::key::asymmetric::Algorithm::ED25519,
          role,
          version,
          reason)
{
    OT_ASSERT(blank_private() != plaintext_key_.Bytes());
}

Ed25519::Ed25519(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const opentxs::Secret& privateKey,
    const opentxs::Secret& chainCode,
    const Data& publicKey,
    const proto::HDPath& path,
    const Bip32Fingerprint parent,
    const crypto::key::asymmetric::Role role,
    const VersionNumber version,
    symmetric::Key& sessionKey,
    const PasswordPrompt& reason) noexcept(false)
    : ot_super(
          api,
          ecdsa,
          crypto::key::asymmetric::Algorithm::ED25519,
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

Ed25519::Ed25519(const Ed25519& rhs) noexcept
    : key::Ed25519()
    , ot_super(rhs)
{
}

Ed25519::Ed25519(const Ed25519& rhs, const ReadView newPublic) noexcept
    : key::Ed25519()
    , ot_super(rhs, newPublic)
{
}

Ed25519::Ed25519(const Ed25519& rhs, Secret&& newSecretKey) noexcept
    : key::Ed25519()
    , ot_super(rhs, std::move(newSecretKey))
{
}

auto Ed25519::blank_private() const noexcept -> ReadView
{
    static const auto blank = space(64);

    return reader(blank);
}

auto Ed25519::TransportKey(
    Data& publicKey,
    Secret& privateKey,
    const PasswordPrompt& reason) const noexcept -> bool
{
    auto lock = Lock{lock_};

    if (false == HasPublic()) { return false; }
    if (false == has_private(lock)) { return false; }

    return sodium::ToCurveKeypair(
        private_key(lock, reason),
        PublicKey(),
        privateKey.WriteInto(Secret::Mode::Mem),
        publicKey.WriteInto());
}
}  // namespace opentxs::crypto::key::implementation
