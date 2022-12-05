// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/tree/Mailbox.hpp"  // IWYU pragma: associated

#include <StorageEnums.pb.h>
#include <StorageItemHash.pb.h>
#include <StorageNymList.pb.h>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <tuple>
#include <utility>

#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/StorageNymList.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/storage/Driver.hpp"
#include "util/storage/Plugin.hpp"
#include "util/storage/tree/Node.hpp"

namespace opentxs::storage
{
Mailbox::Mailbox(
    const api::Crypto& crypto,
    const api::session::Factory& factory,
    const Driver& storage,
    const UnallocatedCString& hash)
    : Node(crypto, factory, storage, hash)
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        blank(2);
    }
}

auto Mailbox::Delete(const UnallocatedCString& id) -> bool
{
    return delete_item(id);
}

void Mailbox::init(const UnallocatedCString& hash)
{
    std::shared_ptr<proto::StorageNymList> serialized;
    driver_.LoadProto(hash, serialized);

    if (!serialized) {
        std::cerr << __func__ << ": Failed to load mailbox index file."
                  << std::endl;
        abort();
    }

    init_version(2, *serialized);

    for (const auto& it : serialized->nym()) {
        item_map_.emplace(
            it.itemid(), Metadata{it.hash(), it.alias(), 0, false});
    }
}

auto Mailbox::Load(
    const UnallocatedCString& id,
    UnallocatedCString& output,
    UnallocatedCString& alias,
    const bool checking) const -> bool
{
    return load_raw(id, output, alias, checking);
}

auto Mailbox::save(const std::unique_lock<std::mutex>& lock) const -> bool
{
    if (!verify_write_lock(lock)) {
        std::cerr << __func__ << ": Lock failure." << std::endl;
        abort();
    }

    auto serialized = serialize();

    if (!proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

auto Mailbox::serialize() const -> proto::StorageNymList
{
    proto::StorageNymList serialized;
    serialized.set_version(version_);

    for (const auto& item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = check_hash(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(
                version_,
                item.first,
                item.second,
                *serialized.add_nym(),
                proto::STORAGEHASH_RAW);
        }
    }

    return serialized;
}

auto Mailbox::Store(
    const UnallocatedCString& id,
    const UnallocatedCString& data,
    const UnallocatedCString& alias) -> bool
{
    return store_raw(data, id, alias);
}
}  // namespace opentxs::storage
