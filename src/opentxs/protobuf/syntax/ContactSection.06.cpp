// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/ContactSection.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/ContactSection.pb.h>
#include <cstdint>
#include <string>

#include "opentxs/protobuf/contact/Types.internal.hpp"
#include "opentxs/protobuf/syntax/ContactItem.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerifyContacts.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_6(
    const ContactSection& input,
    const Log& log,
    const ClaimType indexed,
    const uint32_t parentVersion) -> bool
{
    CHECK_EXISTS(name);

    if (!ValidContactSectionName(parentVersion, input.name())) {
        FAIL_2("invalid name", input.name());
    }

    CHECK_SUBOBJECTS_VA(
        item,
        ContactSectionAllowedItem(),
        indexed,
        contact::ContactSectionVersion{input.version(), input.name()});

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
