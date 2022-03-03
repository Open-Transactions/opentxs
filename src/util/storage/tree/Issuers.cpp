// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                   // IWYU pragma: associated
#include "1_Internal.hpp"                 // IWYU pragma: associated
#include "util/storage/tree/Issuers.hpp"  // IWYU pragma: associated

#include <cstdlib>
#include <iostream>
#include <tuple>
#include <utility>

#include "Proto.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/verify/Issuer.hpp"
#include "internal/serialization/protobuf/verify/StorageIssuers.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/storage/Driver.hpp"
#include "serialization/protobuf/Issuer.pb.h"
#include "serialization/protobuf/StorageIssuers.pb.h"
#include "serialization/protobuf/StorageItemHash.pb.h"
#include "util/storage/Plugin.hpp"
#include "util/storage/tree/Node.hpp"

namespace opentxs::storage
{
Issuers::Issuers(const Driver& storage, const UnallocatedCString& hash)
    : Node(storage, hash)
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        blank(current_version_);
    }
}

auto Issuers::Delete(const UnallocatedCString& id) -> bool
{
    return delete_item(id);
}

void Issuers::init(const UnallocatedCString& hash)
{
    std::shared_ptr<proto::StorageIssuers> serialized;
    driver_.LoadProto(hash, serialized);

    if (!serialized) {
        std::cerr << __func__ << ": Failed to load issuers index file."
                  << std::endl;
        abort();
    }

    init_version(current_version_, *serialized);

    for (const auto& it : serialized->issuer()) {
        item_map_.emplace(
            it.itemid(), Metadata{it.hash(), it.alias(), 0, false});
    }
}

auto Issuers::Load(
    const UnallocatedCString& id,
    std::shared_ptr<proto::Issuer>& output,
    UnallocatedCString& alias,
    const bool checking) const -> bool
{
    return load_proto<proto::Issuer>(id, output, alias, checking);
}

auto Issuers::save(const std::unique_lock<std::mutex>& lock) const -> bool
{
    if (!verify_write_lock(lock)) {
        std::cerr << __func__ << ": Lock failure." << std::endl;
        abort();
    }

    auto serialized = serialize();

    if (false == proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

auto Issuers::serialize() const -> proto::StorageIssuers
{
    proto::StorageIssuers serialized;
    serialized.set_version(version_);

    for (const auto& item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = check_hash(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(
                version_, item.first, item.second, *serialized.add_issuer());
        }
    }

    return serialized;
}

auto Issuers::Store(const proto::Issuer& data, const UnallocatedCString& alias)
    -> bool
{
    return store_proto(data, data.id(), alias);
}
}  // namespace opentxs::storage
