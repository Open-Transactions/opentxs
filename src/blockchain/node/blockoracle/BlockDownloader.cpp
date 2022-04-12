// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/blockoracle/BlockDownloader.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <atomic>
#include <chrono>
#include <optional>
#include <string_view>
#include <utility>

#include "internal/api/session/Endpoints.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::node::blockoracle
{
BlockDownloader::BlockDownloader(
    const api::Session& api,
    internal::BlockDatabase& db,
    const node::HeaderOracle& header,
    const internal::Network& node,
    const blockchain::Type chain,
    const std::string_view shutdown) noexcept
    : BlockDMBlock(
          [&] { return db.BlockTip(); }(),
          [&] {
              auto promise = std::promise<int>{};
              promise.set_value(0);

              return Finished{promise.get_future()};
          }(),
          "block",
          2000,
          1000)
    , BlockWorkerBlock(api, 20ms)
    , db_(db)
    , header_(header)
    , node_(node)
    , chain_(chain)
    , socket_(api_.Network().ZeroMQ().PublishSocket())
{
    init_executor(
        {UnallocatedCString{shutdown},
         UnallocatedCString{api_.Endpoints().BlockchainReorg()}});
    auto zmq = socket_->Start(
        api_.Endpoints().Internal().BlockchainBlockUpdated(chain_).data());

    OT_ASSERT(zmq);
}

auto BlockDownloader::batch_ready() const noexcept -> void
{
    node_.JobReady(internal::PeerManager::Task::JobAvailableBlock);
}

auto BlockDownloader::batch_size(const std::size_t in) const noexcept
    -> std::size_t
{
    const auto limit = params::Chains().at(chain_).block_download_batch_;

    if (in < 10) {

        return std::min<std::size_t>(1, limit);
    } else if (in < 100) {

        return std::min<std::size_t>(10, limit);
    } else if (in < 1000) {

        return std::min<std::size_t>(100, limit);
    } else {

        return limit;
    }
}

auto BlockDownloader::check_task(TaskType& task) const noexcept -> void
{
    const auto& hash = task.position_.second;

    if (auto block = db_.BlockLoadBitcoin(hash); bool(block)) {
        task.download(std::move(block));
    }
}

auto BlockDownloader::pipeline(const network::zeromq::Message& in) noexcept
    -> void
{
    if (false == running_.load()) { return; }

    const auto body = in.Body();

    OT_ASSERT(1 <= body.size());

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
        case Work::block:
        case Work::reorg: {
            if (dm_enabled()) { process_position(in); }

            run_if_enabled();
        } break;
        case Work::heartbeat: {
            if (dm_enabled()) { process_position(); }

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

auto BlockDownloader::process_position(
    const network::zeromq::Message& in) noexcept -> void
{
    {
        const auto body = in.Body();

        OT_ASSERT(body.size() >= 4);

        const auto chain = body.at(1).as<blockchain::Type>();

        if (chain_ != chain) { return; }
    }

    process_position();
}

auto BlockDownloader::process_position() noexcept -> void
{
    auto current = known();
    auto hashes = header_.BestChain(current, 2000);

    OT_ASSERT(0 < hashes.size());

    auto prior = Previous{std::nullopt};
    {
        auto& first = hashes.front();

        if (first != current) {
            auto promise = std::promise<int>{};
            promise.set_value(0);
            prior.emplace(std::move(first), promise.get_future());
        }
    }
    hashes.erase(hashes.begin());
    update_position(std::move(hashes), 0, std::move(prior));
}

auto BlockDownloader::queue_processing(DownloadedData&& data) noexcept -> void
{
    if (0 == data.size()) { return; }

    for (const auto& task : data) {
        const auto& pBlock = task->data_.get();

        OT_ASSERT(pBlock);

        const auto& block = *pBlock;

        // TODO implement batch db updates
        if (db_.BlockStore(block)) {
            task->process(0);
        } else {
            task->redownload();
            break;
        }
    }
}

auto BlockDownloader::Shutdown() noexcept -> std::shared_future<void>
{
    return signal_shutdown();
}

auto BlockDownloader::shutdown(std::promise<void>& promise) noexcept -> void
{
    if (auto previous = running_.exchange(false); previous) {
        pipeline_.Close();
        promise.set_value();
    }
}

auto BlockDownloader::trigger_state_machine() const noexcept -> void
{
    trigger();
}

auto BlockDownloader::update_tip(const Position& position, const int&)
    const noexcept -> void
{
    const auto saved = db_.SetBlockTip(position);

    OT_ASSERT(saved);

    LogDetail()(print(chain_))(" block chain updated to height ")(
        position.first)
        .Flush();
    auto work = MakeWork(OT_ZMQ_NEW_FULL_BLOCK_SIGNAL);
    work.AddFrame(position.first);
    work.AddFrame(position.second);
    socket_->Send(std::move(work));
}

BlockDownloader::~BlockDownloader() { signal_shutdown().get(); }
}  // namespace opentxs::blockchain::node::blockoracle
