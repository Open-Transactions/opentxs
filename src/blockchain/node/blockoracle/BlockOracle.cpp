// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/node/blockoracle/BlockOracle.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <frozen/bits/algorithms.h>
#include <frozen/bits/basic_types.h>
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
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/block/Block.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"

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

auto BlockOracle::DownloadQueue() const noexcept -> std::size_t
{
    return shared_->DownloadQueue();
}

auto BlockOracle::GetWork(alloc::Default alloc) const noexcept -> BlockBatch
{
    return shared_->GetWork(alloc);
}

auto BlockOracle::Load(const block::Hash& block) const noexcept -> BlockResult
{
    return shared_->Load(block);
}

auto BlockOracle::Load(std::span<const block::Hash> hashes) const noexcept
    -> BlockResults
{
    return shared_->Load(hashes);
}

auto BlockOracle::Start(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const node::Manager> node) noexcept -> void
{
    OT_ASSERT(api);
    OT_ASSERT(node);

    const auto& zmq = api->Network().ZeroMQ().Internal();
    const auto batchID = zmq.PreallocateBatch();
    auto* alloc = zmq.Alloc(batchID);
    // TODO the version of libc++ present in android ndk 23.0.7599858
    // has a broken std::allocate_shared function so we're using
    // boost::shared_ptr instead of std::shared_ptr
    shared_ = boost::allocate_shared<BlockOracle::Shared>(
        alloc::PMR<BlockOracle::Shared>{alloc}, *api, *node);

    OT_ASSERT(shared_);

    auto actor = boost::allocate_shared<BlockOracle::Actor>(
        alloc::PMR<BlockOracle::Actor>{alloc}, api, node, shared_, batchID);

    OT_ASSERT(actor);

    actor->Init(actor);
}

auto BlockOracle::SubmitBlock(const blockchain::block::Block& in) const noexcept
    -> bool
{
    return shared_->SubmitBlock(in);
}

auto BlockOracle::Tip() const noexcept -> block::Position
{
    return shared_->Tip();
}

BlockOracle::~BlockOracle() = default;
}  // namespace opentxs::blockchain::node::internal
