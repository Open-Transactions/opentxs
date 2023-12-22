// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Getheaders.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/PMR.hpp"
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/getheaders/MessagePrivate.hpp"

namespace opentxs::network::blockchain::bitcoin::message::internal
{
Getheaders::Getheaders(allocator_type alloc) noexcept
    : Getheaders(MessagePrivate::Blank(alloc))
{
}

Getheaders::Getheaders(MessagePrivate* imp) noexcept
    : Message(std::move(imp))
{
}

Getheaders::Getheaders(const Getheaders& rhs, allocator_type alloc) noexcept
    : Message(rhs, alloc)
{
}

Getheaders::Getheaders(Getheaders&& rhs) noexcept
    : Message(std::move(rhs))
{
}

Getheaders::Getheaders(Getheaders&& rhs, allocator_type alloc) noexcept
    : Message(std::move(rhs), alloc)
{
}

auto Getheaders::Blank() noexcept -> Getheaders&
{
    static auto blank = Getheaders{};

    return blank;
}

auto Getheaders::get() const noexcept -> std::span<const value_type>
{
    return imp_->asGetheadersPrivate()->get();
}

auto Getheaders::get() noexcept -> std::span<value_type>
{
    return imp_->asGetheadersPrivate()->get();
}

auto Getheaders::operator=(const Getheaders& rhs) noexcept -> Getheaders&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::copy_assign_child<Message>(*this, rhs);
}

auto Getheaders::operator=(Getheaders&& rhs) noexcept -> Getheaders&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::move_assign_child<Message>(*this, std::move(rhs));
}

auto Getheaders::Stop() const noexcept
    -> const opentxs::blockchain::block::Hash&
{
    return imp_->asGetheadersPrivate()->Stop();
}

Getheaders::~Getheaders() = default;
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
