// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Verack.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/PMR.hpp"
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"

namespace opentxs::network::blockchain::bitcoin::message::internal
{
Verack::Verack(allocator_type alloc) noexcept
    : Verack(MessagePrivate::Blank(alloc))
{
}

Verack::Verack(MessagePrivate* imp) noexcept
    : Message(std::move(imp))
{
}

Verack::Verack(const Verack& rhs, allocator_type alloc) noexcept
    : Message(rhs, alloc)
{
}

Verack::Verack(Verack&& rhs) noexcept
    : Message(std::move(rhs))
{
}

Verack::Verack(Verack&& rhs, allocator_type alloc) noexcept
    : Message(std::move(rhs), alloc)
{
}

auto Verack::Blank() noexcept -> Verack&
{
    static auto blank = Verack{};

    return blank;
}

auto Verack::operator=(const Verack& rhs) noexcept -> Verack&
{
    return copy_assign_child<Message>(*this, rhs);
}

auto Verack::operator=(Verack&& rhs) noexcept -> Verack&
{
    return move_assign_child<Message>(*this, std::move(rhs));
}

Verack::~Verack() = default;
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
