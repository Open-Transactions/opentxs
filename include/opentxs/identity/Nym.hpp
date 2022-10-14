// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <memory>
#include <tuple>

#include "opentxs/Export.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/crypto/key/asymmetric/Algorithm.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Iterator.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace crypto
{
namespace key
{
class Asymmetric;
}  // namespace key

namespace symmetric
{
class Key;
}  // namespace symmetric

class Parameters;
}  // namespace crypto

namespace identifier
{
class Generic;
class Nym;
class UnitDefinition;
}  // namespace identifier

namespace identity
{
namespace internal
{
class Nym;
}  // namespace internal

namespace wot
{
namespace claim
{
class Data;
}  // namespace claim
}  // namespace wot

class Authority;
class Nym;
class Source;
}  // namespace identity

namespace proto
{
class ContactData;
class Nym;
class Signature;
}  // namespace proto

class Data;
class PasswordPrompt;
class PaymentCode;
class Secret;
class Signature;
class String;
class Tag;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity
{
class OPENTXS_EXPORT Nym
{
public:
    using KeyTypes = UnallocatedVector<crypto::key::asymmetric::Algorithm>;
    using AuthorityKeys = std::pair<identifier::Generic, KeyTypes>;
    using NymKeys =
        std::pair<identifier::Nym, UnallocatedVector<AuthorityKeys>>;
    using key_type = identifier::Generic;
    using value_type = Authority;
    using const_iterator =
        opentxs::iterator::Bidirectional<const Nym, const value_type>;

    static const VersionNumber DefaultVersion;
    static const VersionNumber MaxVersion;

    virtual auto Alias() const -> UnallocatedCString = 0;
    virtual auto at(const key_type& id) const noexcept(false)
        -> const value_type& = 0;
    virtual auto at(const std::size_t& index) const noexcept(false)
        -> const value_type& = 0;
    virtual auto begin() const noexcept -> const_iterator = 0;
    virtual auto BestEmail() const -> UnallocatedCString = 0;
    virtual auto BestPhoneNumber() const -> UnallocatedCString = 0;
    virtual auto BestSocialMediaProfile(const wot::claim::ClaimType type) const
        -> UnallocatedCString = 0;
    virtual auto cbegin() const noexcept -> const_iterator = 0;
    virtual auto cend() const noexcept -> const_iterator = 0;
    virtual auto Claims() const -> const wot::claim::Data& = 0;
    virtual auto CompareID(const Nym& RHS) const -> bool = 0;
    virtual auto CompareID(const identifier::Nym& rhs) const -> bool = 0;
    virtual auto ContactCredentialVersion() const -> VersionNumber = 0;
    virtual auto ContactDataVersion() const -> VersionNumber = 0;
    virtual auto Contracts(const UnitType currency, const bool onlyActive) const
        -> UnallocatedSet<identifier::Generic> = 0;
    virtual auto EmailAddresses(bool active = true) const
        -> UnallocatedCString = 0;
    virtual auto EncryptionTargets() const noexcept -> NymKeys = 0;
    virtual auto end() const noexcept -> const_iterator = 0;
    virtual auto GetIdentifier(identifier::Nym& theIdentifier) const
        -> void = 0;
    virtual auto GetIdentifier(String& theIdentifier) const -> void = 0;
    virtual auto GetPrivateAuthKey(
        crypto::key::asymmetric::Algorithm keytype =
            crypto::key::asymmetric::Algorithm::Null) const
        -> const crypto::key::Asymmetric& = 0;
    virtual auto GetPrivateEncrKey(
        crypto::key::asymmetric::Algorithm keytype =
            crypto::key::asymmetric::Algorithm::Null) const
        -> const crypto::key::Asymmetric& = 0;
    virtual auto GetPrivateSignKey(
        crypto::key::asymmetric::Algorithm keytype =
            crypto::key::asymmetric::Algorithm::Null) const
        -> const crypto::key::Asymmetric& = 0;

    virtual auto GetPublicAuthKey(
        crypto::key::asymmetric::Algorithm keytype =
            crypto::key::asymmetric::Algorithm::Null) const
        -> const crypto::key::Asymmetric& = 0;
    virtual auto GetPublicEncrKey(
        crypto::key::asymmetric::Algorithm keytype =
            crypto::key::asymmetric::Algorithm::Null) const
        -> const crypto::key::Asymmetric& = 0;
    virtual auto GetPublicSignKey(
        crypto::key::asymmetric::Algorithm keytype =
            crypto::key::asymmetric::Algorithm::Null) const
        -> const crypto::key::Asymmetric& = 0;
    virtual auto HasCapability(const NymCapability& capability) const
        -> bool = 0;
    virtual auto HasPath() const -> bool = 0;
    virtual auto ID() const -> const identifier::Nym& = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Nym& = 0;
    virtual auto Name() const -> UnallocatedCString = 0;
    virtual auto PathRoot() const -> const UnallocatedCString = 0;
    virtual auto PathChildSize() const -> int = 0;
    virtual auto PathChild(int index) const -> std::uint32_t = 0;
    virtual auto PaymentCode() const -> UnallocatedCString = 0;
    virtual auto PaymentCodePath(AllocateOutput destination) const -> bool = 0;
    virtual auto PhoneNumbers(bool active = true) const
        -> UnallocatedCString = 0;
    virtual auto Revision() const -> std::uint64_t = 0;
    virtual auto Serialize(AllocateOutput destination) const -> bool = 0;
    virtual void SerializeNymIDSource(Tag& parent) const = 0;
    virtual auto size() const noexcept -> std::size_t = 0;
    virtual auto SocialMediaProfiles(
        const wot::claim::ClaimType type,
        bool active = true) const -> UnallocatedCString = 0;
    virtual auto SocialMediaProfileTypes() const
        -> const UnallocatedSet<wot::claim::ClaimType> = 0;
    virtual auto Source() const -> const identity::Source& = 0;
    virtual auto TransportKey(Data& pubkey, const PasswordPrompt& reason) const
        -> Secret = 0;
    virtual auto Unlock(
        const crypto::key::Asymmetric& dhKey,
        const std::uint32_t tag,
        const crypto::key::asymmetric::Algorithm type,
        const crypto::symmetric::Key& key,
        PasswordPrompt& reason) const noexcept -> bool = 0;
    virtual auto VerifyPseudonym() const -> bool = 0;

    virtual auto AddChildKeyCredential(
        const identifier::Generic& strMasterID,
        const crypto::Parameters& nymParameters,
        const PasswordPrompt& reason) -> UnallocatedCString = 0;
    virtual auto AddClaim(const Claim& claim, const PasswordPrompt& reason)
        -> bool = 0;
    virtual auto AddContract(
        const identifier::UnitDefinition& instrumentDefinitionID,
        const UnitType currency,
        const PasswordPrompt& reason,
        const bool primary,
        const bool active = true) -> bool = 0;
    virtual auto AddEmail(
        const UnallocatedCString& value,
        const PasswordPrompt& reason,
        const bool primary,
        const bool active) -> bool = 0;
    virtual auto AddPaymentCode(
        const opentxs::PaymentCode& code,
        const UnitType currency,
        const PasswordPrompt& reason,
        const bool primary,
        const bool active = true) -> bool = 0;
    virtual auto AddPhoneNumber(
        const UnallocatedCString& value,
        const PasswordPrompt& reason,
        const bool primary,
        const bool active) -> bool = 0;
    virtual auto AddPreferredOTServer(
        const identifier::Generic& id,
        const PasswordPrompt& reason,
        const bool primary) -> bool = 0;
    virtual auto AddSocialMediaProfile(
        const UnallocatedCString& value,
        const wot::claim::ClaimType type,
        const PasswordPrompt& reason,
        const bool primary,
        const bool active) -> bool = 0;
    virtual auto DeleteClaim(
        const identifier::Generic& id,
        const PasswordPrompt& reason) -> bool = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() noexcept -> internal::Nym& = 0;
    virtual auto SetCommonName(
        const UnallocatedCString& name,
        const PasswordPrompt& reason) -> bool = 0;
    virtual auto SetContactData(
        const ReadView protobuf,
        const PasswordPrompt& reason) -> bool = 0;
    virtual auto SetScope(
        const wot::claim::ClaimType type,
        const UnallocatedCString& name,
        const PasswordPrompt& reason,
        const bool primary) -> bool = 0;

    Nym(const Nym&) = delete;
    Nym(Nym&&) = delete;
    auto operator=(const Nym&) -> Nym& = delete;
    auto operator=(Nym&&) -> Nym& = delete;

    virtual ~Nym() = default;

protected:
    Nym() noexcept = default;
};
}  // namespace opentxs::identity
