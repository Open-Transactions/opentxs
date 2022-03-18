// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::node::implementation::FilterOracle::BlockIndexer
// IWYU pragma: no_forward_declare opentxs::blockchain::node::implementation::FilterOracle::BlockIndexerData

#pragma once

#include <cstddef>
#include <exception>
#include <functional>
#include <future>
#include <iosfwd>
#include <memory>
#include <utility>

#include "blockchain/DownloadManager.hpp"
#include "blockchain/DownloadTask.hpp"
#include "blockchain/node/FilterOracle.hpp"
#include "core/Worker.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "util/JobCounter.hpp"

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
namespace implementation
{
class FilterOracle;
}  // namespace implementation

class HeaderOracle;
}  // namespace node
}  // namespace blockchain

namespace network
{
namespace zeromq
{
namespace socket
{
class Push;
}  // namespace socket

class Message;
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::implementation
{
using BlockDMFilter = download::Manager<
    FilterOracle::BlockIndexer,
    std::shared_ptr<const block::bitcoin::Block>,
    cfilter::pHeader,
    cfilter::Type>;
using BlockWorkerFilter = Worker<FilterOracle::BlockIndexer, api::Session>;

class FilterOracle::BlockIndexer : public BlockDMFilter,
                                   public BlockWorkerFilter
{
public:
    auto NextBatch() noexcept { return allocate_batch(type_); }

    BlockIndexer(
        const api::Session& api,
        internal::FilterDatabase& db,
        const HeaderOracle& header,
        const internal::BlockOracle& block,
        const internal::Network& node,
        FilterOracle& parent,
        const blockchain::Type chain,
        const cfilter::Type type,
        const UnallocatedCString& shutdown,
        const NotifyCallback& notify) noexcept;

    ~BlockIndexer();

private:
    friend BlockDMFilter;
    friend BlockWorkerFilter;

    internal::FilterDatabase& db_;
    const HeaderOracle& header_;
    const internal::BlockOracle& block_;
    const internal::Network& node_;
    FilterOracle& parent_;
    const blockchain::Type chain_;
    const cfilter::Type type_;
    const NotifyCallback& notify_;
    JobCounter job_counter_;

    auto batch_ready() const noexcept -> void { trigger(); }
    auto batch_size(const std::size_t in) const noexcept -> std::size_t;
    auto calculate_cfheaders(
        UnallocatedVector<BlockIndexerData>& cache) const noexcept -> bool;
    auto check_task(TaskType&) const noexcept -> void {}
    auto trigger_state_machine() const noexcept -> void { trigger(); }
    auto update_tip(const Position& position, const cfilter::pHeader&)
        const noexcept -> void;

    auto download() noexcept -> void;
    auto pipeline(const zmq::Message& in) noexcept -> void;
    auto process_position(const zmq::Message& in) noexcept -> void;
    auto process_position(const Position& pos) noexcept -> void;
    auto queue_processing(DownloadedData&& data) noexcept -> void;
    auto reset_to_genesis() noexcept -> void;
    auto shutdown(std::promise<void>& promise) noexcept -> void;
};

struct FilterOracle::BlockIndexerData {
    using Task = FilterOracle::BlockIndexer::BatchType::TaskType;

    const Task& incoming_data_;
    const cfilter::Type type_;
    cfilter::pHash filter_hash_;
    internal::FilterDatabase::Filter& filter_data_;
    internal::FilterDatabase::Header& header_data_;
    Outstanding& job_counter_;

    BlockIndexerData(
        OTData blank,
        const Task& data,
        const cfilter::Type type,
        internal::FilterDatabase::Filter& filter,
        internal::FilterDatabase::Header& header,
        Outstanding& jobCounter) noexcept
        : incoming_data_(data)
        , type_(type)
        , filter_hash_(std::move(blank))
        , filter_data_(filter)
        , header_data_(header)
        , job_counter_(jobCounter)
    {
    }
};
}  // namespace opentxs::blockchain::node::implementation
