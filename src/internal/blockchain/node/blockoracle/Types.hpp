// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/protocol/bitcoin/base/block/Block.hpp"
// IWYU pragma: no_include "opentxs/blockchain/block/Hash.hpp"

#pragma once

#include <cstddef>
#include <future>
#include <memory>
#include <span>
#include <string_view>
#include <tuple>
#include <utility>
#include <variant>

#include "internal/blockchain/node/Job.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/WorkType.hpp"  // IWYU pragma: keep
#include "opentxs/WorkType.internal.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class Block;
class Hash;
}  // namespace block
}  // namespace blockchain

namespace network
{
namespace zeromq
{
class Frame;
}  // namespace zeromq
}  // namespace network

class ByteArray;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::blockoracle
{
// WARNING update print function if new values are added or removed
enum class Job : OTZMQWorkType {
    shutdown = value(WorkType::Shutdown),
    header = value(WorkType::BlockchainNewHeader),
    reorg = value(WorkType::BlockchainReorg),
    request_blocks = OT_ZMQ_INTERNAL_SIGNAL + 0u,
    submit_block = OT_ZMQ_INTERNAL_SIGNAL + 1u,
    block_ready = OT_ZMQ_BLOCK_ORACLE_BLOCK_READY,
    report = OT_ZMQ_BLOCKCHAIN_REPORT_STATUS,
    init = OT_ZMQ_INIT_SIGNAL,
    statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};  // IWYU pragma: export

auto print(Job) noexcept -> std::string_view;

using download::JobID;
using Promise = std::promise<block::Block>;
using Future = std::shared_future<block::Block>;
using Task = std::pair<Promise, Future>;
using Requests = Map<block::Hash, Task>;
using Index = Set<block::Hash>;
using Hashes = std::span<const block::Hash>;
using Work =
    std::tuple<download::JobID, Vector<block::Hash>, std::size_t, std::size_t>;
using MissingBlock = std::monostate;
using PersistentBlock = ReadView;
using CachedBlock = std::shared_ptr<const ByteArray>;
using BlockLocation = std::variant<MissingBlock, PersistentBlock, CachedBlock>;
using QueueData = std::pair<std::size_t, std::size_t>;

[[nodiscard]] auto is_valid(const BlockLocation&) noexcept -> bool;
[[nodiscard]] auto reader(
    const BlockLocation& block,
    alloc::Default monotonic) noexcept -> ReadView;
[[nodiscard]] auto parse_block_location(
    const network::zeromq::Frame& frame) noexcept -> BlockLocation;
[[nodiscard]] auto serialize(const BlockLocation& bytes, Writer&& out) noexcept
    -> bool;
}  // namespace opentxs::blockchain::node::blockoracle
