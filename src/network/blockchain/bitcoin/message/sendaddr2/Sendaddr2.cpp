// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Sendaddr2.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/PMR.hpp"
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"

namespace opentxs::network::blockchain::bitcoin::message::internal
{
Sendaddr2::Sendaddr2(allocator_type alloc) noexcept
    : Sendaddr2(MessagePrivate::Blank(alloc))
{
}

Sendaddr2::Sendaddr2(MessagePrivate* imp) noexcept
    : Message(std::move(imp))
{
}

Sendaddr2::Sendaddr2(const Sendaddr2& rhs, allocator_type alloc) noexcept
    : Message(rhs, alloc)
{
}

Sendaddr2::Sendaddr2(Sendaddr2&& rhs) noexcept
    : Message(std::move(rhs))
{
}

Sendaddr2::Sendaddr2(Sendaddr2&& rhs, allocator_type alloc) noexcept
    : Message(std::move(rhs), alloc)
{
}

auto Sendaddr2::Blank() noexcept -> Sendaddr2&
{
    static auto blank = Sendaddr2{};

    return blank;
}

auto Sendaddr2::operator=(const Sendaddr2& rhs) noexcept -> Sendaddr2&
{
    return copy_assign_child<Message>(*this, rhs);
}

auto Sendaddr2::operator=(Sendaddr2&& rhs) noexcept -> Sendaddr2&
{
    return move_assign_child<Message>(*this, std::move(rhs));
}

Sendaddr2::~Sendaddr2() = default;
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
