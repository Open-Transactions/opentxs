// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "Proto.hpp"
#include "crypto/key/asymmetric/Asymmetric.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/RSA.hpp"
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
class Asymmetric;
}  // namespace key

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

namespace opentxs::crypto::key::implementation
{
class RSA final : public key::RSA, public Asymmetric
{
public:
    auto asPublic() const noexcept -> std::unique_ptr<key::Asymmetric> final;
    auto Params() const noexcept -> ReadView final { return params_.Bytes(); }
    auto SigHashType() const noexcept -> crypto::HashType final
    {
        return crypto::HashType::Sha256;
    }

    RSA(const api::Session& api,
        const crypto::AsymmetricProvider& engine,
        const crypto::key::asymmetric::Role role,
        const VersionNumber version,
        const Parameters& options,
        Space& params,
        const PasswordPrompt& reason)
    noexcept(false);
    RSA(const api::Session& api,
        const crypto::AsymmetricProvider& engine,
        const proto::AsymmetricKey& serialized)
    noexcept(false);
    RSA() = delete;
    RSA(const RSA&) noexcept;
    RSA(RSA&&) = delete;
    auto operator=(const RSA&) -> RSA& = delete;
    auto operator=(RSA&&) -> RSA& = delete;

    ~RSA() final = default;

private:
    const ByteArray params_;

    static auto deserialize_key(
        const api::Session& api,
        const proto::AsymmetricKey& serialized,
        Data& publicKey,
        Secret& privateKey) noexcept(false)
        -> std::unique_ptr<proto::Ciphertext>;

    auto clone() const noexcept -> RSA* final { return new RSA{*this}; }
    auto serialize(const Lock& lock, Serialized& serialized) const noexcept
        -> bool final;
};
}  // namespace opentxs::crypto::key::implementation
