// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "internal/blockchain/node/blockoracle/BlockFetcher.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <memory>
#include <string_view>
#include <utility>

#include "blockchain/node/blockoracle/blockfetcher/Actor.hpp"
#include "blockchain/node/blockoracle/blockfetcher/Shared.hpp"
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
auto print(BlockFetcherJob job) noexcept -> std::string_view
{
    using namespace std::literals;

    try {
        using Job = BlockFetcherJob;
        static const auto map = Map<Job, std::string_view>{
            {Job::shutdown, "shutdown"sv},
            {Job::header, "header"sv},
            {Job::reorg, "reorg"sv},
            {Job::heartbeat, "heartbeat"sv},
            {Job::report, "report"sv},
            {Job::init, "init"sv},
            {Job::statemachine, "statemachine"sv},
        };

        return map.at(job);
    } catch (...) {
        LogError()(__FUNCTION__)("invalid BlockFetcherJob: ")(
            static_cast<OTZMQWorkType>(job))
            .Flush();

        OT_FAIL;
    }
}
}  // namespace opentxs::blockchain::node::blockoracle

namespace opentxs::blockchain::node::blockoracle
{
BlockFetcher::BlockFetcher(
    const api::Session& api,
    const node::Manager& node,
    network::zeromq::BatchID batchID,
    allocator_type alloc) noexcept
    : shared_(boost::allocate_shared<Shared>(
          alloc::PMR<Shared>{alloc},
          api,
          node,
          batchID))
{
    OT_ASSERT(shared_);
}

BlockFetcher::BlockFetcher(
    const api::Session& api,
    const node::Manager& node,
    network::zeromq::BatchID batchID) noexcept
    : BlockFetcher(
          api,
          node,
          batchID,
          api.Network().ZeroMQ().Internal().Alloc(batchID))
{
}

BlockFetcher::BlockFetcher(
    const api::Session& api,
    const node::Manager& node) noexcept
    : BlockFetcher(
          api,
          node,
          api.Network().ZeroMQ().Internal().PreallocateBatch())
{
}

auto BlockFetcher::get_allocator() const noexcept -> allocator_type
{
    return shared_->get_allocator();
}

auto BlockFetcher::GetJob(allocator_type alloc) const noexcept
    -> internal::BlockBatch
{
    return shared_->GetJob(shared_, alloc);
}

auto BlockFetcher::Init(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const node::Manager> node) noexcept -> void
{
    OT_ASSERT(shared_);

    // TODO the version of libc++ present in android ndk 23.0.7599858 has a
    // broken std::allocate_shared function so we're using boost::shared_ptr
    // instead of std::shared_ptr
    auto actor = boost::allocate_shared<Actor>(
        alloc::PMR<Actor>{get_allocator()},
        std::move(api),
        std::move(node),
        shared_,
        shared_->batch_id_);

    OT_ASSERT(actor);

    actor->Init(actor);
}

BlockFetcher::~BlockFetcher() = default;
}  // namespace opentxs::blockchain::node::blockoracle
