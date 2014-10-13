/************************************************************
 *
 *  OTOffer.cpp
 *
 */

/************************************************************
 -----BEGIN PGP SIGNED MESSAGE-----
 Hash: SHA1

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
 *  Copyright (C) 2010-2013 by "Fellow Traveler" (A pseudonym)
 *
 *  EMAIL:
 *  FellowTraveler@rayservers.net
 *
 *  BITCOIN:  1NtTPVVjDsUfDWybS4BwvHpG2pdS9RnYyQ
 *
 *  KEY FINGERPRINT (PGP Key in license file):
 *  9DD5 90EB 9292 4B48 0484  7910 0308 00ED F951 BB8E
 *
 *  OFFICIAL PROJECT WIKI(s):
 *  https://github.com/FellowTraveler/Moneychanger
 *  https://github.com/FellowTraveler/Open-Transactions/wiki
 *
 *  WEBSITE:
 *  http://www.OpenTransactions.org/
 *
 *  Components and licensing:
 *   -- Moneychanger..A Java client GUI.....LICENSE:.....GPLv3
 *   -- otlib.........A class library.......LICENSE:...LAGPLv3
 *   -- otapi.........A client API..........LICENSE:...LAGPLv3
 *   -- opentxs/ot....Command-line client...LICENSE:...LAGPLv3
 *   -- otserver......Server Application....LICENSE:....AGPLv3
 *  Github.com/FellowTraveler/Open-Transactions/wiki/Components
 *
 *  All of the above OT components were designed and written by
 *  Fellow Traveler, with the exception of Moneychanger, which
 *  was contracted out to Vicky C (bitcointrader4@gmail.com).
 *  The open-source community has since actively contributed.
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This program is free software: you can redistribute it
 *   and/or modify it under the terms of the GNU Affero
 *   General Public License as published by the Free Software
 *   Foundation, either version 3 of the License, or (at your
 *   option) any later version.
 *
 *   ADDITIONAL PERMISSION under the GNU Affero GPL version 3
 *   section 7: (This paragraph applies only to the LAGPLv3
 *   components listed above.) If you modify this Program, or
 *   any covered work, by linking or combining it with other
 *   code, such other code is not for that reason alone subject
 *   to any of the requirements of the GNU Affero GPL version 3.
 *   (==> This means if you are only using the OT API, then you
 *   don't have to open-source your code--only your changes to
 *   Open-Transactions itself must be open source. Similar to
 *   LGPLv3, except it applies to software-as-a-service, not
 *   just to distributing binaries.)
 *
 *   Extra WAIVER for OpenSSL, Lucre, and all other libraries
 *   used by Open Transactions: This program is released under
 *   the AGPL with the additional exemption that compiling,
 *   linking, and/or using OpenSSL is allowed. The same is true
 *   for any other open source libraries included in this
 *   project: complete waiver from the AGPL is hereby granted to
 *   compile, link, and/or use them with Open-Transactions,
 *   according to their own terms, as long as the rest of the
 *   Open-Transactions terms remain respected, with regard to
 *   the Open-Transactions code itself.
 *
 *   Lucre License:
 *   This code is also "dual-license", meaning that Ben Lau-
 *   rie's license must also be included and respected, since
 *   the code for Lucre is also included with Open Transactions.
 *   See Open-Transactions/src/otlib/lucre/LUCRE_LICENSE.txt
 *   The Laurie requirements are light, but if there is any
 *   problem with his license, simply remove the Lucre code.
 *   Although there are no other blind token algorithms in Open
 *   Transactions (yet. credlib is coming), the other functions
 *   will continue to operate.
 *   See Lucre on Github:  https://github.com/benlaurie/lucre
 *   -----------------------------------------------------
 *   You should have received a copy of the GNU Affero General
 *   Public License along with this program.  If not, see:
 *   http://www.gnu.org/licenses/
 *
 *   If you would like to use this software outside of the free
 *   software license, please contact FellowTraveler.
 *   (Unfortunately many will run anonymously and untraceably,
 *   so who could really stop them?)
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will be
 *   useful, but WITHOUT ANY WARRANTY; without even the implied
 *   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *   PURPOSE.  See the GNU Affero General Public License for
 *   more details.

 -----BEGIN PGP SIGNATURE-----
 Version: GnuPG v1.4.9 (Darwin)

 iQIcBAEBAgAGBQJRSsfJAAoJEAMIAO35UbuOQT8P/RJbka8etf7wbxdHQNAY+2cC
 vDf8J3X8VI+pwMqv6wgTVy17venMZJa4I4ikXD/MRyWV1XbTG0mBXk/7AZk7Rexk
 KTvL/U1kWiez6+8XXLye+k2JNM6v7eej8xMrqEcO0ZArh/DsLoIn1y8p8qjBI7+m
 aE7lhstDiD0z8mwRRLKFLN2IH5rAFaZZUvj5ERJaoYUKdn4c+RcQVei2YOl4T0FU
 LWND3YLoH8naqJXkaOKEN4UfJINCwxhe5Ke9wyfLWLUO7NamRkWD2T7CJ0xocnD1
 sjAzlVGNgaFDRflfIF4QhBx1Ddl6wwhJfw+d08bjqblSq8aXDkmFA7HeunSFKkdn
 oIEOEgyj+veuOMRJC5pnBJ9vV+7qRdDKQWaCKotynt4sWJDGQ9kWGWm74SsNaduN
 TPMyr9kNmGsfR69Q2Zq/FLcLX/j8ESxU+HYUB4vaARw2xEOu2xwDDv6jt0j3Vqsg
 x7rWv4S/Eh18FDNDkVRChiNoOIilLYLL6c38uMf1pnItBuxP3uhgY6COm59kVaRh
 nyGTYCDYD2TK+fI9o89F1297uDCwEJ62U0Q7iTDp5QuXCoxkPfv8/kX6lS6T3y9G
 M9mqIoLbIQ1EDntFv7/t6fUTS2+46uCrdZWbQ5RjYXdrzjij02nDmJAm2BngnZvd
 kamH0Y/n11lCvo1oQxM+
 =uSzz
 -----END PGP SIGNATURE-----
 **************************************************************/

#include <opentxs/core/stdafx.hpp>
#include <opentxs/core/trade/OTOffer.hpp>
#include <opentxs/core/OTLog.hpp>

#include <irrxml/irrXML.hpp>

#include <cstring>

// Each instance of OTOffer represents a Bid or Ask. (A Market has a list of bid
// offers and a list of ask offers.)

// Also allows for x == 1.
//

namespace opentxs
{

bool OTOffer::isPowerOfTen(const int64_t& x)
{
    if (1 == x) return true;

    const int64_t lBase = 10;
    int64_t lIt = lBase;

    for (int32_t i = 1; i < 23; i++) {
        if (x == lIt) return true;
        lIt *= lBase;
    }

    return false;
}

/*
 Let's say you wanted to add an Offer to a Market. But you don't know
 which market.  There are different markets for different combinations
 of asset and currency. There are also higher and lower level markets
 for different trade minimums.

 The server has to be able to match up your Offer to the Right Market,
 so that it can trade with similar offers.

 So in this method, I combine the Asset Type ID, the Currency Type ID,
 and the minimum increment, and use them to generate a UNIQUE ID, which
 will also be the same, given the same input.

 That is the ID I will use for looking up the offers on the market.
 Basically it's the Market ID, and the Offer just has the SAME ID,
 and that's how you match it up to the market.

 (This is analogous to how Transactions and Transaction Items have the
 same transaction number.)

 THIS MEANS that the user cannot simply set his minimum increment to
 a "divide-into equally" with the market minimum increment. Why not?
 Because since his number will be different from the next guy, they
 will calculate different IDs and thus end up on different markets.

 TODO: therefore the user MUST SUPPLY the EXACT SAME minimum increment
 of the market he wishes to trade on. There's no other way. However,
 I CAN allow the user to ALSO provide a second minimum, which must be
 a multiple of the first.

 TODO: Should add this same method to the Market object as well.


 To use OTOffer::GetIdentifier is simple:

 void blah (OTOffer& theOffer)
 {
    OTIdentifier MARKET_ID(theOffer); // the magic happens right here.

    // (Done.)
 }
 */
void OTOffer::GetIdentifier(OTIdentifier& theIdentifier) const
{
    OTString strTemp, strAsset(GetAssetID()), strCurrency(GetCurrencyID());

    int64_t lScale = GetScale();

    // In this way we generate a unique ID that will always be consistent
    // for the same asset ID, currency ID, and market scale.
    strTemp.Format("ASSET TYPE:\n%s\nCURRENCY TYPE:\n%s\nMARKET SCALE:\n%lld\n",
                   strAsset.Get(), strCurrency.Get(), lScale);

    theIdentifier.CalculateDigest(strTemp);
}

bool OTOffer::IsMarketOrder() const
{
    return (0 == GetPriceLimit());
}

bool OTOffer::IsLimitOrder() const
{
    return (0 != GetPriceLimit());
}

// return -1 if error, 0 if nothing, and 1 if the node was processed.
int32_t OTOffer::ProcessXMLNode(irr::io::IrrXMLReader*& xml)
{
    int32_t nReturnVal = 0;

    // Here we call the parent class first.
    // If the node is found there, or there is some error,
    // then we just return either way.  But if it comes back
    // as '0', then nothing happened, and we'll continue executing.
    //
    // -- Note you can choose not to call the parent if
    // you don't want to use any of those xml tags.
    // As I do below, in the case of OTAccount.
    // if (nReturnVal = OTContract::ProcessXMLNode(xml))
    //    return nReturnVal;

    if (!strcmp("marketOffer", xml->getNodeName())) {
        m_strVersion = xml->getAttributeValue("version");

        OTString strIsSelling;
        strIsSelling = xml->getAttributeValue("isSelling");
        if (strIsSelling.Compare("true"))
            m_bSelling = true;
        else
            m_bSelling = false;

        m_strContractType.Set((m_bSelling ? "ASK" : "BID"));

        const OTString strServerID(xml->getAttributeValue("serverID")),
            strAssetTypeID(xml->getAttributeValue("assetTypeID")),
            strCurrencyTypeID(xml->getAttributeValue("currencyTypeID"));

        const OTIdentifier SERVER_ID(strServerID), ASSET_ID(strAssetTypeID),
            CURRENCY_TYPE_ID(strCurrencyTypeID);

        SetServerID(SERVER_ID);
        SetAssetID(ASSET_ID);
        SetCurrencyID(CURRENCY_TYPE_ID);

        const OTString strScale = xml->getAttributeValue("marketScale");
        const int64_t lScale = strScale.Exists() ? atol(strScale.Get())
                                                 : 0; // if it doesn't exist,
                                                      // the 0 here causes the
                                                      // below error to fire.

        if (!isPowerOfTen(lScale)) {
            otOut << "OTOffer::ProcessXMLNode: Failure: marketScale *must* be "
                     "1, or a power of 10. Instead I got: " << lScale << ".\n";
            return (-1);
        }
        else
            SetScale(lScale);

        const OTString strPriceLimit = xml->getAttributeValue("priceLimit");
        const int64_t lPriceLimit = strPriceLimit.Exists()
                                        ? atol(strPriceLimit.Get())
                                        : 0; // if it doesn't exist, the 0 here
                                             // causes the below error to fire.

        // NOTE: Market Orders (new) have a 0 price, so this error condition was
        // changed.
        if (!strPriceLimit.Exists())
        //      if (lPriceLimit < 1)
        {
            otOut << "OTOffer::ProcessXMLNode: Failure: priceLimit *must* be "
                     "provided(" << lPriceLimit << ").\n";
            return (-1);
        }
        else
            SetPriceLimit(lPriceLimit);

        const OTString strTotal = xml->getAttributeValue("totalAssetsOnOffer");
        const int64_t lTotal = strTotal.Exists() ? atol(strTotal.Get())
                                                 : 0; // if it doesn't exist,
                                                      // the 0 here causes the
                                                      // below error to fire.
        if (lTotal < 1) {
            otOut << "OTOffer::ProcessXMLNode: Failure: totalAssetsOnOffer "
                     "*must* be larger than 0. Instead I got: " << lTotal
                  << ".\n";
            return (-1);
        }
        else
            SetTotalAssetsOnOffer(lTotal);

        const OTString strFinished = xml->getAttributeValue("finishedSoFar");
        const int64_t lFinished = strFinished.Exists()
                                      ? atol(strFinished.Get())
                                      : 0; // if it doesn't exist, the 0 here
                                           // causes the below error to fire.
        if (lFinished < 0) {
            otOut << "OTOffer::ProcessXMLNode: Failure: finishedSoFar *must* "
                     "be 0 or larger. Instead I got: " << lFinished << ".\n";
            return (-1);
        }
        else
            SetFinishedSoFar(lFinished);

        const OTString strMinInc = xml->getAttributeValue("minimumIncrement");
        const int64_t lMinInc = strMinInc.Exists() ? atol(strMinInc.Get())
                                                   : 0; // if it doesn't exist,
                                                        // the 0 here causes the
                                                        // below error to fire.
        if ((lMinInc < 1) || (lMinInc > lTotal)) // Minimum increment cannot
                                                 // logically be higher than the
                                                 // total assets on offer...
        {
            otOut << "OTOffer::ProcessXMLNode: Failure: minimumIncrement "
                     "*must* be 1 or larger, \n"
                     "and must also be less than the total assets on offer. "
                     "Instead I got: " << lMinInc << ".\n";
            return (-1);
        }
        else
            SetMinimumIncrement(lMinInc);

        const OTString strTransNum = xml->getAttributeValue("transactionNum");
        const int64_t lTransNum =
            strTransNum.Exists() ? atol(strTransNum.Get()) : 0;

        SetTransactionNum(lTransNum);

        const OTString str_valid_from = xml->getAttributeValue("validFrom");
        const OTString str_valid_to = xml->getAttributeValue("validTo");

        int64_t tValidFrom =
            str_valid_from.Exists() ? str_valid_from.ToLong() : 0;
        int64_t tValidTo = str_valid_to.Exists() ? str_valid_to.ToLong() : 0;

        if ((tValidTo < tValidFrom) && (tValidTo != 0)) {
            otOut << "OTOffer::" << __FUNCTION__ << ": Failure: validTo date ("
                  << tValidFrom << ") cannot be earlier than "
                                   "validFrom date (" << tValidTo << ").\n";
            return (-1);
        }

        SetValidFrom(OTTimeGetTimeFromSeconds(tValidFrom));
        SetValidTo(OTTimeGetTimeFromSeconds(tValidTo));

        otLog4 << "\n\nOffer. Transaction Number: " << m_lTransactionNum
               << "\n Valid From: " << tValidFrom << "\n Valid To: " << tValidTo
               << "\n"
                  " AssetTypeID: " << strAssetTypeID
               << "\n  CurrencyTypeID: " << strCurrencyTypeID
               << "\n ServerID: " << strServerID
               << "\n"
                  " Price Limit: " << GetPriceLimit()
               << ",  Total Assets on Offer: " << GetTotalAssetsOnOffer()
               << ",  " << (m_bSelling ? "sold" : "bought")
               << " so far: " << GetFinishedSoFar() << "\n "
                                                       " Scale: " << GetScale()
               << ".   Minimum Increment: " << GetMinimumIncrement()
               << ".  This offer is a" << (m_bSelling ? "n ASK" : " BID")
               << ".\n";

        nReturnVal = 1;
    }

    return nReturnVal;
}

void OTOffer::UpdateContents()
{
    const OTString SERVER_ID(GetServerID()), ASSET_TYPE_ID(GetAssetID()),
        CURRENCY_TYPE_ID(GetCurrencyID());

    const int64_t lFrom = OTTimeGetSecondsFromTime(GetValidFrom());
    const int64_t lTo = OTTimeGetSecondsFromTime(GetValidTo());
    const int64_t lPriceLimit = GetPriceLimit();
    const int64_t lTotalAssetsOnOffer = GetTotalAssetsOnOffer();
    const int64_t lFinishedSoFar = GetFinishedSoFar();
    const int64_t lScale = GetScale();
    const int64_t lMinimumIncrement = GetMinimumIncrement();
    const int64_t lTransactionNum = GetTransactionNum();

    // I release this because I'm about to repopulate it.
    m_xmlUnsigned.Release();

    m_xmlUnsigned.Concatenate("<?xml version=\"%s\"?>\n\n", "1.0");

    m_xmlUnsigned.Concatenate(
        "<marketOffer\n version=\"%s\"\n"
        " isSelling=\"%s\"\n" // true or false.
        " serverID=\"%s\"\n"
        " assetTypeID=\"%s\"\n"
        " currencyTypeID=\"%s\"\n"
        " priceLimit=\"%lld\"\n"
        " totalAssetsOnOffer=\"%lld\"\n"
        " finishedSoFar=\"%lld\"\n"
        " marketScale=\"%lld\"\n"
        " minimumIncrement=\"%lld\"\n"
        " transactionNum=\"%lld\"\n"
        " validFrom=\"%" PRId64 "\"\n"
        " validTo=\"%" PRId64 "\""
        " />\n\n", //  <=== the tag ends here.
        m_strVersion.Get(),
        (IsBid() ? "false" : "true"), SERVER_ID.Get(), ASSET_TYPE_ID.Get(),
        CURRENCY_TYPE_ID.Get(), lPriceLimit, lTotalAssetsOnOffer,
        lFinishedSoFar, lScale, lMinimumIncrement, lTransactionNum, lFrom, lTo);

    //    m_xmlUnsigned.Concatenate("</marketOffer>\n");    // ^^^ Tag already
    // ended above.
}

bool OTOffer::MakeOffer(
    bool bBuyingOrSelling,      // True == SELLING, False == BUYING
    const int64_t& lPriceLimit, // Per Minimum Increment... (Zero price means
                                // it's a market order.)
    const int64_t& lTotalAssetsOffer, // Total assets available for sale or
                                      // purchase.
    const int64_t& lMinimumIncrement, // The minimum increment that must be
                                      // bought or sold for each transaction
    const int64_t& lTransactionNum,   // The transaction number authorizing this
                                      // trade.
    const time64_t& VALID_FROM,       // defaults to RIGHT NOW
    const time64_t& VALID_TO)         // defaults to 24 hours (a "Day Order")
{
    m_bSelling = bBuyingOrSelling; // Bid or Ask?
    SetTransactionNum(lTransactionNum);
    SetTotalAssetsOnOffer(lTotalAssetsOffer); // 500 bushels for sale.

    m_strContractType.Set((m_bSelling ? "ASK" : "BID"));

    // Make sure minimum increment isn't bigger than total Assets.
    // (If you pass them into this function as the same value, it's functionally
    // a "FILL OR KILL" order.)
    int64_t lRealMinInc = lMinimumIncrement;
    if (lMinimumIncrement > lTotalAssetsOffer) // Once the total, minus finish
                                               // so far, is smaller than the
                                               // minimum increment,
        lRealMinInc = lTotalAssetsOffer; // then the OTTrade object I am linked
                                         // to will expire and remove me from
                                         // the market.
    // OR it could set the minimum increment to the remainder. But then need to
    // calc price.

    SetMinimumIncrement(lRealMinInc); // Must sell in 50 bushel increments.
                                      // (Perhaps on the 10-bushel market it
                                      // will sell in 5 increments of 10.)
    SetPriceLimit(lPriceLimit);       // Won't sell for any less than $10 per
    // increment. (Always get best market price.)
    SetFinishedSoFar(0); // So far have already sold 350 bushels. Actual amount
                         // available is (total - finished).

    time64_t REAL_VALID_FROM = VALID_FROM;
    time64_t REAL_VALID_TO = VALID_TO;

    if (OT_TIME_ZERO >= VALID_FROM) {
        REAL_VALID_FROM =
            OTTimeGetCurrentTime(); // This time is set to TODAY NOW
    }

    if (OT_TIME_ZERO >= VALID_TO) {
        // (All offers default to a "DAY ORDER" if valid dates not specified.)
        REAL_VALID_TO = OTTimeAddTimeInterval(
            REAL_VALID_FROM,
            OTTimeGetSecondsFromTime(OT_TIME_DAY_IN_SECONDS)); // 1 day.
    }

    SetValidFrom(REAL_VALID_FROM);
    SetValidTo(REAL_VALID_TO);

    return true;
}

// Note: m_tDateAddedToMarket is not saved in the Offer Contract, but OTMarket
// sets/saves/loads it.
//
time64_t OTOffer::GetDateAddedToMarket() const // Used in OTMarket::GetOfferList
                                               // and GetNymOfferList.
{
    return m_tDateAddedToMarket;
}

void OTOffer::SetDateAddedToMarket(time64_t tDate) // Used in OTCron when
                                                   // adding/loading offers.
{
    m_tDateAddedToMarket = tDate;
}

OTOffer::OTOffer()
    : ot_super()
    , m_tDateAddedToMarket(OT_TIME_ZERO)
    , m_pTrade(nullptr)
    , // No need to free m_pTrade, not responsible. Only here for convenience.
    m_bSelling(false)
    , m_lPriceLimit(0)
    , m_lTransactionNum(0)
    , m_lTotalAssetsOffer(0)
    , m_lFinishedSoFar(0)
    , m_lScale(1)
    , // This must be 1 or greater. CANNOT be zero. Enforced.
    m_lMinimumIncrement(
        1) // This must be 1 or greater. CANNOT be zero. Enforced.
{
    InitOffer();
}

OTOffer::OTOffer(const OTIdentifier& SERVER_ID, const OTIdentifier& ASSET_ID,
                 const OTIdentifier& CURRENCY_ID, const int64_t& lScale)
    : ot_super(SERVER_ID, ASSET_ID)
    , m_tDateAddedToMarket(OT_TIME_ZERO)
    , m_pTrade(nullptr)
    , // No need to free m_pTrade, not responsible. Only here for convenience.
    m_bSelling(false)
    , m_lPriceLimit(0)
    , m_lTransactionNum(0)
    , m_lTotalAssetsOffer(0)
    , m_lFinishedSoFar(0)
    , m_lScale(1)
    , // This must be 1 or greater. CANNOT be zero. Enforced.
    m_lMinimumIncrement(
        1) // This must be 1 or greater. CANNOT be zero. Enforced.
{
    InitOffer();

    SetScale(lScale);

    m_CURRENCY_TYPE_ID = CURRENCY_ID;
}

OTOffer::~OTOffer()
{
    Release_Offer();
}

void OTOffer::Release_Offer()
{
    // If there were any dynamically allocated objects, clean them up here.
    m_CURRENCY_TYPE_ID.clear();
}

void OTOffer::Release()
{
    // If there were any dynamically allocated objects, clean them up here.
    Release_Offer();

    ot_super::Release(); // since I've overridden the base class, I call it
                         // now...

    // Then I call this to re-initialize everything
    InitOffer();
}

void OTOffer::InitOffer()
{
    m_strContractType.Set(
        "OFFER"); // in practice should never appear. BID/ASK will overwrite.

    // This pointer will get wiped anytime Release() is called... which means
    // anytime LoadContractFromString()
    // is called. For some objects, that screws them up because suddenly the
    // pointer went nullptr when they needed it.
    // In the case of this object, the pointer is reset whenever Cron processes,
    // so this is safe. But in
    // the case of other objects, it's better not to initialize the pointer
    // here, but in the constructor instead.
    // FYI. For example, OTCron has a pointer to m_pServerNym. LoadCron() and
    // the pointer is nullptr. Can't have that!
    // So I moved it to the constructor in that case.

    m_bSelling = false;
    m_lPriceLimit = 0;
    m_lTransactionNum = 0;
    m_lTotalAssetsOffer = 0;
    m_lFinishedSoFar = 0;
    m_lMinimumIncrement = 1; // This must be 1 or greater. Enforced.
    SetScale(1);             // This must be 1 or greater. Enforced.
}

} // namespace opentxs
