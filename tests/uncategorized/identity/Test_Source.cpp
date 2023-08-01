// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <AsymmetricKey.pb.h>
#include <ChildCredentialParameters.pb.h>
#include <Credential.pb.h>
#include <Enums.pb.h>
#include <KeyCredential.pb.h>
#include <MasterCredentialParameters.pb.h>
#include <NymIDSource.pb.h>  // IWYU pragma: keep
#include <PaymentCode.pb.h>
#include <Signature.pb.h>
#include <SourceProof.pb.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <tuple>

#include "2_Factory.hpp"
#include "internal/api/crypto/Seed.hpp"
#include "internal/identity/Authority.hpp"
#include "internal/identity/Nym.hpp"
#include "internal/identity/Source.hpp"
#include "ottest/data/crypto/PaymentCodeV3.hpp"
#include "ottest/env/OTTestEnvironment.hpp"
#include "ottest/fixtures/core/Identifier.hpp"
#include "ottest/mocks/identity/credential/Primary.hpp"
#include "util/HDIndex.hpp"

namespace ottest
{
class Test_Source : public ::testing::Test
{
public:
    const ot::api::session::Client& api_;
    const ot::PasswordPrompt reason_;
    ot::Secret words_;
    ot::Secret phrase_;
    const std::uint8_t version_ = 1;
    const int nym_ = 1;
    const ot::UnallocatedCString alias_ = ot::UnallocatedCString{"alias"};

    ot::crypto::Parameters parameters_;
    std::unique_ptr<ot::identity::Source> source_;
    std::unique_ptr<ot::identity::internal::Nym> internal_nym_;
    std::unique_ptr<ot::identity::internal::Authority> authority_;

    Test_Source()
        : api_(OTTestEnvironment::GetOT().StartClientSession(0))
        , reason_(api_.Factory().PasswordPrompt(__func__))
        , words_(api_.Factory().SecretFromText(
              ottest::GetPaymentCodeVector3().alice_.words_))
        , phrase_(api_.Factory().SecretFromText(
              ottest::GetPaymentCodeVector3().alice_.words_))
        , parameters_(api_.Factory())
        , source_{nullptr}
        , internal_nym_{nullptr}
        , authority_{nullptr}
    {
    }

    void Authority()
    {
        const auto& seeds = api_.Crypto().Seed().Internal();
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

        source_.reset(ot::Factory::NymIDSource(api_, parameters_, reason_));

        internal_nym_.reset(ot::Factory::Nym(
            api_,
            parameters_,
            ot::identity::Type::individual,
            alias_,
            reason_));

        const auto& nn =
            dynamic_cast<const opentxs::identity::Nym&>(*internal_nym_);

        authority_.reset(ot::Factory().Authority(
            api_, nn, *source_, parameters_, 6, reason_));
    }

    void setupSourceForBip47(ot::crypto::SeedStyle seedStyle)
    {
        auto seed = api_.Crypto().Seed().ImportSeed(
            words_, phrase_, seedStyle, ot::crypto::Language::en, reason_);

        ot::crypto::Parameters parameters{
            api_.Factory(),
            ot::crypto::Parameters::DefaultType(),
            ot::crypto::Parameters::DefaultCredential(),
            ot::identity::SourceType::Bip47,
            version_};
        parameters.SetSeed(seed);
        parameters.SetNym(nym_);

        source_.reset(ot::Factory::NymIDSource(api_, parameters, reason_));
    }

    void setupSourceForPubKey(ot::crypto::SeedStyle seedStyle)
    {
        ot::crypto::Parameters parameters{
            api_.Factory(),
            ot::crypto::asymmetric::Algorithm::Secp256k1,
            ot::identity::CredentialType::HD,
            ot::identity::SourceType::PubKey,
            version_};

        auto seed = api_.Crypto().Seed().ImportSeed(
            words_, phrase_, seedStyle, ot::crypto::Language::en, reason_);

        parameters.SetSeed(seed);
        parameters.SetNym(nym_);
        source_.reset(ot::Factory::NymIDSource(api_, parameters, reason_));
    }
};

/////////////// Constructors ////////////////
TEST_F(Test_Source, Constructor_WithProtoOfTypeBIP47_ShouldNotThrow)
{
    opentxs::proto::NymIDSource nymIdProto;
    nymIdProto.set_type(ot::proto::SOURCETYPE_BIP47);
    *nymIdProto.mutable_paymentcode()->mutable_chaincode() = "test";
    source_.reset(ot::Factory::NymIDSource(api_, nymIdProto));
    EXPECT_NE(source_, nullptr);
}

TEST_F(Test_Source, Constructor_WithProtoOfTypePUBKEY_ShouldNotThrow)
{
    opentxs::proto::NymIDSource nymIdProto;
    nymIdProto.set_type(ot::proto::SOURCETYPE_PUBKEY);

    source_.reset(ot::Factory::NymIDSource(api_, nymIdProto));
    EXPECT_NE(source_, nullptr);
}

TEST_F(Test_Source, Constructor_WithCredentialTypeError_ShouldThrow)
{
    ot::crypto::Parameters parameters{
        api_.Factory(),
        ot::crypto::asymmetric::Algorithm::Secp256k1,
        ot::identity::CredentialType::Error,
        ot::identity::SourceType::PubKey,
        version_};
    auto seed = api_.Crypto().Seed().ImportSeed(
        words_,
        phrase_,
        ot::crypto::SeedStyle::BIP39,
        ot::crypto::Language::en,
        reason_);

    parameters.SetSeed(seed);
    EXPECT_THROW(
        ot::Factory::NymIDSource(api_, parameters, reason_),
        std::runtime_error);
}

TEST_F(
    Test_Source,
    Constructor_WithParametersCredentialTypeHD_ShouldNotReturnNullptr)
{
    ot::crypto::Parameters parameters{
        api_.Factory(),
        ot::crypto::asymmetric::Algorithm::Secp256k1,
        ot::identity::CredentialType::HD,
        ot::identity::SourceType::PubKey,
        version_};
    auto seed = api_.Crypto().Seed().ImportSeed(
        words_,
        phrase_,
        ot::crypto::SeedStyle::BIP39,
        ot::crypto::Language::en,
        reason_);

    parameters.SetSeed(seed);
    source_.reset(ot::Factory::NymIDSource(api_, parameters, reason_));
    EXPECT_NE(source_, nullptr);
}

TEST_F(
    Test_Source,
    Constructor_WithParametersCredentialTypeLegacy_ShouldNotReturnNullptr)
{
    ot::crypto::Parameters parameters{
        api_.Factory(),
        ot::crypto::asymmetric::Algorithm::Secp256k1,
        ot::identity::CredentialType::Legacy,
        ot::identity::SourceType::PubKey,
        version_};
    source_.reset(ot::Factory::NymIDSource(api_, parameters, reason_));
    EXPECT_NE(source_, nullptr);
}

TEST_F(
    Test_Source,
    Constructor_WithParametersSourceTypeError_ShouldReturnNullptr)
{
    ot::crypto::Parameters parameters{
        api_.Factory(),
        ot::crypto::Parameters::DefaultType(),
        ot::crypto::Parameters::DefaultCredential(),
        ot::identity::SourceType::Error,
        version_};

    EXPECT_EQ(ot::Factory::NymIDSource(api_, parameters, reason_), nullptr);
}

/////////////// Serialize ////////////////
TEST_F(Test_Source, Serialize_seedBIP39SourceBip47_ShouldSetProperFields)
{
    setupSourceForBip47(ot::crypto::SeedStyle::BIP39);
    opentxs::proto::NymIDSource nymIdProto;

    EXPECT_TRUE(source_->Internal().Serialize(nymIdProto));
    EXPECT_EQ(nymIdProto.version(), version_);
    EXPECT_EQ(nymIdProto.type(), ot::proto::SourceType::SOURCETYPE_BIP47);
    EXPECT_FALSE(nymIdProto.paymentcode().key().empty());
    EXPECT_FALSE(nymIdProto.paymentcode().chaincode().empty());
    EXPECT_EQ(nymIdProto.paymentcode().version(), version_);
}

TEST_F(Test_Source, Serialize_seedBIP39SourcePubKey_ShouldSetProperFields)
{
    setupSourceForPubKey(ot::crypto::SeedStyle::BIP39);
    opentxs::proto::NymIDSource nymIdProto;
    EXPECT_TRUE(source_->Internal().Serialize(nymIdProto));
    EXPECT_EQ(
        nymIdProto.version(),
        opentxs::crypto::asymmetric::Key::DefaultVersion());

    EXPECT_EQ(nymIdProto.key().role(), ot::proto::KEYROLE_SIGN);
    EXPECT_EQ(nymIdProto.type(), ot::proto::SourceType::SOURCETYPE_PUBKEY);
    EXPECT_TRUE(nymIdProto.paymentcode().key().empty());
    EXPECT_TRUE(nymIdProto.paymentcode().chaincode().empty());
}

/////////////// SIGN ////////////////
TEST_F(Test_Source, Sign_ShouldReturnTrue)
{
    setupSourceForBip47(ot::crypto::SeedStyle::BIP39);
    const ot::identity::credential::PrimaryMock credentialMock;
    ot::proto::Signature sig;
    Authority();
    EXPECT_CALL(credentialMock, Internal())
        .WillOnce(
            ::testing::ReturnRef(authority_->GetMasterCredential().Internal()));
    EXPECT_TRUE(source_->Internal().Sign(credentialMock, sig, reason_));
}

/////////////// VERIFY ////////////////
TEST_F(Test_Source, Verify_seedPubKeySourceBip47_ShouldReturnTrue)
{
    const auto masterId = api_.Factory().IdentifierFromRandom();
    const auto nymId = api_.Factory().NymIDFromRandom();
    ot::crypto::Parameters parameters{
        api_.Factory(),
        ot::crypto::asymmetric::Algorithm::Secp256k1,
        ot::identity::CredentialType::HD,
        ot::identity::SourceType::PubKey,
        version_};

    auto seed = api_.Crypto().Seed().ImportSeed(
        words_,
        phrase_,
        ot::crypto::SeedStyle::BIP39,
        ot::crypto::Language::en,
        reason_);

    parameters.SetSeed(seed);
    parameters.SetNym(nym_);
    source_.reset(ot::Factory::NymIDSource(api_, parameters, reason_));

    const auto path = ot::UnallocatedVector<ot::Bip32Index>{
        ot::HDIndex{ot::Bip43Purpose::NYM, ot::Bip32Child::HARDENED},
        ot::HDIndex{nym_, ot::Bip32Child::HARDENED},
        ot::HDIndex{0, ot::Bip32Child::HARDENED},
        ot::HDIndex{0, ot::Bip32Child::HARDENED},
        ot::HDIndex{ot::Bip32Child::SIGN_KEY, ot::Bip32Child::HARDENED}};

    opentxs::Bip32Index idx{0};

    const auto& derivedKey = api_.Crypto().BIP32().DeriveKey(
        ot::crypto::EcdsaCurve::secp256k1,
        api_.Crypto().Seed().GetSeed(parameters.Seed(), idx, reason_),
        path);
    const auto& pubkey = std::get<2>(derivedKey);

    opentxs::proto::Credential credential;
    credential.set_version(version_);
    serialize_identifier_to_pb(masterId, *credential.mutable_id());
    credential.set_type(ot::proto::CREDTYPE_HD);
    credential.set_role(ot::proto::CREDROLE_MASTERKEY);
    credential.set_mode(ot::proto::KEYMODE_PUBLIC);
    serialize_identifier_to_pb(nymId, *credential.mutable_nymid());
    credential.mutable_publiccredential()->set_version(version_);
    credential.mutable_publiccredential()->set_mode(ot::proto::KEYMODE_PUBLIC);

    for (int i = 1; i < 4; ++i) {
        auto* key = credential.mutable_publiccredential()->add_key();
        key->set_version(version_);
        key->set_mode(ot::proto::KEYMODE_PUBLIC);
        key->set_type(ot::proto::AKEYTYPE_SECP256K1);
        key->set_role(static_cast<ot::proto::KeyRole>(i));
        *key->mutable_key() = pubkey.Bytes();
    }
    auto* masterData = credential.mutable_masterdata();
    masterData->set_version(version_);
    masterData->mutable_sourceproof()->set_version(version_);
    masterData->mutable_sourceproof()->set_type(
        ot::proto::SOURCEPROOFTYPE_SELF_SIGNATURE);

    auto* source = masterData->mutable_source();
    source->set_version(version_);
    source->set_type(ot::proto::SOURCETYPE_BIP47);
    source->mutable_paymentcode()->set_version(version_);
    *source->mutable_paymentcode()->mutable_key() =
        "SOME CHAINCODE BIGGER THAN 20 CHARS";

    *source->mutable_paymentcode()->mutable_chaincode() =
        "SOME CHAINCODE BIGGER THAN 20 CHARS";

    auto* childCredentialParameters = credential.mutable_childdata();
    childCredentialParameters->set_version(version_);
    serialize_identifier_to_pb(
        masterId, *childCredentialParameters->mutable_masterid());
    opentxs::proto::Signature sourceSignature;
    sourceSignature.set_version(version_);

    source_->Internal().Verify(credential, sourceSignature);
}

/////////////// NO THROW METHODS ////////////////
TEST_F(Test_Source, seedBIP39SourceBip47_NoThrowMethodChecks)
{
    setupSourceForBip47(ot::crypto::SeedStyle::BIP39);
    EXPECT_NO_THROW(source_->Internal().asString());
    EXPECT_NO_THROW(source_->Internal().Description());
    EXPECT_NO_THROW(source_->NymID());
    EXPECT_EQ(source_->Type(), ot::identity::SourceType::Bip47);

    setupSourceForPubKey(ot::crypto::SeedStyle::BIP39);
    EXPECT_NO_THROW(source_->Internal().asString());
    EXPECT_NO_THROW(source_->Internal().Description());
    EXPECT_NO_THROW(source_->NymID());
    EXPECT_EQ(source_->Type(), ot::identity::SourceType::PubKey);
}

}  // namespace ottest
