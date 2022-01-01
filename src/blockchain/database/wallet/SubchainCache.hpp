// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <robin_hood.h>
#include <cstddef>
#include <map>
#include <mutex>
#include <optional>
#include <set>

#include "blockchain/database/wallet/Position.hpp"
#include "blockchain/database/wallet/SubchainID.hpp"
#include "blockchain/database/wallet/Types.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Numbers.hpp"
#include "util/LMDB.hpp"

namespace opentxs
{
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
}  // namespace opentxs

namespace opentxs::blockchain::database::wallet
{
template <typename Input>
auto tsv(const Input& in) noexcept -> ReadView
{
    return {reinterpret_cast<const char*>(&in), sizeof(in)};
}

using Mode = storage::lmdb::LMDB::Mode;

constexpr auto config_{Table::Config};
constexpr auto last_indexed_{Table::SubchainLastIndexed};
constexpr auto last_scanned_{Table::SubchainLastScanned};
constexpr auto id_index_{Table::SubchainID};
constexpr auto patterns_{Table::WalletPatterns};
constexpr auto pattern_index_{Table::SubchainPatterns};
constexpr auto match_index_{Table::SubchainMatches};

class SubchainCache
{
public:
    using dbPatterns = robin_hood::unordered_node_set<db::Pattern>;
    using dbPatternIndex = std::pmr::set<pPatternID>;

    auto AddMatch(
        const eLock&,
        const ReadView key,
        const PatternID& value,
        MDB_txn* tx) noexcept -> bool;
    auto AddPattern(
        const eLock&,
        const PatternID& id,
        const Bip32Index index,
        const ReadView data,
        MDB_txn* tx) noexcept -> bool;
    auto AddPatternIndex(
        const eLock&,
        const SubchainIndex& key,
        const PatternID& value,
        MDB_txn* tx) noexcept -> bool;
    auto Clear(const eLock&) noexcept -> void;
    auto DecodeIndex(const sLock&, const SubchainIndex& key) noexcept(false)
        -> const db::SubchainID&;
    auto GetIndex(
        const sLock&,
        const NodeID& subaccount,
        const Subchain subchain,
        const filter::Type type,
        const VersionNumber version,
        MDB_txn* tx) noexcept -> pSubchainIndex;
    auto GetLastIndexed(const sLock&, const SubchainIndex& subchain) noexcept
        -> std::optional<Bip32Index>;
    auto GetLastScanned(const sLock&, const SubchainIndex& subchain) noexcept
        -> block::Position;
    auto GetLastScanned(const eLock&, const SubchainIndex& subchain) noexcept
        -> block::Position;
    auto GetMatchIndex(const sLock&, const ReadView id) noexcept
        -> const dbPatternIndex&;
    auto GetPattern(const sLock&, const PatternID& id) noexcept
        -> const dbPatterns&;
    auto GetPatternIndex(const sLock&, const SubchainIndex& id) noexcept
        -> const dbPatternIndex&;
    auto SetLastIndexed(
        const eLock&,
        const SubchainIndex& subchain,
        const Bip32Index value,
        MDB_txn* tx) noexcept -> bool;
    auto SetLastScanned(
        const eLock&,
        const SubchainIndex& subchain,
        const block::Position& value,
        MDB_txn* tx) noexcept -> bool;

    SubchainCache(
        const api::Session& api,
        const storage::lmdb::LMDB& lmdb) noexcept;

    ~SubchainCache();

private:
    static constexpr auto reserve_ = std::size_t{1000u};

    // NOTE if an exclusive lock is being held in the parent then locking
    // these mutexes is redundant
    const api::Session& api_;
    const storage::lmdb::LMDB& lmdb_;
    std::mutex id_lock_;
    robin_hood::unordered_node_map<pSubchainIndex, db::SubchainID> subchain_id_;
    std::mutex index_lock_;
    robin_hood::unordered_flat_map<pSubchainIndex, Bip32Index> last_indexed_;
    std::mutex scanned_lock_;
    robin_hood::unordered_node_map<pSubchainIndex, db::Position> last_scanned_;
    std::mutex pattern_lock_;
    robin_hood::unordered_node_map<pPatternID, dbPatterns> patterns_;
    std::mutex pattern_index_lock_;
    robin_hood::unordered_node_map<pSubchainIndex, dbPatternIndex>
        pattern_index_;
    std::mutex match_index_lock_;
    robin_hood::unordered_node_map<block::pHash, dbPatternIndex> match_index_;

    auto subchain_index(
        const NodeID& subaccount,
        const Subchain subchain,
        const filter::Type type,
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
