// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "OTAPI_Func.hpp"

#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/Workflow.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/client/OT_API.hpp"
#include "opentxs/client/OTAPI_Exec.hpp"
#include "opentxs/client/Utility.hpp"
#include "opentxs/consensus/ServerContext.hpp"
#include "opentxs/core/contract/peer/PeerReply.hpp"
#include "opentxs/core/contract/peer/PeerRequest.hpp"
#include "opentxs/core/script/OTVariable.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/ext/OTPayment.hpp"

#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#define OT_METHOD "opentxs::OTAPI_Func::"

namespace opentxs
{
const std::map<OTAPI_Func_Type, std::string> OTAPI_Func::type_name_{
    {NO_FUNC, "NO_FUNC"},
    {REMOVED1, "REMOVED1"},
    {REGISTER_NYM, "REGISTER_NYM"},
    {DELETE_NYM, "DELETE_NYM"},
    {CHECK_NYM, "CHECK_NYM"},
    {SEND_USER_MESSAGE, "SEND_USER_MESSAGE"},
    {SEND_USER_INSTRUMENT, "SEND_USER_INSTRUMENT"},
    {ISSUE_ASSET_TYPE, "ISSUE_ASSET_TYPE"},
    {ISSUE_BASKET, "ISSUE_BASKET"},
    {CREATE_ASSET_ACCT, "CREATE_ASSET_ACCT"},
    {DELETE_ASSET_ACCT, "DELETE_ASSET_ACCT"},
    {ACTIVATE_SMART_CONTRACT, "ACTIVATE_SMART_CONTRACT"},
    {TRIGGER_CLAUSE, "TRIGGER_CLAUSE"},
    {PROCESS_INBOX, "PROCESS_INBOX"},
    {EXCHANGE_BASKET, "EXCHANGE_BASKET"},
    {DEPOSIT_CASH, "DEPOSIT_CASH"},
    {EXCHANGE_CASH, "EXCHANGE_CASH"},
    {DEPOSIT_CHEQUE, "DEPOSIT_CHEQUE"},
    {WITHDRAW_VOUCHER, "WITHDRAW_VOUCHER"},
    {PAY_DIVIDEND, "PAY_DIVIDEND"},
    {WITHDRAW_CASH, "WITHDRAW_CASH"},
    {GET_CONTRACT, "GET_CONTRACT"},
    {SEND_TRANSFER, "SEND_TRANSFER"},
    {GET_MARKET_LIST, "GET_MARKET_LIST"},
    {CREATE_MARKET_OFFER, "CREATE_MARKET_OFFER"},
    {KILL_MARKET_OFFER, "KILL_MARKET_OFFER"},
    {KILL_PAYMENT_PLAN, "KILL_PAYMENT_PLAN"},
    {DEPOSIT_PAYMENT_PLAN, "DEPOSIT_PAYMENT_PLAN"},
    {GET_NYM_MARKET_OFFERS, "GET_NYM_MARKET_OFFERS"},
    {GET_MARKET_OFFERS, "GET_MARKET_OFFERS"},
    {GET_MARKET_RECENT_TRADES, "GET_MARKET_RECENT_TRADES"},
    {GET_MINT, "GET_MINT"},
    {GET_BOX_RECEIPT, "GET_BOX_RECEIPT"},
    {ADJUST_USAGE_CREDITS, "ADJUST_USAGE_CREDITS"},
    {INITIATE_BAILMENT, "INITIATE_BAILMENT"},
    {INITIATE_OUTBAILMENT, "INITIATE_OUTBAILMENT"},
    {ACKNOWLEDGE_BAILMENT, "ACKNOWLEDGE_BAILMENT"},
    {ACKNOWLEDGE_OUTBAILMENT, "ACKNOWLEDGE_OUTBAILMENT"},
    {NOTIFY_BAILMENT, "NOTIFY_BAILMENT"},
    {ACKNOWLEDGE_NOTICE, "ACKNOWLEDGE_NOTICE"},
    {REQUEST_CONNECTION, "REQUEST_CONNECTION"},
    {ACKNOWLEDGE_CONNECTION, "ACKNOWLEDGE_CONNECTION"},
    {REGISTER_CONTRACT_NYM, "REGISTER_CONTRACT_NYM"},
    {REGISTER_CONTRACT_SERVER, "REGISTER_CONTRACT_SERVER"},
    {REGISTER_CONTRACT_UNIT, "REGISTER_CONTRACT_UNIT"},
    {REQUEST_ADMIN, "REQUEST_ADMIN"},
    {SERVER_ADD_CLAIM, "SERVER_ADD_CLAIM"},
    {STORE_SECRET, "STORE_SECRET"},
    {GET_TRANSACTION_NUMBERS, "GET_TRANSACTION_NUMBERS"},
};

const std::map<OTAPI_Func_Type, bool> OTAPI_Func::type_type_{
    {REGISTER_NYM, false},
    {DELETE_NYM, false},
    {CHECK_NYM, false},
    {SEND_USER_MESSAGE, false},
    {SEND_USER_INSTRUMENT, false},
    {ISSUE_ASSET_TYPE, false},
    {ISSUE_BASKET, false},
    {CREATE_ASSET_ACCT, false},
    {DELETE_ASSET_ACCT, false},
    {ACTIVATE_SMART_CONTRACT, true},
    {TRIGGER_CLAUSE, false},
    {PROCESS_INBOX, true},
    {EXCHANGE_BASKET, true},
    {DEPOSIT_CASH, true},
    {EXCHANGE_CASH, true},
    {DEPOSIT_CHEQUE, true},
    {WITHDRAW_VOUCHER, true},
    {PAY_DIVIDEND, true},
    {WITHDRAW_CASH, true},
    {GET_CONTRACT, false},
    {SEND_TRANSFER, true},
    {GET_MARKET_LIST, false},
    {CREATE_MARKET_OFFER, true},
    {KILL_MARKET_OFFER, true},
    {KILL_PAYMENT_PLAN, true},
    {DEPOSIT_PAYMENT_PLAN, true},
    {GET_NYM_MARKET_OFFERS, false},
    {GET_MARKET_OFFERS, false},
    {GET_MARKET_RECENT_TRADES, false},
    {GET_MINT, false},
    {GET_BOX_RECEIPT, false},
    {ADJUST_USAGE_CREDITS, false},
    {INITIATE_BAILMENT, false},
    {INITIATE_OUTBAILMENT, false},
    {ACKNOWLEDGE_BAILMENT, false},
    {ACKNOWLEDGE_OUTBAILMENT, false},
    {NOTIFY_BAILMENT, false},
    {ACKNOWLEDGE_NOTICE, false},
    {REQUEST_CONNECTION, false},
    {ACKNOWLEDGE_CONNECTION, false},
    {REGISTER_CONTRACT_NYM, false},
    {REGISTER_CONTRACT_SERVER, false},
    {REGISTER_CONTRACT_UNIT, false},
    {REQUEST_ADMIN, false},
    {SERVER_ADD_CLAIM, false},
    {STORE_SECRET, false},
    {GET_TRANSACTION_NUMBERS, false},
};

OTAPI_Func::OTAPI_Func(
    std::recursive_mutex& apiLock,
    const api::client::Manager& api,
    const Identifier& nymID,
    const Identifier& serverID,
    const OTAPI_Func_Type type)
    : type_(type)
    , api_lock_(apiLock)
    , accountID_(Identifier::Factory())
    , basketID_(Identifier::Factory())
    , currencyAccountID_(Identifier::Factory())
    , instrumentDefinitionID_(Identifier::Factory())
    , marketID_(Identifier::Factory())
    , recipientID_(Identifier::Factory())
    , requestID_(Identifier::Factory())
    , targetID_(Identifier::Factory())
    , message_id_(Identifier::Factory())
    , request_(nullptr)
    , contract_(nullptr)
    , paymentPlan_(nullptr)
#if OT_CASH
    , purse_(nullptr)
    , senderPurse_(nullptr)
#endif
    , cheque_(nullptr)
    , ledger_(nullptr)
    , payment_(nullptr)
    , clause_("")
    , key_("")
    , login_("")
    , message_("")
    , parameter_("")
    , password_("")
    , primary_("")
    , secondary_("")
    , stopSign_("")
    , txid_("")
    , url_("")
    , value_("")
    , label_("")
    , ack_(false)
    , direction_(false)
    , isPrimary_(false)
    , selling_(false)
#if OT_CASH
    , cash_(false)
#endif
    , resync_(false)
    , lifetime_(OT_TIME_ZERO)
    , nRequestNum_(-1)
    , nTransNumsNeeded_(0)
    , api_(api)
    , context_editor_(api_.Wallet().mutable_ServerContext(nymID, serverID))
    , context_(context_editor_.It())
    , last_attempt_()
    , is_transaction_(type_type_.at(type))
    , peer_reply_(nullptr)
    , peer_request_(nullptr)
    , sectionName_(proto::CONTACTSECTION_ERROR)
    , itemType_(proto::CITEMTYPE_ERROR)
    , activationPrice_(0)
    , adjustment_(0)
    , amount_(0)
    , depth_(0)
    , increment_(0)
    , quantity_(0)
    , price_(0)
    , remoteBoxType_(RemoteBoxType::Error)
#if OT_CASH
    , transactionNumber_(0)  // This is not what gets returned by
#endif
    , infoType_(proto::CONNECTIONINFO_ERROR)
    , unitDefinition_()
{
    OT_ASSERT(verify_lock(api_lock_, apiLock));
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::client::Manager& api,
    const Identifier& nymID,
    const Identifier& serverID)
    : OTAPI_Func(apilock, api, nymID, serverID, theType)
{
    if (theType == DELETE_NYM) {
        nTransNumsNeeded_ = 0;
    } else if (theType == GET_MARKET_LIST) {
        nTransNumsNeeded_ = 0;
    } else if (theType == GET_NYM_MARKET_OFFERS) {
        nTransNumsNeeded_ = 0;
    } else if (theType != GET_TRANSACTION_NUMBERS) {
        OT_FAIL
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::client::Manager& api,
    const Identifier& nymID,
    const Identifier& serverID,
    const bool resync)
    : OTAPI_Func(apilock, api, nymID, serverID, theType)
{
    if (theType == REGISTER_NYM) {
        resync_ = resync;
    } else {
        OT_FAIL
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::client::Manager& api,
    const Identifier& nymID,
    const Identifier& serverID,
    const std::string& password)
    : OTAPI_Func(apilock, api, nymID, serverID, theType)
{
    std::string strError =
        "Warning: Empty std::string passed to OTAPI_Func.OTAPI_Func() as: ";

    if (!VerifyStringVal(password)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": password.").Flush();
    }

    switch (theType) {
        case REQUEST_ADMIN: {
            password_ = password;
        } break;
        default: {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func().")
                .Flush();
            OT_FAIL
        }
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::client::Manager& api,
    const Identifier& nymID,
    const Identifier& serverID,
    const proto::UnitDefinition& unitDefinition,
    const std::string& label)
    : OTAPI_Func(apilock, api, nymID, serverID, theType)
{
    switch (theType) {
        case (ISSUE_BASKET):
        case (ISSUE_ASSET_TYPE): {
            unitDefinition_ = unitDefinition;
            label_ = label;
        } break;
        default: {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func().")
                .Flush();
            OT_FAIL
        }
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::client::Manager& api,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& nymID2)
    : OTAPI_Func(apilock, api, nymID, serverID, theType)
{
    switch (theType) {
        case GET_MINT:
        case GET_CONTRACT:
        case REGISTER_CONTRACT_UNIT: {
            instrumentDefinitionID_ = nymID2;
        } break;
        case GET_MARKET_RECENT_TRADES: {
            marketID_ = nymID2;
        } break;
        case CHECK_NYM:
        case REGISTER_CONTRACT_NYM:
        case REGISTER_CONTRACT_SERVER: {
            targetID_ = nymID2;
        } break;
        case DELETE_ASSET_ACCT: {
            accountID_ = nymID2;
        } break;
        default: {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func().")
                .Flush();
            OT_FAIL
        }
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::client::Manager& api,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& accountID,
    std::unique_ptr<Ledger>& ledger)
    : OTAPI_Func(apilock, api, nymID, serverID, theType)
{
    switch (theType) {
        case PROCESS_INBOX: {
            nTransNumsNeeded_ = 1;
            accountID_ = accountID;
            ledger_.reset(ledger.release());
        } break;
        default: {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func().")
                .Flush();
            OT_FAIL
        }
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::client::Manager& api,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& targetID,
    const Identifier& instrumentDefinitionID)
    : OTAPI_Func(apilock, api, nymID, serverID, theType)
{
    switch (theType) {
        case INITIATE_BAILMENT: {
            targetID_ = targetID;
            instrumentDefinitionID_ = instrumentDefinitionID;
            peer_request_ = PeerRequest::Create(
                api_.Wallet(),
                context_.Nym(),
                proto::PEERREQUEST_BAILMENT,
                instrumentDefinitionID_,
                context_.Server());

            OT_ASSERT(peer_request_);
        } break;
        default: {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func().")
                .Flush();
            OT_FAIL
        }
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::client::Manager& api,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& accountID,
    std::unique_ptr<Cheque>& cheque)
    : OTAPI_Func(apilock, api, nymID, serverID, theType)
{
    switch (theType) {
        case DEPOSIT_CHEQUE: {
            nTransNumsNeeded_ = 1;
            accountID_ = accountID;
            cheque_.reset(cheque.release());
        } break;
        default: {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func().")
                .Flush();
            OT_FAIL
        }
    }
}

#if OT_CASH
OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::client::Manager& api,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& nymID2,
    std::unique_ptr<Purse>& purse)
    : OTAPI_Func(apilock, api, nymID, serverID, theType)
{
    switch (theType) {
        case DEPOSIT_CASH: {
            nTransNumsNeeded_ = 1;
            accountID_ = nymID2;
            purse_.reset(purse.release());
        } break;
        case EXCHANGE_CASH: {
            nTransNumsNeeded_ = 1;
            instrumentDefinitionID_ = nymID2;
            purse_.reset(purse.release());
        } break;
        default: {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func().")
                .Flush();
            OT_FAIL
        }
    }
}
#endif
OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::client::Manager& api,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& recipientID,
    std::unique_ptr<OTPaymentPlan>& paymentPlan)
    : OTAPI_Func(apilock, api, nymID, serverID, theType)
{
    std::string strError =
        "Warning: Empty std::string passed to OTAPI_Func.OTAPI_Func() as: ";

    switch (theType) {
        case DEPOSIT_PAYMENT_PLAN: {
            nTransNumsNeeded_ = 1;
            accountID_ = recipientID;
            paymentPlan_.reset(paymentPlan.release());
        } break;
        default: {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func().")
                .Flush();
            OT_FAIL
        }
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::client::Manager& api,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& nymID2,
    const std::int64_t& int64val)
    : OTAPI_Func(apilock, api, nymID, serverID, theType)
{
    switch (theType) {
        case ADJUST_USAGE_CREDITS: {
            targetID_ = nymID2;      // target nym ID
            adjustment_ = int64val;  // adjustment (up or down.)
        } break;
        case GET_MARKET_OFFERS: {
            marketID_ = nymID2;
            depth_ = int64val;
        } break;
        case KILL_PAYMENT_PLAN:
        case KILL_MARKET_OFFER: {
            nTransNumsNeeded_ = 1;
            accountID_ = nymID2;
            transactionNumber_ = int64val;
        } break;
        case (WITHDRAW_CASH): {
            nTransNumsNeeded_ = 1;
            accountID_ = nymID2;
            amount_ = int64val;
        } break;
        default: {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func().")
                .Flush();
            OT_FAIL
        }
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::client::Manager& api,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& targetID,
    const proto::ConnectionInfoType& infoType)
    : OTAPI_Func(apilock, api, nymID, serverID, theType)
{
    infoType_ = infoType;

    switch (theType) {
        case REQUEST_CONNECTION: {
            targetID_ = targetID;
            peer_request_ = PeerRequest::Create(
                api_.Wallet(),
                context_.Nym(),
                proto::PEERREQUEST_CONNECTIONINFO,
                infoType_,
                targetID_,
                context_.Server());

            OT_ASSERT(peer_request_)
        } break;
        default: {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func().")
                .Flush();
            OT_FAIL
        }
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::client::Manager& api,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& recipientID,
    const Identifier& requestID,
    const bool ack)
    : OTAPI_Func(apilock, api, nymID, serverID, theType)
{
    if (theType == ACKNOWLEDGE_NOTICE) {
        nTransNumsNeeded_ = 0;
        requestID_ = requestID;
        accountID_ = recipientID;
        peer_reply_ = PeerReply::Create(
            api_.Wallet(), context_.Nym(), requestID_, context_.Server(), ack);

        OT_ASSERT(peer_reply_)
    } else {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func(). "
            "ERROR!!!!!!")
            .Flush();
        OT_FAIL
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::client::Manager& api,
    const Identifier& nymID,
    const Identifier& serverID,
    const TransactionNumber& transactionNumber,
    const std::string& clause,
    const std::string& parameter)
    : OTAPI_Func(apilock, api, nymID, serverID, theType)
{
    std::string strError =
        "Warning: Empty std::string passed to OTAPI_Func.OTAPI_Func() as: ";

    if (!VerifyStringVal(clause)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": clause.").Flush();
    }

    if (!VerifyStringVal(parameter)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": parameter.").Flush();
    }

    nTransNumsNeeded_ = 1;

    if (theType == TRIGGER_CLAUSE) {
        transactionNumber_ = transactionNumber;
        clause_ = clause;
        parameter_ = parameter;
    } else {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func(). "
            "ERROR!!!!!!")
            .Flush();
        OT_FAIL
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::client::Manager& api,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& recipientID,
    std::unique_ptr<const OTPayment>& payment)
    : OTAPI_Func(apilock, api, nymID, serverID, theType)
{
    nTransNumsNeeded_ = 1;

    if (theType == SEND_USER_INSTRUMENT) {
        nTransNumsNeeded_ = 0;
        recipientID_ = recipientID;
        payment_.reset(payment.release());
    } else {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func(). "
            "ERROR!!!!!!")
            .Flush();
        OT_FAIL
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::client::Manager& api,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& recipientID,
    const std::string& message)
    : OTAPI_Func(apilock, api, nymID, serverID, theType)
{
    std::string strError =
        "Warning: Empty std::string passed to OTAPI_Func.OTAPI_Func() as: ";

    if (!VerifyStringVal(message)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": message.").Flush();
    }

    switch (theType) {
        case SEND_USER_MESSAGE: {
            nTransNumsNeeded_ = 0;
            recipientID_ = recipientID;
            message_ = message;
        } break;
        case CREATE_ASSET_ACCT: {
            nTransNumsNeeded_ = 1;  // So it's done at least one "transaction
            // statement" before it can ever processInbox
            // on an account.
            instrumentDefinitionID_ = recipientID;
            label_ = message;
        } break;
        default: {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func().")
                .Flush();
            OT_FAIL
        }
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::client::Manager& api,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& accountID,
    const RemoteBoxType& remoteBoxType,
    const TransactionNumber& transactionNumber)
    : OTAPI_Func(apilock, api, nymID, serverID, theType)
{
    nTransNumsNeeded_ = 1;

    if (theType == GET_BOX_RECEIPT) {
        nTransNumsNeeded_ = 0;
        accountID_ = accountID;
        remoteBoxType_ = remoteBoxType;
        transactionNumber_ = transactionNumber;
    } else {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func(). "
            "ERROR!!!!!!")
            .Flush();
        OT_FAIL
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::client::Manager& api,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& accountID,
    const std::string& agentName,
    std::unique_ptr<OTSmartContract>& contract)
    : OTAPI_Func(apilock, api, nymID, serverID, theType)
{
    std::string strError =
        "Warning: Empty std::string passed to OTAPI_Func.OTAPI_Func() as: ";

    if (!VerifyStringVal(agentName)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": agentName.").Flush();
    }

    nTransNumsNeeded_ = 1;

    if (theType == ACTIVATE_SMART_CONTRACT) {

        accountID_ = accountID;  // the "official" asset account of the party
                                 // activating the contract.;
        agentName_ = agentName;  // the agent's name for that party, as listed
                                 // on the contract.;
        contract_.reset(contract.release());  // the smart contract itself.;

        std::int32_t nNumsNeeded = api_.Exec().SmartContract_CountNumsNeeded(
            String::Factory(*contract_)->Get(), agentName_);

        if (nNumsNeeded > 0) { nTransNumsNeeded_ = nNumsNeeded; }
    } else {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func(). "
            "ERROR!!!!!!")
            .Flush();
        OT_FAIL
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::client::Manager& api,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& recipientID,
    const Identifier& requestID,
    const std::string& instructions)
    : OTAPI_Func(apilock, api, nymID, serverID, theType)
{
    switch (theType) {
        case ACKNOWLEDGE_BAILMENT: {
            nTransNumsNeeded_ = 0;
            accountID_ = recipientID;
            requestID_ = requestID;
            message_ = instructions;
            peer_reply_ = PeerReply::Create(
                api_.Wallet(),
                context_.Nym(),
                proto::PEERREQUEST_BAILMENT,
                requestID_,
                context_.Server(),
                message_);

            OT_ASSERT(peer_reply_)
        } break;

        case ACKNOWLEDGE_OUTBAILMENT: {
            nTransNumsNeeded_ = 0;
            accountID_ = recipientID;
            requestID_ = requestID;
            message_ = instructions;
            peer_reply_ = PeerReply::Create(
                api_.Wallet(),
                context_.Nym(),
                proto::PEERREQUEST_OUTBAILMENT,
                requestID_,
                context_.Server(),
                message_);

            OT_ASSERT(peer_reply_)
        } break;
        default: {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func(). "
                "ERROR!!!!!!")
                .Flush();
            OT_FAIL
        }
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::client::Manager& api,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& nymID2,
    const Identifier& targetID,
    const Amount& amount,
    const std::string& message)
    : OTAPI_Func(apilock, api, nymID, serverID, theType)
{
    amount_ = amount;
    nTransNumsNeeded_ = 0;
    message_ = message;

    switch (theType) {
        case SEND_TRANSFER: {
            std::string strError = "Warning: Empty std::string passed to "
                                   "OTAPI_Func.OTAPI_Func() as: ";

            if (!VerifyStringVal(message)) {
                LogOutput(OT_METHOD)(__FUNCTION__)(": message.").Flush();
            }
            nTransNumsNeeded_ = 1;
            accountID_ = nymID2;
            targetID_ = targetID;
        } break;
        case INITIATE_OUTBAILMENT: {
            targetID_ = nymID2;
            instrumentDefinitionID_ = targetID;
            peer_request_ = PeerRequest::Create(
                api_.Wallet(),
                context_.Nym(),
                proto::PEERREQUEST_OUTBAILMENT,
                instrumentDefinitionID_,
                context_.Server(),
                amount_,
                message_);

            OT_ASSERT(peer_request_)
        } break;
        case PAY_DIVIDEND: {
            accountID_ = targetID;
            instrumentDefinitionID_ = nymID2;
        } break;
        case WITHDRAW_VOUCHER: {
            accountID_ = targetID;
            recipientID_ = nymID2;
        } break;
        default: {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func(). "
                "ERROR!!!!!!")
                .Flush();
            OT_FAIL
        }
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::client::Manager& api,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& targetID,
    const std::string& primary,
    const std::string& secondary,
    const proto::SecretType& secretType)
    : OTAPI_Func(apilock, api, nymID, serverID, theType)
{
    primary_ = primary;
    secondary_ = secondary;
    secretType_ = secretType;
    std::string strError =
        "Warning: Empty std::string passed to OTAPI_Func.OTAPI_Func() as: ";

    if (!VerifyStringVal(primary)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": primary.").Flush();
    }

    if (!VerifyStringVal(secondary)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": secondary.").Flush();
    }

    switch (theType) {
        case STORE_SECRET: {
            nTransNumsNeeded_ = 0;
            targetID_ = targetID;
            peer_request_ = PeerRequest::Create(
                api_.Wallet(),
                context_.Nym(),
                proto::PEERREQUEST_STORESECRET,
                secretType_,
                targetID_,
                primary_,
                secondary_,
                context_.Server());

            OT_ASSERT(peer_request_)
        } break;
        default: {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func(). "
                "ERROR!!!!!!")
                .Flush();
            OT_FAIL
        }
    }
}

#if OT_CASH
OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::client::Manager& api,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& recipientID,
    std::unique_ptr<const Purse>& purse,
    std::unique_ptr<const Purse>& senderPurse)
    : OTAPI_Func(apilock, api, nymID, serverID, theType)
{
    nTransNumsNeeded_ = 1;

    if (theType == SEND_USER_INSTRUMENT) {
        nTransNumsNeeded_ = 0;
        recipientID_ = recipientID;     // Recipient Nym;
        purse_.reset(purse.release());  // Instrument for recipient.;
        senderPurse_.reset(senderPurse.release());  // sender_instrument is
                                                    // attached here.
                                                    // (Optional.);
        cash_ = true;
    } else {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func(). "
            "ERROR!!!!!!")
            .Flush();
        OT_FAIL
    }
}
#endif
OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::client::Manager& api,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& recipientID,
    const Identifier& requestID,
    const Identifier& instrumentDefinitionID,
    const std::string& txid,
    const Amount& amount)
    : OTAPI_Func(apilock, api, nymID, serverID, theType)
{
    std::string strError =
        "Warning: Empty std::string passed to OTAPI_Func.OTAPI_Func() as: ";

    nTransNumsNeeded_ = 1;

    if (theType == NOTIFY_BAILMENT) {
        if (!VerifyStringVal(txid)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": txid.").Flush();
        }
        nTransNumsNeeded_ = 0;
        accountID_ = recipientID;
        requestID_ = requestID;
        instrumentDefinitionID_ = instrumentDefinitionID;
        txid_ = txid;
        amount_ = amount;
        peer_request_ = PeerRequest::Create(
            api_.Wallet(),
            context_.Nym(),
            proto::PEERREQUEST_PENDINGBAILMENT,
            instrumentDefinitionID_,
            context_.Server(),
            accountID_,  // Recepient
            requestID_,  // Request ID
            txid_,       // txid
            amount_);

        OT_ASSERT(peer_request_)
    } else {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func(). "
            "ERROR!!!!!!")
            .Flush();
        OT_FAIL
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::client::Manager& api,
    const Identifier& nymID,
    const Identifier& serverID,
    bool primary,
    const proto::ContactSectionName& sectionName,
    const proto::ContactItemType& itemType,
    const std::string& value)
    : OTAPI_Func(apilock, api, nymID, serverID, theType)
{
    const std::string strError =
        "Warning: Empty std::string passed to OTAPI_Func.OTAPI_Func() as: ";

    if (!VerifyStringVal(value)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": value.").Flush();
    }

    switch (theType) {
        case SERVER_ADD_CLAIM: {
            sectionName_ = sectionName;
            itemType_ = itemType;
            value_ = value;
            isPrimary_ = primary;
        } break;
        default: {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func().")
                .Flush();
            OT_FAIL
        }
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::client::Manager& api,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& instrumentDefinitionID,
    const Identifier& basketID,
    const Identifier& accountID,
    bool direction,
    std::int32_t nTransNumsNeeded)
    : OTAPI_Func(apilock, api, nymID, serverID, theType)
{
    if (EXCHANGE_BASKET == theType) {
        // FYI. This is a transaction.
        nTransNumsNeeded_ = nTransNumsNeeded;
        direction_ = direction;
        instrumentDefinitionID_ = instrumentDefinitionID;
        basketID_ = basketID;
        accountID_ = accountID;
    } else {
        OT_FAIL
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::client::Manager& api,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& recipientID,
    const Identifier& requestID,
    const std::string& url,
    const std::string& login,
    const std::string& password,
    const std::string& key,
    bool ack)
    : OTAPI_Func(apilock, api, nymID, serverID, theType)
{
    accountID_ = recipientID;
    requestID_ = requestID;
    url_ = url;
    login_ = login;
    password_ = password;
    key_ = key;
    ack_ = ack;
    const std::string strError =
        "Warning: Empty std::string passed to OTAPI_Func.OTAPI_Func() as: ";

    if (ack_) {
        if (!VerifyStringVal(url)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": url.").Flush();
        }

        if (!VerifyStringVal(login)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": login.").Flush();
        }

        if (!VerifyStringVal(password)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": password.").Flush();
        }

        if (!VerifyStringVal(key)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": key.").Flush();
        }
    }

    switch (theType) {
        case ACKNOWLEDGE_CONNECTION: {
            peer_reply_ = PeerReply::Create(
                api_.Wallet(),
                context_.Nym(),
                requestID_,
                context_.Server(),
                ack_,
                url_,
                login_,
                password_,
                key_);

            OT_ASSERT(peer_reply_)
        } break;
        default: {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func().")
                .Flush();
            OT_FAIL
        }
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::client::Manager& api,
    const Identifier& nymID,
    const Identifier& serverID,
    const Identifier& assetAccountID,
    const Identifier& currencyAccountID,
    const Amount& scale,
    const Amount& increment,
    const Amount& quantity,
    const Amount& price,
    const bool selling,
    const time64_t lifetime,
    const Amount& activationPrice,
    const std::string& stopSign)
    : OTAPI_Func(apilock, api, nymID, serverID, theType)
{
    if (VerifyStringVal(stopSign)) { stopSign_ = stopSign; }

    switch (theType) {
        case CREATE_MARKET_OFFER: {
            nTransNumsNeeded_ = 3;
            accountID_ = assetAccountID;
            currencyAccountID_ = currencyAccountID;
            scale_ = scale;
            increment_ = increment;
            quantity_ = quantity;
            price_ = price;
            selling_ = selling;
            lifetime_ = lifetime;
            activationPrice_ = activationPrice;
        } break;
        default: {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func().")
                .Flush();
            OT_FAIL
        }
    }
}

TransactionNumber OTAPI_Func::GetTransactionNumber() const
{
    return std::get<1>(last_attempt_);
}

SendResult OTAPI_Func::LastSendResult() const
{
    Lock lock(lock_);

    return std::get<0>(std::get<2>(last_attempt_));
}

const OTIdentifier OTAPI_Func::MessageID() const
{
    Lock lock(lock_);

    return message_id_;
}

const std::shared_ptr<Message>& OTAPI_Func::Reply() const
{
    Lock lock(lock_);

    return std::get<1>(std::get<2>(last_attempt_));
}

std::string OTAPI_Func::Run(const std::size_t totalRetries)
{
    if (is_transaction_) {

        return send_transaction(totalRetries);
    } else {

        return send_request();
    }
}

void OTAPI_Func::run()
{
    Lock lock(lock_);
    const auto triggerParameter = String::Factory(parameter_);
    auto& [requestNum, transactionNum, result] = last_attempt_;
    auto& [status, reply] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();

    switch (type_) {
        case CHECK_NYM: {
            last_attempt_ = api_.OTAPI().checkNym(context_, targetID_);
        } break;
        case REGISTER_NYM: {
            last_attempt_ = api_.OTAPI().registerNym(context_, resync_);
        } break;
        case DELETE_NYM: {
            last_attempt_ = api_.OTAPI().unregisterNym(context_);
        } break;
        case SEND_USER_MESSAGE: {
            last_attempt_ = api_.OTAPI().sendNymMessage(
                context_, recipientID_, message_, message_id_);
        } break;
        case SEND_USER_INSTRUMENT: {
#if OT_CASH
            if (cash_) {
                OT_ASSERT(purse_)

                payment_ = api_.Factory().Payment(String::Factory(*purse_));
            }
#endif

            OT_ASSERT(payment_)

            auto serialized = String::Factory();
            payment_->GetPaymentContents(serialized);
            auto payment{api_.Factory().Payment(serialized)};

            if (!payment->IsValid() || !payment->SetTempValues()) {
                LogNormal(OT_METHOD)(__FUNCTION__)(
                    ": Failure loading payment instrument "
                    "(intended for recipient) from string: ")(
                    payment->Payment())
                    .Flush();
                return;
            }

#if OT_CASH
            if (cash_) {
                OT_ASSERT(senderPurse_)

                const String& senderPurseString =
                    String::Factory(*senderPurse_);
                auto theSenderPayment{
                    api_.Factory().Payment(senderPurseString)};

                if (!theSenderPayment->IsValid() ||
                    !theSenderPayment->SetTempValues()) {
                    LogNormal(OT_METHOD)(__FUNCTION__)(
                        ": Failure loading payment instrument (copy "
                        "intended for sender's records) from string: ")(
                        senderPurseString.Get())
                        .Flush();

                    return;
                }

                last_attempt_ = api_.OTAPI().sendNymInstrument(
                    context_,
                    request_,
                    recipientID_,
                    *payment,
                    true,
                    theSenderPayment.get());
            } else {
#endif
                last_attempt_ = api_.OTAPI().sendNymInstrument(
                    context_,
                    request_,
                    recipientID_,
                    *payment,
                    !payment->IsCheque());
#if OT_CASH
            }
#endif
            if (request_ && payment->IsCheque()) {
                bool workflowUpdated{false};
                auto cheque{api_.Factory().Cheque()};
                const auto loaded =
                    cheque->LoadContractFromString(payment->Payment());

                if (false == loaded) {
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Failed to load cheque.")
                        .Flush();
                    break;
                }

                if (payment->IsCancelledCheque()) {
                    workflowUpdated = api_.Workflow().CancelCheque(
                        *cheque,
                        *request_,
                        std::get<1>(std::get<2>(last_attempt_)).get());
                } else {
                    workflowUpdated = api_.Workflow().SendCheque(
                        *cheque,
                        *request_,
                        std::get<1>(std::get<2>(last_attempt_)).get());
                }

                if (workflowUpdated) {
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Successfully updated workflow.")
                        .Flush();
                } else {
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Failed to update workflow.")
                        .Flush();
                }
            }
        } break;
        case GET_NYM_MARKET_OFFERS: {
            last_attempt_ = api_.OTAPI().getNymMarketOffers(context_);
        } break;
        case CREATE_ASSET_ACCT: {
            last_attempt_ = api_.OTAPI().registerAccount(
                context_, instrumentDefinitionID_, label_);
        } break;
        case DELETE_ASSET_ACCT: {
            last_attempt_ =
                api_.OTAPI().deleteAssetAccount(context_, accountID_);
        } break;
        case ACTIVATE_SMART_CONTRACT: {
            OT_ASSERT(contract_)

            last_attempt_ = api_.OTAPI().activateSmartContract(
                context_, String::Factory(*contract_));
        } break;
        case TRIGGER_CLAUSE: {
            last_attempt_ = api_.OTAPI().triggerClause(
                context_,
                transactionNumber_,
                String::Factory(clause_.c_str()),
                triggerParameter->Exists() ? triggerParameter
                                           : String::Factory());
        } break;
        case EXCHANGE_BASKET: {
            last_attempt_ = api_.OTAPI().exchangeBasket(
                context_,
                instrumentDefinitionID_,
                String::Factory(basketID_),
                direction_);
        } break;
        case GET_CONTRACT: {
            last_attempt_ = api_.OTAPI().getInstrumentDefinition(
                context_, instrumentDefinitionID_);
        } break;
        case GET_MINT: {
            last_attempt_ =
                api_.OTAPI().getMint(context_, instrumentDefinitionID_);
        } break;
        case ISSUE_BASKET: {
            last_attempt_ =
                api_.OTAPI().issueBasket(context_, unitDefinition_, label_);
        } break;
        case ISSUE_ASSET_TYPE: {
            last_attempt_ = api_.OTAPI().registerInstrumentDefinition(
                context_, unitDefinition_, label_);
        } break;
        case EXCHANGE_CASH: {
#if OT_CASH
            LogOutput(OT_METHOD)(__FUNCTION__)(": TODO (NOT CODED YET).")
                .Flush();
#endif  // OT_CASH
        } break;
        case KILL_MARKET_OFFER: {
            last_attempt_ = api_.OTAPI().cancelCronItem(
                context_, accountID_, transactionNumber_);
        } break;
        case KILL_PAYMENT_PLAN: {
            last_attempt_ = api_.OTAPI().cancelCronItem(
                context_, accountID_, transactionNumber_);
        } break;
        case GET_BOX_RECEIPT: {
            last_attempt_ = api_.OTAPI().getBoxReceipt(
                context_,
                accountID_,
                static_cast<std::uint32_t>(remoteBoxType_),
                transactionNumber_);
        } break;
        case PROCESS_INBOX: {
            OT_ASSERT(ledger_)

            last_attempt_ = api_.OTAPI().processInbox(
                context_, accountID_, String::Factory(*ledger_));
        } break;
        case DEPOSIT_CASH: {
#if OT_CASH
            OT_ASSERT(purse_)

            last_attempt_ = api_.OTAPI().notarizeDeposit(
                context_, accountID_, String::Factory(*purse_));
#endif  // OT_CASH
        } break;
        case DEPOSIT_CHEQUE: {
            OT_ASSERT(cheque_)

            last_attempt_ =
                api_.OTAPI().depositCheque(context_, accountID_, *cheque_);
        } break;
        case DEPOSIT_PAYMENT_PLAN: {
            OT_ASSERT(paymentPlan_)

            last_attempt_ = api_.OTAPI().depositPaymentPlan(
                context_, String::Factory(*paymentPlan_));
        } break;
        case WITHDRAW_CASH: {
#if OT_CASH
            last_attempt_ =
                api_.OTAPI().notarizeWithdrawal(context_, accountID_, amount_);
#endif  // OT_CASH
        } break;
        case WITHDRAW_VOUCHER: {
            last_attempt_ = api_.OTAPI().withdrawVoucher(
                context_,
                accountID_,
                recipientID_,
                String::Factory(message_.c_str()),
                amount_);
        } break;
        case PAY_DIVIDEND: {
            last_attempt_ = api_.OTAPI().payDividend(
                context_,
                accountID_,
                instrumentDefinitionID_,
                String::Factory(message_.c_str()),
                amount_);
        } break;
        case SEND_TRANSFER: {
            last_attempt_ = api_.OTAPI().notarizeTransfer(
                context_,
                accountID_,
                targetID_,
                amount_,
                String::Factory(message_.c_str()));
        } break;
        case GET_MARKET_LIST: {
            last_attempt_ = api_.OTAPI().getMarketList(context_);
        } break;
        case GET_MARKET_OFFERS: {
            last_attempt_ =
                api_.OTAPI().getMarketOffers(context_, marketID_, depth_);
        } break;
        case GET_MARKET_RECENT_TRADES: {
            last_attempt_ =
                api_.OTAPI().getMarketRecentTrades(context_, marketID_);
        } break;
        case CREATE_MARKET_OFFER: {
            const auto ASSET_ACCT_ID = Identifier::Factory(accountID_);
            const auto CURRENCY_ACCT_ID =
                Identifier::Factory(currencyAccountID_);
            const std::int64_t MARKET_SCALE = scale_;
            const std::int64_t MINIMUM_INCREMENT = increment_;
            const std::int64_t TOTAL_ASSETS_ON_OFFER = quantity_;
            const Amount PRICE_LIMIT = price_;
            const auto& bBuyingOrSelling = selling_;
            const auto& tLifespanInSeconds = lifetime_;
            const auto& STOP_SIGN = stopSign_;
            const auto& ACTIVATION_PRICE = activationPrice_;
            char cStopSign = 0;

            if (0 == STOP_SIGN.compare("<")) {
                cStopSign = '<';
            } else if (0 == STOP_SIGN.compare(">")) {
                cStopSign = '>';
            }

            if (!STOP_SIGN.empty() &&
                ((ACTIVATION_PRICE == 0) ||
                 ((cStopSign != '<') && (cStopSign != '>')))) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": If STOP_SIGN is provided, it must be < "
                    "or >, and in that case ACTIVATION_PRICE "
                    "must be non-zero.")
                    .Flush();

                return;
            }

            const auto str_asset_notary_id =
                api_.Exec().GetAccountWallet_NotaryID(accountID_->str());
            const auto str_currency_notary_id =
                api_.Exec().GetAccountWallet_NotaryID(
                    currencyAccountID_->str());
            const auto str_asset_nym_id =
                api_.Exec().GetAccountWallet_NymID(accountID_->str());
            const auto str_currency_nym_id =
                api_.Exec().GetAccountWallet_NymID(currencyAccountID_->str());

            if (str_asset_notary_id.empty() || str_currency_notary_id.empty() ||
                str_asset_nym_id.empty() || str_currency_nym_id.empty()) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Failed determining server or nym ID for "
                    "either asset or currency account.")
                    .Flush();

                return;
            }

            last_attempt_ = api_.OTAPI().issueMarketOffer(
                context_,
                ASSET_ACCT_ID,
                CURRENCY_ACCT_ID,
                (0 == MARKET_SCALE) ? 1 : MARKET_SCALE,
                (0 == MINIMUM_INCREMENT) ? 1 : MINIMUM_INCREMENT,
                (0 == TOTAL_ASSETS_ON_OFFER) ? 1 : TOTAL_ASSETS_ON_OFFER,
                PRICE_LIMIT,
                bBuyingOrSelling,
                tLifespanInSeconds,
                cStopSign,
                ACTIVATION_PRICE);
        } break;
        case ADJUST_USAGE_CREDITS: {
            last_attempt_ =
                api_.OTAPI().usageCredits(context_, targetID_, adjustment_);
        } break;
        case INITIATE_BAILMENT: {
            OT_ASSERT(peer_request_);

            last_attempt_ = api_.OTAPI().initiatePeerRequest(
                context_, targetID_, peer_request_);
        } break;
        case INITIATE_OUTBAILMENT: {
            OT_ASSERT(peer_request_);

            last_attempt_ = api_.OTAPI().initiatePeerRequest(
                context_, targetID_, peer_request_);
        } break;
        case NOTIFY_BAILMENT: {
            OT_ASSERT(peer_request_);

            last_attempt_ = api_.OTAPI().initiatePeerRequest(
                context_, accountID_, peer_request_);
        } break;
        case REQUEST_CONNECTION:
        case STORE_SECRET: {
            OT_ASSERT(peer_request_);

            last_attempt_ = api_.OTAPI().initiatePeerRequest(
                context_, targetID_, peer_request_);
        } break;
        case ACKNOWLEDGE_BAILMENT:
        case ACKNOWLEDGE_OUTBAILMENT:
        case ACKNOWLEDGE_NOTICE:
        case ACKNOWLEDGE_CONNECTION: {
            OT_ASSERT(peer_reply_);

            last_attempt_ = api_.OTAPI().initiatePeerReply(
                context_, accountID_, requestID_, peer_reply_);
        } break;
        case REGISTER_CONTRACT_NYM: {
            last_attempt_ = api_.OTAPI().registerContract(
                context_, ContractType::NYM, targetID_);
        } break;
        case REGISTER_CONTRACT_SERVER: {
            last_attempt_ = api_.OTAPI().registerContract(
                context_, ContractType::SERVER, targetID_);
        } break;
        case REGISTER_CONTRACT_UNIT: {
            last_attempt_ = api_.OTAPI().registerContract(
                context_, ContractType::UNIT, instrumentDefinitionID_);
        } break;
        case REQUEST_ADMIN: {
            last_attempt_ = api_.OTAPI().requestAdmin(context_, password_);
        } break;
        case SERVER_ADD_CLAIM: {
            last_attempt_ = api_.OTAPI().serverAddClaim(
                context_,
                std::to_string(static_cast<std::uint32_t>(sectionName_)),
                std::to_string(static_cast<std::uint32_t>(itemType_)),
                value_,
                isPrimary_);
        } break;
        case GET_TRANSACTION_NUMBERS: {
            last_attempt_ = api_.OTAPI().getTransactionNumbers(context_);
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Error: unhandled function "
                                               "type: ")(type_)(".")
                .Flush();

            OT_FAIL;
        }
    }
}

std::int32_t OTAPI_Func::send()
{
    std::string strLocation{OT_METHOD};
    strLocation += __FUNCTION__;
    strLocation += ": " + type_name_.at(type_);
    run();
    const auto& requestNumber = std::get<0>(last_attempt_);
    const auto& status = std::get<0>(std::get<2>(last_attempt_));

    switch (status) {
        case SendResult::TRANSACTION_NUMBERS:
        case SendResult::INVALID_REPLY:
        case SendResult::TIMEOUT:
        case SendResult::ERROR: {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Failed to send message due to error.")
                .Flush();

            nRequestNum_ = -1;
        } break;
        case SendResult::UNNECESSARY: {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Didn't send this message, but NO error "
                "occurred, either. (For example, a request to process "
                "an "
                "empty Nymbox will return 0, meaning, nothing was sent, "
                "but also no error occurred).")
                .Flush();

            nRequestNum_ = 0;
        } break;
        case SendResult::VALID_REPLY: {
            // BY this point, we definitely have the request number, which
            // means the message was actually SENT. (At least.) This also
            // means we can use nRun later to query for a copy of that sent
            // message (like if we need to clawback the transaction numbers
            // from it later, once we confirm whether the server actually
            // never got it.)

            nRequestNum_ = requestNumber;
        } break;
        default: {
            OT_FAIL
        }
    }

    return nRequestNum_;
}

std::string OTAPI_Func::send_transaction(std::size_t totalRetries)
{
    Utility MsgUtil(context_, api_);
    std::string strLocation =
        "OTAPI_Func::SendTransaction: " + type_name_.at(type_);

    if (!MsgUtil.getIntermediaryFiles(
            context_.Server().str(),
            context_.Nym()->ID().str(),
            accountID_->str(),
            false))  // bForceDownload=false))
    {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": getIntermediaryFiles returned false. (It "
            "couldn't download files that it needed).")
            .Flush();
        return "";
    }

    // GET TRANSACTION NUMBERS HERE IF NECESSARY.
    //
    std::int32_t getnym_trnsnum_count = api_.Exec().GetNym_TransactionNumCount(
        context_.Server().str(), context_.Nym()->ID().str());
    std::int32_t configTxnCount = MsgUtil.getNbrTransactionCount();
    bool b1 = (nTransNumsNeeded_ > configTxnCount);
    std::int32_t comparative = 0;

    if (b1) {
        comparative = nTransNumsNeeded_;
    } else {
        comparative = configTxnCount;
    }

    if (getnym_trnsnum_count < comparative) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": I don't have enough transaction numbers to "
            "perform this transaction.")
            .Flush();
        return "";
    }

    bool bCanRetryAfterThis = false;

    std::string strResult = send_once(true, true, bCanRetryAfterThis);

    if (VerifyStringVal(strResult)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Getting Intermediary files...")
            .Flush();

        if (!MsgUtil.getIntermediaryFiles(
                context_.Server().str(),
                context_.Nym()->ID().str(),
                accountID_->str(),
                true)) {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": getIntermediaryFiles returned false. "
                "(After a success sending the transaction. "
                "Strange...).")
                .Flush();
            return "";
        }

        return strResult;
    }

    //
    // Maybe we have an old Inbox or something.
    //

    // TODO!!  SECURITY:  This is where a GOOD CLIENT (vs. a test client)
    // will verify these std::intermediary files against your LAST SIGNED
    // RECEIPT,
    // using SwigWrap::VerifySomethingorother().
    // See verifyFiles() at the bottom of this file.
    // Add some kind of warning Modal Dialog here, since it's actually
    // normal for a NEW account (never had any receipts yet.) But for
    // any other account, this should ALWAYS VERIFY!
    //
    // Another note: this should happen INSIDE the getIntermediaryFiles call
    // itself,
    // and all similar calls.  You simply should not download those files,
    // without verifying them also. Otherwise you could end up signing
    // a future bad receipt, based on malicious, planted std::intermediary
    // files.

    auto nRetries = totalRetries;

    while ((nRetries > 0) && !VerifyStringVal(strResult) &&
           bCanRetryAfterThis) {
        --nRetries;

        bool bWillRetryAfterThis = true;

        if ((nRetries == 0) || !bCanRetryAfterThis) {
            bWillRetryAfterThis = false;
        }

        if (api_.Exec().CheckConnection(context_.Server().str())) {
            strResult =
                send_once(true, bWillRetryAfterThis, bCanRetryAfterThis);
        }

        // In case of failure, we want to get these before we re-try.
        // But in case of success, we also want to get these, so we can
        // see the results of our success. So we get these either way...
        //
        if (VerifyStringVal(strResult)) {
            if (!MsgUtil.getIntermediaryFiles(
                    context_.Server().str(),
                    context_.Nym()->ID().str(),
                    accountID_->str(),
                    true)) {
                LogNormal(OT_METHOD)(__FUNCTION__)(
                    ": getIntermediaryFiles (loop) returned false even "
                    "after successfully sending the transaction.")
                    .Flush();
                return "";
            }
            break;
        }
    }

    return strResult;
}

std::string OTAPI_Func::send_request()
{
    Utility MsgUtil(context_, api_);
    bool bCanRetryAfterThis{false};
    send_once(false, true, bCanRetryAfterThis);
    const auto& result = std::get<0>(std::get<2>(last_attempt_));
    const bool needRetry = (SendResult::VALID_REPLY != result);

    if (needRetry && bCanRetryAfterThis) {
        if (api_.Exec().CheckConnection(context_.Server().str())) {
            send_once(false, false, bCanRetryAfterThis);
        }
    }

    const auto& reply = std::get<1>(std::get<2>(last_attempt_));

    if (reply) { return String::Factory(*reply)->Get(); }

    return {};
}

std::string OTAPI_Func::send_once(
    const bool bIsTransaction,
    const bool bWillRetryAfterThis,
    bool& bCanRetryAfterThis)
{
    Utility MsgUtil(context_, api_);
    std::string strLocation =
        "OTAPI_Func::SendRequestOnce: " + type_name_.at(type_);
    ;

    bCanRetryAfterThis = false;

    std::string strReply = "";
    std::int32_t nlocalRequestNum = send();

    if ((nlocalRequestNum == -1) || (nlocalRequestNum == 0)) {
        // bCanRetryAfterThis is already false here.
        return "";
    } else  // 1 and default.
    {
        if (nlocalRequestNum < -1) { return ""; }

        const auto& reply = std::get<1>(std::get<2>(last_attempt_));

        OT_ASSERT(reply);

        strReply = String::Factory(*reply)->Get();
    }

    // Below this point, we definitely have a request number.
    // (But not yet necessarily a reply...)
    //

    // nlocalRequestNum is positive and contains the request number from
    // sending.
    //
    // nReplySuccess contains status of the REPLY to that sent message.
    // nReplySuccess contains:
    //   0 == FAILURE reply msg from server,
    //   1 == SUCCESS reply msg from server (transaction could be success or
    // fail.)
    //  -1 == (ERROR)
    //
    // strReply contains the reply itself (or null.)
    //
    std::int32_t nReplySuccess = VerifyMessageSuccess(api_, strReply);

    bool bMsgReplyError = (!VerifyStringVal(strReply) || (nReplySuccess < 0));

    bool bMsgReplySuccess = (!bMsgReplyError && (nReplySuccess > 0));
    bool bMsgReplyFailure = (!bMsgReplyError && (nReplySuccess == 0));

    bool bMsgTransSuccess{false};
    bool bMsgTransFailure{false};

    bool bMsgAnyError{false};
    bool bMsgAnyFailure{false};

    bool bMsgAllSuccess{false};

    // If you EVER are in a situation where you have to harvest numbers
    // back, it will ONLY be for transactions, not normal messages. (Those
    // are the only ones that USE transaction numbers.)
    //
    if (bIsTransaction)  // This request contains a TRANSACTION...
    {
        std::int32_t nTransSuccess{-1};
        std::int32_t nBalanceSuccess{-1};
        if (bMsgReplySuccess)  // If message was success, then let's see if
                               // the transaction was, too.
        {
            nBalanceSuccess = api_.Exec().Message_GetBalanceAgreementSuccess(
                context_.Server().str(),
                context_.Nym()->ID().str(),
                accountID_->str(),
                strReply);

            if (nBalanceSuccess > 0) {
                // Sometimes a transaction is sent that is meant to "fail"
                // in order to cancel itself from ever being run in the
                // future. It's being cancelled. In that case, whether the
                // server reply itself is acknowledged or rejection, either
                // way, IsCancelled() will be set to TRUE. This is used when
                // cancelling a cheque, or a payment plan, or a smart
                // contract, so that it can never be activated at some
                // future time.
                //
                // Therefore when we see that IsCancelled is set to TRUE, we
                // std::interpret it as a "success" as far as the UI is
                // concerned,
                // even though behind the scenes, it is still "rejected" and
                // transaction numbers were harvested from it.
                //
                std::int32_t nTransCancelled =
                    api_.Exec().Message_IsTransactionCanceled(
                        context_.Server().str(),
                        context_.Nym()->ID().str(),
                        accountID_->str(),
                        strReply);

                // If it's not cancelled, then we assume it's a normal
                // transaction (versus a cancellation)
                // and we check for success/failure as normal...
                //
                if (1 != nTransCancelled) {
                    nTransSuccess = api_.Exec().Message_GetTransactionSuccess(
                        context_.Server().str(),
                        context_.Nym()->ID().str(),
                        accountID_->str(),
                        strReply);
                } else  // If it WAS cancelled, then for the UI we say
                        // "Success" even though OT behind the scenes is
                        // harvesting as though it failed.
                {       // (Which is what we want to happen, in the case that a
                    // cancellation was performed.)
                    // This way, the UI won't go off doing a bunch of
                    // unnecessary retries for a "failed" transaction.
                    // (After all, if it was cancelled, then we know for a
                    // fact that all future retries will fail anyway.)
                    //
                    nTransSuccess = 1;
                }
            } else {
                nTransSuccess = -1;
            }
        } else {
            nBalanceSuccess = -1;
            nTransSuccess = -1;
        }
        // All of these booleans (except "error") represent RECEIVED ANSWERS
        // from the server.
        // In other words, "failure" does not mean "failed to find message."
        // Rather, it means "DEFINITELY got a reply, and that reply says
        // status==failure."
        //

        bool bMsgBalanceError =
            (!VerifyStringVal(strReply) || (nBalanceSuccess < 0));
        bool bMsgBalanceSuccess =
            (!bMsgReplyError && !bMsgBalanceError && (nBalanceSuccess > 0));
        bool bMsgBalanceFailure =
            (!bMsgReplyError && !bMsgBalanceError && (nBalanceSuccess == 0));

        bool bMsgTransError =
            (!VerifyStringVal(strReply) || (nTransSuccess < 0));
        bMsgTransSuccess =
            (!bMsgReplyError && !bMsgBalanceError && !bMsgTransError &&
             (nTransSuccess > 0));
        bMsgTransFailure =
            (!bMsgReplyError && !bMsgBalanceError && !bMsgTransError &&
             (nTransSuccess == 0));

        bMsgAnyError = (bMsgReplyError || bMsgBalanceError || bMsgTransError);
        bMsgAnyFailure =
            (bMsgReplyFailure || bMsgBalanceFailure || bMsgTransFailure);

        bMsgAllSuccess =
            (bMsgReplySuccess && bMsgBalanceSuccess && bMsgTransSuccess);

    } else  // it's NOT a transaction, but a normal message..
    {
        bMsgTransSuccess = false;
        bMsgTransFailure = false;

        bMsgAnyError = (bMsgReplyError);
        bMsgAnyFailure = (bMsgReplyFailure);

        bMsgAllSuccess = (bMsgReplySuccess);
    }

    // We know the message SENT. The above logic is about figuring out
    // whether the reply message, the transaction inside it, and the balance
    // agreement inside that transaction, whether any of those three things
    // is a definite error, a definite failure, or a definite success. (Any
    // one of those things could be true, OR NOT, and we can only act as if
    // they are, if we
    // have definitive proof in any of those cases.)
    //
    // The below logic is about what sort of REPLY we may have gotten (if
    // anything.)
    // Without a definite reply we cannot claw back. But the Nymbox can show
    // us this answer, either now, or later...
    //
    if (bMsgAllSuccess) {
        // the Msg was a complete success, including the message
        // AND the transaction AND the balance agreement.
        // Therefore, there's DEFINITELY nothing to clawback.
        //
        // (Thus I RemoveSentMessage for the message, since
        // I'm totally done with it now. NO NEED TO HARVEST anything,
        // either.)
        //
        //          var nRemoved =
        // SwigWrap::RemoveSentMessage(Integer.toString(nlocalRequestNum),
        // String(context_.Server()).Get(),
        // String(context_.Nym()->ID()).Get());
        //
        // NOTE: The above call is unnecessary, since a successful reply
        // means we already received the successful server reply, and OT's
        // "ProcessServerReply"
        // already removed the sent message from the sent buffer (so no need
        // to do that here.)

        //          otOut << strLocation << ", SendRequestOnce():
        // OT_API_RemoveSentMessage: " + nRemoved);

        return strReply;

    }
    // In this case we want to grab the Nymbox and see if the replyNotice is
    // there.
    // If it IS, then OT server DEFINITELY replied to it (the MESSAGE was a
    // success,
    // whether the transaction inside of it was success or fail.) So we know
    // bMsgReplySuccess
    // is true, if we find a replyNotice.
    // From there, we can also check for transaction success.
    //
    else if (bMsgAnyError || bMsgAnyFailure)  // let's resync, and clawback
                                              // whatever transaction
                                              // numbers we might have used
                                              // on the Request...
    {
        bool bWasGetReqSent = false;
        const auto nGetRequestNumber =
            context_.UpdateRequestNumber(bWasGetReqSent);

        // GET REQUEST WAS A SUCCESS.
        //
        if (bWasGetReqSent && (nGetRequestNumber > 0)) {
            bCanRetryAfterThis = true;

            // But--if it was a TRANSACTION, then we're not done syncing
            // yet!
            //
            if (bIsTransaction) {
                bCanRetryAfterThis = false;

                //
                // Maybe we have an old Inbox or something.
                // NEW CODE HERE FOR DEBUGGING (THIS BLOCK)
                //
                bool bForceDownload = true;
                if (!MsgUtil.getIntermediaryFiles(
                        context_.Server().str(),
                        context_.Nym()->ID().str(),
                        accountID_->str(),
                        bForceDownload)) {
                    LogNormal(OT_METHOD)(__FUNCTION__)(
                        ": getIntermediaryFiles returned "
                        "false. (After a failure to send "
                        "the transaction. Thus, I give "
                        "up).")
                        .Flush();
                    return "";
                }

                bool bWasFound = false;
                bool bWasSent = false;

                OTfourbool the_foursome = {bMsgReplySuccess,
                                           bMsgReplyFailure,
                                           bMsgTransSuccess,
                                           bMsgTransFailure};

                bForceDownload = false;

                std::int32_t nProcessNymboxResult =
                    MsgUtil.getAndProcessNymbox_8(
                        context_.Server().str(),
                        context_.Nym()->ID().str(),
                        bWasSent,
                        bForceDownload,
                        nlocalRequestNum,
                        bWasFound,
                        bWillRetryAfterThis,
                        the_foursome);

                // bHarvestingForRetry,// bHarvestingForRetry is INPUT, in
                // the case nlocalRequestNum needs to be harvested before a
                // flush occurs.

                //  bMsgReplySuccess,    // bMsgReplySuccess is INPUT, and
                //  is in
                // case nlocalRequestNum needs to be HARVESTED before a
                // FLUSH happens.
                //  bMsgReplyFailure,    // bMsgReplyFailure is INPUT, and
                //  is in
                // case nlocalRequestNum needs to be HARVESTED before a
                // FLUSH happens.
                //  bMsgTransSuccess,    // bMsgTransSuccess is INPUT, and
                //  is in
                // case nlocalRequestNum needs to be HARVESTED before a
                // FLUSH happens.
                //  bMsgTransFailure)    // Etc.

                // FIX: Add '(' and ')' here to silence warning. But where?
                if ((bWasSent && (1 == nProcessNymboxResult)) ||
                    (!bWasSent && (0 == nProcessNymboxResult)))  // success
                                                                 // processing
                                                                 // Nymbox.
                {
                    bCanRetryAfterThis = true;
                }

                // This means a request number was returned, since the
                // processNymbox failed,
                // and hasn't been properly harvested, so we either need to
                // harvest it for a re-try,
                // or flush it.
                //
                else if (bWasSent && (nProcessNymboxResult > 1)) {
                    std::string strNymbox = api_.Exec().LoadNymboxNoVerify(
                        context_.Server().str(),
                        context_.Nym()->ID().str());  // FLUSH SENT
                                                      // MESSAGES!!!!
                                                      // (AND
                                                      // HARVEST.);

                    if (VerifyStringVal(strNymbox)) {
                        api_.Exec().FlushSentMessages(
                            false,
                            context_.Server().str(),
                            context_.Nym()->ID().str(),
                            strNymbox);
                    }
                }
            }  // if (bIsTransaction)

        }  // if getRequestNumber was success.
        else {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Failure: Never got reply from server, "
                "so tried getRequestNumber, and that failed too. "
                "(I give up).")
                .Flush();

            // Note: cannot harvest transaction nums here because I do NOT
            // know for sure whether the server has replied to the message
            // or not! (Not until I successfully download my Nymbox.)
            // Therefore, do NOT harvest or flush, but hold back and wait
            // until the next attempt does a successful getNymbox and THEN
            // do a "flush sent" after that. (That's the time we'll know for
            // SURE what happened to the original reply.)
            //
            // (Therefore LEAVE the sent message in the sent queue.)

            return "";
        }
    }  // else if (bMsgAnyError || bMsgAnyFailure)

    // Returning an empty std::string.

    return "";
}

const std::shared_ptr<PeerRequest>& OTAPI_Func::SentPeerRequest() const
{
    return peer_request_;
}

const std::shared_ptr<PeerReply>& OTAPI_Func::SentPeerReply() const
{
    return peer_reply_;
}

OTAPI_Func::~OTAPI_Func() {}
}  // namespace opentxs
