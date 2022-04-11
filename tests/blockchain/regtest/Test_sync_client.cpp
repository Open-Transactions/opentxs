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

#include <chrono>
#include <thread>

namespace ottest
{

TEST_F(Regtest_fixture_simple, start_stop_client)
{
    EXPECT_TRUE(Start());
    EXPECT_TRUE(Connect());

    const size_t numbers_of_test = 5;
    const std::string name = "Alice";
    const std::string words = "worry myself exile unit believe climb pitch theme two truly alter daughter";
    const auto blocks_number = 2;
    Height targetHeight = 0, begin = 0;

    for(size_t number_of_test =  0; number_of_test < numbers_of_test; number_of_test++) {
        Counter account_list_{};
        Counter account_activity_{};
        account_activity_.expected_ += 0;
        account_list_.expected_ += 1;

        auto user_name = name;
        auto [user, success] =
            CreateClient(opentxs::Options{}, number_of_test + 3, user_name, words, address_);
        EXPECT_TRUE(success);

        init_account_activity(
            user, GetHDAccount(user).Parent().AccountID(), account_activity_);
        init_account_list(user, account_list_);

        targetHeight += static_cast<Height>(blocks_number);

        auto scan_listener = std::make_unique<ScanListener>(*user.api_);

        ot::Sleep(std::chrono::seconds(1));

        auto scan_listener_external_f = scan_listener->get_future(
            GetHDAccount(user), bca::Subchain::External, targetHeight);
        auto scan_listener_internal_f = scan_listener->get_future(
            GetHDAccount(user), bca::Subchain::Internal, targetHeight);

        // mine coin for Alice
        auto mined_header = MineBlocks(
            user,
            begin,
            blocks_number,
            transaction_in_block_,
            amount_in_transaction_);

        EXPECT_TRUE(scan_listener->wait(scan_listener_external_f));
        EXPECT_TRUE(scan_listener->wait(scan_listener_internal_f));

        begin += blocks_number;
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

        EXPECT_EQ(
            GetBalance(user),
            amount_in_transaction_ *
                (blocks_number * (number_of_test + 1)) *
                transaction_in_block_);

        users_.erase(user_name);
        user.api_->Network().Blockchain().Stop(test_chain_);
        user_listeners_.erase(user_name);
    }

    Shutdown();
}

TEST_F(Regtest_fixture_simple, send_to_client)
{
    EXPECT_TRUE(Start());
    EXPECT_TRUE(Connect());

    const std::string name_alice = "Alice";
    const std::string words_alice = "worry myself exile unit believe climb pitch theme two truly alter daughter";

    const std::string name_bob = "Bob";
    const std::string words_bob = "myself two exile unit believe worry daughter climb pitch theme truly alter";

    Height targetHeight = 0, begin = 0;
    const auto blocks_number = 2;
    const auto coin_to_send = 100000;

    auto [user_alice, success_alice] = CreateClient(opentxs::Options{}, 3, name_alice, words_alice, address_);
    EXPECT_TRUE(success_alice);
    Counter account_list_{};
    Counter account_activity_{};
    account_activity_.expected_ += 0;
    account_list_.expected_ += 1;

    auto [user_bob, success_bob] = CreateClient(opentxs::Options{}, 4, name_bob, words_bob, address_);
    EXPECT_TRUE(success_bob);
    Counter account_list2_{};
    Counter account_activity2_{};
    account_activity2_.expected_ += 0;
    account_list2_.expected_ += 1;

    init_account_activity(
        user_alice, GetHDAccount(user_alice).Parent().AccountID(), account_activity_);
    init_account_list(user_alice, account_list_);

    init_account_activity(
        user_bob, GetHDAccount(user_bob).Parent().AccountID(), account_activity2_);
    init_account_list(user_bob, account_list2_);

    auto scan_listener_alice = std::make_unique<ScanListener>(*user_alice.api_);
//    auto block_listener_alice = std::make_unique<BlockListener>(*user_alice.api_);
//    auto wallet_listener_alice = std::make_unique<WalletListener>(*user_alice.api_);

    auto scan_listener_bob = std::make_unique<ScanListener>(*user_bob.api_);
//    auto block_listener_bob = std::make_unique<BlockListener>(*user_bob.api_);
//    auto wallet_listener_bob = std::make_unique<WalletListener>(*user_bob.api_);

    targetHeight += blocks_number;
//    auto block_listener_alice_f = block_listener_alice->GetFuture(targetHeight);
//    auto block_listener_bob_f = block_listener_bob->GetFuture(targetHeight);

    std::cout << "Mine for alice\n";
    // mine coin for Alice
    auto mined_header = MineBlocks(
        user_alice,
        begin,
        blocks_number,
        transaction_in_block_,
        amount_in_transaction_);

//    EXPECT_TRUE(
//        block_listener_alice_f.wait_for(wait_time_limit_) == std::future_status::ready);
//    EXPECT_TRUE(
//        block_listener_bob_f.wait_for(wait_time_limit_) == std::future_status::ready);

    begin += blocks_number;
    targetHeight += blocks_number;
//    block_listener_alice_f = block_listener_alice->GetFuture(targetHeight);
//    block_listener_bob_f = block_listener_bob->GetFuture(targetHeight);

    std::cout << "Mine for bob\n";
    // mine coin for Bob
    auto mined_header2 = MineBlocks(
        user_bob,
        begin,
        blocks_number,
        transaction_in_block_,
        amount_in_transaction_);

//    EXPECT_TRUE(
//        block_listener_alice_f.wait_for(wait_time_limit_) == std::future_status::ready);
//    EXPECT_TRUE(
//        block_listener_bob_f.wait_for(wait_time_limit_) == std::future_status::ready);

    begin += blocks_number;
    targetHeight += static_cast<int>(MaturationInterval()) + 1;

    auto scan_listener_external_alice_f = scan_listener_alice->get_future(
        GetHDAccount(user_alice), bca::Subchain::External, targetHeight);
    auto scan_listener_internal_alice_f = scan_listener_alice->get_future(
        GetHDAccount(user_alice), bca::Subchain::Internal, targetHeight);
    auto scan_listener_external_bob_f = scan_listener_bob->get_future(
        GetHDAccount(user_bob), bca::Subchain::External, targetHeight);
    auto scan_listener_internal_bob_f = scan_listener_bob->get_future(
        GetHDAccount(user_bob), bca::Subchain::Internal, targetHeight);

//    block_listener_alice_f = block_listener_alice->GetFuture(targetHeight);
//    block_listener_bob_f = block_listener_bob->GetFuture(targetHeight);
    //    auto wallet_listener_f = wallet_listener->GetFuture(targetHeight);

    std::cout << "Mine targetHeight: " << targetHeight << std::endl;
    // mine MaturationInterval number block with
    MineBlocks(begin, static_cast<int>(MaturationInterval()) + 1);
    begin += MaturationInterval() + 1;

//    EXPECT_TRUE(
//        block_listener_alice_f.wait_for(wait_time_limit_) == std::future_status::ready);
//    EXPECT_TRUE(
//        block_listener_bob_f.wait_for(wait_time_limit_) == std::future_status::ready);

    EXPECT_TRUE(scan_listener_alice->wait(scan_listener_external_alice_f));
    EXPECT_TRUE(scan_listener_alice->wait(scan_listener_internal_alice_f));
    EXPECT_EQ(GetBalance(user_alice), amount_in_transaction_ * blocks_number * transaction_in_block_);

    EXPECT_TRUE(scan_listener_bob->wait(scan_listener_external_bob_f));
    EXPECT_TRUE(scan_listener_bob->wait(scan_listener_internal_bob_f));
    EXPECT_EQ(GetBalance(user_bob), amount_in_transaction_ * blocks_number * transaction_in_block_);

    const User* sender = &user_alice;
    const User* receiver = &user_bob;
    Amount sender_amount = amount_in_transaction_ * blocks_number * transaction_in_block_;
    Amount receiver_amount = amount_in_transaction_ * blocks_number * transaction_in_block_;

    const size_t numbers_of_test = 4;

    for(size_t number_of_test =  0; number_of_test < numbers_of_test; number_of_test++) {
        const auto& network =
            sender->api_->Network().Blockchain().GetChain(test_chain_);

        const ot::UnallocatedCString memo_outgoing = "test";

        const auto address = GetNextBlockchainAddress(*receiver);

        std::cout << sender->name_ + " send " << coin_to_send << " to "
                  << receiver->name_ << " address: " << address << std::endl;

        auto future = network.SendToAddress(
            sender->nym_id_, address, coin_to_send, memo_outgoing);

        future.get();

        ot::Sleep(std::chrono::seconds(10));

        EXPECT_TRUE(GetBalance(*sender) < sender_amount - coin_to_send);
        EXPECT_EQ(GetBalance(*receiver), receiver_amount + coin_to_send);

        receiver_amount = GetBalance(*sender);
        sender_amount = GetBalance(*receiver);
        std::swap(sender, receiver);
    }

    Shutdown();
}

}
