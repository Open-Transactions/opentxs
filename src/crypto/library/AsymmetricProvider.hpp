// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/crypto/asymmetric/Role.hpp"
// IWYU pragma: no_include "opentxs/crypto/HashType.hpp"

#pragma once

#include "internal/crypto/library/AsymmetricProvider.hpp"

#include "internal/serialization/protobuf/Proto.hpp"
#include "opentxs/crypto/Parameters.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

class PasswordPrompt;
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

    AsymmetricProvider(const AsymmetricProvider&) = delete;
    AsymmetricProvider(AsymmetricProvider&&) = delete;
    auto operator=(const AsymmetricProvider&) -> AsymmetricProvider& = delete;
    auto operator=(AsymmetricProvider&&) -> AsymmetricProvider& = delete;

    ~AsymmetricProvider() override = default;

protected:
    AsymmetricProvider() noexcept;
};
}  // namespace opentxs::crypto::implementation
