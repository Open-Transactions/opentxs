// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/crypto/library/AsymmetricProvider.hpp"

#include <memory>

#include "opentxs/Types.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace internal
{
class Factory;
}  // namespace internal

class Session;
}  // namespace api

namespace crypto
{
class Parameters;
}  // namespace crypto

class Signature;
class String;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::implementation
{
class AsymmetricProvider : virtual public crypto::AsymmetricProvider
{
public:
    using crypto::AsymmetricProvider::RandomKeypair;
    auto RandomKeypair(Writer&& privateKey, Writer&& publicKey, Writer&& params)
        const noexcept -> bool final;
    auto RandomKeypair(
        Writer&& privateKey,
        Writer&& publicKey,
        const opentxs::crypto::asymmetric::Role role,
        Writer&& params) const noexcept -> bool final;
    auto RandomKeypair(
        Writer&& privateKey,
        Writer&& publicKey,
        const Parameters& options,
        Writer&& params) const noexcept -> bool final;
    auto SeedToCurveKey(
        const ReadView seed,
        Writer&& privateKey,
        Writer&& publicKey) const noexcept -> bool final;
    auto SignContract(
        const api::Session& api,
        const String& contract,
        const ReadView key,
        const crypto::HashType hashType,
        Signature& output) const -> bool override;
    auto VerifyContractSignature(
        const api::Session& api,
        const String& strContractToVerify,
        const ReadView key,
        const Signature& theSignature,
        const crypto::HashType hashType) const -> bool override;

    auto Init(
        const std::shared_ptr<const api::internal::Factory>& factory) noexcept
        -> void final;

    AsymmetricProvider(const AsymmetricProvider&) = delete;
    AsymmetricProvider(AsymmetricProvider&&) = delete;
    auto operator=(const AsymmetricProvider&) -> AsymmetricProvider& = delete;
    auto operator=(AsymmetricProvider&&) -> AsymmetricProvider& = delete;

    ~AsymmetricProvider() override = default;

protected:
    std::weak_ptr<const api::internal::Factory> factory_;

    AsymmetricProvider() noexcept;
};
}  // namespace opentxs::crypto::implementation
