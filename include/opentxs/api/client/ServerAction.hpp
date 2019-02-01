// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CLIENT_SERVERACTION_HPP
#define OPENTXS_API_CLIENT_SERVERACTION_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <chrono>
#include <string>

namespace opentxs
{
namespace api
{
namespace client
{
class [[deprecated]] ServerAction
{
public:
    using Action = Pimpl<opentxs::client::ServerAction>;

    EXPORT virtual Action ActivateSmartContract(
        const identifier::Nym& localNymID,
        const Identifier& serverID,
        const Identifier& accountID,
        const std::string& agentName,
        std::unique_ptr<OTSmartContract>& contract) const = 0;
    EXPORT virtual Action AdjustUsageCredits(
        const identifier::Nym& localNymID,
        const Identifier& serverID,
        const identifier::Nym& targetNymID,
        const Amount adjustment) const = 0;
    EXPORT virtual Action CancelPaymentPlan(
        const identifier::Nym& localNymID,
        const Identifier& serverID,
        std::unique_ptr<OTPaymentPlan>& plan) const = 0;
    EXPORT virtual Action CreateMarketOffer(
        const Identifier& assetAccountID,
        const Identifier& currencyAccountID,
        const Amount scale,
        const Amount increment,
        const Amount quantity,
        const Amount price,
        const bool selling,
        const std::chrono::seconds lifetime,
        const std::string& stopSign,
        const Amount activationPrice) const = 0;
    EXPORT virtual Action DepositPaymentPlan(
        const identifier::Nym& localNymID,
        const Identifier& serverID,
        std::unique_ptr<OTPaymentPlan>& plan) const = 0;
    EXPORT virtual Action DownloadMarketList(
        const identifier::Nym& localNymID, const Identifier& serverID)
        const = 0;
    EXPORT virtual Action DownloadMarketOffers(
        const identifier::Nym& localNymID,
        const Identifier& serverID,
        const Identifier& marketID,
        const Amount depth) const = 0;
    EXPORT virtual Action DownloadMarketRecentTrades(
        const identifier::Nym& localNymID,
        const Identifier& serverID,
        const Identifier& marketID) const = 0;
    EXPORT virtual Action DownloadNymMarketOffers(
        const identifier::Nym& localNymID, const Identifier& serverID)
        const = 0;
    EXPORT virtual Action ExchangeBasketCurrency(
        const identifier::Nym& localNymID,
        const Identifier& serverID,
        const Identifier& instrumentDefinitionID,
        const Identifier& accountID,
        const Identifier& basketID,
        const bool direction) const = 0;
    EXPORT virtual Action IssueBasketCurrency(
        const identifier::Nym& localNymID,
        const Identifier& serverID,
        const proto::UnitDefinition& basket,
        const std::string& label = "") const = 0;
    EXPORT virtual Action KillMarketOffer(
        const identifier::Nym& localNymID,
        const Identifier& serverID,
        const Identifier& accountID,
        const TransactionNumber number) const = 0;
    EXPORT virtual Action KillPaymentPlan(
        const identifier::Nym& localNymID,
        const Identifier& serverID,
        const Identifier& accountID,
        const TransactionNumber number) const = 0;
    EXPORT virtual Action PayDividend(
        const identifier::Nym& localNymID,
        const Identifier& serverID,
        const Identifier& instrumentDefinitionID,
        const Identifier& accountID,
        const std::string& memo,
        const Amount amountPerShare) const = 0;
    EXPORT virtual Action TriggerClause(
        const identifier::Nym& localNymID,
        const Identifier& serverID,
        const TransactionNumber transactionNumber,
        const std::string& clause,
        const std::string& parameter) const = 0;
    EXPORT virtual Action UnregisterAccount(
        const identifier::Nym& localNymID,
        const Identifier& serverID,
        const Identifier& accountID) const = 0;
    EXPORT virtual Action UnregisterNym(
        const identifier::Nym& localNymID, const Identifier& serverID)
        const = 0;
    EXPORT virtual Action WithdrawVoucher(
        const identifier::Nym& localNymID,
        const Identifier& serverID,
        const Identifier& accountID,
        const identifier::Nym& recipientNymID,
        const Amount amount,
        const std::string& memo) const = 0;

    EXPORT virtual ~ServerAction() = default;

protected:
    ServerAction() = default;

private:
    ServerAction(const ServerAction&) = delete;
    ServerAction(ServerAction &&) = delete;
    ServerAction& operator=(const ServerAction&) = delete;
    ServerAction& operator=(ServerAction&&) = delete;
};
}  // namespace client
}  // namespace api
}  // namespace opentxs
#endif
