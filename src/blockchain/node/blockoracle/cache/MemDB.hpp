// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/container/flat_map.hpp>
#include <boost/container/vector.hpp>
#include <cstddef>
#include <functional>
#include <string_view>
#include <tuple>
#include <utility>

#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
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
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::blockoracle
{
class MemDB final : public Allocated
{
public:
    auto find(const ReadView id) const noexcept -> BitcoinBlockResult;
    auto get_allocator() const noexcept -> allocator_type final
    {
        return queue_.get_allocator();
    }

    auto clear() noexcept -> void;
    auto push(block::Hash&& id, BitcoinBlockResult&& future) noexcept -> void;

    MemDB(const std::size_t limit, allocator_type alloc) noexcept;

private:
    using CachedBlock = std::pair<block::Hash, BitcoinBlockResult>;
    using Completed = Deque<CachedBlock>;
    using Index = Map<ReadView, const CachedBlock*>;

    const std::size_t limit_;
    std::size_t bytes_;
    Completed queue_;
    Index index_;
};
}  // namespace opentxs::blockchain::node::blockoracle
