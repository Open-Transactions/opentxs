// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/bitcoin/block/transaction/TransactionPrivate.hpp"  // IWYU pragma: associated

#include <utility>

namespace opentxs::blockchain::bitcoin::block
{
TransactionPrivate::TransactionPrivate(allocator_type alloc) noexcept
    : blockchain::block::TransactionPrivate(std::move(alloc))
    , self_(this)
{
}

TransactionPrivate::TransactionPrivate(
    const TransactionPrivate& rhs,
    allocator_type alloc) noexcept
    : blockchain::block::TransactionPrivate(rhs, std::move(alloc))
    , self_(this)
{
}

TransactionPrivate::~TransactionPrivate() { Reset(self_); }
}  // namespace opentxs::blockchain::bitcoin::block
