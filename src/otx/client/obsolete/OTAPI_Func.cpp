// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "otx/client/obsolete/OTAPI_Func.hpp"  // IWYU pragma: associated

#include <boost/cstdint.hpp>
#include <cstdint>
#include <exception>
#include <map>
#include <string>

#include "core/Amount.hpp"
#include "internal/api/session/Client.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/otx/client/OTPayment.hpp"
#include "internal/otx/client/obsolete/OTAPI_Exec.hpp"
#include "internal/otx/smartcontract/OTSmartContract.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/contract/peer/ConnectionInfoType.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/core/recurring/OTPaymentPlan.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "serialization/protobuf/UnitDefinition.pb.h"

auto VerifyStringVal(const std::string& nValue) -> bool
{
    return 0 < nValue.length();
}

namespace opentxs
{
const std::pmr::map<OTAPI_Func_Type, std::string> OTAPI_Func::type_name_{
    {NO_FUNC, "NO_FUNC"},
    {DELETE_NYM, "DELETE_NYM"},
    {ISSUE_BASKET, "ISSUE_BASKET"},
    {DELETE_ASSET_ACCT, "DELETE_ASSET_ACCT"},
    {ACTIVATE_SMART_CONTRACT, "ACTIVATE_SMART_CONTRACT"},
    {TRIGGER_CLAUSE, "TRIGGER_CLAUSE"},
    {EXCHANGE_BASKET, "EXCHANGE_BASKET"},
    {WITHDRAW_VOUCHER, "WITHDRAW_VOUCHER"},
    {PAY_DIVIDEND, "PAY_DIVIDEND"},
    {GET_MARKET_LIST, "GET_MARKET_LIST"},
    {CREATE_MARKET_OFFER, "CREATE_MARKET_OFFER"},
    {KILL_MARKET_OFFER, "KILL_MARKET_OFFER"},
    {KILL_PAYMENT_PLAN, "KILL_PAYMENT_PLAN"},
    {DEPOSIT_PAYMENT_PLAN, "DEPOSIT_PAYMENT_PLAN"},
    {GET_NYM_MARKET_OFFERS, "GET_NYM_MARKET_OFFERS"},
    {GET_MARKET_OFFERS, "GET_MARKET_OFFERS"},
    {GET_MARKET_RECENT_TRADES, "GET_MARKET_RECENT_TRADES"},
    {ADJUST_USAGE_CREDITS, "ADJUST_USAGE_CREDITS"},
};

const std::pmr::map<OTAPI_Func_Type, bool> OTAPI_Func::type_type_{
    {DELETE_NYM, false},
    {ISSUE_BASKET, false},
    {DELETE_ASSET_ACCT, false},
    {ACTIVATE_SMART_CONTRACT, true},
    {TRIGGER_CLAUSE, false},
    {EXCHANGE_BASKET, true},
    {WITHDRAW_VOUCHER, true},
    {PAY_DIVIDEND, true},
    {GET_MARKET_LIST, false},
    {CREATE_MARKET_OFFER, true},
    {KILL_MARKET_OFFER, true},
    {KILL_PAYMENT_PLAN, true},
    {DEPOSIT_PAYMENT_PLAN, true},
    {GET_NYM_MARKET_OFFERS, false},
    {GET_MARKET_OFFERS, false},
    {GET_MARKET_RECENT_TRADES, false},
    {ADJUST_USAGE_CREDITS, false},
};

OTAPI_Func::OTAPI_Func(
    const PasswordPrompt& reason,
    std::recursive_mutex& apiLock,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const identifier::Server& serverID,
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
    , cheque_(nullptr)
    , ledger_(nullptr)
    , payment_(nullptr)
    , agentName_()
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
    , selling_(false)
    , lifetime_()
    , nRequestNum_(-1)
    , nTransNumsNeeded_(0)
    , api_(api)
    , context_editor_(api_.Wallet().Internal().mutable_ServerContext(
          nymID,
          serverID,
          reason))
    , context_(context_editor_.get())
    , last_attempt_()
    , is_transaction_(type_type_.at(type))
    , activationPrice_(0)
    , adjustment_(0)
    , amount_(0)
    , depth_(0)
    , increment_(0)
    , quantity_(0)
    , price_(0)
    , infoType_(contract::peer::ConnectionInfoType::Error)
    , unitDefinition_()
{
    OT_ASSERT(CheckLock(api_lock_, apiLock));
}

OTAPI_Func::OTAPI_Func(
    const PasswordPrompt& reason,
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const identifier::Server& serverID)
    : OTAPI_Func(reason, apilock, api, nymID, serverID, theType)
{
    if (theType == DELETE_NYM) {
        nTransNumsNeeded_ = 0;
    } else if (theType == GET_MARKET_LIST) {
        nTransNumsNeeded_ = 0;
    } else if (theType != GET_NYM_MARKET_OFFERS) {
        OT_FAIL
    }
}

OTAPI_Func::OTAPI_Func(
    const PasswordPrompt& reason,
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const identifier::Server& serverID,
    const proto::UnitDefinition& unitDefinition,
    const std::string& label)
    : OTAPI_Func(reason, apilock, api, nymID, serverID, theType)
{
    switch (theType) {
        case (ISSUE_BASKET): {
            unitDefinition_ = unitDefinition;
            label_ = label;
        } break;
        default: {
            LogConsole()(OT_PRETTY_CLASS())(
                "ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func().")
                .Flush();
            OT_FAIL
        }
    }
}

OTAPI_Func::OTAPI_Func(
    const PasswordPrompt& reason,
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const identifier::Server& serverID,
    const identifier::Nym& nymID2)
    : OTAPI_Func(reason, apilock, api, nymID, serverID, theType)
{
    switch (theType) {
        case GET_MARKET_RECENT_TRADES: {
            marketID_ = nymID2;
        } break;
        case DELETE_ASSET_ACCT: {
            accountID_ = nymID2;
        } break;
        default: {
            LogConsole()(OT_PRETTY_CLASS())(
                "ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func().")
                .Flush();
            OT_FAIL
        }
    }
}

OTAPI_Func::OTAPI_Func(
    const PasswordPrompt& reason,
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const identifier::Server& serverID,
    const Identifier& recipientID,
    std::unique_ptr<OTPaymentPlan>& paymentPlan)
    : OTAPI_Func(reason, apilock, api, nymID, serverID, theType)
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
            LogConsole()(OT_PRETTY_CLASS())(
                "ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func().")
                .Flush();
            OT_FAIL
        }
    }
}

OTAPI_Func::OTAPI_Func(
    const PasswordPrompt& reason,
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const identifier::Server& serverID,
    const identifier::Nym& nymID2,
    const Amount& int64val)
    : OTAPI_Func(reason, apilock, api, nymID, serverID, theType)
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
            try {
                transactionNumber_ =
                    int64val.Internal().extract_int<std::int64_t>();
            } catch (const std::exception& e) {
                LogConsole()(OT_PRETTY_CLASS())(
                    "Error setting transaction number. ")(e.what())
                    .Flush();
                OT_FAIL
            }
        } break;
        default: {
            LogConsole()(OT_PRETTY_CLASS())(
                "ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func().")
                .Flush();
            OT_FAIL
        }
    }
}

OTAPI_Func::OTAPI_Func(
    const PasswordPrompt& reason,
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const identifier::Server& serverID,
    const TransactionNumber& transactionNumber,
    const std::string& clause,
    const std::string& parameter)
    : OTAPI_Func(reason, apilock, api, nymID, serverID, theType)
{
    std::string strError =
        "Warning: Empty std::string passed to OTAPI_Func.OTAPI_Func() as: ";

    if (!VerifyStringVal(clause)) {
        LogError()(OT_PRETTY_CLASS())("clause.").Flush();
    }

    if (!VerifyStringVal(parameter)) {
        LogError()(OT_PRETTY_CLASS())("parameter.").Flush();
    }

    nTransNumsNeeded_ = 1;

    if (theType == TRIGGER_CLAUSE) {
        transactionNumber_ = transactionNumber;
        clause_ = clause;
        parameter_ = parameter;
    } else {
        LogConsole()(OT_PRETTY_CLASS())(
            "ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func(). "
            "ERROR!!!!!!")
            .Flush();
        OT_FAIL
    }
}

OTAPI_Func::OTAPI_Func(
    const PasswordPrompt& reason,
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const identifier::Server& serverID,
    const Identifier& accountID,
    const std::string& agentName,
    std::unique_ptr<OTSmartContract>& contract)
    : OTAPI_Func(reason, apilock, api, nymID, serverID, theType)
{
    std::string strError =
        "Warning: Empty std::string passed to OTAPI_Func.OTAPI_Func() as: ";

    if (!VerifyStringVal(agentName)) {
        LogError()(OT_PRETTY_CLASS())("agentName.").Flush();
    }

    nTransNumsNeeded_ = 1;

    if (theType == ACTIVATE_SMART_CONTRACT) {

        accountID_ = accountID;  // the "official" asset account of the party
                                 // activating the contract.;
        agentName_ = agentName;  // the agent's name for that party, as listed
                                 // on the contract.;
        contract_.reset(contract.release());  // the smart contract itself.;

        std::int32_t nNumsNeeded =
            api_.InternalClient().Exec().SmartContract_CountNumsNeeded(
                String::Factory(*contract_)->Get(), agentName_);

        if (nNumsNeeded > 0) { nTransNumsNeeded_ = nNumsNeeded; }
    } else {
        LogConsole()(OT_PRETTY_CLASS())(
            "ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func(). "
            "ERROR!!!!!!")
            .Flush();
        OT_FAIL
    }
}

OTAPI_Func::OTAPI_Func(
    const PasswordPrompt& reason,
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const identifier::Server& serverID,
    const identifier::Nym& nymID2,
    const Identifier& targetID,
    const Amount& amount,
    const std::string& message)
    : OTAPI_Func(reason, apilock, api, nymID, serverID, theType)
{
    amount_ = amount;
    nTransNumsNeeded_ = 0;
    message_ = message;

    switch (theType) {
        case PAY_DIVIDEND: {
            accountID_ = targetID;
            instrumentDefinitionID_ = nymID2;
        } break;
        case WITHDRAW_VOUCHER: {
            accountID_ = targetID;
            recipientID_ = nymID2;
        } break;
        default: {
            LogConsole()(OT_PRETTY_CLASS())(
                "ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func(). "
                "ERROR!!!!!!")
                .Flush();
            OT_FAIL
        }
    }
}

OTAPI_Func::OTAPI_Func(
    const PasswordPrompt& reason,
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const identifier::Server& serverID,
    const identifier::UnitDefinition& instrumentDefinitionID,
    const Identifier& basketID,
    const Identifier& accountID,
    bool direction,
    std::int32_t nTransNumsNeeded)
    : OTAPI_Func(reason, apilock, api, nymID, serverID, theType)
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
    const PasswordPrompt& reason,
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const identifier::Server& serverID,
    const Identifier& assetAccountID,
    const Identifier& currencyAccountID,
    const Amount& scale,
    const Amount& increment,
    const std::int64_t& quantity,
    const Amount& price,
    const bool selling,
    const Time lifetime,
    const Amount& activationPrice,
    const std::string& stopSign)
    : OTAPI_Func(reason, apilock, api, nymID, serverID, theType)
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
            LogConsole()(OT_PRETTY_CLASS())(
                "ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func().")
                .Flush();
            OT_FAIL
        }
    }
}

auto OTAPI_Func::Run(const std::size_t) -> std::string
{
    LogConsole()(OT_PRETTY_CLASS())("Not implemented").Flush();

    return {};
}

void OTAPI_Func::run()
{
    /*
        Lock lock(lock_);
        const auto triggerParameter = String::Factory(parameter_);
        auto& [requestNum, transactionNum, result] = last_attempt_;
        auto& [status, reply] = result;
        requestNum = -1;
        transactionNum = 0;
        status = SendResult::ERROR;
        reply.reset();

        switch (type_) {
            case DELETE_NYM: {
                last_attempt_ =
       api_.InternalClient().OTAPI().unregisterNym(context_); } break; case
       GET_NYM_MARKET_OFFERS: { last_attempt_ =
       api_.InternalClient().OTAPI().getNymMarketOffers(context_); } break; case
       DELETE_ASSET_ACCT: { last_attempt_ =
                    api_.InternalClient().OTAPI().deleteAssetAccount(context_,
       accountID_); } break; case ACTIVATE_SMART_CONTRACT: {
                OT_ASSERT(contract_)

                last_attempt_ =
       api_.InternalClient().OTAPI().activateSmartContract( context_,
       String::Factory(*contract_)); } break; case TRIGGER_CLAUSE: {
                last_attempt_ = api_.InternalClient().OTAPI().triggerClause(
                    context_,
                    transactionNumber_,
                    String::Factory(clause_.c_str()),
                    triggerParameter->Exists() ? triggerParameter
                                               : String::Factory());
            } break;
            case EXCHANGE_BASKET: {
                last_attempt_ = api_.InternalClient().OTAPI().exchangeBasket(
                    context_,
                    instrumentDefinitionID_,
                    String::Factory(basketID_),
                    direction_);
            } break;
            case ISSUE_BASKET: {
                last_attempt_ =
                    api_.InternalClient().OTAPI().issueBasket(context_,
       unitDefinition_, label_); } break; case KILL_MARKET_OFFER: {
                last_attempt_ = api_.InternalClient().OTAPI().cancelCronItem(
                    context_, accountID_, transactionNumber_);
            } break;
            case KILL_PAYMENT_PLAN: {
                last_attempt_ = api_.InternalClient().OTAPI().cancelCronItem(
                    context_, accountID_, transactionNumber_);
            } break;
            case DEPOSIT_PAYMENT_PLAN: {
                OT_ASSERT(paymentPlan_)

                last_attempt_ =
       api_.InternalClient().OTAPI().depositPaymentPlan( context_,
       String::Factory(*paymentPlan_)); } break; case WITHDRAW_VOUCHER: {
                last_attempt_ = api_.InternalClient().OTAPI().withdrawVoucher(
                    context_,
                    accountID_,
                    recipientID_,
                    String::Factory(message_.c_str()),
                    amount_);
            } break;
            case PAY_DIVIDEND: {
                last_attempt_ = api_.InternalClient().OTAPI().payDividend(
                    context_,
                    accountID_,
                    instrumentDefinitionID_,
                    String::Factory(message_.c_str()),
                    amount_);
            } break;
            case GET_MARKET_LIST: {
                last_attempt_ =
       api_.InternalClient().OTAPI().getMarketList(context_); } break; case
       GET_MARKET_OFFERS: { last_attempt_ =
                    api_.InternalClient().OTAPI().getMarketOffers(context_,
       marketID_, depth_); } break; case GET_MARKET_RECENT_TRADES: {
                last_attempt_ =
                    api_.InternalClient().OTAPI().getMarketRecentTrades(context_,
       marketID_); } break; case CREATE_MARKET_OFFER: { const auto ASSET_ACCT_ID
       = Identifier::Factory(accountID_); const auto CURRENCY_ACCT_ID =
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
                    LogError()(OT_PRETTY_CLASS())(
"If STOP_SIGN is provided, it must be < "
                        "or >, and in that case ACTIVATION_PRICE "
                        "must be non-zero.")
                        .Flush();

                    return;
                }

                const auto str_asset_notary_id =
                    api_.InternalClient().Exec().GetAccountWallet_NotaryID(accountID_->str());
                const auto str_currency_notary_id =
                    api_.InternalClient().Exec().GetAccountWallet_NotaryID(
                        currencyAccountID_->str());
                const auto str_asset_nym_id =
                    api_.InternalClient().Exec().GetAccountWallet_NymID(accountID_->str());
                const auto str_currency_nym_id =
                    api_.InternalClient().Exec().GetAccountWallet_NymID(currencyAccountID_->str());

                if (str_asset_notary_id.empty() ||
       str_currency_notary_id.empty() || str_asset_nym_id.empty() ||
       str_currency_nym_id.empty()) { LogError()(OT_PRETTY_CLASS())(
"Failed determining server or nym ID for "
                        "either asset or currency account.")
                        .Flush();

                    return;
                }

                last_attempt_ = api_.InternalClient().OTAPI().issueMarketOffer(
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
                    api_.InternalClient().OTAPI().usageCredits(context_,
       targetID_, adjustment_); } break; default: {
                LogError()(OT_PRETTY_CLASS())("Error: unhandled function
       " "type: ")(type_)(".") .Flush();

                OT_FAIL;
            }
        }
    */
}

OTAPI_Func::~OTAPI_Func() = default;
}  // namespace opentxs
