// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <iostream>
#include <memory>

#include "internal/api/crypto/Seed.hpp"
#include "opentxs/api/crypto/Seed.hpp"

#include "../tests/mocks/crypto/key/KeypairMock.hpp"
#include "../tests/mocks/identity/NymMock.hpp"
#include "../tests/mocks/identity/SourceMock.hpp"
#include "../tests/mocks/identity/credential/PrimaryMock.hpp"
#include "2_Factory.hpp"
#include "identity/credential/Primary.hpp"
#include "internal/identity/Identity.hpp"
#include "internal/otx/common/crypto/Signature.hpp"
#include "internal/otx/common/util/Tag.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Language.hpp"
#include "opentxs/crypto/ParameterType.hpp"
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
#include "opentxs/api/Context.hpp"
#include "opentxs/crypto/Parameters.hpp"
#include "../core/ClientMock.hpp"
#include "../core/StorageMock.hpp"
#include "../core/CryptoMock.hpp"
#include "../core/EncodeMock.hpp"
#include "../core/FactoryMock.hpp"
#include "opentxs/core/Contact.hpp"
using namespace testing;

namespace opentxs
{

    class Test_Authority : public ::testing::Test
    {
    public:
       const api::session::Client& client_;
        const OTPasswordPrompt reason_;
       // OTSecret words_;
      //  OTSecret phrase_;
        const std::uint8_t version_ = 6;
        const int nym_ = 1;
        const identity::SourceMock sourceMock_;
       const identity::NymMock nymMock_;
        crypto::key::KeypairMock keypairMock_;
        crypto::Parameters parameters_;
        std::unique_ptr<identity::Source> source_;
        ot::OTNymID* nymID;
        std::unique_ptr<ot::identity::internal::Nym> internalNym_;
        //   const identity::credential::PrimaryMock credentialMock;
        const ot::UnallocatedCString alias = ot::UnallocatedCString{"alias"};

        std::unique_ptr<identity::internal::Authority> authority_;


        /*StorageMock storage_;
        opentxs::CryptoMock crypto_;
        opentxs::EncodeMock encode_;
        opentxs::FactoryMock factory_;*/
        //opentxs::ClientMock client_;

        Test_Authority()
                :
                client_(Context().StartClientSession(0))
               // client_(storage_)
                , reason_(client_.Factory().PasswordPrompt(__func__))
              //  , words_(client_.Factory().SecretFromText(
             //           ottest::GetVectors3().alice_.words_))
            //    , phrase_(client_.Factory().SecretFromText(
               //         ottest::GetVectors3().alice_.words_))
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

            if (fingerprint.empty()) {

                seeds.GetOrCreateDefaultSeed(
                        fingerprint, style, lang, nymIndex, parameters_.SeedStrength(), reason_);
            }

            auto seed = seeds.GetSeed(fingerprint, nymIndex, reason_);
            const auto defaultIndex = parameters_.UseAutoIndex();

            if (false == defaultIndex) {

                nymIndex = parameters_.Nym();
            }

            static constexpr auto maxIndex =
                    std::numeric_limits<std::int32_t>::max() - 1;

            if (nymIndex >= static_cast<Bip32Index>(maxIndex)) {
                throw std::runtime_error(
                        "Requested seed has already generated maximum number of nyms");
            }

            const auto newIndex = static_cast<std::int32_t>(nymIndex) + 1;
            seeds.UpdateIndex(fingerprint, newIndex, reason_);
            parameters_.SetEntropy(seed);
            parameters_.SetSeed(fingerprint);
            parameters_.SetNym(nymIndex);

            source_.reset(Factory::NymIDSource(client_, parameters_, reason_));


            internalNym_.reset(ot::Factory::Nym(
                    client_, parameters_, ot::identity::Type::individual, alias, reason_));
            //proto::KeyMode mode = proto::KeyMode::KEYMODE_PUBLIC;
            //proto::Authority serialized;


            auto& nym = *internalNym_;
            nym.SetAlias(alias);
            const auto id = ot::OTNymID{nym.ID()};

            EXPECT_TRUE(nym.VerifyPseudonym());

            {
                auto bytes = ot::Space{};
                EXPECT_TRUE(nym.SerializeCredentialIndex(
                        ot::writer(bytes),
                        ot::identity::internal::Nym::Mode::Abbreviated));

                EXPECT_TRUE(client_.Storage().Store(ot::reader(bytes), nym.Alias()));
            }

            {
                const auto nymList = client_.Storage().NymList();

                EXPECT_EQ(1, nymList.size());


                const auto& item = *nymList.begin();

                EXPECT_EQ(item.first, id->str());
                EXPECT_EQ(item.second, alias);
            }

            {
                auto bytes = ot::Space{};

                EXPECT_TRUE(client_.Storage().LoadNym(id, ot::writer(bytes)));
                EXPECT_TRUE(ot::valid(ot::reader(bytes)));

                internalNym_.reset(ot::Factory::Nym(client_, ot::reader(bytes), alias));


                const auto& loadedNym = *internalNym_;

                EXPECT_TRUE(loadedNym.CompareID(id));
                EXPECT_TRUE(loadedNym.VerifyPseudonym());
            }

           authority_.reset(Factory().Authority(client_, nymMock_, *source_,
                                                       parameters_,
                                                       version_,
                                                       reason_));
        }
    };

/////////////// Constructors ////////////////
    /*TEST_F(Test_Authority, Constructor_WithReason_ShouldNotThrow42)
    {
        EXPECT_EQ(authority_->ContactCredentialVersion(), version_);
        auto p = authority_->EncryptionTargets();
        proto::ContactData contactData;
        authority_->GetContactData(contactData);
        //auto master = authority_->GetMasterCredential();
        auto getMasterCredID = authority_->GetMasterCredID();

    }

    TEST_F(Test_Authority, Constructor_WithReason_ShouldNotThrow422)
    {
        proto::ContactData contactData;
        EXPECT_FALSE(authority_->GetContactData(contactData));

        contactData.set_version(version_);
        //contactData.mutable_section().set
        //authority_->AddContactCredential(contactData, reason_);

        //EXPECT_TRUE(authority_->GetContactData(contactData));
       // EXPECT_EQ(contactData.version(), version_);

    }*/

    TEST_F(Test_Authority, Constructor_WithReason_ShouldNotThrow4221)
    {
        auto& nym = *internalNym_;
        nym.SetAlias(alias);
        const auto id = ot::OTNymID{nym.ID()};

        EXPECT_CALL(nymMock_, Source()).WillRepeatedly(ReturnRef(sourceMock_));
        EXPECT_CALL(sourceMock_, NymID()).WillRepeatedly(Return(id));

        std::cerr << "1: " << authority_->AddChildKeyCredential(parameters_, reason_);

        std::cerr << "2\n : "<< authority_->GetPublicAuthKey(crypto::key::asymmetric::Algorithm::Secp256k1);

        proto::VerificationSet verificationSet;
        verificationSet.set_version(1);

        std::cerr << "1: " << authority_->AddVerificationCredential(verificationSet, reason_);
        std::cerr << "1: " << authority_->GetVerificationSet(verificationSet);

        proto::ContactData contactData;
        contactData.set_version(version_);
        std::cerr << "1: " << authority_->AddContactCredential(contactData, reason_);
        std::cerr << "1: " << authority_->GetContactData(contactData);
        UnallocatedList<UnallocatedCString> list;
        authority_->RevokeContactCredentials(list);

        authority_->RevokeVerificationCredentials(list);
        authority_->WriteCredentials();

        EXPECT_EQ(authority_->ContactCredentialVersion(), version_);
        auto ppp = authority_->EncryptionTargets();
        (void)ppp;
       // identity::credential::internal::Primary p123 = authority_->GetMasterCredential();
        const auto xxx = authority_->GetMasterCredID();


        authority_->GetPublicEncrKey(crypto::key::asymmetric::Algorithm::Secp256k1);
        crypto::key::Keypair::Keys a;


       /* opentxs::Signature signature;
        authority_->GetPublicKeysBySignature(a, signature);*/


        std::cerr << "1: " <<authority_->GetPublicSignKey(crypto::key::asymmetric::Algorithm::Secp256k1);
        std::cerr << "1: " << authority_->GetPrivateSignKey(crypto::key::asymmetric::Algorithm::Secp256k1);
        std::cerr << "1: " << authority_->GetPrivateEncrKey(crypto::key::asymmetric::Algorithm::Secp256k1);
        std::cerr << "1: " << authority_->GetPrivateAuthKey(crypto::key::asymmetric::Algorithm::Secp256k1);
        std::cerr << "1: " << authority_->GetAuthKeypair(crypto::key::asymmetric::Algorithm::Secp256k1);

        std::cerr << "1: " << authority_->GetEncrKeypair(crypto::key::asymmetric::Algorithm::Secp256k1);

        std::cerr << "1: " << authority_->GetSignKeypair(crypto::key::asymmetric::Algorithm::Secp256k1);

        authority_->GetTagCredential(crypto::key::asymmetric::Algorithm::Secp256k1);


        proto::VerificationSet verificationSet2;

        std::cerr << "1: " << authority_->GetVerificationSet(verificationSet2);

        NymCapability nymC;
        std::cerr << "1: " << authority_->hasCapability(nymC);

        std::cerr << "1: " << authority_->Params(crypto::key::asymmetric::Algorithm::Secp256k1);

        proto::HDPath output;
        std::cerr << "1: " << authority_->Path(output);


        /*
         *    proto::Authority serialized;
        CredentialIndexModeFlag mode = true;
        std::cerr << "1: " << authority_->Serialize(serialized, mode);
         *
         * std::function<UnallocatedCString()> fc;
        crypto::SignatureRole role;
        proto::Signature signature;
        std::cerr << "1: " << authority_->Sign(fc, role, signature, );*/

    }
/*
    TEST_F(Test_Authority, Constructor_WithReason_ShouldNotThrow4)
    {


        const auto reason = client_.Factory().PasswordPrompt(__func__);
        const auto alias = ot::UnallocatedCString{"alias"};
        std::unique_ptr<ot::identity::internal::Nym> pNym(ot::Factory::Nym(
                client_, {}, ot::identity::Type::individual, alias, reason));
        //proto::KeyMode mode = proto::KeyMode::KEYMODE_PUBLIC;
        //proto::Authority serialized;

        crypto::Parameters parameters;
        const auto& seeds = client_.Crypto().Seed().Internal();
        parameters.SetCredset(0);
        auto nymIndex = Bip32Index{0};
        auto fingerprint = parameters.Seed();
        auto style = parameters.SeedStyle();
        auto lang = parameters.SeedLanguage();

        if (fingerprint.empty()) {
            seeds.GetOrCreateDefaultSeed(
                    fingerprint, style, lang, nymIndex, parameters.SeedStrength(), reason);
        }

        auto seed = seeds.GetSeed(fingerprint, nymIndex, reason);
        const auto defaultIndex = parameters.UseAutoIndex();

        if (false == defaultIndex) {

            nymIndex = parameters.Nym();
        }

        static constexpr auto maxIndex =
                std::numeric_limits<std::int32_t>::max() - 1;

        if (nymIndex >= static_cast<Bip32Index>(maxIndex)) {
            throw std::runtime_error(
                    "Requested seed has already generated maximum number of nyms");
        }

        const auto newIndex = static_cast<std::int32_t>(nymIndex) + 1;
        seeds.UpdateIndex(fingerprint, newIndex, reason);
        parameters.SetEntropy(seed);
        parameters.SetSeed(fingerprint);
        parameters.SetNym(nymIndex);

        auto pSource = std::unique_ptr<identity::Source>{
                Factory::NymIDSource(client_, parameters, reason)};


        EXPECT_TRUE(nullptr != Factory().Authority(client_, *pNym.get(), *pSource,
                                                   parameters,
                                                   version_,
                                                   reason_));
    }
    */
}  // namespace opentxs

/*
 *
 *         auto nym = identifier::Nym::Factory("ot2xuVPJDdweZvKLQD42UMCzhCmT3okn3W1PktLgCbmQLRnaKy848sX");
        EXPECT_CALL(sourceMock_, NymID())
                .WillOnce(Invoke([&]() -> OTNymID {
                    return nym;
                }));

        EXPECT_CALL(sourceMock_, Serialize(_))
                .WillRepeatedly(Return(true));

        EXPECT_CALL(sourceMock_, Sign(_,_,_ ))
                .WillRepeatedly(Return(true));
 *
 *
TEST_F(Test_Authority, Constructor_WithReason_ShouldNotThrow4)
{
    const auto alias = UnallocatedCString{"alias"};
    std::unique_ptr<identity::internal::Nym> pNym(
            Factory::Nym(client_, {}, identity::Type::individual, alias, reason_));
    auto& nym = *pNym;
    nym.SetAlias(alias);
    const auto id = OTNymID{nym.ID()};

    //const auto nym = identifier::Nym::Factory();

    EXPECT_CALL(sourceMock_, NymID())
            .WillOnce(Invoke([&]() -> OTNymID {
                return id;
            }));

    EXPECT_CALL(sourceMock_, Serialize(_))
            .WillRepeatedly(Return(true));

    EXPECT_CALL(sourceMock_, Sign(_,_,_ ))
            .WillRepeatedly(Return(true));

    auto seed = client_.Crypto().Seed().ImportSeed(
            words_,
            phrase_,
            ot::crypto::SeedStyle::BIP39,
            ot::crypto::Language::en,
            reason_);

    proto::NymIDSource serialized;
    serialized.set_type(proto::SOURCETYPE_PUBKEY);
    serialized.set_version(version_);

    crypto::Parameters parameters{
            crypto::Parameters::DefaultType(),
            crypto::Parameters::DefaultCredential(),
            identity::SourceType::Bip47,
            version_};
    parameters.SetSeed(seed);
    parameters.SetNym(nym_);

    EXPECT_TRUE(
            nullptr != Factory().Authority(
                    client_,
                    *pNym.get(),
                    sourceMock_,
                    parameters,
                    version_,
                    reason_));
}
*/