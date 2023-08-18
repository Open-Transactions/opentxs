// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/tree/Notary.hpp"  // IWYU pragma: associated

#include <BlindedSeriesList.pb.h>
#include <SpentTokenList.pb.h>
#include <StorageEnums.pb.h>
#include <StorageItemHash.pb.h>
#include <StorageNotary.pb.h>
#include <atomic>
#include <stdexcept>
#include <variant>

#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/SpentTokenList.hpp"
#include "internal/serialization/protobuf/verify/StorageNotary.hpp"
#include "internal/util/DeferredConstruction.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/storage/Types.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/util/Log.hpp"
#include "util/storage/tree/Node.hpp"

namespace opentxs
{
constexpr auto STORAGE_NOTARY_VERSION = 1;
constexpr auto STORAGE_MINT_SERIES_VERSION = 1;
constexpr auto STORAGE_MINT_SPENT_LIST_VERSION = 1;
}  // namespace opentxs

namespace opentxs::storage::tree
{
using namespace std::literals;

Notary::Notary(
    const api::Crypto& crypto,
    const api::session::Factory& factory,
    const driver::Plugin& storage,
    const Hash& hash,
    const identifier::Notary& id)
    : Node(
          crypto,
          factory,
          storage,
          hash,
          OT_PRETTY_CLASS(),
          STORAGE_NOTARY_VERSION)
    , id_(id)
    , mint_map_()
{
    if (is_valid(hash)) {
        init(hash);
    } else {
        blank();
    }
}

auto Notary::CheckSpent(
    const identifier::UnitDefinition& unit,
    const MintSeries series,
    std::string_view key) const -> bool
{
    if (key.empty()) { throw std::runtime_error("Invalid token key"); }

    Lock lock(write_lock_);
    const auto list = get_or_create_list(lock, unit, series);

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
    const identifier::UnitDefinition& unitID,
    const MintSeries series,
    std::shared_ptr<proto::SpentTokenList>& output) const -> Hash
{
    auto hash = Hash{};
    output.reset(new proto::SpentTokenList);

    OT_ASSERT(output);

    auto& list = *output;
    list.set_version(STORAGE_MINT_SPENT_LIST_VERSION);
    list.set_notary(id_.asBase58(crypto_));
    list.set_unit(unitID.asBase58(crypto_));
    list.set_series(series);

    const auto saved = StoreProto(list, hash);

    if (false == saved) {
        throw std::runtime_error("Failed to create spent token list");
    }

    return hash;
}

auto Notary::dump(const Lock& lock, const Log& log, Vector<Hash>& out)
    const noexcept -> bool
{
    if (false == is_valid(root_)) { return true; }

    if (false == Node::dump(lock, log, out)) { return false; }

    for (const auto& [unitID, map] : mint_map_) {
        out.reserve(out.size() + map.size());

        for (const auto& [series, hash] : map) {
            log(OT_PRETTY_CLASS())(name_)("adding cash series hash ")(hash)
                .Flush();
            out.emplace_back(hash);
        }
    }

    return true;
}

auto Notary::get_or_create_list(
    const Lock& lock,
    const identifier::UnitDefinition& unitID,
    const MintSeries series) const -> proto::SpentTokenList
{
    OT_ASSERT(verify_write_lock(lock));

    std::shared_ptr<proto::SpentTokenList> output{};
    auto& hash = mint_map_[unitID][series];

    if (false == is_valid(hash)) {
        hash = create_list(unitID, series, output);
    } else {
        LoadProto(hash, output, verbose);
    }

    if (false == bool(output)) {
        throw std::runtime_error("Failed to load spent token list");
    }

    return *output;
}

auto Notary::init(const Hash& hash) noexcept(false) -> void
{
    auto p = std::shared_ptr<proto::StorageNotary>{};

    if (LoadProto(hash, p, verbose) && p) {
        const auto& proto = *p;

        switch (set_original_version(proto.version())) {
            case 1u:
            default: {
                if (id_.empty()) {
                    id_ = factory_.NotaryIDFromBase58(proto.id());
                } else if (id_ != factory_.NotaryIDFromBase58(proto.id())) {
                    throw std::runtime_error{
                        "notary id does not match expected value "s};
                }

                for (const auto& it : proto.series()) {
                    auto& unitMap =
                        mint_map_[factory_.UnitIDFromBase58(it.unit())];

                    for (const auto& storageHash : it.series()) {
                        const auto series = std::stoul(storageHash.alias());
                        unitMap[series] = read(storageHash.hash());
                    }
                }
            }
        }
    } else {
        throw std::runtime_error{
            "failed to load root object file in "s.append(OT_PRETTY_CLASS())};
    }
}

auto Notary::MarkSpent(
    const identifier::UnitDefinition& unit,
    const MintSeries series,
    std::string_view key) -> bool
{
    if (key.empty()) {
        LogError()(OT_PRETTY_CLASS())("Invalid key ").Flush();

        return false;
    }

    Lock lock(write_lock_);
    auto list = get_or_create_list(lock, unit, series);
    list.add_spent(key.data(), key.size());

    OT_ASSERT(proto::Validate(list, VERBOSE));

    auto& hash = mint_map_[unit][series];
    LogTrace()(OT_PRETTY_CLASS())("Token ")(key)(" marked as spent.").Flush();

    return StoreProto(list, hash);
}

auto Notary::save(const Lock& lock) const -> bool
{
    if (false == verify_write_lock(lock)) {
        LogError()(OT_PRETTY_CLASS())("Lock failure").Flush();

        OT_FAIL;
    }

    auto serialized = serialize();

    if (false == proto::Validate(serialized, VERBOSE)) { return false; }

    return StoreProto(serialized, root_);
}

auto Notary::serialize() const -> proto::StorageNotary
{
    proto::StorageNotary serialized;
    serialized.set_version(version_);
    serialized.set_id(id_.asBase58(crypto_));

    for (const auto& [unitID, seriesMap] : mint_map_) {
        auto& series = *serialized.add_series();
        series.set_version(STORAGE_MINT_SERIES_VERSION);
        series.set_notary(id_.asBase58(crypto_));
        series.set_unit(unitID.asBase58(crypto_));

        for (const auto& [seriesNumber, hash] : seriesMap) {
            serialize_index(
                {},
                hash,
                std::to_string(seriesNumber),
                *series.add_series(),
                proto::STORAGEHASH_PROTO);
        }
    }

    return serialized;
}

auto Notary::upgrade(const Lock& lock) noexcept -> bool
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
