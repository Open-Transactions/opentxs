// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/otx/common/cron/OTCron.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <cstring>
#include <memory>
#include <utility>

#include "internal/core/Armored.hpp"
#include "internal/core/String.hpp"
#include "internal/otx/common/Contract.hpp"
#include "internal/otx/common/StringXML.hpp"
#include "internal/otx/common/XML.hpp"
#include "internal/otx/common/cron/OTCronItem.hpp"
#include "internal/otx/common/trade/OTMarket.hpp"
#include "internal/otx/common/util/Common.hpp"
#include "internal/otx/common/util/Tag.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Paths.internal.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/Session.internal.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Factory.internal.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/UnitDefinition.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "opentxs/util/Writer.hpp"
#include "otx/common/OTStorage.hpp"

namespace opentxs
{
// NOTE: these are only code defaults -- the values are actually loaded from
// ~/.ot/server.cfg.

// The number of transaction numbers Cron will grab for itself, when it gets
// low, before each round.
std::int32_t OTCron::_trans_refill_amount = 500;

// The number of milliseconds (ideally) between each "Cron Process" event.
std::chrono::milliseconds OTCron::_cron_ms_between_process{10000};

// The maximum number of cron items any given Nym can have active at the same
// time.
std::int32_t OTCron::_cron_max_items_per_nym{10};

Time OTCron::last_executed_{};

OTCron::OTCron(const api::Session& server)
    : Contract(server)
    , markets_()
    , cron_items_()
    , cron_items_multi_()
    , notary_id_()
    , list_transaction_numbers_()
    , is_activated_(false)
    , server_nym_(nullptr)  // just here for convenience, not responsible to
                            // cleanup this pointer.
{
    InitCron();
    LogDebug()()("Finished calling InitCron 0.").Flush();
}

// Make sure Server Nym is set on this cron object before loading or saving,
// since it's
// used for signing and verifying..
auto OTCron::LoadCron() -> bool
{
    const char* szFoldername = api_.Internal().Paths().Cron();
    const char* szFilename = "OT-CRON.crn";  // todo stop hardcoding filenames.

    assert_false(nullptr == GetServerNym());

    bool bSuccess = LoadContract(szFoldername, szFilename);

    if (bSuccess) { bSuccess = VerifySignature(*(GetServerNym())); }

    return bSuccess;
}

auto OTCron::SaveCron() -> bool
{
    const char* szFoldername = api_.Internal().Paths().Cron();
    const char* szFilename = "OT-CRON.crn";  // todo stop hardcoding filenames.
    auto reason = api_.Factory().PasswordPrompt(__func__);

    assert_false(nullptr == GetServerNym());

    ReleaseSignatures();

    // Sign it, save it internally to string, and then save that out to the
    // file.
    if (!SignContract(*server_nym_, reason) || !SaveContract() ||
        !SaveContract(szFoldername, szFilename)) {
        LogError()()("Error saving main Cronfile: ")(szFoldername)('/')(
            szFilename)(".")
            .Flush();
        return false;
    } else {
        return true;
    }
}

// Loops through ALL markets, and calls pMarket->GetNym_OfferList(NYM_ID,
// *pOfferList) for each.
// Returns a list of all the offers that a specific Nym has on all the markets.
//
auto OTCron::GetNym_OfferList(
    Armored& ascOutput,
    const identifier::Nym& NYM_ID,
    std::int32_t& nOfferCount) -> bool
{
    nOfferCount = 0;  // Outputs the number of offers on this nym.

    std::unique_ptr<OTDB::OfferListNym> pOfferList(
        dynamic_cast<OTDB::OfferListNym*>(
            OTDB::CreateObject(OTDB::STORED_OBJ_OFFER_LIST_NYM)));

    for (auto& it : markets_) {
        auto pMarket = it.second;
        assert_true(false != bool(pMarket));

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
        } else {  // Success!
            nOfferCount += nNymOfferCount;
        }
    }

    // Now pack the list into strOutput...
    if (nOfferCount == 0) {
        return true;  // Success, but 0 offers being returned. (List is empty.)

    } else if (nOfferCount > 0) {
        OTDB::Storage* pStorage = OTDB::GetDefaultStorage();
        assert_false(nullptr == pStorage);

        OTDB::OTPacker* pPacker =
            pStorage->GetPacker();  // No need to check for failure, since this
                                    // already ASSERTS. No need to cleanup
                                    // either.

        std::unique_ptr<OTDB::PackedBuffer> pBuffer(
            pPacker->Pack(*pOfferList));  // Now we PACK our nym's offer list.

        if (nullptr == pBuffer) {
            LogError()()(
                "Failed packing pOfferList in OTCron::GetNym_OfferList.")
                .Flush();
            return false;
        }

        // Now we need to translate pBuffer into strOutput.

        const std::uint8_t* pUint = pBuffer->GetData();
        const std::size_t theSize = pBuffer->GetSize();

        if ((nullptr != pUint) || (theSize < 2)) {
            auto theData = ByteArray{pUint, theSize};

            // This function will base64 ENCODE theData,
            // and then Set() that as the string contents.
            ascOutput.SetData(theData);

            return true;
        } else {
            LogError()()(
                "Null returned, or bad size, while getting buffer data in "
                "OTCron::GetNym_OfferList.")
                .Flush();
        }
    } else {
        LogError()()("Error: Less-than-zero nOfferCount in "
                     "OTCron::GetNym_OfferList: ")(nOfferCount)(".")
            .Flush();
    }

    return false;
}

auto OTCron::GetMarketList(Armored& ascOutput, std::int32_t& nMarketCount)
    -> bool
{
    nMarketCount = 0;  // This parameter is set to zero here, and incremented in
                       // the loop below.

    std::unique_ptr<OTDB::MarketList> pMarketList(
        dynamic_cast<OTDB::MarketList*>(
            OTDB::CreateObject(OTDB::STORED_OBJ_MARKET_LIST)));

    for (auto& it : markets_) {
        auto pMarket = it.second;
        assert_true(false != bool(pMarket));

        std::unique_ptr<OTDB::MarketData> pMarketData(
            dynamic_cast<OTDB::MarketData*>(
                OTDB::CreateObject(OTDB::STORED_OBJ_MARKET_DATA)));

        const auto MARKET_ID = api_.Factory().Internal().Identifier(*pMarket);
        const auto str_MARKET_ID = String::Factory(MARKET_ID, api_.Crypto());
        const auto str_NotaryID =
            String::Factory(pMarket->GetNotaryID(), api_.Crypto());
        const auto str_INSTRUMENT_DEFINITION_ID = String::Factory(
            pMarket->GetInstrumentDefinitionID(), api_.Crypto());
        const auto str_CURRENCY_ID =
            String::Factory(pMarket->GetCurrencyID(), api_.Crypto());

        pMarketData->notary_id_ = str_NotaryID->Get();
        pMarketData->market_id_ = str_MARKET_ID->Get();
        pMarketData->instrument_definition_id_ =
            str_INSTRUMENT_DEFINITION_ID->Get();
        pMarketData->currency_type_id_ = str_CURRENCY_ID->Get();
        // --------------------------------------------
        const Amount& lScale = pMarket->GetScale();
        pMarketData->scale_ = [&] {
            auto buf = UnallocatedCString{};
            lScale.Serialize(writer(buf));
            return buf;
        }();

        const Amount theCurrentBid = pMarket->GetHighestBidPrice();
        const Amount theCurrentAsk = pMarket->GetLowestAskPrice();

        pMarketData->current_bid_ = [&] {
            auto buf = UnallocatedCString{};
            theCurrentBid.Serialize(writer(buf));
            return buf;
        }();
        pMarketData->current_ask_ = [&] {
            auto buf = UnallocatedCString{};
            theCurrentAsk.Serialize(writer(buf));
            return buf;
        }();

        const Amount& lLastSalePrice = pMarket->GetLastSalePrice();
        const Amount& lTotalAvailableAssets =
            pMarket->GetTotalAvailableAssets();

        pMarketData->total_assets_ = [&] {
            auto buf = UnallocatedCString{};
            lTotalAvailableAssets.Serialize(writer(buf));
            return buf;
        }();
        pMarketData->last_sale_price_ = [&] {
            auto buf = UnallocatedCString{};
            lLastSalePrice.Serialize(writer(buf));
            return buf;
        }();

        pMarketData->last_sale_date_ = pMarket->GetLastSaleDate();

        const mapOfOffers::size_type theBidCount = pMarket->GetBidCount();
        const mapOfOffers::size_type theAskCount = pMarket->GetAskCount();

        pMarketData->number_bids_ = std::to_string(theBidCount);
        pMarketData->number_asks_ = std::to_string(theAskCount);

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
        assert_false(nullptr == pStorage);

        OTDB::OTPacker* pPacker =
            pStorage->GetPacker();  // No need to check for failure, since this
                                    // already ASSERTS. No need to cleanup
                                    // either.

        std::unique_ptr<OTDB::PackedBuffer> pBuffer(
            pPacker->Pack(*pMarketList));  // Now we PACK our market list.

        if (nullptr == pBuffer) {
            LogError()()("Failed packing pMarketList in OTCron::GetMarketList.")
                .Flush();
            return false;
        }

        // --------------------------------------------------------

        // Now we need to translate pBuffer into strOutput.

        const std::uint8_t* pUint = pBuffer->GetData();
        const std::size_t theSize = pBuffer->GetSize();

        if ((theSize > 0) && (nullptr != pUint)) {
            auto theData = ByteArray{pUint, theSize};

            // This function will base64 ENCODE theData,
            // and then Set() that as the string contents.
            ascOutput.SetData(theData);
            //            bool bSuccessSetData = false;
            //            bSuccessSetData = ascOutput.SetData(theData);

            return true;
        } else {
            LogError()()("0 size, or null return value, "
                         "while getting raw data from packed buffer.")
                .Flush();
        }
    } else {
        LogError()()("nMarketCount is less than zero: ")(nMarketCount)(".")
            .Flush();
    }

    return false;
}

auto OTCron::GetTransactionCount() const -> std::int32_t
{
    if (list_transaction_numbers_.empty()) { return 0; }

    return static_cast<std::int32_t>(list_transaction_numbers_.size());
}

void OTCron::AddTransactionNumber(const std::int64_t& lTransactionNum)
{
    list_transaction_numbers_.push_back(lTransactionNum);
}

// Once this starts returning 0, OTCron can no longer process trades and
// payment plans until the server object replenishes this list.
auto OTCron::GetNextTransactionNumber() -> std::int64_t
{
    if (list_transaction_numbers_.empty()) { return 0; }

    const std::int64_t lTransactionNum = list_transaction_numbers_.front();

    list_transaction_numbers_.pop_front();

    return lTransactionNum;
}

// return -1 if error, 0 if nothing, and 1 if the node was processed.
auto OTCron::ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t
{
    assert_false(nullptr == GetServerNym());

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
        version_ = String::Factory(xml->getAttributeValue("version"));

        const auto strNotaryID =
            String::Factory(xml->getAttributeValue("notaryID"));

        notary_id_ = api_.Factory().NotaryIDFromBase58(strNotaryID->Bytes());

        LogConsole()()("Loading OTCron for NotaryID: ")(strNotaryID.get())(".")
            .Flush();

        nReturnVal = 1;
    } else if (!strcmp("transactionNum", xml->getNodeName())) {
        const std::int64_t lTransactionNum =
            String::StringToLong(xml->getAttributeValue("value"));

        LogDetail()()("Transaction Number ")(
            lTransactionNum)(" available for Cron.")
            .Flush();

        AddTransactionNumber(lTransactionNum);  // This doesn't save to disk.
        // Make sure to save Cron when it
        // changes.

        nReturnVal = 1;
    } else if (!strcmp("cronItem", xml->getNodeName())) {
        const auto str_date_added =
            String::Factory(xml->getAttributeValue("dateAdded"));
        const auto tDateAdded =
            (!str_date_added->Exists() ? Time{}
                                       : parseTimestamp(str_date_added->Get()));

        auto strData = String::Factory();

        if (!LoadEncodedTextField(api_.Crypto(), xml, strData) ||
            !strData->Exists()) {
            LogError()()(
                "Error in OTCron::ProcessXMLNode: cronItem field without "
                "value.")
                .Flush();
            return (-1);  // error condition
        } else {
            auto pItem{api_.Factory().Internal().Session().CronItem(strData)};

            if (false == bool(pItem)) {
                LogError()()(
                    "Unable to create cron item from data in cron file.")
                    .Flush();
                return (-1);
            }

            // Why not do this here (when loading from storage), as well as when
            // first adding the item to cron,
            // and thus save myself the trouble of verifying the signature EVERY
            // ITERATION of ProcessCron().
            //
            const std::shared_ptr<OTCronItem> item{pItem.release()};
            if (!item->VerifySignature(*server_nym_)) {
                LogError()()("ERROR SECURITY: Server "
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
                LogVerbose()()(
                    "Successfully loaded cron item and added to list. ")
                    .Flush();
            } else {
                LogError()()("Though loaded / verified "
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

        const auto INSTRUMENT_DEFINITION_ID = api_.Factory().UnitIDFromBase58(
                       strInstrumentDefinitionID->Bytes()),
                   CURRENCY_ID =
                       api_.Factory().UnitIDFromBase58(strCurrencyID->Bytes());

        LogDetail()()("Loaded cron entry for Market: ")(strMarketID.get())(".")
            .Flush();

        // LoadMarket() needs this info to do its thing.
        auto pMarket{api_.Factory().Internal().Session().Market(
            notary_id_, INSTRUMENT_DEFINITION_ID, CURRENCY_ID, lScale)};

        assert_true(false != bool(pMarket));

        pMarket->SetCronPointer(*this);  // This way every Market has a pointer
                                         // to Cron.

        // AddMarket normally saves to file, but we don't want that when
        // we're LOADING from file, now do we?
        const std::shared_ptr<OTMarket> market{pMarket.release()};
        if (!market->LoadMarket() ||
            !market->VerifySignature(*GetServerNym()) ||
            !AddMarket(market, false))  // bSaveFile=false: don't save this
                                        // file WHILE loading it!!!
        {
            LogError()()(
                "Somehow error while loading, verifying, or adding market "
                "while loading Cron file.")
                .Flush();
            return (-1);
        } else {
            LogDetail()()(
                "Loaded market entry from cronfile, and also loaded the "
                "market file itself.")
                .Flush();
        }
        nReturnVal = 1;
    }

    return nReturnVal;
}

void OTCron::UpdateContents(const PasswordPrompt& reason)
{
    // I release this because I'm about to repopulate it.
    xml_unsigned_->Release();

    const auto NOTARY_ID = String::Factory(notary_id_, api_.Crypto());

    Tag tag("cron");

    tag.add_attribute("version", version_->Get());
    tag.add_attribute("notaryID", NOTARY_ID->Get());

    // Save the Market entries (the markets themselves are saved in a markets
    // folder.)
    for (auto& it : markets_) {
        auto pMarket = it.second;
        assert_true(false != bool(pMarket));

        auto MARKET_ID = api_.Factory().Internal().Identifier(*pMarket);
        auto str_MARKET_ID = String::Factory(MARKET_ID, api_.Crypto());

        auto str_INSTRUMENT_DEFINITION_ID = String::Factory(
            pMarket->GetInstrumentDefinitionID(), api_.Crypto());
        auto str_CURRENCY_ID =
            String::Factory(pMarket->GetCurrencyID(), api_.Crypto());

        TagPtr tagMarket(new Tag("market"));
        tagMarket->add_attribute("marketID", str_MARKET_ID->Get());
        tagMarket->add_attribute(
            "instrumentDefinitionID", str_INSTRUMENT_DEFINITION_ID->Get());
        tagMarket->add_attribute("currencyID", str_CURRENCY_ID->Get());
        tagMarket->add_attribute("marketScale", [&] {
            auto buf = UnallocatedCString{};
            pMarket->GetScale().Serialize(writer(buf));
            return buf;
        }());
        tag.add_tag(tagMarket);
    }

    // Save the Cron Items
    for (auto& it : cron_items_multi_) {
        auto pItem = it.second;
        assert_true(false != bool(pItem));

        const auto tDateAdded{it.first};
        auto strItem = String::Factory(*pItem);  // Extract the cron item
                                                 // contract into string form.
        auto ascItem = Armored::Factory(
            api_.Crypto(), strItem);  // Base64-encode that for storage.

        TagPtr tagCronItem(new Tag("cronItem", ascItem->Get()));
        tagCronItem->add_attribute("dateAdded", formatTimestamp(tDateAdded));
        tag.add_tag(tagCronItem);
    }

    // Save the transaction numbers.
    //
    for (auto& lTransactionNumber : list_transaction_numbers_) {
        TagPtr tagNumber(new Tag("transactionNum"));
        tagNumber->add_attribute("value", std::to_string(lTransactionNumber));
        tag.add_tag(tagNumber);
    }  // for

    UnallocatedCString str_result;
    tag.output(str_result);

    xml_unsigned_->Concatenate(String::Factory(str_result));
}

auto OTCron::computeTimeout() -> std::chrono::milliseconds
{
    return GetCronMsBetweenProcess() -
           std::chrono::duration_cast<std::chrono::milliseconds>(
               Clock::now() - last_executed_);
}

// Make sure to call this regularly so the CronItems get a chance to process and
// expire.
void OTCron::ProcessCronItems()
{
    auto reason = api_.Factory().PasswordPrompt(__func__);
    if (!is_activated_) {
        LogError()()("Not activated yet. (Skipping).").Flush();
        return;
    }

    // check elapsed time since last items processing
    if (computeTimeout().count() > 0) { return; }

    last_executed_ = Clock::now();

    const std::int32_t nTwentyPercent = OTCron::GetCronRefillAmount() / 5;
    if (GetTransactionCount() <= nTwentyPercent) {
        LogError()()(
            "WARNING: Cron has fewer than 20 percent of its normal "
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
    for (auto it = cron_items_multi_.begin(); it != cron_items_multi_.end();) {
        if (GetTransactionCount() <= nTwentyPercent) {
            LogError()()(
                "WARNING: Cron has fewer than 20 percent of its normal "
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
        assert_true(false != bool(pItem));
        LogVerbose()()("Processing item number: ")(pItem->GetTransactionNum())
            .Flush();

        if (pItem->ProcessCron(reason)) {
            it++;
            continue;
        }
        pItem->HookRemovalFromCron(
            api_.Wallet(), nullptr, GetNextTransactionNumber(), reason);
        LogConsole()()("Removing cron item: ")(pItem->GetTransactionNum())(".")
            .Flush();
        it = cron_items_multi_.erase(it);
        auto it_map = FindItemOnMap(pItem->GetTransactionNum());
        assert_true(cron_items_.end() != it_map);
        cron_items_.erase(it_map);

        bNeedToSave = true;
    }
    if (bNeedToSave) { SaveCron(); }
}

// OTCron IS responsible for cleaning up theItem, and takes ownership.
// So make SURE it is allocated on the HEAP before you pass it in here, and
// also make sure to delete it again if this call fails!
auto OTCron::AddCronItem(
    std::shared_ptr<OTCronItem> theItem,
    const bool bSaveReceipt,
    const Time tDateAdded) -> bool
{
    assert_false(nullptr == GetServerNym());
    auto reason = api_.Factory().PasswordPrompt(__func__);

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
            (!theItem->SignContract(
                 *GetServerNym(),
                 reason) ||  // Notice the
                             // server adds its signature before saving the cron
                             // receipt to local storage. This way, the server
                             // can verify its own signature later, as evidence
                             // the file hasn't been tampered with. (BOTH
                             // signatures are there now--user's and server's.)
             !theItem->SaveContract() ||
             !theItem->SaveCronReceipt())) {
            LogError()()("Error saving receipt while adding new "
                         "CronItem to Cron.")
                .Flush();
            return false;
        }

        // Insert to the MAP (by Transaction Number)
        //
        cron_items_.insert(std::pair<std::int64_t, std::shared_ptr<OTCronItem>>(
            theItem->GetTransactionNum(), theItem));

        // Insert to the MULTIMAP (by Date)
        //
        cron_items_multi_.insert(
            cron_items_multi_.upper_bound(tDateAdded),
            std::pair<Time, std::shared_ptr<OTCronItem>>(tDateAdded, theItem));

        theItem->SetCronPointer(*this);
        theItem->setServerNym(server_nym_);
        theItem->setNotaryID(notary_id_);

        bool bSuccess = true;

        theItem->HookActivationOnCron(reason, bSaveReceipt);  // If merely being
                                                              // reloaded after
                                                              // server reboot,
                                                              // this is false.
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

            if (bSuccess) {
                LogConsole()()("New CronItem has been added to Cron: ")(
                    theItem->GetTransactionNum())(".")
                    .Flush();
            } else {
                LogError()()(
                    "Error saving while adding new CronItem to Cron: ")(
                    theItem->GetTransactionNum())(".")
                    .Flush();
            }
        }

        return bSuccess;
    }
    // Otherwise, if it was already there, log an error.
    else {
        LogError()()("Failed attempt to add CronItem with pre-existing "
                     "transaction number: ")(theItem->GetTransactionNum())(".")
            .Flush();
    }

    return false;
}

auto OTCron::RemoveCronItem(
    std::int64_t lTransactionNum,
    Nym_p theRemover,
    const PasswordPrompt& reason) -> bool  // if returns false, item
                                           // wasn't found.
{
    // See if there's a cron item with that transaction number.
    auto it_map = FindItemOnMap(lTransactionNum);

    // If it's not already on the list, then there's nothing to remove.
    if (cron_items_.end() == it_map) {
        LogError()()("Attempt to remove non-existent CronItem from OTCron. "
                     "Transaction #: ")(lTransactionNum)(".")
            .Flush();
    }

    // Otherwise, if it WAS already there, remove it properly.
    else {
        auto pItem = it_map->second;
        //      assert_false(nullptr == pItem); // Already done in
        //      FindItemOnMap.

        // We have to remove it from the multimap as well.
        auto it_multimap = FindItemOnMultimap(lTransactionNum);
        assert_true(cron_items_multi_.end() != it_multimap);  // If found on
                                                              // map, MUST be on
                                                              // multimap also.

        pItem->HookRemovalFromCron(
            api_.Wallet(), theRemover, GetNextTransactionNumber(), reason);

        cron_items_.erase(it_map);             // Remove from MAP.
        cron_items_multi_.erase(it_multimap);  // Remove from MULTIMAP.

        // An item has been removed from Cron. SAVE.
        return SaveCron();
    }

    return false;
}

// Look up a transaction by transaction number and see if it is in the map.
// If it is, return an iterator to it, otherwise return cron_items_.end()
//
// Note: only the "official" transaction number will work here.
// If the cron item contains multiple opening numbers, the only one
// that will work in this function is the "official" one, the one that
// belongs to the Nym who actually activated this Cron Item.
//
auto OTCron::FindItemOnMap(std::int64_t lTransactionNum)
    -> mapOfCronItems::iterator
{
    // See if there's something there with lTransactionNum
    // as its "official" number.
    //
    auto itt = cron_items_.find(lTransactionNum);

    if (itt != cron_items_.end())  // Found it!
    {
        auto pItem = itt->second;
        assert_true(false != bool(pItem));
        assert_true(pItem->GetTransactionNum() == lTransactionNum);

        return itt;
    }

    return itt;
}

// Look up a transaction by transaction number and see if it is in the multimap.
// If it is, return an iterator to it, otherwise return
// cron_items_multi_.end()
//
// Note: only the "official" transaction number will work here.
// If the cron item contains multiple opening numbers, the only one
// that will work in this function is the "official" one, the one that
// belongs to the Nym who actually activated this Cron Item.
//
auto OTCron::FindItemOnMultimap(std::int64_t lTransactionNum)
    -> multimapOfCronItems::iterator
{
    auto itt = cron_items_multi_.begin();

    while (cron_items_multi_.end() != itt) {
        auto pItem = itt->second;
        assert_true(false != bool(pItem));

        if (pItem->GetTransactionNum() == lTransactionNum) { break; }

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
auto OTCron::GetItemByOfficialNum(std::int64_t lTransactionNum)
    -> std::shared_ptr<OTCronItem>
{
    // See if there's something there with lTransactionNum
    // as its "official" number.
    //
    auto itt = cron_items_.find(lTransactionNum);

    if (itt != cron_items_.end())  // Found it!
    {
        auto pItem = itt->second;
        assert_true(false != bool(pItem));
        assert_true(pItem->GetTransactionNum() == lTransactionNum);

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
auto OTCron::GetItemByValidOpeningNum(std::int64_t lOpeningNum)
    -> std::shared_ptr<OTCronItem>
{
    // See if there's something there with that transaction number.
    auto itt = cron_items_.find(lOpeningNum);

    if (itt == cron_items_.end()) {
        // We didn't find it as the "official" number, so let's loop
        // through the cron items one-by-one and see if it's a valid
        // opening number. (We searched for the "official" number first,
        // since it will often be the right one, and avoids doing this
        // longer search. Basically for optimization purposes.)
        //
        for (auto& it : cron_items_) {
            auto pItem = it.second;
            assert_true(false != bool(pItem));

            if (pItem->IsValidOpeningNumber(lOpeningNum)) {  // Todo
                                                             // optimization.
                // Probably can remove
                // this check.
                return pItem;
            }
        }
    }
    // Found it!
    else {
        auto pItem = itt->second;
        assert_true(false != bool(pItem));
        assert_true(pItem->IsValidOpeningNumber(lOpeningNum));  // Todo
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
auto OTCron::AddMarket(
    std::shared_ptr<OTMarket> theMarket,
    bool bSaveMarketFile) -> bool
{
    assert_false(nullptr == GetServerNym());
    auto reason = api_.Factory().PasswordPrompt(__func__);

    theMarket->SetCronPointer(*this);  // This way every Market has a pointer to
                                       // Cron.

    auto MARKET_ID = api_.Factory().Internal().Identifier(*theMarket);
    auto str_MARKET_ID = String::Factory(MARKET_ID, api_.Crypto());
    const UnallocatedCString std_MARKET_ID = str_MARKET_ID->Get();

    // See if there's something else already there with the same market ID.
    auto it = markets_.find(std_MARKET_ID);

    // If it's not already on the list, then add it...
    if (it == markets_.end()) {
        // If I've been instructed to save the market, and Cron did NOT
        // successfully save the market
        //  (to its own file), then return false.  This will happen if
        // filesystem problems.
        if (bSaveMarketFile && !theMarket->SaveMarket(reason)) {
            LogError()()(
                "Error saving market file while adding new Market to Cron: ")(
                std_MARKET_ID)(".")
                .Flush();
            return false;
        }

        markets_[std_MARKET_ID] = theMarket;

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

            if (bSuccess) {
                LogDebug()()("New Market has been added to Cron.").Flush();
            } else {
                LogError()()("Error saving while adding new Market to Cron.")
                    .Flush();
            }
        }

        return bSuccess;
    }
    // Otherwise, if it was already there, log an error.
    else {
        LogError()()("Attempt to add Market that was already there: ")(
            std_MARKET_ID)(".")
            .Flush();
    }

    return false;
}

// Create it if it's not there.
auto OTCron::GetOrCreateMarket(
    const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
    const identifier::UnitDefinition& CURRENCY_ID,
    const Amount& lScale) -> std::shared_ptr<OTMarket>
{
    auto pMarket{api_.Factory().Internal().Session().Market(
        GetNotaryID(), INSTRUMENT_DEFINITION_ID, CURRENCY_ID, lScale)};

    assert_true(false != bool(pMarket));

    auto MARKET_ID = api_.Factory().Internal().Identifier(*pMarket);

    auto pExistingMarket = GetMarket(MARKET_ID);

    // If it was already there, there's no need to create it.
    if (false != bool(pExistingMarket)) { return pExistingMarket; }

    // If we got this far, it means the Market does NOT already exist in this
    // Cron.
    // So let's add it...
    std::shared_ptr<OTMarket> market{pMarket.release()};
    const bool bAdded = AddMarket(market, true);  // bool bSaveMarketFile=true,
                                                  // since it was created new.

    if (bAdded) {
        LogConsole()()("New market created and added to Cron.").Flush();
    } else {
        LogError()()("Error trying to add new market to Cron.").Flush();
    }

    return market;
}

// Look up a transaction by transaction number and see if it is in the ledger.
// If it is, return a pointer to it, otherwise return nullptr.
auto OTCron::GetMarket(const identifier::Generic& MARKET_ID)
    -> std::shared_ptr<OTMarket>
{
    auto str_MARKET_ID = String::Factory(MARKET_ID, api_.Crypto());
    const UnallocatedCString std_MARKET_ID = str_MARKET_ID->Get();

    // See if there's something there with that transaction number.
    auto it = markets_.find(std_MARKET_ID);

    if (it == markets_.end()) {
        // nothing found.
        return nullptr;
    }
    // Found it!
    else {
        auto pMarket = it->second;

        assert_true(false != bool(pMarket));

        const auto LOOP_MARKET_ID =
            api_.Factory().Internal().Identifier(*pMarket);
        const auto str_LOOP_MARKET_ID =
            String::Factory(LOOP_MARKET_ID, api_.Crypto());

        if (MARKET_ID == LOOP_MARKET_ID) {
            return pMarket;
        } else {
            LogError()()("Expected Market with ID: ")(
                std_MARKET_ID)(" but found ")(str_LOOP_MARKET_ID.get())(".")
                .Flush();
        }
    }

    return nullptr;
}

void OTCron::SetServerNym(Nym_p pServerNym)
{
    assert_false(nullptr == pServerNym);
    server_nym_ = pServerNym;
}

void OTCron::InitCron() { contract_type_ = String::Factory("CRON"); }

void OTCron::Release() { Contract::Release(); }

OTCron::~OTCron() { server_nym_ = nullptr; }
}  // namespace opentxs
