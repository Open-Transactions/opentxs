// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CLIENT_CONTACTS_HPP
#define OPENTXS_API_CLIENT_CONTACTS_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/api/Editor.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <memory>

namespace opentxs
{
namespace api
{
namespace client
{
class Contacts
{
public:
    EXPORT virtual OTIdentifier BlockchainAddressToContact(
        const std::string& address,
        const proto::ContactItemType currency = proto::CITEMTYPE_BTC) const = 0;
    EXPORT virtual std::shared_ptr<const class Contact> Contact(
        const Identifier& id) const = 0;
    /** Returns the contact ID for a nym, if it exists */
    EXPORT virtual OTIdentifier ContactID(
        const identifier::Nym& nymID) const = 0;
    EXPORT virtual ObjectList ContactList() const = 0;
    EXPORT virtual std::string ContactName(
        const Identifier& contactID) const = 0;
    EXPORT virtual std::shared_ptr<const class Contact> Merge(
        const Identifier& parent,
        const Identifier& child) const = 0;
    EXPORT virtual std::unique_ptr<Editor<class Contact>> mutable_Contact(
        const Identifier& id) const = 0;
    EXPORT virtual std::shared_ptr<const class Contact> NewContact(
        const std::string& label) const = 0;
    EXPORT virtual std::shared_ptr<const class Contact> NewContact(
        const std::string& label,
        const identifier::Nym& nymID
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
        ,
        const PaymentCode& paymentCode
#endif
        ) const = 0;
    EXPORT virtual std::shared_ptr<const class Contact> NewContactFromAddress(
        const std::string& address,
        const std::string& label,
        const proto::ContactItemType currency = proto::CITEMTYPE_BTC) const = 0;
    /** Returns an existing contact ID if it exists, or creates a new one */
    EXPORT virtual OTIdentifier NymToContact(
        const identifier::Nym& nymID) const = 0;
    EXPORT virtual std::shared_ptr<const class Contact> Update(
        const proto::CredentialIndex& nym) const = 0;

    virtual ~Contacts() = default;

protected:
    Contacts() = default;

private:
    Contacts(const Contacts&) = delete;
    Contacts(Contacts&&) = delete;
    Contacts& operator=(const Contacts&) = delete;
    Contacts& operator=(Contacts&&) = delete;
};
}  // namespace client
}  // namespace api
}  // namespace opentxs
#endif
