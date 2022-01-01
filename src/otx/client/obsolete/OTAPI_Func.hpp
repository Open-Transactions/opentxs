// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "Proto.hpp"
#include "internal/otx/client/OTPayment.hpp"
#include "internal/otx/client/obsolete/ServerAction.hpp"
#include "internal/otx/smartcontract/OTSmartContract.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Editor.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/contract/peer/ConnectionInfoType.hpp"
#include "opentxs/core/contract/peer/SecretType.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/recurring/OTPaymentPlan.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"
#include "serialization/protobuf/UnitDefinition.pb.h"

namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api

namespace identifier
{
class Nym;
class Server;
class UnitDefinition;
}  // namespace identifier

namespace otx
{
namespace context
{
class Server;
}  // namespace context
}  // namespace otx

class Cheque;
class Ledger;
class Message;
class OTPayment;
class OTPaymentPlan;
class OTSmartContract;
class PasswordPrompt;
}  // namespace opentxs

auto VerifyStringVal(const std::string&) -> bool;

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

class OTAPI_Func final : virtual public opentxs::client::ServerAction, Lockable
{
public:
    explicit OTAPI_Func(
        const PasswordPrompt& reason,
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const identifier::Server& serverID);
    explicit OTAPI_Func(
        const PasswordPrompt& reason,
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const identifier::Server& serverID,
        const proto::UnitDefinition& unitDefinition,
        const std::string& label);
    explicit OTAPI_Func(
        const PasswordPrompt& reason,
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const identifier::Server& serverID,
        const identifier::Nym& nymID2);
    explicit OTAPI_Func(
        const PasswordPrompt& reason,
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const identifier::Server& serverID,
        const identifier::Nym& nymID2,
        const Amount& int64Val);
    explicit OTAPI_Func(
        const PasswordPrompt& reason,
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const identifier::Server& serverID,
        const Identifier& recipientID,
        std::unique_ptr<OTPaymentPlan>& paymentPlan);
    explicit OTAPI_Func(
        const PasswordPrompt& reason,
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const identifier::Server& serverID,
        const TransactionNumber& transactionNumber,
        const std::string& clause,
        const std::string& parameter);
    explicit OTAPI_Func(
        const PasswordPrompt& reason,
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const identifier::Server& serverID,
        const Identifier& accountID,
        const std::string& agentName,
        std::unique_ptr<OTSmartContract>& contract);
    explicit OTAPI_Func(
        const PasswordPrompt& reason,
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const identifier::Server& serverID,
        const Identifier& recipientID,
        const Identifier& requestID,
        const bool ack);
    explicit OTAPI_Func(
        const PasswordPrompt& reason,
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const identifier::Server& serverID,
        const identifier::Nym& nymID2,
        const Identifier& targetID,
        const Amount& amount,
        const std::string& message);
    explicit OTAPI_Func(
        const PasswordPrompt& reason,
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const identifier::Server& serverID,
        const Identifier& targetID,
        const std::string& primary,
        const std::string& secondary,
        const contract::peer::SecretType& secretType);
    explicit OTAPI_Func(
        const PasswordPrompt& reason,
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const identifier::Server& serverID,
        const Identifier& recipientID,
        const Identifier& requestID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const std::string& txid,
        const Amount& amount);
    explicit OTAPI_Func(
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
        std::int32_t nTransNumsNeeded);
    explicit OTAPI_Func(
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
        const std::string& stopSign);

    auto LastSendResult() const -> SendResult final { return {}; }
    auto Reply() const -> const std::shared_ptr<Message> final { return {}; }

    auto Run(const std::size_t totalRetries = 2) -> std::string final;

    ~OTAPI_Func() final;

private:
    static const std::pmr::map<OTAPI_Func_Type, std::string> type_name_;
    static const std::pmr::map<OTAPI_Func_Type, bool> type_type_;

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
    Time lifetime_{};
    std::int32_t nRequestNum_{-1};
    std::int32_t nTransNumsNeeded_{0};
    const api::session::Client& api_;
    Editor<otx::context::Server> context_editor_;
    otx::context::Server& context_;
    CommandResult last_attempt_;
    const bool is_transaction_{false};
    Amount activationPrice_{0};
    Amount adjustment_{0};
    Amount amount_{0};
    Amount depth_{0};
    Amount increment_{0};
    std::int64_t quantity_{0};
    Amount price_{0};
    Amount scale_{0};
    TransactionNumber transactionNumber_{0};  // This is not what gets returned
                                              // by GetTransactionNumber.
    contract::peer::ConnectionInfoType infoType_{
        contract::peer::ConnectionInfoType::Error};
    contract::peer::SecretType secretType_{contract::peer::SecretType::Error};
    proto::UnitDefinition unitDefinition_{};

    void run();
    auto send_once(
        const bool bIsTransaction,
        const bool bWillRetryAfterThis,
        bool& bCanRetryAfterThis) -> std::string;

    explicit OTAPI_Func(
        const PasswordPrompt& reason,
        std::recursive_mutex& apilock,
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const identifier::Server& serverID,
        const OTAPI_Func_Type type);
    OTAPI_Func() = delete;
};
}  // namespace opentxs
