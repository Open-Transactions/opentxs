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

// A market has a list of OTOffers for all the bids, and another list of
// OTOffers for all the asks.
// Presumably the server will have different markets for different instrument
// definitions.

#ifndef OPENTXS_CORE_TRADE_OTMARKET_HPP
#define OPENTXS_CORE_TRADE_OTMARKET_HPP

#include "opentxs/core/cron/OTCron.hpp"
#include "opentxs/core/trade/OTOffer.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/OTStorage.hpp"

#include <cstdint>
#include <map>
#include <string>

namespace opentxs
{

class Account;
class OTASCIIArmor;
class OTCron;
class OTOffer;
class OTTrade;
class String;
namespace OTDB {
class OfferListNym;
class TradeListMarket;
}  // namespace OTDB

#define MAX_MARKET_QUERY_DEPTH                                                 \
    50 // todo add this to the ini file. (Now that we actually have one.)

// Multiple offers, mapped by price limit.
// Using multi-map since there will be more than one offer for each single
// price.
// (Map would only allow a single item on the map for each price.)
typedef std::multimap<int64_t, OTOffer*> mapOfOffers;

// The same offers are also mapped (uniquely) to transaction number.
typedef std::map<int64_t, OTOffer*> mapOfOffersTrnsNum;

class OTMarket : public Contract
{
private: // Private prevents erroneous use by other classes.
    typedef Contract ot_super;

private:
    OTCron* m_pCron{nullptr}; // The Cron object that owns this Market.

    OTDB::TradeListMarket* m_pTradeList{nullptr};

    mapOfOffers m_mapBids; // The buyers, ordered by price limit
    mapOfOffers m_mapAsks; // The sellers, ordered by price limit

    mapOfOffersTrnsNum m_mapOffers; // All of the offers on a single list,
                                    // ordered by transaction number.

    Identifier m_NOTARY_ID; // Always store this in any object that's
                            // associated with a specific server.

    // Every market involves a certain instrument definition being traded in a
    // certain
    // currency.
    Identifier m_INSTRUMENT_DEFINITION_ID; // This is the GOLD market. (Say.)
                                           // | (GOLD
                                           // for
    Identifier m_CURRENCY_TYPE_ID; // Gold is trading for DOLLARS.        |
                                   // DOLLARS, for example.)

    // Each Offer on the market must have a minimum increment that this divides
    // equally into.
    // (There is a "gold for dollars, minimum 1 oz" market, a "gold for dollars,
    // min 500 oz" market, etc.)
    int64_t m_lScale{0};

    int64_t m_lLastSalePrice{0};
    std::string m_strLastSaleDate;

    // The server stores a map of markets, one for each unique combination of
    // instrument definitions.
    // That's what this market class represents: one instrument definition being
    // traded and
    // priced in another.
    // It could be wheat for dollars, wheat for yen, or gold for dollars, or
    // gold for wheat, or
    // gold for oil, or oil for wheat.  REALLY, THE TWO ARE JUST ARBITRARY ASSET
    // TYPES. But in
    // order to keep terminology clear, I will refer to one as the "instrument
    // definition"
    // and the other as
    // the "currency type" so that it stays VERY clear which instrument
    // definition is up
    // for sale, and which
    // instrument definition (currency type) it is being priced in. Other than
    // that, the
    // two are technically
    // interchangeable.

    void cleanup_four_accounts(Account* p1, Account* p2, Account* p3,
                               Account* p4);
    void rollback_four_accounts(Account& p1, bool b1, const int64_t& a1,
                                Account& p2, bool b2, const int64_t& a2,
                                Account& p3, bool b3, const int64_t& a3,
                                Account& p4, bool b4, const int64_t& a4);

public:
    bool ValidateOfferForMarket(OTOffer& theOffer, String* pReason = nullptr);

    OTOffer* GetOffer(const int64_t& lTransactionNum);
    bool AddOffer(OTTrade* pTrade, OTOffer& theOffer, bool bSaveFile = true,
                  time64_t tDateAddedToMarket = OT_TIME_ZERO);
    bool RemoveOffer(const int64_t& lTransactionNum);
    // returns general information about offers on the market
    EXPORT bool GetOfferList(OTASCIIArmor& ascOutput, int64_t lDepth,
                             int32_t& nOfferCount);
    EXPORT bool GetRecentTradeList(OTASCIIArmor& ascOutput,
                                   int32_t& nTradeCount);

    // Returns more detailed information about offers for a specific Nym.
    bool GetNym_OfferList(const Identifier& NYM_ID,
                          OTDB::OfferListNym& theOutputList,
                          int32_t& nNymOfferCount);

    // Assumes a few things: Offer is part of Trade, and both have been
    // proven already to be a part of this market.
    // Basically the Offer is looked up on the Market by the Trade, and
    // then both are passed in here.
    // --Returns True if Trade should stay on the Cron list for more processing.
    // --Returns False if it should be removed and deleted.
    void ProcessTrade(OTTrade& theTrade, OTOffer& theOffer,
                      OTOffer& theOtherOffer);
    bool ProcessTrade(OTTrade& theTrade, OTOffer& theOffer);

    int64_t GetHighestBidPrice();
    int64_t GetLowestAskPrice();

    mapOfOffers::size_type GetBidCount()
    {
        return m_mapBids.size();
    }
    mapOfOffers::size_type GetAskCount()
    {
        return m_mapAsks.size();
    }
    void SetInstrumentDefinitionID(const Identifier& INSTRUMENT_DEFINITION_ID)
    {
        m_INSTRUMENT_DEFINITION_ID = INSTRUMENT_DEFINITION_ID;
    }
    void SetCurrencyID(const Identifier& CURRENCY_ID)
    {
        m_CURRENCY_TYPE_ID = CURRENCY_ID;
    }
    void SetNotaryID(const Identifier& NOTARY_ID)
    {
        m_NOTARY_ID = NOTARY_ID;
    }

    inline const Identifier& GetInstrumentDefinitionID() const
    {
        return m_INSTRUMENT_DEFINITION_ID;
    }
    inline const Identifier& GetCurrencyID() const
    {
        return m_CURRENCY_TYPE_ID;
    }
    inline const Identifier& GetNotaryID() const
    {
        return m_NOTARY_ID;
    }

    inline const int64_t& GetScale() const
    {
        return m_lScale;
    }
    inline void SetScale(const int64_t& lScale)
    {
        m_lScale = lScale;
        if (m_lScale < 1) m_lScale = 1;
    }

    inline const int64_t& GetLastSalePrice()
    {
        if (m_lLastSalePrice < 1) m_lLastSalePrice = 1;
        return m_lLastSalePrice;
    }
    inline void SetLastSalePrice(const int64_t& lLastSalePrice)
    {
        m_lLastSalePrice = lLastSalePrice;
        if (m_lLastSalePrice < 1) m_lLastSalePrice = 1;
    }

    const std::string& GetLastSaleDate()
    {
        return m_strLastSaleDate;
    }
    int64_t GetTotalAvailableAssets();
    OTMarket();
    OTMarket(const char* szFilename);
    OTMarket(const Identifier& NOTARY_ID,
             const Identifier& INSTRUMENT_DEFINITION_ID,
             const Identifier& CURRENCY_TYPE_ID, const int64_t& lScale);

    virtual ~OTMarket();

    void GetIdentifier(Identifier& theIdentifier) const override;

    inline void SetCronPointer(OTCron& theCron)
    {
        m_pCron = &theCron;
    }
    inline OTCron* GetCron()
    {
        return m_pCron;
    }
    bool LoadMarket();
    bool SaveMarket();

    void InitMarket();

    void Release() override;
    void Release_Market();

    // return -1 if error, 0 if nothing, and 1 if the node was processed.
    int32_t ProcessXMLNode(irr::io::IrrXMLReader*& xml) override;

    void UpdateContents() override; // Before transmission or serialization, this
                                   // is where the ledger saves its contents
};

} // namespace opentxs

#endif // OPENTXS_CORE_TRADE_OTMARKET_HPP
