// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "opentxs/identity/Nym.hpp"

namespace opentxs::identity
{
class NymMock : public Nym
{
public:
    MOCK_METHOD(UnallocatedCString, Alias, (), (const, override));
    MOCK_METHOD(const value_type&, at, (const key_type& id), (const, override));
    MOCK_METHOD(
        const value_type&,
        at,
        (const std::size_t& id),
        (const, override));
    MOCK_METHOD(const_iterator, begin, (), (const, noexcept, override));
    MOCK_METHOD(UnallocatedCString, BestEmail, (), (const, override));
    MOCK_METHOD(UnallocatedCString, BestPhoneNumber, (), (const, override));
    MOCK_METHOD(
        UnallocatedCString,
        BestSocialMediaProfile,
        (const wot::claim::ClaimType type),
        (const, override));
    MOCK_METHOD(const_iterator, cbegin, (), (const, noexcept, override));
    MOCK_METHOD(const_iterator, cend, (), (const, noexcept, override));
    MOCK_METHOD(const wot::claim::Data&, Claims, (), (const, override));
    MOCK_METHOD(bool, CompareID, (const Nym& RHS), (const, override));
    MOCK_METHOD(
        bool,
        CompareID,
        (const identifier::Nym& rhs),
        (const, override));
    MOCK_METHOD(VersionNumber, ContactCredentialVersion, (), (const, override));
    MOCK_METHOD(VersionNumber, ContactDataVersion, (), (const, override));
    MOCK_METHOD(
        UnallocatedSet<OTIdentifier>,
        Contracts,
        (const UnitType currency, const bool onlyActive),
        (const, override));
    MOCK_METHOD(
        UnallocatedCString,
        EmailAddresses,
        (bool active),
        (const, override));
    MOCK_METHOD(NymKeys, EncryptionTargets, (), (const, noexcept, override));
    MOCK_METHOD(const_iterator, end, (), (const, noexcept, override));
    MOCK_METHOD(
        void,
        GetIdentifier,
        (identifier::Nym & theIdentifier),
        (const, override));
    MOCK_METHOD(
        void,
        GetIdentifier,
        (String & theIdentifier),
        (const, override));
    MOCK_METHOD(
        const crypto::key::Asymmetric&,
        GetPrivateAuthKey,
        (crypto::key::asymmetric::Algorithm keytype),
        (const, override));
    MOCK_METHOD(
        const crypto::key::Asymmetric&,
        GetPrivateEncrKey,
        (crypto::key::asymmetric::Algorithm keytype),
        (const, override));
    MOCK_METHOD(
        const crypto::key::Asymmetric&,
        GetPrivateSignKey,
        (crypto::key::asymmetric::Algorithm keytype),
        (const, override));
    MOCK_METHOD(
        const crypto::key::Asymmetric&,
        GetPublicAuthKey,
        (crypto::key::asymmetric::Algorithm keytype),
        (const, override));
    MOCK_METHOD(
        const crypto::key::Asymmetric&,
        GetPublicEncrKey,
        (crypto::key::asymmetric::Algorithm keytype),
        (const, override));
    MOCK_METHOD(
        std::int32_t,
        GetPublicKeysBySignature,
        (crypto::key::Keypair::Keys & listOutput,
         const Signature& theSignature,
         char cKeyType),
        (const, override));
    MOCK_METHOD(
        const crypto::key::Asymmetric&,
        GetPublicSignKey,
        (crypto::key::asymmetric::Algorithm keytype),
        (const, override));
    MOCK_METHOD(
        bool,
        HasCapability,
        (const NymCapability& capability),
        (const, override));
    MOCK_METHOD(bool, HasPath, (), (const, override));
    MOCK_METHOD(const identifier::Nym&, ID, (), (const, override));
    MOCK_METHOD(UnallocatedCString, Name, (), (const, override));
    MOCK_METHOD(bool, Path, (proto::HDPath & output), (const, override));
    MOCK_METHOD(const UnallocatedCString, PathRoot, (), (const, override));
    MOCK_METHOD(int, PathChildSize, (), (const, override));
    MOCK_METHOD(std::uint32_t, PathChild, (int index), (const, override));
    MOCK_METHOD(UnallocatedCString, PaymentCode, (), (const, override));
    MOCK_METHOD(
        bool,
        PaymentCodePath,
        (AllocateOutput destination),
        (const, override));
    MOCK_METHOD(
        bool,
        PaymentCodePath,
        (proto::HDPath & output),
        (const, override));
    MOCK_METHOD(
        UnallocatedCString,
        PhoneNumbers,
        (bool active),
        (const, override));
    MOCK_METHOD(std::uint64_t, Revision, (), (const, override));
    MOCK_METHOD(
        bool,
        Serialize,
        (AllocateOutput destination),
        (const, override));
    MOCK_METHOD(bool, Serialize, (Serialized & serialized), (const, override));
    MOCK_METHOD(void, SerializeNymIDSource, (Tag & parent), (const, override));
    MOCK_METHOD(
        bool,
        Sign,
        (const google::protobuf::MessageLite& input,
         const crypto::SignatureRole role,
         proto::Signature& signature,
         const PasswordPrompt& reason,
         const crypto::HashType hash),
        (const, override));
    MOCK_METHOD(std::size_t, size, (), (const, noexcept, override));
    MOCK_METHOD(
        UnallocatedCString,
        SocialMediaProfiles,
        (const wot::claim::ClaimType type, bool active),
        (const, override));
    MOCK_METHOD(
        const UnallocatedSet<wot::claim::ClaimType>,
        SocialMediaProfileTypes,
        (),
        (const, override));
    MOCK_METHOD(const identity::Source&, Source, (), (const, override));
    MOCK_METHOD(
        OTSecret,
        TransportKey,
        (Data & pubkey, const PasswordPrompt& reason),
        (const, override));
    MOCK_METHOD(
        bool,
        Unlock,
        (const crypto::key::Asymmetric& dhKey,
         const std::uint32_t tag,
         const crypto::key::asymmetric::Algorithm type,
         const crypto::key::Symmetric& key,
         PasswordPrompt& reason),
        (const, noexcept, override));
    MOCK_METHOD(
        bool,
        Verify,
        (const google::protobuf::MessageLite& input,
         proto::Signature& signature),
        (const, override));
    MOCK_METHOD(bool, VerifyPseudonym, (), (const, override));
    MOCK_METHOD(
        UnallocatedCString,
        AddChildKeyCredential,
        (const Identifier& strMasterID,
         const crypto::Parameters& nymParameters,
         const PasswordPrompt& reason),
        (override));
    MOCK_METHOD(
        bool,
        AddClaim,
        (const Claim& claim, const PasswordPrompt& reason),
        (override));
    MOCK_METHOD(
        bool,
        AddContract,
        (const identifier::UnitDefinition& instrumentDefinitionID,
         const UnitType currency,
         const PasswordPrompt& reason,
         const bool primary,
         const bool active),
        (override));
    MOCK_METHOD(
        bool,
        AddEmail,
        (const UnallocatedCString& value,
         const PasswordPrompt& reason,
         const bool primary,
         const bool active),
        (override));
    MOCK_METHOD(
        bool,
        AddPaymentCode,
        (const opentxs::PaymentCode& code,
         const UnitType currency,
         const PasswordPrompt& reason,
         const bool primary,
         const bool active),
        (override));
    MOCK_METHOD(
        bool,
        AddPhoneNumber,
        (const UnallocatedCString& value,
         const PasswordPrompt& reason,
         const bool primary,
         const bool active),
        (override));
    MOCK_METHOD(
        bool,
        AddPreferredOTServer,
        (const Identifier& id,
         const PasswordPrompt& reason,
         const bool primary),
        (override));
    MOCK_METHOD(
        bool,
        AddSocialMediaProfile,
        (const UnallocatedCString& value,
         const wot::claim::ClaimType type,
         const PasswordPrompt& reason,
         const bool primary,
         const bool active),
        (override));
    MOCK_METHOD(
        bool,
        DeleteClaim,
        (const Identifier& id, const PasswordPrompt& reason),
        (override));
    MOCK_METHOD(
        bool,
        SetCommonName,
        (const UnallocatedCString& name, const PasswordPrompt& reason),
        (override));
    MOCK_METHOD(
        bool,
        SetContactData,
        (const ReadView protobuf, const PasswordPrompt& reason),
        (override));
    MOCK_METHOD(
        bool,
        SetContactData,
        (const proto::ContactData& data, const PasswordPrompt& reason),
        (override));
    MOCK_METHOD(
        bool,
        SetScope,
        (const wot::claim::ClaimType type,
         const UnallocatedCString& name,
         const PasswordPrompt& reason,
         const bool primary),
        (override));
};
}  // namespace opentxs::identity