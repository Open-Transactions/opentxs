// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstdlib>
#include <functional>
#include <iosfwd>
#include <memory>
#include <optional>
#include <utility>

#include "interface/ui/base/Items.hpp"
#include "internal/interface/ui/UI.hpp"
#include "ottest/fixtures/ui/Items.hpp"

namespace ot = opentxs;

namespace opentxs::ui::implementation
{
template <>
auto ListItems<ottest::ID, ottest::Key, std::shared_ptr<ottest::Value>>::
    compare_id(const ottest::ID& lhs, const ottest::ID& rhs) const noexcept
    -> bool
{
    static const auto compare = std::less<ottest::ID>{};

    return compare(lhs, rhs);
}

template <>
auto ListItems<ottest::ID, ottest::Key, std::shared_ptr<ottest::Value>>::
    compare_key(const ottest::Key& lhs, const ottest::Key& rhs) const noexcept
    -> bool
{
    static const auto compare = std::less<ottest::Key>{};

    return compare(lhs, rhs);
}
}  // namespace opentxs::ui::implementation

namespace ottest
{
Type items_{0, false};
const Vector vector_{
    {"a", 0, "first"},
    {"b", 1, "second"},
    {"c", 2, "third"},
    {"d", 3, "fourth"},
    {"d", 9, "fifth"},
    {"f", 5, "sixth"},
};
const Vector revised_{
    {"a", -1, vector_.at(2).value_},
    {"d", 6, vector_.at(1).value_},
};

auto test_row(const std::size_t pos, const Data& expected) -> bool;
auto test_row(Type& items, const std::size_t pos, const Data& expected) -> bool;

auto test_row(const std::size_t pos, const Data& expected) -> bool
{
    return test_row(items_, pos, expected);
}
auto test_row(Type& items, const std::size_t pos, const Data& expected) -> bool
{
    const auto index = items.get_index(expected.id_);

    EXPECT_TRUE(index);

    if (false == index.has_value()) { return false; }

    EXPECT_EQ(index.value(), pos);

    if (index.value() != pos) { return false; }

    try {
        const auto& data = items.at(pos);

        {
            const auto& row = items.get(expected.id_);

            EXPECT_EQ(data.key_, row.key_);
            EXPECT_EQ(data.id_, row.id_);
            EXPECT_TRUE(data.item_);
            EXPECT_TRUE(row.item_);

            if ((false == bool(data.item_)) || (false == bool(row.item_))) {
                return false;
            }

            EXPECT_EQ(*data.item_, *row.item_);
        }

        EXPECT_EQ(data.key_, expected.key_);
        EXPECT_EQ(data.id_, expected.id_);
        EXPECT_TRUE(data.item_);

        if (false == bool(data.item_)) { return false; }

        EXPECT_EQ(*data.item_, expected.value_);
    } catch (...) {
        EXPECT_TRUE(false);

        return false;
    }

    return true;
}

TEST(UI_items, empty)
{
    const auto active = Active{};

    EXPECT_EQ(items_.begin(), items_.end());
    EXPECT_EQ(items_.size(), 0);
    EXPECT_FALSE(items_.get_index(vector_.at(0).id_));
    EXPECT_FALSE(items_.find_delete_position(vector_.at(0).id_));
    EXPECT_FALSE(items_.find_move_position(
        vector_.at(0).id_, vector_.at(0).key_, vector_.at(0).id_));
    EXPECT_EQ(items_.active(), active);

    try {
        items_.at(0);

        EXPECT_TRUE(false);
    } catch (...) {
        EXPECT_TRUE(true);
    }
}

TEST(UI_items, insert_first_item)
{
    const auto& [key, id, value] = vector_.at(2);
    const auto [it, prev] = items_.find_insert_position(key, id);

    EXPECT_EQ(it, items_.end());
    EXPECT_EQ(prev, nullptr);

    items_.insert_before(it, key, id, std::make_shared<Value>(value));
    const auto active = Active{vector_.at(2).id_};

    EXPECT_NE(items_.begin(), items_.end());
    EXPECT_EQ(items_.size(), 1);
    EXPECT_TRUE(test_row(0, vector_.at(2)));
    EXPECT_EQ(items_.active(), active);

    try {
        items_.at(0);

        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }

    try {
        items_.at(1);

        EXPECT_TRUE(false);
    } catch (...) {
        EXPECT_TRUE(true);
    }
}

TEST(UI_items, insert_before)
{
    {
        const auto& [key, id, value] = vector_.at(0);
        const auto [it, prev] = items_.find_insert_position(key, id);

        ASSERT_NE(it, items_.end());
        EXPECT_EQ(it->id_, vector_.at(2).id_);
        EXPECT_EQ(prev, nullptr);

        items_.insert_before(it, key, id, std::make_shared<Value>(value));
        const auto active = Active{vector_.at(0).id_, vector_.at(2).id_};

        EXPECT_NE(items_.begin(), items_.end());
        EXPECT_EQ(items_.size(), 2);
        EXPECT_TRUE(test_row(0, vector_.at(0)));
        EXPECT_TRUE(test_row(1, vector_.at(2)));
        EXPECT_EQ(items_.active(), active);
    }
    {

        const auto& [key, id, value] = vector_.at(1);
        const auto [it, prev] = items_.find_insert_position(key, id);

        ASSERT_NE(it, items_.end());
        EXPECT_EQ(it->id_, vector_.at(2).id_);
        EXPECT_EQ(prev, items_.at(0).item_.get());

        items_.insert_before(it, key, id, std::make_shared<Value>(value));
        const auto active =
            Active{vector_.at(0).id_, vector_.at(1).id_, vector_.at(2).id_};

        EXPECT_NE(items_.begin(), items_.end());
        EXPECT_EQ(items_.size(), 3);
        EXPECT_TRUE(test_row(0, vector_.at(0)));
        EXPECT_TRUE(test_row(1, vector_.at(1)));
        EXPECT_TRUE(test_row(2, vector_.at(2)));
        EXPECT_EQ(items_.active(), active);
    }
}

TEST(UI_items, insert_after)
{
    {
        const auto& [key, id, value] = vector_.at(4);
        const auto [it, prev] = items_.find_insert_position(key, id);

        EXPECT_EQ(it, items_.end());
        EXPECT_EQ(prev, items_.at(2).item_.get());

        items_.insert_before(it, key, id, std::make_shared<Value>(value));
        const auto active = Active{
            vector_.at(0).id_,
            vector_.at(1).id_,
            vector_.at(2).id_,
            vector_.at(4).id_};

        EXPECT_NE(items_.begin(), items_.end());
        EXPECT_EQ(items_.size(), 4);
        EXPECT_TRUE(test_row(0, vector_.at(0)));
        EXPECT_TRUE(test_row(1, vector_.at(1)));
        EXPECT_TRUE(test_row(2, vector_.at(2)));
        EXPECT_TRUE(test_row(3, vector_.at(4)));
        EXPECT_EQ(items_.active(), active);
    }
    {
        const auto& [key, id, value] = vector_.at(3);
        const auto [it, prev] = items_.find_insert_position(key, id);

        ASSERT_NE(it, items_.end());
        EXPECT_EQ(it->id_, vector_.at(4).id_);
        EXPECT_EQ(prev, items_.at(2).item_.get());

        items_.insert_before(it, key, id, std::make_shared<Value>(value));
        const auto active = Active{
            vector_.at(0).id_,
            vector_.at(1).id_,
            vector_.at(2).id_,
            vector_.at(3).id_,
            vector_.at(4).id_};

        EXPECT_NE(items_.begin(), items_.end());
        EXPECT_EQ(items_.size(), 5);
        EXPECT_TRUE(test_row(0, vector_.at(0)));
        EXPECT_TRUE(test_row(1, vector_.at(1)));
        EXPECT_TRUE(test_row(2, vector_.at(2)));
        EXPECT_TRUE(test_row(3, vector_.at(3)));
        EXPECT_TRUE(test_row(4, vector_.at(4)));
        EXPECT_EQ(items_.active(), active);
    }
    {
        const auto& [key, id, value] = vector_.at(5);
        const auto [it, prev] = items_.find_insert_position(key, id);

        EXPECT_EQ(it, items_.end());
        EXPECT_EQ(prev, items_.at(4).item_.get());

        items_.insert_before(it, key, id, std::make_shared<Value>(value));
        const auto active = Active{
            vector_.at(0).id_,
            vector_.at(1).id_,
            vector_.at(2).id_,
            vector_.at(3).id_,
            vector_.at(5).id_,
            vector_.at(4).id_};

        EXPECT_NE(items_.begin(), items_.end());
        EXPECT_EQ(items_.size(), 6);
        EXPECT_TRUE(test_row(0, vector_.at(0)));
        EXPECT_TRUE(test_row(1, vector_.at(1)));
        EXPECT_TRUE(test_row(2, vector_.at(2)));
        EXPECT_TRUE(test_row(3, vector_.at(3)));
        EXPECT_TRUE(test_row(4, vector_.at(4)));
        EXPECT_TRUE(test_row(5, vector_.at(5)));
        EXPECT_EQ(items_.active(), active);
    }
}

TEST(UI_items, move_up)
{
    const auto& newKey = revised_.at(0).key_;
    const auto& newID = revised_.at(0).id_;
    const auto& oldID = vector_.at(2).id_;
    auto move = items_.find_move_position(oldID, newKey, newID);

    ASSERT_TRUE(move);

    auto& [before, after] = move.value();

    {
        const auto& [it, prev] = before;

        ASSERT_NE(it, items_.end());
        EXPECT_EQ(it->id_, oldID);
        EXPECT_EQ(prev, items_.at(1).item_.get());
    }
    {
        const auto& [it, prev] = after;

        ASSERT_NE(it, items_.end());
        EXPECT_EQ(it->id_, vector_.at(0).id_);
        EXPECT_EQ(prev, nullptr);
    }

    items_.move_before(oldID, before.first, newKey, newID, after.first);
    const auto active = Active{
        revised_.at(0).id_,
        vector_.at(0).id_,
        vector_.at(1).id_,
        vector_.at(3).id_,
        vector_.at(5).id_,
        vector_.at(4).id_};

    EXPECT_EQ(items_.size(), 6);
    EXPECT_TRUE(test_row(0, revised_.at(0)));
    EXPECT_TRUE(test_row(1, vector_.at(0)));
    EXPECT_TRUE(test_row(2, vector_.at(1)));
    EXPECT_TRUE(test_row(3, vector_.at(3)));
    EXPECT_TRUE(test_row(4, vector_.at(4)));
    EXPECT_TRUE(test_row(5, vector_.at(5)));
    EXPECT_EQ(items_.active(), active);
}

TEST(UI_items, move_down)
{
    const auto& newKey = revised_.at(1).key_;
    const auto& newID = revised_.at(1).id_;
    const auto& oldID = vector_.at(1).id_;
    auto move = items_.find_move_position(oldID, newKey, newID);

    ASSERT_TRUE(move);

    auto& [before, after] = move.value();

    {
        const auto& [it, prev] = before;

        ASSERT_NE(it, items_.end());
        EXPECT_EQ(it->id_, oldID);
        EXPECT_EQ(prev, items_.at(1).item_.get());
    }
    {
        const auto& [it, prev] = after;

        ASSERT_NE(it, items_.end());
        EXPECT_EQ(it->id_, vector_.at(4).id_);
        EXPECT_EQ(prev, items_.at(3).item_.get());
    }

    items_.move_before(oldID, before.first, newKey, newID, after.first);
    const auto active = Active{
        revised_.at(0).id_,
        vector_.at(0).id_,
        vector_.at(3).id_,
        vector_.at(5).id_,
        revised_.at(1).id_,
        vector_.at(4).id_};

    EXPECT_EQ(items_.size(), 6);
    EXPECT_TRUE(test_row(0, revised_.at(0)));
    EXPECT_TRUE(test_row(1, vector_.at(0)));
    EXPECT_TRUE(test_row(2, vector_.at(3)));
    EXPECT_TRUE(test_row(3, revised_.at(1)));
    EXPECT_TRUE(test_row(4, vector_.at(4)));
    EXPECT_TRUE(test_row(5, vector_.at(5)));
    EXPECT_EQ(items_.active(), active);
}

TEST(UI_items, delete_middle)
{
    const auto& id = revised_.at(1).id_;
    auto position = items_.find_delete_position(id);

    ASSERT_TRUE(position);

    auto& [it, index] = position.value();

    ASSERT_NE(it, items_.end());
    EXPECT_EQ(it->id_, id);
    EXPECT_EQ(index, 3);

    items_.delete_row(id, it);
    const auto active = Active{
        revised_.at(0).id_,
        vector_.at(0).id_,
        vector_.at(3).id_,
        vector_.at(5).id_,
        vector_.at(4).id_};

    EXPECT_EQ(items_.size(), 5);
    EXPECT_TRUE(test_row(0, revised_.at(0)));
    EXPECT_TRUE(test_row(1, vector_.at(0)));
    EXPECT_TRUE(test_row(2, vector_.at(3)));
    EXPECT_TRUE(test_row(3, vector_.at(4)));
    EXPECT_TRUE(test_row(4, vector_.at(5)));
    EXPECT_FALSE(items_.find_delete_position(id));
    EXPECT_FALSE(items_.get_index(id));
    EXPECT_EQ(items_.active(), active);
}

TEST(UI_items, delete_first)
{
    const auto& id = revised_.at(0).id_;
    auto position = items_.find_delete_position(id);

    ASSERT_TRUE(position);

    auto& [it, index] = position.value();

    ASSERT_NE(it, items_.end());
    EXPECT_EQ(it->id_, id);
    EXPECT_EQ(index, 0);

    items_.delete_row(id, it);
    const auto active = Active{
        vector_.at(0).id_,
        vector_.at(3).id_,
        vector_.at(5).id_,
        vector_.at(4).id_};

    EXPECT_EQ(items_.size(), 4);
    EXPECT_TRUE(test_row(0, vector_.at(0)));
    EXPECT_TRUE(test_row(1, vector_.at(3)));
    EXPECT_TRUE(test_row(2, vector_.at(4)));
    EXPECT_TRUE(test_row(3, vector_.at(5)));
    EXPECT_FALSE(items_.find_delete_position(id));
    EXPECT_FALSE(items_.get_index(id));
    EXPECT_EQ(items_.active(), active);
}

TEST(UI_items, delete_last)
{
    const auto& id = vector_.at(5).id_;
    auto position = items_.find_delete_position(id);

    ASSERT_TRUE(position);

    auto& [it, index] = position.value();

    ASSERT_NE(it, items_.end());
    EXPECT_EQ(it->id_, id);
    EXPECT_EQ(index, 3);

    items_.delete_row(id, it);
    const auto active =
        Active{vector_.at(0).id_, vector_.at(3).id_, vector_.at(4).id_};

    EXPECT_EQ(items_.size(), 3);
    EXPECT_TRUE(test_row(0, vector_.at(0)));
    EXPECT_TRUE(test_row(1, vector_.at(3)));
    EXPECT_TRUE(test_row(2, vector_.at(4)));
    EXPECT_FALSE(items_.find_delete_position(id));
    EXPECT_FALSE(items_.get_index(id));
    EXPECT_EQ(items_.active(), active);
}

TEST(UI_items, reverse_sort)
{
    auto items = Type{0, true};
    {
        const auto& [key, id, value] = vector_.at(0);
        const auto [it, index] = items.find_insert_position(key, id);
        items.insert_before(it, key, id, std::make_shared<Value>(value));
    }
    {
        const auto& [key, id, value] = vector_.at(1);
        const auto [it, index] = items.find_insert_position(key, id);
        items.insert_before(it, key, id, std::make_shared<Value>(value));
    }
    {
        const auto& [key, id, value] = vector_.at(2);
        const auto [it, index] = items.find_insert_position(key, id);
        items.insert_before(it, key, id, std::make_shared<Value>(value));
    }
    {
        const auto& [key, id, value] = vector_.at(3);
        const auto [it, index] = items.find_insert_position(key, id);
        items.insert_before(it, key, id, std::make_shared<Value>(value));
    }
    {
        const auto& [key, id, value] = vector_.at(4);
        const auto [it, index] = items.find_insert_position(key, id);
        items.insert_before(it, key, id, std::make_shared<Value>(value));
    }
    {
        const auto& [key, id, value] = vector_.at(5);
        const auto [it, index] = items.find_insert_position(key, id);
        items.insert_before(it, key, id, std::make_shared<Value>(value));
    }

    EXPECT_TRUE(test_row(items, 0, vector_.at(5)));
    EXPECT_TRUE(test_row(items, 1, vector_.at(4)));
    EXPECT_TRUE(test_row(items, 2, vector_.at(3)));
    EXPECT_TRUE(test_row(items, 3, vector_.at(2)));
    EXPECT_TRUE(test_row(items, 4, vector_.at(1)));
    EXPECT_TRUE(test_row(items, 5, vector_.at(0)));
}
}  // namespace ottest
