// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/block/block/BlockPrivate.hpp"  // IWYU pragma: associated

#include <utility>

#include "blockchain/protocol/bitcoin/base/block/block/BlockPrivate.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Block.hpp"

namespace opentxs::blockchain::block
{
BlockPrivate::BlockPrivate(allocator_type alloc) noexcept
    : Allocated(alloc)
{
}

BlockPrivate::BlockPrivate(const BlockPrivate&, allocator_type alloc) noexcept
    : BlockPrivate(std::move(alloc))
{
}

auto BlockPrivate::asBitcoinPrivate() const noexcept
    -> const protocol::bitcoin::base::block::BlockPrivate*
{
    static const auto blank = protocol::bitcoin::base::block::BlockPrivate{{}};

    return std::addressof(blank);
}

auto BlockPrivate::asBitcoinPrivate() noexcept
    -> protocol::bitcoin::base::block::BlockPrivate*
{
    static auto blank = protocol::bitcoin::base::block::BlockPrivate{{}};

    return std::addressof(blank);
}

auto BlockPrivate::asBitcoinPublic() const noexcept
    -> const protocol::bitcoin::base::block::Block&
{
    return protocol::bitcoin::base::block::Block::Blank();
}

auto BlockPrivate::asBitcoinPublic() noexcept
    -> protocol::bitcoin::base::block::Block&
{
    return protocol::bitcoin::base::block::Block::Blank();
}

auto BlockPrivate::Reset(block::Block& header) noexcept -> void
{
    header.imp_ = nullptr;
}

BlockPrivate::~BlockPrivate() = default;
}  // namespace opentxs::blockchain::block
