// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/block/transaction/Imp.hpp"  // IWYU pragma: associated

#include <utility>

namespace opentxs::blockchain::block::implementation
{
Transaction::Transaction(
    TransactionHash id,
    TransactionHash hash,
    allocator_type alloc) noexcept
    : TransactionPrivate(alloc)
    , id_(std::move(id))
    , hash_(std::move(hash))
{
}

Transaction::Transaction(const Transaction& rhs, allocator_type alloc) noexcept
    : TransactionPrivate(rhs, alloc)
    , id_(rhs.id_)
    , hash_(rhs.hash_)
{
}

Transaction::~Transaction() = default;
}  // namespace opentxs::blockchain::block::implementation
