// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/peer/request/bailment/BailmentPrivate.hpp"  // IWYU pragma: associated

#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"

namespace opentxs::contract::peer::request
{
BailmentPrivate::BailmentPrivate(allocator_type alloc) noexcept
    : RequestPrivate(alloc)
{
}

BailmentPrivate::BailmentPrivate(
    const BailmentPrivate& rhs,
    allocator_type alloc) noexcept
    : RequestPrivate(rhs, alloc)
{
}

auto BailmentPrivate::Notary() const noexcept -> const identifier::Notary&
{
    static const auto blank = identifier::Notary{};

    return blank;
}

auto BailmentPrivate::Type() const noexcept -> RequestType
{
    return RequestType::Bailment;
}

auto BailmentPrivate::Unit() const noexcept -> const identifier::UnitDefinition&
{
    static const auto blank = identifier::UnitDefinition{};

    return blank;
}

BailmentPrivate::~BailmentPrivate() = default;
}  // namespace opentxs::contract::peer::request
