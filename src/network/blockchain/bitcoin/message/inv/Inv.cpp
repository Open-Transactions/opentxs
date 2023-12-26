// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Inv.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/PMR.hpp"
#include "network/blockchain/bitcoin/Inventory.hpp"  // IWYU pragma: keep
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/inv/MessagePrivate.hpp"

namespace opentxs::network::blockchain::bitcoin::message::internal
{
Inv::Inv(allocator_type alloc) noexcept
    : Inv(MessagePrivate::Blank(alloc))
{
}

Inv::Inv(MessagePrivate* imp) noexcept
    : Message(std::move(imp))
{
}

Inv::Inv(const Inv& rhs, allocator_type alloc) noexcept
    : Message(rhs, alloc)
{
}

Inv::Inv(Inv&& rhs) noexcept
    : Message(std::move(rhs))
{
}

Inv::Inv(Inv&& rhs, allocator_type alloc) noexcept
    : Message(std::move(rhs), alloc)
{
}

auto Inv::Blank() noexcept -> Inv&
{
    static auto blank = Inv{};

    return blank;
}

auto Inv::get() const noexcept -> std::span<const value_type>
{
    return imp_->asInvPrivate()->get();
}

auto Inv::get() noexcept -> std::span<value_type>
{
    return imp_->asInvPrivate()->get();
}

auto Inv::operator=(const Inv& rhs) noexcept -> Inv&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::copy_assign_child<Message>(*this, rhs);
}

auto Inv::operator=(Inv&& rhs) noexcept -> Inv&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::move_assign_child<Message>(*this, std::move(rhs));
}

Inv::~Inv() = default;
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
