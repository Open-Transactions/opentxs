// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "ActivityThreadItem.hpp"

namespace opentxs::ui::implementation
{
class MailItem final : public ActivityThreadItem
{
public:
    ~MailItem();

private:
    friend opentxs::Factory;

    std::unique_ptr<std::thread> load_{nullptr};

    void load();

    MailItem(
        const ActivityThreadInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const identifier::Nym& nymID,
        const ActivityThreadRowID& rowID,
        const ActivityThreadSortKey& sortKey,
        const CustomData& custom,
        const bool loading,
        const bool pending);
    MailItem(
        const ActivityThreadInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const identifier::Nym& nymID,
        const ActivityThreadRowID& rowID,
        const ActivityThreadSortKey& sortKey,
        const CustomData& custom);
    MailItem() = delete;
    MailItem(const MailItem&) = delete;
    MailItem(MailItem&&) = delete;
    MailItem& operator=(const MailItem&) = delete;
    MailItem& operator=(MailItem&&) = delete;
};
}  // namespace opentxs::ui::implementation
