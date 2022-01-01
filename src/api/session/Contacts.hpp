// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/UnitType.hpp"

#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <type_traits>
#include <utility>

#include "Proto.hpp"
#include "internal/api/session/Contacts.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/contact/ClaimType.hpp"
#include "opentxs/core/Editor.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"

namespace opentxs
{
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto

namespace session
{
class Client;
class Contacts;
class Storage;
}  // namespace session

class Session;
}  // namespace api

namespace contact
{
class Contact;
}  // namespace contact

namespace identifier
{
class Nym;
}  // namespace identifier

namespace identity
{
class Nym;
}  // namespace identity

namespace proto
{
class Contact;
class Nym;
}  // namespace proto

class PaymentCode;
}  // namespace opentxs

namespace opentxs::api::session::imp
{
class Contacts final : public session::internal::Contacts
{
public:
    auto Contact(const Identifier& id) const
        -> std::shared_ptr<const contact::Contact> final;
    auto ContactID(const identifier::Nym& nymID) const -> OTIdentifier final;
    auto ContactList() const -> ObjectList final;
    auto ContactName(const Identifier& contactID) const -> std::string final;
    auto ContactName(const Identifier& contactID, core::UnitType currencyHint)
        const -> std::string final;
    auto Merge(const Identifier& parent, const Identifier& child) const
        -> std::shared_ptr<const contact::Contact> final;
    auto mutable_Contact(const Identifier& id) const
        -> std::unique_ptr<Editor<contact::Contact>> final;
    auto NewContact(const std::string& label) const
        -> std::shared_ptr<const contact::Contact> final;
    auto NewContact(
        const std::string& label,
        const identifier::Nym& nymID,
        const PaymentCode& paymentCode) const
        -> std::shared_ptr<const contact::Contact> final;
    auto NewContactFromAddress(
        const std::string& address,
        const std::string& label,
        const opentxs::blockchain::Type currency) const
        -> std::shared_ptr<const contact::Contact> final;
    auto NymToContact(const identifier::Nym& nymID) const -> OTIdentifier final;
    auto PaymentCodeToContact(
        const PaymentCode& code,
        const opentxs::blockchain::Type currency) const -> OTIdentifier final;
    auto PaymentCodeToContact(
        const std::string& code,
        const opentxs::blockchain::Type currency) const -> OTIdentifier final;
    auto Update(const identity::Nym& nym) const
        -> std::shared_ptr<const contact::Contact> final;

    Contacts(const api::session::Client& api);

    ~Contacts() final = default;

private:
    using ContactLock =
        std::pair<std::mutex, std::shared_ptr<contact::Contact>>;
    using Address = std::pair<contact::ClaimType, std::string>;
    using ContactMap = std::pmr::map<OTIdentifier, ContactLock>;
    using ContactNameMap = std::pmr::map<OTIdentifier, std::string>;

    const api::session::Client& api_;
    mutable std::recursive_mutex lock_{};
    std::weak_ptr<const crypto::Blockchain> blockchain_;
    mutable ContactMap contact_map_{};
    mutable ContactNameMap contact_name_map_;
    OTZMQPublishSocket publisher_;

    void check_identifiers(
        const Identifier& inputNymID,
        const PaymentCode& paymentCode,
        bool& haveNymID,
        bool& havePaymentCode,
        identifier::Nym& outputNymID) const;
    auto verify_write_lock(const rLock& lock) const -> bool;

    // takes ownership
    auto add_contact(const rLock& lock, contact::Contact* contact) const
        -> ContactMap::iterator;
    auto contact(const rLock& lock, const std::string& label) const
        -> std::shared_ptr<const contact::Contact>;
    auto contact(const rLock& lock, const Identifier& id) const
        -> std::shared_ptr<const contact::Contact>;
    void import_contacts(const rLock& lock);
    auto init(const std::shared_ptr<const crypto::Blockchain>& blockchain)
        -> void final;
    void init_nym_map(const rLock& lock);
    auto load_contact(const rLock& lock, const Identifier& id) const
        -> ContactMap::iterator;
    auto mutable_contact(const rLock& lock, const Identifier& id) const
        -> std::unique_ptr<Editor<contact::Contact>>;
    auto obtain_contact(const rLock& lock, const Identifier& id) const
        -> ContactMap::iterator;
    auto new_contact(
        const rLock& lock,
        const std::string& label,
        const identifier::Nym& nymID,
        const PaymentCode& paymentCode) const
        -> std::shared_ptr<const contact::Contact>;
    void prepare_shutdown() final { blockchain_.reset(); }
    void refresh_indices(const rLock& lock, contact::Contact& contact) const;
    void save(contact::Contact* contact) const;
    void start() final;
    auto update_existing_contact(
        const rLock& lock,
        const std::string& label,
        const PaymentCode& code,
        const Identifier& contactID) const
        -> std::shared_ptr<const contact::Contact>;
    void update_nym_map(
        const rLock& lock,
        const identifier::Nym& nymID,
        contact::Contact& contact,
        const bool replace = false) const;

    Contacts() = delete;
    Contacts(const Contacts&) = delete;
    Contacts(Contacts&&) = delete;
    auto operator=(const Contacts&) -> Contacts& = delete;
    auto operator=(Contacts&&) -> Contacts& = delete;
};
}  // namespace opentxs::api::session::imp
