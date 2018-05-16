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

#ifndef OPENTXS_CONTACT_CONTACT_GROUP_HPP
#define OPENTXS_CONTACT_CONTACT_GROUP_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Proto.hpp"

#include <map>
#include <memory>

namespace opentxs
{
class ContactGroup
{
public:
    typedef std::map<OTIdentifier, std::shared_ptr<ContactItem>> ItemMap;

    ContactGroup(
        const std::string& nym,
        const proto::ContactSectionName section,
        const proto::ContactItemType type,
        const ItemMap& items);
    ContactGroup(
        const std::string& nym,
        const proto::ContactSectionName section,
        const std::shared_ptr<ContactItem>& item);
    ContactGroup(const ContactGroup&) = default;
    ContactGroup(ContactGroup&&) = default;

    ContactGroup operator+(const ContactGroup& rhs) const;

    ItemMap::const_iterator begin() const;
    std::shared_ptr<ContactItem> Best() const;
    std::shared_ptr<ContactItem> Claim(const Identifier& item) const;
    bool HaveClaim(const Identifier& item) const;
    ContactGroup AddItem(const std::shared_ptr<ContactItem>& item) const;
    ContactGroup AddPrimary(const std::shared_ptr<ContactItem>& item) const;
    ContactGroup Delete(const Identifier& id) const;
    ItemMap::const_iterator end() const;
    const Identifier& Primary() const;
    std::shared_ptr<ContactItem> PrimaryClaim() const;
    bool SerializeTo(proto::ContactSection& section, const bool withIDs = false)
        const;
    std::size_t Size() const;
    const proto::ContactItemType& Type() const;

    ~ContactGroup() = default;

private:
    const std::string nym_{};
    const proto::ContactSectionName section_{proto::CONTACTSECTION_ERROR};
    const proto::ContactItemType type_{proto::CITEMTYPE_ERROR};
    const Identifier& primary_;
    const ItemMap items_{};

    static ItemMap create_item(const std::shared_ptr<ContactItem>& item);
    static OTIdentifier get_primary_item(const ItemMap& items);
    static ItemMap normalize_items(const ItemMap& items);

    ContactGroup() = delete;
    ContactGroup& operator=(const ContactGroup&) = delete;
    ContactGroup& operator=(ContactGroup&&) = delete;
};
}  // namespace opentxs
#endif  // OPENTXS_CONTACT_CONTACT_GROUP_HPP
