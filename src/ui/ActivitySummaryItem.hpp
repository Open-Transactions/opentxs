// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
using ActivitySummaryItemRow =
    Row<ActivitySummaryRowInternal,
        ActivitySummaryInternalInterface,
        ActivitySummaryRowID>;

class ActivitySummaryItem final : public ActivitySummaryItemRow
{
public:
    std::string DisplayName() const override;
    std::string ImageURI() const override;
    std::string Text() const override;
    std::string ThreadID() const override;
    std::chrono::system_clock::time_point Timestamp() const override;
    StorageBox Type() const override;

    void reindex(const ActivitySummarySortKey& key, const CustomData& custom)
        override;

    ~ActivitySummaryItem();

private:
    friend opentxs::Factory;
    // id, box, account
    using ItemLocator = std::tuple<std::string, StorageBox, std::string>;

    const Flag& running_;
    const OTNymID nym_id_;
    ActivitySummarySortKey key_;
    std::shared_ptr<proto::StorageThread> thread_{nullptr};
    std::string& display_name_;
    std::string text_{""};
    StorageBox type_{StorageBox::UNKNOWN};
    std::chrono::system_clock::time_point time_;
    std::unique_ptr<std::thread> newest_item_thread_{nullptr};
    UniqueQueue<ItemLocator> newest_item_;
    std::atomic<int> next_task_id_;

    std::string find_text(const ItemLocator& locator) const;

    void get_text();
    void startup(const CustomData& custom, UniqueQueue<ItemLocator>& queue);

    ActivitySummaryItem(
        const ActivitySummaryInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const identifier::Nym& nymID,
        const ActivitySummaryRowID& rowID,
        const ActivitySummarySortKey& sortKey,
        const CustomData& custom,
        const Flag& running);
    ActivitySummaryItem(const ActivitySummaryItem&) = delete;
    ActivitySummaryItem(ActivitySummaryItem&&) = delete;
    ActivitySummaryItem& operator=(const ActivitySummaryItem&) = delete;
    ActivitySummaryItem& operator=(ActivitySummaryItem&&) = delete;
};
}  // namespace opentxs::ui::implementation
