// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/circular_buffer.hpp>
#include <atomic>
#include <chrono>
#include <deque>
#include <functional>
#include <future>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "1_Internal.hpp"
#include "core/Worker.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/node/BlockOracle.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/JobCounter.hpp"
#include "util/Work.hpp"

namespace opentxs
{
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
class HeaderOracle;
}  // namespace node

class GCS;
}  // namespace blockchain

namespace network
{
namespace p2p
{
class Data;
}  // namespace p2p

namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket

class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace zmq = opentxs::network::zeromq;

namespace opentxs::blockchain::node::implementation
{
class FilterOracle final : virtual public node::internal::FilterOracle
{
public:
    class BlockIndexer;
    class FilterDownloader;
    class HeaderDownloader;
    class SyncIndexer;
    struct BlockIndexerData;
    struct SyncClientFilterData;

    using NotifyCallback =
        std::function<void(const filter::Type, const block::Position&)>;

    enum class Work : OTZMQWorkType {
        shutdown = value(WorkType::Shutdown),
        block = value(WorkType::BlockchainNewHeader),
        reorg = value(WorkType::BlockchainReorg),
        reset_filter_tip = OT_ZMQ_INTERNAL_SIGNAL + 0,
        heartbeat = OT_ZMQ_HEARTBEAT_SIGNAL,
        full_block = OT_ZMQ_NEW_FULL_BLOCK_SIGNAL,
        statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
    };

    auto DefaultType() const noexcept -> filter::Type final
    {
        return default_type_;
    }
    auto FilterTip(const filter::Type type) const noexcept
        -> block::Position final
    {
        return database_.FilterTip(type);
    }
    auto GetFilterJob() const noexcept -> CfilterJob final;
    auto GetHeaderJob() const noexcept -> CfheaderJob final;
    auto Heartbeat() const noexcept -> void final;
    auto LoadFilter(const filter::Type type, const block::Hash& block)
        const noexcept -> std::unique_ptr<const GCS> final
    {
        return database_.LoadFilter(type, block.Bytes());
    }
    auto LoadFilterHeader(const filter::Type type, const block::Hash& block)
        const noexcept -> Header final
    {
        return database_.LoadFilterHeader(type, block.Bytes());
    }
    auto LoadFilterOrResetTip(
        const filter::Type type,
        const block::Position& position) const noexcept
        -> std::unique_ptr<const GCS> final;
    auto ProcessBlock(const block::bitcoin::Block& block) const noexcept
        -> bool final;
    auto ProcessBlock(BlockIndexerData& data) const noexcept -> void;
    auto ProcessSyncData(
        const block::Hash& prior,
        const std::pmr::vector<block::pHash>& hashes,
        const network::p2p::Data& data) const noexcept -> void final;
    auto ProcessSyncData(SyncClientFilterData& data) const noexcept -> void;
    auto ProcessSyncData(
        std::pmr::vector<SyncClientFilterData>& cache) const noexcept -> bool;
    auto Tip(const filter::Type type) const noexcept -> block::Position final
    {
        return database_.FilterTip(type);
    }

    auto Shutdown() noexcept -> void final;
    auto Start() noexcept -> void final;

    FilterOracle(
        const api::Session& api,
        const internal::Config& config,
        const internal::Network& node,
        const HeaderOracle& header,
        const internal::BlockOracle& block,
        const internal::FilterDatabase& database,
        const blockchain::Type chain,
        const blockchain::filter::Type filter,
        const std::string& shutdown) noexcept;

    ~FilterOracle() final;

private:
    friend Worker<FilterOracle, api::Session>;
    friend internal::FilterOracle;

    using FilterHeaderHex = std::string;
    using FilterHeaderMap = std::pmr::map<filter::Type, FilterHeaderHex>;
    using ChainMap = std::pmr::map<block::Height, FilterHeaderMap>;
    using CheckpointMap = std::pmr::map<blockchain::Type, ChainMap>;
    using OutstandingMap = std::pmr::map<int, std::atomic_int>;

    static const CheckpointMap filter_checkpoints_;

    const api::Session& api_;
    const internal::Network& node_;
    const HeaderOracle& header_;
    const internal::FilterDatabase& database_;
    const network::zeromq::socket::Publish& filter_notifier_;
    const blockchain::Type chain_;
    const filter::Type default_type_;
    mutable std::recursive_mutex lock_;
    OTZMQPublishSocket new_filters_;
    const NotifyCallback cb_;
    mutable std::unique_ptr<FilterDownloader> filter_downloader_;
    mutable std::unique_ptr<HeaderDownloader> header_downloader_;
    mutable std::unique_ptr<BlockIndexer> block_indexer_;
    mutable Time last_sync_progress_;
    mutable std::pmr::map<filter::Type, block::Position> last_broadcast_;
    mutable JobCounter outstanding_jobs_;
    std::atomic_bool running_;

    auto new_tip(
        const rLock&,
        const filter::Type type,
        const block::Position& tip) const noexcept -> void;
    auto process_block(
        const filter::Type type,
        const block::bitcoin::Block& block) const noexcept
        -> std::unique_ptr<const GCS>;
    auto reset_tips_to(
        const filter::Type type,
        const block::Position& position,
        const std::optional<bool> resetHeader = std::nullopt,
        const std::optional<bool> resetfilter = std::nullopt) const noexcept
        -> bool;
    auto reset_tips_to(
        const filter::Type type,
        const block::Position& headerTip,
        const block::Position& position,
        const std::optional<bool> resetHeader = std::nullopt) const noexcept
        -> bool;
    auto reset_tips_to(
        const filter::Type type,
        const block::Position& headerTip,
        const block::Position& filterTip,
        const block::Position& position,
        std::optional<bool> resetHeader = std::nullopt,
        std::optional<bool> resetfilter = std::nullopt) const noexcept -> bool;

    auto compare_header_to_checkpoint(
        const block::Position& block,
        const filter::Header& header) noexcept -> block::Position;
    auto compare_tips_to_checkpoint() noexcept -> void;
    auto compare_tips_to_header_chain() noexcept -> bool;

    FilterOracle() = delete;
    FilterOracle(const FilterOracle&) = delete;
    FilterOracle(FilterOracle&&) = delete;
    auto operator=(const FilterOracle&) -> FilterOracle& = delete;
    auto operator=(FilterOracle&&) -> FilterOracle& = delete;
};
}  // namespace opentxs::blockchain::node::implementation
