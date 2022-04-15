// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Helpers.hpp"  // IWYU pragma: associated
#include "Regtest_fixture_simple.hpp"

#include <gtest/gtest.h>
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/blockchain/crypto/HDProtocol.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/BlockOracle.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/blockchain/crypto/AddressStyle.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/OT.hpp"

#include <chrono>
#include <thread>

namespace ottest
{

TEST_F(Regtest_fixture_simple, start_stop_client)
{
    EXPECT_TRUE(Start());
    EXPECT_TRUE(Connect());

    const int numbers_of_test = 2;
    const std::string name = "Alice";
    const std::string words = "worry myself exile unit believe climb pitch "
                              "theme two truly alter daughter";
    const auto blocks_mine_for_alice = 2;
    bool mine_only_in_first_test = false;
    Height targetHeight = 0, begin = 0;
    auto expected_balance = 0;

    for (int number_of_test = 0; number_of_test < numbers_of_test;
         number_of_test++) {
        ot::LogConsole()(
            "Start test number: " + std::to_string(number_of_test + 1))
            .Flush();

        auto [user, success] = CreateClient(
            opentxs::Options{}, number_of_test + 3, name, words, address_);
        EXPECT_TRUE(success);

        WaitForSynchro(user, targetHeight, expected_balance);
        if (!mine_only_in_first_test || number_of_test == 0) {
            targetHeight += static_cast<Height>(blocks_mine_for_alice);

            auto scan_listener = std::make_unique<ScanListener>(*user.api_);

            auto scan_listener_external_f = scan_listener->get_future(
                GetHDAccount(user), bca::Subchain::External, targetHeight);
            auto scan_listener_internal_f = scan_listener->get_future(
                GetHDAccount(user), bca::Subchain::Internal, targetHeight);

            // mine coin for Alice
            auto mined_header = MineBlocks(
                user,
                begin,
                blocks_mine_for_alice,
                transaction_in_block_,
                amount_in_transaction_);

            EXPECT_TRUE(scan_listener->wait(scan_listener_external_f));
            EXPECT_TRUE(scan_listener->wait(scan_listener_internal_f));

            begin += blocks_mine_for_alice;
            auto count = static_cast<int>(MaturationInterval());
            targetHeight += count;

            scan_listener_external_f = scan_listener->get_future(
                GetHDAccount(user), bca::Subchain::External, targetHeight);
            scan_listener_internal_f = scan_listener->get_future(
                GetHDAccount(user), bca::Subchain::Internal, targetHeight);

            // mine MaturationInterval number block with
            MineBlocks(begin, count);
            begin += count;

            EXPECT_TRUE(scan_listener->wait(scan_listener_external_f));
            EXPECT_TRUE(scan_listener->wait(scan_listener_internal_f));

            expected_balance += amount_in_transaction_ * blocks_mine_for_alice *
                                transaction_in_block_;
            WaitForSynchro(user, targetHeight, expected_balance);
        }

        EXPECT_EQ(GetBalance(user), expected_balance);

        users_.erase(name);
        user.api_->Network().Blockchain().Stop(test_chain_);
        user_listeners_.erase(name);
    }

    Shutdown();
}

}  // namespace ottest
