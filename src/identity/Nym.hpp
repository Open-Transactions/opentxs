// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::proto::NymMode
// IWYU pragma: no_include "opentxs/util/Writer.hpp"

#pragma once

#include <Enums.pb.h>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>

#include "internal/core/String.hpp"
#include "internal/crypto/key/Keypair.hpp"
#include "internal/identity/Authority.hpp"
#include "internal/identity/Nym.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/Lockable.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/Types.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Parameters.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/identity/Source.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/identity/wot/claim/Data.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace crypto
{
namespace asymmetric
{
class Key;
}  // namespace asymmetric

namespace symmetric
{
class Key;
}  // namespace symmetric
}  // namespace crypto

namespace identifier
{
class UnitDefinition;
}  // namespace identifier

namespace identity
{
namespace wot
{
class Claim;
class Verification;
}  // namespace wot

class Nym;
}  // namespace identity

namespace proto
{
class ContactData;
class HDPath;
class Nym;
class Signature;
class VerificationSet;
}  // namespace proto

class Data;
class Factory;
class PasswordPrompt;
class Signature;
class Tag;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::implementation
{
class Nym final : virtual public identity::internal::Nym, Lockable
{
public:
    auto Alias() const -> std::string_view final;
    auto at(const key_type& id) const noexcept(false) -> const value_type& final
    {
        return *active_.at(id);
    }
    auto at(const std::size_t& index) const noexcept(false)
        -> const value_type& final;
    auto begin() const noexcept -> const_iterator final { return cbegin(); }
    auto BestEmail() const -> UnallocatedCString final;
    auto BestPhoneNumber() const -> UnallocatedCString final;
    auto BestSocialMediaProfile(const wot::claim::ClaimType type) const
        -> UnallocatedCString final;
    auto cbegin() const noexcept -> const_iterator final { return {this, 0}; }
    auto cend() const noexcept -> const_iterator final
    {
        return {this, size()};
    }
    auto Claims() const -> const wot::claim::Data& final;
    auto CompareID(const identity::Nym& RHS) const -> bool final;
    auto CompareID(const identifier::Nym& rhs) const -> bool final;
    auto ContactCredentialVersion() const -> VersionNumber final;
    auto ContactDataVersion() const -> VersionNumber final
    {
        return contact_credential_to_contact_data_version_.at(
            ContactCredentialVersion());
    }
    auto Contracts(const UnitType currency, const bool onlyActive) const
        -> UnallocatedSet<identifier::Generic> final;
    auto EmailAddresses(bool active) const -> UnallocatedCString final;
    auto EncryptionTargets() const noexcept -> NymKeys final;
    auto end() const noexcept -> const_iterator final { return cend(); }
    void GetIdentifier(identifier::Nym& theIdentifier) const final;
    void GetIdentifier(String& theIdentifier) const final;
    auto GetPrivateAuthKey() const -> const crypto::asymmetric::Key& final;
    auto GetPrivateAuthKey(crypto::asymmetric::Algorithm keytype) const
        -> const crypto::asymmetric::Key& final;
    auto GetPrivateEncrKey() const -> const crypto::asymmetric::Key& final;
    auto GetPrivateEncrKey(crypto::asymmetric::Algorithm keytype) const
        -> const crypto::asymmetric::Key& final;
    auto GetPrivateSignKey() const -> const crypto::asymmetric::Key& final;
    auto GetPrivateSignKey(crypto::asymmetric::Algorithm keytype) const
        -> const crypto::asymmetric::Key& final;
    auto GetPublicAuthKey() const -> const crypto::asymmetric::Key& final;
    auto GetPublicAuthKey(crypto::asymmetric::Algorithm keytype) const
        -> const crypto::asymmetric::Key& final;
    auto GetPublicEncrKey() const -> const crypto::asymmetric::Key& final;
    auto GetPublicEncrKey(crypto::asymmetric::Algorithm keytype) const
        -> const crypto::asymmetric::Key& final;
    auto GetPublicSignKey() const -> const crypto::asymmetric::Key& final;
    auto GetPublicSignKey(crypto::asymmetric::Algorithm keytype) const
        -> const crypto::asymmetric::Key& final;
    auto GetPublicKeysBySignature(
        crypto::key::Keypair::Keys& listOutput,
        const Signature& theSignature,
        char cKeyType) const -> std::int32_t final;
    auto HasCapability(const NymCapability& capability) const -> bool final;
    auto HasPath() const -> bool final;
    auto ID() const -> const identifier::Nym& final { return id_; }
    auto Name() const -> UnallocatedCString final;
    auto Path(proto::HDPath& output) const -> bool final;
    auto PathRoot() const -> const UnallocatedCString final;
    auto PathChildSize() const -> int final;
    auto PathChild(int index) const -> std::uint32_t final;
    auto PaymentCodePublic() const -> opentxs::PaymentCode final;
    auto PaymentCodeSecret(const PasswordPrompt& reason) const
        -> opentxs::PaymentCode final;
    auto PaymentCodePath(proto::HDPath& output) const -> bool final;
    auto PaymentCodePath(Writer&& destination) const -> bool final;
    auto PhoneNumbers(bool active) const -> UnallocatedCString final;
    auto Revision() const -> std::uint64_t final;
    auto Serialize(Writer&& destination) const -> bool final;
    auto Serialize(Serialized& serialized) const -> bool final;
    auto SerializeCredentialIndex(Writer&&, const Mode mode) const
        -> bool final;
    auto SerializeCredentialIndex(Serialized& serialized, const Mode mode) const
        -> bool final;
    void SerializeNymIDSource(Tag& parent) const final;
    auto size() const noexcept -> std::size_t final { return active_.size(); }
    auto SocialMediaProfiles(const wot::claim::ClaimType type, bool active)
        const -> UnallocatedCString final;
    auto SocialMediaProfileTypes() const
        -> const UnallocatedSet<wot::claim::ClaimType> final;
    auto Source() const -> const identity::Source& final { return source_; }
    auto TransportKey(Data& pubkey, const PasswordPrompt& reason) const
        -> Secret final;
    auto Unlock(
        const crypto::asymmetric::Key& dhKey,
        const std::uint32_t tag,
        const crypto::asymmetric::Algorithm type,
        const crypto::symmetric::Key& key,
        PasswordPrompt& reason) const noexcept -> bool final;
    auto VerifyPseudonym() const -> bool final;
    auto WriteCredentials() const -> bool final;

    auto AddChildKeyCredential(
        const identifier::Generic& strMasterID,
        const crypto::Parameters& nymParameters,
        const PasswordPrompt& reason) -> UnallocatedCString final;
    auto AddClaim(const wot::Claim& claim, const PasswordPrompt& reason)
        -> bool final;
    auto AddContract(
        const identifier::UnitDefinition& instrumentDefinitionID,
        const UnitType currency,
        const PasswordPrompt& reason,
        const bool primary,
        const bool active) -> bool final;
    auto AddEmail(
        const UnallocatedCString& value,
        const PasswordPrompt& reason,
        const bool primary,
        const bool active) -> bool final;
    auto AddPaymentCode(
        const opentxs::PaymentCode& code,
        const UnitType currency,
        const PasswordPrompt& reason,
        const bool primary,
        const bool active) -> bool final;
    auto AddPreferredOTServer(
        const identifier::Generic& id,
        const PasswordPrompt& reason,
        const bool primary) -> bool final;
    auto AddPhoneNumber(
        const UnallocatedCString& value,
        const PasswordPrompt& reason,
        const bool primary,
        const bool active) -> bool final;
    auto AddSocialMediaProfile(
        const UnallocatedCString& value,
        const wot::claim::ClaimType type,
        const PasswordPrompt& reason,
        const bool primary,
        const bool active) -> bool final;
    auto AddVerification(
        const wot::Verification& verification,
        const PasswordPrompt& reason) -> bool final;
    auto DeleteClaim(
        const identifier::Generic& id,
        const PasswordPrompt& reason) -> bool final;
    void SetAlias(std::string_view alias) final;
    void SetAliasStartup(std::string_view alias) final { alias_ = alias; }
    auto SetCommonName(
        const UnallocatedCString& name,
        const PasswordPrompt& reason) -> bool final;
    auto SetContactData(const ReadView protobuf, const PasswordPrompt& reason)
        -> bool final;
    auto SetContactData(
        const proto::ContactData& data,
        const PasswordPrompt& reason) -> bool final;
    auto SetScope(
        const wot::claim::ClaimType type,
        const UnallocatedCString& name,
        const PasswordPrompt& reason,
        const bool primary) -> bool final;
    auto Sign(
        const ProtobufType& input,
        const crypto::SignatureRole role,
        proto::Signature& signature,
        const PasswordPrompt& reason,
        const crypto::HashType hash) const -> bool final;
    auto Verify(const ProtobufType& input, proto::Signature& signature) const
        -> bool final;

    Nym() = delete;
    Nym(const Nym&) = delete;
    Nym(Nym&&) = delete;
    auto operator=(const Nym&) -> Nym& = delete;
    auto operator=(Nym&&) -> Nym& = delete;

    ~Nym() final;

private:
    using MasterID = identifier::Generic;
    using CredentialMap = UnallocatedMap<
        MasterID,
        std::unique_ptr<identity::internal::Authority>>;

    friend opentxs::Factory;

    static const VersionConversionMap akey_to_session_key_version_;
    static const VersionConversionMap
        contact_credential_to_contact_data_version_;

    const api::Session& api_;
    const std::unique_ptr<const identity::Source> source_p_;
    const identity::Source& source_;
    const identifier::Nym id_;
    const proto::NymMode mode_;
    std::int32_t version_;
    std::uint32_t index_;
    UnallocatedCString alias_;
    std::atomic<std::uint64_t> revision_;
    mutable std::unique_ptr<wot::claim::Data> contact_data_;
    CredentialMap active_;
    CredentialMap revoked_sets_;
    // Revoked child credential IDs
    String::List list_revoked_ids_;

    static auto create_authority(
        const api::Session& api,
        const identity::Nym& parent,
        const identity::Source& source,
        const VersionNumber version,
        const crypto::Parameters& params,
        const PasswordPrompt& reason) noexcept(false) -> CredentialMap;
    static auto load_authorities(
        const api::Session& api,
        const identity::Nym& parent,
        const identity::Source& source,
        const Serialized& serialized) noexcept(false) -> CredentialMap;
    static auto load_revoked(
        const api::Session& api,
        const identity::Nym& parent,
        const identity::Source& source,
        const Serialized& serialized,
        CredentialMap& revoked) noexcept(false) -> String::List;
    static auto normalize(
        const api::Session& api,
        const crypto::Parameters& in,
        const PasswordPrompt& reason) noexcept(false) -> crypto::Parameters;

    template <typename T>
    auto get_private_auth_key(
        const T& lock,
        crypto::asymmetric::Algorithm keytype) const
        -> const crypto::asymmetric::Key&;
    template <typename T>
    auto get_private_sign_key(
        const T& lock,
        crypto::asymmetric::Algorithm keytype) const
        -> const crypto::asymmetric::Key&;
    template <typename T>
    auto get_public_sign_key(
        const T& lock,
        crypto::asymmetric::Algorithm keytype) const
        -> const crypto::asymmetric::Key&;
    auto has_capability(const eLock& lock, const NymCapability& capability)
        const -> bool;
    void init_claims(const eLock& lock) const;
    auto set_contact_data(
        const eLock& lock,
        const proto::ContactData& data,
        const PasswordPrompt& reason) -> bool;
    auto verify_pseudonym(const eLock& lock) const -> bool;

    auto add_contact_credential(
        const eLock& lock,
        const proto::ContactData& data,
        const PasswordPrompt& reason) -> bool;
    auto add_verification_credential(
        const eLock& lock,
        const proto::VerificationSet& data,
        const PasswordPrompt& reason) -> bool;
    void revoke_contact_credentials(const eLock& lock);
    void revoke_verification_credentials(const eLock& lock);
    auto update_nym(
        const eLock& lock,
        const std::int32_t version,
        const PasswordPrompt& reason) -> bool;
    auto path(const sLock& lock, proto::HDPath& output) const -> bool;

    Nym(const api::Session& api,
        crypto::Parameters& nymParameters,
        std::unique_ptr<const identity::Source> source,
        const PasswordPrompt& reason) noexcept(false);
    Nym(const api::Session& api,
        const proto::Nym& serialized,
        std::string_view alias) noexcept(false);
};
}  // namespace opentxs::identity::implementation
