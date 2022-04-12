// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/filteroracle/BlockIndexer.hpp"  // IWYU pragma: associated

#include <atomic>
#include <chrono>
#include <exception>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <tuple>

#include "blockchain/DownloadManager.hpp"
#include "blockchain/DownloadTask.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/blockchain/node/BlockOracle.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/GCS.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Hash.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Header.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "util/JobCounter.hpp"
#include "util/ScopeGuard.hpp"

namespace opentxs::blockchain::node::implementation
{
FilterOracle::BlockIndexer::BlockIndexer(
    const api::Session& api,
    internal::FilterDatabase& db,
    const HeaderOracle& header,
    const internal::BlockOracle& block,
    const internal::Network& node,
    FilterOracle& parent,
    const blockchain::Type chain,
    const cfilter::Type type,
    const UnallocatedCString& shutdown,
    const NotifyCallback& notify) noexcept
    : BlockDMFilter(
          [&] { return db.FilterTip(type); }(),
          [&] {
              auto promise = std::promise<cfilter::Header>{};
              const auto tip = db.FilterTip(type);
              promise.set_value(db.LoadFilterHeader(type, tip.second.Bytes()));

              return Finished{promise.get_future()};
          }(),
          "filter",
          2000,
          1000)
    , BlockWorkerFilter(api, 20ms)
    , db_(db)
    , header_(header)
    , block_(block)
    , node_(node)
    , parent_(parent)
    , chain_(chain)
    , type_(type)
    , notify_(notify)
    , job_counter_()
{
    init_executor(
        {shutdown,
         UnallocatedCString{
             api_.Endpoints().Internal().BlockchainBlockUpdated(chain_)}});
}

auto FilterOracle::BlockIndexer::batch_size(const std::size_t in) const noexcept
    -> std::size_t
{
    if (in < 10) {

        return 1;
    } else if (in < 100) {

        return 10;
    } else if (in < 1000) {

        return 100;
    } else {

        return 1000;
    }
}

auto FilterOracle::BlockIndexer::calculate_cfheaders(
    UnallocatedVector<BlockIndexerData>& cache) const noexcept -> bool
{
    auto failures{0};

    for (auto& data : cache) {
        auto& [blockHashView, cfilter] = data.filter_data_;
        auto& task = data.incoming_data_;
        const auto& [height, block] = task.position_;

        try {
            LogTrace()(OT_PRETTY_CLASS())("Calculating cfheader for ")(
                print(chain_))(" block at height ")(height)
                .Flush();
            auto& [blockHash, cfheader, filterHashView] = data.header_data_;
            auto& previous = task.previous_;
            static constexpr auto zero = 0s;
            using State = std::future_status;

            if (auto status = previous.wait_for(zero); State::ready != status) {
                LogError()(OT_PRETTY_CLASS())("Timeout waiting for previous ")(
                    print(chain_))(" cfheader #")(height - 1)
                    .Flush();

                throw std::runtime_error("timeout");
            }

            OT_ASSERT(cfilter.IsValid());

            cfheader = cfilter.Header(previous.get().Bytes());

            if (cfheader.empty()) {
                LogError()(OT_PRETTY_CLASS())("failed to calculate ")(
                    print(chain_))(" cfheader #")(height)
                    .Flush();

                throw std::runtime_error("Failed to calculate cfheader");
            }

            LogTrace()(OT_PRETTY_CLASS())(
                "Finished calculating cfheader and cfilter "
                "for ")(print(chain_))(" block at height ")(height)
                .Flush();
            task.process(std::move(cfheader));
        } catch (...) {
            task.process(std::current_exception());
            ++failures;
        }
    }

    return (0 == failures);
}

auto FilterOracle::BlockIndexer::download() noexcept -> void
{
    auto work = NextBatch();
    constexpr auto none = 0s;

    for (const auto& task : work.data_) {
        const auto& hash = task->position_.second;
        auto future = block_.LoadBitcoin(hash);

        if (std::future_status::ready == future.wait_for(none)) {
            auto block = future.get();

            if (!block) { return; }  // Might occur during shutdown

            OT_ASSERT(block->ID() == hash);

            task->download(std::move(block));
        }
    }
}

auto FilterOracle::BlockIndexer::pipeline(const zmq::Message& in) noexcept
    -> void
{
    if (false == running_.load()) { return; }

    const auto body = in.Body();

    OT_ASSERT(1 <= body.size());

    using Work = FilterOracle::Work;
    const auto work = [&] {
        try {

            return body.at(0).as<Work>();
        } catch (...) {

            OT_FAIL;
        }
    }();

    switch (work) {
        case Work::shutdown: {
            shutdown(shutdown_promise_);
        } break;
        case Work::heartbeat: {
            if (dm_enabled()) { process_position(block_.Tip()); }

            run_if_enabled();
        } break;
        case Work::full_block: {
            if (dm_enabled()) { process_position(in); }

            run_if_enabled();
        } break;
        case Work::statemachine: {
            download();
            run_if_enabled();
        } break;
        default: {
            OT_FAIL;
        }
    }
}

auto FilterOracle::BlockIndexer::process_position(
    const zmq::Message& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(body.size() > 2);

    process_position(
        Position{body.at(1).as<block::Height>(), body.at(2).Bytes()});
}

auto FilterOracle::BlockIndexer::process_position(const Position& pos) noexcept
    -> void
{
    const auto current = known();
    auto compare{current};
    LogTrace()(OT_PRETTY_CLASS())(" Current position: ")(print(current))
        .Flush();
    LogTrace()(OT_PRETTY_CLASS())("Incoming position: ")(print(pos)).Flush();
    auto hashes = decltype(header_.Ancestors(current, pos)){};
    auto prior = Previous{std::nullopt};
    auto searching{true};

    while (searching) {
        try {
            hashes = header_.Ancestors(compare, pos, 2000u);

            OT_ASSERT(0 < hashes.size());
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
            reset_to_genesis();

            return;
        }

        auto postcondition = ScopeGuard{[&] { hashes.erase(hashes.begin()); }};
        auto& first = hashes.front();
        LogTrace()(OT_PRETTY_CLASS())("         Ancestor: ")(print(first))
            .Flush();

        if (first == pos) { return; }

        if (first == current) {
            searching = false;
            break;
        }

        auto cfheader = db_.LoadFilterHeader(type_, first.second.Bytes());
        // TODO allocator
        const auto cfilter = db_.LoadFilter(type_, first.second.Bytes(), {});

        if (cfheader.IsNull()) {
            LogError()(OT_PRETTY_CLASS())("Missing cfheader for block ")(
                print(first))
                .Flush();
        }

        if (false == cfilter.IsValid()) {
            LogError()(OT_PRETTY_CLASS())("Missing cfilter for block ")(
                print(first))
                .Flush();
        }

        if (cfilter.IsValid() && (false == cfheader.IsNull())) {
            auto promise = std::promise<cfilter::Header>{};
            promise.set_value(std::move(cfheader));
            prior.emplace(std::move(first), promise.get_future());
            searching = false;
            break;
        }

        OT_ASSERT(0 < first.first);

        const auto height = first.first - 1;
        compare = block::Position{height, header_.BestHash(height)};
    }

    update_position(std::move(hashes), type_, std::move(prior));
}

auto FilterOracle::BlockIndexer::queue_processing(
    DownloadedData&& data) noexcept -> void
{
    if (0u == data.size()) { return; }

    auto filters = Vector<internal::FilterDatabase::CFilterParams>{};
    auto headers = Vector<internal::FilterDatabase::CFHeaderParams>{};
    auto cache = UnallocatedVector<BlockIndexerData>{};
    const auto& tip = data.back();

    {
        auto jobCounter = job_counter_.Allocate();
        const auto count{data.size()};

        if (0 == data.size()) { return; }

        filters.reserve(count);
        headers.reserve(count);
        cache.reserve(count);
        static const auto blankHash = cfilter::Hash{};

        for (const auto& task : data) {
            if (false == running_.load()) { return; }

            auto& filter = filters.emplace_back();
            auto& header = headers.emplace_back();
            auto& job = cache.emplace_back(
                blankHash, *task, type_, filter, header, jobCounter);
            ++jobCounter;
            const auto queued = api_.Network().Asio().Internal().Post(
                ThreadPool::General, [&] { parent_.ProcessBlock(job); });

            if (false == queued) {
                --jobCounter;

                return;
            }
        }
    }

    if (false == calculate_cfheaders(cache)) { return; }

    try {
        tip->output_.get();
        const auto stored =
            db_.StoreFilters(type_, headers, filters, tip->position_);

        if (false == stored) {
            const auto error = CString{print(chain_)} + " database error";

            throw std::runtime_error{error.c_str()};
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        for (auto& task : data) { task->process(std::current_exception()); }
    }
}

auto FilterOracle::BlockIndexer::reset_to_genesis() noexcept -> void
{
    LogError()(OT_PRETTY_CLASS())("Performing full reset").Flush();
    static const auto genesis =
        block::Position{0, header_.GenesisBlockHash(chain_)};
    auto promise = std::promise<cfilter::Header>{};
    promise.set_value(db_.LoadFilterHeader(type_, genesis.second.Bytes()));
    Reset(genesis, promise.get_future());
}

auto FilterOracle::BlockIndexer::shutdown(std::promise<void>& promise) noexcept
    -> void
{
    if (auto previous = running_.exchange(false); previous) {
        pipeline_.Close();
        promise.set_value();
    }
}

auto FilterOracle::BlockIndexer::update_tip(
    const Position& position,
    const cfilter::Header&) const noexcept -> void
{
    auto saved = db_.SetFilterTip(type_, position);

    OT_ASSERT(saved);

    saved = db_.SetFilterHeaderTip(type_, position);

    OT_ASSERT(saved);

    LogDetail()(print(chain_))(
        " cfheader and cfilter chain updated to height ")(position.first)
        .Flush();
    notify_(type_, position);
}

FilterOracle::BlockIndexer::~BlockIndexer() { signal_shutdown().get(); }
}  // namespace opentxs::blockchain::node::implementation
