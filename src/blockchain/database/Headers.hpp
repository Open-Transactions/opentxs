// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <mutex>
#include <utility>
#include <variant>

#include "internal/blockchain/database/Types.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
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
namespace database
{
namespace common
{
class Database;
}  // namespace common
}  // namespace database

namespace node
{
class UpdateTransaction;
struct Endpoints;
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

namespace opentxs::blockchain::database
{
struct Headers {
public:
    auto BestBlock(const block::Height position) const noexcept(false)
        -> block::Hash;
    auto CurrentBest() const noexcept -> block::Header;
    auto CurrentCheckpoint() const noexcept -> block::Position;
    auto DisconnectedHashes() const noexcept -> database::DisconnectedList;
    auto HasDisconnectedChildren(const block::Hash& hash) const noexcept
        -> bool;
    auto HaveCheckpoint() const noexcept -> bool;
    auto HeaderExists(const block::Hash& hash) const noexcept -> bool;
    auto import_genesis(const blockchain::Type type) const noexcept -> void;
    auto IsSibling(const block::Hash& hash) const noexcept -> bool;
    // Throws std::out_of_range if the header does not exist
    auto LoadHeader(const block::Hash& hash) const -> block::Header
    {
        return load_header(hash);
    }
    auto RecentHashes(alloc::Default alloc) const noexcept
        -> Vector<block::Hash>;
    auto SiblingHashes() const noexcept -> database::Hashes;
    auto TryLoadHeader(const block::Hash& hash) const noexcept -> block::Header;

    auto ApplyUpdate(const node::UpdateTransaction& update) noexcept -> bool;
    auto ReportTip() noexcept -> void;

    Headers(
        const api::Session& api,
        const node::Endpoints& endpoints,
        const common::Database& common,
        const storage::lmdb::Database& lmdb,
        const blockchain::Type type) noexcept;

private:
    using TipData = block::Position;
    using ReorgData = std::pair<block::Position, block::Position>;
    using LastUpdate = std::variant<std::monostate, TipData, ReorgData>;

    class IsSameReorg;
    class IsSameTip;

    const api::Session& api_;
    const common::Database& common_;
    const storage::lmdb::Database& lmdb_;
    const blockchain::Type chain_;
    mutable std::mutex lock_;
    network::zeromq::socket::Raw publish_tip_internal_;
    network::zeromq::socket::Raw to_blockchain_api_;
    LastUpdate last_update_;

    auto best() const noexcept -> block::Position;
    auto best(const Lock& lock) const noexcept -> block::Position;
    auto checkpoint(const Lock& lock) const noexcept -> block::Position;
    auto header_exists(const Lock& lock, const block::Hash& hash) const noexcept
        -> bool;
    // Throws std::out_of_range if the header does not exist
    auto load_header(const block::Hash& hash) const noexcept(false)
        -> block::Header;
    auto pop_best(block::Height i, storage::lmdb::Transaction& parent)
        const noexcept -> bool;
    auto push_best(
        const block::Position next,
        const bool setTip,
        storage::lmdb::Transaction& parent) const noexcept -> bool;
    auto recent_hashes(const Lock& lock, alloc::Default alloc) const noexcept
        -> Vector<block::Hash>;

    auto report(const Lock&) noexcept -> void;
    auto report(const Lock&, const block::Position& tip) noexcept -> void;
};
}  // namespace opentxs::blockchain::database
