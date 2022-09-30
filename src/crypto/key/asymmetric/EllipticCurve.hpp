// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "Proto.hpp"
#include "crypto/key/asymmetric/Asymmetric.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/crypto/key/Types.hpp"
#include "opentxs/crypto/key/asymmetric/Algorithm.hpp"
#include "opentxs/crypto/key/asymmetric/Role.hpp"
#include "opentxs/crypto/library/EcdsaProvider.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
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
class Asymmetric;
class Symmetric;
}  // namespace key

class EcdsaProvider;
}  // namespace crypto

namespace proto
{
class AsymmetricKey;
class Ciphertext;
class HDPath;
}  // namespace proto

class Data;
class PasswordPrompt;
class Secret;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::key::implementation
{
class EllipticCurve : virtual public key::EllipticCurve, public Asymmetric
{
public:
    operator bool() const noexcept final { return Asymmetric::operator bool(); }

    auto asPublic() const noexcept -> std::unique_ptr<key::Asymmetric> final
    {
        return asPublicEC();
    }
    auto asPublicEC() const noexcept
        -> std::unique_ptr<key::EllipticCurve> final;
    auto CloneEC() const noexcept -> std::unique_ptr<key::EllipticCurve> final
    {
        return std::unique_ptr<key::EllipticCurve>{clone_ec()};
    }
    virtual auto CreateType() const -> ParameterType = 0;
    auto ECDSA() const noexcept -> const crypto::EcdsaProvider& final
    {
        return ecdsa_;
    }
    auto IncrementPrivate(
        const opentxs::Secret& scalar,
        const PasswordPrompt& reason) const noexcept
        -> std::unique_ptr<key::EllipticCurve> final;
    auto IncrementPublic(const opentxs::Secret& scalar) const noexcept
        -> std::unique_ptr<key::EllipticCurve> final;
    auto Path() const noexcept -> const UnallocatedCString override
    {
        return {};
    }
    auto Path(proto::HDPath&) const noexcept -> bool override { return {}; }
    auto SignDER(
        const ReadView preimage,
        const crypto::HashType hash,
        Space& output,
        const PasswordPrompt& reason) const noexcept -> bool final;

    EllipticCurve() = delete;
    EllipticCurve(EllipticCurve&&) = delete;
    auto operator=(const EllipticCurve&) -> EllipticCurve& = delete;
    auto operator=(EllipticCurve&&) -> EllipticCurve& = delete;

    ~EllipticCurve() override = default;

protected:
    const crypto::EcdsaProvider& ecdsa_;

    static auto serialize_public(EllipticCurve* copy)
        -> std::shared_ptr<proto::AsymmetricKey>;

    virtual auto blank_private() const noexcept -> ReadView = 0;
    virtual auto clone_ec() const -> EllipticCurve* = 0;
    virtual auto get_public() const
        -> std::shared_ptr<proto::AsymmetricKey> = 0;

    EllipticCurve(
        const api::Session& api,
        const crypto::EcdsaProvider& ecdsa,
        const proto::AsymmetricKey& serializedKey) noexcept(false);
    EllipticCurve(
        const api::Session& api,
        const crypto::EcdsaProvider& ecdsa,
        const crypto::key::asymmetric::Algorithm keyType,
        const crypto::key::asymmetric::Role role,
        const VersionNumber version,
        const PasswordPrompt& reason) noexcept(false);
    EllipticCurve(
        const api::Session& api,
        const crypto::EcdsaProvider& ecdsa,
        const crypto::key::asymmetric::Algorithm keyType,
        const opentxs::Secret& privateKey,
        const Data& publicKey,
        const crypto::key::asymmetric::Role role,
        const VersionNumber version,
        key::Symmetric& sessionKey,
        const PasswordPrompt& reason) noexcept(false);
    EllipticCurve(
        const api::Session& api,
        const crypto::EcdsaProvider& ecdsa,
        const crypto::key::asymmetric::Algorithm keyType,
        const opentxs::Secret& privateKey,
        const Data& publicKey,
        const crypto::key::asymmetric::Role role,
        const VersionNumber version) noexcept(false);
    EllipticCurve(const EllipticCurve&) noexcept;
    EllipticCurve(const EllipticCurve& rhs, const ReadView newPublic) noexcept;
    EllipticCurve(const EllipticCurve& rhs, OTSecret&& newSecretKey) noexcept;

private:
    friend crypto::EcdsaProvider;

    static auto extract_key(
        const api::Session& api,
        const crypto::EcdsaProvider& ecdsa,
        const proto::AsymmetricKey& serialized,
        Data& publicKey) -> std::unique_ptr<proto::Ciphertext>;

    virtual auto replace_public_key(const ReadView newPubkey) const noexcept
        -> std::unique_ptr<EllipticCurve> = 0;
    virtual auto replace_secret_key(OTSecret&& newSecretKey) const noexcept
        -> std::unique_ptr<EllipticCurve> = 0;
};
}  // namespace opentxs::crypto::key::implementation
