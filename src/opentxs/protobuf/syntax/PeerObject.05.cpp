// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/PeerObject.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/Nym.pb.h>  // IWYU pragma: keep
#include <opentxs/protobuf/PeerEnums.pb.h>
#include <opentxs/protobuf/PeerObject.pb.h>
#include <string>

#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/Nym.hpp"          // IWYU pragma: keep
#include "opentxs/protobuf/syntax/PeerReply.hpp"    // IWYU pragma: keep
#include "opentxs/protobuf/syntax/PeerRequest.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerifyPeer.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_5(const PeerObject& input, const Log& log) -> bool
{
    if (!input.has_type()) { FAIL_1("missing type"); }

    switch (input.type()) {
        case PEEROBJECT_MESSAGE:
        case PEEROBJECT_REQUEST:
        case PEEROBJECT_RESPONSE: {

            return version_1(input, log);
        }
        case PEEROBJECT_PAYMENT: {
            OPTIONAL_SUBOBJECT(nym, PeerObjectAllowedNym());
            CHECK_EXISTS(otpayment);
            CHECK_EXCLUDED(otrequest);
            CHECK_EXCLUDED(otreply);
            CHECK_EXCLUDED(otmessage);
            CHECK_EXCLUDED(purse);
        } break;
        case PEEROBJECT_CASH:
        case PEEROBJECT_ERROR:
        default: {
            FAIL_1("invalid type");
        }
    }

    CHECK_EXCLUDED(purse);

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
