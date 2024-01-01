// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/ContactEvent.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/ContactEvent.pb.h>
#include <opentxs/protobuf/RPCEnums.pb.h>
#include <string>

#include "opentxs/protobuf/syntax/AccountEvent.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerifyRPC.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const ContactEvent& input, const Log& log) -> bool
{
    CHECK_IDENTIFIER(id);

    switch (input.type()) {
        case CONTACTEVENT_INCOMINGMESSAGE:
        case CONTACTEVENT_OUTGOINGMESSAGE: {
            CHECK_EXISTS_STRING(message);
            CHECK_EXCLUDED(account);
            CHECK_EXCLUDED(accountevent);
        } break;
        case CONTACTEVENT_INCOMONGPAYMENT:
        case CONTACTEVENT_OUTGOINGPAYMENT: {
            CHECK_EXCLUDED(timestamp);
            CHECK_EXCLUDED(message);
            CHECK_IDENTIFIER(account);
            CHECK_SUBOBJECT(accountevent, ContactEventAllowedAccountEvent());
        } break;
        case CONTACTEVENT_ERROR:
        default: {
            FAIL_2("Invalid type", input.type());
        }
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
