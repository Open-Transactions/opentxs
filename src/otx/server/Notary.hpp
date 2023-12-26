// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "internal/network/zeromq/socket/Push.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/otx/common/Cheque.hpp"
#include "internal/otx/common/Item.hpp"
#include "internal/otx/common/Ledger.hpp"
#include "internal/otx/common/OTTransaction.hpp"
#include "opentxs/api/session/Wallet.internal.hpp"
#include "opentxs/identifier/Account.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Notary;
}  // namespace session
}  // namespace api

namespace identifier
{
class Notary;
class Nym;
class UnitDefinition;
}  // namespace identifier

namespace identity
{
class Nym;
}  // namespace identity

namespace otx
{
namespace blind
{
class Mint;
class Purse;
class Token;
}  // namespace blind

namespace context
{
class Client;
}  // namespace context
}  // namespace otx

namespace server
{
class Server;
}  // namespace server

class PasswordPrompt;
class String;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::server
{
class Notary
{
public:
    void NotarizeProcessInbox(
        otx::context::Client& context,
        ExclusiveAccount& account,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        Ledger& inbox,
        Ledger& outbox,
        bool& outSuccess);
    auto NotarizeProcessNymbox(
        otx::context::Client& context,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess) -> bool;
    void NotarizeTransaction(
        otx::context::Client& context,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);

    Notary() = delete;
    Notary(const Notary&) = delete;
    Notary(Notary&&) = delete;
    auto operator=(const Notary&) -> Notary& = delete;
    auto operator=(Notary&&) -> Notary& = delete;

private:
    friend Server;

    class Finalize
    {
    public:
        Finalize(
            const identity::Nym& signer,
            Item& item,
            Item& balanceItem,
            const PasswordPrompt& reason);
        Finalize() = delete;

        ~Finalize();

    private:
        const identity::Nym& signer_;
        Item& item_;
        Item& balance_item_;
        const PasswordPrompt& reason_;
    };

    Server& server_;
    const PasswordPrompt& reason_;
    const opentxs::api::session::Notary& api_;
    OTZMQPushSocket notification_socket_;

    void AddHashesToTransaction(
        OTTransaction& transaction,
        const Ledger& inbox,
        const Ledger& outbox,
        const identifier::Account& accounthash) const;
    auto extract_cheque(
        const identifier::Notary& serverID,
        const identifier::UnitDefinition& unitID,
        const Item& item) const -> std::unique_ptr<Cheque>;
    void send_push_notification(
        const Account& account,
        const std::shared_ptr<const Ledger>& inbox,
        const std::shared_ptr<const Ledger>& outbox,
        const std::shared_ptr<const OTTransaction>& item) const;

    void cancel_cheque(
        const OTTransaction& input,
        const Cheque& cheque,
        const Item& depositItem,
        const String& serializedDepositItem,
        const Item& balanceItem,
        otx::context::Client& context,
        Account& account,
        Ledger& inbox,
        const Ledger& outbox,
        OTTransaction& output,
        bool& success,
        Item& responseItem,
        Item& responseBalanceItem);
    void deposit_cheque(
        const OTTransaction& input,
        const Item& depositItem,
        const String& serializedDepositItem,
        const Item& balanceItem,
        const Cheque& cheque,
        otx::context::Client& depositorContext,
        ExclusiveAccount& depositorAccount,
        Ledger& depositorInbox,
        const Ledger& depositorOutbox,
        OTTransaction& output,
        bool& success,
        Item& responseItem,
        Item& responseBalanceItem);
    void deposit_cheque(
        const OTTransaction& input,
        const Item& depositItem,
        const String& serializedDepositItem,
        const Item& balanceItem,
        const Cheque& cheque,
        const bool isVoucher,
        const bool cancelling,
        const identifier::Nym& senderNymID,
        otx::context::Client& senderContext,
        Account& senderAccount,
        Ledger& senderInbox,
        std::shared_ptr<OTTransaction>& inboxItem,
        Account& sourceAccount,
        const otx::context::Client& depositorContext,
        Account& depositorAccount,
        const Ledger& depositorInbox,
        const Ledger& depositorOutbox,
        bool& success,
        Item& responseItem,
        Item& responseBalanceItem);
    void NotarizeCancelCronItem(
        otx::context::Client& context,
        ExclusiveAccount& assetAccount,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);
    void NotarizeDeposit(
        otx::context::Client& context,
        ExclusiveAccount& account,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        Ledger& inbox,
        Ledger& outbox,
        bool& outSuccess);
    void NotarizeExchangeBasket(
        otx::context::Client& context,
        ExclusiveAccount& sourceAccount,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        Ledger& inbox,
        Ledger& outbox,
        bool& outSuccess);
    void NotarizeMarketOffer(
        otx::context::Client& context,
        ExclusiveAccount& assetAccount,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);
    void NotarizePayDividend(
        otx::context::Client& context,
        ExclusiveAccount& account,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        Ledger& inbox,
        Ledger& outbox,
        bool& outSuccess);
    void NotarizePaymentPlan(
        otx::context::Client& context,
        ExclusiveAccount& depositorAccount,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);
    void NotarizeSmartContract(
        otx::context::Client& context,
        ExclusiveAccount& activatingAccount,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);
    void NotarizeTransfer(
        otx::context::Client& context,
        ExclusiveAccount& fromAccount,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        Ledger& inbox,
        Ledger& outbox,
        bool& outSuccess);
    void NotarizeWithdrawal(
        otx::context::Client& context,
        ExclusiveAccount& account,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        Ledger& inbox,
        Ledger& outbox,
        bool& outSuccess);
    void process_cash_deposit(
        const OTTransaction& input,
        const Item& depositItem,
        const Item& balanceItem,
        otx::context::Client& context,
        ExclusiveAccount& depositorAccount,
        OTTransaction& output,
        Ledger& inbox,
        Ledger& outbox,
        bool& success,
        Item& responseItem,
        Item& responseBalanceItem);
    void process_cash_withdrawal(
        const OTTransaction& requestTransaction,
        const Item& requestItem,
        const Item& balanceItem,
        otx::context::Client& context,
        ExclusiveAccount& account,
        identifier::Account& accountHash,
        Ledger& inbox,
        Ledger& outbox,
        Item& responseItem,
        Item& responseBalanceItem,
        bool& success);
    void process_cheque_deposit(
        const OTTransaction& input,
        const Item& depositItem,
        const Item& balanceItem,
        otx::context::Client& context,
        ExclusiveAccount& depositorAccount,
        OTTransaction& output,
        Ledger& inbox,
        Ledger& outbox,
        bool& success,
        Item& responseItem,
        Item& responseBalanceItem);
    auto process_token_deposit(
        ExclusiveAccount& reserveAccount,
        Account& depositAccount,
        otx::blind::Token& token) -> bool;
    auto process_token_withdrawal(
        const identifier::UnitDefinition& unit,
        otx::context::Client& context,
        ExclusiveAccount& reserveAccount,
        Account& account,
        otx::blind::Purse& replyPurse,
        otx::blind::Token&& token) -> bool;
    auto verify_token(otx::blind::Mint& mint, otx::blind::Token& token) -> bool;

    Notary(
        Server& server,
        const PasswordPrompt& reason,
        const opentxs::api::session::Notary& manager);
};
}  // namespace opentxs::server
