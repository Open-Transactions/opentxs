// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <cstddef>
#include <memory>
#include <string_view>
#include <utility>

#include "internal/blockchain/node/Job.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
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
class Block;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Hash;
}  // namespace block

namespace database
{
class Block;
}  // namespace database

namespace node
{
namespace internal
{
class BlockBatch;
struct Config;
}  // namespace internal

class Manager;
}  // namespace node
}  // namespace blockchain

namespace network
{
namespace zeromq
{
namespace socket
{
class Raw;
}  // namespace socket

class Frame;
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::blockoracle
{
class Cache final : public Allocated
{
public:
    class Actor;
    class Shared;

    using BatchID = download::JobID;

    auto DownloadQueue() const noexcept -> std::size_t;
    auto get_allocator() const noexcept -> allocator_type final;

    auto GetBlockBatch(alloc::Default alloc) noexcept
        -> node::internal::BlockBatch;
    auto ReceiveBlock(std::shared_ptr<const bitcoin::block::Block> in) noexcept
        -> bool;
    auto Request(const block::Hash& block) noexcept -> BitcoinBlockResult;
    auto Request(const Vector<block::Hash>& hashes) noexcept
        -> BitcoinBlockResults;
    auto Start(
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<const node::Manager> node) noexcept -> void;

    Cache(const api::Session& api, const node::Manager& node) noexcept;

    ~Cache() final;

private:
    // TODO switch to std::shared_ptr once the android ndk ships a version of
    // libc++ with unfucked pmr / allocate_shared support
    boost::shared_ptr<Shared> shared_;
};
}  // namespace opentxs::blockchain::node::blockoracle
