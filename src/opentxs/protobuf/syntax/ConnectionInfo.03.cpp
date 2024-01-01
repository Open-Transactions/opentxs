// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/ConnectionInfo.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/ConnectionInfo.pb.h>
#include <opentxs/protobuf/PeerEnums.pb.h>
#include <string>

#include "opentxs/protobuf/syntax/Identifier.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerifyPeer.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_3(const ConnectionInfo& input, const Log& log) -> bool
{
    CHECK_EXISTS(type);

    if ((CONNECTIONINFO_BITCOIN > input.type()) ||
        (CONNECTIONINFO_CJDNS < input.type())) {
        FAIL_2("invalid type", input.type());
    }

    OPTIONAL_SUBOBJECT(nym, ConnectionInfoAllowedIdentifier());

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
