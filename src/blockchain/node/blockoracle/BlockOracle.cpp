// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/blockoracle/BlockOracle.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <chrono>
#include <memory>
#include <utility>

#include "blockchain/node/blockoracle/BlockBatch.hpp"
#include "blockchain/node/blockoracle/BlockDownloader.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/node/Factory.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/ScopeGuard.hpp"
#include "util/Work.hpp"

namespace opentxs::factory
{
auto BlockOracle(
    const api::Session& api,
    const blockchain::node::internal::Network& node,
    const blockchain::node::HeaderOracle& header,
    blockchain::node::internal::BlockDatabase& db,
    const blockchain::Type chain,
    const UnallocatedCString& shutdown) noexcept
    -> blockchain::node::internal::BlockOracle
{
    using ReturnType = blockchain::node::internal::BlockOracle::Imp;
    const auto& zmq = api.Network().ZeroMQ().Internal();
    const auto batchID = zmq.PreallocateBatch();
    // TODO the version of libc++ present in android ndk 23.0.7599858
    // has a broken std::allocate_shared function so we're using
    // boost::shared_ptr instead of std::shared_ptr

    return boost::allocate_shared<ReturnType>(
        alloc::PMR<ReturnType>{zmq.Alloc(batchID)},
        api,
        node,
        header,
        db,
        chain,
        shutdown,
        batchID);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::node
{
auto print(BlockOracleJobs state) noexcept -> std::string_view
{
    using namespace std::literals;

    try {
        static const auto map = Map<BlockOracleJobs, std::string_view>{
            {BlockOracleJobs::shutdown, "shutdown"sv},
            {BlockOracleJobs::request_blocks, "request_blocks"sv},
            {BlockOracleJobs::process_block, "process_block"sv},
            {BlockOracleJobs::start_downloader, "start_downloader"sv},
            {BlockOracleJobs::init, "init"sv},
            {BlockOracleJobs::statemachine, "statemachine"sv},
        };

        return map.at(state);
    } catch (...) {
        LogError()(__FUNCTION__)(": invalid BlockOracleJobs: ")(
            static_cast<OTZMQWorkType>(state))
            .Flush();

        OT_FAIL;
    }
}
}  // namespace opentxs::blockchain::node

namespace opentxs::blockchain::node::internal
{
BlockOracle::Imp::Imp(
    const api::Session& api,
    const internal::Network& node,
    const node::HeaderOracle& header,
    internal::BlockDatabase& db,
    const blockchain::Type chain,
    const std::string_view parent,
    const network::zeromq::BatchID batch,
    CString&& submitEndpoint,
    allocator_type alloc) noexcept
    : Actor(
          api,
          LogTrace(),
          [&] {
              using namespace std::literals;
              auto out = CString{alloc};
              out.append(print(chain));
              out.append(" block oracle"sv);

              return out;
          }(),
          500ms,
          batch,
          alloc,
          [&] {
              auto subscribe = network::zeromq::EndpointArgs{alloc};
              subscribe.emplace_back(parent, Direction::Connect);

              return subscribe;
          }(),
          [&] {
              auto pull = network::zeromq::EndpointArgs{alloc};
              pull.emplace_back(submitEndpoint, Direction::Bind);

              return pull;
          }())
    , api_(api)
    , node_(node)
    , db_(db)
    , submit_endpoint_(std::move(submitEndpoint))
    , validator_(get_validator(chain, header))
    , block_downloader_([&]() -> std::unique_ptr<blockoracle::BlockDownloader> {
        using Policy = database::BlockStorage;

        if (Policy::All != db.BlockPolicy()) { return nullptr; }

        return std::make_unique<blockoracle::BlockDownloader>(
            api_, db, header, node_, chain, parent);
    }())
    , cache_(api, node, db, chain, alloc)
{
    OT_ASSERT(validator_);
}

BlockOracle::Imp::Imp(
    const api::Session& api,
    const internal::Network& node,
    const node::HeaderOracle& header,
    internal::BlockDatabase& db,
    const blockchain::Type chain,
    const std::string_view parent,
    const network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : Imp(api,
          node,
          header,
          db,
          chain,
          parent,
          batch,
          network::zeromq::MakeArbitraryInproc(alloc.resource()),
          alloc)
{
}

auto BlockOracle::Imp::do_shutdown() noexcept -> void
{
    if (block_downloader_) { block_downloader_->Shutdown(); }

    cache_.lock()->Shutdown();
}

auto BlockOracle::Imp::do_startup() noexcept -> void
{
    // NOTE no action required
}

auto BlockOracle::Imp::GetBlockBatch(boost::shared_ptr<Imp> me) const noexcept
    -> BlockBatch
{
    auto alloc = alloc::PMR<BlockBatch::Imp>{get_allocator()};
    auto [id, hashes] = cache_.lock()->GetBatch(alloc);
    const auto batchID{id};  // TODO c++20 lambda capture structured binding
    auto* imp = alloc.allocate(1);
    alloc.construct(
        imp,
        id,
        std::move(hashes),
        [me](const auto bytes) { me->cache_.lock()->ReceiveBlock(bytes); },
        std::make_shared<ScopeGuard>(
            [me, batchID] { me->cache_.lock()->FinishBatch(batchID); }));

    return imp;
}

auto BlockOracle::Imp::GetBlockJob() const noexcept -> BlockJob
{
    if (block_downloader_) {

        return block_downloader_->NextBatch();
    } else {

        return {};
    }
}

auto BlockOracle::Imp::Heartbeat() const noexcept -> void
{
    trigger();

    if (block_downloader_) { block_downloader_->Heartbeat(); }
}

auto BlockOracle::Imp::LoadBitcoin(const block::Hash& block) const noexcept
    -> BitcoinBlockResult
{
    auto output = cache_.lock()->Request(block);
    trigger();

    return output;
}

auto BlockOracle::Imp::LoadBitcoin(
    const Vector<block::Hash>& hashes) const noexcept -> BitcoinBlockResults
{
    auto output = cache_.lock()->Request(hashes);
    trigger();

    OT_ASSERT(hashes.size() == output.size());

    return output;
}

auto BlockOracle::Imp::pipeline(const Work work, Message&& msg) noexcept -> void
{
    switch (work) {
        case Work::shutdown: {
            shutdown_actor();
        } break;
        case Work::request_blocks: {
            cache_.lock()->ProcessBlockRequests(std::move(msg));
        } break;
        case Work::process_block: {
            const auto body = msg.Body();

            if (1 >= body.size()) {
                LogError()(OT_PRETTY_CLASS())("No block").Flush();

                OT_FAIL;
            }

            cache_.lock()->ReceiveBlock(body.at(1));
            do_work();
        } break;
        case Work::start_downloader: {
            if (block_downloader_) { block_downloader_->Start(); }
        } break;
        case Work::init: {
            do_init();
        } break;
        case Work::statemachine: {
            do_work();
        } break;
        default: {
            LogError()(OT_PRETTY_CLASS())(name_)(" unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Flush();

            OT_FAIL;
        }
    }
}

auto BlockOracle::Imp::StartDownloader() noexcept -> void
{
    pipeline_.Push(MakeWork(Work::start_downloader));
}

auto BlockOracle::Imp::SubmitBlock(const ReadView in) const noexcept -> void
{
    pipeline_.Push([&] {
        auto out = MakeWork(Task::process_block);
        out.AddFrame(in.data(), in.size());

        return out;
    }());
}

auto BlockOracle::Imp::work() noexcept -> bool
{
    return cache_.lock()->StateMachine();
}

BlockOracle::Imp::~Imp() = default;
}  // namespace opentxs::blockchain::node::internal

namespace opentxs::blockchain::node::internal
{
BlockOracle::BlockOracle(boost::shared_ptr<Imp>&& imp) noexcept
    : imp_(std::move(imp))
{
    OT_ASSERT(nullptr != imp_);

    imp_->Init(imp_);
}

auto BlockOracle::DownloadQueue() const noexcept -> std::size_t
{
    return imp_->DownloadQueue();
}

auto BlockOracle::Endpoint() const noexcept -> std::string_view
{
    return imp_->Endpoint();
}

auto BlockOracle::GetBlockBatch() const noexcept -> BlockBatch
{
    return imp_->GetBlockBatch(imp_);
}

auto BlockOracle::GetBlockJob() const noexcept -> BlockJob
{
    return imp_->GetBlockJob();
}

auto BlockOracle::Heartbeat() const noexcept -> void
{
    return imp_->Heartbeat();
}

auto BlockOracle::Init() noexcept -> void { imp_->StartDownloader(); }

auto BlockOracle::LoadBitcoin(const block::Hash& block) const noexcept
    -> BitcoinBlockResult
{
    return imp_->LoadBitcoin(block);
}

auto BlockOracle::LoadBitcoin(const Vector<block::Hash>& hashes) const noexcept
    -> BitcoinBlockResults
{
    return imp_->LoadBitcoin(hashes);
}

auto BlockOracle::Shutdown() noexcept -> void { imp_->Shutdown(); }

auto BlockOracle::SubmitBlock(const ReadView in) const noexcept -> void
{
    return imp_->SubmitBlock(in);
}

auto BlockOracle::Tip() const noexcept -> block::Position
{
    return imp_->Tip();
}

auto BlockOracle::Validate(const block::bitcoin::Block& block) const noexcept
    -> bool
{
    return imp_->Validate(block);
}

BlockOracle::~BlockOracle() { imp_->Shutdown(); }
}  // namespace opentxs::blockchain::node::internal
