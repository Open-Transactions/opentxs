// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <Enums.pb.h>
#include <memory>
#include <utility>

#include "crypto/key/asymmetric/EllipticCurve.hpp"
#include "crypto/key/asymmetric/HD.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/ParameterType.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/Secp256k1.hpp"
#include "opentxs/crypto/key/asymmetric/Role.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace crypto
{
namespace key
{
namespace implementation
{
class EllipticCurve;
}  // namespace implementation

class Symmetric;
}  // namespace key

class EcdsaProvider;
}  // namespace crypto

namespace proto
{
class AsymmetricKey;
class HDPath;
}  // namespace proto

class Data;
class PasswordPrompt;
class Secret;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::key::implementation
{
class Secp256k1 final : virtual public key::Secp256k1, public HD
{
public:
    auto CreateType() const -> ParameterType final
    {
        return ParameterType::secp256k1;
    }

    Secp256k1(
        const api::Session& api,
        const crypto::EcdsaProvider& ecdsa,
        const proto::AsymmetricKey& serializedKey) noexcept(false);
    Secp256k1(
        const api::Session& api,
        const crypto::EcdsaProvider& ecdsa,
        const crypto::key::asymmetric::Role role,
        const VersionNumber version,
        const PasswordPrompt& reason) noexcept(false);
    Secp256k1(
        const api::Session& api,
        const crypto::EcdsaProvider& ecdsa,
        const opentxs::Secret& privateKey,
        const Data& publicKey,
        const crypto::key::asymmetric::Role role,
        const VersionNumber version,
        key::Symmetric& sessionKey,
        const PasswordPrompt& reason) noexcept(false);
    Secp256k1(
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
        const PasswordPrompt& reason) noexcept(false);
    Secp256k1(
        const api::Session& api,
        const crypto::EcdsaProvider& ecdsa,
        const opentxs::Secret& privateKey,
        const opentxs::Secret& chainCode,
        const Data& publicKey,
        const proto::HDPath& path,
        const Bip32Fingerprint parent,
        const crypto::key::asymmetric::Role role,
        const VersionNumber version) noexcept(false);
    Secp256k1(const Secp256k1& rhs, const ReadView newPublic) noexcept;
    Secp256k1(const Secp256k1& rhs, OTSecret&& newSecretKey) noexcept;
    Secp256k1() = delete;
    Secp256k1(const Secp256k1&) noexcept;
    Secp256k1(Secp256k1&&) = delete;
    auto operator=(const Secp256k1&) -> Secp256k1& = delete;
    auto operator=(Secp256k1&&) -> Secp256k1& = delete;

    ~Secp256k1() final = default;

private:
    using ot_super = HD;

    auto blank_private() const noexcept -> ReadView final;
    auto clone() const noexcept -> Secp256k1* final
    {
        return new Secp256k1(*this);
    }
    auto clone_ec() const noexcept -> Secp256k1* final { return clone(); }
    auto get_public() const -> std::shared_ptr<proto::AsymmetricKey> final
    {
        return serialize_public(clone());
    }
    auto replace_public_key(const ReadView newPubkey) const noexcept
        -> std::unique_ptr<EllipticCurve> final
    {
        return std::make_unique<Secp256k1>(*this, newPubkey);
    }
    auto replace_secret_key(OTSecret&& newSecretKey) const noexcept
        -> std::unique_ptr<EllipticCurve> final
    {
        return std::make_unique<Secp256k1>(*this, std::move(newSecretKey));
    }
};
}  // namespace opentxs::crypto::key::implementation
