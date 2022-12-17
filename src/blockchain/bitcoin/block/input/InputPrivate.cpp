// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "blockchain/bitcoin/block/input/InputPrivate.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/BoostPMR.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/bitcoin/block/Input.hpp"
#include "opentxs/util/Allocator.hpp"

namespace opentxs::blockchain::bitcoin::block
{
InputPrivate::InputPrivate(allocator_type alloc) noexcept
    : Allocated(std::move(alloc))
{
}

InputPrivate::InputPrivate(const InputPrivate&, allocator_type alloc) noexcept
    : InputPrivate(std::move(alloc))
{
}

auto InputPrivate::Blank(allocator_type alloc) noexcept -> InputPrivate*
{
    auto pmr = alloc::PMR<InputPrivate>{alloc};
    auto* out = pmr.allocate(1_uz);

    OT_ASSERT(nullptr != out);

    pmr.construct(out);

    return out;
}

auto InputPrivate::clone(allocator_type alloc) const noexcept -> InputPrivate*
{
    auto pmr = alloc::PMR<InputPrivate>{alloc};
    auto* out = pmr.allocate(1_uz);

    OT_ASSERT(nullptr != out);

    pmr.construct(out, *this);

    return out;
}

auto InputPrivate::get_deleter() noexcept -> std::function<void()>
{
    return make_deleter(this);
}

auto InputPrivate::Reset(block::Input& tx) noexcept -> void
{
    tx.imp_ = nullptr;
}

InputPrivate::~InputPrivate() = default;
}  // namespace opentxs::blockchain::bitcoin::block
