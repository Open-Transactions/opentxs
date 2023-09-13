// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <atomic>
#include <memory>
#include <utility>

#include "ottest/fixtures/common/Counter.hpp"
#include "ottest/fixtures/ui/NymList.hpp"

namespace ottest
{
TEST_F(NymList, initialize_opentxs) { init_nym_list(api_, counter_); }

TEST_F(NymList, empty)
{
    ASSERT_TRUE(wait_for_counter(counter_));
    EXPECT_TRUE(check_nym_list(api_, expected_));
    EXPECT_TRUE(check_nym_list_qt(api_, expected_));
}

TEST_F(NymList, add_chris)
{
    counter_.expected_ += 1;
    const auto nym =
        api_.Wallet().Nym({api_.Factory(), {}, 0}, reason_, chris_);

    ASSERT_TRUE(nym);

    const auto& id = nym->ID();
    auto name = nym->Name();

    EXPECT_FALSE(id.empty());
    EXPECT_EQ(nym->Name(), chris_);

    expected_.rows_.emplace_back(
        NymListRow{id.asBase58(api_.Crypto()), std::move(name)});

    ASSERT_TRUE(wait_for_counter(counter_));
    EXPECT_TRUE(check_nym_list(api_, expected_));
    EXPECT_TRUE(check_nym_list_qt(api_, expected_));
}

TEST_F(NymList, add_daniel)
{
    counter_.expected_ += 1;
    const auto nym =
        api_.Wallet().Nym({api_.Factory(), {}, 1}, reason_, daniel_);

    ASSERT_TRUE(nym);

    const auto& id = nym->ID();
    auto name = nym->Name();

    EXPECT_FALSE(id.empty());
    EXPECT_EQ(nym->Name(), daniel_);

    expected_.rows_.emplace_back(
        NymListRow{id.asBase58(api_.Crypto()), std::move(name)});

    ASSERT_TRUE(wait_for_counter(counter_));
    EXPECT_TRUE(check_nym_list(api_, expected_));
    EXPECT_TRUE(check_nym_list_qt(api_, expected_));
}

TEST_F(NymList, shutdown) { EXPECT_EQ(counter_.expected_, counter_.updated_); }
}  // namespace ottest
