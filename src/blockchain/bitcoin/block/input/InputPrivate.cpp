// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/bitcoin/block/input/InputPrivate.hpp"  // IWYU pragma: associated

#include <utility>

#include "opentxs/blockchain/bitcoin/block/Input.hpp"

namespace opentxs::blockchain::bitcoin::block
{
InputPrivate::InputPrivate(allocator_type alloc) noexcept
    : Allocated(alloc)
{
}

InputPrivate::InputPrivate(const InputPrivate&, allocator_type alloc) noexcept
    : InputPrivate(std::move(alloc))
{
}

auto InputPrivate::Reset(block::Input& tx) noexcept -> void
{
    tx.imp_ = nullptr;
}

InputPrivate::~InputPrivate() = default;
}  // namespace opentxs::blockchain::bitcoin::block
