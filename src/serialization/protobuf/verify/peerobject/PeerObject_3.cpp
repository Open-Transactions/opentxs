// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/PeerObject.hpp"  // IWYU pragma: associated

#include <Identifier.pb.h>
#include <PeerEnums.pb.h>
#include <PeerObject.pb.h>
#include <PeerReply.pb.h>
#include <PeerRequest.pb.h>

#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/Nym.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/PeerReply.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/PeerRequest.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/Purse.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyPeer.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_7(const PeerObject& input, const bool silent) -> bool
{
    if (!input.has_type()) { FAIL_1("missing type"); }

    switch (input.type()) {
        case PEEROBJECT_MESSAGE: {
            CHECK_EXISTS(otmessage);
            CHECK_EXCLUDED(otrequest);
            CHECK_EXCLUDED(otreply);
            CHECK_EXCLUDED(otpayment);
            CHECK_EXCLUDED(purse);
            OPTIONAL_SUBOBJECT(nym, PeerObjectAllowedNym());
        } break;
        case PEEROBJECT_REQUEST: {
            CHECK_EXCLUDED(otmessage);
            CHECK_SUBOBJECT(otrequest, PeerObjectAllowedPeerRequest());
            CHECK_EXCLUDED(otreply);
            CHECK_EXCLUDED(otpayment);
            CHECK_EXCLUDED(purse);
            CHECK_SUBOBJECT(nym, PeerObjectAllowedNym());
        } break;
        case PEEROBJECT_RESPONSE: {
            CHECK_EXCLUDED(otmessage);
            CHECK_SUBOBJECT(otrequest, PeerObjectAllowedPeerRequest());
            CHECK_SUBOBJECT(otreply, PeerObjectAllowedPeerReply());
            CHECK_EXCLUDED(otpayment);
            CHECK_EXCLUDED(purse);
            OPTIONAL_SUBOBJECT(nym, PeerObjectAllowedNym());

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
        } break;
        case PEEROBJECT_PAYMENT: {
            CHECK_EXCLUDED(otmessage);
            CHECK_EXCLUDED(otrequest);
            CHECK_EXCLUDED(otreply);
            CHECK_EXISTS(otpayment);
            CHECK_EXCLUDED(purse);
            OPTIONAL_SUBOBJECT(nym, PeerObjectAllowedNym());
        } break;
        case PEEROBJECT_CASH: {
            CHECK_EXCLUDED(otmessage);
            CHECK_EXCLUDED(otrequest);
            CHECK_EXCLUDED(otreply);
            CHECK_EXCLUDED(otpayment);
            CHECK_SUBOBJECT(purse, PeerObjectAllowedPurse());
            OPTIONAL_SUBOBJECT(nym, PeerObjectAllowedNym());
        } break;
        case PEEROBJECT_ERROR:
        default: {
            FAIL_1("invalid type");
        }
    }

    return true;
}

auto CheckProto_8(const PeerObject& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(const PeerObject& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(const PeerObject& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(const PeerObject& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(const PeerObject& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(const PeerObject& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(const PeerObject& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(const PeerObject& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(const PeerObject& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(const PeerObject& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(const PeerObject& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(const PeerObject& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(const PeerObject& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
