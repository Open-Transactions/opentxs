// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/ui/AccountTree.hpp"

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <compare>
#include <iterator>
#include <sstream>
#include <string_view>

#include "internal/api/session/UI.hpp"
#include "internal/interface/ui/AccountCurrency.hpp"
#include "internal/interface/ui/AccountTree.hpp"
#include "internal/interface/ui/AccountTreeItem.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "ottest/fixtures/common/Counter.hpp"
#include "ottest/fixtures/common/User.hpp"

namespace ottest
{
using namespace opentxs::literals;

auto check_account_tree_items(
    const ot::ui::AccountCurrency& widget,
    const ot::UnallocatedVector<AccountTreeRow>& v) noexcept -> bool;
}  // namespace ottest

namespace ottest
{
auto check_account_tree(
    const User& user,
    const AccountTreeData& expected) noexcept -> bool
{
    const auto& widget = user.api_->UI().Internal().AccountTree(user.nym_id_);
    auto output{true};
    const auto& v = expected.rows_;
    auto row = widget.First();

    if (const auto valid = row->Valid(); 0 < v.size()) {
        output &= valid;

        EXPECT_TRUE(valid);

        if (false == valid) { return output; }
    } else {
        output &= (false == valid);

        EXPECT_FALSE(valid);
    }

    for (auto it{v.begin()}; it < v.end(); ++it, row = widget.Next()) {
        output &= (row->Name() == it->name_);
        output &= (row->Currency() == it->type_);
        output &= check_account_tree_items(row.get(), it->rows_);

        EXPECT_EQ(row->Name(), it->name_);
        EXPECT_EQ(row->Currency(), it->type_);

        const auto lastVector = std::next(it) == v.end();
        const auto lastRow = row->Last();
        output &= (lastVector == lastRow);

        if (lastVector) {
            EXPECT_TRUE(lastRow);
        } else {
            EXPECT_FALSE(lastRow);

            if (lastRow) { return output; }
        }
    }

    return output;
}

auto check_account_tree_items(
    const ot::ui::AccountCurrency& widget,
    const ot::UnallocatedVector<AccountTreeRow>& v) noexcept -> bool
{
    auto output{true};
    auto row = widget.First();

    if (const auto valid = row->Valid(); 0 < v.size()) {
        output &= valid;

        EXPECT_TRUE(valid);

        if (false == valid) { return output; }
    } else {
        output &= (false == valid);

        EXPECT_FALSE(valid);
    }

    for (auto it{v.begin()}; it < v.end(); ++it, row = widget.Next()) {
        const auto lastVector = std::next(it) == v.end();
        const auto lastRow = row->Last();
        output &= (row->AccountID() == it->account_id_);
        output &= (row->Balance() == it->balance_);
        output &= (row->ContractID() == it->contract_id_);
        output &= (row->DisplayBalance() == it->display_balance_);
        output &= (row->DisplayUnit() == it->display_unit_);
        output &= (row->Name() == it->name_);
        output &= (row->NotaryID() == it->notary_id_);
        output &= (row->NotaryName() == it->notary_name_);
        output &= (row->Type() == it->type_);
        output &= (row->Unit() == it->unit_);
        output &= (lastVector == lastRow);

        EXPECT_EQ(row->AccountID(), it->account_id_);
        EXPECT_EQ(row->Balance(), it->balance_);
        EXPECT_EQ(row->ContractID(), it->contract_id_);
        EXPECT_EQ(row->DisplayBalance(), it->display_balance_);
        EXPECT_EQ(row->DisplayUnit(), it->display_unit_);
        EXPECT_EQ(row->Name(), it->name_);
        EXPECT_EQ(row->NotaryID(), it->notary_id_);
        EXPECT_EQ(row->NotaryName(), it->notary_name_);
        EXPECT_EQ(row->Type(), it->type_);
        EXPECT_EQ(row->Unit(), it->unit_);
        EXPECT_EQ(lastVector, lastRow);

        if (lastVector) {
            EXPECT_TRUE(lastRow);
        } else {
            EXPECT_FALSE(lastRow);

            if (lastRow) { return output; }
        }
    }

    return output;
}

auto init_account_tree(const User& user, Counter& counter) noexcept -> void
{
    user.api_->UI().Internal().AccountTree(user.nym_id_, make_cb(counter, [&] {
                                               auto out = std::stringstream{};
                                               out << u8"account_tree_"_sv;
                                               out << user.name_lower_;

                                               return out.str();
                                           }()));
    wait_for_counter(counter);
}

auto print_account_tree(const User& user) noexcept -> ot::UnallocatedCString
{
    const auto& widget = user.api_->UI().Internal().AccountTree(user.nym_id_);

    return widget.Debug();
}
}  // namespace ottest
