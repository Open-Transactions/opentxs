// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/enable_shared_from.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <atomic>
#include <cstddef>
#include <filesystem>
#include <future>
#include <memory>
#include <span>
#include <string_view>
#include <utility>

#include "blockchain/node/Downloader.hpp"
#include "internal/blockchain/node/blockoracle/Types.hpp"
#include "internal/blockchain/node/filteroracle/BlockIndexer.hpp"
#include "internal/blockchain/node/filteroracle/Types.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/GCS.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Header.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Actor.hpp"
#include "util/JobCounter.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace bitcoin
{
namespace block
{
class Block;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Hash;
class Position;
}  // namespace block

namespace node
{
namespace filteroracle
{
class Shared;
}  // namespace filteroracle

class Manager;
}  // namespace node
}  // namespace blockchain

namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network

class ScopeGuard;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::filteroracle
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
class BlockIndexer::Imp final : public Actor<Imp, BlockIndexerJob>,
                                public boost::enable_shared_from
{
public:
    auto Init(boost::shared_ptr<Imp> me) noexcept -> void;

    Imp(std::shared_ptr<const api::Session> api,
        std::shared_ptr<const node::Manager> node,
        std::shared_ptr<Shared> shared,
        const network::zeromq::BatchID batch,
        allocator_type alloc) noexcept;
    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;

    ~Imp() final;

private:
    friend Actor<Imp, BlockIndexerJob>;

    using PreviousCfheader = std::shared_future<cfilter::Header>;

    struct Tip {
        block::Position position_{};
        PreviousCfheader cfheader_{};
    };

    struct Job {
        using allocator_type = Imp::allocator_type;

        enum class State : int {
            waiting,
            downloaded,
            running,
            finished,
            redownload,
        };

        const block::Position position_;
        const PreviousCfheader previous_cfheader_;
        std::atomic<State> state_;
        blockoracle::BlockLocation block_;
        GCS cfilter_;
        std::promise<cfilter::Header> promise_;
        PreviousCfheader future_;

        Job(const block::Hash& block,
            const PreviousCfheader& cfheader,
            block::Height height,
            allocator_type alloc) noexcept
            : position_(height, block)
            , previous_cfheader_(cfheader)
            , state_(State::waiting)
            , block_()
            , cfilter_(alloc)
            , promise_()
            , future_(promise_.get_future())
        {
        }
    };

    using JobPointer = boost::shared_ptr<Job>;
    using WorkMap = Map<block::Height, JobPointer>;
    using Index = Map<block::Hash, block::Height>;

    std::shared_ptr<const api::Session> api_p_;
    std::shared_ptr<const node::Manager> node_p_;
    std::shared_ptr<Shared> shared_p_;
    const api::Session& api_;
    const node::Manager& node_;
    const blockchain::Type chain_;
    const std::filesystem::path checkpoints_;
    Shared& shared_;
    std::atomic_bool notified_;
    std::size_t cached_cfilter_bytes_;
    Tip tip_;
    Downloader downloader_;
    Index index_;
    WorkMap downloading_;
    WorkMap processing_;
    WorkMap finished_;
    JobCounter counter_;
    Outstanding running_;

    static auto background(
        boost::shared_ptr<Imp> me,
        JobPointer job,
        std::shared_ptr<const ScopeGuard> post) noexcept -> void;

    auto previous_cfheader() const noexcept
        -> std::pair<block::Height, PreviousCfheader>;
    auto ready() const noexcept -> bool;

    auto adjust_tip(const block::Position& ancestor) noexcept -> void;
    auto calculate_cfheaders(allocator_type monotonic) noexcept -> bool;
    auto calculate_cfilters() noexcept -> bool;
    auto do_shutdown() noexcept -> void;
    auto do_startup(allocator_type monotonic) noexcept -> bool;
    auto fetch_blocks(allocator_type monotonic) noexcept -> bool;
    auto find_finished(allocator_type monotonic) noexcept -> void;
    auto load_tip(const block::Position& value) noexcept -> void;
    auto pipeline(const Work work, Message&& msg, allocator_type) noexcept
        -> void;
    auto process_block_ready(Message&& in, allocator_type monotonic) noexcept
        -> void;
    auto process_job_finished(Message&& in) noexcept -> void;
    auto process_reindex(Message&& in) noexcept -> void;
    auto process_reorg(Message&& in) noexcept -> void;
    auto process_reorg(block::Position&& parent) noexcept -> void;
    auto process_report(Message&& in) noexcept -> void;
    auto request_blocks(std::span<const block::Hash> hashes) noexcept -> void;
    auto update_checkpoint() noexcept -> void;
    auto work(allocator_type monotonic) noexcept -> bool;
    auto write_checkpoint(block::Height target) noexcept -> void;
    auto write_last_checkpoint(const block::Position& tip) noexcept -> void;
};
#pragma GCC diagnostic pop
}  // namespace opentxs::blockchain::node::filteroracle
