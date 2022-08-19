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
#include <tuple>
#include <utility>

#include "internal/blockchain/node/Types.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

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

class HeaderOracle;
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
// }  // namespace v1
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
        const std::size_t limit) const noexcept(false) -> Positions final;
    auto BestChain() const noexcept -> block::Position final;
    auto BestChain(const block::Position& tip, const std::size_t limit) const
        noexcept(false) -> Positions final;
    auto BestHash(const block::Height height) const noexcept
        -> block::Hash final;
    auto BestHash(const block::Height height, const block::Position& check)
        const noexcept -> block::Hash final;
    auto BestHashes(
        const block::Height start,
        const std::size_t limit,
        alloc::Default alloc) const noexcept -> Hashes final;
    auto BestHashes(
        const block::Height start,
        const block::Hash& stop,
        const std::size_t limit,
        alloc::Default alloc) const noexcept -> Hashes final;
    auto BestHashes(
        const Hashes& previous,
        const block::Hash& stop,
        const std::size_t limit,
        alloc::Default alloc) const noexcept -> Hashes final;
    using node::HeaderOracle::CalculateReorg;
    auto CalculateReorg(const block::Position& tip) const noexcept(false)
        -> Positions final;
    auto CalculateReorg(
        const HeaderOraclePrivate& data,
        const block::Position& tip) const noexcept(false) -> Positions;
    auto CommonParent(const block::Position& position) const noexcept
        -> std::pair<block::Position, block::Position> final;
    auto Execute(Vector<ReorgTask>&& jobs) const noexcept -> bool;
    auto Exists(const block::Hash& hash) const noexcept -> bool final;
    auto GetCheckpoint() const noexcept -> block::Position final;
    auto GetDefaultCheckpoint() const noexcept -> CheckpointData;
    auto GetJob(alloc::Default alloc) const noexcept -> HeaderJob;
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
    auto LoadBitcoinHeader(const block::Hash& hash) const noexcept
        -> std::unique_ptr<bitcoin::block::Header>;
    auto LoadHeader(const block::Hash& hash) const noexcept
        -> std::unique_ptr<block::Header> final;
    auto RecentHashes(alloc::Default alloc) const noexcept -> Hashes final;
    auto Siblings() const noexcept -> UnallocatedSet<block::Hash> final;
    auto Target() const noexcept -> block::Height;

    auto AddCheckpoint(
        const block::Height position,
        const block::Hash& requiredHash) noexcept -> bool;
    auto AddHeader(std::unique_ptr<block::Header>) noexcept -> bool;
    auto AddHeaders(Vector<std::unique_ptr<block::Header>>&) noexcept -> bool;
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
