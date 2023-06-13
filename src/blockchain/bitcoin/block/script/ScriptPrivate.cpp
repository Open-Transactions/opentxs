// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/bitcoin/block/script/ScriptPrivate.hpp"  // IWYU pragma: associated

#include <utility>

#include "opentxs/blockchain/bitcoin/block/Script.hpp"

namespace opentxs::blockchain::bitcoin::block
{
ScriptPrivate::ScriptPrivate(allocator_type alloc) noexcept
    : Allocated(alloc)
{
}

ScriptPrivate::ScriptPrivate(
    const ScriptPrivate&,
    allocator_type alloc) noexcept
    : ScriptPrivate(std::move(alloc))
{
}

auto ScriptPrivate::Reset(block::Script& tx) noexcept -> void
{
    tx.imp_ = nullptr;
}

ScriptPrivate::~ScriptPrivate() = default;
}  // namespace opentxs::blockchain::bitcoin::block
