// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Contacts.hpp"

#include "storage/Plugin.hpp"

#include <set>

#define CURRENT_VERSION 2
#define NYM_INDEX_VERSION 1
#define MERGE_VERSION 1

//#define OT_METHOD "opentxs::storage::Contacts::"

namespace opentxs::storage
{
Contacts::Contacts(
    const opentxs::api::storage::Driver& storage,
    const std::string& hash)
    : Node(storage, hash)
    , address_index_()
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        version_ = CURRENT_VERSION;
        root_ = Node::BLANK_HASH;
    }
}

std::string Contacts::AddressOwner(
    proto::ContactItemType chain,
    std::string address) const
{
    Lock lock(write_lock_);

    const auto it = address_index_.find({chain, address});

    if (address_index_.end() == it) { return {}; }

    return it->second;
}

std::string Contacts::Alias(const std::string& id) const
{
    return get_alias(id);
}

bool Contacts::Delete(const std::string& id) { return delete_item(id); }

void Contacts::extract_addresses(const Lock& lock, const proto::Contact& data)
    const
{
    const auto& contact = data.id();
    const auto& version = data.version();

    if (false == verify_write_lock(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Lock failure.").Flush();

        abort();
    }

    if (false == data.has_contactdata()) { return; }

    for (const auto& section : data.contactdata().section()) {
        if (section.name() != proto::CONTACTSECTION_ADDRESS) { break; }

        for (const auto& item : section.item()) {
            const auto& type = item.type();
            const bool validChain = proto::ValidContactItemType(
                {version, proto::CONTACTSECTION_CONTRACT}, type);

            if (false == validChain) { break; }

            const auto& address = item.value();
            address_index_[{type, address}] = contact;
        }
    }
}

void Contacts::extract_nyms(const Lock& lock, const proto::Contact& data) const
{
    if (false == verify_write_lock(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Lock failure.").Flush();

        abort();
    }

    const auto& contact = data.id();

    for (const auto& section : data.contactdata().section()) {
        if (section.name() != proto::CONTACTSECTION_RELATIONSHIP) { break; }

        for (const auto& item : section.item()) {
            if (item.type() != proto::CITEMTYPE_CONTACT) { break; }

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
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to load contact index file.")
            .Flush();

        abort();
    }

    original_version_ = serialized->version();

    // Upgrade version
    if (CURRENT_VERSION > original_version_) {
        version_ = CURRENT_VERSION;
        LogOutput(OT_METHOD)(__FUNCTION__)(": Upgrading from version ")(
            original_version_)(" to ")(version_)(".")
            .Flush();
    } else {
        version_ = original_version_;
    }

    for (const auto& parent : serialized->merge()) {
        auto& list = merge_[parent.id()];

        for (const auto& id : parent.list()) { list.emplace(id); }
    }

    reverse_merged();

    for (const auto& it : serialized->contact()) {
        item_map_.emplace(
            it.itemid(), Metadata{it.hash(), it.alias(), 0, false});
    }

    for (const auto& index : serialized->address()) {
        const auto& type = index.chain();
        const auto& contact = index.contact();

        for (const auto& address : index.address()) {
            address_index_[{type, address}] = contact;
        }
    }

    for (const auto& index : serialized->nym()) {
        const auto& contact = index.contact();

        for (const auto& nym : index.nym()) {
            nym_contact_index_[nym] = contact;
        }
    }
}

ObjectList Contacts::List() const
{
    auto list = ot_super::List();

    for (const auto& it : merged_) {
        const auto& child = it.first;
        list.remove_if([&](const auto& i) { return i.first == child; });
    }

    return list;
}

bool Contacts::Load(
    const std::string& id,
    std::shared_ptr<proto::Contact>& output,
    std::string& alias,
    const bool checking) const
{
    const auto& normalized = nomalize_id(id);
    const bool loaded =
        load_proto<proto::Contact>(normalized, output, alias, checking);

    if (loaded) {
        OT_ASSERT(output);

        Lock lock(write_lock_);
        extract_addresses(lock, *output);
        extract_nyms(lock, *output);
    }

    return loaded;
}

const std::string& Contacts::nomalize_id(const std::string& input) const
{
    Lock lock(write_lock_);

    const auto it = merged_.find(input);

    if (merged_.end() == it) { return input; }

    return it->second;
}

std::string Contacts::NymOwner(std::string nym) const
{
    Lock lock(write_lock_);

    const auto it = nym_contact_index_.find(nym);

    if (nym_contact_index_.end() == it) { return {}; }

    return it->second;
}

void Contacts::reconcile_maps(const Lock& lock, const proto::Contact& data)
{
    if (false == verify_write_lock(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Lock failure.").Flush();

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

bool Contacts::save(const Lock& lock) const
{
    if (false == verify_write_lock(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Lock failure.").Flush();

        abort();
    }

    auto serialized = serialize();

    if (false == proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

bool Contacts::Save() const
{
    Lock lock(write_lock_);

    return save(lock);
}

proto::StorageContacts Contacts::serialize() const
{
    proto::StorageContacts serialized;
    serialized.set_version(version_);

    for (const auto& parent : merge_) {
        const auto& parentID = parent.first;
        const auto& list = parent.second;
        auto& item = *serialized.add_merge();
        item.set_version(MERGE_VERSION);
        item.set_id(parentID);

        for (const auto& child : list) { item.add_list(child); }
    }

    for (const auto item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = check_hash(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(item.first, item.second, *serialized.add_contact());
        }
    }

    std::map<
        std::pair<std::string, proto::ContactItemType>,
        std::set<std::string>>
        addresses;

    for (const auto& it : address_index_) {
        const auto& type = it.first.first;
        const auto& address = it.first.second;
        const auto& contact = it.second;
        auto& list = addresses[{contact, type}];
        list.insert(address);
    }

    for (const auto& it : addresses) {
        const auto& contact = it.first.first;
        const auto& type = it.first.second;
        const auto& addressList = it.second;
        auto& index = *serialized.add_address();
        index.set_version(CURRENT_VERSION);
        index.set_contact(contact);
        index.set_chain(type);

        for (const auto& address : addressList) { index.add_address(address); }
    }

    std::map<std::string, std::set<std::string>> nyms;

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
        index.set_version(NYM_INDEX_VERSION);
        index.set_contact(contact);

        for (const auto& nym : nymList) { index.add_nym(nym); }
    }

    return serialized;
}

bool Contacts::SetAlias(const std::string& id, const std::string& alias)
{
    const auto& normalized = nomalize_id(id);

    return set_alias(normalized, alias);
}

bool Contacts::Store(const proto::Contact& data, const std::string& alias)
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
            OT_METHOD, incomingRevision, metadata);

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
    extract_addresses(lock, data);
    extract_nyms(lock, data);

    return save(lock);
}
}  // namespace opentxs::storage
