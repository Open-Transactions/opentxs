// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Cfcheckpt.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/PMR.hpp"
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/cfcheckpt/MessagePrivate.hpp"

namespace opentxs::network::blockchain::bitcoin::message::internal
{
Cfcheckpt::Cfcheckpt(allocator_type alloc) noexcept
    : Cfcheckpt(MessagePrivate::Blank(alloc))
{
}

Cfcheckpt::Cfcheckpt(MessagePrivate* imp) noexcept
    : Message(std::move(imp))
{
}

Cfcheckpt::Cfcheckpt(const Cfcheckpt& rhs, allocator_type alloc) noexcept
    : Message(rhs, alloc)
{
}

Cfcheckpt::Cfcheckpt(Cfcheckpt&& rhs) noexcept
    : Message(std::move(rhs))
{
}

Cfcheckpt::Cfcheckpt(Cfcheckpt&& rhs, allocator_type alloc) noexcept
    : Message(std::move(rhs), alloc)
{
}

auto Cfcheckpt::Blank() noexcept -> Cfcheckpt&
{
    static auto blank = Cfcheckpt{};

    return blank;
}

auto Cfcheckpt::get() const noexcept -> std::span<const value_type>
{
    return imp_->asCfcheckptPrivate()->get();
}

auto Cfcheckpt::get() noexcept -> std::span<value_type>
{
    return imp_->asCfcheckptPrivate()->get();
}

auto Cfcheckpt::operator=(const Cfcheckpt& rhs) noexcept -> Cfcheckpt&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::copy_assign_child<Message>(*this, rhs);
}

auto Cfcheckpt::operator=(Cfcheckpt&& rhs) noexcept -> Cfcheckpt&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::move_assign_child<Message>(*this, std::move(rhs));
}

auto Cfcheckpt::Stop() const noexcept -> const opentxs::blockchain::block::Hash&
{
    return imp_->asCfcheckptPrivate()->Stop();
}

auto Cfcheckpt::Type() const noexcept -> opentxs::blockchain::cfilter::Type
{
    return imp_->asCfcheckptPrivate()->Type();
}

Cfcheckpt::~Cfcheckpt() = default;
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
