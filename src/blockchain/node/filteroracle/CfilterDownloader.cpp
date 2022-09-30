// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "blockchain/node/filteroracle/CfilterDownloader.hpp"  // IWYU pragma: associated

#include <atomic>
#include <chrono>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>

#include "blockchain/node/filteroracle/Shared.hpp"
#include "internal/blockchain/database/Cfilter.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/Types.hpp"
#include "internal/blockchain/node/filteroracle/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/GCS.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Hash.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::node::filteroracle
{
CfilterDownloader::CfilterDownloader(std::shared_ptr<Shared> shared) noexcept
    : FilterDM(
          [shared] { return shared->CfilterTip(); }(),
          [shared] {
              auto promise = std::promise<cfilter::Header>{};
              const auto& type = shared->default_type_;
              const auto tip = shared->CfilterTip(type);
              promise.set_value(shared->LoadCfheader(type, tip.hash_));

              return Finished{promise.get_future()};
          }(),
          "cfilter",
          20000,
          10000)
    , FilterWorker(
          shared->api_,
          20ms,
          "blockchain::node::CfilterDownloader",
          {},
          {
              {shared->node_.Internal().Endpoints().cfilter_downloader_pull_,
               network::zeromq::socket::Direction::Bind},
          })
    , shared_p_(std::move(shared))
    , shared_(*shared_p_)
{
    init_executor(
        {shared_.node_.Internal().Endpoints().shutdown_publish_.c_str()});
}

auto CfilterDownloader::batch_ready() const noexcept -> void
{
    shared_.node_.Internal().JobReady(PeerManagerJobs::JobAvailableCfilters);
}

auto CfilterDownloader::batch_size(std::size_t in) const noexcept -> std::size_t
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

auto CfilterDownloader::check_task(TaskType&) const noexcept -> void {}

auto CfilterDownloader::pipeline(const network::zeromq::Message& in) noexcept
    -> void
{
    if (false == running_.load()) { return; }

    const auto body = in.Body();

    OT_ASSERT(0 < body.size());

    const auto work = [&] {
        try {

            return body.at(0).as<DownloadJob>();
        } catch (...) {

            OT_FAIL;
        }
    }();

    switch (work) {
        case DownloadJob::shutdown: {
            shutdown(shutdown_promise_);
        } break;
        case DownloadJob::reset_filter_tip: {
            process_reset(in);
        } break;
        case DownloadJob::heartbeat: {
            UpdatePosition(shared_.CfheaderTip());
            run_if_enabled();
        } break;
        case DownloadJob::statemachine: {
            run_if_enabled();
        } break;
        case DownloadJob::block:
        case DownloadJob::reorg:
        case DownloadJob::full_block:
        default: {
            OT_FAIL;
        }
    }
}

auto CfilterDownloader::process_reset(
    const network::zeromq::Message& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(3 < body.size());

    auto position = Position{
        body.at(1).as<block::Height>(), block::Hash{body.at(2).Bytes()}};
    auto promise = std::promise<cfilter::Header>{};
    promise.set_value(body.at(3).Bytes());
    Reset(position, promise.get_future());
}

auto CfilterDownloader::queue_processing(DownloadedData&& data) noexcept -> void
{
    if (0 == data.size()) { return; }

    auto filters = Vector<database::Cfilter::CFilterParams>{};
    auto handle = shared_.Lock();
    auto& d = *handle;

    for (const auto& task : data) {
        const auto& priorCfheader = task->previous_.get();
        auto& cfilter = const_cast<GCS&>(task->data_.get());
        const auto& block = task->position_.hash_;
        const auto expected = shared_.LoadCfilterHash(block, d);

        if (expected == cfilter.Hash()) {
            task->process(cfilter.Header(priorCfheader));
            filters.emplace_back(block, std::move(cfilter));
        } else {
            LogError()("Filter for block ")(task->position_)(
                " does not match header. Received: ")(cfilter.Hash().asHex())(
                " expected: ")(expected.asHex())
                .Flush();
            task->redownload();
            break;
        }
    }

    const auto saved = shared_.StoreCfilters(std::move(filters), d);

    OT_ASSERT(saved);
}

auto CfilterDownloader::shutdown(std::promise<void>& promise) noexcept -> void
{
    if (auto previous = running_.exchange(false); previous) {
        pipeline_.Close();
        shared_p_.reset();
        promise.set_value();
    }
}

auto CfilterDownloader::trigger_state_machine() const noexcept -> void
{
    trigger();
}

auto CfilterDownloader::UpdatePosition(const Position& pos) -> void
{
    try {
        auto current = known();
        auto hashes = shared_.header_.Ancestors(current, pos, 20000);

        OT_ASSERT(0 < hashes.size());

        auto prior = Previous{std::nullopt};
        auto& first = hashes.front();

        if (first != current) {
            auto promise = std::promise<cfilter::Header>{};
            auto cfheader = shared_.LoadCfheader(
                shared_.default_type_, first.hash_.Bytes());

            OT_ASSERT(false == cfheader.IsNull());

            promise.set_value(std::move(cfheader));
            prior.emplace(std::move(first), promise.get_future());
        }
        hashes.erase(hashes.begin());
        update_position(
            std::move(hashes), shared_.default_type_, std::move(prior));
    } catch (...) {
    }
}

auto CfilterDownloader::update_tip(
    const Position& position,
    const cfilter::Header&) const noexcept -> void
{
    const auto saved = shared_.SetCfilterTip(shared_.default_type_, position);

    OT_ASSERT(saved);

    LogDetail()(print(shared_.chain_))(" cfilter chain updated to height ")(
        position.height_)
        .Flush();
    shared_.UpdateCfilterTip(position);
}

CfilterDownloader::~CfilterDownloader() { signal_shutdown().get(); }
}  // namespace opentxs::blockchain::node::filteroracle
