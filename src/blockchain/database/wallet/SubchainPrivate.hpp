// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_shared_guarded.h>
#include <future>
#include <optional>
#include <shared_mutex>

#include "blockchain/database/wallet/SubchainCache.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "internal/blockchain/database/Types.hpp"
#include "internal/blockchain/database/Wallet.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Allocator.hpp"
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
class Position;
}  // namespace block

namespace database
{
namespace wallet
{
namespace db
{
class SubchainID;
}  // namespace db
}  // namespace wallet
}  // namespace database

namespace node
{
namespace internal
{
struct HeaderOraclePrivate;
}  // namespace internal

class HeaderOracle;
}  // namespace node
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
class SubchainPrivate
{
public:
    auto GetID(const SubaccountID& subaccount, const crypto::Subchain subchain)
        const noexcept -> SubchainID;
    auto GetID(
        const SubaccountID& subaccount,
        const crypto::Subchain subchain,
        storage::lmdb::Transaction& tx) const noexcept -> SubchainID;
    auto GetLastIndexed(const SubchainID& subchain) const noexcept
        -> std::optional<Bip32Index>;
    auto GetLastScanned(const SubchainID& subchain) const noexcept
        -> block::Position;
    auto GetPatterns(const SubchainID& id, alloc::Default alloc) const noexcept
        -> Patterns;

    auto AddElements(
        const SubchainID& subchain,
        const ElementMap& elements) noexcept -> bool;
    auto Reorg(
        const node::internal::HeaderOraclePrivate& data,
        const node::HeaderOracle& headers,
        const SubchainID& subchain,
        const block::Height lastGoodHeight,
        storage::lmdb::Transaction& tx) noexcept(false) -> bool;
    auto SetLastScanned(
        const SubchainID& subchain,
        const block::Position& position) noexcept -> bool;

    SubchainPrivate(
        const api::Session& api,
        const storage::lmdb::Database& lmdb,
        const blockchain::cfilter::Type filter) noexcept;
    SubchainPrivate() = delete;
    SubchainPrivate(const SubchainPrivate&) = delete;
    auto operator=(const SubchainPrivate&) -> SubchainPrivate& = delete;
    auto operator=(SubchainPrivate&&) -> SubchainPrivate& = delete;

    ~SubchainPrivate();

private:
    using GuardedCache =
        libguarded::shared_guarded<SubchainCache, std::shared_mutex>;

    const api::Session& api_;
    const storage::lmdb::Database& lmdb_;
    const cfilter::Type default_filter_type_;
    const VersionNumber current_version_;
    std::promise<void> upgrade_promise_;
    std::shared_future<void> upgrade_future_;
    GuardedCache cache_;

    auto get_patterns(
        const SubchainID& id,
        const SubchainCache& cache,
        alloc::Default alloc) const noexcept(false) -> Patterns;
    auto get_id(
        const SubaccountID& subaccount,
        const crypto::Subchain subchain,
        const SubchainCache& cache,
        storage::lmdb::Transaction& tx) const noexcept -> SubchainID;
    auto pattern_id(const SubchainID& subchain, const Bip32Index index)
        const noexcept -> ElementID;
    auto subchain_index(
        const SubaccountID& subaccount,
        const crypto::Subchain subchain,
        const cfilter::Type type,
        const VersionNumber version) const noexcept -> SubchainID;

    auto add_elements(
        const SubchainID& subchain,
        const ElementMap& elements,
        SubchainCache& cache,
        storage::lmdb::Transaction& tx) noexcept -> bool;
    auto reorg(
        const node::internal::HeaderOraclePrivate& data,
        const node::HeaderOracle& headers,
        const SubchainID& subchain,
        const block::Height lastGoodHeight,
        SubchainCache& cache,
        storage::lmdb::Transaction& tx) noexcept(false) -> bool;
    auto set_last_scanned(
        const SubchainID& subchain,
        const block::Position& position,
        SubchainCache& cache,
        storage::lmdb::Transaction& tx) noexcept -> bool;
    auto upgrade() noexcept -> void;
};
}  // namespace opentxs::blockchain::database::wallet
