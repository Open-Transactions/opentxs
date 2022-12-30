// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Getcfheaders.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/PMR.hpp"
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/getcfheaders/MessagePrivate.hpp"

namespace opentxs::network::blockchain::bitcoin::message::internal
{
Getcfheaders::Getcfheaders(allocator_type alloc) noexcept
    : Getcfheaders(MessagePrivate::Blank(alloc))
{
}

Getcfheaders::Getcfheaders(MessagePrivate* imp) noexcept
    : Message(std::move(imp))
{
}

Getcfheaders::Getcfheaders(
    const Getcfheaders& rhs,
    allocator_type alloc) noexcept
    : Message(rhs, alloc)
{
}

Getcfheaders::Getcfheaders(Getcfheaders&& rhs) noexcept
    : Message(std::move(rhs))
{
}

Getcfheaders::Getcfheaders(Getcfheaders&& rhs, allocator_type alloc) noexcept
    : Message(std::move(rhs), alloc)
{
}

auto Getcfheaders::Blank() noexcept -> Getcfheaders&
{
    static auto blank = Getcfheaders{};

    return blank;
}

auto Getcfheaders::operator=(const Getcfheaders& rhs) noexcept -> Getcfheaders&
{
    return copy_assign_child<Message>(*this, rhs);
}

auto Getcfheaders::operator=(Getcfheaders&& rhs) noexcept -> Getcfheaders&
{
    return move_assign_child<Message>(*this, std::move(rhs));
}

auto Getcfheaders::Start() const noexcept -> opentxs::blockchain::block::Height
{
    return imp_->asGetcfheadersPrivate()->Start();
}

auto Getcfheaders::Stop() const noexcept
    -> const opentxs::blockchain::block::Hash&
{
    return imp_->asGetcfheadersPrivate()->Stop();
}

auto Getcfheaders::Type() const noexcept -> opentxs::blockchain::cfilter::Type
{
    return imp_->asGetcfheadersPrivate()->Type();
}

Getcfheaders::~Getcfheaders() = default;
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
