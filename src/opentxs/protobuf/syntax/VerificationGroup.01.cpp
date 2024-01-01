// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/VerificationGroup.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/VerificationGroup.pb.h>
#include <utility>

#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerificationIdentity.hpp"
#include "opentxs/protobuf/syntax/VerifyContacts.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(
    const VerificationGroup& input,
    const Log& log,
    const VerificationType indexed) -> bool
{
    VerificationNymMap nymMap;

    CHECK_SUBOBJECTS_VA(
        identity, VerificationGroupAllowedIdentity(), nymMap, indexed);

    for (auto& nym : nymMap) {
        if (nym.second > 1) { FAIL_2("duplicate identity", nym.first); }
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
