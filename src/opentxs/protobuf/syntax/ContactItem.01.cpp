// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::protobuf::ContactItemAttribute

#include "opentxs/protobuf/syntax/ContactItem.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/ContactItem.pb.h>
#include <opentxs/protobuf/ContactItemAttribute.pb.h>
#include <string>

#include "opentxs/protobuf/contact/Types.internal.hpp"
#include "opentxs/protobuf/syntax/Identifier.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerifyContacts.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const ContactItem& input, const Log& log) -> bool
{
    OPTIONAL_SUBOBJECT(id, ContactItemAllowedIdentifier());

    if (input.end() < input.start()) { FAIL_1("invalid time range"); }

    CHECK_EXISTS(type);

    if (false == input.has_value()) {
        CHECK_SUBOBJECT(commitment, ContactItemAllowedIdentifier());
    }

    for (const auto& it : input.attribute()) {
        if (!ValidContactItemAttribute(
                input.version(), static_cast<ContactItemAttribute>(it))) {
            FAIL_1("invalid attribute");
        }
    }

    if (input.has_subtype()) {
        if (3 > input.version()) { FAIL_1("Subtype present but not allowed"); }
    }

    OPTIONAL_SUBOBJECTS(superscedes, ContactItemAllowedIdentifier());

    return true;
}

auto version_1(
    const ContactItem& input,
    const Log& log,
    const ClaimType indexed,
    const contact::ContactSectionVersion parentVersion) -> bool
{
    if (false == version_1(input, log)) { return false; }

    if (ClaimType::Indexed == indexed) {
        CHECK_EXISTS(id);
    } else {
        CHECK_EXCLUDED(id);
    }

    if (false == ValidContactItemType(parentVersion, input.type())) {
        FAIL_1("invalid type");
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
