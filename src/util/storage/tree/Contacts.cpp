// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/tree/Contacts.hpp"  // IWYU pragma: associated

#include <Contact.pb.h>
#include <ContactData.pb.h>
#include <ContactItem.pb.h>
#include <ContactSection.pb.h>
#include <StorageContactNymIndex.pb.h>
#include <StorageContacts.pb.h>
#include <StorageIDList.pb.h>
#include <StorageItemHash.pb.h>
#include <cstdlib>
#include <tuple>

#include "internal/identity/wot/claim/Types.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/Contact.hpp"
#include "internal/serialization/protobuf/verify/StorageContacts.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/identity/wot/claim/ClaimType.hpp"    // IWYU pragma: keep
#include "opentxs/identity/wot/claim/SectionType.hpp"  // IWYU pragma: keep
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/storage/Driver.hpp"
#include "util/storage/Plugin.hpp"
#include "util/storage/tree/Node.hpp"

namespace opentxs::storage
{
Contacts::Contacts(
    const api::Crypto& crypto,
    const api::session::Factory& factory,
    const Driver& storage,
    const UnallocatedCString& hash)
    : Node(crypto, factory, storage, hash)
    , merge_()
    , merged_()
    , nym_contact_index_()
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        blank(CurrentVersion);
    }
}

auto Contacts::Alias(const UnallocatedCString& id) const -> UnallocatedCString
{
    return get_alias(id);
}

auto Contacts::Delete(const UnallocatedCString& id) -> bool
{
    return delete_item(id);
}

void Contacts::extract_nyms(const Lock& lock, const proto::Contact& data) const
{
    if (false == verify_write_lock(lock)) {
        LogError()(OT_PRETTY_CLASS())("Lock failure.").Flush();

        abort();
    }

    const auto contact = factory_.IdentifierFromBase58(data.id());

    for (const auto& section : data.contactdata().section()) {
        if (section.name() !=
            translate(identity::wot::claim::SectionType::Relationship)) {
            break;
        }

        for (const auto& item : section.item()) {
            if (translate(item.type()) !=
                identity::wot::claim::ClaimType::Contact) {
                break;
            }

            const auto& nymID = item.value();
            nym_contact_index_[factory_.NymIDFromBase58(nymID)] = contact;
        }
    }
}

void Contacts::init(const UnallocatedCString& hash)
{
    std::shared_ptr<proto::StorageContacts> serialized{nullptr};
    driver_.LoadProto(hash, serialized);

    if (false == bool(serialized)) {
        LogError()(OT_PRETTY_CLASS())("Failed to load contact index file.")
            .Flush();

        abort();
    }

    init_version(CurrentVersion, *serialized);

    for (const auto& parent : serialized->merge()) {
        auto& list = merge_[parent.id()];

        for (const auto& id : parent.list()) { list.emplace(id); }
    }

    reverse_merged();

    for (const auto& it : serialized->contact()) {
        item_map_.emplace(
            it.itemid(), Metadata{it.hash(), it.alias(), 0, false});
    }

    // NOTE the address field is no longer used

    for (const auto& index : serialized->nym()) {
        const auto contact = factory_.IdentifierFromBase58(index.contact());

        for (const auto& nym : index.nym()) {
            nym_contact_index_[factory_.NymIDFromBase58(nym)] = contact;
        }
    }
}

auto Contacts::List() const -> ObjectList
{
    auto list = ot_super::List();

    for (const auto& it : merged_) {
        const auto& child = it.first;
        list.remove_if([&](const auto& i) { return i.first == child; });
    }

    return list;
}

auto Contacts::Load(
    const UnallocatedCString& id,
    std::shared_ptr<proto::Contact>& output,
    UnallocatedCString& alias,
    const bool checking) const -> bool
{
    const auto& normalized = nomalize_id(id);

    return load_proto<proto::Contact>(normalized, output, alias, checking);
}

auto Contacts::nomalize_id(const UnallocatedCString& input) const
    -> const UnallocatedCString&
{
    Lock lock(write_lock_);

    const auto it = merged_.find(input);

    if (merged_.end() == it) { return input; }

    return it->second;
}

auto Contacts::NymOwner(const identifier::Nym& nym) const -> identifier::Generic
{
    Lock lock(write_lock_);

    const auto it = nym_contact_index_.find(nym);

    if (nym_contact_index_.end() == it) { return {}; }

    return it->second;
}

void Contacts::reconcile_maps(const Lock& lock, const proto::Contact& data)
{
    if (false == verify_write_lock(lock)) {
        LogError()(OT_PRETTY_CLASS())("Lock failure.").Flush();

        abort();
    }

    const auto& contactID = data.id();

    for (const auto& merged : data.merged()) {
        auto& list = merge_[contactID];
        list.emplace(merged);
        merged_[merged].assign(contactID);
    }

    const auto& newParent = data.mergedto();

    if (false == newParent.empty()) {
        UnallocatedCString oldParent{};
        const auto it = merged_.find(contactID);

        if (merged_.end() != it) { oldParent = it->second; }

        if (false == oldParent.empty()) { merge_[oldParent].erase(contactID); }

        merge_[newParent].emplace(contactID);
    }

    reverse_merged();
}

void Contacts::reverse_merged()
{
    for (const auto& parent : merge_) {
        const auto& parentID = parent.first;
        const auto& list = parent.second;

        for (const auto& childID : list) { merged_[childID].assign(parentID); }
    }
}

auto Contacts::save(const Lock& lock) const -> bool
{
    if (false == verify_write_lock(lock)) {
        LogError()(OT_PRETTY_CLASS())("Lock failure.").Flush();

        abort();
    }

    auto serialized = serialize();

    if (false == proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

auto Contacts::Save() const -> bool
{
    Lock lock(write_lock_);

    return save(lock);
}

auto Contacts::serialize() const -> proto::StorageContacts
{
    proto::StorageContacts serialized;
    serialized.set_version(version_);

    for (const auto& parent : merge_) {
        const auto& parentID = parent.first;
        const auto& list = parent.second;
        auto& item = *serialized.add_merge();
        item.set_version(MergeIndexVersion);
        item.set_id(parentID);

        for (const auto& child : list) { item.add_list(child); }
    }

    for (const auto& item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = check_hash(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(
                version_, item.first, item.second, *serialized.add_contact());
        }
    }

    auto nyms = Map<UnallocatedCString, UnallocatedSet<UnallocatedCString>>{};

    for (const auto& it : nym_contact_index_) {
        const auto& nym = it.first;
        const auto& contact = it.second;
        auto& list = nyms[contact.asBase58(crypto_)];
        list.insert(nym.asBase58(crypto_));
    }

    for (const auto& it : nyms) {
        const auto& contact = it.first;
        const auto& nymList = it.second;
        auto& index = *serialized.add_nym();
        index.set_version(NymIndexVersion);
        index.set_contact(contact);

        for (const auto& nym : nymList) { index.add_nym(nym); }
    }

    return serialized;
}

auto Contacts::SetAlias(
    const UnallocatedCString& id,
    const UnallocatedCString& alias) -> bool
{
    const auto& normalized = nomalize_id(id);

    return set_alias(normalized, alias);
}

auto Contacts::Store(
    const proto::Contact& data,
    const UnallocatedCString& alias) -> bool
{
    if (false == proto::Validate(data, VERBOSE)) { return false; }

    Lock lock(write_lock_);

    const UnallocatedCString id = data.id();
    const auto incomingRevision = data.revision();
    const bool existingKey = (item_map_.end() != item_map_.find(id));
    auto& metadata = item_map_[id];
    auto& hash = std::get<0>(metadata);

    if (existingKey) {
        const bool revisionCheck = check_revision<proto::Contact>(
            (OT_PRETTY_CLASS()), incomingRevision, metadata);

        if (false == revisionCheck) {
            // We're trying to save a contact with a lower revision than has
            // already been saved. Just silently skip this update instead.

            return true;
        }
    }

    if (false == driver_.StoreProto(data, hash)) { return false; }

    if (false == alias.empty()) {
        std::get<1>(metadata) = alias;
    } else {
        if (false == data.label().empty()) {
            std::get<1>(metadata) = data.label();
        }
    }

    reconcile_maps(lock, data);
    extract_nyms(lock, data);

    return save(lock);
}
}  // namespace opentxs::storage
