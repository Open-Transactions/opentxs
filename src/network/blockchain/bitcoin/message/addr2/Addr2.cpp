// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Addr2.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/PMR.hpp"
#include "network/blockchain/bitcoin/message/addr2/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"

namespace opentxs::network::blockchain::bitcoin::message::internal
{
Addr2::Addr2(allocator_type alloc) noexcept
    : Addr2(MessagePrivate::Blank(alloc))
{
}

Addr2::Addr2(MessagePrivate* imp) noexcept
    : Message(std::move(imp))
{
}

Addr2::Addr2(const Addr2& rhs, allocator_type alloc) noexcept
    : Message(rhs, alloc)
{
}

Addr2::Addr2(Addr2&& rhs) noexcept
    : Message(std::move(rhs))
{
}

Addr2::Addr2(Addr2&& rhs, allocator_type alloc) noexcept
    : Message(std::move(rhs), alloc)
{
}

auto Addr2::Blank() noexcept -> Addr2&
{
    static auto blank = Addr2{};

    return blank;
}

auto Addr2::get() const noexcept -> std::span<const value_type>
{
    return imp_->asAddr2Private()->get();
}

auto Addr2::get() noexcept -> std::span<value_type>
{
    return imp_->asAddr2Private()->get();
}

auto Addr2::operator=(const Addr2& rhs) noexcept -> Addr2&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::copy_assign_child<Message>(*this, rhs);
}

auto Addr2::operator=(Addr2&& rhs) noexcept -> Addr2&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::move_assign_child<Message>(*this, std::move(rhs));
}

Addr2::~Addr2() = default;
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
