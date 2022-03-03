// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <cstdint>
#include <memory>

#include "internal/otx/client/ServerAction.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

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
class Nym;
class Notary;
class UnitDefinition;
}  // namespace identifier

namespace proto
{
class UnitDefinition;
}  // namespace proto

class Identifier;
class OTPaymentPlan;
class OTSmartContract;
class PasswordPrompt;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::client::implementation
{
class ServerAction final : virtual public opentxs::otx::client::ServerAction
{
public:
    auto ActivateSmartContract(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const Identifier& accountID,
        const UnallocatedCString& agentName,
        std::unique_ptr<OTSmartContract>& contract) const -> Action final;
    auto AdjustUsageCredits(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const identifier::Nym& targetNymID,
        const Amount& adjustment) const -> Action final;
    auto CancelPaymentPlan(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        std::unique_ptr<OTPaymentPlan>& plan) const -> Action final;
    auto CreateMarketOffer(
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
        const Amount activationPrice) const -> Action final;
    auto DepositPaymentPlan(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        std::unique_ptr<OTPaymentPlan>& plan) const -> Action final;
    auto DownloadMarketList(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID) const -> Action final;
    auto DownloadMarketOffers(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const Identifier& marketID,
        const Amount depth) const -> Action final;
    auto DownloadMarketRecentTrades(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const Identifier& marketID) const -> Action final;
    auto DownloadNymMarketOffers(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID) const -> Action final;
    auto ExchangeBasketCurrency(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const Identifier& accountID,
        const Identifier& basketID,
        const bool direction) const -> Action final;
    auto IssueBasketCurrency(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const proto::UnitDefinition& basket,
        const UnallocatedCString& label) const -> Action final;
    auto KillMarketOffer(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const Identifier& accountID,
        const TransactionNumber number) const -> Action final;
    auto KillPaymentPlan(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const Identifier& accountID,
        const TransactionNumber number) const -> Action final;
    auto PayDividend(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const Identifier& accountID,
        const UnallocatedCString& memo,
        const Amount amountPerShare) const -> Action final;
    auto TriggerClause(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const TransactionNumber transactionNumber,
        const UnallocatedCString& clause,
        const UnallocatedCString& parameter) const -> Action final;
    auto UnregisterAccount(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const Identifier& accountID) const -> Action final;
    auto UnregisterNym(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID) const -> Action final;
    auto WithdrawVoucher(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Notary& serverID,
        const Identifier& accountID,
        const identifier::Nym& recipientNymID,
        const Amount amount,
        const UnallocatedCString& memo) const -> Action final;

    ServerAction(
        const api::session::Client& api,
        const ContextLockCallback& lockCallback);

    ~ServerAction() final = default;

private:
    const api::session::Client& api_;
    ContextLockCallback lock_callback_;

    ServerAction() = delete;
    ServerAction(const ServerAction&) = delete;
    ServerAction(ServerAction&&) = delete;
    auto operator=(const ServerAction&) -> ServerAction& = delete;
    auto operator=(ServerAction&&) -> ServerAction& = delete;
};
}  // namespace opentxs::otx::client::implementation
