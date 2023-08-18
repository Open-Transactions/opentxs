// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/tree/Issuers.hpp"  // IWYU pragma: associated

#include <Issuer.pb.h>
#include <StorageIssuers.pb.h>
#include <atomic>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <tuple>
#include <utility>

#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/Issuer.hpp"
#include "internal/serialization/protobuf/verify/StorageIssuers.hpp"
#include "internal/util/DeferredConstruction.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/storage/Types.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"
#include "util/storage/tree/Node.hpp"

namespace opentxs::storage::tree
{
using namespace std::literals;

Issuers::Issuers(
    const api::Crypto& crypto,
    const api::session::Factory& factory,
    const driver::Plugin& storage,
    const Hash& hash)
    : Node(crypto, factory, storage, hash, OT_PRETTY_CLASS(), current_version_)
{
    if (is_valid(hash)) {
        init(hash);
    } else {
        blank();
    }
}

auto Issuers::Delete(const identifier::Nym& id) -> bool
{
    return delete_item(id);
}

auto Issuers::init(const Hash& hash) noexcept(false) -> void
{
    auto p = std::shared_ptr<proto::StorageIssuers>{};

    if (LoadProto(hash, p, verbose) && p) {
        const auto& proto = *p;

        switch (set_original_version(proto.version())) {
            case 1u:
            default: {
                init_map(proto.issuer());
            }
        }
    } else {
        throw std::runtime_error{
            "failed to load root object file in "s.append(OT_PRETTY_CLASS())};
    }
}

auto Issuers::Load(
    const identifier::Nym& id,
    std::shared_ptr<proto::Issuer>& output,
    UnallocatedCString& alias,
    ErrorReporting checking) const -> bool
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

    return StoreProto(serialized, root_);
}

auto Issuers::serialize() const -> proto::StorageIssuers
{
    proto::StorageIssuers serialized;
    serialized.set_version(version_);

    for (const auto& item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = is_valid(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(item.first, item.second, *serialized.add_issuer());
        }
    }

    return serialized;
}

auto Issuers::Store(const proto::Issuer& data, std::string_view alias) -> bool
{
    return store_proto(data, factory_.IdentifierFromBase58(data.id()), alias);
}

auto Issuers::upgrade(const Lock& lock) noexcept -> bool
{
    auto changed = Node::upgrade(lock);

    switch (original_version_.get()) {
        case 1u:
        default: {
        }
    }

    return changed;
}
}  // namespace opentxs::storage::tree
