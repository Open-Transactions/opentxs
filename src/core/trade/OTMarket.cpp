/************************************************************
 *
 *  OTMarket.cpp
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

#include <opentxs/core/trade/OTMarket.hpp>
#include <opentxs/core/trade/OTOffer.hpp>
#include <opentxs/core/trade/OTTrade.hpp>
#include <opentxs/core/OTAccount.hpp>
#include <opentxs/core/OTLedger.hpp>
#include <opentxs/core/OTLog.hpp>
#include <opentxs/core/OTPseudonym.hpp>
#include <opentxs/core/util/OTFolders.hpp>

#include <irrxml/irrXML.hpp>

#include <memory>

// return -1 if error, 0 if nothing, and 1 if the node was processed.

namespace opentxs
{

int32_t OTMarket::ProcessXMLNode(irr::io::IrrXMLReader*& xml)
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

    if (!strcmp("market", xml->getNodeName())) {
        m_strVersion = xml->getAttributeValue("version");
        SetScale(atol(xml->getAttributeValue("marketScale")));
        m_lLastSalePrice = atol(xml->getAttributeValue("lastSalePrice"));
        m_strLastSaleDate = xml->getAttributeValue("lastSaleDate");

        const OTString strServerID(xml->getAttributeValue("serverID")),
            strAssetTypeID(xml->getAttributeValue("assetTypeID")),
            strCurrencyTypeID(xml->getAttributeValue("currencyTypeID"));

        m_SERVER_ID.SetString(strServerID);
        m_ASSET_TYPE_ID.SetString(strAssetTypeID);
        m_CURRENCY_TYPE_ID.SetString(strCurrencyTypeID);

        otOut << "\n\nMarket. Scale: " << m_lScale << "\n";

        otWarn << " assetTypeID: " << strAssetTypeID
               << "\n"
                  " currencyTypeID: " << strCurrencyTypeID
               << "\n"
                  " ServerID: " << strServerID << "\n";

        nReturnVal = 1;
    }
    else if (!strcmp("offer", xml->getNodeName())) {
        const OTString strDateAdded(xml->getAttributeValue("dateAdded"));
        const int64_t lDateAdded =
            strDateAdded.Exists() ? strDateAdded.ToLong() : 0;
        const time64_t tDateAdded = OTTimeGetTimeFromSeconds(lDateAdded);

        OTString strData;

        if (!OTContract::LoadEncodedTextField(xml, strData) ||
            !strData.Exists()) {
            otErr << "Error in OTMarket::" << __FUNCTION__
                  << ": offer field without value.\n";
            return (-1); // error condition
        }
        else {
            OTOffer* pOffer = new OTOffer(m_SERVER_ID, m_ASSET_TYPE_ID,
                                          m_CURRENCY_TYPE_ID, m_lScale);

            OT_ASSERT(nullptr != pOffer);

            if (pOffer->LoadContractFromString(strData) &&
                AddOffer(nullptr, *pOffer, false, tDateAdded)) // bSaveMarket =
            // false (Don't SAVE
            // -- we're loading
            // right now!)
            {
                otWarn << "Successfully loaded offer and added to market.\n";
            }
            else {
                otErr << "Error adding offer to market while loading market.\n";
                delete pOffer;
                pOffer = nullptr;
                return (-1);
            }
        }

        nReturnVal = 1;
    }

    return nReturnVal;
}

void OTMarket::UpdateContents()
{
    // I release this because I'm about to repopulate it.
    m_xmlUnsigned.Release();

    m_xmlUnsigned.Concatenate("<?xml version=\"%s\"?>\n\n", "1.0");

    const OTString SERVER_ID(m_SERVER_ID), ASSET_TYPE_ID(m_ASSET_TYPE_ID),
        CURRENCY_TYPE_ID(m_CURRENCY_TYPE_ID);

    m_xmlUnsigned.Concatenate("<market\n version=\"%s\"\n"
                              " serverID=\"%s\"\n"
                              " assetTypeID=\"%s\"\n"
                              " currencyTypeID=\"%s\"\n"
                              " marketScale=\"%lld\"\n"
                              " lastSaleDate=\"%s\"\n"
                              " lastSalePrice=\"%lld\""
                              " >\n\n",
                              m_strVersion.Get(), SERVER_ID.Get(),
                              ASSET_TYPE_ID.Get(), CURRENCY_TYPE_ID.Get(),
                              m_lScale, m_strLastSaleDate.c_str(),
                              m_lLastSalePrice);

    // Save the offers for sale.
    for (auto& it : m_mapAsks) {
        OTOffer* pOffer = it.second;
        OT_ASSERT(nullptr != pOffer);

        OTString strOffer(
            *pOffer); // Extract the offer contract into string form.
        OTASCIIArmor ascOffer(strOffer); // Base64-encode that for storage.

        int64_t lDateAddedToMarket =
            OTTimeGetSecondsFromTime(pOffer->GetDateAddedToMarket());

        m_xmlUnsigned.Concatenate("<offer dateAdded=\"%" PRId64
                                  "\">\n%s</offer>\n\n",
                                  lDateAddedToMarket, ascOffer.Get());
    }

    // Save the bids.
    for (auto& it : m_mapBids) {
        OTOffer* pOffer = it.second;
        OT_ASSERT(nullptr != pOffer);

        OTString strOffer(
            *pOffer); // Extract the offer contract into string form.
        OTASCIIArmor ascOffer(strOffer); // Base64-encode that for storage.

        int64_t lDateAddedToMarket =
            OTTimeGetSecondsFromTime(pOffer->GetDateAddedToMarket());

        m_xmlUnsigned.Concatenate("<offer dateAdded=\"%" PRId64
                                  "\">\n%s</offer>\n\n",
                                  lDateAddedToMarket, ascOffer.Get());
    }

    m_xmlUnsigned.Concatenate("</market>\n");
}

int64_t OTMarket::GetTotalAvailableAssets()
{
    int64_t lTotal = 0;

    for (auto& it : m_mapAsks) {
        OTOffer* pOffer = it.second;
        OT_ASSERT(nullptr != pOffer);

        lTotal += pOffer->GetAmountAvailable();
    }

    return lTotal;
}

// Get list of offers for a particular Nym, to send that Nym
//
bool OTMarket::GetNym_OfferList(const OTIdentifier& NYM_ID,
                                OTDB::OfferListNym& theOutputList,
                                int32_t& nNymOfferCount)
{
    nNymOfferCount =
        0; // Outputs the count of offers for NYM_ID (on this market.)

    // Loop through the offers, up to some maximum depth, and then add each
    // as a data member to an offer list, then pack it into ascOutput.
    //
    for (auto& it : m_mapOffers) {
        OTOffer* pOffer = it.second;
        OT_ASSERT(nullptr != pOffer);

        OTTrade* pTrade = pOffer->GetTrade();

        // We only return offers for a specific Nym ID, since this is private
        // info only for that Nym.
        //
        if ((nullptr == pTrade) || (pTrade->GetSenderUserID() != NYM_ID))
            continue;

        // Below this point, I KNOW pTrade and pOffer are both good pointers.
        // with no need to cleanup. I also know they are for the right Nym.

        std::unique_ptr<OTDB::OfferDataNym> pOfferData(
            dynamic_cast<OTDB::OfferDataNym*>(
                OTDB::CreateObject(OTDB::STORED_OBJ_OFFER_DATA_NYM)));

        const int64_t& lTransactionNum = pOffer->GetTransactionNum();
        const int64_t& lPriceLimit = pOffer->GetPriceLimit();
        const int64_t& lTotalAssets = pOffer->GetTotalAssetsOnOffer();
        const int64_t& lFinishedSoFar = pOffer->GetFinishedSoFar();
        const int64_t& lMinimumIncrement = pOffer->GetMinimumIncrement();
        const int64_t& lScale = pOffer->GetScale();

        const time64_t tValidFrom = pOffer->GetValidFrom();
        const time64_t tValidTo = pOffer->GetValidTo();

        const time64_t tDateAddedToMarket = pOffer->GetDateAddedToMarket();

        const OTIdentifier& theServerID = pOffer->GetServerID();
        const OTString strServerID(theServerID);
        const OTIdentifier& theAssetID = pOffer->GetAssetID();
        const OTString strAssetID(theAssetID);
        const OTIdentifier& theAssetAcctID = pTrade->GetSenderAcctID();
        const OTString strAssetAcctID(theAssetAcctID);
        const OTIdentifier& theCurrencyID = pOffer->GetCurrencyID();
        const OTString strCurrencyID(theCurrencyID);
        const OTIdentifier& theCurrencyAcctID = pTrade->GetCurrencyAcctID();
        const OTString strCurrencyAcctID(theCurrencyAcctID);

        const bool bSelling = pOffer->IsAsk();

        if (pTrade->IsStopOrder()) {
            if (pTrade->IsGreaterThan())
                pOfferData->stop_sign = ">";
            else if (pTrade->IsLessThan())
                pOfferData->stop_sign = "<";

            if (!pOfferData->stop_sign.compare(">") ||
                !pOfferData->stop_sign.compare("<")) {
                const int64_t& lStopPrice = pTrade->GetStopPrice();
                pOfferData->stop_price = to_string<int64_t>(lStopPrice);
            }
        }

        pOfferData->transaction_id = to_string<int64_t>(lTransactionNum);
        pOfferData->price_per_scale = to_string<int64_t>(lPriceLimit);
        pOfferData->total_assets = to_string<int64_t>(lTotalAssets);
        pOfferData->finished_so_far = to_string<int64_t>(lFinishedSoFar);
        pOfferData->minimum_increment = to_string<int64_t>(lMinimumIncrement);
        pOfferData->scale = to_string<int64_t>(lScale);

        pOfferData->valid_from = to_string<time64_t>(tValidFrom);
        pOfferData->valid_to = to_string<time64_t>(tValidTo);

        pOfferData->date = to_string<time64_t>(tDateAddedToMarket);

        pOfferData->server_id = strServerID.Get();
        pOfferData->asset_type_id = strAssetID.Get();
        pOfferData->asset_acct_id = strAssetAcctID.Get();
        pOfferData->currency_type_id = strCurrencyID.Get();
        pOfferData->currency_acct_id = strCurrencyAcctID.Get();

        pOfferData->selling = bSelling;

        // *pOfferData is CLONED at this time (I'm still responsible to delete.)
        // That's also why I add it here, below: So the data is set right before
        // the cloning occurs.
        //
        theOutputList.AddOfferDataNym(*pOfferData);
        nNymOfferCount++;
    }

    return true;
}

bool OTMarket::GetRecentTradeList(OTASCIIArmor& ascOutput, int32_t& nTradeCount)
{
    nTradeCount = 0; // Output the count of trades in the list being returned.
                     // (If success..)

    if (nullptr == m_pTradeList) {
        //      otErr << "OTMarket::GetRecentTradeList: m_pTradeList is nullptr.
        // \n";
        return true;
        // Returning true, since it's normal for this to be nullptr when the
        // list
        // is empty.
    }

    // The market already keeps a list of recent trades (informational only)
    //

    const size_t sizeList = m_pTradeList->GetTradeDataMarketCount();
    nTradeCount = static_cast<int32_t>(sizeList);

    if (nTradeCount == 0)
        return true; // Success, but there are 0 trade datas to return. (empty
                     // list.)

    // So now, let's pack the list into strOutput...
    else if (nTradeCount > 0) {
        OTDB::Storage* pStorage = OTDB::GetDefaultStorage();
        OT_ASSERT(nullptr != pStorage);

        OTDB::OTPacker* pPacker =
            pStorage->GetPacker(); // No need to check for failure, since this
                                   // already ASSERTS. No need to cleanup
                                   // either.

        std::unique_ptr<OTDB::PackedBuffer> pBuffer(pPacker->Pack(
            *m_pTradeList)); // Now we PACK our market's recent trades list.

        if (nullptr == pBuffer) {
            otErr << "Failed packing pTradeList in OTCron::GetRecentTradeList. "
                     "\n";
            return false;
        }

        // Now we need to translate pBuffer into strOutput.

        auto pUint = pBuffer->GetData();
        auto theSize = pBuffer->GetSize();

        if ((nullptr != pUint) || (theSize < 2)) {
            ot_data_t theData(pUint, pUint + theSize);

            // This function will base64 ENCODE theData,
            // and then Set() that as the string contents.
            ascOutput.SetData(theData);

            return true;
        }
        else
            otErr << "Error while getting buffer data in "
                     "OTMarket::GetRecentTradeList.\n";
    }
    else
        otErr << "Error: nTradeCount with negative value in "
                 "OTMarket::GetRecentTradeList: " << nTradeCount << ".\n";

    return false;
}

// OTDB::OfferListMarket
//
bool OTMarket::GetOfferList(OTASCIIArmor& ascOutput, int64_t lDepth,
                            int32_t& nOfferCount)
{
    nOfferCount = 0; // Outputs the actual count of offers being returned.

    if (0 == lDepth) lDepth = MAX_MARKET_QUERY_DEPTH;

    // Loop through the offers, up to some maximum depth, and then add each
    // as a data member to an offer list, then pack it into ascOutput.

    std::unique_ptr<OTDB::OfferListMarket> pOfferList(
        dynamic_cast<OTDB::OfferListMarket*>(
            OTDB::CreateObject(OTDB::STORED_OBJ_OFFER_LIST_MARKET)));

    //    mapOfOffers            m_mapBids;        // The buyers, ordered by
    // price limit
    //    mapOfOffers            m_mapAsks;        // The sellers, ordered by
    // price limit

    int32_t nTempDepth = 0;

    for (auto& it : m_mapBids) {
        if (nTempDepth++ > lDepth) break;

        OTOffer* pOffer = it.second;
        OT_ASSERT(nullptr != pOffer);

        const int64_t& lPriceLimit = pOffer->GetPriceLimit();

        if (0 == lPriceLimit) // Skipping any market orders.
            continue;

        // OfferDataMarket
        std::unique_ptr<OTDB::BidData> pOfferData(dynamic_cast<OTDB::BidData*>(
            OTDB::CreateObject(OTDB::STORED_OBJ_BID_DATA)));

        const int64_t& lTransactionNum = pOffer->GetTransactionNum();
        const int64_t lAvailableAssets = pOffer->GetAmountAvailable();
        const int64_t& lMinimumIncrement = pOffer->GetMinimumIncrement();
        const time64_t tDateAddedToMarket = pOffer->GetDateAddedToMarket();

        pOfferData->transaction_id = to_string<int64_t>(lTransactionNum);
        pOfferData->price_per_scale = to_string<int64_t>(lPriceLimit);
        pOfferData->available_assets = to_string<int64_t>(lAvailableAssets);
        pOfferData->minimum_increment = to_string<int64_t>(lMinimumIncrement);
        pOfferData->date = to_string<time64_t>(tDateAddedToMarket);

        // *pOfferData is CLONED at this time (I'm still responsible to delete.)
        // That's also why I add it here, below: So the data is set right before
        // the cloning occurs.
        //
        pOfferList->AddBidData(*pOfferData);
        nOfferCount++;
    }

    nTempDepth = 0;

    for (auto& it : m_mapAsks) {
        if (nTempDepth++ > lDepth) break;

        OTOffer* pOffer = it.second;
        OT_ASSERT(nullptr != pOffer);

        // OfferDataMarket
        std::unique_ptr<OTDB::AskData> pOfferData(dynamic_cast<OTDB::AskData*>(
            OTDB::CreateObject(OTDB::STORED_OBJ_ASK_DATA)));

        const int64_t& lTransactionNum = pOffer->GetTransactionNum();
        const int64_t& lPriceLimit = pOffer->GetPriceLimit();
        const int64_t lAvailableAssets = pOffer->GetAmountAvailable();
        const int64_t& lMinimumIncrement = pOffer->GetMinimumIncrement();
        const time64_t tDateAddedToMarket = pOffer->GetDateAddedToMarket();

        pOfferData->transaction_id = to_string<int64_t>(lTransactionNum);
        pOfferData->price_per_scale = to_string<int64_t>(lPriceLimit);
        pOfferData->available_assets = to_string<int64_t>(lAvailableAssets);
        pOfferData->minimum_increment = to_string<int64_t>(lMinimumIncrement);
        pOfferData->date = to_string<time64_t>(tDateAddedToMarket);

        // *pOfferData is CLONED at this time (I'm still responsible to delete.)
        // That's also why I add it here, below: So the data is set right before
        // the cloning occurs.
        //
        pOfferList->AddAskData(*pOfferData);
        nOfferCount++;
    }

    // Now pack the list into strOutput...

    if (nOfferCount == 0)
        return true; // Success, but there were zero offers found.

    if (nOfferCount > 0) {
        OTDB::Storage* pStorage = OTDB::GetDefaultStorage();
        OT_ASSERT(nullptr != pStorage);

        OTDB::OTPacker* pPacker =
            pStorage->GetPacker(); // No need to check for failure, since this
                                   // already ASSERTS. No need to cleanup
                                   // either.

        std::unique_ptr<OTDB::PackedBuffer> pBuffer(
            pPacker->Pack(*pOfferList)); // Now we PACK our market's offer list.

        if (nullptr == pBuffer) {
            otErr << "Failed packing pOfferList in OTCron::GetOfferList. \n";
            return false;
        }

        // Now we need to translate pBuffer into strOutput.

        auto pUint = pBuffer->GetData();
        auto theSize = pBuffer->GetSize();

        if (nullptr != pUint) {
            ot_data_t theData(pUint, pUint + theSize);

            // This function will base64 ENCODE theData,
            // and then Set() that as the string contents.
            ascOutput.SetData(theData);

            return true;
        }
        else
            otErr << "Error while getting buffer data in "
                     "OTMarket::GetOfferList.\n";
    }
    else
        otErr << "invalid: nOfferCount is < 0 in OTMarket::GetOfferList.\n";

    return false;
}

// To insure that offers of a specific price are always inserted at the upper
// bound,
// use this:  my_mmap.insert(my_mmap.upper_bound(key), my_mmap::value_type(key,
// val));
// This way I can read them from the lower bound later, and always get them in
// the
// order received for that price.
//
// typedef std::multimap<int64_t, OTOffer *>    mapOfOffers;
// mapOfOffers    m_mapBids; // The buyers, ordered
// mapOfOffers    m_mapAsks; // The sellers, ordered

OTOffer* OTMarket::GetOffer(const int64_t& lTransactionNum)
{
    // See if there's something there with that transaction number.
    auto it = m_mapOffers.find(lTransactionNum);

    if (it == m_mapOffers.end()) {
        // nothing found.
        return nullptr;
    }
    // Found it!
    else {
        OTOffer* pOffer = it->second;

        OT_ASSERT((nullptr != pOffer));

        if (pOffer->GetTransactionNum() == lTransactionNum)
            return pOffer;
        else
            otErr << "Expected Offer with transaction number "
                  << lTransactionNum << ", but found "
                  << pOffer->GetTransactionNum() << " inside. Bad data?\n";
    }

    return nullptr;
}

bool OTMarket::RemoveOffer(const int64_t& lTransactionNum) // if false, offer
                                                           // wasn't found.
{
    bool bReturnValue = false;

    // See if there's something there with that transaction number.
    auto it = m_mapOffers.find(lTransactionNum);

    // If it's not already on the list, then there's nothing to remove.
    if (it == m_mapOffers.end()) {
        otErr << "Attempt to remove non-existent Offer from Market. "
                 "Transaction #: " << lTransactionNum << "\n";
        return false;
    }
    // Otherwise, if it WAS already there, remove it properly.
    else {
        OTOffer* pOffer = it->second;

        OT_ASSERT(nullptr != pOffer);

        // This removes it from one list (the one indexed by transaction
        // number.)
        // But it's still on one of the other lists...
        m_mapOffers.erase(it);

        // The code operates the same whether ask or bid. Just use a pointer.
        mapOfOffers* pMap = (pOffer->IsBid() ? &m_mapBids : &m_mapAsks);

        // Future solution here: instead of looping through all the offers and
        // finding it,
        // I could just have each Offer store a copy of the iterator that's
        // returned when
        // it's first inserted. Later I just erase that iterator from the right
        // list to remove. Todo.
        OTOffer* pSameOffer = nullptr;

        for (mapOfOffers::iterator iii = pMap->begin(); iii != pMap->end();
             ++iii) {
            pSameOffer = iii->second;

            OT_ASSERT_MSG(nullptr != pSameOffer,
                          "nullptr offer pointer in OTMarket::RemoveOffer.\n");

            // found it!
            if (lTransactionNum == pSameOffer->GetTransactionNum()) {
                pMap->erase(iii);
                break;
            }

            // Later on, below this loop, this pointer will be nullptr or not,
            // and
            // I will know if it was removed.
            pSameOffer = nullptr;
        }

        if (nullptr == pSameOffer) {
            otErr << "Removed Offer from offers list, but not found on bid/ask "
                     "list.\n";
        }
        else // This means it was found and removed from the second list as
               // well.
        {
            bReturnValue = true; // Success.
        }

        // pOffer was found on the Offers list.
        // pSameOffer was found on the Bid or Ask list, with the same
        // transaction ID.
        // They SHOULD be pointers to the SAME object.
        // Therefore I CANNOT delete them both.
        //
        OT_ASSERT(pOffer == pSameOffer);

        delete pOffer;
        pOffer = nullptr;
        pSameOffer = nullptr;
    }

    if (bReturnValue)
        return SaveMarket(); // <====== SAVE since an offer was removed.
    else
        return false;
}

// This method demands an Offer reference in order to verify that it really
// exists.
// HOWEVER, it MUST be heap-allocated, since the Market takes ownership and will
// delete it.
//
// If NOT successful adding, caller must clear up his own memory.
//
bool OTMarket::AddOffer(OTTrade* pTrade, OTOffer& theOffer, bool bSaveFile,
                        time64_t tDateAddedToMarket)
{
    const int64_t lTransactionNum = theOffer.GetTransactionNum(),
                  lPriceLimit = theOffer.GetPriceLimit();

    // Make sure the offer is even appropriate for this market...
    if (!ValidateOfferForMarket(theOffer)) {
        otErr << "Failed attempt to add invalid offer to market.\n";

        if (nullptr != pTrade) pTrade->FlagForRemoval();
    }
    else {
        // I store duplicate lists of offer pointers. Two multimaps ordered by
        // price,
        // (for buyers and sellers) and one map ordered by transaction number.

        // See if there's something else already there with the same transaction
        // number.
        //
        auto it = m_mapOffers.find(lTransactionNum);

        // If it's not already on the list, then add it...
        if (it == m_mapOffers.end()) {
            m_mapOffers[lTransactionNum] = &theOffer;
            otLog4 << "Offer added as an offer to the market...\n";
        }
        // Otherwise, if it was already there, log an error.
        else {
            otErr << "Attempt to add Offer to Market with pre-existing "
                     "transaction number: " << lTransactionNum << "\n";
            return false;
        }

        // Okay so we successfully added it to one list (indexed by Transaction
        // Num) and we
        // know it validated as an offer, AND we know it wasn't already on the
        // market.
        //
        // So next, let's add it to the lists that are indexed by price:

        // Determine if it's a buy or sell, and add it to the right list.
        if (theOffer.IsBid()) {
            // No bother checking if the offer is already on this list,
            // since the code above basically already verifies that for us.

            m_mapBids.insert(
                m_mapBids.lower_bound(lPriceLimit), // highest bidders go first,
                                                    // so I am last in line at
                                                    // lower bound.
                std::pair<int64_t, OTOffer*>(lPriceLimit, &theOffer));
            otLog4 << "Offer added as a bid to the market.\n";
        }
        else {
            m_mapAsks.insert(
                m_mapAsks.upper_bound(lPriceLimit), // lowest price sells first,
                                                    // so I am last in line at
                                                    // upper bound.
                std::pair<int64_t, OTOffer*>(lPriceLimit, &theOffer));
            otLog4 << "Offer added as an ask to the market.\n";
        }

        if (bSaveFile) {
            // Set this to the current date/time, since the offer is
            // being added for the first time.
            //
            theOffer.SetDateAddedToMarket(OTTimeGetCurrentTime());

            return SaveMarket(); // <====== SAVE since an offer was added to the
                                 // Market.
        }
        else {
            // Set this to the date passed in, since this offer was
            // added to the market in the past, and we are preserving that date.
            //
            theOffer.SetDateAddedToMarket(tDateAddedToMarket);

            return true;
        }
    }

    return false;
}

bool OTMarket::LoadMarket()
{
    OT_ASSERT(nullptr != GetCron());
    OT_ASSERT(nullptr != GetCron()->GetServerNym());

    OTIdentifier MARKET_ID(*this);
    OTString str_MARKET_ID(MARKET_ID);

    const char* szFoldername = OTFolders::Market().Get();
    const char* szFilename = str_MARKET_ID.Get();

    bool bSuccess = OTDB::Exists(szFoldername, szFilename);

    if (bSuccess) bSuccess = LoadContract(szFoldername, szFilename); // todo ??

    if (bSuccess) bSuccess = VerifySignature(*(GetCron()->GetServerNym()));

    // Load the list of recent market trades (informational only.)
    //
    if (bSuccess) {
        if (nullptr != m_pTradeList) delete m_pTradeList;

        const char* szSubFolder = "recent"; // todo stop hardcoding.

        m_pTradeList = dynamic_cast<OTDB::TradeListMarket*>(OTDB::QueryObject(
            OTDB::STORED_OBJ_TRADE_LIST_MARKET, szFoldername, // markets
            szSubFolder,                                      // markets/recent
            szFilename)); // markets/recent/market_ID
    }

    return bSuccess;
}

bool OTMarket::SaveMarket()
{
    OT_ASSERT(nullptr != GetCron());
    OT_ASSERT(nullptr != GetCron()->GetServerNym());

    OTIdentifier MARKET_ID(*this);
    OTString str_MARKET_ID(MARKET_ID);

    const char* szFoldername = OTFolders::Market().Get();
    const char* szFilename = str_MARKET_ID.Get();

    // Remember, if the market has changed, the new contents will not be written
    // anywhere
    // until that market has been signed. So I have to re-sign here, or it would
    // just save
    // the old version of the market from before the most recent changes.
    ReleaseSignatures();

    // Sign it, save it internally to string, and then save that out to the
    // file.
    if (!SignContract(*(GetCron()->GetServerNym())) || !SaveContract() ||
        !SaveContract(szFoldername, szFilename)) {
        otErr << "Error saving Market:\n" << szFoldername
              << OTLog::PathSeparator() << szFilename << "\n";
        return false;
    }

    // Save a copy of recent trades.

    if (nullptr != m_pTradeList) {
        const char* szSubFolder = "recent"; // todo stop hardcoding.

        // If this fails, oh well. It's informational, anyway.
        if (!OTDB::StoreObject(*m_pTradeList, szFoldername, // markets
                               szSubFolder,                 // markets/recent
                               szFilename)) // markets/recent/Market_ID
            otErr << "Error saving recent trades for Market:\n" << szFoldername
                  << OTLog::PathSeparator() << szSubFolder
                  << OTLog::PathSeparator() << szFilename << "\n";
    }

    return true;
}

// A Market's ID is based on the asset type, the currency type, and the scale.
//
void OTMarket::GetIdentifier(OTIdentifier& theIdentifier) const
{
    OTString strTemp, strAsset(GetAssetID()), strCurrency(GetCurrencyID());

    int64_t lScale = GetScale();

    // In this way we generate a unique ID that will always be consistent
    // for the same asset ID, currency ID, and market scale.
    strTemp.Format("ASSET TYPE:\n%s\nCURRENCY TYPE:\n%s\nMARKET SCALE:\n%lld\n",
                   strAsset.Get(), strCurrency.Get(), lScale);

    theIdentifier.CalculateDigest(strTemp);
}

// returns 0 if there are no bids. Otherwise returns the value of the highest
// bid on the market.
int64_t OTMarket::GetHighestBidPrice()
{
    int64_t lPrice = 0;

    mapOfOffers::reverse_iterator rr = m_mapBids.rbegin();

    if (rr != m_mapBids.rend()) {
        lPrice = rr->first;
    }

    return lPrice;
}

// returns 0 if there are no asks. Otherwise returns the value of the lowest ask
// on the market.
int64_t OTMarket::GetLowestAskPrice()
{
    int64_t lPrice = 0;

    auto it = m_mapAsks.begin();

    if (it != m_mapAsks.end()) {
        lPrice = it->first;

        // Market orders have a 0 price, so we need to skip any if they are
        // here.
        //
        // Note that we don't have to do this with the highest bid price (above
        // function) but in the case of asks, a "0 price" will undercut the
        // other
        // actual prices, so we need to skip any that have a 0 price.
        //
        while (0 == lPrice) {
            ++it;

            if (it == m_mapAsks.end()) break;

            lPrice = it->first;
        }
    }

    return lPrice;
}

// This utility function is used directly below (only).
void OTMarket::cleanup_four_accounts(OTAccount* p1, OTAccount* p2,
                                     OTAccount* p3, OTAccount* p4)
{
    if (p1) delete p1;
    if (p2) delete p2;
    if (p3) delete p3;
    if (p4) delete p4;
}

// This utility function is used directly below (only).
// It is ASSUMED that the first two accounts are DEBITS, and the second two
// accounts are CREDITS.
// The bool is the original result of the debit or credit that was attempted
// (and is now being rolled back.)
// so for example, p1 tried to debit, and if it returned true (debit was
// successful!) then we NEED to roll
// p1 back (since that's what this function does.)
// Whereas if it had returned false (debit failed) then the false would be
// passed in here, and I would know
// NOT to try to credit the account again, since the money never left.  If b1 is
// false, for each var, do nothing.
// If true, try to roll it back.
void OTMarket::rollback_four_accounts(OTAccount& p1, bool b1, const int64_t& a1,
                                      OTAccount& p2, bool b2, const int64_t& a2,
                                      OTAccount& p3, bool b3, const int64_t& a3,
                                      OTAccount& p4, bool b4, const int64_t& a4)
{
    if (b1) p1.Credit(a1);
    if (b2) p2.Credit(a2);
    if (b3) p3.Debit(a3);
    if (b4) p4.Debit(a4);
}

// By the time this is called, we have already verified most things:
// -- The trade and offer belong to each other.
// -- theOtherOffer is a matching offer that was found to exist already on the
// market.
// -- Both offers have been verified on and to the market. (Their Asset types
// included.)
// -- theOffer MIGHT be a market order (with 0 price), but theOtherOffer
// definitely is NOT a market order.
// -- Both offers are within each other's price limit, if there is one.
// -- Both offers have enough assets for sale/purchase to meet their respective
// minimums.
// -- TheTrade will get the PriceLimit offered by theOtherOffer, and not
// vice-versa.
//    (See explanation below this function if you need to know the reasoning.)
//
// Basically, by the time you get to this function, the two offers do not need
// to be
// verified in terms of whether they can trade. They can DEFINITELY trade,
// that's why
// they're here. So any failures within here will be more related to
// successfully processing
// that trade, not determining whether to do it. (For example, perhaps one of
// the accounts
// doesn't actually have enough money to do the trade... that failure would
// occur here.)
//
void OTMarket::ProcessTrade(OTTrade& theTrade, OTOffer& theOffer,
                            OTOffer& theOtherOffer)
{
    OTTrade* pOtherTrade = theOtherOffer.GetTrade();
    OTCron* pCron = theTrade.GetCron();

    // Make sure have pointer to both trades.
    //
    OT_ASSERT_MSG(nullptr != pOtherTrade,
                  "Offer was on the market, but somehow "
                  "got into processing without a trade "
                  "pointer.\n");
    OT_ASSERT(nullptr !=
              pCron); // Also need the Cron pointer which SHOULD ALWAYS
                      // be there.

    OTPseudonym* pServerNym = pCron->GetServerNym();

    OT_ASSERT_MSG(nullptr != pServerNym,
                  "Somehow a Market is running even though "
                  "there is no Server Nym on the Cron "
                  "object authorizing the trades.");

    const OTIdentifier SERVER_ID(pCron->GetServerID());

    if (pCron->GetTransactionCount() < 1) {
        otOut << "Failed to process trades: Out of transaction numbers!\n";
        return;
    }

    // Make sure these two separate trades don't have the same Account IDs
    // inside
    // otherwise we would have to take care not to load them twice, like with
    // the Nyms below.
    // (Instead I just disallow the trade entirely.)
    //
    if ((theTrade.GetSenderAcctID() == pOtherTrade->GetSenderAcctID()) ||
        (theTrade.GetSenderAcctID() == pOtherTrade->GetCurrencyAcctID()) ||
        (theTrade.GetCurrencyAcctID() == pOtherTrade->GetSenderAcctID()) ||
        (theTrade.GetCurrencyAcctID() == pOtherTrade->GetCurrencyAcctID())) {
        otLog5 << "Failed to process trades: they had account IDs in common.\n";

        // No need to remove either of the trades since they might still be
        // valid when
        // matched up to others.
        // I put the log on the most verbose level because if this happens, it
        // will probably
        // happen over and over again. (Unless the market has enough depth to
        // process them
        // both out.)

        // TODO May need to choose (or add a config option) so server operators
        // can remove the
        // oldest or newest trade when this happens, or both. Or it can choose
        // to do nothing,
        // and let them both process out with other trades (as this does now.)

        return;
    }
    // No need to compare asset types, since those are already verified by the
    // time
    // we get here. BUT, when the accounts are actually loaded up, THEN should
    // compare
    // the asset types to make sure they were what we expected them to be.

    // -------------- Make sure have both nyms loaded and checked out.
    // --------------------------------------------------
    // WARNING: 1 or both of the Nyms could be also the Server Nym. They could
    // also be the same Nym, but NOT the Server.
    // In all of those different cases, I don't want to load the same file twice
    // and overwrite it with itself, losing
    // half of all my changes. I have to check all three IDs carefully and set
    // the pointers accordingly, and then operate
    // using the pointers from there.

    const OTIdentifier FIRST_NYM_ID(
        theTrade.GetSenderUserID()),                  // The newest trade's Nym.
        OTHER_NYM_ID(pOtherTrade->GetSenderUserID()), // The Nym of the trade
                                                      // that was already on the
                                                      // market. (Could be same
                                                      // Nym.)
        SERVER_NYM_ID(
            *pServerNym); // The Server Nym (could be one or both of the above.)

    OTPseudonym theNym,
        theOtherNym; // We MIGHT use ONE, OR BOTH, of these, or none.

    // Find out if either Nym is actually also the server.
    bool bFirstNymIsServerNym =
        ((FIRST_NYM_ID == SERVER_NYM_ID) ? true : false);
    bool bOtherNymIsServerNym =
        ((OTHER_NYM_ID == SERVER_NYM_ID) ? true : false);

    // We also see, after all that is done, whether both pointers go to the same
    // entity. We'll want to know that later.
    bool bTradersAreSameNym = ((FIRST_NYM_ID == OTHER_NYM_ID) ? true : false);

    // Initially both nym pointers are set to their own blank objects
    OTPseudonym* pFirstNym = nullptr;
    OTPseudonym* pOtherNym = nullptr;

    // Unless either of them is actually the server,
    // in which case the pointer is re-pointed to the server Nym.
    if (bFirstNymIsServerNym) // If the First Nym is the server, then just point
                              // to that.
    {
        pFirstNym = pServerNym;
    }
    else // Else load the First Nym from storage.
    {
        theNym.SetIdentifier(FIRST_NYM_ID); // theNym is pFirstNym

        if (!theNym.LoadPublicKey()) {
            OTString strNymID(FIRST_NYM_ID);
            otErr << "Failure loading First Nym public key in OTMarket::"
                  << __FUNCTION__ << ": " << strNymID << "\n";
            theTrade.FlagForRemoval();
            return;
        }

        if (theNym.VerifyPseudonym() && theTrade.VerifySignature(*pServerNym) &&
            theOffer.VerifySignature(*pServerNym) &&
            theNym.LoadSignedNymfile(*pServerNym)) // ServerNym here is not
                                                   // theNym's identity, but
                                                   // merely the signer on this
                                                   // file.
        {
            pFirstNym = &theNym; //  <=====
        }
        else {
            OTString strNymID(FIRST_NYM_ID);
            otErr << "OTMarket::" << __FUNCTION__
                  << ": Failure verifying trade, offer, or nym, or loading "
                     "signed Nymfile: " << strNymID << "\n";
            theTrade.FlagForRemoval();
            return;
        }
    }

    if (bOtherNymIsServerNym) // If the Other Nym is the server, then just point
                              // to that.
    {
        pOtherNym = pServerNym;
    }
    else if (bTradersAreSameNym) // Else if the Traders are the same Nym,
                                   // point to the one we already loaded.
    {
        pOtherNym = pFirstNym; // theNym is pFirstNym
    }
    else // Otherwise load the Other Nym from Disk and point to that.
    {
        theOtherNym.SetIdentifier(OTHER_NYM_ID);

        if (!theOtherNym.LoadPublicKey()) {
            OTString strNymID(OTHER_NYM_ID);
            otErr << "Failure loading Other Nym public key in OTMarket::"
                  << __FUNCTION__ << ": " << strNymID << "\n";
            pOtherTrade->FlagForRemoval();
            return;
        }

        if (theOtherNym.VerifyPseudonym() &&
            pOtherTrade->VerifySignature(*pServerNym) &&
            theOtherOffer.VerifySignature(*pServerNym) &&
            theOtherNym.LoadSignedNymfile(*pServerNym)) {
            pOtherNym = &theOtherNym; //  <=====
        }
        else {
            OTString strNymID(OTHER_NYM_ID);
            otErr << "Failure loading or verifying Other Nym public key in "
                     "OTMarket::" << __FUNCTION__ << ": " << strNymID << "\n";
            pOtherTrade->FlagForRemoval();
            return;
        }
    }

    // AT THIS POINT, I have pServerNym, pFirstNym, and pOtherNym.
    // ALL are loaded from disk (where necessary.) AND...
    // ALL are valid pointers, (even if they sometimes point to the same
    // object,)
    // AND none are capable of overwriting the storage of the other (by
    // accidentally
    // loading the same storage twice.)
    // We also have boolean variables at this point to tell us exactly which are
    // which,
    // (in case some of those pointers do go to the same object.)
    // They are:
    // bFirstNymIsServerNym, bOtherNymIsServerNym, and bTradersAreSameNym.
    //
    // We also have theTrade, theOffer, pOtherTrade, and theOtherOffer,
    // and we know they are all good also.
    //
    // We also know that pOtherTrade/theOtherOffer are a Limit order (NOT a
    // Market order.)
    // (Meaning pOtherTrade/theOtherOffer definitely has a price set.)
    //
    // We also know that theTrade/theOffer MIGHT be a Limit order OR a Market
    // order.
    // (Meaning theTrade/theOffer MIGHT have a 0 price.)
    //

    // Make sure have ALL FOUR accounts loaded and checked out.
    // (first nym's asset/currency, and other nym's asset/currency.)

    OTAccount* pFirstAssetAcct =
        OTAccount::LoadExistingAccount(theTrade.GetSenderAcctID(), SERVER_ID);
    OTAccount* pFirstCurrencyAcct =
        OTAccount::LoadExistingAccount(theTrade.GetCurrencyAcctID(), SERVER_ID);

    OTAccount* pOtherAssetAcct = OTAccount::LoadExistingAccount(
        pOtherTrade->GetSenderAcctID(), SERVER_ID);
    OTAccount* pOtherCurrencyAcct = OTAccount::LoadExistingAccount(
        pOtherTrade->GetCurrencyAcctID(), SERVER_ID);

    if ((nullptr == pFirstAssetAcct) || (nullptr == pFirstCurrencyAcct)) {
        otOut << "ERROR verifying existence of one of the first trader's "
                 "accounts during attempted Market trade.\n";
        cleanup_four_accounts(pFirstAssetAcct, pFirstCurrencyAcct,
                              pOtherAssetAcct, pOtherCurrencyAcct);
        theTrade.FlagForRemoval(); // Removes from Cron.
        return;
    }
    else if ((nullptr == pOtherAssetAcct) ||
               (nullptr == pOtherCurrencyAcct)) {
        otOut << "ERROR verifying existence of one of the second trader's "
                 "accounts during attempted Market trade.\n";
        cleanup_four_accounts(pFirstAssetAcct, pFirstCurrencyAcct,
                              pOtherAssetAcct, pOtherCurrencyAcct);
        pOtherTrade->FlagForRemoval(); // Removes from Cron.
        return;
    }

    // Are the accounts of the first trader of the right asset types?
    // We already know the asset types matched as far as the market, trades, and
    // offers were concerned.
    // But only once the accounts themselves have been loaded can we VERIFY this
    // to be true.
    else if ((pFirstAssetAcct->GetAssetTypeID() !=
              GetAssetID()) || // the trader's asset accts have same asset type
                               // as the market.
             (pFirstCurrencyAcct->GetAssetTypeID() !=
              GetCurrencyID()) // the trader's currency accts have same asset
                               // type as the market.
             ) {
        otErr << "ERROR - First Trader has accounts of wrong "
                 "asset types in OTMarket::" << __FUNCTION__ << "\n";
        cleanup_four_accounts(pFirstAssetAcct, pFirstCurrencyAcct,
                              pOtherAssetAcct, pOtherCurrencyAcct);
        theTrade.FlagForRemoval(); // Removes from Cron.
        return;
    }
    else if ((pOtherAssetAcct->GetAssetTypeID() !=
                GetAssetID()) || // the trader's asset accts have same asset
                                 // type as the market.
               (pOtherCurrencyAcct->GetAssetTypeID() !=
                GetCurrencyID())) // the trader's currency accts have same asset
                                  // type as market.
    {
        otErr << "ERROR - Other Trader has accounts of wrong "
                 "asset types in OTMarket::" << __FUNCTION__ << "\n";
        cleanup_four_accounts(pFirstAssetAcct, pFirstCurrencyAcct,
                              pOtherAssetAcct, pOtherCurrencyAcct);
        pOtherTrade->FlagForRemoval(); // Removes from Cron.
        return;
    }

    // Make sure all accounts are signed by the server and have the owner they
    // are expected to have.

    // I call VerifySignature here since VerifyContractID was already called in
    // LoadExistingAccount().
    else if ((!pFirstAssetAcct->VerifyOwner(*pFirstNym) ||
              !pFirstAssetAcct->VerifySignature(*pServerNym)) ||
             (!pFirstCurrencyAcct->VerifyOwner(*pFirstNym) ||
              !pFirstCurrencyAcct->VerifySignature(*pServerNym))) {
        otErr << "ERROR verifying ownership or signature on one of first "
                 "trader's accounts in OTMarket::" << __FUNCTION__ << "\n";
        cleanup_four_accounts(pFirstAssetAcct, pFirstCurrencyAcct,
                              pOtherAssetAcct, pOtherCurrencyAcct);
        theTrade.FlagForRemoval(); // Removes from Cron.
        return;
    }
    else if ((!pOtherAssetAcct->VerifyOwner(*pOtherNym) ||
                !pOtherAssetAcct->VerifySignature(*pServerNym)) ||
               (!pOtherCurrencyAcct->VerifyOwner(*pOtherNym) ||
                !pOtherCurrencyAcct->VerifySignature(*pServerNym))) {
        otErr << "ERROR verifying ownership or signature on one of other "
                 "trader's accounts in OTMarket::" << __FUNCTION__ << "\n";
        cleanup_four_accounts(pFirstAssetAcct, pFirstCurrencyAcct,
                              pOtherAssetAcct, pOtherCurrencyAcct);
        pOtherTrade->FlagForRemoval(); // Removes from Cron.
        return;
    }

    // By this point, I know I have all four accounts loaded, and I know that
    // they have the right asset types,
    // and I know they have the right owners and they were all signed by the
    // server.
    // I also know that their account IDs in their internal records matched the
    // account filename for each acct.
    // I also have pointers to the Nyms who own these accounts, as well as the
    // Trades and Offers associated with them.
    else {
        // Okay then, everything checks out. Let's add this to the sender's
        // outbox and the recipient's inbox.
        // IF they can be loaded up from file, or generated, that is.

        // Load the inbox/outbox in case they already exist
        OTLedger theFirstAssetInbox(FIRST_NYM_ID, theTrade.GetSenderAcctID(),
                                    SERVER_ID),
            theFirstCurrencyInbox(FIRST_NYM_ID, theTrade.GetCurrencyAcctID(),
                                  SERVER_ID),
            theOtherAssetInbox(OTHER_NYM_ID, pOtherTrade->GetSenderAcctID(),
                               SERVER_ID),
            theOtherCurrencyInbox(OTHER_NYM_ID,
                                  pOtherTrade->GetCurrencyAcctID(), SERVER_ID);

        // ALL inboxes -- no outboxes. All will receive notification of
        // something ALREADY DONE.
        bool bSuccessLoadingFirstAsset = theFirstAssetInbox.LoadInbox();
        bool bSuccessLoadingFirstCurrency = theFirstCurrencyInbox.LoadInbox();
        bool bSuccessLoadingOtherAsset = theOtherAssetInbox.LoadInbox();
        bool bSuccessLoadingOtherCurrency = theOtherCurrencyInbox.LoadInbox();

        // ...or generate them otherwise...

        if (true == bSuccessLoadingFirstAsset)
            bSuccessLoadingFirstAsset =
                theFirstAssetInbox.VerifyAccount(*pServerNym);
        else
            bSuccessLoadingFirstAsset = theFirstAssetInbox.GenerateLedger(
                theTrade.GetSenderAcctID(), SERVER_ID, OTLedger::inbox,
                true); // bGenerateFile=true

        if (true == bSuccessLoadingFirstCurrency)
            bSuccessLoadingFirstCurrency =
                theFirstCurrencyInbox.VerifyAccount(*pServerNym);
        else
            bSuccessLoadingFirstCurrency = theFirstCurrencyInbox.GenerateLedger(
                theTrade.GetCurrencyAcctID(), SERVER_ID, OTLedger::inbox,
                true); // bGenerateFile=true

        if (true == bSuccessLoadingOtherAsset)
            bSuccessLoadingOtherAsset =
                theOtherAssetInbox.VerifyAccount(*pServerNym);
        else
            bSuccessLoadingOtherAsset = theOtherAssetInbox.GenerateLedger(
                pOtherTrade->GetSenderAcctID(), SERVER_ID, OTLedger::inbox,
                true); // bGenerateFile=true

        if (true == bSuccessLoadingOtherCurrency)
            bSuccessLoadingOtherCurrency =
                theOtherCurrencyInbox.VerifyAccount(*pServerNym);
        else
            bSuccessLoadingOtherCurrency = theOtherCurrencyInbox.GenerateLedger(
                pOtherTrade->GetCurrencyAcctID(), SERVER_ID, OTLedger::inbox,
                true); // bGenerateFile=true

        if ((false == bSuccessLoadingFirstAsset) ||
            (false == bSuccessLoadingFirstCurrency)) {
            otErr << "ERROR loading or generating an inbox for first trader in "
                     "OTMarket::" << __FUNCTION__ << ".\n";
            cleanup_four_accounts(pFirstAssetAcct, pFirstCurrencyAcct,
                                  pOtherAssetAcct, pOtherCurrencyAcct);
            theTrade.FlagForRemoval(); // Removes from Cron.
            return;
        }
        else if ((false == bSuccessLoadingOtherAsset) ||
                   (false == bSuccessLoadingOtherCurrency)) {
            otErr << "ERROR loading or generating an inbox for other trader in "
                     "OTMarket::" << __FUNCTION__ << ".\n";
            cleanup_four_accounts(pFirstAssetAcct, pFirstCurrencyAcct,
                                  pOtherAssetAcct, pOtherCurrencyAcct);
            pOtherTrade->FlagForRemoval(); // Removes from Cron.
            return;
        }
        else {
            // Generate new transaction numbers for these new transactions
            int64_t lNewTransactionNumber = pCron->GetNextTransactionNumber();

            //            OT_ASSERT(lNewTransactionNumber > 0); // this can be
            // my reminder.
            if (0 == lNewTransactionNumber) {
                otOut << "WARNING: Market is unable to process because there "
                         "are no more transaction numbers available.\n";
                cleanup_four_accounts(pFirstAssetAcct, pFirstCurrencyAcct,
                                      pOtherAssetAcct, pOtherCurrencyAcct);
                // (Here I flag neither trade for removal.)
                return;
            }

            // Why a new transaction number here?
            //
            // Each user had to burn one of his own numbers to generate his own
            // Trade.
            //
            // So the First Trader might have used number 345, and the Other
            // Trader might have
            // used the number 912. Those numbers were used to CREATE the Trades
            // initially, and
            // they can be used to lookup the original trade request from each
            // user.
            //
            // So whenever I give a receipt to either Trader, I am sure to tell
            // Trader A that
            // this is in reference to transaction 345, and I tell Trader B that
            // this is in
            // reference to transaction 912.
            //
            // I also have to get a new transaction number (here) because the
            // first two numbers
            // were requests signed by the users to post the trades. That was
            // completed--those
            // numbers have been used now, and they could not be used again
            // without introducing
            // duplication and confusion. To remove money from someone's
            // account, which we have just
            // done, you need a new transaction number for that! Because you
            // need a new number if
            // you're going to put some new receipt in their inbox, which is
            // exactly what we are
            // doing right now.
            //
            // So let's say I generate transaction # 10023 to represent this
            // money exchange that has
            // just occurred between these two traders.  To me as a user, 345
            // was my original request
            // to post the trade, 912 was the other guy's request to post his
            // trade, and 10023 was
            // the server's transferring funds (based on the authorization in
            // those trades). Put
            // another way, 345 has my signature, 912 has the other trader's
            // signature, and 10023
            // has the server's signature.

            // Start generating the receipts (for all four inboxes.)

            OTTransaction* pTrans1 = OTTransaction::GenerateTransaction(
                theFirstAssetInbox, OTTransaction::marketReceipt,
                lNewTransactionNumber);

            OTTransaction* pTrans2 = OTTransaction::GenerateTransaction(
                theFirstCurrencyInbox, OTTransaction::marketReceipt,
                lNewTransactionNumber);

            OTTransaction* pTrans3 = OTTransaction::GenerateTransaction(
                theOtherAssetInbox, OTTransaction::marketReceipt,
                lNewTransactionNumber);

            OTTransaction* pTrans4 = OTTransaction::GenerateTransaction(
                theOtherCurrencyInbox, OTTransaction::marketReceipt,
                lNewTransactionNumber);

            // (No need to OT_ASSERT on the above transactions since it occurs
            // in GenerateTransaction().)

            // All four inboxes will get receipts with the same (new)
            // transaction ID.
            // They all have a "reference to" containing the original trade.
            // The first two will contain theTrade as the reference field,
            // but the second two contain pOtherTrade as the reference field.
            // The first two also thus reference a different original
            // transaction number than the other two.
            // That's because each end of the trade was originally authorized
            // separately by each trader, and
            // in each case he used his own unique transaction number to do so.

            // set up the transaction items (each transaction may have multiple
            // items... but not in this case.)
            OTItem* pItem1 = OTItem::CreateItemFromTransaction(
                *pTrans1, OTItem::marketReceipt);
            OTItem* pItem2 = OTItem::CreateItemFromTransaction(
                *pTrans2, OTItem::marketReceipt);
            OTItem* pItem3 = OTItem::CreateItemFromTransaction(
                *pTrans3, OTItem::marketReceipt);
            OTItem* pItem4 = OTItem::CreateItemFromTransaction(
                *pTrans4, OTItem::marketReceipt);

            // these may be unnecessary, I'll have to check
            // CreateItemFromTransaction. I'll leave em.
            OT_ASSERT(nullptr != pItem1);
            OT_ASSERT(nullptr != pItem2);
            OT_ASSERT(nullptr != pItem3);
            OT_ASSERT(nullptr != pItem4);

            pItem1->SetStatus(OTItem::rejection); // the default.
            pItem2->SetStatus(OTItem::rejection); // the default.
            pItem3->SetStatus(OTItem::rejection); // the default.
            pItem4->SetStatus(OTItem::rejection); // the default.

            // Calculate the amount and remove / add it to the relevant
            // accounts.
            // Make sure each Account can afford it, and roll back in case of
            // failure.

            // -- TheTrade will get the PriceLimit offered by theOtherOffer, and
            // not vice-versa.
            //    (See explanation below this function if you need to know the
            // reasoning.)

            // NOTICE: This is one of the reasons why theOtherOffer CANNOT be a
            // Market order,
            // because we will be using his price limit to determine the price.
            // (Another reason FYI, is because Market Orders are only allowed to
            // process once,
            // IN TURN.)

            // Some logic described:
            //
            // Calculate the price
            // I know the amount available for trade is at LEAST the minimum
            // increment on both
            // sides, since that was validated before this function was even
            // called. Therefore,
            // let's calculate a price based on the largest of the two minimum
            // increments.
            // Figure out which is largest, then divide that by the scale to get
            // the multiplier.
            // Multiply THAT by the theOtherOffer's Price Limit to get the price
            // per minimum increment.
            //
            // Since we are calculating the MINIMUM that must be processed per
            // round, let's also
            // calculate the MOST that could be traded between these two offers.
            // Then let's
            // mod that to the Scale and remove the remainder, then divide by
            // the Scale and
            // multiply by the price (to get the MOST that would have to be
            // paid, if the offers
            // fulfilled each other to the maximum possible according to their
            // limits.)
            // !! It's better to process it all at once and avoid the loop
            // entirely.
            // Plus, there's SUPPOSED to be enough funds in all the accounts to
            // cover it.
            //
            // Anyway, if there aren't:
            //
            // Then LOOP (while available >= minimum increment) on both sides of
            // the trade
            // Inside, try to move funds across all 4 accounts. If any of them
            // fail, roll them
            // back and break. (I'll check balances first to avoid this.)
            // As long as there's enough available in the accounts to continue
            // exchanging the
            // largest minimum increment, then keep looping like this and moving
            // the digital assets.
            // Each time, also, be sure to update the Offer so that less and
            // less is available on each trade.
            // At some point, the Trades will run out of authorization to
            // transfer any more from the accounts.
            // Perhaps one has a 10,000 bushel total limit and it has been
            // reached. The trade is complete,
            // from his side of it, anyway. Then the loop will be over for sure.

            OTAccount* pAssetAccountToDebit = nullptr;
            OTAccount* pAssetAccountToCredit = nullptr;
            OTAccount* pCurrencyAccountToDebit = nullptr;
            OTAccount* pCurrencyAccountToCredit = nullptr;

            if (theOffer.IsAsk()) // I'm selling, he's buying
            {
                pAssetAccountToDebit = pFirstAssetAcct; // I am selling gold.
                                                        // When I sell, my gold
                                                        // balance goes down.
                pAssetAccountToCredit =
                    pOtherAssetAcct; // He is bidding on gold. When he buys, his
                                     // gold balance goes up.
                pCurrencyAccountToDebit =
                    pOtherCurrencyAcct; // He is paying in dollars. When he
                                        // pays, his dollar balance goes down.
                pCurrencyAccountToCredit =
                    pFirstCurrencyAcct; // I am being paid in dollars. When I
                                        // get paid, my dollar balance goes up.
            }
            else                      // I'm buying, he's selling
            {
                pAssetAccountToDebit =
                    pOtherAssetAcct; // He is selling gold. When he sells, his
                                     // gold balance goes down.
                pAssetAccountToCredit =
                    pFirstAssetAcct; // I am bidding on gold. When I buy, my
                                     // gold balance goes up.
                pCurrencyAccountToDebit =
                    pFirstCurrencyAcct; // I am paying in dollars. When I pay,
                                        // my dollar balance goes down.
                pCurrencyAccountToCredit =
                    pOtherCurrencyAcct; // He is being paid in dollars. When he
                                        // gets paid, his dollar balance goes
                                        // up.
            }

            // Calculate minimum increment to be traded each round.
            int64_t lMinIncrementPerRound =
                ((theOffer.GetMinimumIncrement() >
                  theOtherOffer.GetMinimumIncrement())
                     ? theOffer.GetMinimumIncrement()
                     : theOtherOffer.GetMinimumIncrement());

            const int64_t lMultiplier =
                (lMinIncrementPerRound / GetScale()); // If the Market scale is
                                                      // 10, and the minimum
                                                      // increment is 50,
                                                      // multiplier is 5..
            // The price limit is per scale. (Per 10.) So if 1oz gold is $1300,
            // then 10oz scale
            // would be $13,000. So if my price limit is per SCALE, I might set
            // my limit
            // to $12,000 or $13,000 (PER 10 OZ OF GOLD, which is the SCALE for
            // this market.)

            // Calc price of each round.
            int64_t lPrice =
                (lMultiplier * theOtherOffer.GetPriceLimit()); // So if my
                                                               // minimum
                                                               // increment is
                                                               // 50, then my
                                                               // multiplier is
                                                               // 5, which means
            // multiply my price by 5: $13,000 * 5 == $65,000 for 50 oz. per
            // minimum inc.

            // Why am I using the OTHER Offer's price limit, and not my own?
            // See notes at top and bottom of this function for the answer.

            // There's two ways to process this: in rounds (by minimum
            // increment), for which the numbers were
            // just calulated above...
            //
            // ...OR based on whatever is the MOST available from BOTH parties.
            // (Whichever is the least of the
            // two parties' "Most Available Left To Trade".) So I'll try THAT
            // first, to avoid processing in
            // rounds. (Since the funds SHOULD be there...)

            int64_t lMostAvailable = ((theOffer.GetAmountAvailable() >
                                       theOtherOffer.GetAmountAvailable())
                                          ? theOtherOffer.GetAmountAvailable()
                                          : theOffer.GetAmountAvailable());

            int64_t lTemp = lMostAvailable % GetScale(); // The Scale may not
                                                         // evenly divide into
                                                         // the amount available

            lMostAvailable -= lTemp; // We'll subtract remainder amount, so it's
                                     // even to scale (which is how it's
                                     // priced.)

            // We KNOW the amount available on the offer is at least as much as
            // the minimum increment (on both sides)
            // because that is verified in the caller function. So we KNOW
            // either side can process the minimum, at least
            // based on the authorization in the offers (not necessarily in the
            // accounts themselves, though they are
            // SUPPOSED to have enough funds to cover it...)
            //
            // Next question is: can both sides process the MOST AVAILABLE? If
            // so, do THAT, instead of processing by rounds.

            const int64_t lOverallMultiplier =
                lMostAvailable / GetScale(); // Price is per scale  // This line
                                             // was commented with the line
                                             // below it. They go together.

            // Why theOtherOffer's price limit instead of theOffer's? See notes
            // top/bottom this function.
            const int64_t lMostPrice =
                (lOverallMultiplier * theOtherOffer.GetPriceLimit());
            // TO REMOVE MULTIPLIER FROM PRICE, AT LEAST THE ABOVE LINE WOULD
            // REMOVE MULTIPLIER.

            // To avoid rounds, first I see if I can satisfy the entire order at
            // once on either side...
            if ((pAssetAccountToDebit->GetBalance() >= lMostAvailable) &&
                (pCurrencyAccountToDebit->GetBalance() >=
                 lMostPrice)) { // There's enough the accounts to do it all at
                                // once! No need for rounds.

                lMinIncrementPerRound = lMostAvailable;
                lPrice = lMostPrice;

                // By setting the ABOVE two values the way that I did, it means
                // the below loop
                // will execute properly, BUT ONLY ITERATING ONCE. Basically
                // this section of
                // code just optimizes that loop when possible by allowing it to
                // execute
                // only once.
            }

            // Otherwise, I go ahead and process it in rounds, by minimum
            // increment...
            // (Since the funds ARE supposed to be there to process the whole
            // thing, I COULD
            // choose to cancel the offender right now! I might put an extra fee
            // in this loop or
            // just remove it. The software would still be functional, it would
            // just enforce the
            // account having enough funds to cover the full offer at all
            // times--if it wants to
            // trade at all.)

            bool bSuccess = false;

            int64_t lOfferFinished = 0,
                    lOtherOfferFinished = 0, // We store these up and then add
                                             // the totals to the offers at the
                                             // end (only upon success.)
                lTotalPaidOut =
                    0; // However much is paid for the assets, total.

            // Continuing the example from above, each round I will trade:
            //        50 oz lMinIncrementPerRound, in return for $65,000 lPrice.
            while ((lMinIncrementPerRound <=
                    (theOffer.GetAmountAvailable() -
                     lOfferFinished)) && // The primary offer has at least 50
                                         // available to trade (buy OR sell)
                   (lMinIncrementPerRound <=
                    (theOtherOffer.GetAmountAvailable() -
                     lOtherOfferFinished)) && // The other offer has at least 50
                                              // available for trade also.
                   (lMinIncrementPerRound <=
                    pAssetAccountToDebit->GetBalance()) && // Asset Acct to be
                                                           // debited has at
                                                           // least 50
                                                           // available.
                   (lPrice <=
                    pCurrencyAccountToDebit->GetBalance())) // Currency Acct to
                                                            // be debited has at
                                                            // least 65000
                                                            // available.
            {
                // Within this block, the offer is authorized on both sides, and
                // there is enough in
                // each of the relevant accounts to cover the round, (for SURE.)
                // So let's DO it.

                bool bMove1 =
                    pAssetAccountToDebit->Debit(lMinIncrementPerRound);
                bool bMove2 = pCurrencyAccountToDebit->Debit(lPrice);
                bool bMove3 =
                    pAssetAccountToCredit->Credit(lMinIncrementPerRound);
                bool bMove4 = pCurrencyAccountToCredit->Credit(lPrice);

                // If ANY of these failed, then roll them all back and break.
                if (!bMove1 || !bMove2 || !bMove3 || !bMove4) {
                    otErr << "Very strange! Funds were available, yet debit or "
                             "credit failed while performing trade. "
                             "Attempting rollback!\n";
                    // We won't save the files anyway, if this failed. So the
                    // rollback is actually superfluous but somehow worthwhile.
                    rollback_four_accounts(
                        *pAssetAccountToDebit, bMove1, lMinIncrementPerRound,
                        *pCurrencyAccountToDebit, bMove2, lPrice,
                        *pAssetAccountToCredit, bMove3, lMinIncrementPerRound,
                        *pCurrencyAccountToCredit, bMove4, lPrice);

                    bSuccess = false;
                    break;
                }

                // At this point, we know all the debits and credits were
                // successful (FOR THIS ROUND.)
                // Also notice that the Trades and Offers have not been changed
                // at all--only the accounts.
                bSuccess = true;

                // So let's adjust the offers to reflect this also...
                // The while() above checks these values in
                // GetAmountAvailable().
                lOfferFinished += lMinIncrementPerRound;
                lOtherOfferFinished += lMinIncrementPerRound;

                lTotalPaidOut += lPrice;
            }

            // At this point, it has gone god-knows-how-many rounds, (or just
            // one) and then broke out,
            // with bSuccess=false, OR finished properly when the while() terms
            // were fulfilled, with bSuccess = true.
            //
            // My point? If there was a screw-up, EVEN if the rollback was
            // successful, it STILL only rolled-back
            // the most RECENT round -- There still might have been 20 rounds
            // processed before the break.
            // (Funds could still have been moved, even if rollback was
            // successful.) THEREFORE, DO NOT SAVE if false.
            // We only save those accounts if bSuccess == true.  The Rollback is
            // not good enough

            if (true == bSuccess) {
                // ALL of the four accounts involved need to get a receipt of
                // this trade in their inboxes...
                pItem1->SetStatus(OTItem::acknowledgement); // pFirstAssetAcct
                pItem2->SetStatus(
                    OTItem::acknowledgement); // pFirstCurrencyAcct
                pItem3->SetStatus(OTItem::acknowledgement); // pOtherAssetAcct
                pItem4->SetStatus(
                    OTItem::acknowledgement); // pOtherCurrencyAcct

                // Everytime a trade processes, a receipt is put in each
                // account's inbox.
                // This contains a copy of the current trade (which took money
                // from the acct.)
                //
                // ==> So I increment the trade count each time before dropping
                // the receipt. (I also use a fresh
                // transaction number when I put it into the inbox.) That way,
                // the user will never get the same
                // receipt for the same trade twice. It cannot take funds from
                // his account, without a new trade
                // count and a new transaction number on a new receipt. Until
                // the user accepts the receipt out
                // of his inbox with a new balance agreement, the existing
                // receipts can be added up and compared
                // to the last balance agreement, to verify the current balance.
                // Every receipt from a processing
                // trade will have the user's authorization, signature, and
                // terms, as well as the update in balances
                // due to the trade, signed by the server.

                theTrade.IncrementTradesAlreadyDone();
                pOtherTrade->IncrementTradesAlreadyDone();

                theOffer.IncrementFinishedSoFar(lOfferFinished); // I was
                                                                 // storing
                                                                 // these up in
                                                                 // the loop
                                                                 // above.
                theOtherOffer.IncrementFinishedSoFar(
                    lOtherOfferFinished); // I was storing these up in the loop
                                          // above.

                // These have updated values, so let's save them.
                theTrade.ReleaseSignatures();
                theTrade.SignContract(*pServerNym);
                theTrade.SaveContract();

                pOtherTrade->ReleaseSignatures();
                pOtherTrade->SignContract(*pServerNym);
                pOtherTrade->SaveContract();

                theOffer.ReleaseSignatures();
                theOffer.SignContract(*pServerNym);
                theOffer.SaveContract();

                theOtherOffer.ReleaseSignatures();
                theOtherOffer.SignContract(*pServerNym);
                theOtherOffer.SaveContract();

                m_lLastSalePrice =
                    theOtherOffer.GetPriceLimit(); // Priced per scale.

                // Here we save this trade in a list of the most recent 50
                // trades.
                {
                    if (nullptr == m_pTradeList) {
                        m_pTradeList = dynamic_cast<OTDB::TradeListMarket*>(
                            OTDB::CreateObject(
                                OTDB::STORED_OBJ_TRADE_LIST_MARKET));
                    }

                    std::unique_ptr<OTDB::TradeDataMarket> pTradeData(
                        dynamic_cast<OTDB::TradeDataMarket*>(OTDB::CreateObject(
                            OTDB::STORED_OBJ_TRADE_DATA_MARKET)));

                    const int64_t& lTransactionNum =
                        theOffer.GetTransactionNum();
                    const time64_t theDate = OTTimeGetCurrentTime();
                    const int64_t& lPriceLimit =
                        theOtherOffer.GetPriceLimit(); // Priced per scale.
                    const int64_t& lAmountSold = lOfferFinished;

                    pTradeData->transaction_id =
                        to_string<int64_t>(lTransactionNum);
                    pTradeData->date = to_string<time64_t>(theDate);
                    pTradeData->price = to_string<int64_t>(lPriceLimit);
                    pTradeData->amount_sold = to_string<int64_t>(lAmountSold);

                    m_strLastSaleDate = pTradeData->date;

                    // *pTradeData is CLONED at this time (I'm still responsible
                    // to delete.)
                    // That's also why I add it here, after all the above: So
                    // the data is set right BEFORE the cloning occurs.
                    //
                    m_pTradeList->AddTradeDataMarket(*pTradeData);

                    // Here we erase the oldest elements so the list never
                    // exceeds 50 elements total.
                    //
                    while (m_pTradeList->GetTradeDataMarketCount() >
                           MAX_MARKET_QUERY_DEPTH)
                        m_pTradeList->RemoveTradeDataMarket(0);
                }

                // Account balances have changed based on these trades that we
                // just processed.
                // Make sure to save the Market since it contains those offers
                // that have just updated.
                SaveMarket();

                // The Trade has changed, and it is stored as a CronItem. So I
                // save Cron as well, for
                // the same reason I saved the Market.
                pCron->SaveCron();
            }

            //
            // EVERYTHING BELOW is just about notifying the users, by dropping
            // the receipt in their
            // inboxes.
            //
            // (The Trade, Offer, Cron, and Market are ALL updated and SAVED as
            // of this point.)
            //

            // The TRANSACTION will be sent with "In Reference To" information
            // containing the
            // ORIGINAL SIGNED TRADE (which includes the ORIGINAL SIGNED OFFER
            // inside of it.)
            //
            // Whereas the TRANSACTION ITEM includes a "Note" containing the
            // UPDATED TRADE,
            // (with the SERVER's SIGNATURE) as well as an "attachment"
            // containing the UPDATED
            // OFFER (again, with the server's signature on it.)

            // I was doing this above, but had to move it all down here, since
            // the Trade
            //  and Offer have just finally been updated to their final
            // values...

            // Here I make sure that each trader's receipt (each inbox notice)
            // references the
            // transaction number that the trader originally used to issue the
            // trade...
            // This number is used to match up offers to trades, and used to
            // track all cron items.
            // (All Cron items require a transaction from the user to add them
            // to Cron in the
            // first place.)
            //
            pTrans1->SetReferenceToNum(theTrade.GetTransactionNum());
            pTrans2->SetReferenceToNum(theTrade.GetTransactionNum());
            pTrans3->SetReferenceToNum(pOtherTrade->GetTransactionNum());
            pTrans4->SetReferenceToNum(pOtherTrade->GetTransactionNum());

            // Then, I make sure the Reference string on the Transaction
            // contains the
            // ORIGINAL TRADE (with the TRADER's SIGNATURE ON IT!) For both
            // traders.

            OTCronItem* pOrigTrade = nullptr;
            OTCronItem* pOrigOtherTrade = nullptr;

            // OTCronItem::LoadCronReceipt loads the original version with the
            // user's signature.
            // (Updated versions, as processing occurs, are signed by the
            // server.)
            pOrigTrade =
                OTCronItem::LoadCronReceipt(theTrade.GetTransactionNum());
            pOrigOtherTrade =
                OTCronItem::LoadCronReceipt(pOtherTrade->GetTransactionNum());

            OT_ASSERT(nullptr != pOrigTrade);
            OT_ASSERT(nullptr != pOrigOtherTrade);

            OT_ASSERT_MSG(pOrigTrade->VerifySignature(*pFirstNym),
                          "Signature was already verified on Trade when first "
                          "added to market, but now it fails.\n");
            OT_ASSERT_MSG(pOrigOtherTrade->VerifySignature(*pOtherNym),
                          "Signature was already verified on Trade when first "
                          "added to market, but now it fails.\n");

            // I now have String copies (PGP-signed XML files) of the original
            // Trade requests...
            // Plus I know they were definitely signed by the Nyms (even though
            // that was already
            // verified when they were first added to the market--and they have
            // been signed by the
            // server nym ever since.)
            OTString strOrigTrade(*pOrigTrade),
                strOrigOtherTrade(*pOrigOtherTrade);

            // The reference on the transaction contains OTCronItem, in this
            // case.
            // The original trade for each party, versus the updated trade
            // (which is stored
            // on the marketReceipt item just below here.)
            //
            pTrans1->SetReferenceString(strOrigTrade);
            pTrans2->SetReferenceString(strOrigTrade);
            pTrans3->SetReferenceString(strOrigOtherTrade);
            pTrans4->SetReferenceString(strOrigOtherTrade);

            // Make sure to clean these up.
            // TODO: Run a scanner on the code for memory leaks and buffer
            // overflows.
            delete pOrigTrade;
            pOrigTrade = nullptr;
            delete pOrigOtherTrade;
            pOrigOtherTrade = nullptr;

            // Here's where the item stores the UPDATED TRADE (in its Note)
            // and the UPDATED OFFER (in its attachment) with the SERVER's
            // SIGNATURE
            // on both.
            // (As a receipt for each trader, so they can see their offer
            // updating.)

            // Lucky I just signed and saved these trades / offers above, or
            // they would
            // still have the old data in them.
            OTString strTrade(theTrade), strOtherTrade(*pOtherTrade),
                strOffer(theOffer), strOtherOffer(theOtherOffer);

            // The marketReceipt ITEM's NOTE contains the UPDATED TRADE.
            //
            pItem1->SetNote(strTrade);
            pItem2->SetNote(strTrade);
            pItem3->SetNote(strOtherTrade);
            pItem4->SetNote(strOtherTrade);

            /*

             NOTE, todo: Someday, need to reverse these, so that the updated
             Trade is stored in
             the attachment, and the updated offer is stored in the note. This
             is much more consistent
             with other cron receipts, such as paymentReceipt, and finalReceipt.
             Unfortunately,
             marketReceipt is already implemented the opposite of these, but I
             will fix it someday just
             for consistency. See large notes 2/3rds of the way down in
             OTTrade::onFinalReceipt().

             Todo security: really each receipt should contain a copy of BOTH
             (asset+currency) so
             the user can calculate the sale price and compare it to the terms
             on the original offer.
             */

            // Also set the ** UPDATED OFFER ** as the ATTACHMENT on the **
            // item.**
            // (With the SERVER's signature on it!)
            // (As a receipt for each trader, so they can see their offer
            // updating.)
            pItem1->SetAttachment(strOffer);
            pItem2->SetAttachment(strOffer);
            pItem3->SetAttachment(strOtherOffer);
            pItem4->SetAttachment(strOtherOffer);

            // Inbox receipts need to clearly show the AMOUNT moved...
            // Also need to clearly show negative or positive, since that
            // is otherwise not obvious just because you have a marketReceipt...
            //
            // The AMOUNT is stored on the marketReceipt ITEM, on the item list
            // for
            // the marketReceipt TRANSACTION.
            //
            if (theOffer.IsAsk()) // I'm selling, he's buying
            {
                pItem1->SetAmount(lOfferFinished * (-1)); // first asset
                pItem2->SetAmount(lTotalPaidOut);         // first currency
                pItem3->SetAmount(lOtherOfferFinished);   // other asset
                pItem4->SetAmount(lTotalPaidOut * (-1));  // other currency
            }
            else // I'm buying, he's selling
            {
                pItem1->SetAmount(lOfferFinished);             // first asset
                pItem2->SetAmount(lTotalPaidOut * (-1));       // first currency
                pItem3->SetAmount(lOtherOfferFinished * (-1)); // other asset
                pItem4->SetAmount(lTotalPaidOut);              // other currency
            }

            if (true == bSuccess) {
                // sign the item
                pItem1->SignContract(*pServerNym);
                pItem2->SignContract(*pServerNym);
                pItem3->SignContract(*pServerNym);
                pItem4->SignContract(*pServerNym);

                pItem1->SaveContract();
                pItem2->SaveContract();
                pItem3->SaveContract();
                pItem4->SaveContract();

                // the Transaction "owns" the item now and will handle cleaning
                // it up.
                pTrans1->AddItem(*pItem1);
                pTrans2->AddItem(*pItem2);
                pTrans3->AddItem(*pItem3);
                pTrans4->AddItem(*pItem4);

                pTrans1->SignContract(*pServerNym);
                pTrans2->SignContract(*pServerNym);
                pTrans3->SignContract(*pServerNym);
                pTrans4->SignContract(*pServerNym);

                pTrans1->SaveContract();
                pTrans2->SaveContract();
                pTrans3->SaveContract();
                pTrans4->SaveContract();

                // Here the transactions we just created are actually added to
                // the ledgers.
                theFirstAssetInbox.AddTransaction(*pTrans1);
                theFirstCurrencyInbox.AddTransaction(*pTrans2);
                theOtherAssetInbox.AddTransaction(*pTrans3);
                theOtherCurrencyInbox.AddTransaction(*pTrans4);

                // Release any signatures that were there before (They won't
                // verify anymore anyway, since the content has changed.)
                theFirstAssetInbox.ReleaseSignatures();
                theFirstCurrencyInbox.ReleaseSignatures();
                theOtherAssetInbox.ReleaseSignatures();
                theOtherCurrencyInbox.ReleaseSignatures();

                // Sign all four of them.
                theFirstAssetInbox.SignContract(*pServerNym);
                theFirstCurrencyInbox.SignContract(*pServerNym);
                theOtherAssetInbox.SignContract(*pServerNym);
                theOtherCurrencyInbox.SignContract(*pServerNym);

                // Save all four of them internally
                theFirstAssetInbox.SaveContract();
                theFirstCurrencyInbox.SaveContract();
                theOtherAssetInbox.SaveContract();
                theOtherCurrencyInbox.SaveContract();

                // TODO: Better rollback capabilities in case of failures here:

                // Save the four inboxes to storage. (File, DB, wherever it
                // goes.)

                pFirstAssetAcct->SaveInbox(theFirstAssetInbox);
                pFirstCurrencyAcct->SaveInbox(theFirstCurrencyInbox);
                pOtherAssetAcct->SaveInbox(theOtherAssetInbox);
                pOtherCurrencyAcct->SaveInbox(theOtherCurrencyInbox);

                // These correspond to the AddTransaction() calls just above.
                // The actual receipts are stored in separate files now.
                //
                pTrans1->SaveBoxReceipt(theFirstAssetInbox);
                pTrans2->SaveBoxReceipt(theFirstCurrencyInbox);
                pTrans3->SaveBoxReceipt(theOtherAssetInbox);
                pTrans4->SaveBoxReceipt(theOtherCurrencyInbox);

                // Save the four accounts.
                pFirstAssetAcct->ReleaseSignatures();
                pFirstCurrencyAcct->ReleaseSignatures();
                pOtherAssetAcct->ReleaseSignatures();
                pOtherCurrencyAcct->ReleaseSignatures();
                pFirstAssetAcct->SignContract(*pServerNym);
                pFirstCurrencyAcct->SignContract(*pServerNym);
                pOtherAssetAcct->SignContract(*pServerNym);
                pOtherCurrencyAcct->SignContract(*pServerNym);
                pFirstAssetAcct->SaveContract();
                pFirstCurrencyAcct->SaveContract();
                pOtherAssetAcct->SaveContract();
                pOtherCurrencyAcct->SaveContract();
                pFirstAssetAcct->SaveAccount();
                pFirstCurrencyAcct->SaveAccount();
                pOtherAssetAcct->SaveAccount();
                pOtherCurrencyAcct->SaveAccount();
            }
            // If money was short, let's see WHO was short so we can remove his
            // trade.
            // Also, if money was short, inbox notices only go to the rejectees.
            // But if success, then notices go to all four inboxes.
            else {
                otWarn << "Unable to perform trade in OTMarket::"
                       << __FUNCTION__ << "\n";

                // Let's figure out which one it was and remove his trade and
                // offer.
                bool bFirstTraderIsBroke = false, bOtherTraderIsBroke = false;

                // Here's what's going on here:
                // "Figure out if this guy was short, or if it was that guy.
                // Send a notice
                // to the one who got rejected for being short on cash.
                //
                // Else NEITHER was short, so delete them both with no notice.
                //
                // (After checking both asset accounts, there's another If
                // statement below where
                // I repeat this process for the currency accounts as well.)
                //
                // This whole section occurs because even though the trade and
                // offer were valid
                // and good to go, at least one of the four accounts was short
                // of funds.
                //
                if (pAssetAccountToDebit->GetBalance() <
                    lMinIncrementPerRound) {
                    OTItem* pTempItem = nullptr;
                    OTTransaction* pTempTransaction = nullptr;
                    OTLedger* pTempInbox = nullptr;

                    if (pAssetAccountToDebit == pFirstAssetAcct) {
                        pTempItem = pItem1;
                        bFirstTraderIsBroke = true;
                        pTempTransaction = pTrans1;
                        pTempInbox = &theFirstAssetInbox;

                        delete pItem3;
                        pItem3 = nullptr;
                        delete pTrans3;
                        pTrans3 = nullptr;
                    }
                    else // it's the other asset account
                    {
                        pTempItem = pItem3;
                        bOtherTraderIsBroke = true;
                        pTempTransaction = pTrans3;
                        pTempInbox = &theOtherAssetInbox;

                        delete pItem1;
                        pItem1 = nullptr;
                        delete pTrans1;
                        pTrans1 = nullptr;
                    }

                    pTempItem->SetStatus(OTItem::rejection);
                    pTempItem->SignContract(*pServerNym);
                    pTempItem->SaveContract();

                    pTempTransaction->AddItem(*pTempItem);
                    pTempTransaction->SignContract(*pServerNym);
                    pTempTransaction->SaveContract();

                    pTempInbox->AddTransaction(*pTempTransaction);

                    pTempInbox->ReleaseSignatures();
                    pTempInbox->SignContract(*pServerNym);
                    pTempInbox->SaveContract();
                    pTempInbox->SaveInbox();

                    pTempTransaction->SaveBoxReceipt(*pTempInbox);
                }
                else {
                    delete pItem1;
                    pItem1 = nullptr;
                    delete pTrans1;
                    pTrans1 = nullptr;
                    delete pItem3;
                    pItem3 = nullptr;
                    delete pTrans3;
                    pTrans3 = nullptr;
                }

                // This section is identical to the one above, except for the
                // currency accounts.
                //
                if (pCurrencyAccountToDebit->GetBalance() < lPrice) {
                    OTItem* pTempItem = nullptr;
                    OTTransaction* pTempTransaction = nullptr;
                    OTLedger* pTempInbox = nullptr;

                    if (pCurrencyAccountToDebit == pFirstCurrencyAcct) {
                        pTempItem = pItem2;
                        bFirstTraderIsBroke = true;
                        pTempTransaction = pTrans2;
                        pTempInbox = &theFirstCurrencyInbox;

                        delete pItem4;
                        pItem4 = nullptr;
                        delete pTrans4;
                        pTrans4 = nullptr;
                    }
                    else // it's the other asset account
                    {
                        pTempItem = pItem4;
                        bOtherTraderIsBroke = true;
                        pTempTransaction = pTrans4;
                        pTempInbox = &theOtherCurrencyInbox;

                        delete pItem2;
                        pItem2 = nullptr;
                        delete pTrans2;
                        pTrans2 = nullptr;
                    }

                    pTempItem->SetStatus(OTItem::rejection);
                    pTempItem->SignContract(*pServerNym);
                    pTempItem->SaveContract();

                    pTempTransaction->AddItem(*pTempItem);
                    pTempTransaction->SignContract(*pServerNym);
                    pTempTransaction->SaveContract();

                    pTempInbox->AddTransaction(*pTempTransaction);

                    pTempInbox->ReleaseSignatures();
                    pTempInbox->SignContract(*pServerNym);
                    pTempInbox->SaveContract();
                    pTempInbox->SaveInbox();

                    pTempTransaction->SaveBoxReceipt(*pTempInbox);
                }
                else {
                    delete pItem2;
                    pItem2 = nullptr;
                    delete pTrans2;
                    pTrans2 = nullptr;
                    delete pItem4;
                    pItem4 = nullptr;
                    delete pTrans4;
                    pTrans4 = nullptr;
                }

                // If either trader is broke, we flag the trade for removal.
                // No other trades will process against it and it will be
                // removed from market soon.
                //
                if (bFirstTraderIsBroke) theTrade.FlagForRemoval();
                if (bOtherTraderIsBroke) pOtherTrade->FlagForRemoval();

            } // success == false
        }     // all four boxes were successfully loaded or generated.
    }         // "this entire function can be divided..."

    cleanup_four_accounts(pFirstAssetAcct, pFirstCurrencyAcct, pOtherAssetAcct,
                          pOtherCurrencyAcct);
}
// Let's say pBid->Price is $10. He's bidding $10 as his price limit.
// If I was ALREADY selling at $11, then NOTHING HAPPENS. (If we're the only two
// people on the market.)
// If I was ALREADY SELLING at $8, and a $10 bid came in, it would immediately
// process our orders and be done.
// If I was ALREADY BIDDING at $10, and an $8 ask came in, it would immediately
// process our orders and be done.
//
// So the question is, who gets WHAT price? Does the transaction occur at $8 or
// at $10? Do I get my price limit,
// or does he? Do you split the difference? I decided what to do... here is the
// REASONING, then CONCLUSION:
//
// REASONING:
//
// If I'm already selling for $8, and a $10 bid comes in, he's only going to
// choose me since I'm the lowest
// one on the market (and first in line for my price.) Otherwise he has no
// reason not to choose one of the
// others who are also available at that low price, instead of choosing me. The
// server would just pick
// whoever was next in line.
//
// And the other sellers also do have a right to get their sales completed,
// since they ARE willing to sell
// for $8. Obviously none of the other bids wanted me so far, even though I'm
// the best deal on my market,
// or I would have been snapped up already. But I'm still here. The bidder
// shouldn't pay more than fair market
// rate, which means more than whatever my competition is charging, and as long
// as I'm not getting any less
// than my own ask limit, then I HAVE agreed to the price, and it's fair. (I
// always could have set it higher.)
//
// The prices also were different when I came onto the market. Things were
// different then. Obviously
// since I'm still here, I wasn't ALWAYS the lowest price. Maybe in fact the
// price was $3 before, and I had a
// int64_t-standing trade there that said not to sell for less than $8 (with a
// stop order too, so it didn't even
// activate until then.) THE POINT? I COULD have had the best price on the
// market THEN, whatever it was, simply
// by checking it and then setting my limit to match. But I didn't choose that.
// Instead, I set it to $8 limit,
// and then my trade sat there waiting for 6 months or god knows how int64_t
// until it became valid, when market
// conditions became more favorable to my trade.
// THEN my trade, at some point, became the lowest price on the market (finally)
// so when someone's brand
// new $10 limit bid comes in, he ALSO deserves the best price on the market,
// just as I had the same
// opportunity to get the best price when *I* first entered the market. And
// since I am now first in line with
// $8 as the best price in the market, he should get it at that $8 price. We
// have both agreed to it! It is
// within BOTH of our limits! Notice we also BOTH got our fair shot up front to
// have the absolute best price,
// and instead we set our limit outside of what prices were available in order
// to wait for better offers.
//
//
// CONCLUSION:
//
// THEREFORE: The new Trade should always get the better deal in this function,
// in the sense that he gets
// the best price possible (the limit) from the other trade (that was already on
// the market.) And he gets the
// number-one best deal available from that side of the market. The existing
// trade does NOT get theOffer's
// limit price, but theOffer DOES get the other trade's limit price. This is how
// it should work. We are not
// going to "split the difference" -- although we might split a percentage off
// the difference as a server fee.
// I haven't thought that through yet (it's an idea suggested by Andrew Muck.)

// Return True if Trade should stay on the Cron list for more processing.
// Return False if it should be removed and deleted.
bool OTMarket::ProcessTrade(OTTrade& theTrade, OTOffer& theOffer)
{
    if (theOffer.GetAmountAvailable() < theOffer.GetMinimumIncrement()) {
        otOut << "OTMarket::" << __FUNCTION__ << ": Removing offer from "
                                                 "market. (Amount Available is "
                                                 "less than Min Increment.)\n";
        return false;
    }

    int64_t lRelevantPrice = 0;

    // If I'm trying to SELL something, then I care about the highest bidder.
    if (theOffer.IsAsk()) lRelevantPrice = GetHighestBidPrice();

    // ...But if I'm trying to BUY something, then I care about the lowest ask
    // price.
    else
        lRelevantPrice = GetLowestAskPrice();

    // If there were no relevant offers (or if they were all market orders, aka:
    // 0 == lRelevantPrice),
    // AND if *I* am a market order, then REMOVE me from the market. (Market
    // orders only process once,
    // so if we are processing here and we haven't found any potential matches,
    // we're REMOVED.)
    //
    if ((0 == lRelevantPrice) &&
        theOffer.IsMarketOrder()) // Market order has 0 price.
        return false;

    // If there were no bids/asks (whichever is relevant to this trade) on the
    // market at ALL,
    // then lRelevant price will be 0 at this point. In which case we're DONE.
    //
    // If I'm selling, and the highest bid is less than my price limit, then
    // we're DONE.
    // If I'm buying, and the lowest ask price is higher than my price limit,
    // then we're DONE.
    //
    if ((0 == lRelevantPrice) || // If 0, we KNOW we're not a market order,
                                 // since we would have returned already
                                 // (above.)

        // We only care about price-based restrictions if we're a limit order.
        // (Market orders don't care about price, so they skip this condition.)
        (theOffer.IsLimitOrder() &&
         ((theOffer.IsAsk() && (lRelevantPrice < theOffer.GetPriceLimit())) ||
          (theOffer.IsBid() && (lRelevantPrice > theOffer.GetPriceLimit()))))) {
        // There is nothing on the market currently within my price limits.
        // We're DONE. (For now.)
        return true;
    }

    // If I got this far, that means there ARE bidders or sellers (whichever the
    // current trade cares about)
    // in the market WITHIN THIS TRADE'S PRICE LIMITS. So we're going to go up
    // the list of what's available, and trade.

    if (theOffer.IsAsk()) // If I'm selling,
    {
        // rbegin puts us on the upper bound of the highest bidder (any new
        // bidders at the same price would
        // be added at the lower bound, where they are last in line.) The upper
        // bound, on the other hand, is
        // first in line.  So we start there, and loop backwards until there are
        // no other bids within my price range.
        for (mapOfOffers::reverse_iterator rr = m_mapBids.rbegin();
             rr != m_mapBids.rend(); ++rr) {
            // then I want to start at the highest bidder and loop DOWN until
            // hitting my price limit.
            OTOffer* pBid = rr->second;
            OT_ASSERT(nullptr != pBid);

            // NOTE: Market orders only process once, and they are processed in
            // the order they were added to the market.
            //
            // If BOTH offers are market orders, we just skip this bid.
            //
            // But FURTHERMORE: We ONLY process a market order as theOffer, not
            // as pBid!
            // Imagine if pBid is a market order and theOffer isn't -- that
            // would mean pBid
            // hasn't been processed yet (since it will only process once.) So
            // it needs to
            // wait its turn! It will get its one shot WHEN ITS TURN comes.
            //
            if (pBid->IsMarketOrder())
                //          if (theOffer.IsMarketOrder() &&
                // pBid->IsMarketOrder())
                //              continue;
                break;
            // NOTE: Why break, instead of continue? Because since we are
            // looping through the bids,
            // from the HIGHEST down to the LOWEST, and since market orders have
            // a ZERO price, we know
            // for a fact that there are not any other non-zero bids. (So we
            // might as well break.)

            // I'm selling.
            //
            // If the bid is larger than, or equal to, my low-side-limit,
            // and the amount available is at least my minimum increment, (and
            // vice versa),
            // ...then let's trade!
            //
            if (theOffer.IsMarketOrder() || // If I don't care about price...
                (pBid->GetPriceLimit() >=
                 theOffer.GetPriceLimit())) // Or if this bid is within my price
                                            // range...
            {
                // Notice the above "if" is ONLY based on price... because the
                // "else" returns!
                // (Once I am out of my price range, no point to continue
                // looping.)
                //
                // ...So all the other "if"s have to go INSIDE the block here:
                //
                if ((pBid->GetAmountAvailable() >=
                     theOffer.GetMinimumIncrement()) &&
                    (theOffer.GetAmountAvailable() >=
                     pBid->GetMinimumIncrement()) &&
                    (nullptr != pBid->GetTrade()) &&
                    !pBid->GetTrade()->IsFlaggedForRemoval())

                    ProcessTrade(theTrade, theOffer, *pBid); // <========
            }

            // Else, the bid is lower than I am willing to sell. (And all the
            // remaining bids are even lower.)
            //
            else if (theOffer.IsLimitOrder()) {
                pBid = nullptr;
                return true; // stay on cron for more processing (for now.)
            }

            // The offer has no more trading to do--it's done.
            if (theTrade.IsFlaggedForRemoval() || // during processing, the
                                                  // trade may have gotten
                                                  // flagged.
                (theOffer.GetMinimumIncrement() >
                 theOffer.GetAmountAvailable()))
                return false; // remove this trade from cron

            pBid = nullptr;
        }
    }
    // I'm buying
    else {
        // Begin puts us on the lower bound of the lowest seller (any new
        // sellers at the same price would
        // be added at the upper bound for that price, where they are last in
        // line.) The lower bound, on the other hand, is
        // first in line.  So we start there, and loop forwards until there are
        // no other asks within my price range.
        //
        for (auto& it : m_mapAsks) {
            // then I want to start at the lowest seller and loop UP until
            // hitting my price limit.
            OTOffer* pAsk = it.second;
            OT_ASSERT(nullptr != pAsk);

            // NOTE: Market orders only process once, and they are processed in
            // the order they were added to the market.
            //
            // If BOTH offers are market orders, we just skip this ask.
            //
            // But FURTHERMORE: We ONLY process a market order as theOffer, not
            // as pAsk!
            // Imagine if pAsk is a market order and theOffer isn't -- that
            // would mean pAsk
            // hasn't been processed yet (since it will only process once.) So
            // it needs to
            // wait its turn! It will get its one shot WHEN ITS TURN comes.
            //
            if (pAsk->IsMarketOrder())
                //          if (theOffer.IsMarketOrder() &&
                // pAsk->IsMarketOrder())
                continue;

            // I'm buying.
            // If the ask price is less than, or equal to, my price limit,
            // and the amount available for purchase is at least my minimum
            // increment, (and vice versa),
            // ...then let's trade!
            //
            if (theOffer.IsMarketOrder() || // If I don't care about price...
                (pAsk->GetPriceLimit() <=
                 theOffer.GetPriceLimit())) // Or if this ask is within my price
                                            // range...
            {
                // Notice the above "if" is ONLY based on price... because the
                // "else" returns!
                // (Once I am out of my price range, no point to continue
                // looping.)
                // So all the other "if"s have to go INSIDE the block here:
                //
                if ((pAsk->GetAmountAvailable() >=
                     theOffer.GetMinimumIncrement()) &&
                    (theOffer.GetAmountAvailable() >=
                     pAsk->GetMinimumIncrement()) &&
                    (nullptr != pAsk->GetTrade()) &&
                    !pAsk->GetTrade()->IsFlaggedForRemoval())

                    ProcessTrade(theTrade, theOffer, *pAsk); // <=======
            }
            // Else, the ask price is higher than I am willing to pay. (And all
            // the remaining sellers are even HIGHER.)
            else if (theOffer.IsLimitOrder()) {
                pAsk = nullptr;
                return true; // stay on the market for now.
            }

            // The offer has no more trading to do--it's done.
            if (theTrade.IsFlaggedForRemoval() || // during processing, the
                                                  // trade may have gotten
                                                  // flagged.
                (theOffer.GetMinimumIncrement() >
                 theOffer.GetAmountAvailable()))
                return false; // remove this trade from the market.

            pAsk = nullptr;
        }
    }

    // Market orders only process once.
    // (So tell the caller to remove it.)
    //
    if (theOffer.IsMarketOrder()) return false; // remove from market.

    return true; // stay on the market for now.
}

// Make sure the offer is for the right asset type, the right currency, etc.
bool OTMarket::ValidateOfferForMarket(OTOffer& theOffer, OTString* pReason)
{
    bool bValidOffer = true;
    OTString strReason("");

    if (GetServerID() != theOffer.GetServerID()) {
        bValidOffer = false;
        const OTString strID(GetServerID()), strOtherID(theOffer.GetServerID());
        strReason.Format("Wrong Server ID on offer. Expected %s, but found %s",
                         strID.Get(), strOtherID.Get());
    }
    else if (GetAssetID() != theOffer.GetAssetID()) {
        bValidOffer = false;
        const OTString strID(GetAssetID()), strOtherID(theOffer.GetAssetID());
        strReason.Format("Wrong Asset ID on offer. Expected %s, but found %s",
                         strID.Get(), strOtherID.Get());
    }
    else if (GetCurrencyID() != theOffer.GetCurrencyID()) {
        bValidOffer = false;
        const OTString strID(GetCurrencyID()),
            strOtherID(theOffer.GetCurrencyID());
        strReason.Format(
            "Wrong Currency ID on offer. Expected %s, but found %s",
            strID.Get(), strOtherID.Get());
    }
    else if (GetScale() != theOffer.GetScale()) {
        bValidOffer = false;
        strReason.Format(
            "Wrong Market Scale on offer. Expected %lld, but found %lld",
            GetScale(), theOffer.GetScale());
    }

    // The above four items must match in order for it to even be the same
    // MARKET.
    else if (theOffer.GetMinimumIncrement() <= 0) {
        bValidOffer = false;
        strReason.Format("Minimum Increment on offer is <= 0: %lld",
                         theOffer.GetMinimumIncrement());
    }
    else if (theOffer.GetMinimumIncrement() < GetScale()) {
        bValidOffer = false;
        strReason.Format("Minimum Increment on offer (%lld) is less than "
                         "market scale (%lld).",
                         theOffer.GetMinimumIncrement(), GetScale());
    }
    else if ((theOffer.GetMinimumIncrement() % GetScale()) != 0) {
        bValidOffer = false;
        strReason.Format("Minimum Increment on offer (%lld) Mod market scale "
                         "(%lld) is not equal to zero.",
                         theOffer.GetMinimumIncrement(), GetScale());
    }
    else if (theOffer.GetMinimumIncrement() > theOffer.GetAmountAvailable()) {
        bValidOffer = false;
        strReason.Format(
            "Minimum Increment on offer (%lld) is more than the amount of "
            "assets available for trade on that same offer (%lld).",
            theOffer.GetMinimumIncrement(), theOffer.GetAmountAvailable());
    }

    if (bValidOffer)
        otLog4 << "Offer is valid for market.\n";
    else {
        otOut << __FUNCTION__
              << ": Offer is invalid for this market: " << strReason << "\n";

        if (nullptr != pReason) *pReason = strReason;
    }

    return bValidOffer;
}

OTMarket::OTMarket(const char* szFilename)
    : OTContract()
    , m_pCron(nullptr)
    , m_pTradeList(nullptr)
    , m_lScale(1)
    , m_lLastSalePrice(0)
{
    OT_ASSERT(nullptr != szFilename);

    m_pCron = nullptr; // just for convenience, not responsible to delete.
    InitMarket();

    m_strFilename.Set(szFilename);
    m_strFoldername.Set(OTFolders::Market().Get());
}

OTMarket::OTMarket()
    : OTContract()
    , m_pCron(nullptr)
    , m_pTradeList(nullptr)
    , m_lScale(1)
    , m_lLastSalePrice(0)
{
    m_pCron = nullptr; // just for convenience, not responsible to delete.
    InitMarket();
}

OTMarket::OTMarket(const OTIdentifier& SERVER_ID,
                   const OTIdentifier& ASSET_TYPE_ID,
                   const OTIdentifier& CURRENCY_TYPE_ID, const int64_t& lScale)
    : OTContract()
    , m_pCron(nullptr)
    , m_pTradeList(nullptr)
    , m_lScale(1)
    , m_lLastSalePrice(0)
{
    m_pCron = nullptr; // just for convenience, not responsible to delete.
    InitMarket();

    m_ASSET_TYPE_ID = ASSET_TYPE_ID;
    m_CURRENCY_TYPE_ID = CURRENCY_TYPE_ID;

    m_SERVER_ID = SERVER_ID;

    SetScale(lScale);
}

OTMarket::~OTMarket()
{
    Release_Market();
}

void OTMarket::InitMarket()
{
    m_strContractType = "MARKET";

    SetScale(1);
}

void OTMarket::Release_Market()
{
    m_ASSET_TYPE_ID.clear();
    m_CURRENCY_TYPE_ID.clear();

    m_SERVER_ID.clear();

    // Elements of this list are cleaned up automatically.
    if (nullptr != m_pTradeList) {
        delete m_pTradeList;
        m_pTradeList = nullptr;
    }

    // If there were any dynamically allocated objects, clean them up here.
    while (!m_mapBids.empty()) {
        OTOffer* pOffer = m_mapBids.begin()->second;
        m_mapBids.erase(m_mapBids.begin());
        delete pOffer;
        pOffer = nullptr;
    }
    while (!m_mapAsks.empty()) {
        OTOffer* pOffer = m_mapAsks.begin()->second;
        m_mapAsks.erase(m_mapAsks.begin());
        delete pOffer;
        pOffer = nullptr;
    }
}

void OTMarket::Release()
{
    Release_Market(); // since I've overridden the base class, I call it now...

    ot_super::Release(); // since I've overridden the base class, I call it
                         // now...

    // Then I call this to re-initialize everything (just out of convenience.
    // Not always the right move.)
    InitMarket();
}

} // namespace opentxs
