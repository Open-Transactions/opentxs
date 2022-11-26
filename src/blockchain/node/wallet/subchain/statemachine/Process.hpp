// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/blockchain/node/wallet/subchain/statemachine/Process.hpp"

#include <boost/smart_ptr/shared_ptr.hpp>
#include <robin_hood.h>
#include <atomic>
#include <cstddef>
#include <memory>
#include <queue>

#include "blockchain/node/wallet/subchain/statemachine/Job.hpp"
#include "internal/blockchain/node/wallet/Reorg.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/BlockOracle.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Actor.hpp"
#include "util/JobCounter.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
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
class Position;
}  // namespace block

namespace node
{
namespace internal
{
struct HeaderOraclePrivate;
}  // namespace internal

namespace wallet
{
class SubchainStateData;
}  // namespace wallet

class HeaderOracle;
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
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class Process::Imp final : public statemachine::Job
{
public:
    Imp(const boost::shared_ptr<const SubchainStateData>& parent,
        const network::zeromq::BatchID batch,
        allocator_type alloc) noexcept;
    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;

    ~Imp() final = default;

private:
    using Waiting = Deque<block::Position>;
    using Downloading = Set<block::Position>;
    using DownloadIndex = Map<block::Hash, Downloading::iterator>;
    using Ready =
        Map<block::Position, std::shared_ptr<const bitcoin::block::Block>>;
    using Blocks = Vector<block::Position>;

    const std::size_t download_limit_;
    network::zeromq::socket::Raw& to_index_;
    network::zeromq::socket::Raw& to_block_oracle_;
    Waiting waiting_;
    Downloading downloading_;
    DownloadIndex downloading_index_;
    Ready ready_;
    Ready processing_;
    robin_hood::unordered_flat_set<block::pTxid> txid_cache_;
    JobCounter counter_;
    Outstanding running_;

    auto active() const noexcept -> std::size_t;
    auto have_items() const noexcept -> bool;

    auto check_cache() noexcept -> void;
    auto check_process() noexcept -> bool;
    auto do_process(
        const Ready::value_type& data,
        allocator_type monotonic) noexcept -> void;
    auto do_process(
        const block::Position position,
        const std::shared_ptr<const bitcoin::block::Block> block) noexcept
        -> void;
    auto do_process_common(
        const block::Position position,
        const std::shared_ptr<const bitcoin::block::Block>& block,
        allocator_type monotonic) noexcept -> void;
    auto do_process_update(Message&& msg, allocator_type monotonic) noexcept
        -> void final;
    auto do_reorg(
        const node::HeaderOracle& oracle,
        const node::internal::HeaderOraclePrivate& data,
        Reorg::Params& params) noexcept -> bool final;
    auto do_startup_internal(allocator_type monotonic) noexcept -> void final;
    auto download(Blocks&& blocks) noexcept -> void;
    auto forward_to_next(Message&& msg) noexcept -> void final;
    auto process_blocks(
        Vector<std::shared_ptr<bitcoin::block::Block>> blocks,
        allocator_type monotonic) noexcept -> void final;
    auto process_do_rescan(Message&& in) noexcept -> void final;
    auto process_filter(
        Message&& in,
        block::Position&& tip,
        allocator_type monotonic) noexcept -> void final;
    auto process_mempool(Message&& in, allocator_type monotonic) noexcept
        -> void final;
    auto process_process(
        block::Position&& position,
        allocator_type monotonic) noexcept -> void final;
    auto process_reprocess(Message&& msg, allocator_type monotonic) noexcept
        -> void final;
    auto queue_downloads(allocator_type monotonic) noexcept -> void;
    auto queue_process() noexcept -> bool;
    auto work(allocator_type monotonic) noexcept -> bool final;
};
}  // namespace opentxs::blockchain::node::wallet
