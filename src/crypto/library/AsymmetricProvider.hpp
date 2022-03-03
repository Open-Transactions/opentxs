// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/crypto/key/asymmetric/Role.hpp"
// IWYU pragma: no_include "opentxs/crypto/HashType.hpp"

#pragma once

#include "Proto.hpp"
#include "opentxs/crypto/Parameters.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/library/AsymmetricProvider.hpp"
#include "opentxs/util/Bytes.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
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
}  // namespace crypto

class PasswordPrompt;
class Signature;
class String;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::implementation
{
class AsymmetricProvider : virtual public crypto::AsymmetricProvider
{
public:
    using crypto::AsymmetricProvider::RandomKeypair;
    auto RandomKeypair(
        const AllocateOutput privateKey,
        const AllocateOutput publicKey,
        const AllocateOutput params) const noexcept -> bool final;
    auto RandomKeypair(
        const AllocateOutput privateKey,
        const AllocateOutput publicKey,
        const opentxs::crypto::key::asymmetric::Role role,
        const AllocateOutput params) const noexcept -> bool final;
    auto RandomKeypair(
        const AllocateOutput privateKey,
        const AllocateOutput publicKey,
        const Parameters& options,
        const AllocateOutput params) const noexcept -> bool final;
    auto SeedToCurveKey(
        const ReadView seed,
        const AllocateOutput privateKey,
        const AllocateOutput publicKey) const noexcept -> bool final;
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

    ~AsymmetricProvider() override = default;

protected:
    AsymmetricProvider() noexcept;

private:
    AsymmetricProvider(const AsymmetricProvider&) = delete;
    AsymmetricProvider(AsymmetricProvider&&) = delete;
    auto operator=(const AsymmetricProvider&) -> AsymmetricProvider& = delete;
    auto operator=(AsymmetricProvider&&) -> AsymmetricProvider& = delete;
};
}  // namespace opentxs::crypto::implementation
