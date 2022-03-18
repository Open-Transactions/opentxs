// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "blockchain/node/BlockOracle.hpp"  // IWYU pragma: associated

#include <atomic>
#include <memory>

#include "blockchain/node/blockoracle/BlockDownloader.hpp"
#include "core/Worker.hpp"
#include "internal/api/network/Blockchain.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/node/Factory.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/node/BlockOracle.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BlockOracle(
    const api::Session& api,
    const blockchain::node::internal::Network& node,
    const blockchain::node::HeaderOracle& header,
    blockchain::node::internal::BlockDatabase& db,
    const blockchain::Type chain,
    const UnallocatedCString& shutdown) noexcept
    -> std::unique_ptr<blockchain::node::internal::BlockOracle>
{
    using ReturnType = blockchain::node::implementation::BlockOracle;

    return std::make_unique<ReturnType>(api, node, header, db, chain, shutdown);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::node::implementation
{
BlockOracle::BlockOracle(
    const api::Session& api,
    const internal::Network& node,
    const HeaderOracle& header,
    internal::BlockDatabase& db,
    const blockchain::Type chain,
    const UnallocatedCString& shutdown) noexcept
    : Worker(api, 500ms)
    , node_(node)
    , db_(db)
    , lock_()
    , cache_(
          api,
          node,
          db,
          api_.Network().Blockchain().Internal().BlockAvailable(),
          api_.Network().Blockchain().Internal().BlockQueueUpdate(),
          chain)
    , block_downloader_([&]() -> std::unique_ptr<BlockDownloader> {
        using Policy = database::BlockStorage;

        if (Policy::All != db.BlockPolicy()) { return nullptr; }

        return std::make_unique<BlockDownloader>(
            api_, db, header, node_, chain, shutdown);
    }())
    , validator_(get_validator(chain, header))
{
    OT_ASSERT(validator_);

    init_executor({shutdown});
}

auto BlockOracle::GetBlockJob() const noexcept -> BlockJob
{
    if (block_downloader_) {

        return block_downloader_->NextBatch();
    } else {

        return {};
    }
}

auto BlockOracle::Heartbeat() const noexcept -> void
{
    trigger();

    if (block_downloader_) { block_downloader_->Heartbeat(); }
}

auto BlockOracle::Init() noexcept -> void
{
    if (block_downloader_) { block_downloader_->Start(); }
}

auto BlockOracle::LoadBitcoin(const block::Hash& block) const noexcept
    -> BitcoinBlockFuture
{
    auto output = cache_.Request(block);
    trigger();

    return output;
}

auto BlockOracle::LoadBitcoin(const BlockHashes& hashes) const noexcept
    -> BitcoinBlockFutures
{
    auto output = cache_.Request(hashes);
    trigger();

    OT_ASSERT(hashes.size() == output.size());

    return output;
}

auto BlockOracle::pipeline(const zmq::Message& in) noexcept -> void
{
    if (false == running_.load()) { return; }

    const auto body = in.Body();

    if (1 > body.size()) {
        LogError()(OT_PRETTY_CLASS())("Invalid message").Flush();

        OT_FAIL;
    }

    const auto task = [&] {
        try {

            return body.at(0).as<Task>();
        } catch (...) {

            OT_FAIL;
        }
    }();

    switch (task) {
        case Task::ProcessBlock: {
            if (2 > body.size()) {
                LogError()(OT_PRETTY_CLASS())("No block").Flush();

                OT_FAIL;
            }

            cache_.ReceiveBlock(body.at(1));
            [[fallthrough]];
        }
        case Task::StateMachine: {
            do_work();
        } break;
        case Task::Shutdown: {
            shutdown(shutdown_promise_);
        } break;
        default: {
            LogError()(OT_PRETTY_CLASS())("Unhandled type").Flush();

            OT_FAIL;
        }
    }
}

auto BlockOracle::shutdown(std::promise<void>& promise) noexcept -> void
{
    if (auto previous = running_.exchange(false); previous) {
        pipeline_.Close();

        if (block_downloader_) { block_downloader_->Shutdown(); }

        cache_.Shutdown();
        promise.set_value();
    }
}

auto BlockOracle::state_machine() noexcept -> bool
{
    if (false == running_.load()) { return false; }

    return cache_.StateMachine();
}

auto BlockOracle::SubmitBlock(const ReadView in) const noexcept -> void
{
    auto work = MakeWork(Task::ProcessBlock);
    work.AddFrame(in.data(), in.size());
    pipeline_.Push(std::move(work));
}

BlockOracle::~BlockOracle() { signal_shutdown().get(); }
}  // namespace opentxs::blockchain::node::implementation
