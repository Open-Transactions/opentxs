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

#include "opentxs/core/stdafx.hpp"

#include "opentxs/core/cron/OTCron.hpp"

#include "opentxs/core/cron/OTCronItem.hpp"
#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/trade/OTMarket.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/OTFolders.hpp"
#include "opentxs/core/util/StringUtils.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/core/util/Timer.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/core/OTStringXML.hpp"
#include "opentxs/core/String.hpp"

#include <irrxml/irrXML.hpp>
#include <string.h>
#include <cstdint>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <utility>

namespace opentxs
{

// Note: these are only code defaults -- the values are actually loaded from
// ~/.ot/server.cfg.
int32_t OTCron::__trans_refill_amount =
    500; // The number of transaction numbers Cron will grab for itself, when it
         // gets low, before each round.
int32_t OTCron::__cron_ms_between_process = 10000; // The number of milliseconds
                                                   // (ideally) between each
                                                   // "Cron Process" event.

int32_t OTCron::__cron_max_items_per_nym = 10; // The maximum number of cron
                                               // items any given Nym can have
                                               // active at the same time.

Timer OTCron::tCron(true);

// Make sure Server Nym is set on this cron object before loading or saving,
// since it's
// used for signing and verifying..
bool OTCron::LoadCron()
{
    const char* szFoldername = OTFolders::Cron().Get();
    const char* szFilename = "OT-CRON.crn"; // todo stop hardcoding filenames.

    OT_ASSERT(nullptr != GetServerNym());

    bool bSuccess = LoadContract(szFoldername, szFilename);

    if (bSuccess) bSuccess = VerifySignature(*(GetServerNym()));

    return bSuccess;
}

bool OTCron::SaveCron()
{
    const char* szFoldername = OTFolders::Cron().Get();
    const char* szFilename = "OT-CRON.crn"; // todo stop hardcoding filenames.

    OT_ASSERT(nullptr != GetServerNym());

    ReleaseSignatures();

    // Sign it, save it internally to string, and then save that out to the
    // file.
    if (!SignContract(*m_pServerNym) || !SaveContract() ||
        !SaveContract(szFoldername, szFilename)) {
        otErr << "Error saving main Cronfile:\n" << szFoldername
              << Log::PathSeparator() << szFilename << "\n";
        return false;
    }
    else
        return true;
}

// Loops through ALL markets, and calls pMarket->GetNym_OfferList(NYM_ID,
// *pOfferList) for each.
// Returns a list of all the offers that a specific Nym has on all the markets.
//
bool OTCron::GetNym_OfferList(OTASCIIArmor& ascOutput, const Identifier& NYM_ID,
                              int32_t& nOfferCount)
{
    nOfferCount = 0; // Outputs the number of offers on this nym.

    std::unique_ptr<OTDB::OfferListNym> pOfferList(
        dynamic_cast<OTDB::OfferListNym*>(
            OTDB::CreateObject(OTDB::STORED_OBJ_OFFER_LIST_NYM)));

    for (auto& it : m_mapMarkets) {
        OTMarket* pMarket = it.second;
        OT_ASSERT(nullptr != pMarket);

        int32_t nNymOfferCount = 0;

        if (false ==
            pMarket->GetNym_OfferList(NYM_ID, *pOfferList,
                                      nNymOfferCount)) // appends to
                                                       // *pOfferList,
                                                       // each
                                                       // iteration.
        {
            // may wish to add a log later. Anyway, keep iterationg and
            // appending, then send back whatever we have.
        }
        else // Success!
            nOfferCount += nNymOfferCount;
    }

    // Now pack the list into strOutput...
    if (nOfferCount == 0)
        return true; // Success, but 0 offers being returned. (List is empty.)

    else if (nOfferCount > 0) {
        OTDB::Storage* pStorage = OTDB::GetDefaultStorage();
        OT_ASSERT(nullptr != pStorage);

        OTDB::OTPacker* pPacker =
            pStorage->GetPacker(); // No need to check for failure, since this
                                   // already ASSERTS. No need to cleanup
                                   // either.

        std::unique_ptr<OTDB::PackedBuffer> pBuffer(
            pPacker->Pack(*pOfferList)); // Now we PACK our nym's offer list.

        if (nullptr == pBuffer) {
            otErr
                << "Failed packing pOfferList in OTCron::GetNym_OfferList. \n";
            return false;
        }

        // Now we need to translate pBuffer into strOutput.

        const uint8_t* pUint = pBuffer->GetData();
        const size_t theSize = pBuffer->GetSize();

        if ((nullptr != pUint) || (theSize < 2)) {
            Data theData(pUint, static_cast<uint32_t>(theSize));

            // This function will base64 ENCODE theData,
            // and then Set() that as the string contents.
            ascOutput.SetData(theData);

            return true;
        }
        else
            otErr << "Null returned, or bad size, while getting buffer data in "
                     "OTCron::GetNym_OfferList.\n";
    }
    else
        otErr
            << "Error: Less-than-zero nOfferCount in OTCron::GetNym_OfferList: "
            << nOfferCount << ".\n";

    return false;
}

bool OTCron::GetMarketList(OTASCIIArmor& ascOutput, int32_t& nMarketCount)
{
    nMarketCount = 0; // This parameter is set to zero here, and incremented in
                      // the loop below.

    OTMarket* pMarket = nullptr;

    std::unique_ptr<OTDB::MarketList> pMarketList(
        dynamic_cast<OTDB::MarketList*>(
            OTDB::CreateObject(OTDB::STORED_OBJ_MARKET_LIST)));

    for (auto& it : m_mapMarkets) {
        pMarket = it.second;
        OT_ASSERT(nullptr != pMarket);

        std::unique_ptr<OTDB::MarketData> pMarketData(
            dynamic_cast<OTDB::MarketData*>(
                OTDB::CreateObject(OTDB::STORED_OBJ_MARKET_DATA)));

        const Identifier MARKET_ID(*pMarket);
        const String str_MARKET_ID(MARKET_ID);
        const String str_NotaryID(pMarket->GetNotaryID());
        const String str_INSTRUMENT_DEFINITION_ID(
            pMarket->GetInstrumentDefinitionID());
        const String str_CURRENCY_ID(pMarket->GetCurrencyID());

        pMarketData->notary_id = str_NotaryID.Get();
        pMarketData->market_id = str_MARKET_ID.Get();
        pMarketData->instrument_definition_id =
            str_INSTRUMENT_DEFINITION_ID.Get();
        pMarketData->currency_type_id = str_CURRENCY_ID.Get();
        // --------------------------------------------
        const int64_t& lScale = pMarket->GetScale();

        pMarketData->scale = to_string<int64_t>(lScale);

        const uint64_t theCurrentBid = pMarket->GetHighestBidPrice();
        const uint64_t theCurrentAsk = pMarket->GetLowestAskPrice();

        pMarketData->current_bid = to_string<uint64_t>(theCurrentBid);
        pMarketData->current_ask = to_string<uint64_t>(theCurrentAsk);

        const int64_t& lLastSalePrice = pMarket->GetLastSalePrice();
        const int64_t& lTotalAvailableAssets =
            pMarket->GetTotalAvailableAssets();

        pMarketData->total_assets = to_string<int64_t>(lTotalAvailableAssets);
        pMarketData->last_sale_price = to_string<int64_t>(lLastSalePrice);

        pMarketData->last_sale_date = pMarket->GetLastSaleDate();

        const mapOfOffers::size_type theBidCount = pMarket->GetBidCount();
        const mapOfOffers::size_type theAskCount = pMarket->GetAskCount();

        pMarketData->number_bids =
            to_string<mapOfOffers::size_type>(theBidCount);
        pMarketData->number_asks =
            to_string<mapOfOffers::size_type>(theAskCount);

        // In the past 24 hours.
        // (I'm not collecting this data yet, (maybe never), so these values
        // aren't set at all.)
        //
        //        pMarketData->volume_trades        = ???;
        //        pMarketData->volume_assets        = ???;
        //        pMarketData->volume_currency    = ???;
        //
        //        pMarketData->recent_highest_bid    = ???;
        //        pMarketData->recent_lowest_ask    = ???;

        // *pMarketData is CLONED at this time (I'm still responsible to
        // delete.)
        // That's also why I add it here, below: So the data is set right before
        // the cloning occurs.
        //
        pMarketList->AddMarketData(*pMarketData);
        nMarketCount++;
    }

    // Now pack the list into strOutput...
    if (0 == nMarketCount) {
        return true; // Success, but the list contains 0 markets.
    }
    else if (nMarketCount > 0) {
        OTDB::Storage* pStorage = OTDB::GetDefaultStorage();
        OT_ASSERT(nullptr != pStorage);

        OTDB::OTPacker* pPacker =
            pStorage->GetPacker(); // No need to check for failure, since this
                                   // already ASSERTS. No need to cleanup
                                   // either.

        std::unique_ptr<OTDB::PackedBuffer> pBuffer(
            pPacker->Pack(*pMarketList)); // Now we PACK our market list.

        if (nullptr == pBuffer) {
            otErr << "Failed packing pMarketList in OTCron::GetMarketList. \n";
            return false;
        }

        // --------------------------------------------------------

        // Now we need to translate pBuffer into strOutput.

        const uint8_t* pUint = pBuffer->GetData();
        const size_t theSize = pBuffer->GetSize();

        if ((theSize > 0) && (nullptr != pUint)) {
            Data theData(pUint, static_cast<uint32_t>(theSize));

            // This function will base64 ENCODE theData,
            // and then Set() that as the string contents.
            ascOutput.SetData(theData);
            //            bool bSuccessSetData = false;
            //            bSuccessSetData = ascOutput.SetData(theData);

            return true;
        }
        else
            otErr << "OTCron::GetMarketList: 0 size, or null return value, "
                     "while getting raw data from packed buffer.\n";
    }
    else
        otErr << "OTCron::GetMarketList: nMarketCount is less than zero: "
              << nMarketCount << ".\n";

    return false;
}

int32_t OTCron::GetTransactionCount() const
{
    if (m_listTransactionNumbers.empty()) return 0;

    return static_cast<int32_t>(m_listTransactionNumbers.size());
}

void OTCron::AddTransactionNumber(const int64_t& lTransactionNum)
{
    m_listTransactionNumbers.push_back(lTransactionNum);
}

// Once this starts returning 0, OTCron can no longer process trades and
// payment plans until the server object replenishes this list.
int64_t OTCron::GetNextTransactionNumber()
{
    if (m_listTransactionNumbers.empty()) return 0;

    int64_t lTransactionNum = m_listTransactionNumbers.front();

    m_listTransactionNumbers.pop_front();

    return lTransactionNum;
}

// return -1 if error, 0 if nothing, and 1 if the node was processed.
int32_t OTCron::ProcessXMLNode(irr::io::IrrXMLReader*& xml)
{
    OT_ASSERT(nullptr != GetServerNym());

    int32_t nReturnVal = 0;

    // Here we call the parent class first.
    // If the node is found there, or there is some error,
    // then we just return either way.  But if it comes back
    // as '0', then nothing happened, and we'll continue executing.
    //
    // -- Note you can choose not to call the parent if
    // you don't want to use any of those xml tags.
    // As I do below, in the case of OTAccount.
    // if (nReturnVal = Contract::ProcessXMLNode(xml))
    //    return nReturnVal;

    if (!strcmp("cron", xml->getNodeName())) {
        m_strVersion = xml->getAttributeValue("version");

        const String strNotaryID(xml->getAttributeValue("notaryID"));

        m_NOTARY_ID.SetString(strNotaryID);

        otOut << "\n\nLoading OTCron for NotaryID: " << strNotaryID << "\n";

        nReturnVal = 1;
    }
    else if (!strcmp("transactionNum", xml->getNodeName())) {
        const int64_t lTransactionNum =
            String::StringToLong(xml->getAttributeValue("value"));

        otWarn << "Transaction Number " << lTransactionNum
               << " available for Cron.\n";

        AddTransactionNumber(lTransactionNum); // This doesn't save to disk.
                                               // Make sure to save Cron when it
                                               // changes.

        nReturnVal = 1;
    }
    else if (!strcmp("cronItem", xml->getNodeName())) {
        const String str_date_added = xml->getAttributeValue("dateAdded");
        const int64_t lDateAdded =
            (!str_date_added.Exists() ? 0
                                      : parseTimestamp(str_date_added.Get()));
        const time64_t tDateAdded = OTTimeGetTimeFromSeconds(lDateAdded);

        String strData;

        if (!Contract::LoadEncodedTextField(xml, strData) ||
            !strData.Exists()) {
            otErr << "Error in OTCron::ProcessXMLNode: cronItem field without "
                     "value.\n";
            return (-1); // error condition
        }
        else {
            OTCronItem* pItem = OTCronItem::NewCronItem(strData);

            if (nullptr == pItem) {
                otErr << "Unable to create cron item from data in cron file.\n";
                return (-1);
            }

            // Why not do this here (when loading from storage), as well as when
            // first adding the item to cron,
            // and thus save myself the trouble of verifying the signature EVERY
            // ITERATION of ProcessCron().
            //
            if (!pItem->VerifySignature(*m_pServerNym)) {
                otErr << "OTCron::ProcessXMLNode: ERROR SECURITY: Server "
                         "signature failed to "
                         "verify on a cron item while loading: "
                      << pItem->GetTransactionNum() << "\n";
                delete pItem;
                pItem = nullptr;
                return (-1);
            }
            else if (AddCronItem(*pItem, nullptr,
                                   false, // bSaveReceipt=false. The receipt is
                                          // only saved once: When item FIRST
                                          // added to cron...
                                   tDateAdded)) { // ...But here, the item was
                                                  // ALREADY in cron, and is
                                                  // merely being loaded from
                                                  // disk.
                // Thus, it would be wrong to try to create the "original
                // record" as if it were brand
                // new and still had the user's signature on it. (Once added to
                // Cron, the signatures are
                // released and the SERVER signs it from there. That's why the
                // user's version is saved
                // as a receipt in the first place -- so we have a record of the
                // user's authorization.)
                otInfo << "Successfully loaded cron item and added to list.\n";
            }
            else {
                otErr << "OTCron::ProcessXMLNode: Though loaded / verified "
                         "successfully, "
                         "unable to add cron item (from cron file) to cron "
                         "list.\n";
                delete pItem;
                pItem = nullptr;
                return (-1);
            }
        }

        nReturnVal = 1;
    }
    else if (!strcmp("market", xml->getNodeName())) {
        const String strMarketID(xml->getAttributeValue("marketID"));
        const String strInstrumentDefinitionID(
            xml->getAttributeValue("instrumentDefinitionID"));
        const String strCurrencyID(xml->getAttributeValue("currencyID"));

        const int64_t lScale =
            String::StringToLong(xml->getAttributeValue("marketScale"));

        const Identifier INSTRUMENT_DEFINITION_ID(strInstrumentDefinitionID),
            CURRENCY_ID(strCurrencyID);

        otWarn << "Loaded cron entry for Market:\n" << strMarketID << ".\n";

        // LoadMarket() needs this info to do its thing.
        OTMarket* pMarket = new OTMarket(m_NOTARY_ID, INSTRUMENT_DEFINITION_ID,
                                         CURRENCY_ID, lScale);

        OT_ASSERT(nullptr != pMarket);

        pMarket->SetCronPointer(
            *this); // This way every Market has a pointer to Cron.

        //    AddMarket normally saves to file, but we don't want that when
        // we're LOADING from file, now do we?
        if (!pMarket->LoadMarket() ||
            !pMarket->VerifySignature(*GetServerNym()) ||
            !AddMarket(*pMarket, false)) // bSaveFile=false: don't save this
                                         // file WHILE loading it!!!
        {
            otErr << "Somehow error while loading, verifying, or adding market "
                     "while loading Cron file.\n";
            delete pMarket;
            pMarket = nullptr;
            return (-1);
        }
        else {
            otWarn << "Loaded market entry from cronfile, and also loaded the "
                      "market file itself.\n";
        }
        nReturnVal = 1;
    }

    return nReturnVal;
}

void OTCron::UpdateContents()
{
    // I release this because I'm about to repopulate it.
    m_xmlUnsigned.Release();

    const String NOTARY_ID(m_NOTARY_ID);

    Tag tag("cron");

    tag.add_attribute("version", m_strVersion.Get());
    tag.add_attribute("notaryID", NOTARY_ID.Get());

    // Save the Market entries (the markets themselves are saved in a markets
    // folder.)
    for (auto& it : m_mapMarkets) {
        OTMarket* pMarket = it.second;
        OT_ASSERT(nullptr != pMarket);

        Identifier MARKET_ID(*pMarket);
        String str_MARKET_ID(MARKET_ID);

        String str_INSTRUMENT_DEFINITION_ID(
            pMarket->GetInstrumentDefinitionID());
        String str_CURRENCY_ID(pMarket->GetCurrencyID());

        TagPtr tagMarket(new Tag("market"));
        tagMarket->add_attribute("marketID", str_MARKET_ID.Get());
        tagMarket->add_attribute("instrumentDefinitionID",
                                 str_INSTRUMENT_DEFINITION_ID.Get());
        tagMarket->add_attribute("currencyID", str_CURRENCY_ID.Get());
        tagMarket->add_attribute("marketScale",
                                 formatLong(pMarket->GetScale()));
        tag.add_tag(tagMarket);
    }

    // Save the Cron Items
    for (auto& it : m_multimapCronItems) {
        OTCronItem* pItem = it.second;
        OT_ASSERT(nullptr != pItem);

        time64_t tDateAdded = it.first;
        String strItem(
            *pItem); // Extract the cron item contract into string form.
        OTASCIIArmor ascItem(strItem); // Base64-encode that for storage.

        TagPtr tagCronItem(new Tag("cronItem", ascItem.Get()));
        tagCronItem->add_attribute("dateAdded", formatTimestamp(tDateAdded));
        tag.add_tag(tagCronItem);
    }

    // Save the transaction numbers.
    //
    for (auto& lTransactionNumber : m_listTransactionNumbers) {
        TagPtr tagNumber(new Tag("transactionNum"));
        tagNumber->add_attribute("value", formatLong(lTransactionNumber));
        tag.add_tag(tagNumber);
    } // for

    std::string str_result;
    tag.output(str_result);

    m_xmlUnsigned.Concatenate("%s", str_result.c_str());
}

int64_t OTCron::computeTimeout()
{
    return OTCron::GetCronMsBetweenProcess() - tCron.getElapsedTimeInMilliSec();
}

// Make sure to call this regularly so the CronItems get a chance to process and
// expire.
void OTCron::ProcessCronItems()
{
    if (!m_bIsActivated) {
        otErr << "OTCron::ProcessCronItems: Not activated yet. (Skipping.)\n";
        return;
    }

    // check elapsed time since last items processing
    if (computeTimeout() > 0) {
        return;
    }
    tCron.start();

    const int32_t nTwentyPercent = OTCron::GetCronRefillAmount() / 5;
    if (GetTransactionCount() <= nTwentyPercent) {
        otErr << "WARNING: Cron has fewer than 20 percent of its normal "
                 "transaction number count available since the previous round! "
                 "\n"
                 "That is, " << GetTransactionCount()
              << " are currently available, with a max of "
              << OTCron::GetCronRefillAmount() << ", meaning "
              << OTCron::GetCronRefillAmount() - GetTransactionCount()
              << " were used in the last round alone!!! \n"
                 "SKIPPING THE CRON ITEMS THAT WERE SCHEDULED FOR THIS "
                 "ROUND!!!\n\n";
        return;
    }
    bool bNeedToSave = false;

    // loop through the cron items and tell each one to ProcessCron().
    // If the item returns true, that means leave it on the list. Otherwise,
    // if it returns false, that means "it's done: remove it."
    for (auto it = m_multimapCronItems.begin();
         it != m_multimapCronItems.end();) {
        if (GetTransactionCount() <= nTwentyPercent) {
            otErr << "WARNING: Cron has fewer than 20 percent of its normal "
                     "transaction "
                     "number count available since the previous cron item "
                     "alone! \n"
                     "That is, " << GetTransactionCount()
                  << " are currently available, with a max of "
                  << OTCron::GetCronRefillAmount() << ", meaning "
                  << OTCron::GetCronRefillAmount() - GetTransactionCount()
                  << " were used in the current round alone!!! \n"
                     "SKIPPING THE REMAINDER OF THE CRON ITEMS THAT WERE "
                     "SCHEDULED FOR THIS ROUND!!!\n\n";
            break;
        }
        OTCronItem* pItem = it->second;
        OT_ASSERT(nullptr != pItem);
        otInfo << "OTCron::" << __FUNCTION__
               << ": Processing item number: " << pItem->GetTransactionNum()
               << " \n";

        if (pItem->ProcessCron()) {
            it++;
            continue;
        }
        pItem->HookRemovalFromCron(nullptr, GetNextTransactionNumber());
        otOut << "OTCron::" << __FUNCTION__
              << ": Removing cron item: " << pItem->GetTransactionNum() << "\n";
        it = m_multimapCronItems.erase(it);
        auto it_map = FindItemOnMap(pItem->GetTransactionNum());
        OT_ASSERT(m_mapCronItems.end() != it_map);
        m_mapCronItems.erase(it_map);

        delete pItem;
        pItem = nullptr;

        bNeedToSave = true;
    }
    if (bNeedToSave) SaveCron();
}

// OTCron IS responsible for cleaning up theItem, and takes ownership.
// So make SURE it is allocated on the HEAP before you pass it in here, and
// also make sure to delete it again if this call fails!
bool OTCron::AddCronItem(OTCronItem& theItem, Nym* pActivator,
                         bool bSaveReceipt, time64_t tDateAdded)
{
    OT_ASSERT(nullptr != GetServerNym());

    // See if there's something else already there with the same transaction
    // number.
    OTCronItem* pCronItem = GetItemByOfficialNum(theItem.GetTransactionNum());

    // If it's not already on the list, then add it...
    if (nullptr == pCronItem) {
        // If I've been instructed to save the receipt, and theItem did NOT
        // successfully save the receipt,
        // then return false.
        //
        // This will happen if filesystem problems, but it will also happen if
        // the cron item WAS ALREADY THERE.
        // I don't want to save over it. If I'm trying to save over one that is
        // already there, then THAT is the
        // real problem.
        //
        if (bSaveReceipt &&
            (!theItem.SignContract(*GetServerNym()) || // Notice the server adds
                                                       // its signature
             // before saving the cron receipt to local
             // storage. This way, the server can
             // verify its own signature later, as
             // evidence the file hasn't been tampered
             // with. (BOTH signatures are there
             // now--user's and server's.)
             !theItem.SaveContract() || !theItem.SaveCronReceipt())) {
            otErr << __FUNCTION__ << ": Error saving receipt while adding new "
                                     "CronItem to Cron.\n";
            return false;
        }

        // Insert to the MAP (by Transaction Number)
        //
        m_mapCronItems.insert(std::pair<int64_t, OTCronItem*>(
            theItem.GetTransactionNum(), &theItem));

        // Insert to the MULTIMAP (by Date)
        //
        m_multimapCronItems.insert(
            m_multimapCronItems.upper_bound(tDateAdded),
            std::pair<time64_t, OTCronItem*>(tDateAdded, &theItem));

        theItem.SetCronPointer(*this);
        theItem.setServerNym(m_pServerNym);
        theItem.setNotaryID(&m_NOTARY_ID);

        bool bSuccess = true;

        theItem.HookActivationOnCron(
            pActivator,    // (OTPseudonym* pActivator) // sometimes nullptr.
            bSaveReceipt); // If merely being reloaded after server reboot, this
                           // is false.
        // But if actually being activated for the first time, then this is
        // true.

        // When an item is added to Cron for the first time, a copy of it is
        // saved to the
        // cron folder, and it has the user's original signature on it. (If it's
        // a Trade,
        // it also contains an Offer with the user's original signature.) This
        // occurs
        // wherever this function is called with bSaveReceipt=true.
        //
        if (bSaveReceipt) // This executes only the first time that an item is
                          // added to Cron.
        // (versus when it's just being reloaded from file and added back to
        // the internal list.)
        {
            // Now that a copy of the cronitem is safely stored, I can release
            // the signature on it
            // and sign it with the Server's Nym instead. That way I can use the
            // server to verify
            // all cron and market-related activity from here on out.
            //            theItem.ReleaseSignatures();
            //            theItem.SignContract(*GetServerNym()); // THIS IS NOW
            // DONE ABOVE. See if (bSaveReceipt) ...
            //            theItem.SaveContract();

            // Since we added an item to the Cron, we SAVE it.
            bSuccess = SaveCron();

            if (bSuccess)
                otOut << __FUNCTION__
                      << ": New CronItem has been added to Cron: "
                      << theItem.GetTransactionNum() << "\n";
            else
                otErr << __FUNCTION__
                      << ": Error saving while adding new CronItem to Cron: "
                      << theItem.GetTransactionNum() << "\n";
        }

        return bSuccess;
    }
    // Otherwise, if it was already there, log an error.
    else {
        otErr << __FUNCTION__
              << ": Failed attempt to add CronItem with pre-existing "
                 "transaction number: " << theItem.GetTransactionNum() << "\n";
    }

    return false;
}

bool OTCron::RemoveCronItem(int64_t lTransactionNum,
                            Nym& theRemover) // if returns false, item
                                             // wasn't found.
{
    // See if there's a cron item with that transaction number.
    auto it_map = FindItemOnMap(lTransactionNum);

    // If it's not already on the list, then there's nothing to remove.
    if (m_mapCronItems.end() == it_map) {
        otErr << __FUNCTION__
              << ": Attempt to remove non-existent CronItem from OTCron. "
                 "Transaction #: " << lTransactionNum << "\n";
    }

    // Otherwise, if it WAS already there, remove it properly.
    else {
        OTCronItem* pItem = it_map->second;
        //      OT_ASSERT(nullptr != pItem); // Already done in FindItemOnMap.

        // We have to remove it from the multimap as well.
        auto it_multimap = FindItemOnMultimap(lTransactionNum);
        OT_ASSERT(m_multimapCronItems.end() !=
                  it_multimap); // If found on map, MUST be on multimap also.

        pItem->HookRemovalFromCron(&theRemover, GetNextTransactionNumber());

        m_mapCronItems.erase(it_map);           // Remove from MAP.
        m_multimapCronItems.erase(it_multimap); // Remove from MULTIMAP.

        delete pItem;

        // An item has been removed from Cron. SAVE.
        return SaveCron();
    }

    return false;
}

// Look up a transaction by transaction number and see if it is in the map.
// If it is, return an iterator to it, otherwise return m_mapCronItems.end()
//
// Note: only the "official" transaction number will work here.
// If the cron item contains multiple opening numbers, the only one
// that will work in this function is the "official" one, the one that
// belongs to the Nym who actually activated this Cron Item.
//
mapOfCronItems::iterator OTCron::FindItemOnMap(int64_t lTransactionNum)
{
    // See if there's something there with lTransactionNum
    // as its "official" number.
    //
    auto itt = m_mapCronItems.find(lTransactionNum);

    if (itt != m_mapCronItems.end()) // Found it!
    {
        OTCronItem* pItem = itt->second;
        OT_ASSERT((nullptr != pItem));
        OT_ASSERT(pItem->GetTransactionNum() == lTransactionNum);

        return itt;
    }

    return itt;
}

// Look up a transaction by transaction number and see if it is in the multimap.
// If it is, return an iterator to it, otherwise return
// m_multimapCronItems.end()
//
// Note: only the "official" transaction number will work here.
// If the cron item contains multiple opening numbers, the only one
// that will work in this function is the "official" one, the one that
// belongs to the Nym who actually activated this Cron Item.
//
multimapOfCronItems::iterator OTCron::FindItemOnMultimap(
    int64_t lTransactionNum)
{
    auto itt = m_multimapCronItems.begin();

    while (m_multimapCronItems.end() != itt) {
        OTCronItem* pItem = itt->second;
        OT_ASSERT((nullptr != pItem));

        if (pItem->GetTransactionNum() == lTransactionNum) break;

        ++itt;
    }

    return itt;
}

// Look up a transaction by transaction number and see if it is in the map.
// If it is, return a pointer to it, otherwise return nullptr.
//
// Note: only the "official" transaction number will work here.
// If the cron item contains multiple opening numbers, the only one
// that will work in this function is the "official" one, the one that
// belongs to the Nym who actually activated this Cron Item.
//
OTCronItem* OTCron::GetItemByOfficialNum(int64_t lTransactionNum)
{
    // See if there's something there with lTransactionNum
    // as its "official" number.
    //
    auto itt = m_mapCronItems.find(lTransactionNum);

    if (itt != m_mapCronItems.end()) // Found it!
    {
        OTCronItem* pItem = itt->second;
        OT_ASSERT((nullptr != pItem));
        OT_ASSERT(pItem->GetTransactionNum() == lTransactionNum);

        return pItem;
    }

    return nullptr;
}

// Look up a transaction by opening number and see if it is in the map.
// If it is, return a pointer to it, otherwise return nullptr.
//
// Note: The "official" transaction number for a cron item belongs
// to to the Nym who activated it. But there are several "valid"
// opening numbers, each belonging to a different Nym who signed the
// Cron Item.
//
// This function searches based on any valid opening number, not necessarily
// by the one "official" number.
//
OTCronItem* OTCron::GetItemByValidOpeningNum(int64_t lOpeningNum)
{
    // See if there's something there with that transaction number.
    auto itt = m_mapCronItems.find(lOpeningNum);

    if (itt == m_mapCronItems.end()) {
        // We didn't find it as the "official" number, so let's loop
        // through the cron items one-by-one and see if it's a valid
        // opening number. (We searched for the "official" number first,
        // since it will often be the right one, and avoids doing this
        // longer search. Basically for optimization purposes.)
        //
        for (auto& it : m_mapCronItems) {
            OTCronItem* pItem = it.second;
            OT_ASSERT((nullptr != pItem));

            if (pItem->IsValidOpeningNumber(lOpeningNum)) // Todo optimization.
                                                          // Probably can remove
                                                          // this check.
                return pItem;
        }
    }
    // Found it!
    else {
        OTCronItem* pItem = itt->second;
        OT_ASSERT((nullptr != pItem));
        OT_ASSERT(pItem->IsValidOpeningNumber(
            lOpeningNum)); // Todo optimization. Probably can remove this check.

        return pItem;
    }

    return nullptr;
}

// OTCron IS responsible for cleaning up theMarket, and takes ownership.
// So make SURE it is allocated on the HEAP before you pass it in here, and
// also make sure to delete it again if this call fails!
bool OTCron::AddMarket(OTMarket& theMarket, bool bSaveMarketFile)
{
    OT_ASSERT(nullptr != GetServerNym());

    theMarket.SetCronPointer(
        *this); // This way every Market has a pointer to Cron.

    Identifier MARKET_ID(theMarket);
    String str_MARKET_ID(MARKET_ID);
    std::string std_MARKET_ID = str_MARKET_ID.Get();

    // See if there's something else already there with the same market ID.
    auto it = m_mapMarkets.find(std_MARKET_ID);

    // If it's not already on the list, then add it...
    if (it == m_mapMarkets.end()) {
        // If I've been instructed to save the market, and Cron did NOT
        // successfully save the market
        //  (to its own file), then return false.  This will happen if
        // filesystem problems.
        if (bSaveMarketFile && !theMarket.SaveMarket()) {
            otErr
                << "Error saving market file while adding new Market to Cron:\n"
                << std_MARKET_ID << "\n";
            return false;
        }

        m_mapMarkets[std_MARKET_ID] = &theMarket;

        bool bSuccess = true;

        // When Cron serializes, it stores a list of all its markets in the main
        // cron file.
        // The actual markets themselves serialize separately to the market
        // folder.
        //
        if (bSaveMarketFile) // This executes only the first time that a market
                             // is added to Cron.
        // (versus when it's just being reloaded from file and added back to
        // the internal list.)
        {
            // Since we added a market to the Cron, we SAVE it.
            bSuccess = SaveCron(); // If we're loading from file, and
                                   // bSaveMarketFile is false, I don't want to
                                   // save here. that's why it's in this block.

            if (bSuccess)
                otLog3 << "New Market has been added to Cron.\n";
            else
                otErr << "Error saving while adding new Market to Cron.\n";
        }

        return bSuccess;
    }
    // Otherwise, if it was already there, log an error.
    else {
        otErr << "Attempt to add Market that was already there: "
              << std_MARKET_ID << "\n";
    }

    return false;
}

// Create it if it's not there.
OTMarket* OTCron::GetOrCreateMarket(const Identifier& INSTRUMENT_DEFINITION_ID,
                                    const Identifier& CURRENCY_ID,
                                    const int64_t& lScale)
{
    OTMarket* pMarket = new OTMarket(GetNotaryID(), INSTRUMENT_DEFINITION_ID,
                                     CURRENCY_ID, lScale);

    OT_ASSERT(nullptr != pMarket);

    Identifier MARKET_ID(*pMarket);

    OTMarket* pExistingMarket = GetMarket(MARKET_ID);

    // If it was already there, there's no need to create it.
    if (nullptr != pExistingMarket) {
        delete pMarket;
        pMarket = nullptr;
        return pExistingMarket;
    }

    // If we got this far, it means the Market does NOT already exist in this
    // Cron.
    // So let's add it...
    bool bAdded = AddMarket(
        *pMarket, true); // bool bSaveMarketFile=true, since it was created new.

    if (bAdded) {
        otOut << "New market created and added to Cron.\n";
    }
    else {
        otErr << "Error trying to add new market to Cron.\n";
        delete pMarket;
        pMarket = nullptr;
    }

    return pMarket;
}

// Look up a transaction by transaction number and see if it is in the ledger.
// If it is, return a pointer to it, otherwise return nullptr.
OTMarket* OTCron::GetMarket(const Identifier& MARKET_ID)
{
    String str_MARKET_ID(MARKET_ID);
    std::string std_MARKET_ID = str_MARKET_ID.Get();

    // See if there's something there with that transaction number.
    auto it = m_mapMarkets.find(std_MARKET_ID);

    if (it == m_mapMarkets.end()) {
        // nothing found.
        return nullptr;
    }
    // Found it!
    else {
        OTMarket* pMarket = it->second;

        OT_ASSERT((nullptr != pMarket));

        const Identifier LOOP_MARKET_ID(*pMarket);
        const String str_LOOP_MARKET_ID(LOOP_MARKET_ID);

        if (MARKET_ID == LOOP_MARKET_ID)
            return pMarket;
        else
            otErr << "Expected Market with ID:\n" << std_MARKET_ID
                  << "\n but found " << str_LOOP_MARKET_ID << "\n";
    }

    return nullptr;
}

OTCron::OTCron()
    : Contract()
    , m_bIsActivated(false)
    , m_pServerNym(nullptr) // just here for convenience, not responsible to
                            // cleanup this pointer.
{
    InitCron();
    otLog3 << "OTCron::OTCron: Finished calling InitCron 0.\n";
}

OTCron::OTCron(const Identifier& NOTARY_ID)
    : Contract()
    , m_bIsActivated(false)
    , m_pServerNym(nullptr) // just here for convenience, not responsible to
                            // cleanup this pointer.
{
    InitCron();
    SetNotaryID(NOTARY_ID);
    otLog3 << "OTCron::OTCron: Finished calling InitCron 1.\n";
}

OTCron::OTCron(const char* szFilename)
    : Contract()
    , m_bIsActivated(false)
    , m_pServerNym(nullptr) // just here for convenience, not responsible to
                            // cleanup this pointer.
{
    OT_ASSERT(nullptr != szFilename);
    InitCron();

    m_strFoldername.Set(OTFolders::Cron().Get());
    m_strFilename.Set(szFilename);
    otLog3 << "OTCron::OTCron: Finished calling InitCron 2.\n";
}

OTCron::~OTCron()
{
    Release_Cron();

    m_pServerNym = nullptr;
}

void OTCron::InitCron()
{
    m_strContractType = "CRON";
}

void OTCron::Release()
{
    Release_Cron();

    Contract::Release();
}

void OTCron::Release_Cron()
{
    // If there were any dynamically allocated objects, clean them up here.

    while (!m_multimapCronItems.empty()) {
        auto it = m_multimapCronItems.begin();
        m_multimapCronItems.erase(it);

        // We don't delete the pItem in here, since these are the
        // same pItems being deleted in the next block.
    }

    while (!m_mapCronItems.empty()) {
        OTCronItem* pItem = m_mapCronItems.begin()->second;
        auto it = m_mapCronItems.begin();
        m_mapCronItems.erase(it);
        delete pItem;
        pItem = nullptr;
    }

    while (!m_mapMarkets.empty()) {
        OTMarket* pMarket = m_mapMarkets.begin()->second;
        auto it = m_mapMarkets.begin();
        m_mapMarkets.erase(it);
        delete pMarket;
        pMarket = nullptr;
    }
}

} // namespace opentxs
