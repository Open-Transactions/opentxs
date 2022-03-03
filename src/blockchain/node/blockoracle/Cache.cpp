// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "blockchain/node/BlockOracle.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <chrono>
#include <iterator>
#include <memory>

#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/node/BlockOracle.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs::blockchain::node::implementation
{
const std::size_t BlockOracle::Cache::cache_limit_{16};
const std::chrono::seconds BlockOracle::Cache::download_timeout_{60};

BlockOracle::Cache::Cache(
    const api::Session& api,
    const internal::Network& node,
    const internal::BlockDatabase& db,
    const network::zeromq::socket::Publish& blockAvailable,
    const network::zeromq::socket::Publish& downloadCache,
    const blockchain::Type chain) noexcept
    : api_(api)
    , node_(node)
    , db_(db)
    , block_available_(blockAvailable)
    , cache_size_publisher_(downloadCache)
    , chain_(chain)
    , lock_()
    , pending_()
    , mem_(cache_limit_)
    , running_(true)
{
}

auto BlockOracle::Cache::DownloadQueue() const noexcept -> std::size_t
{
    auto lock = Lock{lock_};

    return pending_.size();
}

auto BlockOracle::Cache::publish(std::size_t size) const noexcept -> void
{
    cache_size_publisher_.Send([&] {
        auto work = network::zeromq::tagged_message(
            WorkType::BlockchainBlockDownloadQueue);
        work.AddFrame(chain_);
        work.AddFrame(size);

        return work;
    }());
}

auto BlockOracle::Cache::publish(const block::Hash& block) const noexcept
    -> void
{
    block_available_.Send([&] {
        auto work =
            network::zeromq::tagged_message(WorkType::BlockchainBlockAvailable);
        work.AddFrame(chain_);
        work.AddFrame(block);

        return work;
    }());
}

auto BlockOracle::Cache::ReceiveBlock(const zmq::Frame& in) const noexcept
    -> void
{
    ReceiveBlock(api_.Factory().BitcoinBlock(chain_, in.Bytes()));
}

auto BlockOracle::Cache::ReceiveBlock(BitcoinBlock_p in) const noexcept -> void
{
    if (false == bool(in)) {
        LogError()(OT_PRETTY_CLASS())("Invalid block").Flush();

        return;
    }

    auto lock = Lock{lock_};
    auto& block = *in;

    if (database::BlockStorage::None != db_.BlockPolicy()) {
        const auto saved = db_.BlockStore(block);

        OT_ASSERT(saved);
    }

    const auto& id = block.ID();
    auto pending = pending_.find(id);

    if (pending_.end() == pending) {
        LogVerbose()(OT_PRETTY_CLASS())("Received block not in request list")
            .Flush();

        return;
    }

    auto& [time, promise, future, queued] = pending->second;
    promise.set_value(std::move(in));
    publish(id);
    LogVerbose()(OT_PRETTY_CLASS())("Cached block ")(id.asHex()).Flush();
    mem_.push(id, std::move(future));
    pending_.erase(pending);
    publish(pending_.size());
}

auto BlockOracle::Cache::Request(const block::Hash& block) const noexcept
    -> BitcoinBlockFuture
{
    const auto output = Request(BlockHashes{block});

    OT_ASSERT(1 == output.size());

    return output.at(0);
}

auto BlockOracle::Cache::Request(const BlockHashes& hashes) const noexcept
    -> BitcoinBlockFutures
{
    auto output = BitcoinBlockFutures{};
    output.reserve(hashes.size());
    auto ready = UnallocatedVector<const block::Hash*>{};
    auto download =
        UnallocatedMap<block::pHash, BitcoinBlockFutures::iterator>{};
    auto lock = Lock{lock_};

    if (false == running_) {
        std::for_each(hashes.begin(), hashes.end(), [&](const auto&) {
            auto promise = Promise{};
            promise.set_value(nullptr);
            output.emplace_back(promise.get_future());
        });

        return output;
    }

    for (const auto& block : hashes) {
        const auto& log = LogTrace();
        const auto start = Clock::now();
        auto found{false};

        if (auto future = mem_.find(block->Bytes()); future.valid()) {
            output.emplace_back(std::move(future));
            ready.emplace_back(&block.get());
            found = true;
        }

        const auto mem = Clock::now();

        if (found) {
            log(OT_PRETTY_CLASS())(" block is cached in memory. Found in ")(
                std::chrono::nanoseconds{mem - start})
                .Flush();

            continue;
        }

        {
            auto it = pending_.find(block);

            if (pending_.end() != it) {
                const auto& [time, promise, future, queued] = it->second;
                output.emplace_back(future);
                found = true;
            }
        }

        const auto pending = Clock::now();

        if (found) {
            log(OT_PRETTY_CLASS())(
                " block is already in download queue. Found in ")(
                std::chrono::nanoseconds{pending - mem})
                .Flush();

            continue;
        }

        if (auto pBlock = db_.BlockLoadBitcoin(block); bool(pBlock)) {
            // TODO this should be checked in the block factory function
            OT_ASSERT(pBlock->ID() == block);

            auto promise = Promise{};
            promise.set_value(std::move(pBlock));
            mem_.push(OTData{block}, promise.get_future());
            output.emplace_back(mem_.find(block->Bytes()));
            ready.emplace_back(&block.get());
            found = true;
        }

        const auto disk = Clock::now();

        if (found) {
            log(OT_PRETTY_CLASS())(
                " block is already downloaded. Loaded from storage in ")(
                std::chrono::nanoseconds{disk - pending})
                .Flush();

            continue;
        }

        output.emplace_back();
        auto it = output.begin();
        std::advance(it, output.size() - 1);
        download.emplace(block, it);

        log(OT_PRETTY_CLASS())(" block queued for download in ")(
            std::chrono::nanoseconds{Clock::now() - pending})
            .Flush();
    }

    OT_ASSERT(output.size() == hashes.size());

    if (0 < download.size()) {
        auto blockList = UnallocatedVector<ReadView>{};
        std::transform(
            std::begin(download),
            std::end(download),
            std::back_inserter(blockList),
            [](const auto& in) -> auto {
                const auto& [key, value] = in;

                return key->Bytes();
            });
        LogVerbose()(OT_PRETTY_CLASS())("Downloading ")(blockList.size())(
            " blocks from peers")
            .Flush();
        const auto messageSent = node_.RequestBlocks(blockList);

        for (auto& [hash, futureOut] : download) {
            auto& [time, promise, future, queued] = pending_[hash];
            time = Clock::now();
            future = promise.get_future();
            *futureOut = future;
            queued = messageSent;
        }

        publish(pending_.size());
    }

    for (const auto* hash : ready) { publish(*hash); }

    return output;
}

auto BlockOracle::Cache::Shutdown() noexcept -> void
{
    auto lock = Lock{lock_};

    if (running_) {
        running_ = false;
        mem_.clear();

        for (auto& [hash, item] : pending_) {
            auto& [time, promise, future, queued] = item;
            promise.set_value(nullptr);
        }

        pending_.clear();
        publish(pending_.size());
    }
}

auto BlockOracle::Cache::StateMachine() const noexcept -> bool
{
    auto lock = Lock{lock_};

    if (false == running_) { return false; }

    LogVerbose()(OT_PRETTY_CLASS())(DisplayString(chain_))(
        " download queue contains ")(pending_.size())(" blocks.")
        .Flush();
    auto blockList = UnallocatedVector<ReadView>{};
    blockList.reserve(pending_.size());

    for (auto& [hash, item] : pending_) {
        auto& [time, promise, future, queued] = item;
        const auto now = Clock::now();
        namespace c = std::chrono;
        const auto elapsed = std::chrono::nanoseconds{now - time};
        const auto timeout = download_timeout_ <= elapsed;

        if (timeout || (false == queued)) {
            LogVerbose()(OT_PRETTY_CLASS())("Requesting ")(
                DisplayString(chain_))(" block ")(hash->asHex())(" from peers")
                .Flush();
            blockList.emplace_back(hash->Bytes());
            queued = true;
            time = now;
        } else {
            LogVerbose()(OT_PRETTY_CLASS())(elapsed)(" elapsed waiting for ")(
                DisplayString(chain_))(" block ")(hash->asHex())
                .Flush();
        }
    }

    if (0 < blockList.size()) { node_.RequestBlocks(blockList); }

    return 0 < pending_.size();
}
}  // namespace opentxs::blockchain::node::implementation
