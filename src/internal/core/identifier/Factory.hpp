// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Types.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class IdentifierPrivate;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
auto Identifier(
    const identifier::Type type,
    identifier::AccountSubtype accountSubtype,
    identifier::Generic::allocator_type alloc) noexcept
    -> identifier::IdentifierPrivate*;
auto Identifier(
    const identifier::Type type,
    const identifier::Algorithm algorithm,
    const ReadView hash,
    identifier::AccountSubtype accountSubtype,
    identifier::Generic::allocator_type alloc) noexcept
    -> identifier::IdentifierPrivate*;
auto IdentifierInvalid(identifier::Generic::allocator_type alloc) noexcept
    -> identifier::IdentifierPrivate*;
}  // namespace opentxs::factory
