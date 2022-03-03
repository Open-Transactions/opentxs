// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "1_Internal.hpp"                    // IWYU pragma: associated
#include "blockchain/node/FilterOracle.hpp"  // IWYU pragma: associated

#include "blockchain/DownloadManager.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/GCS.hpp"
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
    std::unique_ptr<const GCS>,
    filter::pHeader,
    filter::Type>;
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
                auto promise = std::promise<filter::pHeader>{};
                auto header =
                    db_.LoadFilterHeader(type_, first.second->Bytes());

                OT_ASSERT(false == header->empty());

                promise.set_value(std::move(header));
                prior.emplace(std::move(first), promise.get_future());
            }
            hashes.erase(hashes.begin());
            update_position(std::move(hashes), type_, std::move(prior));
        } catch (...) {
        }
    }

    FilterDownloader(
        const api::Session& api,
        const internal::FilterDatabase& db,
        const HeaderOracle& header,
        const internal::Network& node,
        const blockchain::Type chain,
        const filter::Type type,
        const UnallocatedCString& shutdown,
        const NotifyCallback& notify) noexcept
        : FilterDM(
              [&] { return db.FilterTip(type); }(),
              [&] {
                  auto promise = std::promise<filter::pHeader>{};
                  const auto tip = db.FilterTip(type);
                  promise.set_value(
                      db.LoadFilterHeader(type, tip.second->Bytes()));

                  return Finished{promise.get_future()};
              }(),
              "cfilter",
              20000,
              10000)
        , FilterWorker(api, 20ms)
        , db_(db)
        , header_(header)
        , node_(node)
        , chain_(chain)
        , type_(type)
        , notify_(notify)
    {
        init_executor({shutdown});
    }

    ~FilterDownloader() { signal_shutdown().get(); }

private:
    friend FilterDM;
    friend FilterWorker;

    const internal::FilterDatabase& db_;
    const HeaderOracle& header_;
    const internal::Network& node_;
    const blockchain::Type chain_;
    const filter::Type type_;
    const NotifyCallback& notify_;

    auto batch_ready() const noexcept -> void
    {
        node_.JobReady(internal::PeerManager::Task::JobAvailableCfilters);
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
    auto update_tip(const Position& position, const filter::pHeader&)
        const noexcept -> void
    {
        const auto saved = db_.SetFilterTip(type_, position);

        OT_ASSERT(saved);

        LogDetail()(DisplayString(chain_))(" cfilter chain updated to height ")(
            position.first)
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
            body.at(1).as<block::Height>(), api_.Factory().Data(body.at(2))};
        auto promise = std::promise<filter::pHeader>{};
        promise.set_value(api_.Factory().Data(body.at(3)));
        Reset(position, promise.get_future());
    }
    auto queue_processing(DownloadedData&& data) noexcept -> void
    {
        if (0 == data.size()) { return; }

        auto filters = UnallocatedVector<internal::FilterDatabase::Filter>{};

        for (const auto& task : data) {
            const auto& prior = task->previous_.get();
            auto& gcs =
                const_cast<std::unique_ptr<const GCS>&>(task->data_.get());
            const auto block = task->position_.second->Bytes();
            const auto expected = db_.LoadFilterHash(type_, block);

            if (expected == gcs->Hash()) {
                task->process(gcs->Header(prior->Bytes()));
                filters.emplace_back(block, gcs.release());
            } else {
                LogError()("Filter for block ")(
                    task->position_.second->asHex())(" at height ")(
                    task->position_.first)(
                    " does not match header. Received: ")(gcs->Hash()->asHex())(
                    " expected: ")(expected->asHex())
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
