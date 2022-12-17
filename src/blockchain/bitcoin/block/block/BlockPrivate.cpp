// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/bitcoin/block/block/BlockPrivate.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/BoostPMR.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/util/Allocator.hpp"

namespace opentxs::blockchain::bitcoin::block
{
BlockPrivate::BlockPrivate(allocator_type alloc) noexcept
    : blockchain::block::BlockPrivate(std::move(alloc))
    , self_(this)
{
}

BlockPrivate::BlockPrivate(
    const BlockPrivate& rhs,
    allocator_type alloc) noexcept
    : blockchain::block::BlockPrivate(rhs, std::move(alloc))
    , self_(this)
{
}

auto BlockPrivate::Blank(allocator_type alloc) noexcept -> BlockPrivate*
{
    auto pmr = alloc::PMR<BlockPrivate>{alloc};
    auto* out = pmr.allocate(1_uz);

    OT_ASSERT(nullptr != out);

    pmr.construct(out);

    return out;
}

auto BlockPrivate::clone(allocator_type alloc) const noexcept
    -> blockchain::block::BlockPrivate*
{
    auto pmr = alloc::PMR<BlockPrivate>{alloc};
    auto* out = pmr.allocate(1_uz);

    OT_ASSERT(nullptr != out);

    pmr.construct(out, *this);

    return out;
}

auto BlockPrivate::get_deleter() noexcept -> std::function<void()>
{
    return make_deleter(this);
}

BlockPrivate::~BlockPrivate() { Reset(self_); }
}  // namespace opentxs::blockchain::bitcoin::block
