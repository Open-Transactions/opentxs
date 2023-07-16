// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/blockchain/bitcoin/block/Block.hpp"  // IWYU pragma: associated

#include <utility>

#include "blockchain/bitcoin/block/block/BlockPrivate.hpp"
#include "blockchain/block/block/BlockPrivate.hpp"
#include "internal/util/PMR.hpp"

namespace opentxs::blockchain::bitcoin::block
{
Block::Block(blockchain::block::BlockPrivate* imp) noexcept
    : blockchain::block::Block(std::move(imp))
{
}

Block::Block(allocator_type alloc) noexcept
    : Block(BlockPrivate::Blank(alloc))
{
}

Block::Block(const Block& rhs, allocator_type alloc) noexcept
    : Block(rhs.imp_->clone(alloc))
{
}

Block::Block(Block&& rhs) noexcept
    : Block(std::exchange(rhs.imp_, nullptr))
{
}

Block::Block(Block&& rhs, allocator_type alloc) noexcept
    : Block(alloc)
{
    operator=(std::move(rhs));
}

auto Block::Blank() noexcept -> Block&
{
    static auto blank = Block{};

    return blank;
}

auto Block::operator=(const Block& rhs) noexcept -> Block&
{
    return pmr::copy_assign_child<blockchain::block::Block>(*this, rhs);
}

auto Block::operator=(Block&& rhs) noexcept -> Block&
{
    return pmr::move_assign_child<blockchain::block::Block>(
        *this, std::move(rhs));
}

Block::~Block() = default;
}  // namespace opentxs::blockchain::bitcoin::block
