// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Addr.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/PMR.hpp"
#include "network/blockchain/bitcoin/message/addr/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"

namespace opentxs::network::blockchain::bitcoin::message::internal
{
Addr::Addr(allocator_type alloc) noexcept
    : Addr(MessagePrivate::Blank(alloc))
{
}

Addr::Addr(MessagePrivate* imp) noexcept
    : Message(std::move(imp))
{
}

Addr::Addr(const Addr& rhs, allocator_type alloc) noexcept
    : Message(rhs, alloc)
{
}

Addr::Addr(Addr&& rhs) noexcept
    : Message(std::move(rhs))
{
}

Addr::Addr(Addr&& rhs, allocator_type alloc) noexcept
    : Message(std::move(rhs), alloc)
{
}

auto Addr::Blank() noexcept -> Addr&
{
    static auto blank = Addr{};

    return blank;
}

auto Addr::get() const noexcept -> std::span<const value_type>
{
    return imp_->asAddrPrivate()->get();
}

auto Addr::get() noexcept -> std::span<value_type>
{
    return imp_->asAddrPrivate()->get();
}

auto Addr::operator=(const Addr& rhs) noexcept -> Addr&
{
    return copy_assign_child<Message>(*this, rhs);
}

auto Addr::operator=(Addr&& rhs) noexcept -> Addr&
{
    return move_assign_child<Message>(*this, std::move(rhs));
}

Addr::~Addr() = default;
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
