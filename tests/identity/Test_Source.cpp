// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <iostream>
#include <memory>

#include "../tests/mocks/crypto/ParametersMock.h"
#include "../tests/mocks/identity/credential/PrimaryMock.h"
#include "2_Factory.hpp"
#include "identity/credential/Primary.hpp"
#include "internal/api/session/Client.hpp"
#include "internal/core/PaymentCode.hpp"
#include "internal/otx/client/obsolete/OTAPI_Exec.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/crypto/Config.hpp"
#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/HDProtocol.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/crypto/Bip44Type.hpp"
#include "opentxs/crypto/Language.hpp"
#include "opentxs/crypto/Parameters.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/SeedStyle.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/key/HD.hpp"             // IWYU pragma: keep
#include "opentxs/crypto/key/asymmetric/Algorithm.hpp"
#include "opentxs/crypto/key/asymmetric/Role.hpp"
#include "opentxs/identity/CredentialType.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/Source.hpp"
#include "opentxs/identity/SourceType.hpp"
#include "opentxs/util/NymEditor.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "paymentcode/VectorsV3.hpp"
#include "serialization/protobuf/AsymmetricKey.pb.h"
#include "serialization/protobuf/Credential.pb.h"
#include "serialization/protobuf/Enums.pb.h"
#include "serialization/protobuf/Nym.pb.h"
#include "serialization/protobuf/NymIDSource.pb.h"  // IWYU pragma: keep
#include "serialization/protobuf/SourceProof.pb.h"

using namespace testing;
namespace opentxs
{

class Test_Source : public ::testing::Test
{
public:
    const api::session::Client& client_;
    const OTPasswordPrompt reason_;
    OTSecret words_;
    OTSecret phrase_;
    const int version_ = 1;
    const int nym_ = 1;
    crypto::ParametersMock parametersMock_ = crypto::ParametersMock();
    std::unique_ptr<identity::Source> source_;

    Test_Source()
        : client_(ot::Context().StartClientSession(0))
        , reason_(client_.Factory().PasswordPrompt(__func__))
        , words_(client_.Factory().SecretFromText(
              ottest::GetVectors3().alice_.words_))
        , phrase_(client_.Factory().SecretFromText(
              ottest::GetVectors3().alice_.words_))
    {
    }

    void setupSourceForBip47(ot::crypto::SeedStyle seedStyle)
    {
        auto seed = client_.Crypto().Seed().ImportSeed(
            words_, phrase_, seedStyle, ot::crypto::Language::en, reason_);

        EXPECT_CALL(parametersMock_, SourceType())
            .WillOnce(Return(identity::SourceType::Bip47));

        EXPECT_CALL(parametersMock_, PaymentCodeVersion())
            .WillOnce(Return(version_));
        EXPECT_CALL(parametersMock_, Nym()).WillOnce(Return(nym_));
        EXPECT_CALL(parametersMock_, Seed()).WillOnce(Return(seed));

        source_.reset(
            ot::Factory::NymIDSource(client_, parametersMock_, reason_));
    }

    void setupSourceForPubKey(ot::crypto::SeedStyle seedStyle)
    {
        auto seed = client_.Crypto().Seed().ImportSeed(
            words_,
            phrase_,
            ot::crypto::SeedStyle::BIP39,
            ot::crypto::Language::en,
            reason_);

        auto OTkeyPair = client_.Factory().Keypair(
            seed,
            1,
            0,
            0,
            EcdsaCurve::secp256k1,
            crypto::key::asymmetric::Role::Sign,
            reason_);

        EXPECT_CALL(parametersMock_, SourceType())
            .Times(2)
            .WillRepeatedly(Return(identity::SourceType::PubKey));
        EXPECT_CALL(parametersMock_, credentialType())
            .WillOnce(Return(identity::CredentialType::HD));
        EXPECT_CALL(parametersMock_, Algorithm())
            .WillOnce(Return(crypto::key::asymmetric::Algorithm::Secp256k1));

        EXPECT_CALL(parametersMock_, CredIndex()).WillOnce(Return(0));
        EXPECT_CALL(parametersMock_, Credset()).WillOnce(Return(0));
        EXPECT_CALL(parametersMock_, Nym()).WillOnce(Return(nym_));
        EXPECT_CALL(parametersMock_, Seed()).WillOnce(Return(seed));

        EXPECT_CALL(parametersMock_, Keypair())
            .Times(2)
            .WillRepeatedly(ReturnRef(OTkeyPair))
            .RetiresOnSaturation();

        const auto& constMock = parametersMock_;
        EXPECT_CALL(constMock, Keypair())
            .WillOnce(Invoke([&]() -> const crypto::key::Keypair& {
                return OTkeyPair.get();
            }));

        source_.reset(
            ot::Factory::NymIDSource(client_, parametersMock_, reason_));
    }
};

/////////////// Constructors ////////////////
TEST_F(Test_Source, Constructor_WithNymIDSource_ShouldNotThrow)
{
    opentxs::proto::NymIDSource nymIdProto;
    nymIdProto.set_type(proto::SOURCETYPE_BIP47);

    EXPECT_NO_THROW(ot::Factory::NymIDSource(client_, nymIdProto));

    nymIdProto.set_type(proto::SOURCETYPE_PUBKEY);

    EXPECT_NO_THROW(ot::Factory::NymIDSource(client_, nymIdProto));
}

TEST_F(Test_Source, Constructor_WithParametersAndReason_ShouldNotThrow)
{
    auto seed = client_.Crypto().Seed().ImportSeed(
        words_,
        phrase_,
        ot::crypto::SeedStyle::BIP39,
        ot::crypto::Language::en,
        reason_);

    EXPECT_CALL(parametersMock_, SourceType())
        .WillOnce(Return(identity::SourceType::PubKey));

    EXPECT_CALL(parametersMock_, credentialType())
        .WillOnce(Return(identity::CredentialType::Error));

    EXPECT_THROW(
        ot::Factory::NymIDSource(client_, parametersMock_, reason_),
        std::runtime_error);
}

TEST_F(Test_Source, Constructor_WIthCredentialTypeLegacy_ShouldNotThrow)
{
    auto seed = client_.Crypto().Seed().ImportSeed(
        words_,
        phrase_,
        ot::crypto::SeedStyle::BIP39,
        ot::crypto::Language::en,
        reason_);

    EXPECT_CALL(parametersMock_, SourceType())
        .Times(2)
        .WillRepeatedly(Return(identity::SourceType::PubKey));

    EXPECT_CALL(parametersMock_, credentialType())
        .WillOnce(Return(identity::CredentialType::Legacy));
    EXPECT_CALL(parametersMock_, Algorithm())
        .WillOnce(Return(crypto::key::asymmetric::Algorithm::Secp256k1));

    auto OTkeyPair = client_.Factory().Keypair(
        seed,
        2,
        0,
        0,
        EcdsaCurve::secp256k1,
        crypto::key::asymmetric::Role::Sign,
        reason_);

    EXPECT_CALL(parametersMock_, Keypair())
        .Times(2)
        .WillRepeatedly(ReturnRef(OTkeyPair))
        .RetiresOnSaturation();

    const auto& constMock = parametersMock_;
    EXPECT_CALL(constMock, Keypair())
        .WillOnce(Invoke(
            [&]() -> const crypto::key::Keypair& { return OTkeyPair.get(); }));

    EXPECT_NO_THROW(
        ot::Factory::NymIDSource(client_, parametersMock_, reason_));
}

TEST_F(Test_Source, Constructor_WithAlgorithmError_ShouldThrowRuntimeError)
{
    auto seed = client_.Crypto().Seed().ImportSeed(
        words_,
        phrase_,
        ot::crypto::SeedStyle::BIP39,
        ot::crypto::Language::en,
        reason_);

    EXPECT_CALL(parametersMock_, SourceType())
        .WillOnce(Return(identity::SourceType::PubKey));

    EXPECT_CALL(parametersMock_, credentialType())
        .WillOnce(Return(identity::CredentialType::HD));
    EXPECT_CALL(parametersMock_, Algorithm())
        .WillOnce(Return(crypto::key::asymmetric::Algorithm::Error));

    EXPECT_THROW(
        ot::Factory::NymIDSource(client_, parametersMock_, reason_),
        std::runtime_error);
}

TEST_F(Test_Source, Constructor_SourceTypeError_ShouldReturnNullptr)
{
    auto seed = client_.Crypto().Seed().ImportSeed(
        words_,
        phrase_,
        ot::crypto::SeedStyle::BIP39,
        ot::crypto::Language::en,
        reason_);

    EXPECT_CALL(parametersMock_, SourceType())
        .WillOnce(Return(identity::SourceType::Error));

    EXPECT_EQ(
        ot::Factory::NymIDSource(client_, parametersMock_, reason_), nullptr);
}
/////////////// Serialize ////////////////
TEST_F(Test_Source, seedBIP39SourceBip47_Serialize_ShouldSetProperFields)
{
    setupSourceForBip47(ot::crypto::SeedStyle::BIP39);
    opentxs::proto::NymIDSource nymIdProto;

    EXPECT_TRUE(source_->Serialize(nymIdProto));
    EXPECT_EQ(nymIdProto.version(), version_);
    EXPECT_EQ(nymIdProto.type(), proto::SourceType::SOURCETYPE_BIP47);
    EXPECT_FALSE(nymIdProto.paymentcode().key().empty());
    EXPECT_FALSE(nymIdProto.paymentcode().chaincode().empty());
    EXPECT_EQ(nymIdProto.paymentcode().version(), version_);
}

TEST_F(Test_Source, seedBIP39SourcePubKey_Serialize_ShouldSetProperFields)
{
    setupSourceForPubKey(ot::crypto::SeedStyle::BIP39);
    opentxs::proto::NymIDSource nymIdProto;
    EXPECT_TRUE(source_->Serialize(nymIdProto));
    EXPECT_EQ(
        nymIdProto.version(), opentxs::crypto::key::Asymmetric::DefaultVersion);

    EXPECT_EQ(nymIdProto.key().role(), proto::KEYROLE_SIGN);
    EXPECT_EQ(nymIdProto.type(), proto::SourceType::SOURCETYPE_PUBKEY);
    EXPECT_TRUE(nymIdProto.paymentcode().key().empty());
    EXPECT_TRUE(nymIdProto.paymentcode().chaincode().empty());
}

TEST_F(Test_Source, seedBIP32SourcePubKey_Serialize_ShouldSetProperFields)
{
    setupSourceForPubKey(ot::crypto::SeedStyle::BIP32);
    opentxs::proto::NymIDSource nymIdProto;
    EXPECT_TRUE(source_->Serialize(nymIdProto));
    EXPECT_EQ(
        nymIdProto.version(), opentxs::crypto::key::Asymmetric::DefaultVersion);

    EXPECT_EQ(nymIdProto.key().role(), proto::KEYROLE_SIGN);
    EXPECT_EQ(nymIdProto.type(), proto::SourceType::SOURCETYPE_PUBKEY);
    EXPECT_TRUE(nymIdProto.paymentcode().key().empty());
    EXPECT_TRUE(nymIdProto.paymentcode().chaincode().empty());
}

TEST_F(Test_Source, seedBIP32SourceBip47_Serialize_ShouldSetProperFields)
{
    setupSourceForBip47(ot::crypto::SeedStyle::BIP32);
    opentxs::proto::NymIDSource nymIdProto;

    EXPECT_TRUE(source_->Serialize(nymIdProto));
    EXPECT_EQ(nymIdProto.version(), version_);
    EXPECT_EQ(nymIdProto.type(), proto::SourceType::SOURCETYPE_BIP47);
    EXPECT_TRUE(nymIdProto.paymentcode().key().empty());
    EXPECT_TRUE(nymIdProto.paymentcode().chaincode().empty());
    EXPECT_EQ(nymIdProto.paymentcode().version(), version_);
}

TEST_F(Test_Source, seedPKTSourcePubKey_Serialize_ShouldSetProperFields)
{
    setupSourceForPubKey(ot::crypto::SeedStyle::PKT);
    opentxs::proto::NymIDSource nymIdProto;
    EXPECT_TRUE(source_->Serialize(nymIdProto));

    EXPECT_EQ(
        nymIdProto.version(), opentxs::crypto::key::Asymmetric::DefaultVersion);
    EXPECT_EQ(nymIdProto.key().role(), proto::KEYROLE_SIGN);
    EXPECT_EQ(nymIdProto.type(), proto::SourceType::SOURCETYPE_PUBKEY);
    EXPECT_TRUE(nymIdProto.paymentcode().key().empty());
    EXPECT_TRUE(nymIdProto.paymentcode().chaincode().empty());
}

/////////////// SIGN ////////////////
TEST_F(
    Test_Source,
    seedBIP39SourceBip47_Sign_SerializeReturnsTrueShouldReturnTrue)
{
    setupSourceForBip47(ot::crypto::SeedStyle::BIP39);
    const identity::credential::PrimaryMock credentialMock;
    proto::Signature sig;
    proto::Credential credential;

    EXPECT_CALL(credentialMock, Serialize(_, _, _)).WillOnce(Return(true));

    EXPECT_TRUE(source_->Sign(credentialMock, sig, reason_));
}

TEST_F(
    Test_Source,
    seedBIP39SourceBip47_Sign_SerializeReturnsFalseShouldReturnFalse)
{
    setupSourceForBip47(ot::crypto::SeedStyle::BIP39);
    const identity::credential::PrimaryMock credentialMock;
    proto::Signature sig;
    proto::Credential credential;

    EXPECT_CALL(credentialMock, Serialize(_, _, _)).WillOnce(Return(false));

    EXPECT_FALSE(source_->Sign(credentialMock, sig, reason_));
}

/////////////// NO THROW METHODS ////////////////
TEST_F(Test_Source, seedBIP39SourceBip47_NoThrowMethodChecks)
{
    setupSourceForBip47(ot::crypto::SeedStyle::BIP39);
    EXPECT_NO_THROW(source_->asString());
    EXPECT_NO_THROW(source_->Description());
    EXPECT_NO_THROW(source_->NymID());
    EXPECT_EQ(source_->Type(), identity::SourceType::Bip47);

    setupSourceForPubKey(ot::crypto::SeedStyle::BIP39);
    EXPECT_NO_THROW(source_->asString());
    EXPECT_NO_THROW(source_->Description());
    EXPECT_NO_THROW(source_->NymID());
    EXPECT_EQ(source_->Type(), identity::SourceType::PubKey);
}

TEST_F(Test_Source, seedBIP39SourceBip47_WithVersionSet_VerifyShouldReturnTrue)
{
    auto masterId = "SOME ID BIGGER THAN 20 CHARS";
    auto nymId = "SOME NYM ID BIGGER THAN 20 CHARS";
    setupSourceForBip47(ot::crypto::SeedStyle::BIP39);
    opentxs::proto::Credential credential;
    credential.set_version(version_);
    credential.set_id(masterId);
    credential.set_type(proto::CREDTYPE_HD);
    credential.set_role(proto::CREDROLE_MASTERKEY);
    credential.set_mode(proto::KEYMODE_PUBLIC);
    credential.set_nymid(nymId);
    credential.mutable_publiccredential()->set_version(version_);
    credential.mutable_publiccredential()->set_mode(proto::KEYMODE_PUBLIC);

    for (int i = 1; i < 4; ++i) {
        auto key = credential.mutable_publiccredential()->add_key();
        key->set_version(version_);
        key->set_mode(proto::KEYMODE_PUBLIC);
        key->set_type(proto::AKEYTYPE_SECP256K1);
        key->set_role(static_cast<proto::KeyRole>(i));
        *key->mutable_key() = "SOME KEY BIGGER THAN 20 CHARS";
    }
    auto masterData = credential.mutable_masterdata();
    masterData->set_version(version_);
    masterData->mutable_sourceproof()->set_version(version_);
    masterData->mutable_sourceproof()->set_type(
        proto::SOURCEPROOFTYPE_SELF_SIGNATURE);

    auto source = masterData->mutable_source();
    source->set_version(version_);
    source->set_type(proto::SOURCETYPE_BIP47);
    source->mutable_paymentcode()->set_version(version_);
    *source->mutable_paymentcode()->mutable_key() =
        "SOME CHAINCODE BIGGER THAN 20 CHARS";

    *source->mutable_paymentcode()->mutable_chaincode() =
        "SOME CHAINCODE BIGGER THAN 20 CHARS";

    auto childCredentialParameters = credential.mutable_childdata();
    childCredentialParameters->set_version(version_);
    childCredentialParameters->set_masterid(masterId);

    opentxs::proto::Signature sourceSignature;
    sourceSignature.set_version(version_);

    source_->Verify(credential, sourceSignature);
}

TEST_F(Test_Source, seedPubKeySourceBip47_WithVersionSet_VerifyShouldReturnTrue)
{
    auto masterId = "SOME ID BIGGER THAN 20 CHARS";
    auto nymId = "SOME NYM ID BIGGER THAN 20 CHARS";
    setupSourceForPubKey(ot::crypto::SeedStyle::BIP39);
    opentxs::proto::Credential credential;
    credential.set_version(version_);
    credential.set_id(masterId);
    credential.set_type(proto::CREDTYPE_HD);
    credential.set_role(proto::CREDROLE_MASTERKEY);
    credential.set_mode(proto::KEYMODE_PUBLIC);
    credential.set_nymid(nymId);
    credential.mutable_publiccredential()->set_version(version_);
    credential.mutable_publiccredential()->set_mode(proto::KEYMODE_PUBLIC);

    for (int i = 1; i < 4; ++i) {
        auto key = credential.mutable_publiccredential()->add_key();
        key->set_version(version_);
        key->set_mode(proto::KEYMODE_PUBLIC);
        key->set_type(proto::AKEYTYPE_SECP256K1);
        key->set_role(static_cast<proto::KeyRole>(i));
        *key->mutable_key() = "SOME KEY BIGGER THAN 20 CHARS";
    }
    auto masterData = credential.mutable_masterdata();
    masterData->set_version(version_);
    masterData->mutable_sourceproof()->set_version(version_);
    masterData->mutable_sourceproof()->set_type(
        proto::SOURCEPROOFTYPE_SELF_SIGNATURE);

    auto source = masterData->mutable_source();
    source->set_version(version_);
    source->set_type(proto::SOURCETYPE_BIP47);
    source->mutable_paymentcode()->set_version(version_);
    *source->mutable_paymentcode()->mutable_key() =
        "SOME CHAINCODE BIGGER THAN 20 CHARS";

    *source->mutable_paymentcode()->mutable_chaincode() =
        "SOME CHAINCODE BIGGER THAN 20 CHARS";

    auto childCredentialParameters = credential.mutable_childdata();
    childCredentialParameters->set_version(version_);
    childCredentialParameters->set_masterid(masterId);

    opentxs::proto::Signature sourceSignature;
    sourceSignature.set_version(version_);

    source_->Verify(credential, sourceSignature);
}

}  // namespace opentxs