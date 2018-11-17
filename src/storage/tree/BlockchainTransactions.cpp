// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "BlockchainTransactions.hpp"

#include "storage/Plugin.hpp"

#define CURRENT_VERSION 1

//#define OT_METHOD "opentxs::storage::BlockchainTransactions::"

namespace opentxs
{
namespace storage
{
BlockchainTransactions::BlockchainTransactions(
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

bool BlockchainTransactions::Delete(const std::string& id)
{
    return delete_item(id);
}

void BlockchainTransactions::init(const std::string& hash)
{
    std::shared_ptr<proto::StorageBlockchainTransactions> serialized{nullptr};
    driver_.LoadProto(hash, serialized);

    if (false == bool(serialized)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to load blockchain transaction index file.")
            .Flush();

        abort();
    }

    version_ = serialized->version();

    // Upgrade version
    if (CURRENT_VERSION > version_) { version_ = CURRENT_VERSION; }

    for (const auto& it : serialized->transaction()) {
        item_map_.emplace(
            it.itemid(), Metadata{it.hash(), it.alias(), 0, false});
    }
}

bool BlockchainTransactions::Load(
    const std::string& id,
    std::shared_ptr<proto::BlockchainTransaction>& output,
    const bool checking) const
{
    std::string alias{};

    return load_proto<proto::BlockchainTransaction>(
        id, output, alias, checking);
}

bool BlockchainTransactions::save(const Lock& lock) const
{
    if (false == verify_write_lock(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Lock failure.").Flush();

        abort();
    }

    auto serialized = serialize();

    if (false == proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

proto::StorageBlockchainTransactions BlockchainTransactions::serialize() const
{
    proto::StorageBlockchainTransactions serialized{};
    serialized.set_version(version_);

    for (const auto item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = check_hash(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(
                item.first, item.second, *serialized.add_transaction());
        }
    }

    return serialized;
}

bool BlockchainTransactions::Store(const proto::BlockchainTransaction& data)
{
    std::string alias{};
    std::string plaintext{};

    return store_proto(data, data.txid(), alias, plaintext);
}
}  // namespace storage
}  // namespace opentxs
