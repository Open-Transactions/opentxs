// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/core/contract/peer/reply/Outbailment.hpp"  // IWYU pragma: associated

#include <string_view>
#include <utility>

#include "core/contract/peer/reply/base/ReplyPrivate.hpp"
#include "core/contract/peer/reply/outbailment/OutbailmentPrivate.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/util/Allocator.hpp"

namespace opentxs::contract::peer::reply
{
Outbailment::Outbailment(ReplyPrivate* imp) noexcept
    : Reply(imp)
{
}

Outbailment::Outbailment(allocator_type alloc) noexcept
    : Outbailment(OutbailmentPrivate::Blank(alloc))
{
}

Outbailment::Outbailment(const Outbailment& rhs, allocator_type alloc) noexcept
    : Reply(rhs, alloc)
{
}

Outbailment::Outbailment(Outbailment&& rhs) noexcept
    : Reply(std::move(rhs))
{
}

Outbailment::Outbailment(Outbailment&& rhs, allocator_type alloc) noexcept
    : Reply(std::move(rhs), alloc)
{
}

auto Outbailment::Blank() noexcept -> Outbailment&
{
    static auto blank = Outbailment{allocator_type{alloc::Default()}};

    return blank;
}

auto Outbailment::Description() const noexcept -> std::string_view
{
    return imp_->asOutbailmentPrivate()->Description();
}

auto Outbailment::IsValid() const noexcept -> bool
{
    using enum RequestType;

    return imp_->IsValid() && (Type() == OutBailment);
}

auto Outbailment::operator=(const Outbailment& rhs) noexcept -> Outbailment&
{
    return pmr::copy_assign_child<Reply>(*this, rhs);
}

auto Outbailment::operator=(Outbailment&& rhs) noexcept -> Outbailment&
{
    return pmr::move_assign_child<Reply>(*this, std::move(rhs));
}

Outbailment::~Outbailment() = default;
}  // namespace opentxs::contract::peer::reply
