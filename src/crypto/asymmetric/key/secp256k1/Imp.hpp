// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "crypto/asymmetric/key/hd/Imp.hpp"

#include <functional>

#include "crypto/asymmetric/key/secp256k1/Secp256k1Private.hpp"
#include "internal/crypto/asymmetric/key/Secp256k1.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"
#include "opentxs/crypto/asymmetric/key/Secp256k1.hpp"
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
namespace asymmetric
{
namespace key
{
namespace implementation
{
class EllipticCurve;
}  // namespace implementation
}  // namespace key

class KeyPrivate;
}  // namespace asymmetric

namespace symmetric
{
class Key;
}  // namespace symmetric

class EcdsaProvider;
}  // namespace crypto

namespace proto
{
class AsymmetricKey;
class HDPath;
}  // namespace proto

class Data;
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::asymmetric::key::implementation
{
class Secp256k1 final : public Secp256k1Private, public HD
{
public:
    [[nodiscard]] auto asEllipticCurvePublic() const noexcept
        -> const key::EllipticCurve& final
    {
        return self_;
    }
    auto asSecp256k1() const noexcept
        -> const internal::key::Secp256k1& override
    {
        return *this;
    }
    [[nodiscard]] auto asSecp256k1Private() const noexcept
        -> const key::Secp256k1Private* override
    {
        return this;
    }
    auto clone(allocator_type alloc) const noexcept -> Secp256k1* final;
    auto CreateType() const noexcept -> ParameterType final;
    auto get_deleter() const noexcept -> std::function<void(KeyPrivate*)> final;

    [[nodiscard]] auto asEllipticCurvePublic() noexcept
        -> key::EllipticCurve& final
    {
        return self_;
    }
    auto asSecp256k1() noexcept -> internal::key::Secp256k1& override
    {
        return *this;
    }
    [[nodiscard]] auto asSecp256k1Private() noexcept
        -> key::Secp256k1Private* override
    {
        return this;
    }

    Secp256k1(
        const api::Session& api,
        const crypto::EcdsaProvider& ecdsa,
        const proto::AsymmetricKey& serializedKey,
        allocator_type alloc) noexcept(false);
    Secp256k1(
        const api::Session& api,
        const crypto::EcdsaProvider& ecdsa,
        const crypto::asymmetric::Role role,
        const VersionNumber version,
        symmetric::Key& sessionKey,
        const PasswordPrompt& reason,
        allocator_type alloc) noexcept(false);
    Secp256k1(
        const api::Session& api,
        const crypto::EcdsaProvider& ecdsa,
        const opentxs::Secret& privateKey,
        const Data& publicKey,
        const crypto::asymmetric::Role role,
        const VersionNumber version,
        symmetric::Key& sessionKey,
        const PasswordPrompt& reason,
        allocator_type alloc) noexcept(false);
    Secp256k1(
        const api::Session& api,
        const crypto::EcdsaProvider& ecdsa,
        const opentxs::Secret& privateKey,
        const opentxs::Secret& chainCode,
        const Data& publicKey,
        const proto::HDPath& path,
        const Bip32Fingerprint parent,
        const crypto::asymmetric::Role role,
        const VersionNumber version,
        symmetric::Key& sessionKey,
        const PasswordPrompt& reason,
        allocator_type alloc) noexcept(false);
    Secp256k1(
        const api::Session& api,
        const crypto::EcdsaProvider& ecdsa,
        const opentxs::Secret& privateKey,
        const opentxs::Secret& chainCode,
        const Data& publicKey,
        const proto::HDPath& path,
        const Bip32Fingerprint parent,
        const crypto::asymmetric::Role role,
        const VersionNumber version,
        allocator_type alloc) noexcept(false);
    Secp256k1(
        const Secp256k1& rhs,
        const ReadView newPublic,
        allocator_type alloc) noexcept;
    Secp256k1(
        const Secp256k1& rhs,
        Secret&& newSecretKey,
        allocator_type alloc) noexcept;
    Secp256k1(const Secp256k1& rhs, allocator_type alloc = {}) noexcept;
    Secp256k1() = delete;
    Secp256k1(const Secp256k1&) = delete;
    Secp256k1(Secp256k1&&) = delete;
    auto operator=(const Secp256k1&) -> Secp256k1& = delete;
    auto operator=(Secp256k1&&) -> Secp256k1& = delete;

    ~Secp256k1() override;

private:
    key::Secp256k1 self_;

    auto replace_public_key(const ReadView newPubkey, allocator_type alloc)
        const noexcept -> EllipticCurve* final;
    auto replace_secret_key(Secret&& newSecretKey, allocator_type alloc)
        const noexcept -> EllipticCurve* final;
};
}  // namespace opentxs::crypto::asymmetric::key::implementation
