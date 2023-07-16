// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_shared_guarded.h>
#include <atomic>
#include <cstddef>
#include <shared_mutex>
#include <span>
#include <utility>

#include "blockchain/node/headeroracle/HeaderOraclePrivate.hpp"
#include "internal/blockchain/node/headeroracle/HeaderOracle.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"
#include "util/Allocated.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
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
class Header;
}  // namespace block

namespace database
{
class Header;
}  // namespace database

namespace node
{
namespace internal
{
class HeaderJob;
}  // namespace internal

class UpdateTransaction;
struct Endpoints;
}  // namespace node
}  // namespace blockchain

namespace network
{
namespace otdht
{
class Data;
}  // namespace otdht
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::internal
{
class HeaderOracle::Shared final : public opentxs::implementation::Allocated
{
public:
    using Data =
        libguarded::shared_guarded<HeaderOraclePrivate, std::shared_mutex>;

    const network::zeromq::BatchID batch_;
    std::atomic<HeaderOracle*> parent_;
    mutable Data data_;

    auto Ancestors(
        const block::Position& start,
        const block::Position& target,
        const std::size_t limit,
        alloc::Default alloc) const noexcept(false) -> Positions;
    auto Ancestors(
        const block::Position& start,
        const std::size_t limit,
        alloc::Default alloc) const noexcept(false) -> Positions;
    auto BestChain() const noexcept -> block::Position;
    auto BestChain(
        const block::Position& tip,
        const std::size_t limit,
        alloc::Default alloc) const noexcept(false) -> Positions;
    auto BestHash(const block::Height height) const noexcept -> block::Hash;
    auto BestHash(const block::Height height, const block::Position& check)
        const noexcept -> block::Hash;
    auto BestHashes(
        const block::Height start,
        const std::size_t limit,
        alloc::Default alloc) const noexcept -> Hashes;
    auto BestHashes(
        const block::Height start,
        const block::Hash& stop,
        const std::size_t limit,
        alloc::Default alloc) const noexcept -> Hashes;
    auto BestHashes(
        const std::span<const block::Hash> previous,
        const block::Hash& stop,
        const std::size_t limit,
        alloc::Default alloc) const noexcept -> Hashes;
    auto CalculateReorg(const block::Position& tip, alloc::Default alloc) const
        noexcept(false) -> Positions;
    auto CalculateReorg(
        const HeaderOraclePrivate& lock,
        const block::Position& tip,
        alloc::Default alloc) const noexcept(false) -> Positions;
    auto CommonParent(const block::Position& position) const noexcept
        -> std::pair<block::Position, block::Position>;
    auto Execute(Vector<ReorgTask>&& jobs) const noexcept -> bool;
    auto Exists(const block::Hash& hash) const noexcept -> bool;
    auto GetCheckpoint() const noexcept -> block::Position;
    auto GetDefaultCheckpoint() const noexcept -> CheckpointData;
    auto GetJob(alloc::Default alloc) const noexcept -> HeaderJob;
    auto GetPosition(const block::Height height) const noexcept
        -> block::Position;
    auto GetPosition(
        const HeaderOraclePrivate& lock,
        const block::Height height) const noexcept -> block::Position;
    auto IsInBestChain(const block::Hash& hash) const noexcept -> bool;
    auto IsInBestChain(const block::Position& position) const noexcept -> bool;
    auto IsSynchronized() const noexcept -> bool;
    auto LoadHeader(const block::Hash& hash) const noexcept -> block::Header;
    auto RecentHashes(alloc::Default alloc) const noexcept -> Hashes;
    auto Siblings() const noexcept -> UnallocatedSet<block::Hash>;
    auto Target() const noexcept -> block::Height;

    auto AddCheckpoint(
        const block::Height position,
        const block::Hash& requiredHash) noexcept -> bool;
    auto AddHeader(block::Header header) noexcept -> bool;
    auto AddHeaders(std::span<block::Header> headers) noexcept -> bool;
    auto DeleteCheckpoint() noexcept -> bool;
    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }
    auto Init() noexcept -> void;
    auto ProcessSyncData(
        block::Hash& prior,
        Vector<block::Hash>& hashes,
        const network::otdht::Data& data) noexcept -> std::size_t;
    auto Report() noexcept -> void;
    auto SubmitBlock(const ReadView in) noexcept -> void;

    Shared(
        const api::Session& api,
        const blockchain::Type chain,
        const node::Endpoints& endpoints,
        database::Header& database,
        network::zeromq::BatchID batch,
        allocator_type alloc) noexcept;
    Shared() = delete;
    Shared(const Shared&) = delete;
    Shared(Shared&&) = delete;
    auto operator=(const Shared&) -> Shared& = delete;
    auto operator=(Shared&&) -> Shared& = delete;

    ~Shared() final;

private:
    struct Candidate {
        bool blacklisted_{false};
        Deque<block::Position> chain_{};
    };

    using Candidates = Vector<Candidate>;

    static auto evaluate_candidate(
        const block::Header& current,
        const block::Header& candidate) noexcept -> bool;

    auto ancestors(
        const HeaderOraclePrivate& data,
        const block::Position& start,
        const block::Position& target,
        const std::size_t limit,
        alloc::Default alloc) const noexcept(false) -> Positions;
    auto best_chain(const HeaderOraclePrivate& data) const noexcept
        -> block::Position;
    auto best_chain(
        const HeaderOraclePrivate& data,
        const block::Position& tip,
        const std::size_t limit,
        alloc::Default alloc) const noexcept -> Positions;
    auto best_hash(const HeaderOraclePrivate& data, const block::Height height)
        const noexcept -> block::Hash;
    auto best_hashes(
        const HeaderOraclePrivate& data,
        const block::Height start,
        const block::Hash& stop,
        const std::size_t limit,
        alloc::Default alloc) const noexcept -> Hashes;
    auto blank_hash() const noexcept -> const block::Hash&;
    auto blank_position() const noexcept -> const block::Position&;
    auto calculate_reorg(
        const HeaderOraclePrivate& data,
        const block::Position& tip,
        alloc::Default alloc) const noexcept(false) -> Positions;
    auto common_parent(
        const HeaderOraclePrivate& data,
        const block::Position& position) const noexcept
        -> std::pair<block::Position, block::Position>;
    auto delete_checkpoint(HeaderOraclePrivate& data) noexcept -> bool;
    auto get_checkpoint(const HeaderOraclePrivate& data) const noexcept
        -> block::Position;
    auto get_default_checkpoint(const HeaderOraclePrivate& data) const noexcept
        -> CheckpointData;
    auto get_default_checkpoint(const blockchain::Type chain) const noexcept
        -> CheckpointData;
    auto get_position(
        const HeaderOraclePrivate& data,
        const block::Height height) const noexcept -> block::Position;
    auto is_in_best_chain(
        const HeaderOraclePrivate& data,
        const block::Hash& hash) const noexcept
        -> std::pair<bool, block::Height>;
    auto is_in_best_chain(
        const HeaderOraclePrivate& data,
        const block::Position& position) const noexcept -> bool;
    auto is_in_best_chain(
        const HeaderOraclePrivate& data,
        const block::Height height,
        const block::Hash& hash) const noexcept -> bool;
    auto is_synchronized(const HeaderOraclePrivate& data) const noexcept
        -> bool;
    auto recent_hashes(const HeaderOraclePrivate& data, alloc::Default alloc)
        const noexcept -> Hashes;

    auto add_checkpoint(
        HeaderOraclePrivate& data,
        const block::Height position,
        const block::Hash& requiredHash) noexcept -> bool;
    auto add_header(
        const HeaderOraclePrivate& data,
        UpdateTransaction& update,
        block::Header header) noexcept -> bool;
    auto apply_checkpoint(
        const HeaderOraclePrivate& data,
        const block::Height height,
        UpdateTransaction& update) noexcept -> bool;
    auto apply_update(
        HeaderOraclePrivate& data,
        UpdateTransaction& update) noexcept -> bool;
    auto choose_candidate(
        const block::Header& current,
        const Candidates& candidates,
        UpdateTransaction& update) noexcept(false) -> std::pair<bool, bool>;
    auto connect_children(
        const HeaderOraclePrivate& data,
        block::Header& parentHeader,
        Candidates& candidates,
        Candidate& candidate,
        UpdateTransaction& update) -> void;
    // Returns true if the child is checkpoint blacklisted
    auto connect_to_parent(
        const HeaderOraclePrivate& data,
        const UpdateTransaction& update,
        const block::Header& parent,
        block::Header& child) noexcept -> bool;
    auto initialize_candidate(
        const HeaderOraclePrivate& data,
        const block::Header& best,
        const block::Header& parent,
        UpdateTransaction& update,
        Candidates& candidates,
        block::Header& child,
        const block::Hash& stopHash = {}) noexcept(false) -> Candidate&;
    auto is_disconnected(
        const block::Hash& parent,
        UpdateTransaction& update) noexcept -> const block::Header*;
    auto stage_candidate(
        const HeaderOraclePrivate& data,
        const block::Header& best,
        Candidates& candidates,
        UpdateTransaction& update,
        block::Header& child) noexcept(false) -> void;
};
}  // namespace opentxs::blockchain::node::internal
