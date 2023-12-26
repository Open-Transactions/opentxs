// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <atomic>

#include "internal/interface/ui/BlockchainSelection.hpp"
#include "ottest/fixtures/common/Counter.hpp"
#include "ottest/fixtures/ui/BlockchainSelection.hpp"
#include "ottest/fixtures/ui/BlockchainSelector.hpp"

namespace ottest
{
TEST_F(BlockchainSelector, initialize_opentxs) {}

TEST_F(BlockchainSelector, initial_state)
{
    const auto expectedA = BlockchainSelectionData{{
        {"Bitcoin", false, false, Type::Bitcoin},
        {"Bitcoin Cash", false, false, Type::BitcoinCash},
        {"Bitcoin SV", false, false, Type::BitcoinSV},
        {"Dash", false, false, Type::Dash},
        {"Litecoin", false, false, Type::Litecoin},
        {"PKT", false, false, Type::PKT},
        {"eCash", false, false, Type::eCash},
        {"Bitcoin (testnet3)", false, true, Type::Bitcoin_testnet3},
        {"Bitcoin Cash (testnet3)", false, true, Type::BitcoinCash_testnet3},
        {"Bitcoin Cash (testnet4)", false, true, Type::BitcoinCash_testnet4},
        {"Bitcoin SV (testnet3)", false, true, Type::BitcoinSV_testnet3},
        {"Dash (testnet3)", false, true, Type::Dash_testnet3},
        {"Litecoin (testnet4)", false, true, Type::Litecoin_testnet4},
        {"eCash (testnet3)", false, true, Type::eCash_testnet3},
    }};
    const auto expectedM = BlockchainSelectionData{{
        {"Bitcoin", false, false, Type::Bitcoin},
        {"Bitcoin Cash", false, false, Type::BitcoinCash},
        {"Bitcoin SV", false, false, Type::BitcoinSV},
        {"Dash", false, false, Type::Dash},
        {"Litecoin", false, false, Type::Litecoin},
        {"PKT", false, false, Type::PKT},
        {"eCash", false, false, Type::eCash},
    }};
    const auto expectedT = BlockchainSelectionData{{
        {"Bitcoin (testnet3)", false, true, Type::Bitcoin_testnet3},
        {"Bitcoin Cash (testnet3)", false, true, Type::BitcoinCash_testnet3},
        {"Bitcoin Cash (testnet4)", false, true, Type::BitcoinCash_testnet4},
        {"Bitcoin SV (testnet3)", false, true, Type::BitcoinSV_testnet3},
        {"Dash (testnet3)", false, true, Type::Dash_testnet3},
        {"Litecoin (testnet4)", false, true, Type::Litecoin_testnet4},
        {"eCash (testnet3)", false, true, Type::eCash_testnet3},
    }};

    ASSERT_TRUE(wait_for_counter(counter_full_));
    ASSERT_TRUE(wait_for_counter(counter_main_));
    ASSERT_TRUE(wait_for_counter(counter_test_));
    EXPECT_TRUE(check_blockchain_selection(
        client_, ot::ui::Blockchains::All, expectedA));
    EXPECT_TRUE(check_blockchain_selection_qt(
        client_, ot::ui::Blockchains::All, expectedA));
    EXPECT_TRUE(check_blockchain_selection(
        client_, ot::ui::Blockchains::Main, expectedM));
    EXPECT_TRUE(check_blockchain_selection_qt(
        client_, ot::ui::Blockchains::Main, expectedM));
    EXPECT_TRUE(check_blockchain_selection(
        client_, ot::ui::Blockchains::Test, expectedT));
    EXPECT_TRUE(check_blockchain_selection_qt(
        client_, ot::ui::Blockchains::Test, expectedT));
}

TEST_F(BlockchainSelector, disable_disabled)
{
    {
        counter_full_.expected_ += 0;
        counter_main_.expected_ += 0;

        EXPECT_TRUE(full_.Disable(Type::Bitcoin));
    }

    const auto expectedA = BlockchainSelectionData{{
        {"Bitcoin", false, false, Type::Bitcoin},
        {"Bitcoin Cash", false, false, Type::BitcoinCash},
        {"Bitcoin SV", false, false, Type::BitcoinSV},
        {"Dash", false, false, Type::Dash},
        {"Litecoin", false, false, Type::Litecoin},
        {"PKT", false, false, Type::PKT},
        {"eCash", false, false, Type::eCash},
        {"Bitcoin (testnet3)", false, true, Type::Bitcoin_testnet3},
        {"Bitcoin Cash (testnet3)", false, true, Type::BitcoinCash_testnet3},
        {"Bitcoin Cash (testnet4)", false, true, Type::BitcoinCash_testnet4},
        {"Bitcoin SV (testnet3)", false, true, Type::BitcoinSV_testnet3},
        {"Dash (testnet3)", false, true, Type::Dash_testnet3},
        {"Litecoin (testnet4)", false, true, Type::Litecoin_testnet4},
        {"eCash (testnet3)", false, true, Type::eCash_testnet3},
    }};
    const auto expectedM = BlockchainSelectionData{{
        {"Bitcoin", false, false, Type::Bitcoin},
        {"Bitcoin Cash", false, false, Type::BitcoinCash},
        {"Bitcoin SV", false, false, Type::BitcoinSV},
        {"Dash", false, false, Type::Dash},
        {"Litecoin", false, false, Type::Litecoin},
        {"PKT", false, false, Type::PKT},
        {"eCash", false, false, Type::eCash},
    }};
    const auto expectedT = BlockchainSelectionData{{
        {"Bitcoin (testnet3)", false, true, Type::Bitcoin_testnet3},
        {"Bitcoin Cash (testnet3)", false, true, Type::BitcoinCash_testnet3},
        {"Bitcoin Cash (testnet4)", false, true, Type::BitcoinCash_testnet4},
        {"Bitcoin SV (testnet3)", false, true, Type::BitcoinSV_testnet3},
        {"Dash (testnet3)", false, true, Type::Dash_testnet3},
        {"Litecoin (testnet4)", false, true, Type::Litecoin_testnet4},
        {"eCash (testnet3)", false, true, Type::eCash_testnet3},
    }};

    ASSERT_TRUE(wait_for_counter(counter_full_));
    ASSERT_TRUE(wait_for_counter(counter_main_));
    ASSERT_TRUE(wait_for_counter(counter_test_));
    EXPECT_TRUE(check_blockchain_selection(
        client_, ot::ui::Blockchains::All, expectedA));
    EXPECT_TRUE(check_blockchain_selection_qt(
        client_, ot::ui::Blockchains::All, expectedA));
    EXPECT_TRUE(check_blockchain_selection(
        client_, ot::ui::Blockchains::Main, expectedM));
    EXPECT_TRUE(check_blockchain_selection_qt(
        client_, ot::ui::Blockchains::Main, expectedM));
    EXPECT_TRUE(check_blockchain_selection(
        client_, ot::ui::Blockchains::Test, expectedT));
    EXPECT_TRUE(check_blockchain_selection_qt(
        client_, ot::ui::Blockchains::Test, expectedT));
}

TEST_F(BlockchainSelector, enable_disabled)
{
    {
        counter_full_.expected_ += 1;
        counter_main_.expected_ += 1;

        EXPECT_TRUE(full_.Enable(Type::Bitcoin));
    }

    const auto expectedA = BlockchainSelectionData{{
        {"Bitcoin", true, false, Type::Bitcoin},
        {"Bitcoin Cash", false, false, Type::BitcoinCash},
        {"Bitcoin SV", false, false, Type::BitcoinSV},
        {"Dash", false, false, Type::Dash},
        {"Litecoin", false, false, Type::Litecoin},
        {"PKT", false, false, Type::PKT},
        {"eCash", false, false, Type::eCash},
        {"Bitcoin (testnet3)", false, true, Type::Bitcoin_testnet3},
        {"Bitcoin Cash (testnet3)", false, true, Type::BitcoinCash_testnet3},
        {"Bitcoin Cash (testnet4)", false, true, Type::BitcoinCash_testnet4},
        {"Bitcoin SV (testnet3)", false, true, Type::BitcoinSV_testnet3},
        {"Dash (testnet3)", false, true, Type::Dash_testnet3},
        {"Litecoin (testnet4)", false, true, Type::Litecoin_testnet4},
        {"eCash (testnet3)", false, true, Type::eCash_testnet3},
    }};
    const auto expectedM = BlockchainSelectionData{{
        {"Bitcoin", true, false, Type::Bitcoin},
        {"Bitcoin Cash", false, false, Type::BitcoinCash},
        {"Bitcoin SV", false, false, Type::BitcoinSV},
        {"Dash", false, false, Type::Dash},
        {"Litecoin", false, false, Type::Litecoin},
        {"PKT", false, false, Type::PKT},
        {"eCash", false, false, Type::eCash},
    }};
    const auto expectedT = BlockchainSelectionData{{
        {"Bitcoin (testnet3)", false, true, Type::Bitcoin_testnet3},
        {"Bitcoin Cash (testnet3)", false, true, Type::BitcoinCash_testnet3},
        {"Bitcoin Cash (testnet4)", false, true, Type::BitcoinCash_testnet4},
        {"Bitcoin SV (testnet3)", false, true, Type::BitcoinSV_testnet3},
        {"Dash (testnet3)", false, true, Type::Dash_testnet3},
        {"Litecoin (testnet4)", false, true, Type::Litecoin_testnet4},
        {"eCash (testnet3)", false, true, Type::eCash_testnet3},
    }};

    ASSERT_TRUE(wait_for_counter(counter_full_));
    ASSERT_TRUE(wait_for_counter(counter_main_));
    ASSERT_TRUE(wait_for_counter(counter_test_));
    EXPECT_TRUE(check_blockchain_selection(
        client_, ot::ui::Blockchains::All, expectedA));
    EXPECT_TRUE(check_blockchain_selection_qt(
        client_, ot::ui::Blockchains::All, expectedA));
    EXPECT_TRUE(check_blockchain_selection(
        client_, ot::ui::Blockchains::Main, expectedM));
    EXPECT_TRUE(check_blockchain_selection_qt(
        client_, ot::ui::Blockchains::Main, expectedM));
    EXPECT_TRUE(check_blockchain_selection(
        client_, ot::ui::Blockchains::Test, expectedT));
    EXPECT_TRUE(check_blockchain_selection_qt(
        client_, ot::ui::Blockchains::Test, expectedT));
}

TEST_F(BlockchainSelector, enable_enabled)
{
    {
        counter_full_.expected_ += 0;
        counter_main_.expected_ += 0;

        EXPECT_TRUE(full_.Enable(Type::Bitcoin));
    }

    const auto expectedA = BlockchainSelectionData{{
        {"Bitcoin", true, false, Type::Bitcoin},
        {"Bitcoin Cash", false, false, Type::BitcoinCash},
        {"Bitcoin SV", false, false, Type::BitcoinSV},
        {"Dash", false, false, Type::Dash},
        {"Litecoin", false, false, Type::Litecoin},
        {"PKT", false, false, Type::PKT},
        {"eCash", false, false, Type::eCash},
        {"Bitcoin (testnet3)", false, true, Type::Bitcoin_testnet3},
        {"Bitcoin Cash (testnet3)", false, true, Type::BitcoinCash_testnet3},
        {"Bitcoin Cash (testnet4)", false, true, Type::BitcoinCash_testnet4},
        {"Bitcoin SV (testnet3)", false, true, Type::BitcoinSV_testnet3},
        {"Dash (testnet3)", false, true, Type::Dash_testnet3},
        {"Litecoin (testnet4)", false, true, Type::Litecoin_testnet4},
        {"eCash (testnet3)", false, true, Type::eCash_testnet3},
    }};
    const auto expectedM = BlockchainSelectionData{{
        {"Bitcoin", true, false, Type::Bitcoin},
        {"Bitcoin Cash", false, false, Type::BitcoinCash},
        {"Bitcoin SV", false, false, Type::BitcoinSV},
        {"Dash", false, false, Type::Dash},
        {"Litecoin", false, false, Type::Litecoin},
        {"PKT", false, false, Type::PKT},
        {"eCash", false, false, Type::eCash},
    }};
    const auto expectedT = BlockchainSelectionData{{
        {"Bitcoin (testnet3)", false, true, Type::Bitcoin_testnet3},
        {"Bitcoin Cash (testnet3)", false, true, Type::BitcoinCash_testnet3},
        {"Bitcoin Cash (testnet4)", false, true, Type::BitcoinCash_testnet4},
        {"Bitcoin SV (testnet3)", false, true, Type::BitcoinSV_testnet3},
        {"Dash (testnet3)", false, true, Type::Dash_testnet3},
        {"Litecoin (testnet4)", false, true, Type::Litecoin_testnet4},
        {"eCash (testnet3)", false, true, Type::eCash_testnet3},
    }};

    ASSERT_TRUE(wait_for_counter(counter_full_));
    ASSERT_TRUE(wait_for_counter(counter_main_));
    ASSERT_TRUE(wait_for_counter(counter_test_));
    EXPECT_TRUE(check_blockchain_selection(
        client_, ot::ui::Blockchains::All, expectedA));
    EXPECT_TRUE(check_blockchain_selection_qt(
        client_, ot::ui::Blockchains::All, expectedA));
    EXPECT_TRUE(check_blockchain_selection(
        client_, ot::ui::Blockchains::Main, expectedM));
    EXPECT_TRUE(check_blockchain_selection_qt(
        client_, ot::ui::Blockchains::Main, expectedM));
    EXPECT_TRUE(check_blockchain_selection(
        client_, ot::ui::Blockchains::Test, expectedT));
    EXPECT_TRUE(check_blockchain_selection_qt(
        client_, ot::ui::Blockchains::Test, expectedT));
}

TEST_F(BlockchainSelector, disable_enabled)
{
    {
        counter_full_.expected_ += 1;
        counter_main_.expected_ += 1;

        EXPECT_TRUE(full_.Disable(Type::Bitcoin));
    }

    const auto expectedA = BlockchainSelectionData{{
        {"Bitcoin", false, false, Type::Bitcoin},
        {"Bitcoin Cash", false, false, Type::BitcoinCash},
        {"Bitcoin SV", false, false, Type::BitcoinSV},
        {"Dash", false, false, Type::Dash},
        {"Litecoin", false, false, Type::Litecoin},
        {"PKT", false, false, Type::PKT},
        {"eCash", false, false, Type::eCash},
        {"Bitcoin (testnet3)", false, true, Type::Bitcoin_testnet3},
        {"Bitcoin Cash (testnet3)", false, true, Type::BitcoinCash_testnet3},
        {"Bitcoin Cash (testnet4)", false, true, Type::BitcoinCash_testnet4},
        {"Bitcoin SV (testnet3)", false, true, Type::BitcoinSV_testnet3},
        {"Dash (testnet3)", false, true, Type::Dash_testnet3},
        {"Litecoin (testnet4)", false, true, Type::Litecoin_testnet4},
        {"eCash (testnet3)", false, true, Type::eCash_testnet3},
    }};
    const auto expectedM = BlockchainSelectionData{{
        {"Bitcoin", false, false, Type::Bitcoin},
        {"Bitcoin Cash", false, false, Type::BitcoinCash},
        {"Bitcoin SV", false, false, Type::BitcoinSV},
        {"Dash", false, false, Type::Dash},
        {"Litecoin", false, false, Type::Litecoin},
        {"PKT", false, false, Type::PKT},
        {"eCash", false, false, Type::eCash},
    }};
    const auto expectedT = BlockchainSelectionData{{
        {"Bitcoin (testnet3)", false, true, Type::Bitcoin_testnet3},
        {"Bitcoin Cash (testnet3)", false, true, Type::BitcoinCash_testnet3},
        {"Bitcoin Cash (testnet4)", false, true, Type::BitcoinCash_testnet4},
        {"Bitcoin SV (testnet3)", false, true, Type::BitcoinSV_testnet3},
        {"Dash (testnet3)", false, true, Type::Dash_testnet3},
        {"Litecoin (testnet4)", false, true, Type::Litecoin_testnet4},
        {"eCash (testnet3)", false, true, Type::eCash_testnet3},
    }};

    ASSERT_TRUE(wait_for_counter(counter_full_));
    ASSERT_TRUE(wait_for_counter(counter_main_));
    ASSERT_TRUE(wait_for_counter(counter_test_));
    EXPECT_TRUE(check_blockchain_selection(
        client_, ot::ui::Blockchains::All, expectedA));
    EXPECT_TRUE(check_blockchain_selection_qt(
        client_, ot::ui::Blockchains::All, expectedA));
    EXPECT_TRUE(check_blockchain_selection(
        client_, ot::ui::Blockchains::Main, expectedM));
    EXPECT_TRUE(check_blockchain_selection_qt(
        client_, ot::ui::Blockchains::Main, expectedM));
    EXPECT_TRUE(check_blockchain_selection(
        client_, ot::ui::Blockchains::Test, expectedT));
    EXPECT_TRUE(check_blockchain_selection_qt(
        client_, ot::ui::Blockchains::Test, expectedT));
}

TEST_F(BlockchainSelector, shutdown)
{
    EXPECT_EQ(counter_full_.expected_, counter_full_.updated_);
    EXPECT_EQ(counter_main_.expected_, counter_main_.updated_);
    EXPECT_EQ(counter_test_.expected_, counter_test_.updated_);
}
}  // namespace ottest
