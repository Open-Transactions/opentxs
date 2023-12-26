// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "otx/client/ServerAction.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/otx/client/Factory.hpp"
#include "internal/otx/common/Account.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/api/session/Wallet.internal.hpp"
#include "opentxs/identifier/Notary.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "otx/client/obsolete/OTAPI_Func.hpp"

namespace opentxs::factory
{
auto ServerAction(
    const api::session::Client& api,
    ContextLockCallback lockCallback)
    -> std::unique_ptr<otx::client::ServerAction>
{
    return std::make_unique<otx::client::imp::ServerAction>(
        api, std::move(lockCallback));
}
}  // namespace opentxs::factory

namespace opentxs::otx::client::imp
{
ServerAction::ServerAction(
    const api::session::Client& api,
    ContextLockCallback lockCallback)
    : api_(api)
    , lock_callback_(std::move(lockCallback))
{
    // WARNING: do not access api_.Wallet() during construction
}

auto ServerAction::ActivateSmartContract(
    const PasswordPrompt& reason,
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID,
    const identifier::Account& accountID,
    const UnallocatedCString& agentName,
    std::unique_ptr<OTSmartContract>& contract) const -> Action
{
    return std::make_unique<OTAPI_Func>(reason,
        ACTIVATE_SMART_CONTRACT,
        lock_callback_({localNymID.asBase58(api_.Crypto()), serverID.asBase58(api_.Crypto())}),
        api_,
        localNymID,
        serverID/*,
        accountID,
        agentName,
        contract*/);
}

auto ServerAction::AdjustUsageCredits(
    const PasswordPrompt& reason,
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID,
    const identifier::Nym& targetNymID,
    const Amount& adjustment) const -> Action
{
    return std::make_unique<OTAPI_Func>(reason,
        ADJUST_USAGE_CREDITS,
        lock_callback_({localNymID.asBase58(api_.Crypto()), serverID.asBase58(api_.Crypto())}),
        api_,
        localNymID,
        serverID/*,
        targetNymID,
        adjustment*/);
}

auto ServerAction::CancelPaymentPlan(
    const PasswordPrompt& reason,
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID,
    std::unique_ptr<OTPaymentPlan>& plan) const -> Action
{
    // NOTE: Normally the SENDER (PAYER) is the one who deposits a payment plan.
    // But in this case, the RECIPIENT (PAYEE) deposits it -- which means
    // "Please cancel this plan." It SHOULD fail, since it's only been signed
    // by the recipient, and not the sender. And that failure is what burns
    // the transaction number on the plan, so that it can no longer be used.
    //
    // So how do we know the difference between an ACTUAL "failure" versus a
    // purposeful "failure" ? Because if the failure comes from cancelling the
    // plan, the server reply transaction will have IsCancelled() set to true.
    return std::make_unique<OTAPI_Func>(reason,
        DEPOSIT_PAYMENT_PLAN,
        lock_callback_({localNymID.asBase58(api_.Crypto()), serverID.asBase58(api_.Crypto())}),
        api_,
        localNymID,
        serverID/*,
        plan->GetRecipientAcctID(),
        plan*/);
}

auto ServerAction::CreateMarketOffer(
    const PasswordPrompt& reason,
    const identifier::Account& assetAccountID,
    const identifier::Account& currencyAccountID,
    const Amount& scale,
    const Amount& increment,
    const std::int64_t quantity,
    const Amount& price,
    const bool selling,
    const std::chrono::seconds lifetime,
    const UnallocatedCString& stopSign,
    const Amount activationPrice) const -> Action
{
    auto notaryID = identifier::Notary{};
    auto nymID = identifier::Nym{};
    const auto assetAccount = api_.Wallet().Internal().Account(assetAccountID);

    if (assetAccount) {
        nymID = assetAccount.get().GetNymID();
        notaryID = assetAccount.get().GetPurportedNotaryID();
    }

    return std::make_unique<OTAPI_Func>(reason,
        CREATE_MARKET_OFFER,
        lock_callback_({nymID.asBase58(api_.Crypto()), notaryID.asBase58(api_.Crypto())}),
        api_,
        nymID,
        notaryID/*,
        assetAccountID,
        currencyAccountID,
        scale,
        increment,
        quantity,
        price,
        selling,
        lifetime.count(),
        activationPrice,
        stopSign*/);
}

auto ServerAction::DepositPaymentPlan(
    const PasswordPrompt& reason,
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID,
    std::unique_ptr<OTPaymentPlan>& plan) const -> Action
{
    return std::make_unique<OTAPI_Func>(reason,
        DEPOSIT_PAYMENT_PLAN,
        lock_callback_({localNymID.asBase58(api_.Crypto()), serverID.asBase58(api_.Crypto())}),
        api_,
        localNymID,
        serverID/*,
        plan->GetSenderAcctID(),
        plan*/);
}

auto ServerAction::DownloadMarketList(
    const PasswordPrompt& reason,
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID) const -> Action
{
    return std::make_unique<OTAPI_Func>(
        reason,
        GET_MARKET_LIST,
        lock_callback_(
            {localNymID.asBase58(api_.Crypto()),
             serverID.asBase58(api_.Crypto())}),
        api_,
        localNymID,
        serverID);
}

auto ServerAction::DownloadMarketOffers(
    const PasswordPrompt& reason,
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID,
    const identifier::Generic& marketID,
    const Amount depth) const -> Action
{
    return std::make_unique<OTAPI_Func>(reason,
        GET_MARKET_OFFERS,
        lock_callback_({localNymID.asBase58(api_.Crypto()), serverID.asBase58(api_.Crypto())}),
        api_,
        localNymID,
        serverID/*,
        marketID,
        depth*/);
}

auto ServerAction::DownloadMarketRecentTrades(
    const PasswordPrompt& reason,
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID,
    const identifier::Generic& marketID) const -> Action
{
    return std::make_unique<OTAPI_Func>(reason,
        GET_MARKET_RECENT_TRADES,
        lock_callback_({localNymID.asBase58(api_.Crypto()), serverID.asBase58(api_.Crypto())}),
        api_,
        localNymID,
        serverID/*,
        marketID*/);
}

auto ServerAction::DownloadNymMarketOffers(
    const PasswordPrompt& reason,
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID) const -> Action
{
    return std::make_unique<OTAPI_Func>(
        reason,
        GET_NYM_MARKET_OFFERS,
        lock_callback_(
            {localNymID.asBase58(api_.Crypto()),
             serverID.asBase58(api_.Crypto())}),
        api_,
        localNymID,
        serverID);
}

auto ServerAction::ExchangeBasketCurrency(
    const PasswordPrompt& reason,
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID,
    const identifier::UnitDefinition& instrumentDefinitionID,
    const identifier::Account& accountID,
    const identifier::Generic& basketID,
    const bool direction) const -> Action
{
    return std::make_unique<OTAPI_Func>(reason,
        EXCHANGE_BASKET,
        lock_callback_({localNymID.asBase58(api_.Crypto()), serverID.asBase58(api_.Crypto())}),
        api_,
        localNymID,
        serverID/*,
        instrumentDefinitionID,
        basketID,
        accountID,
        direction,
        api_.Internal().asClient().OTAPI().GetBasketMemberCount(basketID)*/);
}

auto ServerAction::IssueBasketCurrency(
    const PasswordPrompt& reason,
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID,
    const proto::UnitDefinition& basket,
    const UnallocatedCString& label) const -> Action
{
    return std::make_unique<OTAPI_Func>(reason,
        ISSUE_BASKET,
        lock_callback_({localNymID.asBase58(api_.Crypto()), serverID.asBase58(api_.Crypto())}),
        api_,
        localNymID,
        serverID/*,
        basket,
        label*/);
}

auto ServerAction::KillMarketOffer(
    const PasswordPrompt& reason,
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID,
    const identifier::Account& accountID,
    const TransactionNumber number) const -> Action
{
    return std::make_unique<OTAPI_Func>(reason,
        KILL_MARKET_OFFER,
        lock_callback_({localNymID.asBase58(api_.Crypto()), serverID.asBase58(api_.Crypto())}),
        api_,
        localNymID,
        serverID/*,
        accountID,
        number*/);
}

auto ServerAction::KillPaymentPlan(
    const PasswordPrompt& reason,
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID,
    const identifier::Account& accountID,
    const TransactionNumber number) const -> Action
{
    return std::make_unique<OTAPI_Func>(reason,
        KILL_PAYMENT_PLAN,
        lock_callback_({localNymID.asBase58(api_.Crypto()), serverID.asBase58(api_.Crypto())}),
        api_,
        localNymID,
        serverID/*,
        accountID,
        number*/);
}

auto ServerAction::PayDividend(
    const PasswordPrompt& reason,
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID,
    const identifier::UnitDefinition& instrumentDefinitionID,
    const identifier::Account& accountID,
    const UnallocatedCString& memo,
    const Amount amountPerShare) const -> Action
{
    return std::make_unique<OTAPI_Func>(reason,
        PAY_DIVIDEND,
        lock_callback_({localNymID.asBase58(api_.Crypto()), serverID.asBase58(api_.Crypto())}),
        api_,
        localNymID,
        serverID/*,
        accountID,
        instrumentDefinitionID,
        amountPerShare,
        memo*/);
}

auto ServerAction::TriggerClause(
    const PasswordPrompt& reason,
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID,
    const TransactionNumber transactionNumber,
    const UnallocatedCString& clause,
    const UnallocatedCString& parameter) const -> Action
{
    return std::make_unique<OTAPI_Func>(reason,
        TRIGGER_CLAUSE,
        lock_callback_({localNymID.asBase58(api_.Crypto()), serverID.asBase58(api_.Crypto())}),
        api_,
        localNymID,
        serverID/*,
        transactionNumber,
        clause,
        parameter*/);
}

auto ServerAction::UnregisterAccount(
    const PasswordPrompt& reason,
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID,
    const identifier::Account& accountID) const -> Action
{
    return std::make_unique<OTAPI_Func>(reason,
        DELETE_ASSET_ACCT,
        lock_callback_({localNymID.asBase58(api_.Crypto()), serverID.asBase58(api_.Crypto())}),
        api_,
        localNymID,
        serverID/*,
        accountID*/);
}

auto ServerAction::UnregisterNym(
    const PasswordPrompt& reason,
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID) const -> Action
{
    return std::make_unique<OTAPI_Func>(
        reason,
        DELETE_NYM,
        lock_callback_(
            {localNymID.asBase58(api_.Crypto()),
             serverID.asBase58(api_.Crypto())}),
        api_,
        localNymID,
        serverID);
}

auto ServerAction::WithdrawVoucher(
    const PasswordPrompt& reason,
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID,
    const identifier::Account& accountID,
    const identifier::Nym& recipientNymID,
    const Amount amount,
    const UnallocatedCString& memo) const -> Action
{
    return std::make_unique<OTAPI_Func>(reason,
        WITHDRAW_VOUCHER,
        lock_callback_({localNymID.asBase58(api_.Crypto()), serverID.asBase58(api_.Crypto())}),
        api_,
        localNymID,
        serverID/*,
        accountID,
        recipientNymID,
        amount,
        memo*/);
}
}  // namespace opentxs::otx::client::imp
