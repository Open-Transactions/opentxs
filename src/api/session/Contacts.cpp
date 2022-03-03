// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"              // IWYU pragma: associated
#include "1_Internal.hpp"            // IWYU pragma: associated
#include "api/session/Contacts.hpp"  // IWYU pragma: associated

#include <boost/system/error_code.hpp>
#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <type_traits>

#include "internal/api/crypto/Blockchain.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/api/session/Factory.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/BoostPMR.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Contact.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/wot/claim/Data.hpp"
#include "opentxs/identity/wot/claim/Group.hpp"
#include "opentxs/identity/wot/claim/Item.hpp"
#include "opentxs/identity/wot/claim/Section.hpp"
#include "opentxs/identity/wot/claim/SectionType.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/WorkType.hpp"
#include "serialization/protobuf/Contact.pb.h"  // IWYU pragma: keep
#include "serialization/protobuf/Nym.pb.h"      // IWYU pragma: keep

namespace opentxs::factory
{
auto ContactAPI(const api::session::Client& api) noexcept
    -> std::unique_ptr<api::session::Contacts>
{
    using ReturnType = opentxs::api::session::imp::Contacts;

    return std::make_unique<ReturnType>(api);
}
}  // namespace opentxs::factory

namespace opentxs::api::session::imp
{
Contacts::Contacts(const api::session::Client& api)
    : api_(api)
    , lock_()
    , blockchain_()
    , contact_map_()
    , contact_name_map_([&] {
        auto output = ContactNameMap{};

        for (const auto& [id, alias] : api_.Storage().ContactList()) {
            output.emplace(api_.Factory().Identifier(id), alias);
        }

        return output;
    }())
    , publisher_(api_.Network().ZeroMQ().PublishSocket())
    , pipeline_(api_.Network().ZeroMQ().Internal().Pipeline(
          [this](auto&& in) { pipeline(std::move(in)); }))
    , timer_(api_.Network().Asio().Internal().GetTimer())
{
    // WARNING: do not access api_.Wallet() during construction
    publisher_->Start(api_.Endpoints().ContactUpdate().data());

    // TODO update Storage to record contact ids that need to be updated
    // in blockchain api in cases where the process was interrupted due to
    // library shutdown

    LogTrace()(OT_PRETTY_CLASS())("using ZMQ batch ")(pipeline_.BatchID())
        .Flush();
    pipeline_.SubscribeTo(api_.Endpoints().NymCreated());
    pipeline_.SubscribeTo(api_.Endpoints().NymDownload());
    pipeline_.SubscribeTo(api_.Endpoints().Shutdown());
}

auto Contacts::add_contact(const rLock& lock, opentxs::Contact* contact) const
    -> Contacts::ContactMap::iterator
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

void Contacts::check_identifiers(
    const Identifier& inputNymID,
    const PaymentCode& paymentCode,
    bool& haveNymID,
    bool& havePaymentCode,
    identifier::Nym& outputNymID) const
{
    if (paymentCode.Valid()) { havePaymentCode = true; }

    if (false == inputNymID.empty()) {
        haveNymID = true;
        outputNymID.Assign(inputNymID);
    } else if (havePaymentCode) {
        haveNymID = true;
        outputNymID.Assign(paymentCode.ID());
    }
}

auto Contacts::check_nyms() noexcept -> void
{
    auto buf = std::array<std::byte, 4096>{};
    auto alloc = alloc::BoostMonotonic{buf.data(), buf.size()};
    const auto contacts = [&] {
        auto out = Vector<OTIdentifier>{&alloc};
        auto lock = rLock{lock_};
        out.reserve(contact_name_map_.size());

        for (auto& [key, value] : contact_name_map_) { out.emplace_back(key); }

        return out;
    }();
    auto nyms = Vector<OTNymID>{&alloc};

    for (const auto& id : contacts) {
        const auto contact = Contact(id);

        OT_ASSERT(contact);

        auto ids = contact->Nyms();
        std::move(ids.begin(), ids.end(), std::back_inserter(nyms));
    }

    for (const auto& id : nyms) {
        const auto nym = api_.Wallet().Nym(id);

        if (nym) {
            LogInsane()(OT_PRETTY_CLASS())(id)("found").Flush();
        } else {
            LogInsane()(OT_PRETTY_CLASS())(id)("not found").Flush();
        }
    }
}

auto Contacts::contact(const rLock& lock, const Identifier& id) const
    -> std::shared_ptr<const opentxs::Contact>
{
    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    const auto it = obtain_contact(lock, id);

    if (contact_map_.end() != it) { return it->second.second; }

    return {};
}

auto Contacts::contact(const rLock& lock, const UnallocatedCString& label) const
    -> std::shared_ptr<const opentxs::Contact>
{
    auto contact = std::make_unique<opentxs::Contact>(api_, label);

    if (false == bool(contact)) {
        LogError()(OT_PRETTY_CLASS())("Unable to create new contact.").Flush();

        return {};
    }

    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    const auto& contactID = contact->ID();

    OT_ASSERT(0 == contact_map_.count(contactID));

    auto it = add_contact(lock, contact.release());
    auto& output = it->second.second;
    const auto proto = [&] {
        auto out = proto::Contact{};
        output->Serialize(out);
        return out;
    }();

    if (false == api_.Storage().Store(proto)) {
        LogError()(OT_PRETTY_CLASS())("Unable to save contact.").Flush();
        contact_map_.erase(it);

        return {};
    }

    contact_name_map_[contactID] = output->Label();
    // Not parsing changed addresses because this is a new contact

    return output;
}

auto Contacts::Contact(const Identifier& id) const
    -> std::shared_ptr<const opentxs::Contact>
{
    auto lock = rLock{lock_};

    return contact(lock, id);
}

auto Contacts::ContactID(const identifier::Nym& nymID) const -> OTIdentifier
{
    return api_.Factory().Identifier(
        api_.Storage().ContactOwnerNym(nymID.str()));
}

auto Contacts::ContactList() const -> ObjectList
{
    return api_.Storage().ContactList();
}

auto Contacts::ContactName(const Identifier& id) const -> UnallocatedCString
{
    return ContactName(id, UnitType::Error);
}

auto Contacts::ContactName(const Identifier& id, UnitType currencyHint) const
    -> UnallocatedCString
{
    auto alias = UnallocatedCString{};
    const auto fallback = [&](const rLock&) {
        if (false == alias.empty()) { return alias; }

        auto [it, added] = contact_name_map_.try_emplace(id, id.str());

        OT_ASSERT(added);

        return it->second;
    };
    auto lock = rLock{lock_};

    {
        auto it = contact_name_map_.find(id);

        if (contact_name_map_.end() != it) {
            alias = it->second;

            if (alias.empty()) { contact_name_map_.erase(it); }
        }
    }

    using Type = UnitType;

    if ((Type::Error == currencyHint) && (false == alias.empty())) {
        const auto isPaymentCode = [&] {
            auto code = api_.Factory().PaymentCode(alias);

            return code.Valid();
        }();

        if (false == isPaymentCode) { return alias; }
    }

    auto contact = this->contact(lock, id);

    if (!contact) { return fallback(lock); }

    if (auto& label = contact->Label(); false == label.empty()) {
        auto& output = contact_name_map_[id];
        output = std::move(label);

        return output;
    }

    const auto data = contact->Data();

    OT_ASSERT(data);

    if (auto name = data->Name(); false == name.empty()) {
        auto& output = contact_name_map_[id];
        output = std::move(name);

        return output;
    }

    using Section = identity::wot::claim::SectionType;

    if (Type::Error != currencyHint) {
        auto group = data->Group(Section::Procedure, UnitToClaim(currencyHint));

        if (group) {
            if (auto best = group->Best(); best) {
                if (auto value = best->Value(); false == value.empty()) {

                    return best->Value();
                }
            }
        }
    }

    const auto procedure = data->Section(Section::Procedure);

    if (procedure) {
        for (const auto& [type, group] : *procedure) {
            OT_ASSERT(group);

            if (0 < group->Size()) {
                const auto item = group->Best();

                OT_ASSERT(item);

                if (auto value = item->Value(); false == value.empty()) {

                    return value;
                }
            }
        }
    }

    return fallback(lock);
}

auto Contacts::import_contacts(const rLock& lock) -> void
{
    auto nyms = api_.Wallet().NymList();

    for (const auto& it : nyms) {
        const auto nymID = api_.Factory().NymID(it.first);
        const auto contactID = [&] {
            auto out = api_.Factory().Identifier();
            out->Assign(nymID->data(), nymID->size());

            return out;
        }();

        api_.Storage().ContactOwnerNym(nymID->str());

        if (contactID->empty()) {
            const auto nym = api_.Wallet().Nym(nymID);

            if (false == bool(nym)) {
                throw std::runtime_error("Unable to load nym");
            }

            switch (nym->Claims().Type()) {
                case identity::wot::claim::ClaimType::Individual:
                case identity::wot::claim::ClaimType::Organization:
                case identity::wot::claim::ClaimType::Business:
                case identity::wot::claim::ClaimType::Government: {
                    auto code = api_.Factory().PaymentCode(nym->PaymentCode());
                    new_contact(lock, nym->Alias(), nymID, code);
                } break;
                case identity::wot::claim::ClaimType::Error:
                case identity::wot::claim::ClaimType::Server:
                default: {
                }
            }
        }
    }
}

auto Contacts::init(const std::shared_ptr<const crypto::Blockchain>& blockchain)
    -> void
{
    OT_ASSERT(blockchain);

    blockchain_ = blockchain;

    OT_ASSERT(false == blockchain_.expired());
}

void Contacts::init_nym_map(const rLock& lock)
{
    LogDetail()(OT_PRETTY_CLASS())("Upgrading indices.").Flush();

    for (const auto& it : api_.Storage().ContactList()) {
        const auto& contactID = api_.Factory().Identifier(it.first);
        auto loaded = load_contact(lock, contactID);

        if (contact_map_.end() == loaded) {

            throw std::runtime_error("failed to load contact");
        }

        auto& contact = loaded->second.second;

        if (false == bool(contact)) {

            throw std::runtime_error("null contact pointer");
        }

        const auto type = contact->Type();

        if (identity::wot::claim::ClaimType::Error == type) {
            LogError()(OT_PRETTY_CLASS())("Invalid contact ")(it.first)(".")
                .Flush();
            api_.Storage().DeleteContact(it.first);
        }

        const auto nyms = contact->Nyms();

        for (const auto& nym : nyms) { update_nym_map(lock, nym, *contact); }
    }

    api_.Storage().ContactSaveIndices();
}

auto Contacts::load_contact(const rLock& lock, const Identifier& id) const
    -> Contacts::ContactMap::iterator
{
    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    auto serialized = proto::Contact{};
    const auto loaded = api_.Storage().Load(id.str(), serialized, SILENT);

    if (false == loaded) {
        LogDetail()(OT_PRETTY_CLASS())("Unable to load contact ")(id).Flush();

        return contact_map_.end();
    }

    auto contact = std::make_unique<opentxs::Contact>(api_, serialized);

    if (false == bool(contact)) {
        LogError()(OT_PRETTY_CLASS())(
            ": Unable to instantate serialized contact.")
            .Flush();

        return contact_map_.end();
    }

    return add_contact(lock, contact.release());
}

auto Contacts::Merge(const Identifier& parent, const Identifier& child) const
    -> std::shared_ptr<const opentxs::Contact>
{
    auto lock = rLock{lock_};
    auto childContact = contact(lock, child);

    if (false == bool(childContact)) {
        LogError()(OT_PRETTY_CLASS())("Child contact ")(
            child)(" can not be loaded.")
            .Flush();

        return {};
    }

    const auto& childID = childContact->ID();

    if (childID != child) {
        LogError()(OT_PRETTY_CLASS())("Child contact ")(
            child)(" is already merged into ")(childID)(".")
            .Flush();

        return {};
    }

    auto parentContact = contact(lock, parent);

    if (false == bool(parentContact)) {
        LogError()(OT_PRETTY_CLASS())("Parent contact ")(
            parent)(" can not be loaded.")
            .Flush();

        return {};
    }

    const auto& parentID = parentContact->ID();

    if (parentID != parent) {
        LogError()(OT_PRETTY_CLASS())("Parent contact ")(
            parent)(" is merged into ")(parentID)(".")
            .Flush();

        return {};
    }

    OT_ASSERT(childContact);
    OT_ASSERT(parentContact);

    auto& lhs = const_cast<opentxs::Contact&>(*parentContact);
    auto& rhs = const_cast<opentxs::Contact&>(*childContact);
    lhs += rhs;
    const auto lProto = [&] {
        auto out = proto::Contact{};
        lhs.Serialize(out);
        return out;
    }();
    const auto rProto = [&] {
        auto out = proto::Contact{};
        rhs.Serialize(out);
        return out;
    }();

    if (false == api_.Storage().Store(rProto)) {
        LogError()(OT_PRETTY_CLASS())(": Unable to create save child contact.")
            .Flush();

        OT_FAIL;
    }

    if (false == api_.Storage().Store(lProto)) {
        LogError()(OT_PRETTY_CLASS())(": Unable to create save parent contact.")
            .Flush();

        OT_FAIL;
    }

    contact_map_.erase(child);
    auto blockchain = blockchain_.lock();

    if (blockchain) {
        blockchain->Internal().ProcessMergedContact(lhs, rhs);
    } else {
        LogVerbose()(OT_PRETTY_CLASS())(
            ": Warning: contact not updated in blockchain API")
            .Flush();
    }

    return parentContact;
}

auto Contacts::mutable_contact(const rLock& lock, const Identifier& id) const
    -> std::unique_ptr<Editor<opentxs::Contact>>
{
    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    std::unique_ptr<Editor<opentxs::Contact>> output{nullptr};

    auto it = contact_map_.find(id);

    if (contact_map_.end() == it) { it = load_contact(lock, id); }

    if (contact_map_.end() == it) { return {}; }

    std::function<void(opentxs::Contact*)> callback =
        [&](opentxs::Contact* in) -> void { this->save(in); };
    output = std::make_unique<Editor<opentxs::Contact>>(
        it->second.second.get(), callback);

    return output;
}

auto Contacts::mutable_Contact(const Identifier& id) const
    -> std::unique_ptr<Editor<opentxs::Contact>>
{
    auto lock = rLock{lock_};
    auto output = mutable_contact(lock, id);
    lock.unlock();

    return output;
}

auto Contacts::new_contact(
    const rLock& lock,
    const UnallocatedCString& label,
    const identifier::Nym& nymID,
    const PaymentCode& code) const -> std::shared_ptr<const opentxs::Contact>
{
    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    bool haveNymID{false};
    bool havePaymentCode{false};
    auto inputNymID = identifier::Nym::Factory();
    check_identifiers(nymID, code, haveNymID, havePaymentCode, inputNymID);

    if (haveNymID) {
        const auto contactID =
            api_.Storage().ContactOwnerNym(inputNymID->str());

        if (false == contactID.empty()) {

            return update_existing_contact(
                lock, label, code, api_.Factory().Identifier(contactID));
        }
    }

    auto newContact = contact(lock, label);

    if (false == bool(newContact)) { return {}; }

    OTIdentifier contactID = newContact->ID();
    newContact.reset();
    auto output = mutable_contact(lock, contactID);

    if (false == bool(output)) { return {}; }

    auto& mContact = output->get();

    if (false == inputNymID->empty()) {
        auto nym = api_.Wallet().Nym(inputNymID);

        if (nym) {
            mContact.AddNym(nym, true);
        } else {
            mContact.AddNym(inputNymID, true);
        }

        update_nym_map(lock, inputNymID, mContact, true);
    }

    if (code.Valid()) { mContact.AddPaymentCode(code, true); }

    output.reset();

    return contact(lock, contactID);
}

auto Contacts::NewContact(const UnallocatedCString& label) const
    -> std::shared_ptr<const opentxs::Contact>
{
    auto lock = rLock{lock_};

    return contact(lock, label);
}

auto Contacts::NewContact(
    const UnallocatedCString& label,
    const identifier::Nym& nymID,
    const PaymentCode& paymentCode) const
    -> std::shared_ptr<const opentxs::Contact>
{
    auto lock = rLock{lock_};

    return new_contact(lock, label, nymID, paymentCode);
}

auto Contacts::NewContactFromAddress(
    const UnallocatedCString& address,
    const UnallocatedCString& label,
    const opentxs::blockchain::Type currency) const
    -> std::shared_ptr<const opentxs::Contact>
{
    auto blockchain = blockchain_.lock();

    if (false == bool(blockchain)) {
        LogVerbose()(OT_PRETTY_CLASS())("shutting down ").Flush();

        return {};
    }

    auto lock = rLock{lock_};
    const auto existing = blockchain->LookupContacts(address);

    switch (existing.size()) {
        case 0: {
        } break;
        case 1: {
            return contact(lock, *existing.cbegin());
        }
        default: {
            LogError()(OT_PRETTY_CLASS())(": multiple contacts claim address ")(
                address)
                .Flush();

            return {};
        }
    }

    auto newContact = contact(lock, label);

    OT_ASSERT(newContact);

    auto& it = contact_map_.at(newContact->ID());
    auto& contact = *it.second;

    if (false == contact.AddBlockchainAddress(address, currency)) {
        LogError()(OT_PRETTY_CLASS())(": Failed to add address to contact.")
            .Flush();

        OT_FAIL;
    }

    const auto proto = [&] {
        auto out = proto::Contact{};
        contact.Serialize(out);
        return out;
    }();

    if (false == api_.Storage().Store(proto)) {
        LogError()(OT_PRETTY_CLASS())("Unable to save contact.").Flush();

        OT_FAIL;
    }

    blockchain->Internal().ProcessContact(contact);

    return newContact;
}

auto Contacts::NymToContact(const identifier::Nym& nymID) const -> OTIdentifier
{
    const auto contactID = ContactID(nymID);

    if (false == contactID->empty()) { return contactID; }

    // Contact does not yet exist. Create it.
    UnallocatedCString label{""};
    auto nym = api_.Wallet().Nym(nymID);
    auto code = api_.Factory().PaymentCode(UnallocatedCString{});

    if (nym) {
        label = nym->Claims().Name();
        code = api_.Factory().PaymentCode(nym->PaymentCode());
    }

    const auto contact = NewContact(label, nymID, code);

    if (contact) { return contact->ID(); }

    static const auto blank = api_.Factory().Identifier();

    return blank;
}

auto Contacts::obtain_contact(const rLock& lock, const Identifier& id) const
    -> Contacts::ContactMap::iterator
{
    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    auto it = contact_map_.find(id);

    if (contact_map_.end() != it) { return it; }

    return load_contact(lock, id);
}

auto Contacts::PaymentCodeToContact(
    const UnallocatedCString& serialized,
    const opentxs::blockchain::Type currency) const -> OTIdentifier
{
    static const auto blank = api_.Factory().Identifier();
    const auto code = api_.Factory().PaymentCode(serialized);

    if (0 == code.Version()) { return blank; }

    return PaymentCodeToContact(code, currency);
}

auto Contacts::PaymentCodeToContact(
    const PaymentCode& code,
    const opentxs::blockchain::Type currency) const -> OTIdentifier
{
    // NOTE for now we assume that payment codes are always nym id sources. This
    // won't always be true.

    const auto id = NymToContact(code.ID());

    if (false == id->empty()) {
        auto lock = rLock{lock_};
        auto contactE = mutable_contact(lock, id);
        auto& contact = contactE->get();
        const auto chain = BlockchainToUnit(currency);
        const auto existing = contact.PaymentCode(chain);
        contact.AddPaymentCode(code, existing.empty(), chain);
    }

    return id;
}

auto Contacts::pipeline(opentxs::network::zeromq::Message&& in) noexcept -> void
{
    const auto body = in.Body();

    if (1 > body.size()) {
        LogError()(OT_PRETTY_CLASS())("Invalid message").Flush();

        OT_FAIL;
    }

    const auto work = [&] {
        try {

            return body.at(0).as<Work>();
        } catch (...) {

            OT_FAIL;
        }
    }();
    switch (work) {
        case Work::shutdown: {
            pipeline_.Close();
        } break;
        case Work::nymcreated:
        case Work::nymupdated: {
            OT_ASSERT(1 < body.size());

            const auto id = [&] {
                auto out = api_.Factory().NymID();
                out->Assign(body.at(1).Bytes());

                return out;
            }();
            const auto nym = api_.Wallet().Nym(id);

            OT_ASSERT(nym);

            update(*nym);
        } break;
        case Work::refresh: {
            check_nyms();
        } break;
        default: {
            LogError()(OT_PRETTY_CLASS())("Unhandled type").Flush();

            OT_FAIL;
        }
    }
}

auto Contacts::refresh_indices(const rLock& lock, opentxs::Contact& contact)
    const -> void
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
    publisher_->Send([&] {
        auto work =
            opentxs::network::zeromq::tagged_message(WorkType::ContactUpdated);
        work.AddFrame(id);

        return work;
    }());
}

auto Contacts::refresh_nyms() noexcept -> void
{
    static constexpr auto interval = std::chrono::minutes{5};
    timer_.SetRelative(interval);
    timer_.Wait([this](const auto& error) {
        if (error) {
            if (boost::system::errc::operation_canceled != error.value()) {
                LogError()(OT_PRETTY_CLASS())(error).Flush();
            }
        } else {
            pipeline_.Push(
                opentxs::network::zeromq::tagged_message(Work::refresh));
            refresh_nyms();
        }
    });
}

void Contacts::save(opentxs::Contact* contact) const
{
    OT_ASSERT(nullptr != contact);

    const auto proto = [&] {
        auto out = proto::Contact{};
        contact->Serialize(out);
        return out;
    }();

    if (false == api_.Storage().Store(proto)) {
        LogError()(OT_PRETTY_CLASS())(": Unable to create or save contact.")
            .Flush();

        OT_FAIL;
    }

    const auto& id = contact->ID();

    if (false == api_.Storage().SetContactAlias(id.str(), contact->Label())) {
        LogError()(OT_PRETTY_CLASS())(": Unable to create or save contact.")
            .Flush();

        OT_FAIL;
    }

    auto lock = rLock{lock_};
    refresh_indices(lock, *contact);
    auto blockchain = blockchain_.lock();

    if (blockchain) {
        blockchain->Internal().ProcessContact(*contact);
    } else {
        LogVerbose()(OT_PRETTY_CLASS())(
            ": Warning: contact not updated in blockchain API")
            .Flush();
    }
}

void Contacts::start()
{
    const auto level = api_.Storage().ContactUpgradeLevel();

    switch (level) {
        case 0:
        case 1: {
            auto lock = rLock{lock_};
            init_nym_map(lock);
            import_contacts(lock);
            [[fallthrough]];
        }
        case 2:
        default: {
        }
    }
}

auto Contacts::update(const identity::Nym& nym) const
    -> std::shared_ptr<const opentxs::Contact>
{
    auto& data = nym.Claims();

    switch (data.Type()) {
        case identity::wot::claim::ClaimType::Individual:
        case identity::wot::claim::ClaimType::Organization:
        case identity::wot::claim::ClaimType::Business:
        case identity::wot::claim::ClaimType::Government: {
            break;
        }
        default: {
            return {};
        }
    }

    const auto& nymID = nym.ID();
    auto lock = rLock{lock_};
    const auto contactIdentifier = api_.Storage().ContactOwnerNym(nymID.str());
    const auto contactID = api_.Factory().Identifier(contactIdentifier);
    const auto label = Contact::ExtractLabel(nym);

    if (contactIdentifier.empty()) {
        LogDetail()(OT_PRETTY_CLASS())("Nym ")(
            nymID)(" is not associated with a contact. Creating a "
                   "new contact named ")(label)
            .Flush();
        auto code = api_.Factory().PaymentCode(nym.PaymentCode());
        return new_contact(lock, label, nymID, code);
    }

    {
        auto contact = mutable_contact(lock, contactID);
        auto serialized = proto::Nym{};
        if (false == nym.Serialize(serialized)) {
            LogError()(OT_PRETTY_CLASS())("Failed to serialize nym.").Flush();
            return {};
        }
        contact->get().Update(serialized);
        contact.reset();
    }

    auto contact = obtain_contact(lock, contactID);

    OT_ASSERT(contact_map_.end() != contact);

    auto& output = contact->second.second;

    OT_ASSERT(output);

    api_.Storage().RelabelThread(output->ID().str(), output->Label());

    return output;
}

auto Contacts::update_existing_contact(
    const rLock& lock,
    const UnallocatedCString& label,
    const PaymentCode& code,
    const Identifier& contactID) const
    -> std::shared_ptr<const opentxs::Contact>
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

    contact->AddPaymentCode(code, true);
    save(contact.get());

    return contact;
}

void Contacts::update_nym_map(
    const rLock& lock,
    const identifier::Nym& nymID,
    opentxs::Contact& contact,
    const bool replace) const
{
    if (false == verify_write_lock(lock)) {
        throw std::runtime_error("lock error");
    }

    const auto contactIdentifier = api_.Storage().ContactOwnerNym(nymID.str());
    const bool exists = (false == contactIdentifier.empty());
    const auto& incomingID = contact.ID();
    const auto contactID = api_.Factory().Identifier(contactIdentifier);
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
            const auto proto = [&] {
                auto out = proto::Contact{};
                oldContact->Serialize(out);
                return out;
            }();

            if (false == api_.Storage().Store(proto)) {
                LogError()(OT_PRETTY_CLASS())(
                    ": Unable to create or save contact.")
                    .Flush();

                OT_FAIL;
            }
        } else {
            LogError()(OT_PRETTY_CLASS())("Duplicate nym found.").Flush();
            contact.RemoveNym(nymID);
            const auto proto = [&] {
                auto out = proto::Contact{};
                contact.Serialize(out);
                return out;
            }();

            if (false == api_.Storage().Store(proto)) {
                LogError()(OT_PRETTY_CLASS())(
                    ": Unable to create or save contact.")
                    .Flush();

                OT_FAIL;
            }
        }
    }

    auto blockchain = blockchain_.lock();

    if (blockchain) {
        blockchain->Internal().ProcessContact(contact);
    } else {
        LogVerbose()(OT_PRETTY_CLASS())(
            ": Warning: contact not updated in blockchain API")
            .Flush();
    }
}

auto Contacts::verify_write_lock(const rLock& lock) const -> bool
{
    if (lock.mutex() != &lock_) {
        LogError()(OT_PRETTY_CLASS())("Incorrect mutex.").Flush();

        return false;
    }

    if (false == lock.owns_lock()) {
        LogError()(OT_PRETTY_CLASS())("Lock not owned.").Flush();

        return false;
    }

    return true;
}

Contacts::~Contacts()
{
    timer_.Cancel();
    pipeline_.Close();
}
}  // namespace opentxs::api::session::imp
