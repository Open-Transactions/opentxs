// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Lockable.hpp"

#include "internal/core/Core.hpp"
#include "Widget.hpp"

#include <future>
#include <type_traits>
#include <utility>

#define LIST_METHOD "opentxs::ui::implementation::List::"

namespace opentxs::ui::implementation
{
template <typename T>
struct reverse_display {
    static const bool value{false};
};
template <>
struct reverse_display<ActivitySummaryInternalInterface> {
    static const bool value{true};
};
template <>
struct reverse_display<AccountActivityInternalInterface> {
    static const bool value{true};
};
template <typename Map, typename T, typename Enable = void>
struct sort_order {
    using iterator = typename Map::const_iterator;
    static iterator begin(const Map& map) noexcept { return map.cbegin(); }
    static iterator end(const Map& map) noexcept { return map.cend(); }
};
template <typename Map, typename T>
struct sort_order<
    Map,
    T,
    typename std::enable_if<reverse_display<T>::value>::type> {
    using iterator = typename Map::const_reverse_iterator;
    static iterator begin(const Map& map) noexcept { return map.crbegin(); }
    static iterator end(const Map& map) noexcept { return map.crend(); }
};

template <
    typename ExternalInterface,
    typename InternalInterface,
    typename RowID,
    typename RowInterface,
    typename RowInternal,
    typename RowBlank,
    typename SortKey,
    typename PrimaryID>
class List : virtual public ExternalInterface,
             virtual public InternalInterface,
             public Widget,
             public Lockable
#if OT_QT
    ,
             virtual public QAbstractItemModel
#endif
{
#if OT_QT
public:
    // Data method needs this. And it needs to match the parent type for the Qt
    using qt_super = QAbstractItemModel;  // class that appears below this one.

    int columnCount(const QModelIndex& parent = QModelIndex()) const
        noexcept override
    {
        return column_count_;
    }
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole)
        const noexcept override
    {
        return {};
    }
    QModelIndex index(
        int row,
        int column,
        const QModelIndex& parent = QModelIndex()) const noexcept override
    {
        Lock lock(lock_);

        return get_index(lock, row, column, parent);
    }
    QModelIndex parent(const QModelIndex& index) const noexcept override
    {
        return {};
    }
    QHash<int, QByteArray> roleNames() const noexcept override
    {
        return qt_roles_;
    }
    int rowCount(const QModelIndex& parent = QModelIndex()) const
        noexcept override
    {
        return row_count_.load();
    }
#endif

public:
    using Inner = std::map<RowID, std::shared_ptr<RowInternal>>;
    using Outer = std::map<SortKey, Inner>;
    using Sort = sort_order<Outer, InternalInterface>;
    using OuterIterator = typename Sort::iterator;
#if OT_QT
    using Roles = QHash<int, QByteArray>;
#endif

    SharedPimpl<RowInterface> First() const noexcept override
    {
        Lock lock(lock_);

        return SharedPimpl<RowInterface>(first(lock));
    }
    virtual bool last(const RowID& id) const noexcept override
    {
        Lock lock(lock_);

        return start_.get() && same(id, last_id_);
    }
    SharedPimpl<RowInterface> Next() const noexcept override
    {
        Lock lock(lock_);

        if (start_.get()) { return SharedPimpl<RowInterface>(first(lock)); }

        return SharedPimpl<RowInterface>(next(lock));
    }
    OTIdentifier WidgetID() const noexcept override { return widget_id_; }

    ~List() override
    {
        if (startup_ && startup_->joinable()) {
            startup_->join();
            startup_.reset();
        }
    }

protected:
    using ReverseType = std::map<RowID, SortKey>;

#if OT_QT
    const bool enable_qt_;
    const Roles qt_roles_;
    const int column_count_;
    const int start_row_;
    mutable std::atomic<int> row_count_;
    const RowCallbacks insert_callbacks_;
    const RowCallbacks remove_callbacks_;
#endif
    const PrimaryID primary_id_;
    mutable Outer items_;
    mutable OuterIterator outer_;
    mutable typename Inner::const_iterator inner_;
    mutable ReverseType names_;
    mutable RowID last_id_;
    mutable OTFlag have_items_;
    mutable OTFlag start_;
    std::unique_ptr<std::thread> startup_{nullptr};
    const std::shared_ptr<const RowInternal> blank_p_{nullptr};
    const RowInternal& blank_;

#if OT_QT
    std::pair<bool, RowInterface*> check_index(const QModelIndex& index) const
        noexcept
    {
        std::pair<bool, RowInterface*> output{false, nullptr};
        // auto& [ valid, row ] = output;
        auto& valid = output.first;
        auto& row = output.second;

        if (false == index.isValid()) { return output; }

        if (columnCount() < index.column()) { return output; }

        if (nullptr == index.internalPointer()) { return output; }

        row = static_cast<RowInterface*>(index.internalPointer());
        valid = (nullptr != row);

        return output;
    }
#endif
    void delete_inactive(const std::set<RowID>& active) const noexcept
    {
        Lock lock(lock_);
        LogVerbose(LIST_METHOD)(__FUNCTION__)(": Removing ")(
            names_.size() - active.size())("items.")
            .Flush();
        std::vector<RowID> deleteIDs{};

        for (const auto& it : names_) {
            const auto& id = it.first;

            if (0 == active.count(id)) { deleteIDs.emplace_back(id); }
        }

        for (const auto& id : deleteIDs) { delete_item(lock, id); }

        OT_ASSERT(names_.size() == active.size())

        UpdateNotify();
    }
    void delete_item(const Lock& lock, const RowID& id) const noexcept
    {
        OT_ASSERT(verify_lock(lock));

        auto& key = names_.at(id);
        auto& inner = items_.at(key);

        auto item = inner.find(id);

        // I'm about to delete this row. Make sure iterators are not
        // pointing to it
        if (init_.load() && inner_ == item) { increment_inner(lock); }

        start_remove_row(lock, id);
        const auto itemDeleted = inner.erase(id);

        OT_ASSERT(1 == itemDeleted)

        if (0 == inner.size()) { items_.erase(key); }

        const auto indexDeleted = names_.erase(id);

        OT_ASSERT(1 == indexDeleted)

        finish_remove_row();
    }
    RowInternal& find_by_id(const Lock& lock, const RowID& id) const noexcept
    {
        OT_ASSERT(verify_lock(lock));

        try {
            auto& key = names_.at(id);
            auto& inner = items_.at(key);
            auto item = inner.find(id);

            if (inner.end() == item) {
                return const_cast<RowInternal&>(blank_);
            }

            OT_ASSERT(item->second)

            return *item->second;
        } catch (const std::out_of_range&) {
        }

        return const_cast<RowInternal&>(blank_);
    }

    /** Searches for the first name with at least one contact and sets
     *  iterators to match
     *
     *  If this function returns false, then no valid names are present and
     *  the values of outer_ and inner_ are undefined.
     */
    bool first_valid_item(const Lock& lock) const noexcept
    {
        OT_ASSERT(verify_lock(lock));

        if (0 == items_.size()) {
            outer_ = outer_first();
            inner_ = items_.begin()->second.begin();
            init_.store(false);

            return false;
        }

        outer_ = outer_first();

        while (outer_end() != outer_) {
            const auto& item = outer_->second;

            if (0 < item.size()) {
                inner_ = item.begin();
                init_.store(true);
                valid_iterators();

                return true;
            }

            ++outer_;
        }

        return false;
    }
    void wait_for_startup() const noexcept { startup_future_.get(); }

    virtual void add_item(
        const RowID& id,
        const SortKey& index,
        const CustomData& custom) noexcept
    {
        insert_outer(id, index, custom);
    }
    void finish_startup() noexcept
    {
        try {
            startup_promise_.set_value();
        } catch (...) {
        }
    }
    void init() noexcept { outer_ = outer_first(); }

    List(
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const typename PrimaryID::interface_type& primaryID,
        const Identifier& widgetID
#if OT_QT
        ,
        const bool qt,
        const RowCallbacks insertCallback,
        const RowCallbacks removeCallback,
        const Roles& roles = {},
        const int columns = 0,
        const int startRow = 0
#endif
        ) noexcept
        : Widget(api, publisher, widgetID)
#if OT_QT
        , enable_qt_(qt)
        , qt_roles_(roles)
        , column_count_(columns)
        , start_row_(startRow)
        , row_count_(startRow)
        , insert_callbacks_(insertCallback)
        , remove_callbacks_(removeCallback)
#endif
        , primary_id_(primaryID)
        , items_()
        , outer_(items_.begin())
        , inner_(items_.begin()->second.begin())
        , names_()
        , last_id_(make_blank<RowID>::value())
        , have_items_(Flag::Factory(false))
        , start_(Flag::Factory(true))
        , startup_(nullptr)
        , blank_p_(new RowBlank)
        , blank_(*blank_p_)
        , init_(false)
        , startup_promise_()
        , startup_future_(startup_promise_.get_future())
    {
        OT_ASSERT(blank_p_);
    }
    List(
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const typename PrimaryID::interface_type& primaryID
#if OT_QT
        ,
        const bool qt,
        const RowCallbacks insertCallback,
        const RowCallbacks removeCallback,
        const Roles& roles = {},
        const int columns = 0,
        const int startRow = 0
#endif
        ) noexcept
        : List(
              api,
              publisher,
              primaryID,
              Identifier::Random()
#if OT_QT
                  ,
              qt,
              insertCallback,
              removeCallback,
              roles,
              columns,
              startRow
#endif
          )
    {
    }

private:
    mutable std::atomic<bool> init_;
    std::promise<void> startup_promise_;
    std::shared_future<void> startup_future_;

    virtual void construct_row(
        const RowID& id,
        const SortKey& index,
        const CustomData& custom) const noexcept = 0;
    /** Returns item reference by the inner_ iterator. Does not increment
     *  iterators. */
    const std::shared_ptr<const RowInternal> current(const Lock& lock) const
    {
        OT_ASSERT(verify_lock(lock));

        valid_iterators();

        OT_ASSERT(init_.load());

        /* TODO: this line will cause a segfault in the clang-5 ast parser.
        const auto & [ id, item ] = *inner_;
        */
        const auto& id = std::get<0>(*inner_);
        const auto& item = std::get<1>(*inner_);
        last_id_ = id;

        OT_ASSERT(item)

        return item;
    }
#if OT_QT
    int find_delete_point(const Lock& lock, const RowID& id) const noexcept
    {
        OT_ASSERT(verify_lock(lock));

        int output{start_row_};

        for (const auto& [sortKey, map] : items_) {
            for (const auto& [rowID, pRow] : map) {
                if (rowID == id) {

                    return output;
                } else {
                    ++output;
                }
            }
        }

        return -1;
    }
    int find_insert_point(
        const Lock& lock,
        const RowID& id,
        const SortKey& index) const noexcept
    {
        OT_ASSERT(verify_lock(lock));

        int output{start_row_};

        for (const auto& [sortKey, map] : items_) {
            if (sortKey < index) {
                output += map.size();
            } else if (sortKey == index) {
                for (const auto& [rowID, pRow] : map) {
                    if (rowID == id) {

                        return output;
                    } else if (id < rowID) {

                        return output - 1;
                    } else {
                        ++output;
                    }
                }
            } else {
                break;
            }
        }

        return output;
    }
#endif
    void finish_insert_row() const noexcept
    {
#if OT_QT
        ++row_count_;

        if (enable_qt_ && insert_callbacks_.second) {
            insert_callbacks_.second();
        }
#endif
    }
    void finish_remove_row() const noexcept
    {
#if OT_QT
        --row_count_;

        if (enable_qt_ && remove_callbacks_.second) {
            remove_callbacks_.second();
        }
#endif
    }
    /** Returns first contact, or blank if none exists. Sets up iterators for
     *  next row
     */
    virtual std::shared_ptr<const RowInternal> first(const Lock& lock) const
        noexcept
    {
        OT_ASSERT(verify_lock(lock));

        have_items_->Set(first_valid_item(lock));
        start_->Set(!have_items_.get());

        if (have_items_.get()) {

            return next(lock);
        } else {
            last_id_ = make_blank<RowID>::value();

            return blank_p_;
        }
    }
#if OT_QT
    QModelIndex get_index(
        const Lock& lock,
        int row,
        int column,
        const QModelIndex& parent = QModelIndex()) const noexcept
    {
        if (columnCount() < column) { return {}; }

        int i{start_row_};
        RowInterface* item{nullptr};

        for (const auto& [sortKey, map] : items_) {
            for (const auto& [index, pRow] : map) {
                if (i == row) {
                    item = pRow.get();
                    goto exit;
                } else {
                    ++i;
                }
            }
        }

    exit:
        if (nullptr == item) { return {}; }

        return createIndex(row, column, item);
    }
#endif
    /** Increment iterators to the next valid item, or loop back to start */
    void increment_inner(const Lock& lock) const noexcept
    {
        valid_iterators();
        const auto& item = outer_->second;

        OT_ASSERT(init_.load());

        ++inner_;

        if (item.end() != inner_) {
            valid_iterators();

            return;
        }

        // The previous position was the last item for this index.
        increment_outer(lock);
    }
    /** Move to the next valid item, or loop back to beginning
     *
     *  inner_ is an invalid iterator at this point
     */
    bool increment_outer(const Lock& lock) const noexcept
    {
        OT_ASSERT(outer_end() != outer_)

        bool searching{true};

        while (searching) {
            ++outer_;

            if (outer_end() == outer_) {
                // End of the list. Both iterators are invalid at this point
                start_->On();
                have_items_->Set(first_valid_item(lock));

                if (have_items_.get()) { valid_iterators(); }

                return false;
            }

            const auto& item = outer_->second;

            if (0 < item.size()) {
                searching = false;
                inner_ = item.begin();
            }
        }

        valid_iterators();

        return true;
    }
#if OT_QT
    QModelIndex me() const noexcept
    {
        return createIndex(
            0,
            0,
            const_cast<ExternalInterface*>(
                static_cast<const ExternalInterface*>(this)));
    }
#endif
    /** Returns the next item and increments iterators */
    const std::shared_ptr<const RowInternal> next(const Lock& lock) const
        noexcept
    {
        const auto output = current(lock);
        increment_inner(lock);

        return output;
    }
    OuterIterator outer_first() const noexcept { return Sort::begin(items_); }
    OuterIterator outer_end() const noexcept { return Sort::end(items_); }
    void reindex_item(
        const Lock& lock,
        const RowID& id,
        const SortKey& oldIndex,
        const SortKey& newIndex,
        const CustomData& custom) const noexcept
    {
        OT_ASSERT(verify_lock(lock));
        OT_ASSERT(1 == items_.count(oldIndex))

        auto index = items_.find(oldIndex);

        OT_ASSERT(items_.end() != index);

        auto& itemMap = index->second;
        auto item = itemMap.find(id);

        OT_ASSERT(itemMap.end() != item);

        // I'm about to delete this row. Make sure iterators are not pointing
        // to it
        if (init_.load() && inner_ == item) { increment_inner(lock); }

        start_remove_row(lock, id);
        std::shared_ptr<RowInternal> row = std::move(item->second);
        const auto deleted = itemMap.erase(id);

        OT_ASSERT(1 == deleted)

        if (0 == itemMap.size()) { items_.erase(index); }

        finish_remove_row();
        start_insert_row(lock, id, newIndex);
        names_[id] = newIndex;
        row->reindex(newIndex, custom);
        items_[newIndex].emplace(id, std::move(row));
        finish_insert_row();
    }
    virtual bool same(const RowID& lhs, const RowID& rhs) const noexcept
    {
        return (lhs == rhs);
    }
    void start_insert_row(
        [[maybe_unused]] const Lock& lock,
        [[maybe_unused]] const RowID& id,
        [[maybe_unused]] const SortKey& index) const noexcept
    {
#if OT_QT
        if (enable_qt_ && insert_callbacks_.first) {
            const auto row = find_insert_point(lock, id, index);
            insert_callbacks_.first(me(), row, row);
        }
#endif
    }
    void start_remove_row(
        [[maybe_unused]] const Lock& lock,
        [[maybe_unused]] const RowID& id) const noexcept
    {
#if OT_QT
        if (enable_qt_ && remove_callbacks_.first) {
            const auto row = find_delete_point(lock, id);
            remove_callbacks_.first(me(), row, row);
        }
#endif
    }
    void valid_iterators() const noexcept
    {
        OT_ASSERT(outer_end() != outer_)

        if (init_.load()) {
            const auto& item = outer_->second;

            OT_ASSERT(item.end() != inner_)
        }
    }

    void insert_outer(
        const RowID& id,
        const SortKey& index,
        const CustomData& custom) noexcept
    {
        Lock lock(lock_);

        if (0 == names_.count(id)) {
            start_insert_row(lock, id, index);
            construct_row(id, index, custom);
            finish_insert_row();

            OT_ASSERT(1 == items_.count(index))
            OT_ASSERT(1 == names_.count(id))

            UpdateNotify();

            return;
        }

        const auto& oldIndex = names_.at(id);

        if (oldIndex == index) { return; }

        reindex_item(lock, id, oldIndex, index, custom);
        UpdateNotify();
    }

    List() = delete;
    List(const List&) = delete;
    List(List&&) = delete;
    List& operator=(const List&) = delete;
    List& operator=(List&&) = delete;
};
}  // namespace opentxs::ui::implementation
