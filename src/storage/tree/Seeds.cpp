// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Seeds.hpp"

#include "storage/Plugin.hpp"

#define CURRENT_VERSION 2

//#define OT_METHOD "opentxs::storage::Seeds::"

namespace opentxs
{
namespace storage
{
Seeds::Seeds(
    const opentxs::api::storage::Driver& storage,
    const std::string& hash)
    : Node(storage, hash)
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        version_ = CURRENT_VERSION;
        root_ = Node::BLANK_HASH;
    }
}

std::string Seeds::Alias(const std::string& id) const { return get_alias(id); }

std::string Seeds::Default() const
{
    std::lock_guard<std::mutex> lock(write_lock_);

    return default_seed_;
}

bool Seeds::Delete(const std::string& id) { return delete_item(id); }

void Seeds::init(const std::string& hash)
{
    std::shared_ptr<proto::StorageSeeds> serialized;
    driver_.LoadProto(hash, serialized);

    if (!serialized) {
        std::cerr << __FUNCTION__ << ": Failed to load seed index file."
                  << std::endl;
        abort();
    }

    version_ = serialized->version();

    // Upgrade version
    if (CURRENT_VERSION > version_) { version_ = CURRENT_VERSION; }

    default_seed_ = serialized->defaultseed();

    for (const auto& it : serialized->seed()) {
        item_map_.emplace(
            it.itemid(), Metadata{it.hash(), it.alias(), 0, false});
    }
}

bool Seeds::Load(
    const std::string& id,
    std::shared_ptr<proto::Seed>& output,
    std::string& alias,
    const bool checking) const
{
    return load_proto<proto::Seed>(id, output, alias, checking);
}

bool Seeds::save(const std::unique_lock<std::mutex>& lock) const
{
    if (!verify_write_lock(lock)) {
        std::cerr << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    auto serialized = serialize();

    if (!proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

proto::StorageSeeds Seeds::serialize() const
{
    proto::StorageSeeds serialized;
    serialized.set_version(version_);
    serialized.set_defaultseed(default_seed_);

    for (const auto item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = check_hash(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(item.first, item.second, *serialized.add_seed());
        }
    }

    return serialized;
}
bool Seeds::SetAlias(const std::string& id, const std::string& alias)
{
    return set_alias(id, alias);
}

void Seeds::set_default(
    const std::unique_lock<std::mutex>& lock,
    const std::string& id)
{
    if (!verify_write_lock(lock)) {
        std::cerr << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    default_seed_ = id;
}

bool Seeds::SetDefault(const std::string& id)
{
    std::unique_lock<std::mutex> lock(write_lock_);

    set_default(lock, id);

    return save(lock);
}

bool Seeds::Store(const proto::Seed& data, const std::string& alias)
{
    std::unique_lock<std::mutex> lock(write_lock_);

    const std::string id = data.fingerprint();
    const auto incomingRevision = data.index();
    const bool existingKey = (item_map_.end() != item_map_.find(id));
    auto& metadata = item_map_[id];
    auto& hash = std::get<0>(metadata);

    if (existingKey) {
        const bool revisionCheck =
            check_revision<proto::Seed>(OT_METHOD, incomingRevision, metadata);

        if (false == revisionCheck) {
            // We're trying to save a seed with a lower index than has already
            // been saved. Just silently skip this update instead.

            return true;
        }
    }

    if (!driver_.StoreProto(data, hash)) { return false; }

    if (default_seed_.empty()) { set_default(lock, id); }

    if (!alias.empty()) { std::get<1>(metadata) = alias; }

    return save(lock);
}
}  // namespace storage
}  // namespace opentxs
