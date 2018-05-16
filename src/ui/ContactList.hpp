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

#ifndef OPENTXS_UI_CONTACTLIST_IMPLEMENTATION_HPP
#define OPENTXS_UI_CONTACTLIST_IMPLEMENTATION_HPP

#include "opentxs/Internal.hpp"

namespace opentxs::ui::implementation
{
using ContactListPimpl = std::unique_ptr<opentxs::ui::ContactListItem>;
using ContactListID = OTIdentifier;
using ContactListSortKey = std::string;
using ContactListInner = std::map<ContactListID, ContactListPimpl>;
using ContactListOuter = std::map<ContactListSortKey, ContactListInner>;
using ContactListReverse = std::map<ContactListID, ContactListSortKey>;
using ContactListType = List<
    opentxs::ui::ContactList,
    ContactListParent,
    opentxs::ui::ContactListItem,
    ContactListID,
    ContactListPimpl,
    ContactListInner,
    ContactListSortKey,
    ContactListOuter,
    ContactListOuter::const_iterator,
    ContactListReverse>;

class ContactList : virtual public ContactListType
{
public:
    const Identifier& ID() const override;

    ~ContactList() = default;

private:
    friend Factory;

    const Identifier& owner_contact_id_;
    std::unique_ptr<opentxs::ui::ContactListItem> owner_p_;
    opentxs::ui::ContactListItem& owner_;
    OTZMQListenCallback contact_subscriber_callback_;
    OTZMQSubscribeSocket contact_subscriber_;

    ContactListID blank_id() const override;
    void construct_item(
        const ContactListID& id,
        const ContactListSortKey& index,
        void* custom = nullptr) const override;
    const opentxs::ui::ContactListItem& first(const Lock& lock) const override;
    bool last(const ContactListID& id) const override
    {
        return ContactListType::last(id);
    }
    ContactListOuter::const_iterator outer_first() const override;
    ContactListOuter::const_iterator outer_end() const override;

    void add_item(
        const ContactListID& id,
        const ContactListSortKey& index,
        void* custom = nullptr) override;
    void process_contact(const network::zeromq::Message& message);
    void startup();

    ContactList(
        const network::zeromq::Context& zmq,
        const api::ContactManager& contact,
        const Identifier& nymID);
    ContactList() = delete;
    ContactList(const ContactList&) = delete;
    ContactList(ContactList&&) = delete;
    ContactList& operator=(const ContactList&) = delete;
    ContactList& operator=(ContactList&&) = delete;
};
}  // opentxs::ui::implementation
#endif  // OPENTXS_UI_CONTACTLIST_IMPLEMENTATION_HPP
