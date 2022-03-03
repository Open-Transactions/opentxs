// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <memory>

#include "opentxs/core/String.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/Keypair.hpp"
#include "opentxs/crypto/key/asymmetric/Role.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/util/Bytes.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace identity
{
namespace credential
{
class Key;
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

class Secret;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity
{
class OPENTXS_EXPORT Authority
{
public:
    using AuthorityKeys = Nym::AuthorityKeys;
    using Serialized = proto::Authority;

    virtual auto ContactCredentialVersion() const -> VersionNumber = 0;
    virtual auto EncryptionTargets() const noexcept -> AuthorityKeys = 0;
    virtual auto GetContactData(proto::ContactData& contactData) const
        -> bool = 0;
    virtual auto GetMasterCredID() const -> OTIdentifier = 0;
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
    virtual auto GetTagCredential(crypto::key::asymmetric::Algorithm keytype)
        const noexcept(false) -> const credential::Key& = 0;
    virtual auto GetVerificationSet(
        proto::VerificationSet& verificationSet) const -> bool = 0;
    virtual auto hasCapability(const NymCapability& capability) const
        -> bool = 0;
    virtual auto Params(const crypto::key::asymmetric::Algorithm type)
        const noexcept -> ReadView = 0;
    virtual auto Path(proto::HDPath& output) const -> bool = 0;
    OPENTXS_NO_EXPORT virtual auto Serialize(
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
    virtual auto Source() const -> const identity::Source& = 0;
    virtual auto TransportKey(
        Data& publicKey,
        Secret& privateKey,
        const PasswordPrompt& reason) const -> bool = 0;
    virtual auto Unlock(
        const crypto::key::Asymmetric& dhKey,
        const std::uint32_t tag,
        const crypto::key::asymmetric::Algorithm type,
        const crypto::key::Symmetric& key,
        PasswordPrompt& reason) const noexcept -> bool = 0;
    virtual auto VerificationCredentialVersion() const -> VersionNumber = 0;
    virtual auto Verify(
        const Data& plaintext,
        const proto::Signature& sig,
        const opentxs::crypto::key::asymmetric::Role key =
            opentxs::crypto::key::asymmetric::Role::Sign) const -> bool = 0;
    virtual auto Verify(const proto::Verification& item) const -> bool = 0;
    virtual auto VerifyInternally() const -> bool = 0;

    virtual auto AddChildKeyCredential(
        const crypto::Parameters& nymParameters,
        const PasswordPrompt& reason) -> UnallocatedCString = 0;
    virtual auto AddVerificationCredential(
        const proto::VerificationSet& verificationSet,
        const PasswordPrompt& reason) -> bool = 0;
    virtual auto AddContactCredential(
        const proto::ContactData& contactData,
        const PasswordPrompt& reason) -> bool = 0;
    virtual void RevokeContactCredentials(
        UnallocatedList<UnallocatedCString>& contactCredentialIDs) = 0;
    virtual void RevokeVerificationCredentials(
        UnallocatedList<UnallocatedCString>& verificationCredentialIDs) = 0;

    virtual ~Authority() = default;

protected:
    Authority() noexcept = default;

private:
    Authority(const Authority&) = delete;
    Authority(Authority&&) = delete;
    auto operator=(const Authority&) -> Authority& = delete;
    auto operator=(Authority&&) -> Authority& = delete;
};
}  // namespace opentxs::identity
