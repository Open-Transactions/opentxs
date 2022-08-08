// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/smart_ptr/detail/operator_bool.hpp>

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/wallet/subchain/statemachine/Process.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <future>
#include <memory>
#include <string_view>
#include <thread>
#include <type_traits>
#include <utility>

#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/node/Mempool.hpp"
#include "internal/blockchain/node/wallet/Reorg.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/node/BlockOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Log.hpp"
#include "util/ScopeGuard.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::wallet
{
Process::Imp::Imp(
    const boost::shared_ptr<const SubchainStateData>& parent,
    const network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : Job(LogTrace(),
          parent,
          batch,
          JobType::process,
          alloc,
          {
              {CString{
                   parent->api_.Endpoints().BlockchainBlockAvailable(),
                   alloc},
               Direction::Connect},
              {CString{parent->api_.Endpoints().BlockchainMempool(), alloc},
               Direction::Connect},
          },
          {
              {parent->to_process_endpoint_, Direction::Bind},
          },
          {},
          {
              {SocketType::Push,
               {
                   {parent->to_index_endpoint_, Direction::Connect},
               }},
          })
    , download_limit_(
          2u * params::Chains().at(parent_.chain_).block_download_batch_)
    , to_index_(pipeline_.Internal().ExtraSocket(1))
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

        if (0u < status.size()) {
            to_index_.SendDeferred(
                [&] {
                    auto out = MakeWork(Work::update);
                    add_last_reorg(out);
                    encode(status, out);

                    return out;
                }(),
                __FILE__,
                __LINE__);
        }
    };
    const auto queue = waiting_.size() + downloading_.size() + ready_.size();
    parent_.CheckCache(queue, cb);
}

auto Process::Imp::check_process() noexcept -> bool { return queue_process(); }

auto Process::Imp::do_process(const Ready::value_type& data) noexcept -> void
{
    const auto& [position, block] = data;
    do_process_common(position, block);
}

auto Process::Imp::do_process(
    const block::Position position,
    const std::shared_ptr<const bitcoin::block::Block> block) noexcept -> void
{
    do_process_common(position, block);

    pipeline_.Push([&] {
        auto out = MakeWork(Work::process);
        out.AddFrame(position.height_);
        out.AddFrame(position.hash_);

        return out;
    }());
}

auto Process::Imp::do_process_common(
    const block::Position position,
    const std::shared_ptr<const bitcoin::block::Block>& block) noexcept -> void
{
    OT_ASSERT(block);

    if (false == parent_.ProcessBlock(position, *block)) { OT_FAIL; }
}

auto Process::Imp::do_process_update(Message&& msg) noexcept -> void
{
    auto dirty = Vector<ScanStatus>{get_allocator()};
    extract_dirty(parent_.api_, msg, dirty);
    parent_.process_queue_ += dirty.size();

    for (auto& [type, position] : dirty) {
        waiting_.emplace_back(std::move(position));
    }

    to_index_.SendDeferred(std::move(msg), __FILE__, __LINE__);
    do_work();
}

auto Process::Imp::do_reorg(
    const node::HeaderOracle& oracle,
    const Lock& oracleLock,
    Reorg::Params& params) noexcept -> bool
{
    if (false == parent_.need_reorg_) { return true; }

    const auto& [position, tx] = params;
    // TODO c++20 capture structured binding
    const auto pos{position};
    txid_cache_.clear();
    waiting_.erase(
        std::remove_if(
            waiting_.begin(),
            waiting_.end(),
            [&](const auto& pos) { return pos > pos; }),
        waiting_.end());

    for (auto i{ready_.begin()}, end{ready_.end()}; i != end;) {
        const auto& [position, block] = *i;

        if (position > position) {
            ready_.erase(i, end);

            break;
        } else {
            ++i;
        }
    }

    for (auto i{processing_.begin()}, end{processing_.end()}; i != end;) {
        const auto& [position, block] = *i;

        if (position > position) {
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
            const auto& [position, future] = *i;

            if (erase || (position > position)) {
                erase = true;
                downloading_index_.erase(position.hash_);
                i = map.erase(i);
            } else {
                ++i;
            }
        }
    }

    parent_.process_queue_.store(active());

    return Job::do_reorg(oracle, oracleLock, params);
}

auto Process::Imp::do_startup_internal() noexcept -> void
{
    const auto& oracle = parent_.mempool_oracle_;

    for (const auto& txid : oracle.Dump()) {
        if (auto tx = oracle.Query(txid); tx) {
            parent_.ProcessTransaction(*tx, log_);
        }
    }

    do_work();
}

auto Process::Imp::download(
    block::Position&& position,
    BitcoinBlockResult&& future) noexcept -> void
{
    auto [it, added] =
        downloading_.try_emplace(std::move(position), std::move(future));
    downloading_index_.emplace(it->first.hash_, it);
}

auto Process::Imp::download(block::Position&& position) noexcept -> void
{
    auto future = parent_.node_.BlockOracle().LoadBitcoin(position.hash_);
    download(std::move(position), std::move(future));
}

auto Process::Imp::forward_to_next(Message&& msg) noexcept -> void
{
    to_index_.Send(std::move(msg), __FILE__, __LINE__);
}

auto Process::Imp::have_items() const noexcept -> bool
{
    return 0u < ready_.size();
}

auto Process::Imp::process_block(block::Hash&& hash) noexcept -> void
{
    if (auto index = downloading_index_.find(hash);
        downloading_index_.end() != index) {
        log_(OT_PRETTY_CLASS())(name_)(" processing block ")(hash.asHex())
            .Flush();
        auto& data = index->second;
        const auto& [position, future] = *data;
        auto block = future.get();

        OT_ASSERT(block);

        ready_.try_emplace(position, std::move(block));
        downloading_.erase(data);
        downloading_index_.erase(index);
        txid_cache_.emplace(std::move(hash));
    }

    do_work();
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
    to_index_.Send(std::move(in), __FILE__, __LINE__);
}

auto Process::Imp::process_filter(Message&& in, block::Position&&) noexcept
    -> void
{
    to_index_.Send(std::move(in), __FILE__, __LINE__);
}

auto Process::Imp::process_mempool(Message&& in) noexcept -> void
{
    const auto body = in.Body();
    const auto chain = body.at(1).as<blockchain::Type>();

    if (parent_.chain_ != chain) { return; }

    const auto txid = parent_.api_.Factory().Data(body.at(2));

    // TODO guarantee that already-confirmed transactions can never be processed
    // as mempool transactions even if they are erroneously received from peers
    // on a subsequent run of the application
    if (0u < txid_cache_.count(txid)) {
        log_(OT_PRETTY_CLASS())(name_)(" transaction ")
            .asHex(txid)(" already process as confirmed")
            .Flush();

        return;
    }

    if (auto tx = parent_.mempool_oracle_.Query(txid.Bytes()); tx) {
        parent_.ProcessTransaction(*tx, log_);
    }
}

auto Process::Imp::process_process(block::Position&& pos) noexcept -> void
{
    if (const auto i = processing_.find(pos); i == processing_.end()) {
        log_(OT_PRETTY_CLASS())(name_)(" block ")(
            pos)(" has been removed from the processing list due to reorg")
            .Flush();
    } else {
        --parent_.process_queue_;
        processing_.erase(i);
        log_(OT_PRETTY_CLASS())(name_)(" finished processing block ")(pos)
            .Flush();
    }

    do_work();
}

auto Process::Imp::process_reprocess(Message&& msg) noexcept -> void
{
    log_(OT_PRETTY_CLASS())(name_)(" received re-process request").Flush();
    auto dirty = Vector<ScanStatus>{get_allocator()};
    extract_dirty(parent_.api_, msg, dirty);
    parent_.process_queue_ += dirty.size();

    for (auto& [type, position] : dirty) {
        log_(OT_PRETTY_CLASS())(name_)(" scheduling re-processing for block ")(
            position)
            .Flush();
        auto future = parent_.node_.BlockOracle().LoadBitcoin(position.hash_);
        static constexpr auto ready = std::future_status::ready;
        using namespace std::literals;
        // NOTE re-process requests are holding up the Rescan job from updating
        // its last scanned value, so instead of treating them like new process
        // requests from the Scan we expedite them as much as is reasonable.

        if (ready == future.wait_for(1s)) {
            log_(OT_PRETTY_CLASS())(name_)(" adding block ")(
                position)(" to front of process queue since it is already "
                          "downloaded")
                .Flush();
            ready_.emplace(std::move(position), future.get());
        } else {
            log_(OT_PRETTY_CLASS())(name_)(" adding block ")(
                position)(" to download queue")
                .Flush();
            download(std::move(position), std::move(future));
        }
    }

    do_work();
}

auto Process::Imp::queue_downloads() noexcept -> void
{
    while ((downloading_.size() < download_limit_) && (0u < waiting_.size())) {
        auto& position = waiting_.front();
        log_(OT_PRETTY_CLASS())(name_)(" adding block ")(
            position)(" to download queue")
            .Flush();
        download(std::move(position));
        waiting_.pop_front();
    }
}

auto Process::Imp::queue_process() noexcept -> bool
{
    auto counter = 0u;
    const auto limit = std::max(std::thread::hardware_concurrency(), 1u);
    const auto CanProcess = [&] {
        ++counter;

        return (counter <= limit) && (processing_.size() < download_limit_) &&
               (false == running_.is_limited());
    };

    while (have_items() && CanProcess()) {
        const auto i = processing_.insert(
            processing_.begin(), ready_.extract(ready_.begin()));

        OT_ASSERT(processing_.end() != i);

        auto& [position, block] = *i;
        log_(OT_PRETTY_CLASS())(name_)(" adding block ")(
            position)(" to process queue")
            .Flush();
        parent_.api_.Network().Asio().Internal().Post(
            ThreadPool::Blockchain,
            [this,
             post = std::make_shared<ScopeGuard>(
                 [this] { ++running_; }, [this] { --running_; }),
             pos{i->first},
             ptr{i->second}] { do_process(pos, ptr); },
            "Process block");
    }

    return have_items();
}

auto Process::Imp::work() noexcept -> bool
{
    if (State::reorg == state()) { return false; }

    check_cache();
    queue_downloads();

    return Job::work() || check_process();
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
Process::Process(
    const boost::shared_ptr<const SubchainStateData>& parent) noexcept
    : imp_([&] {
        const auto& asio = parent->api_.Network().ZeroMQ().Internal();
        const auto batchID = asio.PreallocateBatch();
        // TODO the version of libc++ present in android ndk 23.0.7599858
        // has a broken std::allocate_shared function so we're using
        // boost::shared_ptr instead of std::shared_ptr

        return boost::allocate_shared<Imp>(
            alloc::PMR<Imp>{asio.Alloc(batchID)}, parent, batchID);
    }())
{
    OT_ASSERT(imp_);
}

auto Process::Init() noexcept -> void
{
    imp_->Init(imp_);
    imp_.reset();
}

Process::~Process() = default;
}  // namespace opentxs::blockchain::node::wallet
