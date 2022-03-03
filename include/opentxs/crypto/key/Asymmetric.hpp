// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <memory>

#include "opentxs/core/Data.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/library/AsymmetricProvider.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace crypto
{
namespace key
{
class Asymmetric;
}  // namespace key
}  // namespace crypto

namespace identity
{
class Authority;
}  // namespace identity

namespace proto
{
class AsymmetricKey;
class Signature;
}  // namespace proto

class OTSignatureMetadata;
class PasswordPrompt;
class Secret;

using OTAsymmetricKey = Pimpl<crypto::key::Asymmetric>;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::key
{
class OPENTXS_EXPORT Asymmetric
{
public:
    using Serialized = proto::AsymmetricKey;

    static const VersionNumber DefaultVersion;
    static const VersionNumber MaxVersion;

    static auto Factory() noexcept -> OTAsymmetricKey;

    virtual auto asPublic() const noexcept -> std::unique_ptr<Asymmetric> = 0;
    virtual auto CalculateHash(
        const crypto::HashType hashType,
        const PasswordPrompt& reason) const noexcept -> OTData = 0;
    virtual auto CalculateID(Identifier& theOutput) const noexcept -> bool = 0;
    virtual auto CalculateTag(
        const identity::Authority& nym,
        const crypto::key::asymmetric::Algorithm type,
        const PasswordPrompt& reason,
        std::uint32_t& tag,
        Secret& password) const noexcept -> bool = 0;
    virtual auto CalculateTag(
        const Asymmetric& dhKey,
        const Identifier& credential,
        const PasswordPrompt& reason,
        std::uint32_t& tag) const noexcept -> bool = 0;
    virtual auto CalculateSessionPassword(
        const Asymmetric& dhKey,
        const PasswordPrompt& reason,
        Secret& password) const noexcept -> bool = 0;
    virtual auto engine() const noexcept
        -> const opentxs::crypto::AsymmetricProvider& = 0;
    virtual auto GetMetadata() const noexcept -> const OTSignatureMetadata* = 0;
    virtual auto hasCapability(const NymCapability& capability) const noexcept
        -> bool = 0;
    virtual auto HasPrivate() const noexcept -> bool = 0;
    virtual auto HasPublic() const noexcept -> bool = 0;
    virtual auto keyType() const noexcept
        -> crypto::key::asymmetric::Algorithm = 0;
    virtual auto Params() const noexcept -> ReadView = 0;
    virtual auto Path() const noexcept -> const UnallocatedCString = 0;
    OPENTXS_NO_EXPORT virtual auto Path(proto::HDPath& output) const noexcept
        -> bool = 0;
    virtual auto PrivateKey(const PasswordPrompt& reason) const noexcept
        -> ReadView = 0;
    virtual auto PublicKey() const noexcept -> ReadView = 0;
    virtual auto Role() const noexcept
        -> opentxs::crypto::key::asymmetric::Role = 0;
    OPENTXS_NO_EXPORT virtual auto Serialize(
        Serialized& serialized) const noexcept -> bool = 0;
    virtual auto SigHashType() const noexcept -> crypto::HashType = 0;
    OPENTXS_NO_EXPORT virtual auto Sign(
        const GetPreimage input,
        const crypto::SignatureRole role,
        proto::Signature& signature,
        const Identifier& credential,
        const PasswordPrompt& reason,
        const crypto::HashType hash = crypto::HashType::Error) const noexcept
        -> bool = 0;
    virtual auto Sign(
        const ReadView preimage,
        const crypto::HashType hash,
        const AllocateOutput output,
        const PasswordPrompt& reason) const noexcept -> bool = 0;
    virtual auto TransportKey(
        Data& publicKey,
        Secret& privateKey,
        const PasswordPrompt& reason) const noexcept -> bool = 0;
    OPENTXS_NO_EXPORT virtual auto Verify(
        const Data& plaintext,
        const proto::Signature& sig) const noexcept -> bool = 0;
    virtual auto Version() const noexcept -> VersionNumber = 0;

    virtual operator bool() const noexcept = 0;
    OPENTXS_NO_EXPORT virtual auto operator==(const Serialized&) const noexcept
        -> bool = 0;

    virtual ~Asymmetric() = default;

protected:
    Asymmetric() = default;

private:
    friend OTAsymmetricKey;

    virtual auto clone() const noexcept -> Asymmetric* = 0;

    Asymmetric(const Asymmetric&) = delete;
    Asymmetric(Asymmetric&&) = delete;
    auto operator=(const Asymmetric&) -> Asymmetric& = delete;
    auto operator=(Asymmetric&&) -> Asymmetric& = delete;
};
}  // namespace opentxs::crypto::key
