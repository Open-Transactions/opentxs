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

#ifndef OPENTXS_UI_PAYABLELIST_IMPLEMENTATION_HPP
#define OPENTXS_UI_PAYABLELIST_IMPLEMENTATION_HPP

#include "opentxs/Internal.hpp"

namespace opentxs::ui::implementation
{
using PayableListPimpl = std::unique_ptr<opentxs::ui::PayableListItem>;
using PayableListID = OTIdentifier;
using PayableListSortKey = std::string;
using PayableListInner = std::map<PayableListID, PayableListPimpl>;
using PayableListOuter = std::map<PayableListSortKey, PayableListInner>;
using PayableListReverse = std::map<PayableListID, PayableListSortKey>;
using PayableListType = List<
    opentxs::ui::PayableList,
    ContactListParent,
    opentxs::ui::PayableListItem,
    PayableListID,
    PayableListPimpl,
    PayableListInner,
    PayableListSortKey,
    PayableListOuter,
    PayableListOuter::const_iterator,
    PayableListReverse>;

class PayableList : virtual public PayableListType
{
public:
    const Identifier& ID() const override;

    ~PayableList() = default;

private:
    friend Factory;

    const api::client::Sync& sync_;
    const Identifier& owner_contact_id_;
    OTZMQListenCallback contact_subscriber_callback_;
    OTZMQSubscribeSocket contact_subscriber_;
    OTZMQListenCallback nym_subscriber_callback_;
    OTZMQSubscribeSocket nym_subscriber_;
    const proto::ContactItemType currency_;

    PayableListID blank_id() const override;
    void construct_item(
        const PayableListID& id,
        const PayableListSortKey& index,
        void* custom = nullptr) const override;
    bool last(const PayableListID& id) const override
    {
        return PayableListType::last(id);
    }
    PayableListOuter::const_iterator outer_first() const override;
    PayableListOuter::const_iterator outer_end() const override;

    void process_contact(
        const PayableListID& id,
        const PayableListSortKey& key);
    void process_contact(const network::zeromq::Message& message);
    void process_nym(const network::zeromq::Message& message);
    void startup();

    PayableList(
        const network::zeromq::Context& zmq,
        const api::ContactManager& contact,
        const api::client::Sync& sync,
        const Identifier& nymID,
        const proto::ContactItemType& currency);
    PayableList() = delete;
    PayableList(const PayableList&) = delete;
    PayableList(PayableList&&) = delete;
    PayableList& operator=(const PayableList&) = delete;
    PayableList& operator=(PayableList&&) = delete;
};
}  // opentxs::ui::implementation
#endif  // OPENTXS_UI_PAYABLELIST_IMPLEMENTATION_HPP
