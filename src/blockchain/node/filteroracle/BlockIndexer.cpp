// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/filteroracle/BlockIndexer.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <frozen/bits/algorithms.h>
#include <frozen/bits/basic_types.h>
#include <frozen/unordered_map.h>
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <variant>

#include "TBB.hpp"
#include "blockchain/node/filteroracle/Shared.hpp"
#include "internal/api/Legacy.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/block/Parser.hpp"
#include "internal/blockchain/database/Cfilter.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/blockoracle/Types.hpp"
#include "internal/blockchain/node/filteroracle/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/util/Future.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "internal/util/Thread.hpp"
#include "internal/util/Timer.hpp"
#include "internal/util/alloc/Boost.hpp"
#include "internal/util/alloc/Logging.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/GCS.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Hash.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/bitcoin/cfilter/Header.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/ByteLiterals.hpp"
#include "util/ScopeGuard.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::filteroracle
{
using namespace std::literals;

auto print(BlockIndexerJob in) noexcept -> std::string_view
{
    using enum BlockIndexerJob;
    static constexpr auto map =
        frozen::make_unordered_map<BlockIndexerJob, std::string_view>({
            {shutdown, "shutdown"sv},
            {reindex, "reindex"sv},
            {job_finished, "job_finished"sv},
            {report, "report"sv},
            {reorg, "reorg"sv},
            {header, "header"sv},
            {block_ready, "block_ready"sv},
            {init, "init"sv},
            {statemachine, "statemachine"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {
        LogAbort()(__FUNCTION__)(": invalid filteroracle::BlockIndexerJob: ")(
            static_cast<OTZMQWorkType>(in))
            .Abort();
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
              using enum network::zeromq::socket::Direction;
              auto sub = network::zeromq::EndpointArgs{alloc};
              sub.emplace_back(
                  shared->api_.Endpoints().Internal().BlockchainReportStatus(),
                  Connect);
              sub.emplace_back(shared->api_.Endpoints().Shutdown(), Connect);
              sub.emplace_back(
                  shared->node_.Internal()
                      .Endpoints()
                      .filter_oracle_reindex_publish_,
                  Connect);
              sub.emplace_back(
                  shared->node_.Internal().Endpoints().new_header_publish_,
                  Connect);
              sub.emplace_back(
                  shared->node_.Internal().Endpoints().shutdown_publish_,
                  Connect);

              return sub;
          }(),
          {},
          [&] {
              using enum network::zeromq::socket::Direction;
              auto dealer = network::zeromq::EndpointArgs{alloc};
              dealer.emplace_back(
                  node->Internal().Endpoints().block_oracle_router_, Connect);

              return dealer;
          }())
    , api_p_(std::move(api))
    , node_p_(std::move(node))
    , shared_p_(std::move(shared))
    , api_(*api_p_)
    , node_(*node_p_)
    , chain_(node_.Internal().Chain())
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
    , notified_(false)
    , cached_cfilter_bytes_()
    , tip_()
    , downloader_(
          log_,
          name_,
          [this](const auto& tip) { downloader_.SetTip(tip); },
          [this](const auto& tip) { adjust_tip(tip); },
          alloc)
    , index_(alloc)
    , downloading_(alloc)
    , processing_(alloc)
    , finished_(alloc)
    , counter_()
    , running_(counter_.Allocate())
{
}

auto BlockIndexer::Imp::adjust_tip(const block::Position& tip) noexcept -> void
{
    const auto& log = log_;
    const auto reset = [&] {
        const auto& existing = tip_.position_;

        if (tip.height_ == existing.height_) {
            if (tip.hash_ == existing.hash_) {
                log(OT_PRETTY_CLASS())(name_)(": ancestor block ")(
                    tip)(" matches existing tip ")(existing)
                    .Flush();

                return false;
            } else {
                log(OT_PRETTY_CLASS())(name_)(": ancestor block ")(
                    tip)(" does not match existing tip ")(existing)
                    .Flush();

                return true;
            }
        } else if (tip.height_ < existing.height_) {

            return true;
        } else {

            return false;
        }
    }();

    if (reset) {
        load_tip(tip);
        downloading_.clear();
        processing_.clear();
        finished_.clear();
        cached_cfilter_bytes_ = 0_uz;
    } else {
        constexpr auto null = [](const auto&) {};
        const auto cfilter = [this](const auto& job) {
            cached_cfilter_bytes_ -= job->cfilter_.size();
        };
        const auto clean = [&](auto& map, auto name, const auto& cleanup) {
            if (auto i = map.lower_bound(tip.height_); map.end() != i) {
                if (const auto& pos = i->second->position_; pos == tip) {
                    log(OT_PRETTY_CLASS())(name_)(": position ")(
                        pos)(" in the ")(
                        name)(" queue matches incoming ancestor")
                        .Flush();
                    ++i;
                }

                while (map.end() != i) {
                    const auto& stale = i->second->position_;
                    std::invoke(cleanup, i->second);
                    log(OT_PRETTY_CLASS())(name_)(": erasing stale position ")(
                        stale)(" from ")(name)(" queue")
                        .Flush();
                    i = map.erase(i);
                }
            } else {
                log(OT_PRETTY_CLASS())(name_)(": all ")(map.size())(
                    " blocks in the ")(name)(" queue are below height ")(
                    tip.height_)
                    .Flush();
            }
        };
        clean(downloading_, "download"sv, null);
        clean(processing_, "ready"sv, null);
        clean(finished_, "finished"sv, cfilter);
    }
}

auto BlockIndexer::Imp::background(
    boost::shared_ptr<Imp> me,
    JobPointer job,
    std::shared_ptr<const ScopeGuard> post) noexcept -> void
{
    using block::Parser;
    using namespace blockoracle;
    using enum Job::State;

    try {
        OT_ASSERT(me);
        OT_ASSERT(job);
        OT_ASSERT(post);

        const auto& log = me->log_;
        auto alloc = me->get_allocator();
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        std::byte buf[thread_pool_monotonic_];
        auto upstream = alloc::StandardToBoost(alloc.resource());
        auto mr =
            alloc::BoostMonotonic(buf, sizeof(buf), std::addressof(upstream));
        auto monotonic = allocator_type{std::addressof(mr)};
        auto block = block::Block{alloc};
        const auto parsed = Parser::Construct(
            me->api_.Crypto(),
            me->chain_,
            job->position_.hash_,
            reader(job->block_),
            block,
            alloc);

        if (false == parsed) {
            log(OT_PRETTY_STATIC(Imp))(me->name_)(
                ": received invalid block from block ")(job->position_)(
                " oracle")
                .Flush();
            job->state_.store(redownload);
        }

        auto& cfilter = job->cfilter_;
        cfilter = me->shared_.ProcessBlock(
            me->shared_.default_type_, block, alloc, monotonic);

        if (cfilter.IsValid()) {
            job->state_.store(finished);
        } else {
            const auto error =
                UnallocatedCString{"failed to calculate cfilter for "}.append(
                    job->position_.print());

            throw std::runtime_error{error};
        }
    } catch (const std::exception& e) {
        LogAbort()(OT_PRETTY_STATIC(Imp))(me->name_)(": ")(e.what()).Abort();
    }
}

auto BlockIndexer::Imp::calculate_cfilters() noexcept -> bool
{
    if (processing_.empty()) { return false; }

    const auto& log = log_;
    log(OT_PRETTY_CLASS())(name_)(": processing ")(processing_.size())(
        " downloaded jobs")
        .Flush();
    log(OT_PRETTY_CLASS())(name_)(": ")(running_.count())(
        " jobs already running")
        .Flush();

    for (const auto& [_, job] : processing_) {
        if (running_.is_limited()) {
            log(OT_PRETTY_CLASS())(name_)(": maximum job count of ")(
                running_.limit())(" reached")
                .Flush();

            return true;
        } else {
            log(OT_PRETTY_CLASS())(name_)(": processing block ")(job->position_)
                .Flush();
        }

        using enum Job::State;

        auto expected{downloaded};

        if (job->state_.compare_exchange_strong(expected, running)) {
            auto me = boost::shared_from(this);
            auto post = std::make_shared<ScopeGuard>(
                [me] { ++me->running_; },
                [me] {
                    --me->running_;

                    if (auto n = me->notified_.exchange(true); false == n) {
                        using enum BlockIndexerJob;
                        me->pipeline_.Push(MakeWork(job_finished));
                    }
                });
            tbb::fire_and_forget(
                [me, work = job, post] { background(me, work, post); });
        }
    }

    return false;
}

auto BlockIndexer::Imp::calculate_cfheaders(allocator_type monotonic) noexcept
    -> bool
{
    const auto& log = log_;

    try {
        using enum Job::State;
        auto tip{tip_.position_};
        auto next{tip_.cfheader_};
        auto filters = Vector<database::Cfilter::CFilterParams>{monotonic};
        auto headers = Vector<database::Cfilter::CFHeaderParams>{monotonic};
        auto limited{false};
        constexpr auto limit = 1000_uz;

        for (auto i = finished_.begin(), end = finished_.end(); i != end;) {
            auto& [height, pJob] = *i;
            auto& job = *pJob;

            if (const auto target = tip.height_ + 1; height < target) {
                LogAbort()(OT_PRETTY_CLASS())(name_)(": block at height ")(
                    height)(" is in the ready queue even though it should have "
                            "been processed already since the current tip "
                            "height is ")(tip.height_)
                    .Abort();
            } else if (height > target) {
                if (downloading_.contains(target)) {
                    log(OT_PRETTY_CLASS())(name_)(": next block at height ")(
                        target)(" is downloading")
                        .Flush();

                    break;
                } else if (processing_.contains(target)) {
                    log(OT_PRETTY_CLASS())(name_)(": next block at height ")(
                        target)(" is processing")
                        .Flush();

                    break;
                } else {
                    LogAbort()(OT_PRETTY_CLASS())(
                        name_)(": next block at height ")(
                        target)(" does not exist in any queue")
                        .Abort();
                }
            } else {
                log(OT_PRETTY_CLASS())(name_)(": next block at height ")(
                    target)(" is processed")
                    .Flush();
            }

            const auto& previous = job.previous_cfheader_;
            auto& cfilter = job.cfilter_;

            OT_ASSERT(finished == job.state_.load());
            OT_ASSERT(IsReady(previous));
            OT_ASSERT(cfilter.IsValid());

            const auto cachedBytes = cfilter.size();
            const auto& [ignore1, gcs] =
                filters.emplace_back(job.position_.hash_, std::move(cfilter));
            const auto& [ignore2, cfheader, cfhash] = headers.emplace_back(
                job.position_.hash_, gcs.Header(previous.get()), gcs.Hash());

            if (cfheader.IsNull()) {
                const auto error =
                    UnallocatedCString{"failed to calculate cfheader for "}
                        .append(job.position_.print());

                throw std::runtime_error{error};
            }

            job.promise_.set_value(cfheader);
            tip = job.position_;
            next = job.future_;
            cached_cfilter_bytes_ -= cachedBytes;
            i = finished_.erase(i);

            if (filters.size() >= limit) {
                limited = true;
                break;
            }
        }

        const auto rc = shared_.StoreCfilters(
            shared_.default_type_,
            tip,
            std::move(headers),
            std::move(filters),
            monotonic);

        if (false == rc) {

            throw std::runtime_error{"failed to update database"};
        }

        if (shared_.SetTips(tip)) {
            tip_.position_ = tip;
            tip_.cfheader_ = next;
        } else {
            const auto error =
                UnallocatedCString{"failed to update tip to "}.append(
                    tip.print());

            throw std::runtime_error{error};
        }

        return limited;
    } catch (const std::exception& e) {
        LogAbort()(OT_PRETTY_CLASS())(name_)(": ")(e.what()).Abort();
    }
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

    write_last_checkpoint(cfilterTip);
    downloader_.SetTip(cfilterTip);
    load_tip(cfilterTip);
    do_work(monotonic);

    return false;
}

auto BlockIndexer::Imp::fetch_blocks(allocator_type monotonic) noexcept -> bool
{
    try {
        auto [height, hashes, more] =
            downloader_.AddBlocks(node_.HeaderOracle(), monotonic);

        if (false == hashes.empty()) {
            request_blocks(hashes);
            auto previous = previous_cfheader();

            for (const auto& hash : hashes) {
                index_[hash] = height;
                auto& [prior, cfheader] = previous;

                OT_ASSERT(prior + 1 == height);

                const auto [i, added] = downloading_.try_emplace(
                    height,
                    boost::allocate_shared<Job>(
                        alloc::PMR<Job>{get_allocator()},
                        hash,
                        cfheader,
                        height));

                OT_ASSERT(added);

                const auto& pJob = i->second;

                OT_ASSERT(pJob);

                const auto& job = *pJob;
                cfheader = job.future_;
                prior = height++;
            }
        }

        return more;
    } catch (const std::exception& e) {
        LogAbort()(OT_PRETTY_CLASS())(name_)(": ")(e.what()).Abort();
    }
}

auto BlockIndexer::Imp::find_finished(allocator_type monotonic) noexcept -> void
{
    using enum Job::State;
    auto bad = Vector<block::Hash>{monotonic};
    bad.clear();

    for (auto i = processing_.begin(), end = processing_.end(); i != end;) {
        auto& [_, job] = *i;

        switch (job->state_.load()) {
            case waiting: {
                LogAbort()(OT_PRETTY_CLASS())(
                    name_)(": waiting job found in processing queue")
                    .Abort();
            }
            case downloaded:
            case running: {
                ++i;
            } break;
            case finished: {
                auto j = std::next(i);
                cached_cfilter_bytes_ += job->cfilter_.size();
                finished_.insert(processing_.extract(i));
                i = j;
            } break;
            case redownload: {
                auto j = std::next(i);
                job->state_.store(waiting);
                const auto& position = job->position_;
                bad.emplace_back(position.hash_);
                index_[position.hash_] = position.height_;
                downloading_.insert(processing_.extract(i));
                i = j;
            } break;
            default: {
                LogAbort()(OT_PRETTY_CLASS())(name_)(": invalid job state")
                    .Abort();
            }
        }
    }

    request_blocks(bad);
}

auto BlockIndexer::Imp::Init(boost::shared_ptr<Imp> me) noexcept -> void
{
    signal_startup(me);
}

auto BlockIndexer::Imp::load_tip(const block::Position& value) noexcept -> void
{
    auto& [tip, cfheader] = tip_;
    tip = value;
    auto promise = std::promise<cfilter::Header>{};
    cfheader = promise.get_future();
    promise.set_value(shared_.LoadCfheader(shared_.default_type_, tip.hash_));

    if (cfheader.get().empty()) {
        LogAbort()(OT_PRETTY_CLASS())(
            name_)(": failed to load cfheader for block ")(tip)
            .Abort();
    }
}

auto BlockIndexer::Imp::pipeline(
    const Work work,
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    switch (work) {
        case Work::reindex: {
            process_reindex(std::move(msg));
        } break;
        case Work::job_finished: {
            process_job_finished(std::move(msg));
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
        case Work::block_ready: {
            process_block_ready(std::move(msg), monotonic);
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

auto BlockIndexer::Imp::previous_cfheader() const noexcept
    -> std::pair<block::Height, PreviousCfheader>
{
    const auto download = [this] { return downloading_.crbegin()->first; };
    const auto process = [this] { return processing_.crbegin()->first; };
    const auto finished = [this] { return finished_.crbegin()->first; };
    const auto get = [](const auto& map) {
        const auto& last = map.crbegin();
        const auto& height = last->first;
        const auto& job = *last->second;

        return std::make_pair(height, job.future_);
    };

    if (downloading_.empty()) {
        if (processing_.empty()) {
            if (finished_.empty()) {
                const auto& tip = tip_;

                return std::make_pair(tip.position_.height_, tip.cfheader_);
            } else {

                return get(finished_);
            }
        } else {
            if (finished_.empty()) {

                return get(processing_);
            } else {
                if (process() > finished()) {

                    return get(processing_);
                } else {

                    return get(finished_);
                }
            }
        }
    } else {
        if (processing_.empty()) {
            if (finished_.empty()) {

                return get(downloading_);
            } else {
                if (download() > finished()) {

                    return get(downloading_);
                } else {

                    return get(finished_);
                }
            }
        } else {
            if (finished_.empty()) {
                if (download() > process()) {

                    return get(downloading_);
                } else {

                    return get(processing_);
                }
            } else {
                const auto d = download();
                const auto p = process();
                const auto f = finished();

                if ((d > p) && (d > f)) {

                    return get(downloading_);
                } else if ((p > d) && (p > f)) {

                    return get(processing_);
                } else {
                    return get(finished_);
                }
            }
        }
    }
}

auto BlockIndexer::Imp::process_block_ready(
    Message&& in,
    allocator_type monotonic) noexcept -> void
{
    const auto& log = log_;

    try {
        using namespace blockoracle;
        const auto body = in.Payload();
        const auto count = body.size();

        if ((3_uz > count) || (0_uz == count % 2_uz)) {
            const auto error =
                UnallocatedCString{"invalid message frame count: "}.append(
                    std::to_string(count));

            throw std::runtime_error{error};
        }

        const auto cb = [this](const auto& hash, const auto& block) {
            if (auto i = index_.find(hash); index_.end() != i) {
                const auto& height = i->second;
                auto& from = downloading_;
                auto& to = processing_;

                if (auto j = from.find(height); from.end() != j) {
                    using enum Job::State;
                    auto& job = *j->second;
                    job.block_ = block;
                    job.state_.store(downloaded);
                    to.insert(from.extract(j));
                } else {
                    const auto error = UnallocatedCString{"block "}
                                           .append(hash.asHex())
                                           .append(" not in download index");

                    throw std::runtime_error{error};
                }

                index_.erase(i);
            } else {
                const auto error = UnallocatedCString{"block "}
                                       .append(hash.asHex())
                                       .append(" not in download cache");

                throw std::runtime_error{error};
            }
        };

        for (auto n = 1_uz; n < count; n += 2_uz) {
            const auto hash = block::Hash{body[n].Bytes()};
            const auto block = parse_block_location(body[n + 1_uz]);
            downloader_.ReceiveBlock(hash, block, cb);
        }

        log(OT_PRETTY_CLASS())(name_)(": moved ")(
            count)(" blocks from download queue to process queue.")
            .Flush();
        log(OT_PRETTY_CLASS())(name_)(": download queue size: ")(
            downloading_.size())
            .Flush();
        log(OT_PRETTY_CLASS())(name_)(": process queue size: ")(
            processing_.size())
            .Flush();
        log(OT_PRETTY_CLASS())(name_)(": finished queue size: ")(
            finished_.size())
            .Flush();
    } catch (const std::exception& e) {
        LogAbort()(OT_PRETTY_CLASS())(name_)(": ")(e.what()).Abort();
    }
}

auto BlockIndexer::Imp::process_job_finished(Message&& in) noexcept -> void
{
    const auto& log = log_;
    notified_.store(false);
    log(OT_PRETTY_CLASS())(name_)(": download queue size: ")(
        downloading_.size())
        .Flush();
    log(OT_PRETTY_CLASS())(name_)(": process queue size: ")(processing_.size())
        .Flush();
    log(OT_PRETTY_CLASS())(name_)(": finished queue size: ")(finished_.size())
        .Flush();
}

auto BlockIndexer::Imp::process_reindex(Message&&) noexcept -> void
{
    process_reorg(node_.HeaderOracle().GetPosition(0));
}

auto BlockIndexer::Imp::process_reorg(Message&& in) noexcept -> void
{
    const auto body = in.Payload();

    OT_ASSERT(body.size() > 2_uz);

    process_reorg(block::Position{
        body[2].as<block::Height>(),
        body[1].Bytes(),
    });
}

auto BlockIndexer::Imp::process_reorg(block::Position&&) noexcept -> void
{
    // NOTE no action required
}

auto BlockIndexer::Imp::process_report(Message&& in) noexcept -> void
{
    shared_.Report();
}

auto BlockIndexer::Imp::ready() const noexcept -> bool
{
    constexpr auto minimum = 100_uz;
    static const auto maximum = convert_to_size(16_mib);

    if (const auto count = finished_.size(); 0_uz == count) {

        return false;
    } else if (maximum <= cached_cfilter_bytes_) {

        return true;
    } else if (downloading_.empty() && processing_.empty()) {

        return true;
    } else {

        return count >= minimum;
    }
}

auto BlockIndexer::Imp::request_blocks(
    std::span<const block::Hash> hashes) noexcept -> void
{
    if (hashes.empty()) { return; }

    pipeline_.Internal().SendFromThread([&] {
        using enum blockoracle::Job;
        auto msg = MakeWork(request_blocks);

        for (const auto& hash : hashes) { msg.AddFrame(hash); }

        return msg;
    }());
}

auto BlockIndexer::Imp::update_checkpoint() noexcept -> void
{
    const auto& tip = tip_.position_;
    const auto target = block::Height{tip.height_ - 1000};

    if (0 != target % 2000) { return; }

    write_checkpoint(target);
}

auto BlockIndexer::Imp::work(allocator_type monotonic) noexcept -> bool
{
    update_checkpoint();
    auto out = fetch_blocks(monotonic);
    out |= calculate_cfilters();
    find_finished(monotonic);

    if (ready()) { out |= calculate_cfheaders(monotonic); }

    downloader_.Update();

    return out;
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
