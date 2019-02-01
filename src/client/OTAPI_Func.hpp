// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Forward.hpp"

#include "opentxs/api/Editor.hpp"
#include "opentxs/client/ServerAction.hpp"
#include "opentxs/core/recurring/OTPaymentPlan.hpp"
#include "opentxs/core/script/OTSmartContract.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/ext/OTPayment.hpp"

#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <mutex>

namespace opentxs
{
typedef enum {
    NO_FUNC = 0,
    // TODO
    DELETE_ASSET_ACCT,
    DELETE_NYM,
    ADJUST_USAGE_CREDITS,
    // Vouchers
    WITHDRAW_VOUCHER,
    // Shares
    PAY_DIVIDEND,
    // Payment plans
    KILL_PAYMENT_PLAN,
    DEPOSIT_PAYMENT_PLAN,
    // Basket currencies
    ISSUE_BASKET,
    EXCHANGE_BASKET,
    // Markets
    GET_MARKET_LIST,
    CREATE_MARKET_OFFER,
    KILL_MARKET_OFFER,
    GET_NYM_MARKET_OFFERS,
    GET_MARKET_OFFERS,
    GET_MARKET_RECENT_TRADES,
    // Smart contracts
    ACTIVATE_SMART_CONTRACT,
    TRIGGER_CLAUSE,
} OTAPI_Func_Type;

class OTAPI_Func : virtual public opentxs::client::ServerAction, Lockable
{
public:
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Manager& api,
        const identifier::Nym& nymID,
        const Identifier& serverID);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Manager& api,
        const identifier::Nym& nymID,
        const Identifier& serverID,
        const proto::UnitDefinition& unitDefinition,
        const std::string& label);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Manager& api,
        const identifier::Nym& nymID,
        const Identifier& serverID,
        const identifier::Nym& nymID2);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Manager& api,
        const identifier::Nym& nymID,
        const Identifier& serverID,
        const identifier::Nym& nymID2,
        const std::int64_t& int64Val);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Manager& api,
        const identifier::Nym& nymID,
        const Identifier& serverID,
        const Identifier& recipientID,
        std::unique_ptr<OTPaymentPlan>& paymentPlan);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Manager& api,
        const identifier::Nym& nymID,
        const Identifier& serverID,
        const TransactionNumber& transactionNumber,
        const std::string& clause,
        const std::string& parameter);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Manager& api,
        const identifier::Nym& nymID,
        const Identifier& serverID,
        const Identifier& accountID,
        const std::string& agentName,
        std::unique_ptr<OTSmartContract>& contract);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Manager& api,
        const identifier::Nym& nymID,
        const Identifier& serverID,
        const Identifier& recipientID,
        const Identifier& requestID,
        const bool ack);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Manager& api,
        const identifier::Nym& nymID,
        const Identifier& serverID,
        const identifier::Nym& nymID2,
        const Identifier& targetID,
        const Amount& amount,
        const std::string& message);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Manager& api,
        const identifier::Nym& nymID,
        const Identifier& serverID,
        const Identifier& targetID,
        const std::string& primary,
        const std::string& secondary,
        const proto::SecretType& secretType);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Manager& api,
        const identifier::Nym& nymID,
        const Identifier& serverID,
        const Identifier& recipientID,
        const Identifier& requestID,
        const Identifier& instrumentDefinitionID,
        const std::string& txid,
        const Amount& amount);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Manager& api,
        const identifier::Nym& nymID,
        const Identifier& serverID,
        const Identifier& instrumentDefinitionID,
        const Identifier& basketID,
        const Identifier& accountID,
        bool direction,
        std::int32_t nTransNumsNeeded);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Manager& api,
        const identifier::Nym& nymID,
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
        const std::string& stopSign);

    SendResult LastSendResult() const override { return {}; }
    const std::shared_ptr<Message> Reply() const override { return {}; }
    const std::shared_ptr<PeerReply> SentPeerReply() const override
    {
        return {};
    }
    const std::shared_ptr<PeerRequest> SentPeerRequest() const override
    {
        return {};
    }

    std::string Run(const std::size_t totalRetries = 2) override;

    ~OTAPI_Func();

private:
    static const std::map<OTAPI_Func_Type, std::string> type_name_;
    static const std::map<OTAPI_Func_Type, bool> type_type_;

    OTAPI_Func_Type type_{NO_FUNC};
    rLock api_lock_;
    OTIdentifier accountID_;
    OTIdentifier basketID_;
    OTIdentifier currencyAccountID_;
    OTIdentifier instrumentDefinitionID_;
    OTIdentifier marketID_;
    OTIdentifier recipientID_;
    OTIdentifier requestID_;
    OTIdentifier targetID_;
    OTIdentifier message_id_;
    std::unique_ptr<Message> request_;
    std::unique_ptr<OTSmartContract> contract_;
    std::unique_ptr<OTPaymentPlan> paymentPlan_;
    std::unique_ptr<Cheque> cheque_;
    std::unique_ptr<Ledger> ledger_;
    std::unique_ptr<const OTPayment> payment_;
    std::string agentName_;
    std::string clause_;
    std::string key_;
    std::string login_;
    std::string message_;
    std::string parameter_;
    std::string password_;
    std::string primary_;
    std::string secondary_;
    std::string stopSign_;
    std::string txid_;
    std::string url_;
    std::string value_;
    std::string label_;
    bool ack_{false};
    bool direction_{false};
    bool selling_{false};
    time64_t lifetime_{OT_TIME_ZERO};
    std::int32_t nRequestNum_{-1};
    std::int32_t nTransNumsNeeded_{0};
    const api::client::Manager& api_;
    Editor<ServerContext> context_editor_;
    ServerContext& context_;
    CommandResult last_attempt_;
    const bool is_transaction_{false};
    std::shared_ptr<PeerReply> peer_reply_;
    std::shared_ptr<PeerRequest> peer_request_;
    Amount activationPrice_{0};
    Amount adjustment_{0};
    Amount amount_{0};
    Amount depth_{0};
    Amount increment_{0};
    Amount quantity_{0};
    Amount price_{0};
    Amount scale_{0};
    TransactionNumber transactionNumber_{0};  // This is not what gets returned
                                              // by GetTransactionNumber.
    proto::ConnectionInfoType infoType_{proto::CONNECTIONINFO_ERROR};
    proto::SecretType secretType_{proto::SECRETTYPE_ERROR};
    proto::UnitDefinition unitDefinition_{};

    void run();
    std::string send_once(
        const bool bIsTransaction,
        const bool bWillRetryAfterThis,
        bool& bCanRetryAfterThis);

    explicit OTAPI_Func(
        std::recursive_mutex& apilock,
        const api::client::Manager& api,
        const identifier::Nym& nymID,
        const Identifier& serverID,
        const OTAPI_Func_Type type);
    OTAPI_Func() = delete;
};
}  // namespace opentxs
