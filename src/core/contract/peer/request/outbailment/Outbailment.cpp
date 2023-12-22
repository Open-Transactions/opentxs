// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::identifier::Notary
// IWYU pragma: no_forward_declare opentxs::identifier::UnitDefinition

#include "opentxs/core/contract/peer/reply/Outbailment.hpp"  // IWYU pragma: associated

#include <string_view>
#include <utility>

#include "core/contract/peer/request/base/RequestPrivate.hpp"
#include "core/contract/peer/request/outbailment/OutbailmentPrivate.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/contract/peer/Request.hpp"
#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/contract/peer/request/Outbailment.hpp"
#include "opentxs/util/Allocator.hpp"

namespace opentxs::contract::peer::request
{
Outbailment::Outbailment(RequestPrivate* imp) noexcept
    : Request(imp)
{
}

Outbailment::Outbailment(allocator_type alloc) noexcept
    : Outbailment(OutbailmentPrivate::Blank(alloc))
{
}

Outbailment::Outbailment(const Outbailment& rhs, allocator_type alloc) noexcept
    : Request(rhs, alloc)
{
}

Outbailment::Outbailment(Outbailment&& rhs) noexcept
    : Request(std::move(rhs))
{
}

Outbailment::Outbailment(Outbailment&& rhs, allocator_type alloc) noexcept
    : Request(std::move(rhs), alloc)
{
}

auto Outbailment::Blank() noexcept -> Outbailment&
{
    static auto blank = Outbailment{allocator_type{alloc::Default()}};

    return blank;
}

auto Outbailment::Amount() const noexcept -> opentxs::Amount
{
    return imp_->asOutbailmentPrivate()->Amount();
}

auto Outbailment::Instructions() const noexcept -> std::string_view
{
    return imp_->asOutbailmentPrivate()->Instructions();
}

auto Outbailment::IsValid() const noexcept -> bool
{
    using enum RequestType;

    return imp_->IsValid() && (Type() == OutBailment);
}

auto Outbailment::Notary() const noexcept -> const identifier::Notary&
{
    return imp_->asOutbailmentPrivate()->Notary();
}

auto Outbailment::operator=(const Outbailment& rhs) noexcept -> Outbailment&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::copy_assign_child<Request>(*this, rhs);
}

auto Outbailment::operator=(Outbailment&& rhs) noexcept -> Outbailment&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::move_assign_child<Request>(*this, std::move(rhs));
}

auto Outbailment::Unit() const noexcept -> const identifier::UnitDefinition&
{
    return imp_->asOutbailmentPrivate()->Unit();
}

Outbailment::~Outbailment() = default;
}  // namespace opentxs::contract::peer::request
