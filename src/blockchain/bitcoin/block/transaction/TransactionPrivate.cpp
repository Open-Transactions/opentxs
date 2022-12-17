// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/bitcoin/block/transaction/TransactionPrivate.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/BoostPMR.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/util/Allocator.hpp"

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

auto TransactionPrivate::Blank(allocator_type alloc) noexcept
    -> TransactionPrivate*
{
    auto pmr = alloc::PMR<TransactionPrivate>{alloc};
    auto* out = pmr.allocate(1_uz);

    OT_ASSERT(nullptr != out);

    pmr.construct(out);

    return out;
}

auto TransactionPrivate::clone(allocator_type alloc) const noexcept
    -> blockchain::block::TransactionPrivate*
{
    auto pmr = alloc::PMR<TransactionPrivate>{alloc};
    auto* out = pmr.allocate(1_uz);

    OT_ASSERT(nullptr != out);

    pmr.construct(out, *this);

    return out;
}

auto TransactionPrivate::get_deleter() noexcept -> std::function<void()>
{
    return make_deleter(this);
}

TransactionPrivate::~TransactionPrivate() { Reset(self_); }
}  // namespace opentxs::blockchain::bitcoin::block
