/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#ifndef OPENTXS_UI_LIST_HPP
#define OPENTXS_UI_LIST_HPP

#include "opentxs/Internal.hpp"

#include "Widget.hpp"

#define STARTUP_WAIT_MILLISECONDS 100

namespace opentxs::ui::implementation
{
template <
    typename InterfaceType,
    typename InternalInterfaceType,
    typename RowType,
    typename IDType,
    typename PimplType,
    typename InnerType,
    typename SortKeyType,
    typename OuterType,
    typename OuterIteratorType,
    typename ReverseType>
class List : virtual public InterfaceType,
             virtual public InternalInterfaceType,
             public Widget,
             public Lockable
{
public:
    const RowType& First() const override
    {
        Lock lock(lock_);

        return first(lock);
    }

    virtual bool last(const IDType& id) const override
    {
        Lock lock(lock_);

        return start_.get() && same(id, last_id_);
    }

    const RowType& Next() const override
    {
        Lock lock(lock_);

        if (start_.get()) {

            return first(lock);
        }

        return next(lock);
    }
    using InternalInterfaceType::reindex_item;
    void reindex_item(const IDType& id, const SortKeyType& newIndex)
        const override
    {
        Lock lock(lock_);
        const auto& oldIndex = names_.at(id);
        reindex_item(lock, id, oldIndex, newIndex);
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
    const api::ContactManager& contact_manager_;
    const Identifier& nym_id_;
    mutable OuterType items_;
    mutable OuterIteratorType outer_;
    mutable typename InnerType::const_iterator inner_;
    mutable ReverseType names_;
    mutable IDType last_id_;
    mutable OTFlag have_items_;
    mutable OTFlag start_;
    mutable OTFlag startup_complete_;
    std::unique_ptr<std::thread> startup_{nullptr};
    const std::unique_ptr<RowType> blank_p_{nullptr};
    const RowType& blank_;
    const Identifier& widget_id_;

    virtual IDType blank_id() const = 0;
    virtual void construct_item(
        const IDType& id,
        const SortKeyType& index,
        void* custom = nullptr) const = 0;
    /** Returns item reference by the inner_ iterator. Does not increment
     *  iterators. */
    const RowType& current(const Lock& lock) const
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

        return *item;
    }
    void delete_inactive(const std::set<IDType>& active) const
    {
        Lock lock(lock_);
        std::vector<IDType> deleteIDs{};

        for (const auto& it : names_) {
            const auto& id = it.first;

            if (0 == active.count(id)) {
                deleteIDs.emplace_back(id);
            }
        }

        for (const auto& id : deleteIDs) {
            delete_item(lock, id);
        }

        UpdateNotify();
    }
    void delete_item(const Lock& lock, const IDType& id) const
    {
        OT_ASSERT(verify_lock(lock))

        auto& key = names_.at(id);
        auto& inner = items_.at(key);
        auto item = inner.find(id);

        // I'm about to delete this row. Make sure iterators are not pointing
        // to it
        if (inner_ == item) {
            increment_inner(lock);
        }

        const auto itemDeleted = inner.erase(id);

        OT_ASSERT(1 == itemDeleted)

        if (0 == inner.size()) {
            items_.erase(key);
        }

        const auto indexDeleted = names_.erase(id);

        OT_ASSERT(1 == indexDeleted)
    }
    /** Returns first contact, or blank if none exists. Sets up iterators for
     *  next row
     *
     *  WARNING: if blank_p_ is not set, you must override this method in a
     *  child class
     */
    virtual const RowType& first(const Lock& lock) const
    {
        OT_ASSERT(verify_lock(lock))

        have_items_->Set(first_valid_item(lock));
        start_->Set(!have_items_.get());

        if (have_items_.get()) {

            return next(lock);
        } else {
            last_id_ = blank_id();

            return blank_;
        }
    }
    RowType& find_by_id(const Lock& lock, const IDType& id) const
    {
        OT_ASSERT(verify_lock(lock))

        try {
            auto& key = names_.at(id);
            auto& inner = items_.at(key);
            auto item = inner.find(id);

            if (inner.end() == item) {

                return const_cast<RowType&>(blank_);
            }

            OT_ASSERT(item->second)

            return *item->second;
        } catch (const std::out_of_range&) {
        }

        return const_cast<RowType&>(blank_);
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

                if (have_items_.get()) {
                    valid_iterators();
                }

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
    const RowType& next(const Lock& lock) const
    {
        const auto& output = current(lock);
        increment_inner(lock);

        return output;
    }
    virtual OuterIteratorType outer_first() const = 0;
    virtual OuterIteratorType outer_end() const = 0;
    void reindex_item(
        const Lock& lock,
        const IDType& id,
        const SortKeyType& oldIndex,
        const SortKeyType& newIndex,
        void* custom = nullptr) const
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
        if (inner_ == item) {
            increment_inner(lock);
        }

        PimplType row = std::move(item->second);
        const auto deleted = itemMap.erase(id);

        OT_ASSERT(1 == deleted)

        if (0 == itemMap.size()) {
            items_.erase(index);
        }

        if (nullptr != custom) {
            update(row, custom);
        }

        names_[id] = newIndex;
        items_[newIndex].emplace(id, std::move(row));
    }
    virtual bool same(const IDType& lhs, const IDType& rhs) const
    {
        return (lhs == rhs);
    }
    virtual void update(PimplType& row, const void* custom) const {}
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
        const IDType& id,
        const SortKeyType& index,
        void* custom = nullptr)
    {
        insert_outer(id, index, custom);
    }
    void init() { outer_ = outer_first(); }
    void insert_outer(
        const IDType& id,
        const SortKeyType& index,
        void* custom = nullptr)
    {
        Lock lock(lock_);

        if (0 == names_.count(id)) {
            construct_item(id, index, custom);

            OT_ASSERT(1 == items_.count(index))
            OT_ASSERT(1 == names_.count(id))

            UpdateNotify();

            return;
        }

        const auto& oldIndex = names_.at(id);

        if (oldIndex == index) {

            return;
        }

        reindex_item(lock, id, oldIndex, index, custom);
        UpdateNotify();
    }

    List(
        const network::zeromq::Context& zmq,
        const api::ContactManager& contact,
        const IDType lastID,
        const Identifier& nymID,
        RowType* blank)
        : Widget(zmq)
        , contact_manager_(contact)
        , nym_id_(Identifier::Factory(nymID))
        , items_()
        , outer_(items_.begin())
        , inner_(items_.begin()->second.begin())
        , names_()
        , last_id_(lastID)
        , have_items_(Flag::Factory(false))
        , start_(Flag::Factory(true))
        , startup_complete_(Flag::Factory(false))
        , startup_(nullptr)
        , blank_p_(blank)
        , blank_(*blank_p_)
        , widget_id_(Identifier::Random())
    {
        // WARNING if you plan on using blank_, check blank_p_ in the child
        // class constructor
    }

private:
    List() = delete;
    List(const List&) = delete;
    List(List&&) = delete;
    List& operator=(const List&) = delete;
    List& operator=(List&&) = delete;
};
}  // opentxs::ui::implementation
#endif  // OPENTXS_UI_LIST_HPP
