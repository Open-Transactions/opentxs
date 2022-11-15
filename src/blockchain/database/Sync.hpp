// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#pragma once

#include <boost/container/flat_set.hpp>
#include <algorithm>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <optional>
#include <utility>

#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/blockchain/database/Types.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Input.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/network/otdht/Block.hpp"
#include "opentxs/network/otdht/Types.hpp"
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
namespace common
{
class Database;
}  // namespace common
}  // namespace database
}  // namespace blockchain

namespace network
{
namespace otdht
{
class Block;
class Data;
}  // namespace otdht
}  // namespace network

namespace storage
{
namespace lmdb
{
class Database;
}  // namespace lmdb
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::database::implementation
{
class Sync
{
public:
    using Message = network::otdht::Data;

    auto Load(const block::Height height, Message& output) const noexcept
        -> bool;
    auto Reorg(const block::Height height) const noexcept -> bool;
    auto SetTip(const block::Position& position) const noexcept -> bool;
    auto Store(
        const block::Position& tip,
        const network::otdht::SyncData& items) const noexcept -> bool;
    auto Tip() const noexcept -> block::Position;

    Sync(
        const api::Session& api,
        const common::Database& common,
        const storage::lmdb::Database& lmdb,
        const blockchain::Type type) noexcept;

private:
    const api::Session& api_;
    const common::Database& common_;
    const storage::lmdb::Database& lmdb_;
    const block::Position blank_position_;
    const blockchain::Type chain_;
    const block::Hash genesis_;
};
}  // namespace opentxs::blockchain::database::implementation
