// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Pong.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/PMR.hpp"
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/pong/MessagePrivate.hpp"

namespace opentxs::network::blockchain::bitcoin::message::internal
{
Pong::Pong(allocator_type alloc) noexcept
    : Pong(MessagePrivate::Blank(alloc))
{
}

Pong::Pong(MessagePrivate* imp) noexcept
    : Message(std::move(imp))
{
}

Pong::Pong(const Pong& rhs, allocator_type alloc) noexcept
    : Message(rhs, alloc)
{
}

Pong::Pong(Pong&& rhs) noexcept
    : Message(std::move(rhs))
{
}

Pong::Pong(Pong&& rhs, allocator_type alloc) noexcept
    : Message(std::move(rhs), alloc)
{
}

auto Pong::Blank() noexcept -> Pong&
{
    static auto blank = Pong{};

    return blank;
}

auto Pong::Nonce() const noexcept -> message::Nonce
{
    return imp_->asPongPrivate()->Nonce();
}

auto Pong::operator=(const Pong& rhs) noexcept -> Pong&
{
    return copy_assign_child<Message>(*this, rhs);
}

auto Pong::operator=(Pong&& rhs) noexcept -> Pong&
{
    return move_assign_child<Message>(*this, std::move(rhs));
}

Pong::~Pong() = default;
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
