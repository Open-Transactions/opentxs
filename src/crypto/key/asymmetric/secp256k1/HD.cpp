// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "internal/crypto/key/Factory.hpp"  // IWYU pragma: associated

#include <HDPath.pb.h>
#include <exception>

#include "crypto/key/asymmetric/secp256k1/Secp256k1.hpp"
#include "internal/api/Crypto.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/crypto/key/Secp256k1.hpp"
#include "opentxs/crypto/key/asymmetric/Algorithm.hpp"
#include "opentxs/crypto/symmetric/Key.hpp"  // IWYU pragma: keep
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto Secp256k1Key(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const opentxs::Secret& privateKey,
    const opentxs::Secret& chainCode,
    const Data& publicKey,
    const proto::HDPath& path,
    const Bip32Fingerprint parent,
    const crypto::key::asymmetric::Role role,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::unique_ptr<crypto::key::Secp256k1>
{
    using ReturnType = crypto::key::implementation::Secp256k1;

    try {
        auto sessionKey = api.Crypto().Symmetric().Key(reason);

        return std::make_unique<ReturnType>(
            api,
            ecdsa,
            privateKey,
            chainCode,
            publicKey,
            path,
            parent,
            role,
            version,
            sessionKey,
            reason);
    } catch (const std::exception& e) {
        LogError()("opentxs::Factory::")(__func__)(
            ": Failed to generate key: ")(e.what())
            .Flush();

        return nullptr;
    }
}

auto Secp256k1Key(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const opentxs::Secret& privateKey,
    const opentxs::Secret& chainCode,
    const Data& publicKey,
    const proto::HDPath& path,
    const Bip32Fingerprint parent,
    const crypto::key::asymmetric::Role role,
    const VersionNumber version) noexcept
    -> std::unique_ptr<crypto::key::Secp256k1>
{
    using ReturnType = crypto::key::implementation::Secp256k1;

    try {
        return std::make_unique<ReturnType>(
            api,
            ecdsa,
            privateKey,
            chainCode,
            publicKey,
            path,
            parent,
            role,
            version);
    } catch (const std::exception& e) {
        LogError()("opentxs::Factory::")(__func__)(
            ": Failed to generate key: ")(e.what())
            .Flush();

        return nullptr;
    }
}

auto Secp256k1Key(
    const api::Session& api,
    const ReadView key,
    const ReadView chaincode) noexcept
    -> std::unique_ptr<crypto::key::Secp256k1>
{
    static const auto blank = api.Factory().Secret(0);
    static const auto path = proto::HDPath{};
    using Type = opentxs::crypto::key::asymmetric::Algorithm;

    return factory::Secp256k1Key(
        api,
        api.Crypto().Internal().EllipticProvider(Type::Secp256k1),
        blank,
        api.Factory().SecretFromBytes(chaincode),
        api.Factory().DataFromBytes(key),
        path,
        {},
        opentxs::crypto::key::asymmetric::Role::Sign,
        opentxs::crypto::key::EllipticCurve::DefaultVersion);
}
}  // namespace opentxs::factory
