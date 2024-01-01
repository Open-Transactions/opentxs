// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/crypto/asymmetric/Factory.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/HDPath.pb.h>
#include <cstddef>
#include <stdexcept>
#include <utility>

#include "crypto/asymmetric/key/secp256k1/Imp.hpp"
#include "crypto/asymmetric/key/secp256k1/Secp256k1Private.hpp"
#include "internal/api/Crypto.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/asymmetric/Algorithm.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/Role.hpp"       // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"
#include "opentxs/crypto/asymmetric/key/Secp256k1.hpp"
#include "opentxs/crypto/symmetric/Key.hpp"  // IWYU pragma: keep
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto Secp256k1Key(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const opentxs::Secret& privateKey,
    const opentxs::Secret& chainCode,
    const Data& publicKey,
    const protobuf::HDPath& path,
    const crypto::Bip32Fingerprint parent,
    const crypto::asymmetric::Role role,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason,
    alloc::Default alloc) noexcept -> crypto::asymmetric::key::Secp256k1
{
    using ReturnType = crypto::asymmetric::key::implementation::Secp256k1;
    using BlankType = crypto::asymmetric::key::Secp256k1Private;

    try {
        // TODO use alloc::Strategy::work_
        std::byte b[512_uz];  // NOLINT(modernize-avoid-c-arrays)
        auto mono = alloc::MonotonicUnsync{std::addressof(b), sizeof(b)};
        auto sessionKey =
            api.Crypto().Symmetric().Key(reason, {std::addressof(mono)});

        return pmr::construct<ReturnType>(
            alloc,
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
        LogError()()(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc);
    }
}

auto Secp256k1Key(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const opentxs::Secret& privateKey,
    const opentxs::Secret& chainCode,
    const Data& publicKey,
    const protobuf::HDPath& path,
    const crypto::Bip32Fingerprint parent,
    const crypto::asymmetric::Role role,
    const VersionNumber version,
    alloc::Default alloc) noexcept -> crypto::asymmetric::key::Secp256k1
{
    using ReturnType = crypto::asymmetric::key::implementation::Secp256k1;
    using BlankType = crypto::asymmetric::key::Secp256k1Private;

    try {

        return pmr::construct<ReturnType>(
            alloc,
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
        LogError()()(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc);
    }
}

auto Secp256k1Key(
    const api::Session& api,
    const ReadView key,
    const ReadView chaincode,
    alloc::Default alloc) noexcept -> crypto::asymmetric::key::Secp256k1
{
    using ReturnType = crypto::asymmetric::key::implementation::Secp256k1;
    using BlankType = crypto::asymmetric::key::Secp256k1Private;

    try {
        static const auto blank = api.Factory().Secret(0);
        static const auto path = protobuf::HDPath{};
        using Type = opentxs::crypto::asymmetric::Algorithm;

        return pmr::construct<ReturnType>(
            alloc,
            api,
            api.Crypto().Internal().EllipticProvider(Type::Secp256k1),
            blank,
            api.Factory().SecretFromBytes(chaincode),
            api.Factory().DataFromBytes(key),
            path,
            crypto::Bip32Fingerprint{},
            crypto::asymmetric::Role::Sign,
            crypto::asymmetric::key::EllipticCurve::DefaultVersion());
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc);
    }
}
}  // namespace opentxs::factory
