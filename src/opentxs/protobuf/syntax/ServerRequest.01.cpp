// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/ServerRequest.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/Enums.pb.h>
#include <opentxs/protobuf/OTXEnums.pb.h>
#include <opentxs/protobuf/ServerRequest.pb.h>
#include <string>

#include "opentxs/protobuf/syntax/Identifier.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/Nym.hpp"              // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Signature.hpp"        // IWYU pragma: keep
#include "opentxs/protobuf/syntax/StorageItemHash.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerifyOTX.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const ServerRequest& input, const Log& log) -> bool
{
    CHECK_SUBOBJECT(id, ServerRequestAllowedIdentifier());
    CHECK_SUBOBJECT(nym, ServerRequestAllowedIdentifier());
    CHECK_SUBOBJECT(server, ServerRequestAllowedIdentifier());
    CHECK_SUBOBJECT_VA(
        signature, ServerRequestAllowedSignature(), SIGROLE_SERVERREQUEST);

    switch (input.type()) {
        case SERVERREQUEST_ACTIVATE: {
        } break;
        case SERVERREQUEST_ERROR:
        default: {
            FAIL_1("Invalid type");
        }
    }

    OPTIONAL_SUBOBJECT(credentials, ServerRequestAllowedNym());

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
