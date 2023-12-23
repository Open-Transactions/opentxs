// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <variant>

#include "opentxs/Export.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/util/Allocator.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Generic;
class Nym;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::wot
{
using ClaimID = identifier::Generic;
using VerificationID = identifier::Generic;
using Claimant = std::variant<identifier::Nym, ContactID>;

OPENTXS_EXPORT auto copy(const Claimant& in, alloc::Strategy alloc) noexcept
    -> Claimant;
OPENTXS_EXPORT auto get_identifier(const Claimant& in) noexcept
    -> const identifier::Generic&;
}  // namespace opentxs::identity::wot
