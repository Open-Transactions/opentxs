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
#include <optional>
#include <span>
#include <utility>

#include "blockchain/node/Downloader.hpp"
#include "internal/blockchain/node/blockoracle/Types.hpp"
#include "internal/blockchain/node/filteroracle/BlockIndexer.hpp"
#include "internal/blockchain/node/filteroracle/Types.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/GCS.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Header.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Actor.hpp"
#include "util/ByteLiterals.hpp"
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
namespace node
{
namespace filteroracle
{
class Shared;
}  // namespace filteroracle

class Manager;
}  // namespace node
}  // namespace blockchain

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
    auto get_deleter() noexcept -> delete_function final
    {
        return make_deleter(this);
    }
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

    static constexpr auto interval_ = block::Height{1000};
    static constexpr auto delay_ = block::Height{500};
    static constexpr auto calculation_batch_ = 1000_uz;
    static constexpr auto max_blocks_ = 1000_uz;
    static constexpr auto max_cached_cfilters_ = 256_mib;

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
    WorkMap queued_;
    WorkMap downloading_;
    WorkMap processing_;
    WorkMap finished_;
    std::optional<block::Height> next_checkpoint_;
    JobCounter counter_;
    Outstanding running_;

    static auto background(
        boost::shared_ptr<Imp> me,
        JobPointer job,
        std::shared_ptr<const ScopeGuard> post) noexcept -> void;

    auto open_blocks() const noexcept -> std::size_t;
    auto previous_cfheader(allocator_type monotonic) const noexcept
        -> std::pair<block::Height, PreviousCfheader>;
    auto ready() const noexcept -> bool;

    auto adjust_tip(const block::Position& ancestor) noexcept -> void;
    auto calculate_cfheaders(allocator_type monotonic) noexcept -> bool;
    auto calculate_cfilters() noexcept -> bool;
    auto do_shutdown() noexcept -> void;
    auto do_startup(allocator_type monotonic) noexcept -> bool;
    auto fetch_blocks(allocator_type monotonic) noexcept -> bool;
    auto find_finished(allocator_type monotonic) noexcept -> void;
    auto get_next_checkpoint(block::Height tip) noexcept -> void;
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
    auto queue_blocks(allocator_type monotonic) noexcept -> bool;
    auto request_blocks(std::span<const block::Hash> hashes) noexcept -> void;
    auto reset_to_genesis() noexcept -> void;
    auto update_checkpoint() noexcept -> void;
    auto work(allocator_type monotonic) noexcept -> bool;
    auto write_checkpoint(block::Height target) noexcept -> void;
};
#pragma GCC diagnostic pop
}  // namespace opentxs::blockchain::node::filteroracle
