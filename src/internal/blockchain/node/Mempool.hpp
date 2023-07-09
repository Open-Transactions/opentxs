// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <span>

#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class Block;
class Transaction;
class TransactionHash;
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::internal
{
class Mempool
{
public:
    virtual auto Dump(alloc::Strategy alloc) const noexcept
        -> Set<block::TransactionHash> = 0;
    virtual auto Prune(const block::Block& block, alloc::Strategy monotonic)
        const noexcept -> void = 0;
    virtual auto Query(
        const block::TransactionHash& txid,
        alloc::Strategy alloc) const noexcept -> block::Transaction = 0;
    virtual auto Submit(const block::TransactionHash& txid) const noexcept
        -> bool = 0;
    virtual auto Submit(
        std::span<const block::TransactionHash> txids,
        alloc::Strategy alloc) const noexcept -> Vector<bool> = 0;
    virtual auto Submit(block::Transaction tx) const noexcept -> void = 0;

    virtual auto Heartbeat() noexcept -> void = 0;

    virtual ~Mempool() = default;
};
}  // namespace opentxs::blockchain::node::internal
