// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Cfheaders.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/PMR.hpp"
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/cfheaders/MessagePrivate.hpp"
#include "opentxs/blockchain/cfilter/Hash.hpp"  // IWYU pragma: keep

namespace opentxs::network::blockchain::bitcoin::message::internal
{
Cfheaders::Cfheaders(allocator_type alloc) noexcept
    : Cfheaders(MessagePrivate::Blank(alloc))
{
}

Cfheaders::Cfheaders(MessagePrivate* imp) noexcept
    : Message(std::move(imp))
{
}

Cfheaders::Cfheaders(const Cfheaders& rhs, allocator_type alloc) noexcept
    : Message(rhs, alloc)
{
}

Cfheaders::Cfheaders(Cfheaders&& rhs) noexcept
    : Message(std::move(rhs))
{
}

Cfheaders::Cfheaders(Cfheaders&& rhs, allocator_type alloc) noexcept
    : Message(std::move(rhs), alloc)
{
}

auto Cfheaders::Blank() noexcept -> Cfheaders&
{
    static auto blank = Cfheaders{};

    return blank;
}

auto Cfheaders::get() const noexcept -> std::span<const value_type>
{
    return imp_->asCfheadersPrivate()->get();
}

auto Cfheaders::get() noexcept -> std::span<value_type>
{
    return imp_->asCfheadersPrivate()->get();
}

auto Cfheaders::operator=(const Cfheaders& rhs) noexcept -> Cfheaders&
{
    return pmr::copy_assign_child<Message>(*this, rhs);
}

auto Cfheaders::operator=(Cfheaders&& rhs) noexcept -> Cfheaders&
{
    return pmr::move_assign_child<Message>(*this, std::move(rhs));
}

auto Cfheaders::Previous() const noexcept
    -> const opentxs::blockchain::cfilter::Header&
{
    return imp_->asCfheadersPrivate()->Previous();
}

auto Cfheaders::Stop() const noexcept -> const opentxs::blockchain::block::Hash&
{
    return imp_->asCfheadersPrivate()->Stop();
}

auto Cfheaders::Type() const noexcept -> opentxs::blockchain::cfilter::Type
{
    return imp_->asCfheadersPrivate()->Type();
}

Cfheaders::~Cfheaders() = default;
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
