// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::crypto::HashType
// IWYU pragma: no_forward_declare opentxs::crypto::ParameterType
// IWYU pragma: no_forward_declare opentxs::crypto::asymmetric::Algorithm
// IWYU pragma: no_forward_declare opentxs::crypto::asymmetric::Role
// IWYU pragma: no_include "internal/crypto/library/EcdsaProvider.hpp"

#pragma once

#include "crypto/asymmetric/base/Imp.hpp"

#include <memory>
#include <new>

#include "crypto/asymmetric/key/ellipticcurve/EllipticCurvePrivate.hpp"
#include "internal/crypto/asymmetric/key/EllipticCurve.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace crypto
{
namespace symmetric
{
class Key;
}  // namespace symmetric
}  // namespace crypto

namespace proto
{
class AsymmetricKey;
class Ciphertext;
}  // namespace proto

class Data;
class PasswordPrompt;
class Secret;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::asymmetric::key::implementation
{
class EllipticCurve : virtual public EllipticCurvePrivate,
                      public asymmetric::implementation::Key

{
public:
    auto asEllipticCurve() const noexcept
        -> const internal::key::EllipticCurve& final
    {
        return *this;
    }
    [[nodiscard]] auto asEllipticCurvePrivate() const noexcept
        -> const key::EllipticCurvePrivate* final
    {
        return this;
    }
    virtual auto CreateType() const noexcept -> ParameterType = 0;
    auto ECDSA() const noexcept -> const crypto::EcdsaProvider& final
    {
        return ecdsa_;
    }
    auto IncrementPrivate(
        const Secret& scalar,
        const PasswordPrompt& reason,
        allocator_type alloc) const noexcept
        -> asymmetric::key::EllipticCurve final;
    auto IncrementPublic(const Secret& scalar, allocator_type alloc)
        const noexcept -> asymmetric::key::EllipticCurve final;
    auto SignDER(
        const ReadView preimage,
        const crypto::HashType hash,
        Writer&& output,
        const PasswordPrompt& reason) const noexcept -> bool final;

    auto asEllipticCurve() noexcept -> internal::key::EllipticCurve& final
    {
        return *this;
    }
    [[nodiscard]] auto asEllipticCurvePrivate() noexcept
        -> key::EllipticCurvePrivate* final
    {
        return this;
    }

    EllipticCurve() = delete;
    EllipticCurve(const EllipticCurve&) = delete;

    ~EllipticCurve() override;

protected:
    const crypto::EcdsaProvider& ecdsa_;

    static auto serialize_public(EllipticCurve* copy)
        -> std::shared_ptr<proto::AsymmetricKey>;

    EllipticCurve(
        const api::Session& api,
        const crypto::EcdsaProvider& ecdsa,
        const proto::AsymmetricKey& serializedKey,
        allocator_type alloc) noexcept(false);
    EllipticCurve(
        const api::Session& api,
        const crypto::EcdsaProvider& ecdsa,
        const crypto::asymmetric::Algorithm keyType,
        const crypto::asymmetric::Role role,
        const VersionNumber version,
        symmetric::Key& sessionKey,
        const PasswordPrompt& reason,
        allocator_type alloc) noexcept(false);
    EllipticCurve(
        const api::Session& api,
        const crypto::EcdsaProvider& ecdsa,
        const crypto::asymmetric::Algorithm keyType,
        const opentxs::Secret& privateKey,
        const Data& publicKey,
        const crypto::asymmetric::Role role,
        const VersionNumber version,
        symmetric::Key& sessionKey,
        const PasswordPrompt& reason,
        allocator_type alloc) noexcept(false);
    EllipticCurve(
        const api::Session& api,
        const crypto::EcdsaProvider& ecdsa,
        const crypto::asymmetric::Algorithm keyType,
        const opentxs::Secret& privateKey,
        const Data& publicKey,
        const crypto::asymmetric::Role role,
        const VersionNumber version,
        allocator_type alloc) noexcept(false);
    EllipticCurve(const EllipticCurve&, allocator_type alloc) noexcept;
    EllipticCurve(
        const EllipticCurve& rhs,
        const ReadView newPublic,
        allocator_type alloc) noexcept;
    EllipticCurve(
        const EllipticCurve& rhs,
        Secret&& newSecretKey,
        allocator_type alloc) noexcept;

private:
    static auto extract_key(
        const api::Session& api,
        const crypto::EcdsaProvider& ecdsa,
        const proto::AsymmetricKey& serialized,
        Data& publicKey) -> std::unique_ptr<proto::Ciphertext>;

    virtual auto replace_public_key(
        const ReadView newPubkey,
        allocator_type alloc) const noexcept -> EllipticCurve* = 0;
    virtual auto replace_secret_key(Secret&& newSecretKey, allocator_type alloc)
        const noexcept -> EllipticCurve* = 0;
};
}  // namespace opentxs::crypto::asymmetric::key::implementation
