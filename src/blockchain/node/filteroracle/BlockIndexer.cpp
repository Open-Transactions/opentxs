// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "blockchain/node/filteroracle/BlockIndexer.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <algorithm>
#include <chrono>
#include <exception>
#include <iterator>
#include <memory>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#include "blockchain/node/filteroracle/Shared.hpp"
#include "internal/api/Legacy.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/database/Cfilter.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/filteroracle/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/util/BoostPMR.hpp"
#include "internal/util/Future.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Thread.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Block.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/GCS.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Hash.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/bitcoin/cfilter/Header.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/BlockOracle.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/ScopeGuard.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::filteroracle
{
auto print(BlockIndexerJob job) noexcept -> std::string_view
{
    try {
        using Job = BlockIndexerJob;
        using namespace std::literals;
        static const auto map = Map<Job, std::string_view>{
            {Job::shutdown, "shutdown"sv},
            {Job::block_ready, "block_ready"sv},
            {Job::reindex, "reindex"sv},
            {Job::report, "report"sv},
            {Job::reorg, "reorg"sv},
            {Job::header, "header"sv},
            {Job::full_block, "full_block"sv},
            {Job::init, "init"sv},
            {Job::statemachine, "statemachine"sv},
        };

        return map.at(job);
    } catch (...) {
        LogError()(__FUNCTION__)("invalid BlockIndexerJob: ")(
            static_cast<OTZMQWorkType>(job))
            .Flush();

        OT_FAIL;
    }
}
}  // namespace opentxs::blockchain::node::filteroracle

namespace opentxs::blockchain::node::filteroracle
{
BlockIndexer::Imp::BlockQueue::BlockQueue(allocator_type alloc) noexcept
    : requested_(alloc)
    , ready_(alloc)
{
}

auto BlockIndexer::Imp::BlockQueue::Reorg(
    const block::Position& parent) noexcept -> void
{
    requested_.erase(requested_.upper_bound(parent), requested_.end());
    ready_.erase(ready_.upper_bound(parent), ready_.end());
}
}  // namespace opentxs::blockchain::node::filteroracle

namespace opentxs::blockchain::node::filteroracle
{
BlockIndexer::Imp::WorkQueue::WorkQueue() noexcept
    : position_()
    , cfheader_()
    , previous_position_()
    , previous_cfheader_()
{
}

auto BlockIndexer::Imp::WorkQueue::Reorg(
    const Shared& shared,
    const block::Position& parent) noexcept -> void
{
    if (position_ > parent) { Reset(shared, parent); }
}

auto BlockIndexer::Imp::WorkQueue::Reset(
    const Shared& shared,
    block::Position tip) noexcept -> void
{
    position_ = std::move(tip);
    cfheader_ = shared.LoadCfheader(shared.default_type_, position_.hash_);

    if (0 < position_.height_) {
        const auto prev = position_.height_ - 1;
        previous_position_ = {prev, shared.header_.BestHash(prev)};
        previous_cfheader_ =
            shared.LoadCfheader(shared.default_type_, previous_position_.hash_);
    } else {
        previous_position_ = {};
        previous_cfheader_ = {};
    }
}
}  // namespace opentxs::blockchain::node::filteroracle

namespace opentxs::blockchain::node::filteroracle
{
BlockIndexer::Imp::Imp(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const node::Manager> node,
    std::shared_ptr<Shared> shared,
    const network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : Actor(
          shared->api_,
          LogTrace(),
          [&] {
              auto out = CString{print(shared->chain_), alloc};
              out.append(" filter oracle block indexer");

              return out;
          }(),
          0ms,
          batch,
          alloc,
          [&] {
              auto sub = network::zeromq::EndpointArgs{alloc};
              sub.emplace_back(
                  CString{
                      shared->api_.Endpoints()
                          .Internal()
                          .BlockchainReportStatus(),
                      alloc},
                  Direction::Connect);
              sub.emplace_back(
                  CString{
                      shared->api_.Endpoints().BlockchainBlockAvailable(),
                      alloc},
                  Direction::Connect);
              sub.emplace_back(
                  CString{shared->api_.Endpoints().Shutdown(), alloc},
                  Direction::Connect);
              sub.emplace_back(
                  CString{
                      shared->node_.Internal().Endpoints().block_tip_publish_,
                      alloc},
                  Direction::Connect);
              sub.emplace_back(
                  CString{
                      shared->node_.Internal()
                          .Endpoints()
                          .filter_oracle_reindex_publish_,
                      alloc},
                  Direction::Connect);
              sub.emplace_back(
                  CString{
                      shared->node_.Internal().Endpoints().new_header_publish_,
                      alloc},
                  Direction::Connect);
              sub.emplace_back(
                  CString{
                      shared->node_.Internal().Endpoints().shutdown_publish_,
                      alloc},
                  Direction::Connect);

              return sub;
          }())
    , api_p_(std::move(api))
    , node_p_(std::move(node))
    , shared_p_(std::move(shared))
    , api_(*api_p_)
    , node_(*node_p_)
    , checkpoints_([&] {
        auto out = std::filesystem::path{};
        const auto& legacy = api_.Internal().Legacy();
        auto rc = legacy.AppendFolder(
            out, api_.DataFolder(), legacy.BlockchainCheckpoints());

        OT_ASSERT(rc);

        rc = legacy.BuildFolderPath(out);

        OT_ASSERT(rc);

        return out;
    }())
    , shared_(*shared_p_)
    , best_position_()
    , queue_(alloc)
    , blocks_(alloc)
    , work_()
    , tip_()
    , counter_()
    , running_(counter_.Allocate(1))
{
}

auto BlockIndexer::Imp::background() noexcept -> void
{
    const auto& log = log_;
    auto ready = BlockQueue::Map{get_allocator()};
    ready.clear();
    blocks_.lock()->ready_.swap(ready);

    if (ready.empty()) {
        log(OT_PRETTY_CLASS())(name_)(": nothing to do").Flush();

        return;
    } else {
        log(OT_PRETTY_CLASS())(name_)(": processing ")(ready.size())(
            " blocks from ")(ready.cbegin()->first)(" to ")(
            ready.crbegin()->first)
            .Flush();
    }

    // WARNING preserve lock order to avoid deadlocks
    auto wHandle = work_.lock();
    auto& work = *wHandle;
    // WARNING this function must be called from an asio thread and not a zmq
    // thread
    std::byte buf[thread_pool_monotonic_];  // NOLINT(modernize-avoid-c-arrays)
    auto upstream = alloc::StandardToBoost(get_allocator().resource());
    auto monotonic =
        alloc::BoostMonotonic(buf, sizeof(buf), std::addressof(upstream));
    auto alloc = allocator_type{std::addressof(monotonic)};
    auto filters = Vector<database::Cfilter::CFilterParams>{alloc};
    auto headers = Vector<database::Cfilter::CFHeaderParams>{alloc};

    OT_ASSERT(0 <= work.position_.height_);

    for (const auto& [position, future] : ready) {
        OT_ASSERT(IsReady(future));

        const auto& [height, hash] = position;
        const auto pBlock = future.get();

        if (false == bool(pBlock)) {
            // NOTE the only time the future should contain an uninitialized
            // pointer is if the block oracle is shutting down
            log(OT_PRETTY_CLASS())(name_)(": block ")(position)(" unavailable")
                .Flush();

            break;
        }

        const auto& block = *pBlock;

        OT_ASSERT(block.ID() == hash);

        if (block.Header().ParentHash() != work.position_.hash_) {
            log(OT_PRETTY_CLASS())(name_)(": block ")(
                position)(" is not connected to current tip ")(work.position_)
                .Flush();

            break;
        }

        const auto& [ignore1, cfilter] = filters.emplace_back(
            hash,
            shared_.ProcessBlock(shared_.default_type_, block, alloc, alloc));

        if (false == cfilter.IsValid()) {
            LogAbort()(OT_PRETTY_CLASS())(
                name_)(": failed to calculate cfilter for ")(position)
                .Abort();
        }

        auto& [ignore2, cfheader, cfhash] = headers.emplace_back(
            hash, cfilter.Header(work.cfheader_), cfilter.Hash());

        if (cfheader.IsNull()) {
            LogAbort()(OT_PRETTY_CLASS())(
                name_)(": failed to calculate cfheader for ")(position)
                .Abort();
        }

        work.previous_position_ = std::move(work.position_);
        work.previous_cfheader_ = std::move(work.cfheader_);
        work.position_ = position;
        work.cfheader_ = cfheader;
    }

    auto rc = shared_.StoreCfilters(
        shared_.default_type_,
        work.position_,
        std::move(headers),
        std::move(filters),
        alloc);

    if (false == rc) {
        LogAbort()(OT_PRETTY_CLASS())(name_)(": failed to update database")
            .Abort();
    }

    log(OT_PRETTY_CLASS())(name_)(": current position updated to ")(
        work.position_)
        .Flush();
    rc = shared_.SetTips(work.position_);

    if (false == rc) {
        LogAbort()(OT_PRETTY_CLASS())(name_)(": failed to update tip").Abort();
    }

    // WARNING preserve lock order to avoid deadlocks
    *tip_.lock() = work.position_;
    pipeline_.Push(MakeWork(Work::statemachine));
}

auto BlockIndexer::Imp::check_blocks(allocator_type monotonic) noexcept -> void
{
    const auto& log = log_;
    auto ready = Vector<BlockQueue::Map::iterator>{monotonic};
    auto handle = blocks_.lock();
    auto& blocks = *handle;
    auto& from = blocks.requested_;
    auto& to = blocks.ready_;
    ready.reserve(from.size());
    ready.clear();

    for (auto i = from.begin(); i != from.end(); ++i) {
        auto& [position, future] = *i;

        if (IsReady(future)) {
            ready.emplace_back(i);
        } else {
            break;
        }
    }

    for (auto& i : ready) { to.insert(from.extract(i)); }

    log(OT_PRETTY_CLASS())(name_)(": ")(to.size())(" blocks ready").Flush();
}

auto BlockIndexer::Imp::do_shutdown() noexcept -> void
{
    shared_p_.reset();
    node_p_.reset();
    api_p_.reset();
}

auto BlockIndexer::Imp::do_startup(allocator_type monotonic) noexcept -> bool
{
    if ((api_.Internal().ShuttingDown()) || (node_.Internal().ShuttingDown())) {
        return true;
    }

    update_best_position(node_.BlockOracle().Tip());
    const auto& type = shared_.default_type_;
    auto [cfheaderTip, cfilterTip] = shared_.Tips();

    if (cfheaderTip.height_ > cfilterTip.height_) {
        LogError()(OT_PRETTY_CLASS())(name_)(": cfilter tip (")(
            cfilterTip)(") is behind cfheader tip (")(cfheaderTip)(")")
            .Flush();
        cfheaderTip = cfilterTip;
        const auto rc = shared_.SetCfheaderTip(type, cfheaderTip);

        OT_ASSERT(rc);
    }

    // TODO c++20 lambda capture structured binding
    const auto tip = [&](auto&& cfilter) {
        // WARNING preserve lock order to avoid deadlocks
        auto wHandle = work_.lock();
        auto tHandle = tip_.lock();
        auto& work = *wHandle;
        auto& tip = *tHandle;
        work.Reset(shared_, std::move(cfilter));
        tip = work.position_;

        return tip;
    }(std::move(cfilterTip));

    write_last_checkpoint(tip);
    do_work(monotonic);

    return false;
}

auto BlockIndexer::Imp::drain_queue() noexcept -> std::size_t
{
    const auto limit = params::get(shared_.chain_).BlockDownloadBatch();
    const auto& oracle = node_.BlockOracle();
    auto handle = blocks_.lock();
    auto& blocks = *handle;
    auto count = blocks.requested_.size() + blocks.ready_.size();

    while ((false == queue_.empty()) && (limit > count)) {
        auto post = ScopeGuard{[&] { queue_.pop_front(); }};
        const auto& block = queue_.front();
        const auto [_, added] = blocks.requested_.try_emplace(
            block, oracle.LoadBitcoin(block.hash_));

        if (added) { ++count; }
    }

    return blocks.ready_.size();
}

auto BlockIndexer::Imp::fill_queue() noexcept -> void
{
    const auto& log = log_;
    const auto current = [&] {
        if (queue_.empty()) {

            return *tip_.lock_shared();
        } else {

            return queue_.back();
        }
    }();
    const auto& oracle = best_position_;

    if (0 == oracle.height_) { return; }

    log(OT_PRETTY_CLASS())(name_)(": current position: ")(current).Flush();
    log(OT_PRETTY_CLASS())(name_)(":  oracle position: ")(oracle).Flush();
    auto blocks = node_.HeaderOracle().Ancestors(current, oracle, 0_uz);

    OT_ASSERT(false == blocks.empty());

    log(OT_PRETTY_CLASS())(name_)(": loaded ")(blocks.size())(
        " blocks hashes from oracle")
        .Flush();
    log(OT_PRETTY_CLASS())(name_)(": newest common parent is ")(blocks.front())
        .Flush();
    const auto& best = blocks.front();

    if (1_uz < blocks.size()) {
        log(OT_PRETTY_CLASS())(name_)(": first unqueued block is ")(
            blocks.at(1_uz))
            .Flush();
        log(OT_PRETTY_CLASS())(name_)(":  last unqueued block is ")(
            blocks.back())
            .Flush();
    }

    while ((false == queue_.empty()) &&
           (queue_.back().height_ > best.height_)) {
        log(OT_PRETTY_CLASS())(name_)(": removing orphaned block")(
            queue_.back())
            .Flush();
        queue_.pop_back();
    }

    if (false == queue_.empty()) {
        const auto& last = queue_.back();

        OT_ASSERT(last == best);
    }

    if (const auto count = blocks.size(); 1_uz < count) {
        const auto first = std::next(blocks.begin());
        log(OT_PRETTY_CLASS())(name_)(": adding ")(count - 1_uz)(
            " blocks to queue from ") (*first)(" to ")(blocks.back())
            .Flush();
        std::for_each(first, blocks.end(), [this](auto& block) {
            queue_.emplace_back(std::move(block));
        });
    } else {
        log(OT_PRETTY_CLASS())(name_)(": no blocks to add to queue").Flush();
    }
}

auto BlockIndexer::Imp::Init(boost::shared_ptr<Imp> me) noexcept -> void
{
    signal_startup(me);
}

auto BlockIndexer::Imp::pipeline(
    const Work work,
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    switch (work) {
        case Work::block_ready: {
            process_block_ready(std::move(msg));
        } break;
        case Work::reindex: {
            process_reindex(std::move(msg));
        } break;
        case Work::report: {
            process_report(std::move(msg));
        } break;
        case Work::reorg: {
            process_reorg(std::move(msg));
        } break;
        case Work::header: {
            // NOTE no action necessary
        } break;
        case Work::full_block: {
            process_block(std::move(msg));
        } break;
        case Work::shutdown:
        case Work::init:
        case Work::statemachine: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(": unhandled message type ")(
                print(work))
                .Abort();
        }
        default: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(": unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Abort();
        }
    }

    do_work(monotonic);
}

auto BlockIndexer::Imp::process_block(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(body.size() > 2);

    process_block(
        block::Position{body.at(1).as<block::Height>(), body.at(2).Bytes()});
}

auto BlockIndexer::Imp::process_block(block::Position&& position) noexcept
    -> void
{
    update_best_position(std::move(position));
}

auto BlockIndexer::Imp::process_block_ready(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(body.size() > 2);

    log_(OT_PRETTY_CLASS())(name_)(": block ")
        .asHex(body.at(2).Bytes())(" is available for processing")
        .Flush();
}

auto BlockIndexer::Imp::process_reindex(Message&&) noexcept -> void
{
    process_reorg(node_.HeaderOracle().GetPosition(0));
}

auto BlockIndexer::Imp::process_reorg(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(body.size() > 2_uz);

    process_reorg(block::Position{
        body.at(2).as<block::Height>(),
        body.at(1).Bytes(),
    });
}

auto BlockIndexer::Imp::process_reorg(block::Position&& parent) noexcept -> void
{
    {
        // WARNING preserve lock order to avoid deadlocks
        auto bHandle = blocks_.lock();
        auto wHandle = work_.lock();
        auto tHandle = tip_.lock();
        auto& blocks = *bHandle;
        auto& work = *wHandle;
        auto& tip = *tHandle;
        blocks.Reorg(parent);
        work.Reorg(shared_, parent);
        tip = work.position_;
    }

    std::erase_if(queue_, [&](const auto& pos) { return (pos > parent); });

    if (best_position_ > parent) { update_best_position(std::move(parent)); }
}

auto BlockIndexer::Imp::process_report(Message&& in) noexcept -> void
{
    shared_.Report();
}

auto BlockIndexer::Imp::update_best_position(
    block::Position&& position) noexcept -> void
{
    best_position_ = std::move(position);
    log_(OT_PRETTY_CLASS())(name_)(": best position updated to ")(
        best_position_)
        .Flush();
}

auto BlockIndexer::Imp::update_checkpoint() noexcept -> void
{
    const auto tip = block::Position{*tip_.lock_shared()};
    const auto target = block::Height{tip.height_ - 1000};

    if (0 != target % 2000) { return; }

    write_checkpoint(target);
}

auto BlockIndexer::Imp::work(allocator_type monotonic) noexcept -> bool
{
    const auto& log = log_;
    update_checkpoint();
    check_blocks(monotonic);
    fill_queue();
    const auto count = drain_queue();

    if (0_uz < count) {
        if (running_.is_limited()) {
            log(OT_PRETTY_CLASS())(name_)(
                ": waiting for existing job to finish")
                .Flush();
        } else {
            log(OT_PRETTY_CLASS())(name_)(": scheduling ")(
                count)(" blocks for processing")
                .Flush();
            auto post = std::make_shared<ScopeGuard>(
                [this] { ++running_; }, [this] { --running_; });
            api_.Network().Asio().Internal().Post(
                ThreadPool::Blockchain,
                [this, post] { this->background(); },
                name_);
        }
    } else {
        log(OT_PRETTY_CLASS())(name_)(": no blocks ready for processing")
            .Flush();
    }

    return false;
}

auto BlockIndexer::Imp::write_checkpoint(block::Height target) noexcept -> void
{
    if (0 == target) { return; }

    const auto prior = target - 1;
    const auto& header = node_.HeaderOracle();
    const auto position = block::Position{target, header.BestHash(target)};
    const auto previous = block::Position{prior, header.BestHash(prior)};
    const auto cfheader =
        shared_.LoadCfheader(shared_.default_type_, position.hash_);
    params::WriteCheckpoint(
        checkpoints_, position, previous, cfheader, shared_.chain_);
}

auto BlockIndexer::Imp::write_last_checkpoint(
    const block::Position& tip) noexcept -> void
{
    static constexpr auto get_target = [](const auto height) {
        auto target = height - 1000;
        target -= target % 2000;

        return std::max<block::Height>(target, 0);
    };
    static_assert(get_target(2999) == 0);
    static_assert(get_target(3000) == 2000);
    static_assert(get_target(3001) == 2000);
    static_assert(get_target(4999) == 2000);
    static_assert(get_target(5000) == 4000);
    static_assert(get_target(5001) == 4000);

    write_checkpoint(get_target(tip.height_));
}

BlockIndexer::Imp::~Imp() = default;
}  // namespace opentxs::blockchain::node::filteroracle

namespace opentxs::blockchain::node::filteroracle
{
BlockIndexer::BlockIndexer(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const node::Manager> node,
    std::shared_ptr<Shared> shared) noexcept
    : imp_([&] {
        OT_ASSERT(api);
        OT_ASSERT(node);
        OT_ASSERT(shared);

        const auto& zmq = shared->api_.Network().ZeroMQ().Internal();
        const auto batchID = zmq.PreallocateBatch();
        // TODO the version of libc++ present in android ndk 23.0.7599858
        // has a broken std::allocate_shared function so we're using
        // boost::shared_ptr instead of std::shared_ptr

        return boost::allocate_shared<Imp>(
            alloc::PMR<Imp>{zmq.Alloc(batchID)}, api, node, shared, batchID);
    }())
{
    OT_ASSERT(imp_);
}

auto BlockIndexer::Start() noexcept -> void { imp_->Init(imp_); }

BlockIndexer::~BlockIndexer() = default;
}  // namespace opentxs::blockchain::node::filteroracle
