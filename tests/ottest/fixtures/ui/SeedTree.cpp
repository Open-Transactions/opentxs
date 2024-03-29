// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/ui/SeedTree.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <compare>
#include <iterator>
#include <optional>

#include "internal/api/session/UI.hpp"
#include "internal/interface/ui/SeedTree.hpp"
#include "internal/interface/ui/SeedTreeItem.hpp"
#include "internal/interface/ui/SeedTreeNym.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "ottest/env/OTTestEnvironment.hpp"
#include "ottest/fixtures/common/Counter.hpp"
#include "ottest/fixtures/common/User.hpp"

namespace ottest
{
auto check_seed_tree_items(
    const ot::ui::SeedTreeItem& widget,
    const ot::UnallocatedVector<SeedTreeNym>& v) noexcept -> bool;
}  // namespace ottest

namespace ottest
{
Counter SeedTree::counter_{};
std::optional<User> SeedTree::alex_{std::nullopt};
std::optional<User> SeedTree::bob_{std::nullopt};
std::optional<User> SeedTree::chris_{std::nullopt};

SeedTree::SeedTree()
    : api_(OTTestEnvironment::GetOT().StartClientSession(0))
    , seed_1_id_(api_.Factory().SeedIDFromBase58(
          "ot2xku1FsJryQkxUnHpY7thRc7ActebtanLZcfTs3rGaUfyTefpsTne"))
    , seed_2_id_(api_.Factory().SeedIDFromBase58(
          "ot2xkue7cSsxNN4wGoz3wvoTNFyFTbMfLfXd8Bk6yqQE4awnhWsezNM"))
    , seed_3_id_(api_.Factory().SeedIDFromBase58(
          "ot2xkuFMFyL3JbDg8377CWX7eNNga7X7KHi11qF7xKZNTAYeXXwmgi3"))
    , reason_(api_.Factory().PasswordPrompt(__func__))
{
}

auto check_seed_tree(
    const ot::api::session::Client& api,
    const SeedTreeData& expected) noexcept -> bool
{
    const auto& widget = api.UI().Internal().SeedTree();
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
        output &= (row->SeedID() == it->id_);
        output &= (row->Name() == it->name_);
        output &= (row->Type() == it->type_);
        output &= check_seed_tree_items(row.get(), it->rows_);

        EXPECT_EQ(row->SeedID(), it->id_);
        EXPECT_EQ(row->Name(), it->name_);
        EXPECT_EQ(row->Type(), it->type_);

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

auto check_seed_tree_items(
    const ot::ui::SeedTreeItem& widget,
    const ot::UnallocatedVector<SeedTreeNym>& v) noexcept -> bool
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
        output &= (row->Index() == it->index_);
        output &= (row->Name() == it->name_);
        output &= (row->NymID() == it->id_);
        output &= (lastVector == lastRow);

        EXPECT_EQ(row->Index(), it->index_);
        EXPECT_EQ(row->Name(), it->name_);
        EXPECT_EQ(row->NymID(), it->id_);
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

auto init_seed_tree(
    const ot::api::session::Client& api,
    Counter& counter) noexcept -> void
{
    api.UI().Internal().SeedTree(make_cb(counter, "seed_tree"));
}

auto print_seed_tree(const ot::api::session::Client& api) noexcept
    -> ot::UnallocatedCString
{
    const auto& widget = api.UI().Internal().SeedTree();

    return widget.Debug();
}
}  // namespace ottest
