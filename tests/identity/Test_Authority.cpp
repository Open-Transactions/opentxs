// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <iostream>
#include <memory>


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

using namespace testing;

namespace opentxs
{

    class Test_Authority : public ::testing::Test
    {
    public:
        const api::session::Client& client_;
        const OTPasswordPrompt reason_;
        OTSecret words_;
        OTSecret phrase_;
        const std::uint8_t version_ = 1;
        const int nym_ = 1;
        const identity::SourceMock sourceMock_;
        const identity::NymMock nymMock_ = identity::NymMock();
        crypto::key::KeypairMock keypairMock_;
        const identity::credential::PrimaryMock credentialMock;

        std::unique_ptr<identity::internal::Authority> authority_;

        Test_Authority()
                : client_(Context().StartClientSession(0))
                , reason_(client_.Factory().PasswordPrompt(__func__))
                , words_(client_.Factory().SecretFromText(
                        ottest::GetVectors3().alice_.words_))
                , phrase_(client_.Factory().SecretFromText(
                        ottest::GetVectors3().alice_.words_))
        {
        }
    };

/////////////// Constructors ////////////////

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

}  // namespace opentxs