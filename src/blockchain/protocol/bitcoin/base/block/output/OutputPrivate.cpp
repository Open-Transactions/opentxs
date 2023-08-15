// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/protocol/bitcoin/base/block/output/OutputPrivate.hpp"  // IWYU pragma: associated

#include <utility>

#include "opentxs/blockchain/protocol/bitcoin/base/block/Output.hpp"

namespace opentxs::blockchain::protocol::bitcoin::base::block
{
OutputPrivate::OutputPrivate(allocator_type alloc) noexcept
    : Allocated(alloc)
{
}

OutputPrivate::OutputPrivate(
    const OutputPrivate&,
    allocator_type alloc) noexcept
    : OutputPrivate(std::move(alloc))
{
}

auto OutputPrivate::Reset(block::Output& tx) noexcept -> void
{
    tx.imp_ = nullptr;
}

OutputPrivate::~OutputPrivate() = default;
}  // namespace opentxs::blockchain::protocol::bitcoin::base::block
