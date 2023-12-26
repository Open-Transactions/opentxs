// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Reject.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/PMR.hpp"
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/reject/MessagePrivate.hpp"

namespace opentxs::network::blockchain::bitcoin::message::internal
{
Reject::Reject(allocator_type alloc) noexcept
    : Reject(MessagePrivate::Blank(alloc))
{
}

Reject::Reject(MessagePrivate* imp) noexcept
    : Message(std::move(imp))
{
}

Reject::Reject(const Reject& rhs, allocator_type alloc) noexcept
    : Message(rhs, alloc)
{
}

Reject::Reject(Reject&& rhs) noexcept
    : Message(std::move(rhs))
{
}

Reject::Reject(Reject&& rhs, allocator_type alloc) noexcept
    : Message(std::move(rhs), alloc)
{
}

auto Reject::Blank() noexcept -> Reject&
{
    static auto blank = Reject{};

    return blank;
}

auto Reject::operator=(const Reject& rhs) noexcept -> Reject&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::copy_assign_child<Message>(*this, rhs);
}

auto Reject::operator=(Reject&& rhs) noexcept -> Reject&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::move_assign_child<Message>(*this, std::move(rhs));
}

auto Reject::Reason() const noexcept -> ReadView
{
    return imp_->asRejectPrivate()->Reason();
}

auto Reject::RejectedMessage() const noexcept -> ReadView
{
    return imp_->asRejectPrivate()->RejectedMessage();
}

Reject::~Reject() = default;
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
