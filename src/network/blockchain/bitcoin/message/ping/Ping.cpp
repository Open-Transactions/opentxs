// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Ping.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/PMR.hpp"
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/ping/MessagePrivate.hpp"

namespace opentxs::network::blockchain::bitcoin::message::internal
{
Ping::Ping(allocator_type alloc) noexcept
    : Ping(MessagePrivate::Blank(alloc))
{
}

Ping::Ping(MessagePrivate* imp) noexcept
    : Message(std::move(imp))
{
}

Ping::Ping(const Ping& rhs, allocator_type alloc) noexcept
    : Message(rhs, alloc)
{
}

Ping::Ping(Ping&& rhs) noexcept
    : Message(std::move(rhs))
{
}

Ping::Ping(Ping&& rhs, allocator_type alloc) noexcept
    : Message(std::move(rhs), alloc)
{
}

auto Ping::Blank() noexcept -> Ping&
{
    static auto blank = Ping{};

    return blank;
}

auto Ping::Nonce() const noexcept -> message::Nonce
{
    return imp_->asPingPrivate()->Nonce();
}

auto Ping::operator=(const Ping& rhs) noexcept -> Ping&
{
    return pmr::copy_assign_child<Message>(*this, rhs);
}

auto Ping::operator=(Ping&& rhs) noexcept -> Ping&
{
    return pmr::move_assign_child<Message>(*this, std::move(rhs));
}

Ping::~Ping() = default;
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
