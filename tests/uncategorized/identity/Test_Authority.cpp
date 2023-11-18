// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <Authority.pb.h>
#include <ContactData.pb.h>
#include <Enums.pb.h>
#include <HDPath.pb.h>
#include <Signature.pb.h>
#include <VerificationItem.pb.h>
#include <VerificationSet.pb.h>
#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstdint>
#include <functional>
#include <memory>
#include <utility>

#include "internal/crypto/key/Keypair.hpp"
#include "internal/identity/Authority.hpp"
#include "internal/identity/Nym.hpp"
#include "internal/identity/Types.hpp"
#include "internal/identity/credential/Credential.hpp"
#include "internal/otx/common/crypto/Signature.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "ottest/fixtures/core/Identifier.hpp"
#include "ottest/fixtures/identity/Authority.hpp"

namespace ottest
{
TEST_F(Authority, GetPublicAuthKey_DefaultSetup_ShouldReturnDefaultKey)
{
    const auto& asymmetricKey = authority_->GetPublicAuthKey(
        ot::crypto::asymmetric::Algorithm::Secp256k1);
    EXPECT_EQ(
        asymmetricKey.Type(), ot::crypto::asymmetric::Algorithm::Secp256k1);
    EXPECT_EQ(asymmetricKey.Role(), ot::crypto::asymmetric::Role::Auth);
}

TEST_F(
    Authority,
    GetPublicAuthKey_AddedPublicKeyWIthDifferentAlgorith_ShouldReturnProperData)
{
    ot::crypto::Parameters parameters{
        api_.Factory(),
        ot::crypto::ParameterType::ed25519,
        ot::identity::CredentialType::HD,
        ot::identity::SourceType::PubKey,
        2};

    parameters.SetSeed(parameters_.Seed());
    authority_->AddChildKeyCredential(parameters, reason_);

    const auto& asymmetricKey = authority_->GetPublicAuthKey(
        ot::crypto::asymmetric::Algorithm::ED25519);
    EXPECT_EQ(asymmetricKey.Type(), ot::crypto::asymmetric::Algorithm::ED25519);
    EXPECT_EQ(asymmetricKey.Role(), ot::crypto::asymmetric::Role::Auth);
}

TEST_F(Authority, GetPrivateAuthKey_DefaultSetup_ShouldReturnProperData)
{
    const auto& asymmetricKey = authority_->GetPrivateAuthKey(
        ot::crypto::asymmetric::Algorithm::Secp256k1);
    EXPECT_EQ(
        asymmetricKey.Type(), ot::crypto::asymmetric::Algorithm::Secp256k1);
    EXPECT_EQ(asymmetricKey.Role(), ot::crypto::asymmetric::Role::Auth);
}

TEST_F(Authority, GetVerificationSet_DefaultSetup_ShouldReturnFalse)
{
    ot::proto::VerificationSet verificationSet;
    EXPECT_FALSE(authority_->GetVerificationSet(verificationSet));
    EXPECT_FALSE(verificationSet.has_version());
}

TEST_F(
    Authority,
    GetVerificationSet_AddVerificationCredentialCalledFirst_ShouldReturnProperData)
{
    ot::proto::VerificationSet verificationSet;
    verificationSet.set_version(1);
    EXPECT_TRUE(
        authority_->AddVerificationCredential(verificationSet, reason_));

    ot::proto::VerificationSet verificationSet2;
    EXPECT_TRUE(authority_->GetVerificationSet(verificationSet2));
    EXPECT_EQ(verificationSet.version(), 1);
}

TEST_F(Authority, GetContactData_DefaultSetup_ShouldReturnFalse)
{
    ot::proto::ContactData contactData;
    EXPECT_FALSE(authority_->GetContactData(contactData));
    EXPECT_FALSE(contactData.has_version());
}

TEST_F(
    Authority,
    GetContactData_AddContactCredentialCalledFirst_ShouldReturnCorrectData)
{
    ot::proto::ContactData contactData;
    contactData.set_version(version_);
    EXPECT_TRUE(authority_->AddContactCredential(contactData, reason_));

    ot::proto::ContactData contactData2;
    EXPECT_TRUE(authority_->GetContactData(contactData2));
    EXPECT_EQ(contactData2.version(), version_);
}

TEST_F(
    Authority,
    ContactCredentialVersion_DefaultSetup_ShouldReturnProperVersion)
{
    EXPECT_EQ(authority_->ContactCredentialVersion(), version_);
}

TEST_F(Authority, GetPublicEncrKey_DefaultSetup_ShouldReturnProperData)
{
    const auto& asymmetricKey = authority_->GetPublicEncrKey(
        ot::crypto::asymmetric::Algorithm::Secp256k1);

    EXPECT_EQ(
        asymmetricKey.Type(), ot::crypto::asymmetric::Algorithm::Secp256k1);
    EXPECT_EQ(asymmetricKey.Role(), ot::crypto::asymmetric::Role::Encrypt);
}

TEST_F(
    Authority,
    GetPublicEncrKey_AddedPublicKeyWIthDifferentAlgorith_ShouldReturnProperData)
{
    ot::crypto::Parameters parameters{
        api_.Factory(),
        ot::crypto::ParameterType::ed25519,
        ot::identity::CredentialType::HD,
        ot::identity::SourceType::PubKey,
        2};

    parameters.SetSeed(parameters_.Seed());
    authority_->AddChildKeyCredential(parameters, reason_);

    const auto& asymmetricKey = authority_->GetPublicEncrKey(
        ot::crypto::asymmetric::Algorithm::ED25519);

    EXPECT_EQ(asymmetricKey.Type(), ot::crypto::asymmetric::Algorithm::ED25519);
    EXPECT_EQ(asymmetricKey.Role(), ot::crypto::asymmetric::Role::Encrypt);
}

TEST_F(Authority, GetPrivateEncrKey_DefaultSetup_ShouldReturnProperData)
{
    const auto& asymmetricKey = authority_->GetPrivateEncrKey(
        ot::crypto::asymmetric::Algorithm::Secp256k1);

    EXPECT_EQ(
        asymmetricKey.Type(), ot::crypto::asymmetric::Algorithm::Secp256k1);
    EXPECT_EQ(asymmetricKey.Role(), ot::crypto::asymmetric::Role::Encrypt);
}

TEST_F(Authority, GetPublicSignKey_DefaultSetup_ShouldReturnProperData)
{
    const auto& asymmetricKey = authority_->GetPublicSignKey(
        ot::crypto::asymmetric::Algorithm::Secp256k1);

    EXPECT_EQ(
        asymmetricKey.Type(), ot::crypto::asymmetric::Algorithm::Secp256k1);
    EXPECT_EQ(asymmetricKey.Role(), ot::crypto::asymmetric::Role::Sign);
}

TEST_F(Authority, GetPrivateSignKey_DefaultSetup_ShouldReturnProperData)
{
    const auto& asymmetricKey = authority_->GetPrivateSignKey(
        ot::crypto::asymmetric::Algorithm::Secp256k1);

    EXPECT_EQ(
        asymmetricKey.Type(), ot::crypto::asymmetric::Algorithm::Secp256k1);
    EXPECT_EQ(asymmetricKey.Role(), ot::crypto::asymmetric::Role::Sign);
}

TEST_F(
    Authority,
    RevokeContactCredentials_AddContactCredentials_ShouldReturnProperData)
{
    ot::UnallocatedList<ot::identifier::Generic> list;
    authority_->RevokeContactCredentials(list);
    EXPECT_TRUE(list.empty());

    ot::proto::ContactData contactData;
    contactData.set_version(version_);
    EXPECT_TRUE(authority_->AddContactCredential(contactData, reason_));

    authority_->RevokeContactCredentials(list);
    EXPECT_FALSE(list.empty());
}

TEST_F(
    Authority,
    RevokeVerificationCredentials_AddVerificationCredentialCalledFirst_ShouldReturnProperData)
{
    ot::UnallocatedList<ot::identifier::Generic> list;
    authority_->RevokeVerificationCredentials(list);
    EXPECT_TRUE(list.empty());

    ot::proto::VerificationSet verificationSet;
    verificationSet.set_version(1);
    EXPECT_TRUE(
        authority_->AddVerificationCredential(verificationSet, reason_));

    authority_->RevokeVerificationCredentials(list);
    EXPECT_FALSE(list.empty());
}

TEST_F(Authority, EncryptionTargets_DefaultSetup_ShouldReturnProperData)
{
    const auto& keyCredentials = authority_->EncryptionTargets();
    const auto& masterCredID = authority_->GetMasterCredID();
    const auto& tagCredential = authority_->GetTagCredential(
        ot::crypto::asymmetric::Algorithm::Secp256k1);

    EXPECT_EQ(keyCredentials.first, masterCredID);
    EXPECT_EQ(
        keyCredentials.second.front(),
        tagCredential.Internal()
            .asKey()
            .GetKeypair(ot::crypto::asymmetric::Role::Encrypt)
            .GetPublicKey()
            .Type());
    EXPECT_EQ(keyCredentials.second.size(), 1);
}

TEST_F(
    Authority,
    EncryptionTargets_AddChildKeyCredentialCalledFirst_ShouldReturnProperData)
{
    const auto& masterCredID = authority_->GetMasterCredID();

    ot::crypto::Parameters parameters{
        api_.Factory(),
        ot::crypto::ParameterType::ed25519,
        ot::identity::CredentialType::HD,
        ot::identity::SourceType::PubKey,
        2};

    parameters.SetSeed(parameters_.Seed());
    authority_->AddChildKeyCredential(parameters, reason_);

    const auto& keyCredentials2 = authority_->EncryptionTargets();
    const auto& tagCredential2 = authority_->GetTagCredential(
        ot::crypto::asymmetric::Algorithm::ED25519);

    EXPECT_EQ(keyCredentials2.first, masterCredID);
    EXPECT_EQ(
        keyCredentials2.second.back(),
        tagCredential2.Internal()
            .asKey()
            .GetKeypair(ot::crypto::asymmetric::Role::Encrypt)
            .GetPublicKey()
            .Type());
    EXPECT_EQ(keyCredentials2.second.size(), 2);
}

TEST_F(Authority, GetMasterCredID_DefaultSetup_ShouldReturnProperData)
{
    const auto& masterCredID = authority_->GetMasterCredID();

    EXPECT_EQ(ot::identifier::Type::generic, masterCredID.Type());
    EXPECT_EQ(ot::identifier::Algorithm::blake2b256, masterCredID.Algorithm());
}

TEST_F(Authority, GetPublicKeysBySignature_DefaultSetup_ShouldReturnProperData)
{
    ot::crypto::key::Keypair::Keys keys;

    auto signature = opentxs::Signature::Factory(api_);
    EXPECT_EQ(0, authority_->GetPublicKeysBySignature(keys, signature));
    EXPECT_EQ(0, keys.size());

    EXPECT_EQ(1, authority_->GetPublicKeysBySignature(keys, signature, 'A'));
    EXPECT_EQ(1, keys.size());

    EXPECT_EQ(
        ot::crypto::asymmetric::Algorithm::Secp256k1, keys.front()->Type());
}

TEST_F(
    Authority,
    HasCapability_DefaultSetup_ShouldReturnProperTrueForBothParameters)
{
    EXPECT_TRUE(
        authority_->hasCapability(ot::identity::NymCapability::SIGN_CHILDCRED));
    EXPECT_TRUE(
        authority_->hasCapability(ot::identity::NymCapability::SIGN_MESSAGE));
}

TEST_F(Authority, Params_Secp256k1AsParameter_ShouldReturnProperData)
{
    const auto& params =
        authority_->Params(ot::crypto::asymmetric::Algorithm::Secp256k1);

    EXPECT_EQ(params.size(), 0);
}

TEST_F(
    Authority,
    Params_AddChildKeyCredentialCalledFirstLegacyAsParameter_ShouldReturnProperData)
{
    const auto& params =
        authority_->Params(ot::crypto::asymmetric::Algorithm::Legacy);

    EXPECT_EQ(params.size(), 0);

    ot::crypto::Parameters parameters{
        api_.Factory(),
        ot::crypto::ParameterType::rsa,
        ot::identity::CredentialType::HD,
        ot::identity::SourceType::PubKey,
        2};

    parameters.SetSeed(parameters_.Seed());
    authority_->AddChildKeyCredential(parameters, reason_);

    const auto& params2 =
        authority_->Params(ot::crypto::asymmetric::Algorithm::Legacy);

    EXPECT_NE(params2.size(), 0);
}

TEST_F(Authority, Path_DefaultSetup_ShouldReturnProperData)
{
    ot::proto::HDPath output;
    EXPECT_EQ(output.child_size(), 0);
    EXPECT_EQ(output.has_version(), false);

    EXPECT_TRUE(authority_->Path(output));

    EXPECT_NE(output.child_size(), 0);
    EXPECT_EQ(output.has_version(), true);
}

TEST_F(Authority, Serialize_AddedCredentialsFirst_ShouldReturnProperData)
{
    ot::proto::ContactData contactData;
    contactData.set_version(version_);
    authority_->AddContactCredential(contactData, reason_);

    ot::proto::VerificationSet verificationSet;
    verificationSet.set_version(1);
    authority_->AddVerificationCredential(verificationSet, reason_);

    ot::proto::Authority serialized;
    ot::identity::CredentialIndexModeFlag mode =
        ot::identity::CREDENTIAL_INDEX_MODE_ONLY_IDS;
    EXPECT_TRUE(authority_->Serialize(serialized, mode));

    EXPECT_EQ(serialized.version(), version_);
    EXPECT_EQ(serialized.mode(), ot::proto::AUTHORITYMODE_INDEX);

    ot::UnallocatedList<ot::identifier::Generic> list, list2;
    authority_->RevokeContactCredentials(list);
    authority_->RevokeVerificationCredentials(list2);

    // Under 1th index there is some credential created during Authority
    // creation
    EXPECT_EQ(
        api_.Factory().Internal().Identifier(serialized.activechildids(1)),
        list.front());
    EXPECT_EQ(
        api_.Factory().Internal().Identifier(serialized.activechildids(2)),
        list2.front());
    EXPECT_EQ(
        api_.Factory().Internal().Identifier(serialized.masterid()),
        authority_->GetMasterCredID());
    EXPECT_EQ(
        api_.Factory().Internal().Identifier(serialized.nymid()),
        internal_nym_->ID());
}

auto func() -> ot::UnallocatedCString;

auto func() -> ot::UnallocatedCString { return "Test"; }

TEST_F(Authority, Sign_ShouldReturnProperData)
{
    std::function<ot::UnallocatedCString()> fc = func;

    ot::crypto::SignatureRole role =
        ot::crypto::SignatureRole::PublicCredential;
    ot::proto::Signature signature;
    EXPECT_TRUE(authority_->Sign(fc, role, signature, reason_));

    EXPECT_EQ(signature.version(), 1);
}

TEST_F(Authority, Sign_SignatureRoleIsNymIDSource_ShouldReturnProperData)
{
    std::function<ot::UnallocatedCString()> fc = func;

    ot::crypto::SignatureRole role = ot::crypto::SignatureRole::NymIDSource;
    ot::proto::Signature signature;
    EXPECT_FALSE(authority_->Sign(fc, role, signature, reason_));

    EXPECT_FALSE(signature.has_version());
}

TEST_F(Authority, Sign_SignatureRoleIsPrivateCredential_ShouldReturnProperData)
{
    std::function<ot::UnallocatedCString()> fc = func;

    ot::crypto::SignatureRole role =
        ot::crypto::SignatureRole::PrivateCredential;
    ot::proto::Signature signature;
    EXPECT_FALSE(authority_->Sign(fc, role, signature, reason_));

    EXPECT_FALSE(signature.has_version());
}

TEST_F(Authority, Sign_SignatureRoleIsServerContract_ShouldReturnProperData)
{
    std::function<ot::UnallocatedCString()> fc = func;

    ot::crypto::SignatureRole role = ot::crypto::SignatureRole::ServerContract;
    ot::proto::Signature signature;
    EXPECT_TRUE(authority_->Sign(fc, role, signature, reason_));

    EXPECT_TRUE(signature.has_version());
}

TEST_F(Authority, Source_DefaultSetup_ShouldReturnProperData)
{
    const auto& source = authority_->Source();
    EXPECT_EQ(std::addressof(source), std::addressof(internal_nym_->Source()));
}

TEST_F(Authority, Unlock_DefaultSetup_ShouldReturnProperData)
{
    auto testTag = std::uint32_t{};
    const auto symmetricKey = ot::crypto::symmetric::Key{};
    const auto& str = authority_->GetPublicAuthKey(
        ot::crypto::asymmetric::Algorithm::Secp256k1);
    const auto& tagCredential = authority_->GetTagCredential(
        ot::crypto::asymmetric::Algorithm::Secp256k1);
    const auto& encryptKey =
        tagCredential.Internal()
            .asKey()
            .GetKeypair(ot::crypto::asymmetric::Role::Encrypt)
            .GetPrivateKey();

    EXPECT_TRUE(GetTag(encryptKey, str, testTag));
    EXPECT_FALSE(authority_->Unlock(
        str,
        testTag,
        ot::crypto::asymmetric::Algorithm::Secp256k1,
        symmetricKey,
        non_const_reason_));
}

TEST_F(Authority, Unlock_DefaultSetup_ShouldReturnProperData2)
{
    const auto& tagCredential = authority_->GetTagCredential(
        ot::crypto::asymmetric::Algorithm::Secp256k1);
    const auto& encryptKey =
        tagCredential.Internal()
            .asKey()
            .GetKeypair(ot::crypto::asymmetric::Role::Encrypt)
            .GetPrivateKey();
    auto testTag = std::uint32_t{};
    const auto key = api_.Crypto().Symmetric().Key(
        opentxs::crypto::symmetric::Algorithm::ChaCha20Poly1305, reason_);
    const auto& str = authority_->GetPublicAuthKey(
        ot::crypto::asymmetric::Algorithm::Secp256k1);

    EXPECT_TRUE(GetTag(encryptKey, str, testTag));
    EXPECT_TRUE(authority_->Unlock(
        str,
        testTag,
        ot::crypto::asymmetric::Algorithm::Secp256k1,
        key,
        non_const_reason_));
}

TEST_F(Authority, TransportKey_DefaultSetup_ShouldReturnTrue)
{
    auto publicKey = ot::ByteArray{};
    EXPECT_TRUE(authority_->TransportKey(publicKey, words_, reason_));
}

TEST_F(
    Authority,
    VerificationCredentialVersion_DefaultSetup_ShouldReturnProperData)
{
    EXPECT_EQ(authority_->VerificationCredentialVersion(), 1);
}

TEST_F(Authority, Verify_DefaultSetup_ShouldReturnProperData)
{
    auto publicKey2 = ot::ByteArray{};
    ot::proto::Signature signature2;
    EXPECT_FALSE(authority_->Verify(
        publicKey2, signature2, opentxs::crypto::asymmetric::Role::Auth));
}

TEST_F(Authority, Verify_WithCredentialsEqualToMasterCredID_ShouldReturnFalse)
{
    auto publicKey = ot::ByteArray{};
    ot::proto::Signature signature;
    serialize_identifier_to_pb(
        authority_->GetMasterCredID(), *signature.mutable_credentialid());

    EXPECT_FALSE(authority_->Verify(
        publicKey, signature, opentxs::crypto::asymmetric::Role::Auth));
}
TEST_F(Authority, Verify_WithChildKeyCredential_ShouldReturnFalse)
{
    auto publicKey = ot::ByteArray{};

    ot::crypto::Parameters parameters{
        api_.Factory(),
        ot::crypto::ParameterType::ed25519,
        ot::identity::CredentialType::HD,
        ot::identity::SourceType::PubKey,
        2};

    parameters.SetSeed(parameters_.Seed());
    auto credential = authority_->AddChildKeyCredential(parameters, reason_);

    ot::proto::Signature signature;
    serialize_identifier_to_pb(credential, *signature.mutable_credentialid());

    EXPECT_FALSE(authority_->Verify(
        publicKey, signature, opentxs::crypto::asymmetric::Role::Auth));
}

TEST_F(Authority, Verify_DefaultSetup_ShouldReturnFalse)
{
    ot::proto::VerificationItem verification;
    EXPECT_FALSE(authority_->Verify(verification));
}
TEST_F(Authority, VerifyInternally_DefaultSetup_ShouldReturnTrue)
{
    EXPECT_TRUE(authority_->VerifyInternally());
}

TEST_F(Authority, WriteCredentials_DefaultSetup_ShouldReturnProperData)
{
    EXPECT_TRUE(authority_->WriteCredentials());

    ot::proto::VerificationSet verificationSet;
    verificationSet.set_version(1);
    authority_->AddVerificationCredential(verificationSet, reason_);

    EXPECT_TRUE(authority_->WriteCredentials());

    ot::proto::ContactData contactData;
    contactData.set_version(version_);
    authority_->AddContactCredential(contactData, reason_);

    EXPECT_TRUE(authority_->WriteCredentials());

    ot::crypto::Parameters parameters{
        api_.Factory(),
        ot::crypto::ParameterType::ed25519,
        ot::identity::CredentialType::HD,
        ot::identity::SourceType::PubKey,
        2};

    parameters.SetSeed(parameters_.Seed());
    authority_->AddChildKeyCredential(parameters, reason_);

    EXPECT_TRUE(authority_->WriteCredentials());
}
//???????
TEST_F(Authority, GetPairs_DefaultSetup_ShouldReturnProperData)
{
    const auto& authKeyPair = authority_->GetAuthKeypair(
        ot::crypto::asymmetric::Algorithm::Secp256k1);

    EXPECT_EQ(
        authKeyPair.GetPrivateKey().Type(),
        ot::crypto::asymmetric::Algorithm::Secp256k1);
    EXPECT_EQ(
        authKeyPair.GetPrivateKey().Role(), ot::crypto::asymmetric::Role::Auth);

    const auto& encrKeyPair = authority_->GetEncrKeypair(
        ot::crypto::asymmetric::Algorithm::Secp256k1);

    EXPECT_EQ(
        encrKeyPair.GetPrivateKey().Type(),
        ot::crypto::asymmetric::Algorithm::Secp256k1);
    EXPECT_EQ(
        encrKeyPair.GetPrivateKey().Role(),
        ot::crypto::asymmetric::Role::Encrypt);

    const auto& signKeyPair = authority_->GetSignKeypair(
        ot::crypto::asymmetric::Algorithm::Secp256k1);

    EXPECT_EQ(
        signKeyPair.GetPrivateKey().Type(),
        ot::crypto::asymmetric::Algorithm::Secp256k1);
    EXPECT_EQ(
        signKeyPair.GetPrivateKey().Role(), ot::crypto::asymmetric::Role::Sign);
}

TEST_F(Authority, GetTagCredential_DefaultSetup_ShouldReturnProperData)
{
    const auto& tagCredential = authority_->GetTagCredential(
        ot::crypto::asymmetric::Algorithm::Secp256k1);
    const auto& keyPair = tagCredential.Internal().asKey().GetKeypair(
        ot::crypto::asymmetric::Role::Sign);

    EXPECT_EQ(
        keyPair.GetPrivateKey().Type(),
        ot::crypto::asymmetric::Algorithm::Secp256k1);
    EXPECT_EQ(
        keyPair.GetPrivateKey().Role(), ot::crypto::asymmetric::Role::Sign);
}
}  // namespace ottest
