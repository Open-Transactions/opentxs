// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/node/blockoracle/BlockOracle.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
#include <cstddef>
#include <memory>
#include <string_view>
#include <utility>

#include "blockchain/node/blockoracle/Actor.hpp"
#include "blockchain/node/blockoracle/Shared.hpp"
#include "internal/blockchain/node/blockoracle/BlockBatch.hpp"
#include "internal/blockchain/node/blockoracle/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/alloc/Logging.hpp"
#include "opentxs/api/Network.hpp"
#include "opentxs/api/Session.internal.hpp"
#include "opentxs/api/network/ZeroMQ.hpp"
#include "opentxs/blockchain/block/Block.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"

namespace opentxs::blockchain::node::blockoracle
{
using namespace std::literals;

auto print(Job in) noexcept -> std::string_view
{
    using enum Job;
    static constexpr auto map =
        frozen::make_unordered_map<Job, std::string_view>({
            {shutdown, "shutdown"sv},
            {header, "header"sv},
            {reorg, "reorg"sv},
            {request_blocks, "request_blocks"sv},
            {submit_block, "submit_block"sv},
            {block_ready, "block_ready"sv},
            {report, "report"sv},
            {init, "init"sv},
            {statemachine, "statemachine"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {
        LogAbort()(__FUNCTION__)(": invalid blockoracle::Job: ")(
            static_cast<OTZMQWorkType>(in))
            .Abort();
    }
}
}  // namespace opentxs::blockchain::node::blockoracle

namespace opentxs::blockchain::node::internal
{
BlockOracle::BlockOracle() noexcept
    : shared_()
{
}

auto BlockOracle::BlockExists(const block::Hash& block) const noexcept -> bool
{
    return shared_->BlockExists(block);
}

auto BlockOracle::DownloadQueue() const noexcept -> std::size_t
{
    return shared_->DownloadQueue();
}

auto BlockOracle::GetWork(alloc::Default alloc) const noexcept -> BlockBatch
{
    return shared_->GetWork(alloc);
}

auto BlockOracle::FetchAllBlocks() const noexcept -> bool
{
    return shared_->FetchAllBlocks();
}

auto BlockOracle::Load(const block::Hash& block) const noexcept -> BlockResult
{
    return shared_->Load(block, {});  // TODO monotonic allocator
}

auto BlockOracle::Load(std::span<const block::Hash> hashes) const noexcept
    -> BlockResults
{
    return shared_->Load(hashes, {}, {});  // TODO monotonic allocator
}

auto BlockOracle::Start(
    std::shared_ptr<const api::internal::Session> api,
    std::shared_ptr<const node::Manager> node) noexcept -> void
{
    assert_false(nullptr == api);
    assert_false(nullptr == node);

    const auto& zmq = api->Network().ZeroMQ().Context().Internal();
    const auto batchID = zmq.PreallocateBatch();
    auto* alloc = zmq.Alloc(batchID);
    shared_ = std::allocate_shared<BlockOracle::Shared>(
        alloc::PMR<BlockOracle::Shared>{alloc}, api->Self(), *node);

    assert_false(nullptr == shared_);

    auto actor = std::allocate_shared<BlockOracle::Actor>(
        alloc::PMR<BlockOracle::Actor>{alloc}, api, node, shared_, batchID);

    assert_false(nullptr == actor);

    actor->Init(actor);
}

auto BlockOracle::SubmitBlock(
    const blockchain::block::Block& in,
    alloc::Default monotonic) const noexcept -> bool
{
    return shared_->SubmitBlock(in, monotonic);
}

auto BlockOracle::Tip() const noexcept -> block::Position
{
    return shared_->Tip();
}

BlockOracle::~BlockOracle() = default;
}  // namespace opentxs::blockchain::node::internal
