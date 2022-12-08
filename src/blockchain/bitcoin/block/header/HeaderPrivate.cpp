// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "blockchain/bitcoin/block/header/HeaderPrivate.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/BoostPMR.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/util/Allocator.hpp"

namespace opentxs::blockchain::bitcoin::block
{
HeaderPrivate::HeaderPrivate(allocator_type alloc) noexcept
    : blockchain::block::HeaderPrivate(std::move(alloc))
    , self_(this)
{
}

HeaderPrivate::HeaderPrivate(
    const HeaderPrivate& rhs,
    allocator_type alloc) noexcept
    : blockchain::block::HeaderPrivate(rhs, std::move(alloc))
    , self_(this)
{
}

auto HeaderPrivate::Blank(allocator_type alloc) noexcept -> HeaderPrivate*
{
    auto pmr = alloc::PMR<HeaderPrivate>{alloc};
    auto* out = pmr.allocate(1_uz);

    OT_ASSERT(nullptr != out);

    pmr.construct(out);

    return out;
}

auto HeaderPrivate::clone(allocator_type alloc) const noexcept
    -> blockchain::block::HeaderPrivate*
{
    auto pmr = alloc::PMR<HeaderPrivate>{alloc};
    auto* out = pmr.allocate(1_uz);

    OT_ASSERT(nullptr != out);

    pmr.construct(out, *this);

    return out;
}

auto HeaderPrivate::get_deleter() noexcept -> std::function<void()>
{
    return make_deleter(this);
}

HeaderPrivate::~HeaderPrivate() { Reset(self_); }
}  // namespace opentxs::blockchain::bitcoin::block
