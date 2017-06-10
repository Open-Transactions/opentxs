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

// Each instance of OTOffer represents a Bid or Ask. (A Market has a list of bid
// offers and a list of ask offers.)

#ifndef OPENTXS_CORE_TRADE_OTOFFER_HPP
#define OPENTXS_CORE_TRADE_OTOFFER_HPP

#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Instrument.hpp"

#include <stdint.h>

namespace opentxs
{

class OTTrade;

/*
 OTOffer

 Offer MUST STORE:

 X 1) Transaction ID (MUST be linked to a trade, so it can expire, and so it can
 be paid for.)
 X 2) ASSET TYPE ID of whatever I’m trying to BUY or SELL. (Is this the Gold
 market?)
 X 7) CURRENCY TYPE ID of whatever I’m trying to buy or sell it WITH. (Is it
 dollars? Euro? Yen?)
 X 8) BUY OR SELL? (BOOL)
 X 9) Bid/Ask price (limit / per minimum increment.)

 X 4) Total number of assets available for sale or purchase. (4 ounces of gold?
 12 ounces of gold?)
 X 5) Number of assets already traded, against the above total.
 X 6) Minimum increment for sale or purchase (if matches “total number of assets
 for sale”, effectively becomes a FILL OR KILL order. MUST be 1 or greater.
 CANNOT be zero.)
*/
class OTOffer : public Instrument
{
private: // Private prevents erroneous use by other classes.
    typedef Instrument ot_super;

    // From OTInstrument (parent class of this)
    /*
public:
     inline time64_t GetValidFrom()    const { return m_VALID_FROM; }
     inline time64_t GetValidTo()        const { return m_VALID_TO; }

     inline void SetValidFrom(time64_t TIME_FROM)    { m_VALID_FROM    =
TIME_FROM; }
     inline void SetValidTo(time64_t TIME_TO)        { m_VALID_TO    = TIME_TO;
}


     inline const OTIdentifier& GetInstrumentDefinitionID() const { return
m_InstrumentDefinitionID; }
     inline const OTIdentifier& GetNotaryID() const { return m_NotaryID; }

     inline void SetInstrumentDefinitionID(const OTIdentifier&
INSTRUMENT_DEFINITION_ID)  {
m_InstrumentDefinitionID    =
INSTRUMENT_DEFINITION_ID; }
     inline void SetNotaryID(const OTIdentifier& NOTARY_ID) { m_NotaryID    =
NOTARY_ID; }

     bool VerifyCurrentDate(); // Verify the current date against the VALID FROM
/ TO dates.
     */
    time64_t m_tDateAddedToMarket{0};

    bool isPowerOfTen(const int64_t& x);

protected:
    OTTrade* m_pTrade{nullptr}; // If this offer is actually connected to a trade, it
                       // will have a pointer.

    Identifier m_CURRENCY_TYPE_ID; // GOLD (Asset) is trading for DOLLARS
                                   // (Currency).
    bool m_bSelling{false};               // true = ask. false = bid.
    // If a bid, this is the most I will pay. If an ask, this is the least I
    // will sell for. My limit.
    // (Normally the price I get is whatever is the best one on the market right
    // now.)
    int64_t m_lPriceLimit{0}; // Denominated in CURRENCY TYPE, and priced per
                           // SCALE.
                           // 1oz market price limit might be 1,300
    // 100oz market price limit might be 130,000 (or 127,987 or whatever)

    int64_t m_lTransactionNum{0};   // Matches to an OTTrade stored in OTCron.
    int64_t m_lTotalAssetsOffer{0}; // Total amount of ASSET TYPE trying to BUY or
                                 // SELL, this trade.
    int64_t m_lFinishedSoFar{0}; // Number of ASSETs bought or sold already against
                              // the above total.

    int64_t m_lScale{0}; // 1oz market? 100oz market? 10,000oz market? This
                      // determines size and granularity.
    int64_t m_lMinimumIncrement{0}; // Each sale or purchase against the above
                                 // total must be in minimum increments.
    // Minimum Increment must be evenly divisible by m_lScale.
    // (This effectively becomes a "FILL OR KILL" order if set to the same value
    // as m_lTotalAssetsOffer. Also, MUST be 1
    // or great. CANNOT be zero. Enforce this at class level. You cannot sell
    // something in minimum increments of 0.)
    inline void SetTransactionNum(const int64_t& lTransactionNum)
    {
        m_lTransactionNum = lTransactionNum;
    }
    inline void SetPriceLimit(const int64_t& lPriceLimit)
    {
        m_lPriceLimit = lPriceLimit;
    }
    inline void SetTotalAssetsOnOffer(const int64_t& lTotalAssets)
    {
        m_lTotalAssetsOffer = lTotalAssets;
    }
    inline void SetFinishedSoFar(const int64_t& lFinishedSoFar)
    {
        m_lFinishedSoFar = lFinishedSoFar;
    }
    inline void SetMinimumIncrement(const int64_t& lMinIncrement)
    {
        m_lMinimumIncrement = lMinIncrement;
        if (m_lMinimumIncrement < 1) m_lMinimumIncrement = 1;
    }
    inline void SetScale(const int64_t& lScale)
    {
        m_lScale = lScale;
        if (m_lScale < 1) m_lScale = 1;
    }

public:
    EXPORT bool MakeOffer(
        bool bBuyingOrSelling,            // True == SELLING, False == BUYING
        const int64_t& lPriceLimit,       // Per Scale...
        const int64_t& lTotalAssetsOffer, // Total assets available for sale or
                                          // purchase.
        const int64_t& lMinimumIncrement, // The minimum increment that must be
                                          // bought or sold for each transaction
        const int64_t& lTransactionNum,   // The transaction number authorizing
                                          // this trade.
        const time64_t& VALID_FROM = OT_TIME_ZERO, // defaults to RIGHT NOW
        const time64_t& VALID_TO = OT_TIME_ZERO);  // defaults to 24 hours (a
                                                   // "Day Order")
    inline void IncrementFinishedSoFar(const int64_t& lFinishedSoFar)
    {
        m_lFinishedSoFar += lFinishedSoFar;
    }

    inline int64_t GetAmountAvailable() const
    {
        return GetTotalAssetsOnOffer() - GetFinishedSoFar();
    }
    inline const int64_t& GetTransactionNum() const
    {
        return m_lTransactionNum;
    }

    inline const int64_t& GetPriceLimit() const
    {
        return m_lPriceLimit;
    }
    inline const int64_t& GetTotalAssetsOnOffer() const
    {
        return m_lTotalAssetsOffer;
    }
    inline const int64_t& GetFinishedSoFar() const
    {
        return m_lFinishedSoFar;
    }
    inline const int64_t& GetMinimumIncrement()
    {
        if (m_lMinimumIncrement < 1) m_lMinimumIncrement = 1;
        return m_lMinimumIncrement;
    }
    inline const int64_t& GetScale() const
    {
        return m_lScale;
    }

    inline const Identifier& GetCurrencyID() const
    {
        return m_CURRENCY_TYPE_ID;
    }
    inline void SetCurrencyID(const Identifier& CURRENCY_ID)
    {
        m_CURRENCY_TYPE_ID = CURRENCY_ID;
    }

    // Buying or selling?
    inline bool IsBid()
    {
        return !m_bSelling;
    }
    inline bool IsAsk()
    {
        return m_bSelling;
    }

    bool IsMarketOrder() const;
    bool IsLimitOrder() const;

    // Stores a pointer to theTrade for later use. (Not responsible to clean up,
    // just convenient.)
    inline OTTrade* GetTrade()
    {
        return m_pTrade;
    }
    inline void SetTrade(const OTTrade& theTrade)
    {
        m_pTrade = &(const_cast<OTTrade&>(theTrade));
    }
    // Note: m_tDateAddedToMarket is not saved in the Offer Contract, but
    // OTMarket sets/saves/loads it.
    //
    EXPORT time64_t GetDateAddedToMarket() const; // Used in
                                                  // OTMarket::GetOfferList and
                                                  // GetNymOfferList.
    EXPORT void SetDateAddedToMarket(time64_t tDate); // Used in OTCron when
                                                      // adding/loading offers.
    EXPORT OTOffer(); // The constructor contains the 3 variables needed to
                      // identify any market.
    EXPORT OTOffer(const Identifier& NOTARY_ID,
                   const Identifier& INSTRUMENT_DEFINITION_ID,
                   const Identifier& CURRENCY_ID, const int64_t& MARKET_SCALE);
    EXPORT virtual ~OTOffer();

    // Overridden from Contract.
    void GetIdentifier(Identifier& theIdentifier) const override;

    void InitOffer();

    void Release() override;
    void Release_Offer();

    // return -1 if error, 0 if nothing, and 1 if the node was processed.
    int32_t ProcessXMLNode(irr::io::IrrXMLReader*& xml) override;

    void UpdateContents() override; // Before transmission or serialization, this
                                   // is where the ledger saves its contents
};

} // namespace opentxs

#endif // OPENTXS_CORE_TRADE_OTOFFER_HPP
