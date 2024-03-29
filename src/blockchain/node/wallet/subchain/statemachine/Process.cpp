// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/wallet/subchain/statemachine/Process.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <atomic>
#include <memory>
#include <utility>

#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/Mempool.hpp"
#include "internal/blockchain/node/blockoracle/Types.hpp"
#include "internal/blockchain/node/wallet/Reorg.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Types.hpp"
#include "internal/blockchain/params/ChainData.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Thread.hpp"
#include "internal/util/alloc/Logging.hpp"
#include "internal/util/alloc/MonotonicSync.hpp"
#include "opentxs/Context.hpp"
#include "opentxs/WorkType.internal.hpp"
#include "opentxs/api/Network.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/network/ZeroMQ.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Direction.hpp"   // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Policy.hpp"      // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Options.hpp"
#include "util/ScopeGuard.hpp"

namespace opentxs::blockchain::node::wallet
{
using enum opentxs::network::zeromq::socket::Direction;
using enum opentxs::network::zeromq::socket::Policy;
using enum opentxs::network::zeromq::socket::Type;

Process::Imp::Imp(
    const std::shared_ptr<const SubchainStateData>& parent,
    const network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : Job(LogTrace(),
          parent,
          batch,
          JobType::process,
          alloc,
          {
              {parent->api_.Endpoints().BlockchainMempool(), Connect},
          },
          {
              {parent->to_process_endpoint_, Bind},
          },
          {},
          {
              {Push,
               Internal,
               {
                   {parent->to_index_endpoint_, Connect},
               }},
              {Dealer,
               Internal,
               {
                   {parent->node_.Internal().Endpoints().block_oracle_router_,
                    Connect},
               }},
          })
    , download_limit_(2_uz * params::get(parent_.chain_).BlockDownloadBatch())
    , to_index_(pipeline_.Internal().ExtraSocket(1))
    , to_block_oracle_(pipeline_.Internal().ExtraSocket(2))
    , waiting_(alloc)
    , downloading_(alloc)
    , downloading_index_(alloc)
    , ready_(alloc)
    , processing_(alloc)
    , txid_cache_()
    , counter_()
    , running_(counter_.Allocate())
{
    txid_cache_.reserve(1024);
}

auto Process::Imp::active() const noexcept -> std::size_t
{
    return waiting_.size() + downloading_.size() + ready_.size() +
           processing_.size();
}

auto Process::Imp::check_cache() noexcept -> void
{
    const auto cb = [this](const auto& positions) {
        auto status = Vector<ScanStatus>{get_allocator()};

        for (const auto& pos : positions) {
            status.emplace_back(ScanState::processed, pos);
        }

        if (const auto count = status.size(); 0_uz == count) {
            log_()(name_)(": no processed blocks in cache").Flush();
        } else {
            log_()(name_)(": found ")(count)(" processed blocks in cache")
                .Flush();
            to_index_.SendDeferred([&] {
                auto out = MakeWork(Work::update);
                add_last_reorg(out);
                encode(status, out);

                return out;
            }());
        }
    };
    const auto queue = waiting_.size() + downloading_.size() + ready_.size();
    parent_.CheckCache(queue, cb);
}

auto Process::Imp::check_process() noexcept -> bool { return queue_process(); }

auto Process::Imp::do_process(
    Ready::value_type& data,
    allocator_type monotonic) noexcept -> void
{
    auto& [position, block] = data;
    do_process_common(position, block, monotonic);
}

auto Process::Imp::do_process(
    const block::Position position,
    block::Block block) noexcept -> void
{
    // WARNING this function must not be be called from a zmq thread
    auto alloc = alloc::MonotonicSync{get_allocator().resource()};
    do_process_common(position, block, std::addressof(alloc));
    pipeline_.Push([&] {
        auto out = MakeWork(Work::process);
        out.AddFrame(position.height_);
        out.AddFrame(position.hash_);

        return out;
    }());
}

auto Process::Imp::do_process_common(
    const block::Position position,
    block::Block& block,
    allocator_type monotonic) noexcept -> void
{
    if (false == parent_.ProcessBlock(position, block, monotonic)) {
        LogAbort()().Abort();
    }
}

auto Process::Imp::do_process_update(
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    auto dirty = Vector<ScanStatus>{get_allocator()};
    extract_dirty(api_, msg, dirty);
    parent_.process_queue_ += dirty.size();

    for (auto& [type, position] : dirty) {
        waiting_.emplace_back(std::move(position));
    }

    to_index_.SendDeferred(std::move(msg));
    do_work(monotonic);
}

auto Process::Imp::do_reorg(
    const node::HeaderOracle& oracle,
    const node::internal::HeaderOraclePrivate& data,
    Reorg::Params& params) noexcept -> bool
{
    if (false == parent_.need_reorg_) { return true; }

    const auto& [target, tx] = params;
    txid_cache_.clear();
    waiting_.erase(
        std::remove_if(
            waiting_.begin(),
            waiting_.end(),
            [&, pos{target}](const auto& p) { return p.IsReplacedBy(pos); }),
        waiting_.end());

    for (auto i{ready_.begin()}, end{ready_.end()}; i != end;) {
        const auto& [ready, block] = *i;

        if (ready.IsReplacedBy(target)) {
            ready_.erase(i, end);

            break;
        } else {
            ++i;
        }
    }

    for (auto i{processing_.begin()}, end{processing_.end()}; i != end;) {
        const auto& [processing, block] = *i;

        if (processing.IsReplacedBy(target)) {
            processing_.erase(i, end);

            break;
        } else {
            ++i;
        }
    }

    {
        auto erase{false};
        auto& map = downloading_;

        for (auto i = map.begin(), end = map.end(); i != end;) {
            const auto& downloading = *i;

            if (erase || (downloading.IsReplacedBy(target))) {
                erase = true;
                downloading_index_.erase(downloading.hash_);
                i = map.erase(i);
            } else {
                ++i;
            }
        }
    }

    parent_.process_queue_.store(active());

    return Job::do_reorg(oracle, data, params);
}

auto Process::Imp::do_startup_internal(allocator_type monotonic) noexcept
    -> void
{
    do_work(monotonic);
}

auto Process::Imp::download(Blocks&& blocks) noexcept -> void
{
    to_block_oracle_.SendDeferred([&] {
        using enum blockchain::node::blockoracle::Job;
        auto out = MakeWork(request_blocks);

        for (auto& position : blocks) {
            out.AddFrame(position.hash_);
            auto [it, added] = downloading_.emplace(std::move(position));
            downloading_index_.emplace(it->hash_, it);
        }

        return out;
    }());
}

auto Process::Imp::forward_to_next(Message&& msg) noexcept -> void
{
    to_index_.SendDeferred(std::move(msg));
}

auto Process::Imp::have_items() const noexcept -> bool
{
    return false == ready_.empty();
}

auto Process::Imp::process_blocks(
    std::span<block::Block> blocks,
    allocator_type monotonic) noexcept -> void
{
    for (auto& block : blocks) {
        auto id{block.Header().Hash()};

        if (auto index = downloading_index_.find(id);
            downloading_index_.end() != index) {
            log_()(name_)(" processing block ")(id.asHex()).Flush();

            for (const auto& tx : block.get()) { txid_cache_.emplace(tx.ID()); }

            auto& data = index->second;
            const auto& position = *data;
            ready_.try_emplace(position, std::move(block));
            downloading_.erase(data);
            downloading_index_.erase(index);
        }
    }

    do_work(monotonic);
}

auto Process::Imp::process_do_rescan(Message&& in) noexcept -> void
{
    waiting_.clear();
    downloading_.clear();
    downloading_index_.clear();
    ready_.clear();
    processing_.clear();
    txid_cache_.clear();
    parent_.process_queue_.store(0);
    to_index_.SendDeferred(std::move(in));
}

auto Process::Imp::process_filter(
    Message&& in,
    block::Position&&,
    allocator_type) noexcept -> void
{
    to_index_.SendDeferred(std::move(in));
}

auto Process::Imp::process_mempool(
    Message&& in,
    allocator_type monotonic) noexcept -> void
{
    const auto& log = api_.GetOptions().TestMode() ? LogConsole() : log_;
    const auto body = in.Payload();
    const auto chain = body[1].as<blockchain::Type>();

    if (parent_.chain_ != chain) { return; }

    const auto txid = block::TransactionHash{body[2].Bytes()};

    // TODO guarantee that already-confirmed transactions can never be processed
    // as mempool transactions even if they are erroneously received from peers
    // on a subsequent run of the application
    if (txid_cache_.contains(txid)) {
        log()(name_)(" transaction ")
            .asHex(txid)(" already process as confirmed")
            .Flush();

        return;
    }

    if (auto t = parent_.mempool_oracle_.Query(txid, monotonic); t.IsValid()) {
        parent_.ProcessTransaction(t, log, monotonic);
    }
}

auto Process::Imp::process_process(
    block::Position&& pos,
    allocator_type monotonic) noexcept -> void
{
    if (const auto i = processing_.find(pos); i == processing_.end()) {
        log_()(name_)(" block ")(
            pos)(" has been removed from the processing list due to reorg")
            .Flush();
    } else {
        --parent_.process_queue_;
        processing_.erase(i);
        log_()(name_)(" finished processing block ")(pos).Flush();
    }

    do_work(monotonic);
}

auto Process::Imp::process_reprocess(
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    log_()(name_)(" received re-process request").Flush();
    auto dirty = Vector<ScanStatus>{get_allocator()};
    extract_dirty(api_, msg, dirty);
    const auto count = dirty.size();
    parent_.process_queue_ += count;
    auto blocks = Blocks{monotonic};
    blocks.reserve(count);

    for (auto& [type, position] : dirty) {
        log_()(name_)(" scheduling re-processing for block ")(position).Flush();
        blocks.emplace_back(std::move(position));
    }

    download(std::move(blocks));
    do_work(monotonic);
}

auto Process::Imp::queue_downloads(allocator_type monotonic) noexcept -> void
{
    const auto count = std::min(
        waiting_.size(),
        (std::max(download_limit_, downloading_.size()) - downloading_.size()));
    auto blocks = Blocks{monotonic};
    blocks.reserve(count);

    for (auto n = 0_uz; n < count; ++n) {
        auto& position = waiting_.front();
        log_()(name_)(" adding block ")(position)(" to download queue").Flush();
        blocks.emplace_back(std::move(position));
        waiting_.pop_front();
    }

    download(std::move(blocks));
}

auto Process::Imp::queue_process() noexcept -> bool
{
    auto counter = 0u;
    const auto hardCap = 1_uz;  // TODO download_limit_;
    const auto limit = MaxJobs();
    const auto CanProcess = [&] {
        ++counter;

        return (counter <= limit) && (processing_.size() < hardCap) &&
               (false == running_.is_limited());
    };

    while (have_items() && CanProcess()) {
        const auto i = processing_.insert(
            processing_.begin(), ready_.extract(ready_.begin()));

        assert_true(processing_.end() != i);

        auto& [position, block] = *i;
        log_()(name_)(" adding block ")(position)(" to process queue").Flush();
        auto me = shared_from_this();
        auto post = std::make_shared<ScopeGuard>(
            [me] { ++me->running_; }, [me] { --me->running_; });
        RunJob([me, post, pos{i->first}, ptr{i->second}] {
            me->do_process(pos, ptr);
        });
    }

    return have_items();
}

auto Process::Imp::work(allocator_type monotonic) noexcept -> bool
{
    if (State::reorg == state()) { return false; }

    check_cache();
    queue_downloads(monotonic);

    return Job::work(monotonic) || check_process();
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
Process::Process(
    const std::shared_ptr<const SubchainStateData>& parent) noexcept
    : imp_([&] {
        const auto& asio = parent->api_.Network().ZeroMQ().Context().Internal();
        const auto batchID = asio.PreallocateBatch();

        return std::allocate_shared<Imp>(
            alloc::PMR<Imp>{asio.Alloc(batchID)}, parent, batchID);
    }())
{
    assert_false(nullptr == imp_);
}

auto Process::Init() noexcept -> void
{
    imp_->Init(imp_);
    imp_.reset();
}

Process::~Process() = default;
}  // namespace opentxs::blockchain::node::wallet
