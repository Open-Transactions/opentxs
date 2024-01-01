// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/OTXPush.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/OTXEnums.pb.h>
#include <opentxs/protobuf/OTXPush.pb.h>

#include "opentxs/protobuf/syntax/Macros.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const OTXPush& input, const Log& log) -> bool
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
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
