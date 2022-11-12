// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::identifier::Algorithm
// IWYU pragma: no_forward_declare opentxs::identifier::Type
// IWYU pragma: no_include "opentxs/core/identifier/Algorithm.hpp"
// IWYU pragma: no_include "opentxs/core/identifier/Type.hpp"

#pragma once

#include <memory>

#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Types.hpp"

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
    identifier::Generic::allocator_type alloc) noexcept
    -> identifier::IdentifierPrivate*;
auto Identifier(
    const identifier::Type type,
    const identifier::Algorithm algorithm,
    const ReadView hash,
    identifier::Generic::allocator_type alloc) noexcept
    -> identifier::IdentifierPrivate*;
auto IdentifierInvalid(identifier::Generic::allocator_type alloc) noexcept
    -> identifier::IdentifierPrivate*;
}  // namespace opentxs::factory
