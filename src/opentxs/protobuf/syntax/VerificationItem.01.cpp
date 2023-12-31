// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/VerificationItem.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/Enums.pb.h>
#include <opentxs/protobuf/Signature.pb.h>  // IWYU pragma: keep
#include <opentxs/protobuf/VerificationItem.pb.h>
#include <string>

#include "opentxs/protobuf/syntax/Identifier.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/Signature.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerifyContacts.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(
    const VerificationItem& input,
    const Log& log,
    const VerificationType indexed) -> bool
{
    if (VerificationType::Indexed == indexed) {
        CHECK_SUBOBJECT(id, VerificationItemAllowedIdentifier());
    } else {
        CHECK_EXCLUDED(id);
    }

    CHECK_SUBOBJECT(claim, VerificationItemAllowedIdentifier());
    CHECK_EXISTS(kind);

    if (input.end() < input.start()) {
        FAIL_1("verification expires before start time");
    }

    CHECK_SUBOBJECT_VA(
        sig, VerificationItemAllowedSignature(), protobuf::SIGROLE_CLAIM);
    OPTIONAL_SUBOBJECTS(superscedes, VerificationItemAllowedIdentifier());

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
