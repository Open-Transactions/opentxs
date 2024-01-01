// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/RPCStatus.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/RPCEnums.pb.h>
#include <opentxs/protobuf/RPCStatus.pb.h>

#include "opentxs/protobuf/syntax/Macros.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_2(const RPCStatus& input, const Log& log) -> bool
{
    switch (input.code()) {
        case RPCRESPONSE_INVALID:
        case RPCRESPONSE_SUCCESS:
        case RPCRESPONSE_BAD_SESSION:
        case RPCRESPONSE_NONE:
        case RPCRESPONSE_QUEUED:
        case RPCRESPONSE_UNNECESSARY:
        case RPCRESPONSE_RETRY:
        case RPCRESPONSE_NO_PATH_TO_RECIPIENT:
        case RPCRESPONSE_ERROR:
        case RPCRESPONSE_BAD_SERVER_ARGUMENT:
        case RPCRESPONSE_CHEQUE_NOT_FOUND:
        case RPCRESPONSE_PAYMENT_NOT_FOUND:
        case RPCRESPONSE_START_TASK_FAILED:
        case RPCRESPONSE_NYM_NOT_FOUND:
        case RPCRESPONSE_ADD_CLAIM_FAILED:
        case RPCRESPONSE_ADD_CONTACT_FAILED:
        case RPCRESPONSE_REGISTER_ACCOUNT_FAILED:
        case RPCRESPONSE_BAD_SERVER_RESPONSE:
        case RPCRESPONSE_WORKFLOW_NOT_FOUND:
        case RPCRESPONSE_UNITDEFINITION_NOT_FOUND:
        case RPCRESPONSE_SESSION_NOT_FOUND:
        case RPCRESPONSE_CREATE_NYM_FAILED:
        case RPCRESPONSE_CREATE_UNITDEFINITION_FAILED:
        case RPCRESPONSE_DELETE_CLAIM_FAILED:
        case RPCRESPONSE_ACCOUNT_NOT_FOUND:
        case RPCRESPONSE_MOVE_FUNDS_FAILED:
        case RPCRESPONSE_REGISTER_NYM_FAILED:
        case RPCRESPONSE_CONTACT_NOT_FOUND:
        case RPCRESPONSE_ACCOUNT_OWNER_NOT_FOUND:
        case RPCRESPONSE_SEND_PAYMENT_FAILED:
        case RPCRESPONSE_TRANSACTION_FAILED:
        case RPCRESPONSE_TXID:
        case RPCRESPONSE_UNIMPLEMENTED: {
        } break;
        default: {
            FAIL_1("invalid success code");
        }
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
