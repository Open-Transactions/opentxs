// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/tree/Seeds.hpp"  // IWYU pragma: associated

#include <Seed.pb.h>
#include <StorageItemHash.pb.h>
#include <StorageSeeds.pb.h>
#include <cstdlib>
#include <iostream>
#include <tuple>
#include <utility>

#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/Seed.hpp"
#include "internal/serialization/protobuf/verify/StorageSeeds.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/storage/Driver.hpp"
#include "util/storage/Plugin.hpp"
#include "util/storage/tree/Node.hpp"

namespace opentxs::storage
{
Seeds::Seeds(
    const api::Crypto& crypto,
    const api::session::Factory& factory,
    const Driver& storage,
    const UnallocatedCString& hash)
    : Node(crypto, factory, storage, hash)
    , default_seed_()
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        blank(current_version_);
    }
}

auto Seeds::Alias(const UnallocatedCString& id) const -> UnallocatedCString
{
    return get_alias(id);
}

auto Seeds::Default() const -> UnallocatedCString
{
    std::lock_guard<std::mutex> lock(write_lock_);

    return default_seed_;
}

auto Seeds::Delete(const UnallocatedCString& id) -> bool
{
    return delete_item(id);
}

auto Seeds::init(const UnallocatedCString& hash) -> void
{
    std::shared_ptr<proto::StorageSeeds> serialized;
    driver_.LoadProto(hash, serialized);

    if (!serialized) {
        std::cerr << __func__ << ": Failed to load seed index file."
                  << std::endl;
        abort();
    }

    init_version(current_version_, *serialized);
    default_seed_ = serialized->defaultseed();

    for (const auto& it : serialized->seed()) {
        item_map_.emplace(
            it.itemid(), Metadata{it.hash(), it.alias(), 0, false});
    }
}

auto Seeds::Load(
    const UnallocatedCString& id,
    std::shared_ptr<proto::Seed>& output,
    UnallocatedCString& alias,
    const bool checking) const -> bool
{
    return load_proto<proto::Seed>(id, output, alias, checking);
}

auto Seeds::save(const std::unique_lock<std::mutex>& lock) const -> bool
{
    if (!verify_write_lock(lock)) {
        std::cerr << __func__ << ": Lock failure." << std::endl;
        abort();
    }

    auto serialized = serialize();

    if (!proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

auto Seeds::serialize() const -> proto::StorageSeeds
{
    proto::StorageSeeds serialized;
    serialized.set_version(version_);
    serialized.set_defaultseed(default_seed_);

    for (const auto& item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = check_hash(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(
                version_, item.first, item.second, *serialized.add_seed());
        }
    }

    return serialized;
}
auto Seeds::SetAlias(const UnallocatedCString& id, std::string_view alias)
    -> bool
{
    return set_alias(id, alias);
}

auto Seeds::set_default(
    const std::unique_lock<std::mutex>& lock,
    const UnallocatedCString& id) -> void
{
    if (!verify_write_lock(lock)) {
        std::cerr << __func__ << ": Lock failure." << std::endl;
        abort();
    }

    default_seed_ = id;
}

auto Seeds::SetDefault(const UnallocatedCString& id) -> bool
{
    auto lock = Lock{write_lock_};
    set_default(lock, id);

    return save(lock);
}

auto Seeds::Store(const proto::Seed& data) -> bool
{
    auto lock = Lock{write_lock_};

    const UnallocatedCString id = data.fingerprint();
    const auto incomingRevision = data.index();
    const bool existingKey = (item_map_.end() != item_map_.find(id));
    auto& metadata = item_map_[id];
    auto& hash = std::get<0>(metadata);

    if (existingKey) {
        const bool revisionCheck = check_revision<proto::Seed>(
            (OT_PRETTY_CLASS()), incomingRevision, metadata);

        if (false == revisionCheck) {
            // We're trying to save a seed with a lower index than has already
            // been saved. Just silently skip this update instead.

            return true;
        }
    }

    if (!driver_.StoreProto(data, hash)) { return false; }

    if (default_seed_.empty()) { set_default(lock, id); }

    return save(lock);
}
}  // namespace opentxs::storage
