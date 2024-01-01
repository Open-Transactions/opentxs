// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/CreateNym.hpp"  // IWYU pragma: associated

#include <boost/unordered/unordered_flat_set.hpp>
#include <opentxs/protobuf/ContactSectionName.pb.h>
#include <opentxs/protobuf/CreateNym.pb.h>
#include <string>
#include <utility>

#include "opentxs/protobuf/contact/Types.internal.hpp"
#include "opentxs/protobuf/syntax/AddClaim.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerifyRPC.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const CreateNym& input, const Log& log) -> bool
{
    const auto allowedtype = 1 == contact::AllowedItemTypes()
                                      .at({5, CONTACTSECTION_SCOPE})
                                      .count(input.type());

    if (false == allowedtype) { FAIL_1("Invalid type"); }

    CHECK_NAME(name);
    OPTIONAL_IDENTIFIER(seedid);
    OPTIONAL_SUBOBJECTS(claims, CreateNymAllowedAddClaim());

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
