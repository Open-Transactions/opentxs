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

#ifndef OPENTXS_CONTACT_CONTACT_ITEM_HPP
#define OPENTXS_CONTACT_CONTACT_ITEM_HPP

#define NULL_START 0
#define NULL_END 0

#include "opentxs/Forward.hpp"

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <chrono>
#include <cstdint>
#include <set>

namespace opentxs
{

class ContactItem
{
public:
    ContactItem(
        const std::string& nym,
        const std::uint32_t version,
        const std::uint32_t parentVersion,
        const proto::ContactSectionName section,
        const proto::ContactItemType& type,
        const std::string& value,
        const std::set<proto::ContactItemAttribute>& attributes,
        const std::time_t start,
        const std::time_t end);
    ContactItem(
        const std::string& nym,
        const std::uint32_t version,
        const std::uint32_t parentVersion,
        const Claim& claim);
    ContactItem(
        const std::string& nym,
        const std::uint32_t parentVersion,
        const proto::ContactSectionName section,
        const proto::ContactItem& serialized);
    ContactItem(const ContactItem&) = default;
    ContactItem(ContactItem&&) = default;

    bool operator==(const ContactItem& rhs) const;

    // Includes IDs
    operator proto::ContactItem() const;

    const std::time_t& End() const;
    const Identifier& ID() const;
    bool isActive() const;
    bool isLocal() const;
    bool isPrimary() const;
    const proto::ContactSectionName& Section() const;
    proto::ContactItem Serialize(const bool withID = false) const;
    ContactItem SetActive(const bool active) const;
    ContactItem SetEnd(const std::time_t end) const;
    ContactItem SetLocal(const bool local) const;
    ContactItem SetPrimary(const bool primary) const;
    ContactItem SetStart(const std::time_t start) const;
    ContactItem SetValue(const std::string& value) const;
    const std::time_t& Start() const;
    const proto::ContactItemType& Type() const;
    const std::string& Value() const;
    const std::uint32_t& Version() const;

    ~ContactItem() = default;

private:
    const std::uint32_t version_{0};
    const std::string nym_{};
    const proto::ContactSectionName section_{proto::CONTACTSECTION_ERROR};
    const proto::ContactItemType type_{proto::CITEMTYPE_ERROR};
    const std::string value_;
    const std::time_t start_{0};
    const std::time_t end_{0};
    const std::set<proto::ContactItemAttribute> attributes_{};
    const Identifier& id_;

    static std::uint32_t check_version(
        const std::uint32_t in,
        const std::uint32_t targetVersion);
    static std::set<proto::ContactItemAttribute> extract_attributes(
        const proto::ContactItem& serialized);
    static std::set<proto::ContactItemAttribute> extract_attributes(
        const Claim& claim);

    ContactItem set_attribute(
        const proto::ContactItemAttribute& attribute,
        const bool value) const;

    ContactItem() = delete;
    ContactItem& operator=(const ContactItem&) = delete;
    ContactItem& operator=(ContactItem&&) = delete;
};
}  // namespace opentxs

#endif  // OPENTXS_CONTACT_CONTACT_ITEM_HPP
