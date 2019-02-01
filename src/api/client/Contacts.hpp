// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::api::client::implementation
{
class Contacts final : public client::internal::Contacts
{
public:
    OTIdentifier BlockchainAddressToContact(
        const std::string& address,
        const proto::ContactItemType currency =
            proto::CITEMTYPE_BTC) const override;
    std::shared_ptr<const class Contact> Contact(
        const Identifier& id) const override;
    OTIdentifier ContactID(const identifier::Nym& nymID) const override;
    ObjectList ContactList() const override;
    std::string ContactName(const Identifier& contactID) const override;
    std::shared_ptr<const class Contact> Merge(
        const Identifier& parent,
        const Identifier& child) const override;
    std::unique_ptr<Editor<class Contact>> mutable_Contact(
        const Identifier& id) const override;
    std::shared_ptr<const class Contact> NewContact(
        const std::string& label) const override;
    std::shared_ptr<const class Contact> NewContact(
        const std::string& label,
        const identifier::Nym& nymID
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
        ,
        const PaymentCode& paymentCode
#endif
        ) const override;
    std::shared_ptr<const class Contact> NewContactFromAddress(
        const std::string& address,
        const std::string& label,
        const proto::ContactItemType currency =
            proto::CITEMTYPE_BTC) const override;
    OTIdentifier NymToContact(const identifier::Nym& nymID) const override;
    std::shared_ptr<const class Contact> Update(
        const proto::CredentialIndex& nym) const override;

    ~Contacts() = default;

private:
    friend opentxs::Factory;

    using ContactLock = std::pair<std::mutex, std::shared_ptr<class Contact>>;
    using Address = std::pair<proto::ContactItemType, std::string>;
    using ContactMap = std::map<OTIdentifier, ContactLock>;
    using ContactNameMap = std::map<OTIdentifier, std::string>;

    const api::Core& api_;
    mutable std::recursive_mutex lock_{};
    mutable ContactMap contact_map_{};
    mutable ContactNameMap contact_name_map_;
    OTZMQPublishSocket publisher_;

    static ContactNameMap build_name_map(const api::storage::Storage& storage);

    void check_identifiers(
        const identifier::Nym& inputNymID,
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
        const PaymentCode& paymentCode,
#endif
        bool& haveNymID,
        bool& havePaymentCode,
        OTNymID& outputNymID) const;
    bool verify_write_lock(const rLock& lock) const;

    // takes ownership
    ContactMap::iterator add_contact(const rLock& lock, class Contact* contact)
        const;
    OTIdentifier address_to_contact(
        const rLock& lock,
        const std::string& address,
        const proto::ContactItemType currency) const;
    std::shared_ptr<const class Contact> contact(
        const rLock& lock,
        const std::string& label) const;
    std::shared_ptr<const class Contact> contact(
        const rLock& lock,
        const Identifier& id) const;
    void import_contacts(const rLock& lock);
    void init_nym_map(const rLock& lock);
    ContactMap::iterator load_contact(const rLock& lock, const Identifier& id)
        const;
    std::unique_ptr<Editor<class Contact>> mutable_contact(
        const rLock& lock,
        const Identifier& id) const;
    ContactMap::iterator obtain_contact(const rLock& lock, const Identifier& id)
        const;
    std::shared_ptr<const class Contact> new_contact(
        const rLock& lock,
        const std::string& label,
        const identifier::Nym& nymID
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
        ,
        const PaymentCode& paymentCode
#endif
        ) const;
    void refresh_indices(const rLock& lock, class Contact& contact) const;
    void save(class Contact* contact) const;
    void start() override;
    std::shared_ptr<const class Contact> update_existing_contact(
        const rLock& lock,
        const std::string& label,
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
        const PaymentCode& code,
#endif
        const Identifier& contactID) const;
    void update_nym_map(
        const rLock& lock,
        const OTNymID nymID,
        class Contact& contact,
        const bool replace = false) const;

    Contacts(const api::Core& api);
    Contacts() = delete;
    Contacts(const Contacts&) = delete;
    Contacts(Contacts&&) = delete;
    Contacts& operator=(const Contacts&) = delete;
    Contacts& operator=(Contacts&&) = delete;
};
}  // namespace opentxs::api::client::implementation
