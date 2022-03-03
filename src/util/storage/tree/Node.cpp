// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                // IWYU pragma: associated
#include "1_Internal.hpp"              // IWYU pragma: associated
#include "util/storage/tree/Node.hpp"  // IWYU pragma: associated

#include "opentxs/util/Log.hpp"
#include "opentxs/util/storage/Driver.hpp"
#include "serialization/protobuf/Contact.pb.h"
#include "serialization/protobuf/Nym.pb.h"
#include "serialization/protobuf/Seed.pb.h"
#include "serialization/protobuf/StorageEnums.pb.h"
#include "serialization/protobuf/StorageItemHash.pb.h"

namespace opentxs::storage
{
const UnallocatedCString Node::BLANK_HASH = "blankblankblankblankblank";

Node::Node(const Driver& storage, const UnallocatedCString& key)
    : driver_(storage)
    , version_(0)
    , original_version_(0)
    , root_(key)
    , write_lock_()
    , item_map_()
{
}

void Node::blank(const VersionNumber version)
{
    version_ = version;
    original_version_ = version_;
    root_ = BLANK_HASH;
}

auto Node::check_hash(const UnallocatedCString& hash) const -> bool
{
    const bool empty = hash.empty();
    const bool blank = (Node::BLANK_HASH == hash);

    return !(empty || blank);
}

auto Node::delete_item(const UnallocatedCString& id) -> bool
{
    auto lock = Lock{write_lock_};

    return delete_item(lock, id);
}

auto Node::delete_item(const Lock& lock, const UnallocatedCString& id) -> bool
{
    OT_ASSERT(verify_write_lock(lock))

    const auto items = item_map_.erase(id);

    if (0 == items) { return false; }

    return save(lock);
}

auto Node::extract_revision(const proto::Contact& input) const -> std::uint64_t
{
    return input.revision();
}

auto Node::extract_revision(const proto::Nym& input) const -> std::uint64_t
{
    return input.revision();
}

auto Node::extract_revision(const proto::Seed& input) const -> std::uint64_t
{
    return input.index();
}

auto Node::get_alias(const UnallocatedCString& id) const -> UnallocatedCString
{
    UnallocatedCString output;
    std::lock_guard<std::mutex> lock(write_lock_);
    const auto& it = item_map_.find(id);

    if (item_map_.end() != it) { output = std::get<1>(it->second); }

    return output;
}

auto Node::List() const -> ObjectList
{
    ObjectList output;
    auto lock = Lock{write_lock_};

    for (const auto& it : item_map_) {
        output.push_back({it.first, std::get<1>(it.second)});
    }

    lock.unlock();

    return output;
}

auto Node::load_raw(
    const UnallocatedCString& id,
    UnallocatedCString& output,
    UnallocatedCString& alias,
    const bool checking) const -> bool
{
    std::lock_guard<std::mutex> lock(write_lock_);
    const auto& it = item_map_.find(id);
    const bool exists = (item_map_.end() != it);

    if (!exists) {
        if (!checking) {
            LogError()(OT_PRETTY_CLASS())("Error: item with id ")(
                id)(" does not exist.")
                .Flush();
        }

        return false;
    }

    alias = std::get<1>(it->second);

    return driver_.Load(std::get<0>(it->second), checking, output);
}

auto Node::migrate(const UnallocatedCString& hash, const Driver& to) const
    -> bool
{
    if (false == check_hash(hash)) { return true; }

    return driver_.Migrate(hash, to);
}

auto Node::Migrate(const Driver& to) const -> bool
{
    if (UnallocatedCString(BLANK_HASH) == root_) {
        if (0 < item_map_.size()) {
            LogError()(OT_PRETTY_CLASS())(
                "Items present in object with blank root hash.")
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

auto Node::normalize_hash(const UnallocatedCString& hash) -> UnallocatedCString
{
    if (hash.empty()) { return BLANK_HASH; }

    if (20 > hash.size()) {
        LogError()(OT_PRETTY_STATIC(Node))("Blanked out short hash ")(hash)(".")
            .Flush();

        return BLANK_HASH;
    }

    if (116 < hash.size()) {
        LogError()(OT_PRETTY_STATIC(Node))("Blanked out long hash ")(hash)(".")
            .Flush();

        return BLANK_HASH;
    }

    return hash;
}

auto Node::Root() const -> UnallocatedCString
{
    Lock lock_(write_lock_);

    return root_;
}

void Node::serialize_index(
    const VersionNumber version,
    const UnallocatedCString& id,
    const Metadata& metadata,
    proto::StorageItemHash& output,
    const proto::StorageHashType type) const
{
    set_hash(version, id, std::get<0>(metadata), output, type);
    output.set_alias(std::get<1>(metadata));
}

auto Node::set_alias(
    const UnallocatedCString& id,
    const UnallocatedCString& value) -> bool
{
    auto lock = Lock{write_lock_};

    if (auto it = item_map_.find(id); item_map_.end() != it) {
        auto& [hash, alias, revision, b] = it->second;
        auto old = alias;
        alias = value;

        if (false == save(lock)) {
            alias.swap(old);
            LogError()(OT_PRETTY_CLASS())("Failed to save node").Flush();

            return false;
        }

        return true;
    }

    LogError()(OT_PRETTY_CLASS())("item ")(id)(" does not exist").Flush();

    return false;
}

void Node::set_hash(
    const VersionNumber version,
    const UnallocatedCString& id,
    const UnallocatedCString& hash,
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

auto Node::store_raw(
    const UnallocatedCString& data,
    const UnallocatedCString& id,
    const UnallocatedCString& alias) -> bool
{
    auto lock = Lock{write_lock_};

    return store_raw(lock, data, id, alias);
}

auto Node::store_raw(
    const Lock& lock,
    const UnallocatedCString& data,
    const UnallocatedCString& id,
    const UnallocatedCString& alias) -> bool
{
    OT_ASSERT(verify_write_lock(lock))

    auto& metadata = item_map_[id];
    auto& hash = std::get<0>(metadata);

    if (!driver_.Store(true, data, hash)) { return false; }

    if (!alias.empty()) { std::get<1>(metadata) = alias; }

    return save(lock);
}

auto Node::UpgradeLevel() const -> VersionNumber { return original_version_; }

auto Node::verify_write_lock(const Lock& lock) const -> bool
{
    if (lock.mutex() != &write_lock_) {
        LogError()(OT_PRETTY_CLASS())("Incorrect mutex.").Flush();

        return false;
    }

    if (false == lock.owns_lock()) {
        LogError()(OT_PRETTY_CLASS())("Lock not owned.").Flush();

        return false;
    }

    return true;
}
}  // namespace opentxs::storage
