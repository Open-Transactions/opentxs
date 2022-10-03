// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "blockchain/node/blockoracle/blockfetcher/Actor.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/shared_ptr.hpp>
#include <chrono>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>

#include "blockchain/node/blockoracle/blockfetcher/Shared.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/blockchain/database/Block.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/ScopeGuard.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::blockoracle
{
BlockFetcher::Actor::Actor(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const node::Manager> node,
    boost::shared_ptr<Shared> shared,
    network::zeromq::BatchID batchID,
    allocator_type alloc) noexcept
    : opentxs::Actor<BlockFetcher::Actor, BlockFetcherJob>(
          *api,
          LogTrace(),
          [&] {
              using opentxs::blockchain::print;
              auto out = CString{print(node->Internal().Chain()), alloc};
              out.append(" block fetcher");

              return out;
          }(),
          0ms,
          batchID,
          alloc,
          [&] {
              using Dir = network::zeromq::socket::Direction;
              auto sub = network::zeromq::EndpointArgs{alloc};
              sub.emplace_back(
                  CString{
                      api->Endpoints().Internal().BlockchainReportStatus(),
                      alloc},
                  Dir::Connect);
              sub.emplace_back(
                  CString{api->Endpoints().BlockchainReorg(), alloc},
                  Dir::Connect);
              sub.emplace_back(
                  CString{api->Endpoints().Shutdown(), alloc}, Dir::Connect);
              sub.emplace_back(
                  node->Internal().Endpoints().shutdown_publish_, Dir::Connect);

              return sub;
          }(),
          [&] {
              using Dir = network::zeromq::socket::Direction;
              auto pull = network::zeromq::EndpointArgs{alloc};
              pull.emplace_back(
                  node->Internal().Endpoints().block_fetcher_pull_, Dir::Bind);

              return pull;
          }(),
          {},
          [&] {
              auto out = Vector<network::zeromq::SocketData>{alloc};
              using Socket = network::zeromq::socket::Type;
              using Args = network::zeromq::EndpointArgs;
              using Dir = network::zeromq::socket::Direction;
              out.emplace_back(std::make_pair<Socket, Args>(
                  Socket::Publish,
                  {
                      {node->Internal()
                           .Endpoints()
                           .block_fetcher_job_ready_publish_,
                       Dir::Bind},
                  }));  // NOTE job_ready_
              out.emplace_back(std::make_pair<Socket, Args>(
                  Socket::Publish,
                  {
                      {node->Internal().Endpoints().block_tip_publish_,
                       Dir::Bind},
                  }));  // NOTE tip_updated_
              out.emplace_back(std::make_pair<Socket, Args>(
                  Socket::Push,
                  {
                      {CString{
                           api->Endpoints()
                               .Internal()
                               .BlockchainMessageRouter(),
                           alloc},
                       Dir::Connect},
                  }));  // NOTE to_blockchain_api_

              return out;
          }())
    , api_p_(std::move(api))
    , node_p_(std::move(node))
    , shared_(std::move(shared))
    , api_(*api_p_)
    , node_(*node_p_)
    , header_oracle_(node_.HeaderOracle())
    , job_ready_(pipeline_.Internal().ExtraSocket(0))
    , tip_updated_(pipeline_.Internal().ExtraSocket(1))
    , to_blockchain_api_(pipeline_.Internal().ExtraSocket(2))
    , chain_(node_.Internal().Chain())
    , data_(shared_->data_)
    , job_available_(api_.Network().Asio().Internal().GetTimer())
{
}

auto BlockFetcher::Actor::broadcast_tip(
    Shared::Data& data,
    const block::Position& tip) noexcept -> void
{
    log_(OT_PRETTY_CLASS())(name_)(": block tip updated to ")(tip).Flush();
    const auto saved = data.db_.SetBlockTip(tip);

    OT_ASSERT(saved);

    broadcast_tip(tip);
}

auto BlockFetcher::Actor::broadcast_tip(const block::Position& tip) noexcept
    -> void
{
    tip_updated_.SendDeferred(
        [&] {
            auto msg = MakeWork(OT_ZMQ_NEW_FULL_BLOCK_SIGNAL);
            msg.AddFrame(tip.height_);
            msg.AddFrame(tip.hash_);

            return msg;
        }(),
        __FILE__,
        __LINE__);
    to_blockchain_api_.SendDeferred(
        [&] {
            auto msg = MakeWork(WorkType::BlockchainBlockOracleProgress);
            msg.AddFrame(chain_);
            msg.AddFrame(tip.height_);
            msg.AddFrame(tip.hash_);

            return msg;
        }(),
        __FILE__,
        __LINE__);
}

auto BlockFetcher::Actor::do_shutdown() noexcept -> void
{
    node_p_.reset();
    api_p_.reset();
}

auto BlockFetcher::Actor::do_startup() noexcept -> bool
{
    if ((api_.Internal().ShuttingDown()) || (node_.Internal().ShuttingDown())) {
        return true;
    }

    const auto& log = log_;

    {
        auto handle = data_.lock();
        auto& data = *handle;
        auto tip = data.db_.BlockTip();

        OT_ASSERT(0 <= tip.height_);

        log(OT_PRETTY_CLASS())(name_)(": block tip from database is ")(tip)
            .Flush();

        if (auto r = header_oracle_.CalculateReorg(tip); false == r.empty()) {
            const auto& last = r.back();

            OT_ASSERT(0 < last.height_);

            const auto header = header_oracle_.LoadHeader(last.hash_);

            OT_ASSERT(header);

            tip = {last.height_ - 1, header->ParentHash()};
            log(OT_PRETTY_CLASS())(name_)(": block tip adjusted to ")(
                tip)(" due to unprocessed reorg")
                .Flush();
        }

        while (tip.height_ > 0) {
            if (auto block = data.db_.BlockLoadBitcoin(tip.hash_); block) {

                break;
            } else {
                log(OT_PRETTY_CLASS())(name_)(": unable to load block ")(tip)
                    .Flush();
                tip = header_oracle_.GetPosition(tip.height_ - 1);
                log(OT_PRETTY_CLASS())(name_)(": tip reverted to ")(tip)
                    .Flush();
            }
        }

        if (data.ReviseTip(tip)) { broadcast_tip(data, tip); }

        LogConsole()(print(chain_))(" block tip is ")(tip).Flush();
    }

    do_work();

    return false;
}

auto BlockFetcher::Actor::pipeline(const Work work, Message&& msg) noexcept
    -> void
{
    switch (work) {
        case Work::heartbeat: {
            process_heartbeat(std::move(msg));
        } break;
        case Work::reorg: {
            process_reorg(std::move(msg));
        } break;
        case Work::header: {
            do_work();
        } break;
        case Work::report: {
            process_report(std::move(msg));
        } break;
        case Work::shutdown:
        case Work::init:
        case Work::statemachine: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(": unhandled message type ")(
                print(work))
                .Abort();
        }
        default: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(": unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Abort();
        }
    }
}

auto BlockFetcher::Actor::process_heartbeat(Message&& msg) noexcept -> void
{
    auto handle = data_.lock();
    const auto& data = *handle;

    if (data.JobAvailable()) { publish_job_ready(); }
}

auto BlockFetcher::Actor::process_reorg(Message&& msg) noexcept -> void
{
    const auto body = msg.Body();

    OT_ASSERT(5 < body.size());

    if (body.at(1).as<decltype(chain_)>() != chain_) { return; }

    auto ancestor =
        block::Position{body.at(3).as<block::Height>(), body.at(2).Bytes()};

    {
        auto handle = data_.lock();
        auto& data = *handle;
        auto tip{data.Tip()};
        data.PruneStale(ancestor);

        if (tip.height_ > ancestor.height_) {
            tip = std::move(ancestor);
        } else if ((tip.height_ == ancestor.height_) && (tip != ancestor)) {
            OT_ASSERT(0 < tip.height_);

            tip = header_oracle_.GetPosition(tip.height_ - 1);
        }

        if (data.ReviseTip(tip)) {
            broadcast_tip(data, tip);
        } else {
            log_(OT_PRETTY_CLASS())(name_)(": new tip ")(
                tip)(" matches previous value")
                .Flush();
        }
    }

    do_work();
}

auto BlockFetcher::Actor::process_report(Message&& msg) noexcept -> void
{
    broadcast_tip(data_.lock()->Tip());
}

auto BlockFetcher::Actor::publish_job_ready() noexcept -> void
{
    log_(OT_PRETTY_CLASS())(name_)(
        ": notifying listeners about new block download jobs")
        .Flush();
    job_ready_.SendDeferred(
        MakeWork(OT_ZMQ_BLOCK_FETCH_JOB_AVAILABLE), __FILE__, __LINE__);
    reset_timer(15s, job_available_, Work::heartbeat);
}

auto BlockFetcher::Actor::update_tip(Shared::Data& data) noexcept -> void
{
    if (auto tip = data.UpdateTip(); tip.has_value()) {
        broadcast_tip(data, *tip);
    }
}

auto BlockFetcher::Actor::work() noexcept -> bool
{
    log_(OT_PRETTY_CLASS())(name_)(": checking for new blocks").Flush();
    auto handle = data_.lock();
    auto& data = *handle;
    const auto start = data.LastBlock();
    log_(OT_PRETTY_CLASS())(name_)(": have blocks up to ")(start).Flush();
    auto post = ScopeGuard{[&] {
        if (data.JobAvailable()) {
            publish_job_ready();
        } else {
            job_available_.Cancel();
        }
    }};
    auto add = [&] {
        auto alloc = get_allocator();
        auto out = Shared::Data::NewBlocks{alloc};
        // TODO HeaderOracle should have a BestPositions function
        auto hashes = header_oracle_.BestHashes(start + 1, 0_uz, alloc);
        out.reserve(hashes.size());
        auto height{start};

        for (auto& hash : hashes) {
            const auto exists = data.db_.BlockExists(hash);
            out.emplace_back(block::Position{++height, std::move(hash)}, [&] {
                if (exists) {

                    return Status::success;
                } else {

                    return Status::pending;
                }
            }());
        }

        return out;
    }();

    if (add.empty()) {
        log_(OT_PRETTY_CLASS())(name_)(
            ": block tip caught up to block header tip")
            .Flush();
    } else {
        data.AddBlocks(std::move(add));
    }

    update_tip(data);

    return false;
}

BlockFetcher::Actor::~Actor() = default;
}  // namespace opentxs::blockchain::node::blockoracle
