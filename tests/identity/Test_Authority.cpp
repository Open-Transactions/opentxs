// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <iostream>
#include <memory>

#include "../tests/mocks/crypto/key/Asymmetric.hpp"
#include "../tests/mocks/crypto/key/KeypairMock.hpp"
#include "../tests/mocks/identity/NymMock.hpp"
#include "../tests/mocks/identity/SourceMock.hpp"
#include "../tests/mocks/identity/credential/PrimaryMock.hpp"
#include "2_Factory.hpp"
#include "identity/credential/Primary.hpp"
#include "internal/api/crypto/Seed.hpp"
#include "internal/identity/Identity.hpp"
#include "internal/otx/common/crypto/Signature.hpp"
#include "internal/otx/common/util/Tag.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/Contact.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Language.hpp"
#include "opentxs/crypto/ParameterType.hpp"
#include "opentxs/crypto/Parameters.hpp"
#include "opentxs/crypto/SeedStyle.hpp"
#include "opentxs/crypto/key/asymmetric/Algorithm.hpp"
#include "opentxs/identity/IdentityType.hpp"
#include "opentxs/identity/SourceType.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/identity/wot/claim/Data.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "paymentcode/VectorsV3.hpp"
#include "serialization/protobuf/Credential.pb.h"
#include "serialization/protobuf/Nym.pb.h"
#include "serialization/protobuf/Signature.pb.h"
#include "opentxs/core/identifier/Algorithm.hpp"
#include "opentxs/core/identifier/Type.hpp"

using namespace testing;

namespace opentxs
{

class Test_Authority : public ::testing::Test
{
public:
    const api::session::Client& client_;
    const OTPasswordPrompt reason_;
    OTPasswordPrompt nonConstReason_;

    OTSecret words_;
    const std::uint8_t version_ = 6;
    const int nym_ = 1;
    const identity::SourceMock sourceMock_;
    const identity::NymMock nymMock_;
    crypto::Parameters parameters_;
    std::unique_ptr<identity::Source> source_;
    ot::OTNymID* nymID;
    std::unique_ptr<ot::identity::internal::Nym> internalNym_;
    const ot::UnallocatedCString alias_ = ot::UnallocatedCString{"alias"};

    std::unique_ptr<identity::internal::Authority> authority_;

    Test_Authority()
        : client_(Context().StartClientSession(0))
        , reason_(client_.Factory().PasswordPrompt(__func__))
        , nonConstReason_(client_.Factory().PasswordPrompt(__func__))
        , words_(client_.Factory().SecretFromText(
              ottest::GetVectors3().alice_.words_))
    {
    }

    void SetUp() override
    {
        const auto& seeds = client_.Crypto().Seed().Internal();
        parameters_.SetCredset(0);
        auto nymIndex = Bip32Index{0};
        auto fingerprint = parameters_.Seed();
        auto style = parameters_.SeedStyle();
        auto lang = parameters_.SeedLanguage();

        seeds.GetOrCreateDefaultSeed(
            fingerprint,
            style,
            lang,
            nymIndex,
            parameters_.SeedStrength(),
            reason_);

        auto seed = seeds.GetSeed(fingerprint, nymIndex, reason_);
        const auto defaultIndex = parameters_.UseAutoIndex();

        if (false == defaultIndex) { nymIndex = parameters_.Nym(); }

        const auto newIndex = static_cast<std::int32_t>(nymIndex) + 1;
        seeds.UpdateIndex(fingerprint, newIndex, reason_);
        parameters_.SetEntropy(seed);
        parameters_.SetSeed(fingerprint);
        parameters_.SetNym(nymIndex);

        source_.reset(Factory::NymIDSource(client_, parameters_, reason_));

        internalNym_.reset(ot::Factory::Nym(
            client_,
            parameters_,
            ot::identity::Type::individual,
            alias_,
            reason_));

        authority_.reset(Factory().Authority(
            client_, *internalNym_, *source_, parameters_, version_, reason_));
    }
};

/////////////// Constructors ////////////////
UnallocatedCString func();

UnallocatedCString func() { return "Test"; }

TEST_F(Test_Authority, GetPublicAuthKey_DefaultSetup_ShouldReturnDefaultKey)
{
    const auto& asymmetricKey = authority_->GetPublicAuthKey(
        crypto::key::asymmetric::Algorithm::Secp256k1);
    EXPECT_EQ(
        asymmetricKey.keyType(), crypto::key::asymmetric::Algorithm::Secp256k1);
    EXPECT_EQ(asymmetricKey.Role(), crypto::key::asymmetric::Role::Auth);
}

TEST_F(
    Test_Authority,
    GetPublicAuthKey_AddedPublicKeyWIthDifferentAlgorith_ShouldReturnProperData)
{
    crypto::Parameters parameters{
        crypto::ParameterType::ed25519,
        identity::CredentialType::HD,
        identity::SourceType::PubKey,
        2};

    parameters.SetSeed(parameters_.Seed());
    authority_->AddChildKeyCredential(parameters, reason_);

    const auto& asymmetricKey = authority_->GetPublicAuthKey(
        crypto::key::asymmetric::Algorithm::ED25519);
    EXPECT_EQ(
        asymmetricKey.keyType(), crypto::key::asymmetric::Algorithm::ED25519);
    EXPECT_EQ(asymmetricKey.Role(), crypto::key::asymmetric::Role::Auth);
}

TEST_F(Test_Authority, GetPrivateAuthKey_DefaultSetup_ShouldReturnProperData)
{
    const auto& asymmetricKey = authority_->GetPrivateAuthKey(
        crypto::key::asymmetric::Algorithm::Secp256k1);
    EXPECT_EQ(
        asymmetricKey.keyType(), crypto::key::asymmetric::Algorithm::Secp256k1);
    EXPECT_EQ(asymmetricKey.Role(), crypto::key::asymmetric::Role::Auth);
}

TEST_F(Test_Authority, GetVerificationSet_DefaultSetup_ShouldReturnFalse)
{
    proto::VerificationSet verificationSet;
    EXPECT_FALSE(authority_->GetVerificationSet(verificationSet));
    EXPECT_FALSE(verificationSet.has_version());
}

TEST_F(
    Test_Authority,
    GetVerificationSet_AddVerificationCredentialCalledFirst_ShouldReturnProperData)
{
    proto::VerificationSet verificationSet;
    verificationSet.set_version(1);
    EXPECT_TRUE(authority_->AddVerificationCredential(verificationSet, reason_));

    proto::VerificationSet verificationSet2;
    EXPECT_TRUE(authority_->GetVerificationSet(verificationSet2));
    EXPECT_EQ(verificationSet.version(), 1);
}

TEST_F(Test_Authority, GetContactData_DefaultSetup_ShouldReturnFalse)
{
    proto::ContactData contactData;
    EXPECT_FALSE(authority_->GetContactData(contactData));
    EXPECT_FALSE(contactData.has_version());
}

TEST_F(
    Test_Authority,
    GetContactData_AddContactCredentialCalledFirst_ShouldReturnCorrectData)
{
    proto::ContactData contactData;
    contactData.set_version(version_);
    EXPECT_TRUE(authority_->AddContactCredential(contactData, reason_));

    proto::ContactData contactData2;
    EXPECT_TRUE(authority_->GetContactData(contactData2));
    EXPECT_EQ(contactData2.version(), version_);
}

TEST_F(
    Test_Authority,
    ContactCredentialVersion_DefaultSetup_ShouldReturnProperVersion)
{
    EXPECT_EQ(authority_->ContactCredentialVersion(), version_);
}

TEST_F(Test_Authority, GetPublicEncrKey_DefaultSetup_ShouldReturnProperData)
{
    const auto& asymmetricKey = authority_->GetPublicEncrKey(
        crypto::key::asymmetric::Algorithm::Secp256k1);

    EXPECT_EQ(
        asymmetricKey.keyType(), crypto::key::asymmetric::Algorithm::Secp256k1);
    EXPECT_EQ(asymmetricKey.Role(), crypto::key::asymmetric::Role::Encrypt);
}

TEST_F(
    Test_Authority,
    GetPublicEncrKey_AddedPublicKeyWIthDifferentAlgorith_ShouldReturnProperData)
{
    crypto::Parameters parameters{
        crypto::ParameterType::ed25519,
        identity::CredentialType::HD,
        identity::SourceType::PubKey,
        2};

    parameters.SetSeed(parameters_.Seed());
    authority_->AddChildKeyCredential(parameters, reason_);

    const auto& asymmetricKey = authority_->GetPublicEncrKey(
        crypto::key::asymmetric::Algorithm::ED25519);

    EXPECT_EQ(
        asymmetricKey.keyType(), crypto::key::asymmetric::Algorithm::ED25519);
    EXPECT_EQ(asymmetricKey.Role(), crypto::key::asymmetric::Role::Encrypt);
}

TEST_F(Test_Authority, GetPrivateEncrKey_DefaultSetup_ShouldReturnProperData)
{
    const auto& asymmetricKey = authority_->GetPrivateEncrKey(
        crypto::key::asymmetric::Algorithm::Secp256k1);

    EXPECT_EQ(
        asymmetricKey.keyType(), crypto::key::asymmetric::Algorithm::Secp256k1);
    EXPECT_EQ(asymmetricKey.Role(), crypto::key::asymmetric::Role::Encrypt);
}

TEST_F(Test_Authority, GetPublicSignKey_DefaultSetup_ShouldReturnProperData)
{
    const auto& asymmetricKey = authority_->GetPublicSignKey(
        crypto::key::asymmetric::Algorithm::Secp256k1);

    EXPECT_EQ(
        asymmetricKey.keyType(), crypto::key::asymmetric::Algorithm::Secp256k1);
    EXPECT_EQ(asymmetricKey.Role(), crypto::key::asymmetric::Role::Sign);
}

TEST_F(Test_Authority, GetPrivateSignKey_DefaultSetup_ShouldReturnProperData)
{
    const auto& asymmetricKey = authority_->GetPrivateSignKey(
        crypto::key::asymmetric::Algorithm::Secp256k1);

    EXPECT_EQ(
        asymmetricKey.keyType(), crypto::key::asymmetric::Algorithm::Secp256k1);
    EXPECT_EQ(asymmetricKey.Role(), crypto::key::asymmetric::Role::Sign);
}

TEST_F(
    Test_Authority,
    RevokeContactCredentials_AddContactCredentials_ShouldReturnProperData)
{
    UnallocatedList<UnallocatedCString> list;
    authority_->RevokeContactCredentials(list);
    EXPECT_TRUE(list.empty());

    proto::ContactData contactData;
    contactData.set_version(version_);
    EXPECT_TRUE(authority_->AddContactCredential(contactData, reason_));

    authority_->RevokeContactCredentials(list);
    EXPECT_FALSE(list.empty());
}

TEST_F(
    Test_Authority,
    RevokeVerificationCredentials_AddVerificationCredential_ShouldReturnProperData)
{
    UnallocatedList<UnallocatedCString> list;
    authority_->RevokeVerificationCredentials(list);
    EXPECT_TRUE(list.empty());

    proto::VerificationSet verificationSet;
    verificationSet.set_version(1);
    EXPECT_TRUE(authority_->AddVerificationCredential(verificationSet, reason_));

    authority_->RevokeVerificationCredentials(list);
    EXPECT_FALSE(list.empty());
}

TEST_F(
    Test_Authority,
    EncryptionTargets_DefaultSetup_ShouldReturnProperData)
{
    const auto& keyCredentials = authority_->EncryptionTargets();
    const auto& masterCredID = authority_->GetMasterCredID();
    const auto& tagCredential = authority_->GetTagCredential(
        crypto::key::asymmetric::Algorithm::Secp256k1);

    EXPECT_EQ(keyCredentials.first, masterCredID);
    EXPECT_EQ(keyCredentials.second.front(), tagCredential.GetKeypair(crypto::key::asymmetric::Role::Encrypt).GetPublicKey().keyType());
    EXPECT_EQ(keyCredentials.second.size(), 1);
}

TEST_F(
    Test_Authority,
    EncryptionTargets_AddChildKeyCredentialCalledFirst_ShouldReturnProperData)
{
    const auto& masterCredID = authority_->GetMasterCredID();

    crypto::Parameters parameters{
        crypto::ParameterType::ed25519,
        identity::CredentialType::HD,
        identity::SourceType::PubKey,
        2};

    parameters.SetSeed(parameters_.Seed());
    authority_->AddChildKeyCredential(parameters, reason_);

    const auto& keyCredentials2 = authority_->EncryptionTargets();
    const auto& tagCredential2 = authority_->GetTagCredential(
        crypto::key::asymmetric::Algorithm::ED25519);

    EXPECT_EQ(keyCredentials2.first, masterCredID);
    EXPECT_EQ(keyCredentials2.second.back(), tagCredential2.GetKeypair(crypto::key::asymmetric::Role::Encrypt).GetPublicKey().keyType());
    EXPECT_EQ(keyCredentials2.second.size(), 2);
}

TEST_F(
    Test_Authority,
    GetMasterCredID_DefaultSetup_ShouldReturnProperData)
{
    const auto& masterCredID = authority_->GetMasterCredID();

    EXPECT_EQ(identifier::Type::generic , masterCredID.get().Type());
    EXPECT_EQ(identifier::Algorithm::blake2b256 , masterCredID.get().Algorithm());
}

TEST_F(
    Test_Authority,
    GetPublicKeysBySignature_DefaultSetup_ShouldReturnProperData)
{
    crypto::key::Keypair::Keys keys;

    auto signature = opentxs::Signature::Factory(client_);
    EXPECT_EQ(0, authority_->GetPublicKeysBySignature(keys, signature));
    EXPECT_EQ(0, keys.size());

    EXPECT_EQ(1, authority_->GetPublicKeysBySignature(keys, signature, 'A'));
    EXPECT_EQ(1, keys.size());

    EXPECT_EQ(crypto::key::asymmetric::Algorithm::Secp256k1, keys.front()->keyType());
}

TEST_F(
    Test_Authority,
    hasCapability_DefaultSetup_ShouldReturnProperData)
{
       EXPECT_TRUE(authority_->hasCapability(NymCapability::SIGN_CHILDCRED));
       EXPECT_TRUE(authority_->hasCapability(NymCapability::SIGN_MESSAGE));
}

TEST_F(
    Test_Authority,
    Params_DefaultSetup_ShouldReturnProperData)
{
    const auto& params = authority_->Params(
        crypto::key::asymmetric::Algorithm::Secp256k1);

    EXPECT_EQ(params.size(), 0);
}

TEST_F(
    Test_Authority,
    Params_AddChildKeyCredentialCalledFirst_ShouldReturnProperData)
{
    const auto& params = authority_->Params(
        crypto::key::asymmetric::Algorithm::Legacy);

    EXPECT_EQ(params.size(), 0);

    crypto::Parameters parameters{
        crypto::ParameterType::rsa,
        identity::CredentialType::HD,
        identity::SourceType::PubKey,
        2};

    parameters.SetSeed(parameters_.Seed());
    authority_->AddChildKeyCredential(parameters, reason_);

    const auto& params2 = authority_->Params(
                     crypto::key::asymmetric::Algorithm::Legacy);

    EXPECT_NE(params2.size(), 0);
}

TEST_F(
    Test_Authority,
    Path_DefaultSetup_ShouldReturnProperData)
{
    proto::HDPath output;
    EXPECT_EQ(output.child_size(), 0);
    EXPECT_EQ(output.has_version(), false);

    EXPECT_TRUE(authority_->Path(output));

    EXPECT_NE(output.child_size(), 0);
    EXPECT_EQ(output.has_version(), true);
}

TEST_F(
    Test_Authority,
    SerializeDefaultSetup_ShouldReturnProperData)
{
    proto::ContactData contactData;
    contactData.set_version(version_);
    authority_->AddContactCredential(contactData, reason_);

    proto::VerificationSet verificationSet;
    verificationSet.set_version(1);
    authority_->AddVerificationCredential(verificationSet, reason_);

    proto::Authority serialized;
    CredentialIndexModeFlag mode = CREDENTIAL_INDEX_MODE_ONLY_IDS;
    EXPECT_TRUE(authority_->Serialize(serialized, mode));

    EXPECT_EQ(serialized.version(), version_);
    EXPECT_EQ(serialized.mode(), proto::AUTHORITYMODE_INDEX);

    UnallocatedList<UnallocatedCString> list, list2;
    authority_->RevokeContactCredentials(list);
    authority_->RevokeVerificationCredentials(list2);

    //Under 1th index there is some credential created during Authority creation
    EXPECT_EQ(serialized.activechildids(1), list.front());
    EXPECT_EQ(serialized.activechildids(2), list2.front());

    EXPECT_EQ(serialized.masterid(), authority_->GetMasterCredID()->str());
    EXPECT_EQ(serialized.nymid(), internalNym_->ID().str());
}

TEST_F(
    Test_Authority,
    Sign_ShouldReturnProperData)
{

        std::function<UnallocatedCString()> fc = func;

        crypto::SignatureRole role = crypto::SignatureRole::PublicCredential;
        proto::Signature signature1;
        std::cerr << "1: " << authority_->Sign(fc, role, signature1, reason_);
}

//Not sure how to trigger fail here. It looks like you cannot add invalid credentials
//here so WriteCredentials be always successful
TEST_F(
    Test_Authority,
    WriteCredentials_DefaultSetup_ShouldReturnProperData)
{
    EXPECT_TRUE(authority_->WriteCredentials());

    proto::VerificationSet verificationSet;
    verificationSet.set_version(1);
    authority_->AddVerificationCredential(verificationSet, reason_);

    EXPECT_TRUE(authority_->WriteCredentials());

    proto::ContactData contactData;
    contactData.set_version(version_);
    authority_->AddContactCredential(contactData, reason_);

    EXPECT_TRUE(authority_->WriteCredentials());

    crypto::Parameters parameters{
        crypto::ParameterType::ed25519,
        identity::CredentialType::HD,
        identity::SourceType::PubKey,
        2};

    parameters.SetSeed(parameters_.Seed());
    authority_->AddChildKeyCredential(parameters, reason_);

    EXPECT_TRUE(authority_->WriteCredentials());

}
//???????
TEST_F(Test_Authority, Get___KeyPairs_DefaultSetup_ShouldReturnProperData)
{
    const auto& authKeyPair = authority_->GetAuthKeypair(
        crypto::key::asymmetric::Algorithm::Secp256k1);

    EXPECT_EQ(
        authKeyPair.GetPrivateKey().keyType(),
        crypto::key::asymmetric::Algorithm::Secp256k1);
    EXPECT_EQ(
        authKeyPair.GetPrivateKey().Role(),
        crypto::key::asymmetric::Role::Auth);

    const auto& encrKeyPair = authority_->GetEncrKeypair(
        crypto::key::asymmetric::Algorithm::Secp256k1);

    EXPECT_EQ(
        encrKeyPair.GetPrivateKey().keyType(),
        crypto::key::asymmetric::Algorithm::Secp256k1);
    EXPECT_EQ(
        encrKeyPair.GetPrivateKey().Role(),
        crypto::key::asymmetric::Role::Encrypt);

    const auto& signKeyPair = authority_->GetSignKeypair(
        crypto::key::asymmetric::Algorithm::Secp256k1);

    EXPECT_EQ(
        signKeyPair.GetPrivateKey().keyType(),
        crypto::key::asymmetric::Algorithm::Secp256k1);
    EXPECT_EQ(
        signKeyPair.GetPrivateKey().Role(),
        crypto::key::asymmetric::Role::Sign);
}

//???????
TEST_F(Test_Authority, GetTagCredential_DefaultSetup_ShouldReturnProperData)
{
    const auto& tagCredential = authority_->GetTagCredential(
        crypto::key::asymmetric::Algorithm::Secp256k1);
    const auto& keyPair =
        tagCredential.GetKeypair(crypto::key::asymmetric::Role::Sign);

    EXPECT_EQ(
        keyPair.GetPrivateKey().keyType(),
        crypto::key::asymmetric::Algorithm::Secp256k1);
    EXPECT_EQ(
        keyPair.GetPrivateKey().Role(), crypto::key::asymmetric::Role::Sign);
}

TEST_F(Test_Authority, Constructor_WithReason_ShouldNotThrow4221)
{


    // EXPECT_CALL(nymMock_, ID()).WillRepeatedly(ReturnRef(nym.ID()));


    //const auto& source = authority_->Source();

    // EXPECT_EQ(source.Type(), version_);
    //EXPECT_EQ(source.NymID(), id);
   // EXPECT_EQ(std::addressof(source), std::addressof(internalNym_->Source()));

    auto publicKey = Data::Factory();
    std::cerr << "1: " << authority_->TransportKey(publicKey, words_, reason_);

    const auto asymmetricKeyMock = crypto::key::AsymmetricMock();
    const auto symmetricKey = crypto::key::Symmetric::Factory();
    const std::uint32_t tag = 1;
    const auto& str = authority_->GetPublicAuthKey(
        crypto::key::asymmetric::Algorithm::Secp256k1);
    // EXPECT_CALL(asymmetricKeyMock,
    // PublicKey()).WillRepeatedly(Return(str.PublicKey()));

    std::cerr << "1: "
              << authority_->Unlock(
                     str,
                     tag,
                     crypto::key::asymmetric::Algorithm::Secp256k1,
                     symmetricKey,
                     nonConstReason_);

    std::cerr << "1: " << authority_->VerificationCredentialVersion();

    auto publicKey2 = Data::Factory();
    proto::Signature signature2;
    std::cerr << "1: "
              << authority_->Verify(
                     publicKey2,
                     signature2,
                     opentxs::crypto::key::asymmetric::Role::Auth);

    proto::Verification verification;
    std::cerr << "1: " << authority_->Verify(verification);

    std::cerr << "1: " << authority_->VerifyInternally();
}

}