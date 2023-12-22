// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Getcfcheckpt.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/PMR.hpp"
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/getcfcheckpt/MessagePrivate.hpp"

namespace opentxs::network::blockchain::bitcoin::message::internal
{
Getcfcheckpt::Getcfcheckpt(allocator_type alloc) noexcept
    : Getcfcheckpt(MessagePrivate::Blank(alloc))
{
}

Getcfcheckpt::Getcfcheckpt(MessagePrivate* imp) noexcept
    : Message(std::move(imp))
{
}

Getcfcheckpt::Getcfcheckpt(
    const Getcfcheckpt& rhs,
    allocator_type alloc) noexcept
    : Message(rhs, alloc)
{
}

Getcfcheckpt::Getcfcheckpt(Getcfcheckpt&& rhs) noexcept
    : Message(std::move(rhs))
{
}

Getcfcheckpt::Getcfcheckpt(Getcfcheckpt&& rhs, allocator_type alloc) noexcept
    : Message(std::move(rhs), alloc)
{
}

auto Getcfcheckpt::Blank() noexcept -> Getcfcheckpt&
{
    static auto blank = Getcfcheckpt{};

    return blank;
}

auto Getcfcheckpt::operator=(const Getcfcheckpt& rhs) noexcept -> Getcfcheckpt&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::copy_assign_child<Message>(*this, rhs);
}

auto Getcfcheckpt::operator=(Getcfcheckpt&& rhs) noexcept -> Getcfcheckpt&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::move_assign_child<Message>(*this, std::move(rhs));
}

auto Getcfcheckpt::Stop() const noexcept
    -> const opentxs::blockchain::block::Hash&
{
    return imp_->asGetcfcheckptPrivate()->Stop();
}

auto Getcfcheckpt::Type() const noexcept -> opentxs::blockchain::cfilter::Type
{
    return imp_->asGetcfcheckptPrivate()->Type();
}

Getcfcheckpt::~Getcfcheckpt() = default;
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
