// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <chrono>
#include <cstdint>

#include "ListRow.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace ui
{
class ActivityThreadItem;
}  // namespace ui

using OTUIActivityThreadItem = SharedPimpl<ui::ActivityThreadItem>;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
/**
   ActivityThreadItem is an individual chat message (or payment notice) as it appears inside the
   ActivityThread. For example, if Alice has sent 2 chat messages to Bob, then both users will see
   those messages as 2 individual ActivityThreadItems that appear as 2 rows inside an ActivityThread.
 */
class OPENTXS_EXPORT ActivityThreadItem : virtual public ListRow
{
public:
    /// Returns the amount of the thread item, if relevant, as an Amount object.
    virtual auto Amount() const noexcept -> opentxs::Amount = 0;
    /// Boolean value showing whether or not this item is a deposit to a server.
    virtual auto Deposit() const noexcept -> bool = 0;
    /// Returns the amount of this thread item, if relevant, as a formatted string for display in the UI.
    virtual auto DisplayAmount() const noexcept -> UnallocatedCString = 0;
    /// Returns a string containing the ID of the sender.
    virtual auto From() const noexcept -> UnallocatedCString = 0;
    /// Boolean value showing whether or not this item is still in the process of loading.
    virtual auto Loading() const noexcept -> bool = 0;
    /// Boolean value showing whether or not this item is marked as read.
    virtual auto MarkRead() const noexcept -> bool = 0;
    /// Returns the memo, if relevant (for transactions).
    virtual auto Memo() const noexcept -> UnallocatedCString = 0;
    /// Boolean value showing whether or not this item is outgoing.
    virtual auto Outgoing() const noexcept -> bool = 0;
    /// Boolean value showing whether or not this item is pending.
    virtual auto Pending() const noexcept -> bool = 0;
    /// Returns the main display text for this thread item.
    virtual auto Text() const noexcept -> UnallocatedCString = 0;
    /// Returns the timestamp for this thread item.
    virtual auto Timestamp() const noexcept -> Time = 0;
    /// Returns the type for this item. (Message, incoming cheque, outgoing blockchain transfer, etc).
    virtual auto Type() const noexcept -> otx::client::StorageBox = 0;

    ActivityThreadItem(const ActivityThreadItem&) = delete;
    ActivityThreadItem(ActivityThreadItem&&) = delete;
    auto operator=(const ActivityThreadItem&) -> ActivityThreadItem& = delete;
    auto operator=(ActivityThreadItem&&) -> ActivityThreadItem& = delete;

    ~ActivityThreadItem() override = default;

protected:
    ActivityThreadItem() noexcept = default;
};
}  // namespace opentxs::ui
