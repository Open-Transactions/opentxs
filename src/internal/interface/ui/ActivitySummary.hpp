// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/interface/ui/List.hpp"
#include "internal/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
class ActivitySummary;
class ActivitySummaryItem;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
/**
  This model manages the ActivitySummary. Each row is a different activity
  thread. For example, my chat session with Bob, and my chat session with Alice,
  constitute 2 rows in the ActivitySummary. Similar to a smart phone's list of
  past SMS conversations, each ActivitySummaryItem represents a different
  thread.
 */
class ActivitySummary : virtual public List
{
public:
    /// returns the first row, containing a valid ActivitySummaryItem or an
    /// empty smart pointer (if list is empty).
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ActivitySummaryItem> = 0;
    /// returns the next row, containing a valid ActivitySummaryItem or an empty
    /// smart pointer (if at end of list).
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ActivitySummaryItem> = 0;

    ActivitySummary(const ActivitySummary&) = delete;
    ActivitySummary(ActivitySummary&&) = delete;
    auto operator=(const ActivitySummary&) -> ActivitySummary& = delete;
    auto operator=(ActivitySummary&&) -> ActivitySummary& = delete;

    ~ActivitySummary() override = default;

protected:
    ActivitySummary() = default;
};
}  // namespace opentxs::ui
