// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/AccountEvent.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/AccountEvent.pb.h>
#include <opentxs/protobuf/RPCEnums.pb.h>

#include "opentxs/protobuf/syntax/Macros.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_2(const AccountEvent& input, const Log& log) -> bool
{
    OPTIONAL_IDENTIFIER(id);

    switch (input.type()) {
        case ACCOUNTEVENT_INCOMINGTRANSFER:
        case ACCOUNTEVENT_INCOMINGINVOICE:
        case ACCOUNTEVENT_INCOMINGVOUCHER:
        case ACCOUNTEVENT_INCOMINGCHEQUE:
        case ACCOUNTEVENT_OUTGOINGCHEQUE:
        case ACCOUNTEVENT_OUTGOINGTRANSFER:
        case ACCOUNTEVENT_OUTGOINGINVOICE:
        case ACCOUNTEVENT_OUTGOINGVOUCHER: {
            CHECK_IDENTIFIER(workflow);
        } break;
        case ACCOUNTEVENT_INCOMINGBLOCKCHAIN:
        case ACCOUNTEVENT_OUTGOINGBLOCKCHAIN: {
            OPTIONAL_IDENTIFIER(workflow);
        } break;
        case ACCOUNTEVENT_ERROR:
        default: {
            FAIL_2("Invalid type", input.type());
        }
    }

    switch (input.type()) {
        case ACCOUNTEVENT_INCOMINGTRANSFER:
        case ACCOUNTEVENT_INCOMINGINVOICE:
        case ACCOUNTEVENT_INCOMINGVOUCHER:
        case ACCOUNTEVENT_INCOMINGCHEQUE: {
            CHECK_IDENTIFIER(contact);
        } break;
        case ACCOUNTEVENT_OUTGOINGCHEQUE:
        case ACCOUNTEVENT_OUTGOINGTRANSFER:
        case ACCOUNTEVENT_OUTGOINGINVOICE:
        case ACCOUNTEVENT_OUTGOINGVOUCHER:
        case ACCOUNTEVENT_INCOMINGBLOCKCHAIN:
        case ACCOUNTEVENT_OUTGOINGBLOCKCHAIN: {
            OPTIONAL_IDENTIFIER(contact);
        } break;
        case ACCOUNTEVENT_ERROR:
        default: {
            FAIL_2("Invalid type", input.type());
        }
    }

    OPTIONAL_IDENTIFIER(uuid);

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
