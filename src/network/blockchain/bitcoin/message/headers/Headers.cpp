// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Headers.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/PMR.hpp"
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/headers/MessagePrivate.hpp"
#include "opentxs/blockchain/block/Header.hpp"  // IWYU pragma: keep

namespace opentxs::network::blockchain::bitcoin::message::internal
{
Headers::Headers(allocator_type alloc) noexcept
    : Headers(MessagePrivate::Blank(alloc))
{
}

Headers::Headers(MessagePrivate* imp) noexcept
    : Message(std::move(imp))
{
}

Headers::Headers(const Headers& rhs, allocator_type alloc) noexcept
    : Message(rhs, alloc)
{
}

Headers::Headers(Headers&& rhs) noexcept
    : Message(std::move(rhs))
{
}

Headers::Headers(Headers&& rhs, allocator_type alloc) noexcept
    : Message(std::move(rhs), alloc)
{
}

auto Headers::Blank() noexcept -> Headers&
{
    static auto blank = Headers{};

    return blank;
}

auto Headers::get() const noexcept -> std::span<const value_type>
{
    return imp_->asHeadersPrivate()->get();
}

auto Headers::get() noexcept -> std::span<value_type>
{
    return imp_->asHeadersPrivate()->get();
}

auto Headers::operator=(const Headers& rhs) noexcept -> Headers&
{
    return copy_assign_child<Message>(*this, rhs);
}

auto Headers::operator=(Headers&& rhs) noexcept -> Headers&
{
    return move_assign_child<Message>(*this, std::move(rhs));
}

Headers::~Headers() = default;
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
