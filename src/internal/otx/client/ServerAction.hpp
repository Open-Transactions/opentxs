// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>

#include "opentxs/Version.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "otx/client/obsolete/OTAPI_Func.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace client
{
class ServerAction;
}  // namespace client

namespace identifier
{
class Nym;
class Notary;
class UnitDefinition;
}  // namespace identifier

namespace proto
{
class UnitDefinition;
}  // namespace proto

class Amount;
class OTPaymentPlan;
class OTSmartContract;
class PasswordPrompt;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::client
{
class ServerAction
{
public:
    using Action = std::unique_ptr<opentxs::client::ServerAction>;

    virtual auto ActivateSmartContract(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const Identifier& accountID,
        const UnallocatedCString& agentName,
        std::unique_ptr<OTSmartContract>& contract) const -> Action = 0;
    virtual auto AdjustUsageCredits(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const identifier::Nym& targetNymID,
        const Amount& adjustment) const -> Action = 0;
    virtual auto CancelPaymentPlan(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        std::unique_ptr<OTPaymentPlan>& plan) const -> Action = 0;
    virtual auto CreateMarketOffer(
        const PasswordPrompt& reason,
        const Identifier& assetAccountID,
        const Identifier& currencyAccountID,
        const Amount& scale,
        const Amount& increment,
        const std::int64_t quantity,
        const Amount& price,
        const bool selling,
        const std::chrono::seconds lifetime,
        const UnallocatedCString& stopSign,
        const Amount activationPrice) const -> Action = 0;
    virtual auto DepositPaymentPlan(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        std::unique_ptr<OTPaymentPlan>& plan) const -> Action = 0;
    virtual auto DownloadMarketList(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID) const -> Action = 0;
    virtual auto DownloadMarketOffers(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const Identifier& marketID,
        const Amount depth) const -> Action = 0;
    virtual auto DownloadMarketRecentTrades(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const Identifier& marketID) const -> Action = 0;
    virtual auto DownloadNymMarketOffers(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID) const -> Action = 0;
    virtual auto ExchangeBasketCurrency(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const Identifier& accountID,
        const Identifier& basketID,
        const bool direction) const -> Action = 0;
    virtual auto IssueBasketCurrency(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const proto::UnitDefinition& basket,
        const UnallocatedCString& label = "") const -> Action = 0;
    virtual auto KillMarketOffer(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const Identifier& accountID,
        const TransactionNumber number) const -> Action = 0;
    virtual auto KillPaymentPlan(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const Identifier& accountID,
        const TransactionNumber number) const -> Action = 0;
    virtual auto PayDividend(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const Identifier& accountID,
        const UnallocatedCString& memo,
        const Amount amountPerShare) const -> Action = 0;
    virtual auto TriggerClause(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const TransactionNumber transactionNumber,
        const UnallocatedCString& clause,
        const UnallocatedCString& parameter) const -> Action = 0;
    virtual auto UnregisterAccount(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const Identifier& accountID) const -> Action = 0;
    virtual auto UnregisterNym(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID) const -> Action = 0;
    virtual auto WithdrawVoucher(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const Identifier& accountID,
        const identifier::Nym& recipientNymID,
        const Amount amount,
        const UnallocatedCString& memo) const -> Action = 0;

    virtual ~ServerAction() = default;

protected:
    ServerAction() = default;

private:
    ServerAction(const ServerAction&) = delete;
    ServerAction(ServerAction&&) = delete;
    auto operator=(const ServerAction&) -> ServerAction& = delete;
    auto operator=(ServerAction&&) -> ServerAction& = delete;
};
}  // namespace opentxs::otx::client
