// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/crypto/key/asymmetric/Role.hpp"

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <optional>

#include "opentxs/Types.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/Types.hpp"
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
class Parameters;
}  // namespace crypto

class Signature;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto
{
class OPENTXS_EXPORT AsymmetricProvider
{
public:
    static auto CurveToKeyType(const EcdsaCurve& curve)
        -> crypto::key::asymmetric::Algorithm;
    static auto KeyTypeToCurve(const crypto::key::asymmetric::Algorithm& type)
        -> EcdsaCurve;

    virtual auto SeedToCurveKey(
        const ReadView seed,
        const AllocateOutput privateKey,
        const AllocateOutput publicKey) const noexcept -> bool = 0;
    virtual auto SharedSecret(
        const ReadView publicKey,
        const ReadView privateKey,
        const SecretStyle style,
        Secret& secret) const noexcept -> bool = 0;
    virtual auto RandomKeypair(
        const AllocateOutput privateKey,
        const AllocateOutput publicKey,
        const AllocateOutput params = {}) const noexcept -> bool = 0;
    virtual auto RandomKeypair(
        const AllocateOutput privateKey,
        const AllocateOutput publicKey,
        const opentxs::crypto::key::asymmetric::Role role,
        const AllocateOutput params = {}) const noexcept -> bool = 0;
    virtual auto RandomKeypair(
        const AllocateOutput privateKey,
        const AllocateOutput publicKey,
        const Parameters& options,
        const AllocateOutput params = {}) const noexcept -> bool = 0;
    virtual auto RandomKeypair(
        const AllocateOutput privateKey,
        const AllocateOutput publicKey,
        const opentxs::crypto::key::asymmetric::Role role,
        const Parameters& options,
        const AllocateOutput params = {}) const noexcept -> bool = 0;
    virtual auto Sign(
        const ReadView plaintext,
        const ReadView key,
        const crypto::HashType hash,
        const AllocateOutput signature) const -> bool = 0;
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

    OPENTXS_NO_EXPORT virtual ~AsymmetricProvider() = default;

protected:
    AsymmetricProvider() = default;

private:
    AsymmetricProvider(const AsymmetricProvider&) = delete;
    AsymmetricProvider(AsymmetricProvider&&) = delete;
    auto operator=(const AsymmetricProvider&) -> AsymmetricProvider& = delete;
    auto operator=(AsymmetricProvider&&) -> AsymmetricProvider& = delete;
};
}  // namespace opentxs::crypto
