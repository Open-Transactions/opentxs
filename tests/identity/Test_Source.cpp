// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <iostream>
#include <memory>

#include "../tests/mocks/identity/credential/PrimaryMock.hpp"
#include "2_Factory.hpp"
#include "identity/credential/Primary.hpp"
#include "internal/api/crypto/Seed.hpp"
#include "internal/api/session/Client.hpp"
#include "internal/core/PaymentCode.hpp"
#include "internal/identity/Authority.hpp"
#include "internal/identity/Nym.hpp"
#include "internal/otx/client/obsolete/OTAPI_Exec.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/crypto/Config.hpp"
#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Contacts.hpp"
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
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/crypto/Bip32Child.hpp"
#include "opentxs/crypto/Bip43Purpose.hpp"
#include "opentxs/crypto/Bip44Type.hpp"
#include "opentxs/crypto/Language.hpp"
#include "opentxs/crypto/Parameters.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/SeedStyle.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/key/HD.hpp"             // IWYU pragma: keep
#include "opentxs/crypto/key/asymmetric/Algorithm.hpp"
#include "opentxs/identity/CredentialType.hpp"
#include "opentxs/identity/IdentityType.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/Source.hpp"
#include "opentxs/identity/SourceType.hpp"
#include "opentxs/util/NymEditor.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "paymentcode/VectorsV3.hpp"
#include "serialization/protobuf/Credential.pb.h"
#include "serialization/protobuf/Enums.pb.h"
#include "serialization/protobuf/Nym.pb.h"
#include "serialization/protobuf/NymIDSource.pb.h"  // IWYU pragma: keep
#include "serialization/protobuf/SourceProof.pb.h"
#include "util/HDIndex.hpp"

namespace opentxs
{
using namespace testing;

class Test_Source : public ::testing::Test
{
public:
    const api::session::Client& client_;
    const OTPasswordPrompt reason_;
    OTSecret words_;
    OTSecret phrase_;
    const std::uint8_t version_ = 1;
    const int nym_ = 1;
    const ot::UnallocatedCString alias_ = ot::UnallocatedCString{"alias"};

    crypto::Parameters parameters_;
    std::unique_ptr<identity::Source> source_;
    std::unique_ptr<ot::identity::internal::Nym> internalNym_;
    std::unique_ptr<identity::internal::Authority> authority_;

    Test_Source()
        : client_(ot::Context().StartClientSession(0))
        , reason_(client_.Factory().PasswordPrompt(__func__))
        , words_(client_.Factory().SecretFromText(
              ottest::GetVectors3().alice_.words_))
        , phrase_(client_.Factory().SecretFromText(
              ottest::GetVectors3().alice_.words_))
        , parameters_()
        , source_{nullptr}
        , internalNym_{nullptr}
        , authority_{nullptr}
    {
    }

    void Authority()
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

        const auto& nn =
            dynamic_cast<const opentxs::identity::Nym&>(*internalNym_);

        authority_.reset(Factory().Authority(
            client_, nn, *source_, parameters_, 6, reason_));

        /*    auto nym = client_.Wallet().Nym(reason_);

            authority_.reset(Factory().Authority(
                client_, *nym, *source_, parameters_, version_, reason_));*/
    }
    void setupSourceForBip47(ot::crypto::SeedStyle seedStyle)
    {
        auto seed = client_.Crypto().Seed().ImportSeed(
            words_, phrase_, seedStyle, ot::crypto::Language::en, reason_);

        crypto::Parameters parameters{
            crypto::Parameters::DefaultType(),
            crypto::Parameters::DefaultCredential(),
            identity::SourceType::Bip47,
            version_};
        parameters.SetSeed(seed);
        parameters.SetNym(nym_);

        source_.reset(ot::Factory::NymIDSource(client_, parameters, reason_));
    }

    void setupSourceForPubKey(ot::crypto::SeedStyle seedStyle)
    {
        crypto::Parameters parameters{
            crypto::key::asymmetric::Algorithm::Secp256k1,
            identity::CredentialType::HD,
            identity::SourceType::PubKey,
            version_};

        auto seed = client_.Crypto().Seed().ImportSeed(
            words_, phrase_, seedStyle, ot::crypto::Language::en, reason_);

        parameters.SetSeed(seed);
        parameters.SetNym(nym_);
        source_.reset(ot::Factory::NymIDSource(client_, parameters, reason_));
    }
};

/////////////// Constructors ////////////////
TEST_F(Test_Source, Constructor_WithProtoOfTypeBIP47_ShouldNotThrow)
{
    opentxs::proto::NymIDSource nymIdProto;
    nymIdProto.set_type(proto::SOURCETYPE_BIP47);

    EXPECT_NO_THROW(ot::Factory::NymIDSource(client_, nymIdProto));
}

TEST_F(Test_Source, Constructor_WithProtoOfTypePUBKEY_ShouldNotThrow)
{
    opentxs::proto::NymIDSource nymIdProto;
    nymIdProto.set_type(proto::SOURCETYPE_PUBKEY);

    EXPECT_NO_THROW(ot::Factory::NymIDSource(client_, nymIdProto));
}

TEST_F(Test_Source, Constructor_WithCredentialTypeError_ShouldThrow)
{
    crypto::Parameters parameters{
        crypto::key::asymmetric::Algorithm::Secp256k1,
        identity::CredentialType::Error,
        identity::SourceType::PubKey,
        version_};
    auto seed = client_.Crypto().Seed().ImportSeed(
        words_,
        phrase_,
        ot::crypto::SeedStyle::BIP39,
        ot::crypto::Language::en,
        reason_);

    parameters.SetSeed(seed);
    EXPECT_THROW(
        ot::Factory::NymIDSource(client_, parameters, reason_),
        std::runtime_error);
}

TEST_F(Test_Source, Constructor_WithParametersCredentialTypeHD_ShouldNotThrow)
{
    crypto::Parameters parameters{
        crypto::key::asymmetric::Algorithm::Secp256k1,
        identity::CredentialType::HD,
        identity::SourceType::PubKey,
        version_};
    auto seed = client_.Crypto().Seed().ImportSeed(
        words_,
        phrase_,
        ot::crypto::SeedStyle::BIP39,
        ot::crypto::Language::en,
        reason_);

    parameters.SetSeed(seed);
    EXPECT_NO_THROW(ot::Factory::NymIDSource(client_, parameters, reason_));
}

TEST_F(
    Test_Source,
    Constructor_WithParametersCredentialTypeLegacy_ShouldNotThrow)
{
    crypto::Parameters parameters{
        crypto::key::asymmetric::Algorithm::Secp256k1,
        identity::CredentialType::Legacy,
        identity::SourceType::PubKey,
        version_};
    EXPECT_NO_THROW(ot::Factory::NymIDSource(client_, parameters, reason_));
}

TEST_F(
    Test_Source,
    Constructor_WithParametersSourceTypeError_ShouldReturnNullptr)
{
    crypto::Parameters parameters{
        crypto::Parameters::DefaultType(),
        crypto::Parameters::DefaultCredential(),
        identity::SourceType::Error,
        version_};

    EXPECT_EQ(ot::Factory::NymIDSource(client_, parameters, reason_), nullptr);
}

/////////////// Serialize ////////////////
TEST_F(Test_Source, Serialize_seedBIP39SourceBip47_ShouldSetProperFields)
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

TEST_F(Test_Source, Serialize_seedBIP39SourcePubKey_ShouldSetProperFields)
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

/////////////// SIGN ////////////////
TEST_F(Test_Source, Sign_ShouldReturnTrue)
{
    setupSourceForBip47(ot::crypto::SeedStyle::BIP39);
    const identity::credential::PrimaryMock credentialMock;
    proto::Signature sig;
    Authority();
    EXPECT_CALL(credentialMock, Internal())
        .WillOnce(ReturnRef(authority_->GetMasterCredential().Internal()));
    EXPECT_TRUE(source_->Sign(credentialMock, sig, reason_));
}

/////////////// VERIFY ////////////////
TEST_F(Test_Source, Verify_seedPubKeySourceBip47_ShouldReturnTrue)
{
    auto masterId = "SOME ID BIGGER THAN 20 CHARS";
    auto nymId = "SOME NYM ID BIGGER THAN 20 CHARS";
    crypto::Parameters parameters{
        crypto::key::asymmetric::Algorithm::Secp256k1,
        identity::CredentialType::HD,
        identity::SourceType::PubKey,
        version_};

    auto seed = client_.Crypto().Seed().ImportSeed(
        words_,
        phrase_,
        ot::crypto::SeedStyle::BIP39,
        ot::crypto::Language::en,
        reason_);

    parameters.SetSeed(seed);
    parameters.SetNym(nym_);
    source_.reset(ot::Factory::NymIDSource(client_, parameters, reason_));

    const auto path = UnallocatedVector<Bip32Index>{
        HDIndex{Bip43Purpose::NYM, Bip32Child::HARDENED},
        HDIndex{nym_, Bip32Child::HARDENED},
        HDIndex{0, Bip32Child::HARDENED},
        HDIndex{0, Bip32Child::HARDENED},
        HDIndex{Bip32Child::SIGN_KEY, Bip32Child::HARDENED}};

    opentxs::Bip32Index idx{0};

    const auto& derivedKey = client_.Crypto().BIP32().DeriveKey(
        crypto::EcdsaCurve::secp256k1,
        client_.Crypto().Seed().GetSeed(parameters.Seed(), idx, reason_),
        path);
    const auto& pubkey = std::get<2>(derivedKey);

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
        *key->mutable_key() = pubkey->str();
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

}  // namespace opentxs
