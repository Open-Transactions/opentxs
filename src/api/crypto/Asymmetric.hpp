// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>
#include <memory>

#include "Proto.hpp"
#include "internal/api/crypto/Asymmetric.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/crypto/Asymmetric.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/crypto/key/Secp256k1.hpp"
#include "opentxs/crypto/key/asymmetric/Algorithm.hpp"
#include "opentxs/crypto/key/asymmetric/Role.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "serialization/protobuf/HDPath.pb.h"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
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
namespace key
{
class Asymmetric;
class EllipticCurve;
class HD;
class Secp256k1;
}  // namespace key

class Parameters;
}  // namespace crypto

namespace proto
{
class AsymmetricKey;
}  // namespace proto

class OTPassword;
class PasswordPrompt;
class Secret;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::crypto::imp
{
class Asymmetric final : virtual public api::crypto::internal::Asymmetric
{
public:
    auto API() const noexcept -> const api::Session& final { return api_; }
    auto InstantiateECKey(const proto::AsymmetricKey& serialized) const noexcept
        -> std::unique_ptr<opentxs::crypto::key::EllipticCurve> final;
    auto InstantiateHDKey(const proto::AsymmetricKey& serialized) const noexcept
        -> std::unique_ptr<opentxs::crypto::key::HD> final;
    auto InstantiateKey(
        const opentxs::crypto::key::asymmetric::Algorithm type,
        const UnallocatedCString& seedID,
        const opentxs::crypto::Bip32::Key& serialized,
        const PasswordPrompt& reason) const noexcept
        -> std::unique_ptr<opentxs::crypto::key::HD> final;
    auto InstantiateKey(
        const opentxs::crypto::key::asymmetric::Algorithm type,
        const UnallocatedCString& seedID,
        const opentxs::crypto::Bip32::Key& serialized,
        const opentxs::crypto::key::asymmetric::Role role,
        const PasswordPrompt& reason) const noexcept
        -> std::unique_ptr<opentxs::crypto::key::HD> final;
    auto InstantiateKey(
        const opentxs::crypto::key::asymmetric::Algorithm type,
        const UnallocatedCString& seedID,
        const opentxs::crypto::Bip32::Key& serialized,
        const VersionNumber version,
        const PasswordPrompt& reason) const noexcept
        -> std::unique_ptr<opentxs::crypto::key::HD> final;
    auto InstantiateKey(
        const opentxs::crypto::key::asymmetric::Algorithm type,
        const UnallocatedCString& seedID,
        const opentxs::crypto::Bip32::Key& serialized,
        const opentxs::crypto::key::asymmetric::Role role,
        const VersionNumber version,
        const PasswordPrompt& reason) const noexcept
        -> std::unique_ptr<opentxs::crypto::key::HD> final;
    auto InstantiateKey(const proto::AsymmetricKey& serialized) const noexcept
        -> std::unique_ptr<opentxs::crypto::key::Asymmetric> final;
    auto InstantiateSecp256k1Key(
        const ReadView publicKey,
        const PasswordPrompt& reason) const noexcept
        -> std::unique_ptr<opentxs::crypto::key::Secp256k1> final;
    auto InstantiateSecp256k1Key(
        const ReadView publicKey,
        const opentxs::crypto::key::asymmetric::Role role,
        const PasswordPrompt& reason) const noexcept
        -> std::unique_ptr<opentxs::crypto::key::Secp256k1> final;
    auto InstantiateSecp256k1Key(
        const ReadView publicKey,
        const VersionNumber version,
        const PasswordPrompt& reason) const noexcept
        -> std::unique_ptr<opentxs::crypto::key::Secp256k1> final;
    auto InstantiateSecp256k1Key(
        const ReadView publicKey,
        const opentxs::crypto::key::asymmetric::Role role,
        const VersionNumber version,
        const PasswordPrompt& reason) const noexcept
        -> std::unique_ptr<opentxs::crypto::key::Secp256k1> final;
    auto InstantiateSecp256k1Key(
        const Secret& privateKey,
        const PasswordPrompt& reason) const noexcept
        -> std::unique_ptr<opentxs::crypto::key::Secp256k1> final;
    auto InstantiateSecp256k1Key(
        const Secret& privateKey,
        const opentxs::crypto::key::asymmetric::Role role,
        const PasswordPrompt& reason) const noexcept
        -> std::unique_ptr<opentxs::crypto::key::Secp256k1> final;
    auto InstantiateSecp256k1Key(
        const Secret& privateKey,
        const VersionNumber version,
        const PasswordPrompt& reason) const noexcept
        -> std::unique_ptr<opentxs::crypto::key::Secp256k1> final;
    auto InstantiateSecp256k1Key(
        const Secret& privateKey,
        const opentxs::crypto::key::asymmetric::Role role,
        const VersionNumber version,
        const PasswordPrompt& reason) const noexcept
        -> std::unique_ptr<opentxs::crypto::key::Secp256k1> final;
    auto NewHDKey(
        const UnallocatedCString& seedID,
        const Secret& seed,
        const EcdsaCurve& curve,
        const opentxs::crypto::Bip32::Path& path,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> final;
    auto NewHDKey(
        const UnallocatedCString& seedID,
        const Secret& seed,
        const EcdsaCurve& curve,
        const opentxs::crypto::Bip32::Path& path,
        const opentxs::crypto::key::asymmetric::Role role,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> final;
    auto NewHDKey(
        const UnallocatedCString& seedID,
        const Secret& seed,
        const EcdsaCurve& curve,
        const opentxs::crypto::Bip32::Path& path,
        const VersionNumber version,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> final;
    auto NewHDKey(
        const UnallocatedCString& seedID,
        const Secret& seed,
        const EcdsaCurve& curve,
        const opentxs::crypto::Bip32::Path& path,
        const opentxs::crypto::key::asymmetric::Role role,
        const VersionNumber version,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> final;
    auto NewKey(
        const opentxs::crypto::Parameters& params,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::Asymmetric> final;
    auto NewKey(
        const opentxs::crypto::Parameters& params,
        const opentxs::crypto::key::asymmetric::Role role,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::Asymmetric> final;
    auto NewKey(
        const opentxs::crypto::Parameters& params,
        const VersionNumber version,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::Asymmetric> final;
    auto NewKey(
        const opentxs::crypto::Parameters& params,
        const opentxs::crypto::key::asymmetric::Role role,
        const VersionNumber version,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::Asymmetric> final;
    auto NewSecp256k1Key(
        const UnallocatedCString& seedID,
        const Secret& seed,
        const opentxs::crypto::Bip32::Path& path,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::Secp256k1> final;
    auto NewSecp256k1Key(
        const UnallocatedCString& seedID,
        const Secret& seed,
        const opentxs::crypto::Bip32::Path& path,
        const opentxs::crypto::key::asymmetric::Role role,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::Secp256k1> final;
    auto NewSecp256k1Key(
        const UnallocatedCString& seedID,
        const Secret& seed,
        const opentxs::crypto::Bip32::Path& path,
        const VersionNumber version,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::Secp256k1> final;
    auto NewSecp256k1Key(
        const UnallocatedCString& seedID,
        const Secret& seed,
        const opentxs::crypto::Bip32::Path& path,
        const opentxs::crypto::key::asymmetric::Role role,
        const VersionNumber version,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::Secp256k1> final;

    Asymmetric(const api::Session& api) noexcept;

    ~Asymmetric() final = default;

private:
    using TypeMap =
        UnallocatedMap<EcdsaCurve, opentxs::crypto::key::asymmetric::Algorithm>;

    static const VersionNumber serialized_path_version_;
    static const TypeMap curve_to_key_type_;

    const api::Session& api_;

    static auto serialize_path(
        const UnallocatedCString& seedID,
        const opentxs::crypto::Bip32::Path& children) -> proto::HDPath;

    template <typename ReturnType, typename NullType>
    auto instantiate_hd_key(
        const opentxs::crypto::key::asymmetric::Algorithm type,
        const UnallocatedCString& seedID,
        const opentxs::crypto::Bip32::Key& serialized,
        const opentxs::crypto::key::asymmetric::Role role,
        const VersionNumber version,
        const PasswordPrompt& reason) const noexcept
        -> std::unique_ptr<ReturnType>;
    template <typename ReturnType, typename NullType>
    auto instantiate_serialized_key(const proto::AsymmetricKey& serialized)
        const noexcept -> std::unique_ptr<ReturnType>;

    Asymmetric() = delete;
    Asymmetric(const Asymmetric&) = delete;
    Asymmetric(Asymmetric&&) = delete;
    auto operator=(const Asymmetric&) -> Asymmetric& = delete;
    auto operator=(Asymmetric&&) -> Asymmetric& = delete;
};
}  // namespace opentxs::api::crypto::imp
