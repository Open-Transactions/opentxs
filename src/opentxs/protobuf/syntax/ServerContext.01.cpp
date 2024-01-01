// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/ServerContext.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/ServerContext.pb.h>
#include <string>

#include "opentxs/protobuf/syntax/Identifier.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerifyConsensus.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const ServerContext& input, const Log& log) -> bool
{
    CHECK_SUBOBJECT(serverid, ServerContextAllowedIdentifier());
    CHECK_EXCLUDED(revision);
    CHECK_EXCLUDED(adminpassword);
    CHECK_EXCLUDED(adminattempted);
    CHECK_EXCLUDED(adminsuccess);
    CHECK_EXCLUDED(state);
    CHECK_EXCLUDED(laststatus);
    CHECK_EXCLUDED(pending);

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
