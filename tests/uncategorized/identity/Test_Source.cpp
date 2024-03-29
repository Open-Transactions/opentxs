// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <opentxs/protobuf/AsymmetricKey.pb.h>
#include <opentxs/protobuf/ChildCredentialParameters.pb.h>
#include <opentxs/protobuf/Credential.pb.h>
#include <opentxs/protobuf/Enums.pb.h>
#include <opentxs/protobuf/KeyCredential.pb.h>
#include <opentxs/protobuf/MasterCredentialParameters.pb.h>
#include <opentxs/protobuf/NymIDSource.pb.h>  // IWYU pragma: keep
#include <opentxs/protobuf/PaymentCode.pb.h>
#include <opentxs/protobuf/Signature.pb.h>
#include <opentxs/protobuf/SourceProof.pb.h>
#include <memory>
#include <stdexcept>
#include <tuple>

#include "internal/identity/Authority.hpp"
#include "internal/identity/Source.hpp"
#include "opentxs/internal.factory.hpp"
#include "ottest/fixtures/core/Identifier.hpp"
#include "ottest/fixtures/identity/Source.hpp"
#include "ottest/mocks/identity/credential/Primary.hpp"
#include "util/HDIndex.hpp"

namespace ot = opentxs;

namespace ottest
{
/////////////// Constructors ////////////////
TEST_F(Source, Constructor_WithProtoOfTypeBIP47_ShouldNotThrow)
{
    opentxs::protobuf::NymIDSource nymIdProto;
    nymIdProto.set_type(ot::protobuf::SOURCETYPE_BIP47);
    *nymIdProto.mutable_paymentcode()->mutable_chaincode() = "test";
    source_.reset(ot::Factory::NymIDSource(api_, nymIdProto));
    EXPECT_NE(source_, nullptr);
}

TEST_F(Source, Constructor_WithProtoOfTypePUBKEY_ShouldNotThrow)
{
    opentxs::protobuf::NymIDSource nymIdProto;
    nymIdProto.set_type(ot::protobuf::SOURCETYPE_PUBKEY);

    source_.reset(ot::Factory::NymIDSource(api_, nymIdProto));
    EXPECT_NE(source_, nullptr);
}

TEST_F(Source, Constructor_WithCredentialTypeError_ShouldThrow)
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
    Source,
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
    Source,
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

TEST_F(Source, Constructor_WithParametersSourceTypeError_ShouldReturnNullptr)
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
TEST_F(Source, Serialize_seedBIP39SourceBip47_ShouldSetProperFields)
{
    setupSourceForBip47(ot::crypto::SeedStyle::BIP39);
    opentxs::protobuf::NymIDSource nymIdProto;

    EXPECT_TRUE(source_->Internal().Serialize(nymIdProto));
    EXPECT_EQ(nymIdProto.version(), version_);
    EXPECT_EQ(nymIdProto.type(), ot::protobuf::SourceType::SOURCETYPE_BIP47);
    EXPECT_FALSE(nymIdProto.paymentcode().key().empty());
    EXPECT_FALSE(nymIdProto.paymentcode().chaincode().empty());
    EXPECT_EQ(nymIdProto.paymentcode().version(), version_);
}

TEST_F(Source, Serialize_seedBIP39SourcePubKey_ShouldSetProperFields)
{
    setupSourceForPubKey(ot::crypto::SeedStyle::BIP39);
    opentxs::protobuf::NymIDSource nymIdProto;
    EXPECT_TRUE(source_->Internal().Serialize(nymIdProto));
    EXPECT_EQ(
        nymIdProto.version(),
        opentxs::crypto::asymmetric::Key::DefaultVersion());

    EXPECT_EQ(nymIdProto.key().role(), ot::protobuf::KEYROLE_SIGN);
    EXPECT_EQ(nymIdProto.type(), ot::protobuf::SourceType::SOURCETYPE_PUBKEY);
    EXPECT_TRUE(nymIdProto.paymentcode().key().empty());
    EXPECT_TRUE(nymIdProto.paymentcode().chaincode().empty());
}

/////////////// SIGN ////////////////
TEST_F(Source, Sign_ShouldReturnTrue)
{
    setupSourceForBip47(ot::crypto::SeedStyle::BIP39);
    const ot::identity::credential::PrimaryMock credentialMock;
    ot::protobuf::Signature sig;
    Authority();
    EXPECT_CALL(credentialMock, Internal())
        .WillOnce(
            ::testing::ReturnRef(authority_->GetMasterCredential().Internal()));
    EXPECT_TRUE(source_->Internal().Sign(credentialMock, sig, reason_));
}

/////////////// VERIFY ////////////////
TEST_F(Source, Verify_seedPubKeySourceBip47_ShouldReturnTrue)
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

    const auto path = ot::UnallocatedVector<ot::crypto::Bip32Index>{
        ot::HDIndex{
            ot::crypto::Bip43Purpose::NYM, ot::crypto::Bip32Child::HARDENED},
        ot::HDIndex{nym_, ot::crypto::Bip32Child::HARDENED},
        ot::HDIndex{0, ot::crypto::Bip32Child::HARDENED},
        ot::HDIndex{0, ot::crypto::Bip32Child::HARDENED},
        ot::HDIndex{
            ot::crypto::Bip32Child::SIGN_KEY,
            ot::crypto::Bip32Child::HARDENED}};

    opentxs::Bip32Index idx{0};

    const auto& derivedKey = api_.Crypto().BIP32().DeriveKey(
        ot::crypto::EcdsaCurve::secp256k1,
        api_.Crypto().Seed().GetSeed(parameters.Seed(), idx, reason_),
        path);
    const auto& pubkey = std::get<2>(derivedKey);

    opentxs::protobuf::Credential credential;
    credential.set_version(version_);
    serialize_identifier_to_pb(masterId, *credential.mutable_id());
    credential.set_type(ot::protobuf::CREDTYPE_HD);
    credential.set_role(ot::protobuf::CREDROLE_MASTERKEY);
    credential.set_mode(ot::protobuf::KEYMODE_PUBLIC);
    serialize_identifier_to_pb(nymId, *credential.mutable_nymid());
    credential.mutable_publiccredential()->set_version(version_);
    credential.mutable_publiccredential()->set_mode(
        ot::protobuf::KEYMODE_PUBLIC);

    for (int i = 1; i < 4; ++i) {
        auto* key = credential.mutable_publiccredential()->add_key();
        key->set_version(version_);
        key->set_mode(ot::protobuf::KEYMODE_PUBLIC);
        key->set_type(ot::protobuf::AKEYTYPE_SECP256K1);
        key->set_role(static_cast<ot::protobuf::KeyRole>(i));
        *key->mutable_key() = pubkey.Bytes();
    }
    auto* masterData = credential.mutable_masterdata();
    masterData->set_version(version_);
    masterData->mutable_sourceproof()->set_version(version_);
    masterData->mutable_sourceproof()->set_type(
        ot::protobuf::SOURCEPROOFTYPE_SELF_SIGNATURE);

    auto* source = masterData->mutable_source();
    source->set_version(version_);
    source->set_type(ot::protobuf::SOURCETYPE_BIP47);
    source->mutable_paymentcode()->set_version(version_);
    *source->mutable_paymentcode()->mutable_key() =
        "SOME CHAINCODE BIGGER THAN 20 CHARS";

    *source->mutable_paymentcode()->mutable_chaincode() =
        "SOME CHAINCODE BIGGER THAN 20 CHARS";

    auto* childCredentialParameters = credential.mutable_childdata();
    childCredentialParameters->set_version(version_);
    serialize_identifier_to_pb(
        masterId, *childCredentialParameters->mutable_masterid());
    opentxs::protobuf::Signature sourceSignature;
    sourceSignature.set_version(version_);

    source_->Internal().Verify(credential, sourceSignature);
}

/////////////// NO THROW METHODS ////////////////
TEST_F(Source, seedBIP39SourceBip47_NoThrowMethodChecks)
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
