// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/Context.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/ConsensusEnums.pb.h>
#include <opentxs/protobuf/Context.pb.h>
#include <opentxs/protobuf/Enums.pb.h>
#include <string>

#include "opentxs/protobuf/syntax/ClientContext.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Identifier.hpp"     // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/ServerContext.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Signature.hpp"      // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerifyConsensus.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const Context& input, const Log& log) -> bool
{
    CHECK_SUBOBJECT(localnym, ContextAllowedIdentifier());
    CHECK_SUBOBJECT(remotenym, ContextAllowedIdentifier());
    OPTIONAL_SUBOBJECT(localnymboxhash, ContextAllowedIdentifier());
    OPTIONAL_SUBOBJECT(remotenymboxhash, ContextAllowedIdentifier());
    CHECK_EXISTS(type);

    switch (input.type()) {
        case CONSENSUSTYPE_SERVER: {
            CHECK_EXCLUDED(clientcontext);
            CHECK_SUBOBJECT(servercontext, ContextAllowedServer());
        } break;
        case CONSENSUSTYPE_CLIENT: {
            CHECK_EXCLUDED(servercontext);
            CHECK_SUBOBJECT(clientcontext, ContextAllowedServer());
        } break;
        case CONSENSUSTYPE_PEER:
        case CONSENSUSTYPE_ERROR:
        default: {
            FAIL_1("invalid type");
        }
    }

    CHECK_SUBOBJECT_VA(signature, ContextAllowedSignature(), SIGROLE_CONTEXT);

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
