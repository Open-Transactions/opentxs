// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"

#include <gtest/gtest.h>

using namespace opentxs;

namespace
{
class Test_AccountList : public ::testing::Test
{
public:
    const opentxs::api::client::Manager& client_;

    Test_AccountList()
        : client_(opentxs::OT::App().StartClient({}, 0))
    {
    }
};

/* Tests new account and size
 */
TEST_F(Test_AccountList, testList)
{
    static const proto::ContactItemType INDIVIDUAL =
        proto::CITEMTYPE_INDIVIDUAL;
    const auto AliceNymID =
        client_.Exec().CreateNymHD(INDIVIDUAL, "testList_Alice", "", 10);
    std::cout << "Created Alice's Nym: " << AliceNymID << " !!\n";
    const auto AliceNymID2 =
        client_.Exec().CreateNymHD(INDIVIDUAL, "testList_Alice", "", 11);
    std::cout << "Created Alice's 2nd Nym: " << AliceNymID2 << " !!\n";
    const auto BobNymID =
        client_.Exec().CreateNymHD(INDIVIDUAL, "testList_Bob", "", 20);
    std::cout << "Created Bob's Nym: " << BobNymID << " !!\n";

    ASSERT_EQ(
        0,
        client_.Blockchain()
            .AccountList(
                identifier::Nym::Factory(AliceNymID), proto::CITEMTYPE_BTC)
            .size());

    OTIdentifier AliceAccountID = client_.Blockchain().NewAccount(
        identifier::Nym::Factory(AliceNymID),
        BlockchainAccountType::BIP44,
        proto::CITEMTYPE_BTC);
    std::cout << "Created Alice's Account: "
              << String::Factory(AliceAccountID)->Get() << " !!\n";

    OTIdentifier AliceAccountID2 = client_.Blockchain().NewAccount(
        identifier::Nym::Factory(AliceNymID2),
        BlockchainAccountType::BIP32,
        proto::CITEMTYPE_BTC);
    std::cout << "Created Alice's 2nd Account: "
              << String::Factory(AliceAccountID2)->Get() << " !!\n";

    const std::set<OTIdentifier> as = client_.Blockchain().AccountList(
        identifier::Nym::Factory(AliceNymID), proto::CITEMTYPE_BTC);
    const std::set<OTIdentifier> bs = client_.Blockchain().AccountList(
        identifier::Nym::Factory(BobNymID), proto::CITEMTYPE_BTC);
    EXPECT_EQ(1, as.size());
    ASSERT_EQ(0, bs.size());

    OTIdentifier BobAccountID = client_.Blockchain().NewAccount(
        identifier::Nym::Factory(BobNymID),
        BlockchainAccountType::BIP44,
        proto::CITEMTYPE_BTC);

    const std::set<OTIdentifier> bcs = client_.Blockchain().AccountList(
        identifier::Nym::Factory(BobNymID), proto::CITEMTYPE_BTC);
    EXPECT_EQ(1, bcs.size());

    OTIdentifier AliceLTCAccountID = client_.Blockchain().NewAccount(
        identifier::Nym::Factory(AliceNymID),
        BlockchainAccountType::BIP44,
        proto::CITEMTYPE_LTC);

    const std::set<OTIdentifier> als = client_.Blockchain().AccountList(
        identifier::Nym::Factory(AliceNymID), proto::CITEMTYPE_LTC);
    EXPECT_EQ(1, als.size());
}
}  // namespace
