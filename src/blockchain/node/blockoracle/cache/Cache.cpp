// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "blockchain/node/blockoracle/cache/Cache.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <memory>
#include <string_view>
#include <utility>

#include "blockchain/node/blockoracle/cache/Actor.hpp"
#include "blockchain/node/blockoracle/cache/Shared.hpp"
#include "internal/blockchain/node/blockoracle/BlockBatch.hpp"
#include "internal/blockchain/node/blockoracle/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs::blockchain::node::blockoracle
{
auto print(CacheJob state) noexcept -> std::string_view
{
    using namespace std::literals;
    using Job = CacheJob;

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
        LogAbort()(__FUNCTION__)(": invalid CacheJob: ")(
            static_cast<OTZMQWorkType>(state))
            .Abort();
    }
}
}  // namespace opentxs::blockchain::node::blockoracle

namespace opentxs::blockchain::node::blockoracle
{
Cache::Cache(const api::Session& api, const node::Manager& node) noexcept
    : shared_([&] {
        const auto& zmq = api.Network().ZeroMQ().Internal();
        const auto batchID = zmq.PreallocateBatch();
        auto* alloc = zmq.Alloc(batchID);
        // TODO the version of libc++ present in android ndk 23.0.7599858
        // has a broken std::allocate_shared function so we're using
        // boost::shared_ptr instead of std::shared_ptr

        return boost::allocate_shared<Cache::Shared>(
            alloc::PMR<Cache::Shared>{alloc}, api, node, batchID);
    }())
{
    OT_ASSERT(shared_);
}

auto Cache::DownloadQueue() const noexcept -> std::size_t
{
    return shared_->DownloadQueue();
}

auto Cache::get_allocator() const noexcept -> allocator_type
{
    return shared_->get_allocator();
}

auto Cache::GetBlockBatch(alloc::Default alloc) noexcept
    -> node::internal::BlockBatch
{
    return shared_->GetBlockBatch(shared_, alloc);
}

auto Cache::ReceiveBlock(
    std::shared_ptr<const bitcoin::block::Block> in) noexcept -> bool
{
    return shared_->ReceiveBlock(std::move(in));
}

auto Cache::Request(const block::Hash& block) noexcept -> BitcoinBlockResult
{
    return shared_->Request(block);
}

auto Cache::Request(const Vector<block::Hash>& hashes) noexcept
    -> BitcoinBlockResults
{
    return shared_->Request(hashes);
}

auto Cache::Start(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const node::Manager> node) noexcept -> void
{
    OT_ASSERT(api);
    OT_ASSERT(node);
    OT_ASSERT(shared_);

    const auto& zmq = api->Network().ZeroMQ().Internal();
    const auto batchID = zmq.PreallocateBatch();
    auto* alloc = zmq.Alloc(batchID);
    // TODO the version of libc++ present in android ndk 23.0.7599858
    // has a broken std::allocate_shared function so we're using
    // boost::shared_ptr instead of std::shared_ptr
    auto actor = boost::allocate_shared<Cache::Actor>(
        alloc::PMR<Cache::Actor>{alloc}, api, node, shared_, batchID);

    OT_ASSERT(actor);

    actor->Init(actor);
}

Cache::~Cache() = default;
}  // namespace opentxs::blockchain::node::blockoracle
