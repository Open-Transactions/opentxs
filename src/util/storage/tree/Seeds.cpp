// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/tree/Seeds.hpp"  // IWYU pragma: associated

#include <Seed.pb.h>
#include <StorageSeeds.pb.h>
#include <atomic>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <tuple>
#include <utility>

#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/Seed.hpp"
#include "internal/serialization/protobuf/verify/StorageSeeds.hpp"
#include "internal/util/DeferredConstruction.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/storage/Types.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Container.hpp"
#include "util/storage/tree/Node.hpp"

namespace opentxs::storage::tree
{
using namespace std::literals;

Seeds::Seeds(
    const api::Crypto& crypto,
    const api::session::Factory& factory,
    const driver::Plugin& storage,
    const Hash& hash)
    : Node(crypto, factory, storage, hash, OT_PRETTY_CLASS(), current_version_)
    , default_seed_()
{
    if (is_valid(hash)) {
        init(hash);
    } else {
        blank();
    }
}

auto Seeds::Alias(const opentxs::crypto::SeedID& id) const -> UnallocatedCString
{
    return get_alias(id);
}

auto Seeds::Default() const -> opentxs::crypto::SeedID
{
    std::lock_guard<std::mutex> lock(write_lock_);

    return default_seed_;
}

auto Seeds::Delete(const opentxs::crypto::SeedID& id) -> bool
{
    return delete_item(id);
}

auto Seeds::init(const Hash& hash) noexcept(false) -> void
{
    auto p = std::shared_ptr<proto::StorageSeeds>{};

    if (LoadProto(hash, p, verbose) && p) {
        const auto& proto = *p;

        switch (set_original_version(proto.version())) {
            case 2u: {
                default_seed_ = factory_.SeedIDFromBase58(proto.defaultseed());
                [[fallthrough]];
            }
            case 1u:
            default: {
                init_map(proto.seed());
            }
        }
    } else {
        throw std::runtime_error{
            "failed to load root object file in "s.append(OT_PRETTY_CLASS())};
    }
}

auto Seeds::Load(
    const opentxs::crypto::SeedID& id,
    std::shared_ptr<proto::Seed>& output,
    UnallocatedCString& alias,
    ErrorReporting checking) const -> bool
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

    return StoreProto(serialized, root_);
}

auto Seeds::serialize() const -> proto::StorageSeeds
{
    proto::StorageSeeds serialized;
    serialized.set_version(version_);
    serialized.set_defaultseed(default_seed_.asBase58(crypto_));

    for (const auto& item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = is_valid(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(item.first, item.second, *serialized.add_seed());
        }
    }

    return serialized;
}
auto Seeds::SetAlias(const opentxs::crypto::SeedID& id, std::string_view alias)
    -> bool
{
    return set_alias(id, alias);
}

auto Seeds::set_default(
    const std::unique_lock<std::mutex>& lock,
    const opentxs::crypto::SeedID& id) -> void
{
    if (!verify_write_lock(lock)) {
        std::cerr << __func__ << ": Lock failure." << std::endl;
        abort();
    }

    default_seed_ = id;
}

auto Seeds::SetDefault(const opentxs::crypto::SeedID& id) -> bool
{
    auto lock = Lock{write_lock_};
    set_default(lock, id);

    return save(lock);
}

auto Seeds::Store(const opentxs::crypto::SeedID& id, const proto::Seed& data)
    -> bool
{
    auto lock = Lock{write_lock_};
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

    if (!StoreProto(data, hash)) { return false; }

    if (default_seed_.empty()) { set_default(lock, id); }

    return save(lock);
}

auto Seeds::upgrade(const Lock& lock) noexcept -> bool
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
