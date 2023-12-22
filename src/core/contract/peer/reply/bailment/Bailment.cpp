// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/core/contract/peer/reply/Bailment.hpp"  // IWYU pragma: associated

#include <string_view>
#include <utility>

#include "core/contract/peer/reply/bailment/BailmentPrivate.hpp"
#include "core/contract/peer/reply/base/ReplyPrivate.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/util/Allocator.hpp"

namespace opentxs::contract::peer::reply
{
Bailment::Bailment(ReplyPrivate* imp) noexcept
    : Reply(imp)
{
}

Bailment::Bailment(allocator_type alloc) noexcept
    : Bailment(BailmentPrivate::Blank(alloc))
{
}

Bailment::Bailment(const Bailment& rhs, allocator_type alloc) noexcept
    : Reply(rhs, alloc)
{
}

Bailment::Bailment(Bailment&& rhs) noexcept
    : Reply(std::move(rhs))
{
}

Bailment::Bailment(Bailment&& rhs, allocator_type alloc) noexcept
    : Reply(std::move(rhs), alloc)
{
}

auto Bailment::Blank() noexcept -> Bailment&
{
    static auto blank = Bailment{allocator_type{alloc::Default()}};

    return blank;
}

auto Bailment::Instructions() const noexcept -> std::string_view
{
    return imp_->asBailmentPrivate()->Instructions();
}

auto Bailment::IsValid() const noexcept -> bool
{
    using enum RequestType;

    return imp_->IsValid() && (Type() == Bailment);
}

auto Bailment::operator=(const Bailment& rhs) noexcept -> Bailment&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::copy_assign_child<Reply>(*this, rhs);
}

auto Bailment::operator=(Bailment&& rhs) noexcept -> Bailment&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::move_assign_child<Reply>(*this, std::move(rhs));
}

Bailment::~Bailment() = default;
}  // namespace opentxs::contract::peer::reply
