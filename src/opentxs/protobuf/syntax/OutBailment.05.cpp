// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/OutBailment.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/OutBailment.pb.h>
#include <string>

#include "opentxs/protobuf/syntax/Identifier.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerifyPeer.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_5(const OutBailment& input, const Log& log) -> bool
{
    CHECK_SUBOBJECT(unitid, OutbailmentAllowedIdentifier());
    CHECK_SUBOBJECT(serverid, OutbailmentAllowedIdentifier());
    CHECK_EXISTS(amount);
    CHECK_EXISTS(instructions);

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
