// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/OTXPush.hpp"  // IWYU pragma: associated

#include "serialization/protobuf/OTXEnums.pb.h"
#include "serialization/protobuf/OTXPush.pb.h"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_1(const OTXPush& input, const bool silent) -> bool
{
    switch (input.type()) {
        case OTXPUSH_NYMBOX: {
            CHECK_EXCLUDED(accountid);
            CHECK_EXCLUDED(itemid);
            CHECK_EXCLUDED(account);
            CHECK_EXCLUDED(inbox);
            CHECK_EXCLUDED(inboxhash);
            CHECK_EXCLUDED(outbox);
            CHECK_EXCLUDED(outboxhash);
            CHECK_EXISTS(item);
        } break;
        case OTXPUSH_INBOX:
        case OTXPUSH_OUTBOX: {
            CHECK_IDENTIFIER(accountid);
            CHECK_EXISTS(itemid);
            CHECK_EXISTS_STRING(account);
            CHECK_EXISTS_STRING(inbox);
            CHECK_IDENTIFIER(inboxhash);
            CHECK_EXISTS_STRING(outbox);
            CHECK_IDENTIFIER(outboxhash);
            CHECK_EXISTS_STRING(item);
        } break;
        case OTXPUSH_ERROR:
        default: {
            FAIL_1("Invalid type");
        }
    }

    return true;
}

auto CheckProto_2(const OTXPush& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(2)
}

auto CheckProto_3(const OTXPush& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(3)
}

auto CheckProto_4(const OTXPush& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(4)
}

auto CheckProto_5(const OTXPush& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(5)
}

auto CheckProto_6(const OTXPush& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(6)
}

auto CheckProto_7(const OTXPush& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(const OTXPush& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(const OTXPush& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(const OTXPush& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(const OTXPush& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(const OTXPush& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(const OTXPush& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(const OTXPush& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(const OTXPush& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(const OTXPush& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(const OTXPush& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(const OTXPush& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(const OTXPush& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(const OTXPush& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace opentxs::proto
