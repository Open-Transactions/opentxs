// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "otx/client/obsolete/OTAPI_Func.hpp"  // IWYU pragma: associated

#include <UnitDefinition.pb.h>
#include <cstdint>
#include <exception>
#include <utility>

#include "internal/core/Amount.hpp"
#include "internal/core/String.hpp"
#include "internal/otx/client/OTPayment.hpp"
#include "internal/otx/client/obsolete/OTAPI_Exec.hpp"
#include "internal/otx/common/Cheque.hpp"
#include "internal/otx/common/Ledger.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/otx/common/recurring/OTPaymentPlan.hpp"
#include "internal/otx/smartcontract/OTSmartContract.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/Session.internal.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Client.internal.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/api/session/Wallet.internal.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/AccountSubtype.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Types.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
auto VerifyStringVal(const UnallocatedCString& nValue) -> bool
{
    return 0 < nValue.length();
}

const UnallocatedMap<OTAPI_Func_Type, UnallocatedCString>
    OTAPI_Func::type_name_{
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

const UnallocatedMap<OTAPI_Func_Type, bool> OTAPI_Func::type_type_{
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
    const identifier::Notary& serverID,
    const OTAPI_Func_Type type)
    : type_(type)
    , api_lock_(apiLock)
    , account_id_()
    , basket_id_()
    , currency_account_id_()
    , instrument_definition_id_()
    , market_id_()
    , recipient_id_()
    , request_id_()
    , target_id_()
    , message_id_()
    , request_(nullptr)
    , contract_(nullptr)
    , payment_plan_(nullptr)
    , cheque_(nullptr)
    , ledger_(nullptr)
    , payment_(nullptr)
    , agent_name_()
    , clause_("")
    , key_("")
    , login_("")
    , message_("")
    , parameter_("")
    , password_("")
    , primary_("")
    , secondary_("")
    , stop_sign_("")
    , txid_("")
    , url_("")
    , value_("")
    , label_("")
    , ack_(false)
    , direction_(false)
    , selling_(false)
    , lifetime_()
    , request_num_(-1)
    , trans_nums_needed_(0)
    , api_(api)
    , context_editor_(api_.Wallet().Internal().mutable_ServerContext(
          nymID,
          serverID,
          reason))
    , context_(context_editor_.get())
    , last_attempt_()
    , is_transaction_(type_type_.at(type))
    , activation_price_(0)
    , adjustment_(0)
    , amount_(0)
    , depth_(0)
    , increment_(0)
    , quantity_(0)
    , price_(0)
    , info_type_(contract::peer::ConnectionInfoType::Error)
    , unit_definition_()
{
    assert_true(CheckLock(api_lock_, apiLock));
}

OTAPI_Func::OTAPI_Func(
    const PasswordPrompt& reason,
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const identifier::Notary& serverID)
    : OTAPI_Func(reason, apilock, api, nymID, serverID, theType)
{
    if (theType == DELETE_NYM) {
        trans_nums_needed_ = 0;
    } else if (theType == GET_MARKET_LIST) {
        trans_nums_needed_ = 0;
    } else if (theType != GET_NYM_MARKET_OFFERS) {
        LogAbort()().Abort();
    }
}

OTAPI_Func::OTAPI_Func(
    const PasswordPrompt& reason,
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const identifier::Notary& serverID,
    const proto::UnitDefinition& unitDefinition,
    const UnallocatedCString& label)
    : OTAPI_Func(reason, apilock, api, nymID, serverID, theType)
{
    switch (theType) {
        case (ISSUE_BASKET): {
            unit_definition_ = unitDefinition;
            label_ = label;
        } break;
        default: {
            LogConsole()()(
                "ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func().")
                .Flush();
            LogAbort()().Abort();
        }
    }
}

OTAPI_Func::OTAPI_Func(
    const PasswordPrompt& reason,
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const identifier::Notary& serverID,
    const identifier::Nym& nymID2)
    : OTAPI_Func(reason, apilock, api, nymID, serverID, theType)
{
    switch (theType) {
        case GET_MARKET_RECENT_TRADES: {
            market_id_ = nymID2;
        } break;
        case DELETE_ASSET_ACCT: {
            account_id_ = nym_to_account(nymID2);
        } break;
        default: {
            LogConsole()()(
                "ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func().")
                .Flush();
            LogAbort()().Abort();
        }
    }
}

OTAPI_Func::OTAPI_Func(
    const PasswordPrompt& reason,
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const identifier::Notary& serverID,
    const identifier::Account& recipientID,
    std::unique_ptr<OTPaymentPlan>& paymentPlan)
    : OTAPI_Func(reason, apilock, api, nymID, serverID, theType)
{
    const UnallocatedCString strError =
        "Warning: Empty UnallocatedCString passed to "
        "OTAPI_Func.OTAPI_Func() as: ";

    switch (theType) {
        case DEPOSIT_PAYMENT_PLAN: {
            trans_nums_needed_ = 1;
            account_id_ = recipientID;
            payment_plan_ = std::move(paymentPlan);
        } break;
        default: {
            LogConsole()()(
                "ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func().")
                .Flush();
            LogAbort()().Abort();
        }
    }
}

OTAPI_Func::OTAPI_Func(
    const PasswordPrompt& reason,
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const identifier::Notary& serverID,
    const identifier::Nym& nymID2,
    const Amount& int64val)
    : OTAPI_Func(reason, apilock, api, nymID, serverID, theType)
{
    switch (theType) {
        case ADJUST_USAGE_CREDITS: {
            target_id_ = nymID2;     // target nym ID
            adjustment_ = int64val;  // adjustment (up or down.)
        } break;
        case GET_MARKET_OFFERS: {
            market_id_ = nymID2;
            depth_ = int64val;
        } break;
        case KILL_PAYMENT_PLAN:
        case KILL_MARKET_OFFER: {
            trans_nums_needed_ = 1;
            account_id_ = nym_to_account(nymID2);
            try {
                transaction_number_ = int64val.Internal().ExtractInt64();
            } catch (const std::exception& e) {
                LogConsole()()("Error setting transaction number. ")(e.what())
                    .Flush();
                LogAbort()().Abort();
            }
        } break;
        default: {
            LogConsole()()(
                "ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func().")
                .Flush();
            LogAbort()().Abort();
        }
    }
}

OTAPI_Func::OTAPI_Func(
    const PasswordPrompt& reason,
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const identifier::Notary& serverID,
    const TransactionNumber& transactionNumber,
    const UnallocatedCString& clause,
    const UnallocatedCString& parameter)
    : OTAPI_Func(reason, apilock, api, nymID, serverID, theType)
{
    const UnallocatedCString strError =
        "Warning: Empty UnallocatedCString passed to "
        "OTAPI_Func.OTAPI_Func() as: ";

    if (!VerifyStringVal(clause)) { LogError()()("clause.").Flush(); }

    if (!VerifyStringVal(parameter)) { LogError()()("parameter.").Flush(); }

    trans_nums_needed_ = 1;

    if (theType == TRIGGER_CLAUSE) {
        transaction_number_ = transactionNumber;
        clause_ = clause;
        parameter_ = parameter;
    } else {
        LogConsole()()("ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func(). "
                       "ERROR!!!!!!")
            .Flush();
        LogAbort()().Abort();
    }
}

OTAPI_Func::OTAPI_Func(
    const PasswordPrompt& reason,
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const identifier::Notary& serverID,
    const identifier::Account& accountID,
    const UnallocatedCString& agentName,
    std::unique_ptr<OTSmartContract>& contract)
    : OTAPI_Func(reason, apilock, api, nymID, serverID, theType)
{
    const UnallocatedCString strError =
        "Warning: Empty UnallocatedCString passed to "
        "OTAPI_Func.OTAPI_Func() as: ";

    if (!VerifyStringVal(agentName)) { LogError()()("agentName.").Flush(); }

    trans_nums_needed_ = 1;

    if (theType == ACTIVATE_SMART_CONTRACT) {

        account_id_ = accountID;  // the "official" asset account of the party
                                  // activating the contract.;
        agent_name_ = agentName;  // the agent's name for that party, as listed
                                  // on the contract.;
        contract_ = std::move(contract);  // the smart contract itself.;

        const std::int32_t nNumsNeeded =
            api_.Internal().asClient().Exec().SmartContract_CountNumsNeeded(
                String::Factory(*contract_)->Get(), agent_name_);

        if (nNumsNeeded > 0) { trans_nums_needed_ = nNumsNeeded; }
    } else {
        LogConsole()()("ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func(). "
                       "ERROR!!!!!!")
            .Flush();
        LogAbort()().Abort();
    }
}

OTAPI_Func::OTAPI_Func(
    const PasswordPrompt& reason,
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const identifier::Notary& serverID,
    const identifier::Nym& nymID2,
    const identifier::Account& targetID,
    const Amount& amount,
    const UnallocatedCString& message)
    : OTAPI_Func(reason, apilock, api, nymID, serverID, theType)
{
    amount_ = amount;
    trans_nums_needed_ = 0;
    message_ = message;

    switch (theType) {
        case PAY_DIVIDEND: {
            account_id_ = targetID;
            instrument_definition_id_ = nymID2;
        } break;
        case WITHDRAW_VOUCHER: {
            account_id_ = targetID;
            recipient_id_ = nymID2;
        } break;
        default: {
            LogConsole()()(
                "ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func(). "
                "ERROR!!!!!!")
                .Flush();
            LogAbort()().Abort();
        }
    }
}

OTAPI_Func::OTAPI_Func(
    const PasswordPrompt& reason,
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const identifier::Notary& serverID,
    const identifier::UnitDefinition& instrumentDefinitionID,
    const identifier::Generic& basketID,
    const identifier::Account& accountID,
    bool direction,
    std::int32_t nTransNumsNeeded)
    : OTAPI_Func(reason, apilock, api, nymID, serverID, theType)
{
    if (EXCHANGE_BASKET == theType) {
        // FYI. This is a transaction.
        trans_nums_needed_ = nTransNumsNeeded;
        direction_ = direction;
        instrument_definition_id_ = instrumentDefinitionID;
        basket_id_ = basketID;
        account_id_ = accountID;
    } else {
        LogAbort()().Abort();
    }
}

OTAPI_Func::OTAPI_Func(
    const PasswordPrompt& reason,
    OTAPI_Func_Type theType,
    std::recursive_mutex& apilock,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const identifier::Notary& serverID,
    const identifier::Account& assetAccountID,
    const identifier::Account& currencyAccountID,
    const Amount& scale,
    const Amount& increment,
    const std::int64_t& quantity,
    const Amount& price,
    const bool selling,
    const Time lifetime,
    const Amount& activationPrice,
    const UnallocatedCString& stopSign)
    : OTAPI_Func(reason, apilock, api, nymID, serverID, theType)
{
    if (VerifyStringVal(stopSign)) { stop_sign_ = stopSign; }

    switch (theType) {
        case CREATE_MARKET_OFFER: {
            trans_nums_needed_ = 3;
            account_id_ = assetAccountID;
            currency_account_id_ = currencyAccountID;
            scale_ = scale;
            increment_ = increment;
            quantity_ = quantity;
            price_ = price;
            selling_ = selling;
            lifetime_ = lifetime;
            activation_price_ = activationPrice;
        } break;
        default: {
            LogConsole()()(
                "ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func().")
                .Flush();
            LogAbort()().Abort();
        }
    }
}

auto OTAPI_Func::nym_to_account(const identifier::Nym& id) const noexcept
    -> identifier::Account
{
    return api_.Factory().AccountIDFromHash(
        id.Bytes(), identifier::AccountSubtype::custodial_account);
}

auto OTAPI_Func::Run(const std::size_t) -> UnallocatedCString
{
    LogConsole()()("Not implemented").Flush();

    return {};
}

void OTAPI_Func::run()
{
    /*
        auto lock = Lock{lock_};
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
       api_.Internal().asClient().OTAPI().unregisterNym(context_); } break; case
       GET_NYM_MARKET_OFFERS: { last_attempt_ =
       api_.Internal().asClient().OTAPI().getNymMarketOffers(context_); } break;
case DELETE_ASSET_ACCT: { last_attempt_ =
                    api_.Internal().asClient().OTAPI().deleteAssetAccount(context_,
       account_id_); } break; case ACTIVATE_SMART_CONTRACT: {
                assert_true(contract_);

                last_attempt_ =
       api_.Internal().asClient().OTAPI().activateSmartContract( context_,
       String::Factory(*contract_)); } break; case TRIGGER_CLAUSE: {
                last_attempt_ =
api_.Internal().asClient().OTAPI().triggerClause( context_, transaction_number_,
                    String::Factory(clause_.c_str()),
                    triggerParameter->Exists() ? triggerParameter
                                               : String::Factory());
            } break;
            case EXCHANGE_BASKET: {
                last_attempt_ =
api_.Internal().asClient().OTAPI().exchangeBasket( context_,
                    instrument_definition_id_,
                    String::Factory(basket_id_),
                    direction_);
            } break;
            case ISSUE_BASKET: {
                last_attempt_ =
                    api_.Internal().asClient().OTAPI().issueBasket(context_,
       unit_definition_, label_); } break; case KILL_MARKET_OFFER: {
                last_attempt_ =
api_.Internal().asClient().OTAPI().cancelCronItem( context_, account_id_,
transaction_number_); } break; case KILL_PAYMENT_PLAN: { last_attempt_ =
api_.Internal().asClient().OTAPI().cancelCronItem( context_, account_id_,
transaction_number_); } break; case DEPOSIT_PAYMENT_PLAN: {
                assert_true(payment_plan_);

                last_attempt_ =
       api_.Internal().asClient().OTAPI().depositPaymentPlan( context_,
       String::Factory(*payment_plan_)); } break; case WITHDRAW_VOUCHER: {
                last_attempt_ =
api_.Internal().asClient().OTAPI().withdrawVoucher( context_, account_id_,
                    recipient_id_,
                    String::Factory(message_.c_str()),
                    amount_);
            } break;
            case PAY_DIVIDEND: {
                last_attempt_ = api_.Internal().asClient().OTAPI().payDividend(
                    context_,
                    account_id_,
                    instrument_definition_id_,
                    String::Factory(message_.c_str()),
                    amount_);
            } break;
            case GET_MARKET_LIST: {
                last_attempt_ =
       api_.Internal().asClient().OTAPI().getMarketList(context_); } break; case
       GET_MARKET_OFFERS: { last_attempt_ =
                    api_.Internal().asClient().OTAPI().getMarketOffers(context_,
       market_id_, depth_); } break; case GET_MARKET_RECENT_TRADES: {
                last_attempt_ =
                    api_.Internal().asClient().OTAPI().getMarketRecentTrades(context_,
       market_id_); } break; case CREATE_MARKET_OFFER: { const auto
ASSET_ACCT_ID = identifier::Generic::Factory(account_id_); const auto
CURRENCY_ACCT_ID = identifier::Generic::Factory(currency_account_id_); const
std::int64_t MARKET_SCALE = scale_; const std::int64_t MINIMUM_INCREMENT =
increment_; const std::int64_t TOTAL_ASSETS_ON_OFFER = quantity_; const Amount
PRICE_LIMIT = price_; const auto& bBuyingOrSelling = selling_; const auto&
tLifespanInSeconds = lifetime_; const auto& STOP_SIGN = stop_sign_; const auto&
ACTIVATION_PRICE = activation_price_; char cStopSign = 0;

                if (0 == STOP_SIGN.compare("<")) {
                    cStopSign = '<';
                } else if (0 == STOP_SIGN.compare(">")) {
                    cStopSign = '>';
                }

                if (!STOP_SIGN.empty() &&
                    ((ACTIVATION_PRICE == 0) ||
                     ((cStopSign != '<') && (cStopSign != '>')))) {
                    LogError()()(
"If STOP_SIGN is provided, it must be < "
                        "or >, and in that case ACTIVATION_PRICE "
                        "must be non-zero.")
                        .Flush();

                    return;
                }

                const auto str_asset_notary_id =
                    api_.Internal().asClient().Exec().GetAccountWallet_NotaryID(account_id_->str());
                const auto str_currency_notary_id =
                    api_.Internal().asClient().Exec().GetAccountWallet_NotaryID(
                        currency_account_id_->str());
                const auto str_asset_nym_id =
                    api_.Internal().asClient().Exec().GetAccountWallet_NymID(account_id_->str());
                const auto str_currency_nym_id =
                    api_.Internal().asClient().Exec().GetAccountWallet_NymID(currency_account_id_->str());

                if (str_asset_notary_id.empty() ||
       str_currency_notary_id.empty() || str_asset_nym_id.empty() ||
       str_currency_nym_id.empty()) { LogError()()(
"Failed determining server or nym ID for "
                        "either asset or currency account.")
                        .Flush();

                    return;
                }

                last_attempt_ =
api_.Internal().asClient().OTAPI().issueMarketOffer( context_, ASSET_ACCT_ID,
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
                    api_.Internal().asClient().OTAPI().usageCredits(context_,
       target_id_, adjustment_); } break; default: {
                LogError()()("Error: unhandled function
       " "type: ")(type_)(".") .Flush();

                LogAbort()().Abort();
            }
        }
    */
}

OTAPI_Func::~OTAPI_Func() = default;
}  // namespace opentxs
