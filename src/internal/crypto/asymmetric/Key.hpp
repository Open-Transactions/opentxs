// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/Types.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/Types.internal.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace crypto
{
namespace asymmetric
{
namespace internal
{
namespace key
{
class EllipticCurve;
class RSA;
}  // namespace key
}  // namespace internal

class Key;
}  // namespace asymmetric

class AsymmetricProvider;
}  // namespace crypto

namespace identifier
{
class Generic;
}  // namespace identifier

namespace identity
{
class Authority;
}  // namespace identity

namespace proto
{
class AsymmetricKey;
class HDPath;
class Signature;
}  // namespace proto

class ByteArray;
class Data;
class OTSignatureMetadata;
class PasswordPrompt;
class Secret;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::asymmetric::internal
{
class Key
{
public:
    using Serialized = proto::AsymmetricKey;

    virtual auto asEllipticCurve() const noexcept -> const key::EllipticCurve&;
    virtual auto asRSA() const noexcept -> const key::RSA&;
    virtual auto CalculateHash(
        const crypto::HashType hashType,
        const PasswordPrompt& reason) const noexcept -> ByteArray;
    virtual auto CalculateID(identifier::Generic& theOutput) const noexcept
        -> bool;
    virtual auto CalculateSessionPassword(
        const asymmetric::Key& dhKey,
        const PasswordPrompt& reason,
        Secret& password) const noexcept -> bool;
    virtual auto CalculateTag(
        const asymmetric::Key& dhKey,
        const identifier::Generic& credential,
        const PasswordPrompt& reason,
        std::uint32_t& tag) const noexcept -> bool;
    virtual auto CalculateTag(
        const identity::Authority& nym,
        const Algorithm type,
        const PasswordPrompt& reason,
        std::uint32_t& tag,
        Secret& password) const noexcept -> bool;
    virtual auto GetMetadata() const noexcept -> const OTSignatureMetadata*;
    virtual auto operator==(const Serialized& rhs) const noexcept -> bool;
    virtual auto Params() const noexcept -> ReadView;
    virtual auto Path() const noexcept -> const UnallocatedCString;
    virtual auto Path(proto::HDPath& output) const noexcept -> bool;
    virtual auto Provider() const noexcept -> const crypto::AsymmetricProvider&;
    virtual auto Serialize(Serialized& serialized) const noexcept -> bool;
    virtual auto Sign(
        const GetPreimage input,
        const crypto::SignatureRole role,
        proto::Signature& signature,
        const identifier::Generic& credential,
        const crypto::HashType hash,
        const PasswordPrompt& reason) const noexcept -> bool;
    virtual auto Sign(
        const GetPreimage input,
        const crypto::SignatureRole role,
        proto::Signature& signature,
        const identifier::Generic& credential,
        const PasswordPrompt& reason) const noexcept -> bool;
    virtual auto TransportKey(
        Data& publicKey,
        Secret& privateKey,
        const PasswordPrompt& reason) const noexcept -> bool;
    virtual auto Verify(const Data& plaintext, const proto::Signature& sig)
        const noexcept -> bool;

    virtual auto asEllipticCurve() noexcept -> key::EllipticCurve&;
    virtual auto asRSA() noexcept -> key::RSA&;

    Key(const Key&) = delete;
    Key(Key&&) = delete;
    auto operator=(const Key& rhs) noexcept -> Key& = delete;
    auto operator=(Key&& rhs) noexcept -> Key& = delete;

    virtual ~Key() = default;

protected:
    Key() = default;
};
}  // namespace opentxs::crypto::asymmetric::internal
