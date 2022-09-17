// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <cs_shared_guarded.h>
#include <atomic>
#include <cstddef>
#include <memory>
#include <shared_mutex>
#include <utility>

#include "blockchain/node/headeroracle/HeaderOraclePrivate.hpp"
#include "internal/blockchain/node/headeroracle/HeaderOracle.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Allocated.hpp"

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
namespace bitcoin
{
namespace block
{
class Header;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Hash;
class Header;
class Position;
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

class Manager;
class UpdateTransaction;
}  // namespace node
}  // namespace blockchain

namespace network
{
namespace otdht
{
class Data;
}  // namespace otdht
}  // namespace network
// }  // namespace v1
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
        const std::size_t limit) const noexcept(false) -> Positions;
    auto BestChain() const noexcept -> block::Position;
    auto BestChain(const block::Position& tip, const std::size_t limit) const
        noexcept(false) -> Positions;
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
        const Hashes& previous,
        const block::Hash& stop,
        const std::size_t limit,
        alloc::Default alloc) const noexcept -> Hashes;
    auto CalculateReorg(const block::Position& tip) const noexcept(false)
        -> Positions;
    auto CalculateReorg(
        const HeaderOraclePrivate& lock,
        const block::Position& tip) const noexcept(false) -> Positions;
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
    auto LoadBitcoinHeader(const block::Hash& hash) const noexcept
        -> std::unique_ptr<bitcoin::block::Header>;
    auto LoadHeader(const block::Hash& hash) const noexcept
        -> std::unique_ptr<block::Header>;
    auto RecentHashes(alloc::Default alloc) const noexcept -> Hashes;
    auto Siblings() const noexcept -> UnallocatedSet<block::Hash>;
    auto Target() const noexcept -> block::Height;

    auto AddCheckpoint(
        const block::Height position,
        const block::Hash& requiredHash) noexcept -> bool;
    auto AddHeader(std::unique_ptr<block::Header> header) noexcept -> bool;
    auto AddHeaders(Vector<std::unique_ptr<block::Header>>&) noexcept -> bool;
    auto DeleteCheckpoint() noexcept -> bool;
    auto Init() noexcept -> void;
    auto ProcessSyncData(
        block::Hash& prior,
        Vector<block::Hash>& hashes,
        const network::otdht::Data& data) noexcept -> std::size_t;
    auto Report() noexcept -> void;
    auto SubmitBlock(const ReadView in) noexcept -> void;

    Shared(
        const api::Session& api,
        const node::Manager& node,
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

    auto best_chain(const HeaderOraclePrivate& data) const noexcept
        -> block::Position;
    auto best_chain(
        const HeaderOraclePrivate& data,
        const block::Position& tip,
        const std::size_t limit) const noexcept -> Positions;
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
        const block::Position& tip) const noexcept(false) -> Positions;
    auto common_parent(
        const HeaderOraclePrivate& data,
        const block::Position& position) const noexcept
        -> std::pair<block::Position, block::Position>;
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
        std::unique_ptr<block::Header> header) noexcept -> bool;
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