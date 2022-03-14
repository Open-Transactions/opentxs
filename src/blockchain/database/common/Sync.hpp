// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/thread/thread.hpp>
#include <array>
#include <cstdint>
#include <cstring>
#include <iosfwd>
#include <iterator>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>

#include "internal/blockchain/node/Node.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/p2p/Block.hpp"
#include "opentxs/network/p2p/Data.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
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
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::database::common
{
class Sync
{
public:
    using Chain = opentxs::blockchain::Type;
    using Height = opentxs::blockchain::block::Height;
    using Block = opentxs::network::p2p::Block;
    using Message = opentxs::network::p2p::Data;
    using Items = UnallocatedVector<Block>;

    auto Load(const Chain chain, const Height height, Message& output)
        const noexcept -> bool;
    // Delete all entries with a height greater than specified
    auto Reorg(const Chain chain, const Height height) const noexcept -> bool;
    auto Store(const Chain chain, const Items& items) const noexcept -> bool;
    auto Tip(const Chain chain) const noexcept -> Height;

    Sync(
        const api::Session& api,
        storage::lmdb::LMDB& lmdb,
        const UnallocatedCString& path) noexcept(false);

    ~Sync();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};
}  // namespace opentxs::blockchain::database::common
