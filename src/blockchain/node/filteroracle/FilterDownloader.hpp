// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/filteroracle/FilterOracle.hpp"  // IWYU pragma: associated

#include "blockchain/DownloadManager.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/database/Cfilter.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/HeaderOracle.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/GCS.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::node::implementation
{
using FilterDM = download::Manager<
    FilterOracle::FilterDownloader,
    GCS,
    cfilter::Header,
    cfilter::Type>;
using FilterWorker = Worker<FilterOracle::FilterDownloader, api::Session>;

class FilterOracle::FilterDownloader : public FilterDM, public FilterWorker
{
public:
    auto NextBatch() noexcept { return allocate_batch(type_); }
    auto UpdatePosition(const Position& pos) -> void
    {
        try {
            auto current = known();
            auto hashes = header_.Ancestors(current, pos, 20000);

            OT_ASSERT(0 < hashes.size());

            auto prior = Previous{std::nullopt};
            auto& first = hashes.front();

            if (first != current) {
                auto promise = std::promise<cfilter::Header>{};
                auto cfheader =
                    db_.LoadFilterHeader(type_, first.hash_.Bytes());

                OT_ASSERT(false == cfheader.IsNull());

                promise.set_value(std::move(cfheader));
                prior.emplace(std::move(first), promise.get_future());
            }
            hashes.erase(hashes.begin());
            update_position(std::move(hashes), type_, std::move(prior));
        } catch (...) {
        }
    }

    FilterDownloader(
        const api::Session& api,
        database::Cfilter& db,
        const node::HeaderOracle& header,
        const node::Manager& node,
        const blockchain::Type chain,
        const cfilter::Type type,
        const node::Endpoints& endpoints,
        const filteroracle::NotifyCallback& notify) noexcept
        : FilterDM(
              [&] { return db.FilterTip(type); }(),
              [&] {
                  auto promise = std::promise<cfilter::Header>{};
                  const auto tip = db.FilterTip(type);
                  promise.set_value(
                      db.LoadFilterHeader(type, tip.hash_.Bytes()));

                  return Finished{promise.get_future()};
              }(),
              "cfilter",
              20000,
              10000)
        , FilterWorker(
              api,
              20ms,
              "blockchain::node::FilterOracle::FilterDownloader")
        , db_(db)
        , header_(header)
        , node_(node)
        , chain_(chain)
        , type_(type)
        , notify_(notify)
    {
        init_executor({endpoints.shutdown_publish_.c_str()});
    }

    ~FilterDownloader() { signal_shutdown().get(); }

private:
    friend FilterDM;
    friend FilterWorker;

    database::Cfilter& db_;
    const node::HeaderOracle& header_;
    const node::Manager& node_;
    const blockchain::Type chain_;
    const cfilter::Type type_;
    const filteroracle::NotifyCallback& notify_;

    auto batch_ready() const noexcept -> void
    {
        node_.Internal().JobReady(PeerManagerJobs::JobAvailableCfilters);
    }
    auto batch_size(std::size_t in) const noexcept -> std::size_t
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
    auto check_task(TaskType&) const noexcept -> void {}
    auto trigger_state_machine() const noexcept -> void { trigger(); }
    auto update_tip(const Position& position, const cfilter::Header&)
        const noexcept -> void
    {
        const auto saved = db_.SetFilterTip(type_, position);

        OT_ASSERT(saved);

        LogDetail()(print(chain_))(" cfilter chain updated to height ")(
            position.height_)
            .Flush();
        notify_(type_, position);
    }

    auto pipeline(const zmq::Message& in) noexcept -> void
    {
        if (false == running_.load()) { return; }

        const auto body = in.Body();

        OT_ASSERT(0 < body.size());

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
            case Work::reset_filter_tip: {
                process_reset(in);
            } break;
            case Work::heartbeat: {
                UpdatePosition(db_.FilterHeaderTip(type_));
                run_if_enabled();
            } break;
            case Work::statemachine: {
                run_if_enabled();
            } break;
            default: {
                OT_FAIL;
            }
        }
    }
    auto process_reset(const zmq::Message& in) noexcept -> void
    {
        const auto body = in.Body();

        OT_ASSERT(3 < body.size());

        auto position = Position{
            body.at(1).as<block::Height>(), block::Hash{body.at(2).Bytes()}};
        auto promise = std::promise<cfilter::Header>{};
        promise.set_value(body.at(3).Bytes());
        Reset(position, promise.get_future());
    }
    auto queue_processing(DownloadedData&& data) noexcept -> void
    {
        if (0 == data.size()) { return; }

        auto filters = Vector<database::Cfilter::CFilterParams>{};

        for (const auto& task : data) {
            const auto& priorCfheader = task->previous_.get();
            auto& cfilter = const_cast<GCS&>(task->data_.get());
            const auto& block = task->position_.hash_;
            const auto expected = db_.LoadFilterHash(type_, block.Bytes());

            if (expected == cfilter.Hash()) {
                task->process(cfilter.Header(priorCfheader));
                filters.emplace_back(block, std::move(cfilter));
            } else {
                LogError()("Filter for block ")(task->position_)(
                    " does not match header. Received: ")(
                    cfilter.Hash().asHex())(" expected: ")(expected.asHex())
                    .Flush();
                task->redownload();
                break;
            }
        }

        const auto saved = db_.StoreFilters(type_, std::move(filters));

        OT_ASSERT(saved);
    }
    auto shutdown(std::promise<void>& promise) noexcept -> void
    {
        if (auto previous = running_.exchange(false); previous) {
            pipeline_.Close();
            promise.set_value();
        }
    }
};
}  // namespace opentxs::blockchain::node::implementation
