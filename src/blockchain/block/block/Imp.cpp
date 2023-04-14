// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/block/block/Imp.hpp"  // IWYU pragma: associated

#include <utility>
#include <vector>

#include "opentxs/util/Allocator.hpp"

namespace opentxs::blockchain::block::implementation
{
Block::Block(
    block::Header header,
    TxidIndex&& ids,
    TxidIndex&& hashes,
    TransactionMap&& transactions,
    allocator_type alloc) noexcept
    : BlockPrivate(alloc)
    , header_(std::move(header), alloc)
    , id_index_(std::move(ids), alloc)
    , hash_index_(std::move(hashes), alloc)
    , transactions_(std::move(transactions), alloc)
{
}

Block::Block(const Block& rhs, allocator_type alloc) noexcept
    : BlockPrivate(rhs, alloc)
    , header_(rhs.header_, alloc)
    , id_index_(rhs.id_index_, alloc)
    , hash_index_(rhs.hash_index_, alloc)
    , transactions_(rhs.transactions_, alloc)
{
}

auto Block::ContainsHash(const TransactionHash& hash) const noexcept -> bool
{
    return hash_index_.contains(hash);
}

auto Block::ContainsID(const TransactionHash& id) const noexcept -> bool
{
    return id_index_.contains(id);
}

auto Block::FindByHash(const TransactionHash& hash) const noexcept
    -> const block::Transaction&
{
    if (auto i = hash_index_.find(hash); hash_index_.end() != i) {

        return transactions_[i->second];
    } else {

        return block::Transaction::Blank();
    }
}

auto Block::FindByID(const TransactionHash& id) const noexcept
    -> const block::Transaction&
{
    if (auto i = id_index_.find(id); id_index_.end() != i) {

        return transactions_[i->second];
    } else {

        return block::Transaction::Blank();
    }
}

Block::~Block() = default;
}  // namespace opentxs::blockchain::block::implementation
