// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Node.hpp"

#include "opentxs/core/Log.hpp"

#include "storage/Plugin.hpp"

//#define OT_METHOD "opentxs::storage::Node::"

namespace opentxs::storage
{
const std::string Node::BLANK_HASH = "blankblankblankblankblank";

Node::Node(const opentxs::api::storage::Driver& storage, const std::string& key)
    : driver_(storage)
    , root_(key)
{
}

bool Node::check_hash(const std::string& hash) const
{
    const bool empty = hash.empty();
    const bool blank = (Node::BLANK_HASH == hash);

    return !(empty || blank);
}

bool Node::delete_item(const std::string& id)
{
    Lock lock(write_lock_);

    return delete_item(lock, id);
}

bool Node::delete_item(const Lock& lock, const std::string& id)
{
    OT_ASSERT(verify_write_lock(lock))

    const auto items = item_map_.erase(id);

    if (0 == items) { return false; }

    return save(lock);
}

std::uint64_t Node::extract_revision(const proto::Contact& input) const
{
    return input.revision();
}

std::uint64_t Node::extract_revision(const proto::CredentialIndex& input) const
{
    return input.revision();
}

std::uint64_t Node::extract_revision(const proto::Seed& input) const
{
    return input.index();
}

std::string Node::get_alias(const std::string& id) const
{
    std::string output;
    std::lock_guard<std::mutex> lock(write_lock_);
    const auto& it = item_map_.find(id);

    if (item_map_.end() != it) { output = std::get<1>(it->second); }

    return output;
}

ObjectList Node::List() const
{
    ObjectList output;
    Lock lock(write_lock_);

    for (const auto it : item_map_) {
        output.push_back({it.first, std::get<1>(it.second)});
    }

    lock.unlock();

    return output;
}

bool Node::load_raw(
    const std::string& id,
    std::string& output,
    std::string& alias,
    const bool checking) const
{
    std::lock_guard<std::mutex> lock(write_lock_);
    const auto& it = item_map_.find(id);
    const bool exists = (item_map_.end() != it);

    if (!exists) {
        if (!checking) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Error: item with id ")(id)(
                " does not exist.")
                .Flush();
        }

        return false;
    }

    alias = std::get<1>(it->second);

    return driver_.Load(std::get<0>(it->second), checking, output);
}

bool Node::migrate(
    const std::string& hash,
    const opentxs::api::storage::Driver& to) const
{
    if (false == check_hash(hash)) { return true; }

    return driver_.Migrate(hash, to);
}

bool Node::Migrate(const opentxs::api::storage::Driver& to) const
{
    if (std::string(BLANK_HASH) == root_) {
        if (0 < item_map_.size()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Items present in object with blank root hash.")
                .Flush();

            OT_FAIL;
        }

        return true;
    }

    bool output{true};
    output &= migrate(root_, to);

    for (const auto& item : item_map_) {
        const auto& hash = std::get<0>(item.second);
        output &= migrate(hash, to);
    }

    return output;
}

std::string Node::normalize_hash(const std::string& hash)
{
    if (hash.empty()) { return BLANK_HASH; }

    if (proto::MIN_PLAUSIBLE_IDENTIFIER > hash.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Blanked out short hash ")(hash)(
            ".")
            .Flush();

        return BLANK_HASH;
    }

    if (proto::MAX_PLAUSIBLE_IDENTIFIER < hash.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Blanked out long hash ")(hash)(
            ".")
            .Flush();

        return BLANK_HASH;
    }

    return hash;
}

std::string Node::Root() const
{
    std::lock_guard<std::mutex> lock_(write_lock_);

    return root_;
}

void Node::serialize_index(
    const std::string& id,
    const Metadata& metadata,
    proto::StorageItemHash& output,
    const proto::StorageHashType type) const
{
    set_hash(version_, id, std::get<0>(metadata), output, type);
    output.set_alias(std::get<1>(metadata));
}

bool Node::set_alias(const std::string& id, const std::string& alias)
{
    std::string output;
    Lock lock(write_lock_);

    const bool exists = (item_map_.end() != item_map_.find(id));

    if (!exists) { return false; }

    std::get<1>(item_map_[id]) = alias;

    return save(lock);
}

void Node::set_hash(
    const std::uint32_t version,
    const std::string& id,
    const std::string& hash,
    proto::StorageItemHash& output,
    const proto::StorageHashType type) const
{
    if (2 > version) {
        if (proto::STORAGEHASH_ERROR != type) {
            output.set_version(2);
        } else {
            output.set_version(version);
        }
    } else {
        output.set_version(version);
    }

    output.set_itemid(id);

    if (hash.empty()) {
        output.set_hash(Node::BLANK_HASH);
    } else {
        output.set_hash(hash);
    }

    output.set_type(type);
}

bool Node::store_raw(
    const std::string& data,
    const std::string& id,
    const std::string& alias)
{
    Lock lock(write_lock_);

    return store_raw(lock, data, id, alias);
}

bool Node::store_raw(
    const Lock& lock,
    const std::string& data,
    const std::string& id,
    const std::string& alias)
{
    OT_ASSERT(verify_write_lock(lock))

    auto& metadata = item_map_[id];
    auto& hash = std::get<0>(metadata);

    if (!driver_.Store(true, data, hash)) { return false; }

    if (!alias.empty()) { std::get<1>(metadata) = alias; }

    return save(lock);
}

std::uint32_t Node::UpgradeLevel() const { return original_version_; }

bool Node::verify_write_lock(const Lock& lock) const
{
    if (lock.mutex() != &write_lock_) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect mutex.").Flush();

        return false;
    }

    if (false == lock.owns_lock()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Lock not owned.").Flush();

        return false;
    }

    return true;
}
}  // namespace opentxs::storage
