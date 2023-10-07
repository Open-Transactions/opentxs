// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "ottest/fixtures/blockchain/SyncListener.hpp"
// IWYU pragma: no_include "ottest/fixtures/blockchain/TXOState.hpp"

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <array>
#include <atomic>
#include <chrono>
#include <future>
#include <iostream>
#include <string_view>

#include "internal/api/session/UI.hpp"
#include "internal/interface/ui/AccountActivity.hpp"
#include "internal/interface/ui/BalanceItem.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "ottest/fixtures/blockchain/Common.hpp"
#include "ottest/fixtures/blockchain/ScanListener.hpp"
#include "ottest/fixtures/blockchain/regtest/Stress.hpp"
#include "ottest/fixtures/common/Counter.hpp"

namespace ottest
{
using namespace opentxs::literals;
using namespace std::literals;

Counter account_activity_{};

TEST_F(Regtest_stress, init_opentxs) {}

TEST_F(Regtest_stress, start_chains) { EXPECT_TRUE(Start()); }

TEST_F(Regtest_stress, connect_peers) { EXPECT_TRUE(Connect()); }

TEST_F(Regtest_stress, mine_initial_balance)
{
    auto future1 =
        listener_alex_.get_future(alex_account_, Subchain::External, 1);
    auto future2 =
        listener_alex_.get_future(alex_account_, Subchain::Internal, 1);

    std::cout << "Block 1\n";
    namespace c = std::chrono;
    EXPECT_TRUE(Mine(0, 1, mine_to_alex_));
    EXPECT_TRUE(listener_alex_.wait(future1));
    EXPECT_TRUE(listener_alex_.wait(future2));
}

TEST_F(Regtest_stress, alex_after_receive_wallet)
{
    const auto handle = client_1_.Network().Blockchain().GetChain(test_chain_);

    ASSERT_TRUE(handle);

    const auto& network = handle.get();
    const auto& wallet = network.Wallet();
    const auto& nym = alex_.ID();
    const auto& account = alex_account_.ID();
    const auto blankNym = ot::identifier::Nym{};
    const auto blankAccount = ot::identifier::Account{};
    using Balance = ot::blockchain::Balance;
    const auto outputs = tx_per_block_ * 2u;
    const auto amount = outputs * amount_;
    const auto balance = Balance{amount, amount};
    const auto noBalance = Balance{0, 0};

    EXPECT_EQ(wallet.GetBalance(), balance);
    EXPECT_EQ(network.GetBalance(), balance);
    EXPECT_EQ(wallet.GetBalance(nym), balance);
    EXPECT_EQ(network.GetBalance(nym), balance);
    EXPECT_EQ(wallet.GetBalance(nym, account), balance);
    EXPECT_EQ(wallet.GetBalance(blankNym), noBalance);
    EXPECT_EQ(network.GetBalance(blankNym), noBalance);
    EXPECT_EQ(wallet.GetBalance(blankNym, blankAccount), noBalance);
    EXPECT_EQ(wallet.GetBalance(nym, blankAccount), noBalance);
    EXPECT_EQ(wallet.GetBalance(blankNym, account), noBalance);

    using TxoState = ot::blockchain::node::TxoState;
    auto type = TxoState::All;

    EXPECT_EQ(wallet.GetOutputs(type).size(), outputs);
    EXPECT_EQ(wallet.GetOutputs(nym, type).size(), outputs);
    EXPECT_EQ(wallet.GetOutputs(nym, account, type).size(), outputs);
    EXPECT_EQ(wallet.GetOutputs(blankNym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, account, type).size(), 0u);

    type = TxoState::UnconfirmedNew;

    EXPECT_EQ(wallet.GetOutputs(type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, account, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, account, type).size(), 0u);

    type = TxoState::UnconfirmedSpend;

    EXPECT_EQ(wallet.GetOutputs(type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, account, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, account, type).size(), 0u);

    type = TxoState::ConfirmedNew;

    EXPECT_EQ(wallet.GetOutputs(type).size(), outputs);
    EXPECT_EQ(wallet.GetOutputs(nym, type).size(), outputs);
    EXPECT_EQ(wallet.GetOutputs(nym, account, type).size(), outputs);
    EXPECT_EQ(wallet.GetOutputs(blankNym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, account, type).size(), 0u);

    type = TxoState::ConfirmedSpend;

    EXPECT_EQ(wallet.GetOutputs(type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, account, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, account, type).size(), 0u);

    type = TxoState::OrphanedNew;

    EXPECT_EQ(wallet.GetOutputs(type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, account, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, account, type).size(), 0u);

    type = TxoState::OrphanedSpend;

    EXPECT_EQ(wallet.GetOutputs(type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, account, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, account, type).size(), 0u);
}

TEST_F(Regtest_stress, generate_transactions)
{
    namespace c = std::chrono;
    const auto handle = client_1_.Network().Blockchain().GetChain(test_chain_);

    ASSERT_TRUE(handle);

    const auto& alex = handle.get().Wallet();
    const auto previous{1u};
    const auto stop = previous + blocks_;
    auto future1 =
        listener_bob_.get_future(bob_account_, Subchain::External, stop);
    auto transactions =
        ot::UnallocatedVector<ot::blockchain::block::TransactionHash>{
            tx_per_block_, ot::blockchain::block::TransactionHash{}};
    using Future = ot::blockchain::node::PendingOutgoing;
    auto futures = std::array<Future, tx_per_block_>{};

    ASSERT_EQ(transactions.size(), tx_per_block_);

    for (auto b{0u}; b < blocks_; ++b) {
        std::cout << "Block " << std::to_string(previous + b + 1) << '\n';
        const auto start = ot::Clock::now();
        const auto destinations = GetAddresses();
        const auto addresses = ot::Clock::now();
        std::cout
            << std::to_string(
                   c::duration_cast<c::seconds>(addresses - start).count())
            << " sec to calculate receiving addresses\n";

        for (auto t{0u}; t < tx_per_block_; ++t) {
            auto spend = alex.CreateSpend(alex_.ID());

            if (false == spend.SetUseEnhancedNotifications(false)) {
                ADD_FAILURE();
            }

            if (false == spend.SendToAddress(destinations.at(t), amount_)) {
                ADD_FAILURE();
            }

            futures.at(t) = alex.Execute(spend);
        }

        const auto init = ot::Clock::now();
        std::cout << std::to_string(
                         c::duration_cast<c::seconds>(init - addresses).count())
                  << " sec to queue outgoing transactions\n";

        {
            auto f{-1};

            for (auto& future : futures) {
                using State = std::future_status;
                constexpr auto limit = std::chrono::minutes{10};

                while (State::ready != future.wait_for(limit)) {}

                try {
                    auto [code, txid] = future.get();

                    opentxs::assert_true(false == txid.empty());

                    const auto rc = transactions.at(++f).Assign(txid);

                    EXPECT_TRUE(rc);
                } catch (...) {

                    opentxs::LogAbort()().Abort();
                }
            }
        }

        const auto sigs = ot::Clock::now();
        std::cout << std::to_string(
                         c::duration_cast<c::seconds>(sigs - init).count())
                  << " sec to sign and broadcast transactions\n";
        const auto extra = [&] {
            auto output = ot::UnallocatedVector<Transaction>{};

            for (const auto& txid : transactions) {
                output.emplace_back(
                    client_1_.Crypto().Blockchain().LoadTransaction(txid));
            }

            return output;
        }();

        const auto target = previous + b + 1;
        auto future3 = listener_alex_.get_future(
            alex_account_, Subchain::External, target);
        auto future4 = listener_alex_.get_future(
            alex_account_, Subchain::Internal, target);

        EXPECT_TRUE(Mine(previous + b, 1, mine_to_alex_, extra));

        const auto mined = ot::Clock::now();
        std::cout << std::to_string(
                         c::duration_cast<c::seconds>(mined - sigs).count())
                  << " sec to mine block\n";

        EXPECT_TRUE(listener_alex_.wait(future3));
        EXPECT_TRUE(listener_alex_.wait(future4));

        const auto scanned = ot::Clock::now();
        std::cout << std::to_string(
                         c::duration_cast<c::seconds>(scanned - mined).count())
                  << " sec to scan accounts\n";
    }

    EXPECT_TRUE(listener_bob_.wait(future1));
}

TEST_F(Regtest_stress, bob_after_receive)
{
    account_activity_.expected_ += transaction_count_ + 1u;
    const auto& widget = client_2_.UI().Internal().AccountActivity(
        bob_.ID(),
        expected_account_bob_,
        make_cb(account_activity_, u8"account_activity_"sv));
    constexpr auto expectedTotal = amount_ * transaction_count_;
    wait_for_counter(account_activity_, false);

    EXPECT_EQ(widget.Balance(), expectedTotal);
    EXPECT_EQ(widget.BalancePolarity(), 1);

    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->Amount(), amount_);

    while (false == row->Last()) {
        row = widget.Next();

        EXPECT_EQ(row->Amount(), amount_);
    }
}

TEST_F(Regtest_stress, shutdown) { Shutdown(); }
}  // namespace ottest
