// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/VerificationIdentity.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/Identifier.pb.h>
#include <opentxs/protobuf/VerificationIdentity.pb.h>

#include "opentxs/protobuf/syntax/Identifier.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerificationItem.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerifyContacts.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(
    const VerificationIdentity& input,
    const Log& log,
    VerificationNymMap& map,
    const VerificationType indexed) -> bool
{
    CHECK_SUBOBJECT(nym, VerificationIdentityAllowedIdentifier());
    CHECK_SUBOBJECTS_VA(
        verification, VerificationIdentityAllowedVerificationItem(), indexed);

    map[input.nym().hash()] += 1;

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
