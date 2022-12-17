// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <mutex>
#include <optional>
#include <utility>

#include "internal/blockchain/block/Types.hpp"
#include "internal/blockchain/database/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

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
class SubchainPrivate;
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
class SubchainData
{
public:
    auto GetSubchainID(
        const SubaccountID& subaccount,
        const crypto::Subchain subchain) const noexcept -> SubchainID;
    auto GetSubchainID(
        const SubaccountID& subaccount,
        const crypto::Subchain subchain,
        storage::lmdb::Transaction& tx) const noexcept -> SubchainID;
    auto GetPatterns(const SubchainID& subchain, alloc::Default alloc)
        const noexcept -> Patterns;
    auto Reorg(
        const node::internal::HeaderOraclePrivate& data,
        storage::lmdb::Transaction& tx,
        const node::HeaderOracle& headers,
        const SubchainID& subchain,
        const block::Height lastGoodHeight) const noexcept(false) -> bool;
    auto SubchainAddElements(
        const SubchainID& subchain,
        const ElementMap& elements) const noexcept -> bool;
    auto SubchainLastIndexed(const SubchainID& subchain) const noexcept
        -> std::optional<Bip32Index>;
    auto SubchainLastScanned(const SubchainID& subchain) const noexcept
        -> block::Position;
    auto SubchainSetLastScanned(
        const SubchainID& subchain,
        const block::Position& position) const noexcept -> bool;

    SubchainData(
        const api::Session& api,
        const storage::lmdb::Database& lmdb,
        const blockchain::cfilter::Type filter) noexcept;
    SubchainData() = delete;
    SubchainData(const SubchainData&) = delete;
    auto operator=(const SubchainData&) -> SubchainData& = delete;
    auto operator=(SubchainData&&) -> SubchainData& = delete;

    ~SubchainData();

private:
    std::unique_ptr<SubchainPrivate> imp_;
};
}  // namespace opentxs::blockchain::database::wallet
