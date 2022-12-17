// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <chrono>
#include <iosfwd>
#include <memory>
#include <thread>
#include <tuple>

#include "interface/ui/base/Row.hpp"
#include "internal/interface/ui/ActivitySummaryItem.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "internal/util/UniqueQueue.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"

class QVariant;

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

namespace identifier
{
class Generic;
}  // namespace identifier

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

namespace proto
{
class StorageThread;
}  // namespace proto

namespace ui
{
class ActivitySummaryItem;
}  // namespace ui

class Flag;
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
using ActivitySummaryItemRow =
    Row<ActivitySummaryRowInternal,
        ActivitySummaryInternalInterface,
        ActivitySummaryRowID>;

class ActivitySummaryItem final : public ActivitySummaryItemRow
{
public:
    const api::session::Client& api_;

    static auto LoadItemText(
        const api::session::Client& api,
        const identifier::Nym& nym,
        const CustomData& custom) noexcept -> UnallocatedCString;

    auto DisplayName() const noexcept -> UnallocatedCString final;
    auto ImageURI() const noexcept -> UnallocatedCString final;
    auto Text() const noexcept -> UnallocatedCString final;
    auto ThreadID() const noexcept -> UnallocatedCString final;
    auto Timestamp() const noexcept -> Time final;
    auto Type() const noexcept -> otx::client::StorageBox final;

    ActivitySummaryItem(
        const ActivitySummaryInternalInterface& parent,
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const ActivitySummaryRowID& rowID,
        const ActivitySummarySortKey& sortKey,
        CustomData& custom,
        const Flag& running,
        UnallocatedCString text) noexcept;
    ActivitySummaryItem(const ActivitySummaryItem&) = delete;
    ActivitySummaryItem(ActivitySummaryItem&&) = delete;
    auto operator=(const ActivitySummaryItem&) -> ActivitySummaryItem& = delete;
    auto operator=(ActivitySummaryItem&&) -> ActivitySummaryItem& = delete;

    ~ActivitySummaryItem() final;

private:
    // id, box, account, thread
    using ItemLocator = std::tuple<
        UnallocatedCString,
        otx::client::StorageBox,
        UnallocatedCString,
        identifier::Generic>;

    const Flag& running_;
    const identifier::Nym nym_id_;
    ActivitySummarySortKey key_;
    UnallocatedCString& display_name_;
    UnallocatedCString text_;
    otx::client::StorageBox type_;
    Time time_;
    std::unique_ptr<std::thread> newest_item_thread_;
    UniqueQueue<ItemLocator> newest_item_;
    std::atomic<int> next_task_id_;
    std::atomic<bool> break_;

    auto find_text(const PasswordPrompt& reason, const ItemLocator& locator)
        const noexcept -> UnallocatedCString;
    auto qt_data(const int column, const int role, QVariant& out) const noexcept
        -> void final;

    auto get_text() noexcept -> void;
    auto reindex(const ActivitySummarySortKey& key, CustomData& custom) noexcept
        -> bool final;
    auto startup(CustomData& custom) noexcept -> void;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::ActivitySummaryItem>;
