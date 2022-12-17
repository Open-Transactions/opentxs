// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/bitcoin/block/script/ScriptPrivate.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/BoostPMR.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/bitcoin/block/Script.hpp"
#include "opentxs/util/Allocator.hpp"

namespace opentxs::blockchain::bitcoin::block
{
ScriptPrivate::ScriptPrivate(allocator_type alloc) noexcept
    : Allocated(std::move(alloc))
{
}

ScriptPrivate::ScriptPrivate(
    const ScriptPrivate&,
    allocator_type alloc) noexcept
    : ScriptPrivate(std::move(alloc))
{
}

auto ScriptPrivate::Blank(allocator_type alloc) noexcept -> ScriptPrivate*
{
    auto pmr = alloc::PMR<ScriptPrivate>{alloc};
    auto* out = pmr.allocate(1_uz);

    OT_ASSERT(nullptr != out);

    pmr.construct(out);

    return out;
}

auto ScriptPrivate::clone(allocator_type alloc) const noexcept -> ScriptPrivate*
{
    auto pmr = alloc::PMR<ScriptPrivate>{alloc};
    auto* out = pmr.allocate(1_uz);

    OT_ASSERT(nullptr != out);

    pmr.construct(out, *this);

    return out;
}

auto ScriptPrivate::get_deleter() noexcept -> std::function<void()>
{
    return make_deleter(this);
}

auto ScriptPrivate::Reset(block::Script& tx) noexcept -> void
{
    tx.imp_ = nullptr;
}

ScriptPrivate::~ScriptPrivate() = default;
}  // namespace opentxs::blockchain::bitcoin::block
