// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <iosfwd>
#include <memory>
#include <mutex>

#include "Proto.hpp"
#include "internal/otx/client/OTPayment.hpp"
#include "internal/otx/client/obsolete/ServerAction.hpp"
#include "internal/otx/common/Cheque.hpp"
#include "internal/otx/common/Ledger.hpp"
#include "internal/otx/common/recurring/OTPaymentPlan.hpp"
#include "internal/otx/smartcontract/OTSmartContract.hpp"
#include "internal/util/Editor.hpp"
#include "internal/util/Lockable.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/contract/peer/ConnectionInfoType.hpp"
#include "opentxs/core/contract/peer/SecretType.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"
#include "serialization/protobuf/UnitDefinition.pb.h"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api

namespace identifier
{
class Notary;
class Nym;
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
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
auto VerifyStringVal(const UnallocatedCString&) -> bool;

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
        const identifier::Notary& serverID);
    explicit OTAPI_Func(
        const PasswordPrompt& reason,
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const identifier::Notary& serverID,
        const proto::UnitDefinition& unitDefinition,
        const UnallocatedCString& label);
    explicit OTAPI_Func(
        const PasswordPrompt& reason,
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const identifier::Notary& serverID,
        const identifier::Nym& nymID2);
    explicit OTAPI_Func(
        const PasswordPrompt& reason,
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const identifier::Notary& serverID,
        const identifier::Nym& nymID2,
        const Amount& int64Val);
    explicit OTAPI_Func(
        const PasswordPrompt& reason,
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const identifier::Notary& serverID,
        const Identifier& recipientID,
        std::unique_ptr<OTPaymentPlan>& paymentPlan);
    explicit OTAPI_Func(
        const PasswordPrompt& reason,
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const identifier::Notary& serverID,
        const TransactionNumber& transactionNumber,
        const UnallocatedCString& clause,
        const UnallocatedCString& parameter);
    explicit OTAPI_Func(
        const PasswordPrompt& reason,
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const identifier::Notary& serverID,
        const Identifier& accountID,
        const UnallocatedCString& agentName,
        std::unique_ptr<OTSmartContract>& contract);
    explicit OTAPI_Func(
        const PasswordPrompt& reason,
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const identifier::Notary& serverID,
        const Identifier& recipientID,
        const Identifier& requestID,
        const bool ack);
    explicit OTAPI_Func(
        const PasswordPrompt& reason,
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const identifier::Notary& serverID,
        const identifier::Nym& nymID2,
        const Identifier& targetID,
        const Amount& amount,
        const UnallocatedCString& message);
    explicit OTAPI_Func(
        const PasswordPrompt& reason,
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const identifier::Notary& serverID,
        const Identifier& targetID,
        const UnallocatedCString& primary,
        const UnallocatedCString& secondary,
        const contract::peer::SecretType& secretType);
    explicit OTAPI_Func(
        const PasswordPrompt& reason,
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const identifier::Notary& serverID,
        const Identifier& recipientID,
        const Identifier& requestID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const UnallocatedCString& txid,
        const Amount& amount);
    explicit OTAPI_Func(
        const PasswordPrompt& reason,
        OTAPI_Func_Type theType,
        std::recursive_mutex& apilock,
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const identifier::Notary& serverID,
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
        const identifier::Notary& serverID,
        const Identifier& assetAccountID,
        const Identifier& currencyAccountID,
        const Amount& scale,
        const Amount& increment,
        const std::int64_t& quantity,
        const Amount& price,
        const bool selling,
        const Time lifetime,
        const Amount& activationPrice,
        const UnallocatedCString& stopSign);

    auto LastSendResult() const -> SendResult final { return {}; }
    auto Reply() const -> const std::shared_ptr<Message> final { return {}; }

    auto Run(const std::size_t totalRetries = 2) -> UnallocatedCString final;

    ~OTAPI_Func() final;

private:
    static const UnallocatedMap<OTAPI_Func_Type, UnallocatedCString> type_name_;
    static const UnallocatedMap<OTAPI_Func_Type, bool> type_type_;

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
    UnallocatedCString agentName_;
    UnallocatedCString clause_;
    UnallocatedCString key_;
    UnallocatedCString login_;
    UnallocatedCString message_;
    UnallocatedCString parameter_;
    UnallocatedCString password_;
    UnallocatedCString primary_;
    UnallocatedCString secondary_;
    UnallocatedCString stopSign_;
    UnallocatedCString txid_;
    UnallocatedCString url_;
    UnallocatedCString value_;
    UnallocatedCString label_;
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
        bool& bCanRetryAfterThis) -> UnallocatedCString;

    explicit OTAPI_Func(
        const PasswordPrompt& reason,
        std::recursive_mutex& apilock,
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const identifier::Notary& serverID,
        const OTAPI_Func_Type type);
    OTAPI_Func() = delete;
};
}  // namespace opentxs
