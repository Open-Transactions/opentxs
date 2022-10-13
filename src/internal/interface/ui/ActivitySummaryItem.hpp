// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>

#include "ListRow.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
class ActivitySummaryItem;
}  // namespace ui

using OTUIActivitySummaryItem = SharedPimpl<ui::ActivitySummaryItem>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
/**
   ActivitySummaryItem is a high-level summary of a single conversational
   thread, meant to appear in a list of other recently active threads. Similar
   to the chat history on a smart phone, ActivitySummaryItem represents a single
   one of the conversational threads in that history.
 */
class ActivitySummaryItem : virtual public ListRow
{
public:
    /// Returns the contact's display name for this thread.
    virtual auto DisplayName() const noexcept -> UnallocatedCString = 0;
    /// Returns the contact's image as a URI string.
    virtual auto ImageURI() const noexcept -> UnallocatedCString = 0;
    /// Returns the display text (such as a message preview) for this thread.
    virtual auto Text() const noexcept -> UnallocatedCString = 0;
    /// Returns the thread ID
    virtual auto ThreadID() const noexcept -> UnallocatedCString = 0;
    /// Returns the timestamp of the most recent update to this thread.
    virtual auto Timestamp() const noexcept -> Time = 0;
    /// Returns the thread type as an enum.
    virtual auto Type() const noexcept -> otx::client::StorageBox = 0;

    ActivitySummaryItem(const ActivitySummaryItem&) = delete;
    ActivitySummaryItem(ActivitySummaryItem&&) = delete;
    auto operator=(const ActivitySummaryItem&) -> ActivitySummaryItem& = delete;
    auto operator=(ActivitySummaryItem&&) -> ActivitySummaryItem& = delete;

    ~ActivitySummaryItem() override = default;

protected:
    ActivitySummaryItem() noexcept = default;
};
}  // namespace opentxs::ui
