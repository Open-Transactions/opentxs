// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Getaddr.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/PMR.hpp"
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"

namespace opentxs::network::blockchain::bitcoin::message::internal
{
Getaddr::Getaddr(allocator_type alloc) noexcept
    : Getaddr(MessagePrivate::Blank(alloc))
{
}

Getaddr::Getaddr(MessagePrivate* imp) noexcept
    : Message(std::move(imp))
{
}

Getaddr::Getaddr(const Getaddr& rhs, allocator_type alloc) noexcept
    : Message(rhs, alloc)
{
}

Getaddr::Getaddr(Getaddr&& rhs) noexcept
    : Message(std::move(rhs))
{
}

Getaddr::Getaddr(Getaddr&& rhs, allocator_type alloc) noexcept
    : Message(std::move(rhs), alloc)
{
}

auto Getaddr::Blank() noexcept -> Getaddr&
{
    static auto blank = Getaddr{};

    return blank;
}

auto Getaddr::operator=(const Getaddr& rhs) noexcept -> Getaddr&
{
    return copy_assign_child<Message>(*this, rhs);
}

auto Getaddr::operator=(Getaddr&& rhs) noexcept -> Getaddr&
{
    return move_assign_child<Message>(*this, std::move(rhs));
}

Getaddr::~Getaddr() = default;
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
