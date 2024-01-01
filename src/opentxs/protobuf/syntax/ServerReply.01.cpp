// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/ServerReply.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/Enums.pb.h>
#include <opentxs/protobuf/OTXEnums.pb.h>
#include <opentxs/protobuf/ServerReply.pb.h>
#include <string>

#include "opentxs/protobuf/syntax/Identifier.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/OTXPush.hpp"    // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Signature.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerifyOTX.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const ServerReply& input, const Log& log) -> bool
{
    CHECK_SUBOBJECT(id, ServerReplyAllowedIdentifier());
    CHECK_SUBOBJECT(nym, ServerReplyAllowedIdentifier());
    CHECK_SUBOBJECT(server, ServerReplyAllowedIdentifier());
    CHECK_SUBOBJECT_VA(
        signature, ServerReplyAllowedSignature(), SIGROLE_SERVERREPLY);

    switch (input.type()) {
        case SERVERREPLY_ACTIVATE: {
            CHECK_EXCLUDED(push);
        } break;
        case SERVERREPLY_PUSH: {
            CHECK_SUBOBJECT(push, ServerReplyAllowedOTXPush());
        } break;
        case SERVERREPLY_ERROR:
        default: {
            FAIL_1("Invalid type");
        }
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
