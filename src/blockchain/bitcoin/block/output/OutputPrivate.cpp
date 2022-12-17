// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/bitcoin/block/output/OutputPrivate.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/BoostPMR.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/bitcoin/block/Output.hpp"
#include "opentxs/util/Allocator.hpp"

namespace opentxs::blockchain::bitcoin::block
{
OutputPrivate::OutputPrivate(allocator_type alloc) noexcept
    : Allocated(std::move(alloc))
{
}

OutputPrivate::OutputPrivate(
    const OutputPrivate&,
    allocator_type alloc) noexcept
    : OutputPrivate(std::move(alloc))
{
}

auto OutputPrivate::Blank(allocator_type alloc) noexcept -> OutputPrivate*
{
    auto pmr = alloc::PMR<OutputPrivate>{alloc};
    auto* out = pmr.allocate(1_uz);

    OT_ASSERT(nullptr != out);

    pmr.construct(out);

    return out;
}

auto OutputPrivate::clone(allocator_type alloc) const noexcept -> OutputPrivate*
{
    auto pmr = alloc::PMR<OutputPrivate>{alloc};
    auto* out = pmr.allocate(1_uz);

    OT_ASSERT(nullptr != out);

    pmr.construct(out, *this);

    return out;
}

auto OutputPrivate::get_deleter() noexcept -> std::function<void()>
{
    return make_deleter(this);
}

auto OutputPrivate::Reset(block::Output& tx) noexcept -> void
{
    tx.imp_ = nullptr;
}

OutputPrivate::~OutputPrivate() = default;
}  // namespace opentxs::blockchain::bitcoin::block
