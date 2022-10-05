// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_plain_guarded.h>
#include <robin_hood.h>
#include <cstddef>
#include <mutex>
#include <optional>

#include "blockchain/database/wallet/Pattern.hpp"
#include "blockchain/database/wallet/Position.hpp"
#include "blockchain/database/wallet/SubchainID.hpp"
#include "blockchain/database/wallet/Types.hpp"
#include "internal/blockchain/database/Types.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/TSV.hpp"
#include "internal/util/storage/lmdb/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
class Hash;
class Position;
}  // namespace block

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

namespace storage
{
namespace lmdb
{
class Database;
class Transaction;
}  // namespace lmdb
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::database::wallet
{
using Mode = storage::lmdb::Mode;

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
    using dbPatternIndex = Set<PatternID>;

    auto DecodeIndex(const SubchainIndex& key) const noexcept(false)
        -> const db::SubchainID&;
    auto GetIndex(
        const NodeID& subaccount,
        const crypto::Subchain subchain,
        const cfilter::Type type,
        const VersionNumber version,
        storage::lmdb::Transaction& tx) const noexcept -> SubchainIndex;
    auto GetLastIndexed(const SubchainIndex& subchain) const noexcept
        -> std::optional<Bip32Index>;
    auto GetLastScanned(const SubchainIndex& subchain) const noexcept
        -> block::Position;
    auto GetPattern(const PatternID& id) const noexcept -> const dbPatterns&;
    auto GetPatternIndex(const SubchainIndex& id) const noexcept
        -> const dbPatternIndex&;

    auto AddPattern(
        const PatternID& id,
        const Bip32Index index,
        const ReadView data,
        storage::lmdb::Transaction& tx) noexcept -> bool;
    auto AddPatternIndex(
        const SubchainIndex& key,
        const PatternID& value,
        storage::lmdb::Transaction& tx) noexcept -> bool;
    auto Clear() noexcept -> void;
    auto SetLastIndexed(
        const SubchainIndex& subchain,
        const Bip32Index value,
        storage::lmdb::Transaction& tx) noexcept -> bool;
    auto SetLastScanned(
        const SubchainIndex& subchain,
        const block::Position& value,
        storage::lmdb::Transaction& tx) noexcept -> bool;

    SubchainCache(
        const api::Session& api,
        const storage::lmdb::Database& lmdb) noexcept;

    ~SubchainCache();

private:
    static constexpr auto reserve_ = 1000_uz;

    using SubchainIDMap =
        robin_hood::unordered_node_map<SubchainIndex, db::SubchainID>;
    using LastIndexedMap =
        robin_hood::unordered_flat_map<SubchainIndex, Bip32Index>;
    using LastScannedMap =
        robin_hood::unordered_node_map<SubchainIndex, db::Position>;
    using PatternsMap = robin_hood::unordered_node_map<PatternID, dbPatterns>;
    using PatternIndexMap =
        robin_hood::unordered_node_map<SubchainIndex, dbPatternIndex>;
    using GuardedSubchainID = libguarded::plain_guarded<SubchainIDMap>;
    using GuardedLastIndexed = libguarded::plain_guarded<LastIndexedMap>;
    using GuardedLastScanned = libguarded::plain_guarded<LastScannedMap>;
    using GuardedPatterns = libguarded::plain_guarded<PatternsMap>;
    using GuardedPatternIndex = libguarded::plain_guarded<PatternIndexMap>;

    const api::Session& api_;
    const storage::lmdb::Database& lmdb_;
    mutable GuardedSubchainID subchain_id_;
    mutable GuardedLastIndexed last_indexed_;
    mutable GuardedLastScanned last_scanned_;
    mutable GuardedPatterns patterns_;
    mutable GuardedPatternIndex pattern_index_;

    auto load_index(const SubchainIndex& key, SubchainIDMap& map) const
        noexcept(false) -> const db::SubchainID&;
    auto load_last_indexed(const SubchainIndex& key, LastIndexedMap& map) const
        noexcept(false) -> const Bip32Index&;
    auto load_last_scanned(const SubchainIndex& key, LastScannedMap& map) const
        noexcept(false) -> const db::Position&;
    auto load_pattern(const PatternID& key, PatternsMap& map) const noexcept
        -> const dbPatterns&;
    auto load_pattern_index(const SubchainIndex& key, PatternIndexMap& map)
        const noexcept -> const dbPatternIndex&;
    auto subchain_index(
        const NodeID& subaccount,
        const crypto::Subchain subchain,
        const cfilter::Type type,
        const VersionNumber version) const noexcept -> SubchainIndex;
};
}  // namespace opentxs::blockchain::database::wallet
