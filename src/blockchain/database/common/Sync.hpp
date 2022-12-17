// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/thread/thread.hpp>
#include <array>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <iosfwd>
#include <iterator>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>

#include "opentxs/api/session/Client.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/network/otdht/Block.hpp"
#include "opentxs/network/otdht/Data.hpp"
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
namespace database
{
namespace common
{
class SyncPrivate;
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

namespace opentxs::blockchain::database::common
{
class Sync
{
public:
    using Chain = blockchain::Type;
    using Height = block::Height;
    using Message = network::otdht::Data;

    auto Load(const Chain chain, const Height height, Message& output)
        const noexcept -> bool;
    // Delete all entries with a height greater than specified
    auto Reorg(const Chain chain, const Height height) const noexcept -> bool;
    auto Store(const network::otdht::SyncData& items, Chain chain)
        const noexcept -> bool;
    auto Tip(const Chain chain) const noexcept -> Height;

    Sync(
        const api::Session& api,
        storage::lmdb::Database& lmdb,
        const std::filesystem::path& path) noexcept(false);

    ~Sync();

private:
    std::unique_ptr<SyncPrivate> imp_;
};
}  // namespace opentxs::blockchain::database::common
