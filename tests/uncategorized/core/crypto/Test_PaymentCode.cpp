// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>

#include "internal/api/session/Client.hpp"
#include "internal/otx/client/obsolete/OTAPI_Exec.hpp"

namespace ot = opentxs;

namespace ottest
{
class Test_PaymentCode : public ::testing::Test
{
public:
    static const bool have_hd_;

    const ot::api::session::Client& client_;
    ot::PasswordPrompt reason_;
    ot::UnallocatedCString seed_, fingerprint_, nym_id_0_, paycode_0_,
        nym_id_1_, paycode_1_, nym_id_2_, paycode_2_, nym_id_3_, paycode_3_;
    ot::NymData nym_data_0_, nym_data_1_, nym_data_2_, nym_data_3_;
    ot::UnitType currency_ = ot::UnitType::Bch;
    ot::UnitType currency_2_ = ot::UnitType::Btc;

    /* Is evaluated every test, therefore indexes are fixed to 0,1,2,3 */
    Test_PaymentCode()
        : client_(ot::Context().StartClientSession(0))
        , reason_(client_.Factory().PasswordPrompt(__func__))
        , seed_("trim thunder unveil reduce crop cradle zone inquiry anchor "
                "skate property fringe obey butter text tank drama palm guilt "
                "pudding laundry stay axis prosper")
        , fingerprint_(
              client_.InternalClient().Exec().Wallet_ImportSeed(seed_, ""))
        , nym_id_0_(client_.Wallet()
                        .Nym({fingerprint_, 0, 1}, reason_, "PaycodeNym")
                        ->ID()
                        .asBase58(client_.Crypto()))
        , paycode_0_(
              "PM8TJhB2CxWDqR8c5y4kWoJwSGRNYaVATdJM85kqfn2dZ9TdSihbFJraQzjYUMYx"
              "bsrnMfjPK6oZFAPQ1tWqzwTfKbtunvLFCzDJFVXVGbUAKxhsz7P5")
        , nym_id_1_(client_.Wallet()
                        .Nym({fingerprint_, 1, 1}, reason_, "PaycodeNym_1")
                        ->ID()
                        .asBase58(client_.Crypto()))
        , paycode_1_(
              "PM8TJWedQTvxaoJpt9Wh25HR54oj5vmor6arAByFk4UTgUh1Tna2srsZLUo2xS3V"
              "iBot1ftf4p8ZUN8khB2zvViHXZkrwkfjcePSeEgsYapESKywge9F")
        , nym_id_2_(client_.Wallet()
                        .Nym({fingerprint_, 2, 1}, reason_, "PaycodeNym_2")
                        ->ID()
                        .asBase58(client_.Crypto()))
        , paycode_2_(
              "PM8TJQmrQ4tSY6Gad59UpzqR8MRMesSYMKXvpMuzdDHByfRXVgvVdiqD5NmjoEH9"
              "V6ZrofFVViBwSg9dvVcP8R2CU1pXejhVQQj3XsWk8sLhAsspqk8F")
        , nym_id_3_(client_.Wallet()
                        .Nym({fingerprint_, 3, 1}, reason_, "PaycodeNym_3")
                        ->ID()
                        .asBase58(client_.Crypto()))
        , paycode_3_(
              "PM8TJbNzqDcdqCcpkMLLa9H83CjoWdHMTQ4Lk11qSpThkyrmDFA4AeGd2kFeLK2s"
              "T6UVXy2jwWABsfLd7JmcS4hMAy9zUdWRFRhmu33RiRJCS6qRmGew")
        , nym_data_0_(client_.Wallet().mutable_Nym(
              client_.Factory().NymIDFromBase58(nym_id_0_),
              reason_))
        , nym_data_1_(client_.Wallet().mutable_Nym(
              client_.Factory().NymIDFromBase58(nym_id_1_),
              reason_))
        , nym_data_2_(client_.Wallet().mutable_Nym(
              client_.Factory().NymIDFromBase58(nym_id_2_),
              reason_))
        , nym_data_3_(client_.Wallet().mutable_Nym(
              client_.Factory().NymIDFromBase58(nym_id_3_),
              reason_))
    {
        nym_data_0_.AddPaymentCode(paycode_0_, currency_, true, true, reason_);

        nym_data_1_.AddPaymentCode(paycode_0_, currency_, true, true, reason_);
        nym_data_1_.AddPaymentCode(
            paycode_1_, currency_, true, false, reason_);  // reset nymdata_1 to
                                                           // paymentcode_1

        nym_data_2_.AddPaymentCode(
            paycode_2_, currency_2_, false, true, reason_);  // nymdata_2 resets
                                                             // paycode_2 to
                                                             // primary
        nym_data_2_.AddPaymentCode(
            paycode_0_, currency_2_, false, true, reason_);  // fail to reset
                                                             // nymdata_2 with
                                                             // primary = false

        nym_data_3_.AddPaymentCode(
            paycode_3_, currency_2_, false, false, reason_);  // nymdata_3
                                                              // resets
                                                              // paycode_3 to
                                                              // primary
        nym_data_3_.AddPaymentCode(
            paycode_0_, currency_2_, false, false, reason_);  // fail to be
                                                              // primary
    }
};

const bool Test_PaymentCode::have_hd_{
    ot::api::crypto::HaveHDKeys() &&
    ot::api::crypto::HaveSupport(
        ot::crypto::key::asymmetric::Algorithm::Secp256k1)

};
/* Test: Gets the last paymentcode to be set as primary
 */
TEST_F(Test_PaymentCode, primary_paycodes)
{
    EXPECT_STREQ(
        paycode_0_.c_str(),
        nym_data_0_.PaymentCode(currency_).c_str());  // primary and active
    EXPECT_STREQ(
        paycode_1_.c_str(),
        nym_data_1_.PaymentCode(currency_).c_str());  // primary but inactive
                                                      // overrides primary
                                                      // active
    EXPECT_STREQ(
        paycode_2_.c_str(),
        nym_data_2_.PaymentCode(currency_2_).c_str());  // not primary but
                                                        // active defaults to
                                                        // primary
    EXPECT_STREQ(
        paycode_3_.c_str(),
        nym_data_3_.PaymentCode(currency_2_).c_str());  // not primary nor
                                                        // active defaults to
                                                        // primary

    auto nym0 =
        client_.Wallet().Nym(client_.Factory().NymIDFromBase58(nym_id_0_));
    auto nym1 =
        client_.Wallet().Nym(client_.Factory().NymIDFromBase58(nym_id_1_));
    auto nym2 =
        client_.Wallet().Nym(client_.Factory().NymIDFromBase58(nym_id_2_));
    auto nym3 =
        client_.Wallet().Nym(client_.Factory().NymIDFromBase58(nym_id_3_));

    if (have_hd_) {
        EXPECT_STREQ(nym0->PaymentCode().c_str(), paycode_0_.c_str());
        EXPECT_STREQ(nym1->PaymentCode().c_str(), paycode_1_.c_str());
        EXPECT_STREQ(nym2->PaymentCode().c_str(), paycode_2_.c_str());
        EXPECT_STREQ(nym3->PaymentCode().c_str(), paycode_3_.c_str());
    } else {
        // TODO
    }
}

/* Test: by setting primary = true it resets best payment code
 */
TEST_F(Test_PaymentCode, test_new_primary)
{
    EXPECT_STREQ(
        paycode_0_.c_str(), nym_data_0_.PaymentCode(currency_).c_str());

    ASSERT_TRUE(
        nym_data_0_.AddPaymentCode(paycode_2_, currency_, true, true, reason_));
    EXPECT_STREQ(
        paycode_2_.c_str(), nym_data_0_.PaymentCode(currency_).c_str());

    ASSERT_TRUE(nym_data_0_.AddPaymentCode(
        paycode_3_, currency_, true, false, reason_));
    EXPECT_STREQ(
        paycode_3_.c_str(), nym_data_0_.PaymentCode(currency_).c_str());
}

/* Test: by setting primary = false it should not override a previous primary
 */
TEST_F(Test_PaymentCode, test_secondary_doesnt_replace)
{
    EXPECT_STREQ(
        paycode_0_.c_str(), nym_data_0_.PaymentCode(currency_).c_str());

    ASSERT_TRUE(nym_data_0_.AddPaymentCode(
        paycode_2_, currency_, false, false, reason_));
    ASSERT_TRUE(nym_data_0_.AddPaymentCode(
        paycode_3_, currency_, false, true, reason_));

    EXPECT_STREQ(
        paycode_0_.c_str(), nym_data_0_.PaymentCode(currency_).c_str());
}

/* Test: Valid paycodes
 */
TEST_F(Test_PaymentCode, valid_paycodes)
{
    ASSERT_TRUE(client_.Factory().PaymentCode(paycode_0_).Valid());
    ASSERT_TRUE(client_.Factory().PaymentCode(paycode_1_).Valid());
    ASSERT_TRUE(client_.Factory().PaymentCode(paycode_2_).Valid());
    ASSERT_TRUE(client_.Factory().PaymentCode(paycode_3_).Valid());
}

/* Test: Invalid paycodes should not be saved
 */
TEST_F(Test_PaymentCode, empty_paycode)
{
    EXPECT_STREQ(
        paycode_0_.c_str(), nym_data_0_.PaymentCode(currency_).c_str());

    ASSERT_FALSE(
        client_.Factory().PaymentCode(ot::UnallocatedCString{}).Valid());
    bool added = nym_data_0_.AddPaymentCode("", currency_, true, true, reason_);
    ASSERT_FALSE(added);

    EXPECT_STREQ(
        paycode_0_.c_str(), nym_data_0_.PaymentCode(currency_).c_str());

    ot::UnallocatedCString invalid_paycode =
        "XM8TJS2JxQ5ztXUpBBRnpTbcUXbUHy2T1abfrb3KkAAtMEGNbey4oumH7Hc578WgQJhPjB"
        "xteQ5GHHToTYHE3A1w6p7tU6KSoFmWBVbFGjKPisZDbP97";
    ASSERT_FALSE(client_.Factory().PaymentCode(invalid_paycode).Valid());

    added = nym_data_0_.AddPaymentCode(
        invalid_paycode, currency_, true, true, reason_);
    ASSERT_FALSE(added);
    ASSERT_STRNE("", nym_data_0_.PaymentCode(currency_).c_str());

    EXPECT_STREQ(
        paycode_0_.c_str(), nym_data_0_.PaymentCode(currency_).c_str());
}

/* Test: Base58 encoding
 */
TEST_F(Test_PaymentCode, asBase58)
{
    auto pcode = client_.Factory().PaymentCode(paycode_0_);
    EXPECT_STREQ(paycode_0_.c_str(), pcode.asBase58().c_str());
}

/* Test: Factory methods create the same paycode
 */
TEST_F(Test_PaymentCode, factory)
{
    // Factory 0: PaymentCode&
    auto factory_0 = client_.Factory().PaymentCode(paycode_0_);

    EXPECT_STREQ(paycode_0_.c_str(), factory_0.asBase58().c_str());

    auto factory_0b = client_.Factory().PaymentCode(paycode_1_);

    EXPECT_STREQ(paycode_1_.c_str(), factory_0b.asBase58().c_str());

    // Factory 1: ot::UnallocatedCString
    auto factory_1 = client_.Factory().PaymentCode(paycode_0_);

    EXPECT_STREQ(paycode_0_.c_str(), factory_1.asBase58().c_str());

    auto factory_1b = client_.Factory().PaymentCode(paycode_1_);

    EXPECT_STREQ(paycode_1_.c_str(), factory_1b.asBase58().c_str());

    // Factory 2: proto::PaymentCode&
    auto bytes = ot::Space{};
    factory_1.Serialize(ot::writer(bytes));
    auto factory_2 = client_.Factory().PaymentCode(ot::reader(bytes));

    EXPECT_STREQ(paycode_0_.c_str(), factory_2.asBase58().c_str());

    factory_1b.Serialize(ot::writer(bytes));
    auto factory_2b = client_.Factory().PaymentCode(ot::reader(bytes));

    EXPECT_STREQ(paycode_1_.c_str(), factory_2b.asBase58().c_str());

    // Factory 3: std:
    const auto nym =
        client_.Wallet().Nym(client_.Factory().NymIDFromBase58(nym_id_0_));
    const auto& fingerprint = nym.get()->PathRoot();
    auto factory_3 = client_.Factory().PaymentCode(
        fingerprint, 0, 1, reason_);  // seed, nym, paycode version
    auto factory_3b = client_.Factory().PaymentCode(
        fingerprint, 1, 1, reason_);  // seed, nym, paycode version

    if (have_hd_) {
        EXPECT_TRUE(nym.get()->HasPath());
        EXPECT_STREQ(paycode_0_.c_str(), factory_3.asBase58().c_str());
        EXPECT_STREQ(paycode_1_.c_str(), factory_3b.asBase58().c_str());
    } else {
        // TODO
    }
}

/* Test: factory method with nym has private key
 */
TEST_F(Test_PaymentCode, factory_seed_nym)
{
    if (have_hd_) {
        ot::UnallocatedCString seed =
            client_.Crypto().Seed().DefaultSeed().first;
        [[maybe_unused]] std::uint32_t nym_idx = 0;
        [[maybe_unused]] std::uint8_t version = 1;
        [[maybe_unused]] bool bitmessage = false;
        [[maybe_unused]] std::uint8_t bitmessage_version = 0;
        [[maybe_unused]] std::uint8_t bitmessage_stream = 0;

        const auto nym =
            client_.Wallet().Nym(client_.Factory().NymIDFromBase58(nym_id_0_));

        EXPECT_TRUE(nym.get()->HasPath());

        auto fingerprint{nym.get()->PathRoot()};
        auto privatekey = client_.Crypto().Seed().GetPaymentCode(
            fingerprint, 10, version, reason_);

        ASSERT_TRUE(privatekey);
    } else {
        // TODO
    }
}

TEST_F(Test_PaymentCode, nymid)
{
    EXPECT_EQ(
        client_.Factory().PaymentCode(paycode_0_).ID(),
        client_.Factory().NymIDFromPaymentCode(paycode_0_));
    EXPECT_EQ(
        client_.Factory().PaymentCode(paycode_1_).ID(),
        client_.Factory().NymIDFromPaymentCode(paycode_1_));
    EXPECT_EQ(
        client_.Factory().PaymentCode(paycode_2_).ID(),
        client_.Factory().NymIDFromPaymentCode(paycode_2_));
    EXPECT_EQ(
        client_.Factory().PaymentCode(paycode_3_).ID(),
        client_.Factory().NymIDFromPaymentCode(paycode_3_));
}
}  // namespace ottest
