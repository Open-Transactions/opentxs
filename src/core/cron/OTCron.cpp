// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/cron/OTCron.hpp"

#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/cron/OTCronItem.hpp"
#include "opentxs/core/trade/OTMarket.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/OTFolders.hpp"
#include "opentxs/core/util/StringUtils.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/core/util/Timer.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/core/StringXML.hpp"
#include "opentxs/core/String.hpp"

#include <irrxml/irrXML.hpp>
#include <string.h>
#include <cstdint>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <utility>

#define OT_METHOD "opentxs::OTCron"

namespace opentxs
{
// Note: these are only code defaults -- the values are actually loaded from
// ~/.ot/server.cfg.
std::int32_t OTCron::__trans_refill_amount = 500;  // The number of transaction
                                                   // numbers Cron will grab for
                                                   // itself, when it
// gets low, before each round.
std::int32_t OTCron::__cron_ms_between_process =
    10000;  // The number of milliseconds
            // (ideally) between each
            // "Cron Process" event.

std::int32_t OTCron::__cron_max_items_per_nym =
    10;  // The maximum number of cron
         // items any given Nym can have
         // active at the same time.

Timer OTCron::tCron(true);

OTCron::OTCron(const api::Core& server)
    : Contract(server)
    , m_mapMarkets()
    , m_mapCronItems()
    , m_multimapCronItems()
    , m_NOTARY_ID(Identifier::Factory())
    , m_listTransactionNumbers()
    , m_bIsActivated(false)
    , m_pServerNym(nullptr)  // just here for convenience, not responsible to
                             // cleanup this pointer.
{
    InitCron();
    LogDebug(OT_METHOD)(__FUNCTION__)(": Finished calling InitCron 0.").Flush();
}

// Make sure Server Nym is set on this cron object before loading or saving,
// since it's
// used for signing and verifying..
bool OTCron::LoadCron()
{
    const char* szFoldername = OTFolders::Cron().Get();
    const char* szFilename = "OT-CRON.crn";  // todo stop hardcoding filenames.

    OT_ASSERT(nullptr != GetServerNym());

    bool bSuccess = LoadContract(szFoldername, szFilename);

    if (bSuccess) bSuccess = VerifySignature(*(GetServerNym()));

    return bSuccess;
}

bool OTCron::SaveCron()
{
    const char* szFoldername = OTFolders::Cron().Get();
    const char* szFilename = "OT-CRON.crn";  // todo stop hardcoding filenames.

    OT_ASSERT(nullptr != GetServerNym());

    ReleaseSignatures();

    // Sign it, save it internally to string, and then save that out to the
    // file.
    if (!SignContract(*m_pServerNym) || !SaveContract() ||
        !SaveContract(szFoldername, szFilename)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error saving main Cronfile: ")(
            szFoldername)(Log::PathSeparator())(szFilename)(".")
            .Flush();
        return false;
    } else
        return true;
}

// Loops through ALL markets, and calls pMarket->GetNym_OfferList(NYM_ID,
// *pOfferList) for each.
// Returns a list of all the offers that a specific Nym has on all the markets.
//
bool OTCron::GetNym_OfferList(
    Armored& ascOutput,
    const Identifier& NYM_ID,
    std::int32_t& nOfferCount)
{
    nOfferCount = 0;  // Outputs the number of offers on this nym.

    std::unique_ptr<OTDB::OfferListNym> pOfferList(
        dynamic_cast<OTDB::OfferListNym*>(
            OTDB::CreateObject(OTDB::STORED_OBJ_OFFER_LIST_NYM)));

    for (auto& it : m_mapMarkets) {
        auto pMarket = it.second;
        OT_ASSERT(false != bool(pMarket));

        std::int32_t nNymOfferCount = 0;

        if (false == pMarket->GetNym_OfferList(
                         NYM_ID,
                         *pOfferList,
                         nNymOfferCount))  // appends to
                                           // *pOfferList,
                                           // each
                                           // iteration.
        {
            // may wish to add a log later. Anyway, keep iterationg and
            // appending, then send back whatever we have.
        } else  // Success!
            nOfferCount += nNymOfferCount;
    }

    // Now pack the list into strOutput...
    if (nOfferCount == 0)
        return true;  // Success, but 0 offers being returned. (List is empty.)

    else if (nOfferCount > 0) {
        OTDB::Storage* pStorage = OTDB::GetDefaultStorage();
        OT_ASSERT(nullptr != pStorage);

        OTDB::OTPacker* pPacker =
            pStorage->GetPacker();  // No need to check for failure, since this
                                    // already ASSERTS. No need to cleanup
                                    // either.

        std::unique_ptr<OTDB::PackedBuffer> pBuffer(
            pPacker->Pack(*pOfferList));  // Now we PACK our nym's offer list.

        if (nullptr == pBuffer) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed packing pOfferList in OTCron::GetNym_OfferList.")
                .Flush();
            return false;
        }

        // Now we need to translate pBuffer into strOutput.

        const std::uint8_t* pUint = pBuffer->GetData();
        const size_t theSize = pBuffer->GetSize();

        if ((nullptr != pUint) || (theSize < 2)) {
            auto theData =
                Data::Factory(pUint, static_cast<std::uint32_t>(theSize));

            // This function will base64 ENCODE theData,
            // and then Set() that as the string contents.
            ascOutput.SetData(theData);

            return true;
        } else
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Null returned, or bad size, while getting buffer data in "
                "OTCron::GetNym_OfferList.")
                .Flush();
    } else
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error: Less-than-zero nOfferCount in "
            "OTCron::GetNym_OfferList: ")(nOfferCount)(".")
            .Flush();

    return false;
}

bool OTCron::GetMarketList(Armored& ascOutput, std::int32_t& nMarketCount)
{
    nMarketCount = 0;  // This parameter is set to zero here, and incremented in
                       // the loop below.

    std::unique_ptr<OTDB::MarketList> pMarketList(
        dynamic_cast<OTDB::MarketList*>(
            OTDB::CreateObject(OTDB::STORED_OBJ_MARKET_LIST)));

    for (auto& it : m_mapMarkets) {
        auto pMarket = it.second;
        OT_ASSERT(false != bool(pMarket));

        std::unique_ptr<OTDB::MarketData> pMarketData(
            dynamic_cast<OTDB::MarketData*>(
                OTDB::CreateObject(OTDB::STORED_OBJ_MARKET_DATA)));

        const auto MARKET_ID = Identifier::Factory(*pMarket);
        const auto str_MARKET_ID = String::Factory(MARKET_ID);
        const auto str_NotaryID = String::Factory(pMarket->GetNotaryID());
        const auto str_INSTRUMENT_DEFINITION_ID =
            String::Factory(pMarket->GetInstrumentDefinitionID());
        const auto str_CURRENCY_ID = String::Factory(pMarket->GetCurrencyID());

        pMarketData->notary_id = str_NotaryID->Get();
        pMarketData->market_id = str_MARKET_ID->Get();
        pMarketData->instrument_definition_id =
            str_INSTRUMENT_DEFINITION_ID->Get();
        pMarketData->currency_type_id = str_CURRENCY_ID->Get();
        // --------------------------------------------
        const std::int64_t& lScale = pMarket->GetScale();

        pMarketData->scale = to_string<std::int64_t>(lScale);

        const std::uint64_t theCurrentBid = pMarket->GetHighestBidPrice();
        const std::uint64_t theCurrentAsk = pMarket->GetLowestAskPrice();

        pMarketData->current_bid = to_string<std::uint64_t>(theCurrentBid);
        pMarketData->current_ask = to_string<std::uint64_t>(theCurrentAsk);

        const std::int64_t& lLastSalePrice = pMarket->GetLastSalePrice();
        const std::int64_t& lTotalAvailableAssets =
            pMarket->GetTotalAvailableAssets();

        pMarketData->total_assets =
            to_string<std::int64_t>(lTotalAvailableAssets);
        pMarketData->last_sale_price = to_string<std::int64_t>(lLastSalePrice);

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
        return true;  // Success, but the list contains 0 markets.
    } else if (nMarketCount > 0) {
        OTDB::Storage* pStorage = OTDB::GetDefaultStorage();
        OT_ASSERT(nullptr != pStorage);

        OTDB::OTPacker* pPacker =
            pStorage->GetPacker();  // No need to check for failure, since this
                                    // already ASSERTS. No need to cleanup
                                    // either.

        std::unique_ptr<OTDB::PackedBuffer> pBuffer(
            pPacker->Pack(*pMarketList));  // Now we PACK our market list.

        if (nullptr == pBuffer) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed packing pMarketList in OTCron::GetMarketList.")
                .Flush();
            return false;
        }

        // --------------------------------------------------------

        // Now we need to translate pBuffer into strOutput.

        const std::uint8_t* pUint = pBuffer->GetData();
        const size_t theSize = pBuffer->GetSize();

        if ((theSize > 0) && (nullptr != pUint)) {
            auto theData =
                Data::Factory(pUint, static_cast<std::uint32_t>(theSize));

            // This function will base64 ENCODE theData,
            // and then Set() that as the string contents.
            ascOutput.SetData(theData);
            //            bool bSuccessSetData = false;
            //            bSuccessSetData = ascOutput.SetData(theData);

            return true;
        } else
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": 0 size, or null return value, "
                "while getting raw data from packed buffer.")
                .Flush();
    } else
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": nMarketCount is less than zero: ")(nMarketCount)(".")
            .Flush();

    return false;
}

std::int32_t OTCron::GetTransactionCount() const
{
    if (m_listTransactionNumbers.empty()) return 0;

    return static_cast<std::int32_t>(m_listTransactionNumbers.size());
}

void OTCron::AddTransactionNumber(const std::int64_t& lTransactionNum)
{
    m_listTransactionNumbers.push_back(lTransactionNum);
}

// Once this starts returning 0, OTCron can no longer process trades and
// payment plans until the server object replenishes this list.
std::int64_t OTCron::GetNextTransactionNumber()
{
    if (m_listTransactionNumbers.empty()) return 0;

    std::int64_t lTransactionNum = m_listTransactionNumbers.front();

    m_listTransactionNumbers.pop_front();

    return lTransactionNum;
}

// return -1 if error, 0 if nothing, and 1 if the node was processed.
std::int32_t OTCron::ProcessXMLNode(irr::io::IrrXMLReader*& xml)
{
    OT_ASSERT(nullptr != GetServerNym());

    std::int32_t nReturnVal = 0;

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
        m_strVersion = String::Factory(xml->getAttributeValue("version"));

        const auto strNotaryID =
            String::Factory(xml->getAttributeValue("notaryID"));

        m_NOTARY_ID->SetString(strNotaryID);

        LogNormal(OT_METHOD)(__FUNCTION__)(": Loading OTCron for NotaryID: ")(
            strNotaryID)(".")
            .Flush();

        nReturnVal = 1;
    } else if (!strcmp("transactionNum", xml->getNodeName())) {
        const std::int64_t lTransactionNum =
            String::StringToLong(xml->getAttributeValue("value"));

        LogDetail(OT_METHOD)(__FUNCTION__)(": Transaction Number ")(
            lTransactionNum)(" available for Cron.")
            .Flush();

        AddTransactionNumber(lTransactionNum);  // This doesn't save to disk.
        // Make sure to save Cron when it
        // changes.

        nReturnVal = 1;
    } else if (!strcmp("cronItem", xml->getNodeName())) {
        const auto str_date_added =
            String::Factory(xml->getAttributeValue("dateAdded"));
        const std::int64_t lDateAdded =
            (!str_date_added->Exists() ? 0
                                       : parseTimestamp(str_date_added->Get()));
        const time64_t tDateAdded = OTTimeGetTimeFromSeconds(lDateAdded);

        auto strData = String::Factory();

        if (!Contract::LoadEncodedTextField(xml, strData) ||
            !strData->Exists()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error in OTCron::ProcessXMLNode: cronItem field without "
                "value.")
                .Flush();
            return (-1);  // error condition
        } else {
            auto pItem{api_.Factory().CronItem(strData)};

            if (false == bool(pItem)) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Unable to create cron item from data in cron file.")
                    .Flush();
                return (-1);
            }

            // Why not do this here (when loading from storage), as well as when
            // first adding the item to cron,
            // and thus save myself the trouble of verifying the signature EVERY
            // ITERATION of ProcessCron().
            //
            std::shared_ptr<OTCronItem> item{pItem.release()};
            if (!item->VerifySignature(*m_pServerNym)) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": ERROR SECURITY: Server "
                    "signature failed to "
                    "verify on a cron item while loading: ")(
                    item->GetTransactionNum())(".")
                    .Flush();
                return (-1);
            } else if (AddCronItem(
                           item,
                           false,          // bSaveReceipt=false. The receipt is
                                           // only saved once: When item FIRST
                                           // added to cron...
                           tDateAdded)) {  // ...But here, the item was
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
                LogVerbose(OT_METHOD)(__FUNCTION__)(
                    ": Successfully loaded cron item and added to list. ")
                    .Flush();
            } else {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Though loaded / verified "
                    "successfully, "
                    "unable to add cron item (from cron file) to cron "
                    " list.")
                    .Flush();
                return (-1);
            }
        }

        nReturnVal = 1;
    } else if (!strcmp("market", xml->getNodeName())) {
        const auto strMarketID =
            String::Factory(xml->getAttributeValue("marketID"));
        const auto strInstrumentDefinitionID =
            String::Factory(xml->getAttributeValue("instrumentDefinitionID"));
        const auto strCurrencyID =
            String::Factory(xml->getAttributeValue("currencyID"));

        const std::int64_t lScale =
            String::StringToLong(xml->getAttributeValue("marketScale"));

        const OTIdentifier INSTRUMENT_DEFINITION_ID =
                               Identifier::Factory(strInstrumentDefinitionID),
                           CURRENCY_ID = Identifier::Factory(strCurrencyID);

        LogDetail(OT_METHOD)(__FUNCTION__)(": Loaded cron entry for Market: ")(
            strMarketID)(".")
            .Flush();

        // LoadMarket() needs this info to do its thing.
        auto pMarket{api_.Factory().Market(
            m_NOTARY_ID, INSTRUMENT_DEFINITION_ID, CURRENCY_ID, lScale)};

        OT_ASSERT(false != bool(pMarket));

        pMarket->SetCronPointer(*this);  // This way every Market has a pointer
                                         // to Cron.

        // AddMarket normally saves to file, but we don't want that when
        // we're LOADING from file, now do we?
        std::shared_ptr<OTMarket> market{pMarket.release()};
        if (!market->LoadMarket() ||
            !market->VerifySignature(*GetServerNym()) ||
            !AddMarket(market, false))  // bSaveFile=false: don't save this
                                        // file WHILE loading it!!!
        {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Somehow error while loading, verifying, or adding market "
                "while loading Cron file.")
                .Flush();
            return (-1);
        } else {
            LogDetail(OT_METHOD)(__FUNCTION__)(
                ": Loaded market entry from cronfile, and also loaded the "
                "market file itself.")
                .Flush();
        }
        nReturnVal = 1;
    }

    return nReturnVal;
}

void OTCron::UpdateContents()
{
    // I release this because I'm about to repopulate it.
    m_xmlUnsigned->Release();

    const auto NOTARY_ID = String::Factory(m_NOTARY_ID);

    Tag tag("cron");

    tag.add_attribute("version", m_strVersion->Get());
    tag.add_attribute("notaryID", NOTARY_ID->Get());

    // Save the Market entries (the markets themselves are saved in a markets
    // folder.)
    for (auto& it : m_mapMarkets) {
        auto pMarket = it.second;
        OT_ASSERT(false != bool(pMarket));

        auto MARKET_ID = Identifier::Factory(*pMarket);
        auto str_MARKET_ID = String::Factory(MARKET_ID);

        auto str_INSTRUMENT_DEFINITION_ID =
            String::Factory(pMarket->GetInstrumentDefinitionID());
        auto str_CURRENCY_ID = String::Factory(pMarket->GetCurrencyID());

        TagPtr tagMarket(new Tag("market"));
        tagMarket->add_attribute("marketID", str_MARKET_ID->Get());
        tagMarket->add_attribute(
            "instrumentDefinitionID", str_INSTRUMENT_DEFINITION_ID->Get());
        tagMarket->add_attribute("currencyID", str_CURRENCY_ID->Get());
        tagMarket->add_attribute(
            "marketScale", formatLong(pMarket->GetScale()));
        tag.add_tag(tagMarket);
    }

    // Save the Cron Items
    for (auto& it : m_multimapCronItems) {
        auto pItem = it.second;
        OT_ASSERT(false != bool(pItem));

        time64_t tDateAdded = it.first;
        auto strItem = String::Factory(*pItem);  // Extract the cron item
                                                 // contract into string form.
        auto ascItem =
            Armored::Factory(strItem);  // Base64-encode that for storage.

        TagPtr tagCronItem(new Tag("cronItem", ascItem->Get()));
        tagCronItem->add_attribute("dateAdded", formatTimestamp(tDateAdded));
        tag.add_tag(tagCronItem);
    }

    // Save the transaction numbers.
    //
    for (auto& lTransactionNumber : m_listTransactionNumbers) {
        TagPtr tagNumber(new Tag("transactionNum"));
        tagNumber->add_attribute("value", formatLong(lTransactionNumber));
        tag.add_tag(tagNumber);
    }  // for

    std::string str_result;
    tag.output(str_result);

    m_xmlUnsigned->Concatenate("%s", str_result.c_str());
}

std::int64_t OTCron::computeTimeout()
{
    return OTCron::GetCronMsBetweenProcess() - tCron.getElapsedTimeInMilliSec();
}

// Make sure to call this regularly so the CronItems get a chance to process and
// expire.
void OTCron::ProcessCronItems()
{
    if (!m_bIsActivated) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Not activated yet. (Skipping).")
            .Flush();
        return;
    }

    // check elapsed time since last items processing
    if (computeTimeout() > 0) { return; }
    tCron.start();

    const std::int32_t nTwentyPercent = OTCron::GetCronRefillAmount() / 5;
    if (GetTransactionCount() <= nTwentyPercent) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": WARNING: Cron has fewer than 20 percent of its normal "
            "transaction number count available since the previous round! "
            "That is, ")(GetTransactionCount())(
            " are currently available, with a max of ")(GetCronRefillAmount())(
            ", meaning ")(GetCronRefillAmount() - GetTransactionCount())(
            " were used in the last round alone!!! "
            "SKIPPING THE CRON ITEMS THAT WERE SCHEDULED FOR THIS "
            "ROUND!!!")
            .Flush();
        return;
    }
    bool bNeedToSave = false;

    // loop through the cron items and tell each one to ProcessCron().
    // If the item returns true, that means leave it on the list. Otherwise,
    // if it returns false, that means "it's done: remove it."
    for (auto it = m_multimapCronItems.begin();
         it != m_multimapCronItems.end();) {
        if (GetTransactionCount() <= nTwentyPercent) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": WARNING: Cron has fewer than 20 percent of its normal "
                "transaction "
                "number count available since the previous cron item "
                "alone! "
                "That is, ")(GetTransactionCount())(
                " are currently available, with a max of ")(
                GetCronRefillAmount())(", meaning ")(
                GetCronRefillAmount() - GetTransactionCount())(
                " were used in the current round alone!!! "
                "SKIPPING THE REMAINDER OF THE CRON ITEMS THAT WERE "
                "SCHEDULED FOR THIS ROUND!!!")
                .Flush();
            break;
        }
        auto pItem = it->second;
        OT_ASSERT(false != bool(pItem));
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Processing item number: ")(
            pItem->GetTransactionNum())
            .Flush();

        if (pItem->ProcessCron()) {
            it++;
            continue;
        }
        pItem->HookRemovalFromCron(
            api_.Wallet(), nullptr, GetNextTransactionNumber());
        LogNormal(OT_METHOD)(__FUNCTION__)(": Removing cron item: ")(
            pItem->GetTransactionNum())(".")
            .Flush();
        it = m_multimapCronItems.erase(it);
        auto it_map = FindItemOnMap(pItem->GetTransactionNum());
        OT_ASSERT(m_mapCronItems.end() != it_map);
        m_mapCronItems.erase(it_map);

        bNeedToSave = true;
    }
    if (bNeedToSave) SaveCron();
}

// OTCron IS responsible for cleaning up theItem, and takes ownership.
// So make SURE it is allocated on the HEAP before you pass it in here, and
// also make sure to delete it again if this call fails!
bool OTCron::AddCronItem(
    std::shared_ptr<OTCronItem> theItem,
    bool bSaveReceipt,
    time64_t tDateAdded)
{
    OT_ASSERT(nullptr != GetServerNym());

    // See if there's something else already there with the same transaction
    // number.
    auto pCronItem = GetItemByOfficialNum(theItem->GetTransactionNum());

    // If it's not already on the list, then add it...
    if (false == bool(pCronItem)) {
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
            (!theItem->SignContract(*GetServerNym()) ||  // Notice the server
                                                         // adds its signature
             // before saving the cron receipt to local
             // storage. This way, the server can
             // verify its own signature later, as
             // evidence the file hasn't been tampered
             // with. (BOTH signatures are there
             // now--user's and server's.)
             !theItem->SaveContract() || !theItem->SaveCronReceipt())) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error saving receipt while adding new "
                "CronItem to Cron.")
                .Flush();
            return false;
        }

        // Insert to the MAP (by Transaction Number)
        //
        m_mapCronItems.insert(
            std::pair<std::int64_t, std::shared_ptr<OTCronItem>>(
                theItem->GetTransactionNum(), theItem));

        // Insert to the MULTIMAP (by Date)
        //
        m_multimapCronItems.insert(
            m_multimapCronItems.upper_bound(tDateAdded),
            std::pair<time64_t, std::shared_ptr<OTCronItem>>(
                tDateAdded, theItem));

        theItem->SetCronPointer(*this);
        theItem->setServerNym(m_pServerNym);
        theItem->setNotaryID(m_NOTARY_ID);

        bool bSuccess = true;

        theItem->HookActivationOnCron(bSaveReceipt);  // If merely being
                                                      // reloaded after server
                                                      // reboot, this is false.
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
        if (bSaveReceipt)  // This executes only the first time that an item is
                           // added to Cron.
        // (versus when it's just being reloaded from file and added back to
        // the internal list.)
        {
            // Now that a copy of the cronitem is safely stored, I can release
            // the signature on it
            // and sign it with the Server's Nym instead. That way I can use the
            // server to verify
            // all cron and market-related activity from here on out.
            //            theItem->ReleaseSignatures();
            //            theItem->SignContract(*GetServerNym()); // THIS IS NOW
            // DONE ABOVE. See if (bSaveReceipt) ...
            //            theItem->SaveContract();

            // Since we added an item to the Cron, we SAVE it.
            bSuccess = SaveCron();

            if (bSuccess)
                LogNormal(OT_METHOD)(__FUNCTION__)(
                    ": New CronItem has been added to Cron: ")(
                    theItem->GetTransactionNum())(".")
                    .Flush();
            else
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Error saving while adding new CronItem to Cron: ")(
                    theItem->GetTransactionNum())(".")
                    .Flush();
        }

        return bSuccess;
    }
    // Otherwise, if it was already there, log an error.
    else {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed attempt to add CronItem with pre-existing "
            "transaction number: ")(theItem->GetTransactionNum())(".")
            .Flush();
    }

    return false;
}

bool OTCron::RemoveCronItem(
    std::int64_t lTransactionNum,
    ConstNym theRemover)  // if returns false, item
                          // wasn't found.
{
    // See if there's a cron item with that transaction number.
    auto it_map = FindItemOnMap(lTransactionNum);

    // If it's not already on the list, then there's nothing to remove.
    if (m_mapCronItems.end() == it_map) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Attempt to remove non-existent CronItem from OTCron. "
            "Transaction #: ")(lTransactionNum)(".")
            .Flush();
    }

    // Otherwise, if it WAS already there, remove it properly.
    else {
        auto pItem = it_map->second;
        //      OT_ASSERT(nullptr != pItem); // Already done in FindItemOnMap.

        // We have to remove it from the multimap as well.
        auto it_multimap = FindItemOnMultimap(lTransactionNum);
        OT_ASSERT(m_multimapCronItems.end() != it_multimap);  // If found on
                                                              // map, MUST be on
                                                              // multimap also.

        pItem->HookRemovalFromCron(
            api_.Wallet(), theRemover, GetNextTransactionNumber());

        m_mapCronItems.erase(it_map);            // Remove from MAP.
        m_multimapCronItems.erase(it_multimap);  // Remove from MULTIMAP.

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
mapOfCronItems::iterator OTCron::FindItemOnMap(std::int64_t lTransactionNum)
{
    // See if there's something there with lTransactionNum
    // as its "official" number.
    //
    auto itt = m_mapCronItems.find(lTransactionNum);

    if (itt != m_mapCronItems.end())  // Found it!
    {
        auto pItem = itt->second;
        OT_ASSERT(false != bool(pItem));
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
    std::int64_t lTransactionNum)
{
    auto itt = m_multimapCronItems.begin();

    while (m_multimapCronItems.end() != itt) {
        auto pItem = itt->second;
        OT_ASSERT(false != bool(pItem));

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
std::shared_ptr<OTCronItem> OTCron::GetItemByOfficialNum(
    std::int64_t lTransactionNum)
{
    // See if there's something there with lTransactionNum
    // as its "official" number.
    //
    auto itt = m_mapCronItems.find(lTransactionNum);

    if (itt != m_mapCronItems.end())  // Found it!
    {
        auto pItem = itt->second;
        OT_ASSERT(false != bool(pItem));
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
std::shared_ptr<OTCronItem> OTCron::GetItemByValidOpeningNum(
    std::int64_t lOpeningNum)
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
            auto pItem = it.second;
            OT_ASSERT(false != bool(pItem));

            if (pItem->IsValidOpeningNumber(lOpeningNum))  // Todo optimization.
                // Probably can remove
                // this check.
                return pItem;
        }
    }
    // Found it!
    else {
        auto pItem = itt->second;
        OT_ASSERT(false != bool(pItem));
        OT_ASSERT(pItem->IsValidOpeningNumber(lOpeningNum));  // Todo
                                                              // optimization.
                                                              // Probably can
                                                              // remove this
                                                              // check.

        return pItem;
    }

    return nullptr;
}

// OTCron IS responsible for cleaning up theMarket, and takes ownership.
// So make SURE it is allocated on the HEAP before you pass it in here, and
// also make sure to delete it again if this call fails!
bool OTCron::AddMarket(
    std::shared_ptr<OTMarket> theMarket,
    bool bSaveMarketFile)
{
    OT_ASSERT(nullptr != GetServerNym());

    theMarket->SetCronPointer(*this);  // This way every Market has a pointer to
                                       // Cron.

    auto MARKET_ID = Identifier::Factory(*theMarket);
    auto str_MARKET_ID = String::Factory(MARKET_ID);
    std::string std_MARKET_ID = str_MARKET_ID->Get();

    // See if there's something else already there with the same market ID.
    auto it = m_mapMarkets.find(std_MARKET_ID);

    // If it's not already on the list, then add it...
    if (it == m_mapMarkets.end()) {
        // If I've been instructed to save the market, and Cron did NOT
        // successfully save the market
        //  (to its own file), then return false.  This will happen if
        // filesystem problems.
        if (bSaveMarketFile && !theMarket->SaveMarket()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error saving market file while adding new Market to Cron: ")(
                std_MARKET_ID)(".")
                .Flush();
            return false;
        }

        m_mapMarkets[std_MARKET_ID] = theMarket;

        bool bSuccess = true;

        // When Cron serializes, it stores a list of all its markets in the main
        // cron file.
        // The actual markets themselves serialize separately to the market
        // folder.
        //
        if (bSaveMarketFile)  // This executes only the first time that a market
                              // is added to Cron.
        // (versus when it's just being reloaded from file and added back to
        // the internal list.)
        {
            // Since we added a market to the Cron, we SAVE it.
            bSuccess = SaveCron();  // If we're loading from file, and
                                    // bSaveMarketFile is false, I don't want to
                                    // save here. that's why it's in this block.

            if (bSuccess)
                LogDebug(OT_METHOD)(__FUNCTION__)(
                    ": New Market has been added to Cron.")
                    .Flush();
            else
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Error saving while adding new Market to Cron.")
                    .Flush();
        }

        return bSuccess;
    }
    // Otherwise, if it was already there, log an error.
    else {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Attempt to add Market that was already there: ")(std_MARKET_ID)(
            ".")
            .Flush();
    }

    return false;
}

// Create it if it's not there.
std::shared_ptr<OTMarket> OTCron::GetOrCreateMarket(
    const Identifier& INSTRUMENT_DEFINITION_ID,
    const Identifier& CURRENCY_ID,
    const std::int64_t& lScale)
{
    auto pMarket{api_.Factory().Market(
        GetNotaryID(), INSTRUMENT_DEFINITION_ID, CURRENCY_ID, lScale)};

    OT_ASSERT(false != bool(pMarket));

    auto MARKET_ID = Identifier::Factory(*pMarket);

    auto pExistingMarket = GetMarket(MARKET_ID);

    // If it was already there, there's no need to create it.
    if (false != bool(pExistingMarket)) { return pExistingMarket; }

    // If we got this far, it means the Market does NOT already exist in this
    // Cron.
    // So let's add it...
    std::shared_ptr<OTMarket> market{pMarket.release()};
    bool bAdded = AddMarket(market, true);  // bool bSaveMarketFile=true,
                                            // since it was created new.

    if (bAdded) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            "New market created and added to Cron.")
            .Flush();
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error trying to add new market to Cron.")
            .Flush();
    }

    return market;
}

// Look up a transaction by transaction number and see if it is in the ledger.
// If it is, return a pointer to it, otherwise return nullptr.
std::shared_ptr<OTMarket> OTCron::GetMarket(const Identifier& MARKET_ID)
{
    auto str_MARKET_ID = String::Factory(MARKET_ID);
    std::string std_MARKET_ID = str_MARKET_ID->Get();

    // See if there's something there with that transaction number.
    auto it = m_mapMarkets.find(std_MARKET_ID);

    if (it == m_mapMarkets.end()) {
        // nothing found.
        return nullptr;
    }
    // Found it!
    else {
        auto pMarket = it->second;

        OT_ASSERT(false != bool(pMarket));

        const auto LOOP_MARKET_ID = Identifier::Factory(*pMarket);
        const auto str_LOOP_MARKET_ID = String::Factory(LOOP_MARKET_ID);

        if (MARKET_ID == LOOP_MARKET_ID)
            return pMarket;
        else
            LogOutput(OT_METHOD)(__FUNCTION__)(": Expected Market with ID: ")(
                std_MARKET_ID)(" but found ")(str_LOOP_MARKET_ID)(".")
                .Flush();
    }

    return nullptr;
}

void OTCron::InitCron() { m_strContractType = String::Factory("CRON"); }

void OTCron::Release() { Contract::Release(); }

OTCron::~OTCron() { m_pServerNym = nullptr; }
}  // namespace opentxs
