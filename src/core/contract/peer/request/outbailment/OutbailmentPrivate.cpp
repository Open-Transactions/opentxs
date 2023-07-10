// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/peer/request/outbailment/OutbailmentPrivate.hpp"  // IWYU pragma: associated

#include "opentxs/core/Amount.hpp"
#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"

namespace opentxs::contract::peer::request
{
OutbailmentPrivate::OutbailmentPrivate(allocator_type alloc) noexcept
    : RequestPrivate(alloc)
{
}

OutbailmentPrivate::OutbailmentPrivate(
    const OutbailmentPrivate& rhs,
    allocator_type alloc) noexcept
    : RequestPrivate(rhs, alloc)
{
}

auto OutbailmentPrivate::Amount() const noexcept -> opentxs::Amount
{
    return {};
}

auto OutbailmentPrivate::Instructions() const noexcept -> std::string_view
{
    return {};
}

auto OutbailmentPrivate::Notary() const noexcept -> const identifier::Notary&
{
    static const auto blank = identifier::Notary{};

    return blank;
}

auto OutbailmentPrivate::Type() const noexcept -> RequestType
{
    return RequestType::OutBailment;
}

auto OutbailmentPrivate::Unit() const noexcept
    -> const identifier::UnitDefinition&
{
    static const auto blank = identifier::UnitDefinition{};

    return blank;
}

OutbailmentPrivate::~OutbailmentPrivate() = default;
}  // namespace opentxs::contract::peer::request
