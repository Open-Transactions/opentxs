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

// An OTTrade is derived from OTCronItem. OTCron has a list of items,
// which may be trades or agreements or who knows what next.

#ifndef OPENTXS_CORE_TRADE_OTTRADE_HPP
#define OPENTXS_CORE_TRADE_OTTRADE_HPP

#include "opentxs/core/cron/OTCronItem.hpp"
#include "opentxs/core/trade/OTMarket.hpp"
#include "opentxs/core/trade/OTOffer.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/OTTransactionType.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/Types.hpp"

#include <stdint.h>

namespace opentxs
{

class Account;
class Identifier;
class Nym;
class OTMarket;
class OTOffer;

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
private:
    typedef OTCronItem ot_super;

    Identifier currencyTypeID_; // GOLD (Asset) is trading for DOLLARS
                                // (Currency).
    Identifier currencyAcctID_; // My Dollar account, used for paying for
                                // my Gold (say) trades.

    OTOffer* offer_{nullptr}; // The pointer to the Offer (NOT responsible for cleaning
                     // this up!!!
    // The offer is owned by the market and I only keep a pointer here for
    // convenience.

    bool hasTradeActivated_{false}; // Has the offer yet been first added to a
                             // market?

    int64_t stopPrice_{0};  // The price limit that activates the STOP order.
    char stopSign_{0x0};      // Value is 0, or '<', or '>'.
    bool stopActivated_{false}; // If the Stop Order has already activated, I need
                         // to know that.

    int32_t tradesAlreadyDone_{0}; // How many trades have already processed
                                // through this order? We keep track.

    String marketOffer_; // The market offer associated with this trade.

protected:
    void onFinalReceipt(OTCronItem& origCronItem,
                                const int64_t& newTransactionNumber,
                                Nym& originator, Nym* remover) override;
    void onRemovalFromCron() override;

public:
    originType GetOriginType() const override
    { return originType::origin_market_offer; }

    EXPORT bool VerifyOffer(OTOffer& offer) const;
    EXPORT bool IssueTrade(OTOffer& offer, char stopSign = 0,
                           int64_t stopPrice = 0);

    // The Trade always stores the original, signed version of its Offer.
    // This method allows you to grab a copy of it.
    inline bool GetOfferString(String& offer)
    {
        offer.Set(marketOffer_);
        if (marketOffer_.Exists()) {
            return true;
        }
        return false;
    }

    inline bool IsStopOrder() const
    {
        if ((stopSign_ == '<') || (stopSign_ == '>')) {
            return true;
        }
        return false;
    }

    inline const int64_t& GetStopPrice() const
    {
        return stopPrice_;
    }

    inline bool IsGreaterThan() const
    {
        if (stopSign_ == '>') {
            return true;
        }
        return false;
    }

    inline bool IsLessThan() const
    {
        if (stopSign_ == '<') {
            return true;
        }
        return false;
    }

    // optionally returns the offer's market ID and a pointer to the market.
    OTOffer* GetOffer(Identifier* offerMarketId = nullptr,
                      OTMarket* *market = nullptr);

    inline const Identifier& GetCurrencyID() const
    {
        return currencyTypeID_;
    }

    inline void SetCurrencyID(const Identifier& currencyId)
    {
        currencyTypeID_ = currencyId;
    }

    inline const Identifier& GetCurrencyAcctID() const
    {
        return currencyAcctID_;
    }

    inline void SetCurrencyAcctID(const Identifier& currencyAcctID)
    {
        currencyAcctID_ = currencyAcctID;
    }

    inline void IncrementTradesAlreadyDone()
    {
        tradesAlreadyDone_++;
    }

    inline int32_t GetCompletedCount()
    {
        return tradesAlreadyDone_;
    }

    EXPORT int64_t GetAssetAcctClosingNum() const;
    EXPORT int64_t GetCurrencyAcctClosingNum() const;

    // Return True if should stay on OTCron's list for more processing.
    // Return False if expired or otherwise should be removed.
    bool ProcessCron() override; // OTCron calls this regularly, which is my
                                // chance to expire, etc.
    bool CanRemoveItemFromCron(const ClientContext& context) override;

    // From OTScriptable, we override this function. OTScriptable now does fancy
    // stuff like checking to see
    // if the Nym is an agent working on behalf of a party to the contract.
    // That's how all OTScriptable-derived
    // objects work by default.  But OTAgreement (payment plan) and OTTrade do
    // it the old way: they just check to
    // see if theNym has signed *this.
    //
    bool VerifyNymAsAgent(
        const Nym& nym,
        const Nym& signerNym,
        mapOfConstNyms* preloadedMap = nullptr) const override;

    bool VerifyNymAsAgentForAccount(
        const Nym& nym,
        Account& account) const override;
    EXPORT OTTrade();
    EXPORT OTTrade(const Identifier& notaryID,
                   const Identifier& instrumentDefinitionID,
                   const Identifier& assetAcctId, const Identifier& nymID,
                   const Identifier& currencyId,
                   const Identifier& currencyAcctId);
    EXPORT virtual ~OTTrade();

    void InitTrade();

    void Release_Trade();
    void Release() override;
    int64_t GetClosingNumber(const Identifier& acctId) const override;
    // return -1 if error, 0 if nothing, and 1 if the node was processed.
    int32_t ProcessXMLNode(irr::io::IrrXMLReader*& xml) override;

    void UpdateContents() override; // Before transmission or serialization, this
                                   // is where the ledger saves its contents
};

} // namespace opentxs

#endif // OPENTXS_CORE_TRADE_OTTRADE_HPP
