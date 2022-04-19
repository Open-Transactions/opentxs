// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.ko

#include "Helpers.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <chrono>
#include <thread>

#include "Regtest_fixture_simple.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/AddressStyle.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/HDProtocol.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/node/BlockOracle.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/blockchain/node/TxoState.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"

namespace ottest
{
class TestRestart : public Regtest_fixture_simple
{

public:
    using Output = std::pair<
        ot::blockchain::block::Outpoint,
        std::unique_ptr<ot::blockchain::block::bitcoin::Output>>;
    using Outputs = std::pmr::vector<Output>;
    using OutputsSet = std::set<Output>;

    using OutputType = ot::blockchain::node::TxoState;

    TestRestart() {}

    const std::string name_alice = "Alice";
    const std::string name_bob = "Bob";

    int blocks_number_ = 1;
    const int coin_to_send_ = 100000;
    const int receiver_balance_before_send =
        amount_in_transaction_ * blocks_number_ * transaction_in_block_;

    const User& createSenderAlice()
    {
        const std::string words_alice = "worry myself exile unit believe climb "
                                        "pitch theme two truly alter daughter";

        auto [user_alice, success_alice] = CreateClient(
            opentxs::Options{}, 3, name_alice, words_alice, address_);
        EXPECT_TRUE(success_alice);

        return user_alice;
    }

    const User& createReceiverBob()
    {
        const std::string words_bob = "myself two exile unit believe worry "
                                      "daughter climb pitch theme truly alter";

        auto [user_bob, success_bob] =
            CreateClient(opentxs::Options{}, 4, name_bob, words_bob, address_);
        EXPECT_TRUE(success_bob);

        return user_bob;
    }

    void removeUsers(const User& user_alice, const User& user_bob)
    {
        users_.erase(name_alice);
        users_.erase(name_bob);

        user_alice.api_->Network().Blockchain().Stop(test_chain_);
        user_bob.api_->Network().Blockchain().Stop(test_chain_);

        user_listeners_.erase(name_alice);
        user_listeners_.erase(name_bob);
    }

    void mineForBothUsers(const User& user_bob, const User& user_alice)
    {
        Height targetHeight = 0, begin = 0;
        auto scan_listener_alice =
            std::make_unique<ScanListener>(*user_alice.api_);
        auto scan_listener_bob = std::make_unique<ScanListener>(*user_bob.api_);

        targetHeight += blocks_number_;

        std::cout << "Mine for alice\n";
        // mine coin for Alice
        auto mined_header = MineBlocks(
            user_alice,
            begin,
            blocks_number_,
            transaction_in_block_,
            amount_in_transaction_);

        begin += blocks_number_;
        targetHeight += blocks_number_;

        std::cout << "Mine for bob\n";
        // mine coin for Bob
        auto mined_header2 = MineBlocks(
            user_bob,
            begin,
            blocks_number_,
            transaction_in_block_,
            amount_in_transaction_);

        begin += blocks_number_;
        targetHeight += static_cast<int>(MaturationInterval()) + 1;

        auto scan_listener_external_alice_f = scan_listener_alice->get_future(
            GetHDAccount(user_alice), bca::Subchain::External, targetHeight);
        auto scan_listener_internal_alice_f = scan_listener_alice->get_future(
            GetHDAccount(user_alice), bca::Subchain::Internal, targetHeight);
        auto scan_listener_external_bob_f = scan_listener_bob->get_future(
            GetHDAccount(user_bob), bca::Subchain::External, targetHeight);
        auto scan_listener_internal_bob_f = scan_listener_bob->get_future(
            GetHDAccount(user_bob), bca::Subchain::Internal, targetHeight);

        std::cout << "Mine targetHeight: " << targetHeight << std::endl;
        // mine MaturationInterval number block with
        MineBlocks(begin, static_cast<int>(MaturationInterval()) + 1);

        EXPECT_TRUE(scan_listener_alice->wait(scan_listener_external_alice_f));
        EXPECT_TRUE(scan_listener_alice->wait(scan_listener_internal_alice_f));

        EXPECT_EQ(
            GetBalance(user_alice),
            amount_in_transaction_ * blocks_number_ * transaction_in_block_);

        EXPECT_TRUE(scan_listener_bob->wait(scan_listener_external_bob_f));
        EXPECT_TRUE(scan_listener_bob->wait(scan_listener_internal_bob_f));
        EXPECT_EQ(
            GetBalance(user_bob),
            amount_in_transaction_ * blocks_number_ * transaction_in_block_);
    }

    void sendFromAliceToBob(const User& receiver, const User& sender)
    {
        const auto& network =
            sender.api_->Network().Blockchain().GetChain(test_chain_);

        const ot::UnallocatedCString memo_outgoing = "test";

        const auto address = GetNextBlockchainAddress(receiver);

        std::cout << sender.name_ + " send " << coin_to_send_ << " to "
                  << receiver.name_ << " address: " << address << std::endl;

        auto future = network.SendToAddress(
            sender.nym_id_, address, coin_to_send_, memo_outgoing);

        future.get();

        ot::Sleep(std::chrono::seconds(20));
    }

    void collectOutputs(
        const User& user_bob,
        const User& user_alice,
        Outputs& all_bobs_outputs,
        Outputs& all_alice_outputs,
        std::map<OutputType, std::size_t>& number_of_oututs_per_type_bob,
        std::map<OutputType, std::size_t>& number_of_oututs_per_type_alice)
    {
        const auto& bobs_network =
            user_bob.api_->Network().Blockchain().GetChain(test_chain_);
        const auto& bobs_wallet = bobs_network.Wallet();

        const auto& alice_network =
            user_alice.api_->Network().Blockchain().GetChain(test_chain_);
        const auto& alice_wallet = alice_network.Wallet();

        all_bobs_outputs = bobs_wallet.GetOutputs(OutputType::All);
        all_alice_outputs = alice_wallet.GetOutputs(OutputType::All);

        auto all_types = {
            OutputType::Immature,
            OutputType::ConfirmedSpend,
            OutputType::UnconfirmedSpend,
            OutputType::ConfirmedNew,
            OutputType::UnconfirmedNew,
            OutputType::OrphanedSpend,
            OutputType::OrphanedNew};
        for (const auto& type : all_types) {
            number_of_oututs_per_type_bob[type] =
                bobs_wallet.GetOutputs(type).size();
            number_of_oututs_per_type_alice[type] =
                alice_wallet.GetOutputs(type).size();
        }
    }

    void collectOutputsAsSet(
        const User& user_bob,
        const User& user_alice,
        OutputsSet& all_bobs_outputs,
        OutputsSet& all_alice_outputs,
        std::map<OutputType, std::size_t>& number_of_oututs_per_type_bob,
        std::map<OutputType, std::size_t>& number_of_oututs_per_type_alice)
    {
        Outputs bobs_outputs;
        Outputs alice_outputs;

        collectOutputs(
            user_bob,
            user_alice,
            bobs_outputs,
            alice_outputs,
            number_of_oututs_per_type_bob,
            number_of_oututs_per_type_alice);

        EXPECT_EQ(bobs_outputs.size(), bobs_outputs.size());
        EXPECT_EQ(alice_outputs.size(), alice_outputs.size());

        for (std::size_t i = 0; i < bobs_outputs.size(); ++i)
            all_bobs_outputs.insert(std::move(bobs_outputs[i]));

        for (std::size_t i = 0; i < alice_outputs.size(); ++i)
            all_alice_outputs.insert(std::move(alice_outputs[i]));
    }

    void collectFeeRate(
        const User& user_bob,
        const User& user_alice,
        ot::Amount& bobs_fee_rate,
        ot::Amount& alice_fee_rate)
    {
        const auto& bobs_network =
            user_bob.api_->Network().Blockchain().GetChain(test_chain_);

        const auto& alice_network =
            user_alice.api_->Network().Blockchain().GetChain(test_chain_);

        bobs_fee_rate = bobs_network.Internal().FeeRate();
        alice_fee_rate = alice_network.Internal().FeeRate();
    }

    void validateOutputs(
        const User& user_bob_after_reboot,
        const User& user_alice_after_reboot,
        const std::set<Output>& bob_outputs,
        const std::set<Output>& alice_outputs,
        const std::map<OutputType, std::size_t>& bob_all_outputs_size,
        const std::map<OutputType, std::size_t>& alice_all_outputs_size)
    {
        std::set<Output> bob_outputs_after_reboot;
        std::set<Output> alice_outputs_after_reboot;
        std::map<OutputType, std::size_t> bob_all_outputs_size_after_reboot;
        std::map<OutputType, std::size_t> alice_all_outputs_size_after_reboot;

        collectOutputsAsSet(
            user_bob_after_reboot,
            user_alice_after_reboot,
            bob_outputs_after_reboot,
            alice_outputs_after_reboot,
            bob_all_outputs_size_after_reboot,
            alice_all_outputs_size_after_reboot);

        EXPECT_EQ(bob_all_outputs_size_after_reboot, bob_all_outputs_size);
        EXPECT_EQ(alice_all_outputs_size_after_reboot, alice_all_outputs_size);

        EXPECT_EQ(bob_outputs_after_reboot.size(), bob_outputs.size());
        EXPECT_EQ(alice_outputs_after_reboot.size(), alice_outputs.size());

        std::cout << "Comparing bob outputs\n";
        compareOutputs(bob_outputs, bob_outputs_after_reboot);
        std::cout << "Comparison finished.\nComparing alice outputs\n";
        compareOutputs(alice_outputs, alice_outputs_after_reboot);
        std::cout << "Comparison finished.\n";
    }

    void compareOutputs(
        const std::set<Output>& pre_reboot_outputs,
        const std::set<Output>& post_reboot_outputs)
    {
        auto iter = post_reboot_outputs.begin();
        for (const auto& [outpoint, output] : pre_reboot_outputs) {
            EXPECT_EQ(iter->first, outpoint);
            EXPECT_EQ(iter->second->Value(), output->Value());
            EXPECT_TRUE(iter->second->Script().CompareScriptElements(output->Script()));
            iter++;
        }
    }
};

TEST_F(TestRestart, send_to_client_reboot_confirm_data)
{
    EXPECT_TRUE(Start());
    EXPECT_TRUE(Connect());

    ot::Amount actual_receiver_balance_after_send;

    // Create wallets
    const auto& user_alice = createSenderAlice();
    const auto& user_bob = createReceiverBob();

    // Get data for later validation
    const auto bobs_payment_code = user_bob.PaymentCode();
    const auto bobs_hd_name = GetHDAccount(user_bob).Name();

    // Mine initial balance
    mineForBothUsers(user_bob, user_alice);

    // Send coins from alice to bob
    sendFromAliceToBob(user_bob, user_alice);

    // Collect fees
    ot::Amount bobs_fee_rate, alice_fee_rate;
    collectFeeRate(user_bob, user_alice, bobs_fee_rate, alice_fee_rate);

    // Save current balance and check if is correct
    actual_receiver_balance_after_send = GetBalance(user_bob);
    EXPECT_EQ(
        receiver_balance_before_send + coin_to_send_,
        actual_receiver_balance_after_send);

    // Collect outputs
    std::set<Output> bob_outputs;
    std::set<Output> alice_outputs;
    std::map<OutputType, std::size_t> bob_all_outputs_size;
    std::map<OutputType, std::size_t> alice_all_outputs_size;

    collectOutputsAsSet(
        user_bob,
        user_alice,
        bob_outputs,
        alice_outputs,
        bob_all_outputs_size,
        alice_all_outputs_size);

    // Cleanup
    removeUsers(user_alice, user_bob);

    // Restore both wallets
    const auto& user_alice_after_reboot = createSenderAlice();
    const auto& user_bob_after_reboot = createReceiverBob();

    // Compare balance
    auto balance_after_restore = GetBalance(user_bob_after_reboot);
    EXPECT_EQ(balance_after_restore, actual_receiver_balance_after_send);
    EXPECT_EQ(user_bob_after_reboot.PaymentCode(), bobs_payment_code);

    // Compare fee rates
    ot::Amount bobs_fee_rate_after_reboot, alice_fee_rate_after_reboot;
    collectFeeRate(
        user_alice_after_reboot,
        user_bob_after_reboot,
        bobs_fee_rate_after_reboot,
        alice_fee_rate_after_reboot);

    EXPECT_EQ(bobs_fee_rate_after_reboot, bobs_fee_rate);
    EXPECT_EQ(alice_fee_rate_after_reboot, alice_fee_rate);

    // Compare wallet name
    EXPECT_EQ(GetHDAccount(user_bob_after_reboot).Name(), bobs_hd_name);

    // Compare outputs
    validateOutputs(
        user_bob_after_reboot,
        user_alice_after_reboot,
        bob_outputs,
        alice_outputs,
        bob_all_outputs_size,
        alice_all_outputs_size);

    Shutdown();
}

}  // namespace ottest
