// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/core/PaymentCode.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <memory>
#include <string_view>

#include "ottest/env/OTTestEnvironment.hpp"

namespace ottest
{
using namespace std::literals;

const bool PaymentCode::have_hd_{
    ot::api::crypto::HaveHDKeys() &&
    ot::api::crypto::HaveSupport(ot::crypto::asymmetric::Algorithm::Secp256k1)};

/* Is evaluated every test, therefore indexes are fixed to 0,1,2,3 */
PaymentCode::PaymentCode()
    : api_(OTTestEnvironment::GetOT().StartClientSession(0))
    , reason_(api_.Factory().PasswordPrompt(__func__))
    , seed_(
          "trim thunder unveil reduce crop cradle zone inquiry anchor skate property fringe obey butter text tank drama palm guilt pudding laundry stay axis prosper"sv)
    , fingerprint_(api_.Crypto().Seed().ImportSeed(
          api_.Factory().SecretFromText(seed_),
          api_.Factory().SecretFromText(""sv),
          opentxs::crypto::SeedStyle::BIP39,
          opentxs::crypto::Language::en,
          api_.Factory().PasswordPrompt("Importing a BIP-39 seed")))
    , nym_id_0_(
          api_.Wallet()
              .Nym({api_.Factory(), fingerprint_, 0, 1}, reason_, "PaycodeNym")
              ->ID()
              .asBase58(api_.Crypto()))
    , paycode_0_(
          "PM8TJhB2CxWDqR8c5y4kWoJwSGRNYaVATdJM85kqfn2dZ9TdSihbFJraQzjYUMYxbsrnMfjPK6oZFAPQ1tWqzwTfKbtunvLFCzDJFVXVGbUAKxhsz7P5"sv)
    , nym_id_1_(api_.Wallet()
                    .Nym(
                        {api_.Factory(), fingerprint_, 1, 1},
                        reason_,
                        "PaycodeNym_1")
                    ->ID()
                    .asBase58(api_.Crypto()))
    , paycode_1_(
          "PM8TJWedQTvxaoJpt9Wh25HR54oj5vmor6arAByFk4UTgUh1Tna2srsZLUo2xS3ViBot1ftf4p8ZUN8khB2zvViHXZkrwkfjcePSeEgsYapESKywge9F"sv)
    , nym_id_2_(api_.Wallet()
                    .Nym(
                        {api_.Factory(), fingerprint_, 2, 1},
                        reason_,
                        "PaycodeNym_2")
                    ->ID()
                    .asBase58(api_.Crypto()))
    , paycode_2_(
          "PM8TJQmrQ4tSY6Gad59UpzqR8MRMesSYMKXvpMuzdDHByfRXVgvVdiqD5NmjoEH9V6ZrofFVViBwSg9dvVcP8R2CU1pXejhVQQj3XsWk8sLhAsspqk8F"sv)
    , nym_id_3_(api_.Wallet()
                    .Nym(
                        {api_.Factory(), fingerprint_, 3, 1},
                        reason_,
                        "PaycodeNym_3")
                    ->ID()
                    .asBase58(api_.Crypto()))
    , paycode_3_(
          "PM8TJbNzqDcdqCcpkMLLa9H83CjoWdHMTQ4Lk11qSpThkyrmDFA4AeGd2kFeLK2sT6UVXy2jwWABsfLd7JmcS4hMAy9zUdWRFRhmu33RiRJCS6qRmGew"sv)
    , nym_data_0_(api_.Wallet().mutable_Nym(
          api_.Factory().NymIDFromBase58(nym_id_0_),
          reason_))
    , nym_data_1_(api_.Wallet().mutable_Nym(
          api_.Factory().NymIDFromBase58(nym_id_1_),
          reason_))
    , nym_data_2_(api_.Wallet().mutable_Nym(
          api_.Factory().NymIDFromBase58(nym_id_2_),
          reason_))
    , nym_data_3_(api_.Wallet().mutable_Nym(
          api_.Factory().NymIDFromBase58(nym_id_3_),
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
}  // namespace ottest
