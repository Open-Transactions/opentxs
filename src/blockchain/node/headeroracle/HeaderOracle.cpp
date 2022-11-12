// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "internal/blockchain/node/headeroracle/HeaderOracle.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <atomic>
#include <string_view>

#include "blockchain/node/headeroracle/Actor.hpp"
#include "blockchain/node/headeroracle/Shared.hpp"
#include "internal/blockchain/node/Factory.hpp"
#include "internal/blockchain/node/headeroracle/HeaderJob.hpp"
#include "internal/blockchain/node/headeroracle/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/bitcoin/block/Header.hpp"    // IWYU pragma: keep
#include "opentxs/blockchain/bitcoin/cfilter/Header.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs::factory
{
auto HeaderOracle(
    const api::Session& api,
    const blockchain::node::Manager& node) noexcept
    -> blockchain::node::internal::HeaderOracle
{
    using ReturnType = blockchain::node::internal::HeaderOracle::Shared;
    const auto& zmq = api.Network().ZeroMQ().Internal();
    const auto batchID = zmq.PreallocateBatch();
    auto* alloc = zmq.Alloc(batchID);
    // TODO the version of libc++ present in android ndk 23.0.7599858
    // has a broken std::allocate_shared function so we're using
    // boost::shared_ptr instead of std::shared_ptr

    return boost::allocate_shared<ReturnType>(
        alloc::PMR<ReturnType>{alloc}, api, node, batchID);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::node::headeroracle
{
auto print(Job job) noexcept -> std::string_view
{
    using namespace std::literals;

    try {
        static const auto map = Map<Job, std::string_view>{
            {Job::shutdown, "shutdown"sv},
            {Job::update_remote_height, "update_remote_height"sv},
            {Job::job_finished, "job_finished"sv},
            {Job::submit_block_header, "submit_block_header"sv},
            {Job::submit_block_hash, "submit_block_hash"sv},
            {Job::report, "report"sv},
            {Job::init, "init"sv},
            {Job::statemachine, "statemachine"sv},
        };

        return map.at(job);
    } catch (...) {
        LogAbort()(__FUNCTION__)(": invalid job: ")(
            static_cast<OTZMQWorkType>(job))
            .Abort();
    }
}
}  // namespace opentxs::blockchain::node::headeroracle

namespace opentxs::blockchain::node::internal
{
HeaderOracle::HeaderOracle(boost::shared_ptr<Shared> shared) noexcept
    : shared_(std::move(shared))
{
    OT_ASSERT(shared_);

    shared_->parent_.store(this);
}

HeaderOracle::HeaderOracle(HeaderOracle&& rhs) noexcept
    : HeaderOracle(std::move(rhs.shared_))
{
    rhs.shared_.reset();
}

auto HeaderOracle::Ancestors(
    const block::Position& start,
    const block::Position& target,
    const std::size_t limit) const noexcept(false) -> Positions
{
    return shared_->Ancestors(start, target, limit);
}

auto HeaderOracle::AddCheckpoint(
    const block::Height position,
    const block::Hash& requiredHash) noexcept -> bool
{
    return shared_->AddCheckpoint(position, requiredHash);
}

auto HeaderOracle::AddHeader(std::unique_ptr<block::Header> header) noexcept
    -> bool
{
    return shared_->AddHeader(std::move(header));
}

auto HeaderOracle::AddHeaders(
    Vector<std::unique_ptr<block::Header>>& headers) noexcept -> bool
{
    return shared_->AddHeaders(headers);
}

auto HeaderOracle::BestChain() const noexcept -> block::Position
{
    return shared_->BestChain();
}

auto HeaderOracle::BestChain(
    const block::Position& tip,
    const std::size_t limit) const noexcept(false) -> Positions
{
    return shared_->BestChain(tip, limit);
}

auto HeaderOracle::BestHash(const block::Height height) const noexcept
    -> block::Hash
{
    return shared_->BestHash(height);
}

auto HeaderOracle::BestHash(
    const block::Height height,
    const block::Position& check) const noexcept -> block::Hash
{
    return shared_->BestHash(height, check);
}

auto HeaderOracle::BestHashes(
    const block::Height start,
    const std::size_t limit,
    alloc::Default alloc) const noexcept -> Hashes
{
    return shared_->BestHashes(start, limit, alloc);
}

auto HeaderOracle::BestHashes(
    const block::Height start,
    const block::Hash& stop,
    const std::size_t limit,
    alloc::Default alloc) const noexcept -> Hashes
{
    return shared_->BestHashes(start, stop, limit, alloc);
}

auto HeaderOracle::BestHashes(
    const Hashes& previous,
    const block::Hash& stop,
    const std::size_t limit,
    alloc::Default alloc) const noexcept -> Hashes
{
    return shared_->BestHashes(previous, stop, limit, alloc);
}

auto HeaderOracle::CalculateReorg(const block::Position& tip) const
    noexcept(false) -> Positions
{
    return shared_->CalculateReorg(tip);
}

auto HeaderOracle::CalculateReorg(
    const HeaderOraclePrivate& data,
    const block::Position& tip) const noexcept(false) -> Positions
{
    return shared_->CalculateReorg(data, tip);
}

auto HeaderOracle::CommonParent(const block::Position& position) const noexcept
    -> std::pair<block::Position, block::Position>
{
    return shared_->CommonParent(position);
}

auto HeaderOracle::DeleteCheckpoint() noexcept -> bool
{
    return shared_->DeleteCheckpoint();
}

auto HeaderOracle::Execute(Vector<ReorgTask>&& jobs) const noexcept -> bool
{
    return shared_->Execute(std::move(jobs));
}

auto HeaderOracle::Exists(const block::Hash& hash) const noexcept -> bool
{
    return shared_->Exists(hash);
}

auto HeaderOracle::GetDefaultCheckpoint() const noexcept -> CheckpointData
{
    return shared_->GetDefaultCheckpoint();
}

auto HeaderOracle::GetCheckpoint() const noexcept -> block::Position
{
    return shared_->GetCheckpoint();
}

auto HeaderOracle::GetJob(alloc::Default alloc) const noexcept -> HeaderJob
{
    return shared_->GetJob(alloc);
}

auto HeaderOracle::GetPosition(const block::Height height) const noexcept
    -> block::Position
{
    return shared_->GetPosition(height);
}

auto HeaderOracle::GetPosition(
    const HeaderOraclePrivate& data,
    const block::Height height) const noexcept -> block::Position
{
    return shared_->GetPosition(data, height);
}

auto HeaderOracle::Init() noexcept -> void { return shared_->Init(); }

auto HeaderOracle::IsInBestChain(const block::Hash& hash) const noexcept -> bool
{
    return shared_->IsInBestChain(hash);
}

auto HeaderOracle::IsInBestChain(const block::Position& position) const noexcept
    -> bool
{
    return shared_->IsInBestChain(position);
}

auto HeaderOracle::IsSynchronized() const noexcept -> bool
{
    return shared_->IsSynchronized();
}

auto HeaderOracle::LoadBitcoinHeader(const block::Hash& hash) const noexcept
    -> std::unique_ptr<bitcoin::block::Header>
{
    return shared_->LoadBitcoinHeader(hash);
}

auto HeaderOracle::LoadHeader(const block::Hash& hash) const noexcept
    -> std::unique_ptr<block::Header>
{
    return shared_->LoadHeader(hash);
}

auto HeaderOracle::ProcessSyncData(
    block::Hash& prior,
    Vector<block::Hash>& hashes,
    const network::otdht::Data& data) noexcept -> std::size_t
{
    return shared_->ProcessSyncData(prior, hashes, data);
}

auto HeaderOracle::RecentHashes(alloc::Default alloc) const noexcept -> Hashes
{
    return shared_->RecentHashes(alloc);
}

auto HeaderOracle::Siblings() const noexcept -> UnallocatedSet<block::Hash>
{
    return shared_->Siblings();
}

auto HeaderOracle::Start(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const node::Manager> node) noexcept -> void
{
    OT_ASSERT(api);
    OT_ASSERT(node);
    OT_ASSERT(shared_);

    auto actor = boost::allocate_shared<HeaderOracle::Actor>(
        alloc::PMR<HeaderOracle::Actor>{shared_->get_allocator()},
        api,
        node,
        shared_,
        shared_->batch_);

    OT_ASSERT(actor);

    actor->Init(actor);
}

auto HeaderOracle::SubmitBlock(const ReadView in) noexcept -> void
{
    shared_->SubmitBlock(in);
}

auto HeaderOracle::Target() const noexcept -> block::Height
{
    return shared_->Target();
}

HeaderOracle::~HeaderOracle() = default;
}  // namespace opentxs::blockchain::node::internal
