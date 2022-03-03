// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/RPCStatus.hpp"  // IWYU pragma: associated

#include "serialization/protobuf/RPCEnums.pb.h"
#include "serialization/protobuf/RPCStatus.pb.h"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_1(const RPCStatus& input, const bool silent) -> bool
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
        case RPCRESPONSE_ERROR: {
        } break;
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
        case RPCRESPONSE_UNIMPLEMENTED:
        default: {
            FAIL_1("invalid success code")
        }
    }

    return true;
}

auto CheckProto_2(const RPCStatus& input, const bool silent) -> bool
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
            FAIL_1("invalid success code")
        }
    }

    return true;
}

auto CheckProto_3(const RPCStatus& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(3)
}

auto CheckProto_4(const RPCStatus& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(4)
}

auto CheckProto_5(const RPCStatus& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(5)
}

auto CheckProto_6(const RPCStatus& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(6)
}

auto CheckProto_7(const RPCStatus& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(const RPCStatus& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(const RPCStatus& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(const RPCStatus& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(const RPCStatus& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(const RPCStatus& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(const RPCStatus& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(const RPCStatus& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(const RPCStatus& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(const RPCStatus& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(const RPCStatus& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(const RPCStatus& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(const RPCStatus& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(const RPCStatus& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace opentxs::proto
