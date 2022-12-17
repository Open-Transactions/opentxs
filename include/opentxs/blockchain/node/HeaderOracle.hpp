// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <tuple>
#include <utility>

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class Header;
}  // namespace block

namespace node
{
namespace internal
{
class HeaderOracle;
}  // namespace internal
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node
{
class OPENTXS_EXPORT HeaderOracle
{
public:
    using Hashes = Vector<block::Hash>;
    using Positions = Vector<block::Position>;

    /** Query a partial set of ancestors of a target block
     *
     *  This function always returns at least one position.
     *
     *  The first position returned will be the newest common ancestor between
     *  the target position and the previous chain as indicated by the start
     *  argument. If no reorg has occurred this will be the start argument.
     *
     *  Subsequent positions represent every block following the common ancestor
     *  up to and including the specified stop position, in ascending order.
     *
     *  \throws std::runtime_error if either of the specified positions does not
     *  exist in the database
     */
    virtual auto Ancestors(
        const block::Position& start,
        const block::Position& target,
        const std::size_t limit = 0,
        alloc::Default alloc = {}) const noexcept(false) -> Positions = 0;
    virtual auto Ancestors(
        const block::Position& start,
        const std::size_t limit = 0,
        alloc::Default alloc = {}) const noexcept(false) -> Positions = 0;
    virtual auto BestChain() const noexcept -> block::Position = 0;
    /** Determine which blocks have updated since the provided position
     *
     *  This function always returns at least one position.
     *
     *  The first position returned will be the newest common ancestor between
     *  the current best chain and the previous chain as indicated by the tip
     *  argument. If no reorg has occurred this will be the tip argument.
     *
     *  Subsequent positions represent every block following the common ancestor
     *  up to and including the current best tip, in ascending order.
     *
     *  \throws std::runtime_error if the specified tip does not exist in the
     *  database
     */
    virtual auto BestChain(
        const block::Position& tip,
        const std::size_t limit = 0,
        alloc::Default alloc = {}) const noexcept(false) -> Positions = 0;
    virtual auto BestHash(const block::Height height) const noexcept
        -> block::Hash = 0;
    /** Returns best hash at specified height if the provides position is still
     *  in the best chain
     *
     *  Returns the best hash at the specified height if it exists and if the
     *  check position is in the best chain, otherwise returns an empty value.
     *
     */
    virtual auto BestHash(
        const block::Height height,
        const block::Position& check) const noexcept -> block::Hash = 0;
    virtual auto BestHashes(
        const block::Height start,
        const std::size_t limit = 0,
        alloc::Default alloc = {}) const noexcept -> Hashes = 0;
    virtual auto BestHashes(
        const block::Height start,
        const block::Hash& stop,
        const std::size_t limit = 0,
        alloc::Default alloc = {}) const noexcept -> Hashes = 0;
    virtual auto BestHashes(
        const Hashes& previous,
        const block::Hash& stop,
        const std::size_t limit,
        alloc::Default alloc = {}) const noexcept -> Hashes = 0;
    /** Determine how which ancestors of a orphaned tip must be rolled back
     *  due to a chain reorg
     *
     *  If the provided tip is in the best chain, the returned vector will be
     *  empty.
     *
     *  Otherwise it will contain a list of orphaned block positions in
     *  descending order starting from the provided tip. The parent block hash
     *  of the block indicated by the final element in the vector
     *  is in the best chain.
     *
     *  \throws std::runtime_error if the provided position is not a descendant
     *  of this chain's genesis block
     */
    virtual auto CalculateReorg(
        const block::Position& tip,
        alloc::Default alloc = {}) const noexcept(false) -> Positions = 0;
    /** Test block position for membership in the best chain
     *
     *  returns {parent position, best position}
     *
     *  parent position is the input block position if that position is in the
     * best chain, otherwise it is the youngest common ancestor of the input
     * block and best chain
     */
    virtual auto CommonParent(const block::Position& input) const noexcept
        -> std::pair<block::Position, block::Position> = 0;
    virtual auto Exists(const block::Hash& hash) const noexcept -> bool = 0;
    virtual auto GetCheckpoint() const noexcept -> block::Position = 0;
    virtual auto GetPosition(const block::Height height) const noexcept
        -> block::Position = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::HeaderOracle& = 0;
    virtual auto IsInBestChain(const block::Hash& hash) const noexcept
        -> bool = 0;
    virtual auto IsInBestChain(const block::Position& position) const noexcept
        -> bool = 0;
    virtual auto LoadHeader(const block::Hash& hash) const noexcept
        -> block::Header = 0;
    virtual auto RecentHashes(alloc::Default alloc = {}) const noexcept
        -> Hashes = 0;
    virtual auto Siblings() const noexcept -> UnallocatedSet<block::Hash> = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept
        -> internal::HeaderOracle& = 0;

    HeaderOracle(const HeaderOracle&) = delete;
    HeaderOracle(HeaderOracle&&) = delete;
    auto operator=(const HeaderOracle&) -> HeaderOracle& = delete;
    auto operator=(HeaderOracle&&) -> HeaderOracle& = delete;

    virtual ~HeaderOracle() = default;

protected:
    HeaderOracle() noexcept = default;
};
}  // namespace opentxs::blockchain::node
