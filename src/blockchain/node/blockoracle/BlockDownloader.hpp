// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <exception>
#include <future>
#include <memory>
#include <string_view>

#include "blockchain/DownloadManager.hpp"
#include "blockchain/DownloadTask.hpp"
#include "core/Worker.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Block;
}  // namespace bitcoin
}  // namespace block

namespace node
{
namespace blockoracle
{
class BlockDownloader;
}  // namespace blockoracle

namespace internal
{
struct BlockDatabase;
struct Network;
}  // namespace internal

class HeaderOracle;
}  // namespace node
}  // namespace blockchain

namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::blockoracle
{
using BlockDMBlock = download::
    Manager<BlockDownloader, std::shared_ptr<const block::bitcoin::Block>, int>;
using BlockWorkerBlock = Worker<BlockDownloader, api::Session>;

class BlockDownloader : public BlockDMBlock, public BlockWorkerBlock
{
public:
    enum class Work : OTZMQWorkType {
        shutdown = value(WorkType::Shutdown),
        block = value(WorkType::BlockchainNewHeader),
        reorg = value(WorkType::BlockchainReorg),
        heartbeat = OT_ZMQ_HEARTBEAT_SIGNAL,
        statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
    };

    auto NextBatch() noexcept { return allocate_batch(0); }
    auto Shutdown() noexcept -> std::shared_future<void>;

    BlockDownloader(
        const api::Session& api,
        internal::BlockDatabase& db,
        const node::HeaderOracle& header,
        const internal::Network& node,
        const blockchain::Type chain,
        const std::string_view shutdown) noexcept;

    ~BlockDownloader();

private:
    friend BlockDMBlock;
    friend BlockWorkerBlock;

    internal::BlockDatabase& db_;
    const node::HeaderOracle& header_;
    const internal::Network& node_;
    const blockchain::Type chain_;
    OTZMQPublishSocket socket_;

    auto batch_ready() const noexcept -> void;
    auto batch_size(const std::size_t in) const noexcept -> std::size_t;
    auto check_task(TaskType& task) const noexcept -> void;
    auto trigger_state_machine() const noexcept -> void;
    auto update_tip(const Position& position, const int&) const noexcept
        -> void;

    auto pipeline(const network::zeromq::Message& in) noexcept -> void;
    auto process_position(const network::zeromq::Message& in) noexcept -> void;
    auto process_position() noexcept -> void;
    auto queue_processing(DownloadedData&& data) noexcept -> void;
    auto shutdown(std::promise<void>& promise) noexcept -> void;
};
}  // namespace opentxs::blockchain::node::blockoracle
