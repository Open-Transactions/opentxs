// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/container/flat_set.hpp>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <optional>
#include <utility>
#include <variant>

#include "Proto.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/blockchain/database/Types.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Input.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"
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
namespace bitcoin
{
namespace block
{
class Header;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Header;
class Position;
}  // namespace block

namespace database
{
namespace common
{
class Database;
}  // namespace common
}  // namespace database

namespace node
{
class Manager;
class UpdateTransaction;
}  // namespace node
}  // namespace blockchain
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::database
{
struct Headers {
public:
    auto BestBlock(const block::Height position) const noexcept(false)
        -> block::Hash;
    auto CurrentBest() const noexcept -> std::unique_ptr<block::Header>
    {
        return load_header(best().hash_);
    }
    auto CurrentCheckpoint() const noexcept -> block::Position;
    auto DisconnectedHashes() const noexcept -> database::DisconnectedList;
    auto HasDisconnectedChildren(const block::Hash& hash) const noexcept
        -> bool;
    auto HaveCheckpoint() const noexcept -> bool;
    auto HeaderExists(const block::Hash& hash) const noexcept -> bool;
    auto import_genesis(const blockchain::Type type) const noexcept -> void;
    auto IsSibling(const block::Hash& hash) const noexcept -> bool;
    // Throws std::out_of_range if the header does not exist
    auto LoadHeader(const block::Hash& hash) const
        -> std::unique_ptr<block::Header>
    {
        return load_header(hash);
    }
    auto RecentHashes(alloc::Default alloc) const noexcept
        -> Vector<block::Hash>;
    auto SiblingHashes() const noexcept -> database::Hashes;
    // Returns null pointer if the header does not exist
    auto TryLoadBitcoinHeader(const block::Hash& hash) const noexcept
        -> std::unique_ptr<bitcoin::block::Header>;
    // Returns null pointer if the header does not exist
    auto TryLoadHeader(const block::Hash& hash) const noexcept
        -> std::unique_ptr<block::Header>;

    auto ApplyUpdate(const node::UpdateTransaction& update) noexcept -> bool;
    auto ReportTip() noexcept -> void;

    Headers(
        const api::Session& api,
        const node::Manager& network,
        const common::Database& common,
        const storage::lmdb::LMDB& lmdb,
        const blockchain::Type type) noexcept;

private:
    using TipData = block::Position;
    using ReorgData = std::pair<block::Position, block::Position>;
    using LastUpdate = std::variant<std::monostate, TipData, ReorgData>;

    class IsSameReorg;
    class IsSameTip;

    const api::Session& api_;
    const common::Database& common_;
    const storage::lmdb::LMDB& lmdb_;
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
    auto load_bitcoin_header(const block::Hash& hash) const noexcept(false)
        -> std::unique_ptr<bitcoin::block::Header>;
    // Throws std::out_of_range if the header does not exist
    auto load_header(const block::Hash& hash) const noexcept(false)
        -> std::unique_ptr<block::Header>;
    auto pop_best(const std::size_t i, MDB_txn* parent) const noexcept -> bool;
    auto push_best(
        const block::Position next,
        const bool setTip,
        MDB_txn* parent) const noexcept -> bool;
    auto recent_hashes(const Lock& lock, alloc::Default alloc) const noexcept
        -> Vector<block::Hash>;

    auto report(const Lock&) noexcept -> void;
    auto report(const Lock&, const block::Position& tip) noexcept -> void;
};
}  // namespace opentxs::blockchain::database
