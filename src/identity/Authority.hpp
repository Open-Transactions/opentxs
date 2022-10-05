// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <Enums.pb.h>
#include <cstdint>
#include <memory>
#include <utility>

#include "internal/identity/Authority.hpp"
#include "internal/identity/Types.hpp"
#include "internal/identity/credential/Credential.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/Types.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/crypto/Parameters.hpp"
#include "opentxs/crypto/SignatureRole.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/Keypair.hpp"
#include "opentxs/crypto/key/asymmetric/Algorithm.hpp"
#include "opentxs/crypto/key/asymmetric/Role.hpp"
#include "opentxs/identity/Authority.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/Source.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/identity/credential/Base.hpp"
#include "opentxs/identity/credential/Key.hpp"
#include "opentxs/identity/credential/Primary.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace crypto
{
namespace key
{
class Asymmetric;
class Parameters;
class Symmetric;
}  // namespace key
}  // namespace crypto

namespace identity
{
namespace credential
{
class Base;
class Key;
class Primary;
}  // namespace credential
}  // namespace identity

namespace proto
{
class Authority;
class ContactData;
class Credential;
class HDPath;
class Signature;
class Verification;
class VerificationSet;
}  // namespace proto

class Data;
class Factory;
class PasswordPrompt;
class Secret;
class Signature;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::implementation
{
class Authority final : virtual public identity::internal::Authority
{
public:
    auto ContactCredentialVersion() const -> VersionNumber final
    {
        return authority_to_contact_.at(version_);
    }
    auto EncryptionTargets() const noexcept -> AuthorityKeys final;
    auto GetContactData(proto::ContactData& contactData) const -> bool final;
    auto GetMasterCredential() const -> const credential::Primary& final
    {
        return *master_;
    }
    auto GetMasterCredID() const -> identifier::Generic final;
    auto GetPublicAuthKey(
        crypto::key::asymmetric::Algorithm keytype,
        const String::List* plistRevokedIDs = nullptr) const
        -> const crypto::key::Asymmetric& final;
    auto GetPublicEncrKey(
        crypto::key::asymmetric::Algorithm keytype,
        const String::List* plistRevokedIDs = nullptr) const
        -> const crypto::key::Asymmetric& final;
    auto GetPublicKeysBySignature(
        crypto::key::Keypair::Keys& listOutput,
        const Signature& theSignature,
        char cKeyType = '0') const -> std::int32_t final;
    auto GetPublicSignKey(
        crypto::key::asymmetric::Algorithm keytype,
        const String::List* plistRevokedIDs = nullptr) const
        -> const crypto::key::Asymmetric& final;
    auto GetPrivateSignKey(
        crypto::key::asymmetric::Algorithm keytype,
        const String::List* plistRevokedIDs = nullptr) const
        -> const crypto::key::Asymmetric& final;
    auto GetPrivateEncrKey(
        crypto::key::asymmetric::Algorithm keytype,
        const String::List* plistRevokedIDs = nullptr) const
        -> const crypto::key::Asymmetric& final;
    auto GetPrivateAuthKey(
        crypto::key::asymmetric::Algorithm keytype,
        const String::List* plistRevokedIDs = nullptr) const
        -> const crypto::key::Asymmetric& final;
    auto GetAuthKeypair(
        crypto::key::asymmetric::Algorithm keytype,
        const String::List* plistRevokedIDs = nullptr) const
        -> const crypto::key::Keypair& final;
    auto GetEncrKeypair(
        crypto::key::asymmetric::Algorithm keytype,
        const String::List* plistRevokedIDs = nullptr) const
        -> const crypto::key::Keypair& final;
    auto GetSignKeypair(
        crypto::key::asymmetric::Algorithm keytype,
        const String::List* plistRevokedIDs = nullptr) const
        -> const crypto::key::Keypair& final;
    auto GetTagCredential(crypto::key::asymmetric::Algorithm keytype) const
        noexcept(false) -> const credential::Key& final;
    auto GetVerificationSet(proto::VerificationSet& verificationSet) const
        -> bool final;
    auto hasCapability(const NymCapability& capability) const -> bool final;
    auto Params(const crypto::key::asymmetric::Algorithm type) const noexcept
        -> ReadView final;
    auto Path(proto::HDPath& output) const -> bool final;
    auto Serialize(Serialized& serialized, const CredentialIndexModeFlag mode)
        const -> bool final;
    auto Sign(
        const GetPreimage input,
        const crypto::SignatureRole role,
        proto::Signature& signature,
        const PasswordPrompt& reason,
        opentxs::crypto::key::asymmetric::Role key,
        const crypto::HashType hash) const -> bool final;
    auto Source() const -> const identity::Source& final
    {
        return parent_.Source();
    }
    auto TransportKey(
        Data& publicKey,
        Secret& privateKey,
        const PasswordPrompt& reason) const -> bool final;
    auto Unlock(
        const crypto::key::Asymmetric& dhKey,
        const std::uint32_t tag,
        const crypto::key::asymmetric::Algorithm type,
        const crypto::key::Symmetric& key,
        PasswordPrompt& reason) const noexcept -> bool final;
    auto VerificationCredentialVersion() const -> VersionNumber final
    {
        return authority_to_verification_.at(version_);
    }
    auto Verify(
        const Data& plaintext,
        const proto::Signature& sig,
        const opentxs::crypto::key::asymmetric::Role key) const -> bool final;
    auto Verify(const proto::Verification& item) const -> bool final;
    auto VerifyInternally() const -> bool final;

    auto AddChildKeyCredential(
        const crypto::Parameters& parameters,
        const PasswordPrompt& reason) -> UnallocatedCString final;
    auto AddVerificationCredential(
        const proto::VerificationSet& verificationSet,
        const PasswordPrompt& reason) -> bool final;
    auto AddContactCredential(
        const proto::ContactData& contactData,
        const PasswordPrompt& reason) -> bool final;
    void RevokeContactCredentials(
        UnallocatedList<UnallocatedCString>& contactCredentialIDs) final;
    void RevokeVerificationCredentials(
        UnallocatedList<UnallocatedCString>& verificationCredentialIDs) final;
    auto WriteCredentials() const -> bool final;

    Authority() = delete;
    Authority(const Authority&) = delete;
    Authority(Authority&&) = delete;
    auto operator=(const Authority&) -> Authority& = delete;
    auto operator=(Authority&&) -> Authority& = delete;

    ~Authority() final = default;

private:
    friend opentxs::Factory;
    friend internal::Authority;

    using ContactCredentialMap = UnallocatedMap<
        identifier::Generic,
        std::unique_ptr<credential::internal::Contact>>;
    using KeyCredentialMap = UnallocatedMap<
        identifier::Generic,
        std::unique_ptr<credential::internal::Secondary>>;
    using KeyCredentialItem = std::pair<
        identifier::Generic,
        std::unique_ptr<credential::internal::Secondary>>;
    using VerificationCredentialMap = UnallocatedMap<
        identifier::Generic,
        std::unique_ptr<credential::internal::Verification>>;
    using mapOfCredentials = UnallocatedMap<
        UnallocatedCString,
        std::unique_ptr<credential::internal::Base>>;

    static const VersionConversionMap authority_to_contact_;
    static const VersionConversionMap authority_to_primary_;
    static const VersionConversionMap authority_to_secondary_;
    static const VersionConversionMap authority_to_verification_;
    static const VersionConversionMap nym_to_authority_;

    const api::Session& api_;
    const identity::Nym& parent_;
    const VersionNumber version_{0};
    std::uint32_t index_{0};
    std::unique_ptr<credential::internal::Primary> master_;
    KeyCredentialMap key_credentials_;
    ContactCredentialMap contact_credentials_;
    VerificationCredentialMap verification_credentials_;
    mapOfCredentials revoked_credentials_;
    proto::KeyMode mode_{proto::KEYMODE_ERROR};

    static auto is_revoked(
        const UnallocatedCString& id,
        const String::List* plistRevokedIDs) -> bool;
    static auto create_child_credential(
        const api::Session& api,
        const crypto::Parameters& parameters,
        const identity::Source& source,
        const credential::internal::Primary& master,
        internal::Authority& parent,
        const VersionNumber parentVersion,
        Bip32Index& index,
        const opentxs::PasswordPrompt& reason) noexcept(false)
        -> KeyCredentialMap;
    static auto create_contact_credental(
        const api::Session& api,
        const crypto::Parameters& parameters,
        const identity::Source& source,
        const credential::internal::Primary& master,
        internal::Authority& parent,
        const VersionNumber parentVersion,
        const opentxs::PasswordPrompt& reason) noexcept(false)
        -> ContactCredentialMap;
    static auto create_key_credential(
        const api::Session& api,
        const crypto::Parameters& parameters,
        const identity::Source& source,
        const credential::internal::Primary& master,
        internal::Authority& parent,
        const VersionNumber parentVersion,
        Bip32Index& index,
        const opentxs::PasswordPrompt& reason) noexcept(false)
        -> KeyCredentialItem;
    static auto create_master(
        const api::Session& api,
        identity::internal::Authority& owner,
        const identity::Source& source,
        const VersionNumber version,
        const crypto::Parameters& parameters,
        const Bip32Index index,
        const opentxs::PasswordPrompt& reason) noexcept(false)
        -> std::unique_ptr<credential::internal::Primary>;
    template <typename Type>
    static void extract_child(
        const api::Session& api,
        const identity::Source& source,
        internal::Authority& authority,
        const credential::internal::Primary& master,
        const credential::internal::Base::SerializedType& serialized,
        const proto::KeyMode mode,
        const proto::CredentialRole role,
        UnallocatedMap<identifier::Generic, std::unique_ptr<Type>>&
            map) noexcept(false);
    static auto load_master(
        const api::Session& api,
        identity::internal::Authority& owner,
        const identity::Source& source,
        const proto::KeyMode mode,
        const Serialized& serialized) noexcept(false)
        -> std::unique_ptr<credential::internal::Primary>;
    template <typename Type>
    static auto load_child(
        const api::Session& api,
        const identity::Source& source,
        internal::Authority& authority,
        const credential::internal::Primary& master,
        const Serialized& serialized,
        const proto::KeyMode mode,
        const proto::CredentialRole role) noexcept(false)
        -> UnallocatedMap<identifier::Generic, std::unique_ptr<Type>>;

    auto get_keypair(
        const crypto::key::asymmetric::Algorithm type,
        const proto::KeyRole role,
        const String::List* plistRevokedIDs) const
        -> const crypto::key::Keypair&;
    auto get_secondary_credential(
        const UnallocatedCString& strSubID,
        const String::List* plistRevokedIDs = nullptr) const
        -> const credential::Base*;

    template <typename Item>
    auto validate_credential(const Item& item) const -> bool;

    auto LoadChildKeyCredential(const String& strSubID) -> bool;
    auto LoadChildKeyCredential(const proto::Credential& serializedCred)
        -> bool;

    Authority(
        const api::Session& api,
        const identity::Nym& parent,
        const identity::Source& source,
        const proto::KeyMode mode,
        const Serialized& serialized) noexcept(false);
    Authority(
        const api::Session& api,
        const identity::Nym& parent,
        const identity::Source& source,
        const crypto::Parameters& parameters,
        VersionNumber nymVersion,
        const PasswordPrompt& reason) noexcept(false);
};
}  // namespace opentxs::identity::implementation
