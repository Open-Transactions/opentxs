// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/tree/Contacts.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/Contact.pb.h>
#include <opentxs/protobuf/ContactData.pb.h>
#include <opentxs/protobuf/ContactItem.pb.h>
#include <opentxs/protobuf/ContactSection.pb.h>
#include <opentxs/protobuf/StorageContactNymIndex.pb.h>
#include <opentxs/protobuf/StorageContacts.pb.h>
#include <opentxs/protobuf/StorageIDList.pb.h>
#include <atomic>
#include <cstdlib>
#include <source_location>
#include <stdexcept>
#include <tuple>

#include "internal/util/DeferredConstruction.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/FixedByteArray.hpp"             // IWYU pragma: keep
#include "opentxs/identity/wot/claim/ClaimType.hpp"    // IWYU pragma: keep
#include "opentxs/identity/wot/claim/SectionType.hpp"  // IWYU pragma: keep
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/identity/wot/claim/Types.internal.hpp"
#include "opentxs/protobuf/Types.internal.hpp"
#include "opentxs/protobuf/syntax/Contact.hpp"
#include "opentxs/protobuf/syntax/StorageContacts.hpp"
#include "opentxs/protobuf/syntax/Types.internal.tpp"
#include "opentxs/storage/Types.internal.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "util/storage/tree/Node.hpp"

namespace opentxs::storage::tree
{
using namespace std::literals;

Contacts::Contacts(
    const api::Crypto& crypto,
    const api::session::Factory& factory,
    const driver::Plugin& storage,
    const Hash& hash)
    : Node(
          crypto,
          factory,
          storage,
          hash,
          std::source_location::current().function_name(),
          CurrentVersion)
    , merge_()
    , merged_()
    , nym_contact_index_()
{
    if (is_valid(hash)) {
        init(hash);
    } else {
        blank();
    }
}

auto Contacts::Alias(const identifier::Generic& id) const -> UnallocatedCString
{
    return get_alias(id);
}

auto Contacts::Delete(const identifier::Generic& id) -> bool
{
    return delete_item(id);
}

void Contacts::extract_nyms(const Lock& lock, const protobuf::Contact& data)
    const
{
    if (false == verify_write_lock(lock)) {
        LogError()()("Lock failure.").Flush();

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

auto Contacts::init(const Hash& hash) noexcept(false) -> void
{
    auto p = std::shared_ptr<protobuf::StorageContacts>{};

    if (LoadProto(hash, p, verbose) && p) {
        const auto& proto = *p;

        switch (set_original_version(proto.version())) {
            case 2u:
            case 1u:
            default: {
                init_map(proto.contact());

                for (const auto& parent : proto.merge()) {
                    auto& list =
                        merge_[factory_.IdentifierFromBase58(parent.id())];

                    for (const auto& id : parent.list()) {
                        list.emplace(factory_.IdentifierFromBase58(id));
                    }
                }

                reverse_merged();

                // NOTE the address field is no longer used

                for (const auto& index : proto.nym()) {
                    const auto contact =
                        factory_.IdentifierFromBase58(index.contact());

                    for (const auto& nym : index.nym()) {
                        nym_contact_index_[factory_.NymIDFromBase58(nym)] =
                            contact;
                    }
                }
            }
        }
    } else {
        throw std::runtime_error{"failed to load root object file in "s.append(
            std::source_location::current().function_name())};
    }
}

auto Contacts::List() const -> ObjectList
{
    auto list = ot_super::List();

    for (const auto& it : merged_) {
        const auto& child = it.first;
        list.remove_if(
            [&](const auto& i) { return i.first == child.asBase58(crypto_); });
    }

    return list;
}

auto Contacts::Load(
    const identifier::Generic& id,
    std::shared_ptr<protobuf::Contact>& output,
    UnallocatedCString& alias,
    ErrorReporting checking) const -> bool
{
    const auto& normalized = nomalize_id(id);

    return load_proto<protobuf::Contact>(normalized, output, alias, checking);
}

auto Contacts::nomalize_id(const identifier::Generic& input) const
    -> const identifier::Generic&
{
    const auto lock = Lock{write_lock_};

    const auto it = merged_.find(input);

    if (merged_.end() == it) { return input; }

    return it->second;
}

auto Contacts::NymOwner(const identifier::Nym& nym) const -> identifier::Generic
{
    const auto lock = Lock{write_lock_};

    const auto it = nym_contact_index_.find(nym);

    if (nym_contact_index_.end() == it) { return {}; }

    return it->second;
}

void Contacts::reconcile_maps(const Lock& lock, const protobuf::Contact& data)
{
    if (false == verify_write_lock(lock)) {
        LogError()()("Lock failure.").Flush();

        abort();
    }

    const auto contactID = factory_.IdentifierFromBase58(data.id());

    for (const auto& merged : data.merged()) {
        const auto id = factory_.IdentifierFromBase58(merged);
        auto& list = merge_[contactID];
        list.emplace(id);
        merged_[id] = contactID;
    }

    const auto& newParent = data.mergedto();

    if (false == newParent.empty()) {
        auto oldParent = identifier::Generic{};
        const auto it = merged_.find(contactID);

        if (merged_.end() != it) { oldParent = it->second; }

        if (false == oldParent.empty()) { merge_[oldParent].erase(contactID); }

        merge_[factory_.IdentifierFromBase58(newParent)].emplace(contactID);
    }

    reverse_merged();
}

void Contacts::reverse_merged()
{
    for (const auto& parent : merge_) {
        const auto& parentID = parent.first;
        const auto& list = parent.second;

        for (const auto& childID : list) { merged_[childID] = parentID; }
    }
}

auto Contacts::save(const Lock& lock) const -> bool
{
    if (false == verify_write_lock(lock)) {
        LogError()()("Lock failure.").Flush();

        abort();
    }

    auto serialized = serialize();

    if (false == protobuf::syntax::check(LogError(), serialized)) {
        return false;
    }

    return StoreProto(serialized, root_);
}

auto Contacts::Save() const -> bool
{
    const auto lock = Lock{write_lock_};

    return save(lock);
}

auto Contacts::serialize() const -> protobuf::StorageContacts
{
    protobuf::StorageContacts serialized;
    serialized.set_version(version_);

    for (const auto& parent : merge_) {
        const auto& parentID = parent.first;
        const auto& list = parent.second;
        auto& item = *serialized.add_merge();
        item.set_version(MergeIndexVersion);
        item.set_id(parentID.asBase58(crypto_));

        for (const auto& child : list) {
            item.add_list(child.asBase58(crypto_));
        }
    }

    for (const auto& item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = is_valid(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(item.first, item.second, *serialized.add_contact());
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

auto Contacts::SetAlias(const identifier::Generic& id, std::string_view alias)
    -> bool
{
    const auto& normalized = nomalize_id(id);

    return set_alias(normalized, alias);
}

auto Contacts::Store(const protobuf::Contact& data, std::string_view alias)
    -> bool
{
    if (false == protobuf::syntax::check(LogError(), data)) { return false; }

    const auto lock = Lock{write_lock_};

    const auto id = factory_.IdentifierFromBase58(data.id());
    const auto incomingRevision = data.revision();
    const bool existingKey = (item_map_.end() != item_map_.find(id));
    auto& metadata = item_map_[id];
    auto& hash = std::get<0>(metadata);

    if (existingKey) {
        const bool revisionCheck =
            check_revision<protobuf::Contact>(incomingRevision, metadata);

        if (false == revisionCheck) {
            // We're trying to save a contact with a lower revision than has
            // already been saved. Just silently skip this update instead.

            return true;
        }
    }

    if (false == StoreProto(data, hash)) { return false; }

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

auto Contacts::upgrade(const Lock& lock) noexcept -> bool
{
    auto changed = Node::upgrade(lock);

    switch (original_version_.get()) {
        case 1u:
        case 2u:
        default: {
        }
    }

    return changed;
}
}  // namespace opentxs::storage::tree
