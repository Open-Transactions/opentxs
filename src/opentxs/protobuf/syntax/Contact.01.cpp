// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/Contact.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/Contact.pb.h>
#include <opentxs/protobuf/ContactData.pb.h>  // IWYU pragma: keep

#include "opentxs/protobuf/Types.internal.hpp"
#include "opentxs/protobuf/syntax/Constants.hpp"
#include "opentxs/protobuf/syntax/ContactData.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerifyContacts.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const Contact& input, const Log& log) -> bool
{
    if (false == input.has_id()) { FAIL_1("missing id"); }

    if (MIN_PLAUSIBLE_IDENTIFIER > input.id().size()) {
        FAIL_2("invalid id", input.id());
    }

    if (MAX_PLAUSIBLE_IDENTIFIER < input.id().size()) {
        FAIL_2("invalid id", input.id());
    }

    if (input.has_label()) {
        if (MAX_VALID_CONTACT_VALUE < input.label().size()) {
            FAIL_2("invalid label", input.id());
        }
    }

    OPTIONAL_SUBOBJECT_VA(
        contactdata, ContactAllowedContactData(), ClaimType::Normal);

    const bool merged = 0 < input.mergedto().size();
    const bool hasMerges = 0 < input.merged().size();

    if (merged && hasMerges) {
        FAIL_1("merged contact may not contain child merges");
    }

    if (merged) { CHECK_IDENTIFIER(mergedto); }

    if (hasMerges) { CHECK_IDENTIFIERS(merged); }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
