// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/AddClaim.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/AddClaim.pb.h>
#include <string>

#include "opentxs/protobuf/contact/Types.internal.hpp"
#include "opentxs/protobuf/syntax/ContactItem.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerifyContacts.hpp"
#include "opentxs/protobuf/syntax/VerifyRPC.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const AddClaim& input, const Log& log) -> bool
{
    CHECK_EXISTS(sectionversion);
    CHECK_EXISTS(sectiontype);
    const contact::ContactSectionVersion section{
        input.sectionversion(), input.sectiontype()};
    CHECK_SUBOBJECT_VA(
        item, AddClaimAllowedContactItem(), ClaimType::Normal, section);

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
