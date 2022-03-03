// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/AccountEvent.hpp"  // IWYU pragma: associated

#include "serialization/protobuf/AccountEvent.pb.h"
#include "serialization/protobuf/RPCEnums.pb.h"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_1(const AccountEvent& input, const bool silent) -> bool
{
    OPTIONAL_IDENTIFIER(id);
    CHECK_IDENTIFIER(workflow);

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
        case ACCOUNTEVENT_OUTGOINGVOUCHER: {
            OPTIONAL_IDENTIFIER(contact);
        } break;
        case ACCOUNTEVENT_ERROR:
        default: {
            FAIL_2("Invalid type", input.type());
        }
    }

    CHECK_EXCLUDED(uuid);

    return true;
}

auto CheckProto_2(const AccountEvent& input, const bool silent) -> bool
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

auto CheckProto_3(const AccountEvent& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(3)
}

auto CheckProto_4(const AccountEvent& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(4)
}

auto CheckProto_5(const AccountEvent& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(5)
}

auto CheckProto_6(const AccountEvent& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(6)
}

auto CheckProto_7(const AccountEvent& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(const AccountEvent& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(const AccountEvent& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(const AccountEvent& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(const AccountEvent& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(const AccountEvent& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(const AccountEvent& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(const AccountEvent& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(const AccountEvent& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(const AccountEvent& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(const AccountEvent& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(const AccountEvent& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(const AccountEvent& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(const AccountEvent& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace opentxs::proto
