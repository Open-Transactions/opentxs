// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <Authority.pb.h>
#include <ContactData.pb.h>
#include <Enums.pb.h>
#include <HDPath.pb.h>
#include <Signature.pb.h>
#include <Verification.pb.h>
#include <VerificationSet.pb.h>
#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstdint>
#include <functional>
#include <memory>
#include <string_view>
#include <utility>

#include "2_Factory.hpp"
#include "internal/api/crypto/Seed.hpp"
#include "internal/identity/Authority.hpp"
#include "internal/identity/Nym.hpp"
#include "internal/identity/Types.hpp"
#include "internal/otx/common/crypto/Signature.hpp"
#include "ottest/data/crypto/PaymentCodeV3.hpp"

namespace ottest
{
using namespace testing;

class Test_Authority : public ::testing::Test
{
public:
    const ot::api::session::Client& client_;
    const ot::PasswordPrompt reason_;
    ot::PasswordPrompt non_const_reason_;

    ot::Secret words_;
    const std::uint8_t version_ = 6;
    const int nym_ = 1;
    const ot::UnallocatedCString alias_ = ot::UnallocatedCString{"alias"};

    ot::crypto::Parameters parameters_;
    std::unique_ptr<ot::identity::Source> source_;
    std::unique_ptr<ot::identity::internal::Nym> internal_nym_;
    std::unique_ptr<ot::identity::internal::Authority> authority_;

    Test_Authority()
        : client_(ot::Context().StartClientSession(0))
        , reason_(client_.Factory().PasswordPrompt(__func__))
        , non_const_reason_(client_.Factory().PasswordPrompt(__func__))
        , words_(client_.Factory().SecretFromText(
              ottest::GetPaymentCodeVector3().alice_.words_))
        , parameters_()
        , source_{nullptr}
        , internal_nym_{nullptr}
        , authority_{nullptr}
    {
    }

    void SetUp() override
    {
        const auto& seeds = client_.Crypto().Seed().Internal();
        parameters_.SetCredset(0);
        auto nymIndex = ot::Bip32Index{0};
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

        source_.reset(ot::Factory::NymIDSource(client_, parameters_, reason_));

        internal_nym_.reset(ot::Factory::Nym(
            client_,
            parameters_,
            ot::identity::Type::individual,
            alias_,
            reason_));

        const auto& nn =
            dynamic_cast<const opentxs::identity::Nym&>(*internal_nym_);

        authority_.reset(ot::Factory().Authority(
            client_, nn, *source_, parameters_, version_, reason_));
    }
};

TEST_F(Test_Authority, GetPublicAuthKey_DefaultSetup_ShouldReturnDefaultKey)
{
    const auto& asymmetricKey = authority_->GetPublicAuthKey(
        ot::crypto::key::asymmetric::Algorithm::Secp256k1);
    EXPECT_EQ(
        asymmetricKey.keyType(),
        ot::crypto::key::asymmetric::Algorithm::Secp256k1);
    EXPECT_EQ(asymmetricKey.Role(), ot::crypto::key::asymmetric::Role::Auth);
}

TEST_F(
    Test_Authority,
    GetPublicAuthKey_AddedPublicKeyWIthDifferentAlgorith_ShouldReturnProperData)
{
    ot::crypto::Parameters parameters{
        ot::crypto::ParameterType::ed25519,
        ot::identity::CredentialType::HD,
        ot::identity::SourceType::PubKey,
        2};

    parameters.SetSeed(parameters_.Seed());
    authority_->AddChildKeyCredential(parameters, reason_);

    const auto& asymmetricKey = authority_->GetPublicAuthKey(
        ot::crypto::key::asymmetric::Algorithm::ED25519);
    EXPECT_EQ(
        asymmetricKey.keyType(),
        ot::crypto::key::asymmetric::Algorithm::ED25519);
    EXPECT_EQ(asymmetricKey.Role(), ot::crypto::key::asymmetric::Role::Auth);
}

TEST_F(Test_Authority, GetPrivateAuthKey_DefaultSetup_ShouldReturnProperData)
{
    const auto& asymmetricKey = authority_->GetPrivateAuthKey(
        ot::crypto::key::asymmetric::Algorithm::Secp256k1);
    EXPECT_EQ(
        asymmetricKey.keyType(),
        ot::crypto::key::asymmetric::Algorithm::Secp256k1);
    EXPECT_EQ(asymmetricKey.Role(), ot::crypto::key::asymmetric::Role::Auth);
}

TEST_F(Test_Authority, GetVerificationSet_DefaultSetup_ShouldReturnFalse)
{
    ot::proto::VerificationSet verificationSet;
    EXPECT_FALSE(authority_->GetVerificationSet(verificationSet));
    EXPECT_FALSE(verificationSet.has_version());
}

TEST_F(
    Test_Authority,
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

TEST_F(Test_Authority, GetContactData_DefaultSetup_ShouldReturnFalse)
{
    ot::proto::ContactData contactData;
    EXPECT_FALSE(authority_->GetContactData(contactData));
    EXPECT_FALSE(contactData.has_version());
}

TEST_F(
    Test_Authority,
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
    Test_Authority,
    ContactCredentialVersion_DefaultSetup_ShouldReturnProperVersion)
{
    EXPECT_EQ(authority_->ContactCredentialVersion(), version_);
}

TEST_F(Test_Authority, GetPublicEncrKey_DefaultSetup_ShouldReturnProperData)
{
    const auto& asymmetricKey = authority_->GetPublicEncrKey(
        ot::crypto::key::asymmetric::Algorithm::Secp256k1);

    EXPECT_EQ(
        asymmetricKey.keyType(),
        ot::crypto::key::asymmetric::Algorithm::Secp256k1);
    EXPECT_EQ(asymmetricKey.Role(), ot::crypto::key::asymmetric::Role::Encrypt);
}

TEST_F(
    Test_Authority,
    GetPublicEncrKey_AddedPublicKeyWIthDifferentAlgorith_ShouldReturnProperData)
{
    ot::crypto::Parameters parameters{
        ot::crypto::ParameterType::ed25519,
        ot::identity::CredentialType::HD,
        ot::identity::SourceType::PubKey,
        2};

    parameters.SetSeed(parameters_.Seed());
    authority_->AddChildKeyCredential(parameters, reason_);

    const auto& asymmetricKey = authority_->GetPublicEncrKey(
        ot::crypto::key::asymmetric::Algorithm::ED25519);

    EXPECT_EQ(
        asymmetricKey.keyType(),
        ot::crypto::key::asymmetric::Algorithm::ED25519);
    EXPECT_EQ(asymmetricKey.Role(), ot::crypto::key::asymmetric::Role::Encrypt);
}

TEST_F(Test_Authority, GetPrivateEncrKey_DefaultSetup_ShouldReturnProperData)
{
    const auto& asymmetricKey = authority_->GetPrivateEncrKey(
        ot::crypto::key::asymmetric::Algorithm::Secp256k1);

    EXPECT_EQ(
        asymmetricKey.keyType(),
        ot::crypto::key::asymmetric::Algorithm::Secp256k1);
    EXPECT_EQ(asymmetricKey.Role(), ot::crypto::key::asymmetric::Role::Encrypt);
}

TEST_F(Test_Authority, GetPublicSignKey_DefaultSetup_ShouldReturnProperData)
{
    const auto& asymmetricKey = authority_->GetPublicSignKey(
        ot::crypto::key::asymmetric::Algorithm::Secp256k1);

    EXPECT_EQ(
        asymmetricKey.keyType(),
        ot::crypto::key::asymmetric::Algorithm::Secp256k1);
    EXPECT_EQ(asymmetricKey.Role(), ot::crypto::key::asymmetric::Role::Sign);
}

TEST_F(Test_Authority, GetPrivateSignKey_DefaultSetup_ShouldReturnProperData)
{
    const auto& asymmetricKey = authority_->GetPrivateSignKey(
        ot::crypto::key::asymmetric::Algorithm::Secp256k1);

    EXPECT_EQ(
        asymmetricKey.keyType(),
        ot::crypto::key::asymmetric::Algorithm::Secp256k1);
    EXPECT_EQ(asymmetricKey.Role(), ot::crypto::key::asymmetric::Role::Sign);
}

TEST_F(
    Test_Authority,
    RevokeContactCredentials_AddContactCredentials_ShouldReturnProperData)
{
    ot::UnallocatedList<ot::UnallocatedCString> list;
    authority_->RevokeContactCredentials(list);
    EXPECT_TRUE(list.empty());

    ot::proto::ContactData contactData;
    contactData.set_version(version_);
    EXPECT_TRUE(authority_->AddContactCredential(contactData, reason_));

    authority_->RevokeContactCredentials(list);
    EXPECT_FALSE(list.empty());
}

TEST_F(
    Test_Authority,
    RevokeVerificationCredentials_AddVerificationCredentialCalledFirst_ShouldReturnProperData)
{
    ot::UnallocatedList<ot::UnallocatedCString> list;
    authority_->RevokeVerificationCredentials(list);
    EXPECT_TRUE(list.empty());

    ot::proto::VerificationSet verificationSet;
    verificationSet.set_version(1);
    EXPECT_TRUE(
        authority_->AddVerificationCredential(verificationSet, reason_));

    authority_->RevokeVerificationCredentials(list);
    EXPECT_FALSE(list.empty());
}

TEST_F(Test_Authority, EncryptionTargets_DefaultSetup_ShouldReturnProperData)
{
    const auto& keyCredentials = authority_->EncryptionTargets();
    const auto& masterCredID = authority_->GetMasterCredID();
    const auto& tagCredential = authority_->GetTagCredential(
        ot::crypto::key::asymmetric::Algorithm::Secp256k1);

    EXPECT_EQ(keyCredentials.first, masterCredID);
    EXPECT_EQ(
        keyCredentials.second.front(),
        tagCredential.GetKeypair(ot::crypto::key::asymmetric::Role::Encrypt)
            .GetPublicKey()
            .keyType());
    EXPECT_EQ(keyCredentials.second.size(), 1);
}

TEST_F(
    Test_Authority,
    EncryptionTargets_AddChildKeyCredentialCalledFirst_ShouldReturnProperData)
{
    const auto& masterCredID = authority_->GetMasterCredID();

    ot::crypto::Parameters parameters{
        ot::crypto::ParameterType::ed25519,
        ot::identity::CredentialType::HD,
        ot::identity::SourceType::PubKey,
        2};

    parameters.SetSeed(parameters_.Seed());
    authority_->AddChildKeyCredential(parameters, reason_);

    const auto& keyCredentials2 = authority_->EncryptionTargets();
    const auto& tagCredential2 = authority_->GetTagCredential(
        ot::crypto::key::asymmetric::Algorithm::ED25519);

    EXPECT_EQ(keyCredentials2.first, masterCredID);
    EXPECT_EQ(
        keyCredentials2.second.back(),
        tagCredential2.GetKeypair(ot::crypto::key::asymmetric::Role::Encrypt)
            .GetPublicKey()
            .keyType());
    EXPECT_EQ(keyCredentials2.second.size(), 2);
}

TEST_F(Test_Authority, GetMasterCredID_DefaultSetup_ShouldReturnProperData)
{
    const auto& masterCredID = authority_->GetMasterCredID();

    EXPECT_EQ(ot::identifier::Type::generic, masterCredID.Type());
    EXPECT_EQ(ot::identifier::Algorithm::blake2b256, masterCredID.Algorithm());
}

TEST_F(
    Test_Authority,
    GetPublicKeysBySignature_DefaultSetup_ShouldReturnProperData)
{
    ot::crypto::key::Keypair::Keys keys;

    auto signature = opentxs::Signature::Factory(client_);
    EXPECT_EQ(0, authority_->GetPublicKeysBySignature(keys, signature));
    EXPECT_EQ(0, keys.size());

    EXPECT_EQ(1, authority_->GetPublicKeysBySignature(keys, signature, 'A'));
    EXPECT_EQ(1, keys.size());

    EXPECT_EQ(
        ot::crypto::key::asymmetric::Algorithm::Secp256k1,
        keys.front()->keyType());
}

TEST_F(
    Test_Authority,
    HasCapability_DefaultSetup_ShouldReturnProperTrueForBothParameters)
{
    EXPECT_TRUE(
        authority_->hasCapability(ot::identity::NymCapability::SIGN_CHILDCRED));
    EXPECT_TRUE(
        authority_->hasCapability(ot::identity::NymCapability::SIGN_MESSAGE));
}

TEST_F(Test_Authority, Params_Secp256k1AsParameter_ShouldReturnProperData)
{
    const auto& params =
        authority_->Params(ot::crypto::key::asymmetric::Algorithm::Secp256k1);

    EXPECT_EQ(params.size(), 0);
}

TEST_F(
    Test_Authority,
    Params_AddChildKeyCredentialCalledFirstLegacyAsParameter_ShouldReturnProperData)
{
    const auto& params =
        authority_->Params(ot::crypto::key::asymmetric::Algorithm::Legacy);

    EXPECT_EQ(params.size(), 0);

    ot::crypto::Parameters parameters{
        ot::crypto::ParameterType::rsa,
        ot::identity::CredentialType::HD,
        ot::identity::SourceType::PubKey,
        2};

    parameters.SetSeed(parameters_.Seed());
    authority_->AddChildKeyCredential(parameters, reason_);

    const auto& params2 =
        authority_->Params(ot::crypto::key::asymmetric::Algorithm::Legacy);

    EXPECT_NE(params2.size(), 0);
}

TEST_F(Test_Authority, Path_DefaultSetup_ShouldReturnProperData)
{
    ot::proto::HDPath output;
    EXPECT_EQ(output.child_size(), 0);
    EXPECT_EQ(output.has_version(), false);

    EXPECT_TRUE(authority_->Path(output));

    EXPECT_NE(output.child_size(), 0);
    EXPECT_EQ(output.has_version(), true);
}

TEST_F(Test_Authority, Serialize_AddedCredentialsFirst_ShouldReturnProperData)
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

    ot::UnallocatedList<ot::UnallocatedCString> list, list2;
    authority_->RevokeContactCredentials(list);
    authority_->RevokeVerificationCredentials(list2);

    // Under 1th index there is some credential created during Authority
    // creation
    EXPECT_EQ(serialized.activechildids(1), list.front());
    EXPECT_EQ(serialized.activechildids(2), list2.front());

    EXPECT_EQ(
        serialized.masterid(),
        authority_->GetMasterCredID().asBase58(client_.Crypto()));
    EXPECT_EQ(
        serialized.nymid(), internal_nym_->ID().asBase58(client_.Crypto()));
}

auto func() -> ot::UnallocatedCString;

auto func() -> ot::UnallocatedCString { return "Test"; }

TEST_F(Test_Authority, Sign_ShouldReturnProperData)
{
    std::function<ot::UnallocatedCString()> fc = func;

    ot::crypto::SignatureRole role =
        ot::crypto::SignatureRole::PublicCredential;
    ot::proto::Signature signature;
    EXPECT_TRUE(authority_->Sign(fc, role, signature, reason_));

    EXPECT_EQ(signature.version(), 1);
}

TEST_F(Test_Authority, Sign_SignatureRoleIsNymIDSource_ShouldReturnProperData)
{
    std::function<ot::UnallocatedCString()> fc = func;

    ot::crypto::SignatureRole role = ot::crypto::SignatureRole::NymIDSource;
    ot::proto::Signature signature;
    EXPECT_FALSE(authority_->Sign(fc, role, signature, reason_));

    EXPECT_FALSE(signature.has_version());
}

TEST_F(
    Test_Authority,
    Sign_SignatureRoleIsPrivateCredential_ShouldReturnProperData)
{
    std::function<ot::UnallocatedCString()> fc = func;

    ot::crypto::SignatureRole role =
        ot::crypto::SignatureRole::PrivateCredential;
    ot::proto::Signature signature;
    EXPECT_FALSE(authority_->Sign(fc, role, signature, reason_));

    EXPECT_FALSE(signature.has_version());
}

TEST_F(
    Test_Authority,
    Sign_SignatureRoleIsServerContract_ShouldReturnProperData)
{
    std::function<ot::UnallocatedCString()> fc = func;

    ot::crypto::SignatureRole role = ot::crypto::SignatureRole::ServerContract;
    ot::proto::Signature signature;
    EXPECT_TRUE(authority_->Sign(fc, role, signature, reason_));

    EXPECT_TRUE(signature.has_version());
}

TEST_F(Test_Authority, Source_DefaultSetup_ShouldReturnProperData)
{
    const auto& source = authority_->Source();
    EXPECT_EQ(std::addressof(source), std::addressof(internal_nym_->Source()));
}

TEST_F(Test_Authority, Unlock_DefaultSetup_ShouldReturnProperData)
{
    auto testTag = std::uint32_t{};
    const auto symmetricKey = ot::crypto::symmetric::Key{};
    const auto& str = authority_->GetPublicAuthKey(
        ot::crypto::key::asymmetric::Algorithm::Secp256k1);

    const auto& tagCredential = authority_->GetTagCredential(
        ot::crypto::key::asymmetric::Algorithm::Secp256k1);

    const auto& encryptKey =
        tagCredential.GetKeypair(ot::crypto::key::asymmetric::Role::Encrypt)
            .GetPrivateKey();

    encryptKey.CalculateTag(
        str, authority_->GetMasterCredID(), reason_, testTag);

    EXPECT_FALSE(authority_->Unlock(
        str,
        testTag,
        ot::crypto::key::asymmetric::Algorithm::Secp256k1,
        symmetricKey,
        non_const_reason_));
}

TEST_F(Test_Authority, Unlock_DefaultSetup_ShouldReturnProperData2)
{
    const auto& tagCredential = authority_->GetTagCredential(
        ot::crypto::key::asymmetric::Algorithm::Secp256k1);

    const auto& encryptKey =
        tagCredential.GetKeypair(ot::crypto::key::asymmetric::Role::Encrypt)
            .GetPrivateKey();

    auto testTag = std::uint32_t{};
    const auto key = client_.Crypto().Symmetric().Key(
        opentxs::crypto::symmetric::Algorithm::ChaCha20Poly1305, reason_);

    const auto& str = authority_->GetPublicAuthKey(
        ot::crypto::key::asymmetric::Algorithm::Secp256k1);

    encryptKey.CalculateTag(
        str, authority_->GetMasterCredID(), reason_, testTag);

    EXPECT_TRUE(authority_->Unlock(
        str,
        testTag,
        ot::crypto::key::asymmetric::Algorithm::Secp256k1,
        key,
        non_const_reason_));
}

TEST_F(Test_Authority, TransportKey_DefaultSetup_ShouldReturnTrue)
{
    auto publicKey = ot::ByteArray{};
    EXPECT_TRUE(authority_->TransportKey(publicKey, words_, reason_));
}

TEST_F(
    Test_Authority,
    VerificationCredentialVersion_DefaultSetup_ShouldReturnProperData)
{
    EXPECT_EQ(authority_->VerificationCredentialVersion(), 1);
}

TEST_F(Test_Authority, Verify_DefaultSetup_ShouldReturnProperData)
{
    auto publicKey2 = ot::ByteArray{};
    ot::proto::Signature signature2;
    EXPECT_FALSE(authority_->Verify(
        publicKey2, signature2, opentxs::crypto::key::asymmetric::Role::Auth));
}

TEST_F(
    Test_Authority,
    Verify_WithCredentialsEqualToMasterCredID_ShouldReturnFalse)
{
    auto publicKey = ot::ByteArray{};
    ot::proto::Signature signature;
    *signature.mutable_credentialid() =
        authority_->GetMasterCredID().asBase58(client_.Crypto());

    EXPECT_FALSE(authority_->Verify(
        publicKey, signature, opentxs::crypto::key::asymmetric::Role::Auth));
}
TEST_F(Test_Authority, Verify_WithChildKeyCredential_ShouldReturnFalse)
{
    auto publicKey = ot::ByteArray{};

    ot::crypto::Parameters parameters{
        ot::crypto::ParameterType::ed25519,
        ot::identity::CredentialType::HD,
        ot::identity::SourceType::PubKey,
        2};

    parameters.SetSeed(parameters_.Seed());
    auto credential = authority_->AddChildKeyCredential(parameters, reason_);

    ot::proto::Signature signature;
    *signature.mutable_credentialid() = credential;

    EXPECT_FALSE(authority_->Verify(
        publicKey, signature, opentxs::crypto::key::asymmetric::Role::Auth));
}

TEST_F(Test_Authority, Verify_DefaultSetup_ShouldReturnFalse)
{
    ot::proto::Verification verification;
    EXPECT_FALSE(authority_->Verify(verification));
}
TEST_F(Test_Authority, VerifyInternally_DefaultSetup_ShouldReturnTrue)
{
    EXPECT_TRUE(authority_->VerifyInternally());
}

TEST_F(Test_Authority, WriteCredentials_DefaultSetup_ShouldReturnProperData)
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
        ot::crypto::ParameterType::ed25519,
        ot::identity::CredentialType::HD,
        ot::identity::SourceType::PubKey,
        2};

    parameters.SetSeed(parameters_.Seed());
    authority_->AddChildKeyCredential(parameters, reason_);

    EXPECT_TRUE(authority_->WriteCredentials());
}
//???????
TEST_F(Test_Authority, GetPairs_DefaultSetup_ShouldReturnProperData)
{
    const auto& authKeyPair = authority_->GetAuthKeypair(
        ot::crypto::key::asymmetric::Algorithm::Secp256k1);

    EXPECT_EQ(
        authKeyPair.GetPrivateKey().keyType(),
        ot::crypto::key::asymmetric::Algorithm::Secp256k1);
    EXPECT_EQ(
        authKeyPair.GetPrivateKey().Role(),
        ot::crypto::key::asymmetric::Role::Auth);

    const auto& encrKeyPair = authority_->GetEncrKeypair(
        ot::crypto::key::asymmetric::Algorithm::Secp256k1);

    EXPECT_EQ(
        encrKeyPair.GetPrivateKey().keyType(),
        ot::crypto::key::asymmetric::Algorithm::Secp256k1);
    EXPECT_EQ(
        encrKeyPair.GetPrivateKey().Role(),
        ot::crypto::key::asymmetric::Role::Encrypt);

    const auto& signKeyPair = authority_->GetSignKeypair(
        ot::crypto::key::asymmetric::Algorithm::Secp256k1);

    EXPECT_EQ(
        signKeyPair.GetPrivateKey().keyType(),
        ot::crypto::key::asymmetric::Algorithm::Secp256k1);
    EXPECT_EQ(
        signKeyPair.GetPrivateKey().Role(),
        ot::crypto::key::asymmetric::Role::Sign);
}

TEST_F(Test_Authority, GetTagCredential_DefaultSetup_ShouldReturnProperData)
{
    const auto& tagCredential = authority_->GetTagCredential(
        ot::crypto::key::asymmetric::Algorithm::Secp256k1);
    const auto& keyPair =
        tagCredential.GetKeypair(ot::crypto::key::asymmetric::Role::Sign);

    EXPECT_EQ(
        keyPair.GetPrivateKey().keyType(),
        ot::crypto::key::asymmetric::Algorithm::Secp256k1);
    EXPECT_EQ(
        keyPair.GetPrivateKey().Role(),
        ot::crypto::key::asymmetric::Role::Sign);
}
}  // namespace ottest
