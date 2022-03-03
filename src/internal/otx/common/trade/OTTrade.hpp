// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <irrxml/irrXML.hpp>
#include <cstdint>

#include "internal/otx/Types.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/otx/common/Contract.hpp"
#include "internal/otx/common/OTTransactionType.hpp"
#include "internal/otx/common/cron/OTCronItem.hpp"
#include "internal/otx/common/trade/OTMarket.hpp"
#include "internal/otx/common/trade/OTOffer.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace session
{
namespace imp
{
class Factory;
}  // namespace imp
}  // namespace session

class Session;
}  // namespace api

namespace identifier
{
class Notary;
class Nym;
}  // namespace identifier

namespace identity
{
class Nym;
}  // namespace identity

namespace otx
{
namespace context
{
class Client;
}  // namespace context
}  // namespace otx

class OTMarket;
class OTOffer;
class PasswordPrompt;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
// An OTTrade is derived from OTCronItem. OTCron has a list of items,
// which may be trades or agreements or who knows what next.

/*
 OTTrade

 Standing Order (for Trades) MUST STORE:

 X 1) Transaction ID // It took a transaction number to create this trade. We
 record it here and use it to uniquely identify the trade, like any other
 transaction.
 X 4) CURRENCY TYPE ID  (Currency type ID of whatever I’m trying to buy or sell
 WITH. Dollars? Euro?)
 X 5) Account ID SENDER (for above currency type. This is the account where I
 make my payments from, to satisfy the trades.)
 X 6) Valid date range. (Start. Expressed as an absolute date.)
 X 7) Valid date range. ( End. Expressed as an absolute date.)

 X 2) Creation date.
 X 3) INTEGER: Number of trades that have processed through this order.

 X 8) STOP ORDER — SIGN (nullptr if not a stop order — otherwise GREATER THAN or
 LESS THAN…)
 X 9) STOP ORDER — PRICE (…AT X PRICE, POST THE OFFER TO THE MARKET.)

 Cron for these orders must check expiration dates and stop order prices.

 ———————————————————————————————
 */

class OTTrade : public OTCronItem
{
public:
    auto GetOriginType() const -> originType override
    {
        return originType::origin_market_offer;
    }

    auto VerifyOffer(OTOffer& offer) const -> bool;
    auto IssueTrade(
        OTOffer& offer,
        char stopSign = 0,
        const Amount& stopPrice = 0) -> bool;

    // The Trade always stores the original, signed version of its Offer.
    // This method allows you to grab a copy of it.
    inline auto GetOfferString(String& offer) -> bool
    {
        offer.Set(marketOffer_);
        if (marketOffer_->Exists()) { return true; }
        return false;
    }

    inline auto IsStopOrder() const -> bool
    {
        if ((stopSign_ == '<') || (stopSign_ == '>')) { return true; }
        return false;
    }

    inline auto GetStopPrice() const -> const Amount& { return stopPrice_; }

    inline auto IsGreaterThan() const -> bool
    {
        if (stopSign_ == '>') { return true; }
        return false;
    }

    inline auto IsLessThan() const -> bool
    {
        if (stopSign_ == '<') { return true; }
        return false;
    }

    // optionally returns the offer's market ID and a pointer to the market.
    auto GetOffer(const PasswordPrompt& reason, OTMarket** market = nullptr)
        -> OTOffer*;
    // optionally returns the offer's market ID and a pointer to the market.
    auto GetOffer(
        Identifier& offerMarketId,
        const PasswordPrompt& reason,
        OTMarket** market = nullptr) -> OTOffer*;

    inline auto GetCurrencyID() const -> const identifier::UnitDefinition&
    {
        return currencyTypeID_;
    }

    inline void SetCurrencyID(const identifier::UnitDefinition& currencyId)
    {
        currencyTypeID_ = currencyId;
    }

    inline auto GetCurrencyAcctID() const -> const Identifier&
    {
        return currencyAcctID_;
    }

    inline void SetCurrencyAcctID(const Identifier& currencyAcctID)
    {
        currencyAcctID_ = currencyAcctID;
    }

    inline void IncrementTradesAlreadyDone() { tradesAlreadyDone_++; }

    inline auto GetCompletedCount() -> std::int32_t
    {
        return tradesAlreadyDone_;
    }

    auto GetAssetAcctClosingNum() const -> std::int64_t;
    auto GetCurrencyAcctClosingNum() const -> std::int64_t;

    // Return True if should stay on OTCron's list for more processing.
    // Return False if expired or otherwise should be removed.
    auto ProcessCron(const PasswordPrompt& reason)
        -> bool override;  // OTCron calls
                           // this regularly,
                           // which is my
                           // chance to
                           // expire, etc.
    auto CanRemoveItemFromCron(const otx::context::Client& context)
        -> bool override;

    // From OTScriptable, we override this function. OTScriptable now does fancy
    // stuff like checking to see
    // if the Nym is an agent working on behalf of a party to the contract.
    // That's how all OTScriptable-derived
    // objects work by default.  But OTAgreement (payment plan) and OTTrade do
    // it the old way: they just check to
    // see if theNym has signed *this.
    //
    auto VerifyNymAsAgent(
        const identity::Nym& nym,
        const identity::Nym& signerNym) const -> bool override;

    auto VerifyNymAsAgentForAccount(
        const identity::Nym& nym,
        const Account& account) const -> bool override;
    void InitTrade();

    void Release_Trade();
    void Release() override;
    auto GetClosingNumber(const Identifier& acctId) const
        -> std::int64_t override;
    // return -1 if error, 0 if nothing, and 1 if the node was processed.
    auto ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t override;

    void UpdateContents(const PasswordPrompt& reason)
        override;  // Before transmission or
                   // serialization, this is where the
                   // ledger saves its contents

    ~OTTrade() override;

protected:
    void onFinalReceipt(
        OTCronItem& origCronItem,
        const std::int64_t& newTransactionNumber,
        Nym_p originator,
        Nym_p remover,
        const PasswordPrompt& reason) override;
    void onRemovalFromCron(const PasswordPrompt& reason) override;

private:
    friend api::session::imp::Factory;

    using ot_super = OTCronItem;

    OTUnitID currencyTypeID_;      // GOLD (Asset) is trading for DOLLARS
                                   // (Currency).
    OTIdentifier currencyAcctID_;  // My Dollar account, used for paying for
                                   // my Gold (say) trades.

    OTOffer* offer_{nullptr};  // The pointer to the Offer (NOT responsible for
                               // cleaning this up!!!
    // The offer is owned by the market and I only keep a pointer here for
    // convenience.

    bool hasTradeActivated_{false};  // Has the offer yet been first added to a
                                     // market?

    Amount stopPrice_{0};        // The price limit that activates the STOP
                                 // order.
    char stopSign_{0x0};         // Value is 0, or '<', or '>'.
    bool stopActivated_{false};  // If the Stop Order has already activated, I
                                 // need to know that.

    std::int32_t tradesAlreadyDone_{0};  // How many trades have already
                                         // processed through this order? We
                                         // keep track.

    OTString marketOffer_;  // The market offer associated with this trade.

    OTTrade(const api::Session& api);
    OTTrade(
        const api::Session& api,
        const identifier::Notary& notaryID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const Identifier& assetAcctId,
        const identifier::Nym& nymID,
        const identifier::UnitDefinition& currencyId,
        const Identifier& currencyAcctId);
    OTTrade(const OTTrade&) = delete;
    OTTrade(OTTrade&&) = delete;
    auto operator=(const OTTrade&) -> OTTrade& = delete;
    auto operator=(OTTrade&&) -> OTTrade& = delete;

    OTTrade() = delete;
};
}  // namespace opentxs
