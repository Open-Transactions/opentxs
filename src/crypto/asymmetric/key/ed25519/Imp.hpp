// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::crypto::asymmetric::KeyPrivate
// IWYU pragma: no_include "crypto/asymmetric/base/KeyPrivate.hpp"

#pragma once

#include <functional>

#include "crypto/asymmetric/key/ed25519/Ed25519Private.hpp"
#include "crypto/asymmetric/key/hd/Imp.hpp"
#include "internal/crypto/asymmetric/key/Ed25519.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/crypto/asymmetric/key/Ed25519.hpp"
#include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"
#include "opentxs/util/Allocator.hpp"
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
class Ed25519 final : public Ed25519Private, public HD
{
public:
    auto asEd25519() const noexcept -> const internal::key::Ed25519& override
    {
        return *this;
    }
    [[nodiscard]] auto asEd25519Private() const noexcept
        -> const key::Ed25519Private* override
    {
        return this;
    }
    auto asEd25519Public() const noexcept
        -> const asymmetric::key::Ed25519& final
    {
        return self_;
    }
    [[nodiscard]] auto asEllipticCurvePublic() const noexcept
        -> const key::EllipticCurve& final
    {
        return self_;
    }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> asymmetric::KeyPrivate* final
    {
        return pmr::clone_as<asymmetric::KeyPrivate>(this, {alloc});
    }
    auto CreateType() const noexcept -> ParameterType final;
    [[nodiscard]] auto get_deleter() noexcept -> std::function<void()> final
    {
        return make_deleter(this);
    }
    auto TransportKey(
        Data& publicKey,
        Secret& privateKey,
        const PasswordPrompt& reason) const noexcept -> bool final;

    auto asEd25519() noexcept -> internal::key::Ed25519& override
    {
        return *this;
    }
    [[nodiscard]] auto asEd25519Private() noexcept
        -> key::Ed25519Private* override
    {
        return this;
    }
    auto asEd25519Public() noexcept -> asymmetric::key::Ed25519& final
    {
        return self_;
    }
    [[nodiscard]] auto asEllipticCurvePublic() noexcept
        -> key::EllipticCurve& final
    {
        return self_;
    }

    Ed25519(
        const api::Session& api,
        const crypto::EcdsaProvider& ecdsa,
        const crypto::asymmetric::Role role,
        const VersionNumber version,
        symmetric::Key& sessionKey,
        const PasswordPrompt& reason,
        allocator_type alloc) noexcept(false);
    Ed25519(
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
    Ed25519(
        const api::Session& api,
        const crypto::EcdsaProvider& ecdsa,
        const proto::AsymmetricKey& serializedKey,
        allocator_type alloc) noexcept(false);
    Ed25519(
        const Ed25519& rhs,
        const ReadView newPublic,
        allocator_type alloc) noexcept;
    Ed25519(
        const Ed25519& rhs,
        Secret&& newSecretKey,
        allocator_type alloc) noexcept;
    Ed25519(const Ed25519& rhs, allocator_type alloc = {}) noexcept;
    Ed25519() = delete;
    Ed25519(const Ed25519&) = delete;
    Ed25519(Ed25519&&) = delete;
    auto operator=(const Ed25519&) -> Ed25519& = delete;
    auto operator=(Ed25519&&) -> Ed25519& = delete;

    ~Ed25519() override;

private:
    key::Ed25519 self_;

    auto replace_public_key(const ReadView newPubkey, alloc::Strategy alloc)
        const noexcept -> EllipticCurve* final;
    auto replace_secret_key(Secret&& newSecretKey, alloc::Strategy alloc)
        const noexcept -> EllipticCurve* final;
};
}  // namespace opentxs::crypto::asymmetric::key::implementation
