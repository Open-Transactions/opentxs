// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/database/wallet/SubchainCache.hpp"  // IWYU pragma: associated

#include <chrono>  // IWYU pragma: keep
#include <cstring>
#include <stdexcept>
#include <utility>

#include "blockchain/database/wallet/Pattern.hpp"
#include "blockchain/database/wallet/Position.hpp"
#include "blockchain/database/wallet/SubchainID.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/TSV.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"  // IWYU pragma: keep

namespace opentxs::blockchain::database::wallet
{
SubchainCache::SubchainCache(
    const api::Session& api,
    const storage::lmdb::LMDB& lmdb) noexcept
    : api_(api)
    , lmdb_(lmdb)
    , id_lock_()
    , subchain_id_()
    , index_lock_()
    , last_indexed_()
    , scanned_lock_()
    , last_scanned_()
    , pattern_lock_()
    , patterns_()
    , pattern_index_lock_()
    , pattern_index_()
    , match_index_lock_()
    , match_index_()
{
    subchain_id_.reserve(reserve_);
    last_indexed_.reserve(reserve_);
    last_scanned_.reserve(reserve_);
    pattern_index_.reserve(reserve_);
    patterns_.reserve(reserve_ * reserve_);
}

auto SubchainCache::AddMatch(
    const eLock&,
    const ReadView key,
    const PatternID& value,
    MDB_txn* tx) noexcept -> bool
{
    try {
        const auto hash = [&] {
            auto out = api_.Factory().Data();
            out->Assign(key);

            return out;
        }();
        auto& index = match_index_[hash];
        auto [it, added] = index.emplace(value);

        if (false == added) {
            LogTrace()(OT_PRETTY_CLASS())("Match index already exists").Flush();

            return true;
        }

        const auto rc =
            lmdb_.Store(wallet::match_index_, key, value.Bytes(), tx).first;

        if (false == rc) {
            index.erase(it);

            throw std::runtime_error{"failed to write match index"};
        }

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto SubchainCache::AddPattern(
    const eLock&,
    const PatternID& id,
    const Bip32Index index,
    const ReadView data,
    MDB_txn* tx) noexcept -> bool
{
    try {
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
    const eLock&,
    const SubchainIndex& key,
    const PatternID& value,
    MDB_txn* tx) noexcept -> bool
{
    try {
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

auto SubchainCache::Clear(const eLock&) noexcept -> void
{
    last_indexed_.clear();
    last_scanned_.clear();
}

auto SubchainCache::DecodeIndex(
    const sLock&,
    const SubchainIndex& key) noexcept(false) -> const db::SubchainID&
{
    auto lock = Lock{id_lock_};

    return load_index(key);
}

auto SubchainCache::GetIndex(
    const sLock&,
    const NodeID& subaccount,
    const Subchain subchain,
    const filter::Type type,
    const VersionNumber version,
    MDB_txn* tx) noexcept -> pSubchainIndex
{
    const auto index = subchain_index(subaccount, subchain, type, version);
    auto lock = Lock{id_lock_};

    try {
        load_index(index);

        return index;
    } catch (...) {
    }

    try {
        auto& map = subchain_id_;
        auto [it, added] =
            map.try_emplace(index, subchain, type, version, subaccount);

        if (false == added) {
            throw std::runtime_error{"failed to update cache"};
        }

        const auto& decoded = it->second;
        const auto key = index->Bytes();

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

auto SubchainCache::GetLastIndexed(
    const sLock&,
    const SubchainIndex& subchain) noexcept -> std::optional<Bip32Index>
{
    auto lock = Lock{index_lock_};

    try {

        return load_last_indexed(subchain);
    } catch (const std::exception& e) {
        LogTrace()(OT_PRETTY_CLASS())(e.what()).Flush();

        return std::nullopt;
    }
}

auto SubchainCache::GetLastScanned(
    const sLock&,
    const SubchainIndex& subchain) noexcept -> block::Position
{
    auto lock = Lock{scanned_lock_};

    try {
        const auto& serialized = load_last_scanned(subchain);

        return serialized.Decode(api_);
    } catch (const std::exception& e) {
        LogVerbose()(OT_PRETTY_CLASS())(e.what()).Flush();
        static const auto null = make_blank<block::Position>::value(api_);

        return null;
    }
}

auto SubchainCache::GetLastScanned(
    const eLock&,
    const SubchainIndex& subchain) noexcept -> block::Position
{
    try {
        const auto& serialized = load_last_scanned(subchain);

        return serialized.Decode(api_);
    } catch (const std::exception& e) {
        LogVerbose()(OT_PRETTY_CLASS())(e.what()).Flush();
        static const auto null = make_blank<block::Position>::value(api_);

        return null;
    }
}

auto SubchainCache::GetMatchIndex(const sLock&, const ReadView id) noexcept
    -> const dbPatternIndex&
{
    auto lock = Lock{match_index_lock_};

    return load_match_index(id);
}

auto SubchainCache::GetPattern(const sLock&, const PatternID& id) noexcept
    -> const dbPatterns&
{
    auto lock = Lock{pattern_lock_};

    return load_pattern(id);
}

auto SubchainCache::GetPatternIndex(
    const sLock&,
    const SubchainIndex& id) noexcept -> const dbPatternIndex&
{
    auto lock = Lock{pattern_index_lock_};

    return load_pattern_index(id);
}

auto SubchainCache::load_index(const SubchainIndex& key) noexcept(false)
    -> const db::SubchainID&
{
    auto& map = subchain_id_;
    auto it = map.find(key);

    if (map.end() != it) { return it->second; }

#if defined OPENTXS_DETAILED_DEBUG
    LogTrace()(OT_PRETTY_CLASS())("cache miss, loading index ")(key.asHex())(
        " from database")
        .Flush();
    const auto start = Clock::now();
#endif  // defined OPENTXS_DETAILED_DEBUG
    lmdb_.Load(wallet::id_index_, key.Bytes(), [&](const auto bytes) {
        const auto [i, added] = map.try_emplace(key, bytes);

        if (false == added) {
            throw std::runtime_error{"Failed to update cache"};
        }

        it = i;
    });
#if defined OPENTXS_DETAILED_DEBUG
    const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
        Clock::now() - start);
    LogTrace()(OT_PRETTY_CLASS())("database query finished in ")(
        elapsed.count())(" microseconds")
        .Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG

    if (map.end() != it) {
#if defined OPENTXS_DETAILED_DEBUG
        LogTrace()(OT_PRETTY_CLASS())("index ")(key.asHex())(
            " found in database")
            .Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG

        return it->second;
    }

    const auto error =
        UnallocatedCString{"index "} + key.asHex() + " not found in database";

    throw std::out_of_range{error};
}

auto SubchainCache::load_last_indexed(const SubchainIndex& key) noexcept(false)
    -> const Bip32Index&
{
    auto& map = last_indexed_;
    auto it = map.find(key);

    if (map.end() != it) { return it->second; }

#if defined OPENTXS_DETAILED_DEBUG
    LogTrace()(OT_PRETTY_CLASS())("cache miss, loading last indexed for ")(
        key.asHex())(" from database")
        .Flush();
    const auto start = Clock::now();
#endif  // defined OPENTXS_DETAILED_DEBUG
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
#if defined OPENTXS_DETAILED_DEBUG
    const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
        Clock::now() - start);
    LogTrace()(OT_PRETTY_CLASS())("database query finished in ")(
        elapsed.count())(" microseconds")
        .Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG

    if (map.end() != it) {
#if defined OPENTXS_DETAILED_DEBUG
        LogTrace()(OT_PRETTY_CLASS())("last indexed for ")(key.asHex())(
            " found in database")
            .Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG

        return it->second;
    }

    const auto error = UnallocatedCString{"last indexed for "} + key.asHex() +
                       " not found in database";

    throw std::out_of_range{error};
}

auto SubchainCache::load_last_scanned(const SubchainIndex& key) noexcept(false)
    -> const db::Position&
{
    auto& map = last_scanned_;
    auto it = map.find(key);

    if (map.end() != it) { return it->second; }

#if defined OPENTXS_DETAILED_DEBUG
    LogTrace()(OT_PRETTY_CLASS())("cache miss, loading last scanned for ")(
        key.asHex())(" from database")
        .Flush();
    const auto start = Clock::now();
#endif  // defined OPENTXS_DETAILED_DEBUG
    lmdb_.Load(wallet::last_scanned_, key.Bytes(), [&](const auto bytes) {
        auto [i, added] = map.try_emplace(key, bytes);

        if (false == added) {
            throw std::runtime_error{"Failed to update cache"};
        }

        it = i;
    });
#if defined OPENTXS_DETAILED_DEBUG
    const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
        Clock::now() - start);
    LogTrace()(OT_PRETTY_CLASS())("database query finished in ")(
        elapsed.count())(" microseconds")
        .Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG

    if (map.end() != it) {
#if defined OPENTXS_DETAILED_DEBUG
        LogTrace()(OT_PRETTY_CLASS())("last scanned for ")(key.asHex())(
            " found in database")
            .Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG

        return it->second;
    }

    const auto error = UnallocatedCString{"last scanned for "} + key.asHex() +
                       " not found in database";

    throw std::out_of_range{error};
}

auto SubchainCache::load_match_index(const ReadView key) noexcept
    -> const dbPatternIndex&
{
    auto& map = match_index_;
    const auto hash = [&] {
        auto out = api_.Factory().Data();
        out->Assign(key);

        return out;
    }();

    if (auto it = map.find(hash); map.end() != it) { return it->second; }

#if defined OPENTXS_DETAILED_DEBUG
    LogTrace()(OT_PRETTY_CLASS())("cache miss, loading match index for ")(
        hash->asHex())(" from database")
        .Flush();
    const auto start = Clock::now();
#endif  // defined OPENTXS_DETAILED_DEBUG
    auto& index = map[hash];
    lmdb_.Load(
        wallet::match_index_,
        key,
        [&](const auto bytes) {
            index.emplace([&] {
                auto out = api_.Factory().Identifier();
                out->Assign(bytes);

                return out;
            }());
        },
        Mode::Multiple);
#if defined OPENTXS_DETAILED_DEBUG
    const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
        Clock::now() - start);
    LogTrace()(OT_PRETTY_CLASS())("database query finished in ")(
        elapsed.count())(" microseconds")
        .Flush();
    LogTrace()(OT_PRETTY_CLASS())(index.size())(" items for match index")(
        hash->asHex())(" found in database")
        .Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG

    return index;
}

auto SubchainCache::load_pattern(const PatternID& key) noexcept
    -> const dbPatterns&
{
    auto& map = patterns_;

    if (auto it = map.find(key); map.end() != it) { return it->second; }

#if defined OPENTXS_DETAILED_DEBUG
    LogTrace()(OT_PRETTY_CLASS())("cache miss, loading patterns for ")(
        key.asHex())(" from database")
        .Flush();
    const auto start = Clock::now();
#endif  // defined OPENTXS_DETAILED_DEBUG
    auto& patterns = map[key];
    lmdb_.Load(
        wallet::patterns_,
        key.Bytes(),
        [&](const auto bytes) { patterns.emplace(bytes); },
        Mode::Multiple);
#if defined OPENTXS_DETAILED_DEBUG
    const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
        Clock::now() - start);
    LogTrace()(OT_PRETTY_CLASS())("database query finished in ")(
        elapsed.count())(" microseconds")
        .Flush();
    LogTrace()(OT_PRETTY_CLASS())(patterns.size())(" items for pattern ")(
        key.asHex())(" found in database")
        .Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG

    return patterns;
}

auto SubchainCache::load_pattern_index(const SubchainIndex& key) noexcept
    -> const dbPatternIndex&
{
    auto& map = pattern_index_;

    if (auto it = map.find(key); map.end() != it) { return it->second; }

#if defined OPENTXS_DETAILED_DEBUG
    LogTrace()(OT_PRETTY_CLASS())("cache miss, loading pattern index for ")(
        key.asHex())(" from database")
        .Flush();
    const auto start = Clock::now();
#endif  // defined OPENTXS_DETAILED_DEBUG
    auto& index = map[key];
    lmdb_.Load(
        wallet::pattern_index_,
        key.Bytes(),
        [&](const auto bytes) {
            index.emplace([&] {
                auto out = api_.Factory().Identifier();
                out->Assign(bytes);

                return out;
            }());
        },
        Mode::Multiple);
#if defined OPENTXS_DETAILED_DEBUG
    const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
        Clock::now() - start);
    LogTrace()(OT_PRETTY_CLASS())("database query finished in ")(
        elapsed.count())(" microseconds")
        .Flush();
    LogTrace()(OT_PRETTY_CLASS())(index.size())(" items for pattern index")(
        key.asHex())(" found in database")
        .Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG

    return index;
}

auto SubchainCache::SetLastIndexed(
    const eLock&,
    const SubchainIndex& subchain,
    const Bip32Index value,
    MDB_txn* tx) noexcept -> bool
{
    try {
        auto& indexed = last_indexed_[subchain];
        indexed = value;
        const auto output =
            lmdb_
                .Store(
                    wallet::last_indexed_, subchain.Bytes(), tsv(indexed), tx)
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
    const eLock&,
    const SubchainIndex& subchain,
    const block::Position& value,
    MDB_txn* tx) noexcept -> bool
{
    try {
        const auto& scanned = [&]() -> auto&
        {
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
    const Subchain subchain,
    const filter::Type type,
    const VersionNumber version) const noexcept -> pSubchainIndex
{
    auto preimage = api_.Factory().Data();
    preimage->Assign(subaccount);
    preimage->Concatenate(&subchain, sizeof(subchain));
    preimage->Concatenate(&type, sizeof(type));
    preimage->Concatenate(&version, sizeof(version));
    auto output = api_.Factory().Identifier();
    output->CalculateDigest(preimage->Bytes());

    return output;
}

SubchainCache::~SubchainCache() = default;
}  // namespace opentxs::blockchain::database::wallet
