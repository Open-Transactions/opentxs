// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "util/storage/tree/Notary.hpp"  // IWYU pragma: associated

#include <BlindedSeriesList.pb.h>
#include <SpentTokenList.pb.h>
#include <StorageEnums.pb.h>
#include <StorageItemHash.pb.h>
#include <StorageNotary.pb.h>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/SpentTokenList.hpp"
#include "internal/serialization/protobuf/verify/StorageNotary.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/storage/Driver.hpp"
#include "util/storage/Plugin.hpp"
#include "util/storage/tree/Node.hpp"

namespace opentxs
{
constexpr auto STORAGE_NOTARY_VERSION = 1;
constexpr auto STORAGE_MINT_SERIES_VERSION = 1;
constexpr auto STORAGE_MINT_SERIES_HASH_VERSION = 2;
constexpr auto STORAGE_MINT_SPENT_LIST_VERSION = 1;
}  // namespace opentxs

namespace opentxs::storage
{
Notary::Notary(
    const api::Crypto& crypto,
    const api::session::Factory& factory,
    const Driver& storage,
    const UnallocatedCString& hash,
    const UnallocatedCString& id)
    : Node(crypto, factory, storage, hash)
    , id_(id)
    , mint_map_()
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        blank(STORAGE_NOTARY_VERSION);
    }
}

auto Notary::CheckSpent(
    const identifier::UnitDefinition& unit,
    const MintSeries series,
    const UnallocatedCString& key) const -> bool
{
    if (key.empty()) { throw std::runtime_error("Invalid token key"); }

    Lock lock(write_lock_);
    const auto list = get_or_create_list(lock, unit.asBase58(crypto_), series);

    for (const auto& spent : list.spent()) {
        if (spent == key) {
            LogTrace()(OT_PRETTY_CLASS())("Token ")(key)(" is already spent.")
                .Flush();

            return true;
        }
    }

    LogTrace()(OT_PRETTY_CLASS())("Token ")(key)(" has never been spent.")
        .Flush();

    return false;
}

auto Notary::create_list(
    const UnallocatedCString& unitID,
    const MintSeries series,
    std::shared_ptr<proto::SpentTokenList>& output) const -> UnallocatedCString
{
    UnallocatedCString hash{};
    output.reset(new proto::SpentTokenList);

    OT_ASSERT(output);

    auto& list = *output;
    list.set_version(STORAGE_MINT_SPENT_LIST_VERSION);
    list.set_notary(id_);
    list.set_unit(unitID);
    list.set_series(series);

    const auto saved = driver_.StoreProto(list, hash);

    if (false == saved) {
        throw std::runtime_error("Failed to create spent token list");
    }

    return hash;
}

auto Notary::get_or_create_list(
    const Lock& lock,
    const UnallocatedCString& unitID,
    const MintSeries series) const -> proto::SpentTokenList
{
    OT_ASSERT(verify_write_lock(lock));

    std::shared_ptr<proto::SpentTokenList> output{};
    auto& hash = mint_map_[unitID][series];

    if (hash.empty()) {
        hash = create_list(unitID, series, output);
    } else {
        driver_.LoadProto(hash, output);
    }

    if (false == bool(output)) {
        throw std::runtime_error("Failed to load spent token list");
    }

    return *output;
}

void Notary::init(const UnallocatedCString& hash)
{
    std::shared_ptr<proto::StorageNotary> serialized;
    driver_.LoadProto(hash, serialized);

    if (false == bool(serialized)) {
        LogError()(OT_PRETTY_CLASS())("Failed to load index file").Flush();

        OT_FAIL;
    }

    init_version(STORAGE_NOTARY_VERSION, *serialized);
    id_ = serialized->id();

    for (const auto& it : serialized->series()) {
        auto& unitMap = mint_map_[it.unit()];

        for (const auto& storageHash : it.series()) {
            const auto series = std::stoul(storageHash.alias());
            unitMap[series] = storageHash.hash();
        }
    }
}

auto Notary::MarkSpent(
    const identifier::UnitDefinition& unit,
    const MintSeries series,
    const UnallocatedCString& key) -> bool
{
    if (key.empty()) {
        LogError()(OT_PRETTY_CLASS())("Invalid key ").Flush();

        return false;
    }

    Lock lock(write_lock_);
    auto list = get_or_create_list(lock, unit.asBase58(crypto_), series);
    list.add_spent(key);

    OT_ASSERT(proto::Validate(list, VERBOSE));

    auto& hash = mint_map_[unit.asBase58(crypto_)][series];
    LogTrace()(OT_PRETTY_CLASS())("Token ")(key)(" marked as spent.").Flush();

    return driver_.StoreProto(list, hash);
}

auto Notary::save(const Lock& lock) const -> bool
{
    if (false == verify_write_lock(lock)) {
        LogError()(OT_PRETTY_CLASS())("Lock failure").Flush();

        OT_FAIL;
    }

    auto serialized = serialize();

    if (false == proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

auto Notary::serialize() const -> proto::StorageNotary
{
    proto::StorageNotary serialized;
    serialized.set_version(version_);
    serialized.set_id(id_);

    for (const auto& [unitID, seriesMap] : mint_map_) {
        auto& series = *serialized.add_series();
        series.set_version(STORAGE_MINT_SERIES_VERSION);
        series.set_notary(id_);
        series.set_unit(unitID);

        for (const auto& [seriesNumber, hash] : seriesMap) {
            auto& storageHash = *series.add_series();
            const auto seriesString = std::to_string(seriesNumber);
            storageHash.set_version(STORAGE_MINT_SERIES_HASH_VERSION);
            storageHash.set_itemid(
                factory_.IdentifierFromBase58(seriesString).asBase58(crypto_));
            storageHash.set_hash(hash);
            storageHash.set_alias(seriesString);
            storageHash.set_type(proto::STORAGEHASH_PROTO);
        }
    }

    return serialized;
}
}  // namespace opentxs::storage
