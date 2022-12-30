// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Getcfilters.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/PMR.hpp"
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/getcfilters/MessagePrivate.hpp"

namespace opentxs::network::blockchain::bitcoin::message::internal
{
Getcfilters::Getcfilters(allocator_type alloc) noexcept
    : Getcfilters(MessagePrivate::Blank(alloc))
{
}

Getcfilters::Getcfilters(MessagePrivate* imp) noexcept
    : Message(std::move(imp))
{
}

Getcfilters::Getcfilters(const Getcfilters& rhs, allocator_type alloc) noexcept
    : Message(rhs, alloc)
{
}

Getcfilters::Getcfilters(Getcfilters&& rhs) noexcept
    : Message(std::move(rhs))
{
}

Getcfilters::Getcfilters(Getcfilters&& rhs, allocator_type alloc) noexcept
    : Message(std::move(rhs), alloc)
{
}

auto Getcfilters::Blank() noexcept -> Getcfilters&
{
    static auto blank = Getcfilters{};

    return blank;
}

auto Getcfilters::operator=(const Getcfilters& rhs) noexcept -> Getcfilters&
{
    return copy_assign_child<Message>(*this, rhs);
}

auto Getcfilters::operator=(Getcfilters&& rhs) noexcept -> Getcfilters&
{
    return move_assign_child<Message>(*this, std::move(rhs));
}

auto Getcfilters::Start() const noexcept -> opentxs::blockchain::block::Height
{
    return imp_->asGetcfiltersPrivate()->Start();
}

auto Getcfilters::Stop() const noexcept
    -> const opentxs::blockchain::block::Hash&
{
    return imp_->asGetcfiltersPrivate()->Stop();
}

auto Getcfilters::Type() const noexcept -> opentxs::blockchain::cfilter::Type
{
    return imp_->asGetcfiltersPrivate()->Type();
}

Getcfilters::~Getcfilters() = default;
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
