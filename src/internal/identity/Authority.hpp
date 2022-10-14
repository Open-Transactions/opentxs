// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/identity/Authority.hpp"

#include "internal/core/String.hpp"
#include "internal/crypto/key/Keypair.hpp"
#include "internal/identity/Types.hpp"
#include "opentxs/crypto/key/asymmetric/Role.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identity
{
namespace credential
{
class Primary;
}  // namespace credential
}  // namespace identity

namespace proto
{
class Authority;
class ContactData;
class HDPath;
class Signature;
class Verification;
class VerificationSet;
}  // namespace proto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::internal
{
class Authority : virtual public identity::Authority
{
public:
    using AuthorityKeys = identity::Nym::AuthorityKeys;
    using Serialized = proto::Authority;

    static auto NymToContactCredential(const VersionNumber nym) noexcept(false)
        -> VersionNumber;

    virtual auto EncryptionTargets() const noexcept -> AuthorityKeys = 0;
    virtual auto GetContactData(proto::ContactData& contactData) const
        -> bool = 0;
    virtual auto GetMasterCredential() const -> const credential::Primary& = 0;
    auto Internal() const noexcept -> const internal::Authority& final
    {
        return *this;
    }
    virtual auto GetPublicAuthKey(
        crypto::key::asymmetric::Algorithm keytype,
        const String::List* plistRevokedIDs = nullptr) const
        -> const crypto::key::Asymmetric& = 0;
    virtual auto GetPublicEncrKey(
        crypto::key::asymmetric::Algorithm keytype,
        const String::List* plistRevokedIDs = nullptr) const
        -> const crypto::key::Asymmetric& = 0;
    virtual auto GetPublicKeysBySignature(
        crypto::key::Keypair::Keys& listOutput,
        const Signature& theSignature,
        char cKeyType = '0') const -> std::int32_t = 0;
    virtual auto GetPublicSignKey(
        crypto::key::asymmetric::Algorithm keytype,
        const String::List* plistRevokedIDs = nullptr) const
        -> const crypto::key::Asymmetric& = 0;
    virtual auto GetPrivateSignKey(
        crypto::key::asymmetric::Algorithm keytype,
        const String::List* plistRevokedIDs = nullptr) const
        -> const crypto::key::Asymmetric& = 0;
    virtual auto GetPrivateEncrKey(
        crypto::key::asymmetric::Algorithm keytype,
        const String::List* plistRevokedIDs = nullptr) const
        -> const crypto::key::Asymmetric& = 0;
    virtual auto GetPrivateAuthKey(
        crypto::key::asymmetric::Algorithm keytype,
        const String::List* plistRevokedIDs = nullptr) const
        -> const crypto::key::Asymmetric& = 0;
    virtual auto GetAuthKeypair(
        crypto::key::asymmetric::Algorithm keytype,
        const String::List* plistRevokedIDs = nullptr) const
        -> const crypto::key::Keypair& = 0;
    virtual auto GetEncrKeypair(
        crypto::key::asymmetric::Algorithm keytype,
        const String::List* plistRevokedIDs = nullptr) const
        -> const crypto::key::Keypair& = 0;
    virtual auto GetSignKeypair(
        crypto::key::asymmetric::Algorithm keytype,
        const String::List* plistRevokedIDs = nullptr) const
        -> const crypto::key::Keypair& = 0;
    virtual auto GetVerificationSet(
        proto::VerificationSet& verificationSet) const -> bool = 0;
    virtual auto Path(proto::HDPath& output) const -> bool = 0;
    virtual auto Serialize(
        Serialized& serialized,
        const CredentialIndexModeFlag mode) const -> bool = 0;
    virtual auto Sign(
        const GetPreimage input,
        const crypto::SignatureRole role,
        proto::Signature& signature,
        const PasswordPrompt& reason,
        opentxs::crypto::key::asymmetric::Role key =
            opentxs::crypto::key::asymmetric::Role::Sign,
        const crypto::HashType hash = crypto::HashType::Error) const
        -> bool = 0;
    virtual auto Verify(
        const Data& plaintext,
        const proto::Signature& sig,
        const opentxs::crypto::key::asymmetric::Role key =
            opentxs::crypto::key::asymmetric::Role::Sign) const -> bool = 0;
    virtual auto Verify(const proto::Verification& item) const -> bool = 0;
    virtual auto VerifyInternally() const -> bool = 0;
    virtual auto WriteCredentials() const -> bool = 0;

    virtual auto AddChildKeyCredential(
        const crypto::Parameters& nymParameters,
        const PasswordPrompt& reason) -> UnallocatedCString = 0;
    virtual auto AddVerificationCredential(
        const proto::VerificationSet& verificationSet,
        const PasswordPrompt& reason) -> bool = 0;
    virtual auto AddContactCredential(
        const proto::ContactData& contactData,
        const PasswordPrompt& reason) -> bool = 0;
    auto Internal() noexcept -> internal::Authority& final { return *this; }
    virtual void RevokeContactCredentials(
        UnallocatedList<UnallocatedCString>& contactCredentialIDs) = 0;
    virtual void RevokeVerificationCredentials(
        UnallocatedList<UnallocatedCString>& verificationCredentialIDs) = 0;

    ~Authority() override = default;
};
}  // namespace opentxs::identity::internal
