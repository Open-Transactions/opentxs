// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"               // IWYU pragma: associated
#include "api/crypto/Asymmetric.hpp"  // IWYU pragma: associated

#include <AsymmetricKey.pb.h>
#include <Enums.pb.h>
#include <HDPath.pb.h>
#include <memory>
#include <utility>

#include "internal/api/Crypto.hpp"
#include "internal/api/crypto/Factory.hpp"
#include "internal/crypto/asymmetric/Factory.hpp"
#include "internal/crypto/key/Key.hpp"
#include "internal/crypto/library/EcdsaProvider.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/crypto/Parameters.hpp"
#include "opentxs/crypto/asymmetric/Algorithm.hpp"
#include "opentxs/crypto/asymmetric/Key.hpp"
#include "opentxs/crypto/asymmetric/Role.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/crypto/asymmetric/key/Ed25519.hpp"
#include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"
#include "opentxs/crypto/asymmetric/key/HD.hpp"
#include "opentxs/crypto/asymmetric/key/RSA.hpp"
#include "opentxs/crypto/asymmetric/key/Secp256k1.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto AsymmetricAPI(const api::Session& api) noexcept
    -> std::unique_ptr<api::crypto::Asymmetric>
{
    using ReturnType = api::crypto::imp::Asymmetric;

    return std::make_unique<ReturnType>(api);
}
}  // namespace opentxs::factory

namespace opentxs::api::crypto::imp
{
const VersionNumber Asymmetric::serialized_path_version_{1};

const Asymmetric::TypeMap Asymmetric::curve_to_key_type_{
    {opentxs::crypto::EcdsaCurve::invalid,
     opentxs::crypto::asymmetric::Algorithm::Error},
    {opentxs::crypto::EcdsaCurve::secp256k1,
     opentxs::crypto::asymmetric::Algorithm::Secp256k1},
    {opentxs::crypto::EcdsaCurve::ed25519,
     opentxs::crypto::asymmetric::Algorithm::ED25519},
};

Asymmetric::Asymmetric(const api::Session& api) noexcept
    : api_(api)
{
}

template <typename ReturnType>
auto Asymmetric::instantiate_hd_key(
    const opentxs::crypto::asymmetric::Algorithm type,
    const UnallocatedCString& seedID,
    const opentxs::crypto::Bip32::Key& serialized,
    const opentxs::crypto::asymmetric::Role role,
    const VersionNumber version,
    const PasswordPrompt& reason,
    alloc::Default alloc) const noexcept -> ReturnType
{
    const auto& [privkey, ccode, pubkey, path, parent] = serialized;

    switch (type) {
        case opentxs::crypto::asymmetric::Algorithm::ED25519: {
            return factory::Ed25519Key(
                api_,
                api_.Crypto().Internal().EllipticProvider(type),
                privkey,
                ccode,
                pubkey,
                serialize_path(seedID, path),
                parent,
                role,
                version,
                reason,
                alloc);
        }
        case opentxs::crypto::asymmetric::Algorithm::Secp256k1: {
            return factory::Secp256k1Key(
                api_,
                api_.Crypto().Internal().EllipticProvider(type),
                privkey,
                ccode,
                pubkey,
                serialize_path(seedID, path),
                parent,
                role,
                version,
                reason,
                alloc);
        }
        case opentxs::crypto::asymmetric::Algorithm::Error:
        case opentxs::crypto::asymmetric::Algorithm::Null:
        case opentxs::crypto::asymmetric::Algorithm::Legacy:
        default: {
            LogError()(OT_PRETTY_CLASS())("Invalid key type: ")(print(type))
                .Flush();

            return {alloc};
        }
    }
}

template <typename ReturnType>
auto Asymmetric::instantiate_serialized_key(
    const proto::AsymmetricKey& serialized,
    alloc::Default alloc) const noexcept -> ReturnType

{
    const auto type = translate(serialized.type());
    using Type = opentxs::crypto::asymmetric::Algorithm;

    switch (type) {
        case Type::ED25519: {
            return factory::Ed25519Key(
                api_,
                api_.Crypto().Internal().EllipticProvider(type),
                serialized,
                alloc);
        }
        case Type::Secp256k1: {
            return factory::Secp256k1Key(
                api_,
                api_.Crypto().Internal().EllipticProvider(type),
                serialized,
                alloc);
        }
        case Type::Error:
        case Type::Null:
        case Type::Legacy:
        default: {
            LogError()(OT_PRETTY_CLASS())("Invalid key type: ")(print(type))
                .Flush();

            return {alloc};
        }
    }
}

auto Asymmetric::InstantiateECKey(
    const proto::AsymmetricKey& serialized,
    alloc::Default alloc) const noexcept
    -> opentxs::crypto::asymmetric::key::EllipticCurve
{
    using ReturnType = opentxs::crypto::asymmetric::key::EllipticCurve;

    switch (serialized.type()) {
        case proto::AKEYTYPE_ED25519:
        case proto::AKEYTYPE_SECP256K1: {

            return instantiate_serialized_key<ReturnType>(serialized, alloc);
        }
        case proto::AKEYTYPE_LEGACY: {
            LogError()(OT_PRETTY_CLASS())("Wrong key type (RSA)").Flush();
        } break;
        case proto::AKEYTYPE_ERROR:
        case proto::AKEYTYPE_NULL:
        default: {
        }
    }

    return {alloc};
}

auto Asymmetric::InstantiateHDKey(
    const proto::AsymmetricKey& serialized,
    alloc::Default alloc) const noexcept -> opentxs::crypto::asymmetric::key::HD
{
    using ReturnType = opentxs::crypto::asymmetric::key::HD;

    switch (serialized.type()) {
        case proto::AKEYTYPE_ED25519:
        case proto::AKEYTYPE_SECP256K1: {

            return instantiate_serialized_key<ReturnType>(serialized, alloc);
        }
        case proto::AKEYTYPE_LEGACY: {
            LogError()(OT_PRETTY_CLASS())("Wrong key type (RSA)").Flush();
        } break;
        case proto::AKEYTYPE_ERROR:
        case proto::AKEYTYPE_NULL:
        default: {
        }
    }

    return {alloc};
}

auto Asymmetric::InstantiateKey(
    const opentxs::crypto::asymmetric::Algorithm type,
    const UnallocatedCString& seedID,
    const opentxs::crypto::Bip32::Key& serialized,
    const PasswordPrompt& reason,
    alloc::Default alloc) const noexcept -> opentxs::crypto::asymmetric::key::HD
{
    return InstantiateKey(
        type,
        seedID,
        serialized,
        opentxs::crypto::asymmetric::Role::Sign,
        opentxs::crypto::asymmetric::key::EllipticCurve::DefaultVersion(),
        reason,
        alloc);
}

auto Asymmetric::InstantiateKey(
    const opentxs::crypto::asymmetric::Algorithm type,
    const UnallocatedCString& seedID,
    const opentxs::crypto::Bip32::Key& serialized,
    const opentxs::crypto::asymmetric::Role role,
    const PasswordPrompt& reason,
    alloc::Default alloc) const noexcept -> opentxs::crypto::asymmetric::key::HD
{
    return InstantiateKey(
        type,
        seedID,
        serialized,
        role,
        opentxs::crypto::asymmetric::key::EllipticCurve::DefaultVersion(),
        reason,
        alloc);
}

auto Asymmetric::InstantiateKey(
    const opentxs::crypto::asymmetric::Algorithm type,
    const UnallocatedCString& seedID,
    const opentxs::crypto::Bip32::Key& serialized,
    const VersionNumber version,
    const PasswordPrompt& reason,
    alloc::Default alloc) const noexcept -> opentxs::crypto::asymmetric::key::HD
{
    return InstantiateKey(
        type,
        seedID,
        serialized,
        opentxs::crypto::asymmetric::Role::Sign,
        version,
        reason,
        alloc);
}

auto Asymmetric::InstantiateKey(
    const opentxs::crypto::asymmetric::Algorithm type,
    const UnallocatedCString& seedID,
    const opentxs::crypto::Bip32::Key& serialized,
    const opentxs::crypto::asymmetric::Role role,
    const VersionNumber version,
    const PasswordPrompt& reason,
    alloc::Default alloc) const noexcept -> opentxs::crypto::asymmetric::key::HD
{
    using ReturnType = opentxs::crypto::asymmetric::key::HD;

    return instantiate_hd_key<ReturnType>(
        type, seedID, serialized, role, version, reason, alloc);
}

auto Asymmetric::InstantiateKey(
    const proto::AsymmetricKey& serialized,
    alloc::Default alloc) const noexcept -> opentxs::crypto::asymmetric::Key
{
    const auto type = translate(serialized.type());
    using Type = opentxs::crypto::asymmetric::Algorithm;
    using ReturnType = opentxs::crypto::asymmetric::Key;

    switch (type) {
        case Type::ED25519:
        case Type::Secp256k1: {
            return instantiate_serialized_key<ReturnType>(serialized, alloc);
        }
        case Type::Legacy: {
            return factory::RSAKey(
                api_,
                api_.Crypto().Internal().AsymmetricProvider(type),
                serialized,
                alloc);
        }
        case Type::Error:
        case Type::Null:
        default: {
            LogError()(OT_PRETTY_CLASS())("Invalid key type: ")(print(type))
                .Flush();

            return {alloc};
        }
    }
}

auto Asymmetric::InstantiateSecp256k1Key(
    const ReadView publicKey,
    const PasswordPrompt& reason,
    alloc::Default alloc) const noexcept
    -> opentxs::crypto::asymmetric::key::Secp256k1
{
    return InstantiateSecp256k1Key(
        publicKey,
        opentxs::crypto::asymmetric::Role::Sign,
        opentxs::crypto::asymmetric::key::Secp256k1::DefaultVersion(),
        reason,
        alloc);
}

auto Asymmetric::InstantiateSecp256k1Key(
    const ReadView publicKey,
    const opentxs::crypto::asymmetric::Role role,
    const PasswordPrompt& reason,
    alloc::Default alloc) const noexcept
    -> opentxs::crypto::asymmetric::key::Secp256k1
{
    return InstantiateSecp256k1Key(
        publicKey,
        role,
        opentxs::crypto::asymmetric::key::Secp256k1::DefaultVersion(),
        reason,
        alloc);
}

auto Asymmetric::InstantiateSecp256k1Key(
    const ReadView publicKey,
    const VersionNumber version,
    const PasswordPrompt& reason,
    alloc::Default alloc) const noexcept
    -> opentxs::crypto::asymmetric::key::Secp256k1
{
    return InstantiateSecp256k1Key(
        publicKey,
        opentxs::crypto::asymmetric::Role::Sign,
        version,
        reason,
        alloc);
}

auto Asymmetric::InstantiateSecp256k1Key(
    const ReadView publicKey,
    const opentxs::crypto::asymmetric::Role role,
    const VersionNumber version,
    const PasswordPrompt& reason,
    alloc::Default alloc) const noexcept
    -> opentxs::crypto::asymmetric::key::Secp256k1
{
    static const auto blank = api_.Factory().Secret(0);
    using Type = opentxs::crypto::asymmetric::Algorithm;

    return factory::Secp256k1Key(
        api_,
        api_.Crypto().Internal().EllipticProvider(Type::Secp256k1),
        blank,
        api_.Factory().DataFromBytes(publicKey),
        role,
        version,
        reason,
        alloc);
}

auto Asymmetric::InstantiateSecp256k1Key(
    const Secret& priv,
    const PasswordPrompt& reason,
    alloc::Default alloc) const noexcept
    -> opentxs::crypto::asymmetric::key::Secp256k1
{
    return InstantiateSecp256k1Key(
        priv,
        opentxs::crypto::asymmetric::Role::Sign,
        opentxs::crypto::asymmetric::key::Secp256k1::DefaultVersion(),
        reason,
        alloc);
}

auto Asymmetric::InstantiateSecp256k1Key(
    const Secret& priv,
    const opentxs::crypto::asymmetric::Role role,
    const PasswordPrompt& reason,
    alloc::Default alloc) const noexcept
    -> opentxs::crypto::asymmetric::key::Secp256k1
{
    return InstantiateSecp256k1Key(
        priv,
        role,
        opentxs::crypto::asymmetric::key::Secp256k1::DefaultVersion(),
        reason,
        alloc);
}

auto Asymmetric::InstantiateSecp256k1Key(
    const Secret& priv,
    const VersionNumber version,
    const PasswordPrompt& reason,
    alloc::Default alloc) const noexcept
    -> opentxs::crypto::asymmetric::key::Secp256k1
{
    return InstantiateSecp256k1Key(
        priv, opentxs::crypto::asymmetric::Role::Sign, version, reason, alloc);
}

auto Asymmetric::InstantiateSecp256k1Key(
    const Secret& priv,
    const opentxs::crypto::asymmetric::Role role,
    const VersionNumber version,
    const PasswordPrompt& reason,
    alloc::Default alloc) const noexcept
    -> opentxs::crypto::asymmetric::key::Secp256k1
{
    auto pub = api_.Factory().Data();
    using Type = opentxs::crypto::asymmetric::Algorithm;
    const auto& ecdsa =
        api_.Crypto().Internal().EllipticProvider(Type::Secp256k1);

    if (false == ecdsa.ScalarMultiplyBase(priv.Bytes(), pub.WriteInto())) {
        LogError()(OT_PRETTY_CLASS())("Failed to calculate public key").Flush();

        return {alloc};
    }

    return factory::Secp256k1Key(
        api_, ecdsa, priv, pub, role, version, reason, alloc);
}

auto Asymmetric::NewHDKey(
    const UnallocatedCString& seedID,
    const Secret& seed,
    const opentxs::crypto::EcdsaCurve& curve,
    const opentxs::crypto::Bip32::Path& path,
    const PasswordPrompt& reason,
    alloc::Default alloc) const -> opentxs::crypto::asymmetric::key::HD
{
    return NewHDKey(
        seedID,
        seed,
        curve,
        path,
        opentxs::crypto::asymmetric::Role::Sign,
        opentxs::crypto::asymmetric::key::EllipticCurve::DefaultVersion(),
        reason,
        alloc);
}

auto Asymmetric::NewHDKey(
    const UnallocatedCString& seedID,
    const Secret& seed,
    const opentxs::crypto::EcdsaCurve& curve,
    const opentxs::crypto::Bip32::Path& path,
    const opentxs::crypto::asymmetric::Role role,
    const PasswordPrompt& reason,
    alloc::Default alloc) const -> opentxs::crypto::asymmetric::key::HD
{
    return NewHDKey(
        seedID,
        seed,
        curve,
        path,
        role,
        opentxs::crypto::asymmetric::key::EllipticCurve::DefaultVersion(),
        reason,
        alloc);
}

auto Asymmetric::NewHDKey(
    const UnallocatedCString& seedID,
    const Secret& seed,
    const opentxs::crypto::EcdsaCurve& curve,
    const opentxs::crypto::Bip32::Path& path,
    const VersionNumber version,
    const PasswordPrompt& reason,
    alloc::Default alloc) const -> opentxs::crypto::asymmetric::key::HD
{
    return NewHDKey(
        seedID,
        seed,
        curve,
        path,
        opentxs::crypto::asymmetric::Role::Sign,
        version,
        reason,
        alloc);
}

auto Asymmetric::NewHDKey(
    const UnallocatedCString& seedID,
    const Secret& seed,
    const opentxs::crypto::EcdsaCurve& curve,
    const opentxs::crypto::Bip32::Path& path,
    const opentxs::crypto::asymmetric::Role role,
    const VersionNumber version,
    const PasswordPrompt& reason,
    alloc::Default alloc) const -> opentxs::crypto::asymmetric::key::HD
{
    return InstantiateKey(
        curve_to_key_type_.at(curve),
        seedID,
        api_.Crypto().BIP32().DeriveKey(curve, seed, path),
        role,
        version,
        reason,
        alloc);
}

auto Asymmetric::NewKey(
    const opentxs::crypto::Parameters& params,
    const PasswordPrompt& reason,
    alloc::Default alloc) const -> opentxs::crypto::asymmetric::Key
{
    return NewKey(
        params,
        opentxs::crypto::asymmetric::Role::Sign,
        opentxs::crypto::asymmetric::Key::DefaultVersion(),
        reason,
        alloc);
}

auto Asymmetric::NewKey(
    const opentxs::crypto::Parameters& params,
    const opentxs::crypto::asymmetric::Role role,
    const PasswordPrompt& reason,
    alloc::Default alloc) const -> opentxs::crypto::asymmetric::Key
{
    return NewKey(
        params,
        role,
        opentxs::crypto::asymmetric::Key::DefaultVersion(),
        reason,
        alloc);
}

auto Asymmetric::NewKey(
    const opentxs::crypto::Parameters& params,
    const VersionNumber version,
    const PasswordPrompt& reason,
    alloc::Default alloc) const -> opentxs::crypto::asymmetric::Key
{
    return NewKey(
        params,
        opentxs::crypto::asymmetric::Role::Sign,
        version,
        reason,
        alloc);
}

auto Asymmetric::NewKey(
    const opentxs::crypto::Parameters& params,
    const opentxs::crypto::asymmetric::Role role,
    const VersionNumber version,
    const PasswordPrompt& reason,
    alloc::Default alloc) const -> opentxs::crypto::asymmetric::Key
{
    const auto type = params.Algorithm();
    using Type = opentxs::crypto::asymmetric::Algorithm;

    switch (type) {
        case (Type::ED25519): {
            return factory::Ed25519Key(
                api_,
                api_.Crypto().Internal().EllipticProvider(type),
                role,
                version,
                reason,
                alloc);
        }
        case (Type::Secp256k1): {
            return factory::Secp256k1Key(
                api_,
                api_.Crypto().Internal().EllipticProvider(type),
                role,
                version,
                reason,
                alloc);
        }
        case (Type::Legacy): {
            return factory::RSAKey(
                api_,
                api_.Crypto().Internal().AsymmetricProvider(type),
                role,
                version,
                params,
                reason,
                alloc);
        }
        case Type::Error:
        case Type::Null:
        default: {
            LogError()(OT_PRETTY_CLASS())("Invalid key type: ")(print(type))
                .Flush();

            return {alloc};
        }
    }
}

auto Asymmetric::NewSecp256k1Key(
    const UnallocatedCString& seedID,
    const Secret& seed,
    const opentxs::crypto::Bip32::Path& derive,
    const PasswordPrompt& reason,
    alloc::Default alloc) const -> opentxs::crypto::asymmetric::key::Secp256k1
{
    return NewSecp256k1Key(
        seedID,
        seed,
        derive,
        opentxs::crypto::asymmetric::Role::Sign,
        opentxs::crypto::asymmetric::key::Secp256k1::DefaultVersion(),
        reason,
        alloc);
}

auto Asymmetric::NewSecp256k1Key(
    const UnallocatedCString& seedID,
    const Secret& seed,
    const opentxs::crypto::Bip32::Path& derive,
    const opentxs::crypto::asymmetric::Role role,
    const PasswordPrompt& reason,
    alloc::Default alloc) const -> opentxs::crypto::asymmetric::key::Secp256k1
{
    return NewSecp256k1Key(
        seedID,
        seed,
        derive,
        role,
        opentxs::crypto::asymmetric::key::Secp256k1::DefaultVersion(),
        reason,
        alloc);
}

auto Asymmetric::NewSecp256k1Key(
    const UnallocatedCString& seedID,
    const Secret& seed,
    const opentxs::crypto::Bip32::Path& derive,
    const VersionNumber version,
    const PasswordPrompt& reason,
    alloc::Default alloc) const -> opentxs::crypto::asymmetric::key::Secp256k1
{
    return NewSecp256k1Key(
        seedID,
        seed,
        derive,
        opentxs::crypto::asymmetric::Role::Sign,
        version,
        reason,
        alloc);
}

auto Asymmetric::NewSecp256k1Key(
    const UnallocatedCString& seedID,
    const Secret& seed,
    const opentxs::crypto::Bip32::Path& derive,
    const opentxs::crypto::asymmetric::Role role,
    const VersionNumber version,
    const PasswordPrompt& reason,
    alloc::Default alloc) const -> opentxs::crypto::asymmetric::key::Secp256k1
{
    const auto serialized = api_.Crypto().BIP32().DeriveKey(
        opentxs::crypto::EcdsaCurve::secp256k1, seed, derive);
    const auto& [privkey, ccode, pubkey, path, parent] = serialized;
    using Type = opentxs::crypto::asymmetric::Algorithm;

    return factory::Secp256k1Key(
        api_,
        api_.Crypto().Internal().EllipticProvider(Type::Secp256k1),
        privkey,
        ccode,
        pubkey,
        serialize_path(seedID, path),
        parent,
        role,
        version,
        reason,
        alloc);
}

auto Asymmetric::serialize_path(
    const UnallocatedCString& seedID,
    const opentxs::crypto::Bip32::Path& children) -> proto::HDPath
{
    proto::HDPath output;
    output.set_version(serialized_path_version_);
    output.set_root(seedID);

    for (const auto& index : children) { output.add_child(index); }

    return output;
}

Asymmetric::~Asymmetric() = default;
}  // namespace opentxs::api::crypto::imp
