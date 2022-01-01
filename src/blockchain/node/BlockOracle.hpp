// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/container/flat_map.hpp>
#include <boost/container/vector.hpp>
#include <chrono>
#include <cstddef>
#include <deque>
#include <functional>
#include <future>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>

#include "core/Worker.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/node/BlockOracle.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace node
{
class HeaderOracle;
}  // namespace node
}  // namespace blockchain

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket

class Frame;
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::blockchain::node::implementation
{
class BlockOracle final : public node::internal::BlockOracle,
                          public Worker<BlockOracle, api::Session>
{
public:
    class BlockDownloader;

    enum class Work : OTZMQWorkType {
        shutdown = value(WorkType::Shutdown),
        block = value(WorkType::BlockchainNewHeader),
        reorg = value(WorkType::BlockchainReorg),
        heartbeat = OT_ZMQ_HEARTBEAT_SIGNAL,
        statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
    };

    auto DownloadQueue() const noexcept -> std::size_t final
    {
        return cache_.DownloadQueue();
    }
    auto GetBlockJob() const noexcept -> BlockJob final;
    auto Heartbeat() const noexcept -> void final;
    auto Internal() const noexcept -> const internal::BlockOracle& final
    {
        return *this;
    }
    auto LoadBitcoin(const block::Hash& block) const noexcept
        -> BitcoinBlockFuture final;
    auto LoadBitcoin(const BlockHashes& hashes) const noexcept
        -> BitcoinBlockFutures final;
    auto SubmitBlock(const ReadView in) const noexcept -> void final;
    auto Tip() const noexcept -> block::Position final
    {
        return db_.BlockTip();
    }
    auto Validate(const BitcoinBlock& block) const noexcept -> bool final
    {
        return validator_->Validate(block);
    }

    auto Init() noexcept -> void final;
    auto Shutdown() noexcept -> std::shared_future<void> final
    {
        return signal_shutdown();
    }

    BlockOracle(
        const api::Session& api,
        const internal::Network& node,
        const HeaderOracle& header,
        const internal::BlockDatabase& db,
        const blockchain::Type chain,
        const std::string& shutdown) noexcept;

    ~BlockOracle() final;

private:
    friend Worker<BlockOracle, api::Session>;

    using Promise = std::promise<BitcoinBlock_p>;
    using PendingData = std::tuple<Time, Promise, BitcoinBlockFuture, bool>;
    using Pending = std::pmr::map<block::pHash, PendingData>;

    struct Cache {
        auto DownloadQueue() const noexcept -> std::size_t;
        auto ReceiveBlock(const zmq::Frame& in) const noexcept -> void;
        auto ReceiveBlock(BitcoinBlock_p in) const noexcept -> void;
        auto Request(const block::Hash& block) const noexcept
            -> BitcoinBlockFuture;
        auto Request(const BlockHashes& hashes) const noexcept
            -> BitcoinBlockFutures;
        auto StateMachine() const noexcept -> bool;

        auto Shutdown() noexcept -> void;

        Cache(
            const api::Session& api_,
            const internal::Network& node,
            const internal::BlockDatabase& db,
            const network::zeromq::socket::Publish& blockAvailable,
            const network::zeromq::socket::Publish& downloadCache,
            const blockchain::Type chain) noexcept;
        ~Cache() { Shutdown(); }

    private:
        static const std::size_t cache_limit_;
        static const std::chrono::seconds download_timeout_;

        struct Mem {
            auto find(const ReadView& id) const noexcept -> BitcoinBlockFuture;

            auto clear() noexcept -> void;
            auto push(block::pHash&& id, BitcoinBlockFuture&& future) noexcept
                -> void;

            Mem(const std::size_t limit) noexcept;

        private:
            using CachedBlock = std::pair<block::pHash, BitcoinBlockFuture>;
            using Completed = std::pmr::deque<CachedBlock>;
            using Index =
                boost::container::flat_map<ReadView, const CachedBlock*>;

            const std::size_t limit_;
            Completed queue_;
            Index index_;
        };

        const api::Session& api_;
        const internal::Network& node_;
        const internal::BlockDatabase& db_;
        const network::zeromq::socket::Publish& block_available_;
        const network::zeromq::socket::Publish& cache_size_publisher_;
        const blockchain::Type chain_;
        mutable std::mutex lock_;
        mutable Pending pending_;
        mutable Mem mem_;
        bool running_;

        auto download(const block::Hash& block) const noexcept -> bool;
        auto publish(std::size_t cache) const noexcept -> void;
        auto publish(const block::Hash& block) const noexcept -> void;
    };

    const internal::Network& node_;
    const internal::BlockDatabase& db_;
    mutable std::mutex lock_;
    Cache cache_;
    std::unique_ptr<BlockDownloader> block_downloader_;
    const std::unique_ptr<const internal::BlockValidator> validator_;

    static auto get_validator(
        const blockchain::Type chain,
        const node::HeaderOracle& headers) noexcept
        -> std::unique_ptr<const internal::BlockValidator>;

    auto pipeline(const zmq::Message& in) noexcept -> void;
    auto shutdown(std::promise<void>& promise) noexcept -> void;
    auto state_machine() noexcept -> bool;

    BlockOracle() = delete;
    BlockOracle(const BlockOracle&) = delete;
    BlockOracle(BlockOracle&&) = delete;
    auto operator=(const BlockOracle&) -> BlockOracle& = delete;
    auto operator=(BlockOracle&&) -> BlockOracle& = delete;
};
}  // namespace opentxs::blockchain::node::implementation
