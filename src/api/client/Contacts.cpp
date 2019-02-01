// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Identity.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/PublishSocket.hpp"
#include "opentxs/Proto.hpp"

#include "InternalClient.hpp"

#include <functional>
#include <map>
#include <mutex>
#include <tuple>

#include "Contacts.hpp"

#define OT_METHOD "opentxs::api::implementation::Contacts::"

namespace opentxs
{
api::client::internal::Contacts* Factory::Contacts(const api::Core& api)
{
    return new opentxs::api::client::implementation::Contacts(api);
}
}  // namespace opentxs

namespace opentxs::api::client::implementation
{
Contacts::Contacts(const api::Core& api)
    : api_(api)
    , lock_()
    , contact_map_()
    , contact_name_map_(build_name_map(api.Storage()))
    , publisher_(api.ZeroMQ().PublishSocket())
{
    // WARNING: do not access api_.Wallet() during construction
    publisher_->Start(api_.Endpoints().ContactUpdate());
}

Contacts::ContactMap::iterator Contacts::add_contact(
    const rLock& lock,
    class Contact* contact) const
{
    OT_ASSERT(nullptr != contact);

    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    const auto& id = contact->ID();
    auto& it = contact_map_[id];
    it.second.reset(contact);

    return contact_map_.find(id);
}

OTIdentifier Contacts::address_to_contact(
    const rLock& lock,
    const std::string& address,
    const proto::ContactItemType currency) const
{
    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    const auto contact =
        api_.Storage().BlockchainAddressOwner(currency, address);

    return Identifier::Factory(contact);
}

OTIdentifier Contacts::BlockchainAddressToContact(
    const std::string& address,
    const proto::ContactItemType currency) const
{
    rLock lock(lock_);

    return address_to_contact(lock, address, currency);
}

Contacts::ContactNameMap Contacts::build_name_map(
    const api::storage::Storage& storage)
{
    ContactNameMap output;

    for (const auto& [id, alias] : storage.ContactList()) {
        output.emplace(Identifier::Factory(id), alias);
    }

    return output;
}

void Contacts::check_identifiers(
    const identifier::Nym& inputNymID,
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    const PaymentCode& paymentCode,
#endif
    bool& haveNymID,
    bool& havePaymentCode,
    OTNymID& outputNymID) const
{
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    if (paymentCode.VerifyInternally()) { havePaymentCode = true; }
#else
    havePaymentCode = false;
#endif

    if (false == inputNymID.empty()) {
        haveNymID = true;
        outputNymID = inputNymID;
    }
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    else if (havePaymentCode) {
        haveNymID = true;
        outputNymID = identifier::Nym::Factory(paymentCode.ID()->str());
    }
#endif
}

std::shared_ptr<const class Contact> Contacts::contact(
    const rLock& lock,
    const Identifier& id) const
{
    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    const auto it = obtain_contact(lock, id);

    if (contact_map_.end() != it) { return it->second.second; }

    return {};
}

std::shared_ptr<const class Contact> Contacts::contact(
    const rLock& lock,
    const std::string& label) const
{
    std::unique_ptr<class Contact> contact(
        new class Contact(api_.Wallet(), label));

    if (false == bool(contact)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to create new contact.")
            .Flush();

        return {};
    }

    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    const auto& contactID = contact->ID();

    OT_ASSERT(0 == contact_map_.count(contactID));

    auto it = add_contact(lock, contact.release());
    auto& output = it->second.second;

    if (false == api_.Storage().Store(*output)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to create save contact.")
            .Flush();
        contact_map_.erase(it);

        return {};
    }

    return output;
}

std::shared_ptr<const class Contact> Contacts::Contact(
    const Identifier& id) const
{
    rLock lock(lock_);

    return contact(lock, id);
}

OTIdentifier Contacts::ContactID(const identifier::Nym& nymID) const
{
    return Identifier::Factory(api_.Storage().ContactOwnerNym(nymID.str()));
}

ObjectList Contacts::ContactList() const
{
    return api_.Storage().ContactList();
}

std::string Contacts::ContactName(const Identifier& contactID) const
{
    rLock lock(lock_);
    auto it = contact_name_map_.find(contactID);

    if (contact_name_map_.end() == it) { return {}; }

    return it->second;
}

void Contacts::import_contacts(const rLock& lock)
{
    auto nyms = api_.Wallet().NymList();

    for (const auto& it : nyms) {
        const auto nymID = identifier::Nym::Factory(it.first);
        const auto contactID = api_.Storage().ContactOwnerNym(nymID->str());

        if (contactID.empty()) {
            const auto nym = api_.Wallet().Nym(nymID);

            if (false == bool(nym)) {
                throw std::runtime_error("Unable to load nym");
            }

            switch (nym->Claims().Type()) {
                case proto::CITEMTYPE_INDIVIDUAL:
                case proto::CITEMTYPE_ORGANIZATION:
                case proto::CITEMTYPE_BUSINESS:
                case proto::CITEMTYPE_GOVERNMENT: {
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
                    auto code = api_.Factory().PaymentCode(nym->PaymentCode());
#endif
                    new_contact(
                        lock,
                        nym->Alias(),
                        nymID
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
                        ,
                        code
#endif
                    );
                } break;
                case proto::CITEMTYPE_ERROR:
                case proto::CITEMTYPE_SERVER:
                default: {
                }
            }
        }
    }
}

void Contacts::init_nym_map(const rLock& lock)
{
    LogDetail(OT_METHOD)(__FUNCTION__)(": Upgrading indices.").Flush();

    for (const auto& it : api_.Storage().ContactList()) {
        const auto& contactID = Identifier::Factory(it.first);
        auto loaded = load_contact(lock, contactID);

        if (contact_map_.end() == loaded) {

            throw std::runtime_error("failed to load contact");
        }

        auto& contact = loaded->second.second;

        if (false == bool(contact)) {

            throw std::runtime_error("null contact pointer");
        }

        const auto type = contact->Type();

        if (proto::CITEMTYPE_ERROR == type) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid contact ")(it.first)(
                ".")
                .Flush();
            api_.Storage().DeleteContact(it.first);
        }

        const auto nyms = contact->Nyms();

        for (const auto& nym : nyms) { update_nym_map(lock, nym, *contact); }
    }

    api_.Storage().ContactSaveIndices();
}

Contacts::ContactMap::iterator Contacts::load_contact(
    const rLock& lock,
    const Identifier& id) const
{
    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    std::shared_ptr<proto::Contact> serialized{nullptr};
    const auto loaded = api_.Storage().Load(id.str(), serialized, SILENT);

    if (false == loaded) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Unable to load contact ")(id)
            .Flush();

        return contact_map_.end();
    }

    OT_ASSERT(serialized);

    std::unique_ptr<class Contact> contact(
        new class Contact(api_.Wallet(), *serialized));

    if (false == bool(contact)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to instantate serialized contact.")
            .Flush();

        return contact_map_.end();
    }

    return add_contact(lock, contact.release());
}

std::shared_ptr<const class Contact> Contacts::Merge(
    const Identifier& parent,
    const Identifier& child) const
{
    rLock lock(lock_);
    auto childContact = contact(lock, child);

    if (false == bool(childContact)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Child contact ")(child)(
            " can not be loaded.")
            .Flush();

        return {};
    }

    const auto& childID = childContact->ID();

    if (childID != child) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Child contact ")(child)(
            " is already merged into ")(childID)(".")
            .Flush();

        return {};
    }

    auto parentContact = contact(lock, parent);

    if (false == bool(parentContact)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Parent contact ")(parent)(
            " can not be loaded.")
            .Flush();

        return {};
    }

    const auto& parentID = parentContact->ID();

    if (parentID != parent) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Parent contact ")(parent)(
            " is merged into ")(parentID)(".")
            .Flush();

        return {};
    }

    OT_ASSERT(childContact);
    OT_ASSERT(parentContact);

    auto& lhs = const_cast<class Contact&>(*parentContact);
    auto& rhs = const_cast<class Contact&>(*childContact);
    lhs += rhs;

    if (false == api_.Storage().Store(rhs)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to create save child contact.")
            .Flush();

        OT_FAIL;
    }

    if (false == api_.Storage().Store(lhs)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to create save parent contact.")
            .Flush();

        OT_FAIL;
    }

    contact_map_.erase(child);

    return parentContact;
}

std::unique_ptr<Editor<class Contact>> Contacts::mutable_contact(
    const rLock& lock,
    const Identifier& id) const
{
    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    std::unique_ptr<Editor<class Contact>> output{nullptr};

    auto it = contact_map_.find(id);

    if (contact_map_.end() == it) { it = load_contact(lock, id); }

    if (contact_map_.end() == it) { return {}; }

    std::function<void(class Contact*)> callback =
        [&](class Contact* in) -> void { this->save(in); };
    output.reset(new Editor<class Contact>(it->second.second.get(), callback));

    return output;
}

std::unique_ptr<Editor<class Contact>> Contacts::mutable_Contact(
    const Identifier& id) const
{
    rLock lock(lock_);
    auto output = mutable_contact(lock, id);
    lock.unlock();

    return output;
}

std::shared_ptr<const class Contact> Contacts::new_contact(
    const rLock& lock,
    const std::string& label,
    const identifier::Nym& nymID
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    ,
    const PaymentCode& code
#endif
    ) const
{
    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    bool haveNymID{false};
    bool havePaymentCode{false};
    auto inputNymID = identifier::Nym::Factory();
    check_identifiers(
        nymID,
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
        code,
#endif
        haveNymID,
        havePaymentCode,
        inputNymID);

    if (haveNymID) {
        const auto contactID = api_.Storage().ContactOwnerNym(nymID.str());

        if (false == contactID.empty()) {

            return update_existing_contact(
                lock,
                label,
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
                code,
#endif
                Identifier::Factory(contactID));
        }
    }

    auto newContact = contact(lock, label);

    if (false == bool(newContact)) { return {}; }

    OTIdentifier contactID = newContact->ID();
    newContact.reset();
    auto output = mutable_contact(lock, contactID);

    if (false == bool(output)) { return {}; }

    auto& mContact = output->It();

    if (false == nymID.empty()) {
        auto nym = api_.Wallet().Nym(nymID);

        if (nym) {
            mContact.AddNym(nym, true);
        } else {
            mContact.AddNym(nymID, true);
        }

        update_nym_map(lock, nymID, mContact, true);
    }

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    if (code.VerifyInternally()) { mContact.AddPaymentCode(code, true); }
#endif

    output.reset();

    return contact(lock, contactID);
}

std::shared_ptr<const class Contact> Contacts::NewContact(
    const std::string& label) const
{
    rLock lock(lock_);

    return contact(lock, label);
}

std::shared_ptr<const class Contact> Contacts::NewContact(
    const std::string& label,
    const identifier::Nym& nymID
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    ,
    const PaymentCode& paymentCode
#endif
    ) const
{
    rLock lock(lock_);

    return new_contact(
        lock,
        label,
        nymID
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
        ,
        paymentCode
#endif
    );
}

std::shared_ptr<const class Contact> Contacts::NewContactFromAddress(
    const std::string& address,
    const std::string& label,
    const proto::ContactItemType currency) const
{
    rLock lock(lock_);

    const auto existingID = address_to_contact(lock, address, currency);

    if (false == existingID->empty()) { return contact(lock, existingID); }

    auto newContact = contact(lock, label);

    OT_ASSERT(newContact);

    const auto newContactID = Identifier::Factory(newContact->ID());
    auto& it = contact_map_.at(newContactID);
    auto& contact = *it.second;

    if (false == contact.AddBlockchainAddress(address, currency)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to add address to contact.")
            .Flush();

        OT_FAIL;
    }

    if (false == api_.Storage().Store(contact)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to save contact.").Flush();

        OT_FAIL;
    }

    return newContact;
}

OTIdentifier Contacts::NymToContact(const identifier::Nym& nymID) const
{
    const auto contactID = ContactID(nymID);

    if (false == contactID->empty()) { return contactID; }

    // Contact does not yet exist. Create it.
    std::string label{""};
    auto nym = api_.Wallet().Nym(nymID);
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    auto code = api_.Factory().PaymentCode("");
#endif

    if (nym) {
        label = nym->Claims().Name();
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
        code = api_.Factory().PaymentCode(nym->PaymentCode());
#endif
    }

    const auto contact = NewContact(
        label,
        nymID
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
        ,
        code
#endif
    );

    if (contact) { return Identifier::Factory(contact->ID()); }

    return Identifier::Factory();
}

Contacts::ContactMap::iterator Contacts::obtain_contact(
    const rLock& lock,
    const Identifier& id) const
{
    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    auto it = contact_map_.find(id);

    if (contact_map_.end() != it) { return it; }

    return load_contact(lock, id);
}

void Contacts::refresh_indices(const rLock& lock, class Contact& contact) const
{
    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    const auto nyms = contact.Nyms();

    for (const auto& nymid : nyms) {
        update_nym_map(lock, nymid, contact, true);
    }

    const auto& id = contact.ID();
    contact_name_map_[id] = contact.Label();
    const std::string rawID{id.str()};
    publisher_->Publish(rawID);
}

void Contacts::save(class Contact* contact) const
{
    OT_ASSERT(nullptr != contact);

    if (false == api_.Storage().Store(*contact)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to create or save contact.")
            .Flush();

        OT_FAIL;
    }

    const auto& id = contact->ID();

    if (false == api_.Storage().SetContactAlias(id.str(), contact->Label())) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to create or save contact.")
            .Flush();

        OT_FAIL;
    }

    rLock lock(lock_);
    refresh_indices(lock, *contact);
}

void Contacts::start()
{
    const auto level = api_.Storage().ContactUpgradeLevel();

    switch (level) {
        case 0:
        case 1: {
            rLock lock(lock_);
            init_nym_map(lock);
            import_contacts(lock);
        }
        case 2:
        default: {
        }
    }
}

std::shared_ptr<const class Contact> Contacts::Update(
    const proto::CredentialIndex& serialized) const
{
    auto nym = api_.Wallet().Nym(serialized);

    if (false == bool(nym)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid nym.").Flush();

        return {};
    }

    auto& data = nym->Claims();

    switch (data.Type()) {
        case proto::CITEMTYPE_INDIVIDUAL:
        case proto::CITEMTYPE_ORGANIZATION:
        case proto::CITEMTYPE_BUSINESS:
        case proto::CITEMTYPE_GOVERNMENT: {
            break;
        }
        default: {
            return {};
        }
    }

    const auto& nymID = nym->ID();
    rLock lock(lock_);
    const auto contactIdentifier = api_.Storage().ContactOwnerNym(nymID.str());
    const auto contactID = Identifier::Factory(contactIdentifier);

    if (contactIdentifier.empty()) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Nym ")(nymID)(
            " is not associated with a contact. Creating a new contact.")
            .Flush();
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
        auto code = api_.Factory().PaymentCode(nym->PaymentCode());
#endif
        return new_contact(
            lock,
            Contact::ExtractLabel(*nym),
            nymID
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
            ,
            code
#endif
        );
    }

    {
        auto contact = mutable_contact(lock, contactID);
        contact->It().Update(serialized);
        contact.reset();
    }

    auto contact = obtain_contact(lock, contactID);

    OT_ASSERT(contact_map_.end() != contact);

    auto& output = contact->second.second;

    OT_ASSERT(output);

    api_.Storage().RelabelThread(output->ID().str(), output->Label());

    return output;
}

std::shared_ptr<const class Contact> Contacts::update_existing_contact(
    const rLock& lock,
    const std::string& label,
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    const PaymentCode& code,
#endif
    const Identifier& contactID) const
{
    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    auto it = obtain_contact(lock, contactID);

    OT_ASSERT(contact_map_.end() != it);

    auto& contactMutex = it->second.first;
    auto& contact = it->second.second;

    OT_ASSERT(contact);

    Lock contactLock(contactMutex);
    const auto& existingLabel = contact->Label();

    if ((existingLabel != label) && (false == label.empty())) {
        contact->SetLabel(label);
    }

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    contact->AddPaymentCode(code, true);
#endif
    save(contact.get());

    return contact;
}

void Contacts::update_nym_map(
    const rLock& lock,
    const OTNymID nymID,
    class Contact& contact,
    const bool replace) const
{
    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    const auto contactIdentifier = api_.Storage().ContactOwnerNym(nymID->str());
    const bool exists = (false == contactIdentifier.empty());
    const auto& incomingID = contact.ID();
    const auto contactID = Identifier::Factory(contactIdentifier);
    const bool same = (incomingID == contactID);

    if (exists && (false == same)) {
        if (replace) {
            auto it = load_contact(lock, contactID);

            if (contact_map_.end() != it) {

                throw std::runtime_error("contact not found");
            }

            auto& oldContact = it->second.second;

            if (false == bool(oldContact)) {
                throw std::runtime_error("null contact pointer");
            }

            oldContact->RemoveNym(nymID);

            if (false == api_.Storage().Store(*oldContact)) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Unable to create or save contact.")
                    .Flush();

                OT_FAIL;
            }
        } else {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Duplicate nym found.")
                .Flush();
            contact.RemoveNym(nymID);

            if (false == api_.Storage().Store(contact)) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Unable to create or save contact.")
                    .Flush();

                OT_FAIL;
            }

            return;
        }
    }
}

bool Contacts::verify_write_lock(const rLock& lock) const
{
    if (lock.mutex() != &lock_) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect mutex.").Flush();

        return false;
    }

    if (false == lock.owns_lock()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Lock not owned.").Flush();

        return false;
    }

    return true;
}
}  // namespace opentxs::api::client::implementation
