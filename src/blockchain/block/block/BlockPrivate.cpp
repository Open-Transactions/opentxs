// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/block/block/BlockPrivate.hpp"  // IWYU pragma: associated

#include <utility>

#include "blockchain/bitcoin/block/block/BlockPrivate.hpp"
#include "opentxs/blockchain/bitcoin/block/Block.hpp"
#include "opentxs/blockchain/block/Block.hpp"

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
    -> const bitcoin::block::BlockPrivate*
{
    static const auto blank = bitcoin::block::BlockPrivate{{}};

    return std::addressof(blank);
}

auto BlockPrivate::asBitcoinPrivate() noexcept -> bitcoin::block::BlockPrivate*
{
    static auto blank = bitcoin::block::BlockPrivate{{}};

    return std::addressof(blank);
}

auto BlockPrivate::asBitcoinPublic() const noexcept
    -> const bitcoin::block::Block&
{
    return bitcoin::block::Block::Blank();
}

auto BlockPrivate::asBitcoinPublic() noexcept -> bitcoin::block::Block&
{
    return bitcoin::block::Block::Blank();
}

auto BlockPrivate::Reset(block::Block& header) noexcept -> void
{
    header.imp_ = nullptr;
}

BlockPrivate::~BlockPrivate() = default;
}  // namespace opentxs::blockchain::block
