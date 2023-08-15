// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/block/transaction/TransactionPrivate.hpp"  // IWYU pragma: associated

#include <utility>

#include "blockchain/protocol/bitcoin/base/block/transaction/TransactionPrivate.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Transaction.hpp"

namespace opentxs::blockchain::block
{
TransactionPrivate::TransactionPrivate(allocator_type alloc) noexcept
    : Allocated(alloc)
{
}

TransactionPrivate::TransactionPrivate(
    const TransactionPrivate&,
    allocator_type alloc) noexcept
    : TransactionPrivate(std::move(alloc))
{
}

auto TransactionPrivate::asBitcoinPrivate() const noexcept
    -> const protocol::bitcoin::base::block::TransactionPrivate*
{
    static const auto blank =
        protocol::bitcoin::base::block::TransactionPrivate{{}};

    return std::addressof(blank);
}

auto TransactionPrivate::asBitcoinPrivate() noexcept
    -> protocol::bitcoin::base::block::TransactionPrivate*
{
    static auto blank = protocol::bitcoin::base::block::TransactionPrivate{{}};

    return std::addressof(blank);
}

auto TransactionPrivate::asBitcoinPublic() const noexcept
    -> const protocol::bitcoin::base::block::Transaction&
{
    return protocol::bitcoin::base::block::Transaction::Blank();
}

auto TransactionPrivate::asBitcoinPublic() noexcept
    -> protocol::bitcoin::base::block::Transaction&
{
    return protocol::bitcoin::base::block::Transaction::Blank();
}

auto TransactionPrivate::Reset(block::Transaction& tx) noexcept -> void
{
    tx.imp_ = nullptr;
}

TransactionPrivate::~TransactionPrivate() = default;
}  // namespace opentxs::blockchain::block
