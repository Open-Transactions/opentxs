// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::crypto::EcdsaCurve
// IWYU pragma: no_forward_declare opentxs::crypto::asymmetric::Algorithm
// IWYU pragma: no_forward_declare opentxs::crypto::asymmetric::Role
// IWYU pragma: no_include "opentxs/crypto/asymmetric/Algorithm.hpp"
// IWYU pragma: no_include "opentxs/crypto/asymmetric/Key.hpp"
// IWYU pragma: no_include "opentxs/crypto/asymmetric/Role.hpp"
// IWYU pragma: no_include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"
// IWYU pragma: no_include "opentxs/crypto/asymmetric/key/HD.hpp"
// IWYU pragma: no_include "opentxs/crypto/asymmetric/key/Secp256k1.hpp"

#pragma once

#include <HDPath.pb.h>
#include <functional>
#include <memory>

#include "internal/api/crypto/Asymmetric.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "opentxs/api/crypto/Asymmetric.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace crypto
{
class Asymmetric;
}  // namespace crypto

class Session;
}  // namespace api

namespace crypto
{
namespace asymmetric
{
namespace key
{
class EllipticCurve;
class HD;
class Secp256k1;
}  // namespace key

class Key;
}  // namespace asymmetric

class Parameters;
}  // namespace crypto

namespace proto
{
class AsymmetricKey;
}  // namespace proto

class PasswordPrompt;
class Secret;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::crypto::imp
{
class Asymmetric final : virtual public api::crypto::internal::Asymmetric
{
public:
    auto API() const noexcept -> const api::Session& final { return api_; }
    auto InstantiateECKey(
        const proto::AsymmetricKey& serialized,
        alloc::Default alloc) const noexcept
        -> opentxs::crypto::asymmetric::key::EllipticCurve final;
    auto InstantiateHDKey(
        const proto::AsymmetricKey& serialized,
        alloc::Default alloc) const noexcept
        -> opentxs::crypto::asymmetric::key::HD final;
    auto InstantiateKey(
        const opentxs::crypto::asymmetric::Algorithm type,
        const UnallocatedCString& seedID,
        const opentxs::crypto::Bip32::Key& serialized,
        const PasswordPrompt& reason,
        alloc::Default alloc) const noexcept
        -> opentxs::crypto::asymmetric::key::HD final;
    auto InstantiateKey(
        const opentxs::crypto::asymmetric::Algorithm type,
        const UnallocatedCString& seedID,
        const opentxs::crypto::Bip32::Key& serialized,
        const opentxs::crypto::asymmetric::Role role,
        const PasswordPrompt& reason,
        alloc::Default alloc) const noexcept
        -> opentxs::crypto::asymmetric::key::HD final;
    auto InstantiateKey(
        const opentxs::crypto::asymmetric::Algorithm type,
        const UnallocatedCString& seedID,
        const opentxs::crypto::Bip32::Key& serialized,
        const VersionNumber version,
        const PasswordPrompt& reason,
        alloc::Default alloc) const noexcept
        -> opentxs::crypto::asymmetric::key::HD final;
    auto InstantiateKey(
        const opentxs::crypto::asymmetric::Algorithm type,
        const UnallocatedCString& seedID,
        const opentxs::crypto::Bip32::Key& serialized,
        const opentxs::crypto::asymmetric::Role role,
        const VersionNumber version,
        const PasswordPrompt& reason,
        alloc::Default alloc) const noexcept
        -> opentxs::crypto::asymmetric::key::HD final;
    auto InstantiateKey(
        const proto::AsymmetricKey& serialized,
        alloc::Default alloc) const noexcept
        -> opentxs::crypto::asymmetric::Key final;
    auto InstantiateSecp256k1Key(
        const ReadView publicKey,
        const PasswordPrompt& reason,
        alloc::Default alloc) const noexcept
        -> opentxs::crypto::asymmetric::key::Secp256k1 final;
    auto InstantiateSecp256k1Key(
        const ReadView publicKey,
        const opentxs::crypto::asymmetric::Role role,
        const PasswordPrompt& reason,
        alloc::Default alloc) const noexcept
        -> opentxs::crypto::asymmetric::key::Secp256k1 final;
    auto InstantiateSecp256k1Key(
        const ReadView publicKey,
        const VersionNumber version,
        const PasswordPrompt& reason,
        alloc::Default alloc) const noexcept
        -> opentxs::crypto::asymmetric::key::Secp256k1 final;
    auto InstantiateSecp256k1Key(
        const ReadView publicKey,
        const opentxs::crypto::asymmetric::Role role,
        const VersionNumber version,
        const PasswordPrompt& reason,
        alloc::Default alloc) const noexcept
        -> opentxs::crypto::asymmetric::key::Secp256k1 final;
    auto InstantiateSecp256k1Key(
        const Secret& privateKey,
        const PasswordPrompt& reason,
        alloc::Default alloc) const noexcept
        -> opentxs::crypto::asymmetric::key::Secp256k1 final;
    auto InstantiateSecp256k1Key(
        const Secret& privateKey,
        const opentxs::crypto::asymmetric::Role role,
        const PasswordPrompt& reason,
        alloc::Default alloc) const noexcept
        -> opentxs::crypto::asymmetric::key::Secp256k1 final;
    auto InstantiateSecp256k1Key(
        const Secret& privateKey,
        const VersionNumber version,
        const PasswordPrompt& reason,
        alloc::Default alloc) const noexcept
        -> opentxs::crypto::asymmetric::key::Secp256k1 final;
    auto InstantiateSecp256k1Key(
        const Secret& privateKey,
        const opentxs::crypto::asymmetric::Role role,
        const VersionNumber version,
        const PasswordPrompt& reason,
        alloc::Default alloc) const noexcept
        -> opentxs::crypto::asymmetric::key::Secp256k1 final;
    auto NewHDKey(
        const UnallocatedCString& seedID,
        const Secret& seed,
        const opentxs::crypto::EcdsaCurve& curve,
        const opentxs::crypto::Bip32::Path& path,
        const PasswordPrompt& reason,
        alloc::Default alloc) const
        -> opentxs::crypto::asymmetric::key::HD final;
    auto NewHDKey(
        const UnallocatedCString& seedID,
        const Secret& seed,
        const opentxs::crypto::EcdsaCurve& curve,
        const opentxs::crypto::Bip32::Path& path,
        const opentxs::crypto::asymmetric::Role role,
        const PasswordPrompt& reason,
        alloc::Default alloc) const
        -> opentxs::crypto::asymmetric::key::HD final;
    auto NewHDKey(
        const UnallocatedCString& seedID,
        const Secret& seed,
        const opentxs::crypto::EcdsaCurve& curve,
        const opentxs::crypto::Bip32::Path& path,
        const VersionNumber version,
        const PasswordPrompt& reason,
        alloc::Default alloc) const
        -> opentxs::crypto::asymmetric::key::HD final;
    auto NewHDKey(
        const UnallocatedCString& seedID,
        const Secret& seed,
        const opentxs::crypto::EcdsaCurve& curve,
        const opentxs::crypto::Bip32::Path& path,
        const opentxs::crypto::asymmetric::Role role,
        const VersionNumber version,
        const PasswordPrompt& reason,
        alloc::Default alloc) const
        -> opentxs::crypto::asymmetric::key::HD final;
    auto NewKey(
        const opentxs::crypto::Parameters& params,
        const PasswordPrompt& reason,
        alloc::Default alloc) const -> opentxs::crypto::asymmetric::Key final;
    auto NewKey(
        const opentxs::crypto::Parameters& params,
        const opentxs::crypto::asymmetric::Role role,
        const PasswordPrompt& reason,
        alloc::Default alloc) const -> opentxs::crypto::asymmetric::Key final;
    auto NewKey(
        const opentxs::crypto::Parameters& params,
        const VersionNumber version,
        const PasswordPrompt& reason,
        alloc::Default alloc) const -> opentxs::crypto::asymmetric::Key final;
    auto NewKey(
        const opentxs::crypto::Parameters& params,
        const opentxs::crypto::asymmetric::Role role,
        const VersionNumber version,
        const PasswordPrompt& reason,
        alloc::Default alloc) const -> opentxs::crypto::asymmetric::Key final;
    auto NewSecp256k1Key(
        const UnallocatedCString& seedID,
        const Secret& seed,
        const opentxs::crypto::Bip32::Path& path,
        const PasswordPrompt& reason,
        alloc::Default alloc) const
        -> opentxs::crypto::asymmetric::key::Secp256k1 final;
    auto NewSecp256k1Key(
        const UnallocatedCString& seedID,
        const Secret& seed,
        const opentxs::crypto::Bip32::Path& path,
        const opentxs::crypto::asymmetric::Role role,
        const PasswordPrompt& reason,
        alloc::Default alloc) const
        -> opentxs::crypto::asymmetric::key::Secp256k1 final;
    auto NewSecp256k1Key(
        const UnallocatedCString& seedID,
        const Secret& seed,
        const opentxs::crypto::Bip32::Path& path,
        const VersionNumber version,
        const PasswordPrompt& reason,
        alloc::Default alloc) const
        -> opentxs::crypto::asymmetric::key::Secp256k1 final;
    auto NewSecp256k1Key(
        const UnallocatedCString& seedID,
        const Secret& seed,
        const opentxs::crypto::Bip32::Path& path,
        const opentxs::crypto::asymmetric::Role role,
        const VersionNumber version,
        const PasswordPrompt& reason,
        alloc::Default alloc) const
        -> opentxs::crypto::asymmetric::key::Secp256k1 final;

    Asymmetric(const api::Session& api) noexcept;
    Asymmetric() = delete;
    Asymmetric(const Asymmetric&) = delete;
    Asymmetric(Asymmetric&&) = delete;
    auto operator=(const Asymmetric&) -> Asymmetric& = delete;
    auto operator=(Asymmetric&&) -> Asymmetric& = delete;

    ~Asymmetric() final;

private:
    using TypeMap = UnallocatedMap<
        opentxs::crypto::EcdsaCurve,
        opentxs::crypto::asymmetric::Algorithm>;

    static const VersionNumber serialized_path_version_;
    static const TypeMap curve_to_key_type_;

    const api::Session& api_;

    static auto serialize_path(
        const UnallocatedCString& seedID,
        const opentxs::crypto::Bip32::Path& children) -> proto::HDPath;

    template <typename ReturnType>
    auto instantiate_hd_key(
        const opentxs::crypto::asymmetric::Algorithm type,
        const UnallocatedCString& seedID,
        const opentxs::crypto::Bip32::Key& serialized,
        const opentxs::crypto::asymmetric::Role role,
        const VersionNumber version,
        const PasswordPrompt& reason,
        alloc::Default alloc) const noexcept -> ReturnType;
    template <typename ReturnType>
    auto instantiate_serialized_key(
        const proto::AsymmetricKey& serialized,
        alloc::Default alloc) const noexcept -> ReturnType;
};
}  // namespace opentxs::api::crypto::imp
