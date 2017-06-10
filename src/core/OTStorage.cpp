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

#include "opentxs/core/OTStorage.hpp"

#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/util/OTDataFolder.hpp"
#include "opentxs/core/util/OTPaths.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/OTStoragePB.hpp"

#include <fstream>
#include <sstream>
#include <typeinfo>

/*
 // We want to store EXISTING OT OBJECTS (Usually signed contracts)
 // These have an EXISTING OT path, such as "inbox/acct_id".
 // These files are always in the form of a STRING.
 // The easiest way for me to store/retrieve those strings is:


 using namespace OTDB;

 bool bSuccessStore = StoreString(strContents, strFolder, strFilename);
 bool bSuccessQuery = QueryString(strRetrieved, strFolder, strFilename);


 // Internal to the above functions, the default Packing/Buffer is
 // used, and the default Storage type is used. But what if I want to
 // CHOOSE the storage and packing? Perhaps the default (filesystem) is not
 // good enough for me, and I prefer a key/value DB.

 // Storage.
 // Before creating my OWN storage, let's try using the default storage object
 // itself, instead of asking the API to use it for me:

 OTDB::Storage * pStorage = OTDB::GetDefaultStorage();
 OT_ASSERT(nullptr!=pStorage);

 bool bSuccessStore = pStorage->StoreString(strContents, strFolder,
 strFilename);
 bool bSuccessQuery = pStorage->QueryString(strRetrieved, strFolder,
 strFilename);


 // So if I wanted to create my OWN instance of storage, instead of using the
 // default one, it would be similar:

 OTDB::Storage * pStorage = OTDB::CreateStorageContext(STORE_FILESYSTEM,
 PACK_MESSAGE_PACK);
 OT_ASSERT(nullptr!=pStorage);

 bool bSuccessInit  = pStorage->Init("/path/to/data_folder", "wallet.xml");

 if (bSuccessInit)
 {
    bool bSuccessStore = pStorage->StoreString(strContents, strFolder,
 strFilename);
    bool bSuccessQuery = pStorage->QueryString(strRetrieved, strFolder,
 strFilename);
 }


 // Creating like above is also how the default storage context gets
 instantiated
 // (internally) when you first start storing and querying.

 // But Storage needs to be SET UP -- whether a database connection initiated,
 // or files loaded, or sub-directories created, or a Tor connection or
 whatever.
 // Therefore, there is an Init() call, which may have different parameters for
 // each storage type. That way, all subclasses might use it differently, and
 // the parameters are easily thrown into a config file later.


 // What if it was a CouchDB database, instead of the filesystem?
 // And using Google's Protocol Buffers for packing, isntead of MsgPack?
 // (Note: OT doesn't actually support CouchDB yet.) But it would look like:

 Storage * pStorage =
 CreateStorageContext(STORE_COUCHDB, PACK_PROTOCOL_BUFFERS);
 OT_ASSERT(nullptr!=pStorage);

 // This time, Init receives database connect info instead of filesystem info...
 bool bSuccessInit  = pStorage->Init("IP ADDRESS", "PORT", "USERNAME",
 "PASSWORD", "DATABASE NAME");

 etc.


 // So what if I want to use the default, but I want that DEFAULT to be CouchDB
 and Google?
 // Just do this (near the beginning of the execution of the application):

 bool bInit = InitDefaultStorage(STORE_COUCHDB, PACK_PROTOCOL_BUFFERS,
                    "IP ADDRESS", "PORT", "USERNAME", "PASSWORD", "DB NAME");

 if (true == bInit)
 {
    // Then do this as normal:

    Storage * pStorage = GetDefaultStorage();
    OT_ASSERT(nullptr!=pStorage);

    bool bSuccessStore = pStorage->StoreString(strContents, strFolder,
 strFilename);
    bool bSuccessQuery = pStorage->QueryString(strRetrieved, strFolder,
 strFilename);
 }


 // What if you want to store an OBJECT in that location instead of a string?
 // The object must be instantiated by the Storage Context...

 BitcoinAcct * pAcct = pStorage->CreateObject(STORED_OBJ_BITCOIN_ACCT);
 OT_ASSERT(nullptr != pAcct);

 pAcct->acct_id                = "jkhsdf987345kjhf8lkjhwef987345";
 pAcct->bitcoin_acct_name    = "Read-Only Label (Bitcoin Internal acct)";
 pAcct->gui_label            = "Editable Label (Moneychanger)";


 // Perhaps you want to load up a Wallet and add this BitcoinAcct to it...

 WalletData * pWalletData =
 pStorage->QueryObject(STORED_OBJ_WALLET_DATA, "moneychanger", "wallet.pak");

 if (nullptr != pWalletData) // It loaded.
 {
    if (pWalletData->AddBitcoinAcct(*pAcct))
        bool bSuccessStore = pStorage->StoreObject(*pWalletData, "moneychanger",
 strFilename);
    else
        delete pAcct;

    delete pWalletData;
 }

 // Voila! The above code creates a BitcoinAcct (data object, not the real
 thing)
 // and then loads up the Moneychanger WalletData object, adds the BitcoinAcct
 to
 // it, and then stores it again.

 // SIMPLE, RIGHT?

 // Through this mechanism:
 //
 // 1) You can store your objects using the same storage context as the rest of
 OT.
 // 2) You can dictate a different storage context, just for yourself, or for
 the entire OT library as well.
 // 3) You can subclass OTDB::Storage and thus invent new storage methods.
 // 4) You can easily store and load objects and strings.
 // 5) You can swap out the packing code (msgpack, protobuf, json, etc) with no
 change to any other code.
 // 6) It's consistent and easy-to-use for all object types.
 // 7) For generic objects, there is a Blob type, a String type, and a StringMap
 type.
 */

opentxs::OTDB::Storage* opentxs::OTDB::details::s_pStorage = nullptr;

opentxs::OTDB::mapOfFunctions* opentxs::OTDB::details::pFunctionMap =
    nullptr;  // This is a pointer so I can control what order it is created in,
              // on
              // startup.

const char* opentxs::OTDB::StoredObjectTypeStrings[] = {
    "OTDBString",     // Just a string.
    "Blob",           // Binary data of arbitrary size.
    "StringMap",      // A StringMap is a list of Key/Value pairs, useful for
                      // storing
                      // nearly anything.
    "WalletData",     // The GUI wallet's stored data
    "BitcoinAcct",    // The GUI wallet's stored data about a Bitcoin acct
    "BitcoinServer",  // The GUI wallet's stored data about a Bitcoin RPC port.
    "RippleServer",   // The GUI wallet's stored data about a Ripple server.
    "LoomServer",     // The GUI wallet's stored data about a Loom server.
    "ServerInfo",     // A Nym has a list of these.
    "ContactNym",     // This is a Nym record inside a contact of your address
                      // book.
    "ContactAcct",    // This is an account record inside a contact of your
                      // address
                      // book.
    "Contact",        // Your address book has a list of these.
    "AddressBook",    // Your address book.
    "MarketData",     // The description data for any given Market ID.
    "MarketList",     // A list of MarketDatas.
    "BidData",        // Offer details (doesn't contain private details)
    "AskData",        // Offer details (doesn't contain private details)
    "OfferListMarket",  // A list of offer details, for a specific market.
    "TradeDataMarket",  // Trade details (doesn't contain private data)
    "TradeListMarket",  // A list of trade details, for a specific market.
    "OfferDataNym",     // Private offer details for a particular Nym and Offer.
    "OfferListNym",     // A list of private offer details for a particular Nym.
    "TradeDataNym",     // Private trade details for a particular Nym and Trade.
    "TradeListNym",  // A list of private trade details for a particular Nym and
                     // Offer.
    "StoredObjError"  // (Should never be.)
};

namespace opentxs
{

namespace OTDB
{
// NAMESPACE CONSTRUCTOR / DESTRUCTOR

// s_pStorage is "private" to the namespace.
// Use GetDefaultStorage() to access this variable.
// Use InitDefaultStorage() to set up this variable.
//
// Storage * ::details::s_pStorage = nullptr;
// These are actually defined in the namespace (.h file).

// mapOfFunctions * details::pFunctionMap;

InitOTDBDetails theOTDBConstructor;  // Constructor for this instance (define
                                     // all
                                     // namespace variables above this line.)

InitOTDBDetails::InitOTDBDetails()  // Constructor for namespace
{
#if defined(OTDB_MESSAGE_PACK) || defined(OTDB_PROTOCOL_BUFFERS)
    OT_ASSERT(nullptr == details::pFunctionMap);
    details::pFunctionMap = new mapOfFunctions;

    OT_ASSERT(nullptr != details::pFunctionMap);
    mapOfFunctions& theMap = *(details::pFunctionMap);
#endif

#if defined(OTDB_MESSAGE_PACK)
    theMap[std::make_pair(PACK_MESSAGE_PACK, STORED_OBJ_STRING)] =
        &StringMsgpack::Instantiate;
    theMap[std::make_pair(PACK_MESSAGE_PACK, STORED_OBJ_BLOB)] =
        &BlobMsgpack::Instantiate;
    theMap[std::make_pair(PACK_MESSAGE_PACK, STORED_OBJ_STRING_MAP)] =
        &StringMapMsgpack::Instantiate;
    theMap[std::make_pair(PACK_MESSAGE_PACK, STORED_OBJ_WALLET_DATA)] =
        &WalletDataMsgpack::Instantiate;
    theMap[std::make_pair(PACK_MESSAGE_PACK, STORED_OBJ_BITCOIN_ACCT)] =
        &BitcoinAcctMsgpack::Instantiate;
    theMap[std::make_pair(PACK_MESSAGE_PACK, STORED_OBJ_BITCOIN_SERVER)] =
        &BitcoinServerMsgpack::Instantiate;
    theMap[std::make_pair(PACK_MESSAGE_PACK, STORED_OBJ_RIPPLE_SERVER)] =
        &RippleServerMsgpack::Instantiate;
    theMap[std::make_pair(PACK_MESSAGE_PACK, STORED_OBJ_LOOM_SERVER)] =
        &LoomServerMsgpack::Instantiate;
    theMap[std::make_pair(PACK_MESSAGE_PACK, STORED_OBJ_SERVER_INFO)] =
        &ServerInfoMsgpack::Instantiate;
    theMap[std::make_pair(PACK_MESSAGE_PACK, STORED_OBJ_CONTACT_ACCT)] =
        &ContactAcctMsgpack::Instantiate;
    theMap[std::make_pair(PACK_MESSAGE_PACK, STORED_OBJ_CONTACT_NYM)] =
        &ContactNymMsgpack::Instantiate;
    theMap[std::make_pair(PACK_MESSAGE_PACK, STORED_OBJ_CONTACT)] =
        &ContactMsgpack::Instantiate;
    theMap[std::make_pair(PACK_MESSAGE_PACK, STORED_OBJ_ADDRESS_BOOK)] =
        &AddressBookMsgpack::Instantiate;
    theMap[std::make_pair(PACK_MESSAGE_PACK, STORED_OBJ_MARKET_DATA)] =
        &MarketDataMsgpack::Instantiate;
    theMap[std::make_pair(PACK_MESSAGE_PACK, STORED_OBJ_MARKET_LIST)] =
        &MarketListMsgpack::Instantiate;
    theMap[std::make_pair(PACK_MESSAGE_PACK, STORED_OBJ_BID_DATA)] =
        &BidDataMsgpack::Instantiate;
    theMap[std::make_pair(PACK_MESSAGE_PACK, STORED_OBJ_ASK_DATA)] =
        &AskDataMsgpack::Instantiate;
    theMap[std::make_pair(PACK_MESSAGE_PACK, STORED_OBJ_OFFER_LIST_MARKET)] =
        &OfferListMarketMsgpack::Instantiate;
    theMap[std::make_pair(PACK_MESSAGE_PACK, STORED_OBJ_TRADE_DATA_MARKET)] =
        &TradeDataMarketMsgpack::Instantiate;
    theMap[std::make_pair(PACK_MESSAGE_PACK, STORED_OBJ_TRADE_LIST_MARKET)] =
        &TradeListMarketMsgpack::Instantiate;
    theMap[std::make_pair(PACK_MESSAGE_PACK, STORED_OBJ_OFFER_DATA_NYM)] =
        &OfferDataNymMsgpack::Instantiate;
    theMap[std::make_pair(PACK_MESSAGE_PACK, STORED_OBJ_OFFER_LIST_NYM)] =
        &OfferListNymMsgpack::Instantiate;
    theMap[std::make_pair(PACK_MESSAGE_PACK, STORED_OBJ_TRADE_DATA_NYM)] =
        &TradeDataNymMsgpack::Instantiate;
    theMap[std::make_pair(PACK_MESSAGE_PACK, STORED_OBJ_TRADE_LIST_NYM)] =
        &TradeListNymMsgpack::Instantiate;
#endif

#if defined(OTDB_PROTOCOL_BUFFERS)
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    theMap[std::make_pair(PACK_PROTOCOL_BUFFERS, STORED_OBJ_STRING)] =
        &StringPB::Instantiate;
    theMap[std::make_pair(PACK_PROTOCOL_BUFFERS, STORED_OBJ_BLOB)] =
        &BlobPB::Instantiate;
    theMap[std::make_pair(PACK_PROTOCOL_BUFFERS, STORED_OBJ_STRING_MAP)] =
        &StringMapPB::Instantiate;
    theMap[std::make_pair(PACK_PROTOCOL_BUFFERS, STORED_OBJ_WALLET_DATA)] =
        &WalletDataPB::Instantiate;
    theMap[std::make_pair(PACK_PROTOCOL_BUFFERS, STORED_OBJ_BITCOIN_ACCT)] =
        &BitcoinAcctPB::Instantiate;
    theMap[std::make_pair(PACK_PROTOCOL_BUFFERS, STORED_OBJ_BITCOIN_SERVER)] =
        &BitcoinServerPB::Instantiate;
    theMap[std::make_pair(PACK_PROTOCOL_BUFFERS, STORED_OBJ_RIPPLE_SERVER)] =
        &RippleServerPB::Instantiate;
    theMap[std::make_pair(PACK_PROTOCOL_BUFFERS, STORED_OBJ_LOOM_SERVER)] =
        &LoomServerPB::Instantiate;
    theMap[std::make_pair(PACK_PROTOCOL_BUFFERS, STORED_OBJ_SERVER_INFO)] =
        &ServerInfoPB::Instantiate;
    theMap[std::make_pair(PACK_PROTOCOL_BUFFERS, STORED_OBJ_CONTACT_ACCT)] =
        &ContactAcctPB::Instantiate;
    theMap[std::make_pair(PACK_PROTOCOL_BUFFERS, STORED_OBJ_CONTACT_NYM)] =
        &ContactNymPB::Instantiate;
    theMap[std::make_pair(PACK_PROTOCOL_BUFFERS, STORED_OBJ_CONTACT)] =
        &ContactPB::Instantiate;
    theMap[std::make_pair(PACK_PROTOCOL_BUFFERS, STORED_OBJ_ADDRESS_BOOK)] =
        &AddressBookPB::Instantiate;
    theMap[std::make_pair(PACK_PROTOCOL_BUFFERS, STORED_OBJ_MARKET_DATA)] =
        &MarketDataPB::Instantiate;
    theMap[std::make_pair(PACK_PROTOCOL_BUFFERS, STORED_OBJ_MARKET_LIST)] =
        &MarketListPB::Instantiate;
    theMap[std::make_pair(PACK_PROTOCOL_BUFFERS, STORED_OBJ_BID_DATA)] =
        &BidDataPB::Instantiate;
    theMap[std::make_pair(PACK_PROTOCOL_BUFFERS, STORED_OBJ_ASK_DATA)] =
        &AskDataPB::Instantiate;
    theMap[std::make_pair(
        PACK_PROTOCOL_BUFFERS, STORED_OBJ_OFFER_LIST_MARKET)] =
        &OfferListMarketPB::Instantiate;
    theMap[std::make_pair(
        PACK_PROTOCOL_BUFFERS, STORED_OBJ_TRADE_DATA_MARKET)] =
        &TradeDataMarketPB::Instantiate;
    theMap[std::make_pair(
        PACK_PROTOCOL_BUFFERS, STORED_OBJ_TRADE_LIST_MARKET)] =
        &TradeListMarketPB::Instantiate;
    theMap[std::make_pair(PACK_PROTOCOL_BUFFERS, STORED_OBJ_OFFER_DATA_NYM)] =
        &OfferDataNymPB::Instantiate;
    theMap[std::make_pair(PACK_PROTOCOL_BUFFERS, STORED_OBJ_OFFER_LIST_NYM)] =
        &OfferListNymPB::Instantiate;
    theMap[std::make_pair(PACK_PROTOCOL_BUFFERS, STORED_OBJ_TRADE_DATA_NYM)] =
        &TradeDataNymPB::Instantiate;
    theMap[std::make_pair(PACK_PROTOCOL_BUFFERS, STORED_OBJ_TRADE_LIST_NYM)] =
        &TradeListNymPB::Instantiate;
#endif
}

InitOTDBDetails::~InitOTDBDetails()  // Destructor for namespace
{
    OT_ASSERT(nullptr != details::pFunctionMap);
    delete details::pFunctionMap;
    details::pFunctionMap = nullptr;

#if defined(OTDB_PROTOCOL_BUFFERS)
    google::protobuf::ShutdownProtobufLibrary();
#endif
}

// INTERFACE for the Namespace (for coders to use.)

Storage* GetDefaultStorage() { return OTDB::details::s_pStorage; }

// You might normally create your own Storage object, choosing the storage type
// and the packing type, and then call Init() on that object in order to get it
// up and running.  This function is the equivalent of doing all that, but with
// the
// DEFAULT storage object (which OT uses when none is specified.)
//
bool InitDefaultStorage(StorageType eStoreType, PackType ePackType)
{
    // This allows you to call multiple times if you want to change the default
    // storage.
    //
    //        if (nullptr != details::s_pStorage)
    //        {
    //            otErr << "OTDB::InitDefaultStorage: Existing storage context
    // already exists. (Erasing / replacing it.)\n";
    //
    //            delete details::s_pStorage;
    //            details::s_pStorage = nullptr;
    //        }

    if (nullptr == details::s_pStorage) {
        otInfo << "OTDB::InitDefaultStorage: Existing storage context doesn't "
                  "already exist. (Creating it.)\n";

        details::s_pStorage = Storage::Create(eStoreType, ePackType);
    }

    if (nullptr == details::s_pStorage) {
        otErr << "OTDB::InitDefaultStorage: Failed while calling "
                 "OTDB::Storage::Create()\n";
        return false;
    }

    return true;
}

// %newobject Factory::createObj();
Storage* CreateStorageContext(StorageType eStoreType, PackType ePackType)
{
    Storage* pStorage = Storage::Create(eStoreType, ePackType);

    return pStorage;  // caller responsible to delete
}

// %newobject Factory::createObj();
Storable* CreateObject(StoredObjectType eType)
{
    Storage* pStorage = details::s_pStorage;

    if (nullptr == pStorage) {
        return nullptr;
    }

    return pStorage->CreateObject(eType);
}

// bool bSuccess = Store(strInbox, "inbox", "lkjsdf908w345ljkvd");
// bool bSuccess = Store(strMint,  "mints", NOTARY_ID,
// INSTRUMENT_DEFINITION_ID);
// bool bSuccess = Store(strPurse, "purse", NOTARY_ID, NYM_ID,
// INSTRUMENT_DEFINITION_ID);

// BELOW FUNCTIONS use the DEFAULT Storage context.

// Check that if oneStr is "", then twoStr and threeStr are "" also... and so
// on...
bool CheckStringsExistInOrder(
    const std::string& strFolder,
    const std::string& oneStr,
    const std::string& twoStr,
    const std::string& threeStr,
    const char* szFuncName)
{
    if (nullptr == szFuncName) szFuncName = __FUNCTION__;

    String ot_strFolder(strFolder), ot_oneStr(oneStr), ot_twoStr(twoStr),
        ot_threeStr(threeStr);

    if (ot_strFolder.Exists()) {
        if (!ot_oneStr.Exists()) {
            if ((!ot_twoStr.Exists()) && (!ot_threeStr.Exists())) {
            } else {
                otErr << szFuncName << ": ot_twoStr or ot_threeStr exist, when "
                                       "ot_oneStr doesn't exist! \n";
                OT_FAIL;
            }
        } else if ((!ot_twoStr.Exists()) && (ot_threeStr.Exists())) {
            otErr << szFuncName << ": ot_twoStr or ot_threeStr exist, when "
                                   "ot_oneStr doesn't exist! \n";
            OT_FAIL;
        }
    } else {
        otErr << szFuncName << ": ot_strFolder must always exist!\n";
        OT_FAIL;
    }
    return true;
}

// See if the file is there.
bool Exists(
    std::string strFolder,
    std::string oneStr,
    std::string twoStr,
    std::string threeStr)
{
    {
        String ot_strFolder(strFolder), ot_oneStr(oneStr), ot_twoStr(twoStr),
            ot_threeStr(threeStr);
        OT_ASSERT_MSG(
            ot_strFolder.Exists(), "OTDB::Exists: strFolder is empty.");

        if (!ot_oneStr.Exists()) {
            OT_ASSERT_MSG(
                (!ot_twoStr.Exists() && !ot_threeStr.Exists()),
                "Exists: bad options");
            oneStr = strFolder;
            strFolder = ".";
        }
    }

    Storage* pStorage = details::s_pStorage;

    if (nullptr == pStorage) {
        otOut << "OTDB::" << __FUNCTION__
              << ": details::s_pStorage is null. (Returning false.)\n";
        return false;
    }

    return pStorage->Exists(strFolder, oneStr, twoStr, threeStr);
}

int64_t FormPathString(
    std::string& strOutput,
    const std::string& strFolder,
    const std::string& oneStr,
    const std::string& twoStr,
    const std::string& threeStr)
{
    String ot_strFolder(strFolder), ot_oneStr(oneStr), ot_twoStr(twoStr),
        ot_threeStr(threeStr);
    OT_ASSERT_MSG(
        ot_strFolder.Exists(), "OTDB::FormPathString: strFolder is empty.");

    if (!ot_oneStr.Exists()) {
        OT_ASSERT_MSG(
            (!ot_twoStr.Exists() && !ot_threeStr.Exists()),
            "FormPathString: bad options");
        ot_oneStr = strFolder.c_str();
        ot_strFolder = ".";
    }

    Storage* pStorage = details::s_pStorage;

    if (nullptr == pStorage) {
        otOut << "OTDB::" << __FUNCTION__
              << ": details::s_pStorage is null. (Returning -1.)\n";
        return -1;
    }

    return pStorage->FormPathString(
        strOutput, ot_strFolder.Get(), ot_oneStr.Get(), twoStr, threeStr);
}

// Store/Retrieve a string.

bool StoreString(
    const std::string& strContents,
    const std::string& strFolder,
    const std::string& oneStr,
    const std::string& twoStr,
    const std::string& threeStr)
{
    String ot_strFolder(strFolder), ot_oneStr(oneStr), ot_twoStr(twoStr),
        ot_threeStr(threeStr);
    OT_ASSERT_MSG(
        ot_strFolder.Exists(), "OTDB::StoreString: strFolder is null");

    if (!ot_oneStr.Exists()) {
        OT_ASSERT_MSG(
            (!ot_twoStr.Exists() && !ot_threeStr.Exists()),
            "OTDB::StoreString: bad options");
        ot_oneStr = strFolder.c_str();
        ot_strFolder = ".";
    }

    Storage* pStorage = details::s_pStorage;

    if (nullptr == pStorage) {
        return false;
    }

    return pStorage->StoreString(
        strContents, ot_strFolder.Get(), ot_oneStr.Get(), twoStr, threeStr);
}

std::string QueryString(
    const std::string& strFolder,
    const std::string& oneStr,
    const std::string& twoStr,
    const std::string& threeStr)
{
    String ot_strFolder(strFolder), ot_oneStr(oneStr), ot_twoStr(twoStr),
        ot_threeStr(threeStr);

    if (!CheckStringsExistInOrder(
            strFolder, oneStr, twoStr, threeStr, __FUNCTION__))
        return std::string("");

    if (!ot_oneStr.Exists()) {
        OT_ASSERT_MSG(
            (!ot_twoStr.Exists() && !ot_threeStr.Exists()),
            "Storage::StoreString: bad options");
        ot_oneStr = strFolder.c_str();
        ot_strFolder = ".";
    }

    Storage* pStorage = details::s_pStorage;

    if (nullptr == pStorage) return std::string("");

    return pStorage->QueryString(
        ot_strFolder.Get(), ot_oneStr.Get(), twoStr, threeStr);
}

// Store/Retrieve a plain string.

bool StorePlainString(
    const std::string& strContents,
    const std::string& strFolder,
    const std::string& oneStr,
    const std::string& twoStr,
    const std::string& threeStr)
{
    String ot_strFolder(strFolder), ot_oneStr(oneStr), ot_twoStr(twoStr),
        ot_threeStr(threeStr);
    OT_ASSERT_MSG(
        ot_strFolder.Exists(), "OTDB::StorePlainString: strFolder is null");

    if (!ot_oneStr.Exists()) {
        OT_ASSERT_MSG(
            (!ot_twoStr.Exists() && !ot_threeStr.Exists()),
            "OTDB::StorePlainString: bad options");
        ot_oneStr = strFolder.c_str();
        ot_strFolder = ".";
    }
    Storage* pStorage = details::s_pStorage;

    OT_ASSERT((strFolder.length() > 3) || (0 == strFolder.compare(0, 1, ".")));
    OT_ASSERT((oneStr.length() < 1) || (oneStr.length() > 3));

    if (nullptr == pStorage) {
        return false;
    }

    return pStorage->StorePlainString(
        strContents, ot_strFolder.Get(), ot_oneStr.Get(), twoStr, threeStr);
}

std::string QueryPlainString(
    const std::string& strFolder,
    const std::string& oneStr,
    const std::string& twoStr,
    const std::string& threeStr)
{
    String ot_strFolder(strFolder), ot_oneStr(oneStr), ot_twoStr(twoStr),
        ot_threeStr(threeStr);
    OT_ASSERT_MSG(
        ot_strFolder.Exists(), "OTDB::QueryPlainString: strFolder is null");

    if (!ot_oneStr.Exists()) {
        OT_ASSERT_MSG(
            (!ot_twoStr.Exists() && !ot_threeStr.Exists()),
            "OTDB::QueryPlainString: bad options");
        ot_oneStr = strFolder.c_str();
        ot_strFolder = ".";
    }

    Storage* pStorage = details::s_pStorage;

    OT_ASSERT((strFolder.length() > 3) || (0 == strFolder.compare(0, 1, ".")));
    OT_ASSERT((oneStr.length() < 1) || (oneStr.length() > 3));

    if (nullptr == pStorage) {
        return std::string("");
    }

    return pStorage->QueryPlainString(
        ot_strFolder.Get(), ot_oneStr.Get(), twoStr, threeStr);
}

// Store/Retrieve an object. (Storable.)

bool StoreObject(
    Storable& theContents,
    const std::string& strFolder,
    const std::string& oneStr,
    const std::string& twoStr,
    const std::string& threeStr)
{
    String ot_strFolder(strFolder), ot_oneStr(oneStr), ot_twoStr(twoStr),
        ot_threeStr(threeStr);
    OT_ASSERT_MSG(
        ot_strFolder.Exists(), "OTDB:StoreObject: strFolder is null");

    if (!ot_oneStr.Exists()) {
        OT_ASSERT_MSG(
            (!ot_twoStr.Exists() && !ot_threeStr.Exists()),
            "OTDB:StoreObject: bad options");
        ot_oneStr = strFolder.c_str();
        ot_strFolder = ".";
    }

    Storage* pStorage = details::s_pStorage;

    if (nullptr == pStorage) {
        otErr << "OTDB::StoreObject: No default storage object allocated.\n";
        return false;
    }

    return pStorage->StoreObject(
        theContents, ot_strFolder.Get(), ot_oneStr.Get(), twoStr, threeStr);
}

// Use %newobject Storage::Query();
Storable* QueryObject(
    const StoredObjectType theObjectType,
    const std::string& strFolder,
    const std::string& oneStr,
    const std::string& twoStr,
    const std::string& threeStr)
{
    String ot_strFolder(strFolder), ot_oneStr(oneStr), ot_twoStr(twoStr),
        ot_threeStr(threeStr);
    OT_ASSERT_MSG(
        ot_strFolder.Exists(), "OTDB::QueryObject: strFolder is null");

    if (!ot_oneStr.Exists()) {
        OT_ASSERT_MSG(
            (!ot_twoStr.Exists() && !ot_threeStr.Exists()),
            "OTDB::QueryObject: bad options");
        ot_oneStr = strFolder.c_str();
        ot_strFolder = ".";
    }

    Storage* pStorage = details::s_pStorage;

    if (nullptr == pStorage) {
        return nullptr;
    }

    return pStorage->QueryObject(
        theObjectType, ot_strFolder.Get(), ot_oneStr.Get(), twoStr, threeStr);
}

// Store/Retrieve a Storable object to/from an OTASCIIArmor object.

std::string EncodeObject(Storable& theContents)
{
    Storage* pStorage = details::s_pStorage;

    if (nullptr == pStorage) {
        otErr << "OTDB::EncodeObject: No Default Storage object allocated.\n";
        return "";
    }
    return pStorage->EncodeObject(theContents);
}

// Use %newobject Storage::DecodeObject();
Storable* DecodeObject(
    const StoredObjectType theObjectType,
    const std::string& strInput)
{
    Storage* pStorage = details::s_pStorage;

    if (nullptr == pStorage) {
        return nullptr;
    }

    return pStorage->DecodeObject(theObjectType, strInput);
}

// Erase a value by location.

bool EraseValueByKey(
    const std::string& strFolder,
    const std::string& oneStr,
    const std::string& twoStr,
    const std::string& threeStr)
{
    Storage* pStorage = details::s_pStorage;

    if (nullptr == pStorage) {
        otErr
            << "OTDB::EraseValueByKey: No Default Storage object allocated.\n";
        return false;
    }

    return pStorage->EraseValueByKey(strFolder, oneStr, twoStr, threeStr);
}

// Used internally. Creates the right subclass for any stored object type,
// based on which packer is needed.

Storable* Storable::Create(StoredObjectType eType, PackType thePackType)
{
    if (nullptr == details::pFunctionMap) return nullptr;

    Storable* pStorable = nullptr;

    // The Pack type, plus the Stored Object type, is the Key to the map of
    // function pointers.
    InstantiateFuncKey theKey(thePackType, eType);

    // If the key works, we get the function pointer to the static Create()
    // method for
    // the appropriate object type.

    auto it = details::pFunctionMap->find(theKey);

    if (details::pFunctionMap->end() == it) return nullptr;

    InstantiateFunc* pFunc = it->second;

    if (nullptr != pFunc) {
        pStorable = (*pFunc)();  // Now we instantiate the object...
    }

    return pStorable;  // May return nullptr...
}

// static. OTPacker Factory.
//
OTPacker* OTPacker::Create(PackType ePackType)
{
    OTPacker* pPacker = nullptr;

    switch (ePackType) {
#if defined(OTDB_MESSAGE_PACK)
        case PACK_MESSAGE_PACK:
            pPacker = new PackerMsgpack;
            OT_ASSERT(nullptr != pPacker);
            break;
#endif
#if defined(OTDB_PROTOCOL_BUFFERS)
        case PACK_PROTOCOL_BUFFERS:
            pPacker = new PackerPB;
            OT_ASSERT(nullptr != pPacker);
            break;
#endif
        case PACK_TYPE_ERROR:
        default:
            break;
    }

    return pPacker;  // May return nullptr...
}

PackType OTPacker::GetType() const
{
#if defined(OTDB_MESSAGE_PACK)
    if (typeid(*this) == typeid(PackerMsgpack)) return PACK_MESSAGE_PACK;
#endif
#if defined(OTDB_PROTOCOL_BUFFERS)
    if (typeid(*this) == typeid(PackerPB)) return PACK_PROTOCOL_BUFFERS;
#endif
    return PACK_TYPE_ERROR;
}

// Basically, ALL of the Storables have to implement the IStorable interface
// (or one of its subclasses).  They can override hookBeforePack(), and they
// can override onPack(). Those two methods will be where all the action is,
// for each subclass of OTPacker.
//
PackedBuffer* OTPacker::Pack(Storable& inObj)
{
    IStorable* pStorable = dynamic_cast<IStorable*>(&inObj);

    if (nullptr ==
        pStorable)  // ALL Storables should implement SOME subinterface
                    // of IStorable
    {
        otErr << "OTPacker::Pack: Error: IStorable dynamic_cast failed.\n";
        return nullptr;
    }

    // This is polymorphic, so we get the right kind of buffer for the packer.
    //
    PackedBuffer* pBuffer = CreateBuffer();
    OT_ASSERT(nullptr != pBuffer);

    // Must delete pBuffer, or return it, below this point.

    pStorable->hookBeforePack();  // Give the subclass a chance to prepare its
                                  // data for packing...

    // This line (commented out) shows how the line below it would have looked
    // if I had ended
    // up using polymorphic templates:
    //    if (!makeTStorablepStorable->pack(*pBuffer))

    if (!pStorable->onPack(*pBuffer, inObj)) {
        delete pBuffer;
        return nullptr;
    }

    return pBuffer;
}

// Similar to Pack, above.
// Unpack takes the contents of the PackedBuffer and unpacks them into
// the Storable. ASSUMES that the PackedBuffer is the right type for
// the Packer, usually because the Packer is the one who instantiated
// it.  Also assumes that the Storable's actual object type is the
// appropriate one for the data that is sitting in that buffer.
//
bool OTPacker::Unpack(PackedBuffer& inBuf, Storable& outObj)
{
    IStorable* pStorable = dynamic_cast<IStorable*>(&outObj);

    if (nullptr == pStorable) return false;

    // outObj is the OUTPUT OBJECT.
    // If we're unable to unpack the contents of inBuf
    // into outObj, return false.
    //
    if (!pStorable->onUnpack(inBuf, outObj)) {
        return false;
    }

    pStorable->hookAfterUnpack();  // Give the subclass a chance to settle its
                                   // data after unpacking...

    return true;
}

PackedBuffer* OTPacker::Pack(const std::string& inObj)
{
    // This is polymorphic, so we get the right kind of buffer for the packer.
    //
    PackedBuffer* pBuffer = CreateBuffer();
    OT_ASSERT(nullptr != pBuffer);

    // Must delete pBuffer, or return it, below this point.

    if (!pBuffer->PackString(inObj)) {
        delete pBuffer;
        return nullptr;
    }

    return pBuffer;
}

bool OTPacker::Unpack(PackedBuffer& inBuf, std::string& outObj)
{

    // outObj is the OUTPUT OBJECT.
    // If we're unable to unpack the contents of inBuf
    // into outObj, return false.
    //
    if (!inBuf.UnpackString(outObj)) return false;

    return true;
}

// NOTICE!!! that when you add something to the list, it is CLONED. (Caller is
// still responsible to delete the argument.)
//

#define IMPLEMENT_GET_ADD_REMOVE(scope, name)                                  \
                                                                               \
    typedef stlplus::simple_ptr_clone<name> PointerTo##name;                   \
                                                                               \
    typedef std::deque<PointerTo##name> listOf##name##s;                       \
                                                                               \
    size_t scope Get##name##Count() { return list_##name##s.size(); }          \
                                                                               \
    name* scope Get##name(size_t nIndex)                                       \
    {                                                                          \
        if (nIndex < list_##name##s.size()) {                                  \
            PointerTo##name theP = list_##name##s.at(nIndex);                  \
            return theP.pointer();                                             \
        }                                                                      \
        return nullptr;                                                        \
    }                                                                          \
                                                                               \
    bool scope Remove##name(size_t nIndex##name)                               \
    {                                                                          \
        if (nIndex##name < list_##name##s.size()) {                            \
            list_##name##s.erase(list_##name##s.begin() + nIndex##name);       \
            return true;                                                       \
        } else                                                                 \
            return false;                                                      \
    }                                                                          \
                                                                               \
    bool scope Add##name(name& disownObject)                                   \
    {                                                                          \
        PointerTo##name theP(disownObject.clone());                            \
        list_##name##s.push_back(theP);                                        \
        return true;                                                           \
    }

IMPLEMENT_GET_ADD_REMOVE(
    WalletData::,
    BitcoinServer)  // No semicolon on this one!

IMPLEMENT_GET_ADD_REMOVE(WalletData::, BitcoinAcct)  // No semicolon on this
                                                     // one!

IMPLEMENT_GET_ADD_REMOVE(
    WalletData::,
    RippleServer)  // No semicolon on this one!

IMPLEMENT_GET_ADD_REMOVE(WalletData::, LoomServer)  // No semicolon on this one!

IMPLEMENT_GET_ADD_REMOVE(ContactNym::, ServerInfo)  // No semicolon on this one!

IMPLEMENT_GET_ADD_REMOVE(Contact::, ContactNym)  // No semicolon on this one!

IMPLEMENT_GET_ADD_REMOVE(Contact::, ContactAcct)  // No semicolon on this one!

IMPLEMENT_GET_ADD_REMOVE(AddressBook::, Contact)  // No semicolon on this one!

IMPLEMENT_GET_ADD_REMOVE(MarketList::, MarketData)  // No semicolon on this one!

IMPLEMENT_GET_ADD_REMOVE(
    OfferListMarket::,
    BidData)  // No semicolon on this one!

IMPLEMENT_GET_ADD_REMOVE(
    OfferListMarket::,
    AskData)  // No semicolon on this one!

IMPLEMENT_GET_ADD_REMOVE(
    TradeListMarket::,
    TradeDataMarket)  // No semicolon on this one!

IMPLEMENT_GET_ADD_REMOVE(
    OfferListNym::,
    OfferDataNym)  // No semicolon on this one!

IMPLEMENT_GET_ADD_REMOVE(
    TradeListNym::,
    TradeDataNym)  // No semicolon on this one!

// Make sure SWIG "loses ownership" of any objects pushed onto these lists.
// (So I am safe to destruct them indiscriminately.)
//
// UPDATE: Nevertheless, no need to erase the lists (below) since they now
// store smart pointers, instead of regular pointers, so they are self-cleaning.
//

ContactNym::~ContactNym()
{
    //      while (GetServerInfoCount() > 0)
    //          RemoveServerInfo(0);
}

Contact::~Contact()
{
    //      while (GetContactNymCount() > 0)
    //          RemoveContactNym(0);
    //
    //      while (GetContactAcctCount() > 0)
    //          RemoveContactAcct(0);
}

AddressBook::~AddressBook()
{
    //      while (GetContactCount() > 0)
    //          RemoveContact(0);
}

//
// Interface:  IStorableMsgpack
//
//
// Msgpack packer.
#if defined(OTDB_MESSAGE_PACK)

// look into git history for old (dead) code.

#endif  // defined (OTDB_MESSAGE_PACK)

/* Protocol Buffers notes.

 // optional string bitcoin_id = 1;
 inline bool has_bitcoin_id() const;
 inline void clear_bitcoin_id();
 static const int32_t kBitcoinIdFieldNumber = 1;
 inline const ::std::string& bitcoin_id() const;
 inline void set_bitcoin_id(const ::std::string& value);
 inline void set_bitcoin_id(const char* value);
 inline void set_bitcoin_id(const char* value, size_t size);
 inline ::std::string* mutable_bitcoin_id();
 inline ::std::string* release_bitcoin_id();

 // optional string bitcoin_name = 2;
 inline bool has_bitcoin_name() const;
 inline void clear_bitcoin_name();
 static const int32_t kBitcoinNameFieldNumber = 2;
 inline const ::std::string& bitcoin_name() const;
 inline void set_bitcoin_name(const ::std::string& value);
 inline void set_bitcoin_name(const char* value);
 inline void set_bitcoin_name(const char* value, size_t size);
 inline ::std::string* mutable_bitcoin_name();
 inline ::std::string* release_bitcoin_name();

 // optional string gui_label = 3;
 inline bool has_gui_label() const;
 inline void clear_gui_label();
 static const int32_t kGuiLabelFieldNumber = 3;
 inline const ::std::string& gui_label() const;
 inline void set_gui_label(const ::std::string& value);
 inline void set_gui_label(const char* value);
 inline void set_gui_label(const char* value, size_t size);
 inline ::std::string* mutable_gui_label();
 inline ::std::string* release_gui_label();
 */
/*
 bool SerializeToString(string* output) const; serializes the message and stores
 the bytes in the given string.
 (Note that the bytes are binary, not text; we only use the string class as a
 convenient container.)
 bool ParseFromString(const string& data); parses a message from the given
 string.

 bool SerializeToOstream(ostream* output) const; writes the message to the given
 C++ ostream.
 bool ParseFromIstream(istream* input); parses a message from the given C++
 istream.
 */

// This is a case for template polymorphism.
// See this article:  http://accu.org/index.php/articles/471
//
/*
 template <class T>    // TStorable...
 class TStorable        // a "template subclass" of Storable. This is like a
 version of java
 {                    // interfaces, which C++ normally implements via pure
 virtual base classes
     T const& t;    // and multiple inheritance. But in this case, I need to
 have a consistent
 public:                // interface across disparate classes (in various
 circumstances including
     TStorable(T const& obj) : t(obj) { }    // here with protocol buffers) and
 template interfaces
     bool pack(PackedBuffer& theBuffer)    // allow me to do that even with
 classes in a different hierarchy.
     { return t.onPack(theBuffer); }
 };

 template <class T>
 TStorable<T> makeTStorable( T& obj )
 {
    return TStorable<T>( obj );
 }
 */

/* // Specialization:
 template<>
 void TStorable<BigBenClock>::talk()
 {
    t.playBongs();
 }

 // Passing and returning as parameter:

 template <class T>
 void makeItTalk( TStorable<T> t )
 {
    t.talk();
 }

 template <class T>
 TStorable<T> makeTalkative( T& obj )
 {
    return TStorable<T>( obj );
 }
 */

// Why have IStorablePB::onPack? What is this all about?
//
// Because normally, packing is done by Packer. I have a packer subclass for
// the protocol buffers library, but notice that I don't have a packer for EVERY
// SINGLE STORABLE OT OBJECT, for the protocol buffers library. So when
// Packer::Pack()
// is called, the subclass being activated is PackerPB, not
// PackerForBitcoinAccountOnPB.
//
// With MsgPack, that would be the end of it, since the MsgPack Storables all
// derive from
// the same base class (due to the msgPack define) and a single call handles all
// of them.
// But with Protocol Buffers (and probably with custom objects, which are coming
// next), EACH
// PB-Storable has to wrap an instance of the PB-derived serializable object
// generated by
// protoc. Each instance thus has a PB member of a slightly different type, and
// there is no
// common base class between them that will give me a reference to that member,
// without
// overriding some virtual function IN ALL OF THE PB-SERIALIZABLE OBJECTS so
// that each can
// individually pass back the reference to its unique PB-derived member.
//
// Even if there were, LET US REMEMBER that all of the various Storables
// (instantiated for
// various specific packers), such as BitcoinAcctPB for example, are supposed to
// be derived
// from a data class such as BitcoinAcct. That way, BitcoinAcct can focus on the
// data itself,
// regardless of packer type, and OT can deal with its data in a pure way,
// meanwhile the
// actual object used can be one of 5 different subclasses of that, depending on
// which
// packer was employed. All of those subclasses (for protocol buffers, for
// msgpack, for json,
// etc) must be derived from the data class, BitcoinAcct.
//
// Remember, if ALL of the protocol-buffers wrapper classes, such as
// BitcoinAcctPB,
// BitcoinServerPB, LoomAcctPB, LoomServerPB, etc, are all derived from some
// StorablePB object,
// so they can all share a virtual function and thereby return a reference to
// their internally-
// wrapped object, then how are all of those classes supposed to ALSO be derived
// from their
// DATA classes, such as BitcoinAcct, BitcoinServer, LoomAcct, LoomServer, etc??
//
// The answer is multiple inheritance. Or INTERFACES, to be more specific. I
// have implemented
// Java-style interfaces as well as polymorphism-by-template to resolve these
// issues.
//
// The Storable (parameter to Pack) is actually the object that somehow has to
// override--or implement--the actual packing. Only it really knows. Therefore I
// have decided
// to add an INTERFACE, which is OPTIONAL, which makes it possible to hook and
// override the
// packing/unpacking, but such that things are otherwise handled in a broad
// stroke, without
// having to override EVERY LITTLE THING to accomplish it.
//
// Storables, as I said, will all be derived from their respective data objects,
// no matter
// which packer is being employed. When packing one, the framework will check to
// see if IStorable
// is present. It it is, then the framework will use it instead of continuing
// with the normal
// Pack procedure. It will also call the hook (first) so values can be copied
// where appropriate,
// before the actual packing occurs, or after (for unpacking.)
//
// This means, first, that few of the storables will ever actually have to
// override Pack() or
// Unpack(), as long as they override onPack() as necessary. AND, when onPack()
// IS overridden,
// it will be able to handle many different objects (via the Interface,
// templates, etc), instead
// of each having to provide a separate Pack() implementation for EVERY SINGLE
// PB object. For
// example, the IStorablePB interface handles ALL of the PB objects, without ANY
// of them having
// to override some special pack function.
//
// It WOULD have been possible to add this interface to Storable itself.
// Functions such as
// Pack(), Unpack(), hookBeforePack(), onPack(), etc could have been added there
// and then passed
// down to all the subclasses. But that is not as elegant, for these reasons:
// 1) Remember that BitcoinAcct is purely data-oriented, and is not a
// packing-related class.
//    (though its subclasses are.) So the members would be out of context,
// except for some lame
//    explanation that the subclasses use them for other purposes unrelated to
// this class.
// 2) EVERY SINGLE class would be forced to provide its own implementation of
// those functions,
//    since a common base class for groups of them is already discounted, since
// they are derived
//    from their data classes, not their packer classes.
//
//
//
//
//
//
// Interface:    IStorablePB
//

// Protocol Buffers packer.
//
#if defined(OTDB_PROTOCOL_BUFFERS)

::google::protobuf::MessageLite* IStorablePB::getPBMessage()  // This is really
                                                              // only here so it
                                                              // can be
// overridden. Only
// subclasses of
// IStorablePB will
// actually exist.
{
    return nullptr;
}

template <class theBaseType,
          class theInternalType,
          StoredObjectType theObjectType>
::google::protobuf::MessageLite* ProtobufSubclass<theBaseType,
                                                  theInternalType,
                                                  theObjectType>::getPBMessage()
{
    return (&__pb_obj);
}

//    if (!makeTStorablepStorable->pack(*pBuffer))
//::google::protobuf::MessageLite& IStorablePB::getPBMessage()
//{
//    return makeTStorablePBgetPBMessage();
//}

bool IStorablePB::onPack(
    PackedBuffer& theBuffer,
    Storable&)  // buffer is OUTPUT.
{
    // check here to make sure theBuffer is the right TYPE.
    BufferPB* pBuffer = dynamic_cast<BufferPB*>(&theBuffer);

    if (nullptr == pBuffer)  // Buffer is wrong type!!
        return false;

    ::google::protobuf::MessageLite* pMessage = getPBMessage();

    if (nullptr == pMessage) return false;

    if (!pMessage->SerializeToString(&(pBuffer->m_buffer))) return false;

    return true;
}

bool IStorablePB::onUnpack(
    PackedBuffer& theBuffer,
    Storable&)  // buffer is INPUT.
{
    // check here to make sure theBuffer is the right TYPE.
    BufferPB* pBuffer = dynamic_cast<BufferPB*>(&theBuffer);

    if (nullptr == pBuffer)  // Buffer is wrong type!!
        return false;

    ::google::protobuf::MessageLite* pMessage = getPBMessage();

    if (nullptr == pMessage) return false;

    if (!pMessage->ParseFromString(pBuffer->m_buffer)) return false;

    return true;
}

/*
 bool SerializeToString(string* output) const;:
 Serializes the message and stores the bytes in the given string. Note that the
 bytes are binary,
 not text; we only use the string class as a convenient container.

 bool ParseFromString(const string& data);:
 parses a message from the given string.
 */
bool BufferPB::PackString(const std::string& theString)
{
    StringPB theWrapper;

    ::google::protobuf::MessageLite* pMessage = theWrapper.getPBMessage();

    if (nullptr == pMessage) return false;

    String_InternalPB* pBuffer = dynamic_cast<String_InternalPB*>(pMessage);

    if (nullptr == pBuffer)  // Buffer is wrong type!!
        return false;

    pBuffer->set_value(theString);

    if (!pBuffer->SerializeToString(&m_buffer)) return false;

    return true;
}

bool BufferPB::UnpackString(std::string& theString)
{
    StringPB theWrapper;

    ::google::protobuf::MessageLite* pMessage = theWrapper.getPBMessage();

    if (nullptr == pMessage) return false;

    String_InternalPB* pBuffer = dynamic_cast<String_InternalPB*>(pMessage);

    if (nullptr == pBuffer)  // Buffer is wrong type!!
        return false;

    if (!pBuffer->ParseFromString(m_buffer)) return false;

    theString = pBuffer->value();

    return true;
}

bool BufferPB::ReadFromIStream(std::istream& inStream, int64_t lFilesize)
{
    unsigned long size = static_cast<unsigned long>(lFilesize);

    char* buf = new char[size];
    OT_ASSERT(nullptr != buf);

    inStream.read(buf, size);

    if (inStream.good()) {
        m_buffer.assign(buf, size);
        delete[] buf;
        return true;
    }

    delete[] buf;
    buf = nullptr;

    return false;

    // m_buffer.ParseFromIstream(&inStream);
}

bool BufferPB::WriteToOStream(std::ostream& outStream)
{
    // bool    SerializeToOstream(ostream* output) const
    if (m_buffer.length() > 0) {
        outStream.write(m_buffer.c_str(), m_buffer.length());
        return outStream.good() ? true : false;
    } else {
        otErr << "Buffer had zero length in BufferPB::WriteToOStream\n";
    }

    return false;
    // m_buffer.SerializeToOstream(&outStream);
}

const uint8_t* BufferPB::GetData()
{
    return reinterpret_cast<const uint8_t*>(m_buffer.c_str());
}

size_t BufferPB::GetSize() { return m_buffer.size(); }

void BufferPB::SetData(const uint8_t* pData, size_t theSize)
{
    m_buffer.assign(reinterpret_cast<const char*>(pData), theSize);
}

// !! All of these have to provide implementations for the hookBeforePack and
// hookAfterUnpack methods.
// In .cpp file:
/*
 void SUBCLASS_HERE::hookBeforePack()
 {
 __pb_obj.set_PROPERTY_NAME_GOES_HERE(PROPERTY_NAME_GOES_HERE);
 }
 void SUBCLASS_HERE::hookAfterUnpack()
 {
 PROPERTY_NAME_GOES_HERE    = __pb_obj.PROPERTY_NAME_GOES_HERE();
 }
 */
//

#define OT_IMPLEMENT_PB_LIST_PACK(pb_name, element_type)                       \
    __pb_obj.clear_##pb_name();                                                \
    for (auto it = list_##element_type##s.begin();                             \
         it != list_##element_type##s.end();                                   \
         ++it) {                                                               \
        PointerTo##element_type thePtr = (*it);                                \
        element_type##PB* pObject =                                            \
            dynamic_cast<element_type##PB*>(thePtr.pointer());                 \
        OT_ASSERT(nullptr != pObject);                                         \
        ::google::protobuf::MessageLite* pMessage = pObject->getPBMessage();   \
        OT_ASSERT(nullptr != pMessage);                                        \
        element_type##_InternalPB* pInternal =                                 \
            dynamic_cast<element_type##_InternalPB*>(pMessage);                \
        OT_ASSERT(nullptr != pInternal);                                       \
        element_type##_InternalPB* pNewInternal = __pb_obj.add_##pb_name();    \
        OT_ASSERT(nullptr != pNewInternal);                                    \
        pObject->hookBeforePack();                                             \
        pNewInternal->CopyFrom(*pInternal);                                    \
    }

#define OT_IMPLEMENT_PB_LIST_UNPACK(pb_name, element_type, ELEMENT_ENUM)       \
    while (Get##element_type##Count() > 0) Remove##element_type(0);            \
    for (int32_t i = 0; i < __pb_obj.pb_name##_size(); i++) {                  \
        const element_type##_InternalPB& theInternal = __pb_obj.pb_name(i);    \
        element_type##PB* pNewWrapper = dynamic_cast<element_type##PB*>(       \
            Storable::Create(ELEMENT_ENUM, PACK_PROTOCOL_BUFFERS));            \
        OT_ASSERT(nullptr != pNewWrapper);                                     \
        ::google::protobuf::MessageLite* pMessage =                            \
            pNewWrapper->getPBMessage();                                       \
        OT_ASSERT(nullptr != pMessage);                                        \
        element_type##_InternalPB* pInternal =                                 \
            dynamic_cast<element_type##_InternalPB*>(pMessage);                \
        OT_ASSERT(nullptr != pInternal);                                       \
        pInternal->CopyFrom(theInternal);                                      \
        pNewWrapper->hookAfterUnpack();                                        \
        PointerTo##element_type thePtr(                                        \
            dynamic_cast<element_type*>(pNewWrapper));                         \
        list_##element_type##s.push_back(thePtr);                              \
    }

template <>
void WalletDataPB::hookBeforePack()
{
    OT_IMPLEMENT_PB_LIST_PACK(bitcoin_server, BitcoinServer)
    OT_IMPLEMENT_PB_LIST_PACK(bitcoin_acct, BitcoinAcct)
    OT_IMPLEMENT_PB_LIST_PACK(ripple_server, RippleServer)
    OT_IMPLEMENT_PB_LIST_PACK(loom_server, LoomServer)
}

template <>
void WalletDataPB::hookAfterUnpack()
{
    OT_IMPLEMENT_PB_LIST_UNPACK(
        bitcoin_server, BitcoinServer, STORED_OBJ_BITCOIN_SERVER)
    OT_IMPLEMENT_PB_LIST_UNPACK(
        bitcoin_acct, BitcoinAcct, STORED_OBJ_BITCOIN_ACCT)
    OT_IMPLEMENT_PB_LIST_UNPACK(
        ripple_server, RippleServer, STORED_OBJ_RIPPLE_SERVER)
    OT_IMPLEMENT_PB_LIST_UNPACK(loom_server, LoomServer, STORED_OBJ_LOOM_SERVER)
}

template <>
void StringMapPB::hookBeforePack()
{
    __pb_obj.clear_node();  // "node" is the repeated field of Key/Values.

    // Loop through all the key/value pairs in the map, and add them to
    // __pb_obj.node.
    //
    for (auto it = the_map.begin(); it != the_map.end(); ++it) {
        KeyValue_InternalPB* pNode = __pb_obj.add_node();
        pNode->set_key(it->first);
        pNode->set_value(it->second);
    }
}

template <>
void StringMapPB::hookAfterUnpack()
{
    //    the_map = __pb_obj.the_map();

    the_map.clear();

    for (int32_t i = 0; i < __pb_obj.node_size(); i++) {
        const KeyValue_InternalPB& theNode = __pb_obj.node(i);

        the_map.insert(
            std::pair<std::string, std::string>(
                theNode.key(), theNode.value()));
    }
}

template <>
void StringPB::hookBeforePack()
{
    __pb_obj.set_value(m_string);
    // The way StringPB is used, this function will never actually get called.
    // (But if you used it like the others, it would work, since this function
    // is here.)
}
template <>
void StringPB::hookAfterUnpack()
{
    m_string = __pb_obj.value();
    // The way StringPB is used, this function will never actually get called.
    // (But if you used it like the others, it would work, since this function
    // is here.)
}

template <>
void BlobPB::hookBeforePack()
{
    if (m_memBuffer.size() > 0)
        __pb_obj.set_value(m_memBuffer.data(), m_memBuffer.size());
}
template <>
void BlobPB::hookAfterUnpack()
{
    if (__pb_obj.has_value()) {
        std::string strTemp = __pb_obj.value();
        m_memBuffer.assign(strTemp.begin(), strTemp.end());
    }
}

template <>
void ContactPB::hookBeforePack()
{
    __pb_obj.set_gui_label(gui_label);
    __pb_obj.set_contact_id(contact_id);
    __pb_obj.set_email(email);
    __pb_obj.set_public_key(public_key);
    __pb_obj.set_memo(memo);

    OT_IMPLEMENT_PB_LIST_PACK(nyms, ContactNym)
    OT_IMPLEMENT_PB_LIST_PACK(accounts, ContactAcct)
}

template <>
void ContactPB::hookAfterUnpack()
{
    gui_label = __pb_obj.gui_label();
    contact_id = __pb_obj.contact_id();
    email = __pb_obj.email();
    public_key = __pb_obj.public_key();
    memo = __pb_obj.memo();

    OT_IMPLEMENT_PB_LIST_UNPACK(nyms, ContactNym, STORED_OBJ_CONTACT_NYM)
    OT_IMPLEMENT_PB_LIST_UNPACK(accounts, ContactAcct, STORED_OBJ_CONTACT_ACCT)
}

template <>
void ContactNymPB::hookBeforePack()
{
    __pb_obj.set_gui_label(gui_label);
    __pb_obj.set_nym_id(nym_id);
    __pb_obj.set_nym_type(nym_type);
    __pb_obj.set_public_key(public_key);
    __pb_obj.set_memo(memo);

    OT_IMPLEMENT_PB_LIST_PACK(servers, ServerInfo)
}

template <>
void ContactNymPB::hookAfterUnpack()
{
    gui_label = __pb_obj.gui_label();
    nym_id = __pb_obj.nym_id();
    nym_type = __pb_obj.nym_type();
    public_key = __pb_obj.public_key();
    memo = __pb_obj.memo();

    OT_IMPLEMENT_PB_LIST_UNPACK(servers, ServerInfo, STORED_OBJ_SERVER_INFO)
}

template <>
void AddressBookPB::hookBeforePack()
{
    OT_IMPLEMENT_PB_LIST_PACK(contacts, Contact)
}

template <>
void AddressBookPB::hookAfterUnpack()
{
    OT_IMPLEMENT_PB_LIST_UNPACK(contacts, Contact, STORED_OBJ_CONTACT)
}

template <>
void ContactAcctPB::hookBeforePack()
{
    __pb_obj.set_gui_label(gui_label);
    __pb_obj.set_notary_id(notary_id);
    __pb_obj.set_server_type(server_type);
    __pb_obj.set_instrument_definition_id(instrument_definition_id);
    __pb_obj.set_acct_id(acct_id);
    __pb_obj.set_nym_id(nym_id);
    __pb_obj.set_memo(memo);
    __pb_obj.set_public_key(public_key);
}
template <>
void ContactAcctPB::hookAfterUnpack()
{
    gui_label = __pb_obj.gui_label();
    notary_id = __pb_obj.notary_id();
    server_type = __pb_obj.server_type();
    instrument_definition_id = __pb_obj.instrument_definition_id();
    acct_id = __pb_obj.acct_id();
    nym_id = __pb_obj.nym_id();
    memo = __pb_obj.memo();
    public_key = __pb_obj.public_key();
}

template <>
void ServerInfoPB::hookBeforePack()
{
    __pb_obj.set_notary_id(notary_id);
    __pb_obj.set_server_type(server_type);
}
template <>
void ServerInfoPB::hookAfterUnpack()
{
    notary_id = __pb_obj.notary_id();
    server_type = __pb_obj.server_type();
}

template <>
void BitcoinAcctPB::hookBeforePack()
{
    __pb_obj.set_gui_label(gui_label);
    __pb_obj.set_acct_id(acct_id);
    __pb_obj.set_notary_id(notary_id);
    __pb_obj.set_bitcoin_acct_name(bitcoin_acct_name);
}
template <>
void BitcoinAcctPB::hookAfterUnpack()
{
    gui_label = __pb_obj.gui_label();
    acct_id = __pb_obj.acct_id();
    notary_id = __pb_obj.notary_id();
    bitcoin_acct_name = __pb_obj.bitcoin_acct_name();
}

template <>
void BitcoinServerPB::hookBeforePack()
{
    __pb_obj.set_gui_label(gui_label);
    __pb_obj.set_notary_id(notary_id);
    __pb_obj.set_server_type(server_type);
    __pb_obj.set_server_host(server_host);
    __pb_obj.set_server_port(server_port);
    __pb_obj.set_bitcoin_username(bitcoin_username);
    __pb_obj.set_bitcoin_password(bitcoin_password);
}
template <>
void BitcoinServerPB::hookAfterUnpack()
{
    gui_label = __pb_obj.gui_label();
    notary_id = __pb_obj.notary_id();
    server_type = __pb_obj.server_type();
    server_host = __pb_obj.server_host();
    server_port = __pb_obj.server_port();
    bitcoin_username = __pb_obj.bitcoin_username();
    bitcoin_password = __pb_obj.bitcoin_password();
}

template <>
void RippleServerPB::hookBeforePack()
{
    __pb_obj.set_gui_label(gui_label);
    __pb_obj.set_notary_id(notary_id);
    __pb_obj.set_server_type(server_type);
    __pb_obj.set_server_host(server_host);
    __pb_obj.set_server_port(server_port);
    __pb_obj.set_ripple_username(ripple_username);
    __pb_obj.set_ripple_password(ripple_password);
    __pb_obj.set_namefield_id(namefield_id);
    __pb_obj.set_passfield_id(passfield_id);
}
template <>
void RippleServerPB::hookAfterUnpack()
{
    gui_label = __pb_obj.gui_label();
    notary_id = __pb_obj.notary_id();
    server_type = __pb_obj.server_type();
    server_host = __pb_obj.server_host();
    server_port = __pb_obj.server_port();
    ripple_username = __pb_obj.ripple_username();
    ripple_password = __pb_obj.ripple_password();
    namefield_id = __pb_obj.namefield_id();
    passfield_id = __pb_obj.passfield_id();
}

template <>
void LoomServerPB::hookBeforePack()
{
    __pb_obj.set_gui_label(gui_label);
    __pb_obj.set_notary_id(notary_id);
    __pb_obj.set_server_type(server_type);
    __pb_obj.set_server_host(server_host);
    __pb_obj.set_server_port(server_port);
    __pb_obj.set_loom_username(loom_username);
    __pb_obj.set_namefield_id(namefield_id);
}
template <>
void LoomServerPB::hookAfterUnpack()
{
    gui_label = __pb_obj.gui_label();
    notary_id = __pb_obj.notary_id();
    server_type = __pb_obj.server_type();
    server_host = __pb_obj.server_host();
    server_port = __pb_obj.server_port();
    loom_username = __pb_obj.loom_username();
    namefield_id = __pb_obj.namefield_id();
}

template <>
void MarketDataPB::hookBeforePack()
{
    __pb_obj.set_gui_label(gui_label);
    __pb_obj.set_notary_id(notary_id);
    __pb_obj.set_market_id(market_id);
    __pb_obj.set_instrument_definition_id(instrument_definition_id);
    __pb_obj.set_currency_type_id(currency_type_id);
    __pb_obj.set_scale(scale);
    __pb_obj.set_total_assets(total_assets);
    __pb_obj.set_number_bids(number_bids);
    __pb_obj.set_number_asks(number_asks);
    __pb_obj.set_last_sale_price(last_sale_price);
    __pb_obj.set_last_sale_date(last_sale_date);
    __pb_obj.set_current_bid(current_bid);
    __pb_obj.set_current_ask(current_ask);
    __pb_obj.set_volume_trades(volume_trades);
    __pb_obj.set_volume_assets(volume_assets);
    __pb_obj.set_volume_currency(volume_currency);
    __pb_obj.set_recent_highest_bid(recent_highest_bid);
    __pb_obj.set_recent_lowest_ask(recent_lowest_ask);
}

template <>
void MarketDataPB::hookAfterUnpack()
{
    gui_label = __pb_obj.gui_label();
    notary_id = __pb_obj.notary_id();
    market_id = __pb_obj.market_id();
    instrument_definition_id = __pb_obj.instrument_definition_id();
    currency_type_id = __pb_obj.currency_type_id();
    scale = __pb_obj.scale();
    total_assets = __pb_obj.total_assets();
    number_bids = __pb_obj.number_bids();
    number_asks = __pb_obj.number_asks();
    last_sale_price = __pb_obj.last_sale_price();
    last_sale_date = __pb_obj.last_sale_date();
    current_bid = __pb_obj.current_bid();
    current_ask = __pb_obj.current_ask();
    volume_trades = __pb_obj.volume_trades();
    volume_assets = __pb_obj.volume_assets();
    volume_currency = __pb_obj.volume_currency();
    recent_highest_bid = __pb_obj.recent_highest_bid();
    recent_lowest_ask = __pb_obj.recent_lowest_ask();
}

template <>
void MarketListPB::hookBeforePack()
{
    OT_IMPLEMENT_PB_LIST_PACK(market_data, MarketData)
}

template <>
void MarketListPB::hookAfterUnpack()
{
    OT_IMPLEMENT_PB_LIST_UNPACK(market_data, MarketData, STORED_OBJ_MARKET_DATA)
}

template <>
void BidDataPB::hookBeforePack()
{
    __pb_obj.set_gui_label(gui_label);
    __pb_obj.set_transaction_id(transaction_id);
    __pb_obj.set_price_per_scale(price_per_scale);
    __pb_obj.set_available_assets(available_assets);
    __pb_obj.set_minimum_increment(minimum_increment);
    __pb_obj.set_date(date);
}

template <>
void BidDataPB::hookAfterUnpack()
{
    gui_label = __pb_obj.gui_label();
    transaction_id = __pb_obj.transaction_id();
    price_per_scale = __pb_obj.price_per_scale();
    available_assets = __pb_obj.available_assets();
    minimum_increment = __pb_obj.minimum_increment();
    date = __pb_obj.date();
}

template <>
void AskDataPB::hookBeforePack()
{
    __pb_obj.set_gui_label(gui_label);
    __pb_obj.set_transaction_id(transaction_id);
    __pb_obj.set_price_per_scale(price_per_scale);
    __pb_obj.set_available_assets(available_assets);
    __pb_obj.set_minimum_increment(minimum_increment);
    __pb_obj.set_date(date);
}

template <>
void AskDataPB::hookAfterUnpack()
{
    gui_label = __pb_obj.gui_label();
    transaction_id = __pb_obj.transaction_id();
    price_per_scale = __pb_obj.price_per_scale();
    available_assets = __pb_obj.available_assets();
    minimum_increment = __pb_obj.minimum_increment();
    date = __pb_obj.date();
}

template <>
void OfferListMarketPB::hookBeforePack()
{
    OT_IMPLEMENT_PB_LIST_PACK(bids, BidData)
    OT_IMPLEMENT_PB_LIST_PACK(asks, AskData)
}

template <>
void OfferListMarketPB::hookAfterUnpack()
{
    OT_IMPLEMENT_PB_LIST_UNPACK(bids, BidData, STORED_OBJ_BID_DATA)
    OT_IMPLEMENT_PB_LIST_UNPACK(asks, AskData, STORED_OBJ_ASK_DATA)
}

template <>
void TradeDataMarketPB::hookBeforePack()
{
    __pb_obj.set_gui_label(gui_label);
    __pb_obj.set_transaction_id(transaction_id);
    __pb_obj.set_date(date);
    __pb_obj.set_price(price);
    __pb_obj.set_amount_sold(amount_sold);
}

template <>
void TradeDataMarketPB::hookAfterUnpack()
{
    gui_label = __pb_obj.gui_label();
    transaction_id = __pb_obj.transaction_id();
    date = __pb_obj.date();
    price = __pb_obj.price();
    amount_sold = __pb_obj.amount_sold();
}

template <>
void TradeListMarketPB::hookBeforePack()
{
    OT_IMPLEMENT_PB_LIST_PACK(trades, TradeDataMarket)
}

template <>
void TradeListMarketPB::hookAfterUnpack()
{
    OT_IMPLEMENT_PB_LIST_UNPACK(
        trades, TradeDataMarket, STORED_OBJ_TRADE_DATA_MARKET)
}

template <>
void OfferDataNymPB::hookBeforePack()
{
    __pb_obj.set_gui_label(gui_label);
    __pb_obj.set_valid_from(valid_from);
    __pb_obj.set_valid_to(valid_to);
    __pb_obj.set_notary_id(notary_id);
    __pb_obj.set_instrument_definition_id(instrument_definition_id);
    __pb_obj.set_asset_acct_id(asset_acct_id);
    __pb_obj.set_currency_type_id(currency_type_id);
    __pb_obj.set_currency_acct_id(currency_acct_id);
    __pb_obj.set_selling(selling);
    __pb_obj.set_scale(scale);
    __pb_obj.set_price_per_scale(price_per_scale);
    __pb_obj.set_transaction_id(transaction_id);
    __pb_obj.set_total_assets(total_assets);
    __pb_obj.set_finished_so_far(finished_so_far);
    __pb_obj.set_minimum_increment(minimum_increment);
    __pb_obj.set_stop_sign(stop_sign);
    __pb_obj.set_stop_price(stop_price);
    __pb_obj.set_date(date);
}

template <>
void OfferDataNymPB::hookAfterUnpack()
{
    gui_label = __pb_obj.gui_label();
    valid_from = __pb_obj.valid_from();
    valid_to = __pb_obj.valid_to();
    notary_id = __pb_obj.notary_id();
    instrument_definition_id = __pb_obj.instrument_definition_id();
    asset_acct_id = __pb_obj.asset_acct_id();
    currency_type_id = __pb_obj.currency_type_id();
    currency_acct_id = __pb_obj.currency_acct_id();
    selling = __pb_obj.selling();
    scale = __pb_obj.scale();
    price_per_scale = __pb_obj.price_per_scale();
    transaction_id = __pb_obj.transaction_id();
    total_assets = __pb_obj.total_assets();
    finished_so_far = __pb_obj.finished_so_far();
    minimum_increment = __pb_obj.minimum_increment();
    stop_sign = __pb_obj.stop_sign();
    stop_price = __pb_obj.stop_price();
    date = __pb_obj.date();
}

template <>
void OfferListNymPB::hookBeforePack()
{
    OT_IMPLEMENT_PB_LIST_PACK(offers, OfferDataNym)
}

template <>
void OfferListNymPB::hookAfterUnpack()
{
    OT_IMPLEMENT_PB_LIST_UNPACK(offers, OfferDataNym, STORED_OBJ_OFFER_DATA_NYM)
}

template <>
void TradeDataNymPB::hookBeforePack()
{
    __pb_obj.set_gui_label(gui_label);
    __pb_obj.set_transaction_id(transaction_id);
    __pb_obj.set_completed_count(completed_count);
    __pb_obj.set_date(date);
    __pb_obj.set_price(price);
    __pb_obj.set_amount_sold(amount_sold);
    __pb_obj.set_updated_id(updated_id);
    __pb_obj.set_offer_price(offer_price);
    __pb_obj.set_finished_so_far(finished_so_far);
    __pb_obj.set_instrument_definition_id(instrument_definition_id);
    __pb_obj.set_currency_id(currency_id);
    __pb_obj.set_currency_paid(currency_paid);
    __pb_obj.set_asset_acct_id(asset_acct_id);
    __pb_obj.set_currency_acct_id(currency_acct_id);
    __pb_obj.set_scale(scale);
    __pb_obj.set_is_bid(is_bid);
    __pb_obj.set_asset_receipt(asset_receipt);
    __pb_obj.set_currency_receipt(currency_receipt);
    __pb_obj.set_final_receipt(final_receipt);
}

template <>
void TradeDataNymPB::hookAfterUnpack()
{
    gui_label = __pb_obj.gui_label();
    transaction_id = __pb_obj.transaction_id();
    completed_count = __pb_obj.completed_count();
    date = __pb_obj.date();
    price = __pb_obj.price();
    amount_sold = __pb_obj.amount_sold();
    updated_id = __pb_obj.updated_id();
    offer_price = __pb_obj.offer_price();
    finished_so_far = __pb_obj.finished_so_far();
    instrument_definition_id = __pb_obj.instrument_definition_id();
    currency_id = __pb_obj.currency_id();
    currency_paid = __pb_obj.currency_paid();
    asset_acct_id = __pb_obj.asset_acct_id();
    currency_acct_id = __pb_obj.currency_acct_id();
    scale = __pb_obj.scale();
    is_bid = __pb_obj.is_bid();
    asset_receipt = __pb_obj.asset_receipt();
    currency_receipt = __pb_obj.currency_receipt();
    final_receipt = __pb_obj.final_receipt();
}

template <>
void TradeListNymPB::hookBeforePack()
{
    OT_IMPLEMENT_PB_LIST_PACK(trades, TradeDataNym)
}

template <>
void TradeListNymPB::hookAfterUnpack()
{
    OT_IMPLEMENT_PB_LIST_UNPACK(trades, TradeDataNym, STORED_OBJ_TRADE_DATA_NYM)
}

#endif  // defined (OTDB_PROTOCOL_BUFFERS)

//
// STORAGE :: GetPacker
//
// Use this to access the OTPacker, throughout duration of this Storage object.
// If it doesn't exist yet, this function will create it on the first call. (The
// parameter allows you the choose what type will be created, other than
// default.
// You probably won't use it. But if you do, you'll only call it once per
// instance
// of Storage.)
//
OTPacker* Storage::GetPacker(PackType ePackType)
{
    // Normally if you use Create(), the packer is created at that time.
    // However, in the future, coders using the API may create subclasses of
    // Storage through SWIG, which Create could not anticipate. This mechanism
    // makes sure that in those cases, the packer still gets set (on the first
    // Get() call), and the coder using the API still has the ability to choose
    // what type of packer will be used.
    //
    if (nullptr == m_pPacker) {
        m_pPacker = OTPacker::Create(ePackType);
    }

    return m_pPacker;  // May return nullptr. (If Create call above fails.)
}

// (SetPacker(), from .h file)
// This is called once, in the factory.
// void Storage::SetPacker(OTPacker& thePacker) { OT_ASSERT(nullptr ==
// m_pPacker);
// m_pPacker =  &thePacker; }

//
// Factory for Storable objects...
//
Storable* Storage::CreateObject(const StoredObjectType& eType)
{
    OTPacker* pPacker = GetPacker();

    if (nullptr == pPacker) {
        otErr << "OTDB::Storage::CreateObject: Failed, since GetPacker() "
                 "returned nullptr.\n";
        return nullptr;
    }

    Storable* pStorable = Storable::Create(eType, pPacker->GetType());

    return pStorable;  // May return nullptr.
}

// Factory for the Storage context itself.
//
Storage* Storage::Create(
    const StorageType& eStorageType,
    const PackType& ePackType)
{
    Storage* pStore = nullptr;

    switch (eStorageType) {
        case STORE_FILESYSTEM:
            pStore = StorageFS::Instantiate();
            OT_ASSERT(nullptr != pStore);
            break;
        //            case STORE_COUCH_DB:
        //                pStore = new StorageCouchDB; OT_ASSERT(nullptr !=
        //                pStore);
        // break;
        default:
            otErr << "OTDB::Storage::Create: Failed: Unknown storage type.\n";
            break;
    }

    // IF we successfully created the storage context, now let's
    // try to create the packer that goes with it.
    // (They are created together and linked until death.)

    if (nullptr != pStore) {
        OTPacker* pPacker = OTPacker::Create(ePackType);

        if (nullptr == pPacker) {
            otErr << "OTDB::Storage::Create: Failed while creating packer.\n";

            // For whatever reason, we failed. Memory issues or whatever.
            delete pStore;
            pStore = nullptr;
            return nullptr;
        }

        // Now they're married.
        pStore->SetPacker(*pPacker);
    } else
        otErr << "OTDB::Storage::Create: Failed, since pStore is nullptr.\n";

    return pStore;  // Possible to return nullptr.
}

StorageType Storage::GetType() const
{
    // If I find the type, then I return it. Otherwise I ASSUME
    // that the coder using the API has subclassed Storage, and
    // that this is a custom Storage type invented by the API user.

    if (typeid(*this) == typeid(StorageFS)) return STORE_FILESYSTEM;
    //    else if (typeid(*this) == typeid(StorageCouchDB))
    //        return STORE_COUCH_DB;
    //  Etc.
    //
    else
        return STORE_TYPE_SUBCLASS;  // The Java coder using API must have
                                     // subclassed Storage himself.
}

bool Storage::StoreString(
    const std::string& strContents,
    const std::string& strFolder,
    const std::string& oneStr,
    const std::string& twoStr,
    const std::string& threeStr)
{
    String ot_strFolder(strFolder), ot_oneStr(oneStr), ot_twoStr(twoStr),
        ot_threeStr(threeStr);
    OT_ASSERT_MSG(
        ot_strFolder.Exists(), "Storage::StoreString: strFolder is null");

    if (!ot_oneStr.Exists()) {
        OT_ASSERT_MSG(
            (!ot_twoStr.Exists() && !ot_threeStr.Exists()),
            "Storage::StoreString: bad options");
        ot_oneStr = strFolder.c_str();
        ot_strFolder = ".";
    }

    OTPacker* pPacker = GetPacker();

    if (nullptr == pPacker) return false;

    PackedBuffer* pBuffer = pPacker->Pack(strContents);

    if (nullptr == pBuffer) return false;

    const bool bSuccess =
        onStorePackedBuffer(
            *pBuffer,
            ot_strFolder.Get(),
            ot_oneStr.Get(),
            twoStr,
            threeStr);

    // Don't want any leaks here, do we?
    delete pBuffer;
    pBuffer = nullptr;

    return bSuccess;
}

std::string Storage::QueryString(
    const std::string& strFolder,
    const std::string& oneStr,
    const std::string& twoStr,
    const std::string& threeStr)
{
    std::string theString("");

    OTPacker* pPacker = GetPacker();

    if (nullptr == pPacker) return theString;

    PackedBuffer* pBuffer = pPacker->CreateBuffer();

    if (nullptr == pBuffer) return theString;

    // Below this point, responsible for pBuffer.

    bool bSuccess =
        onQueryPackedBuffer(*pBuffer, strFolder, oneStr, twoStr, threeStr);

    if (!bSuccess) {
        delete pBuffer;
        pBuffer = nullptr;
        return theString;
    }

    // We got the packed buffer back from the query!
    // Now let's unpack it and return the Storable object.

    bool bUnpacked = pPacker->Unpack(*pBuffer, theString);

    if (!bUnpacked) {
        delete pBuffer;
        theString = "";
        return theString;
    }

    // Success :-)

    // Don't want any leaks here, do we?
    delete pBuffer;
    pBuffer = nullptr;

    return theString;
}

// For when you want NO PACKING.

bool Storage::StorePlainString(
    const std::string& strContents,
    const std::string& strFolder,
    const std::string& oneStr,
    const std::string& twoStr,
    const std::string& threeStr)
{
    return onStorePlainString(strContents, strFolder, oneStr, twoStr, threeStr);
}

std::string Storage::QueryPlainString(
    const std::string& strFolder,
    const std::string& oneStr,
    const std::string& twoStr,
    const std::string& threeStr)
{
    std::string theString("");

    if (!onQueryPlainString(theString, strFolder, oneStr, twoStr, threeStr))
        theString = "";

    return theString;
}

bool Storage::StoreObject(
    Storable& theContents,
    const std::string& strFolder,
    const std::string& oneStr,
    const std::string& twoStr,
    const std::string& threeStr)
{
    OTPacker* pPacker = GetPacker();

    if (nullptr == pPacker) {
        otErr << "No packer allocated in Storage::StoreObject\n";
        return false;
    }

    PackedBuffer* pBuffer = pPacker->Pack(theContents);

    if (nullptr == pBuffer) {
        otErr << "Packing failed in Storage::StoreObject\n";
        return false;
    }

    bool bSuccess =
        onStorePackedBuffer(*pBuffer, strFolder, oneStr, twoStr, threeStr);

    if (!bSuccess) {
        otErr << "Storing failed in Storage::StoreObject (calling "
                 "onStorePackedBuffer) \n";
        return false;
    }

    // Don't want any leaks here, do we?
    delete pBuffer;

    return bSuccess;
}

// Use %newobject Storage::Query();
//
Storable* Storage::QueryObject(
    const StoredObjectType& theObjectType,
    const std::string& strFolder,
    const std::string& oneStr,
    const std::string& twoStr,
    const std::string& threeStr)
{
    OTPacker* pPacker = GetPacker();

    if (nullptr == pPacker) return nullptr;

    PackedBuffer* pBuffer = pPacker->CreateBuffer();

    if (nullptr == pBuffer) return nullptr;

    // Below this point, responsible for pBuffer.

    Storable* pStorable = CreateObject(theObjectType);

    if (nullptr == pStorable) {
        delete pBuffer;
        return nullptr;
    }

    // Below this point, responsible for pBuffer AND pStorable.

    bool bSuccess =
        onQueryPackedBuffer(*pBuffer, strFolder, oneStr, twoStr, threeStr);

    if (!bSuccess) {
        delete pBuffer;
        delete pStorable;

        return nullptr;
    }

    // We got the packed buffer back from the query!
    // Now let's unpack it and return the Storable object.

    bool bUnpacked = pPacker->Unpack(*pBuffer, *pStorable);

    if (!bUnpacked) {
        delete pBuffer;
        delete pStorable;

        return nullptr;
    }

    // Success :-)

    // Don't want any leaks here, do we?
    delete pBuffer;

    return pStorable;  // caller is responsible to delete.
}

std::string Storage::EncodeObject(Storable& theContents)
{
    std::string strReturnValue("");

    OTPacker* pPacker = GetPacker();

    if (nullptr == pPacker) {
        otErr << "Storage::EncodeObject: No packer allocated.\n";
        return strReturnValue;
    }

    PackedBuffer* pBuffer = pPacker->Pack(theContents);

    if (nullptr == pBuffer) {
        otErr << "Storage::EncodeObject: Packing failed.\n";
        return strReturnValue;
    }

    // OTPackedBuffer:
    //        virtual const    uint8_t *    GetData()=0;
    //        virtual            size_t            GetSize()=0;
    //
    const uint32_t nNewSize = static_cast<const uint32_t>(pBuffer->GetSize());
    const void* pNewData = static_cast<const void*>(pBuffer->GetData());

    if ((nNewSize < 1) || (nullptr == pNewData)) {
        delete pBuffer;
        pBuffer = nullptr;

        otErr << "Storage::EncodeObject: Packing failed (2).\n";
        return strReturnValue;
    }

    const Data theData(pNewData, nNewSize);
    const OTASCIIArmor theArmor(theData);

    strReturnValue.assign(theArmor.Get(), theArmor.GetLength());

    // Don't want any leaks here, do we?
    delete pBuffer;

    return strReturnValue;
}

// Use %newobject Storage::DecodeObject();
//
Storable* Storage::DecodeObject(
    const StoredObjectType& theObjectType,
    const std::string& strInput)
{
    if (strInput.size() < 1) return nullptr;

    OTPacker* pPacker = GetPacker();

    if (nullptr == pPacker) return nullptr;

    PackedBuffer* pBuffer = pPacker->CreateBuffer();

    if (nullptr == pBuffer) return nullptr;

    // Below this point, responsible for pBuffer.

    Storable* pStorable = CreateObject(theObjectType);

    if (nullptr == pStorable) {
        delete pBuffer;
        return nullptr;
    }

    // Below this point, responsible for pBuffer AND pStorable.

    OTASCIIArmor theArmor;
    theArmor.Set(strInput.c_str(), static_cast<uint32_t>(strInput.size()));
    const Data thePayload(theArmor);

    // Put thePayload's contents into pBuffer here.
    //
    pBuffer->SetData(
        static_cast<const uint8_t*>(thePayload.GetPointer()),
        thePayload.GetSize());

    // Now let's unpack it and return the Storable object.

    const bool bUnpacked = pPacker->Unpack(*pBuffer, *pStorable);

    if (!bUnpacked) {
        delete pBuffer;
        delete pStorable;

        return nullptr;
    }

    // Success :-)

    // Don't want any leaks here, do we?
    delete pBuffer;

    return pStorable;  // caller is responsible to delete.
}

bool Storage::EraseValueByKey(
    const std::string& strFolder,
    const std::string& oneStr,
    const std::string& twoStr,
    const std::string& threeStr)
{
    bool bSuccess = onEraseValueByKey(strFolder, oneStr, twoStr, threeStr);

    if (!bSuccess)
        otErr << "Storage::EraseValueByKey: Failed trying to erase a value "
                 "(while calling onEraseValueByKey) \n";

    return bSuccess;
}

// STORAGE FS  (OTDB::StorageFS is the filesystem version of OTDB::Storage.)

// ConfirmOrCreateFolder()
// Used for making sure that certain necessary folders actually exist. (Creates
// them otherwise.)
//
// If you pass in "spent", then this function will make sure that "<path>/spent"
// actually exists,
// or create it. WARNING: If what you want to pass is
// "spent/sub-folder-to-spent" then make SURE
// you call it with "spent" FIRST, so you are sure THAT folder has been created,
// otherwise the
// folder creation will definitely fail on the sub-folder call (if the primary
// folder wasn't
// already there, that is.)
//
bool StorageFS::ConfirmOrCreateFolder(const char* szFolderName, struct stat*)
{
    bool bConfirmOrCreateSuccess = false, bFolderAlreadyExist = false;
    String strFolderName(szFolderName);
    if (!OTPaths::ConfirmCreateFolder(
            strFolderName, bConfirmOrCreateSuccess, bFolderAlreadyExist)) {
        OT_FAIL;
    };
    return bConfirmOrCreateSuccess;
}

// Returns true or false whether a specific file exists.
// Adds the main path prior to checking.
//
bool StorageFS::ConfirmFile(const char* szFileName, struct stat*)
{
    String strFilePath("");
    OTPaths::AppendFile(strFilePath, String(m_strDataPath), String(szFileName));
    return OTPaths::PathExists(strFilePath);
}

/*
 - Based on the input, constructs the full path and returns it in strOutput.
 - This function will try to create all the folders leading up to the
   file itself.
 - Also returns true/false based on whether the path actually exists.
 - If some failure occurs along the way, the path returned will not be the
   full path, but the path where the failure occurred.

 New return values:

 -1        -- Error
  0        -- File not found
  1+    -- File found and it's length.

 */
int64_t StorageFS::ConstructAndCreatePath(
    std::string& strOutput,
    const std::string& strFolder,
    const std::string& oneStr,
    const std::string& twoStr,
    const std::string& threeStr)
{
    return ConstructAndConfirmPathImp(
        true, strOutput, strFolder, oneStr, twoStr, threeStr);
}

int64_t StorageFS::ConstructAndConfirmPath(
    std::string& strOutput,
    const std::string& strFolder,
    const std::string& oneStr,
    const std::string& twoStr,
    const std::string& threeStr)
{
    return ConstructAndConfirmPathImp(
        false, strOutput, strFolder, oneStr, twoStr, threeStr);
}

int64_t StorageFS::ConstructAndConfirmPathImp(
    const bool bMakePath,
    std::string& strOutput,
    const std::string& zeroStr,
    const std::string& oneStr,
    const std::string& twoStr,
    const std::string& threeStr)
{
    const std::string strRoot(m_strDataPath.c_str());
    const std::string strZero(3 > zeroStr.length() ? "" : zeroStr);
    const std::string strOne(3 > oneStr.length() ? "" : oneStr);
    const std::string strTwo(3 > twoStr.length() ? "" : twoStr);
    const std::string strThree(3 > threeStr.length() ? "" : threeStr);

    // must be 3chars in length, or equal to "."
    if (strZero.empty() && (0 != zeroStr.compare("."))) {
        otErr << __FUNCTION__ << ": Empty: "
              << "zeroStr"
              << " is too short (and not \".\").!\n"
                 "zeroStr was: \""
              << zeroStr << "\"\n";
        return -1;
    }

    // the first string must not be empty
    if (strOne.empty()) {
        otErr << __FUNCTION__ << ": Empty: "
              << "oneStr"
              << " passed in!\n";
        return -2;
    }

    // if the second string is empty, so must the third.
    if (strTwo.empty() && !strThree.empty()) {
        otErr << __FUNCTION__ << ": Error: strThree passed in: " << strThree
              << " while strTwo is empty!\n";
        return -3;
    }

    const bool bHaveZero = !strZero.empty();
    const bool bOneIsLast = strTwo.empty();
    const bool bTwoIsLast = !bOneIsLast && strThree.empty();
    const bool bThreeIsLast = !bOneIsLast && !bTwoIsLast;

    // main vairables;
    std::string strBufFolder("");
    std::string strBufPath("");

    // main block
    {
        // root (either way)
        strBufFolder += strRoot;

        // Zero
        if (bHaveZero) {
            strBufFolder += strZero;
            strBufFolder += "/";
        }

        // One
        if (bOneIsLast) {
            strBufPath = strBufFolder;
            strBufPath += strOne;
            goto ot_exit_block;
        }

        strBufFolder += strOne;
        strBufFolder += "/";

        // Two
        if (bTwoIsLast) {
            strBufPath = strBufFolder;
            strBufPath += strTwo;
            goto ot_exit_block;
        }

        strBufFolder += strTwo;
        strBufFolder += "/";

        // Three
        if (bThreeIsLast) {
            strBufPath = strBufFolder;
            strBufPath += threeStr;
            goto ot_exit_block;
        }
        // should never get here.
        OT_FAIL;
    }
ot_exit_block:

    // set as constants. (no more changing).
    const std::string strFolder(strBufFolder);
    const std::string strPath(strBufPath);
    strOutput = strPath;

    if (bMakePath) {
        bool bFolderCreated = false;
        OTPaths::BuildFolderPath(strFolder.c_str(), bFolderCreated);
    }

    {
        const bool bFolderExists = OTPaths::PathExists(strFolder.c_str());

        if (bMakePath && !bFolderExists) {
            otErr << __FUNCTION__ << ": Error: was told to make path, however "
                                     "cannot confirm the path!\n";
            return -4;
        }
        if (!bMakePath && !bFolderExists) {
            otWarn << __FUNCTION__
                   << ": Debug: Cannot find Folder: " << strFolder << " \n";
        }
    }

    {
        int64_t lFileLength = 0;
        const bool bFileExists =
            OTPaths::FileExists(strPath.c_str(), lFileLength);

        if (bFileExists)
            return lFileLength;
        else
            return 0;
    }
}

// Store/Retrieve an object. (Storable.)

bool StorageFS::onStorePackedBuffer(
    PackedBuffer& theBuffer,
    const std::string& strFolder,
    const std::string& oneStr,
    const std::string& twoStr,
    const std::string& threeStr)
{
    std::string strOutput;

    if (0 > ConstructAndCreatePath(
                strOutput, strFolder, oneStr, twoStr, threeStr)) {
        otErr << __FUNCTION__ << ": Error writing to " << strOutput << ".\n";
        return false;
    }

    // TODO: Should check here to see if there is a .lock file for the target...

    // TODO: If not, next I should actually create a .lock file for myself right
    // here..

    // SAVE to the file here
    std::ofstream ofs(strOutput.c_str(), std::ios::out | std::ios::binary);

    if (ofs.fail()) {
        otErr << __FUNCTION__ << ": Error opening file: " << strOutput << "\n";
        return false;
    }

    ofs.clear();
    bool bSuccess = theBuffer.WriteToOStream(ofs);
    ofs.close();

    // TODO: Remove the .lock file.

    return bSuccess;
}

bool StorageFS::onQueryPackedBuffer(
    PackedBuffer& theBuffer,
    const std::string& strFolder,
    const std::string& oneStr,
    const std::string& twoStr,
    const std::string& threeStr)
{
    std::string strOutput;

    int64_t lRet =
        ConstructAndConfirmPath(strOutput, strFolder, oneStr, twoStr, threeStr);

    if (0 > lRet) {
        otErr << "StorageFS::" << __FUNCTION__ << ": Error with " << strOutput
              << ".\n";
        return false;
    } else if (0 == lRet) {
        otErr << "StorageFS::" << __FUNCTION__ << ": Failure reading from "
              << strOutput << ": file does not exist.\n";
        return false;
    }

    // READ from the file here

    std::ifstream fin(strOutput.c_str(), std::ios::in | std::ios::binary);

    if (!fin.is_open()) {
        otErr << __FUNCTION__ << ": Error opening file: " << strOutput << "\n";
        return false;
    }

    bool bSuccess = theBuffer.ReadFromIStream(fin, lRet);

    fin.close();

    return bSuccess;
}

// Store/Retrieve a plain string, (without any packing.)

bool StorageFS::onStorePlainString(
    const std::string& theBuffer,
    const std::string& strFolder,
    const std::string& oneStr,
    const std::string& twoStr,
    const std::string& threeStr)
{
    std::string strOutput;

    if (0 > ConstructAndCreatePath(
                strOutput, strFolder, oneStr, twoStr, threeStr)) {
        otErr << "StorageFS::" << __FUNCTION__ << ": Error writing to "
              << strOutput << ".\n";
        return false;
    }

    // TODO: Should check here to see if there is a .lock file for the target...

    // TODO: If not, next I should actually create a .lock file for myself right
    // here..

    // SAVE to the file here.
    //
    // Here's where the serialization code would be changed to CouchDB or
    // whatever.
    // In a key/value database, szFilename is the "key" and strFinal.Get() is
    // the "value".
    //
    std::ofstream ofs(strOutput.c_str(), std::ios::out | std::ios::binary);

    if (ofs.fail()) {
        otErr << __FUNCTION__ << ": Error opening file: " << strOutput << "\n";
        return false;
    }

    ofs.clear();
    ofs << theBuffer;
    bool bSuccess = ofs.good();
    ofs.close();

    // TODO: Remove the .lock file.

    return bSuccess;
}

bool StorageFS::onQueryPlainString(
    std::string& theBuffer,
    const std::string& strFolder,
    const std::string& oneStr,
    const std::string& twoStr,
    const std::string& threeStr)
{
    std::string strOutput;

    int64_t lRet =
        ConstructAndConfirmPath(strOutput, strFolder, oneStr, twoStr, threeStr);

    if (0 > lRet) {
        otErr << "StorageFS::" << __FUNCTION__ << ": Error with " << strOutput
              << ".\n";
        return false;
    } else if (0 == lRet) {
        otErr << "StorageFS::" << __FUNCTION__ << ": Failure reading from "
              << strOutput << ": file does not exist.\n";
        return false;
    }

    // Open the file here

    std::ifstream fin(strOutput.c_str(), std::ios::in | std::ios::binary);

    if (!fin.is_open()) {
        otErr << __FUNCTION__ << ": Error opening file: " << strOutput << "\n";
        return false;
    }

    // Read from the file as a plain string.

    std::stringstream buffer;
    buffer << fin.rdbuf();

    bool bSuccess = fin.good();

    if (bSuccess)
        theBuffer = buffer.str();  // here's the actual output of this function.
    else {
        theBuffer = "";
        return false;
    }

    bSuccess = (theBuffer.length() > 0);

    fin.close();

    return bSuccess;
}

// Erase a value by location.
//
bool StorageFS::onEraseValueByKey(
    const std::string& strFolder,
    const std::string& oneStr,
    const std::string& twoStr,
    const std::string& threeStr)
{
    std::string strOutput;

    if (0 > ConstructAndConfirmPath(
                strOutput, strFolder, oneStr, twoStr, threeStr)) {
        otErr << "Error: " << __FUNCTION__
              << ": Failed calling ConstructAndConfirmPath with:\n"
                 "strOutput: "
              << strOutput << " | strFolder: " << strFolder
              << " | oneStr: " << oneStr << " | twoStr: " << twoStr
              << " | threeStr: " << threeStr << " \n";

        return false;
    }

    // TODO: Should check here to see if there is a .lock file for the target...

    // TODO: If not, next I should actually create a .lock file for myself right
    // here..

    // SAVE to the file here. (a blank string.)
    //
    // Here's where the serialization code would be changed to CouchDB or
    // whatever.
    // In a key/value database, szFilename is the "key" and strFinal.Get() is
    // the "value".
    //
    std::ofstream ofs(strOutput.c_str(), std::ios::out | std::ios::binary);

    if (ofs.fail()) {
        otErr << "Error opening file in StorageFS::onEraseValueByKey: "
              << strOutput << "\n";
        return false;
    }

    ofs.clear();
    ofs << "(This space intentionally left blank.)\n";
    bool bSuccess = ofs.good() ? true : false;
    ofs.close();
    // Note: I bet you think I should be overwriting the file 7 times here with
    // random data, right? Wrong: YOU need to override OTStorage and create your
    // own subclass, where you can override onEraseValueByKey and do that stuff
    // yourself. It's outside of the scope of OT.

    if (remove(strOutput.c_str()) != 0) {
        bSuccess = false;
        otErr << "** Failed trying to delete file:  " << strOutput << " \n";
    } else {
        bSuccess = true;
        otInfo << "** Success deleting file:  " << strOutput << " \n";
    }

    // TODO: Remove the .lock file.

    return bSuccess;
}

// Constructor for Filesystem storage context.
//
StorageFS::StorageFS()
    : Storage()
{
    String strDataPath;
    OTDataFolder::Get(strDataPath);
    m_strDataPath = strDataPath.Get();
}

StorageFS::~StorageFS() {}

// See if the file is there.

bool StorageFS::Exists(
    const std::string& strFolder,
    const std::string& oneStr,
    const std::string& twoStr,
    const std::string& threeStr)
{
    std::string strOutput;

    return (
        0 < ConstructAndConfirmPath(
                strOutput, strFolder, oneStr, twoStr, threeStr));
}

// Returns path size, plus path in strOutput.
//
int64_t StorageFS::FormPathString(
    std::string& strOutput,
    const std::string& strFolder,
    const std::string& oneStr,
    const std::string& twoStr,
    const std::string& threeStr)
{
    return ConstructAndConfirmPath(
        strOutput, strFolder, oneStr, twoStr, threeStr);
}

}  // namespace OTDB

}  // namespace opentxs
