// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <span>

#include "blockchain/block/block/BlockPrivate.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"

namespace opentxs::blockchain::block::implementation
{
class Block : virtual public BlockPrivate
{
public:
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> BlockPrivate* override
    {
        return pmr::clone_as<BlockPrivate>(this, {alloc});
    }
    auto ContainsHash(const TransactionHash& hash) const noexcept -> bool final;
    auto ContainsID(const TransactionHash& id) const noexcept -> bool final;
    auto FindByHash(const TransactionHash& hash) const noexcept
        -> const block::Transaction& final;
    auto FindByID(const TransactionHash& id) const noexcept
        -> const block::Transaction& final;
    auto get() const noexcept -> std::span<const block::Transaction> final
    {
        return transactions_;
    }
    auto Header() const noexcept -> const block::Header& final
    {
        return header_;
    }
    auto ID() const noexcept -> const block::Hash& final
    {
        return header_.Hash();
    }
    auto size() const noexcept -> std::size_t final
    {
        return transactions_.size();
    }

    [[nodiscard]] auto get_deleter() noexcept -> delete_function override
    {
        return pmr::make_deleter(this);
    }

    Block() = delete;
    Block(const Block& rhs, allocator_type alloc) noexcept;
    Block(const Block&) = delete;
    Block(Block&&) = delete;
    auto operator=(const Block&) -> Block& = delete;
    auto operator=(Block&&) -> Block& = delete;

    ~Block() override;

protected:
    const block::Header header_;
    const TxidIndex id_index_;
    const TxidIndex hash_index_;
    const TransactionMap transactions_;

    Block(
        block::Header header,
        TxidIndex&& ids,
        TxidIndex&& hashes,
        TransactionMap&& transactions,
        allocator_type alloc) noexcept;
};
}  // namespace opentxs::blockchain::block::implementation
