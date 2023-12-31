// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/PeerObject.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/Identifier.pb.h>
#include <opentxs/protobuf/Nym.pb.h>  // IWYU pragma: keep
#include <opentxs/protobuf/PeerEnums.pb.h>
#include <opentxs/protobuf/PeerObject.pb.h>
#include <opentxs/protobuf/PeerReply.pb.h>
#include <opentxs/protobuf/PeerRequest.pb.h>
#include <string>

#include "opentxs/protobuf/Types.internal.hpp"
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/Nym.hpp"          // IWYU pragma: keep
#include "opentxs/protobuf/syntax/PeerReply.hpp"    // IWYU pragma: keep
#include "opentxs/protobuf/syntax/PeerRequest.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerifyPeer.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const PeerObject& input, const Log& log) -> bool
{
    if (!input.has_type()) { FAIL_1("missing type"); }

    if (input.has_otpayment()) { FAIL_1("unexpected otpayment found"); }

    switch (input.type()) {
        case PEEROBJECT_MESSAGE: {
            OPTIONAL_SUBOBJECT(nym, PeerObjectAllowedNym());
            CHECK_EXISTS(otmessage);
            CHECK_EXCLUDED(otrequest);
            CHECK_EXCLUDED(otreply);
            CHECK_EXCLUDED(otpayment);
            CHECK_EXCLUDED(purse);
        } break;
        case PEEROBJECT_REQUEST: {
            OPTIONAL_SUBOBJECT(nym, PeerObjectAllowedNym());
            CHECK_SUBOBJECT(otrequest, PeerObjectAllowedPeerRequest());
            CHECK_EXCLUDED(otmessage);
            CHECK_EXCLUDED(otreply);
            CHECK_EXCLUDED(otpayment);
            CHECK_EXCLUDED(purse);
        } break;
        case PEEROBJECT_RESPONSE: {
            OPTIONAL_SUBOBJECT(nym, PeerObjectAllowedNym());
            CHECK_SUBOBJECT(otrequest, PeerObjectAllowedPeerRequest());
            CHECK_SUBOBJECT(otreply, PeerObjectAllowedPeerReply());
            CHECK_EXCLUDED(otmessage);
            CHECK_EXCLUDED(otpayment);
            CHECK_EXCLUDED(purse);

            const bool matchingID =
                (input.otrequest().id() == input.otreply().cookie());

            if (!matchingID) {
                FAIL_1("reply cookie does not match request id");
            }

            const bool matchingtype =
                (input.otrequest().type() == input.otreply().type());

            if (!matchingtype) {
                FAIL_1("reply type does not match request type");
            }

            const bool matchingInitiator =
                (input.otrequest().initiator() == input.otreply().initiator());

            if (!matchingInitiator) {
                FAIL_1("reply initiator does not match request initiator");
            }

            const bool matchingRecipient =
                (input.otrequest().recipient() == input.otreply().recipient());

            if (!matchingRecipient) {
                FAIL_1("reply recipient does not match request recipient");
            }

            if (input.has_otmessage()) { FAIL_1("otmessage not empty"); }
        } break;
        case PEEROBJECT_PAYMENT:
        case PEEROBJECT_CASH:
        case PEEROBJECT_ERROR:
        default: {
            FAIL_1("invalid type");
        }
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
