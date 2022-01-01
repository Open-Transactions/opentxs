// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/container/flat_set.hpp>
#include <algorithm>
#include <cstdint>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "Proto.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/network/p2p/Block.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Pimpl.hpp"
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
namespace common
{
class Database;
}  // namespace common
}  // namespace database
}  // namespace blockchain

namespace network
{
namespace p2p
{
class Block;
class Data;
}  // namespace p2p
}  // namespace network

namespace storage
{
namespace lmdb
{
class LMDB;
}  // namespace lmdb
}  // namespace storage
}  // namespace opentxs

namespace opentxs::blockchain::database
{
class Sync
{
public:
    using Items = std::pmr::vector<network::p2p::Block>;
    using Message = network::p2p::Data;

    auto Load(const block::Height height, Message& output) const noexcept
        -> bool;
    auto Reorg(const block::Height height) const noexcept -> bool;
    auto SetTip(const block::Position& position) const noexcept -> bool;
    auto Store(const block::Position& tip, const Items& items) const noexcept
        -> bool;
    auto Tip() const noexcept -> block::Position;

    Sync(
        const api::Session& api,
        const common::Database& common,
        const storage::lmdb::LMDB& lmdb,
        const blockchain::Type type) noexcept;

private:
    const api::Session& api_;
    const common::Database& common_;
    const storage::lmdb::LMDB& lmdb_;
    const block::Position blank_position_;
    const blockchain::Type chain_;
    const block::pHash genesis_;
};
}  // namespace opentxs::blockchain::database
