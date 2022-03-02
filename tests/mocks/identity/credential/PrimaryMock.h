// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "opentxs/identity/credential/Primary.hpp"

namespace opentxs::identity::credential
{
class PrimaryMock : public Primary {
public:
    MOCK_METHOD(UnallocatedCString, Path, (), (const, override));
    MOCK_METHOD(bool, Path, (proto::HDPath& output), (const, override));
    MOCK_METHOD(UnallocatedCString, Alias, (), (const, noexcept, override));
    MOCK_METHOD(OTIdentifier, ID, (), (const, noexcept, override));
    MOCK_METHOD(UnallocatedCString, Name, (), (const, noexcept, override));
    MOCK_METHOD(Nym_p, Nym, (), (const, noexcept, override));
    MOCK_METHOD(const UnallocatedCString&, Terms, (), (const, noexcept, override));
    MOCK_METHOD(OTData, Serialize, (), (const, noexcept, override));
    MOCK_METHOD(bool, Validate, (), (const, noexcept, override));
    MOCK_METHOD(VersionNumber, Version, (), (const, noexcept, override));
    MOCK_METHOD(Signable*, clone, (), (const, noexcept, override));
    MOCK_METHOD(UnallocatedCString, asString, (const bool asPrivate), (const, noexcept, override));
    MOCK_METHOD(const Identifier&, CredentialID, (), (const, noexcept, override));
    MOCK_METHOD(bool, GetContactData, (proto::ContactData& contactData), (const, noexcept, override));
    MOCK_METHOD(bool, GetVerificationSet, (proto::VerificationSet& verificationSet), (const, noexcept, override));
    MOCK_METHOD(bool, hasCapability, (const NymCapability& capability), (const, noexcept, override));
    MOCK_METHOD(Signature, MasterSignature, (), (const, noexcept, override));
    MOCK_METHOD(crypto::key::asymmetric::Mode, Mode, (), (const, noexcept, override));
    MOCK_METHOD(identity::CredentialRole, Role, (), (const, noexcept, override));
    MOCK_METHOD(bool, Private, (), (const, noexcept, override));
    MOCK_METHOD(bool, Save, (), (const, noexcept, override));
    MOCK_METHOD(Signature, SelfSignature, (CredentialModeFlag version), (const, noexcept, override));
    MOCK_METHOD(bool, Serialize, (SerializedType& serialized,
                                     const SerializationModeFlag asPrivate,
                                     const SerializationSignatureFlag asSigned), (const, noexcept, override));
    MOCK_METHOD(Signature, SourceSignature, (), (const, noexcept, override));
    MOCK_METHOD(bool, TransportKey, (Data& publicKey,
                                     Secret& privateKey,
                                     const PasswordPrompt& reason), (const, noexcept, override));

    MOCK_METHOD(identity::CredentialType, Type, (), (const, noexcept, override));
    MOCK_METHOD(bool, Verify, (const Data& plaintext,
                                                 const proto::Signature& sig,
                                                 const opentxs::crypto::key::asymmetric::Role key), (const, noexcept, override));
    MOCK_METHOD(bool, Verify, (const proto::Credential& credential,
                               const identity::CredentialRole& role,
                               const Identifier& masterID,
                               const proto::Signature& masterSig), (const, noexcept, override));
    MOCK_METHOD(bool, SetAlias, (const UnallocatedCString& alias), (noexcept, override));
    MOCK_METHOD( const crypto::key::Keypair&, GetKeypair, (const crypto::key::asymmetric::Algorithm type,
                                      const opentxs::crypto::key::asymmetric::Role role), (const, override));
    MOCK_METHOD( const crypto::key::Keypair&, GetKeypair, (const opentxs::crypto::key::asymmetric::Role role), (const, override));
    MOCK_METHOD(std::int32_t, GetPublicKeysBySignature, (crypto::key::Keypair::Keys& listOutput,
                                                         const opentxs::Signature& theSignature,
                                                         char cKeyType), (const, override));
    MOCK_METHOD(bool, Sign, (const GetPreimage input,
                                     const crypto::SignatureRole role,
                                     proto::Signature& signature,
                                     const PasswordPrompt& reason,
                                     opentxs::crypto::key::asymmetric::Role key,
                                     const crypto::HashType hash), (const, override));
};
}