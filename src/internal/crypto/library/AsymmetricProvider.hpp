// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/crypto/asymmetric/Algorithm.hpp"
// IWYU pragma: no_include "opentxs/crypto/asymmetric/Role.hpp"

#pragma once

#include <optional>

#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/Writer.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
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

namespace opentxs::crypto
{
class AsymmetricProvider
{
public:
    static auto CurveToKeyType(const EcdsaCurve& curve)
        -> crypto::asymmetric::Algorithm;
    static auto KeyTypeToCurve(const crypto::asymmetric::Algorithm& type)
        -> EcdsaCurve;

    virtual auto SeedToCurveKey(
        const ReadView seed,
        Writer&& privateKey,
        Writer&& publicKey) const noexcept -> bool = 0;
    virtual auto SharedSecret(
        const ReadView publicKey,
        const ReadView privateKey,
        const SecretStyle style,
        Secret& secret) const noexcept -> bool = 0;
    virtual auto RandomKeypair(
        Writer&& privateKey,
        Writer&& publicKey,
        Writer&& params = {}) const noexcept -> bool = 0;
    virtual auto RandomKeypair(
        Writer&& privateKey,
        Writer&& publicKey,
        const opentxs::crypto::asymmetric::Role role,
        Writer&& params = {}) const noexcept -> bool = 0;
    virtual auto RandomKeypair(
        Writer&& privateKey,
        Writer&& publicKey,
        const Parameters& options,
        Writer&& params = {}) const noexcept -> bool = 0;
    virtual auto RandomKeypair(
        Writer&& privateKey,
        Writer&& publicKey,
        const opentxs::crypto::asymmetric::Role role,
        const Parameters& options,
        Writer&& params = {}) const noexcept -> bool = 0;
    virtual auto Sign(
        const ReadView plaintext,
        const ReadView key,
        const crypto::HashType hash,
        Writer&& signature) const -> bool = 0;
    virtual auto SignContract(
        const api::Session& api,
        const String& contract,
        const ReadView key,
        const crypto::HashType hashType,
        Signature& output) const -> bool = 0;
    virtual auto Verify(
        const ReadView plaintext,
        const ReadView key,
        const ReadView signature,
        const crypto::HashType hashType) const -> bool = 0;
    virtual auto VerifyContractSignature(
        const api::Session& api,
        const String& strContractToVerify,
        const ReadView key,
        const Signature& theSignature,
        const crypto::HashType hashType) const -> bool = 0;

    AsymmetricProvider(const AsymmetricProvider&) = delete;
    AsymmetricProvider(AsymmetricProvider&&) = delete;
    auto operator=(const AsymmetricProvider&) -> AsymmetricProvider& = delete;
    auto operator=(AsymmetricProvider&&) -> AsymmetricProvider& = delete;

    virtual ~AsymmetricProvider() = default;

protected:
    AsymmetricProvider() = default;
};
}  // namespace opentxs::crypto
