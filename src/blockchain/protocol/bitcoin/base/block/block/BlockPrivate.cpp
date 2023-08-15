// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/protocol/bitcoin/base/block/block/BlockPrivate.hpp"  // IWYU pragma: associated

#include <utility>

namespace opentxs::blockchain::protocol::bitcoin::base::block
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

BlockPrivate::~BlockPrivate() { Reset(self_); }
}  // namespace opentxs::blockchain::protocol::bitcoin::base::block
