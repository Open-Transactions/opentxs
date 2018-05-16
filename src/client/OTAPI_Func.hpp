/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#ifndef OPENTXS_CLIENT_OT_OTAPI_OT_HPP
#define OPENTXS_CLIENT_OT_OTAPI_OT_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/api/Editor.hpp"
#include "opentxs/cash/Purse.hpp"
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
    REMOVED1 = 1,
    REGISTER_NYM = 2,
    DELETE_NYM = 3,
    CHECK_NYM = 4,
    SEND_USER_MESSAGE = 5,
    SEND_USER_INSTRUMENT = 6,
    ISSUE_ASSET_TYPE = 7,
    ISSUE_BASKET = 8,
    CREATE_ASSET_ACCT = 9,
    DELETE_ASSET_ACCT = 10,
    ACTIVATE_SMART_CONTRACT = 11,
    TRIGGER_CLAUSE = 12,
    PROCESS_INBOX = 13,
    EXCHANGE_BASKET = 14,
    DEPOSIT_CASH = 15,
    EXCHANGE_CASH = 16,
    DEPOSIT_CHEQUE = 17,
    WITHDRAW_VOUCHER = 18,
    PAY_DIVIDEND = 19,
    WITHDRAW_CASH = 20,
    GET_CONTRACT = 21,
    SEND_TRANSFER = 22,
    GET_MARKET_LIST = 23,
    CREATE_MARKET_OFFER = 24,
    KILL_MARKET_OFFER = 25,
    KILL_PAYMENT_PLAN = 26,
    DEPOSIT_PAYMENT_PLAN = 27,
    GET_NYM_MARKET_OFFERS = 28,
    GET_MARKET_OFFERS = 29,
    GET_MARKET_RECENT_TRADES = 30,
    GET_MINT = 31,
    GET_BOX_RECEIPT = 32,
    ADJUST_USAGE_CREDITS = 33,
    INITIATE_BAILMENT = 34,
    INITIATE_OUTBAILMENT = 35,
    ACKNOWLEDGE_BAILMENT = 36,
    ACKNOWLEDGE_OUTBAILMENT = 37,
    NOTIFY_BAILMENT = 38,
    ACKNOWLEDGE_NOTICE = 39,
    REQUEST_CONNECTION = 40,
    ACKNOWLEDGE_CONNECTION = 41,
    REGISTER_CONTRACT_NYM = 42,
    REGISTER_CONTRACT_SERVER = 43,
    REGISTER_CONTRACT_UNIT = 44,
    REQUEST_ADMIN = 45,
    SERVER_ADD_CLAIM = 46,
    STORE_SECRET = 47,
    GET_TRANSACTION_NUMBERS = 48
} OTAPI_Func_Type;

class OTAPI_Func : virtual public opentxs::client::ServerAction, Lockable
{
public:
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        const std::string& password);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        const proto::UnitDefinition& unitDefinition);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        const Identifier& nymID2);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        const Identifier& targetID,
        const proto::ConnectionInfoType& infoType);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        const Identifier& nymID2,
        const std::int64_t& int64Val);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        const Identifier& accountID,
        std::unique_ptr<Ledger>& ledger);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        const Identifier& targetID,
        const Identifier& instrumentDefinitionID);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        const Identifier& accountID,
        std::unique_ptr<Cheque>& cheque);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        const Identifier& nymID2,
        std::unique_ptr<Purse>& purse);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        const Identifier& recipientID,
        std::unique_ptr<OTPaymentPlan>& paymentPlan);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        const TransactionNumber& transactionNumber,
        const std::string& clause,
        const std::string& parameter);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        const Identifier& recipientID,
        std::unique_ptr<const OTPayment>& payment);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        const Identifier& recipientID,
        const std::string& message);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        const Identifier& accountID,
        const RemoteBoxType& remoteBoxType,
        const TransactionNumber& transactionNumber);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        const Identifier& accountID,
        const std::string& agentName,
        std::unique_ptr<OTSmartContract>& contract);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        const Identifier& recipientID,
        const Identifier& requestID,
        const std::string& instructions);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        const Identifier& recipientID,
        const Identifier& requestID,
        const bool ack);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        const Identifier& nymID2,
        const Identifier& targetID,
        const Amount& amount,
        const std::string& message);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        const Identifier& targetID,
        const std::string& primary,
        const std::string& secondary,
        const proto::SecretType& secretType);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        const Identifier& recipientID,
        std::unique_ptr<const Purse>& purse,
        std::unique_ptr<const Purse>& senderPurse);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        const Identifier& recipientID,
        const Identifier& requestID,
        const Identifier& instrumentDefinitionID,
        const std::string& txid,
        const Amount& amount);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        bool primary,
        const proto::ContactSectionName& sectionName,
        const proto::ContactItemType& itemType,
        const std::string& value);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        const Identifier& instrumentDefinitionID,
        const Identifier& basketID,
        const Identifier& accountID,
        bool direction,
        std::int32_t nTransNumsNeeded);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        const Identifier& recipientID,
        const Identifier& requestID,
        const std::string& url,
        const std::string& login,
        const std::string& password,
        const std::string& key,
        bool ack);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
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

    TransactionNumber GetTransactionNumber() const override;
    SendResult LastSendResult() const override;
    const Identifier MessageID() const override;
    const std::shared_ptr<PeerRequest>& SentPeerRequest() const override;
    const std::shared_ptr<PeerReply>& SentPeerReply() const override;
    const std::shared_ptr<Message>& Reply() const override;

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
    std::unique_ptr<const Purse> purse_;
    std::unique_ptr<const Purse> senderPurse_;
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
    bool ack_{false};
    bool direction_{false};
    bool isPrimary_{false};
    bool selling_{false};
    bool cash_{false};
    time64_t lifetime_{OT_TIME_ZERO};
    std::int32_t nRequestNum_{-1};
    std::int32_t nTransNumsNeeded_{0};
    const api::client::Wallet& wallet_;
    const api::client::Workflow& workflow_;
    Editor<ServerContext> context_editor_;
    ServerContext& context_;
    const OTAPI_Exec& exec_;
    const OT_API& otapi_;
    CommandResult last_attempt_;
    const bool is_transaction_{false};
    std::shared_ptr<PeerReply> peer_reply_;
    std::shared_ptr<PeerRequest> peer_request_;
    proto::ContactSectionName sectionName_{proto::CONTACTSECTION_ERROR};
    proto::ContactItemType itemType_{proto::CITEMTYPE_ERROR};
    Amount activationPrice_{0};
    Amount adjustment_{0};
    Amount amount_{0};
    Amount depth_{0};
    Amount increment_{0};
    Amount quantity_{0};
    Amount price_{0};
    Amount scale_{0};
    RemoteBoxType remoteBoxType_{RemoteBoxType::Error};
    TransactionNumber transactionNumber_{
        0};  // This is not what gets returned by GetTransactionNumber.
    proto::ConnectionInfoType infoType_{proto::CONNECTIONINFO_ERROR};
    proto::SecretType secretType_{proto::SECRETTYPE_ERROR};
    proto::UnitDefinition unitDefinition_{};

    void run();
    std::int32_t send();
    std::string send_once(
        const bool bIsTransaction,
        const bool bWillRetryAfterThis,
        bool& bCanRetryAfterThis);
    std::string send_request();
    std::string send_transaction(const std::size_t totalRetries);

    explicit OTAPI_Func(
        std::recursive_mutex& apilock,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Func_Type type);
    OTAPI_Func() = delete;
};
}  // namespace opentxs

#endif  // OPENTXS_CLIENT_OT_OTAPI_OT_HPP
