// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/peer/request/bailmentnotice/BailmentNoticePrivate.hpp"  // IWYU pragma: associated

#include "opentxs/core/Amount.hpp"
#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep
#include "opentxs/identifier/Notary.hpp"
#include "opentxs/identifier/UnitDefinition.hpp"

namespace opentxs::contract::peer::request
{
BailmentNoticePrivate::BailmentNoticePrivate(allocator_type alloc) noexcept
    : RequestPrivate(alloc)
{
}

BailmentNoticePrivate::BailmentNoticePrivate(
    const BailmentNoticePrivate& rhs,
    allocator_type alloc) noexcept
    : RequestPrivate(rhs, alloc)
{
}

auto BailmentNoticePrivate::Amount() const noexcept -> opentxs::Amount
{
    return {};
}

auto BailmentNoticePrivate::Description() const noexcept -> std::string_view
{
    return {};
}

auto BailmentNoticePrivate::InReferenceToRequest() const noexcept
    -> const identifier_type&
{
    return ID();
}

auto BailmentNoticePrivate::Notary() const noexcept -> const identifier::Notary&
{
    static const auto blank = identifier::Notary{};

    return blank;
}

auto BailmentNoticePrivate::Type() const noexcept -> RequestType
{
    return RequestType::PendingBailment;
}

auto BailmentNoticePrivate::Unit() const noexcept
    -> const identifier::UnitDefinition&
{
    static const auto blank = identifier::UnitDefinition{};

    return blank;
}

BailmentNoticePrivate::~BailmentNoticePrivate() = default;
}  // namespace opentxs::contract::peer::request
