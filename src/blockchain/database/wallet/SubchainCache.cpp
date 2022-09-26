// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/database/wallet/SubchainCache.hpp"  // IWYU pragma: associated

#include <boost/exception/exception.hpp>
#include <chrono>  // IWYU pragma: keep
#include <cstring>
#include <mutex>
#include <stdexcept>
#include <utility>

#include "blockchain/database/wallet/Pattern.hpp"
#include "blockchain/database/wallet/Position.hpp"
#include "blockchain/database/wallet/SubchainID.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/TSV.hpp"
#include "internal/util/storage/lmdb/Database.hpp"
#include "internal/util/storage/lmdb/Transaction.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"  // IWYU pragma: keep

namespace opentxs::blockchain::database::wallet
{
SubchainCache::SubchainCache(
    const api::Session& api,
    const storage::lmdb::Database& lmdb) noexcept
    : api_(api)
    , lmdb_(lmdb)
    , subchain_id_lock_()
    , subchain_id_()
    , last_indexed_lock_()
    , last_indexed_()
    , last_scanned_lock_()
    , last_scanned_()
    , patterns_lock_()
    , patterns_()
    , pattern_index_lock_()
    , pattern_index_()
{
    subchain_id_.reserve(reserve_);
    last_indexed_.reserve(reserve_);
    last_scanned_.reserve(reserve_);
    pattern_index_.reserve(reserve_);
    patterns_.reserve(reserve_ * reserve_);
}

auto SubchainCache::AddPattern(
    const PatternID& id,
    const Bip32Index index,
    const ReadView data,
    storage::lmdb::Transaction& tx) noexcept -> bool
{
    try {
        auto lock = boost::unique_lock<Mutex>{patterns_lock_};
        auto& patterns = patterns_[id];
        auto [it, added] = patterns.emplace(index, data);

        if (false == added) {
            LogTrace()(OT_PRETTY_CLASS())("Pattern already exists").Flush();

            return true;
        }

        const auto rc =
            lmdb_.Store(wallet::patterns_, id.Bytes(), reader(it->data_), tx)
                .first;

        if (false == rc) {
            patterns.erase(it);

            throw std::runtime_error{"failed to write pattern"};
        }

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto SubchainCache::AddPatternIndex(
    const SubchainIndex& key,
    const PatternID& value,
    storage::lmdb::Transaction& tx) noexcept -> bool
{
    try {
        auto lock = boost::unique_lock<Mutex>{pattern_index_lock_};
        auto& index = pattern_index_[key];
        auto [it, added] = index.emplace(value);

        if (false == added) {
            LogTrace()(OT_PRETTY_CLASS())("Pattern index already exists")
                .Flush();

            return true;
        }

        const auto rc =
            lmdb_.Store(wallet::pattern_index_, key.Bytes(), value.Bytes(), tx)
                .first;

        if (false == rc) {
            index.erase(it);

            throw std::runtime_error{"failed to write pattern index"};
        }

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto SubchainCache::Clear() noexcept -> void
{
    auto iLock =
        boost::unique_lock<Mutex>{last_indexed_lock_, boost::defer_lock};
    auto sLock =
        boost::unique_lock<Mutex>{last_scanned_lock_, boost::defer_lock};
    std::lock(iLock, sLock);
    last_indexed_.clear();
    last_scanned_.clear();
}

auto SubchainCache::DecodeIndex(const SubchainIndex& key) noexcept(false)
    -> const db::SubchainID&
{
    return load_index(key);
}

auto SubchainCache::GetIndex(
    const NodeID& subaccount,
    const crypto::Subchain subchain,
    const cfilter::Type type,
    const VersionNumber version,
    storage::lmdb::Transaction& tx) noexcept -> SubchainIndex
{
    const auto index = subchain_index(subaccount, subchain, type, version);

    try {
        load_index(index);

        return index;
    } catch (...) {
    }

    try {
        auto lock = boost::unique_lock<Mutex>{subchain_id_lock_};
        auto [it, added] = subchain_id_.try_emplace(
            index, subchain, type, version, subaccount);

        if (false == added) {
            throw std::runtime_error{"failed to update cache"};
        }

        const auto& decoded = it->second;
        const auto key = index.Bytes();

        if (false == lmdb_.Exists(wallet::id_index_, key)) {
            const auto rc =
                lmdb_.Store(wallet::id_index_, key, reader(decoded.data_), tx)
                    .first;

            if (false == rc) {
                throw std::runtime_error{"Failed to write index to database"};
            }
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        OT_FAIL;
    }

    return index;
}

auto SubchainCache::GetLastIndexed(const SubchainIndex& subchain) noexcept
    -> std::optional<Bip32Index>
{
    try {

        return load_last_indexed(subchain);
    } catch (const std::exception& e) {
        LogTrace()(OT_PRETTY_CLASS())(e.what()).Flush();

        return std::nullopt;
    }
}

auto SubchainCache::GetLastScanned(const SubchainIndex& subchain) noexcept
    -> block::Position
{
    try {
        const auto& serialized = load_last_scanned(subchain);

        return serialized.Decode(api_);
    } catch (const std::exception& e) {
        LogVerbose()(OT_PRETTY_CLASS())(e.what()).Flush();
        static const auto null = block::Position{};

        return null;
    }
}

auto SubchainCache::GetPattern(const PatternID& id) noexcept
    -> const dbPatterns&
{
    return load_pattern(id);
}

auto SubchainCache::GetPatternIndex(const SubchainIndex& id) noexcept
    -> const dbPatternIndex&
{
    return load_pattern_index(id);
}

auto SubchainCache::load_index(const SubchainIndex& key) noexcept(false)
    -> const db::SubchainID&
{
    auto lock = boost::upgrade_lock<Mutex>{subchain_id_lock_};
    auto& map = subchain_id_;
    auto it = map.find(key);

    if (map.end() != it) { return it->second; }

    auto write = boost::upgrade_to_unique_lock<Mutex>{lock};
    lmdb_.Load(wallet::id_index_, key.Bytes(), [&](const auto bytes) {
        const auto [i, added] = map.try_emplace(key, bytes);

        if (false == added) {
            throw std::runtime_error{"Failed to update cache"};
        }

        it = i;
    });

    if (map.end() != it) { return it->second; }

    const auto error =
        UnallocatedCString{"index "} + key.asHex() + " not found in database";

    throw std::out_of_range{error};
}

auto SubchainCache::load_last_indexed(const SubchainIndex& key) noexcept(false)
    -> const Bip32Index&
{
    auto lock = boost::upgrade_lock<Mutex>{last_indexed_lock_};
    auto& map = last_indexed_;
    auto it = map.find(key);

    if (map.end() != it) { return it->second; }

    auto write = boost::upgrade_to_unique_lock<Mutex>{lock};
    lmdb_.Load(wallet::last_indexed_, key.Bytes(), [&](const auto bytes) {
        if (sizeof(Bip32Index) == bytes.size()) {
            auto value = Bip32Index{};
            std::memcpy(&value, bytes.data(), bytes.size());
            auto [i, added] = map.try_emplace(key, value);

            OT_ASSERT(added);

            it = i;
        } else {

            throw std::runtime_error{"invalid value in database"};
        }
    });

    if (map.end() != it) { return it->second; }

    const auto error = UnallocatedCString{"last indexed for "} + key.asHex() +
                       " not found in database";

    throw std::out_of_range{error};
}

auto SubchainCache::load_last_scanned(const SubchainIndex& key) noexcept(false)
    -> const db::Position&
{
    auto lock = boost::upgrade_lock<Mutex>{last_scanned_lock_};
    auto& map = last_scanned_;
    auto it = map.find(key);

    if (map.end() != it) { return it->second; }

    auto write = boost::upgrade_to_unique_lock<Mutex>{lock};
    lmdb_.Load(wallet::last_scanned_, key.Bytes(), [&](const auto bytes) {
        auto [i, added] = map.try_emplace(key, bytes);

        if (false == added) {
            throw std::runtime_error{"Failed to update cache"};
        }

        it = i;
    });

    if (map.end() != it) { return it->second; }

    const auto error = UnallocatedCString{"last scanned for "} + key.asHex() +
                       " not found in database";

    throw std::out_of_range{error};
}

auto SubchainCache::load_pattern(const PatternID& key) noexcept
    -> const dbPatterns&
{
    auto lock = boost::upgrade_lock<Mutex>{patterns_lock_};
    auto& map = patterns_;

    if (auto it = map.find(key); map.end() != it) { return it->second; }

    auto write = boost::upgrade_to_unique_lock<Mutex>{lock};
    auto& patterns = map[key];
    lmdb_.Load(
        wallet::patterns_,
        key.Bytes(),
        [&](const auto bytes) { patterns.emplace(bytes); },
        Mode::Multiple);

    return patterns;
}

auto SubchainCache::load_pattern_index(const SubchainIndex& key) noexcept
    -> const dbPatternIndex&
{
    auto lock = boost::upgrade_lock<Mutex>{pattern_index_lock_};
    auto& map = pattern_index_;

    if (auto it = map.find(key); map.end() != it) { return it->second; }

    auto write = boost::upgrade_to_unique_lock<Mutex>{lock};
    auto& index = map[key];
    lmdb_.Load(
        wallet::pattern_index_,
        key.Bytes(),
        [&](const auto bytes) {
            index.emplace([&] {
                auto out = identifier::Generic{};
                out.Assign(bytes);

                return out;
            }());
        },
        Mode::Multiple);

    return index;
}

auto SubchainCache::SetLastIndexed(
    const SubchainIndex& subchain,
    const Bip32Index value,
    storage::lmdb::Transaction& tx) noexcept -> bool
{
    try {
        auto lock = boost::unique_lock<Mutex>{last_indexed_lock_};
        auto& indexed = last_indexed_[subchain];
        indexed = value;
        const auto output =
            lmdb_.Store(wallet::last_indexed_, subchain.Bytes(), tsv(value), tx)
                .first;

        if (false == output) {
            throw std::runtime_error{"failed to update database"};
        }

        return output;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto SubchainCache::SetLastScanned(
    const SubchainIndex& subchain,
    const block::Position& value) noexcept -> bool
{
    auto tx = lmdb_.TransactionRW();

    return SetLastScanned(subchain, value, tx);
}

auto SubchainCache::SetLastScanned(
    const SubchainIndex& subchain,
    const block::Position& value,
    storage::lmdb::Transaction& tx) noexcept -> bool
{
    try {
        const auto& scanned = [&]() -> auto&
        {
            auto lock = boost::unique_lock<Mutex>{last_scanned_lock_};
            last_scanned_.erase(subchain);
            const auto [it, added] = last_scanned_.try_emplace(subchain, value);

            if (false == added) {
                throw std::runtime_error{"failed to update cache"};
            }

            return it->second;
        }
        ();
        const auto output = lmdb_
                                .Store(
                                    wallet::last_scanned_,
                                    subchain.Bytes(),
                                    reader(scanned.data_),
                                    tx)
                                .first;

        if (false == output) {
            throw std::runtime_error{"failed to update database"};
        }

        return output;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto SubchainCache::subchain_index(
    const NodeID& subaccount,
    const crypto::Subchain subchain,
    const cfilter::Type type,
    const VersionNumber version) const noexcept -> SubchainIndex
{
    auto preimage = api_.Factory().Data();
    preimage.Assign(subaccount);
    preimage.Concatenate(&subchain, sizeof(subchain));
    preimage.Concatenate(&type, sizeof(type));
    preimage.Concatenate(&version, sizeof(version));

    return api_.Factory().IdentifierFromPreimage(preimage.Bytes());
}

SubchainCache::~SubchainCache() = default;
}  // namespace opentxs::blockchain::database::wallet
