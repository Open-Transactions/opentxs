// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "blockchain/node/filteroracle/CfheaderDownloader.hpp"  // IWYU pragma: associated

#include <atomic>
#include <chrono>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>

#include "blockchain/node/filteroracle/Shared.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/database/Cfilter.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/Types.hpp"
#include "internal/blockchain/node/filteroracle/Types.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/network/zeromq/socket/SocketType.hpp"
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Log.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::filteroracle
{
CfheaderDownloader::CfheaderDownloader(std::shared_ptr<Shared> shared) noexcept
    : HeaderDM(
          [shared] { return shared->CfheaderTip(); }(),
          [shared] {
              auto promise = std::promise<cfilter::Header>{};
              const auto& type = shared->default_type_;
              const auto tip = shared->CfheaderTip(type);
              promise.set_value(shared->LoadCfheader(type, tip.hash_));

              return Finished{promise.get_future()};
          }(),
          "cfheader",
          20000,
          10000)
    , HeaderWorker(
          shared->api_,
          20ms,
          "blockchain::node::CfheaderDownloader",
          {},
          {},
          {},
          {
              {network::zeromq::socket::Type::Push,
               {
                   {shared->node_.Internal()
                        .Endpoints()
                        .cfilter_downloader_pull_,
                    network::zeromq::socket::Direction::Connect},
               }},
          })
    , shared_p_(std::move(shared))
    , shared_(*shared_p_)
    , to_cfilter_(pipeline_.Internal().ExtraSocket(0))
{
    init_executor(
        {shared_.node_.Internal().Endpoints().shutdown_publish_.c_str(),
         UnallocatedCString{api_.Endpoints().BlockchainReorg()}});
}

auto CfheaderDownloader::batch_ready() const noexcept -> void
{
    shared_.node_.Internal().JobReady(PeerManagerJobs::JobAvailableCfheaders);
}

auto CfheaderDownloader::batch_size(const std::size_t in) const noexcept
    -> std::size_t
{
    if (in < 10) {

        return 1;
    } else if (in < 100) {

        return 10;
    } else if (in < 1000) {

        return 100;
    } else {

        return 2000;
    }
}

auto CfheaderDownloader::check_task(TaskType&) const noexcept -> void {}

auto CfheaderDownloader::NextBatch() noexcept -> BatchType
{
    return allocate_batch(shared_.default_type_);
}

auto CfheaderDownloader::pipeline(const network::zeromq::Message& in) noexcept
    -> void
{
    if (false == running_.load()) { return; }

    const auto body = in.Body();

    OT_ASSERT(1 <= body.size());

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
        case DownloadJob::block:
        case DownloadJob::reorg: {
            process_position(in);
            run_if_enabled();
        } break;
        case DownloadJob::reset_filter_tip: {
            process_reset(in);
        } break;
        case DownloadJob::heartbeat: {
            process_position();
            run_if_enabled();
        } break;
        case DownloadJob::statemachine: {
            run_if_enabled();
        } break;
        case DownloadJob::full_block:
        default: {
            OT_FAIL;
        }
    }
}

auto CfheaderDownloader::process_position(
    const network::zeromq::Message& in) noexcept -> void
{
    {
        const auto body = in.Body();

        OT_ASSERT(body.size() >= 4);

        const auto chain = body.at(1).as<blockchain::Type>();

        if (shared_.chain_ != chain) { return; }
    }

    process_position();
}

auto CfheaderDownloader::process_position() noexcept -> void
{
    auto current = known();
    auto hashes = shared_.header_.BestChain(current, 20000);

    OT_ASSERT(0 < hashes.size());

    auto prior = Previous{std::nullopt};
    {
        auto& first = hashes.front();

        if (first != current) {
            auto promise = std::promise<cfilter::Header>{};
            promise.set_value(shared_.LoadCfheader(
                shared_.default_type_, first.hash_.Bytes()));
            prior.emplace(std::move(first), promise.get_future());
        }
    }
    hashes.erase(hashes.begin());
    update_position(std::move(hashes), shared_.default_type_, std::move(prior));
}

auto CfheaderDownloader::process_reset(
    const network::zeromq::Message& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(3 < body.size());

    auto position =
        Position{body.at(1).as<block::Height>(), body.at(2).Bytes()};
    auto promise = std::promise<cfilter::Header>{};
    promise.set_value(body.at(3).Bytes());
    Reset(position, promise.get_future());
}

auto CfheaderDownloader::queue_processing(DownloadedData&& data) noexcept
    -> void
{
    if (0 == data.size()) { return; }

    const auto& previous = data.front()->previous_.get();
    auto hashes = Vector<cfilter::Hash>{};
    auto headers = Vector<database::Cfilter::CFHeaderParams>{};

    for (const auto& task : data) {
        const auto& hash = hashes.emplace_back(task->data_.get());
        auto header = blockchain::internal::FilterHashToHeader(
            api_, hash.Bytes(), task->previous_.get().Bytes());
        const auto& position = task->position_;
        const auto check = shared_.ValidateAgainstCheckpoint(position, header);

        if (check == position) {
            headers.emplace_back(position.hash_, header, hash);
            task->process(std::move(header));
        } else {
            const auto good = shared_.LoadCfheader(
                shared_.default_type_, check.hash_.Bytes());

            OT_ASSERT(false == good.IsNull());

            auto work = MakeWork(DownloadJob::reset_filter_tip);
            work.AddFrame(check.height_);
            work.AddFrame(check.hash_);
            work.AddFrame(good);
            pipeline_.Push(std::move(work));
        }
    }

    const auto saved = shared_.StoreCfheaders(
        shared_.default_type_, previous.Bytes(), std::move(headers));

    OT_ASSERT(saved);
}

auto CfheaderDownloader::shutdown(std::promise<void>& promise) noexcept -> void
{
    if (auto previous = running_.exchange(false); previous) {
        pipeline_.Close();
        shared_p_.reset();
        promise.set_value();
    }
}

auto CfheaderDownloader::trigger_state_machine() const noexcept -> void
{
    trigger();
}

auto CfheaderDownloader::update_tip(
    const Position& position,
    const cfilter::Header&) const noexcept -> void
{
    const auto saved = shared_.SetCfheaderTip(shared_.default_type_, position);

    OT_ASSERT(saved);

    LogDetail()(print(shared_.chain_))(" cfheader chain updated to height ")(
        position.height_)
        .Flush();
    to_cfilter_.SendDeferred(
        MakeWork(DownloadJob::heartbeat), __FILE__, __LINE__, true);
}

CfheaderDownloader::~CfheaderDownloader() { signal_shutdown().get(); }
}  // namespace opentxs::blockchain::node::filteroracle
