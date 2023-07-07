// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::identifier::Notary
// IWYU pragma: no_forward_declare opentxs::identifier::UnitDefinition

#include "opentxs/core/contract/peer/reply/BailmentNotice.hpp"  // IWYU pragma: associated

#include <string_view>
#include <utility>

#include "core/contract/peer/request/bailmentnotice/BailmentNoticePrivate.hpp"
#include "core/contract/peer/request/base/RequestPrivate.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/contract/peer/Request.hpp"
#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/contract/peer/request/BailmentNotice.hpp"
#include "opentxs/util/Allocator.hpp"

namespace opentxs::contract::peer::request
{
BailmentNotice::BailmentNotice(RequestPrivate* imp) noexcept
    : Request(imp)
{
}

BailmentNotice::BailmentNotice(allocator_type alloc) noexcept
    : BailmentNotice(BailmentNoticePrivate::Blank(alloc))
{
}

BailmentNotice::BailmentNotice(
    const BailmentNotice& rhs,
    allocator_type alloc) noexcept
    : Request(rhs, alloc)
{
}

BailmentNotice::BailmentNotice(BailmentNotice&& rhs) noexcept
    : Request(std::move(rhs))
{
}

BailmentNotice::BailmentNotice(
    BailmentNotice&& rhs,
    allocator_type alloc) noexcept
    : Request(std::move(rhs), alloc)
{
}

auto BailmentNotice::Amount() const noexcept -> opentxs::Amount
{
    return imp_->asBailmentNoticePrivate()->Amount();
}

auto BailmentNotice::Blank() noexcept -> BailmentNotice&
{
    static auto blank = BailmentNotice{allocator_type{alloc::Default()}};

    return blank;
}

auto BailmentNotice::Description() const noexcept -> std::string_view
{
    return imp_->asBailmentNoticePrivate()->Description();
}

auto BailmentNotice::InReferenceToRequest() const noexcept
    -> const identifier_type&
{
    return imp_->asBailmentNoticePrivate()->InReferenceToRequest();
}

auto BailmentNotice::IsValid() const noexcept -> bool
{
    using enum RequestType;

    return imp_->IsValid() && (Type() == PendingBailment);
}

auto BailmentNotice::Notary() const noexcept -> const identifier::Notary&
{
    return imp_->asBailmentNoticePrivate()->Notary();
}

// NOLINTBEGIN(modernize-use-equals-default)
auto BailmentNotice::operator=(const BailmentNotice& rhs) noexcept
    -> BailmentNotice&
{
    Request::operator=(rhs);

    return *this;
}

auto BailmentNotice::operator=(BailmentNotice&& rhs) noexcept -> BailmentNotice&
{
    Request::operator=(std::move(rhs));

    return *this;
}
// NOLINTEND(modernize-use-equals-default)

auto BailmentNotice::Unit() const noexcept -> const identifier::UnitDefinition&
{
    return imp_->asBailmentNoticePrivate()->Unit();
}

BailmentNotice::~BailmentNotice() = default;
}  // namespace opentxs::contract::peer::request
