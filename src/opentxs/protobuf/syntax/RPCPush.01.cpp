// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/RPCPush.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/RPCEnums.pb.h>
#include <opentxs/protobuf/RPCPush.pb.h>
#include <string>

#include "opentxs/protobuf/syntax/AccountEvent.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/ContactEvent.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/TaskComplete.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerifyRPC.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const RPCPush& input, const Log& log) -> bool
{
    CHECK_IDENTIFIER(id);

    switch (input.type()) {
        case RPCPUSH_ACCOUNT: {
            CHECK_SUBOBJECT(accountevent, RPCPushAllowedAccountEvent());
            CHECK_EXCLUDED(contactevent);
            CHECK_EXCLUDED(taskcomplete);
        } break;
        case RPCPUSH_CONTACT: {
            CHECK_EXCLUDED(accountevent);
            CHECK_SUBOBJECT(contactevent, RPCPushAllowedContactEvent());
            CHECK_EXCLUDED(taskcomplete);
        } break;
        case RPCPUSH_TASK: {
            CHECK_EXCLUDED(accountevent);
            CHECK_EXCLUDED(contactevent);
            CHECK_SUBOBJECT(taskcomplete, RPCPushAllowedTaskComplete());
        } break;
        case RPCPUSH_ERROR:
        default: {
            FAIL_2("Invalid type", input.type());
        }
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
