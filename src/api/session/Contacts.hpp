// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::UnitType
// IWYU pragma: no_forward_declare opentxs::blockchain::Type
// IWYU pragma: no_forward_declare opentxs::identity::wot::claim::ClaimType
// IWYU pragma: no_include "opentxs/core/UnitType.hpp"

#pragma once

#include <memory>
#include <mutex>
#include <type_traits>
#include <utility>

#include "internal/api/session/Contacts.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/socket/Publish.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/Editor.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/identity/wot/claim/ClaimType.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
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

namespace identifier
{
class Nym;
}  // namespace identifier

namespace identity
{
class Nym;
}  // namespace identity

namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network

namespace proto
{
class Contact;
class Nym;
}  // namespace proto

class Contact;
class PaymentCode;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::imp
{
class Contacts final : public session::internal::Contacts
{
public:
    auto Contact(const identifier::Generic& id) const
        -> std::shared_ptr<const opentxs::Contact> final;
    auto ContactID(const identifier::Nym& nymID) const
        -> identifier::Generic final;
    auto ContactList() const -> ObjectList final;
    auto ContactName(const identifier::Generic& contactID) const
        -> UnallocatedCString final;
    auto ContactName(
        const identifier::Generic& contactID,
        UnitType currencyHint) const -> UnallocatedCString final;
    auto Merge(
        const identifier::Generic& parent,
        const identifier::Generic& child) const
        -> std::shared_ptr<const opentxs::Contact> final;
    auto mutable_Contact(const identifier::Generic& id) const
        -> std::unique_ptr<Editor<opentxs::Contact>> final;
    auto NewContact(const UnallocatedCString& label) const
        -> std::shared_ptr<const opentxs::Contact> final;
    auto NewContact(
        const UnallocatedCString& label,
        const identifier::Nym& nymID,
        const PaymentCode& paymentCode) const
        -> std::shared_ptr<const opentxs::Contact> final;
    auto NewContactFromAddress(
        const UnallocatedCString& address,
        const UnallocatedCString& label,
        const opentxs::blockchain::Type currency) const
        -> std::shared_ptr<const opentxs::Contact> final;
    auto NymToContact(const identifier::Nym& nymID) const
        -> identifier::Generic final;
    auto PaymentCodeToContact(
        const PaymentCode& code,
        const opentxs::blockchain::Type currency) const
        -> identifier::Generic final;
    auto PaymentCodeToContact(
        const UnallocatedCString& code,
        const opentxs::blockchain::Type currency) const
        -> identifier::Generic final;

    Contacts(const api::session::Client& api);
    Contacts() = delete;
    Contacts(const Contacts&) = delete;
    Contacts(Contacts&&) = delete;
    auto operator=(const Contacts&) -> Contacts& = delete;
    auto operator=(Contacts&&) -> Contacts& = delete;

    ~Contacts() final;

private:
    enum class Work : OTZMQWorkType {
        shutdown = value(WorkType::Shutdown),
        nymcreated = value(WorkType::NymCreated),
        nymupdated = value(WorkType::NymUpdated),
        refresh = OT_ZMQ_INTERNAL_SIGNAL + 0,
    };

    using ContactLock =
        std::pair<std::mutex, std::shared_ptr<opentxs::Contact>>;
    using Address =
        std::pair<identity::wot::claim::ClaimType, UnallocatedCString>;
    using ContactMap = UnallocatedMap<identifier::Generic, ContactLock>;
    using ContactNameMap =
        UnallocatedMap<identifier::Generic, UnallocatedCString>;

    const api::session::Client& api_;
    mutable std::recursive_mutex lock_{};
    std::weak_ptr<const crypto::Blockchain> blockchain_;
    mutable ContactMap contact_map_{};
    mutable ContactNameMap contact_name_map_;
    OTZMQPublishSocket publisher_;
    opentxs::network::zeromq::Pipeline pipeline_;
    Timer timer_;

    void check_identifiers(
        const identifier::Generic& inputNymID,
        const PaymentCode& paymentCode,
        bool& haveNymID,
        bool& havePaymentCode,
        identifier::Nym& outputNymID) const;
    auto check_nyms() noexcept -> void;
    auto refresh_nyms() noexcept -> void;
    auto verify_write_lock(const rLock& lock) const -> bool;

    // takes ownership
    auto add_contact(const rLock& lock, opentxs::Contact* contact) const
        -> ContactMap::iterator;
    auto contact(const rLock& lock, const UnallocatedCString& label) const
        -> std::shared_ptr<const opentxs::Contact>;
    auto contact(const rLock& lock, const identifier::Generic& id) const
        -> std::shared_ptr<const opentxs::Contact>;
    void import_contacts(const rLock& lock);
    auto init(const std::shared_ptr<const crypto::Blockchain>& blockchain)
        -> void final;
    void init_nym_map(const rLock& lock);
    auto load_contact(const rLock& lock, const identifier::Generic& id) const
        -> ContactMap::iterator;
    auto mutable_contact(const rLock& lock, const identifier::Generic& id) const
        -> std::unique_ptr<Editor<opentxs::Contact>>;
    auto obtain_contact(const rLock& lock, const identifier::Generic& id) const
        -> ContactMap::iterator;
    auto new_contact(
        const rLock& lock,
        const UnallocatedCString& label,
        const identifier::Nym& nymID,
        const PaymentCode& paymentCode) const
        -> std::shared_ptr<const opentxs::Contact>;
    auto pipeline(opentxs::network::zeromq::Message&&) noexcept -> void;
    auto prepare_shutdown() -> void final { blockchain_.reset(); }
    auto refresh_indices(const rLock& lock, opentxs::Contact& contact) const
        -> void;
    auto save(opentxs::Contact* contact) const -> void;
    auto start() -> void final;
    auto update(const identity::Nym& nym) const
        -> std::shared_ptr<const opentxs::Contact>;
    auto update_existing_contact(
        const rLock& lock,
        const UnallocatedCString& label,
        const PaymentCode& code,
        const identifier::Generic& contactID) const
        -> std::shared_ptr<const opentxs::Contact>;
    void update_nym_map(
        const rLock& lock,
        const identifier::Nym& nymID,
        opentxs::Contact& contact,
        const bool replace = false) const;
};
}  // namespace opentxs::api::session::imp
