// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/client/Utility.hpp"

#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/client/OT_API.hpp"
#include "opentxs/client/OTAPI_Exec.hpp"
#include "opentxs/consensus/ServerContext.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"

#include <ostream>

#define MIN_MESSAGE_LENGTH 10

#define OT_METHOD "opentxs::Utility::"

namespace opentxs
{
bool VerifyMessage(const std::string& message)
{
    if (MIN_MESSAGE_LENGTH > message.length()) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": too short: ")(message).Flush();

        return false;
    }

    return true;
}

NetworkOperationStatus VerifyMessageSuccess(
    const api::client::Manager& api,
    const std::string& message)
{
    if (!VerifyMessage(message)) { return REPLY_NOT_RECEIVED; }

    const auto status = api.Exec().Message_GetSuccess(message);

    switch (status) {
        case REPLY_NOT_RECEIVED: {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Unable to check success status for message: ")(message)
                .Flush();
        } break;
        case MESSAGE_SUCCESS_FALSE: {
            LogDetail(OT_METHOD)(__FUNCTION__)(
                ": Reply received: success == FALSE for message: ")(message)
                .Flush();
        } break;
        case MESSAGE_SUCCESS_TRUE: {
            LogDetail(OT_METHOD)(__FUNCTION__)(
                ": Reply received: success == TRUE. ")
                .Flush();
        } break;
        default: {
            LogNormal(OT_METHOD)(__FUNCTION__)(": Unknown message status: ")(
                status)(" for message: ")(message)
                .Flush();

            return REPLY_NOT_RECEIVED;
        } break;
    }

    return status;
}

std::int32_t VerifyMsgBalanceAgrmntSuccess(
    const api::client::Manager& api,
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& strMessage)  // For when an OTMessage is the input.
{
    if (!VerifyMessage(strMessage)) { return -1; }

    std::int32_t nSuccess = api.Exec().Message_GetBalanceAgreementSuccess(
        NOTARY_ID, NYM_ID, ACCOUNT_ID, strMessage);
    switch (nSuccess) {
        case -1:
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Error calling "
                "OT_API_Msg_GetBlnceAgrmntSuccess, for message: ")(strMessage)(
                ".")
                .Flush();
            break;
        case 0:
            LogDetail(OT_METHOD)(__FUNCTION__)(": Reply received: success == ")(
                "FALSE. Reply message: ")(strMessage)
                .Flush();
            break;
        case 1:
            LogDetail(OT_METHOD)(__FUNCTION__)(": Reply received: success == ")(
                "TRUE.")
                .Flush();
            break;
        default:
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Error. (This should never "
                "happen!) Input: ")(strMessage)(".")
                .Flush();
            nSuccess = -1;
            break;
    }

    return nSuccess;
}

std::int32_t VerifyMsgTrnxSuccess(
    const api::client::Manager& api,
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& strMessage)
{
    if (!VerifyMessage(strMessage)) { return -1; }

    std::int32_t nSuccess = api.Exec().Message_GetTransactionSuccess(
        NOTARY_ID, NYM_ID, ACCOUNT_ID, strMessage);
    switch (nSuccess) {
        case -1:
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Error calling "
                "OT_API_Message_GetSuccess, for message: ")(strMessage)(".")
                .Flush();
            break;
        case 0:
            LogDetail(OT_METHOD)(__FUNCTION__)(
                ": Reply received: success == FALSE. ")("Reply message: ")(
                strMessage)
                .Flush();
            break;
        case 1:
            LogDetail(OT_METHOD)(__FUNCTION__)(
                ": Reply received: success == TRUE.")
                .Flush();
            break;
        default:
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Error. (This should never happen!) "
                "Input: ")(strMessage)(".")
                .Flush();
            nSuccess = -1;
            break;
    }

    return nSuccess;
}

//
// This code was repeating a lot, so I just added a function for it.
//
std::int32_t InterpretTransactionMsgReply(
    const api::client::Manager& api,
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& strAttempt,
    const std::string& strResponse)
{
    std::int32_t nMessageSuccess = VerifyMessageSuccess(api, strResponse);
    if (-1 == nMessageSuccess) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Message error: ")(strAttempt)(".")
            .Flush();
        return -1;
    }
    if (0 == nMessageSuccess) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Server reply (")(strAttempt)(
            "): Message failure.")
            .Flush();

        return 0;
    }

    std::int32_t nBalanceSuccess = VerifyMsgBalanceAgrmntSuccess(
        api, NOTARY_ID, NYM_ID, ACCOUNT_ID, strResponse);
    if (-1 == nBalanceSuccess) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Balance agreement error: ")(
            strAttempt)(".")
            .Flush();
        return -1;
    }
    if (0 == nBalanceSuccess) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Server reply (")(strAttempt)(
            "): Balance agreement failure.")
            .Flush();
        return 0;
    }

    std::int32_t nTransSuccess =
        VerifyMsgTrnxSuccess(api, NOTARY_ID, NYM_ID, ACCOUNT_ID, strResponse);
    if (-1 == nTransSuccess) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Transaction error: ")(strAttempt)(
            ".")
            .Flush();
        return -1;
    }
    if (0 == nTransSuccess) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Server reply (")(strAttempt)(
            "): Transaction failure.")
            .Flush();
        return 0;
    }

    return 1;
}
}  // namespace opentxs
