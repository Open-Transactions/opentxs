// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"               // IWYU pragma: associated
#include "1_Internal.hpp"             // IWYU pragma: associated
#include "storage/tree/Contacts.hpp"  // IWYU pragma: associated

#include <cstdlib>
#include <list>
#include <memory_resource>
#include <set>
#include <tuple>

#include "Proto.hpp"
#include "internal/contact/Contact.hpp"
#include "internal/protobuf/Check.hpp"
#include "internal/protobuf/verify/Contact.hpp"
#include "internal/protobuf/verify/StorageContacts.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/contact/ClaimType.hpp"
#include "opentxs/contact/SectionType.hpp"
#include "opentxs/storage/Driver.hpp"
#include "opentxs/util/Log.hpp"
#include "serialization/protobuf/Contact.pb.h"
#include "serialization/protobuf/ContactData.pb.h"
#include "serialization/protobuf/ContactItem.pb.h"
#include "serialization/protobuf/ContactSection.pb.h"
#include "serialization/protobuf/StorageContactNymIndex.pb.h"
#include "serialization/protobuf/StorageContacts.pb.h"
#include "serialization/protobuf/StorageIDList.pb.h"
#include "serialization/protobuf/StorageItemHash.pb.h"
#include "storage/Plugin.hpp"
#include "storage/tree/Node.hpp"

namespace opentxs::storage
{
Contacts::Contacts(const Driver& storage, const std::string& hash)
    : Node(storage, hash)
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

auto Contacts::Alias(const std::string& id) const -> std::string
{
    return get_alias(id);
}

auto Contacts::Delete(const std::string& id) -> bool { return delete_item(id); }

void Contacts::extract_nyms(const Lock& lock, const proto::Contact& data) const
{
    if (false == verify_write_lock(lock)) {
        LogError()(OT_PRETTY_CLASS())("Lock failure.").Flush();

        abort();
    }

    const auto& contact = data.id();

    for (const auto& section : data.contactdata().section()) {
        if (section.name() != translate(contact::SectionType::Relationship)) {
            break;
        }

        for (const auto& item : section.item()) {
            if (translate(item.type()) != contact::ClaimType::Contact) {
                break;
            }

            const auto& nymID = item.value();
            nym_contact_index_[nymID] = contact;
        }
    }
}

void Contacts::init(const std::string& hash)
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
        const auto& contact = index.contact();

        for (const auto& nym : index.nym()) {
            nym_contact_index_[nym] = contact;
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
    const std::string& id,
    std::shared_ptr<proto::Contact>& output,
    std::string& alias,
    const bool checking) const -> bool
{
    const auto& normalized = nomalize_id(id);

    return load_proto<proto::Contact>(normalized, output, alias, checking);
}

auto Contacts::nomalize_id(const std::string& input) const -> const std::string&
{
    Lock lock(write_lock_);

    const auto it = merged_.find(input);

    if (merged_.end() == it) { return input; }

    return it->second;
}

auto Contacts::NymOwner(std::string nym) const -> std::string
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
        std::string oldParent{};
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

    std::pmr::map<std::string, std::pmr::set<std::string>> nyms;

    for (const auto& it : nym_contact_index_) {
        const auto& nym = it.first;
        const auto& contact = it.second;
        auto& list = nyms[contact];
        list.insert(nym);
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

auto Contacts::SetAlias(const std::string& id, const std::string& alias) -> bool
{
    const auto& normalized = nomalize_id(id);

    return set_alias(normalized, alias);
}

auto Contacts::Store(const proto::Contact& data, const std::string& alias)
    -> bool
{
    if (false == proto::Validate(data, VERBOSE)) { return false; }

    Lock lock(write_lock_);

    const std::string id = data.id();
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
