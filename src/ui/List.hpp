// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "Widget.hpp"

#include <type_traits>
#include <utility>

#define STARTUP_WAIT_MILLISECONDS 100

#define LIST_METHOD "opentxs::ui::implementation::List::"

namespace opentxs::ui::implementation
{
template <typename T>
struct make_blank {
    static T value() { return T{}; }
};
template <>
struct make_blank<OTIdentifier> {
    static OTIdentifier value() { return Identifier::Factory(); }
};
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
    static iterator begin(const Map& map) { return map.cbegin(); }
    static iterator end(const Map& map) { return map.cend(); }
};
template <typename Map, typename T>
struct sort_order<
    Map,
    T,
    typename std::enable_if<reverse_display<T>::value>::type> {
    using iterator = typename Map::const_reverse_iterator;
    static iterator begin(const Map& map) { return map.crbegin(); }
    static iterator end(const Map& map) { return map.crend(); }
};

template <
    typename ExternalInterface,
    typename InternalInterface,
    typename RowID,
    typename RowInterface,
    typename RowInternal,
    typename RowBlank,
    typename SortKey>
class List : virtual public ExternalInterface,
             virtual public InternalInterface,
             public Widget,
             public Lockable
{
public:
    using Inner = std::map<RowID, std::shared_ptr<RowInternal>>;
    using Outer = std::map<SortKey, Inner>;
    using Sort = sort_order<Outer, InternalInterface>;
    using OuterIterator = typename Sort::iterator;

    SharedPimpl<RowInterface> First() const override
    {
        Lock lock(lock_);

        return SharedPimpl<RowInterface>(first(lock));
    }

    virtual bool last(const RowID& id) const override
    {
        Lock lock(lock_);

        return start_.get() && same(id, last_id_);
    }

    SharedPimpl<RowInterface> Next() const override
    {
        Lock lock(lock_);

        if (start_.get()) { return SharedPimpl<RowInterface>(first(lock)); }

        return SharedPimpl<RowInterface>(next(lock));
    }

    OTIdentifier WidgetID() const override { return widget_id_; }

    virtual ~List()
    {
        if (startup_ && startup_->joinable()) {
            startup_->join();
            startup_.reset();
        }
    }

protected:
    using ReverseType = std::map<RowID, SortKey>;

    const OTNymID nym_id_;
    mutable Outer items_;
    mutable OuterIterator outer_;
    mutable typename Inner::const_iterator inner_;
    mutable ReverseType names_;
    mutable RowID last_id_;
    mutable OTFlag have_items_;
    mutable OTFlag start_;
    mutable OTFlag startup_complete_;
    std::unique_ptr<std::thread> startup_{nullptr};
    const std::shared_ptr<const RowInternal> blank_p_{nullptr};
    const RowInternal& blank_;

    virtual void construct_row(
        const RowID& id,
        const SortKey& index,
        const CustomData& custom) const = 0;
    /** Returns item reference by the inner_ iterator. Does not increment
     *  iterators. */
    const std::shared_ptr<const RowInternal> current(const Lock& lock) const
    {
        OT_ASSERT(verify_lock(lock))

        valid_iterators();
        /* TODO: this line will cause a segfault in the clang-5 ast parser.
        const auto & [ id, item ] = *inner_;
        */
        const auto& id = std::get<0>(*inner_);
        const auto& item = std::get<1>(*inner_);
        last_id_ = id;

        OT_ASSERT(item)

        return item;
    }
    void delete_inactive(const std::set<RowID>& active) const
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
    void delete_item(const Lock& lock, const RowID& id) const
    {
        OT_ASSERT(verify_lock(lock))

        auto& key = names_.at(id);
        auto& inner = items_.at(key);
        auto item = inner.find(id);

        // I'm about to delete this row. Make sure iterators are not pointing
        // to it
        if (inner_ == item) { increment_inner(lock); }

        const auto itemDeleted = inner.erase(id);

        OT_ASSERT(1 == itemDeleted)

        if (0 == inner.size()) { items_.erase(key); }

        const auto indexDeleted = names_.erase(id);

        OT_ASSERT(1 == indexDeleted)
    }
    /** Returns first contact, or blank if none exists. Sets up iterators for
     *  next row
     */
    virtual std::shared_ptr<const RowInternal> first(const Lock& lock) const
    {
        OT_ASSERT(verify_lock(lock))

        have_items_->Set(first_valid_item(lock));
        start_->Set(!have_items_.get());

        if (have_items_.get()) {

            return next(lock);
        } else {
            last_id_ = make_blank<RowID>::value();

            return blank_p_;
        }
    }
    RowInternal& find_by_id(const Lock& lock, const RowID& id) const
    {
        OT_ASSERT(verify_lock(lock))

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
    bool first_valid_item(const Lock& lock) const
    {
        OT_ASSERT(verify_lock(lock));

        if (0 == items_.size()) {
            outer_ = outer_first();
            inner_ = items_.begin()->second.begin();

            return false;
        }

        outer_ = outer_first();

        while (outer_end() != outer_) {
            const auto& item = outer_->second;

            if (0 < item.size()) {
                inner_ = item.begin();
                valid_iterators();

                return true;
            }

            ++outer_;
        }

        return false;
    }
    /** Increment iterators to the next valid item, or loop back to start */
    void increment_inner(const Lock& lock) const
    {
        valid_iterators();
        const auto& item = outer_->second;

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
    bool increment_outer(const Lock& lock) const
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
    /** Returns the next item and increments iterators */
    const std::shared_ptr<const RowInternal> next(const Lock& lock) const
    {
        const auto output = current(lock);
        increment_inner(lock);

        return output;
    }
    OuterIterator outer_first() const { return Sort::begin(items_); }
    OuterIterator outer_end() const { return Sort::end(items_); }
    void reindex_item(
        const Lock& lock,
        const RowID& id,
        const SortKey& oldIndex,
        const SortKey& newIndex,
        const CustomData& custom) const
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
        if (inner_ == item) { increment_inner(lock); }

        std::shared_ptr<RowInternal> row = std::move(item->second);
        const auto deleted = itemMap.erase(id);

        OT_ASSERT(1 == deleted)

        if (0 == itemMap.size()) { items_.erase(index); }

        names_[id] = newIndex;
        row->reindex(newIndex, custom);
        items_[newIndex].emplace(id, std::move(row));
    }
    virtual bool same(const RowID& lhs, const RowID& rhs) const
    {
        return (lhs == rhs);
    }
    void valid_iterators() const
    {
        OT_ASSERT(outer_end() != outer_)

        const auto& item = outer_->second;

        OT_ASSERT(item.end() != inner_)
    }
    void wait_for_startup() const
    {
        while (false == startup_complete_.get()) {
            Log::Sleep(std::chrono::milliseconds(STARTUP_WAIT_MILLISECONDS));
        }
    }

    virtual void add_item(
        const RowID& id,
        const SortKey& index,
        const CustomData& custom)
    {
        insert_outer(id, index, custom);
    }
    void init() { outer_ = outer_first(); }
    void insert_outer(
        const RowID& id,
        const SortKey& index,
        const CustomData& custom)
    {
        Lock lock(lock_);

        if (0 == names_.count(id)) {
            construct_row(id, index, custom);

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

    List(
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const identifier::Nym& nymID,
        const Identifier& widgetID)
        : Widget(api, publisher, widgetID)
        , nym_id_(identifier::Nym::Factory(nymID.str()))
        , items_()
        , outer_(items_.begin())
        , inner_(items_.begin()->second.begin())
        , names_()
        , last_id_(make_blank<RowID>::value())
        , have_items_(Flag::Factory(false))
        , start_(Flag::Factory(true))
        , startup_complete_(Flag::Factory(false))
        , startup_(nullptr)
        , blank_p_(new RowBlank)
        , blank_(*blank_p_)
    {
        OT_ASSERT(blank_p_);
    }
    List(
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const identifier::Nym& nymID)
        : List(api, publisher, nymID, Identifier::Random())
    {
    }

private:
    List() = delete;
    List(const List&) = delete;
    List(List&&) = delete;
    List& operator=(const List&) = delete;
    List& operator=(List&&) = delete;
};
}  // namespace opentxs::ui::implementation
