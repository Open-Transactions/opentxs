// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/thread.hpp>
#include <robin_hood.h>
#include <cstddef>
#include <mutex>
#include <optional>

#include "blockchain/database/wallet/Pattern.hpp"
#include "blockchain/database/wallet/Position.hpp"
#include "blockchain/database/wallet/SubchainID.hpp"
#include "blockchain/database/wallet/Types.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/util/TSV.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "util/LMDB.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace database
{
namespace wallet
{
namespace db
{
class SubchainID;
struct Pattern;
struct Position;
}  // namespace db
}  // namespace wallet
}  // namespace database
}  // namespace blockchain
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::database::wallet
{
using Mode = storage::lmdb::LMDB::Mode;

constexpr auto id_index_{Table::SubchainID};
constexpr auto last_indexed_{Table::SubchainLastIndexed};
constexpr auto last_scanned_{Table::SubchainLastScanned};
constexpr auto match_index_{Table::SubchainMatches};
constexpr auto pattern_index_{Table::SubchainPatterns};
constexpr auto patterns_{Table::WalletPatterns};
constexpr auto subchain_config_{Table::Config};

class SubchainCache
{
public:
    using dbPatterns = robin_hood::unordered_node_set<db::Pattern>;
    using dbPatternIndex = Set<pPatternID>;

    auto AddMatch(
        const ReadView key,
        const PatternID& value,
        MDB_txn* tx) noexcept -> bool;
    auto AddPattern(
        const PatternID& id,
        const Bip32Index index,
        const ReadView data,
        MDB_txn* tx) noexcept -> bool;
    auto AddPatternIndex(
        const SubchainIndex& key,
        const PatternID& value,
        MDB_txn* tx) noexcept -> bool;
    auto Clear() noexcept -> void;
    auto DecodeIndex(const SubchainIndex& key) noexcept(false)
        -> const db::SubchainID&;
    auto GetIndex(
        const NodeID& subaccount,
        const Subchain subchain,
        const cfilter::Type type,
        const VersionNumber version,
        MDB_txn* tx) noexcept -> pSubchainIndex;
    auto GetLastIndexed(const SubchainIndex& subchain) noexcept
        -> std::optional<Bip32Index>;
    auto GetLastScanned(const SubchainIndex& subchain) noexcept
        -> block::Position;
    auto GetMatchIndex(const ReadView id) noexcept -> const dbPatternIndex&;
    auto GetPattern(const PatternID& id) noexcept -> const dbPatterns&;
    auto GetPatternIndex(const SubchainIndex& id) noexcept
        -> const dbPatternIndex&;
    auto SetLastIndexed(
        const SubchainIndex& subchain,
        const Bip32Index value,
        MDB_txn* tx) noexcept -> bool;
    auto SetLastScanned(
        const SubchainIndex& subchain,
        const block::Position& value,
        MDB_txn* tx) noexcept -> bool;

    SubchainCache(
        const api::Session& api,
        const storage::lmdb::LMDB& lmdb) noexcept;

    ~SubchainCache();

private:
    static constexpr auto reserve_ = std::size_t{1000u};

    using SubchainIDMap =
        robin_hood::unordered_node_map<pSubchainIndex, db::SubchainID>;
    using LastIndexedMap =
        robin_hood::unordered_flat_map<pSubchainIndex, Bip32Index>;
    using LastScannedMap =
        robin_hood::unordered_node_map<pSubchainIndex, db::Position>;
    using PatternsMap = robin_hood::unordered_node_map<pPatternID, dbPatterns>;
    using PatternIndexMap =
        robin_hood::unordered_node_map<pSubchainIndex, dbPatternIndex>;
    using MatchIndexMap =
        robin_hood::unordered_node_map<block::pHash, dbPatternIndex>;
    using Mutex = boost::upgrade_mutex;

    const api::Session& api_;
    const storage::lmdb::LMDB& lmdb_;
    Mutex subchain_id_lock_;
    SubchainIDMap subchain_id_;
    Mutex last_indexed_lock_;
    LastIndexedMap last_indexed_;
    Mutex last_scanned_lock_;
    LastScannedMap last_scanned_;
    Mutex patterns_lock_;
    PatternsMap patterns_;
    Mutex pattern_index_lock_;
    PatternIndexMap pattern_index_;
    Mutex match_index_lock_;
    MatchIndexMap match_index_;

    auto subchain_index(
        const NodeID& subaccount,
        const Subchain subchain,
        const cfilter::Type type,
        const VersionNumber version) const noexcept -> pSubchainIndex;

    auto load_index(const SubchainIndex& key) noexcept(false)
        -> const db::SubchainID&;
    auto load_last_indexed(const SubchainIndex& key) noexcept(false)
        -> const Bip32Index&;
    auto load_last_scanned(const SubchainIndex& key) noexcept(false)
        -> const db::Position&;
    auto load_match_index(const ReadView key) noexcept -> const dbPatternIndex&;
    auto load_pattern(const PatternID& key) noexcept -> const dbPatterns&;
    auto load_pattern_index(const SubchainIndex& key) noexcept
        -> const dbPatternIndex&;
};
}  // namespace opentxs::blockchain::database::wallet
