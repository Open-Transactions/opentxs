// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::identifier::Notary
// IWYU pragma: no_forward_declare opentxs::identifier::UnitDefinition

#include "opentxs/core/contract/peer/reply/Bailment.hpp"  // IWYU pragma: associated

#include <utility>

#include "core/contract/peer/request/bailment/BailmentPrivate.hpp"
#include "core/contract/peer/request/base/RequestPrivate.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/contract/peer/Request.hpp"
#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/contract/peer/request/Bailment.hpp"
#include "opentxs/util/Allocator.hpp"

namespace opentxs::contract::peer::request
{
Bailment::Bailment(RequestPrivate* imp) noexcept
    : Request(imp)
{
}

Bailment::Bailment(allocator_type alloc) noexcept
    : Bailment(BailmentPrivate::Blank(alloc))
{
}

Bailment::Bailment(const Bailment& rhs, allocator_type alloc) noexcept
    : Request(rhs, alloc)
{
}

Bailment::Bailment(Bailment&& rhs) noexcept
    : Request(std::move(rhs))
{
}

Bailment::Bailment(Bailment&& rhs, allocator_type alloc) noexcept
    : Request(std::move(rhs), alloc)
{
}

auto Bailment::Blank() noexcept -> Bailment&
{
    static auto blank = Bailment{allocator_type{alloc::Default()}};

    return blank;
}

auto Bailment::IsValid() const noexcept -> bool
{
    using enum RequestType;

    return imp_->IsValid() && (Type() == Bailment);
}

auto Bailment::Notary() const noexcept -> const identifier::Notary&
{
    return imp_->asBailmentPrivate()->Notary();
}

auto Bailment::operator=(const Bailment& rhs) noexcept -> Bailment&
{
    return pmr::copy_assign_child<Request>(*this, rhs);
}

auto Bailment::operator=(Bailment&& rhs) noexcept -> Bailment&
{
    return pmr::move_assign_child<Request>(*this, std::move(rhs));
}

auto Bailment::Unit() const noexcept -> const identifier::UnitDefinition&
{
    return imp_->asBailmentPrivate()->Unit();
}

Bailment::~Bailment() = default;
}  // namespace opentxs::contract::peer::request
