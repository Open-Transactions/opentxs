// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <atomic>
#include <memory>
#include <utility>

#include "opentxs/OT.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Parameters.hpp"  // IWYU pragma: keep
#include "opentxs/identity/Nym.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "ui/Helpers.hpp"

namespace ottest
{
class Test_NymList : public ::testing::Test
{
public:
    static constexpr auto alex_{"Alex"};
    static constexpr auto chris_{"Chris"};
    static constexpr auto daniel_{"Daniel"};

    static Counter counter_;
    static NymListData expected_;

    const ot::api::session::Client& api_;
    ot::OTPasswordPrompt reason_;

    Test_NymList()
        : api_(ot::Context().StartClientSession(0))
        , reason_(api_.Factory().PasswordPrompt(__func__))
    {
    }
};

Counter Test_NymList::counter_{};
NymListData Test_NymList::expected_{};

TEST_F(Test_NymList, initialize_opentxs) { init_nym_list(api_, counter_); }

TEST_F(Test_NymList, empty)
{
    ASSERT_TRUE(wait_for_counter(counter_));
    EXPECT_TRUE(check_nym_list(api_, expected_));
    EXPECT_TRUE(check_nym_list_qt(api_, expected_));
}

TEST_F(Test_NymList, add_chris)
{
    counter_.expected_ += 1;
    const auto nym = api_.Wallet().Nym({"", 0}, reason_, chris_);

    ASSERT_TRUE(nym);

    const auto& id = nym->ID();
    auto name = nym->Name();

    EXPECT_FALSE(id.empty());
    EXPECT_EQ(nym->Name(), chris_);

    expected_.rows_.emplace_back(NymListRow{id.str(), std::move(name)});

    ASSERT_TRUE(wait_for_counter(counter_));
    EXPECT_TRUE(check_nym_list(api_, expected_));
    EXPECT_TRUE(check_nym_list_qt(api_, expected_));
}

TEST_F(Test_NymList, add_daniel)
{
    counter_.expected_ += 1;
    const auto nym = api_.Wallet().Nym({"", 1}, reason_, daniel_);

    ASSERT_TRUE(nym);

    const auto& id = nym->ID();
    auto name = nym->Name();

    EXPECT_FALSE(id.empty());
    EXPECT_EQ(nym->Name(), daniel_);

    expected_.rows_.emplace_back(NymListRow{id.str(), std::move(name)});

    ASSERT_TRUE(wait_for_counter(counter_));
    EXPECT_TRUE(check_nym_list(api_, expected_));
    EXPECT_TRUE(check_nym_list_qt(api_, expected_));
}

TEST_F(Test_NymList, shutdown)
{
    EXPECT_EQ(counter_.expected_, counter_.updated_);
}
}  // namespace ottest
