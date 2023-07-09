// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/block/Hash.hpp"
// IWYU pragma: no_include "opentxs/blockchain/block/Position.hpp"

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <cstddef>
#include <functional>
#include <memory>
#include <span>
#include <tuple>
#include <utility>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{

namespace cfilter
{
class Header;
}  // namespace cfilter

namespace node
{
namespace internal
{
class HeaderJob;
struct HeaderOraclePrivate;
}  // namespace internal

class Manager;
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

namespace opentxs::blockchain::node
{
using ReorgTask = std::function<
    bool(const node::HeaderOracle&, const internal::HeaderOraclePrivate&)>;
}  // namespace opentxs::blockchain::node

namespace opentxs::blockchain::node::internal
{
class HeaderOracle final : public node::HeaderOracle
{
public:
    class Actor;
    class Shared;

    using CheckpointBlockHash = block::Hash;
    using PreviousBlockHash = block::Hash;
    using CheckpointCfheader = cfilter::Header;
    using CheckpointData = std::tuple<
        block::Height,
        CheckpointBlockHash,
        PreviousBlockHash,
        CheckpointCfheader>;

    auto Ancestors(
        const block::Position& start,
        const block::Position& target,
        const std::size_t limit,
        alloc::Strategy alloc) const noexcept(false) -> Positions final;
    auto Ancestors(
        const block::Position& start,
        const std::size_t limit,
        alloc::Strategy alloc) const noexcept(false) -> Positions final;
    auto BestChain() const noexcept -> block::Position final;
    auto BestChain(
        const block::Position& tip,
        const std::size_t limit,
        alloc::Strategy alloc) const noexcept(false) -> Positions final;
    auto BestHash(const block::Height height) const noexcept
        -> block::Hash final;
    auto BestHash(const block::Height height, const block::Position& check)
        const noexcept -> block::Hash final;
    auto BestHashes(
        const block::Height start,
        const std::size_t limit,
        alloc::Strategy alloc) const noexcept -> Hashes final;
    auto BestHashes(
        const block::Height start,
        const block::Hash& stop,
        const std::size_t limit,
        alloc::Strategy alloc) const noexcept -> Hashes final;
    auto BestHashes(
        const std::span<const block::Hash> previous,
        const block::Hash& stop,
        const std::size_t limit,
        alloc::Strategy alloc) const noexcept -> Hashes final;
    using node::HeaderOracle::CalculateReorg;
    auto CalculateReorg(const block::Position& tip, alloc::Strategy alloc) const
        noexcept(false) -> Positions final;
    auto CalculateReorg(
        const HeaderOraclePrivate& data,
        const block::Position& tip,
        alloc::Strategy alloc) const noexcept(false) -> Positions;
    auto CommonParent(const block::Position& position) const noexcept
        -> std::pair<block::Position, block::Position> final;
    auto Execute(Vector<ReorgTask>&& jobs) const noexcept -> bool;
    auto Exists(const block::Hash& hash) const noexcept -> bool final;
    auto GetCheckpoint() const noexcept -> block::Position final;
    auto GetDefaultCheckpoint() const noexcept -> CheckpointData;
    auto GetJob(alloc::Strategy alloc) const noexcept -> HeaderJob;
    using node::HeaderOracle::GetPosition;
    auto GetPosition(const block::Height height) const noexcept
        -> block::Position final;
    auto GetPosition(
        const HeaderOraclePrivate& data,
        const block::Height height) const noexcept -> block::Position;
    auto Internal() const noexcept -> const internal::HeaderOracle& final
    {
        return *this;
    }
    auto IsInBestChain(const block::Hash& hash) const noexcept -> bool final;
    auto IsInBestChain(const block::Position& position) const noexcept
        -> bool final;
    auto IsSynchronized() const noexcept -> bool;
    auto LoadHeader(const block::Hash& hash) const noexcept
        -> block::Header final;
    auto RecentHashes(alloc::Strategy alloc) const noexcept -> Hashes final;
    auto Siblings() const noexcept -> UnallocatedSet<block::Hash> final;
    auto Target() const noexcept -> block::Height;

    auto AddCheckpoint(
        const block::Height position,
        const block::Hash& requiredHash) noexcept -> bool;
    auto AddHeader(block::Header) noexcept -> bool;
    auto AddHeaders(std::span<block::Header>) noexcept -> bool;
    auto DeleteCheckpoint() noexcept -> bool;
    auto Init() noexcept -> void;
    auto Internal() noexcept -> internal::HeaderOracle& final { return *this; }
    auto ProcessSyncData(
        block::Hash& prior,
        Vector<block::Hash>& hashes,
        const network::otdht::Data& data) noexcept -> std::size_t;
    auto Start(
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<const node::Manager> node) noexcept -> void;
    auto SubmitBlock(const ReadView in) noexcept -> void;

    HeaderOracle(boost::shared_ptr<Shared> shared) noexcept;
    HeaderOracle() = delete;
    HeaderOracle(const HeaderOracle&) = delete;
    HeaderOracle(HeaderOracle&&) noexcept;
    auto operator=(const HeaderOracle&) -> HeaderOracle& = delete;
    auto operator=(HeaderOracle&&) -> HeaderOracle& = delete;

    ~HeaderOracle() final;

private:
    // TODO switch to std::shared_ptr once the android ndk ships a version of
    // libc++ with unfucked pmr / allocate_shared support
    boost::shared_ptr<Shared> shared_;
};
}  // namespace opentxs::blockchain::node::internal
