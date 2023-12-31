// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/MasterCredentialParameters.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/MasterCredentialParameters.pb.h>
#include <opentxs/protobuf/NymIDSource.pb.h>  // IWYU pragma: keep
#include <opentxs/protobuf/SourceProof.pb.h>  // IWYU pragma: keep
#include <string>

#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/NymIDSource.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/SourceProof.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerifyCredentials.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(
    const MasterCredentialParameters& input,
    const Log& log,
    bool& expectSourceSignature) -> bool
{
    if (false == input.has_source()) { FAIL_1("missing nym id source"); }

    CHECK_SUBOBJECT(source, MasterParamsAllowedNymIDSource());
    CHECK_SUBOBJECT_VA(
        sourceproof, MasterParamsAllowedSourceProof(), expectSourceSignature);

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
