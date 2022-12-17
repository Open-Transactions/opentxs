// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <iosfwd>

#include "interface/ui/base/Row.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/identifier/Generic.hpp"
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
class Nym;
}  // namespace identifier

namespace ui
{
class ActivityThreadItem;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
using ActivityThreadItemRow =
    Row<ActivityThreadRowInternal,
        ActivityThreadInternalInterface,
        ActivityThreadRowID>;

class ActivityThreadItem : public ActivityThreadItemRow
{
public:
    const api::session::Client& api_;

    auto Amount() const noexcept -> opentxs::Amount override { return 0; }
    auto Deposit() const noexcept -> bool override { return false; }
    auto DisplayAmount() const noexcept -> UnallocatedCString override
    {
        return {};
    }
    auto From() const noexcept -> UnallocatedCString final;
    auto Loading() const noexcept -> bool final { return loading_.get(); }
    auto MarkRead() const noexcept -> bool final;
    auto Memo() const noexcept -> UnallocatedCString override { return {}; }
    auto Outgoing() const noexcept -> bool final { return outgoing_.get(); }
    auto Pending() const noexcept -> bool final { return pending_.get(); }
    auto Text() const noexcept -> UnallocatedCString final;
    auto Timestamp() const noexcept -> Time final;
    auto Type() const noexcept -> otx::client::StorageBox final { return box_; }

    ActivityThreadItem() = delete;
    ActivityThreadItem(const ActivityThreadItem&) = delete;
    ActivityThreadItem(ActivityThreadItem&&) = delete;
    auto operator=(const ActivityThreadItem&) -> ActivityThreadItem& = delete;
    auto operator=(ActivityThreadItem&&) -> ActivityThreadItem& = delete;

    ~ActivityThreadItem() override = default;

protected:
    const identifier::Nym& nym_id_;
    const Time time_;
    const identifier::Generic& item_id_;
    const otx::client::StorageBox& box_;
    const identifier::Generic& account_id_;
    UnallocatedCString from_;
    UnallocatedCString text_;
    OTFlag loading_;
    OTFlag pending_;
    OTFlag outgoing_;

    auto reindex(const ActivityThreadSortKey& key, CustomData& custom) noexcept
        -> bool override;

    ActivityThreadItem(
        const ActivityThreadInternalInterface& parent,
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const ActivityThreadRowID& rowID,
        const ActivityThreadSortKey& sortKey,
        CustomData& custom) noexcept;

private:
    auto qt_data(const int column, const int role, QVariant& out) const noexcept
        -> void final;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::ActivityThreadItem>;
