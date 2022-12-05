// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/tree/Units.hpp"  // IWYU pragma: associated

#include <StorageItemHash.pb.h>
#include <StorageUnits.pb.h>
#include <UnitDefinition.pb.h>
#include <cstdlib>
#include <iostream>
#include <tuple>
#include <utility>

#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/StorageUnits.hpp"
#include "internal/serialization/protobuf/verify/UnitDefinition.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/storage/Driver.hpp"
#include "util/storage/Plugin.hpp"
#include "util/storage/tree/Node.hpp"

namespace opentxs::storage
{
Units::Units(
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

auto Units::Alias(const UnallocatedCString& id) const -> UnallocatedCString
{
    return get_alias(id);
}

auto Units::Delete(const UnallocatedCString& id) -> bool
{
    return delete_item(id);
}

void Units::init(const UnallocatedCString& hash)
{
    std::shared_ptr<proto::StorageUnits> serialized;
    driver_.LoadProto(hash, serialized);

    if (!serialized) {
        std::cerr << __func__ << ": Failed to load unit index file."
                  << std::endl;
        abort();
    }

    init_version(2, *serialized);

    for (const auto& it : serialized->unit()) {
        item_map_.emplace(
            it.itemid(), Metadata{it.hash(), it.alias(), 0, false});
    }
}

auto Units::Load(
    const UnallocatedCString& id,
    std::shared_ptr<proto::UnitDefinition>& output,
    UnallocatedCString& alias,
    const bool checking) const -> bool
{
    return load_proto<proto::UnitDefinition>(id, output, alias, checking);
}

void Units::Map(UnitLambda lambda) const { map<proto::UnitDefinition>(lambda); }

auto Units::save(const std::unique_lock<std::mutex>& lock) const -> bool
{
    if (!verify_write_lock(lock)) {
        std::cerr << __func__ << ": Lock failure." << std::endl;
        abort();
    }

    auto serialized = serialize();

    if (!proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

auto Units::serialize() const -> proto::StorageUnits
{
    proto::StorageUnits serialized;
    serialized.set_version(version_);

    for (const auto& item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = check_hash(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(
                version_, item.first, item.second, *serialized.add_unit());
        }
    }

    return serialized;
}

auto Units::SetAlias(
    const UnallocatedCString& id,
    const UnallocatedCString& alias) -> bool
{
    return set_alias(id, alias);
}

auto Units::Store(
    const proto::UnitDefinition& data,
    const UnallocatedCString& alias,
    UnallocatedCString& plaintext) -> bool
{
    return store_proto(data, data.id(), alias, plaintext);
}
}  // namespace opentxs::storage
