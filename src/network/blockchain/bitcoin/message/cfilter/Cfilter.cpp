// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Cfilter.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/PMR.hpp"
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/cfilter/MessagePrivate.hpp"

namespace opentxs::network::blockchain::bitcoin::message::internal
{
Cfilter::Cfilter(allocator_type alloc) noexcept
    : Cfilter(MessagePrivate::Blank(alloc))
{
}

Cfilter::Cfilter(MessagePrivate* imp) noexcept
    : Message(std::move(imp))
{
}

Cfilter::Cfilter(const Cfilter& rhs, allocator_type alloc) noexcept
    : Message(rhs, alloc)
{
}

Cfilter::Cfilter(Cfilter&& rhs) noexcept
    : Message(std::move(rhs))
{
}

Cfilter::Cfilter(Cfilter&& rhs, allocator_type alloc) noexcept
    : Message(std::move(rhs), alloc)
{
}

auto Cfilter::Bits() const noexcept -> std::uint8_t
{
    return imp_->asCfilterPrivate()->Bits();
}

auto Cfilter::Blank() noexcept -> Cfilter&
{
    static auto blank = Cfilter{};

    return blank;
}

auto Cfilter::ElementCount() const noexcept -> std::uint32_t
{
    return imp_->asCfilterPrivate()->ElementCount();
}

auto Cfilter::FPRate() const noexcept -> std::uint32_t
{
    return imp_->asCfilterPrivate()->FPRate();
}

auto Cfilter::Filter() const noexcept -> ReadView
{
    return imp_->asCfilterPrivate()->Filter();
}

auto Cfilter::Hash() const noexcept -> const opentxs::blockchain::block::Hash&
{
    return imp_->asCfilterPrivate()->Hash();
}

auto Cfilter::operator=(const Cfilter& rhs) noexcept -> Cfilter&
{
    return pmr::copy_assign_child<Message>(*this, rhs);
}

auto Cfilter::operator=(Cfilter&& rhs) noexcept -> Cfilter&
{
    return pmr::move_assign_child<Message>(*this, std::move(rhs));
}

auto Cfilter::Type() const noexcept -> opentxs::blockchain::cfilter::Type
{
    return imp_->asCfilterPrivate()->Type();
}

Cfilter::~Cfilter() = default;
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
