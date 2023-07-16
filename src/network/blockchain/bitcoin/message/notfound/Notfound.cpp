// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Notfound.hpp"  // IWYU pragma: associated

#include <utility>

#include "blockchain/bitcoin/Inventory.hpp"  // IWYU pragma: keep
#include "internal/util/PMR.hpp"
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/notfound/MessagePrivate.hpp"

namespace opentxs::network::blockchain::bitcoin::message::internal
{
Notfound::Notfound(allocator_type alloc) noexcept
    : Notfound(MessagePrivate::Blank(alloc))
{
}

Notfound::Notfound(MessagePrivate* imp) noexcept
    : Message(std::move(imp))
{
}

Notfound::Notfound(const Notfound& rhs, allocator_type alloc) noexcept
    : Message(rhs, alloc)
{
}

Notfound::Notfound(Notfound&& rhs) noexcept
    : Message(std::move(rhs))
{
}

Notfound::Notfound(Notfound&& rhs, allocator_type alloc) noexcept
    : Message(std::move(rhs), alloc)
{
}

auto Notfound::Blank() noexcept -> Notfound&
{
    static auto blank = Notfound{};

    return blank;
}

auto Notfound::get() const noexcept -> std::span<const value_type>
{
    return imp_->asNotfoundPrivate()->get();
}

auto Notfound::get() noexcept -> std::span<value_type>
{
    return imp_->asNotfoundPrivate()->get();
}

auto Notfound::operator=(const Notfound& rhs) noexcept -> Notfound&
{
    return pmr::copy_assign_child<Message>(*this, rhs);
}

auto Notfound::operator=(Notfound&& rhs) noexcept -> Notfound&
{
    return pmr::move_assign_child<Message>(*this, std::move(rhs));
}

Notfound::~Notfound() = default;
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
