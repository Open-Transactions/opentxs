// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>
#include <future>
#include <optional>
#include <type_traits>
#include <utility>

#include "interface/ui/base/Items.hpp"
#include "interface/ui/base/Widget.hpp"
#include "internal/core/Core.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/util/Flag.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "util/Blank.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
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
             public Widget
{
public:
    using ChildObjectDataType = ChildObjectData<RowID, SortKey>;
    using ChildDefinitions = UnallocatedVector<ChildObjectDataType>;
    using ListPrimaryID = PrimaryID;

    auto AddChildrenToList(CustomData&& data) noexcept -> void
    {
        if (0u < data.size()) {
            add_items(extract_custom<ChildDefinitions>(data, 0));
        }
    }
    auto First() const noexcept -> SharedPimpl<RowInterface> override
    {
        auto lock = rLock{recursive_lock_};
        counter_ = 0;

        return first(lock);
    }
    auto last(const RowID& id) const noexcept -> bool override
    {
        auto lock = rLock{recursive_lock_};

        return items_.last(id);
    }
    auto Next() const noexcept -> SharedPimpl<RowInterface> override
    {
        // TODO this logic only works for an offset_ value of 0 or 1
        auto lock = rLock{recursive_lock_};

        try {

            return SharedPimpl<RowInterface>(items_.at(++counter_).item_);
        } catch (...) {

            return First();
        }
    }

    auto GetQt() const noexcept -> ui::qt::internal::Model* final
    {
        return qt_model_;
    }
    auto shutdown(std::promise<void>& promise) noexcept -> void
    {
        wait_for_startup();

        try {
            promise.set_value();
        } catch (...) {
        }
    }
    virtual auto state_machine() noexcept -> bool { return false; }

    List() = delete;
    List(const List&) = delete;
    List(List&&) = delete;
    auto operator=(const List&) -> List& = delete;
    auto operator=(List&&) -> List& = delete;

    ~List() override
    {
        if (startup_ && startup_->joinable()) {
            startup_->join();
            startup_.reset();
        }

        if (nullptr != qt_model_) {
            if (false == subnode_) { delete qt_model_; }

            qt_model_ = nullptr;
        }
    }

protected:
    using RowPointer = std::shared_ptr<RowInternal>;

    ui::qt::internal::Model* qt_model_;
    mutable std::recursive_mutex recursive_lock_;
    mutable std::shared_mutex shared_lock_;
    const PrimaryID primary_id_;
    mutable std::size_t counter_;
    mutable OTFlag have_items_;
    mutable OTFlag start_;
    std::unique_ptr<std::thread> startup_;  // TODO remove

    auto delete_inactive(const UnallocatedSet<RowID>& active) const noexcept
        -> void
    {
        auto lock = rLock{recursive_lock_};
        delete_inactive(lock, active);
    }
    auto delete_inactive(const rLock& lock, const UnallocatedSet<RowID>& active)
        const noexcept -> void
    {
        const auto existing = items_.active();
        auto deleteIDs = UnallocatedVector<RowID>{};
        std::ranges::set_difference(
            existing, active, std::back_inserter(deleteIDs));

        for (const auto& id : deleteIDs) { delete_item(lock, id); }

        if (0 < deleteIDs.size()) { UpdateNotify(); }
    }
    auto delete_item(const RowID& id) const noexcept -> void
    {
        auto lock = rLock{recursive_lock_};
        delete_item(lock, id);
    }
    auto delete_item(const rLock&, const RowID& id) const noexcept -> void
    {
        auto position = items_.find_delete_position(id);

        if (false == position.has_value()) { return; }

        auto& [it, index] = position.value();

        if (nullptr != qt_model_) { qt_model_->DeleteRow(it->item_.get()); }

        pre_delete(*it->item_);
        items_.delete_row(id, it);
    }
    virtual auto default_id() const noexcept -> RowID
    {
        return make_blank<RowID>::value(
            static_cast<const InternalInterface&>(*this).API());
    }
    virtual auto find_index(const RowID& id) const noexcept
        -> std::optional<std::size_t>
    {
        return items_.get_index(id);
    }
    template <typename Callback>
    auto for_each_row(const Callback& cb) const noexcept -> void
    {
        auto lock = rLock{recursive_lock_};

        for (const auto& row : items_) { cb(*row.item_); }
    }

    auto first(const rLock&) const noexcept -> SharedPimpl<RowInterface>
    {
        try {

            return SharedPimpl<RowInterface>(items_.at(0).item_);
        } catch (...) {

            return SharedPimpl<RowInterface>(blank_p_);
        }
    }
    auto lookup(const rLock&, const RowID& id) const noexcept
        -> const RowInternal&
    {
        try {

            return *(items_.get(id).item_);
        } catch (...) {

            return blank_;
        }
    }
    auto size() const noexcept -> std::size_t { return items_.size(); }
    auto sort_key(const rLock&, const RowID& id) const noexcept -> SortKey
    {
        try {

            return items_.get(id).key_;
        } catch (...) {

            return {};
        }
    }
    auto startup_complete() const noexcept -> bool
    {
        static constexpr auto none = 0s;

        return std::future_status::ready == startup_future_.wait_for(none);
    }
    auto wait_for_startup() const noexcept -> void { startup_future_.get(); }

    auto add_item(
        const RowID& id,
        const SortKey& index,
        CustomData& custom) noexcept -> RowInternal&
    {
        auto lock = rLock{recursive_lock_};

        return add_item(lock, id, index, custom);
    }
    auto add_item(
        const rLock& lock,
        const RowID& id,
        const SortKey& index,
        CustomData& custom) noexcept -> RowInternal&
    {
        auto existing = find_index(id);

        if (existing.has_value()) {

            return move_item(lock, id, index, custom);
        } else {

            return insert_item(lock, id, index, custom);
        }
    }
    auto add_items(ChildDefinitions&& items) noexcept -> void
    {
        auto lock = rLock{recursive_lock_};

        for (auto& [id, key, custom, children] : items) {
            auto& item = add_item(lock, id, key, custom);
            item.AddChildren(std::move(children));
        }
    }
    auto finish_startup() noexcept -> void
    {
        if (nullptr != qt_model_) { qt_model_->SetStartupComplete(); }

        try {
            startup_promise_.set_value();
        } catch (...) {
        }
    }
    virtual auto qt_parent() noexcept -> internal::Row* { return nullptr; }
    auto row_modified(
        ui::internal::Row* parent,
        ui::internal::Row* row) noexcept -> void
    {
        if (qt_model_) { qt_model_->ChangeRow(parent, row); }

        UpdateNotify();
    }

    // NOTE lists that are also rows call this constructor
    List(
        const api::session::Client& api,
        const PrimaryID& primaryID,
        const identifier::Generic& widgetID,
        const bool reverseSort,
        const bool subnode,
        const SimpleCallback& cb,
        qt::internal::Model* qt) noexcept
        : Widget(api, widgetID, cb)
        , qt_model_(qt)
        , recursive_lock_()
        , shared_lock_()
        , primary_id_(primaryID)
        , counter_(0)
        , have_items_(Flag::Factory(false))
        , start_(Flag::Factory(true))
        , startup_(nullptr)
        , blank_p_(std::make_unique<RowBlank>())
        , blank_(*blank_p_)
        , subnode_(subnode)
        , init_(false)
        , items_(0, reverseSort)
        , startup_promise_()
        , startup_future_(startup_promise_.get_future())
    {
        assert_false(nullptr == blank_p_);
        assert_false(subnode_ && (nullptr != cb));
    }
    // NOTE basic lists (not subnodes) call this constructor
    List(
        const api::session::Client& api,
        const PrimaryID& primaryID,
        const SimpleCallback& cb,
        const bool reverseSort) noexcept
        : List(
              api,
              primaryID,
              api.Factory().IdentifierFromRandom(),
              reverseSort,
              false,
              cb,
              internal::List::MakeQT(api))
    {
    }

private:
    using ItemsType = ListItems<RowID, SortKey, RowPointer>;

    const std::shared_ptr<const RowInternal> blank_p_;
    const RowInternal& blank_;
    const bool subnode_;
    mutable std::atomic<bool> init_;
    mutable ItemsType items_;
    std::promise<void> startup_promise_;
    std::shared_future<void> startup_future_;

    virtual auto construct_row(
        const RowID& id,
        const SortKey& index,
        CustomData& custom) const noexcept -> RowPointer = 0;
    virtual auto post_insert(const RowInternal&) const noexcept -> void {}
    virtual auto post_reindex(const RowInternal&) const noexcept -> void {}
    virtual auto pre_delete(const RowInternal&) const noexcept -> void {}
    virtual auto pre_reindex(const RowInternal&) const noexcept -> void {}

    auto insert_item(
        const rLock&,
        const RowID& id,
        const SortKey& key,
        CustomData& custom) noexcept -> RowInternal&
    {
        auto pointer = construct_row(id, key, custom);

        assert_false(nullptr == pointer);

        const auto position = items_.find_insert_position(key, id);
        auto& [it, prev] = position;
        auto row = items_.insert_before(it, key, id, pointer);

        if (nullptr != qt_model_) {
            qt_model_->InsertRow(qt_parent(), prev, row);
        }

        post_insert(*pointer);
        UpdateNotify();

        return *pointer;
    }
    auto move_item(
        const rLock&,
        const RowID& id,
        const SortKey& key,
        CustomData& custom) noexcept -> RowInternal&
    {
        auto move = items_.find_move_position(id, key, id);

        assert_true(move.has_value());

        auto& [from, to] = move.value();
        auto& [source, oldBefore] = from;
        auto& [dest, newBefore] = to;
        auto* item = source->item_.get();
        auto* parent = qt_parent();

        assert_false(nullptr == item);

        const auto samePosition{
            (oldBefore == newBefore) || (item == newBefore)};

        if (false == samePosition) {
            if (nullptr != qt_model_) {
                qt_model_->MoveRow(parent, newBefore, item);
            }

            items_.move_before(id, source, key, id, dest);
        }

        pre_reindex(*item);
        const auto changed = item->reindex(key, custom);
        post_reindex(*item);

        if (changed) {
            row_modified(parent, item);
        } else if (false == samePosition) {
            UpdateNotify();
        }

        return *item;
    }
};
}  // namespace opentxs::ui::implementation
