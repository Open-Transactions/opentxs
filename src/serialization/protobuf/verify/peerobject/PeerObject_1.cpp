// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/PeerObject.hpp"  // IWYU pragma: associated

#include <Identifier.pb.h>
#include <Nym.pb.h>  // IWYU pragma: keep
#include <PeerEnums.pb.h>
#include <PeerObject.pb.h>
#include <PeerReply.pb.h>
#include <PeerRequest.pb.h>
#include <stdexcept>
#include <utility>

#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/Nym.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/PeerReply.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/PeerRequest.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyPeer.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_1(const PeerObject& input, const bool silent) -> bool
{
    if (!input.has_type()) { FAIL_1("missing type"); }

    if (input.has_otpayment()) { FAIL_1("unexpected otpayment found"); }

    switch (input.type()) {
        case PEEROBJECT_MESSAGE: {
            if (false == input.has_otmessage()) { FAIL_1("missing otmessage"); }

            if (input.has_otrequest()) { FAIL_1("otrequest not empty"); }

            if (input.has_otreply()) { FAIL_1("otreply not empty"); }
        } break;
        case PEEROBJECT_REQUEST: {
            if (!input.has_otrequest()) { FAIL_1("missing otrequest"); }

            try {
                const bool validrequest = Check(
                    input.otrequest(),
                    PeerObjectAllowedPeerRequest().at(input.version()).first,
                    PeerObjectAllowedPeerRequest().at(input.version()).second,
                    silent);

                if (!validrequest) { FAIL_1("invalid otrequest"); }
            } catch (const std::out_of_range&) {
                FAIL_2(
                    "allowed peer request version not defined for version",
                    input.version());
            }

            if (!input.has_nym()) { FAIL_1(" missing nym"); }

            try {
                const bool validnym = Check(
                    input.nym(),
                    PeerObjectAllowedNym().at(input.version()).first,
                    PeerObjectAllowedNym().at(input.version()).second,
                    silent);

                if (!validnym) { FAIL_1("invalid nym"); }
            } catch (const std::out_of_range&) {
                FAIL_2(
                    "allowed credential index version not defined for version",
                    input.version());
            }

            if (input.has_otmessage()) { FAIL_1("otmessage not empty"); }

            if (input.has_otreply()) { FAIL_1("otreply not empty"); }
        } break;
        case PEEROBJECT_RESPONSE: {
            if (!input.has_otrequest()) { FAIL_1("missing otrequest"); }

            try {
                const bool validrequest = Check(
                    input.otrequest(),
                    PeerObjectAllowedPeerRequest().at(input.version()).first,
                    PeerObjectAllowedPeerRequest().at(input.version()).second,
                    silent);

                if (!validrequest) { FAIL_1("invalid otrequest"); }
            } catch (const std::out_of_range&) {
                FAIL_2(
                    "allowed peer request version not defined for version",
                    input.version());
            }

            if (!input.has_otreply()) { FAIL_1("missing otreply"); }

            try {
                const bool validreply = Check(
                    input.otreply(),
                    PeerObjectAllowedPeerReply().at(input.version()).first,
                    PeerObjectAllowedPeerReply().at(input.version()).second,
                    silent);

                if (!validreply) { FAIL_1("invalid otreply"); }
            } catch (const std::out_of_range&) {
                FAIL_2(
                    "allowed peer reply version not defined for version",
                    input.version());
            }

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

    CHECK_EXCLUDED(otpayment);
    CHECK_EXCLUDED(purse);

    return true;
}
auto CheckProto_2(const PeerObject& input, const bool silent) -> bool
{
    return CheckProto_1(input, silent);
}
auto CheckProto_3(const PeerObject& input, const bool silent) -> bool
{
    return CheckProto_1(input, silent);
}
auto CheckProto_4(const PeerObject& input, const bool silent) -> bool
{
    return CheckProto_1(input, silent);
}

}  // namespace opentxs::proto
