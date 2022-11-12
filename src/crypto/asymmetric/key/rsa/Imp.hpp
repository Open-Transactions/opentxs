// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::crypto::HashType
// IWYU pragma: no_forward_declare opentxs::crypto::asymmetric::Role
// IWYU pragma: no_include "opentxs/crypto/HashType.hpp"
// IWYU pragma: no_include "opentxs/crypto/asymmetric/Role.hpp"

#pragma once

#include "crypto/asymmetric/base/Imp.hpp"

#include <functional>
#include <memory>
#include <new>

#include "crypto/asymmetric/key/rsa/RSAPrivate.hpp"
#include "internal/crypto/asymmetric/Key.hpp"
#include "internal/crypto/asymmetric/key/RSA.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/crypto/asymmetric/key/RSA.hpp"
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

namespace asymmetric
{
class KeyPrivate;
}  // namespace asymmetric

class AsymmetricProvider;
class Parameters;
}  // namespace crypto

namespace proto
{
class AsymmetricKey;
class Ciphertext;
}  // namespace proto

class Data;
class PasswordPrompt;
class Secret;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::asymmetric::key::implementation
{
class RSA final : public RSAPrivate, public asymmetric::implementation::Key
{
public:
    auto asRSA() const noexcept -> const internal::key::RSA& final
    {
        return *this;
    }
    [[nodiscard]] auto asRSAPrivate() const noexcept
        -> const key::RSAPrivate* final
    {
        return this;
    }
    [[nodiscard]] auto asRSAPublic() const noexcept -> const key::RSA& final
    {
        return self_;
    }
    auto clone(allocator_type alloc) const noexcept -> RSA* final;
    auto get_deleter() const noexcept -> std::function<void(KeyPrivate*)> final;
    auto Params() const noexcept -> ReadView final { return params_.Bytes(); }
    auto PreferredHash() const noexcept -> crypto::HashType final;

    auto asRSA() noexcept -> internal::key::RSA& final { return *this; }
    [[nodiscard]] auto asRSAPrivate() noexcept -> key::RSAPrivate* final
    {
        return this;
    }
    [[nodiscard]] auto asRSAPublic() noexcept -> key::RSA& final
    {
        return self_;
    }

    RSA(const api::Session& api,
        const crypto::AsymmetricProvider& engine,
        const crypto::asymmetric::Role role,
        const VersionNumber version,
        const Parameters& options,
        Space& params,
        symmetric::Key& sessionKey,
        const PasswordPrompt& reason,
        allocator_type alloc)
    noexcept(false);
    RSA(const api::Session& api,
        const crypto::AsymmetricProvider& engine,
        const proto::AsymmetricKey& serialized,
        allocator_type alloc)
    noexcept(false);
    RSA(const RSA& rhs, allocator_type alloc) noexcept;
    RSA(const RSA&) = delete;
    RSA(RSA&&) = delete;
    auto operator=(const RSA& rhs) noexcept -> RSA& = delete;
    auto operator=(RSA&& rhs) noexcept -> RSA& = delete;

    ~RSA() final;

private:
    const ByteArray params_;
    key::RSA self_;

    static auto deserialize_key(
        const api::Session& api,
        const proto::AsymmetricKey& serialized,
        Data& publicKey,
        Secret& privateKey) noexcept(false)
        -> std::unique_ptr<proto::Ciphertext>;

    auto serialize(const Lock& lock, Serialized& serialized) const noexcept
        -> bool final;
};
}  // namespace opentxs::crypto::asymmetric::key::implementation
