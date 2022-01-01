// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <cstddef>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#include "blockchain/node/wallet/Batch.hpp"
#include "blockchain/node/wallet/Job.hpp"
#include "blockchain/node/wallet/Types.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/node/BlockOracle.hpp"

namespace opentxs
{
namespace blockchain
{
namespace node
{
namespace wallet
{
class Progress;
class SubchainStateData;
class Work;
}  // namespace wallet
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs

namespace opentxs::blockchain::node::wallet
{
class Process final : public Job
{
public:
    auto Reorg(const block::Position& parent) noexcept -> void final;
    auto Request(
        const std::optional<block::Position>& highestClean,
        const std::pmr::vector<block::Position>& blocks,
        std::pmr::vector<std::unique_ptr<Batch>>&& batches,
        std::pmr::vector<Work*>&& jobs) noexcept -> void;
    auto Run() noexcept -> bool final;

    Process(SubchainStateData& parent, Progress& progress) noexcept;

    ~Process() final = default;

private:
    class Cache
    {
    public:
        using BatchMap = std::pmr::map<Batch::ID, std::unique_ptr<Batch>>;

        auto FinishBatch(BatchMap::iterator batch) noexcept -> void;
        auto Flush() noexcept -> std::pmr::vector<BatchMap::iterator>;
        auto Pop(BlockMap& destination) noexcept -> bool;
        auto Push(
            std::pmr::vector<std::unique_ptr<Batch>>&& batches,
            std::pmr::vector<Work*>&& jobs) noexcept -> void;
        auto Reorg(const block::Position& parent) noexcept -> void;
        auto ReRequest(Work* job) noexcept -> void;

        Cache(const SubchainStateData& parent) noexcept;

    private:
        const SubchainStateData& parent_;
        const std::size_t limit_;
        mutable std::mutex lock_;
        BatchMap batches_;
        std::pmr::deque<Work*> pending_;
        BlockMap downloading_;

        auto download(const Lock& lock) noexcept -> void;
        auto request(const Lock& lock, Work* job) noexcept -> void;

        Cache() = delete;
        Cache(const Cache&) = delete;
        Cache(Cache&&) = delete;
        auto operator=(const Cache&) -> Cache& = delete;
        auto operator=(Cache&&) -> Cache& = delete;
    };

    friend Cache;

    Progress& progress_;
    Cache cache_;
    BlockMap waiting_;
    BlockMap processing_;

    static auto flush(const block::Position& parent, BlockMap& map) noexcept
        -> void;
    static auto move_nodes(
        BlockMap& from,
        BlockMap& to,
        std::function<bool(BlockMap::iterator)> moveCondition,
        std::function<bool(std::size_t)> breakCondition,
        std::function<void(BlockMap::iterator)> post) noexcept -> void;
    static auto move_nodes(
        std::pmr::vector<BlockMap::iterator>& items,
        BlockMap& from,
        BlockMap& to,
        std::function<void(BlockMap::iterator)> cb = {}) noexcept -> void;

    auto limit(const Lock& lock, const std::size_t outstanding) const noexcept
        -> bool;
    auto type() const noexcept -> const char* final { return "process"; }

    auto FinishBatches() noexcept -> bool;
    auto ProcessPosition(const Cookie key, Work* work) noexcept -> void;
    auto ProcessBatch(Cache::BatchMap::iterator it) noexcept -> void;

    Process() = delete;
    Process(const Process&) = delete;
    Process(Process&&) = delete;
    auto operator=(const Process&) -> Process& = delete;
    auto operator=(Process&&) -> Process& = delete;
};
}  // namespace opentxs::blockchain::node::wallet
