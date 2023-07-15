// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <memory>
#include <optional>

#include "ottest/data/crypto/PaymentCodeV3.hpp"
#include "ottest/fixtures/common/User.hpp"
#include "ottest/fixtures/core/contract/PeerRequest.hpp"

namespace ottest
{
TEST_F(PeerRequest, init_opentxs)
{
    SetIntroductionServer(client_1_, notary_);
    SetIntroductionServer(client_2_, notary_);
}

TEST_F(PeerRequest, create_nyms)
{
    const auto seed1 =
        ImportBip39(client_1_, GetPaymentCodeVector3().alice_.words_);
    const auto seed2 =
        ImportBip39(client_2_, GetPaymentCodeVector3().bob_.words_);
    const auto& alex = CreateNym(client_1_, "Alex", seed1, 0);
    const auto& bob = CreateNym(client_2_, "Bob", seed2, 1);
    RegisterNym(notary_, alex);
    RegisterNym(notary_, bob);
    client_1_.OTX().DownloadNym(alex.nym_id_, notary_.ID(), bob.nym_id_);
    client_1_.OTX().ContextIdle(alex.nym_id_, notary_.ID()).get();
}

TEST_F(PeerRequest, request_verification)
{
    const auto& alex = users_.at(0);
    const auto& bob = users_.at(1);
    const auto cb = [&, this](auto&& request) {
        const auto& claim = request.asVerification().Claim();
        const auto reason = client_2_.Factory().PasswordPrompt("");
        using enum opentxs::identity::wot::verification::Type;
        client_2_.OTX().AcknowledgeVerification(
            request.Responder(),
            request.Initiator(),
            request.ID(),
            client_2_.Factory().Verification(
                *bob.nym_, reason, claim.ID(), affirm));
    };
    const auto claim = client_1_.Factory().Claim(
        alex.nym_id_,
        opentxs::identity::wot::claim::SectionType::Identifier,
        opentxs::identity::wot::claim::ClaimType::Swissfortress,
        alex.name_,
        {});
    auto future = InitListener(cb);
    client_1_.OTX().InitiateVerification(alex.nym_id_, bob.nym_id_, claim);
    const auto reply = future.get();

    EXPECT_EQ(reply.Initiator(), alex.nym_id_);
    EXPECT_TRUE(reply.IsValid());
    EXPECT_EQ(reply.Responder(), bob.nym_id_);
    EXPECT_EQ(reply.Type(), opentxs::contract::peer::RequestType::Verification);

    const auto& response = reply.asVerification();

    EXPECT_TRUE(response.Accepted());
    EXPECT_TRUE(response.IsValid());
    ASSERT_TRUE(response.Response().has_value());

    const auto& verification = *response.Response();

    EXPECT_EQ(verification.Claim(), claim.ID());
    EXPECT_TRUE(verification.IsValid());
    EXPECT_EQ(
        verification.Value(),
        opentxs::identity::wot::verification::Type::affirm);

    {
        const auto reason = client_1_.Factory().PasswordPrompt("");
        auto nym = client_1_.Wallet().mutable_Nym(alex.nym_id_, reason);

        EXPECT_TRUE(nym.AddClaim(claim, reason));
        // TODO
        EXPECT_FALSE(nym.AddVerification(verification, reason));
    }

    // TODO check claims and verifications
}

TEST_F(PeerRequest, cleanup)
{
    const auto& alex = users_.at(0);
    const auto& bob = users_.at(1);
    client_1_.OTX().ContextIdle(alex.nym_id_, notary_.ID()).get();
    client_2_.OTX().ContextIdle(bob.nym_id_, notary_.ID()).get();
    CleanupClient();
}
}  // namespace ottest
