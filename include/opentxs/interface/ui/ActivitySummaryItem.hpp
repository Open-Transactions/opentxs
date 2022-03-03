// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <chrono>

#include "ListRow.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace ui
{
class ActivitySummaryItem;
}  // namespace ui

using OTUIActivitySummaryItem = SharedPimpl<ui::ActivitySummaryItem>;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class OPENTXS_EXPORT ActivitySummaryItem : virtual public ListRow
{
public:
    virtual auto DisplayName() const noexcept -> UnallocatedCString = 0;
    virtual auto ImageURI() const noexcept -> UnallocatedCString = 0;
    virtual auto Text() const noexcept -> UnallocatedCString = 0;
    virtual auto ThreadID() const noexcept -> UnallocatedCString = 0;
    virtual auto Timestamp() const noexcept -> Time = 0;
    virtual auto Type() const noexcept -> StorageBox = 0;

    ~ActivitySummaryItem() override = default;

protected:
    ActivitySummaryItem() noexcept = default;

private:
    ActivitySummaryItem(const ActivitySummaryItem&) = delete;
    ActivitySummaryItem(ActivitySummaryItem&&) = delete;
    auto operator=(const ActivitySummaryItem&) -> ActivitySummaryItem& = delete;
    auto operator=(ActivitySummaryItem&&) -> ActivitySummaryItem& = delete;
};
}  // namespace opentxs::ui
