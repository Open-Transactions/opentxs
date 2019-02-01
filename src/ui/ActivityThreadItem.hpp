// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
using ActivityThreadItemRow =
    Row<ActivityThreadRowInternal,
        ActivityThreadInternalInterface,
        ActivityThreadRowID>;

class ActivityThreadItem : public ActivityThreadItemRow
{
public:
    opentxs::Amount Amount() const override { return 0; }
    std::string DisplayAmount() const override { return {}; }
    bool Loading() const override { return loading_.get(); }
    bool MarkRead() const override;
    std::string Memo() const override { return {}; }
    bool Pending() const override { return pending_.get(); }
    std::string Text() const override;
    std::chrono::system_clock::time_point Timestamp() const override;
    StorageBox Type() const override { return box_; }

    void reindex(const ActivityThreadSortKey& key, const CustomData& custom)
        override;

    virtual ~ActivityThreadItem() = default;

protected:
    const identifier::Nym& nym_id_;
    const std::chrono::system_clock::time_point time_;
    const Identifier& item_id_;
    const StorageBox& box_;
    const Identifier& account_id_;
    std::string text_;
    OTFlag loading_;
    OTFlag pending_;

    ActivityThreadItem(
        const ActivityThreadInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const identifier::Nym& nymID,
        const ActivityThreadRowID& rowID,
        const ActivityThreadSortKey& sortKey,
        const CustomData& custom,
        const bool loading,
        const bool pending);

private:
    ActivityThreadItem() = delete;
    ActivityThreadItem(const ActivityThreadItem&) = delete;
    ActivityThreadItem(ActivityThreadItem&&) = delete;
    ActivityThreadItem& operator=(const ActivityThreadItem&) = delete;
    ActivityThreadItem& operator=(ActivityThreadItem&&) = delete;
};
}  // namespace opentxs::ui::implementation
