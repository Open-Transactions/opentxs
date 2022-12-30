// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/block/transaction/TransactionPrivate.hpp"  // IWYU pragma: associated

#include <utility>

#include "blockchain/bitcoin/block/transaction/TransactionPrivate.hpp"
#include "opentxs/blockchain/bitcoin/block/Transaction.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"

namespace opentxs::blockchain::block
{
TransactionPrivate::TransactionPrivate(allocator_type alloc) noexcept
    : Allocated(std::move(alloc))
{
}

TransactionPrivate::TransactionPrivate(
    const TransactionPrivate&,
    allocator_type alloc) noexcept
    : TransactionPrivate(std::move(alloc))
{
}

auto TransactionPrivate::asBitcoinPrivate() const noexcept
    -> const bitcoin::block::TransactionPrivate*
{
    static const auto blank = bitcoin::block::TransactionPrivate{{}};

    return std::addressof(blank);
}

auto TransactionPrivate::asBitcoinPrivate() noexcept
    -> bitcoin::block::TransactionPrivate*
{
    static auto blank = bitcoin::block::TransactionPrivate{{}};

    return std::addressof(blank);
}

auto TransactionPrivate::asBitcoinPublic() const noexcept
    -> const bitcoin::block::Transaction&
{
    return bitcoin::block::Transaction::Blank();
}

auto TransactionPrivate::asBitcoinPublic() noexcept
    -> bitcoin::block::Transaction&
{
    return bitcoin::block::Transaction::Blank();
}

auto TransactionPrivate::Reset(block::Transaction& tx) noexcept -> void
{
    tx.imp_ = nullptr;
}

TransactionPrivate::~TransactionPrivate() = default;
}  // namespace opentxs::blockchain::block
