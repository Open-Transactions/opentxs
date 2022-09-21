// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "internal/blockchain/node/blockoracle/BlockOracle.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <cstddef>
#include <memory>

#include "blockchain/node/blockoracle/Actor.hpp"
#include "blockchain/node/blockoracle/Shared.hpp"
#include "blockchain/node/blockoracle/cache/Cache.hpp"
#include "internal/blockchain/node/blockoracle/BlockBatch.hpp"
#include "internal/blockchain/node/blockoracle/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs::blockchain::node::blockoracle
{
auto print(Job state) noexcept -> std::string_view
{
    using namespace std::literals;

    try {
        static const auto map = Map<Job, std::string_view>{
            {Job::shutdown, "shutdown"sv},
            {Job::request_blocks, "request_blocks"sv},
            {Job::process_block, "process_block"sv},
            {Job::init, "init"sv},
            {Job::statemachine, "statemachine"sv},
        };

        return map.at(state);
    } catch (...) {
        LogAbort()(__FUNCTION__)(": invalid BlockOracleJobs: ")(
            static_cast<OTZMQWorkType>(state))
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
    return shared_->cache_.DownloadQueue();
}

auto BlockOracle::Endpoint() const noexcept -> std::string_view
{
    return shared_->submit_endpoint_;
}

auto BlockOracle::GetBlockBatch(alloc::Default alloc) const noexcept
    -> BlockBatch
{
    return shared_->GetBlockBatch(alloc);
}

auto BlockOracle::GetBlockJob(alloc::Default alloc) const noexcept -> BlockBatch
{
    return shared_->GetBlockJob(alloc);
}

auto BlockOracle::LoadBitcoin(const block::Hash& block) const noexcept
    -> BitcoinBlockResult
{
    return shared_->LoadBitcoin(block);
}

auto BlockOracle::LoadBitcoin(const Vector<block::Hash>& hashes) const noexcept
    -> BitcoinBlockResults
{
    return shared_->LoadBitcoin(hashes);
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
    shared_->StartDownloader(api, node);
}

auto BlockOracle::SubmitBlock(
    std::shared_ptr<const bitcoin::block::Block> in) const noexcept -> bool
{
    return shared_->SubmitBlock(in);
}

auto BlockOracle::Tip() const noexcept -> block::Position
{
    return shared_->Tip();
}

auto BlockOracle::Validate(const bitcoin::block::Block& block) const noexcept
    -> bool
{
    return shared_->Validate(block);
}

BlockOracle::~BlockOracle() = default;
}  // namespace opentxs::blockchain::node::internal
