// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/identity/Authority.hpp"

#include "internal/core/String.hpp"
#include "internal/crypto/key/Keypair.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/Types.internal.hpp"
#include "opentxs/crypto/Types.internal.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/Types.internal.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace crypto
{
namespace asymmetric
{
class Key;
}  // namespace asymmetric
}  // namespace crypto

namespace identity
{
namespace credential
{
class Primary;
}  // namespace credential
}  // namespace identity

namespace protobuf
{
class Authority;
class ContactData;
class HDPath;
class Signature;
class VerificationItem;
class VerificationSet;
}  // namespace protobuf
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::internal
{
class Authority : virtual public identity::Authority
{
public:
    using AuthorityKeys = identity::Nym::AuthorityKeys;
    using Serialized = protobuf::Authority;

    static auto NymToContactCredential(const VersionNumber nym) noexcept(false)
        -> VersionNumber;

    virtual auto EncryptionTargets() const noexcept -> AuthorityKeys = 0;
    virtual auto GetContactData(protobuf::ContactData& contactData) const
        -> bool = 0;
    virtual auto GetMasterCredential() const -> const credential::Primary& = 0;
    auto Internal() const noexcept -> const internal::Authority& final
    {
        return *this;
    }
    virtual auto GetPublicAuthKey(
        crypto::asymmetric::Algorithm keytype,
        const String::List* plistRevokedIDs = nullptr) const
        -> const crypto::asymmetric::Key& = 0;
    virtual auto GetPublicEncrKey(
        crypto::asymmetric::Algorithm keytype,
        const String::List* plistRevokedIDs = nullptr) const
        -> const crypto::asymmetric::Key& = 0;
    virtual auto GetPublicKeysBySignature(
        crypto::key::Keypair::Keys& listOutput,
        const Signature& theSignature,
        char cKeyType = '0') const -> std::int32_t = 0;
    virtual auto GetPublicSignKey(
        crypto::asymmetric::Algorithm keytype,
        const String::List* plistRevokedIDs = nullptr) const
        -> const crypto::asymmetric::Key& = 0;
    virtual auto GetPrivateSignKey(
        crypto::asymmetric::Algorithm keytype,
        const String::List* plistRevokedIDs = nullptr) const
        -> const crypto::asymmetric::Key& = 0;
    virtual auto GetPrivateEncrKey(
        crypto::asymmetric::Algorithm keytype,
        const String::List* plistRevokedIDs = nullptr) const
        -> const crypto::asymmetric::Key& = 0;
    virtual auto GetPrivateAuthKey(
        crypto::asymmetric::Algorithm keytype,
        const String::List* plistRevokedIDs = nullptr) const
        -> const crypto::asymmetric::Key& = 0;
    virtual auto GetAuthKeypair(
        crypto::asymmetric::Algorithm keytype,
        const String::List* plistRevokedIDs = nullptr) const
        -> const crypto::key::Keypair& = 0;
    virtual auto GetEncrKeypair(
        crypto::asymmetric::Algorithm keytype,
        const String::List* plistRevokedIDs = nullptr) const
        -> const crypto::key::Keypair& = 0;
    virtual auto GetSignKeypair(
        crypto::asymmetric::Algorithm keytype,
        const String::List* plistRevokedIDs = nullptr) const
        -> const crypto::key::Keypair& = 0;
    virtual auto GetVerificationSet(
        protobuf::VerificationSet& verificationSet) const -> bool = 0;
    virtual auto Path(protobuf::HDPath& output) const -> bool = 0;
    virtual auto Serialize(
        Serialized& serialized,
        const CredentialIndexModeFlag mode) const -> bool = 0;
    virtual auto Sign(
        const crypto::GetPreimage input,
        crypto::SignatureRole role,
        protobuf::Signature& output,
        const PasswordPrompt& reason) const -> bool = 0;
    virtual auto Sign(
        const crypto::GetPreimage input,
        crypto::SignatureRole role,
        crypto::HashType hash,
        protobuf::Signature& output,
        const PasswordPrompt& reason) const -> bool = 0;
    virtual auto Sign(
        const crypto::GetPreimage input,
        crypto::SignatureRole role,
        opentxs::crypto::asymmetric::Role key,
        protobuf::Signature& output,
        const PasswordPrompt& reason) const -> bool = 0;
    virtual auto Sign(
        const crypto::GetPreimage input,
        crypto::SignatureRole role,
        opentxs::crypto::asymmetric::Role key,
        crypto::HashType hash,
        protobuf::Signature& output,
        const PasswordPrompt& reason) const -> bool = 0;
    virtual auto Verify(const Data& plaintext, const protobuf::Signature& sig)
        const -> bool = 0;
    virtual auto Verify(
        const Data& plaintext,
        const protobuf::Signature& sig,
        const opentxs::crypto::asymmetric::Role key) const -> bool = 0;
    virtual auto Verify(const protobuf::VerificationItem& item) const
        -> bool = 0;
    virtual auto VerifyInternally() const -> bool = 0;
    virtual auto WriteCredentials() const -> bool = 0;

    virtual auto AddChildKeyCredential(
        const crypto::Parameters& nymParameters,
        const PasswordPrompt& reason) -> identifier::Generic = 0;
    virtual auto AddVerificationCredential(
        const protobuf::VerificationSet& verificationSet,
        const PasswordPrompt& reason) -> bool = 0;
    virtual auto AddContactCredential(
        const protobuf::ContactData& contactData,
        const PasswordPrompt& reason) -> bool = 0;
    auto Internal() noexcept -> internal::Authority& final { return *this; }
    virtual void RevokeContactCredentials(
        UnallocatedList<identifier::Generic>& contactCredentialIDs) = 0;
    virtual void RevokeVerificationCredentials(
        UnallocatedList<identifier::Generic>& verificationCredentialIDs) = 0;

    ~Authority() override = default;
};
}  // namespace opentxs::identity::internal
