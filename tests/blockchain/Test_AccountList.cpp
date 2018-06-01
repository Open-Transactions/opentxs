/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#include "opentxs/opentxs.hpp"

#include <gtest/gtest.h>

using namespace opentxs;

namespace
{

/* Tests new account and size
 */

TEST(Test_AccountList, testList)
{
    static const proto::ContactItemType INDIVIDUAL =
        proto::CITEMTYPE_INDIVIDUAL;
    const auto AliceNymID =
        opentxs::OT::App().API().Exec().CreateNymHD(INDIVIDUAL, "testList_Alice", "", 10);
    std::cout << "Created Alice's Nym: " << AliceNymID << " !!\n";
    const auto AliceNymID2 =
        opentxs::OT::App().API().Exec().CreateNymHD(INDIVIDUAL, "testList_Alice", "", 11);
    std::cout << "Created Alice's 2nd Nym: " << AliceNymID2<< " !!\n";
    const auto BobNymID =
        opentxs::OT::App().API().Exec().CreateNymHD(INDIVIDUAL, "testList_Bob", "", 20);
    std::cout << "Created Bob's Nym: " << BobNymID << " !!\n";
    
    const auto BTC = static_cast<proto::ContactItemType>(proto::CITEMTYPE_BTC);
    const auto LTC = static_cast<proto::ContactItemType>(proto::CITEMTYPE_LTC);

    ASSERT_EQ(
        0,
        opentxs::OT::App()
            .Blockchain()
            .AccountList(Identifier(AliceNymID), BTC)
            .size());

    OTIdentifier AliceAccountID = OT::App().Blockchain().NewAccount(
        Identifier(AliceNymID), BlockchainAccountType::BIP44, BTC);
    std::cout << "Created Alice's Account: " << String(AliceAccountID).Get()
              << " !!\n";

    OTIdentifier AliceAccountID2 = OT::App().Blockchain().NewAccount(
        Identifier(AliceNymID2), BlockchainAccountType::BIP32, BTC);
    std::cout << "Created Alice's 2nd Account: "
              << String(AliceAccountID2).Get() << " !!\n";

    const std::set<OTIdentifier> as =
      opentxs::OT::App().Blockchain().AccountList(Identifier(AliceNymID), BTC);
    const std::set<OTIdentifier> bs =
      opentxs::OT::App().Blockchain().AccountList(Identifier(BobNymID), BTC);
    EXPECT_EQ(1, as.size());
    ASSERT_EQ(0, bs.size());
    
    // const std::set<OTIdentifier> acs =
    // opentxs::OT::App().Blockchain().AccountList(Identifier(Alice), BTC);
    // EXPECT_EQ(2, acs.size());

    OTIdentifier BobAccountID = OT::App().Blockchain().NewAccount(
        Identifier(BobNymID), BlockchainAccountType::BIP44, BTC);

    const std::set<OTIdentifier> bcs =
        opentxs::OT::App().Blockchain().AccountList(Identifier(BobNymID), BTC);
    EXPECT_EQ(1, bcs.size());

    OTIdentifier AliceLTCAccountID = OT::App().Blockchain().NewAccount(
        Identifier(AliceNymID), BlockchainAccountType::BIP44, LTC);

    const std::set<OTIdentifier> als =
        opentxs::OT::App().Blockchain().AccountList(Identifier(AliceNymID), LTC);
    EXPECT_EQ(1, als.size());
}
}  // namespace
