// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Block.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/PMR.hpp"
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/block/MessagePrivate.hpp"

namespace opentxs::network::blockchain::bitcoin::message::internal
{
Block::Block(allocator_type alloc) noexcept
    : Block(MessagePrivate::Blank(alloc))
{
}

Block::Block(MessagePrivate* imp) noexcept
    : Message(std::move(imp))
{
}

Block::Block(const Block& rhs, allocator_type alloc) noexcept
    : Message(rhs, alloc)
{
}

Block::Block(Block&& rhs) noexcept
    : Message(std::move(rhs))
{
}

Block::Block(Block&& rhs, allocator_type alloc) noexcept
    : Message(std::move(rhs), alloc)
{
}

auto Block::Blank() noexcept -> Block&
{
    static auto blank = Block{};

    return blank;
}

auto Block::get() const noexcept -> ReadView
{
    return imp_->asBlockPrivate()->get();
}

auto Block::operator=(const Block& rhs) noexcept -> Block&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::copy_assign_child<Message>(*this, rhs);
}

auto Block::operator=(Block&& rhs) noexcept -> Block&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::move_assign_child<Message>(*this, std::move(rhs));
}

Block::~Block() = default;
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
