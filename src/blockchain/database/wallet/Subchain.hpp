// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <mutex>
#include <optional>
#include <utility>
#include <vector>

#include "blockchain/database/wallet/Types.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "util/LMDB.hpp"

namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace node
{
class HeaderOracle;
}  // namespace node
}  // namespace blockchain

namespace storage
{
namespace lmdb
{
class LMDB;
}  // namespace lmdb
}  // namespace storage
}  // namespace opentxs

extern "C" {
typedef struct MDB_txn MDB_txn;
}

namespace opentxs::blockchain::database::wallet
{
class SubchainData
{
public:
    auto GetSubchainID(
        const NodeID& subaccount,
        const Subchain subchain,
        MDB_txn* tx) const noexcept -> pSubchainIndex;
    auto GetPatterns(const SubchainIndex& subchain) const noexcept -> Patterns;
    auto GetUntestedPatterns(
        const SubchainIndex& subchain,
        const ReadView blockID) const noexcept -> Patterns;
    auto Reorg(
        const Lock& headerOracleLock,
        MDB_txn* tx,
        const node::HeaderOracle& headers,
        const SubchainIndex& subchain,
        const block::Height lastGoodHeight) const noexcept(false) -> bool;
    auto SubchainAddElements(
        const SubchainIndex& subchain,
        const ElementMap& elements) const noexcept -> bool;
    auto SubchainLastIndexed(const SubchainIndex& subchain) const noexcept
        -> std::optional<Bip32Index>;
    auto SubchainLastScanned(const SubchainIndex& subchain) const noexcept
        -> block::Position;
    auto SubchainMatchBlock(
        const SubchainIndex& index,
        const std::pmr::vector<std::pair<ReadView, MatchingIndices>>& results)
        const noexcept -> bool;
    auto SubchainSetLastScanned(
        const SubchainIndex& subchain,
        const block::Position& position) const noexcept -> bool;

    SubchainData(
        const api::Session& api,
        const storage::lmdb::LMDB& lmdb,
        const blockchain::filter::Type filter) noexcept;

    ~SubchainData();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;

    SubchainData() = delete;
    SubchainData(const SubchainData&) = delete;
    auto operator=(const SubchainData&) -> SubchainData& = delete;
    auto operator=(SubchainData&&) -> SubchainData& = delete;
};
}  // namespace opentxs::blockchain::database::wallet
