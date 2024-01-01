// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/VerificationSet.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/VerificationGroup.pb.h>  // IWYU pragma: keep
#include <opentxs/protobuf/VerificationSet.pb.h>

#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerificationGroup.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerifyContacts.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(
    const VerificationSet& input,
    const Log& log,
    const VerificationType indexed) -> bool
{
    OPTIONAL_SUBOBJECT_VA(internal, VerificationSetAllowedGroup(), indexed);
    OPTIONAL_SUBOBJECT_VA(external, VerificationSetAllowedGroup(), indexed);
    OPTIONAL_IDENTIFIERS(repudiated);

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
