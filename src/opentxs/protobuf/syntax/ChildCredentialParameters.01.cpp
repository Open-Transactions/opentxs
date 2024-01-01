// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/ChildCredentialParameters.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/ChildCredentialParameters.pb.h>
#include <string>

#include "opentxs/protobuf/syntax/Identifier.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerifyCredentials.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const ChildCredentialParameters& input, const Log& log) -> bool
{
    CHECK_SUBOBJECT(masterid, ChildCredentialParametersAllowedIdentifier());

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
