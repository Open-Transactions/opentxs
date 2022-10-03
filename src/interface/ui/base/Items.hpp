// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <algorithm>
#include <functional>
#include <optional>

#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
namespace internal
{
struct Row;
}  // namespace internal
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
template <typename RowID, typename SortKey, typename RowPointer>
class ListItems
{
public:
    struct Row {
        SortKey key_;
        RowID id_;
        RowPointer item_;
    };
    using Data = UnallocatedList<Row>;
    using Iterator = typename Data::iterator;
    using Index = UnallocatedMap<RowID, Iterator>;
    using Insert = std::pair<Iterator, internal::Row*>;
    using Position = std::pair<Iterator, std::size_t>;
    using Move = std::pair<Insert, Insert>;

    auto active() const noexcept -> UnallocatedVector<RowID>
    {
        auto output = UnallocatedVector<RowID>{};
        output.reserve(index_.size());
        std::transform(
            index_.begin(), index_.end(), std::back_inserter(output), [
            ](const auto& in) -> auto{ return in.first; });

        return output;
    }
    auto last(const RowID& row) const noexcept -> bool
    {
        if (0u == data_.size()) { return true; }

        if (auto i = index_.find(row); index_.end() == i) { return true; }

        const auto& [key, id, item] = data_.back();

        return row == id;
    }
    auto size() const noexcept { return data_.size(); }

    auto at(const std::size_t pos) -> Row&
    {
        if (pos < offset_) {
            throw std::out_of_range("Invalid position (offset)");
        }

        const auto eff = pos - offset_;

        if ((0 == data_.size()) || ((data_.size() - 1) < eff)) {
            throw std::out_of_range("Invalid position");
        }

        auto output = data_.begin();
        std::advance(output, eff);

        return *output;
    }
    auto get(const RowID& id) -> Row& { return *index_.at(id); }
    auto begin() noexcept -> Iterator { return data_.begin(); }
    auto delete_row(const RowID& id, Iterator position) noexcept -> void
    {
        data_.erase(position);
        index_.erase(id);
    }
    auto end() noexcept -> Iterator { return data_.end(); }
    auto find_delete_position(const RowID& id) noexcept
        -> std::optional<Position>
    {
        try {
            const auto it = index_.at(id);

            return Position{
                it,
                static_cast<std::size_t>(std::distance(data_.begin(), it)) +
                    offset_};

        } catch (...) {

            return std::nullopt;
        }
    }
    auto find_insert_position(const SortKey& key, const RowID& id) noexcept
        -> Insert
    {
        auto output = Insert{data_.end(), nullptr};
        auto& [it, before] = output;

        for (auto i{data_.begin()}; i != data_.end(); ++i) {
            const auto& [rKey, rId, item] = *i;

            if (sort(key, id, rKey, rId)) {

                continue;
            } else {
                it = i;

                break;
            }
        }

        if (data_.begin() != it) {
            const auto& [rKey, rId, item] = *std::prev(it);
            before = item.get();
        }

        return output;
    }
    auto find_move_position(
        const RowID& oldId,
        const SortKey& newKey,
        const RowID& newID) noexcept -> std::optional<Move>
    {
        auto output = Move{{data_.end(), nullptr}, {data_.end(), nullptr}};
        auto& [from, to] = output;
        auto& [it, before] = to;

        try {
            const auto it = index_.at(oldId);
            from = Insert{it, [&]() -> internal::Row* {
                              if (data_.begin() == it) {

                                  return nullptr;
                              } else {

                                  return std::prev(it)->item_.get();
                              }
                          }()};
        } catch (...) {

            return std::nullopt;
        }

        for (auto i{data_.begin()}; i != data_.end(); ++i) {
            const auto& [rKey, rId, item] = *i;

            if (sort(newKey, newID, rKey, rId)) {
                before = item.get();

                continue;
            }

            it = i;

            break;
        }

        return output;
    }
    auto get_index(const RowID& id) noexcept -> std::optional<std::size_t>
    {
        try {

            return static_cast<std::size_t>(
                       std::distance(data_.begin(), index_.at(id))) +
                   offset_;
        } catch (...) {

            return std::nullopt;
        }
    }
    auto insert_before(
        const Iterator& position,
        const SortKey& key,
        const RowID& id,
        const RowPointer& item) noexcept -> RowPointer
    {
        auto& index = index_[id];
        index = data_.emplace(position, Row{key, id, item});

        return index->item_;
    }
    auto move_before(
        const RowID& oldId,
        Iterator oldPosition,
        const SortKey& newKey,
        const RowID& newID,
        Iterator newPosition) noexcept -> void
    {
        index_.erase(oldId);
        auto& oldData = *oldPosition;
        auto& index = index_[newID];
        index = data_.emplace(newPosition, Row{newKey, newID, oldData.item_});
        data_.erase(oldPosition);
    }

    ListItems(std::size_t offset, bool reverse) noexcept
        : offset_(offset)
        , reverse_sort_(reverse)
        , data_()
        , index_()
    {
    }

private:
    const std::size_t offset_;
    const bool reverse_sort_;
    Data data_;
    Index index_;

    auto compare_id(const RowID& lhs, const RowID& rhs) const noexcept -> bool;
    auto compare_key(const SortKey& lhs, const SortKey& rhs) const noexcept
        -> bool;

    auto sort(
        const SortKey& incomingKey,
        const RowID& incomingID,
        const SortKey& existingKey,
        const RowID& existingID) const noexcept -> bool
    {
        const auto& lKey = reverse_sort_ ? incomingKey : existingKey;
        const auto& rKey = reverse_sort_ ? existingKey : incomingKey;
        const auto& lID = reverse_sort_ ? incomingID : existingID;
        const auto& rID = reverse_sort_ ? existingID : incomingID;

        if (compare_key(lKey, rKey)) {

            return true;
        } else {

            return (lKey == rKey) && compare_id(lID, rID);
        }
    }
};
}  // namespace opentxs::ui::implementation
