/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#ifndef OPENTXS_UI_ACTIVITYSUMMARYITEM_IMPLEMENTATION_HPP
#define OPENTXS_UI_ACTIVITYSUMMARYITEM_IMPLEMENTATION_HPP

#include "opentxs/Internal.hpp"

namespace opentxs::ui::implementation
{
using ActivitySummaryItemType =
    Row<opentxs::ui::ActivitySummaryItem, ActivitySummaryParent, OTIdentifier>;

class ActivitySummaryItem : virtual public ActivitySummaryItemType
{
public:
    std::string DisplayName() const override;
    std::string ImageURI() const override;
    std::string Text() const override;
    std::string ThreadID() const override;
    std::chrono::system_clock::time_point Timestamp() const override;
    StorageBox Type() const override;

    ~ActivitySummaryItem();

private:
    friend Factory;
    // id, box, account
    using ItemLocator = std::tuple<std::string, StorageBox, std::string>;

    const api::Activity& activity_;
    const Flag& running_;
    const Identifier& nym_id_;
    std::shared_ptr<proto::StorageThread> thread_{nullptr};
    std::string display_name_{""};
    std::string text_{""};
    StorageBox type_{StorageBox::UNKNOWN};
    std::chrono::system_clock::time_point time_;
    std::unique_ptr<std::thread> startup_{nullptr};
    std::unique_ptr<std::thread> newest_item_thread_{nullptr};
    UniqueQueue<ItemLocator> newest_item_;
    OTZMQListenCallback activity_subscriber_callback_;
    OTZMQSubscribeSocket activity_subscriber_;

    bool check_thread(const proto::StorageThread& thread) const;
    std::string display_name(const proto::StorageThread& thread) const;
    std::string find_text(const ItemLocator& locator) const;
    const proto::StorageThreadItem& newest_item(
        const proto::StorageThread& thread) const;

    void get_text();
    void process_thread(const network::zeromq::Message& message);
    void startup();
    void update(const proto::StorageThread& thread);

    ActivitySummaryItem(
        const ActivitySummaryParent& parent,
        const network::zeromq::Context& zmq,
        const api::Activity& activity,
        const api::ContactManager& contact,
        const Flag& running,
        const Identifier& nymID,
        const Identifier& threadID);
    ActivitySummaryItem(const ActivitySummaryItem&) = delete;
    ActivitySummaryItem(ActivitySummaryItem&&) = delete;
    ActivitySummaryItem& operator=(const ActivitySummaryItem&) = delete;
    ActivitySummaryItem& operator=(ActivitySummaryItem&&) = delete;
};
}  // opentxs::ui::implementation
#endif  // OPENTXS_UI_ACTIVITYSUMMARYITEM_IMPLEMENTATION_HPP
