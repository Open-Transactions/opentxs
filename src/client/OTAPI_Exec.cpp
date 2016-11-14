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

#include "opentxs/client/OTAPI_Exec.hpp"

#include "opentxs/cash/Purse.hpp"
#include "opentxs/client/Helpers.hpp"
#include "opentxs/client/OTWallet.hpp"
#include "opentxs/client/OT_API.hpp"
#include "opentxs/core/app/App.hpp"
#include "opentxs/core/app/Identity.hpp"
#include "opentxs/core/app/Wallet.hpp"
#include "opentxs/core/contract/basket/Basket.hpp"
#include "opentxs/core/contract/peer/PeerObject.hpp"
#include "opentxs/core/cron/OTCronItem.hpp"
#if OT_CRYPTO_WITH_BIP39
#include "opentxs/core/crypto/Bip39.hpp"
#endif
#include "opentxs/core/crypto/CredentialSet.hpp"
#include "opentxs/core/crypto/CryptoEngine.hpp"
#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/crypto/OTAsymmetricKey.hpp"
#include "opentxs/core/crypto/OTEnvelope.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/crypto/OTSymmetricKey.hpp"
#include "opentxs/core/recurring/OTPaymentPlan.hpp"
#include "opentxs/core/script/OTAgent.hpp"
#include "opentxs/core/script/OTBylaw.hpp"
#include "opentxs/core/script/OTClause.hpp"
#include "opentxs/core/script/OTParty.hpp"
#include "opentxs/core/script/OTPartyAccount.hpp"
#include "opentxs/core/script/OTScriptable.hpp"
#include "opentxs/core/script/OTVariable.hpp"
#include "opentxs/core/stdafx.hpp"
#include "opentxs/core/transaction/Helpers.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/OTPaths.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Item.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/NumList.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/NymIDSource.hpp"
#include "opentxs/core/OTData.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/ext/InstantiateContract.hpp"
#include "opentxs/ext/OTPayment.hpp"

#include <cstdint>
#include <memory>
#include <sstream>
#include <string>

namespace opentxs
{

#ifndef OT_FALSE
const int32_t OT_FALSE = 0;
#endif

#ifndef OT_TRUE
const int32_t OT_TRUE = 1;
#endif

#ifndef OT_ERROR
const int32_t OT_ERROR = (-1);
#endif

bool OTAPI_Exec::bInitOTApp = false;
bool OTAPI_Exec::bCleanupOTApp = false;

OTAPI_Exec::OTAPI_Exec()
    : p_OTAPI(nullptr)
{
}

OTAPI_Exec::~OTAPI_Exec() {}

bool OTAPI_Exec::AppInit()  // Call this ONLY ONCE, when your App first starts
                            // up.
{
    if (!OTAPI_Exec::bCleanupOTApp) {
        if (!OTAPI_Exec::bInitOTApp) {
            OTAPI_Exec::bInitOTApp = true;
            if (OT_API::InitOTApp()) {
                OT_API* tmpOTAPI = new OT_API();
                if (nullptr != tmpOTAPI) {
                    if (tmpOTAPI->IsInitialized()) {
                        p_OTAPI = tmpOTAPI;
                        return true;
                    }
                    delete tmpOTAPI;
                    tmpOTAPI = nullptr;
                }
            }
        }
    }
    return false;
}

bool OTAPI_Exec::AppCleanup()  // Call this ONLY ONCE, when your App is shutting
                               // down.
{
    if (!OTAPI_Exec::bCleanupOTApp)  // if we haven't cleaned up already
    {
        if (OTAPI_Exec::bInitOTApp)  // and have had a ctx.
        {
            // will cleanup everything.
            if (nullptr != p_OTAPI) delete p_OTAPI;
            p_OTAPI = nullptr;

            OTAPI_Exec::bCleanupOTApp = true;
            return OT_API::CleanupOTApp();
        }
    }
    return false;
}

// SetAppBinaryFolder
// OPTIONAL. Used in Android and Qt.
//
// Certain platforms use this to override the Prefix folder.
// Basically /usr/local is the prefix folder by default, meaning
// /usr/local/lib/opentxs will be the location of the scripts. But
// if you override AppBinary folder to, say, "res/raw"
// (Android does something like that) then even though the prefix remains
// as /usr/local, the scripts folder will be res/raw
//
//
void OTAPI_Exec::SetAppBinaryFolder(const std::string& strFolder) const
{
    OTPaths::SetAppBinaryFolder(strFolder.c_str());
}

// SetHomeFolder
// OPTIONAL. Used in Android.
//
// The AppDataFolder, such as /Users/au/.ot, is constructed from the home
// folder, such as /Users/au.
//
// Normally the home folder is auto-detected, but certain platforms, such as
// Android, require us to explicitly set this folder from the Java code. Then
// the AppDataFolder is constructed from it. (It's the only way it can be done.)
//
// In Android, you would SetAppBinaryFolder to the path to
// "/data/app/packagename/res/raw",
// and you would SetHomeFolder to "/data/data/[app package]/files/"
//
void OTAPI_Exec::SetHomeFolder(const std::string& strFolder) const
{
    OTPaths::SetHomeFolder(strFolder.c_str());
}

OT_API* OTAPI_Exec::OTAPI() const { return p_OTAPI; }

int64_t OTAPI_Exec::StringToLong(const std::string& strNumber) const
{
    return String::StringToLong(strNumber);
}

std::string OTAPI_Exec::LongToString(const int64_t& lNumber) const
{
    return String::LongToString(lNumber);
}

uint64_t OTAPI_Exec::StringToUlong(const std::string& strNumber) const
{
    return String::StringToUlong(strNumber);
}

std::string OTAPI_Exec::UlongToString(const uint64_t& lNumber) const
{
    return String::UlongToString(lNumber);
}

bool OTAPI_Exec::CheckSetConfigSection(const std::string& strSection, const std::string& strComment)
{
    bool b_isNewSection = false;

    const bool bSuccess = App::Me().Config().CheckSetSection(strSection.c_str(),
                                                             strComment.c_str(),
                                                             b_isNewSection);
    if (bSuccess && b_isNewSection)
    {
        if (!App::Me().Config().Save()) {
            Log::vError("%s: Error: Unable to save updated config file.\n", __FUNCTION__);
            OT_FAIL;
        }
    }

    return bSuccess;
}

bool OTAPI_Exec::SetConfig_str(const std::string& strSection, const std::string& strKey,
                               const std::string& strValue)
{
    bool b_isNew = false;

    const bool bSuccess = App::Me().Config().Set_str(strSection.c_str(),
                                                     strKey.c_str(),
                                                     strValue.c_str(),
                                                     b_isNew);
    if (bSuccess && b_isNew)
    {
        if (!App::Me().Config().Save()) {
            Log::vError("%s: Error: Unable to save updated config file.\n", __FUNCTION__);
            OT_FAIL;
        }
    }

    return bSuccess;
}

bool OTAPI_Exec::SetConfig_long(const std::string& strSection, const std::string& strKey,
                                const int64_t& lValue)
{
    bool b_isNew = false;

    const bool bSuccess = App::Me().Config().Set_long(strSection.c_str(),
                                                      strKey.c_str(),
                                                      lValue,
                                                      b_isNew);
    if (bSuccess && b_isNew)
    {
        if (!App::Me().Config().Save()) {
            Log::vError("%s: Error: Unable to save updated config file.\n", __FUNCTION__);
            OT_FAIL;
        }
    }

    return bSuccess;

}

bool OTAPI_Exec::SetConfig_bool(const std::string& strSection, const std::string& strKey,
                                const bool bValue)
{
    bool b_isNew = false;

    const bool bSuccess = App::Me().Config().Set_bool(strSection.c_str(),
                                                      strKey.c_str(),
                                                      bValue,
                                                      b_isNew);
    if (bSuccess && b_isNew)
    {
        if (!App::Me().Config().Save()) {
            Log::vError("%s: Error: Unable to save updated config file.\n", __FUNCTION__);
            OT_FAIL;
        }
    }

    return bSuccess;

}

std::string OTAPI_Exec::GetConfig_str(const std::string& strSection, const std::string& strKey) const
{
    String strOutput;
    bool bKeyExists = false;

    const bool bSuccess = App::Me().Config().Check_str(strSection.c_str(),
                                                       strKey.c_str(),
                                                       strOutput,
                                                       bKeyExists);
    std::string str_result = "";

    if (bSuccess && bKeyExists)
        str_result = strOutput.Get();

    return str_result;
}

int64_t OTAPI_Exec::GetConfig_long(const std::string& strSection, const std::string& strKey) const
{
    int64_t lOutput = 0;
    bool bKeyExists = false;

    const bool bSuccess = App::Me().Config().Check_long(strSection.c_str(),
                                                        strKey.c_str(),
                                                        lOutput,
                                                        bKeyExists);
    if (bSuccess && bKeyExists)
        return lOutput;

    return 0;
}

bool OTAPI_Exec::GetConfig_bool(const std::string& strSection, const std::string& strKey) const
{
    bool bOutput = false;
    bool bKeyExists = false;

    const bool bSuccess = App::Me().Config().Check_bool(strSection.c_str(),
                                                        strKey.c_str(),
                                                        bOutput,
                                                        bKeyExists);
    if (bSuccess && bKeyExists)
        return bOutput;

    return false;
}


/** Output to the screen (stderr.)
(This is so stdout can be left clean for the ACTUAL output.)
Log level is 0 (least verbose) to 5 (most verbose.)
*/
void OTAPI_Exec::Output(const int32_t& nLogLevel, const std::string& strOutput)
    const
{
    const String otstrOutput(!strOutput.empty() ? strOutput : "\n");

    Log::Output(nLogLevel, otstrOutput.Get());
}

bool OTAPI_Exec::SetWallet(const std::string& strWalletFilename) const
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__ << ": Error: OT_API not Initialized!!\n";
        return false;
    } else {
        String sWalletFilename(strWalletFilename);

        if (sWalletFilename.Exists()) {
            return OTAPI()->SetWalletFilename(sWalletFilename);
        } else {
            otErr << __FUNCTION__ << ": Error:: Wallet Filename is Null!\n";
        }
        return false;
    }
}

/**
WALLET EXISTS

Just Checks if the m_pWallet pointer is not null.

WalletExists();

*/
bool OTAPI_Exec::WalletExists() const { return OTAPI()->WalletExists(); }

bool OTAPI_Exec::LoadWallet() const { return OTAPI()->LoadWallet(); }

bool OTAPI_Exec::SwitchWallet() const { return OTAPI()->LoadWallet(); }

int32_t OTAPI_Exec::GetMemlogSize() const { return Log::GetMemlogSize(); }

std::string OTAPI_Exec::GetMemlogAtIndex(const int32_t& nIndex) const
{
    return Log::GetMemlogAtIndex(nIndex).Get();
}

std::string OTAPI_Exec::PeekMemlogFront() const
{
    return Log::PeekMemlogFront().Get();
}

std::string OTAPI_Exec::PeekMemlogBack() const
{
    return Log::PeekMemlogBack().Get();
}

bool OTAPI_Exec::PopMemlogFront() const { return Log::PopMemlogFront(); }

bool OTAPI_Exec::PopMemlogBack() const { return Log::PopMemlogBack(); }

// OpenTransactions.h
// bool      NumList_Add        (OTNumList& theList, const OTNumList&
// theNewNumbers);
// bool      NumList_Remove     (OTNumList& theList, const OTNumList&
// theOldNumbers);
// bool      NumList_VerifyQuery(OTNumList& theList, const OTNumList&
// theQueryNumbers);
// bool      NumList_VerifyAll  (OTNumList& theList, const OTNumList&
// theQueryNumbers);
// int32_t   NumList_Count      (OTNumList& theList);

// OTAPI_funcdef.h
// std::string      OTAPI_Exec::NumList_Add        (const std::string&
// strNumList, const std::string& strNumbers);
// std::string      OTAPI_Exec::NumList_Remove     (const std::string&
// strNumList, const std::string& strNumbers);
// int32_t          OTAPI_Exec::NumList_VerifyQuery(const std::string&
// strNumList, const std::string& strNumbers); // returns bool
// int32_t          OTAPI_Exec::NumList_VerifyAll  (const std::string&
// strNumList, const std::string& strNumbers); // returns bool
// int32_t          OTAPI_Exec::NumList_Count      (const std::string&
// strNumList);

// Returns new list if ALL strNumbers are successfully added to strNumList.
// Otherwise returns "" and doesn't change anything.
//
std::string OTAPI_Exec::NumList_Add(
    const std::string& strNumList,
    const std::string& strNumbers) const
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return "";
    }
    if (strNumbers.empty()) {
        otErr << __FUNCTION__ << ": Null: strNumbers passed in!\n";
        return "";
    }

    // strNumList can be null, (create a new one).
    NumList theList, theNewNumbers(strNumbers);

    if ("" != strNumList) {
        const String otstrNumList(strNumList);
        theList.Add(otstrNumList);
    }

    const bool& bAdded = OTAPI()->NumList_Add(theList, theNewNumbers);

    String strOutput;
    if (bAdded && theList.Output(strOutput)) {
        std::string pBuf = strOutput.Get();

        return pBuf;
    }

    return "";
}

// Returns new list if ALL strNumbers are successfully removed from strNumList.
// Otherwise returns "" and doesn't change anything.
//
std::string OTAPI_Exec::NumList_Remove(
    const std::string& strNumList,
    const std::string& strNumbers) const
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return "";
    }

    if (strNumList.empty()) {
        otErr << __FUNCTION__ << ": Null: strNumList passed in!\n";
        return "";
    }
    if (strNumbers.empty()) {
        otErr << __FUNCTION__ << ": Null: strNumbers passed in!\n";
        return "";
    }

    NumList theList(strNumList), theNewNumbers(strNumbers);

    const bool& bRemoved = OTAPI()->NumList_Remove(theList, theNewNumbers);

    String strOutput;
    if (bRemoved && theList.Output(strOutput)) {
        std::string pBuf = strOutput.Get();

        return pBuf;
    }

    return "";
}

// Verifies strNumbers as a SUBSET of strNumList.
//
bool OTAPI_Exec::NumList_VerifyQuery(
    const std::string& strNumList,
    const std::string& strNumbers) const
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return false;
    }

    if (strNumList.empty()) {
        otErr << __FUNCTION__ << ": Null: strNumList passed in!\n";
        return false;
    }
    if (strNumbers.empty()) {
        otErr << __FUNCTION__ << ": Null: strNumbers passed in!\n";
        return false;
    }

    NumList theList(strNumList), theNewNumbers(strNumbers);

    const bool& bVerified =
        OTAPI()->NumList_VerifyQuery(theList, theNewNumbers);

    return bVerified ? true : false;
}

// Verifies COUNT and CONTENT but NOT ORDER.
//
bool OTAPI_Exec::NumList_VerifyAll(
    const std::string& strNumList,
    const std::string& strNumbers) const
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return false;
    }
    if (strNumList.empty()) {
        otErr << __FUNCTION__ << ": Null: strNumList passed in!\n";
        return false;
    }
    if (strNumbers.empty()) {
        otErr << __FUNCTION__ << ": Null: strNumbers passed in!\n";
        return false;
    }
    NumList theList(strNumList), theNewNumbers(strNumbers);

    const bool& bVerified = OTAPI()->NumList_VerifyAll(theList, theNewNumbers);

    return bVerified ? true : false;
}

int32_t OTAPI_Exec::NumList_Count(const std::string& strNumList) const
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return OT_ERROR;
    }
    if (strNumList.empty()) {
        otErr << __FUNCTION__ << ": Null: strNumList passed in!\n";
        return OT_ERROR;
    }
    NumList theList(strNumList);

    return OTAPI()->NumList_Count(theList);
}

bool OTAPI_Exec::IsValidID(const std::string& strPurportedID) const
{
    return Identifier::validateID(strPurportedID);
}

std::string OTAPI_Exec::NymIDFromPaymentCode(
    const std::string& paymentCode) const
{
    return OT_API::NymIDFromPaymentCode(paymentCode);
}

// CREATE NYM  -- Create new User
//
// Creates a new Nym and adds it to the wallet.
// (Including PUBLIC and PRIVATE KEYS.)
//
// Returns a new Nym ID (with files already created)
// or "" upon failure.
//
// Once it exists, use OTAPI_Exec::registerNym() to
// register your new Nym at any given Server. (Nearly all
// server requests require this...)
//
std::string OTAPI_Exec::CreateNymLegacy(
    const int32_t& nKeySize,  // must be 1024, 2048, 4096, or 8192
    __attribute__((unused))
    const std::string& NYM_ID_SOURCE) const  // Can be empty.
{
    if (0 >= nKeySize) {
        otErr << __FUNCTION__
              << ": Keysize is 0 or less, will fail! Try 1024.\n";
        return "";
    }
    // ---------------------------------------
    switch (nKeySize) {
        case 1024:
        case 2048:
        case 4096:
        case 8192:
            break;
        default:
            otErr << __FUNCTION__ << ": Failure: nKeySize must be one of: "
                                     "1024, 2048, 4096, 8192. ("
                  << nKeySize << " was passed...)\n";
            return "";
    }

    std::shared_ptr<NymParameters> nymParameters;
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    nymParameters = std::make_shared<NymParameters>(nKeySize);
#else
    nymParameters = std::make_shared<NymParameters>(proto::CREDTYPE_LEGACY);
#endif

    Nym* pNym = OTAPI()->CreateNym(*nymParameters);
    if (nullptr == pNym)  // Creation failed.
    {
        otOut << __FUNCTION__ << ": Failed trying to create Nym.\n";
        return "";
    }
    // -----------------------------------------------------}
    String strOutput;
    pNym->GetIdentifier(strOutput);  // We're returning the new Nym ID.
    if (strOutput.Exists()) return strOutput.Get();
    return "";
}

std::string OTAPI_Exec::CreateNymHD(const std::string& fingerprint) const
{
    Log::vOutput(0, "OTAPI_Exec::CreateNymHD: 1\n");

#if OT_CRYPTO_SUPPORTED_KEY_HD
    Log::vOutput(0, "OTAPI_Exec::CreateNymHD: 2\n");

    NymParameters nymParameters(proto::CREDTYPE_HD);

    if (0 < fingerprint.size()) {
        
        Log::vOutput(0, "OTAPI_Exec::CreateNymHD: 3\n");

        nymParameters.SetSeed(fingerprint);
    }

    
    Log::vOutput(0, "OTAPI_Exec::CreateNymHD: 4\n");

    Nym* pNym = OTAPI()->CreateNym(nymParameters);

    Log::vOutput(0, "OTAPI_Exec::CreateNymHD: 5\n");

    if (nullptr == pNym) {
        otOut << __FUNCTION__ << ": Failed trying to create Nym." << std::endl;

        return "";
    }

    Log::vOutput(0, "OTAPI_Exec::CreateNymHD: 1\n");

    String strOutput;
    pNym->GetIdentifier(strOutput);

    if (strOutput.Exists()) {
        return strOutput.Get();
    }

    return "";
#else
    otOut << __FUNCTION__ << ": No support for HD key derivation." << std::endl;

    return "";
#endif
}

std::string OTAPI_Exec::GetNym_ActiveCronItemIDs(
    const std::string& NYM_ID,
    const std::string& NOTARY_ID) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": nullptr NYM_ID passed in!\n";
        return "";
    }
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": nullptr NOTARY_ID passed in!\n";
        return "";
    }
    const Identifier nymId(NYM_ID), notaryID(NOTARY_ID);
    NumList numlist;
    std::string str_return;

    if (OTCronItem::GetActiveCronTransNums(numlist, nymId, notaryID)) {
        String strOutput;
        numlist.Output(strOutput);
        str_return = strOutput.Get();
    }

    return str_return;
}

std::string OTAPI_Exec::GetActiveCronItem(
    const std::string& NOTARY_ID,
    int64_t lTransNum) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": nullptr NOTARY_ID passed in!\n";
        return "";
    }
    if (0 > lTransNum) {
        otErr << __FUNCTION__ << ": Negative: lTransNum passed in!\n";
        return "";
    }
    const Identifier notaryID(NOTARY_ID);
    std::string str_return;
    const int64_t lTransactionNum = lTransNum;

    std::unique_ptr<OTCronItem> pCronItem(
        OTCronItem::LoadActiveCronReceipt(lTransactionNum, notaryID));
    if (nullptr != pCronItem) {
        const String strCronItem(*pCronItem);

        str_return = strCronItem.Get();
    }
    return str_return;
}

std::string OTAPI_Exec::GetNym_SourceForID(const std::string& NYM_ID) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": nullptr NYM_ID passed in!\n";
        return "";
    }
    OTPasswordData thePWData(OT_PW_DISPLAY);
    Identifier nym_id(NYM_ID);
    // This tries to get, then tries to load as public, then tries to load as
    // private.
    const Nym* pNym =
        OTAPI()->GetOrLoadNym(nym_id, false, __FUNCTION__, &thePWData);
    if (nullptr == pNym) return "";
    const std::string str_return(pNym->Source().asString().Get());
    return str_return;
}

std::string OTAPI_Exec::GetNym_Description(const std::string& NYM_ID) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": nullptr NYM_ID passed in!\n";
        return "";
    }
    OTPasswordData thePWData(OT_PW_DISPLAY);
    Identifier nym_id(NYM_ID);
    // This tries to get, then tries to load as public, then tries to load as
    // private.
    const Nym* pNym =
        OTAPI()->GetOrLoadNym(nym_id, false, __FUNCTION__, &thePWData);
    if (nullptr == pNym) return "";
    const std::string str_return(pNym->GetDescription().Get());
    return str_return;
}

int32_t OTAPI_Exec::GetNym_MasterCredentialCount(
    const std::string& NYM_ID) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": nullptr NYM_ID passed in!\n";
        return OT_ERROR;
    }
    OTPasswordData thePWData(OT_PW_DISPLAY);
    Identifier nym_id(NYM_ID);
    // This tries to get, then tries to load as public, then tries to load as
    // private.
    const Nym* pNym =
        OTAPI()->GetOrLoadNym(nym_id, false, __FUNCTION__, &thePWData);
    if (nullptr == pNym) return OT_ERROR;
    const int32_t nReturnValue =
        static_cast<int32_t>(pNym->GetMasterCredentialCount());
    return nReturnValue;
}

std::string OTAPI_Exec::GetNym_MasterCredentialID(
    const std::string& NYM_ID,
    const int32_t& nIndex) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": nullptr NYM_ID passed in!\n";
        return "";
    }
    OTPasswordData thePWData(OT_PW_DISPLAY);
    Identifier nym_id(NYM_ID);
    // This tries to get, then tries to load as public, then tries to load as
    // private.
    const Nym* pNym =
        OTAPI()->GetOrLoadNym(nym_id, false, __FUNCTION__, &thePWData);
    if (nullptr == pNym) return "";
    std::string str_return;
    const CredentialSet* pCredential = pNym->GetMasterCredentialByIndex(nIndex);

    if (nullptr != pCredential)
        str_return = pCredential->GetMasterCredID().Get();
    return str_return;
}

std::string OTAPI_Exec::GetNym_MasterCredentialContents(
    const std::string& NYM_ID,
    const std::string& CREDENTIAL_ID) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": nullptr NYM_ID passed in!\n";
        return "";
    }
    OTPasswordData thePWData(OT_PW_DISPLAY);
    Identifier nym_id(NYM_ID);
    // This tries to get, then tries to load as public, then tries to load as
    // private.
    const Nym* pNym =
        OTAPI()->GetOrLoadNym(nym_id, false, __FUNCTION__, &thePWData);
    if (nullptr == pNym) return "";

    auto serialized = pNym->MasterCredentialContents(CREDENTIAL_ID);

    if (serialized) {
        return proto::ProtoAsString(*serialized);
    }

    return "";
}

int32_t OTAPI_Exec::GetNym_RevokedCredCount(const std::string& NYM_ID) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": nullptr NYM_ID passed in!\n";
        return OT_ERROR;
    }
    OTPasswordData thePWData(OT_PW_DISPLAY);
    Identifier nym_id(NYM_ID);
    // This tries to get, then tries to load as public, then tries to load as
    // private.
    const Nym* pNym =
        OTAPI()->GetOrLoadNym(nym_id, false, __FUNCTION__, &thePWData);
    if (nullptr == pNym) return OT_ERROR;
    const int32_t nReturnValue =
        static_cast<int32_t>(pNym->GetRevokedCredentialCount());
    return nReturnValue;
}

std::string OTAPI_Exec::GetNym_RevokedCredID(
    const std::string& NYM_ID,
    const int32_t& nIndex) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": nullptr NYM_ID passed in!\n";
        return "";
    }
    OTPasswordData thePWData(OT_PW_DISPLAY);
    Identifier nym_id(NYM_ID);
    // This tries to get, then tries to load as public, then tries to load as
    // private.
    const Nym* pNym =
        OTAPI()->GetOrLoadNym(nym_id, false, __FUNCTION__, &thePWData);
    if (nullptr == pNym) return "";
    std::string str_return;
    const CredentialSet* pCredential =
        pNym->GetRevokedCredentialByIndex(nIndex);

    if (nullptr != pCredential) {
        str_return = pCredential->GetMasterCredID().Get();
    }
    return str_return;
}

std::string OTAPI_Exec::GetNym_RevokedCredContents(
    const std::string& NYM_ID,
    const std::string& CREDENTIAL_ID) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": nullptr NYM_ID passed in!\n";
        return "";
    }
    OTPasswordData thePWData(OT_PW_DISPLAY);
    Identifier nym_id(NYM_ID);
    // This tries to get, then tries to load as public, then tries to load as
    // private.
    const Nym* pNym =
        OTAPI()->GetOrLoadNym(nym_id, false, __FUNCTION__, &thePWData);
    if (nullptr == pNym) {
        return "";
    }

    auto serialized = pNym->RevokedCredentialContents(CREDENTIAL_ID);

    if (serialized) {
        return proto::ProtoAsString(*serialized);
    }

    return "";
}

int32_t OTAPI_Exec::GetNym_ChildCredentialCount(
    const std::string& NYM_ID,
    const std::string& MASTER_CRED_ID) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": nullptr NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (MASTER_CRED_ID.empty()) {
        otErr << __FUNCTION__ << ": nullptr MASTER_CRED_ID passed in!\n";
        return OT_ERROR;
    }
    OTPasswordData thePWData(OT_PW_DISPLAY);
    Identifier nym_id(NYM_ID);
    // This tries to get, then tries to load as public, then tries to load as
    // private.
    const Nym* pNym =
        OTAPI()->GetOrLoadNym(nym_id, false, __FUNCTION__, &thePWData);
    if (nullptr == pNym) return OT_ERROR;

    return pNym->ChildCredentialCount(MASTER_CRED_ID);
}

std::string OTAPI_Exec::GetNym_ChildCredentialID(
    const std::string& NYM_ID,
    const std::string& MASTER_CRED_ID,
    const int32_t& nIndex) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": nullptr NYM_ID passed in!\n";
        return "";
    }
    if (MASTER_CRED_ID.empty()) {
        otErr << __FUNCTION__ << ": nullptr MASTER_CRED_ID passed in!\n";
        return "";
    }
    OTPasswordData thePWData(OT_PW_DISPLAY);
    Identifier nym_id(NYM_ID);
    // This tries to get, then tries to load as public, then tries to load as
    // private.
    const Nym* pNym =
        OTAPI()->GetOrLoadNym(nym_id, false, __FUNCTION__, &thePWData);
    if (nullptr == pNym) return "";

    return pNym->ChildCredentialID(MASTER_CRED_ID, nIndex);
}

std::string OTAPI_Exec::GetNym_ChildCredentialContents(
    const std::string& NYM_ID,
    const std::string& MASTER_CRED_ID,
    const std::string& SUB_CRED_ID) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": nullptr NYM_ID passed in!\n";
        return "";
    }
    if (MASTER_CRED_ID.empty()) {
        otErr << __FUNCTION__ << ": nullptr MASTER_CRED_ID passed in!\n";
        return "";
    }
    if (SUB_CRED_ID.empty()) {
        otErr << __FUNCTION__ << ": nullptr SUB_CRED_ID passed in!\n";
        return "";
    }
    OTPasswordData thePWData(OT_PW_DISPLAY);
    Identifier nym_id(NYM_ID);
    // This tries to get, then tries to load as public, then tries to load as
    // private.
    const Nym* pNym =
        OTAPI()->GetOrLoadNym(nym_id, false, __FUNCTION__, &thePWData);
    if (nullptr == pNym) return "";

    auto serialized =
        pNym->ChildCredentialContents(MASTER_CRED_ID, SUB_CRED_ID);

    if (serialized) {
        return proto::ProtoAsString(*serialized);
    }

    return "";
}

bool OTAPI_Exec::RevokeChildCredential(
    const std::string& NYM_ID,
    const std::string& MASTER_CRED_ID,
    const std::string& SUB_CRED_ID) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": nullptr NYM_ID passed in!\n";
        return false;
    }
    if (MASTER_CRED_ID.empty()) {
        otErr << __FUNCTION__ << ": nullptr MASTER_CRED_ID passed in!\n";
        return false;
    }
    if (SUB_CRED_ID.empty()) {
        otErr << __FUNCTION__ << ": nullptr SUB_CRED_ID passed in!\n";
        return false;
    }
    OTPasswordData thePWData(OT_PW_DISPLAY);
    Identifier nym_id(NYM_ID);
    // This tries to get, then tries to load as public, then tries to load as
    // private.
    Nym* pNym =
        OTAPI()->GetOrLoadPrivateNym(nym_id, false, __FUNCTION__, &thePWData);
    if (nullptr == pNym) return false;
    const String strCredID(MASTER_CRED_ID);

    otErr << "\n\n\nOTAPI_Wrap::" << __FUNCTION__
          << ": TODO: REVOKING IS NOT YET CODED. ADD FUNCTION CALL "
          << "HERE TO REVOKE SUB-CREDENTIAL!\n\n\n";

    /* Revokation is not implemented yet
    CredentialSet* pCredential = pNym->GetMasterCredential(strCredID);

    if (nullptr == pCredential)
        otOut << __FUNCTION__ << ": Sorry, (Nym " << NYM_ID
              << ") no master credential found with the ID : " << strCredID
              << "\n";
    else // Found the master credential...
    {
        const String strSubID(SUB_CRED_ID);
        const Credential* pSub = pCredential->GetChildCredential(strSubID);

        if (nullptr == pSub)
            otOut << __FUNCTION__ << ": Found master credential (" << strCredID
                  << "), but unable to "
                     "find child credential with ID: " << strSubID << "\n";
        else {

            // TODO: Okay we found master AND child credential. Now let's revoke
            // it...
            //

            otErr << "\n\n\nOTAPI_Wrap::" << __FUNCTION__
                  << ": TODO: REVOKING IS NOT YET CODED. ADD FUNCTION CALL "
                     "HERE TO REVOKE SUB-CREDENTIAL!\n\n\n";

//          return true;
        }
    }*/
    return false;
}

std::string OTAPI_Exec::GetClaims(
    const std::string& NYM_ID,
    const std::string& lang) const
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return {};
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": nullptr NYM_ID passed in!\n";
        return {};
    }
    opentxs::Identifier nymID(NYM_ID);
    OTPasswordData thePWData(OT_PW_DISPLAY);
    const Nym* pNym =
        OTAPI()->GetOrLoadNym(nymID, false, __FUNCTION__, &thePWData);
    if (nullptr == pNym) return {};
    // ------------------------------
    auto claims = App::Me().Identity().Claims(*pNym);

    if (!claims) {
        return "";
    }

    std::ostringstream output;
    output << "Contact credential for nym: " << String(nymID) << std::endl;

    for (auto& section : claims->section()) {
        const auto name =
            App::Me().Identity().ContactSectionName(section.name(), lang);
        output << "--Section: " << name << std::endl;

        for (auto& item : section.item()) {
            const auto type =
                App::Me().Identity().ContactTypeName(item.type(), lang);
            output << "  Type: " << type;
            output << " Start: " << item.start();
            output << " End: " << item.end();
            output << " Value: " << item.value();

            for (auto& attribute : item.attribute()) {
                output << " "
                       << App::Me().Identity().ContactAttributeName(
                              static_cast<proto::ContactItemAttribute>(
                                  attribute),
                              lang);
            }
            output << std::endl;
        }
    }

    return output.str();
}

std::string OTAPI_Exec::GetContactData(const std::string& NYM_ID) const
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return {};
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": nullptr NYM_ID passed in!\n";
        return {};
    }
    opentxs::Identifier nymID(NYM_ID);
    OTPasswordData thePWData(OT_PW_DISPLAY);
    const Nym* pNym =
        OTAPI()->GetOrLoadNym(nymID, false, __FUNCTION__, &thePWData);
    if (nullptr == pNym) return {};
    // ------------------------------
    auto claims = App::Me().Identity().Claims(*pNym);

    if (!claims) {
        return "";
    }

    return proto::ProtoAsString(*claims);
}

bool OTAPI_Exec::SetContactData(
    const std::string& NYM_ID,
    const std::string& THE_DATA) const
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return false;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": nullptr NYM_ID passed in!\n";
        return false;
    }
    if (THE_DATA.empty()) {
        otErr << __FUNCTION__ << ": nullptr THE_DATA passed in!\n";
        return false;
    }
    opentxs::Identifier nymID(NYM_ID);
    Nym* pNym = OTAPI()->GetOrLoadPrivateNym(nymID, false, __FUNCTION__);
    if (nullptr == pNym) return false;
    // ------------------------------
    auto contactData =
        proto::StringToProto<proto::ContactData>(String(THE_DATA));
    // ------------------------------
    return pNym->SetContactData(contactData);
}

bool OTAPI_Exec::SetClaim(
    const std::string& nymID,
    const std::uint32_t& section,
    const std::string& claim) const
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return false;
    }
    if (nymID.empty()) {
        otErr << __FUNCTION__ << ": nullptr nymID passed in!\n";
        return false;
    }
    Nym* pNym =
        OTAPI()->GetOrLoadPrivateNym(Identifier(nymID), false, __FUNCTION__);
    if (nullptr == pNym) return false;
    // ------------------------------
    const auto item = proto::DataToProto<proto::ContactItem>(
        OTData(claim.c_str(), claim.length()));
    std::set<std::uint32_t> attribute;

    for (const auto& it : item.attribute()) {
        attribute.insert(it);
    }

    const Claim input{item.id(),
                      section,
                      item.type(),
                      item.value(),
                      item.start(),
                      item.end(),
                      attribute};

    return App::Me().Identity().AddClaim(*pNym, input);
}

bool OTAPI_Exec::DeleteClaim(
    const std::string& nymID,
    const std::string& claimID) const
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return false;
    }
    if (nymID.empty()) {
        otErr << __FUNCTION__ << ": nullptr nymID passed in!\n";
        return false;
    }
    Nym* pNym =
        OTAPI()->GetOrLoadPrivateNym(Identifier(nymID), false, __FUNCTION__);
    if (nullptr == pNym) return false;
    // ------------------------------
    return App::Me().Identity().DeleteClaim(*pNym, claimID);
}

std::string OTAPI_Exec::GetVerificationSet(const std::string& nymID) const
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return {};
    }
    if (nymID.empty()) {
        otErr << __FUNCTION__ << ": empty nymID passed in!\n";
        return {};
    }
    const Nym* pNym =
        OTAPI()->GetOrLoadNym(Identifier(nymID), false, __FUNCTION__);
    if (nullptr == pNym) return {};
    // ------------------------------
    auto verifications = App::Me().Identity().Verifications(*pNym);

    if (verifications) {
        return proto::ProtoAsString(*verifications);
    }

    return "";
}

std::string OTAPI_Exec::SetVerification(
    bool& changed,
    const std::string& onNym,
    const std::string& claimantNymID,
    const std::string& claimID,
    const ClaimPolarity polarity,
    const int64_t start,
    const int64_t end) const
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return {};
    }
    if (onNym.empty()) {
        otErr << __FUNCTION__ << ": empty onNym passed in!\n";
        return {};
    }
    opentxs::Nym* pNym =
        OTAPI()->GetOrLoadPrivateNym(Identifier(onNym), false, __FUNCTION__);
    if (nullptr == pNym) return "";
    // ------------------------------
    auto verifications = App::Me().Identity().Verify(
        *pNym, changed, claimantNymID, claimID, polarity, start, end);

    if (verifications) {
        return proto::ProtoAsString(*verifications);
    }

    return "";
}

std::string OTAPI_Exec::GetSignerNymID(const std::string& str_Contract) const
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return "";
    }
    if (str_Contract.empty()) {
        otErr << __FUNCTION__ << ": Null: str_Contract passed in!\n";
        return "";
    }
    std::string str_Trim(str_Contract);
    std::string str_Trim2 = String::trim(str_Trim);
    String strContract(str_Trim2.c_str());

    if (strContract.GetLength() < 2) {
        otOut << __FUNCTION__ << ": Empty contract passed in!\n";
        return "";
    }
    //-----------------------------------
    Contract* pContract = ::InstantiateContract(strContract);

    if (nullptr == pContract) {
        otErr << __FUNCTION__ << ": Failed trying to instantiate contract.\n";
        OT_ASSERT(false);
        return "";
    }
    //-----------------------------------
    // MUST delete pContract below this point, before returning.
    //
    const Nym* pNym = pContract->GetContractPublicNym();

    if (nullptr == pNym) {
        otErr << __FUNCTION__
              << ": Failed trying to retrieve signer nym from contract.\n";
        delete pContract;
        return "";
    }
    //-----------------------------------
    String strContractNymID;
    pNym->GetIdentifier(strContractNymID);
    std::string pBuf = strContractNymID.Get();
    delete pContract;
    return pBuf;
}

std::string OTAPI_Exec::CalculateContractID(
    const std::string& str_Contract) const
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return "";
    }
    if (str_Contract.empty()) {
        otErr << __FUNCTION__ << ": Null: str_Contract passed in!\n";
        return "";
    }
    std::string str_Trim(str_Contract);
    std::string str_Trim2 = String::trim(str_Trim);
    String strContract(str_Trim2.c_str());

    if (strContract.GetLength() < 2) {
        otOut << __FUNCTION__ << ": Empty contract passed in!\n";
        return "";
    }

    Contract* pContract = ::InstantiateContract(strContract);
    std::unique_ptr<Contract> theAngel(pContract);

    if (nullptr != pContract) {
        Identifier idOutput;
        pContract->CalculateContractID(idOutput);
        const String strOutput(idOutput);
        std::string pBuf = strOutput.Get();
        return pBuf;
    }
    else {
        // ----------------------------------------------------------
        {
            auto serialized =
                proto::StringToProto<proto::ServerContract>(strContract);

            auto id(serialized.id());

            if (id.size() > 0)
                return id;
        }
        // ----------------------------------------------------------
        {
            auto serialized =
                proto::StringToProto<proto::UnitDefinition>(strContract);

            auto id(serialized.id());

            if (id.size() > 0)
                return id;
        }
        // ----------------------------------------------------------
    }
    return "";
}

std::string OTAPI_Exec::CreateCurrencyContract(
    const std::string& NYM_ID,
    const std::string& shortname,
    const std::string& terms,
    const std::string& name,
    const std::string& symbol,
    const std::string& tla,
    const uint32_t power,
    const std::string& fraction) const
{
    std::string output = "";

    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return output;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return output;
    }
    if (terms.empty()) {
        otErr << __FUNCTION__ << ": Null: terms passed in!\n";
        return output;
    }
    if (name.empty()) {
        otErr << __FUNCTION__ << ": Null: name passed in!\n";
        return output;
    }
    if (symbol.empty()) {
        otErr << __FUNCTION__ << ": Null: symbol passed in!\n";
        return output;
    }

    auto pContract = App::Me().Contract().UnitDefinition(
        NYM_ID, shortname, name, symbol, terms, tla, power, fraction);

    if (pContract) {
        output = String(pContract->ID()).Get();
    } else {
        otErr << __FUNCTION__ << ": Failed to create currency contract."
              << std::endl;
    }

    return output;
}

std::string OTAPI_Exec::CreateSecurityContract(
    const std::string& NYM_ID,
    const std::string& shortname,
    const std::string& terms,
    const std::string& name,
    const std::string& symbol) const
{
    std::string output = "";

    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return output;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return output;
    }
    if (terms.empty()) {
        otErr << __FUNCTION__ << ": Null: terms passed in!\n";
        return output;
    }
    if (name.empty()) {
        otErr << __FUNCTION__ << ": Null: name passed in!\n";
        return output;
    }
    if (symbol.empty()) {
        otErr << __FUNCTION__ << ": Null: symbol passed in!\n";
        return output;
    }

    auto pContract = App::Me().Contract().UnitDefinition(
        NYM_ID, shortname, name, symbol, terms);

    if (pContract) {
        output = String(pContract->ID()).Get();
    } else {
        otErr << __FUNCTION__ << ": Failed to create currency contract."
              << std::endl;
    }

    return output;
}

// Use these below functions to get the new contract ITSELF, using its ID
// that was returned by the above two functions:
//
// std::string OTAPI_Exec::GetServer_Contract(const std::string& NOTARY_ID); //
// Return's Server's contract (based on server ID)
// std::string OTAPI_Exec::GetAssetType_Contract(const std::string&
// INSTRUMENT_DEFINITION_ID); // Returns currency contract based on Asset Type
// ID

std::string OTAPI_Exec::GetServer_Contract(const std::string& NOTARY_ID)
    const  // Return's Server's contract (based on server
           // ID)
{
    bool bIsInitialized = OTAPI()->IsInitialized();

    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return "";
    }
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }

    const Identifier theNotaryID(NOTARY_ID);
    auto pServer = OTAPI()->GetServer(theNotaryID, __FUNCTION__);
    if (!pServer) return "";
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTData serialized = pServer->Serialize();
    OTASCIIArmor armored(serialized);
    String strOutput;
    armored.WriteArmoredString(strOutput, "SERVER CONTRACT");

    return strOutput.Get();
}

int32_t OTAPI_Exec::GetCurrencyDecimalPower(
    const std::string& INSTRUMENT_DEFINITION_ID) const
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return -1;
    }
    if (INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Empty INSTRUMENT_DEFINITION_ID passed in!\n";
        return -1;
    }
    auto unit = App::Me().Contract().UnitDefinition(
        Identifier(INSTRUMENT_DEFINITION_ID));

    if (!unit) return -1;

    return unit->DecimalPower();
}

std::string OTAPI_Exec::GetCurrencyTLA(
    const std::string& INSTRUMENT_DEFINITION_ID) const
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return "";
    }
    if (INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Empty INSTRUMENT_DEFINITION_ID passed in!\n";
        return "";
    }
    auto unit = App::Me().Contract().UnitDefinition(
        Identifier(INSTRUMENT_DEFINITION_ID));

    if (!unit) return "";

    return unit->TLA();
}

std::string OTAPI_Exec::GetCurrencySymbol(
    const std::string& INSTRUMENT_DEFINITION_ID) const
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return "";
    }
    if (INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Empty INSTRUMENT_DEFINITION_ID passed in!\n";
        return "";
    }
    const Identifier theInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID);
    auto pContract =
        OTAPI()->GetAssetType(theInstrumentDefinitionID, __FUNCTION__);
    if (!pContract) return "";
    // By this point, pContract is a good pointer.  (No need to cleanup.)
    return pContract->GetCurrencySymbol();
}

// Returns amount from formatted string, based on currency contract and locale.
//
int64_t OTAPI_Exec::StringToAmount(
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& str_input) const
{
    const std::string str_thousand(OT_THOUSANDS_SEP);
    const std::string str_decimal(OT_DECIMAL_POINT);

    return StringToAmountLocale(
        INSTRUMENT_DEFINITION_ID, str_input, str_thousand, str_decimal);
}

int64_t OTAPI_Exec::StringToAmountLocale(
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& str_input,
    const std::string& THOUSANDS_SEP,
    const std::string& DECIMAL_POINT) const
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return OT_ERROR_AMOUNT;
    }
    if (INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Empty INSTRUMENT_DEFINITION_ID passed in!\n";
        return OT_ERROR_AMOUNT;
    }
    auto unit = App::Me().Contract().UnitDefinition(
        Identifier(INSTRUMENT_DEFINITION_ID));

    if (!unit) return -1;

    int64_t theResult;
    bool bParsed = unit->StringToAmountLocale(
        theResult, str_input, THOUSANDS_SEP, DECIMAL_POINT);
    return bParsed ? theResult : StringToLong(str_input);
}

// Returns formatted string for output, for a given amount, based on currency
// contract and locale.
//
std::string OTAPI_Exec::FormatAmount(
    const std::string& INSTRUMENT_DEFINITION_ID,
    const int64_t& THE_AMOUNT) const
{
    const std::string str_thousand(OT_THOUSANDS_SEP);
    const std::string str_decimal(OT_DECIMAL_POINT);

    return FormatAmountLocale(
        INSTRUMENT_DEFINITION_ID, THE_AMOUNT, str_thousand, str_decimal);
}

std::string OTAPI_Exec::FormatAmountLocale(
    const std::string& INSTRUMENT_DEFINITION_ID,
    const int64_t& THE_AMOUNT,
    const std::string& THOUSANDS_SEP,
    const std::string& DECIMAL_POINT) const
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return "";
    }
    if (INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Empty INSTRUMENT_DEFINITION_ID passed in!\n";
        return "";
    }

    auto unit = App::Me().Contract().UnitDefinition(
        Identifier(INSTRUMENT_DEFINITION_ID));

    if (!unit) return "";

    // By this point, pContract is a good pointer.  (No need to cleanup.)
    const int64_t lAmount = THE_AMOUNT;
    int64_t theAmount(lAmount);
    String strBackup(LongToString(THE_AMOUNT));
    std::string str_result;
    const bool bFormatted =
        unit->FormatAmountLocale( // Convert 545 to $5.45.
            theAmount, str_result, THOUSANDS_SEP, DECIMAL_POINT);
    const String strOutput(bFormatted ? str_result.c_str() : strBackup.Get());

    return (bFormatted ? str_result : strBackup.Get());
}

// Returns formatted string for output, for a given amount, based on currency
// contract and locale.
std::string OTAPI_Exec::FormatAmountWithoutSymbol(
    const std::string& INSTRUMENT_DEFINITION_ID,
    const int64_t& THE_AMOUNT) const
{
    const std::string str_thousand(OT_THOUSANDS_SEP);
    const std::string str_decimal(OT_DECIMAL_POINT);

    return FormatAmountWithoutSymbolLocale(
        INSTRUMENT_DEFINITION_ID, THE_AMOUNT, str_thousand, str_decimal);
}

std::string OTAPI_Exec::FormatAmountWithoutSymbolLocale(
    const std::string& INSTRUMENT_DEFINITION_ID,
    const int64_t& THE_AMOUNT,
    const std::string& THOUSANDS_SEP,
    const std::string& DECIMAL_POINT) const
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        Log::vError(
            "%s: Not initialized; call OT_API::Init first.\n", __FUNCTION__);
        OT_FAIL;
    }
    if (INSTRUMENT_DEFINITION_ID.empty()) {
        Log::vError(
            "%s: Empty %s passed in!\n",
            __FUNCTION__,
            "INSTRUMENT_DEFINITION_ID");
        OT_FAIL;
    }

    auto unit = App::Me().Contract().UnitDefinition(
        Identifier(INSTRUMENT_DEFINITION_ID));

    if (!unit) return "";

    String strBackup(LongToString(THE_AMOUNT));
    std::string str_result;  // Convert 545 to $5.45.
    const bool bFormatted = unit->FormatAmountWithoutSymbolLocale(
        THE_AMOUNT, str_result, THOUSANDS_SEP, DECIMAL_POINT);

    return (bFormatted ? str_result : strBackup.Get());
}

std::string OTAPI_Exec::GetAssetType_Contract(
    const std::string& INSTRUMENT_DEFINITION_ID) const  // Returns currency
                                                        // contract based on
// Instrument Definition ID
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return "";
    }

    if (INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Null: INSTRUMENT_DEFINITION_ID passed in!\n";
        return "";
    }

    const Identifier theInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID);
    auto pContract =
        OTAPI()->GetAssetType(theInstrumentDefinitionID, __FUNCTION__);
    if (!pContract) return "";
    // By this point, pContract is a good pointer.  (No need to cleanup.)

    return proto::ProtoAsArmored<proto::UnitDefinition>(
               pContract->PublicContract(), "UNIT DEFINITION")
        .Get();
}

// If you have a server contract that you'd like to add
// to your wallet, call this function.
//
std::string OTAPI_Exec::AddServerContract(const std::string& strContract) const
{
    if (strContract.empty()) {
        otErr << __FUNCTION__ << ": Null: strContract passed in!\n";
    } else {
        auto serialized =
            proto::StringToProto<proto::ServerContract>(String(strContract));
        auto contract = App::Me().Contract().Server(serialized);

        if (contract) {
            String id(contract->ID());

            return id.Get();
        }
    }

    return "";
}

// If you have an asset contract that you'd like to add
// to your wallet, call this function.
//
std::string OTAPI_Exec::AddUnitDefinition(const std::string& strContract) const
{
    if (strContract.empty()) {
        otErr << __FUNCTION__ << ": Null: strContract passed in!\n";
    } else {
        auto serialized =
            proto::StringToProto<proto::UnitDefinition>(String(strContract));
        auto contract = App::Me().Contract().UnitDefinition(serialized);

        if (contract) {
            String id(contract->ID());

            return id.Get();
        }
    }

    return "";
}

int32_t OTAPI_Exec::GetNymCount(void) const { return OTAPI()->GetNymCount(); }

int32_t OTAPI_Exec::GetServerCount(void) const
{
    const auto servers = App::Me().Contract().ServerList();
    return servers.size();
}

int32_t OTAPI_Exec::GetAssetTypeCount(void) const
{
    const auto units = App::Me().Contract().UnitDefinitionList();
    return units.size();
}

int32_t OTAPI_Exec::GetAccountCount(void) const
{
    return OTAPI()->GetAccountCount();
}

// *** FUNCTIONS FOR REMOVING VARIOUS CONTRACTS AND NYMS FROM THE WALLET ***

// Can I remove this server contract from my wallet?
//
// You cannot remove the server contract from your wallet if there are accounts
// (or nyms) in there using it.
// This function tells you whether you can remove the server contract or not.
// (Whether there are accounts or nyms using it...)
//
// returns bool
//
bool OTAPI_Exec::Wallet_CanRemoveServer(const std::string& NOTARY_ID) const
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return false;
    }
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return false;
    }
    Identifier theID(NOTARY_ID);
    const int32_t& nCount = OTAPI_Exec::GetAccountCount();

    // Loop through all the accounts.
    for (int32_t i = 0; i < nCount; i++) {
        std::string pAcctID = OTAPI_Exec::GetAccountWallet_ID(i);
        String strAcctID(pAcctID);

        std::string pID =
            OTAPI_Exec::GetAccountWallet_NotaryID(strAcctID.Get());
        Identifier theCompareID(pID);

        if (theID == theCompareID) {
            otOut << __FUNCTION__ << ": Unable to remove server contract "
                  << NOTARY_ID << " from "
                                  "wallet, because Account "
                  << strAcctID << " uses it.\n";
            return false;
        }
    }
    const int32_t& nNymCount = OTAPI_Exec::GetNymCount();

    // Loop through all the Nyms. (One might be registered on that server.)
    //
    for (int32_t i = 0; i < nNymCount; i++) {
        std::string pNymID = OTAPI_Exec::GetNym_ID(i);
        String strNymID(pNymID);

        if (true ==
            OTAPI_Exec::IsNym_RegisteredAtServer(strNymID.Get(), NOTARY_ID)) {
            otOut << __FUNCTION__ << ": Unable to remove server contract "
                  << NOTARY_ID << " from "
                                  "wallet, because Nym "
                  << strNymID
                  << " is registered there. (Delete that first...)\n";
            return false;
        }
    }
    return true;
}

// Remove this server contract from my wallet!
//
// Try to remove the server contract from the wallet.
// This will not work if there are any accounts in the wallet for the same
// server ID.
// returns bool
//
bool OTAPI_Exec::Wallet_RemoveServer(const std::string& NOTARY_ID) const
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return false;
    }

    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return false;
    }

    // Make sure there aren't any dependent accounts..
    if (!OTAPI_Exec::Wallet_CanRemoveServer(NOTARY_ID)) return false;

    // TODO: the above call proves that there are no accounts laying around
    // for this server ID. (No need to worry about "orphaned accounts.")
    //
    // However, there may still be Nyms registered at the server! Therefore,
    // we need to loop through the Nyms, and make sure none of them has been
    // registered at this server ID. If it has, then we need to message the
    // server
    // to "deregister" the Nym, which is much cleaner.  Otherwise server's only
    // other alternative is to expire Nyms that have gone unused for some
    // specific
    // period of time, presumably those terms are described in the server
    // contract.
    //
    Identifier theID(NOTARY_ID);

    if (App::Me().Contract().RemoveServer(theID)) {
        otOut << __FUNCTION__
              << ": Removed server contract from the wallet: " << NOTARY_ID
              << "\n";
        return true;
    }
    return false;
}

// Can I remove this asset contract from my wallet?
//
// You cannot remove the asset contract from your wallet if there are accounts
// in there using it.
// This function tells you whether you can remove the asset contract or not.
// (Whether there are accounts...)
// returns bool
//
bool OTAPI_Exec::Wallet_CanRemoveAssetType(
    const std::string& INSTRUMENT_DEFINITION_ID) const
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return false;
    }

    if (INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Null: INSTRUMENT_DEFINITION_ID passed in!\n";
        return false;
    }

    Identifier theID(INSTRUMENT_DEFINITION_ID);
    const int32_t& nCount = OTAPI_Exec::GetAccountCount();

    // Loop through all the accounts.
    for (int32_t i = 0; i < nCount; i++) {
        std::string pAcctID = OTAPI_Exec::GetAccountWallet_ID(i);
        String strAcctID(pAcctID);

        std::string pID = OTAPI_Exec::GetAccountWallet_InstrumentDefinitionID(
            strAcctID.Get());
        Identifier theCompareID(pID);

        if (theID == theCompareID) {
            otOut << __FUNCTION__ << ": Unable to remove asset contract "
                  << INSTRUMENT_DEFINITION_ID << " from "
                                                 "wallet: Account "
                  << strAcctID << " uses it.\n";
            return false;
        }
    }
    return true;
}

// Remove this asset contract from my wallet!
//
// Try to remove the asset contract from the wallet.
// This will not work if there are any accounts in the wallet for the same asset
// type ID.
// returns bool
//
bool OTAPI_Exec::Wallet_RemoveAssetType(
    const std::string& INSTRUMENT_DEFINITION_ID) const
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return false;
    }

    if (INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Null: INSTRUMENT_DEFINITION_ID passed in!\n";
        return false;
    }

    // Make sure there aren't any dependent accounts..
    if (!OTAPI_Exec::Wallet_CanRemoveAssetType(INSTRUMENT_DEFINITION_ID))
        return false;

    Identifier theID(INSTRUMENT_DEFINITION_ID);

    if (App::Me().Contract().RemoveUnitDefinition(theID)) {
        otOut << __FUNCTION__
              << ": Removed unit definition contract from the wallet: "
              << INSTRUMENT_DEFINITION_ID << "\n";
        return true;
    }
    return false;
}

// Can I remove this Nym from my wallet?
//
// You cannot remove the Nym from your wallet if there are accounts in there
// using it.
// This function tells you whether you can remove the Nym or not. (Whether there
// are accounts...)
// It also checks to see if the Nym in question is registered at any servers.
//
// returns bool
//
bool OTAPI_Exec::Wallet_CanRemoveNym(const std::string& NYM_ID) const
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return false;
    }

    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return false;
    }

    Identifier theID(NYM_ID);
    Nym* pNym = OTAPI()->GetNym(theID, __FUNCTION__);
    if (nullptr == pNym) return false;
    // Make sure the Nym doesn't have any accounts in the wallet.
    // (Client must close those before calling this.)
    //
    const int32_t& nCount = OTAPI_Exec::GetAccountCount();

    // Loop through all the accounts.
    for (int32_t i = 0; i < nCount; i++) {
        std::string pAcctID = OTAPI_Exec::GetAccountWallet_ID(i);
        String strAcctID(pAcctID);

        std::string pID = OTAPI_Exec::GetAccountWallet_NymID(strAcctID.Get());

        if (pID.empty()) {
            otErr << __FUNCTION__ << ": Bug in OTAPI_Exec::Wallet_CanRemoveNym "
                                     "/ OTAPI_Exec::GetAccountWallet_NymID\n";
            return false;
        }

        Identifier theCompareID(pID);

        // Looks like the Nym still has some accounts in this wallet.
        if (theID == theCompareID) {
            otOut << __FUNCTION__ << ": Nym cannot be removed because there "
                                     "are still accounts in the wallet for "
                                     "that Nym.\n";
            return false;
        }
    }

    // Make sure the Nym isn't registered at any servers...
    // (Client must unregister at those servers before calling this function..)
    //
    const int32_t& nServerCount = OTAPI_Exec::GetServerCount();

    for (int32_t i = 0; i < nServerCount; i++) {
        std::string str_NotaryID = OTAPI_Exec::GetServer_ID(i);

        if ("" != str_NotaryID) {
            const String strNotaryID(str_NotaryID);

            if (pNym->IsRegisteredAtServer(strNotaryID)) {
                otOut << __FUNCTION__ << ": Nym cannot be removed because "
                                         "there are still servers in the "
                                         "wallet that the Nym is registered "
                                         "at.\n";
                return false;
            }
        }
    }

    // TODO:  Make sure Nym doesn't have any cash in any purses...

    return true;
}

// Remove this Nym from my wallet!
//
// Try to remove the Nym from the wallet.
// This will fail if the Nym is registered at any servers, or has any accounts.
//
// returns bool
//
bool OTAPI_Exec::Wallet_RemoveNym(const std::string& NYM_ID) const
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return false;
    }

    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return false;
    }

    // DONE: The below call proves already that there are no accounts laying
    // around
    // for this Nym. (No need to worry about "orphaned accounts.")
    //
    // DONE (finally):
    // However, the Nym might still be registered at various servers, even
    // without asset accounts.
    // Therefore, we need to iterate through the server contracts, and if the
    // Nym is registered at
    // any of the servers, then "deregister" (before deleting the Nym entirely.)
    // This is much
    // cleaner for the server side, who otherwise has to expired unused nyms
    // based on some rule
    // presumably to be found in the server contract.
    if (!OTAPI_Exec::Wallet_CanRemoveNym(NYM_ID)) return false;

    OTWallet* pWallet = OTAPI()->GetWallet(__FUNCTION__);

    if (nullptr == pWallet) {
        otErr << __FUNCTION__ << ": No wallet found...!\n";
        return false;
    }

    Identifier theID(NYM_ID);

    if (pWallet->RemovePrivateNym(theID)) {
        otOut << __FUNCTION__ << ": Success erasing Nym from wallet: " << NYM_ID
              << "\n";
        pWallet->SaveWallet();
        return true;
    } else
        otOut << __FUNCTION__ << ": Failure erasing Nym from wallet: " << NYM_ID
              << "\n";

    return false;
}

// Can I remove this Account from my wallet?
//
// You cannot remove the Account from your wallet if there are transactions
// still open.
// This function tells you whether you can remove the Account or not. (Whether
// there are transactions...)
// Also, balance must be zero to do this.
//
// returns bool
//
bool OTAPI_Exec::Wallet_CanRemoveAccount(const std::string& ACCOUNT_ID) const
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return false;
    }

    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return false;
    }

    const Identifier theAccountID(ACCOUNT_ID);

    Account* pAccount = OTAPI()->GetAccount(theAccountID, __FUNCTION__);
    if (nullptr == pAccount) return false;
    // Balance must be zero in order to close an account!
    else if (pAccount->GetBalance() != 0) {
        otOut << __FUNCTION__ << ": Account balance MUST be zero in order to "
                                 "close an asset account: "
              << ACCOUNT_ID << ".\n";
        return false;
    }
    bool BOOL_RETURN_VALUE = false;

    const Identifier& theNotaryID = pAccount->GetPurportedNotaryID();
    const Identifier& theNymID = pAccount->GetNymID();

    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.
    std::unique_ptr<Ledger> pInbox(
        OTAPI()->LoadInbox(theNotaryID, theNymID, theAccountID));
    std::unique_ptr<Ledger> pOutbox(
        OTAPI()->LoadOutbox(theNotaryID, theNymID, theAccountID));

    if (nullptr == pInbox) {
        otOut << __FUNCTION__
              << ": Failure calling OT_API::LoadInbox.\n Account ID : "
              << ACCOUNT_ID << "\n";
    } else if (nullptr == pOutbox) {
        otOut << __FUNCTION__
              << ": Failure calling OT_API::LoadOutbox.\n Account ID : "
              << ACCOUNT_ID << "\n";
    } else if (
        (pInbox->GetTransactionCount() > 0) ||
        (pOutbox->GetTransactionCount() > 0)) {
        otOut << __FUNCTION__ << ": Failure: You cannot remove an asset "
                                 "account if there are inbox/outbox items "
                                 "still waiting to be processed.\n";
    } else
        BOOL_RETURN_VALUE = true;  // SUCCESS!

    return BOOL_RETURN_VALUE;
}

// So the client side knows which ones he has in storage, vs which ones he
// still needs to download.
//
bool OTAPI_Exec::DoesBoxReceiptExist(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,  // Unused here for now, but still convention.
    const std::string& ACCOUNT_ID,  // If for Nymbox (vs inbox/outbox) then pass
                                    // NYM_ID in this field also.
    const int32_t& nBoxType,        // 0/nymbox, 1/inbox, 2/outbox
    const int64_t& TRANSACTION_NUMBER) const
{

    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return false;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return false;
    }
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return false;
    }
    if (!((0 == nBoxType) || (1 == nBoxType) || (2 == nBoxType))) {
        otErr << __FUNCTION__
              << ": nBoxType is of wrong type: value: " << nBoxType << "\n";
        return false;
    }
    if (0 > TRANSACTION_NUMBER) {
        otErr << __FUNCTION__ << ": Negative: TRANSACTION_NUMBER passed in!\n";
        return false;
    }
    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theAccountID(ACCOUNT_ID);
    const int64_t lTransactionNum = TRANSACTION_NUMBER;
    switch (nBoxType) {
        case 0:  // nymbox
        case 1:  // inbox
        case 2:  // outbox
            break;
        default:
            otErr << __FUNCTION__ << ": Error: bad nBoxType: " << nBoxType
                  << " for NymID: " << NYM_ID << " AcctID: " << ACCOUNT_ID
                  << " (expected one of: 0/nymbox, 1/inbox, 2/outbox)\n";
            return false;
    }
    return OTAPI()->DoesBoxReceiptExist(
        theNotaryID,
        theNymID,
        theAccountID,  // If for Nymbox (vs inbox/outbox)
                       // then pass NYM_ID in this field
                       // also.
        nBoxType,      // 0/nymbox, 1/inbox, 2/outbox
        static_cast<int64_t>(lTransactionNum));
}

// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
// >0 means NO error, and the message was sent, and the request number fits into
// an integer...
//  ...and in fact the requestNum IS the return value!
//  ===> In 99% of cases, this LAST option is what actually happens!!
//
int32_t OTAPI_Exec::getBoxReceipt(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,  // If for Nymbox (vs inbox/outbox) then pass
                                    // NYM_ID in this field also.
    const int32_t& nBoxType,        // 0/nymbox, 1/inbox, 2/outbox
    const int64_t& TRANSACTION_NUMBER) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return OT_ERROR;
    }
    if (!((0 == nBoxType) || (1 == nBoxType) || (2 == nBoxType))) {
        otErr << __FUNCTION__
              << ": nBoxType is of wrong type: value: " << nBoxType << "\n";
        return OT_ERROR;
    }
    if (0 > TRANSACTION_NUMBER) {
        otErr << __FUNCTION__ << ": Negative: TRANSACTION_NUMBER passed in!\n";
        return OT_ERROR;
    }
    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theAccountID(ACCOUNT_ID);
    const int64_t lTransactionNum = TRANSACTION_NUMBER;
    switch (nBoxType) {
        case 0:  // nymbox
        case 1:  // inbox
        case 2:  // outbox
            break;
        default:
            otErr << __FUNCTION__ << ": Error: bad nBoxType: " << nBoxType
                  << " for NymID: " << NYM_ID << " AcctID: " << ACCOUNT_ID
                  << "(expected one of: 0/nymbox, 1/inbox, 2/outbox)\n";
            return OT_ERROR;
    }

    return OTAPI()->getBoxReceipt(
        theNotaryID,
        theNymID,
        theAccountID,  // If for Nymbox (vs
                       // inbox/outbox) then pass
                       // NYM_ID in this field also.
        nBoxType,      // 0/nymbox, 1/inbox, 2/outbox
        static_cast<int64_t>(lTransactionNum));
}

// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
// >0 means NO error, and the message was sent, and the request number fits into
// an integer...
//  ...and in fact the requestNum IS the return value!
//  ===> In 99% of cases, this LAST option is what actually happens!!
//
int32_t OTAPI_Exec::deleteAssetAccount(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return OT_ERROR;
    }

    if (!OTAPI_Exec::Wallet_CanRemoveAccount(ACCOUNT_ID)) return 0;

    Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theAccountID(ACCOUNT_ID);

    return OTAPI()->deleteAssetAccount(theNotaryID, theNymID, theAccountID);
}

// OT has the capability to export a Nym (normally stored in several files) as
// an encoded
// object (in base64-encoded form) and then import it again.
// Returns: Exported Nym in String Form.
//
std::string OTAPI_Exec::Wallet_ExportNym(const std::string& NYM_ID) const
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return "";
    }

    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }

    const Identifier theNymID(NYM_ID);

    // Create a StringMap object with these values:
    //
    // id:      The NymID.
    // name:    The display name from the wallet.
    // cert:    The public / private certfile in openssl format.
    // nymfile: The contents of the nymfile.
    //
    // Make sure to use master key when accessing them, but then put that on
    // pause while saving them to string. (Then unpause again.)
    //
    // Then Encode the StringMap into packed string form, and return it
    // from this function (or "".)
    //
    String strOutput;

    const bool& bExported = OTAPI()->Wallet_ExportNym(theNymID, strOutput);

    if (bExported) {
        std::string pBuf = strOutput.Get();

        return pBuf;
    }

    return "";
}

// OT has the capability to export a Nym (normally stored in several files) as
// an encoded
// object (in base64-encoded form) and then import it again.
//
// Returns: Nym ID of newly-imported Nym (or "".)
//
std::string OTAPI_Exec::Wallet_ImportNym(const std::string& FILE_CONTENTS) const
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return "";
    }

    if (FILE_CONTENTS.empty()) {
        otErr << __FUNCTION__ << ": Null: FILE_CONTENTS passed in!\n";
        return "";
    }

    // Pause the master key, since this Nym is coming from outside
    // the wallet.
    //
    const String strFileContents(FILE_CONTENTS);

    Identifier theNymID;

    const bool& bImported =
        OTAPI()->Wallet_ImportNym(strFileContents, &theNymID);

    // Decode the FILE_CONTENTS into a StringMap object,
    // and if success, make sure it contains these values:
    //
    // id:      The NymID.
    // name:    The display name from the wallet.
    // cert:    The public / private certfile in openssl format.
    // nymfile: The contents of the nymfile.
    //
    // Unpause the master key.
    //
    // Do various verifications on the values to make sure there's no funny
    // business.
    //
    // Make sure the Nym with this ID isn't ALREADY in the wallet. If not, then
    // add it.
    //
    //
    //

    if (bImported) {
        const String strNymID(theNymID);

        std::string pBuf = strNymID.Get();

        return pBuf;
    }

    return "";
}

/*
CHANGE MASTER KEY and PASSWORD.

Normally your passphrase is used to derive a key, which is used to unlock
a random number (a symmetric key), which is used as the passphrase to open the
master key, which is used as the passphrase to any given Nym.

Since all the Nyms are encrypted to the master key, and since we can change the
passphrase on the master key without changing the master key itself, then we
don't
have to do anything to update all the Nyms, since that part hasn't changed.

But we might want a separate "Change Master Key" function that replaces that key
itself, in which case we'd HAVE to load up all the Nyms and re-save them.

UPDATE: Seems the easiest thing to do is to just change both the key and
passphase
at the same time here, by loading up all the private nyms, destroying the master
key,
and then saving all the private Nyms. (With master key never actually being
"paused.")
This will automatically cause it to generate a new master key during the saving
process.
(Make sure to save the wallet also.)
*/
bool OTAPI_Exec::Wallet_ChangePassphrase() const
{
    return OTAPI()->Wallet_ChangePassphrase();
}

// bool OTPseudonym::Savex509CertAndPrivateKeyToString(OTString& strOutput,
// OTString * pstrReason)

// bool OTPseudonym::Savex509CertAndPrivateKey(bool       bCreateFile,
//                                            OTString * pstrReason)

// Attempts to find a full ID in the wallet, based on a partial of the same ID.
// Returns "" on failure, otherwise returns the full ID.
//
std::string OTAPI_Exec::Wallet_GetNymIDFromPartial(
    const std::string& PARTIAL_ID) const
{
    //  OTPseudonym *    GetNym(const OTIdentifier& NYM_ID, const std::string&
    // strFuncName="");
    //  OTPseudonym *    GetNymByIDPartialMatch(const std::string&PARTIAL_ID,
    // const std::string& strFuncName="");
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return "";
    }
    if (PARTIAL_ID.empty()) {
        otErr << __FUNCTION__ << ": Empty PARTIAL_ID passed in!\n";
        return "";
    }

    const Nym* pObject = nullptr;

    Identifier thePartialID(PARTIAL_ID);

    // In this case, the user maybe passed in the FULL ID.
    // (We STILL confirm whether he's found in the wallet...)
    //
    if (!thePartialID.empty()) {
        OTPasswordData thePWData(OT_PW_DISPLAY);
        pObject = OTAPI()->GetOrLoadNym(
            thePartialID,
            false,
            __FUNCTION__,
            &thePWData);  // This tries to get, then tries to load as public,
                          // then tries to load as private.
    }

    if (nullptr != pObject)  // Found it (as full ID.)
    {
        String strID_Output(thePartialID);
        std::string pBuf = strID_Output.Get();
        return pBuf;
    }
    // Below this point, it definitely wasn't a FULL ID, at least one that
    // we know about, so now we can go ahead and search for it as a PARTIAL
    // ID...
    //
    pObject = OTAPI()->GetNymByIDPartialMatch(
        PARTIAL_ID, "OTAPI_Exec::Wallet_GetNymIDFromPartial");

    if (nullptr != pObject)  // Found it (as partial ID.)
    {
        String strID_Output;
        pObject->GetIdentifier(strID_Output);
        std::string pBuf = strID_Output.Get();
        return pBuf;
    }

    return "";
}

// Attempts to find a full ID in the wallet, based on a partial of the same ID.
// Returns "" on failure, otherwise returns the full ID.
//
std::string OTAPI_Exec::Wallet_GetNotaryIDFromPartial(
    const std::string& PARTIAL_ID) const
{
    if (PARTIAL_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: PARTIAL_ID passed in!\n";
        return "";
    }

    const auto servers = App::Me().Contract().ServerList();
    std::string fullID = "";

    // Search as an id
    for (auto& it : servers) {
        if (it.first.compare(0, PARTIAL_ID.length(), PARTIAL_ID) == 0) {
            fullID = it.first;
            break;
        }
    }

    // Search as an alias
    if (fullID.empty()) {
        for (auto& it : servers) {
            if (it.second.compare(0, PARTIAL_ID.length(), PARTIAL_ID) == 0) {
                fullID = it.first;
                break;
            }
        }
    }

    return fullID;
}

// Attempts to find a full ID in the wallet, based on a partial of the same ID.
// Returns "" on failure, otherwise returns the full ID.
//
std::string OTAPI_Exec::Wallet_GetInstrumentDefinitionIDFromPartial(
    const std::string& PARTIAL_ID) const
{
    //    OTUnitDefinition *    GetAssetType(const OTIdentifier& THE_ID, const
    // std::string& strFuncName="");
    //    OTUnitDefinition *    GetUnitDefinitionPartialMatch(const std::string
    // &PARTIAL_ID, const std::string& strFuncName="");

    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return "";
    }

    if (PARTIAL_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: PARTIAL_ID passed in!\n";
        return "";
    }

    Identifier theID(PARTIAL_ID);
    ConstUnitDefinition pUnit;  // shared_ptr to const.

    // See if it's available using the full length ID.
    if (!theID.empty()) pUnit = App::Me().Contract().UnitDefinition(theID);

    if (!pUnit) {
        const auto units = App::Me().Contract().UnitDefinitionList();

        // See if it's available using the partial length ID.
        for (auto& it : units) {
            if (0 == it.first.compare(0, PARTIAL_ID.length(), PARTIAL_ID)) {
                pUnit =
                    App::Me().Contract().UnitDefinition(Identifier(it.first));
                break;
            }
        }
        if (!pUnit) {
            // See if it's available using the full length name.
            for (auto& it : units) {
                if (0 == it.second.compare(0, it.second.length(), PARTIAL_ID)) {
                    pUnit = App::Me().Contract().UnitDefinition(
                        Identifier(it.first));
                    break;
                }
            }

            if (!pUnit) {
                // See if it's available using the partial name.
                for (auto& it : units) {
                    if (0 ==
                        it.second.compare(0, PARTIAL_ID.length(), PARTIAL_ID)) {
                        pUnit = App::Me().Contract().UnitDefinition(
                            Identifier(it.first));
                        break;
                    }
                }
            }
        }
    }

    if (pUnit)  // Found it (as partial ID.)
    {
        return proto::ProtoAsArmored<proto::UnitDefinition>(
                   pUnit->PublicContract(), "UNIT DEFINITION")
            .Get();
    }

    return "";
}

std::string OTAPI_Exec::Wallet_GetAccountIDFromPartial(
    const std::string& PARTIAL_ID) const
{
    //    OTAccount *   GetAccount(const OTIdentifier& THE_ID, const
    // std::string& strFuncName="");
    //    OTAccount *   GetAccountPartialMatch(const std::string&PARTIAL_ID,
    // const std::string& strFuncName="");

    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return "";
    }

    if (PARTIAL_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: PARTIAL_ID passed in!\n";
        return "";
    }

    Account* pObject = nullptr;
    Identifier thePartialID(PARTIAL_ID);

    // In this case, the user passed in the FULL ID.
    // (We STILL confirm whether he's found in the wallet...)
    //
    if (!thePartialID.empty())
        pObject = OTAPI()->GetAccount(
            thePartialID, "OTAPI_Exec::Wallet_GetAccountIDFromPartial");

    if (nullptr != pObject)  // Found it (as full ID.)
    {
        String strID_Output(thePartialID);
        std::string pBuf = strID_Output.Get();
        return pBuf;
    }
    // Below this point, it definitely wasn't a FULL ID, so now we can
    // go ahead and search for it as a PARTIAL ID...
    //
    pObject = OTAPI()->GetAccountPartialMatch(
        PARTIAL_ID, "OTAPI_Exec::Wallet_GetAccountIDFromPartial");

    if (nullptr != pObject)  // Found it (as partial ID.)
    {
        String strID_Output;
        pObject->GetIdentifier(strID_Output);
        std::string pBuf = strID_Output.Get();
        return pBuf;
    }

    return "";
}

/// based on Index this returns the Nym's ID
std::string OTAPI_Exec::GetNym_ID(const int32_t& nIndex) const
{
    if (0 > nIndex) {
        otErr << __FUNCTION__
              << ": nIndex is out of bounds (it's in the negative!)\n";
        return "";
    }

    Identifier theNymID;
    String strName;

    bool bGetNym = OTAPI()->GetNym(nIndex, theNymID, strName);

    if (bGetNym) {
        String strNymID(theNymID);

        std::string pBuf = strNymID.Get();

        return pBuf;
    }

    return "";
}

/// Returns Nym Name (based on NymID)
std::string OTAPI_Exec::GetNym_Name(const std::string& NYM_ID) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null NYM_ID passed in!\n";
        return "";
    }
    Identifier theNymID(NYM_ID);
    Nym* pNym = OTAPI()->GetNym(theNymID);

    if (nullptr != pNym) {

        return pNym->Alias();
    }
    return "";
}

bool OTAPI_Exec::IsNym_RegisteredAtServer(
    const std::string& NYM_ID,
    const std::string& NOTARY_ID) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return false;
    }
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return false;
    }

    const Identifier theNymID(NYM_ID), theNotaryID(NOTARY_ID);

    bool bSuccess = OTAPI()->IsNym_RegisteredAtServer(theNymID, theNotaryID);

    return bSuccess;
}

// Returns Nym data (based on NymID)
//
std::string OTAPI_Exec::GetNym_Stats(const std::string& NYM_ID) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }

    Identifier theNymID(NYM_ID);
    Nym* pNym = OTAPI()->GetNym(theNymID, __FUNCTION__);

    if (nullptr != pNym) {
        String strOutput;

        pNym->DisplayStatistics(strOutput);

        std::string pBuf = strOutput.Get();

        return pBuf;
    }

    return "";
}

// Returns NymboxHash (based on NotaryID)
//
std::string OTAPI_Exec::GetNym_NymboxHash(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const  // Returns NymboxHash (based on NotaryID)
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }

    Identifier theNymID(NYM_ID);
    Nym* pNym = OTAPI()->GetNym(theNymID, __FUNCTION__);

    if (nullptr != pNym) {
        Identifier theNymboxHash;
        const std::string str_notary_id(NOTARY_ID);
        const bool& bGothash = pNym->GetNymboxHash(
            str_notary_id, theNymboxHash);  // (theNymboxHash is output.)

        if (!bGothash) {
            const String strNymID(theNymID);  // You might ask, why create this
                                              // string and not just use
                                              // NYM_ID?
            // The answer is because I'm looking forward to a day soon when we
            // don't passconst std::string& in the first
            // place, and thus I can't always expect that variable will be
            // there.
            //
            otWarn << __FUNCTION__
                   << ": NymboxHash not found, on client side, for server "
                   << str_notary_id << " and nym " << strNymID
                   << ". (Returning .)\n";
        } else  // Success: the hash was there, for that Nym, for that server
                // ID.
        {
            String strOutput(theNymboxHash);

            std::string pBuf = strOutput.Get();

            return pBuf;
        }
    }

    return "";
}

// Returns RecentHash (based on NotaryID)
//
std::string OTAPI_Exec::GetNym_RecentHash(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const  // Returns RecentHash (based on NotaryID)
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }

    Identifier theNymID(NYM_ID);
    Nym* pNym = OTAPI()->GetNym(theNymID, __FUNCTION__);

    if (nullptr != pNym) {
        Identifier theHash;
        const std::string str_notary_id(NOTARY_ID);
        const bool& bGothash = pNym->GetRecentHash(
            str_notary_id, theHash);  // (theHash is output.)

        if (!bGothash) {
            const String strNymID(theNymID);  // You might ask, why create this
                                              // string and not just use
                                              // NYM_ID?
            // The answer is because I'm looking forward to a day soon when we
            // don't passconst std::string& in the first
            // place, and thus I can't always expect that variable will be
            // there.
            //
            otWarn << __FUNCTION__
                   << ": RecentHash not found, on client side, for server "
                   << str_notary_id << " and nym " << strNymID
                   << ". (Returning .)\n";
        } else  // Success: the hash was there, for that Nym, for that server
                // ID.
        {
            String strOutput(theHash);

            std::string pBuf = strOutput.Get();

            return pBuf;
        }
    }

    return "";
}

std::string OTAPI_Exec::GetNym_InboxHash(
    const std::string& ACCOUNT_ID,
    const std::string& NYM_ID) const  // InboxHash for "most recently
                                      // DOWNLOADED" Inbox
                                      // (by AccountID)
{
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACOUNT_ID passed in!\n";
        return "";
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }

    Identifier theNymID(NYM_ID);
    Nym* pNym = OTAPI()->GetNym(theNymID, __FUNCTION__);

    if (nullptr != pNym) {
        Identifier theHash;
        const std::string str_acct_id(ACCOUNT_ID);
        const bool& bGothash =
            pNym->GetInboxHash(str_acct_id, theHash);  // (theHash is output.)

        if (!bGothash) {
            const String strNymID(theNymID);  // You might ask, why create this
                                              // string and not just use
                                              // NYM_ID?
            // The answer is because I'm looking forward to a day soon when we
            // don't passconst std::string& in the first
            // place, and thus I can't always expect that variable will be
            // there.
            //
            otWarn << __FUNCTION__
                   << ": InboxHash not found, on client side, for account "
                   << str_acct_id << " and nym " << strNymID
                   << ". (Returning .)\n";
        } else  // Success: the hash was there, for that Nym, for that server
                // ID.
        {
            String strOutput(theHash);

            std::string pBuf = strOutput.Get();

            return pBuf;
        }
    }

    return "";
}

std::string OTAPI_Exec::GetNym_OutboxHash(
    const std::string& ACCOUNT_ID,
    const std::string& NYM_ID) const  // OutboxHash for "most recently
                                      // DOWNLOADED"
                                      // Outbox (by AccountID)
{
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return "";
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }

    Identifier theNymID(NYM_ID);
    Nym* pNym = OTAPI()->GetNym(theNymID, __FUNCTION__);

    if (nullptr != pNym) {
        Identifier theHash;
        const std::string str_acct_id(ACCOUNT_ID);
        const bool& bGothash =
            pNym->GetOutboxHash(str_acct_id, theHash);  // (theHash is output.)

        if (!bGothash) {
            const String strNymID(theNymID);  // You might ask, why create this
                                              // string and not just use
                                              // NYM_ID?
            // The answer is because I'm looking forward to a day soon when we
            // don't passconst std::string& in the first
            // place, and thus I can't always expect that variable will be
            // there.
            //
            otWarn << __FUNCTION__
                   << ": OutboxHash not found, on client side, for account "
                   << str_acct_id << " and nym " << strNymID
                   << ". (Returning .)\n";
        } else  // Success: the hash was there, for that Nym, for that server
                // ID.
        {
            String strOutput(theHash);

            std::string pBuf = strOutput.Get();

            return pBuf;
        }
    }

    return "";
}

int32_t OTAPI_Exec::GetNym_MailCount(const std::string& NYM_ID) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    Identifier theNymID(NYM_ID);
    Nym* pNym = OTAPI()->GetNym(theNymID, __FUNCTION__);
    if (nullptr == pNym) return OT_ERROR;
    return pNym->GetMailCount();
}

// returns the message, optionally with Subject: as first line.
std::string OTAPI_Exec::GetNym_MailContentsByIndex(
    const std::string& NYM_ID,
    const int32_t& nIndex) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    if (0 > nIndex) {
        otErr << __FUNCTION__
              << ": nIndex is out of bounds (it's in the negative!)\n";
        return "";
    }
    Identifier theNymID(NYM_ID);
    Nym* pNym = OTAPI()->GetNym(theNymID, __FUNCTION__);
    if (nullptr == pNym) return "";
    Message* pMessage = pNym->GetMailByIndex(nIndex);

    if (nullptr != pMessage) {
        // SENDER:    pMessage->m_strNymID
        // RECIPIENT: pMessage->m_strNymID2
        // MESSAGE:   pMessage->m_ascPayload (in an OTEnvelope)

        auto recipientNym =
            App::Me().Contract().Nym(pNym->ID());
        auto senderNym =
            App::Me().Contract().Nym(Identifier(pMessage->m_strNymID));
        auto peerObject = PeerObject::Factory(
            recipientNym,
            senderNym,
            pMessage->m_ascPayload);

        if (!peerObject) { return ""; }

        if (!peerObject->Message()) { return ""; }

        return *peerObject->Message();
    }

    return "";
}

// returns the sender ID for a piece of mail. (NymID).
//
std::string OTAPI_Exec::GetNym_MailSenderIDByIndex(
    const std::string& NYM_ID,
    const int32_t& nIndex) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    if (0 > nIndex) {
        otErr << __FUNCTION__
              << ": nIndex is out of bounds (it's in the negative!)\n";
        return "";
    }
    Identifier theNymID(NYM_ID);
    Nym* pNym = OTAPI()->GetNym(theNymID, __FUNCTION__);
    if (nullptr == pNym) return "";
    Message* pMessage = pNym->GetMailByIndex(nIndex);

    if (nullptr != pMessage) {
        // SENDER:    pMessage->m_strNymID
        // SERVER:    pMessage->m_strNotaryID
        // RECIPIENT: pMessage->m_strNymID2
        // MESSAGE:   pMessage->m_ascPayload (in an OTEnvelope)

        std::string pBuf = pMessage->m_strNymID.Get();
        return pBuf;
    }
    return "";
}

// returns the server ID that a piece of mail came from.
//
std::string OTAPI_Exec::GetNym_MailNotaryIDByIndex(
    const std::string& NYM_ID,
    const int32_t& nIndex) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }

    if (0 > nIndex) {
        otErr << __FUNCTION__
              << ": nIndex is out of bounds (it's in the negative!)\n";
        return "";
    }

    Identifier theNymID(NYM_ID);
    Nym* pNym = OTAPI()->GetNym(theNymID, __FUNCTION__);
    if (nullptr == pNym) return "";
    Message* pMessage = pNym->GetMailByIndex(nIndex);

    if (nullptr != pMessage) {
        // SENDER:    pMessage->m_strNymID
        // SERVER:    pMessage->m_strNotaryID
        // RECIPIENT: pMessage->m_strNymID2
        // MESSAGE:   pMessage->m_ascPayload (in an OTEnvelope)

        std::string pBuf = pMessage->m_strNotaryID.Get();
        return pBuf;
    }
    return "";
}

bool OTAPI_Exec::Nym_RemoveMailByIndex(
    const std::string& NYM_ID,
    const int32_t& nIndex) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return false;
    }
    if (0 > nIndex) {
        otErr << __FUNCTION__
              << ": nIndex is out of bounds (it's in the negative!)\n";
        return false;
    }
    Identifier theNymID(NYM_ID);
    Nym* pNym = OTAPI()->GetNym(theNymID, __FUNCTION__);
    if (nullptr == pNym) return false;
    Nym* pSignerNym = pNym;

    if (pNym->RemoveMailByIndex(nIndex)) {
        if (pNym->SaveSignedNymfile(*pSignerNym))  // <== save Nym to local
                                                   // storage, since a mail was
                                                   // erased.
            return true;
        else
            otErr << __FUNCTION__ << ": Error saving Nym: " << NYM_ID << "\n";
    }
    return false;
}

// Returns true (1) if the Sender ID on this piece of Mail (by index)
// loads a public key from my wallet, and if the signature on the message
// verifies with that public key.
// (Not only must the signature be good, but I must have added the nym to
// my wallet sometime in the past, since this func returns false if it's not
// there.)
//
// A good wallet might be designed to automatically download any keys that
// it doesn't already have, using OTAPI_Exec::checkNym(). I'll probably need to
// add something to OTClient where the checkNymResponse response auto-saves the
// new
// key into the wallet. That way you can wait for a tenth of a second and then
// just read the Nym (by ID) straight out of your own wallet. Nifty, eh?
//
// All the wallet has to do is fire off a "check user" whenever this call fails,
// then come back when that succeeds and try this again. If STILL failure, then
// you've got a signature problem. Otherwise it'll usually download the nym
// and verify the signature all in an instant, without the user even noticing
// what happened.
//
bool OTAPI_Exec::Nym_VerifyMailByIndex(
    const std::string& NYM_ID,
    const int32_t& nIndex) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return false;
    }
    if (0 > nIndex) {
        otErr << __FUNCTION__
              << ": nIndex is out of bounds (it's in the negative!)\n";
        return false;
    }
    Identifier theNymID(NYM_ID);
    Nym* pNym = OTAPI()->GetNym(theNymID, __FUNCTION__);
    if (nullptr == pNym) return false;
    Message* pMessage = pNym->GetMailByIndex(nIndex);

    if (nullptr != pMessage) {
        // Grab the NymID of the sender.
        const Identifier theSenderNymID(pMessage->m_strNymID);

        // Grab a pointer to that Nym (if its public key is in my wallet.)
        Nym* pSenderNym = OTAPI()->GetNym(theSenderNymID, __FUNCTION__);

        // If it's there, use it to verify the signature on the message.
        // return true if successful signature verification.
        //
        if (nullptr != pSenderNym) {
            if (pMessage->VerifySignature(*pSenderNym)) return true;
        }
    }
    return false;
}

//
// OUTMAIL!!

int32_t OTAPI_Exec::GetNym_OutmailCount(const std::string& NYM_ID) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    Identifier theNymID(NYM_ID);
    Nym* pNym = OTAPI()->GetNym(theNymID, __FUNCTION__);
    if (nullptr == pNym) return OT_ERROR;
    return pNym->GetOutmailCount();
}

// returns the message, optionally with Subject: as first line.
std::string OTAPI_Exec::GetNym_OutmailContentsByIndex(
    const std::string& NYM_ID,
    const int32_t& nIndex) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    if (0 > nIndex) {
        otErr << __FUNCTION__
              << ": nIndex is out of bounds (it's in the negative!)\n";
        return "";
    }
    Identifier theNymID(NYM_ID);
    Nym* pNym = OTAPI()->GetNym(theNymID, __FUNCTION__);
    if (nullptr == pNym) return "";
    Message* pMessage = pNym->GetOutmailByIndex(nIndex);
    if (nullptr != pMessage) {
        // SENDER:    pMessage->m_strNymID
        // RECIPIENT: pMessage->m_strNymID2
        // MESSAGE:   pMessage->m_ascPayload (in an OTEnvelope)
        String strMailContents;

        if (pMessage->m_ascPayload.Exists() &&
            pMessage->m_ascPayload.GetString(strMailContents)) {
            std::string pBuf = strMailContents.Get();
            return pBuf;
        }
    }
    return "";
}

// returns the recipient ID for a piece of mail. (NymID).
//
std::string OTAPI_Exec::GetNym_OutmailRecipientIDByIndex(
    const std::string& NYM_ID,
    const int32_t& nIndex) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    if (0 > nIndex) {
        otErr << __FUNCTION__
              << ": nIndex is out of bounds (it's in the negative!)\n";
        return "";
    }
    Identifier theNymID(NYM_ID);
    Nym* pNym = OTAPI()->GetNym(theNymID, __FUNCTION__);
    if (nullptr == pNym) return "";
    Message* pMessage = pNym->GetOutmailByIndex(nIndex);
    if (nullptr != pMessage) {
        // SENDER:    pMessage->m_strNymID
        // SERVER:    pMessage->m_strNotaryID
        // RECIPIENT: pMessage->m_strNymID2
        // MESSAGE:   pMessage->m_ascPayload

        std::string pBuf = pMessage->m_strNymID2.Get();
        return pBuf;
    }
    return "";
}

// returns the server ID that a piece of mail came from.
//
std::string OTAPI_Exec::GetNym_OutmailNotaryIDByIndex(
    const std::string& NYM_ID,
    const int32_t& nIndex) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    if (0 > nIndex) {
        otErr << __FUNCTION__
              << ": nIndex is out of bounds (it's in the negative!)\n";
        return "";
    }
    Identifier theNymID(NYM_ID);
    Nym* pNym = OTAPI()->GetNym(theNymID, __FUNCTION__);
    if (nullptr == pNym) return "";
    Message* pMessage = pNym->GetOutmailByIndex(nIndex);

    if (nullptr != pMessage) {
        // SENDER:    pMessage->m_strNymID
        // SERVER:    pMessage->m_strNotaryID
        // RECIPIENT: pMessage->m_strNymID2
        // MESSAGE:   pMessage->m_ascPayload

        std::string pBuf = pMessage->m_strNotaryID.Get();
        return pBuf;
    }
    return "";
}

bool OTAPI_Exec::Nym_RemoveOutmailByIndex(
    const std::string& NYM_ID,
    const int32_t& nIndex) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return false;
    }
    if (0 > nIndex) {
        otErr << __FUNCTION__
              << ": nIndex is out of bounds (it's in the negative!)\n";
        return false;
    }
    Identifier theNymID(NYM_ID);
    Nym* pNym = OTAPI()->GetNym(theNymID, __FUNCTION__);
    if (nullptr == pNym) return false;
    Nym* pSignerNym = pNym;

    if (pNym->RemoveOutmailByIndex(nIndex)) {
        if (pNym->SaveSignedNymfile(*pSignerNym))  // <== save Nym to local
                                                   // storage, since a mail was
                                                   // erased.
            return true;
        else
            otErr << __FUNCTION__ << ": Error saving Nym: " << NYM_ID << "\n";
    }
    return false;
}

// Returns true (1) if the Sender ID on this piece of Mail (by index)
// loads a public key from my wallet, and if the signature on the message
// verifies with that public key.
// (Not only must the signature be good, but I must have added the nym to
// my wallet sometime in the past, since this func returns false if it's not
// there.)
//
// A good wallet might be designed to automatically download any keys that
// it doesn't already have, using OTAPI_Exec::checkNym(). I'll probably need to
// add something to OTClient where the checkNymResponse response auto-saves the
// new
// key into the wallet. That way you can wait for a tenth of a second and then
// just read the Nym (by ID) straight out of your own wallet. Nifty, eh?
//
// All the wallet has to do is fire off a "check user" whenever this call fails,
// then come back when that succeeds and try this again. If STILL failure, then
// you've got a signature problem. Otherwise it'll usually download the nym
// and verify the signature all in an instant, without the user even noticing
// what happened.
//
bool OTAPI_Exec::Nym_VerifyOutmailByIndex(
    const std::string& NYM_ID,
    const int32_t& nIndex) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return false;
    }
    if (0 > nIndex) {
        otErr << __FUNCTION__
              << ": nIndex is out of bounds (it's in the negative!)\n";
        return false;
    }
    Identifier theNymID(NYM_ID);
    Nym* pNym = OTAPI()->GetNym(theNymID, __FUNCTION__);
    if (nullptr == pNym) return false;
    Message* pMessage = pNym->GetOutmailByIndex(nIndex);
    if (nullptr != pMessage) {
        // Grab the NymID of the sender.
        const Identifier theSenderNymID(pMessage->m_strNymID);

        // Grab a pointer to that Nym (if its public key is in my wallet.)
        Nym* pSenderNym = OTAPI()->GetNym(theSenderNymID, __FUNCTION__);

        // If it's there, use it to verify the signature on the message.
        // return true if successful signature verification.
        //
        if (nullptr != pSenderNym) {
            if (pMessage->VerifySignature(*pSenderNym)) return true;
        }
    }
    return false;
}

//
// OUTPAYMENTS!!
//
// (Outbox on payments screen.)
//
// Todo: Move these and all functions to OpenTransactions.cpp.  This should ONLY
// be a wrapper for that class.  That way we can eventually phase this file out
// entirely and replace it with OTAPI_Wrapper.cpp directly on
// OpenTransactions.cpp

int32_t OTAPI_Exec::GetNym_OutpaymentsCount(const std::string& NYM_ID) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    Identifier theNymID(NYM_ID);
    Nym* pNym = OTAPI()->GetNym(theNymID, __FUNCTION__);
    if (nullptr == pNym) return OT_ERROR;
    return pNym->GetOutpaymentsCount();
}

// Returns the payment instrument that was sent.
//
std::string OTAPI_Exec::GetNym_OutpaymentsContentsByIndex(
    const std::string& NYM_ID,
    const int32_t& nIndex) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    if (0 > nIndex) {
        otErr << __FUNCTION__
              << ": nIndex is out of bounds (it's in the negative!)\n";
        return "";
    }
    Identifier theNymID(NYM_ID);
    Nym* pNym = OTAPI()->GetNym(theNymID, __FUNCTION__);
    if (nullptr == pNym) return "";
    Message* pMessage = pNym->GetOutpaymentsByIndex(nIndex);
    if (nullptr != pMessage) {
        // SENDER:     pMessage->m_strNymID
        // RECIPIENT:  pMessage->m_strNymID2
        // INSTRUMENT: pMessage->m_ascPayload (in an OTEnvelope)
        String strPayment;

        // There isn't any encrypted envelope this time, since it's my
        // outPayments box.
        //
        if (pMessage->m_ascPayload.Exists() &&
            pMessage->m_ascPayload.GetString(strPayment) &&
            strPayment.Exists()) {
            OTPayment thePayment(strPayment);
            if (thePayment.IsValid()) {
                std::string pBuf = strPayment.Get();
                return pBuf;
            }
        }
    }
    return "";
}

// returns the recipient ID for a piece of payments outmail. (NymID).
//
std::string OTAPI_Exec::GetNym_OutpaymentsRecipientIDByIndex(
    const std::string& NYM_ID,
    const int32_t& nIndex) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    if (0 > nIndex) {
        otErr << __FUNCTION__
              << ": nIndex is out of bounds (it's in the negative!)\n";
        return "";
    }
    Identifier theNymID(NYM_ID);
    Nym* pNym = OTAPI()->GetNym(theNymID, __FUNCTION__);
    if (nullptr == pNym) return "";
    Message* pMessage = pNym->GetOutpaymentsByIndex(nIndex);
    if (nullptr != pMessage) {
        // SENDER:    pMessage->m_strNymID
        // SERVER:    pMessage->m_strNotaryID
        // RECIPIENT: pMessage->m_strNymID2
        // MESSAGE:   pMessage->m_ascPayload

        std::string pBuf = pMessage->m_strNymID2.Get();
        return pBuf;
    }
    return "";
}

// returns the server ID that a piece of payments outmail came from.
//
std::string OTAPI_Exec::GetNym_OutpaymentsNotaryIDByIndex(
    const std::string& NYM_ID,
    const int32_t& nIndex) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    if (0 > nIndex) {
        otErr << __FUNCTION__
              << ": nIndex is out of bounds (it's in the negative!)\n";
        return "";
    }
    Identifier theNymID(NYM_ID);
    Nym* pNym = OTAPI()->GetNym(theNymID, __FUNCTION__);
    if (nullptr == pNym) return "";
    Message* pMessage = pNym->GetOutpaymentsByIndex(nIndex);

    if (nullptr != pMessage) {
        // SENDER:    pMessage->m_strNymID
        // SERVER:    pMessage->m_strNotaryID
        // RECIPIENT: pMessage->m_strNymID2
        // MESSAGE:   pMessage->m_ascPayload

        int32_t bNotaryIDLength = pMessage->m_strNotaryID.GetLength();
        if (1 >= bNotaryIDLength) {
            otErr << __FUNCTION__ << ": m_strNotaryID Length is 1 or less!\n";
            return "";
        }

        std::string pBuf = pMessage->m_strNotaryID.Get();
        return pBuf;
    }
    return "";
}

bool OTAPI_Exec::Nym_RemoveOutpaymentsByIndex(
    const std::string& NYM_ID,
    const int32_t& nIndex) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return false;
    }
    if (0 > nIndex) {
        otErr << __FUNCTION__
              << ": nIndex is out of bounds (it's in the negative!)\n";
        return false;
    }

    Identifier theNymID(NYM_ID);
    Nym* pNym = OTAPI()->GetNym(theNymID, __FUNCTION__);
    if (nullptr == pNym) return false;
    Nym* pSignerNym = pNym;

    if (pNym->RemoveOutpaymentsByIndex(nIndex)) {
        if (pNym->SaveSignedNymfile(*pSignerNym))  // <== save Nym to local
                                                   // storage, since a payment
                                                   // outmail was erased.
            return true;
        else
            otErr << __FUNCTION__ << ": Error saving Nym: " << NYM_ID << "\n";
    }
    return false;
}

// Returns true (1) if the Sender ID on this piece of Mail (by index)
// loads a public key from my wallet, and if the signature on the message
// verifies with that public key.
// (Not only must the signature be good, but I must have added the nym to
// my wallet sometime in the past, since this func returns false if it's not
// there.)
//
// A good wallet might be designed to automatically download any keys that
// it doesn't already have, using OTAPI_Exec::checkNym(). I'll probably need to
// add something to OTClient where the checkNymResponse response auto-saves the
// new
// key into the wallet. That way you can wait for a tenth of a second and then
// just read the Nym (by ID) straight out of your own wallet. Nifty, eh?
//
// All the wallet has to do is fire off a "check user" whenever this call fails,
// then come back when that succeeds and try this again. If STILL failure, then
// you've got a signature problem. Otherwise it'll usually download the nym
// and verify the signature all in an instant, without the user even noticing
// what happened.
//
bool OTAPI_Exec::Nym_VerifyOutpaymentsByIndex(
    const std::string& NYM_ID,
    const int32_t& nIndex) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return false;
    }
    if (0 > nIndex) {
        otErr << __FUNCTION__
              << ": nIndex is out of bounds (it's in the negative!)\n";
        return false;
    }
    Identifier theNymID(NYM_ID);
    Nym* pNym = OTAPI()->GetNym(theNymID, __FUNCTION__);
    if (nullptr == pNym) return false;
    Message* pMessage = pNym->GetOutpaymentsByIndex(nIndex);
    if (nullptr != pMessage) {
        // Grab the NymID of the sender.
        const Identifier theSenderNymID(pMessage->m_strNymID);

        // Grab a pointer to that Nym (if its public key is in my wallet.)
        Nym* pSenderNym = OTAPI()->GetNym(theSenderNymID, __FUNCTION__);

        // If it's there, use it to verify the signature on the message.
        // return true if successful signature verification.
        //
        if (nullptr != pSenderNym) {
            if (pMessage->VerifySignature(*pSenderNym)) return true;
        }
    }
    return false;
}

//
//
// THESE FUNCTIONS were added for the PAYMENTS screen. (They are fairly new.)
//
// Basically there was a need to have DIFFERENT instruments, but to be able to
// treat them as though they are a single type.
//
// In keeping with that, the below functions will work with disparate types.
// You can pass [ CHEQUES / VOUCHERS / INVOICES ] and PAYMENT PLANS, and
// SMART CONTRACTS, and PURSEs into these functions, and they should be able
// to handle any of those types.
//
//

int64_t OTAPI_Exec::Instrmnt_GetAmount(const std::string& THE_INSTRUMENT) const
{
    if (THE_INSTRUMENT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_INSTRUMENT passed in!\n";
        return -1;
    }
    const String strInstrument(THE_INSTRUMENT);
    OTPayment thePayment(strInstrument);

    if (!thePayment.IsValid()) {
        otOut << __FUNCTION__ << ": Unable to parse instrument:\n\n"
              << strInstrument << "\n\n";
        return -1;
    }
    const bool& bSetValues = thePayment.SetTempValues();
    if (!bSetValues) {
        otOut << __FUNCTION__ << ": Unable to load instrument:\n\n"
              << strInstrument << "\n\n";
        return -1;
    }
    // BY THIS POINT, we have definitely loaded up all the values of the
    // instrument
    // into the OTPayment object. (Meaning we can now return the requested
    // data...)

    int64_t lOutput = 0;
    return thePayment.GetAmount(lOutput) ? lOutput : OT_ERROR_AMOUNT;
}

int64_t OTAPI_Exec::Instrmnt_GetTransNum(
    const std::string& THE_INSTRUMENT) const
{
    if (THE_INSTRUMENT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_INSTRUMENT passed in!\n";
        return -1;
    }
    const String strInstrument(THE_INSTRUMENT);
    OTPayment thePayment(strInstrument);

    if (!thePayment.IsValid()) {
        otOut << __FUNCTION__ << ": Unable to parse instrument:\n\n"
              << strInstrument << "\n\n";
        return -1;
    }
    const bool& bSetValues = thePayment.SetTempValues();
    if (!bSetValues) {
        otOut << __FUNCTION__ << ": Unable to load instrument:\n\n"
              << strInstrument << "\n\n";
        return -1;
    }
    // BY THIS POINT, we have definitely loaded up all the values of the
    // instrument
    // into the OTPayment object. (Meaning we can now return the requested
    // data...)
    String strOutput;
    int64_t lOutput = 0;
    const bool& bGotData = thePayment.GetTransactionNum(lOutput);  // <========

    return bGotData ? lOutput : -1;
}

time64_t OTAPI_Exec::Instrmnt_GetValidFrom(
    const std::string& THE_INSTRUMENT) const
{
    if (THE_INSTRUMENT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_INSTRUMENT passed in!\n";
        return OTTimeGetTimeFromSeconds(-1);
    }
    const String strInstrument(THE_INSTRUMENT);
    OTPayment thePayment(strInstrument);

    if (!thePayment.IsValid()) {
        otOut << __FUNCTION__ << ": Unable to parse instrument:\n\n"
              << strInstrument << "\n\n";
        return OTTimeGetTimeFromSeconds(-1);
    }
    const bool& bSetValues = thePayment.SetTempValues();
    if (!bSetValues) {
        otOut << __FUNCTION__ << ": Unable to load instrument:\n\n"
              << strInstrument << "\n\n";
        return OTTimeGetTimeFromSeconds(-1);
    }

    // BY THIS POINT, we have definitely loaded up all the values of the
    // instrument
    // into the OTPayment object. (Meaning we can now return the requested
    // data...)

    String strOutput;
    time64_t tOutput = OT_TIME_ZERO;
    const bool& bGotData = thePayment.GetValidFrom(tOutput);  // <========

    return bGotData ? tOutput : OTTimeGetTimeFromSeconds(-1);
}

time64_t OTAPI_Exec::Instrmnt_GetValidTo(
    const std::string& THE_INSTRUMENT) const
{
    if (THE_INSTRUMENT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_INSTRUMENT passed in!\n";
        return OTTimeGetTimeFromSeconds(-1);
    }
    const String strInstrument(THE_INSTRUMENT);
    OTPayment thePayment(strInstrument);

    if (!thePayment.IsValid()) {
        otOut << __FUNCTION__ << ": Unable to parse instrument:\n\n"
              << strInstrument << "\n\n";
        return OTTimeGetTimeFromSeconds(-1);
    }
    const bool& bSetValues = thePayment.SetTempValues();
    if (!bSetValues) {
        otOut << __FUNCTION__ << ": Unable to load instrument:\n\n"
              << strInstrument << "\n\n";
        return OTTimeGetTimeFromSeconds(-1);
    }

    // BY THIS POINT, we have definitely loaded up all the values of the
    // instrument
    // into the OTPayment object. (Meaning we can now return the requested
    // data...)

    String strOutput;
    time64_t tOutput = OT_TIME_ZERO;
    const bool& bGotData = thePayment.GetValidTo(tOutput);  // <========

    return bGotData ? tOutput : OTTimeGetTimeFromSeconds(-1);
}

std::string OTAPI_Exec::Instrmnt_GetType(
    const std::string& THE_INSTRUMENT) const
{
    if (THE_INSTRUMENT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_INSTRUMENT passed in!\n";
        return "";
    }
    const String strInstrument(THE_INSTRUMENT);
    OTPayment thePayment(strInstrument);

    if (!thePayment.IsValid()) {
        otOut << __FUNCTION__ << ": Unable to parse instrument:\n\n"
              << strInstrument << "\n\n";
        return "";
    }
    const bool& bSetValues = thePayment.SetTempValues();
    if (!bSetValues) {
        otOut << __FUNCTION__ << ": Unable to load instrument:\n\n"
              << strInstrument << "\n\n";
        return "";
    }
    // BY THIS POINT, we have definitely loaded up all the values of the
    // instrument
    // into the OTPayment object. (Meaning we can now return the requested
    // data...)

    const String strOutput(thePayment.GetTypeString());

    if (strOutput.Exists()) {
        std::string pBuf = strOutput.Get();

        return pBuf;
    }

    return "";
}

std::string OTAPI_Exec::Instrmnt_GetMemo(
    const std::string& THE_INSTRUMENT) const
{
    if (THE_INSTRUMENT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_INSTRUMENT passed in!\n";
        return "";
    }
    const String strInstrument(THE_INSTRUMENT);
    OTPayment thePayment(strInstrument);

    if (!thePayment.IsValid()) {
        otOut << __FUNCTION__ << ": Unable to parse instrument:\n\n"
              << strInstrument << "\n\n";
        return "";
    }
    const bool& bSetValues = thePayment.SetTempValues();
    if (!bSetValues) {
        otOut << __FUNCTION__ << ": Unable to load instrument:\n\n"
              << strInstrument << "\n\n";
        return "";
    }

    // BY THIS POINT, we have definitely loaded up all the values of the
    // instrument
    // into the OTPayment object. (Meaning we can now return the requested
    // data...)

    String strOutput;
    const bool& bGotData = thePayment.GetMemo(strOutput);  // <========

    if (bGotData) {
        std::string pBuf = strOutput.Get();
        return pBuf;
    }

    return "";
}

std::string OTAPI_Exec::Instrmnt_GetNotaryID(
    const std::string& THE_INSTRUMENT) const
{
    if (THE_INSTRUMENT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_INSTRUMENT passed in!\n";
        return "";
    }
    const String strInstrument(THE_INSTRUMENT);
    OTPayment thePayment(strInstrument);

    if (!thePayment.IsValid()) {
        otOut << __FUNCTION__ << ": Unable to parse instrument:\n\n"
              << strInstrument << "\n\n";
        return "";
    }
    const bool& bSetValues = thePayment.SetTempValues();
    if (!bSetValues) {
        otOut << __FUNCTION__ << ": Unable to load instrument:\n\n"
              << strInstrument << "\n\n";
        return "";
    }

    // BY THIS POINT, we have definitely loaded up all the values of the
    // instrument
    // into the OTPayment object. (Meaning we can now return the requested
    // data...)

    Identifier theOutput;
    const bool& bGotData = thePayment.GetNotaryID(theOutput);  // <========

    if (bGotData) {
        const String strOutput(theOutput);
        std::string pBuf = strOutput.Get();
        return pBuf;
    }

    return "";
}

std::string OTAPI_Exec::Instrmnt_GetInstrumentDefinitionID(
    const std::string& THE_INSTRUMENT) const
{
    if (THE_INSTRUMENT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_INSTRUMENT passed in!\n";
        return "";
    }
    const String strInstrument(THE_INSTRUMENT);
    OTPayment thePayment(strInstrument);

    if (!thePayment.IsValid()) {
        otOut << __FUNCTION__ << ": Unable to parse instrument:\n\n"
              << strInstrument << "\n\n";
        return "";
    }
    const bool& bSetValues = thePayment.SetTempValues();
    if (!bSetValues) {
        otOut << __FUNCTION__ << ": Unable to load instrument:\n\n"
              << strInstrument << "\n\n";
        return "";
    }

    // BY THIS POINT, we have definitely loaded up all the values of the
    // instrument
    // into the OTPayment object. (Meaning we can now return the requested
    // data...)

    Identifier theOutput;
    const bool& bGotData =
        thePayment.GetInstrumentDefinitionID(theOutput);  // <========

    if (bGotData) {
        const String strOutput(theOutput);
        std::string pBuf = strOutput.Get();
        return pBuf;
    }

    return "";
}

std::string OTAPI_Exec::Instrmnt_GetRemitterNymID(
    const std::string& THE_INSTRUMENT) const
{
    if (THE_INSTRUMENT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_INSTRUMENT passed in!\n";
        return "";
    }
    const String strInstrument(THE_INSTRUMENT);
    OTPayment thePayment(strInstrument);

    if (!thePayment.IsValid()) {
        otOut << __FUNCTION__ << ": Unable to parse instrument:\n\n"
              << strInstrument << "\n\n";
        return "";
    }
    const bool& bSetValues = thePayment.SetTempValues();
    if (!bSetValues) {
        otOut << __FUNCTION__ << ": Unable to load instrument:\n\n"
              << strInstrument << "\n\n";
        return "";
    }

    // BY THIS POINT, we have definitely loaded up all the values of the
    // instrument
    // into the OTPayment object. (Meaning we can now return the requested
    // data...)

    Identifier theOutput;
    const bool& bGotData = thePayment.GetRemitterNymID(theOutput);  // <========

    if (bGotData) {
        const String strOutput(theOutput);
        std::string pBuf = strOutput.Get();
        return pBuf;
    }

    return "";
}

std::string OTAPI_Exec::Instrmnt_GetRemitterAcctID(
    const std::string& THE_INSTRUMENT) const
{
    if (THE_INSTRUMENT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_INSTRUMENT passed in!\n";
        return "";
    }
    const String strInstrument(THE_INSTRUMENT);
    OTPayment thePayment(strInstrument);

    if (!thePayment.IsValid()) {
        otOut << __FUNCTION__ << ": Unable to parse instrument:\n\n"
              << strInstrument << "\n\n";
        return "";
    }
    const bool& bSetValues = thePayment.SetTempValues();
    if (!bSetValues) {
        otOut << __FUNCTION__ << ": Unable to load instrument:\n\n"
              << strInstrument << "\n\n";
        return "";
    }

    // BY THIS POINT, we have definitely loaded up all the values of the
    // instrument
    // into the OTPayment object. (Meaning we can now return the requested
    // data...)

    Identifier theOutput;
    const bool& bGotData =
        thePayment.GetRemitterAcctID(theOutput);  // <========

    if (bGotData) {
        const String strOutput(theOutput);
        std::string pBuf = strOutput.Get();
        return pBuf;
    }

    return "";
}

std::string OTAPI_Exec::Instrmnt_GetSenderNymID(
    const std::string& THE_INSTRUMENT) const
{
    if (THE_INSTRUMENT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_INSTRUMENT passed in!\n";
        return "";
    }
    const String strInstrument(THE_INSTRUMENT);
    OTPayment thePayment(strInstrument);

    if (!thePayment.IsValid()) {
        otOut << __FUNCTION__ << ": Unable to parse instrument:\n\n"
              << strInstrument << "\n\n";
        return "";
    }
    const bool& bSetValues = thePayment.SetTempValues();
    if (!bSetValues) {
        otOut << __FUNCTION__ << ": Unable to load instrument:\n\n"
              << strInstrument << "\n\n";
        return "";
    }

    // BY THIS POINT, we have definitely loaded up all the values of the
    // instrument
    // into the OTPayment object. (Meaning we can now return the requested
    // data...)

    Identifier theOutput;
    const bool& bGotData = thePayment.GetSenderNymID(theOutput);  // <========

    if (bGotData) {
        const String strOutput(theOutput);
        std::string pBuf = strOutput.Get();
        return pBuf;
    }

    return "";
}

std::string OTAPI_Exec::Instrmnt_GetSenderAcctID(
    const std::string& THE_INSTRUMENT) const
{
    if (THE_INSTRUMENT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_INSTRUMENT passed in!\n";
        return "";
    }
    const String strInstrument(THE_INSTRUMENT);
    OTPayment thePayment(strInstrument);

    if (!thePayment.IsValid()) {
        otOut << __FUNCTION__ << ": Unable to parse instrument:\n\n"
              << strInstrument << "\n\n";
        return "";
    }
    const bool& bSetValues = thePayment.SetTempValues();
    if (!bSetValues) {
        otOut << __FUNCTION__ << ": Unable to load instrument:\n\n"
              << strInstrument << "\n\n";
        return "";
    }

    // BY THIS POINT, we have definitely loaded up all the values of the
    // instrument
    // into the OTPayment object. (Meaning we can now return the requested
    // data...)

    Identifier theOutput;
    const bool& bGotData = thePayment.GetSenderAcctID(theOutput);  // <========

    if (bGotData) {
        const String strOutput(theOutput);
        std::string pBuf = strOutput.Get();
        return pBuf;
    }

    return "";
}

std::string OTAPI_Exec::Instrmnt_GetRecipientNymID(
    const std::string& THE_INSTRUMENT) const
{
    if (THE_INSTRUMENT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_INSTRUMENT passed in!\n";
        return "";
    }
    const String strInstrument(THE_INSTRUMENT);
    OTPayment thePayment(strInstrument);

    if (!thePayment.IsValid()) {
        otOut << __FUNCTION__ << ": Unable to parse instrument:\n\n"
              << strInstrument << "\n\n";
        return "";
    }
    const bool& bSetValues = thePayment.SetTempValues();
    if (!bSetValues) {
        otOut << __FUNCTION__ << ": Unable to load instrument:\n\n"
              << strInstrument << "\n\n";
        return "";
    }

    // BY THIS POINT, we have definitely loaded up all the values of the
    // instrument
    // into the OTPayment object. (Meaning we can now return the requested
    // data...)

    Identifier theOutput;
    const bool& bGotData =
        thePayment.GetRecipientNymID(theOutput);  // <========

    if (bGotData) {
        const String strOutput(theOutput);
        std::string pBuf = strOutput.Get();
        return pBuf;
    }

    return "";
}

std::string OTAPI_Exec::Instrmnt_GetRecipientAcctID(
    const std::string& THE_INSTRUMENT) const
{
    if (THE_INSTRUMENT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_INSTRUMENT passed in!\n";
        return "";
    }
    const String strInstrument(THE_INSTRUMENT);
    OTPayment thePayment(strInstrument);

    if (!thePayment.IsValid()) {
        otOut << __FUNCTION__ << ": Unable to parse instrument:\n\n"
              << strInstrument << "\n\n";
        return "";
    }
    const bool& bSetValues = thePayment.SetTempValues();
    if (!bSetValues) {
        otOut << __FUNCTION__ << ": Unable to load instrument:\n\n"
              << strInstrument << "\n\n";
        return "";
    }
    // BY THIS POINT, we have definitely loaded up all the values of the
    // instrument
    // into the OTPayment object. (Meaning we can now return the requested
    // data...)

    Identifier theOutput;
    const bool& bGotData =
        thePayment.GetRecipientAcctID(theOutput);  // <========

    if (bGotData) {
        const String strOutput(theOutput);
        std::string pBuf = strOutput.Get();
        return pBuf;
    }

    return "";
}

// SET NYM NAME
//
// You might have 40 of your friends' public nyms in
// your wallet. You might have labels on each of them.
// But whenever you change a label (and thus re-sign the
// file for that Nym when you save it), you only SIGN
// using one of your OWN nyms, for which you have a private
// key available for signing.
//
// When testing, there is only one nym, so you just pass it
// twice.  But in real production, a user will have a default
// signing nym, the same way that he might have a default
// signing key in PGP, and that must be passed in whenever
// he changes the name on any of the other nyms in his wallet.
// (In order to properly sign and save the change.)
//
// Returns true (1) or false (0)
//
bool OTAPI_Exec::SetNym_Name(
    const std::string& NYM_ID,
    const std::string& SIGNER_NYM_ID,
    const std::string& NYM_NEW_NAME) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return false;
    }
    if (SIGNER_NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SINGER_NYM_ID passed in!\n";
        return false;
    }
    if (NYM_NEW_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_NEW_NAME passed in!\n";
        return false;
    }

    const Identifier theNymID(NYM_ID), theSignerNymID(SIGNER_NYM_ID);
    const String strNymName(NYM_NEW_NAME);

    bool bSuccess = OTAPI()->SetNym_Name(theNymID, theSignerNymID, strNymName);

    return bSuccess;
}

// Merely a client-side label
bool OTAPI_Exec::SetServer_Name(
    const std::string& NOTARY_ID,
    const std::string& STR_NEW_NAME) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null NOTARY_ID passed in!\n";
        return false;
    }
    if (STR_NEW_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null STR_NEW_NAME passed in!\n";
        return false;
    }

    const Identifier theContractID(NOTARY_ID);
    const String strNewName(STR_NEW_NAME);

    bool bSuccess = OTAPI()->SetServer_Name(theContractID, strNewName);

    return bSuccess;
}

// Merely a client-side label
bool OTAPI_Exec::SetAssetType_Name(
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& STR_NEW_NAME) const
{
    if (INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Null: INSTRUMENT_DEFINITION_ID passed in!\n";
        return false;
    }
    if (STR_NEW_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: STR_NEW_NAME passed in!\n";
        return false;
    }

    const Identifier theContractID(INSTRUMENT_DEFINITION_ID);
    const String strNewName(STR_NEW_NAME);

    bool bSuccess = OTAPI()->SetAssetType_Name(theContractID, strNewName);

    return bSuccess;
}

// GET NYM TRANSACTION NUM COUNT
// How many transaction numbers does the Nym have (for a given server?)
//
// This function returns the count of numbers available. If 0, then no
// transactions will work until you call OTAPI_Exec::getTransactionNumber()
// to replenish your Nym's supply for that NotaryID...
//
// Returns a count (0 through N numbers available),
// or -1 for error (no nym found.)
//
int32_t OTAPI_Exec::GetNym_TransactionNumCount(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }

    Identifier theNotaryID(NOTARY_ID);
    Identifier theNymID(NYM_ID);

    Nym* pNym = OTAPI()->GetNym(theNymID, __FUNCTION__);

    if (nullptr != pNym) return pNym->GetTransactionNumCount(theNotaryID);

    return OT_ERROR;
}

// based on Index (above 4 functions) this returns the Server's ID
std::string OTAPI_Exec::GetServer_ID(const int32_t& nIndex) const
{
    if (0 > nIndex) {
        otErr << __FUNCTION__
              << ": nIndex is out of bounds (it's in the negative!)\n";
        return "";
    }

    uint32_t index(nIndex);
    auto servers = App::Me().Contract().ServerList();

    if (index <= servers.size()) {
        ObjectList::iterator it = servers.begin();
        std::advance(it, index);
        return it->first;
    }

    return "";
}

// Return's Server's name (based on server ID)
std::string OTAPI_Exec::GetServer_Name(const std::string& THE_ID) const
{
    if (THE_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_ID passed in!\n";
        return "";
    }
    Identifier theID(THE_ID);

    auto pServer = OTAPI()->GetServer(theID, __FUNCTION__);
    if (!pServer) return "";

    return pServer->Alias();
}

// returns Instrument Definition ID (based on index from GetAssetTypeCount)
std::string OTAPI_Exec::GetAssetType_ID(const int32_t& nIndex) const
{
    if (0 > nIndex) {
        otErr << __FUNCTION__
              << ": nIndex is out of bounds (it's in the negative!)\n";
        return "";
    }

    uint32_t index(nIndex);
    auto units = App::Me().Contract().UnitDefinitionList();

    if (index <= units.size()) {
        ObjectList::iterator it = units.begin();
        std::advance(it, index);
        return it->first;
    }

    return "";
}

// Returns instrument definition Name based on Instrument Definition ID
std::string OTAPI_Exec::GetAssetType_Name(const std::string& THE_ID) const
{
    if (THE_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_ID passed in!\n";
        return "";
    }

    Identifier theID(THE_ID);
    auto pContract = OTAPI()->GetAssetType(theID, __FUNCTION__);
    if (!pContract) return "";
    String strName(pContract->Alias());
    std::string pBuf = strName.Get();

    return pBuf;
}

// Returns instrument definition TLA based on Instrument Definition ID
std::string OTAPI_Exec::GetAssetType_TLA(const std::string& THE_ID) const
{
    if (THE_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_ID passed in!\n";
        return "";
    }

    auto unit = App::Me().Contract().UnitDefinition(Identifier(THE_ID));

    if (!unit) return "";

    return unit->TLA();
}

// returns a string containing the account ID, based on index.
std::string OTAPI_Exec::GetAccountWallet_ID(const int32_t& nIndex) const
{
    if (0 > nIndex) {
        otErr << __FUNCTION__
              << ": nIndex is out of bounds (it's in the negative!)\n";
        return "";
    }

    Identifier theID;
    String strName;

    bool bGetServer = OTAPI()->GetAccount(nIndex, theID, strName);

    if (bGetServer) {
        String strID(theID);

        std::string pBuf = strID.Get();

        return pBuf;
    }
    return "";
}

// returns the account name, based on account ID.
std::string OTAPI_Exec::GetAccountWallet_Name(const std::string& THE_ID) const
{
    if (THE_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_ID passed in!\n";
        return "";
    }

    Identifier theID(THE_ID);

    std::string strFunc = "OTAPI_Exec::GetAccountWallet_Name";
    Account* pAccount = OTAPI()->GetAccount(theID, strFunc.c_str());
    if (nullptr == pAccount) return "";
    String strName;
    pAccount->GetName(strName);
    std::string pBuf = strName.Get();

    return pBuf;
}

std::string OTAPI_Exec::GetAccountWallet_InboxHash(
    const std::string& ACCOUNT_ID) const  // returns latest InboxHash according
                                          // to the
// account file. (Usually more recent than:
// OTAPI_Exec::GetNym_InboxHash)
{
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return "";
    }

    Identifier theID(ACCOUNT_ID);

    Account* pAccount = OTAPI()->GetAccount(theID, __FUNCTION__);
    if (nullptr == pAccount) return "";

    Identifier theOutput;
    const bool& bGotHash = pAccount->GetInboxHash(theOutput);

    String strOutput;

    if (bGotHash) theOutput.GetString(strOutput);

    std::string pBuf = strOutput.Get();

    return pBuf;
}

std::string OTAPI_Exec::GetAccountWallet_OutboxHash(
    const std::string& ACCOUNT_ID) const  // returns latest OutboxHash according
                                          // to the
// account file. (Usually more recent than:
// OTAPI_Exec::GetNym_OutboxHash)
{
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return "";
    }

    Identifier theID(ACCOUNT_ID);

    Account* pAccount = OTAPI()->GetAccount(theID, __FUNCTION__);
    if (nullptr == pAccount) return "";

    Identifier theOutput;
    const bool& bGotHash = pAccount->GetOutboxHash(theOutput);

    String strOutput;

    if (bGotHash) theOutput.GetString(strOutput);

    std::string pBuf = strOutput.Get();

    return pBuf;
}

/** TIME (in seconds, as string)

This will return the current time in seconds, as a string.
Returns "" if failure.

Todo:  consider making this available on the server side as well,
so the smart contracts can see what time it is.

*/
time64_t OTAPI_Exec::GetTime(void) const { return OTAPI()->GetTime(); }

/** OT-encode a plaintext string.  (NOT ENCRYPT)

std::string OTAPI_Exec::Encode(const std::string& strPlaintext);

This will pack, compress, and base64-encode a plain string.
Returns the base64-encoded string, or "".

Internally:
OTString        strPlain(strPlaintext);
OTASCIIArmor    ascEncoded(thePlaintext);    // ascEncoded now contains the
OT-encoded string.
return            ascEncoded.Get();            // We return it.
*/
std::string OTAPI_Exec::Encode(
    const std::string& strPlaintext,
    const bool& bLineBreaks) const  // bLineBreaks
                                    // should
// usually be set to
// true.
{
    if (strPlaintext.empty()) {
        otErr << __FUNCTION__ << ": Null: strPlaintext passed in!\n";
        return "";
    }

    const String otstrPlaintext(strPlaintext);
    String strOutput;

    bool bEncoded = OTAPI()->Encode(
        otstrPlaintext, strOutput, (true == bLineBreaks) ? true : false);

    if (!bEncoded) return "";

    std::string pBuf = strOutput.Get();

    return pBuf;
}

/** Decode an OT-encoded string (back to plaintext.)  (NOT DECRYPT)

std::string OTAPI_Exec::Decode(const std::string& strEncoded);

This will base64-decode, uncompress, and unpack an OT-encoded string.
Returns the plaintext string, or "".

Internally:
OTASCIIArmor    ascEncoded(strEncoded);
OTString        strPlain(ascEncoded);    // strPlain now contains the decoded
plaintext string.
return            strPlain.Get();            // We return it.
*/
std::string OTAPI_Exec::Decode(
    const std::string& strEncoded,
    const bool& bLineBreaks) const
{
    if (strEncoded.empty()) {
        return "";
    }

    const String otstrEncoded(strEncoded);
    String strOutput;

    bool bDecoded = OTAPI()->Decode(
        otstrEncoded, strOutput, (true == bLineBreaks) ? true : false);

    if (!bDecoded) return "";

    std::string pBuf = strOutput.Get();

    return pBuf;
}

/** OT-ENCRYPT a plaintext string.  ASYMMETRIC

std::string OTAPI_Exec::Encrypt(const std::string& RECIPIENT_NYM_ID, const
std::string& strPlaintext);

This will encode, ENCRYPT, and encode a plain string.
Returns the base64-encoded ciphertext, or "".

Internally the C++ code is:
OTString        strPlain(strPlaintext);
OTEnvelope        theEnvelope;
if (theEnvelope.Seal(RECIPIENT_NYM, strPlain)) {    // Now it's encrypted (in
binary form, inside the envelope), to the recipient's nym.
OTASCIIArmor    ascCiphertext(theEnvelope);        // ascCiphertext now contains
the base64-encoded ciphertext (as a string.)
return ascCiphertext.Get();
}
*/
std::string OTAPI_Exec::Encrypt(
    const std::string& RECIPIENT_NYM_ID,
    const std::string& strPlaintext) const
{
    if (RECIPIENT_NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: RECIPIENT_NYM_ID passed in!\n";
        return "";
    }
    if (strPlaintext.empty()) {
        otErr << __FUNCTION__ << ": Null: strPlaintext passed in!\n";
        return "";
    }
    const String otstrPlaintext(strPlaintext);
    const Identifier theRecipientNymID(RECIPIENT_NYM_ID);
    String strOutput;

    bool bEncrypted =
        OTAPI()->Encrypt(theRecipientNymID, otstrPlaintext, strOutput);

    if (!bEncrypted || !strOutput.Exists()) return "";

    std::string pBuf = strOutput.Get();

    return pBuf;
}

/** OT-DECRYPT an OT-encrypted string back to plaintext.  ASYMMETRIC

std::string OTAPI_Exec::Decrypt(const std::string& RECIPIENT_NYM_ID, const
std::string& strCiphertext);

Decrypts the base64-encoded ciphertext back into a normal string plaintext.
Returns the plaintext string, or "".

Internally the C++ code is:
OTEnvelope        theEnvelope;                    // Here is the envelope
object. (The ciphertext IS the data for an OTEnvelope.)
OTASCIIArmor    ascCiphertext(strCiphertext);    // The base64-encoded
ciphertext passed in. Next we'll try to attach it to envelope object...
if (theEnvelope.SetAsciiArmoredData(ascCiphertext)) {    // ...so that we can
open it using the appropriate Nym, into a plain string object:
OTString    strServerReply;                    // This will contain the output
when we're done.
const bool    bOpened =                        // Now we try to decrypt:
theEnvelope.Open(RECIPIENT_NYM, strServerReply);
if (bOpened) {
return strServerReply.Get();
}
}
*/
std::string OTAPI_Exec::Decrypt(
    const std::string& RECIPIENT_NYM_ID,
    const std::string& strCiphertext) const
{
    if (RECIPIENT_NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: RECIPIENT_NYM_ID passed in!\n";
        return "";
    }
    if (strCiphertext.empty()) {
        otErr << __FUNCTION__ << ": Null: strCiphertext passed in!\n";
        return "";
    }
    const String otstrCiphertext(strCiphertext);
    const Identifier theRecipientNymID(RECIPIENT_NYM_ID);
    String strOutput;

    bool bDecrypted =
        OTAPI()->Decrypt(theRecipientNymID, otstrCiphertext, strOutput);

    if (!bDecrypted || !strOutput.Exists()) return "";

    std::string pBuf = strOutput.Get();

    return pBuf;
}

// SYMMETRIC

// Generates a new symmetric key, based on a passphrase,
// and returns it (or "".)
//
std::string OTAPI_Exec::CreateSymmetricKey() const
{
    String strOutput;
    std::string strDisplay = "OTAPI: Creating a new symmetric key.";
    const String otstrDisplay(strDisplay);
    const bool& bSuccess = OTSymmetricKey::CreateNewKey(
        strOutput, &otstrDisplay);  // pAlreadyHavePW=""

    if (!bSuccess) return "";

    std::string pBuf = strOutput.Get();

    return pBuf;
}

// OTEnvelope:
//     bool Encrypt(const OTString& theInput, OTSymmetricKey& theKey,
// const OTPassword& thePassword);
//     bool Decrypt(OTString& theOutput, const OTSymmetricKey& theKey,
// const OTPassword& thePassword);

// Returns the CIPHERTEXT_ENVELOPE (the Envelope encrypted with the Symmetric
// Key.)
//
std::string OTAPI_Exec::SymmetricEncrypt(
    const std::string& SYMMETRIC_KEY,
    const std::string& PLAINTEXT) const
{
    if (SYMMETRIC_KEY.empty()) {
        otErr << __FUNCTION__ << ": Null: SYMMETRIC_KEY passed in!\n";
        return "";
    }
    if (PLAINTEXT.empty()) {
        otErr << __FUNCTION__ << ": Null: PLAINTEXT passed in!\n";
        return "";
    }
    const String strKey(SYMMETRIC_KEY);
    const String strPlaintext(PLAINTEXT);
    String strOutput;
    std::string strDisplay = "OTAPI: Password-protecting a plaintext.";
    const String otstrDisplay(strDisplay);
    const bool& bSuccess = OTSymmetricKey::Encrypt(
        strKey,
        strPlaintext,
        strOutput,
        &otstrDisplay);  // bBookends=true, pAlreadyHavePW=""

    if (!bSuccess) return "";

    std::string pBuf = strOutput.Get();

    return pBuf;
}

// Returns the PLAINTEXT.
//
std::string OTAPI_Exec::SymmetricDecrypt(
    const std::string& SYMMETRIC_KEY,
    const std::string& CIPHERTEXT_ENVELOPE) const
{
    if (SYMMETRIC_KEY.empty()) {
        otErr << __FUNCTION__ << ": Null: SYMMETRIC_KEY passed in!\n";
        return "";
    }
    if (CIPHERTEXT_ENVELOPE.empty()) {
        otErr << __FUNCTION__ << ": Null: CIPHERTEXT_ENVELOPE passed in!\n";
        return "";
    }
    const String strKey(SYMMETRIC_KEY);
    String strCiphertext(CIPHERTEXT_ENVELOPE);
    String strOutput;
    std::string strDisplay =
        "OTAPI: Decrypting a password-protected ciphertext.";
    const String otstrDisplay(strDisplay);
    const bool& bSuccess = OTSymmetricKey::Decrypt(
        strKey, strCiphertext, strOutput, &otstrDisplay);  // pAlreadyHavePW=""

    if (!bSuccess) return "";

    std::string pBuf = strOutput.Get();

    return pBuf;
}

/** OT-Sign a CONTRACT.  (First signature)

std::string OTAPI_Exec::SignContract(const std::string& SIGNER_NYM_ID, const
std::string& THE_CONTRACT);

Tries to instantiate the contract object, based on the string passed in.
Releases all signatures, and then signs the contract.
Returns the signed contract, or "" if failure.

NOTE: The actual OT functionality (Use Cases) NEVER requires you to sign via
this function. Why not? because, anytime a signature is needed on something,
the relevant OT API call will require you to pass in the Nym, and the API
already
signs internally wherever it deems appropriate. Thus, this function is only for
advanced uses, for OT-Scripts, server operators, etc.

*/
std::string OTAPI_Exec::SignContract(
    const std::string& SIGNER_NYM_ID,
    const std::string& THE_CONTRACT) const
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return "";
    }

    if (SIGNER_NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SIGNER_NYM_ID passed in!\n";
        return "";
    }
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }

    const String strContract(THE_CONTRACT);
    const Identifier theSignerNymID(SIGNER_NYM_ID);
    String strOutput;

    const bool& bSigned =
        OTAPI()->SignContract(theSignerNymID, strContract, strOutput);

    if (!bSigned || !strOutput.Exists()) return "";

    std::string pBuf = strOutput.Get();

    return pBuf;
}

// Instead of signing an existing contract, this is for just signing a flat
// message.
// Or, for example, for signing a new contract that has no signature yet. Let's
// say you
// have a ledger, for example, with no signatures yet. Pass "LEDGER" as the
// CONTRACT_TYPE
// and the resulting output will start like this: -----BEGIN OT SIGNED
// LEDGER----- ...
// Returns the signed output, or "".
//
std::string OTAPI_Exec::FlatSign(
    const std::string& SIGNER_NYM_ID,
    const std::string& THE_INPUT,
    const std::string& CONTRACT_TYPE) const
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return "";
    }

    if (SIGNER_NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SIGNER_NYM_ID passed in!\n";
        return "";
    }
    if (THE_INPUT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_INPUT passed in!\n";
        return "";
    }
    if (CONTRACT_TYPE.empty()) {
        otErr << __FUNCTION__ << ": Null: CONTRACT_TYPE passed in!\n";
        return "";
    }

    const String strContract(THE_INPUT);
    const String strContractType(CONTRACT_TYPE);
    const Identifier theSignerNymID(SIGNER_NYM_ID);
    String strOutput;

    const bool& bSigned = OTAPI()->FlatSign(
        theSignerNymID, strContract, strContractType, strOutput);

    if (!bSigned || !strOutput.Exists()) return "";

    std::string pBuf = strOutput.Get();

    return pBuf;
}

/** OT-Sign a CONTRACT.  (Add a signature)

std::string OTAPI_Exec::AddSignature(const std::string& SIGNER_NYM_ID, const
std::string& THE_CONTRACT);

Tries to instantiate the contract object, based on the string passed in.
Signs the contract, without releasing any signatures that are already there.
Returns the signed contract, or "" if failure.

NOTE: The actual OT functionality (Use Cases) NEVER requires you to sign via
this function. Why not? because, anytime a signature is needed on something,
the relevant OT API call will require you to pass in the Nym, and the API
already
signs internally wherever it deems appropriate. Thus, this function is only for
advanced uses, for OT-Scripts, server operators, etc.

Internally the C++ code is:
*/
std::string OTAPI_Exec::AddSignature(
    const std::string& SIGNER_NYM_ID,
    const std::string& THE_CONTRACT) const
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return "";
    }

    if (SIGNER_NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SIGNER_NYM_ID passed in!\n";
        return "";
    }
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }

    const String strContract(THE_CONTRACT);
    const Identifier theSignerNymID(SIGNER_NYM_ID);
    String strOutput;

    const bool& bSigned =
        OTAPI()->AddSignature(theSignerNymID, strContract, strOutput);

    if (!bSigned || !strOutput.Exists()) return "";

    std::string pBuf = strOutput.Get();

    return pBuf;
}

/** OT-Verify the signature on a CONTRACT stored in a string.

Returns bool -- true (1) or false (0)
*/
bool OTAPI_Exec::VerifySignature(
    const std::string& SIGNER_NYM_ID,
    const std::string& THE_CONTRACT) const
{
    if (SIGNER_NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SIGNER_NYM_ID passed in!\n";
        return false;
    }
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return false;
    }
    const String strContract(THE_CONTRACT);
    const Identifier theNymID(SIGNER_NYM_ID);
    const bool& bVerified = OTAPI()->VerifySignature(
        strContract, theNymID); /*ppContract="" (optional third parameter for
                                   retrieving loaded contract.)*/
    return bVerified ? true : false;
}

// Verify and Retrieve XML Contents.
//
// Pass in a contract and a user ID, and this function will:
// -- Load the contract up and verify it.
// -- Verify the user's signature on it.
// -- Remove the PGP-style bookends (the signatures, etc)
//    and return the XML contents of the contract in string form.
//
std::string OTAPI_Exec::VerifyAndRetrieveXMLContents(
    const std::string& THE_CONTRACT,
    const std::string& SIGNER_ID) const
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (SIGNER_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SIGNER_ID passed in!\n";
        return "";
    }

    const String strContract(THE_CONTRACT);
    const Identifier theSignerID(SIGNER_ID);
    String strOutput;

    if (false ==
        OTAPI()->VerifyAndRetrieveXMLContents(
            strContract, theSignerID, strOutput)) {
        otOut << __FUNCTION__ << ": Failure: "
                                 "OTAPI()->VerifyAndRetrieveXMLContents() "
                                 "returned false.\n";
        return "";
    }
    std::string pBuf = strOutput.Get();

    return pBuf;
}

// === Verify Account Receipt ===
// Returns bool. Verifies any asset account (intermediary files) against its own
// last signed receipt.
// Obviously this will fail for any new account that hasn't done any
// transactions yet, and thus has no receipts.
//
bool OTAPI_Exec::VerifyAccountReceipt(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCT_ID) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return false;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return false;
    }
    if (ACCT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCT_ID passed in!\n";
        return false;
    }

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theAcctID(ACCT_ID);
    const bool& bVerified =
        OTAPI()->VerifyAccountReceipt(theNotaryID, theNymID, theAcctID);
    return bVerified ? true : false;
}

// SET ACCOUNT NAME (client side only. Server cares not about such labels.)
//
// Returns true (1) or false (0)
//
bool OTAPI_Exec::SetAccountWallet_Name(
    const std::string& ACCT_ID,
    const std::string& SIGNER_NYM_ID,
    const std::string& ACCT_NEW_NAME) const
{
    if (ACCT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCT_ID passed in!\n";
        return false;
    }
    if (SIGNER_NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SIGNER_NYM_ID passed in!\n";
        return false;
    }
    if (ACCT_NEW_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCT_NEW_NAME passed in!\n";
        return false;
    }

    Identifier theAcctID(ACCT_ID), theSignerNymID(SIGNER_NYM_ID);
    String strAcctNewName(ACCT_NEW_NAME);

    return OTAPI()->SetAccount_Name(theAcctID, theSignerNymID, strAcctNewName);
}

// returns the account balance, based on account ID.
int64_t OTAPI_Exec::GetAccountWallet_Balance(const std::string& THE_ID) const
{
    if (THE_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_ID passed in!\n";
        return OT_ERROR_AMOUNT;
    }

    Identifier theID(THE_ID);
    Account* pAccount = OTAPI()->GetAccount(theID, __FUNCTION__);
    return nullptr == pAccount ? OT_ERROR_AMOUNT : pAccount->GetBalance();
}

// returns an account's "account type", (simple, issuer, etc.)
std::string OTAPI_Exec::GetAccountWallet_Type(const std::string& THE_ID) const
{
    if (THE_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_ID passed in!\n";
        return "";
    }

    Identifier theID(THE_ID);
    Account* pAccount = OTAPI()->GetAccount(theID, __FUNCTION__);
    if (nullptr == pAccount) return "";
    std::string pBuf = pAccount->GetTypeString();

    return pBuf;
}

// Returns an account's instrument definition ID.
// (Which is a hash of the contract used to issue the instrument definition.)
std::string OTAPI_Exec::GetAccountWallet_InstrumentDefinitionID(
    const std::string& THE_ID) const
{
    if (THE_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_ID passed in!\n";
        return "";
    }

    Identifier theID(THE_ID);
    Account* pAccount = OTAPI()->GetAccount(theID, __FUNCTION__);
    if (nullptr == pAccount) return "";
    Identifier theInstrumentDefinitionID(pAccount->GetInstrumentDefinitionID());

    String strInstrumentDefinitionID(theInstrumentDefinitionID);

    otWarn << __FUNCTION__ << ": Returning instrument definition "
           << strInstrumentDefinitionID << " for account " << THE_ID << "\n";

    std::string pBuf = strInstrumentDefinitionID.Get();

    return pBuf;
}

// Returns an account's Notary ID.
// (Which is a hash of the server contract.)
std::string OTAPI_Exec::GetAccountWallet_NotaryID(
    const std::string& THE_ID) const
{
    if (THE_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_ID passed in!\n";
        return "";
    }

    Identifier theID(THE_ID);
    Account* pAccount = OTAPI()->GetAccount(theID, __FUNCTION__);
    if (nullptr == pAccount) return "";
    Identifier theNotaryID(pAccount->GetPurportedNotaryID());
    String strNotaryID(theNotaryID);

    std::string pBuf = strNotaryID.Get();

    return pBuf;
}

// Returns an account's Nym ID.
// (Which is a hash of the Nym's public key for the owner of this account.)
std::string OTAPI_Exec::GetAccountWallet_NymID(const std::string& THE_ID) const
{
    if (THE_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_ID passed in!\n";
        return "";
    }

    const Identifier theID(THE_ID);

    Account* pAccount = OTAPI()->GetAccount(theID, __FUNCTION__);
    if (nullptr == pAccount) return "";
    Identifier theNymID(pAccount->GetNymID());
    String strNymID(theNymID);

    std::string pBuf = strNymID.Get();

    return pBuf;
}

/*

WRITE A CHEQUE  --- (Returns the cheque in string form.)

==> OTAPI_Exec::WriteCheque() internally constructs an Cheque and issues it,
like so:

Cheque theCheque( NOTARY_ID, INSTRUMENT_DEFINITION_ID );

theCheque.IssueCheque( AMOUNT // The amount of the cheque, in string form, which
OTAPI
// will convert to a int64_t integer. Negative amounts
// allowed, since that is how OT implements invoices.
// (An invoice is just a cheque with a negative amount.)

lTransactionNumber,   // The API will supply this automatically, as long as
// there are some transaction numbers in the wallet. (Call
// OTAPI_Exec::getTransactionNumber() if your wallet needs more.)

VALID_FROM, VALID_TO, // Valid date range (in seconds since Jan 1970...)

ACCOUNT_ID, NYM_ID,  // Nym ID and Acct ID for SENDER.

CHEQUE_MEMO,  // The memo for the cheque.

RECIPIENT_NYM_ID); // Recipient Nym ID is optional. (You can use an
// empty string here, to write a blank cheque.)
*/
std::string OTAPI_Exec::WriteCheque(
    const std::string& NOTARY_ID,
    const int64_t& CHEQUE_AMOUNT,
    const time64_t& VALID_FROM,
    const time64_t& VALID_TO,
    const std::string& SENDER_ACCT_ID,
    const std::string& SENDER_NYM_ID,
    const std::string& CHEQUE_MEMO,
    const std::string& RECIPIENT_NYM_ID) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (0 == CHEQUE_AMOUNT) {
        otErr << __FUNCTION__ << ": Null: CHEQUE_AMOUNT passed in!\n";
        return "";
    }
    if (OT_TIME_ZERO > VALID_FROM) {
        otErr << __FUNCTION__ << ": Null: VALID_FROM passed in!\n";
        return "";
    }
    if (OT_TIME_ZERO > VALID_TO) {
        otErr << __FUNCTION__ << ": Null: VALID_TO passed in!\n";
        return "";
    }
    if (SENDER_ACCT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SENDER_ACCT_ID passed in!\n";
        return "";
    }
    if (SENDER_NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SENDER_NYM_ID passed in!\n";
        return "";
    }
    //    if (CHEQUE_MEMO.empty())        { otErr << __FUNCTION__ << ": Null:
    // CHEQUE_MEMO passed
    // in!\n"; OT_FAIL; } // optional
    //    if (RECIPIENT_NYM_ID.empty())    { otErr << __FUNCTION__ << ": Null:
    // RECIPIENT_NYM_ID passed
    // in!\n"; OT_FAIL; } // optional

    const int64_t lAmount = CHEQUE_AMOUNT;
    const time64_t time_From = static_cast<time64_t>(VALID_FROM),
                   time_To = static_cast<time64_t>(VALID_TO);

    const Identifier theNotaryID(NOTARY_ID);
    const Identifier theSenderAcctID(SENDER_ACCT_ID);
    const Identifier theSenderNymID(SENDER_NYM_ID);
    Identifier theRecipientNymID;
    bool bHasRecipient = !RECIPIENT_NYM_ID.empty();
    if (bHasRecipient) theRecipientNymID.SetString(String(RECIPIENT_NYM_ID));
    String strMemo;

    if (!CHEQUE_MEMO.empty()) strMemo.Set(String(CHEQUE_MEMO));

    std::unique_ptr<Cheque> pCheque(
        OTAPI()->WriteCheque(
            theNotaryID,
            static_cast<int64_t>(lAmount),
            time_From,
            time_To,
            theSenderAcctID,
            theSenderNymID,
            strMemo,
            bHasRecipient ? &(theRecipientNymID) : nullptr));

    if (nullptr == pCheque) {
        otErr << __FUNCTION__ << ": OT_API::WriteCheque failed.\n";
        return "";
    }
    // At this point, I know pCheque is good (and will be cleaned up
    // automatically.)

    String strCheque(*pCheque);  // Extract the cheque to string form.

    std::string pBuf = strCheque.Get();

    return pBuf;
}

bool OTAPI_Exec::DiscardCheque(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCT_ID,
    const std::string& THE_CHEQUE) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return false;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return false;
    }
    if (ACCT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCT_ID passed in!\n";
        return false;
    }
    if (THE_CHEQUE.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CHEQUE passed in!\n";
        return false;
    }

    Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID), theAcctID(ACCT_ID);
    String strCheque(THE_CHEQUE);

    return OTAPI()->DiscardCheque(theNotaryID, theNymID, theAcctID, strCheque);
}

// PROPOSE PAYMENT PLAN
//
// (Return as STRING)
//
// PARAMETER NOTES:
// -- Payment Plan Delay, and Payment Plan Period, both default to 30 days (if
// you pass 0.)
//
// -- Payment Plan Length, and Payment Plan Max Payments, both default to 0,
// which means
//    no maximum length and no maximum number of payments.
//
// FYI, the payment plan creation process (finally) is:
//
// 1) Payment plan is written, and signed, by the recipient.  (This function:
// OTAPI_Exec::ProposePaymentPlan)
// 2) He sends it to the sender, who signs it and submits it.
// (OTAPI_Exec::ConfirmPaymentPlan and OTAPI_Exec::depositPaymentPlan)
// 3) The server loads the recipient nym to verify the transaction
//    number. The sender also had to burn a transaction number (to
//    submit it) so now, both have verified trns#s in this way.
//
std::string OTAPI_Exec::ProposePaymentPlan(
    const std::string& NOTARY_ID,
    const time64_t& VALID_FROM,  // Default (0 or nullptr) == current time
                                 // measured
                                 // in seconds since Jan 1970.
    const time64_t& VALID_TO,    // Default (0 or nullptr) == no expiry / cancel
    // anytime. Otherwise this is ADDED to VALID_FROM
    // (it's a length.)
    const std::string& SENDER_ACCT_ID,  // Mandatory parameters. UPDATE: Making
                                        // sender Acct optional here.
    const std::string& SENDER_NYM_ID,   // Both sender and recipient must sign
                                        // before submitting.
    const std::string& PLAN_CONSIDERATION,  // Like a memo.
    const std::string& RECIPIENT_ACCT_ID,   // NOT optional.
    const std::string& RECIPIENT_NYM_ID,  // Both sender and recipient must sign
                                          // before submitting.
    const int64_t& INITIAL_PAYMENT_AMOUNT,  // zero or "" is no initial payment.
    const time64_t& INITIAL_PAYMENT_DELAY,  // seconds from creation date.
                                            // Default is zero or "".
    const int64_t& PAYMENT_PLAN_AMOUNT,   // Zero or "" is no regular payments.
    const time64_t& PAYMENT_PLAN_DELAY,   // No. of seconds from creation date.
                                          // Default is zero or "".
    const time64_t& PAYMENT_PLAN_PERIOD,  // No. of seconds between payments.
                                          // Default is zero or "".
    const time64_t& PAYMENT_PLAN_LENGTH,  // In seconds. Defaults to 0 or "" (no
                                          // maximum length.)
    const int32_t& PAYMENT_PLAN_MAX_PAYMENTS  // Integer. Defaults to 0 or ""
                                              // (no
                                              // maximum payments.)
    ) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (OT_TIME_ZERO > VALID_FROM) {
        otErr << __FUNCTION__ << ": Negative: VALID_FROM passed in!\n";
        return "";
    }
    if (OT_TIME_ZERO > VALID_TO) {
        otErr << __FUNCTION__ << ": Negative: VALID_TO passed in!\n";
        return "";
    }
    // NOTE: Making this optional for this step. (Since sender account may
    // not be known until the customer receives / confirms the payment plan.)
    //    if (SENDER_ACCT_ID.empty()) {
    //        otErr << __FUNCTION__ << ": Null: SENDER_ACCT_ID passed in!\n";
    //        return "";
    //    }
    if (SENDER_NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SENDER_NYM_ID passed in!\n";
        return "";
    }
    if (PLAN_CONSIDERATION.empty()) {
        otErr << __FUNCTION__ << ": Null: PLAN_CONSIDERATION passed in!\n";
        return "";
    }
    if (RECIPIENT_ACCT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: RECIPIENT_ACCT_ID passed in!\n";
        return "";
    }
    if (RECIPIENT_NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: RECIPIENT_NYM_ID passed in!\n";
        return "";
    }
    if (0 > INITIAL_PAYMENT_AMOUNT) {
        otErr << __FUNCTION__
              << ": Negative: INITIAL_PAYMENT_AMOUNT passed in!\n";
        return "";
    }
    if (OT_TIME_ZERO > INITIAL_PAYMENT_DELAY) {
        otErr << __FUNCTION__
              << ": Negative: INITIAL_PAYMENT_DELAY passed in!\n";
        return "";
    }
    if (0 > PAYMENT_PLAN_AMOUNT) {
        otErr << __FUNCTION__ << ": Negative: PAYMENT_PLAN_AMOUNT passed in!\n";
        return "";
    }
    if (OT_TIME_ZERO > PAYMENT_PLAN_DELAY) {
        otErr << __FUNCTION__ << ": Negative: PAYMENT_PLAN_DELAY passed in!\n";
        return "";
    }
    if (OT_TIME_ZERO > PAYMENT_PLAN_PERIOD) {
        otErr << __FUNCTION__ << ": Negative: PAYMENT_PLAN_PERIOD passed in!\n";
        return "";
    }
    if (OT_TIME_ZERO > PAYMENT_PLAN_LENGTH) {
        otErr << __FUNCTION__ << ": Negative: PAYMENT_PLAN_LENGTH passed in!\n";
        return "";
    }
    if (0 > PAYMENT_PLAN_MAX_PAYMENTS) {
        otErr << __FUNCTION__
              << ": Negative: PAYMENT_PLAN_MAX_PAYMENTS passed in!\n";
        return "";
    }

    std::unique_ptr<Identifier> angelSenderAcctId;

    if (!SENDER_ACCT_ID.empty())
        angelSenderAcctId.reset(new Identifier(SENDER_ACCT_ID));

    std::unique_ptr<OTPaymentPlan> pPlan(
        OTAPI()->ProposePaymentPlan(
            Identifier(NOTARY_ID),
            VALID_FROM,  // Default (0) == NOW
            VALID_TO,    // Default (0) == no expiry / cancel anytime
            // We're making this acct optional here.
            // (Customer acct is unknown until confirmation by customer.)
            angelSenderAcctId.get(),
            Identifier(SENDER_NYM_ID),
            PLAN_CONSIDERATION.empty()
                ? "(Consideration for the agreement between "
                  "the parties is meant to be recorded "
                  "here.)"
                // Like a memo.
                : String(PLAN_CONSIDERATION),
            Identifier(RECIPIENT_ACCT_ID),
            Identifier(RECIPIENT_NYM_ID),
            static_cast<int64_t>(INITIAL_PAYMENT_AMOUNT),
            INITIAL_PAYMENT_DELAY,
            static_cast<int64_t>(PAYMENT_PLAN_AMOUNT),
            PAYMENT_PLAN_DELAY,
            PAYMENT_PLAN_PERIOD,
            PAYMENT_PLAN_LENGTH,
            static_cast<int32_t>(PAYMENT_PLAN_MAX_PAYMENTS)));
    if (nullptr == pPlan) {
        otErr << __FUNCTION__
              << ": failed in OTAPI_Exec::ProposePaymentPlan.\n";
        return "";
    }
    // At this point, I know pPlan is good (and will be cleaned up
    // automatically.)
    String strOutput(*pPlan);  // Extract the payment plan to string form.
    std::string pBuf = strOutput.Get();
    return pBuf;
}

// TODO RESUME:
// 1. Find out what DiscardCheque is used for.
// 2. Make sure we don't need a "Discard Payment Plan" and "Discard Smart
// Contract" as well.
// 3. Add "EasyProposePlan" to the rest of the API like OTAPI_Basic
// 4. Add 'propose' and 'confirm' (for payment plan) commands to opentxs client.
// 5. Finish opentxs basket commands, so opentxs is COMPLETE.

std::string OTAPI_Exec::EasyProposePlan(
    const std::string& NOTARY_ID,
    const std::string& DATE_RANGE,  // "from,to"  Default 'from' (0 or "") ==
                                    // NOW, and default 'to' (0 or "") == no
                                    // expiry / cancel anytime
    const std::string& SENDER_ACCT_ID,  // Mandatory parameters. UPDATE: Making
                                        // sender acct optional here since it
                                        // may not be known at this point.
    const std::string& SENDER_NYM_ID,   // Both sender and recipient must sign
                                        // before submitting.
    const std::string& PLAN_CONSIDERATION,  // Like a memo.
    const std::string& RECIPIENT_ACCT_ID,   // NOT optional.
    const std::string& RECIPIENT_NYM_ID,  // Both sender and recipient must sign
                                          // before submitting.
    const std::string& INITIAL_PAYMENT,   // "amount,delay"  Default 'amount' (0
    // or "") == no initial payment. Default
    // 'delay' (0 or nullptr) is seconds from
    // creation date.
    const std::string& PAYMENT_PLAN,  // "amount,delay,period" 'amount' is a
                                      // recurring payment. 'delay' and 'period'
                                      // cause 30 days if you pass 0 or "".
    const std::string& PLAN_EXPIRY    // "length,number" 'length' is maximum
    // lifetime in seconds. 'number' is maximum
    // number of payments in seconds. 0 or "" is
    // unlimited.
    ) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    // We're making this parameter optional.
    //    if (SENDER_ACCT_ID.empty()) {
    //        otErr << __FUNCTION__ << ": Null: SENDER_ACCT_ID passed in!\n";
    //        return "";
    //    }
    if (SENDER_NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SENDER_NYM_ID passed in!\n";
        return "";
    }
    if (PLAN_CONSIDERATION.empty()) {
        otErr << __FUNCTION__ << ": Null: PLAN_CONSIDERATION passed in!\n";
        return "";
    }
    if (RECIPIENT_ACCT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: RECIPIENT_ACCT_ID passed in!\n";
        return "";
    }
    if (RECIPIENT_NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: RECIPIENT_NYM_ID passed in!\n";
        return "";
    }
    time64_t VALID_FROM = OT_TIME_ZERO;
    time64_t VALID_TO = OT_TIME_ZERO;
    int64_t INITIAL_PAYMENT_AMOUNT = 0;
    time64_t INITIAL_PAYMENT_DELAY = OT_TIME_ZERO;
    int64_t PAYMENT_PLAN_AMOUNT = 0;
    time64_t PAYMENT_PLAN_DELAY = OT_TIME_ZERO;
    time64_t PAYMENT_PLAN_PERIOD = OT_TIME_ZERO;
    time64_t PAYMENT_PLAN_LENGTH = OT_TIME_ZERO;
    int32_t PAYMENT_PLAN_MAX_PAYMENTS = 0;
    if (!DATE_RANGE.empty()) {
        NumList theList;
        const String otstrNumList(DATE_RANGE);
        theList.Add(otstrNumList);
        // VALID_FROM
        if (theList.Count() > 0) {
            int64_t lVal = 0;
            if (theList.Peek(lVal)) VALID_FROM = OTTimeGetTimeFromSeconds(lVal);
            theList.Pop();
        }
        // VALID_TO
        if (theList.Count() > 0) {
            int64_t lVal = 0;
            if (theList.Peek(lVal)) VALID_TO = OTTimeGetTimeFromSeconds(lVal);
            theList.Pop();
        }
    }
    if (!INITIAL_PAYMENT.empty()) {
        NumList theList;
        const String otstrNumList(INITIAL_PAYMENT);
        theList.Add(otstrNumList);
        // INITIAL_PAYMENT_AMOUNT
        if (theList.Count() > 0) {
            int64_t lVal = 0;
            if (theList.Peek(lVal)) INITIAL_PAYMENT_AMOUNT = lVal;
            theList.Pop();
        }
        // INITIAL_PAYMENT_DELAY
        if (theList.Count() > 0) {
            int64_t lVal = 0;
            if (theList.Peek(lVal))
                INITIAL_PAYMENT_DELAY = OTTimeGetTimeFromSeconds(lVal);
            theList.Pop();
        }
    }
    if (!PAYMENT_PLAN.empty()) {
        NumList theList;
        const String otstrNumList(PAYMENT_PLAN);
        theList.Add(otstrNumList);
        // PAYMENT_PLAN_AMOUNT
        if (theList.Count() > 0) {
            int64_t lVal = 0;
            if (theList.Peek(lVal)) PAYMENT_PLAN_AMOUNT = lVal;
            theList.Pop();
        }
        // PAYMENT_PLAN_DELAY
        if (theList.Count() > 0) {
            int64_t lVal = 0;
            if (theList.Peek(lVal))
                PAYMENT_PLAN_DELAY = OTTimeGetTimeFromSeconds(lVal);
            theList.Pop();
        }
        // PAYMENT_PLAN_PERIOD
        if (theList.Count() > 0) {
            int64_t lVal = 0;
            if (theList.Peek(lVal))
                PAYMENT_PLAN_PERIOD = OTTimeGetTimeFromSeconds(lVal);
            theList.Pop();
        }
    }
    if (!PLAN_EXPIRY.empty()) {
        NumList theList;
        const String otstrNumList(PLAN_EXPIRY);
        theList.Add(otstrNumList);
        // PAYMENT_PLAN_LENGTH
        if (theList.Count() > 0) {
            int64_t lVal = 0;
            if (theList.Peek(lVal))
                PAYMENT_PLAN_LENGTH = OTTimeGetTimeFromSeconds(lVal);
            theList.Pop();
        }
        // PAYMENT_PLAN_MAX_PAYMENTS
        if (theList.Count() > 0) {
            int64_t lVal = 0;
            if (theList.Peek(lVal))
                PAYMENT_PLAN_MAX_PAYMENTS = static_cast<int32_t>(lVal);
            theList.Pop();
        }
    }
    return OTAPI_Exec::ProposePaymentPlan(
        NOTARY_ID,
        VALID_FROM,
        VALID_TO,
        SENDER_ACCT_ID,
        SENDER_NYM_ID,
        PLAN_CONSIDERATION,
        RECIPIENT_ACCT_ID,
        RECIPIENT_NYM_ID,
        INITIAL_PAYMENT_AMOUNT,
        INITIAL_PAYMENT_DELAY,
        PAYMENT_PLAN_AMOUNT,
        PAYMENT_PLAN_DELAY,
        PAYMENT_PLAN_PERIOD,
        PAYMENT_PLAN_LENGTH,
        PAYMENT_PLAN_MAX_PAYMENTS);
}

// Called by CUSTOMER.
// "PAYMENT_PLAN" is the output of the above function (ProposePaymentPlan)
// Customer should call OTAPI_Exec::depositPaymentPlan after this.
//
std::string OTAPI_Exec::ConfirmPaymentPlan(
    const std::string& NOTARY_ID,
    const std::string& SENDER_NYM_ID,
    const std::string& SENDER_ACCT_ID,
    const std::string& RECIPIENT_NYM_ID,
    const std::string& PAYMENT_PLAN) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (SENDER_ACCT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SENDER_ACCT_ID passed in!\n";
        return "";
    }
    if (SENDER_NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SENDER_NYM_ID passed in!\n";
        return "";
    }
    if (RECIPIENT_NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: RECIPIENT_NYM_ID passed in!\n";
        return "";
    }
    if (PAYMENT_PLAN.empty()) {
        otErr << __FUNCTION__ << ": Null: PAYMENT_PLAN passed in!\n";
        return "";
    }

    const Identifier theNotaryID(NOTARY_ID);
    const Identifier theSenderNymID(SENDER_NYM_ID);
    const Identifier theSenderAcctID(SENDER_ACCT_ID);
    const Identifier theRecipientNymID(RECIPIENT_NYM_ID);

    OTPaymentPlan thePlan;
    const String strPlan(PAYMENT_PLAN);

    if (!strPlan.Exists() ||
        (false == thePlan.LoadContractFromString(strPlan))) {
        otOut << __FUNCTION__
              << ": Failure loading payment plan from string.\n";
        return "";
    }
    bool bConfirmed = OTAPI()->ConfirmPaymentPlan(
        theNotaryID,
        theSenderNymID,
        theSenderAcctID,
        theRecipientNymID,
        thePlan);
    if (!bConfirmed) {
        otOut << __FUNCTION__
              << ": failed in OTAPI_Exec::ConfirmPaymentPlan().\n";
        return "";
    }

    String strOutput(thePlan);  // Extract the payment plan to string form.

    return strOutput.Get();
}

// RETURNS:  the Smart Contract itself. (Or "".)
//
std::string OTAPI_Exec::Create_SmartContract(
    const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                       // signing
                                       // at this point32_t is only to cause a
                                       // save.)
    const time64_t& VALID_FROM,        // Default (0 or "") == NOW
    const time64_t& VALID_TO,  // Default (0 or "") == no expiry / cancel
                               // anytime
    bool SPECIFY_ASSETS,       // This means asset type IDs must be provided for
                               // every named account.
    bool SPECIFY_PARTIES       // This means Nym IDs must be provided for every
                               // party.
    ) const
{
    if (SIGNER_NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SIGNER_NYM_ID passed in!\n";
        return "";
    }
    if (OT_TIME_ZERO > VALID_FROM) {
        otErr << __FUNCTION__ << ": Negative: VALID_FROM passed in!\n";
        return "";
    }
    if (OT_TIME_ZERO > VALID_TO) {
        otErr << __FUNCTION__ << ": Negative: VALID_TO passed in!\n";
        return "";
    }
    const Identifier theSignerNymID(SIGNER_NYM_ID);
    time64_t tValidFrom = VALID_FROM;
    time64_t tValidTo = VALID_TO;
    String strOutput;

    const bool& bCreated = OTAPI()->Create_SmartContract(
        theSignerNymID,
        tValidFrom,      // Default (0 or "") == NOW
        tValidTo,        // Default (0 or "") == no expiry / cancel anytime
        SPECIFY_ASSETS,  // This means asset type IDs must be provided for every
                         // named account.
        SPECIFY_PARTIES,  // This means Nym IDs must be provided for every
                          // party.
        strOutput);
    if (!bCreated || !strOutput.Exists()) return "";
    // Success!
    //
    return strOutput.Get();
}

bool OTAPI_Exec::Smart_ArePartiesSpecified(
    const std::string& THE_CONTRACT) const
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return false;
    }
    const String strContract(THE_CONTRACT);

    return OTAPI()->Smart_ArePartiesSpecified(strContract);
}

bool OTAPI_Exec::Smart_AreAssetTypesSpecified(
    const std::string& THE_CONTRACT) const
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return false;
    }
    const String strContract(THE_CONTRACT);

    return OTAPI()->Smart_AreAssetTypesSpecified(strContract);
}

// RETURNS:  the Smart Contract itself. (Or "".)
//
std::string OTAPI_Exec::SmartContract_SetDates(
    const std::string& THE_CONTRACT,   // The contract, about to have the
                                       // dates changed on it.
    const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                       // signing at this point is only to
                                       // cause a save.)
    const time64_t& VALID_FROM,        // Default (0 or nullptr) == NOW
    const time64_t& VALID_TO) const    // Default (0 or nullptr) == no expiry /
                                       // cancel
                                       // anytime.
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (SIGNER_NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SIGNER_NYM_ID passed in!\n";
        return "";
    }
    if (OT_TIME_ZERO > VALID_FROM) {
        otErr << __FUNCTION__ << ": Negative: VALID_FROM passed in!\n";
        return "";
    }
    if (OT_TIME_ZERO > VALID_TO) {
        otErr << __FUNCTION__ << ": Negative: VALID_TO passed in!\n";
        return "";
    }
    const String strContract(THE_CONTRACT);
    const Identifier theSignerNymID(SIGNER_NYM_ID);
    time64_t tValidFrom = VALID_FROM;
    time64_t tValidTo = VALID_TO;
    String strOutput;

    const bool& bAdded = OTAPI()->SmartContract_SetDates(
        strContract,     // The contract, about to have the dates changed on it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // point is only to cause a save.)
        tValidFrom,      // Default (0 or "") == NOW
        tValidTo,        // Default (0 or "") == no expiry / cancel anytime
        strOutput);
    if (!bAdded || !strOutput.Exists()) return "";
    // Success!
    //
    return strOutput.Get();
}

//
// todo: Someday add a parameter here BYLAW_LANGUAGE so that people can use
// custom languages in their scripts.  For now I have a default language, so
// I'll just make that the default. (There's only one language right now
// anyway.)
//
// returns: the updated smart contract (or "")
std::string OTAPI_Exec::SmartContract_AddBylaw(
    const std::string& THE_CONTRACT,   // The contract, about to have the bylaw
                                       // added to it.
    const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                       // signing
                                       // at this point is only to cause a
                                       // save.)
    const std::string& BYLAW_NAME) const  // The Bylaw's NAME as referenced in
                                          // the
// smart contract. (And the scripts...)
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (SIGNER_NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SIGNER_NYM_ID passed in!\n";
        return "";
    }
    if (BYLAW_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: BYLAW_NAME passed in!\n";
        return "";
    }
    const String strContract(THE_CONTRACT), strBylawName(BYLAW_NAME);
    const Identifier theSignerNymID(SIGNER_NYM_ID);
    String strOutput;

    const bool& bAdded = OTAPI()->SmartContract_AddBylaw(
        strContract,     // The contract, about to have the bylaw added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // point32_t is only to cause a save.)
        strBylawName,  // The Bylaw's NAME as referenced in the smart contract.
                       // (And the scripts...)
        strOutput);
    if (!bAdded || !strOutput.Exists()) return "";
    // Success!
    //
    return strOutput.Get();
}

// returns: the updated smart contract (or "")
std::string OTAPI_Exec::SmartContract_RemoveBylaw(
    const std::string& THE_CONTRACT,   // The contract, about to have the bylaw
                                       // removed from it.
    const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                       // signing
                                       // at this point32_t is only to cause a
                                       // save.)
    const std::string& BYLAW_NAME) const  // The Bylaw's NAME as referenced in
                                          // the
// smart contract. (And the scripts...)
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (SIGNER_NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SIGNER_NYM_ID passed in!\n";
        return "";
    }
    if (BYLAW_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: BYLAW_NAME passed in!\n";
        return "";
    }
    const String strContract(THE_CONTRACT), strBylawName(BYLAW_NAME);
    const Identifier theSignerNymID(SIGNER_NYM_ID);
    String strOutput;

    const bool& bAdded = OTAPI()->SmartContract_RemoveBylaw(
        strContract,     // The contract, about to have the bylaw added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // point32_t is only to cause a save.)
        strBylawName,  // The Bylaw's NAME as referenced in the smart contract.
                       // (And the scripts...)
        strOutput);
    if (!bAdded || !strOutput.Exists()) return "";
    // Success!
    //
    return strOutput.Get();
}

// returns: the updated smart contract (or "")
std::string OTAPI_Exec::SmartContract_AddClause(
    const std::string& THE_CONTRACT,   // The contract, about to have the clause
                                       // added to it.
    const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                       // signing
                                       // at this point32_t is only to cause a
                                       // save.)
    const std::string& BYLAW_NAME,   // Should already be on the contract. (This
                                     // way we can find it.)
    const std::string& CLAUSE_NAME,  // The Clause's name as referenced in the
                                     // smart contract. (And the scripts...)
    const std::string& SOURCE_CODE) const  // The actual source code for the
                                           // clause.
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (SIGNER_NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SIGNER_NYM_ID passed in!\n";
        return "";
    }
    if (BYLAW_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: BYLAW_NAME passed in!\n";
        return "";
    }
    if (CLAUSE_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: CLAUSE_NAME passed in!\n";
        return "";
    }

    const String strContract(THE_CONTRACT), strBylawName(BYLAW_NAME),
        strClauseName(CLAUSE_NAME), strSourceCode(SOURCE_CODE);
    const Identifier theSignerNymID(SIGNER_NYM_ID);
    String strOutput;

    const bool& bAdded = OTAPI()->SmartContract_AddClause(
        strContract,     // The contract, about to have the clause added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // point32_t is only to cause a save.)
        strBylawName,    // Should already be on the contract. (This way we can
                         // find it.)
        strClauseName,   // The Clause's name as referenced in the smart
                         // contract.
                         // (And the scripts...)
        strSourceCode,   // The actual source code for the clause.
        strOutput);
    if (!bAdded || !strOutput.Exists()) return "";
    // Success!
    //
    return strOutput.Get();
}

// returns: the updated smart contract (or "")
std::string OTAPI_Exec::SmartContract_UpdateClause(
    const std::string& THE_CONTRACT,   // The contract, about to have the clause
                                       // updated on it.
    const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                       // signing
                                       // at this point32_t is only to cause a
                                       // save.)
    const std::string& BYLAW_NAME,   // Should already be on the contract. (This
                                     // way we can find it.)
    const std::string& CLAUSE_NAME,  // The Clause's name as referenced in the
                                     // smart contract. (And the scripts...)
    const std::string& SOURCE_CODE) const  // The actual source code for the
                                           // clause.
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (SIGNER_NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SIGNER_NYM_ID passed in!\n";
        return "";
    }
    if (BYLAW_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: BYLAW_NAME passed in!\n";
        return "";
    }
    if (CLAUSE_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: CLAUSE_NAME passed in!\n";
        return "";
    }
    if (SOURCE_CODE.empty()) {
        otErr << __FUNCTION__ << ": Null: SOURCE_CODE passed in!\n";
        return "";
    }

    const String strContract(THE_CONTRACT), strBylawName(BYLAW_NAME),
        strClauseName(CLAUSE_NAME), strSourceCode(SOURCE_CODE);
    const Identifier theSignerNymID(SIGNER_NYM_ID);
    String strOutput;

    const bool& bAdded = OTAPI()->SmartContract_UpdateClause(
        strContract,     // The contract, about to have the clause added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // point32_t is only to cause a save.)
        strBylawName,    // Should already be on the contract. (This way we can
                         // find it.)
        strClauseName,   // The Clause's name as referenced in the smart
                         // contract.
                         // (And the scripts...)
        strSourceCode,   // The actual source code for the clause.
        strOutput);
    if (!bAdded || !strOutput.Exists()) return "";
    // Success!
    //
    return strOutput.Get();
}

// returns: the updated smart contract (or "")
std::string OTAPI_Exec::SmartContract_RemoveClause(
    const std::string& THE_CONTRACT,   // The contract, about to have the clause
                                       // added to it.
    const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                       // signing
                                       // at this point32_t is only to cause a
                                       // save.)
    const std::string& BYLAW_NAME,  // Should already be on the contract. (This
                                    // way we can find it.)
    const std::string& CLAUSE_NAME) const
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (SIGNER_NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SIGNER_NYM_ID passed in!\n";
        return "";
    }
    if (BYLAW_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: BYLAW_NAME passed in!\n";
        return "";
    }
    if (CLAUSE_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: CLAUSE_NAME passed in!\n";
        return "";
    }
    const String strContract(THE_CONTRACT), strBylawName(BYLAW_NAME),
        strClauseName(CLAUSE_NAME);
    const Identifier theSignerNymID(SIGNER_NYM_ID);
    String strOutput;

    const bool& bAdded = OTAPI()->SmartContract_RemoveClause(
        strContract,     // The contract, about to have the clause added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // point32_t is only to cause a save.)
        strBylawName,    // Should already be on the contract. (This way we can
                         // find it.)
        strClauseName,   // The Clause's name as referenced in the smart
                         // contract.
                         // (And the scripts...)
        strOutput);
    if (!bAdded || !strOutput.Exists()) return "";
    // Success!
    //
    return strOutput.Get();
}

// returns: the updated smart contract (or "")
std::string OTAPI_Exec::SmartContract_AddVariable(
    const std::string& THE_CONTRACT,   // The contract, about to have the
                                       // variable
                                       // added to it.
    const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                       // signing
                                       // at this point32_t is only to cause a
                                       // save.)
    const std::string& BYLAW_NAME,  // Should already be on the contract. (This
                                    // way we can find it.)
    const std::string& VAR_NAME,    // The Variable's name as referenced in the
                                    // smart contract. (And the scripts...)
    const std::string& VAR_ACCESS,  // "constant", "persistent", or "important".
    const std::string& VAR_TYPE,    // "string", "int64_t", or "bool"
    const std::string& VAR_VALUE) const  // Contains a string. If type is
                                         // int64_t,
// StringToLong() will be used to convert
// value to a int64_t. If type is bool, the
// strings "true" or "false" are expected here
// in order to convert to a bool.
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (SIGNER_NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SIGNER_NYM_ID passed in!\n";
        return "";
    }
    if (BYLAW_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: BYLAW_NAME passed in!\n";
        return "";
    }
    if (VAR_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: VAR_NAME passed in!\n";
        return "";
    }
    if (VAR_ACCESS.empty()) {
        otErr << __FUNCTION__ << ": Null: VAR_ACCESS passed in!\n";
        return "";
    }
    if (VAR_TYPE.empty()) {
        otErr << __FUNCTION__ << ": Null: VAR_TYPE passed in!\n";
        return "";
    }
    //    if (VAR_VALUE.empty())     { otErr << __FUNCTION__ << ": Null:
    // VAR_VALUE passed
    // in!\n"; OT_FAIL; }
    const String strContract(THE_CONTRACT), strBylawName(BYLAW_NAME),
        strVarName(VAR_NAME), strVarAccess(VAR_ACCESS), strVarType(VAR_TYPE),
        strVarValue(VAR_VALUE);
    const Identifier theSignerNymID(SIGNER_NYM_ID);
    String strOutput;

    const bool& bAdded = OTAPI()->SmartContract_AddVariable(
        strContract,     // The contract, about to have the clause added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // point32_t is only to cause a save.)
        strBylawName,    // Should already be on the contract. (This way we can
                         // find it.)
        strVarName,  // The Variable's name as referenced in the smart contract.
                     // (And the scripts...)
        strVarAccess,  // "constant", "persistent", or "important".
        strVarType,    // "string", "int64_t", or "bool"
        strVarValue,   // Contains a string. If type is int64_t, StringToLong()
                       // will be used to convert value to a int64_t. If type is
        // bool, the strings "true" or "false" are expected here in
        // order to convert to a bool.
        strOutput);
    if (!bAdded || !strOutput.Exists()) return "";
    // Success!
    //
    return strOutput.Get();
}

// returns: the updated smart contract (or "")
std::string OTAPI_Exec::SmartContract_RemoveVariable(
    const std::string& THE_CONTRACT,   // The contract, about to have the
                                       // variable
                                       // added to it.
    const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                       // signing
                                       // at this point32_t is only to cause a
                                       // save.)
    const std::string& BYLAW_NAME,  // Should already be on the contract. (This
                                    // way we can find it.)
    const std::string& VAR_NAME     // The Variable's name as referenced in the
                                    // smart contract. (And the scripts...)
    ) const
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (SIGNER_NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SIGNER_NYM_ID passed in!\n";
        return "";
    }
    if (BYLAW_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: BYLAW_NAME passed in!\n";
        return "";
    }
    if (VAR_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: VAR_NAME passed in!\n";
        return "";
    }

    const String strContract(THE_CONTRACT), strBylawName(BYLAW_NAME),
        strVarName(VAR_NAME);
    const Identifier theSignerNymID(SIGNER_NYM_ID);
    String strOutput;

    const bool& bAdded = OTAPI()->SmartContract_RemoveVariable(
        strContract,     // The contract, about to have the clause added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // point32_t is only to cause a save.)
        strBylawName,    // Should already be on the contract. (This way we can
                         // find it.)
        strVarName,  // The Variable's name as referenced in the smart contract.
                     // (And the scripts...)
        strOutput);
    if (!bAdded || !strOutput.Exists()) return "";
    // Success!
    //
    return strOutput.Get();
}

// returns: the updated smart contract (or "")
std::string OTAPI_Exec::SmartContract_AddCallback(
    const std::string& THE_CONTRACT,   // The contract, about to have the
                                       // callback
                                       // added to it.
    const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                       // signing
                                       // at this point32_t is only to cause a
                                       // save.)
    const std::string& BYLAW_NAME,  // Should already be on the contract. (This
                                    // way we can find it.)
    const std::string& CALLBACK_NAME,  // The Callback's name as referenced in
                                       // the smart contract. (And the
                                       // scripts...)
    const std::string& CLAUSE_NAME) const  // The actual clause that will be
                                           // triggered
                                           // by the callback. (Must exist.)
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (SIGNER_NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SIGNER_NYM_ID passed in!\n";
        return "";
    }
    if (BYLAW_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: BYLAW_NAME passed in!\n";
        return "";
    }
    if (CALLBACK_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: CALLBACK_NAME passed in!\n";
        return "";
    }
    if (CLAUSE_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: CLAUSE_NAME passed in!\n";
        return "";
    }

    const String strContract(THE_CONTRACT), strBylawName(BYLAW_NAME),
        strCallbackName(CALLBACK_NAME), strClauseName(CLAUSE_NAME);
    const Identifier theSignerNymID(SIGNER_NYM_ID);
    String strOutput;

    const bool& bAdded = OTAPI()->SmartContract_AddCallback(
        strContract,      // The contract, about to have the clause added to it.
        theSignerNymID,   // Use any Nym you wish here. (The signing at this
                          // point32_t is only to cause a save.)
        strBylawName,     // Should already be on the contract. (This way we can
                          // find it.)
        strCallbackName,  // The Callback's name as referenced in the smart
                          // contract. (And the scripts...)
        strClauseName,    // The actual clause that will be triggered by the
                          // callback. (Must exist.)
        strOutput);
    if (!bAdded || !strOutput.Exists()) return "";
    // Success!
    //
    return strOutput.Get();
}

// returns: the updated smart contract (or "")
std::string OTAPI_Exec::SmartContract_RemoveCallback(
    const std::string& THE_CONTRACT,   // The contract, about to have the
                                       // callback
                                       // removed from it.
    const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                       // signing
                                       // at this point32_t is only to cause a
                                       // save.)
    const std::string& BYLAW_NAME,  // Should already be on the contract. (This
                                    // way we can find it.)
    const std::string& CALLBACK_NAME  // The Callback's name as referenced in
                                      // the smart contract. (And the
                                      // scripts...)
    ) const
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (SIGNER_NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SIGNER_NYM_ID passed in!\n";
        return "";
    }
    if (BYLAW_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: BYLAW_NAME passed in!\n";
        return "";
    }
    if (CALLBACK_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: CALLBACK_NAME passed in!\n";
        return "";
    }

    const String strContract(THE_CONTRACT), strBylawName(BYLAW_NAME),
        strCallbackName(CALLBACK_NAME);
    const Identifier theSignerNymID(SIGNER_NYM_ID);
    String strOutput;

    const bool& bAdded = OTAPI()->SmartContract_RemoveCallback(
        strContract,      // The contract, about to have the clause added to it.
        theSignerNymID,   // Use any Nym you wish here. (The signing at this
                          // point32_t is only to cause a save.)
        strBylawName,     // Should already be on the contract. (This way we can
                          // find it.)
        strCallbackName,  // The Callback's name as referenced in the smart
                          // contract. (And the scripts...)
        strOutput);
    if (!bAdded || !strOutput.Exists()) return "";
    // Success!
    //
    return strOutput.Get();
}

// returns: the updated smart contract (or "")
std::string OTAPI_Exec::SmartContract_AddHook(
    const std::string& THE_CONTRACT,   // The contract, about to have the hook
                                       // added to it.
    const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                       // signing
                                       // at this point32_t is only to cause a
                                       // save.)
    const std::string& BYLAW_NAME,  // Should already be on the contract. (This
                                    // way we can find it.)
    const std::string& HOOK_NAME,  // The Hook's name as referenced in the smart
                                   // contract. (And the scripts...)
    const std::string& CLAUSE_NAME) const  // The actual clause that will be
                                           // triggered
// by the hook. (You can call this multiple
// times, and have multiple clauses trigger
// on the same hook.)
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (SIGNER_NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SIGNER_NYM_ID passed in!\n";
        return "";
    }
    if (BYLAW_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: BYLAW_NAME passed in!\n";
        return "";
    }
    if (HOOK_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: HOOK_NAME passed in!\n";
        return "";
    }
    if (CLAUSE_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: CLAUSE_NAME passed in!\n";
        return "";
    }

    const String strContract(THE_CONTRACT), strBylawName(BYLAW_NAME),
        strHookName(HOOK_NAME), strClauseName(CLAUSE_NAME);
    const Identifier theSignerNymID(SIGNER_NYM_ID);
    String strOutput;

    const bool& bAdded = OTAPI()->SmartContract_AddHook(
        strContract,     // The contract, about to have the clause added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // point32_t is only to cause a save.)
        strBylawName,    // Should already be on the contract. (This way we can
                         // find it.)
        strHookName,     // The Hook's name as referenced in the smart contract.
                         // (And the scripts...)
        strClauseName,  // The actual clause that will be triggered by the hook.
                        // (You can call this multiple times, and have multiple
                        // clauses trigger on the same hook.)
        strOutput);
    if (!bAdded || !strOutput.Exists()) return "";
    // Success!
    //
    return strOutput.Get();
}

// returns: the updated smart contract (or "")
std::string OTAPI_Exec::SmartContract_RemoveHook(
    const std::string& THE_CONTRACT,   // The contract, about to have the hook
                                       // removed from it.
    const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                       // signing
                                       // at this point32_t is only to cause a
                                       // save.)
    const std::string& BYLAW_NAME,  // Should already be on the contract. (This
                                    // way we can find it.)
    const std::string& HOOK_NAME,  // The Hook's name as referenced in the smart
                                   // contract. (And the scripts...)
    const std::string& CLAUSE_NAME) const  // The actual clause that will be
                                           // triggered
// by the hook. (You can call this multiple
// times, and have multiple clauses trigger
// on the same hook.)
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (SIGNER_NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SIGNER_NYM_ID passed in!\n";
        return "";
    }
    if (BYLAW_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: BYLAW_NAME passed in!\n";
        return "";
    }
    if (HOOK_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: HOOK_NAME passed in!\n";
        return "";
    }
    if (CLAUSE_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: CLAUSE_NAME passed in!\n";
        return "";
    }

    const String strContract(THE_CONTRACT), strBylawName(BYLAW_NAME),
        strHookName(HOOK_NAME), strClauseName(CLAUSE_NAME);
    const Identifier theSignerNymID(SIGNER_NYM_ID);
    String strOutput;

    const bool& bAdded = OTAPI()->SmartContract_RemoveHook(
        strContract,     // The contract, about to have the clause added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // point32_t is only to cause a save.)
        strBylawName,    // Should already be on the contract. (This way we can
                         // find it.)
        strHookName,     // The Hook's name as referenced in the smart contract.
                         // (And the scripts...)
        strClauseName,  // The actual clause that will be triggered by the hook.
                        // (You can call this multiple times, and have multiple
                        // clauses trigger on the same hook.)
        strOutput);
    if (!bAdded || !strOutput.Exists()) return "";
    // Success!
    //
    return strOutput.Get();
}

// RETURNS: Updated version of THE_CONTRACT. (Or "".)
std::string OTAPI_Exec::SmartContract_AddParty(
    const std::string& THE_CONTRACT,   // The contract, about to have the party
                                       // added to it.
    const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                       // signing
                                       // at this point32_t is only to cause a
                                       // save.)
    const std::string& PARTY_NYM_ID,   // Required when the smart contract is
                                       // configured to require parties to be
                                       // specified. Otherwise must be empty.
    const std::string& PARTY_NAME,     // The Party's NAME as referenced in the
                                       // smart contract. (And the scripts...)
    const std::string& AGENT_NAME) const  // An AGENT will be added by default
                                          // for this
                                          // party. Need Agent NAME.
// (FYI, that is basically the only option, until I code Entities and Roles.
// Until then, a party can ONLY be
// a Nym, with himself as the agent representing that same party. Nym ID is
// supplied on ConfirmParty() below.)
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (SIGNER_NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SIGNER_NYM_ID passed in!\n";
        return "";
    }
    if (PARTY_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: PARTY_NAME passed in!\n";
        return "";
    }
    if (AGENT_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: AGENT_NAME passed in!\n";
        return "";
    }
    const String strContract(THE_CONTRACT), strPartyName(PARTY_NAME),
        strAgentName(AGENT_NAME), strPartyNymID(PARTY_NYM_ID);
    const Identifier theSignerNymID(SIGNER_NYM_ID);
    String strOutput;

    const bool& bAdded = OTAPI()->SmartContract_AddParty(
        strContract,     // The contract, about to have the bylaw added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // point32_t is only to cause a save.)
        strPartyNymID,
        strPartyName,  // The Party's NAME as referenced in the smart contract.
                       // (And the scripts...)
        strAgentName,  // An AGENT will be added by default for this party. Need
                       // Agent NAME.
        strOutput);
    if (!bAdded || !strOutput.Exists()) return "";
    // Success!
    //
    return strOutput.Get();
}

// RETURNS: Updated version of THE_CONTRACT. (Or "".)
std::string OTAPI_Exec::SmartContract_RemoveParty(
    const std::string& THE_CONTRACT,   // The contract, about to have the party
                                       // added to it.
    const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                       // signing
                                       // at this point32_t is only to cause a
                                       // save.)
    const std::string& PARTY_NAME      // The Party's NAME as referenced in the
                                       // smart contract. (And the scripts...)
    ) const
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (SIGNER_NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SIGNER_NYM_ID passed in!\n";
        return "";
    }
    if (PARTY_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: PARTY_NAME passed in!\n";
        return "";
    }

    const String strContract(THE_CONTRACT), strPartyName(PARTY_NAME);
    const Identifier theSignerNymID(SIGNER_NYM_ID);
    String strOutput;

    const bool& bAdded = OTAPI()->SmartContract_RemoveParty(
        strContract,     // The contract, about to have the bylaw added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // point32_t is only to cause a save.)
        strPartyName,  // The Party's NAME as referenced in the smart contract.
                       // (And the scripts...)
        strOutput);
    if (!bAdded || !strOutput.Exists()) return "";
    // Success!
    //
    return strOutput.Get();
}

// Used when creating a theoretical smart contract (that could be used over and
// over again with different parties.)
//
// returns: the updated smart contract (or "")
std::string OTAPI_Exec::SmartContract_AddAccount(
    const std::string& THE_CONTRACT,  // The contract, about to have the account
                                      // added to it.
    const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                       // signing
                                       // at this point32_t is only to cause a
                                       // save.)
    const std::string& PARTY_NAME,     // The Party's NAME as referenced in the
                                       // smart contract. (And the scripts...)
    const std::string& ACCT_NAME,  // The Account's name as referenced in the
                                   // smart contract
    const std::string& INSTRUMENT_DEFINITION_ID) const  // Instrument Definition
// ID for the Account. (Optional.)
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (SIGNER_NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SIGNER_NYM_ID passed in!\n";
        return "";
    }
    if (PARTY_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: PARTY_NAME passed in!\n";
        return "";
    }
    if (ACCT_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCT_NAME passed in!\n";
        return "";
    }
    //    if (INSTRUMENT_DEFINITION_ID.empty())        { otErr << __FUNCTION__
    //    << ": Null:
    // INSTRUMENT_DEFINITION_ID passed
    // in!\n"; OT_FAIL; }

    const String strContract(THE_CONTRACT), strPartyName(PARTY_NAME),
        strAcctName(ACCT_NAME),
        strInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID);
    const Identifier theSignerNymID(SIGNER_NYM_ID);
    String strOutput;

    const bool& bAdded = OTAPI()->SmartContract_AddAccount(
        strContract,     // The contract, about to have the clause added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // point32_t is only to cause a save.)
        strPartyName,  // The Party's NAME as referenced in the smart contract.
                       // (And the scripts...)
        strAcctName,   // The Account's name as referenced in the smart contract
        strInstrumentDefinitionID,  // Instrument Definition ID for the Account.
        strOutput);
    if (!bAdded || !strOutput.Exists()) return "";
    // Success!
    //
    return strOutput.Get();
}

// returns: the updated smart contract (or "")
std::string OTAPI_Exec::SmartContract_RemoveAccount(
    const std::string& THE_CONTRACT,  // The contract, about to have the account
                                      // removed from it.
    const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                       // signing
                                       // at this point32_t is only to cause a
                                       // save.)
    const std::string& PARTY_NAME,     // The Party's NAME as referenced in the
                                       // smart contract. (And the scripts...)
    const std::string& ACCT_NAME  // The Account's name as referenced in the
                                  // smart contract
    ) const
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (SIGNER_NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SIGNER_NYM_ID passed in!\n";
        return "";
    }
    if (PARTY_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: PARTY_NAME passed in!\n";
        return "";
    }
    if (ACCT_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCT_NAME passed in!\n";
        return "";
    }

    const String strContract(THE_CONTRACT), strPartyName(PARTY_NAME),
        strAcctName(ACCT_NAME);
    const Identifier theSignerNymID(SIGNER_NYM_ID);
    String strOutput;

    const bool& bAdded = OTAPI()->SmartContract_RemoveAccount(
        strContract,     // The contract, about to have the clause added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // point32_t is only to cause a save.)
        strPartyName,  // The Party's NAME as referenced in the smart contract.
                       // (And the scripts...)
        strAcctName,   // The Account's name as referenced in the smart contract
        strOutput);
    if (!bAdded || !strOutput.Exists()) return "";
    // Success!
    //
    return strOutput.Get();
}

// This function returns the count of how many trans#s a Nym needs in order to
// confirm as
// a specific agent for a contract. (An opening number is needed for every party
// of which
// agent is the authorizing agent, plus a closing number for every acct of which
// agent is the
// authorized agent.)
//
// Otherwise a Nym might try to confirm a smart contract and be unable to, since
// he doesn't
// have enough transaction numbers, yet he would have no way of finding out HOW
// MANY HE *DOES*
// NEED. This function allows him to find out, before confirmation, so he can
// first grab however
// many transaction#s he will need in order to confirm this smart contract.
//
int32_t OTAPI_Exec::SmartContract_CountNumsNeeded(
    const std::string& THE_CONTRACT,  // The smart contract, about to be queried
                                      // by this function.
    const std::string& AGENT_NAME) const
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return 0;
    }
    if (AGENT_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: AGENT_NAME passed in!\n";
        return 0;
    }
    const String strContract(THE_CONTRACT), strAgentName(AGENT_NAME);
    return OTAPI()->SmartContract_CountNumsNeeded(strContract, strAgentName);
}

// Used when taking a theoretical smart contract, and setting it up to use
// specific Nyms and accounts.
// This function sets the ACCT ID and the AGENT NAME for the acct specified by
// party name and acct name.
// Returns the updated smart contract (or "".)
//
std::string OTAPI_Exec::SmartContract_ConfirmAccount(
    const std::string& THE_CONTRACT,
    const std::string& SIGNER_NYM_ID,
    const std::string& PARTY_NAME,     // Should already be on the contract.
    const std::string& ACCT_NAME,      // Should already be on the contract.
    const std::string& AGENT_NAME,     // The agent name for this asset account.
    const std::string& ACCT_ID) const  // AcctID for the asset account. (For
                                       // acct_name).
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (SIGNER_NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SIGNER_NYM_ID passed in!\n";
        return "";
    }
    if (PARTY_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: PARTY_NAME passed in!\n";
        return "";
    }
    if (ACCT_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCT_NAME passed in!\n";
        return "";
    }
    if (AGENT_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: AGENT_NAME passed in!\n";
        return "";
    }
    if (ACCT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCT_ID passed in!\n";
        return "";
    }
    const String strContract(THE_CONTRACT), strPartyName(PARTY_NAME);
    const String strAccountID(ACCT_ID), strAcctName(ACCT_NAME),
        strAgentName(AGENT_NAME);
    const Identifier theSignerNymID(SIGNER_NYM_ID), theAcctID(strAccountID);
    String strOutput;

    const bool& bConfirmed = OTAPI()->SmartContract_ConfirmAccount(
        strContract,
        theSignerNymID,
        strPartyName,
        strAcctName,
        strAgentName,
        strAccountID,
        strOutput);
    if (!bConfirmed || !strOutput.Exists()) return "";
    // Success!
    return strOutput.Get();
}

// Called by each Party. Pass in the smart contract obtained in the above call.
// Call OTAPI_Exec::SmartContract_ConfirmAccount() first, as much as you need
// to, THEN call this (for final signing.)
// This is the last call you make before either passing it on to another party
// to confirm, or calling OTAPI_Exec::activateSmartContract().
// Returns the updated smart contract (or "".)
std::string OTAPI_Exec::SmartContract_ConfirmParty(
    const std::string& THE_CONTRACT,  // The smart contract, about to be changed
                                      // by this function.
    const std::string& PARTY_NAME,    // Should already be on the contract. This
                                      // way we can find it.
    const std::string& NYM_ID) const  // Nym ID for the party, the actual owner,
// ===> AS WELL AS for the default AGENT of that party. (For now, until I code
// entities)
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (PARTY_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: PARTY_NAME passed in!\n";
        return "";
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    const Identifier theNymID(NYM_ID);
    const String strContract(THE_CONTRACT), strPartyName(PARTY_NAME);
    String strOutput;

    const bool& bConfirmed = OTAPI()->SmartContract_ConfirmParty(
        strContract,   // The smart contract, about to be changed by this
                       // function.
        strPartyName,  // Should already be on the contract. This way we can
                       // find
                       // it.
        theNymID,      // Nym ID for the party, the actual owner,
        strOutput);
    if (!bConfirmed || !strOutput.Exists()) return "";
    // Success!
    return strOutput.Get();
}

bool OTAPI_Exec::Smart_AreAllPartiesConfirmed(
    const std::string& THE_CONTRACT) const  // true or false?
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return false;
    }
    String strContract(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        OTScriptable::InstantiateScriptable(strContract));
    if (nullptr == pScriptable) {
        otOut << __FUNCTION__
              << ": Failed trying to load smart contract from string : \n\n"
              << strContract << "\n\n";
    } else {
        const bool bConfirmed =
            pScriptable->AllPartiesHaveSupposedlyConfirmed();
        const bool bVerified =
            pScriptable->VerifyThisAgainstAllPartiesSignedCopies();
        if (!bConfirmed) {
            //          otOut << __FUNCTION__ << ": Smart contract loaded up,
            // but all
            // parties are NOT confirmed:\n\n" << strContract << "\n\n";
            otWarn << __FUNCTION__ << ": Smart contract loaded up, but all "
                                      "parties are NOT confirmed.\n";
            return false;
        } else if (bVerified) {
            //          otOut << __FUNCTION__ << ": Success: Smart contract
            // loaded
            // up, and all parties have confirmed,\n"
            //                         "AND their signed versions verified
            // also.\n";

            // Todo security: We have confirmed that all parties have provided
            // signed copies, but we have
            // not actually verified the signatures themselves. (Though we HAVE
            // verified that their copies of
            // the contract match the main copy.)
            // The server DOES verify this before activation, but the client
            // should as well, just in case. Todo.
            // (I'd want MY client to do it...)
            //
            return true;
        }
        otOut << __FUNCTION__
              << ": Suspicious: Smart contract loaded up, and is supposedly "
                 "confirmed by all parties, but failed to verify:\n\n"
              << strContract << "\n\n";
    }
    return false;
}

bool OTAPI_Exec::Smart_IsPartyConfirmed(
    const std::string& THE_CONTRACT,
    const std::string& PARTY_NAME) const  // true
                                          // or
                                          // false?
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return false;
    }
    if (PARTY_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: PARTY_NAME passed in!\n";
        return false;
    }
    const String strContract(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        OTScriptable::InstantiateScriptable(strContract));
    if (nullptr == pScriptable) {
        otOut << __FUNCTION__
              << ": Failed trying to load smart contract from string : \n\n"
              << strContract << "\n\n";
        return false;
    }

    OTParty* pParty = pScriptable->GetParty(PARTY_NAME);
    if (nullptr == pParty) {
        otOut << __FUNCTION__
              << ": Smart contract loaded up, but failed to find a party "
                 "with the name: "
              << PARTY_NAME << "\n";
        return false;
    }

    // We found the party...
    //...is he confirmed?
    //
    if (!pParty->GetMySignedCopy().Exists()) {
        otWarn << __FUNCTION__ << ": Smart contract loaded up, and party "
               << PARTY_NAME
               << " was found, but didn't find a signed copy of the "
                  "agreement for that party.\n";
        return false;
    }

    // FYI, this block comes from
    // OTScriptable::VerifyThisAgainstAllPartiesSignedCopies.
    std::unique_ptr<OTScriptable> pPartySignedCopy(
        OTScriptable::InstantiateScriptable(pParty->GetMySignedCopy()));

    if (nullptr == pPartySignedCopy) {
        const std::string current_party_name(pParty->GetPartyName());
        otErr << __FUNCTION__ << ": Error loading party's ("
              << current_party_name
              << ") signed copy of agreement. Has it been "
                 "executed?\n";
        return false;
    }

    if (!pScriptable->Compare(*pPartySignedCopy)) {
        const std::string current_party_name(pParty->GetPartyName());
        otErr << __FUNCTION__ << ": Suspicious: Party's (" << current_party_name
              << ") signed copy of agreement doesn't match the "
                 "contract.\n";
        return false;
    }

    // TODO Security: This function doesn't actually verify
    // the party's SIGNATURE on his signed
    // version, only that it exists and it matches the main
    // contract.
    //
    // The actual signatures are all verified by the server
    // before activation, but I'd still like the client
    // to have the option to do so as well. I can imagine
    // getting someone's signature on something (without
    // signing it yourself) and then just retaining the
    // OPTION to sign it later -- but he might not have
    // actually signed it if he knew that you hadn't either.
    // He'd want his client to tell him, if this were
    // the case. Todo.

    return true;
}

int32_t OTAPI_Exec::Smart_GetPartyCount(const std::string& THE_CONTRACT) const
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return OT_ERROR;
    }
    String strContract(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        OTScriptable::InstantiateScriptable(strContract));
    if (nullptr == pScriptable) {
        otOut << __FUNCTION__
              << " Failed trying to load smart contract from string:\n\n"
              << strContract << "\n\n";
        return OT_ERROR;  // Error condition.
    }

    return pScriptable->GetPartyCount();
}

int32_t OTAPI_Exec::Smart_GetBylawCount(const std::string& THE_CONTRACT) const
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return OT_ERROR;
    }
    String strContract(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        OTScriptable::InstantiateScriptable(strContract));
    if (nullptr == pScriptable) {
        otOut << __FUNCTION__
              << ": Failed trying to load smart contract from string : \n\n"
              << strContract << "\n\n";
        return OT_ERROR;  // Error condition.
    }

    return pScriptable->GetBylawCount();
}

/// returns the name of the party.
std::string OTAPI_Exec::Smart_GetPartyByIndex(
    const std::string& THE_CONTRACT,
    const int32_t& nIndex) const
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    const String strContract(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        OTScriptable::InstantiateScriptable(strContract));
    if (nullptr == pScriptable) {
        otOut << __FUNCTION__
              << ": Failed trying to load smart contract from string:\n\n"
              << strContract << "\n\n";
        return "";
    }

    const int32_t nTempIndex = nIndex;
    OTParty* pParty = pScriptable->GetPartyByIndex(
        nTempIndex);  // has range-checking built-in.
    if (nullptr == pParty) {
        otOut << __FUNCTION__
              << ": Smart contract loaded up, but failed to retrieve the "
                 "party using index: "
              << nTempIndex << "\n";
        return "";
    }

    return pParty->GetPartyName();
}

/// returns the name of the bylaw.
std::string OTAPI_Exec::Smart_GetBylawByIndex(
    const std::string& THE_CONTRACT,
    const int32_t& nIndex) const
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    const String strContract(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        OTScriptable::InstantiateScriptable(strContract));
    if (nullptr == pScriptable) {
        otOut << __FUNCTION__
              << ": Failed trying to load smart contract from string : \n\n"
              << strContract << "\n\n";
        return "";
    }

    const int32_t nTempIndex = nIndex;
    OTBylaw* pBylaw = pScriptable->GetBylawByIndex(
        nTempIndex);  // has range-checking built-in.
    if (nullptr == pBylaw) {
        otOut << __FUNCTION__
              << ": Smart contract loaded up, but failed to retrieve the "
                 "bylaw using index: "
              << nTempIndex << "\n";
        return "";
    }

    // We found the bylaw...
    return pBylaw->GetName().Get();
}

std::string OTAPI_Exec::Bylaw_GetLanguage(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME) const
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (BYLAW_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: BYLAW_NAME passed in!\n";
        return "";
    }
    String strContract(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        OTScriptable::InstantiateScriptable(strContract));
    if (nullptr == pScriptable) {
        otOut << __FUNCTION__
              << ": Failed trying to load smart contract from string : \n\n"
              << strContract << "\n\n";
        return "";
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        otOut << __FUNCTION__
              << ": Smart contract loaded up, but failed to find a bylaw "
                 "with the name: "
              << BYLAW_NAME << "\n";
        return "";
    }
    // We found the bylaw...
    if (nullptr == pBylaw->GetLanguage()) return "error_no_language";
    return pBylaw->GetLanguage();
}

int32_t OTAPI_Exec::Bylaw_GetClauseCount(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME) const
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return OT_ERROR;
    }
    if (BYLAW_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: BYLAW_NAME passed in!\n";
        return OT_ERROR;
    }
    String strContract(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        OTScriptable::InstantiateScriptable(strContract));
    if (nullptr == pScriptable) {
        otOut << __FUNCTION__
              << ": Failed trying to load smart contract from string : \n\n"
              << strContract << "\n\n";
        return OT_ERROR;
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        otOut << __FUNCTION__
              << ": Smart contract loaded up, but failed to find a bylaw "
                 "with the name: "
              << BYLAW_NAME << "\n";
        return OT_ERROR;
    }

    return pBylaw->GetClauseCount();
}

int32_t OTAPI_Exec::Bylaw_GetVariableCount(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME) const
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return OT_ERROR;
    }
    if (BYLAW_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: BYLAW_NAME passed in!\n";
        return OT_ERROR;
    }
    String strContract(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        OTScriptable::InstantiateScriptable(strContract));
    if (nullptr == pScriptable) {
        otOut << __FUNCTION__
              << ": Failed trying to load smart contract from string : \n\n"
              << strContract << "\n\n";
        return OT_ERROR;
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        otOut << __FUNCTION__
              << ": Smart contract loaded up, but failed to find a bylaw "
                 "with the name: "
              << BYLAW_NAME << "\n";
        return OT_ERROR;
    }

    return pBylaw->GetVariableCount();
}

int32_t OTAPI_Exec::Bylaw_GetHookCount(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME) const
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return OT_ERROR;
    }
    if (BYLAW_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: BYLAW_NAME passed in!\n";
        return OT_ERROR;
    }
    String strContract(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        OTScriptable::InstantiateScriptable(strContract));
    if (nullptr == pScriptable) {
        otOut << __FUNCTION__
              << ": Failed trying to load smart contract from string : \n\n"
              << strContract << "\n\n";
        return OT_ERROR;
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        otOut << __FUNCTION__
              << ": Smart contract loaded up, but failed to find a bylaw "
                 "with the name: "
              << BYLAW_NAME << "\n";
        return OT_ERROR;
    }

    return pBylaw->GetHookCount();
}

int32_t OTAPI_Exec::Bylaw_GetCallbackCount(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME) const
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return OT_ERROR;
    }
    if (BYLAW_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: BYLAW_NAME passed in!\n";
        return OT_ERROR;
    }
    String strContract(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        OTScriptable::InstantiateScriptable(strContract));
    if (nullptr == pScriptable) {
        otOut << __FUNCTION__
              << ": Failed trying to load smart contract from string:\n\n"
              << strContract << "\n\n";
        return OT_ERROR;
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        otOut << __FUNCTION__
              << ": Smart contract loaded up, but failed to find a bylaw "
                 "with the name: "
              << BYLAW_NAME << "\n";
        return OT_ERROR;
    }

    return pBylaw->GetCallbackCount();
}

std::string OTAPI_Exec::Clause_GetNameByIndex(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME,
    const int32_t& nIndex) const  // returns the name of the clause.
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (BYLAW_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: BYLAW_NAME passed in!\n";
        return "";
    }
    const String strContract(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        OTScriptable::InstantiateScriptable(strContract));
    if (nullptr == pScriptable) {
        otOut << __FUNCTION__
              << ": Failed trying to load smart contract from string:\n\n"
              << strContract << "\n\n";
        return "";
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        otOut << __FUNCTION__
              << ": Smart contract loaded up, but failed to retrieve the "
                 "bylaw with name: "
              << BYLAW_NAME << "\n";
        return "";
    }

    const int32_t nTempIndex = nIndex;
    OTClause* pClause = pBylaw->GetClauseByIndex(nTempIndex);
    if (nullptr == pClause) {
        otOut << __FUNCTION__ << ": Smart contract loaded up, and "
                                 "bylaw found, but failed to retrieve "
                                 "the clause at index: "
              << nTempIndex << "\n";
        return "";
    }

    // Success.
    return pClause->GetName().Get();
}

std::string OTAPI_Exec::Clause_GetContents(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME,
    const std::string& CLAUSE_NAME) const  // returns the contents of the
                                           // clause.
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (BYLAW_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: BYLAW_NAME passed in!\n";
        return "";
    }
    if (CLAUSE_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: CLAUSE_NAME passed in!\n";
        return "";
    }
    const String strContract(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        OTScriptable::InstantiateScriptable(strContract));
    if (nullptr == pScriptable) {
        otOut << __FUNCTION__
              << ": Failed trying to load smart contract from string:\n\n"
              << strContract << "\n\n";
        return "";
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        otOut << __FUNCTION__
              << ": Smart contract loaded up, but failed to retrieve the "
                 "bylaw with name: "
              << BYLAW_NAME << "\n";
        return "";
    }

    OTClause* pClause = pBylaw->GetClause(CLAUSE_NAME);

    if (nullptr == pClause) {
        otOut << __FUNCTION__ << ": Smart contract loaded up, and "
                                 "bylaw found, but failed to retrieve "
                                 "the clause with name: "
              << CLAUSE_NAME << "\n";
        return "";
    }
    // Success.
    return pClause->GetCode();
}

std::string OTAPI_Exec::Variable_GetNameByIndex(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME,
    const int32_t& nIndex) const  // returns the name of the variable.
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (BYLAW_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: BYLAW_NAME passed in!\n";
        return "";
    }
    const String strContract(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        OTScriptable::InstantiateScriptable(strContract));
    if (nullptr == pScriptable) {
        otOut << __FUNCTION__
              << ": Failed trying to load smart contract from string:\n\n"
              << strContract << "\n\n";
        return "";
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        otOut << __FUNCTION__
              << ": Smart contract loaded up, but failed to retrieve the "
                 "bylaw with name: "
              << BYLAW_NAME << "\n";
        return "";
    }

    const int32_t nTempIndex = nIndex;
    OTVariable* pVar = pBylaw->GetVariableByIndex(nTempIndex);
    if (nullptr == pVar) {
        otOut << __FUNCTION__ << ": Smart contract loaded up, and "
                                 "bylaw found, but failed to retrieve "
                                 "the variable at index: "
              << nTempIndex << "\n";
        return "";
    }

    return pVar->GetName().Get();
}

std::string OTAPI_Exec::Variable_GetType(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME,
    const std::string& VARIABLE_NAME) const  // returns the type of the
                                             // variable.
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (BYLAW_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: BYLAW_NAME passed in!\n";
        return "";
    }
    if (VARIABLE_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: VARIABLE_NAME passed in!\n";
        return "";
    }
    const String strContract(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        OTScriptable::InstantiateScriptable(strContract));
    if (nullptr == pScriptable) {
        otOut << __FUNCTION__
              << ": Failed trying to load smart contract from string:\n\n"
              << strContract << "\n\n";
        return "";
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        otOut << __FUNCTION__
              << ": Smart contract loaded up, but failed to retrieve the "
                 "bylaw with name: "
              << BYLAW_NAME << "\n";
        return "";
    }

    OTVariable* pVar = pBylaw->GetVariable(VARIABLE_NAME);
    if (nullptr == pVar) {
        otOut << __FUNCTION__
              << ": Smart contract loaded up, and bylaw found, but "
                 "failed to retrieve the variable with name: "
              << VARIABLE_NAME << "\n";
        return "";
    }

    if (pVar->IsInteger()) return "integer";
    if (pVar->IsBool()) return "boolean";
    if (pVar->IsString()) return "string";
    return "error_type";
}

std::string OTAPI_Exec::Variable_GetAccess(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME,
    const std::string& VARIABLE_NAME) const  // returns the access level of the
                                             // variable.
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (BYLAW_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: BYLAW_NAME passed in!\n";
        return "";
    }
    if (VARIABLE_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: VARIABLE_NAME passed in!\n";
        return "";
    }
    const String strContract(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        OTScriptable::InstantiateScriptable(strContract));
    if (nullptr == pScriptable) {
        otOut << __FUNCTION__
              << ": Failed trying to load smart contract from string:\n\n"
              << strContract << "\n\n";
        return "";
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        otOut << __FUNCTION__
              << ": Smart contract loaded up, but failed to retrieve the "
                 "bylaw with name: "
              << BYLAW_NAME << "\n";
        return "";
    }

    OTVariable* pVar = pBylaw->GetVariable(VARIABLE_NAME);
    if (nullptr == pVar) {
        otOut << __FUNCTION__
              << ": Smart contract loaded up, and bylaw found, but "
                 "failed to retrieve the variable with name: "
              << VARIABLE_NAME << "\n";
        return "";
    }

    if (pVar->IsConstant()) return "constant";
    if (pVar->IsImportant()) return "important";
    if (pVar->IsPersistent()) return "persistent";
    return "error_access";
}

std::string OTAPI_Exec::Variable_GetContents(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME,
    const std::string& VARIABLE_NAME) const  // returns the contents of the
                                             // variable.
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (BYLAW_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: BYLAW_NAME passed in!\n";
        return "";
    }
    if (VARIABLE_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: VARIABLE_NAME passed in!\n";
        return "";
    }
    const String strContract(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        OTScriptable::InstantiateScriptable(strContract));
    if (nullptr == pScriptable) {
        otOut << __FUNCTION__
              << ": Failed trying to load smart contract from string:\n\n"
              << strContract << "\n\n";
        return "";
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        otOut << __FUNCTION__
              << ": Smart contract loaded up, but failed to retrieve the "
                 "bylaw with name: "
              << BYLAW_NAME << "\n";
        return "";
    }

    OTVariable* pVar = pBylaw->GetVariable(VARIABLE_NAME);
    if (nullptr == pVar) {
        otOut << __FUNCTION__
              << ": Smart contract loaded up, and bylaw found, but "
                 "failed to retrieve the variable with name: "
              << VARIABLE_NAME << "\n";
        return "";
    }

    switch (pVar->GetType()) {
        case OTVariable::Var_String:
            return pVar->GetValueString();
        case OTVariable::Var_Integer:
            return OTAPI_Exec::LongToString(
                static_cast<int64_t>(pVar->GetValueInteger()));
        case OTVariable::Var_Bool:
            return pVar->GetValueBool() ? "true" : "false";
        default:
            otErr << __FUNCTION__ << ": Error: Unknown variable type.\n";
            return "";
    }
}

std::string OTAPI_Exec::Hook_GetNameByIndex(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME,
    const int32_t& nIndex) const  // returns the name of the hook.
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (BYLAW_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: BYLAW_NAME passed in!\n";
        return "";
    }
    const String strContract(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        OTScriptable::InstantiateScriptable(strContract));
    if (nullptr == pScriptable) {
        otOut << __FUNCTION__
              << ": Failed trying to load smart contract from string:\n\n"
              << strContract << "\n\n";
        return "";
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        otOut << __FUNCTION__
              << ": Smart contract loaded up, but failed to retrieve the "
                 "bylaw with name: "
              << BYLAW_NAME << "\n";
        return "";
    }

    const int32_t nTempIndex = nIndex;
    return pBylaw->GetHookNameByIndex(nTempIndex);
}

/// Returns the number of clauses attached to a specific hook.
int32_t OTAPI_Exec::Hook_GetClauseCount(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME,
    const std::string& HOOK_NAME) const
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return OT_ERROR;
    }
    if (BYLAW_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: BYLAW_NAME passed in!\n";
        return OT_ERROR;
    }
    if (HOOK_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: HOOK_NAME passed in!\n";
        return OT_ERROR;
    }
    const String strContract(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        OTScriptable::InstantiateScriptable(strContract));
    if (nullptr == pScriptable) {
        otOut << __FUNCTION__
              << ": Failed trying to load smart contract from string:\n\n"
              << strContract << "\n\n";
        return OT_ERROR;
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        otOut << __FUNCTION__
              << ": Smart contract loaded up, but failed to retrieve the "
                 "bylaw with name: "
              << BYLAW_NAME << "\n";
        return OT_ERROR;
    }

    mapOfClauses theResults;
    // Look up all clauses matching a specific hook.
    if (!pBylaw->GetHooks(HOOK_NAME, theResults)) return OT_ERROR;

    return static_cast<const int32_t>(theResults.size());
}

/// Multiple clauses can trigger from the same hook.
/// Hook_GetClauseCount and Hook_GetClauseAtIndex allow you to iterate through
/// them.
/// This function returns the name for the clause at the specified index.
///
std::string OTAPI_Exec::Hook_GetClauseAtIndex(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME,
    const std::string& HOOK_NAME,
    const int32_t& nIndex) const
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (BYLAW_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: BYLAW_NAME passed in!\n";
        return "";
    }
    if (HOOK_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: HOOK_NAME passed in!\n";
        return "";
    }
    const String strContract(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        OTScriptable::InstantiateScriptable(strContract));
    if (nullptr == pScriptable) {
        otOut << __FUNCTION__
              << ": Failed trying to load smart contract from string:\n\n"
              << strContract << "\n\n";
        return "";
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        otOut << __FUNCTION__
              << ": Smart contract loaded up, but failed to retrieve the "
                 "bylaw with name: "
              << BYLAW_NAME << "\n";
        return "";
    }

    mapOfClauses theResults;

    // Look up all clauses matching a specific hook.
    if (!pBylaw->GetHooks(HOOK_NAME, theResults)) return "";

    if ((nIndex < 0) || (nIndex >= static_cast<int64_t>(theResults.size())))
        return "";

    int32_t nLoopIndex = -1;
    for (auto& it : theResults) {
        OTClause* pClause = it.second;
        OT_ASSERT(nullptr != pClause);
        ++nLoopIndex;  // on first iteration, this is now 0.

        if (nLoopIndex == nIndex) return pClause->GetName().Get();
    }
    return "";
}

std::string OTAPI_Exec::Callback_GetNameByIndex(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME,
    const int32_t& nIndex) const  // returns the name of the callback.
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (BYLAW_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: BYLAW_NAME passed in!\n";
        return "";
    }
    const String strContract(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        OTScriptable::InstantiateScriptable(strContract));
    if (nullptr == pScriptable) {
        otOut << __FUNCTION__
              << ": Failed trying to load smart contract from string:\n\n"
              << strContract << "\n\n";
        return "";
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        otOut << __FUNCTION__
              << ": Smart contract loaded up, but failed to retrieve the "
                 "bylaw with name: "
              << BYLAW_NAME << "\n";
        return "";
    }

    const int32_t nTempIndex = nIndex;
    return pBylaw->GetCallbackNameByIndex(nTempIndex);
}

std::string OTAPI_Exec::Callback_GetClause(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME,
    const std::string& CALLBACK_NAME) const  // returns name of clause attached
                                             // to
                                             // callback.
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (BYLAW_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: BYLAW_NAME passed in!\n";
        return "";
    }
    if (CALLBACK_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: CALLBACK_NAME passed in!\n";
        return "";
    }
    const String strContract(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        OTScriptable::InstantiateScriptable(strContract));
    if (nullptr == pScriptable) {
        otOut << __FUNCTION__
              << ": Failed trying to load smart contract from string:\n\n"
              << strContract << "\n\n";
        return "";
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        otOut << __FUNCTION__
              << ": Smart contract loaded up, but failed to retrieve the "
                 "bylaw with name: "
              << BYLAW_NAME << "\n";
        return "";
    }

    OTClause* pClause = pBylaw->GetCallback(CALLBACK_NAME);
    if (nullptr == pClause) {
        otOut << __FUNCTION__
              << ": Smart contract loaded up, and bylaw found, but "
                 "failed to retrieve the clause for callback: "
              << CALLBACK_NAME << "\n";
        return "";
    }

    return pClause->GetName().Get();
}

int32_t OTAPI_Exec::Party_GetAcctCount(
    const std::string& THE_CONTRACT,
    const std::string& PARTY_NAME) const
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return OT_ERROR;
    }
    if (PARTY_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: PARTY_NAME passed in!\n";
        return OT_ERROR;
    }
    String strContract(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        OTScriptable::InstantiateScriptable(strContract));
    if (nullptr == pScriptable) {
        otOut << __FUNCTION__
              << ": Failed trying to load smart contract from string:\n\n"
              << strContract << "\n\n";
        return OT_ERROR;
    }

    OTParty* pParty = pScriptable->GetParty(PARTY_NAME);
    if (nullptr == pParty) {
        otOut << __FUNCTION__
              << ": Smart contract loaded up, but failed to find a party "
                 "with the name: "
              << PARTY_NAME << "\n";
        return OT_ERROR;
    }

    return pParty->GetAccountCount();
}

int32_t OTAPI_Exec::Party_GetAgentCount(
    const std::string& THE_CONTRACT,
    const std::string& PARTY_NAME) const
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return OT_ERROR;
    }
    if (PARTY_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: PARTY_NAME passed in!\n";
        return OT_ERROR;
    }
    String strContract(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        OTScriptable::InstantiateScriptable(strContract));
    if (nullptr == pScriptable) {
        otOut << __FUNCTION__
              << ": Failed trying to load smart contract from string:\n\n"
              << strContract << "\n\n";
        return OT_ERROR;
    }

    OTParty* pParty = pScriptable->GetParty(PARTY_NAME);
    if (nullptr == pParty) {
        otOut << __FUNCTION__
              << ": Smart contract loaded up, but failed to find a party "
                 "with the name: "
              << PARTY_NAME << "\n";
        return OT_ERROR;
    }

    return pParty->GetAgentCount();
}

std::string OTAPI_Exec::Party_GetID(
    const std::string& THE_CONTRACT,
    const std::string& PARTY_NAME) const  // returns
                                          // either
                                          // NymID or
                                          // Entity ID.
                                          // (If
// there is one... Contract might not be
// signed yet.)
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (PARTY_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: PARTY_NAME passed in!\n";
        return "";
    }
    String strContract(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        OTScriptable::InstantiateScriptable(strContract));
    if (nullptr == pScriptable) {
        otOut << __FUNCTION__
              << ": Failed trying to load smart contract from string:\n\n"
              << strContract << "\n\n";
        return "";
    }

    OTParty* pParty = pScriptable->GetParty(PARTY_NAME);
    if (nullptr == pParty) {
        otOut << __FUNCTION__
              << ": Smart contract loaded up, but failed to find a party "
                 "with the name: "
              << PARTY_NAME << "\n";
        return "";
    }

    return pParty->GetPartyID();
}

std::string OTAPI_Exec::Party_GetAcctNameByIndex(
    const std::string& THE_CONTRACT,
    const std::string& PARTY_NAME,
    const int32_t& nIndex) const  // returns the name of the clause.
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (PARTY_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: PARTY_NAME passed in!\n";
        return "";
    }
    const String strContract(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        OTScriptable::InstantiateScriptable(strContract));
    if (nullptr == pScriptable) {
        otOut << __FUNCTION__
              << ": Failed trying to load smart contract from string:\n\n"
              << strContract << "\n\n";
        return "";
    }

    OTParty* pParty = pScriptable->GetParty(PARTY_NAME);
    if (nullptr == pParty) {
        otOut << __FUNCTION__
              << ": Smart contract loaded up, but failed to retrieve the "
                 "party with name: "
              << PARTY_NAME << "\n";
        return "";
    }

    const int32_t nTempIndex = nIndex;
    OTPartyAccount* pAcct = pParty->GetAccountByIndex(nTempIndex);
    if (nullptr == pAcct) {
        otOut << __FUNCTION__ << ": Smart contract loaded up, and "
                                 "party found, but failed to retrieve "
                                 "the account at index: "
              << nTempIndex << "\n";
        return "";
    }

    return pAcct->GetName().Get();
}

std::string OTAPI_Exec::Party_GetAcctID(
    const std::string& THE_CONTRACT,
    const std::string& PARTY_NAME,
    const std::string& ACCT_NAME) const  // returns the account ID based on the
                                         // account
                                         // name. (If there is one yet...)
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (PARTY_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: PARTY_NAME passed in!\n";
        return "";
    }
    if (ACCT_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCT_NAME passed in!\n";
        return "";
    }
    const String strContract(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        OTScriptable::InstantiateScriptable(strContract));
    if (nullptr == pScriptable) {
        otOut << __FUNCTION__
              << ": Failed trying to load smart contract from string:\n\n"
              << strContract << "\n\n";
        return "";
    }

    OTParty* pParty = pScriptable->GetParty(PARTY_NAME);
    if (nullptr == pParty) {
        otOut << __FUNCTION__
              << ": Smart contract loaded up, but failed to retrieve the "
                 "party with name: "
              << PARTY_NAME << "\n";
        return "";
    }

    const OTPartyAccount* pAcct = pParty->GetAccount(ACCT_NAME);
    if (nullptr == pAcct) {
        otOut << __FUNCTION__ << ": Smart contract loaded up, and "
                                 "party found, but failed to retrieve "
                                 "party's account named: "
              << ACCT_NAME << "\n";
        return "";
    }

    return pAcct->GetAcctID().Get();
}

std::string OTAPI_Exec::Party_GetAcctInstrumentDefinitionID(
    const std::string& THE_CONTRACT,
    const std::string& PARTY_NAME,
    const std::string& ACCT_NAME) const  // returns the instrument definition ID
                                         // based on
                                         // the
                                         // account name.
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (PARTY_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: PARTY_NAME passed in!\n";
        return "";
    }
    if (ACCT_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCT_NAME passed in!\n";
        return "";
    }
    const String strContract(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        OTScriptable::InstantiateScriptable(strContract));
    if (nullptr == pScriptable) {
        otOut << __FUNCTION__
              << ": Failed trying to load smart contract from string:\n\n"
              << strContract << "\n\n";
    } else {
        OTParty* pParty = pScriptable->GetParty(PARTY_NAME);
        if (nullptr == pParty) {
            otOut << __FUNCTION__
                  << ": Smart contract loaded up, but failed to retrieve the "
                     "party with name: "
                  << PARTY_NAME << "\n";
        } else  // We found the party...
        {
            const OTPartyAccount* pAcct = pParty->GetAccount(ACCT_NAME);

            if (nullptr == pAcct) {
                otOut << __FUNCTION__ << ": Smart contract loaded up, and "
                                         "party found, but failed to retrieve "
                                         "party's account named: "
                      << ACCT_NAME << "\n";
            } else  // We found the account...
            {
                const std::string str_return(
                    pAcct->GetInstrumentDefinitionID().Get());  // Success.
                return str_return;
            }
        }
    }
    return "";
}

std::string OTAPI_Exec::Party_GetAcctAgentName(
    const std::string& THE_CONTRACT,
    const std::string& PARTY_NAME,
    const std::string& ACCT_NAME) const  // returns the authorized agent for the
                                         // named
                                         // account.
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (PARTY_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: PARTY_NAME passed in!\n";
        return "";
    }
    if (ACCT_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCT_NAME passed in!\n";
        return "";
    }
    const String strContract(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        OTScriptable::InstantiateScriptable(strContract));
    if (nullptr == pScriptable) {
        otOut << __FUNCTION__
              << ": Failed trying to load smart contract from string:\n\n"
              << strContract << "\n\n";
    } else {
        OTParty* pParty = pScriptable->GetParty(PARTY_NAME);
        if (nullptr == pParty) {
            otOut << __FUNCTION__
                  << ": Smart contract loaded up, but failed to retrieve the "
                     "party with name: "
                  << PARTY_NAME << "\n";
        } else  // We found the party...
        {
            const OTPartyAccount* pAcct = pParty->GetAccount(ACCT_NAME);

            if (nullptr == pAcct) {
                otOut << __FUNCTION__ << ": Smart contract loaded up, and "
                                         "party found, but failed to retrieve "
                                         "party's account named: "
                      << ACCT_NAME << "\n";
            } else  // We found the account...
            {
                const std::string str_return(
                    pAcct->GetAgentName().Get());  // Success.
                return str_return;
            }
        }
    }
    return "";
}

std::string OTAPI_Exec::Party_GetAgentNameByIndex(
    const std::string& THE_CONTRACT,
    const std::string& PARTY_NAME,
    const int32_t& nIndex) const  // returns the name of the agent.
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (PARTY_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: PARTY_NAME passed in!\n";
        return "";
    }
    const String strContract(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        OTScriptable::InstantiateScriptable(strContract));
    if (nullptr == pScriptable) {
        otOut << __FUNCTION__
              << ": Failed trying to load smart contract from string:\n\n"
              << strContract << "\n\n";
    } else {
        OTParty* pParty = pScriptable->GetParty(PARTY_NAME);
        if (nullptr == pParty) {
            otOut << __FUNCTION__
                  << ": Smart contract loaded up, but failed to retrieve the "
                     "party with name: "
                  << PARTY_NAME << "\n";
        } else  // We found the party...
        {
            const int32_t nTempIndex = nIndex;
            OTAgent* pAgent = pParty->GetAgentByIndex(nTempIndex);

            if (nullptr == pAgent) {
                otOut << __FUNCTION__
                      << ": Smart contract loaded up, and party found, but "
                         "failed to retrieve the agent at index: "
                      << nTempIndex << "\n";
            } else  // We found the agent...
            {
                const std::string str_name(
                    pAgent->GetName().Get());  // Success.
                return str_name;
            }
        }
    }
    return "";
}

std::string OTAPI_Exec::Party_GetAgentID(
    const std::string& THE_CONTRACT,
    const std::string& PARTY_NAME,
    const std::string& AGENT_NAME) const  // returns ID of the agent. (If there
                                          // is
                                          // one...)
{
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return "";
    }
    if (PARTY_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: PARTY_NAME passed in!\n";
        return "";
    }
    if (AGENT_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: AGENT_NAME passed in!\n";
        return "";
    }
    const String strContract(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        OTScriptable::InstantiateScriptable(strContract));
    if (nullptr == pScriptable) {
        otOut << __FUNCTION__
              << ": Failed trying to load smart contract from string:\n\n"
              << strContract << "\n\n";
    } else {
        OTParty* pParty = pScriptable->GetParty(PARTY_NAME);
        if (nullptr == pParty) {
            otOut << __FUNCTION__
                  << ": Smart contract loaded up, but failed to retrieve the "
                     "party with name: "
                  << PARTY_NAME << "\n";
        } else  // We found the party...
        {
            OTAgent* pAgent = pParty->GetAgent(AGENT_NAME);

            if (nullptr == pAgent) {
                otOut << __FUNCTION__ << ": Smart contract loaded up, and "
                                         "party found, but failed to retrieve "
                                         "party's agent named: "
                      << AGENT_NAME << "\n";
            } else  // We found the agent...
            {
                std::string str_return;
                Identifier theAgentID;
                if (pAgent->IsAnIndividual() && pAgent->GetNymID(theAgentID)) {
                    const String strTemp(theAgentID);
                    str_return = strTemp.Get();
                    return str_return;
                }
            }
        }
    }
    return "";
}

// ACTIVATE SMART CONTRACT   (This is a transaction, and messages the server.)
// Take an existing smart contract, which has already been set up, confirmed,
// etc,
// and then activate it on the server so it can start processing.
//
// See OTAPI_Exec::Create_SmartContract (etc.)
//
// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
// >0 means NO error, and the message was sent, and the request number fits into
// an integer...
//  ...and in fact the requestNum IS the return value!
//  ===> In 99% of cases, this LAST option is what actually happens!!
//
int32_t OTAPI_Exec::activateSmartContract(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& THE_SMART_CONTRACT) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (THE_SMART_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_SMART_CONTRACT passed in!\n";
        return OT_ERROR;
    }

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID);
    const String strContract(THE_SMART_CONTRACT);

    return OTAPI()->activateSmartContract(theNotaryID, theNymID, strContract);
}

// If a smart contract is already running on the server, this allows a party
// to trigger clauses on that smart contract, by name. This is NOT a
// transaction,
// but it DOES message the server.
//
// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
// >0 means NO error, and the message was sent, and the request number fits into
// an integer...
//  ...and in fact the requestNum IS the return value!
//  ===> In 99% of cases, this LAST option is what actually happens!!
//
int32_t OTAPI_Exec::triggerClause(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const int64_t& TRANSACTION_NUMBER,
    const std::string& CLAUSE_NAME,
    const std::string& STR_PARAM) const  // optional param
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (0 > TRANSACTION_NUMBER) {
        otErr << __FUNCTION__ << ": Null: TRANSACTION_NUMBER passed in!\n";
        return OT_ERROR;
    }
    if (CLAUSE_NAME.empty()) {
        otErr << __FUNCTION__ << ": Null: CLAUSE_NAME passed in!\n";
        return OT_ERROR;
    }
    //    if (STR_PARAM.empty())      { otErr << __FUNCTION__ << ": Null:
    // STR_PARAM passed
    // in!\n"; OT_FAIL; }  // optional
    // param
    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID);
    const String strClauseName(CLAUSE_NAME);
    const int64_t lTransactionNum = TRANSACTION_NUMBER;
    const String strParam((STR_PARAM.empty()) ? "" : STR_PARAM);
    return OTAPI()->triggerClause(
        theNotaryID,
        theNymID,
        static_cast<int64_t>(lTransactionNum),
        strClauseName,
        STR_PARAM.empty() ? nullptr : &strParam);
}

/*
OTAPI_Exec::Msg_HarvestTransactionNumbers

This function will load up the cron item (which is either a market offer, a
payment plan,
or a SMART CONTRACT.)  UPDATE: this function operates on messages, not cron
items.

Then it will try to harvest all of the closing transaction numbers for NYM_ID
that are
available to be harvested from THE_CRON_ITEM. (There might be zero #s available
for that
Nym, which is still a success and will return true. False means error.)

YOU MIGHT ASK:

WHY WOULD I WANT to harvest ONLY the closing numbers for the Nym, and not the
OPENING
numbers as well? The answer is because for this Nym, the opening number might
already
be burned. For example, if Nym just tried to activate a smart contract, and the
activation
FAILED, then maybe the opening number is already gone, even though his closing
numbers, on the
other hand, are still valid for retrieval. (I have to double check this.)

HOWEVER, what if the MESSAGE failed, before it even TRIED the transaction? In
which case,
the opening number is still good also, and should be retrieved.

Remember, I have to keep signing for my transaction numbers until they are
finally closed out.
They will appear on EVERY balance agreement and transaction statement from here
until the end
of time, whenever I finally close out those numbers. If some of them are still
good on a failed
transaction, then I want to retrieve them so I can use them, and eventually
close them out.

==> Whereas, what if I am the PARTY to a smart contract, but I am not the actual
ACTIVATOR / ORIGINATOR
(who activated the smart contract on the server).  Therefore I never sent any
transaction to the
server, and I never burned my opening number. It's probably still a good #. If
my wallet is not a piece
of shit, then I should have a stored copy of any contract that I signed. If it
turns out in the future
that that contract wasn't activated, then I can retrieve not only my closing
numbers, but my OPENING
number as well! IN THAT CASE, I would call OTAPI_Exec::HarvestAllNumbers()
instead of OTAPI_Exec::HarvestClosingNumbers().


UPDATE: The above logic is now handled automatically in
OTAPI_Exec::HarvestTransactionNumbers.
Therefore OTAPI_Exec::HarvestClosingNumbers and OTAPI_Exec::HarvestAllNumbers
have been removed.

*/

bool OTAPI_Exec::Msg_HarvestTransactionNumbers(
    const std::string& THE_MESSAGE,
    const std::string& NYM_ID,
    const bool& bHarvestingForRetry,
    const bool& bReplyWasSuccess,
    const bool& bReplyWasFailure,
    const bool& bTransactionWasSuccess,
    const bool& bTransactionWasFailure) const
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return false;
    }
    if (THE_MESSAGE.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_MESSAGE passed in!\n";
        return false;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return false;
    }
    const Identifier theNymID(NYM_ID);
    Message theMessage;
    const String strMsg(THE_MESSAGE);
    if (!strMsg.Exists()) {
        otErr << __FUNCTION__
              << ": Failed trying to load message from empty string.\n";
        return false;
    }
    // Maybe it's not a message at all. Maybe it's a cron item
    // (smart contract... payment plan...)
    //
    if (strMsg.Contains("PAYMENT PLAN") || strMsg.Contains("SMARTCONTRACT")) {
        const String& strCronItem = strMsg;

        otOut << __FUNCTION__ << ": Attempting to harvest transaction numbers "
                                 "from cron item...\n";
        // Unfortunately the ONLY reason we are loading up this cron item here,
        // is so we can get the server ID off of it.
        //
        std::unique_ptr<OTCronItem> pCronItem(
            OTCronItem::NewCronItem(strCronItem));
        if (nullptr == pCronItem) {
            otErr << __FUNCTION__
                  << ": Failed trying to load message from string.";

            otOut << __FUNCTION__
                  << ": Error trying to load the cron item from string (a cron "
                     "item is a smart contract, or some other recurring "
                     "transaction such as a market offer, or a payment plan.) "
                     "Contents:\n\n"
                  << strCronItem << "\n\n";
            return false;
        }

        // NOTE:
        // If a CronItem is passed in here instead of a Message, that means the
        // client
        // didn't even TRY to send the message. He failed before reaching that
        // point.
        // Therefore in this one, strange case, we don't really care about all
        // the bools
        // that were passed in here. We're just going to harvest ALL the
        // numbers, and
        // ASSUME all the bools were false.
        // Here goes...
        //
        return OTAPI()->HarvestAllNumbers(
            pCronItem->GetNotaryID(), theNymID, strCronItem);
    }
    // Maybe it's not a message at all. Maybe it's a basket exchange request
    // that never
    // even got sent as a message...
    //
    if (strMsg.Contains("currencyBasket")) {
        const String& strBasket = strMsg;

        otOut << __FUNCTION__ << ": Attempting to harvest transaction numbers "
                                 "from a basket currency exchange request...\n";
        Nym* pNym = OTAPI()->GetOrLoadPrivateNym(theNymID, false, __FUNCTION__);
        if (nullptr == pNym) return false;
        Basket theRequestBasket;

        if (theRequestBasket.LoadContractFromString(strBasket)) {
            if (!theRequestBasket.IsExchanging()) {
                otErr << __FUNCTION__ << ": Error: This is apparently NOT a "
                                         "basket exchange request!\nContents:\n"
                      << strBasket << "\n";
                return false;
            }
            // Now we need to find the account ID (so we can find the server
            // ID...)
            //
            Account* pAccount = OTAPI()->GetAccount(
                theRequestBasket.GetRequestAccountID(), __FUNCTION__);
            if (nullptr == pAccount) {
                const String strAcctID(theRequestBasket.GetRequestAccountID());
                otErr << __FUNCTION__
                      << ": Error: Unable to find the main account based on "
                         "the ID from the exchange request: "
                      << strAcctID << "\n";
                return false;
            }
            // Now let's get the server ID...
            //
            auto pServer = OTAPI()->GetServer(
                pAccount->GetPurportedNotaryID(), __FUNCTION__);

            if (!pServer) {
                const String strNotaryID(pAccount->GetPurportedNotaryID());
                otErr << __FUNCTION__
                      << ": Error: Unable to find the server based on the "
                         "exchange request: "
                      << strNotaryID << "\n";
                return false;
            }
            theRequestBasket.HarvestClosingNumbers(
                *pNym, pAccount->GetPurportedNotaryID(), true);  // bSave=true
            return true;
        } else
            otErr << __FUNCTION__
                  << ": Error loading original basket request.\n";

        return false;
    } else if (!theMessage.LoadContractFromString(strMsg)) {
        otErr << __FUNCTION__
              << ": Failed trying to load message from string.\n";
        return false;
    }
    // By this point, we have the actual message loaded up.
    //
    const bool& bSuccess = OTAPI()->Msg_HarvestTransactionNumbers(
        theMessage,
        theNymID,
        bHarvestingForRetry,
        bReplyWasSuccess,
        bReplyWasFailure,
        bTransactionWasSuccess,
        bTransactionWasFailure);
    return bSuccess ? true : false;
}

// bool OTAPI_Exec::HarvestClosingNumbers(const std::string& NOTARY_ID,
//                                    const std::string& NYM_ID,
//                                    const std::string& THE_CRON_ITEM)
//{
//    OT_ASSERT_MSG("" != NOTARY_ID, "OTAPI_Exec::HarvestClosingNumbers: Null
// NOTARY_ID passed in.");
//    OT_ASSERT_MSG("" != NYM_ID, "OTAPI_Exec::HarvestClosingNumbers: Null
// NYM_ID passed in.");
//    OT_ASSERT_MSG("" != THE_CRON_ITEM, "OTAPI_Exec::HarvestClosingNumbers:
// Null THE_CRON_ITEM passed in.");
//
//    const OTIdentifier    theNymID(NYM_ID), theNotaryID(NOTARY_ID);
//    const OTString        strContract(THE_CRON_ITEM);
//
//    const bool& bHarvested = OTAPI()->HarvestClosingNumbers(theNotaryID,
// theNymID, strContract);
//
//    return bHarvested ? true : false;
//}
//
//
// NOTE: usually you will want to call OTAPI_Exec::HarvestClosingNumbers, since
// the Opening number is usually
// burned up from some failed transaction or whatever. You don't want to harvest
// the opening number usually,
// just the closing numbers. (If the opening number is burned up, yet you still
// harvest it, then your OT wallet
// will end up using that number again on some other transaction, which will
// obviously then fail since the number
// isn't good anymore.)
// This function is only for those cases where you are sure that the opening
// transaction # hasn't been burned yet,
// such as when the message failed and the transaction wasn't attempted (because
// it never got that far) or such as
// when the contract simply never got signed or activated by one of the other
// parties, and so you want to claw your
// #'s back, and since in that case your opening number is still good, you would
// use the below function to get it back.
//
// bool OTAPI_Exec::HarvestAllNumbers(const std::string& NOTARY_ID,
//                                const std::string& NYM_ID,
//                                const std::string& THE_CRON_ITEM)
//{
//    OT_ASSERT_MSG("" != NOTARY_ID, "OTAPI_Exec::HarvestAllNumbers: Null
// NOTARY_ID passed in.");
//    OT_ASSERT_MSG("" != NYM_ID, "OTAPI_Exec::HarvestAllNumbers: Null NYM_ID
// passed in.");
//    OT_ASSERT_MSG("" != THE_CRON_ITEM, "OTAPI_Exec::HarvestAllNumbers: Null
// THE_CRON_ITEM passed in.");
//
//    const OTIdentifier    theNymID(NYM_ID), theNotaryID(NOTARY_ID);
//    const OTString        strContract(THE_CRON_ITEM);
//
//    const bool& bHarvested = OTAPI()->HarvestAllNumbers(theNotaryID,
// theNymID, strContract);
//
//    return bHarvested ? true : false;
//}

// LOAD PUBLIC KEY (of other users, where no private key is available)
// This is the "address book" versus the private Nym.
// If nothing found in the address book, it still tries to load
// a Private Nym (just to get the pubkey from it.)
// -- from local storage
//
// Return as STRING (NOT escaped.)
//
// Users will most likely store public keys of OTHER users, and they will need
// to load those from time to time, especially to verify signatures, etc.
//
std::string OTAPI_Exec::LoadPubkey_Encryption(
    const std::string& NYM_ID) const  // returns "", or a public key.
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    String strPubkey;  // For the output
    OTPasswordData thePWData(OT_PW_DISPLAY);
    Identifier nym_id(NYM_ID);
    const Nym* pNym = OTAPI()->GetOrLoadNym(
        nym_id,
        false,
        __FUNCTION__,
        &thePWData);  // This tries to get, then tries to
                      // load as public, then tries to load
                      // as private.
    if (nullptr == pNym) return "";
    if (false == pNym->GetPublicEncrKey().GetPublicKey(strPubkey))  // bEscaped
                                                                    // defaults
                                                                    // to true.
                                                                    // 6/13/12
    {
        String strNymID(nym_id);
        otOut << __FUNCTION__
              << ": Failure retrieving encryption pubkey from Nym: " << strNymID
              << "\n";
    } else  // success
    {
        std::string pBuf = strPubkey.Get();
        return pBuf;
    }
    return "";
}

std::string OTAPI_Exec::LoadPubkey_Signing(
    const std::string& NYM_ID) const  // returns "", or a public key.
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    String strPubkey;  // For the output
    OTPasswordData thePWData(OT_PW_DISPLAY);
    Identifier nym_id(NYM_ID);
    const Nym* pNym = OTAPI()->GetOrLoadNym(
        nym_id,
        false,
        __FUNCTION__,
        &thePWData);  // This tries to get, then tries to
                      // load as public, then tries to load
                      // as private.
    if (nullptr == pNym) return "";
    if (false == pNym->GetPublicSignKey().GetPublicKey(strPubkey)) {
        String strNymID(nym_id);
        otOut << __FUNCTION__
              << ": Failure retrieving signing pubkey from Nym: " << strNymID
              << "\n";
    } else  // success
    {
        std::string pBuf = strPubkey.Get();
        return pBuf;
    }
    return "";
}

// LOAD USER PUBLIC KEY  -- from local storage
//
// (return as STRING)
//
std::string OTAPI_Exec::LoadUserPubkey_Encryption(
    const std::string& NYM_ID) const  // returns "", or a public key.
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    String strPubkey;  // For the output
    Identifier nym_id(NYM_ID);
    Nym* pNym = OTAPI()->GetOrLoadPrivateNym(nym_id);  // No need to cleanup.
    if (nullptr == pNym) return "";
    if (!pNym->GetPublicEncrKey().GetPublicKey(strPubkey)) {
        String strNymID(nym_id);
        otOut << __FUNCTION__
              << ": Failure retrieving encryption pubkey from Nym: " << strNymID
              << "\n";
    } else  // success
    {
        std::string pBuf = strPubkey.Get();
        return pBuf;
    }
    return "";
}

std::string OTAPI_Exec::LoadUserPubkey_Signing(
    const std::string& NYM_ID) const  // returns "", or a public key.
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    String strPubkey;  // For the output
    Identifier nym_id(NYM_ID);
    // No need to cleanup.
    Nym* pNym = OTAPI()->GetOrLoadPrivateNym(Identifier(NYM_ID));
    if (nullptr == pNym) return "";
    if (!pNym->GetPublicSignKey().GetPublicKey(strPubkey)) {
        String strNymID(nym_id);
        otOut << __FUNCTION__
              << ": Failure retrieving signing pubkey from Nym: " << strNymID
              << "\n";
    } else  // success
    {
        std::string pBuf = strPubkey.Get();
        return pBuf;
    }
    return "";
}

//
// Verify that NYM_ID (including its Private Key) is an
// available and verified user in local storage.
//
// Loads the user's private key, verifies, then returns true or false.
//
bool OTAPI_Exec::VerifyUserPrivateKey(
    const std::string& NYM_ID) const  // returns
                                      // bool
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return false;
    }
    Nym* pNym = OTAPI()->GetOrLoadPrivateNym(
        Identifier(NYM_ID));  // No need to cleanup.
    if (nullptr == pNym) return false;
    return true;
}

//
// Is Mint32_t Still Good ?   true  (1) == Yes, this mint32_t is still good.
//                        false (0) == No: expired or other error.
//
bool OTAPI_Exec::Mint_IsStillGood(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return false;
    }
    if (INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Null: INSTRUMENT_DEFINITION_ID passed in!\n";
        return false;
    }

    const Identifier theNotaryID(NOTARY_ID),
        theInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID);
    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.
    std::unique_ptr<Mint> pMint(
        OTAPI()->LoadMint(theNotaryID, theInstrumentDefinitionID));

    if (nullptr == pMint)
        otOut << __FUNCTION__
              << ": Failure calling OT_API::LoadMint.\nServer: " << NOTARY_ID
              << "\n Asset Type: " << INSTRUMENT_DEFINITION_ID << "\n";
    else  // success
    {
        bool bExpired = pMint->Expired();

        if (!bExpired) return true;
    }
    return false;
}

std::string OTAPI_Exec::LoadMint(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID) const  // returns
                                                        // "", or a
                                                        // mint
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Null: INSTRUMENT_DEFINITION_ID passed in!\n";
        return "";
    }

    const Identifier theNotaryID(NOTARY_ID);
    const Identifier theInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID);

    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.
    std::unique_ptr<Mint> pMint(
        OTAPI()->LoadMint(theNotaryID, theInstrumentDefinitionID));

    if (nullptr == pMint)
        otOut << __FUNCTION__ << "OTAPI_Exec::LoadMint: Failure calling "
                                 "OT_API::LoadMint.\nServer: "
              << NOTARY_ID << "\n Asset Type: " << INSTRUMENT_DEFINITION_ID
              << "\n";
    else  // success
    {
        String strOutput(*pMint);  // For the output
        std::string pBuf = strOutput.Get();
        return pBuf;
    }
    return "";
}

std::string OTAPI_Exec::LoadServerContract(
    const std::string& NOTARY_ID) const  // returns "", or an asset contract
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null NOTARY_ID passed in!\n";
        return "";
    }

    const Identifier theNotaryID(NOTARY_ID);

    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.
    auto pContract = App::Me().Contract().Server(theNotaryID);

    if (!pContract) {
        otOut << __FUNCTION__
              << ": Failure calling OT_API::LoadServerContract.\nNotary ID: "
              << NOTARY_ID << "\n";
    } else  // success
    {
        OTASCIIArmor armored(pContract->Serialize());
        String strOutput;
        armored.WriteArmoredString(strOutput, "SERVER CONTRACT");
        return strOutput.Get();
    }
    return "";
}

// LOAD ACCOUNT / INBOX / OUTBOX   --  (from local storage)
//
// Loads an acct, or inbox or outbox, based on account ID, (from local storage)
// and returns it as string (or returns "" if it couldn't load it.)
//
std::string OTAPI_Exec::LoadAssetAccount(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID) const  // Returns "", or an account.
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return "";
    }

    const Identifier theNotaryID(NOTARY_ID);
    const Identifier theNymID(NYM_ID);
    const Identifier theAccountID(ACCOUNT_ID);

    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.
    std::unique_ptr<Account> pAccount(
        OTAPI()->LoadAssetAccount(theNotaryID, theNymID, theAccountID));

    if (nullptr == pAccount) {
        otOut << __FUNCTION__
              << ": Failure calling OT_API::LoadAssetAccount.\nAccount ID: "
              << ACCOUNT_ID << "\n";
    } else  // success
    {
        String strOutput(*pAccount);  // For the output
        std::string pBuf = strOutput.Get();
        return pBuf;
    }
    return "";
}

// Some server replies (to your messages) are so important that a notice is
// dropped
// into your Nymbox with a copy of the server's reply. It's called a
// replyNotice.
// Since the server is usually replying to a message, I've added this function
// for
// quickly looking up the message reply, if it's there, based on the
// requestNumber.
// This is the only example in the entire OT API where a Transaction is
// looked-up from
// a ledger, based on a REQUEST NUMBER. (Normally transactions use transaction
// numbers,
// and messages use request numbers. But in this case, it's a transaction that
// carries
// a copy of a message.)
// Note: Make sure you call this AFTER you download the box receipts, but BEFORE
// you process the Nymbox (because the reply notice will have disappeared.)
// Basically this
// function will be used for cases where you missed a server reply, and you need
// to
// search your Nymbox and see if a copy of the missed reply is still there.
// (Which it
// won't be, once you process the box.)
//
std::string OTAPI_Exec::Nymbox_GetReplyNotice(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const int64_t& REQUEST_NUMBER) const  // returns replyNotice transaction by
                                          // requestNumber.
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    if (0 > REQUEST_NUMBER) {
        otErr << __FUNCTION__ << ": Negative: REQUEST_NUMBER passed in!\n";
        return "";
    }

    const Identifier theNotaryID(NOTARY_ID);
    const Identifier theNymID(NYM_ID);

    const int64_t lRequestNumber = REQUEST_NUMBER;
    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.

    std::unique_ptr<Ledger> pLedger(
        OTAPI()->LoadNymboxNoVerify(theNotaryID, theNymID));

    if (nullptr == pLedger) {
        otOut << __FUNCTION__
              << ": Failure calling OT_API::LoadNymboxNoVerify.\n";
        return "";
    }
    OTTransaction* pTransaction =
        pLedger->GetReplyNotice(static_cast<int64_t>(lRequestNumber));
    // No need to cleanup this transaction, the ledger owns it already.

    if (nullptr == pTransaction) {
        otLog4 << __FUNCTION__
               << ": No replyNotice transactions found in ledger with request "
                  "number: "
               << lRequestNumber << "\n";
        return "";  // Maybe he was just looking; this isn't necessarily an
                    // error.
    }
    // At this point, I actually have the transaction pointer to the
    // replyNotice,
    // so let's return it in string form...
    //
    const int64_t lTransactionNum = pTransaction->GetTransactionNum();

    // Update: for transactions in ABBREVIATED form, the string is empty, since
    // it has never actually
    // been signed (in fact the whole point32_t with abbreviated transactions in
    // a ledger is that they
    // take up very little room, and have no signature of their own, but exist
    // merely as XML tags on
    // their parent ledger.)
    //
    // THEREFORE I must check to see if this transaction is abbreviated and if
    // so, sign it in order to
    // force the UpdateContents() call, so the programmatic user of this API
    // will be able to load it up.
    //

    String strOutput(*pTransaction);  // we only have the Abbreviated, so we
                                      // have to use this one.

    if (pTransaction->IsAbbreviated()) {
        pLedger->LoadBoxReceipt(static_cast<int64_t>(lTransactionNum));
        OTTransaction* pFullTransaction =
            pLedger->GetTransaction(static_cast<int64_t>(lTransactionNum));

        if (nullptr != pFullTransaction) {
            strOutput.Release();
            strOutput.zeroMemory();
            strOutput.Set(
                String(*pFullTransaction));  // we have the FullTransaction,
                                             // lets use that one.
        } else                               // nullptr == pFullTransaction
        {
            otErr << __FUNCTION__ << ": good index but uncovered \"\" pointer "
                                     "after trying to load full version of "
                                     "receipt (from abbreviated.) Thus, saving "
                                     "abbreviated version instead, so I can "
                                     "still return SOMETHING.\n";
            Nym* pNym = OTAPI()->GetNym(theNymID, __FUNCTION__);
            if (nullptr == pNym) return "";
            pTransaction->ReleaseSignatures();
            pTransaction->SignContract(*pNym);
            pTransaction->SaveContract();
        }
        strOutput.Release();
        pTransaction->SaveContractRaw(strOutput);  // if it was abbreviated
                                                   // before, now it either IS
                                                   // the box receipt, or it's
                                                   // the abbreviated version.
    }
    // We return the abbreviated version because the developer using the OT API
    // needs to know if that receipt is there, whether it's abbreviated or not.
    // So rather than passing "" when it's abbreviated, and thus leading him
    // astray, we give him the next-best thing: the abbreviated version. That
    // way at least he knows for sure that the receipt is there, the one he is
    // asking about.

    std::string pBuf = strOutput.Get();

    return pBuf;
}

// If the client-side has ALREADY seen the server's reply to a specific
// request number, he stores that number in a list which can be queried
// using this function.  A copy of that list is sent with nearly every request
// message sent to the server.  This way the server can see which replies you
// have already received. The server will mirror every number it sees (it sends
// its own list in all its replies.) Whenever you see a number mirrored in the
// server's reply, that means the server now knows you got its original reply
// (the one referenced by the number) and the server removed any replyNotice
// of that from your Nymbox (so you don't have to download it.) Basically that
// means you can go ahead and remove it from your list, and once you do, the
// server
// will remove its matching copy as well.
//
// *** When you are downloading your box receipts, you can skip any receipts
// where
// you have ALREADY seen the reply. So you can use this function to see if you
// already
// saw it, and if you did, then you can skip downloading that box receipt.
// Warning: this function isn't "perfect", in the sense that it cannot tell you
// definitively
// whether you have actually seen a server reply, but it CAN tell you if you
// have seen
// one until it finishes the above-described protocol (it will work in that way,
// which is
// how it was intended.) But after that, it will no int64_ter know if you got
// the reply since
// it has removed it from its list.
//
bool OTAPI_Exec::HaveAlreadySeenReply(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const int64_t& REQUEST_NUMBER) const  // returns
                                          // bool
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return false;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return false;
    }
    if (0 > REQUEST_NUMBER) {
        otErr << __FUNCTION__ << ": Negative: REQUEST_NUMBER passed in!\n";
        return false;
    }

    Identifier theNotaryID(NOTARY_ID);
    Identifier theNymID(NYM_ID);

    const int64_t lRequestNumber = REQUEST_NUMBER;

    // const std::string& strFunc = "OTAPI_Exec::HaveAlreadySeenReply";
    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.

    const bool& bTemp = OTAPI()->HaveAlreadySeenReply(
        theNotaryID, theNymID, static_cast<int64_t>(lRequestNumber));
    return bTemp;
}

std::string OTAPI_Exec::LoadNymbox(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const  // Returns
                                      // "", or
// an inbox.
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }

    const Identifier theNotaryID(NOTARY_ID);
    const Identifier theNymID(NYM_ID);

    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.
    std::unique_ptr<Ledger> pLedger(OTAPI()->LoadNymbox(theNotaryID, theNymID));

    if (nullptr == pLedger) {
        otOut << __FUNCTION__ << ": Failure calling OT_API::LoadNymbox.\n";
    } else  // success
    {
        String strOutput(*pLedger);  // For the output
        std::string pBuf = strOutput.Get();
        return pBuf;
    }

    return "";
}

std::string OTAPI_Exec::LoadNymboxNoVerify(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const  // Returns "", or an inbox.
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }

    const Identifier theNotaryID(NOTARY_ID);
    const Identifier theNymID(NYM_ID);

    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.
    std::unique_ptr<Ledger> pLedger(
        OTAPI()->LoadNymboxNoVerify(theNotaryID, theNymID));

    if (nullptr == pLedger) {
        otOut << __FUNCTION__
              << ": Failure calling OT_API::LoadNymboxNoVerify.\n";
    } else  // success
    {
        String strOutput(*pLedger);  // For the output
        std::string pBuf = strOutput.Get();
        return pBuf;
    }

    return "";
}

std::string OTAPI_Exec::LoadInbox(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID) const  // Returns "",
                                          // or an inbox.
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return "";
    }

    const Identifier theNotaryID(NOTARY_ID);
    const Identifier theNymID(NYM_ID);
    const Identifier theAccountID(ACCOUNT_ID);

    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.
    std::unique_ptr<Ledger> pLedger(
        OTAPI()->LoadInbox(theNotaryID, theNymID, theAccountID));

    if (nullptr == pLedger) {
        otWarn << __FUNCTION__
               << ": Failure calling OT_API::LoadInbox.\nAccount ID : "
               << ACCOUNT_ID << "\n";
    } else  // success
    {
        String strOutput(*pLedger);  // For the output
        std::string pBuf = strOutput.Get();
        return pBuf;
    }

    return "";
}

std::string OTAPI_Exec::LoadInboxNoVerify(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID) const  // Returns "", or an inbox.
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return "";
    }

    const Identifier theNotaryID(NOTARY_ID);
    const Identifier theNymID(NYM_ID);
    const Identifier theAccountID(ACCOUNT_ID);

    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.
    std::unique_ptr<Ledger> pLedger(
        OTAPI()->LoadInboxNoVerify(theNotaryID, theNymID, theAccountID));

    if (nullptr == pLedger) {
        otWarn << __FUNCTION__
               << ": Failure calling OT_API::LoadInboxNoVerify.\nAccount ID : "
               << ACCOUNT_ID << "\n";
    } else  // success
    {
        String strOutput(*pLedger);  // For the output
        std::string pBuf = strOutput.Get();
        return pBuf;
    }

    return "";
}

std::string OTAPI_Exec::LoadOutbox(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return "";
    }

    const Identifier theNotaryID(NOTARY_ID);
    const Identifier theNymID(NYM_ID);
    const Identifier theAccountID(ACCOUNT_ID);

    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.
    std::unique_ptr<Ledger> pLedger(
        OTAPI()->LoadOutbox(theNotaryID, theNymID, theAccountID));

    if (nullptr == pLedger) {
        otWarn << __FUNCTION__
               << ": Failure calling OT_API::LoadOutbox().\nAccount ID : "
               << ACCOUNT_ID << "\n";
    } else  // success
    {
        String strOutput(*pLedger);  // For the output
        std::string pBuf = strOutput.Get();
        return pBuf;
    }

    return "";
}

std::string OTAPI_Exec::LoadOutboxNoVerify(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return "";
    }

    const Identifier theNotaryID(NOTARY_ID);
    const Identifier theNymID(NYM_ID);
    const Identifier theAccountID(ACCOUNT_ID);

    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.
    std::unique_ptr<Ledger> pLedger(
        OTAPI()->LoadOutboxNoVerify(theNotaryID, theNymID, theAccountID));

    if (nullptr == pLedger) {
        otWarn << __FUNCTION__
               << ": Failure calling OT_API::LoadOutboxNoVerify.\nAccount ID : "
               << ACCOUNT_ID << "\n";
    } else  // success
    {
        String strOutput(*pLedger);  // For the output
        std::string pBuf = strOutput.Get();
        return pBuf;
    }

    return "";
}

std::string OTAPI_Exec::LoadPaymentInbox(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const  // Returns
                                      // "", or
                                      // an
                                      // inbox.
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }

    const Identifier theNotaryID(NOTARY_ID);
    const Identifier theNymID(NYM_ID);

    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.
    std::unique_ptr<Ledger> pLedger(
        OTAPI()->LoadPaymentInbox(theNotaryID, theNymID));

    if (nullptr == pLedger) {
        otWarn << __FUNCTION__
               << ": Failure calling OT_API::LoadPaymentInbox.\n Nym ID : "
               << NYM_ID << "\n";
    } else  // success
    {
        String strOutput(*pLedger);  // For the output
        std::string pBuf = strOutput.Get();
        return pBuf;
    }

    return "";
}

std::string OTAPI_Exec::LoadPaymentInboxNoVerify(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const  // Returns "", or a paymentInbox.
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }

    const Identifier theNotaryID(NOTARY_ID);
    const Identifier theNymID(NYM_ID);

    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.
    std::unique_ptr<Ledger> pLedger(
        OTAPI()->LoadPaymentInboxNoVerify(theNotaryID, theNymID));

    if (nullptr == pLedger) {
        otWarn
            << __FUNCTION__
            << ": Failure calling OT_API::LoadPaymentInboxNoVerify.\nNym ID: "
            << NYM_ID << "\n";
    } else  // success
    {
        String strOutput(*pLedger);  // For the output
        std::string pBuf = strOutput.Get();
        return pBuf;
    }

    return "";
}

std::string OTAPI_Exec::LoadRecordBox(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return "";
    }

    const Identifier theNotaryID(NOTARY_ID);
    const Identifier theNymID(NYM_ID);
    const Identifier theAccountID(ACCOUNT_ID);

    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.
    std::unique_ptr<Ledger> pLedger(
        OTAPI()->LoadRecordBox(theNotaryID, theNymID, theAccountID));

    if (nullptr == pLedger) {
        otWarn << __FUNCTION__ << ": Failure calling OT_API::LoadRecordBox.\n";
    } else  // success
    {
        String strOutput(*pLedger);  // For the output
        std::string pBuf = strOutput.Get();
        return pBuf;
    }
    return "";
}

std::string OTAPI_Exec::LoadRecordBoxNoVerify(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return "";
    }

    const Identifier theNotaryID(NOTARY_ID);
    const Identifier theNymID(NYM_ID);
    const Identifier theAccountID(ACCOUNT_ID);

    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.
    std::unique_ptr<Ledger> pLedger(
        OTAPI()->LoadRecordBoxNoVerify(theNotaryID, theNymID, theAccountID));

    if (nullptr == pLedger) {
        otWarn << __FUNCTION__
               << ": Failure calling OT_API::LoadRecordBoxNoVerify.\n";
    } else  // success
    {
        const String strOutput(*pLedger);  // For the output
        std::string pBuf = strOutput.Get();
        return pBuf;
    }
    return "";
}

std::string OTAPI_Exec::LoadExpiredBox(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }

    const Identifier theNotaryID(NOTARY_ID);
    const Identifier theNymID(NYM_ID);

    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.
    std::unique_ptr<Ledger> pLedger(
        OTAPI()->LoadExpiredBox(theNotaryID, theNymID));

    if (nullptr == pLedger) {
        otWarn << __FUNCTION__ << ": Failure calling OT_API::LoadExpiredBox.\n";
    } else  // success
    {
        String strOutput(*pLedger);  // For the output
        std::string pBuf = strOutput.Get();
        return pBuf;
    }
    return "";
}

std::string OTAPI_Exec::LoadExpiredBoxNoVerify(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const  // Returns nullptr, or a ExpiredBox.
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }

    const Identifier theNotaryID(NOTARY_ID);
    const Identifier theNymID(NYM_ID);

    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.
    std::unique_ptr<Ledger> pLedger(
        OTAPI()->LoadExpiredBoxNoVerify(theNotaryID, theNymID));

    if (nullptr == pLedger) {
        otWarn << __FUNCTION__
               << ": Failure calling OT_API::LoadExpiredBoxNoVerify.\n";
    } else  // success
    {
        const String strOutput(*pLedger);  // For the output
        std::string pBuf = strOutput.Get();
        return pBuf;
    }
    return "";
}

bool OTAPI_Exec::RecordPayment(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const bool& bIsInbox,   // true == payments inbox. false == payments outbox.
    const int32_t& nIndex,  // removes payment instrument (from payments in or
                            // out box) and moves to record box.
    const bool& bSaveCopy) const  // If false, then will NOT save a copy to
                                  // record box.
{
    OT_ASSERT(nIndex >= 0);
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return false;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return false;
    }

    const Identifier theNotaryID(NOTARY_ID);
    const Identifier theNymID(NYM_ID);

    return OTAPI()->RecordPayment(
        theNotaryID, theNymID, bIsInbox, nIndex, bSaveCopy);
}

bool OTAPI_Exec::ClearRecord(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,  // NYM_ID can be passed here as well.
    const int32_t& nIndex,
    const bool& bClearAll) const  // if true, nIndex is ignored.
{
    OT_ASSERT(nIndex >= 0);
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return false;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return false;
    }
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return false;
    }

    const Identifier theNotaryID(NOTARY_ID);
    const Identifier theNymID(NYM_ID);
    const Identifier theAcctID(ACCOUNT_ID);

    return OTAPI()->ClearRecord(
        theNotaryID, theNymID, theAcctID, nIndex, bClearAll);
}

bool OTAPI_Exec::ClearExpired(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const int32_t& nIndex,
    const bool& bClearAll) const  // if true, nIndex is
                                  // ignored.
{
    OT_ASSERT(nIndex >= 0);
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return false;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return false;
    }

    const Identifier theNotaryID(NOTARY_ID);
    const Identifier theNymID(NYM_ID);

    return OTAPI()->ClearExpired(theNotaryID, theNymID, nIndex, bClearAll);
}

// Returns number of transactions within, or -1 for error.
int32_t OTAPI_Exec::Ledger_GetCount(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_LEDGER) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return OT_ERROR;
    }
    if (THE_LEDGER.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_LEDGER passed in!\n";
        return OT_ERROR;
    }

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theAccountID(ACCOUNT_ID);

    String strLedger(THE_LEDGER);
    Ledger theLedger(theNymID, theAccountID, theNotaryID);

    if (!theLedger.LoadLedgerFromString(strLedger)) {
        String strAcctID(theAccountID);
        otErr << __FUNCTION__
              << ": Error loading ledger from string. Acct ID: " << strAcctID
              << "\n";
        return OT_ERROR;
    }

    return theLedger.GetTransactionCount();
}

// Creates a new 'response' ledger, set up with the right Notary ID, etc, so you
// can add the 'response' items to it, one by one. (Pass in the original ledger
// that you are responding to, as it uses the data from it to set up the
// response.)
// The original ledger is your inbox. Inbox receipts are the only things you
// would
// ever create a "response" to, as part of your "process inbox" process.
//
std::string OTAPI_Exec::Ledger_CreateResponse(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& ORIGINAL_LEDGER) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return "";
    }
    if (ORIGINAL_LEDGER.empty()) {
        otErr << __FUNCTION__ << ": Null: ORIGINAL_LEDGER passed in!\n";
        return "";
    }

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theAccountID(ACCOUNT_ID);
    Nym* pNym = OTAPI()->GetOrLoadPrivateNym(
        theNymID, false, __FUNCTION__);  // These copiously log, and ASSERT.
    if (nullptr == pNym) return "";
    // Let's load up the ledger (an inbox) that was passed in...
    String strOriginalLedger(ORIGINAL_LEDGER);
    Ledger theOriginalLedger(theNymID, theAccountID, theNotaryID);

    if (!theOriginalLedger.LoadLedgerFromString(strOriginalLedger)) {
        String strAcctID(theAccountID);
        otErr << __FUNCTION__
              << ": Error loading ledger from string. Acct ID: " << strAcctID
              << "\n";
        return "";
    }

    if (!theOriginalLedger.VerifyAccount(*pNym)) {
        String strAcctID(theAccountID);
        otErr << __FUNCTION__
              << ": Error verifying original ledger. Acct ID: " << strAcctID
              << "\n";
        return "";
    }
    // By this point, the ledger is loaded properly from the string,
    // Let's create the response to it.
    std::unique_ptr<Ledger> pResponseLedger(
        Ledger::GenerateLedger(
            theNymID, theAccountID, theNotaryID, Ledger::message));
    if (nullptr == pResponseLedger) {
        String strAcctID(theAccountID);
        otErr << __FUNCTION__
              << ": Error generating response ledger. Acct ID: " << strAcctID
              << "\n";
        return "";
    }
    pResponseLedger->SignContract(*pNym);
    pResponseLedger->SaveContract();

    String strOutput(*pResponseLedger);  // For the output

    std::string pBuf = strOutput.Get();

    return pBuf;
}

// Lookup a transaction or its ID (from within a ledger) based on index or
// transaction number.
//
// The transaction number is returned as a string, because I return ALL int64_t
// ints as a string (in the API.)  In C, you can just call StringToLong to
// convert it back.
// This makes it easier for the guys who use scripting languages. (This file is
// primarily for them. If you are lower-level, you should use
// OpenTransactions.h/.cpp
// and then use this one as a model for how to do it. Or use this one if you
// insist
// on straight C, since all these functions are extern "C".)
//
std::string OTAPI_Exec::Ledger_GetTransactionByIndex(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_LEDGER,
    const int32_t& nIndex) const  // returns transaction by index (from ledger)
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return "";
    }
    if (THE_LEDGER.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_LEDGER passed in!\n";
        return "";
    }

    if (0 > nIndex) {
        otErr << __FUNCTION__
              << ": nIndex is out of bounds (it's in the negative!)\n";
        return "";
    }

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theAccountID(ACCOUNT_ID);

    String strLedger(THE_LEDGER);
    Ledger theLedger(theNymID, theAccountID, theNotaryID);
    //    std::set<int64_t> setUnloaded;

    if (!theLedger.LoadLedgerFromString(strLedger)
        //        ||    !theLedger.LoadBoxReceipts(&setUnloaded)    // This is
        // done below, for the individual transaction, for better optimization.
        ) {
        String strAcctID(theAccountID);
        otErr << __FUNCTION__
              << ": Error loading ledger from string, or loading box receipts "
                 "subsequently. Acct ID: "
              << strAcctID << "\n";
        return "";
    }

    // At this point, I know theLedger loaded successfully.

    if (nIndex >= theLedger.GetTransactionCount()) {
        otErr << __FUNCTION__ << ": out of bounds: " << nIndex << "\n";
        return "";  // out of bounds. I'm saving from an OT_ASSERT_MSG()
                    // happening here. (Maybe I shouldn't.)
    }

    OTTransaction* pTransaction = theLedger.GetTransactionByIndex(nIndex);

    if (nullptr == pTransaction) {
        otErr << __FUNCTION__
              << ": Failure: good index but uncovered \"\" pointer: " << nIndex
              << "\n";
        return "";  // Weird.
    }

    const int64_t lTransactionNum = pTransaction->GetTransactionNum();
    // At this point, I actually have the transaction pointer, so let's return
    // it in string form...

    // Update: for transactions in ABBREVIATED form, the string is empty, since
    // it has never actually
    // been signed (in fact the whole point32_t with abbreviated transactions in
    // a ledger is that they
    // take up very little room, and have no signature of their own, but exist
    // merely as XML tags on
    // their parent ledger.)
    //
    // THEREFORE I must check to see if this transaction is abbreviated and if
    // so, sign it in order to
    // force the UpdateContents() call, so the programmatic user of this API
    // will be able to load it up.
    //
    if (pTransaction->IsAbbreviated()) {
        theLedger.LoadBoxReceipt(
            static_cast<int64_t>(lTransactionNum));  // I don't check return val
                                                     // here because I still
        // want it to send the abbreviated form, if this
        // fails.
        pTransaction =
            theLedger.GetTransaction(static_cast<int64_t>(lTransactionNum));
        if (nullptr == pTransaction) {
            otErr << __FUNCTION__ << ": good index but uncovered \"\" pointer "
                                     "after trying to load full version of "
                                     "receipt (from abbreviated): "
                  << nIndex << "\n";
            return "";  // Weird.
        }
        // I was doing this when it was abbreviated. But now (above) I just
        // load the box receipt itself. (This code is a hack that creates a
        // serialized abbreviated version.)
        //        OTPseudonym * pNym = OTAPI()->GetNym(theNymID,
        // "OTAPI_Exec::Ledger_GetTransactionByIndex");
        //        if (nullptr == pNym) return "";
        //
        //        pTransaction->ReleaseSignatures();
        //        pTransaction->SignContract(*pNym);
        //        pTransaction->SaveContract();
    }

    const String strOutput(*pTransaction);  // For the output
    std::string pBuf = strOutput.Get();

    return pBuf;
}

// Returns transaction by ID (transaction numbers are int64_t ints, and thus
// they are passed as strings in this OT high-level API.)
// Note: If this function returns "" for a transaction you KNOW is on
// the ledger, then you probably just need to download it. (The box receipts
// are stored in separate files and downloaded separately as well.)
//
std::string OTAPI_Exec::Ledger_GetTransactionByID(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_LEDGER,
    const int64_t& TRANSACTION_NUMBER) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return "";
    }
    if (THE_LEDGER.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_LEDGER passed in!\n";
        return "";
    }
    if (0 > TRANSACTION_NUMBER) {
        otErr << __FUNCTION__ << ": Negative: TRANSACTION_NUMBER passed in!\n";
        return "";
    }

    const int64_t lTransactionNumber = TRANSACTION_NUMBER;

    if (0 > lTransactionNumber) {
        otErr << __FUNCTION__
              << ": Bad transaction number passed in (negative value)\n";
        return "";
    }

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theAccountID(ACCOUNT_ID);

    String strLedger(THE_LEDGER);

    Ledger theLedger(theNymID, theAccountID, theNotaryID);

    if (!theLedger.LoadLedgerFromString(strLedger)) {
        String strAcctID(theAccountID);
        otErr << __FUNCTION__
              << ": Error loading ledger from string. Acct ID: " << strAcctID
              << "\n";
        return "";
    }
    // At this point, I know theLedger loaded successfully.

    OTTransaction* pTransaction =
        theLedger.GetTransaction(static_cast<int64_t>(lTransactionNumber));
    // No need to cleanup this transaction, the ledger owns it already.

    if (nullptr == pTransaction) {
        otOut << __FUNCTION__
              << ": No transaction found in ledger with that number : "
              << lTransactionNumber << ".\n";
        return "";  // Maybe he was just looking; this isn't necessarily an
                    // error.
    }

    // At this point, I actually have the transaction pointer, so let's return
    // it in string form...
    //
    const int64_t lTransactionNum = pTransaction->GetTransactionNum();
    OT_ASSERT(lTransactionNum == lTransactionNumber);

    // Update: for transactions in ABBREVIATED form, the string is empty, since
    // it has never actually
    // been signed (in fact the whole point32_t with abbreviated transactions in
    // a ledger is that they
    // take up very little room, and have no signature of their own, but exist
    // merely as XML tags on
    // their parent ledger.)
    //
    // THEREFORE I must check to see if this transaction is abbreviated and if
    // so, sign it in order to
    // force the UpdateContents() call, so the programmatic user of this API
    // will be able to load it up.
    //
    if (pTransaction->IsAbbreviated()) {
        // First we see if we are able to load the full version of this box
        // receipt.
        // (Perhaps it has already been downloaded sometime in the past, and
        // simply
        // needs to be loaded up. Worth a shot.)
        //
        const bool& bLoadedBoxReceipt = theLedger.LoadBoxReceipt(
            static_cast<int64_t>(lTransactionNum));  // I still want it to send
                                                     // the abbreviated form, if
                                                     // this fails.

        // Grab this pointer again, since the object was re-instantiated
        // in the case of a successful LoadBoxReceipt.
        //
        if (bLoadedBoxReceipt)
            pTransaction =
                theLedger.GetTransaction(static_cast<int64_t>(lTransactionNum));

        // (else if false == bLoadedBoxReceipt, then pTransaction ALREADY points
        // to the abbreviated version.)
        if (nullptr == pTransaction) {
            otErr << __FUNCTION__ << ": good ID, but uncovered \"\" pointer "
                                     "after trying to load full version of "
                                     "receipt (from abbreviated.) Probably "
                                     "just need to download this one...\n";
            return "";  // Weird.
        }
        // If it's STILL abbreviated after the above efforts, then there's
        // nothing else I can do
        // except return the abbreviated version. The caller may still need the
        // info available on
        // the abbreviated version. (And the caller may yet download the full
        // version...)
        //
        else if (pTransaction->IsAbbreviated()) {
            Nym* pNym = OTAPI()->GetNym(theNymID, __FUNCTION__);
            if (nullptr == pNym) return "";  // Weird.
            pTransaction->ReleaseSignatures();
            pTransaction->SignContract(*pNym);
            pTransaction->SaveContract();
        }
    }
    const String strOutput(*pTransaction);  // For the output
    std::string pBuf = strOutput.Get();

    return pBuf;
}

// OTAPI_Exec::Ledger_GetInstrument (by index)
//
// Lookup a financial instrument (from within a transaction that is inside
// a paymentInbox ledger) based on index.
/*
sendNymInstrument does this:
-- Puts instrument (a contract string) as encrypted Payload on an OTMessage(1).
-- Also puts instrument (same contract string) as CLEAR payload on an
OTMessage(2).
-- (1) is sent to server, and (2) is added to Outpayments messages.
-- (1) gets added to recipient's Nymbox as "in ref to" string on a
"instrumentNotice" transaction.
-- When recipient processes Nymbox, the "instrumentNotice" transaction
(containing (1) in its "in ref to"
field) is copied and added to the recipient's paymentInbox.
-- When recipient iterates through paymentInbox transactions, they are ALL
"instrumentNotice" OTMessages.
Each transaction contains an OTMessage in its "in ref to" field, and that
OTMessage object contains an
encrypted payload of the instrument itself (a contract string.)
-- When sender gets Outpayments contents, the original instrument (contract
string) is stored IN THE
CLEAR as payload on an OTMessage.

THEREFORE:
TO EXTRACT INSTRUMENT FROM PAYMENTS INBOX:
-- Iterate through the transactions in the payments inbox.
-- (They should all be "instrumentNotice" transactions.)
-- Each transaction contains (1) OTMessage in the "in ref to" field, which in
turn contains an encrypted
instrument in the messagePayload field.
-- *** Therefore, this function, based purely on ledger index (as we iterate)
extracts the
OTMessage from the Transaction "in ref to" field (for the transaction at that
index), then decrypts
the payload on that message and returns the decrypted cleartext.
*/

// DONE:  Move most of the code in the below function into
// OTLedger::GetInstrument.
//
// DONE: Finish writing OTClient::ProcessDepositResponse

std::string OTAPI_Exec::Ledger_GetInstrument(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_LEDGER,
    const int32_t& nIndex) const  // returns financial instrument by index.
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return "";
    }
    if (THE_LEDGER.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_LEDGER passed in!\n";
        return "";
    }

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theAccountID(ACCOUNT_ID);
    Nym* pNym = OTAPI()->GetNym(theNymID, __FUNCTION__);
    if (nullptr == pNym) return "";
    String strLedger(THE_LEDGER);
    Ledger theLedger(theNymID, theAccountID, theNotaryID);
    //    std::set<int64_t> setUnloaded;

    if (!theLedger.LoadLedgerFromString(strLedger)
        //        ||    !theLedger.LoadBoxReceipts(&setUnloaded)    // This is
        // now done below, for the individual transaction, for better
        // optimization.
        )  // Update: now in the theLedger.GetInstrument call.
    {
        String strNymID(theNymID);
        String strAcctID(theAccountID);
        otErr << __FUNCTION__
              << ": Error loading ledger from string. NymID / Acct ID: "
              << strNymID << " / " << strAcctID << "\n";
        return "";
    }
    // At this point, I know theLedger loaded successfully.
    //
    std::unique_ptr<OTPayment> pPayment(
        GetInstrument(*pNym, nIndex, theLedger));

    if ((nullptr == pPayment) || !pPayment->IsValid()) {
        otOut << __FUNCTION__ << ": theLedger.GetInstrument either returned "
                                 "nullptr, or an invalid instrument.\n";
    } else {
        // NOTE: instead of loading up an OTPayment, and then loading a
        // cheque/purse/etc from it,
        // we just send the cheque/purse/etc directly and use it to construct
        // the OTPayment.
        // (Saves a step.)
        //
        String strPaymentContents;

        if (!pPayment->GetPaymentContents(strPaymentContents)) {
            otOut << __FUNCTION__ << ": Failed retrieving payment instrument "
                                     "from OTPayment object.\n";
            return "";
        }
        std::string gBuf = strPaymentContents.Get();
        return gBuf;
    }

    return "";
}

/*

// returns the message, optionally with Subject: as first line.

 std::string OTAPI_Exec::GetNym_MailContentsByIndex(const std::string& NYM_ID,
const int32_t& nIndex)
{
    OT_ASSERT_MSG("" != NYM_ID, "Null NYM_ID passed to
OTAPI_Exec::GetNym_MailContentsByIndex");

    std::string strFunc = "OTAPI_Exec::GetNym_MailContentsByIndex";
    OTIdentifier    theNymID(NYM_ID);
    OTPseudonym * pNym = OTAPI()->GetNym(theNymID, strFunc);
    if (nullptr == pNym) return "";
    OTMessage * pMessage = pNym->GetMailByIndex(nIndex);

    if (nullptr != pMessage)
    {
        // SENDER:    pMessage->m_strNymID
        // RECIPIENT: pMessage->m_strNymID2
        // MESSAGE:   pMessage->m_ascPayload (in an OTEnvelope)
        //
        OTEnvelope    theEnvelope;
        OTString    strEnvelopeContents;

        // Decrypt the Envelope.
        if (theEnvelope.SetAsciiArmoredData(pMessage->m_ascPayload) &&
            theEnvelope.Open(*pNym, strEnvelopeContents))
        {
            std::string pBuf = strEnvelopeContents.Get();

            return pBuf;
        }
    }
    return "";
}

*/

// Returns a transaction number, or -1 for error.
int64_t OTAPI_Exec::Ledger_GetTransactionIDByIndex(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_LEDGER,
    const int32_t& nIndex) const  // returns transaction number by index.
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return -1;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return -1;
    }
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return -1;
    }
    if (THE_LEDGER.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_LEDGER passed in!\n";
        return -1;
    }

    if (0 > nIndex) {
        otErr << __FUNCTION__
              << ": nIndex is out of bounds (it's in the negative!)\n";
        return -1;
    }

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theAccountID(ACCOUNT_ID);

    String strLedger(THE_LEDGER);
    String strOutput("-1");  // For the output

    int64_t lTransactionNumber = 0;
    OTTransaction* pTransaction = nullptr;

    Ledger theLedger(theNymID, theAccountID, theNotaryID);

    if (!theLedger.LoadLedgerFromString(strLedger)) {
        String strAcctID(theAccountID);
        otErr << __FUNCTION__
              << ": Error loading ledger from string. Acct ID: " << strAcctID
              << "\n";
    }

    // At this point, I know theLedger loaded successfully.
    else if (nIndex >= theLedger.GetTransactionCount()) {
        otErr << __FUNCTION__ << ": out of bounds: " << nIndex << "\n";
        // out of bounds. I'm saving from an OT_ASSERT_MSG() happening here.
        // (Maybe I shouldn't.)
    } else if (
        nullptr == (pTransaction = theLedger.GetTransactionByIndex(nIndex))) {
        otErr << __FUNCTION__
              << ": good index but uncovered \"\" pointer: " << nIndex << "\n";
    }  // NO NEED TO CLEANUP the transaction, since it is already "owned" by
       // theLedger.

    // At this point, I actually have the transaction pointer, so let's get the
    // ID...
    else if (0 >= (lTransactionNumber = pTransaction->GetTransactionNum())) {
        otErr << __FUNCTION__
              << ": negative or zero transaction num: " << lTransactionNumber
              << "\n";
        return -1;
    } else  // success
    {
        return lTransactionNumber;
    }

    return -1;
}

// Add a transaction to a ledger.
// (Returns the updated ledger.)
//
std::string OTAPI_Exec::Ledger_AddTransaction(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_LEDGER,
    const std::string& THE_TRANSACTION) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return "";
    }
    if (THE_LEDGER.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_LEDGER passed in!\n";
        return "";
    }
    if (THE_TRANSACTION.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_TRANSACTION passed in!\n";
        return "";
    }

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theAccountID(ACCOUNT_ID);

    String strLedger(THE_LEDGER);
    String strTransaction(THE_TRANSACTION);

    Nym* pNym = OTAPI()->GetOrLoadPrivateNym(
        theNymID, false, __FUNCTION__);  // These copiously log, and ASSERT.
    if (nullptr == pNym) return "";

    Ledger theLedger(theNymID, theAccountID, theNotaryID);

    if (!theLedger.LoadLedgerFromString(strLedger)) {
        String strAcctID(theAccountID);
        otErr << __FUNCTION__
              << ": Error loading ledger from string. Acct ID: " << strAcctID
              << "\n";
        return "";
    } else if (!theLedger.VerifyAccount(*pNym)) {
        String strAcctID(theAccountID);
        otErr << __FUNCTION__ << ": Error verifying ledger in "
                                 "OTAPI_Exec::Ledger_AddTransaction. Acct ID: "
              << strAcctID << "\n";
        return "";
    }

    // At this point, I know theLedger loaded and verified successfully.

    OTTransaction* pTransaction =
        new OTTransaction(theNymID, theAccountID, theNotaryID);
    if (nullptr == pTransaction) {
        otErr << __FUNCTION__ << ": Error allocating memory in the OTAPI: "
              << "pTransaction"
              << " !\n";
        return "";
    }

    if (!pTransaction->LoadContractFromString(strTransaction)) {
        String strAcctID(theAccountID);
        otErr << __FUNCTION__
              << ": Error loading transaction from string. Acct ID: "
              << strAcctID << "\n";
        delete pTransaction;
        pTransaction = nullptr;
        return "";
    } else if (!pTransaction->VerifyAccount(*pNym)) {
        String strAcctID(theAccountID);
        otErr << __FUNCTION__
              << ": Error verifying transaction. Acct ID: " << strAcctID
              << "\n";
        delete pTransaction;
        pTransaction = nullptr;
        return "";
    }
    // At this point, I know pTransaction loaded and verified successfully.
    // So let's add it to the ledger, save, and return updated ledger in string
    // form.

    theLedger.AddTransaction(
        *pTransaction);  // Ledger now "owns" it and will handle cleanup.

    theLedger.ReleaseSignatures();
    theLedger.SignContract(*pNym);
    theLedger.SaveContract();

    String strOutput(theLedger);  // For the output

    std::string pBuf = strOutput.Get();

    return pBuf;
}

// Create a 'response' transaction, that will be used to indicate my
// acceptance or rejection of another transaction. Usually an entire
// ledger full of these is sent to the server as I process the various
// transactions in my inbox.
//
// The original transaction is passed in, and I generate a response transaction
// based on it.
// Also, the response ledger is passed in, so I load it, add the response
// transaction, save
// it back to string, and return the string.
//
// This way, users can call this function multiple times, adding transactions
// until done.
//
std::string OTAPI_Exec::Transaction_CreateResponse(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_LEDGER,       // 'Response' ledger be sent to the
                                         // server...
    const std::string& THE_TRANSACTION,  // Responding to...?
    const bool& BOOL_DO_I_ACCEPT) const  // 0 or 1  (true or false.)
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return "";
    }
    if (THE_LEDGER.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_LEDGER passed in!\n";
        return "";
    }
    if (THE_TRANSACTION.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_TRANSACTION passed in!\n";
        return "";
    }

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theAcctID(ACCOUNT_ID);

    String strLedger(THE_LEDGER);
    String strTransaction(THE_TRANSACTION);
    auto pServer = OTAPI()->GetServer(theNotaryID, __FUNCTION__);
    if (!pServer) {
        return "";
    }
    auto pServerNym = pServer->Nym();

    if (!pServerNym) {
        otOut << __FUNCTION__
              << ": No Contract Nym found in that Server Contract.\n";
        return "";
    }
    // By this point, pServerNym is a good pointer.  (No need to cleanup.)
    Nym* pNym = OTAPI()->GetOrLoadPrivateNym(
        theNymID, false, __FUNCTION__);  // This logs, ASSERTs, etc.
    if (nullptr == pNym) return "";
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    Ledger theLedger(theNymID, theAcctID, theNotaryID);

    if (!theLedger.LoadLedgerFromString(strLedger)) {
        String strAcctID(theAcctID);
        otErr << __FUNCTION__
              << ": Error loading ledger from string. Acct ID: " << strAcctID
              << "\n";
        return "";
    } else if (!theLedger.VerifyAccount(*pNym)) {
        String strAcctID(theAcctID);
        otErr << __FUNCTION__
              << ": Error verifying ledger. Acct ID: " << strAcctID << "\n";
        return "";
    }

    // At this point, I know theLedger loaded and verified successfully.
    // (This is the 'response' ledger that the user previously generated,
    // and now he is loading it up with responses that this function will
    // generate on his behalf.)
    OTTransaction theTransaction(theNymID, theAcctID, theNotaryID);

    if (!theTransaction.LoadContractFromString(strTransaction)) {
        String strAcctID(theAcctID);
        otErr << __FUNCTION__
              << ": Error loading transaction from string. Acct ID: "
              << strAcctID << "\n";
        return "";
    }
    OTTransaction* pTransaction = nullptr;
    std::unique_ptr<OTTransaction> theTransAngel;

    if (theTransaction.IsAbbreviated()) {
        pTransaction =
            LoadBoxReceipt(theTransaction, static_cast<int64_t>(Ledger::inbox));

        if (nullptr == pTransaction) {
            String strAcctID(theAcctID);
            otErr << __FUNCTION__ << ": Error loading full transaction from "
                                     "abbreviated version of inbox receipt. "
                                     "Acct ID: "
                  << strAcctID << "\n";
            return "";
        }
        theTransAngel.reset(pTransaction);
    } else
        pTransaction = &theTransaction;
    // BELOW THIS POINT, only use pTransaction, not theTransaction.

    // This transaction is presumably from the server, since we are in this
    // function in order to generate a response back to the server. So therefore
    // I want to verify that the server has actually signed the thing, before
    // I go off responding to it like a damned fool.
    //
    if (false == pTransaction->VerifyAccount(*pServerNym)) {
        String strAcctID(theAcctID);
        otErr << __FUNCTION__
              << ": Error verifying transaction. Acct ID: " << strAcctID
              << "\n";
        return "";
    }

    if ((OTTransaction::pending != pTransaction->GetType()) &&
        (OTTransaction::chequeReceipt != pTransaction->GetType()) &&
        (OTTransaction::voucherReceipt != pTransaction->GetType()) &&
        (OTTransaction::transferReceipt != pTransaction->GetType()) &&
        (OTTransaction::marketReceipt != pTransaction->GetType()) &&
        (OTTransaction::paymentReceipt != pTransaction->GetType()) &&
        (OTTransaction::finalReceipt != pTransaction->GetType()) &&
        (OTTransaction::basketReceipt != pTransaction->GetType())) {
        otErr << __FUNCTION__
              << ": wrong transaction type: " << pTransaction->GetTypeString()
              << ".\n";
        return "";
    }
    // At this point, I know pTransaction loaded and verified successfully.
    // So let's generate a response item based on it, and add it to a
    // processInbox
    // transaction to be added to that ledger (if one's not already there...)

    // First, check to see if there is a processInbox transaction already on
    // the ledger...
    OTTransaction* pResponse =
        theLedger.GetTransaction(OTTransaction::processInbox);

    // If it's not already there, create it and add it.
    if (nullptr == pResponse) {
        String strNotaryID(theNotaryID);
        int64_t lTransactionNumber = 0;
        bool bGotTransNum =
            pNym->GetNextTransactionNum(*pNym, strNotaryID, lTransactionNumber);

        if (!bGotTransNum) {
            String strNymID(theNymID);
            otOut << __FUNCTION__
                  << ": User is all out of transaction numbers:\n"
                  << strNymID << "\n";
            return "";
        }

        pResponse = OTTransaction::GenerateTransaction(
            theNymID,
            theAcctID,
            theNotaryID,
            OTTransaction::processInbox,
            lTransactionNumber);

        if (nullptr == pResponse) {
            String strAcctID(theAcctID);
            otErr << __FUNCTION__
                  << ": Error generating processInbox transaction for AcctID: "
                  << strAcctID << "\n";

            pNym->AddTransactionNum(
                *pNym,
                strNotaryID,
                lTransactionNumber,
                true);  // bSave=true.  Have to add this back
                        // since we failed to use it.

            return "";
        }

        theLedger.AddTransaction(
            *pResponse);  // Ledger now "owns" it and will handle cleanup.
    }

    // At this point32_t I know pResponse is a processInbox transaction, ready
    // to go,
    // and that theLedger will handle any cleanup issues related to it.

    // Next let's create a new item that responds to pTransaction, and add that
    // item to pResponse. Then we'll return the updated ledger.

    Item::itemType theAcceptItemType = Item::error_state;
    Item::itemType theRejectItemType = Item::error_state;

    switch (pTransaction->GetType()) {
        case OTTransaction::pending:
            theAcceptItemType = Item::acceptPending;
            theRejectItemType = Item::rejectPending;
            break;

        case OTTransaction::marketReceipt:
        case OTTransaction::paymentReceipt:
            theAcceptItemType = Item::acceptCronReceipt;
            theRejectItemType = Item::disputeCronReceipt;
            break;

        case OTTransaction::chequeReceipt:
        case OTTransaction::voucherReceipt:
        case OTTransaction::transferReceipt:
            theAcceptItemType = Item::acceptItemReceipt;
            theRejectItemType = Item::disputeItemReceipt;
            break;

        case OTTransaction::finalReceipt:
            theAcceptItemType = Item::acceptFinalReceipt;
            theRejectItemType = Item::disputeFinalReceipt;
            break;

        case OTTransaction::basketReceipt:
            theAcceptItemType = Item::acceptBasketReceipt;
            theRejectItemType = Item::disputeBasketReceipt;
            break;

        default:
            theAcceptItemType = Item::error_state;
            theRejectItemType = Item::error_state;
            otErr << __FUNCTION__ << ": Unexpected transaction type in: "
                  << pTransaction->GetTypeString() << "\n";
            return "";
    }
    int64_t lReferenceTransactionNum = 0;
    int64_t lNumberOfOrigin = 0;
    String strNote;
    switch (pTransaction->GetType()) {
        case OTTransaction::marketReceipt:
        case OTTransaction::paymentReceipt:
        case OTTransaction::finalReceipt:
        case OTTransaction::basketReceipt:
            lNumberOfOrigin = pTransaction->GetReferenceToNum();
            lReferenceTransactionNum =
                pTransaction->GetTransactionNum();  // <=== References the
                                                    // receipt in
                                                    // my inbox.
            break;

        case OTTransaction::transferReceipt:  // Contains "in ref to"
                                              // acceptPending
                                              // item from someone who processed
        // their inbox to accept my transfer.
        case OTTransaction::pending:  // Contains "in ref to" transfer item from
                                      // someone who sent me a transfer.
        case OTTransaction::chequeReceipt:  // Contains "in ref to"
                                            // depositCheque
        // item from someone who deposited my
        // cheque.
        case OTTransaction::voucherReceipt:  // Contains "in ref to"
                                             // depositCheque
            // item from someone who deposited my
            // voucher.
            {
                // Here's some code in case you need to load up the item.
                String strReference;
                pTransaction->GetReferenceString(strReference);

                if (!strReference.Exists()) {
                    otErr << __FUNCTION__
                          << ": No reference string found on transaction.\n";
                    return "";
                }
                std::unique_ptr<Item> pOriginalItem(
                    Item::CreateItemFromString(
                        strReference,
                        theNotaryID,
                        pTransaction->GetReferenceToNum()));

                if (nullptr == pOriginalItem) {
                    otErr << __FUNCTION__
                          << ": Failed loading transaction item from string.\n";
                    return "";
                }
                // pItem will be automatically cleaned up when it goes out of
                // scope.
                if ((Item::request != pOriginalItem->GetStatus()) ||
                    ((Item::acceptPending !=
                      pOriginalItem->GetType()) &&  // I'm accepting a transfer
                                                    // receipt
                     // that was created by someone's
                     // acceptPending (from a transfer I
                     // sent.)
                     (Item::transfer !=
                      pOriginalItem->GetType()) &&  // I'm accepting a pending
                                                    // transfer
                     // that was created by someone's
                     // transfer to me.
                     (Item::depositCheque !=
                      pOriginalItem->GetType())  // I'm accepting a cheque or
                                                 // voucher
                     // receipt that was created by someone's
                     // depositCheque (of a cheque I wrote or
                     // a voucher I remitted.)
                     )) {
                    otErr << __FUNCTION__
                          << ": Wrong item type or status attached "
                             "as reference on transaction.\n";
                    return "";
                }
                if (Item::transfer == pOriginalItem->GetType())
                    pOriginalItem->GetNote(strNote);
                lNumberOfOrigin = pOriginalItem->GetNumberOfOrigin();
                lReferenceTransactionNum =
                    pTransaction->GetTransactionNum();  // <=== References the
                                                        // receipt in
                                                        // my inbox.
                //            lReferenceTransactionNum =
                // pOriginalItem->GetReferenceToNum();  // <=== References the
                // original
                // transfer I sent, or N/A (for pending), or cheque I wrote.
                //            lReferenceTransactionNum =
                // pOriginalItem->GetTransactionNum();  // <=== References
                // someone
                // else's transaction that put the receipt into my inbox.
            }
            break;

        default:
            otErr << __FUNCTION__ << ": Unexpected transaction type in: "
                  << pTransaction->GetTypeString() << "\n";
            return "";
    }
    Item* pAcceptItem = Item::CreateItemFromTransaction(
        *pResponse,
        (true == BOOL_DO_I_ACCEPT) ? theAcceptItemType
                                   : theRejectItemType);  // set above.
    pAcceptItem->SetNumberOfOrigin(lNumberOfOrigin);
    // Set up the "accept" transaction item to be sent to the server
    // (this item references and accepts another item by its transaction
    // number--
    //  one that is already there in my inbox)

    // This is critical. Server needs this to look up the original.
    pAcceptItem->SetReferenceToNum(lReferenceTransactionNum);
    // Don't need to set transaction num on item since the constructor already
    // got it off the owner transaction.
    pAcceptItem->SetAmount(pTransaction->GetReceiptAmount());  // Server
    // validates this,
    // so make sure
    // it's right.
    if (strNote.Exists()) pAcceptItem->SetNote(strNote);
    // sign the item
    pAcceptItem->SignContract(*pNym);
    pAcceptItem->SaveContract();
    // Make sure there's not already a response item in reference to the same
    // receipt.
    //
    // UPDATE, NOTE: Turns out, it's normal to have multiple receipts in
    // reference to the same thing.
    // For example, I might have two transfer receipts that are both in
    // reference to the same notarizeInbox.
    //
    //    OTItem * pExistingItem =
    // pResponse->GetItemInRefTo(lReferenceTransactionNum);
    //    if (nullptr != pExistingItem)
    //    {
    //        otErr << __FUNCTION__ << ": Error: There's already a response item
    // in
    // reference to the same receipt! (In Ref: " <<
    // static_cast<int64_t>(lReferenceTransactionNum) << " User: " << NYM_ID <<
    // " Account: " << ACCOUNT_ID << ")
    // Failure.\n\n";
    //
    //        const OTString strAccept(*pAcceptItem);
    //        otErr << "===> Failed accept item:\n" << strAccept << "\n\n";
    //
    //        const OTString strExisting(*pExistingItem);
    //        otErr << "===> Pre-existing item:\n" << strExisting << "\n\n";
    //
    //        return "";
    //    }
    // the transaction will handle cleaning up the transaction item.
    pResponse->AddItem(*pAcceptItem);

    // I don't attach the original item here because I already reference it by
    // transaction num,
    // and because the server already has it and sent it to me. SO I just need
    // to give the server
    // enough info to look it up again.

    pResponse->ReleaseSignatures();
    pResponse->SignContract(*pNym);
    pResponse->SaveContract();

    theLedger.ReleaseSignatures();
    theLedger.SignContract(*pNym);
    theLedger.SaveContract();

    String strOutput(theLedger);  // For the output

    std::string pBuf = strOutput.Get();

    return pBuf;
}

// (Response Ledger) LEDGER FINALIZE RESPONSE
//
// AFTER you have set up all the transaction responses, call THIS function
// to finalize them by adding a BALANCE AGREEMENT.
//
// MAKE SURE you have the latest copy of the account file, inbox file, and
// outbox file, since we will need those in here to create the balance statement
// properly.
//
// (Client software may wish to check those things, when downloaded, against
// the local copies and the local signed receipts. In this way, clients can
// protect themselves against malicious servers.)
//
std::string OTAPI_Exec::Ledger_FinalizeResponse(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_LEDGER) const  // 'Response' ledger be sent to the
                                          // server...
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return "";
    }
    if (THE_LEDGER.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_LEDGER passed in!\n";
        return "";
    }

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theAcctID(ACCOUNT_ID);

    String strLedger(THE_LEDGER), strNotaryID(theNotaryID);
    auto pServer = OTAPI()->GetServer(theNotaryID, __FUNCTION__);
    if (!pServer) return "";
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    auto pServerNym = pServer->Nym();

    if (!pServerNym) {
        otOut << __FUNCTION__
              << ": No Contract Nym found in that Server Contract.\n";
        return "";
    }
    // By this point, pServerNym is a good pointer.  (No need to cleanup.)
    Nym* pNym = OTAPI()->GetOrLoadPrivateNym(
        theNymID, false, __FUNCTION__);  // This logs, ASSERTs, etc.
    if (nullptr == pNym) return "";
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    Ledger theLedger(theNymID, theAcctID, theNotaryID);

    if (!theLedger.LoadLedgerFromString(strLedger)) {
        String strAcctID(theAcctID);
        otErr << __FUNCTION__
              << ": Error loading ledger from string. Acct ID: " << strAcctID
              << "\n";
        return "";
    } else if (!theLedger.VerifyAccount(*pNym)) {
        String strAcctID(theAcctID);
        otErr << __FUNCTION__
              << ": Error verifying ledger. Acct ID: " << strAcctID << "\n";
        return "";
    }

    // At this point, I know theLedger loaded and verified successfully.
    // (This is the 'response' ledger that the user previously generated
    // in response to the various inbox receipts, and now he is loading
    // it up with responses that this function will finalize for sending.)

    // First, check to see if there is a processInbox transaction already on
    // the ledger...
    OTTransaction* pTransaction =
        theLedger.GetTransaction(OTTransaction::processInbox);

    // If it's not already there, create it and add it.
    if (nullptr == pTransaction) {
        String strAcctID(theAcctID);
        otErr << __FUNCTION__
              << ": Error finding processInbox transaction for AcctID: "
              << strAcctID << "\n";
        return "";
    }
    // At this point32_t I know pTransaction is a processInbox transaction,
    // ready to go,
    // and that theLedger will handle any cleanup issues related to it.
    // If balance statement is already there, return.
    if (nullptr != pTransaction->GetItem(Item::balanceStatement)) {
        otErr << __FUNCTION__
              << ": this response has already been finalized.\n";
        return "";
    }
    // Get the account.
    Account* pAccount = OTAPI()->GetAccount(theAcctID, __FUNCTION__);
    if (nullptr == pAccount) return "";
    // Load the inbox and outbox.

    Ledger theInbox(theNymID, theAcctID, theNotaryID);
    Ledger theOutbox(theNymID, theAcctID, theNotaryID);

    if (!theInbox.LoadInbox() || !theInbox.VerifyAccount(*pNym)) {
        otOut << __FUNCTION__ << ": Unable to load or verify Inbox for acct "
              << ACCOUNT_ID << "\n";
        return "";
    }

    if (!theOutbox.LoadOutbox() || !theOutbox.VerifyAccount(*pNym)) {
        otOut << __FUNCTION__ << ": Unable to load or verify Outbox for acct "
              << ACCOUNT_ID << "\n";
        return "";
    }

    // Setup balance agreement item here!
    // Adapting code from OTServer... with comments:
    //
    // This transaction accepts various incoming pending transfers.
    // So when it's all done, my balance will be higher.
    // AND pending inbox items will be removed from my inbox.
    //
    // I would like to not even process the whole giant loop below,
    // if I can verify here now that the balance agreement is wrong.
    //
    // Thus I will actually loop through the acceptPending items in
    // pTransaction, and then for each one, I'll
    // lookup the ACTUAL transaction in the inbox, and get its ACTUAL value.
    // (And total them all up.)
    //
    // The total of those, (WITHOUT the user having to tell me what it will be,
    // since I'm looking them all up),
    // should equal the difference in the account balance! Meaning the current
    // balance plus that total will be
    // the expected NEW balance, according to this balance agreement -- if it
    // wants to be approved, that is.
    //
    //

    bool bSuccessFindingAllTransactions = true;
    int64_t lTotalBeingAccepted = 0;

    std::list<int64_t> theListOfInboxReceiptsBeingRemoved;

    Nym theTempNym;

    for (auto& it_bigloop : pTransaction->GetItemList()) {
        Item* pItem = it_bigloop;
        if (nullptr == pItem) {
            otErr << __FUNCTION__
                  << ": Pointer: pItem should not have been \"\".\n";
            return "";
        }

        if ((pItem->GetType() == Item::acceptPending) ||
            (pItem->GetType() == Item::acceptItemReceipt)) {

            //            if
            // (theInbox.GetTransactionCountInRefTo(pItem->GetReferenceToNum())
            // > 1)
            //                otErr << __FUNCTION__ << ": WARNING: There are
            // MULTIPLE
            // receipts 'in reference to' " << pItem->GetReferenceToNum() << ".
            // (It will return the first
            // one...)\n";

            OTTransaction* pServerTransaction =
                theInbox.GetTransaction(pItem->GetReferenceToNum());

            otWarn << __FUNCTION__
                   << ": Checking inbox for expected pending or receipt ("
                   << pItem->GetReferenceToNum() << ") Nym: " << NYM_ID
                   << "\n";  // temp remove

            if (nullptr == pServerTransaction) {
                bSuccessFindingAllTransactions = false;
                otOut << __FUNCTION__ << ": Expected receipt "
                      << pItem->GetReferenceToNum()
                      << " NOT found! (Do you have the latest inbox?)\n";
                break;
            } else {
                bSuccessFindingAllTransactions = true;

                // IF I'm accepting a pending transfer, then add the amount to
                // my counter of total amount being accepted.
                //
                // ELSE if I'm accepting an item receipt (which will remove my
                // responsibility for that item) then add it
                // to the temp Nym (which is a list of transaction numbers that
                // will be removed from my responsibility if
                // all is successful.)  Also remove all the Temp Nym numbers
                // from pNym, so we can verify the Balance
                // Statement AS IF they were already removed. Add them
                //
                if (pItem->GetType() == Item::acceptPending)  // acceptPending
                    lTotalBeingAccepted +=
                        pServerTransaction
                            ->GetReceiptAmount();  // <============================

                else if (
                    pItem->GetType() ==
                    Item::acceptItemReceipt)  // acceptItemReceipt
                {
                    // What number do I remove here? the user is accepting a
                    // transfer receipt, which
                    // is in reference to the recipient's acceptPending. THAT
                    // item is in reference to
                    // my original transfer (or contains a cheque with my
                    // original number.) (THAT's the # I need.)
                    //
                    String strOriginalItem;
                    pServerTransaction->GetReferenceString(strOriginalItem);

                    std::unique_ptr<Item> pOriginalItem(
                        Item::CreateItemFromString(
                            strOriginalItem,
                            Identifier(NOTARY_ID),
                            pServerTransaction->GetReferenceToNum()));

                    if (nullptr != pOriginalItem) {
                        // If pOriginalItem is acceptPending, that means the
                        // client is accepting the transfer receipt from the
                        // server, (from his inbox),
                        // which has the recipient's acceptance inside of the
                        // client's transfer as the original item. This means
                        // the transfer that
                        // the client originally sent is now finally closed!
                        //
                        // If it's a depositCheque, that means the client is
                        // accepting the cheque receipt from the server,
                        // (from his inbox)
                        // which has the recipient's deposit inside of it as the
                        // original item. This means that the cheque that
                        // the client originally wrote is now finally closed!
                        //
                        // In both cases, the "original item" itself is not from
                        // the client, but from the recipient! Therefore,
                        // the number on that item is useless for removing
                        // numbers from the client's list of issued numbers.
                        // Rather, I need to load that original cheque, or
                        // pending transfer, from WITHIN the original item,
                        // in order to get THAT number, to remove it from the
                        // client's issued list. (Whether for real, or for
                        // setting up dummy data in order to verify the balance
                        // agreement.) *sigh*
                        //
                        if (Item::depositCheque ==
                            pOriginalItem->GetType())  // client is accepting a
                                                       // cheque receipt, which
                                                       // has a depositCheque
                        // (from the recipient) as
                        // the original item
                        // within.
                        {
                            // Get the cheque from the Item and load it up into
                            // a Cheque object.
                            String strCheque;
                            pOriginalItem->GetAttachment(strCheque);

                            Cheque theCheque;  // allocated on the stack :-)

                            if (false ==
                                ((strCheque.GetLength() > 2) &&
                                 theCheque.LoadContractFromString(strCheque))) {
                                otErr << __FUNCTION__
                                      << ": ERROR loading cheque from string:\n"
                                      << strCheque << "\n";
                            } else  // Since the client wrote the cheque, and he
                                    // is now accepting the cheque receipt, he
                                    // can be cleared for that transaction
                                    // number...
                            {
                                if (pNym->VerifyIssuedNum(
                                        strNotaryID,
                                        theCheque.GetTransactionNum()))
                                    theTempNym.AddIssuedNum(
                                        strNotaryID,
                                        theCheque.GetTransactionNum());
                                else
                                    otErr << __FUNCTION__
                                          << ": cheque receipt, trying to "
                                             "'remove' an issued number ("
                                          << theCheque.GetTransactionNum()
                                          << ") that already wasn't on my "
                                             "issued list. (So what is this in "
                                             "my inbox, then? Maybe need to "
                                             "download a fresh copy of it.)\n";
                            }
                        }
                        // client is accepting a transfer receipt, which has an
                        // acceptPending from the recipient
                        // as the original item within.
                        //
                        else if (
                            Item::acceptPending ==
                            pOriginalItem->GetType())  // (which is in
                                                       // reference to the
                                                       // client's outgoing
                                                       // original
                                                       // transfer.)
                        {
                            if (pNym->VerifyIssuedNum(
                                    strNotaryID,
                                    pOriginalItem->GetNumberOfOrigin()))
                                theTempNym.AddIssuedNum(
                                    strNotaryID,
                                    pOriginalItem->GetNumberOfOrigin());
                            else
                                otErr << __FUNCTION__
                                      << ": transferReceipt, trying to "
                                         "'remove' an issued number ("
                                      << pOriginalItem->GetNumberOfOrigin()
                                      << ") that already wasn't on my issued "
                                         "list. (So what is this in my inbox, "
                                         "then? Maybe need to download a fresh "
                                         "copy of it.)\n";
                        } else  // wrong type.
                        {
                            String strOriginalItemType;
                            pOriginalItem->GetTypeString(strOriginalItemType);
                            otErr << __FUNCTION__
                                  << ": Original item has wrong type, while "
                                     "accepting item receipt:\n"
                                  << strOriginalItemType << "\n";
                        }
                    } else {
                        otErr << __FUNCTION__
                              << ": Unable to load original item from string "
                                 "while accepting item receipt:\n"
                              << strOriginalItem << "\n";
                    }
                }  // acceptItemReceipt

                // I'll also go ahead and remove each transaction from theInbox,
                // and pass said inbox into the VerifyBalanceAgreement call...
                // (So it can simulate as if the inbox was already changed, and
                // the total is already calculated, and if it succeeds,
                // then we can allow the giant loop below to do it all for
                // real.)
                // (I'm not saving this copy of the inbox anyway--there's
                // another one below.)
                //
                // theInbox.RemoveTransaction(pServerTransaction->GetTransactionNum());
                // // <================
                // Now this is done AFTER this loop...
                //
                theListOfInboxReceiptsBeingRemoved.push_back(
                    pServerTransaction->GetTransactionNum());

            }  // pServerTransaction != ""
        }      // if pItem type is accept pending or item receipt.
        else if (
            (pItem->GetType() == Item::acceptCronReceipt) ||
            (pItem->GetType() == Item::acceptFinalReceipt) ||
            (pItem->GetType() == Item::acceptBasketReceipt)) {
            OTTransaction* pServerTransaction =
                theInbox.GetTransaction(pItem->GetReferenceToNum());

            otInfo << __FUNCTION__
                   << ": Checking client-side inbox for expected cron or final "
                      "or basket receipt: "
                   << pItem->GetReferenceToNum() << "... ";  // temp remove

            if (nullptr == pServerTransaction) {
                bSuccessFindingAllTransactions = false;
                otInfo << __FUNCTION__ << ": NOT found!(Do you have the latest "
                                          "inbox ? )\n";  // temp remove
                break;
            } else {
                bSuccessFindingAllTransactions = true;

                switch (pItem->GetType()) {

                    case Item::acceptCronReceipt:
                        // pServerTransaction is a marketReceipt or
                        // paymentReceipt
                        //

                        // When accepting a cron receipt from the inbox, you
                        // don't
                        // have to clear the issued transaction number.
                        // In this case, the original trans# is cleared when the
                        // finalReceipt is generated,
                        // and the closing trans# is cleared when the
                        // finalReceipt
                        // is cleared.

                        // So NO issued numbers being removed or added in this
                        // case.
                        // (But we still remove the receipt from our copy of the
                        // inbox, below,
                        // so that the balance agreement will reflect as if it
                        // had
                        // already been
                        // successfully removed. (Because balance agreement is
                        // meant
                        // to show the new
                        // state of things, in the event of success--a signed
                        // record
                        // of those things.)
                        break;

                    case Item::acceptFinalReceipt:
                        // pServerTransaction is a finalReceipt

                        // IN THIS CASE: If we're accepting a finalReceipt, that
                        // means all the OTHER receipts related to it
                        // (sharing the same "in reference to") must ALSO be
                        // cleared
                        // from the inbox aint64_t with it! That's the
                        // whole point32_t of the finalReceipt -- to make sure
                        // all
                        // related receipts are cleared, when IT is.
                        //
                        // The server WILL verify this also (I tested it)...
                        // That's
                        // why we check here, to save the trouble
                        // of being rejected by the server.
                        //
                        // So let's see if the number of related receipts on
                        // this
                        // process inbox (pTransaction) matches
                        // the number of related receipts in the actual inbox
                        // (theInbox), as found by the finalReceipt's
                        // (pServerTransaction) "in reference to" value, which
                        // should be the same as on the related receipts.

                        // (Below) pTransaction is the processInbox transaction.
                        // Each item on it is in ref to a DIFFERENT receipt,
                        // even though, if they are marketReceipts, all of THOSE
                        // receipts are in ref to the original transaction#.
                        {
                            //                      int32_t nRefCount = 0;
                            std::set<int64_t> setOfRefNumbers;  // we'll store
                                                                // them
                            // here, to disallow
                            // duplicates, to
                            // make sure they are
                            // all unique IDs

                            //
                            // I need to loop through all items on pTransaction
                            // (my
                            // processInbox request)
                            // For each, look it up on the inbox. (Each item may
                            // be
                            // "in reference to"
                            // one original transaction or another.) FIND THE
                            // ONES
                            // that are in reference to
                            // the same # as pServerTransaction is.
                            //
                            for (auto& it : pTransaction->GetItemList()) {
                                Item* pItemPointer = it;
                                if (nullptr == pItemPointer) {
                                    otErr << __FUNCTION__
                                          << ": Pointer: "
                                             "pItemPointer should "
                                             "not have been .\n";
                                    return "";
                                }

                                // pItemPointer->GetReferenceToNum() is the
                                // server's
                                // transaction number for the receipt
                                // that it dropped into my inbox. pTransPointer
                                // is
                                // the receipt itself (hopefully.)
                                OTTransaction* pTransPointer =
                                    theInbox.GetTransaction(
                                        pItemPointer->GetReferenceToNum());

                                // Careful on the logic here...
                                // ONCE EACH INBOX RECEIPT IS DEFINITELY NOT-"",
                                // and
                                // if *IT* is "in reference to"
                                // pServerTransaction->GetReferenceToNum(),
                                // Then increment the count for the transaction.
                                // COMPARE *THAT* to theInbox.GetCount and we're
                                // golden!!
                                // Perhaps the finalReceipt is in reference to
                                // #10,
                                // and there are 6 others that are ALSO in
                                // reference
                                // to #10.
                                // That's a total of 7 receipts in the inbox
                                // that
                                // are in reference to #10, so my request had
                                // better
                                // have the
                                // same count :-)
                                //
                                if ((nullptr != pTransPointer) &&
                                    (pTransPointer->GetReferenceToNum() ==
                                     pServerTransaction->GetReferenceToNum())) {
                                    //                              nRefCount++;
                                    // std::set doesn't allow duplicates.
                                    setOfRefNumbers.insert(
                                        pItemPointer->GetReferenceToNum());
                                }
                            }

                            //
                            if (static_cast<int32_t>(
                                    setOfRefNumbers.size()) !=  // IS NOT EQUAL
                                                                // TO...
                                theInbox.GetTransactionCountInRefTo(
                                    pServerTransaction->GetReferenceToNum()))
                            /* todo: Notice I'm not making sure the count is
                            entirely composed of ACCEPTED receipts. (vs
                            DISPUTED...)
                            I probably should add code to GetItemCountInRefTo()
                            so it only counts ACCEPTED receipts.*/
                            {
                                otOut
                                    << __FUNCTION__
                                    << ": When accepting a finalReceipt, you "
                                       "MUST "
                                       "accept all related receipts (ones that "
                                       "share the same IN REFERENCE TO "
                                       "transaction "
                                       "number as the finalReceipt "
                                    << pServerTransaction->GetReferenceToNum()
                                    << ")\n"
                                       "Transaction item count (in ref to): "
                                    << setOfRefNumbers.size()
                                    << "    Inbox transaction count (in ref "
                                       "to): "
                                    << theInbox.GetTransactionCountInRefTo(
                                           pServerTransaction
                                               ->GetReferenceToNum())
                                    << ".\n";

                                bSuccessFindingAllTransactions = false;
                                break;
                            }
                            // Else NO BREAK;
                            // break;  FALLING THROUGH TO BELOW, to do the
                            // pNym/theTempNym stuff in the BASKET section...

                            // pServerTransaction->GetReferenceToNum() is the
                            // OPENING number and should already be closed.
                            //
                            // IN fact, since the "in reference to" is
                            // supposedly
                            // already closed, then let's just
                            // MAKE SURE of that, since otherwise it'll screw up
                            // my
                            // future balance agreements. (The
                            // instant a finalReceipt appears, the "in ref to"
                            // is
                            // already gone on the server's side.)
                            //
                            if (OTTransaction::finalReceipt ==
                                pServerTransaction->GetType()) {
                                if (pNym->RemoveIssuedNum(
                                        *pNym,
                                        strNotaryID,
                                        pServerTransaction->GetReferenceToNum(),
                                        true))  // bool bSave=true
                                    otWarn
                                        << __FUNCTION__
                                        << ": **** Due to finding a "
                                           "finalReceipt, "
                                           "REMOVING OPENING NUMBER FROM NYM:  "
                                        << pServerTransaction
                                               ->GetReferenceToNum()
                                        << " \n";
                                else
                                    otWarn
                                        << __FUNCTION__
                                        << ": **** Noticed a finalReceipt, but "
                                           "Opening Number "
                                        << pServerTransaction
                                               ->GetReferenceToNum()
                                        << " had ALREADY been removed from "
                                           "nym. \n";
                            } else
                                otErr
                                    << __FUNCTION__
                                    << ": Expected pServerTransaction to be a "
                                       "final receipt (while finalizing for "
                                       "process inbox.)\n";
                            //
                            // pNym won't actually save unless it actually
                            // removes
                            // that #. If the #'s already NOT THERE,
                            // then the removal will fail, and thus it won't
                            // bother
                            // saving here.
                        }

                    // ... (FALL THROUGH) ...

                    case Item::acceptBasketReceipt:
                        // pServerTransaction is a basketReceipt (or
                        // finalReceipt,
                        // since falling through from above.)
                        //
                        // Remove the proper issued number, based on the CLOSING
                        // TRANSACTION NUMBER
                        // of the finalReceipt/basketReceipt I'm accepting...
                        //

                        if (pNym->VerifyIssuedNum(
                                strNotaryID,
                                pServerTransaction->GetClosingNum()))
                            theTempNym.AddIssuedNum(
                                strNotaryID,
                                pServerTransaction->GetClosingNum());
                        else
                            otErr
                                << __FUNCTION__
                                << ": final or basket Receipt, trying to "
                                   "'remove' an issued number ("
                                << pServerTransaction->GetClosingNum()
                                << ") that already wasn't on my issued list. "
                                   "(So "
                                   "what is this in my inbox, then? Maybe need "
                                   "to download a fresh copy of it.)\n";
                        break;

                    default: {
                        String strTempType;
                        pItem->GetTypeString(strTempType);
                        otErr << __FUNCTION__
                              << ": Unexpected item type: " << strTempType
                              << "\n";
                        break;
                    }
                }

                // I'll also go ahead and remove each transaction from theInbox,
                // and pass said inbox into the VerifyBalanceAgreement call...
                // (So it can simulate as if the inbox was already changed, and
                // the total is already calculated, and if it succeeds,
                // then we can allow the giant loop below to do it all for
                // real.)
                // (I'm not saving this copy of the inbox anyway--there's
                // another one below.)
                //
                // theInbox.RemoveTransaction(pItem->GetReferenceToNum());
                // Let's remove it this way instead:
                //
                //                theInbox.RemoveTransaction(pServerTransaction->GetTransactionNum());
                // // <================
                //
                // Actually, let's remove it this way:
                theListOfInboxReceiptsBeingRemoved.push_back(
                    pServerTransaction->GetTransactionNum());

            }  // else if pServerTransaction NOT "".
        }      // If acceptCronReceipt/acceptFinalReceipt/acceptBasketReceipt
    }
    if (!bSuccessFindingAllTransactions)  // failure.
    {
        otOut << __FUNCTION__ << ": transactions in processInbox message do "
                                 "not match actual inbox.\n";

        return "";  // RETURN.
    }
    // SUCCESS finding all transactions

    while (!theListOfInboxReceiptsBeingRemoved.empty()) {
        int64_t lTemp = theListOfInboxReceiptsBeingRemoved.front();
        theListOfInboxReceiptsBeingRemoved.pop_front();

        if (false == theInbox.RemoveTransaction(lTemp))
            otErr << __FUNCTION__
                  << ": Failed removing receipt from temporary Inbox: " << lTemp
                  << " \n";
    }

    // SET UP NYM FOR BALANCE AGREEMENT.

    // By this point, theTempNym contains a list of all the transaction numbers
    // that are issued to me,
    // but that will NOT be issued to me anymore once this processInbox is
    // processed.
    // Therefore I need to REMOVE those items from my issued list (at least
    // temporarily) in order to
    // calculate the balance agreement properly. So I used theTempNym as a temp
    // variable to store those
    // numbers, so I can remove them from my Nym and them add them again after
    // generating the statement.
    //
    for (int32_t i = 0; i < theTempNym.GetIssuedNumCount(theNotaryID); i++) {
        int64_t lTemp = theTempNym.GetIssuedNum(theNotaryID, i);
        pNym->RemoveIssuedNum(strNotaryID, lTemp);
    }
    // BALANCE AGREEMENT
    //
    // The item is signed and saved within this call as well. No need to do that
    // again.
    //
    Item* pBalanceItem = theInbox.GenerateBalanceStatement(
        lTotalBeingAccepted, *pTransaction, *pNym, *pAccount, theOutbox);
    // Here I am adding these numbers back again, since I removed them to
    // generate the balance agreement.
    // (They won't be removed for real until I receive the server's
    // acknowledgment that those numbers
    // really were removed. theTempNym then I have to keep them and use them for
    // my balance agreements.)
    //
    for (int32_t i = 0; i < theTempNym.GetIssuedNumCount(theNotaryID); i++) {
        int64_t lTemp = theTempNym.GetIssuedNum(theNotaryID, i);
        pNym->AddIssuedNum(strNotaryID, lTemp);
    }

    if (nullptr == pBalanceItem) {
        otOut << __FUNCTION__ << ": ERROR generating balance statement.\n";

        return "";
    }

    // the transaction will handle cleaning up the transaction item.
    pTransaction->AddItem(*pBalanceItem);
    // sign the item
    // This already happens in the GenerateBalanceStatement() call above.
    // I would actually have to RELEASE the signatures if I wanted to sign
    // again!
    // (Unless I WANT two signatures...)
    //
    //    pBalanceItem->SignContract(*pNym);
    //    pBalanceItem->SaveContract();

    pTransaction->ReleaseSignatures();
    pTransaction->SignContract(*pNym);
    pTransaction->SaveContract();

    theLedger.ReleaseSignatures();
    theLedger.SignContract(*pNym);
    theLedger.SaveContract();

    String strOutput(theLedger);  // For the output

    std::string pBuf = strOutput.Get();

    return pBuf;
}

// Retrieve Voucher from Transaction
//
// If you withdrew into a voucher instead of cash, this function allows
// you to retrieve the actual voucher cheque from the reply transaction.
// (A voucher is a cheque drawn on an internal server account instead
// of a user's asset account, so the voucher cannot ever bounce due to
// insufficient funds. We are accustomed to this functionality already
// in our daily lives, via "money orders" and "cashier's cheques".)
//
// How would you use this in full?
//
// First, call OTAPI_Exec::withdrawVoucher() in order to send the request
// to the server. (You may optionally call OTAPI_Exec::FlushMessageBuffer()
// before doing this.)
//
// Then, call OTAPI_Exec::PopMessageBuffer() to retrieve any server reply.
//
// If there is a message from the server in reply, then call
// OTAPI_Exec::Message_GetCommand to verify that it's a reply to the message
// that you sent, and call OTAPI_Exec::Message_GetSuccess to verify whether
// the message was a success.
//
// If it was a success, next call OTAPI_Exec::Message_GetLedger to retrieve
// the actual "reply ledger" from the server.
//
// Penultimately, call OTAPI_Exec::Ledger_GetTransactionByID() and then,
// finally, call OTAPI_Exec::Transaction_GetVoucher() (below) in order to
// retrieve the voucher cheque itself from the transaction.
//
std::string OTAPI_Exec::Transaction_GetVoucher(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return "";
    }
    if (THE_TRANSACTION.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_TRANSACTION passed in!\n";
        return "";
    }

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theAccountID(ACCOUNT_ID);

    String strTransaction(THE_TRANSACTION);

    String strOutput;

    Nym* pNym = OTAPI()->GetOrLoadPrivateNym(
        theNymID, false, __FUNCTION__);  // These copiously log, and ASSERT.
    if (nullptr == pNym) return "";

    OTTransaction theTransaction(theNymID, theAccountID, theNotaryID);

    if (!theTransaction.LoadContractFromString(strTransaction)) {
        String strAcctID(theAccountID);
        otErr << __FUNCTION__
              << ": Error loading transaction from string. Acct ID: "
              << strAcctID << "\n";
        return "";
    }
    // No need to check if transaction is abbreviated, since it's coming from a
    // message ledger.
    // (Those always contain the full version of the transactions,
    // automatically.)

    if (OTTransaction::atWithdrawal != theTransaction.GetType()) {
        otErr << __FUNCTION__ << ": Error: tried to retrieve voucher from "
                                 "wrong transaction (not atWithdrawal).\n";
        return "";
    }

    // loop through the ALL items that make up this transaction and check to see
    // if a response to withdrawal.

    // if pointer not null, and it's a withdrawal, and it's an acknowledgement
    // (not a rejection or error)
    for (auto& it : theTransaction.GetItemList()) {
        Item* pItem = it;
        if (nullptr == pItem) {
            otErr << __FUNCTION__ << ": Pointer: "
                  << "pItem"
                  << " should not have been .\n";
            return "";
        }

        if ((Item::atWithdrawVoucher == pItem->GetType()) &&
            (Item::acknowledgement == pItem->GetStatus())) {
            String strVoucher;
            pItem->GetAttachment(strVoucher);

            Cheque theVoucher;
            if (theVoucher.LoadContractFromString(strVoucher))  // Todo
                                                                // additional
                                                                // verification
                                                                // here on the
                                                                // cheque.
            {
                theVoucher.SaveContractRaw(strOutput);
                break;
            }
        }
    }

    // Didn't find one.
    if (!strOutput.Exists()) return "";

    // We found a voucher -- let's return it!
    //
    std::string pBuf = strOutput.Get();

    return pBuf;
}

std::string OTAPI_Exec::Transaction_GetSenderNymID(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return "";
    }
    if (THE_TRANSACTION.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_TRANSACTION passed in!\n";
        return "";
    }

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theAccountID(ACCOUNT_ID);

    String strTransaction(THE_TRANSACTION);

    Nym* pNym = OTAPI()->GetOrLoadPrivateNym(
        theNymID, false, __FUNCTION__);  // These copiously log, and ASSERT.
    if (nullptr == pNym) return "";

    OTTransaction theTransaction(theNymID, theAccountID, theNotaryID);

    if (!theTransaction.LoadContractFromString(strTransaction)) {
        String strAcctID(theAccountID);
        otErr << __FUNCTION__
              << ": Error loading transaction from string. Acct ID: "
              << strAcctID << "\n";
        return "";
    }
    OTTransaction* pTransaction = nullptr;
    std::unique_ptr<OTTransaction> theTransAngel;

    if (theTransaction.IsAbbreviated()) {
        int64_t lBoxType = 0;

        if (theTransaction.Contains("nymboxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::nymbox);
        else if (theTransaction.Contains("inboxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::inbox);
        else if (theTransaction.Contains("outboxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::outbox);
        else if (theTransaction.Contains("paymentInboxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::paymentInbox);
        else if (theTransaction.Contains("recordBoxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::recordBox);
        else if (theTransaction.Contains("expiredBoxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::expiredBox);
        else {
            otErr << __FUNCTION__ << ": Error loading from abbreviated "
                                     "transaction: unknown ledger type.\n";
            return "";
        }
        pTransaction = LoadBoxReceipt(theTransaction, lBoxType);
        if (nullptr == pTransaction) {
            otErr << __FUNCTION__ << ": Error loading from abbreviated "
                                     "transaction: failed loading box "
                                     "receipt.\n";
            return "";
        }
        theTransAngel.reset(pTransaction);
    } else
        pTransaction = &theTransaction;
    Identifier theOutput;

    bool bSuccess = pTransaction->GetSenderNymIDForDisplay(theOutput);

    if (bSuccess) {
        String strOutput(theOutput);

        // Didn't find one.
        if (!strOutput.Exists()) return "";

        // We found it -- let's return the user ID
        //
        std::string pBuf = strOutput.Get();

        return pBuf;
    } else
        return "";
}

std::string OTAPI_Exec::Transaction_GetRecipientNymID(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return "";
    }
    if (THE_TRANSACTION.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_TRANSACTION passed in!\n";
        return "";
    }

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theAccountID(ACCOUNT_ID);

    String strTransaction(THE_TRANSACTION);

    Nym* pNym = OTAPI()->GetOrLoadPrivateNym(
        theNymID, false, __FUNCTION__);  // These copiously log, and ASSERT.
    if (nullptr == pNym) return "";
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    OTTransaction theTransaction(theNymID, theAccountID, theNotaryID);

    if (!theTransaction.LoadContractFromString(strTransaction)) {
        String strAcctID(theAccountID);
        otErr << __FUNCTION__
              << ": Error loading transaction from string. Acct ID: "
              << strAcctID << "\n";
        return "";
    }
    OTTransaction* pTransaction = nullptr;
    std::unique_ptr<OTTransaction> theTransAngel;

    if (theTransaction.IsAbbreviated()) {
        int64_t lBoxType = 0;

        if (theTransaction.Contains("nymboxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::nymbox);
        else if (theTransaction.Contains("inboxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::inbox);
        else if (theTransaction.Contains("outboxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::outbox);
        else if (theTransaction.Contains("paymentInboxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::paymentInbox);
        else if (theTransaction.Contains("recordBoxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::recordBox);
        else if (theTransaction.Contains("expiredBoxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::expiredBox);
        else {
            otErr << __FUNCTION__ << ": Error loading from abbreviated "
                                     "transaction: unknown ledger type. \n";
            return "";
        }
        pTransaction = LoadBoxReceipt(theTransaction, lBoxType);
        if (nullptr == pTransaction) {
            otErr << __FUNCTION__ << ": Error loading from abbreviated "
                                     "transaction: failed loading box receipt.";
            return "";
        }
        theTransAngel.reset(pTransaction);
    } else
        pTransaction = &theTransaction;

    Identifier theOutput;

    bool bSuccess = pTransaction->GetRecipientNymIDForDisplay(theOutput);

    // Normally, there IS NO recipient user ID for a transfer (only a recipient
    // acct ID.)
    // But here, as a special trick, I'll see if the account is in my address
    // book.
    // THIS MEANS THE ADDRESS BOOK needs to store nyms (for other people, their
    // public nym)
    // as well as a list of acct IDs that I have associated with each Nym. That
    // way, I can
    // later look up the Nym ID based on the acct ID, and then look up my
    // display label for
    // that Nym.
    //
    //    if (!bSuccess && (theTransaction.GetType() == OTTransaction::pending))
    //    {
    //        // AS SOON AS ADDRESS BOOK FEATURE IS ADDED, THEN THIS CAN BE
    // COMPLETED HERE.
    //    }

    if (bSuccess) {
        String strOutput(theOutput);

        // Didn't find one.
        if (!strOutput.Exists()) return "";

        // We found it -- let's return the user ID
        //
        std::string pBuf = strOutput.Get();

        return pBuf;
    } else
        return "";
}

std::string OTAPI_Exec::Transaction_GetSenderAcctID(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return "";
    }
    if (THE_TRANSACTION.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_TRANSACTION passed in!\n";
        return "";
    }

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theAccountID(ACCOUNT_ID);

    String strTransaction(THE_TRANSACTION);

    Nym* pNym = OTAPI()->GetOrLoadPrivateNym(
        theNymID, false, __FUNCTION__);  // These copiously log, and ASSERT.
    if (nullptr == pNym) return "";

    OTTransaction theTransaction(theNymID, theAccountID, theNotaryID);

    if (!theTransaction.LoadContractFromString(strTransaction)) {
        String strAcctID(theAccountID);
        otErr << __FUNCTION__
              << ": Error loading transaction from string. Acct ID: "
              << strAcctID << "\n";
        return "";
    }
    OTTransaction* pTransaction = nullptr;
    std::unique_ptr<OTTransaction> theTransAngel;

    if (theTransaction.IsAbbreviated()) {
        int64_t lBoxType = 0;

        if (theTransaction.Contains("nymboxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::nymbox);
        else if (theTransaction.Contains("inboxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::inbox);
        else if (theTransaction.Contains("outboxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::outbox);
        else if (theTransaction.Contains("paymentInboxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::paymentInbox);
        else if (theTransaction.Contains("recordBoxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::recordBox);
        else if (theTransaction.Contains("expiredBoxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::expiredBox);
        else {
            otErr << __FUNCTION__ << ": Error loading from abbreviated "
                                     "transaction: unknown ledger type.\n";
            return "";
        }
        pTransaction = LoadBoxReceipt(theTransaction, lBoxType);
        if (nullptr == pTransaction) {
            otErr << __FUNCTION__ << ": Error loading from abbreviated "
                                     "transaction: failed loading box receipt. "
                                     "\n";
            return "";
        }
        theTransAngel.reset(pTransaction);
    } else
        pTransaction = &theTransaction;

    Identifier theOutput;

    bool bSuccess = pTransaction->GetSenderAcctIDForDisplay(theOutput);

    if (bSuccess) {
        String strOutput(theOutput);

        // Didn't find one.
        if (!strOutput.Exists()) return "";

        // We found it -- let's return the user ID
        //
        std::string pBuf = strOutput.Get();

        return pBuf;
    } else
        return "";
}

std::string OTAPI_Exec::Transaction_GetRecipientAcctID(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return "";
    }
    if (THE_TRANSACTION.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_TRANSACTION passed in!\n";
        return "";
    }

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theAccountID(ACCOUNT_ID);

    String strTransaction(THE_TRANSACTION);

    Nym* pNym = OTAPI()->GetOrLoadPrivateNym(
        theNymID, false, __FUNCTION__);  // These copiously log, and ASSERT.
    if (nullptr == pNym) return "";

    OTTransaction theTransaction(theNymID, theAccountID, theNotaryID);

    if (!theTransaction.LoadContractFromString(strTransaction)) {
        String strAcctID(theAccountID);
        otErr << __FUNCTION__ << ": Error loading transaction from string in "
                                 "OTAPI_Exec::Transaction_GetRecipientAcctID. "
                                 "Acct ID: "
              << strAcctID << "\n";
        return "";
    }

    OTTransaction* pTransaction = nullptr;
    std::unique_ptr<OTTransaction> theTransAngel;

    if (theTransaction.IsAbbreviated()) {
        int64_t lBoxType = 0;

        if (theTransaction.Contains("nymboxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::nymbox);
        else if (theTransaction.Contains("inboxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::inbox);
        else if (theTransaction.Contains("outboxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::outbox);
        else if (theTransaction.Contains("paymentInboxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::paymentInbox);
        else if (theTransaction.Contains("recordBoxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::recordBox);
        else if (theTransaction.Contains("expiredBoxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::expiredBox);
        else {
            otErr << __FUNCTION__ << ": Error loading from abbreviated "
                                     "transaction: unknown ledger type. \n";
            return "";
        }
        pTransaction = LoadBoxReceipt(theTransaction, lBoxType);
        if (nullptr == pTransaction) {
            otErr << __FUNCTION__ << ": Error loading from abbreviated "
                                     "transaction: failed loading box "
                                     "receipt.\n";
            return "";
        }
        theTransAngel.reset(pTransaction);
    } else
        pTransaction = &theTransaction;
    Identifier theOutput;

    bool bSuccess = pTransaction->GetRecipientAcctIDForDisplay(theOutput);

    if (bSuccess) {
        String strOutput(theOutput);

        // Didn't find one.
        if (!strOutput.Exists()) return "";

        // We found it -- let's return the user ID
        //
        std::string pBuf = strOutput.Get();

        return pBuf;
    } else
        return "";
}

//
// PENDING TRANSFER (various functions)
//
// When someone has sent you a PENDING TRANSFER (a form of transaction
// that will be sitting in your inbox waiting for you to accept/reject it)
// then, as you are reading the inbox, you can use these functions in
// order to get data from the pending transfer.
//

std::string OTAPI_Exec::Pending_GetNote(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return "";
    }
    if (THE_TRANSACTION.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_TRANSACTION passed in!\n";
        return "";
    }

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theAccountID(ACCOUNT_ID);

    String strTransaction(THE_TRANSACTION);
    Nym* pNym = OTAPI()->GetOrLoadPrivateNym(
        theNymID, false, __FUNCTION__);  // These copiously log, and ASSERT.
    if (nullptr == pNym) return "";
    OTTransaction theTransaction(theNymID, theAccountID, theNotaryID);

    if (!theTransaction.LoadContractFromString(strTransaction)) {
        String strAcctID(theAccountID);
        otErr << __FUNCTION__
              << ": Error loading transaction from string. Acct ID: "
              << strAcctID << "\n";
        return "";
    }
    OTTransaction* pTransaction = nullptr;
    std::unique_ptr<OTTransaction> theTransAngel;

    if (theTransaction.IsAbbreviated()) {
        int64_t lBoxType = 0;

        if (theTransaction.Contains("nymboxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::nymbox);
        else if (theTransaction.Contains("inboxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::inbox);
        else if (theTransaction.Contains("outboxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::outbox);
        else if (theTransaction.Contains("paymentInboxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::paymentInbox);
        else if (theTransaction.Contains("recordBoxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::recordBox);
        else if (theTransaction.Contains("expiredBoxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::expiredBox);
        else {
            otErr << __FUNCTION__ << " Error loading from abbreviated "
                                     "transaction: unknown ledger type. \n";
            return "";
        }
        pTransaction = LoadBoxReceipt(theTransaction, lBoxType);
        if (nullptr == pTransaction) {
            otErr << __FUNCTION__ << ": Error loading from abbreviated "
                                     "transaction: failed loading box receipt. "
                                     "\n";
            return "";
        }
        theTransAngel.reset(pTransaction);
    } else
        pTransaction = &theTransaction;
    if (OTTransaction::pending != pTransaction->GetType()) {
        otErr << __FUNCTION__
              << ": wrong transaction type: " << pTransaction->GetTypeString()
              << ". (Expected \"pending\".)\n";
        return "";
    }
    String strReference;
    pTransaction->GetReferenceString(strReference);

    if (!strReference.Exists()) {
        otErr << __FUNCTION__
              << ": No reference string found on transaction.\n";
        return "";
    }
    std::unique_ptr<Item> pItem(
        Item::CreateItemFromString(
            strReference, theNotaryID, pTransaction->GetReferenceToNum()));

    if (nullptr == pItem) {
        otErr << __FUNCTION__
              << ": Failed loading transaction item from string.\n";
        return "";
    }

    // pItem will be automatically cleaned up when it goes out of scope.
    if ((Item::transfer != pItem->GetType()) ||
        (Item::request != pItem->GetStatus())) {
        otErr << __FUNCTION__ << ": Wrong item type or status attached as "
                                 "reference on transaction.\n";
        return "";
    }
    String strOutput;

    pItem->GetNote(strOutput);
    // Didn't find one.
    if (!strOutput.Exists()) return "";

    // We found a note -- let's return it!
    //
    std::string pBuf = strOutput.Get();

    return pBuf;
}

int64_t OTAPI_Exec::Transaction_GetAmount(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return -1;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return -1;
    }
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return -1;
    }
    if (THE_TRANSACTION.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_TRANSACTION passed in!\n";
        return -1;
    }

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theAccountID(ACCOUNT_ID);

    String strTransaction(THE_TRANSACTION);

    Nym* pNym = OTAPI()->GetOrLoadPrivateNym(
        theNymID, false, __FUNCTION__);  // These copiously log, and ASSERT.
    if (nullptr == pNym) return OT_ERROR_AMOUNT;

    OTTransaction theTransaction(theNymID, theAccountID, theNotaryID);

    if (!theTransaction.LoadContractFromString(strTransaction)) {
        String strAcctID(theAccountID);
        otErr << __FUNCTION__
              << ": Error loading transaction from string. Acct ID: "
              << strAcctID << "\n";
        return -1;
    }

    OTTransaction* pTransaction = nullptr;
    std::unique_ptr<OTTransaction> theTransAngel;

    if (theTransaction.IsAbbreviated()) {
        int64_t lBoxType = 0;

        if (theTransaction.Contains("nymboxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::nymbox);
        else if (theTransaction.Contains("inboxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::inbox);
        else if (theTransaction.Contains("outboxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::outbox);
        else if (theTransaction.Contains("paymentInboxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::paymentInbox);
        else if (theTransaction.Contains("recordBoxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::recordBox);
        else if (theTransaction.Contains("expiredBoxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::expiredBox);
        else {
            otErr << __FUNCTION__ << ": Error loading from abbreviated "
                                     "transaction: unknown ledger type. \n";
            return -1;
        }
        pTransaction = LoadBoxReceipt(theTransaction, lBoxType);
        if (nullptr == pTransaction) {
            otErr << __FUNCTION__ << ": Error loading from abbreviated "
                                     "transaction: failed loading box receipt. "
                                     "\n";
            return -1;
        }
        theTransAngel.reset(pTransaction);
    } else
        pTransaction = &theTransaction;

    return pTransaction->GetReceiptAmount();
}

// There is a notice in my inbox, from the server, informing me of
// a pending transfer.
// Oh? And this notice is in reference to what transaction ID?
// This function will return the ID of the original transaction
// that the sender used to send me the transfer in the first place.
// Since his actual request is attached to the pending transaction,
// I can use this function to look up the number.
//
// Returns cheque #, or market offer #, or payment plan #, or pending transfer #
// (Meant to be used on inbox items.)
//
int64_t OTAPI_Exec::Transaction_GetDisplayReferenceToNum(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return -1;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return -1;
    }
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return -1;
    }
    if (THE_TRANSACTION.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_TRANSACTION passed in!\n";
        return -1;
    }

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theAccountID(ACCOUNT_ID);

    String strTransaction(THE_TRANSACTION);

    Nym* pNym = OTAPI()->GetOrLoadPrivateNym(
        theNymID, false, __FUNCTION__);  // These copiously log, and ASSERT.
    if (nullptr == pNym) return -1;

    OTTransaction theTransaction(theNymID, theAccountID, theNotaryID);

    if (!theTransaction.LoadContractFromString(strTransaction)) {
        String strAcctID(theAccountID);
        otErr << __FUNCTION__
              << ": Error loading transaction from string. Acct ID: "
              << strAcctID << "\n";
        return -1;
    }

    // NO need to load abbreviated version here, since it already stores this
    // number.
    //
    const int64_t lDisplayNum = theTransaction.GetReferenceNumForDisplay();
    return lDisplayNum;
}

//
// Get Transaction Type  (internally uses GetTransactionTypeString().)
//
std::string OTAPI_Exec::Transaction_GetType(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return "";
    }
    if (THE_TRANSACTION.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_TRANSACTION passed in!\n";
        return "";
    }

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theAccountID(ACCOUNT_ID);

    String strTransaction(THE_TRANSACTION);

    Nym* pNym = OTAPI()->GetOrLoadPrivateNym(
        theNymID, false, __FUNCTION__);  // These copiously log, and ASSERT.
    if (nullptr == pNym) return "";
    OTTransaction theTransaction(theNymID, theAccountID, theNotaryID);

    if (!theTransaction.LoadContractFromString(strTransaction)) {
        String strAcctID(theAccountID);
        otErr << __FUNCTION__
              << ": Error loading transaction from string. Acct ID: "
              << strAcctID << "\n";
        return "";
    }
    // NO need to load abbreviated version, since it already stores this number.

    std::string pBuf = theTransaction.GetTypeString();

    return pBuf;
}

// Transactions do not have request numbers. However, if you have a replyNotice
// in your Nymbox, which is an OTTransaction object, it will CONTAIN a server
// reply to some previous message. This function will only work on a
// replyNotice,
// and it returns the actual request number of the server reply inside that
// notice.
// Used for calling OTAPI_Exec::HaveAlreadySeenReply() in order to see if we've
// already
// processed the reply for that message.
// Returns -1 on Error.
//
int64_t OTAPI_Exec::ReplyNotice_GetRequestNum(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& THE_TRANSACTION) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return -1;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return -1;
    }
    if (THE_TRANSACTION.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_TRANSACTION passed in!\n";
        return -1;
    }

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theAccountID(NYM_ID);  // account IS user, for Nymbox (the only box that
                               // carries replyNotices...)

    String strTransaction(THE_TRANSACTION);

    Nym* pNym = OTAPI()->GetOrLoadPrivateNym(
        theNymID, false, __FUNCTION__);  // These copiously log, and ASSERT.
    if (nullptr == pNym) return -1;

    OTTransaction theTransaction(theNymID, theAccountID, theNotaryID);

    if (!theTransaction.LoadContractFromString(strTransaction)) {
        String strNymID(theNymID);
        otErr << __FUNCTION__
              << ": Error loading transaction from string. Nym ID: " << strNymID
              << "\n";
        return -1;
    }

    if (OTTransaction::replyNotice != theTransaction.GetType()) {
        String strNymID(theNymID);
        otErr << __FUNCTION__ << ": Unexpected transaction type: "
              << theTransaction.GetTypeString()
              << ". (Expected: replyNotice) User: " << strNymID << "\n";
        return -1;
    }
    // NO need to load abbreviated version, since it already stores this number.

    const int64_t lRequestNumber = theTransaction.GetRequestNum();

    return lRequestNumber;
}

//
// Get Transaction Date Signed  (internally uses
// OTTransaction::GetDateSigned().)
//
time64_t OTAPI_Exec::Transaction_GetDateSigned(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OTTimeGetTimeFromSeconds(-1);
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OTTimeGetTimeFromSeconds(-1);
    }
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return OTTimeGetTimeFromSeconds(-1);
    }
    if (THE_TRANSACTION.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_TRANSACTION passed in!\n";
        return OTTimeGetTimeFromSeconds(-1);
    }

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theAccountID(ACCOUNT_ID);

    String strTransaction(THE_TRANSACTION);

    Nym* pNym = OTAPI()->GetOrLoadPrivateNym(
        theNymID, false, __FUNCTION__);  // These copiously log, and ASSERT.
    if (nullptr == pNym) return OTTimeGetTimeFromSeconds(-1);

    OTTransaction theTransaction(theNymID, theAccountID, theNotaryID);

    if (!theTransaction.LoadContractFromString(strTransaction)) {
        String strAcctID(theAccountID);
        otErr << __FUNCTION__
              << ": Error loading transaction from string. Acct ID: "
              << strAcctID << "\n";
        return OTTimeGetTimeFromSeconds(-1);
    }
    // NO need to load abbreviated version here, since it already stores the
    // date.

    String strOutput;
    return theTransaction.GetDateSigned();
}

//
// Get TransactionSuccess
//
// OT_TRUE  (1) == acknowledgment
// OT_FALSE (0) == rejection
// OT_ERROR(-1) == error_state (such as dropped message.)
//
// Returns OT_BOOL.
//
int32_t OTAPI_Exec::Transaction_GetSuccess(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return OT_ERROR;
    }
    if (THE_TRANSACTION.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_TRANSACTION passed in!\n";
        return OT_ERROR;
    }

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theAccountID(ACCOUNT_ID);

    String strTransaction(THE_TRANSACTION);
    Nym* pNym = OTAPI()->GetOrLoadPrivateNym(
        theNymID, false, __FUNCTION__);  // These copiously log, and ASSERT.
    if (nullptr == pNym) return OT_ERROR;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    OTTransaction theTransaction(theNymID, theAccountID, theNotaryID);

    if (!theTransaction.LoadContractFromString(strTransaction)) {
        String strAcctID(theAccountID);
        otErr << __FUNCTION__
              << ": Error loading transaction from string. Acct ID: "
              << strAcctID << "\n";
        return OT_ERROR;
    }
    OTTransaction* pTransaction = nullptr;
    std::unique_ptr<OTTransaction> theTransAngel;

    if (theTransaction.IsAbbreviated())  // Abbreviated.
    {
        int64_t lBoxType = 0;

        if (theTransaction.Contains("nymboxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::nymbox);
        else if (theTransaction.Contains("inboxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::inbox);
        else if (theTransaction.Contains("outboxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::outbox);
        else if (theTransaction.Contains("paymentInboxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::paymentInbox);
        else if (theTransaction.Contains("recordBoxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::recordBox);
        else if (theTransaction.Contains("expiredBoxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::expiredBox);
        else {
            otErr << __FUNCTION__ << ": Error loading from abbreviated "
                                     "transaction: unknown ledger type. \n";
            return OT_ERROR;
        }
        pTransaction = LoadBoxReceipt(theTransaction, lBoxType);
        if (nullptr == pTransaction) {
            otErr << __FUNCTION__ << ": Error loading from abbreviated "
                                     "transaction: failed loading box receipt. "
                                     "\n";
            return OT_ERROR;
        }
        theTransAngel.reset(pTransaction);
    } else  // NOT abbreviated.
        pTransaction = &theTransaction;

    if (pTransaction->GetSuccess()) {
        return OT_TRUE;
    } else {
        const int64_t lTransactionNum = pTransaction->GetTransactionNum();

        otErr << __FUNCTION__
              << ": ** FYI, this transaction has a 'failure' status from the "
                 "server. TransNum: "
              << lTransactionNum << "\n";
    }

    return OT_FALSE;
}

int32_t OTAPI_Exec::Transaction_IsCanceled(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return OT_ERROR;
    }
    if (THE_TRANSACTION.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_TRANSACTION passed in!\n";
        return OT_ERROR;
    }

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theAccountID(ACCOUNT_ID);

    String strTransaction(THE_TRANSACTION);
    Nym* pNym = OTAPI()->GetOrLoadPrivateNym(
        theNymID, false, __FUNCTION__);  // These copiously log, and ASSERT.
    if (nullptr == pNym) return OT_ERROR;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    OTTransaction theTransaction(theNymID, theAccountID, theNotaryID);

    if (!theTransaction.LoadContractFromString(strTransaction)) {
        String strAcctID(theAccountID);
        otErr << __FUNCTION__
              << ": Error loading transaction from string. Acct ID: "
              << strAcctID << "\n";
        return OT_ERROR;
    }
    OTTransaction* pTransaction = nullptr;
    std::unique_ptr<OTTransaction> theTransAngel;

    // Note: This is an artifact from Transaction_GetSuccess, whose code was
    // copied to make
    // this function. In reality, a cancelled transaction will never be
    // abbreviated, since it
    // won't be used as any kind of box receipt in the first place. Rather, a
    // canceled transaction
    // will only occur as a server reply to a transaction request. For example,
    // if you cancel a
    // cheque, or cancel a payment plan, and that cancellation is successful,
    // then the server
    // will return a "reply transaction" to that request, which has 'cancelled'
    // set to true.
    // So why am I leaving this code here for now? We can trim it down later I
    // suppose.
    //
    if (theTransaction.IsAbbreviated())  // Abbreviated.
    {
        int64_t lBoxType = 0;

        if (theTransaction.Contains("nymboxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::nymbox);
        else if (theTransaction.Contains("inboxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::inbox);
        else if (theTransaction.Contains("outboxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::outbox);
        else if (theTransaction.Contains("paymentInboxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::paymentInbox);
        else if (theTransaction.Contains("recordBoxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::recordBox);
        else if (theTransaction.Contains("expiredBoxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::expiredBox);
        else {
            otErr << __FUNCTION__ << ": Error loading from abbreviated "
                                     "transaction: unknown ledger type. \n";
            return OT_ERROR;
        }
        pTransaction = LoadBoxReceipt(theTransaction, lBoxType);
        if (nullptr == pTransaction) {
            otErr << __FUNCTION__ << ": Error loading from abbreviated "
                                     "transaction: failed loading box receipt. "
                                     "\n";
            return OT_ERROR;
        }
        theTransAngel.reset(pTransaction);
    } else  // NOT abbreviated.
        pTransaction = &theTransaction;
    if (pTransaction->IsCancelled()) {
        return OT_TRUE;
    }

    return OT_FALSE;
}

//
// Get Balance Agreement Success
// (from a TRANSACTION.)
//                              true  (1) == acknowledgment
//                              false (0) == rejection
// NEW: -1 (-1) for error
//
int32_t OTAPI_Exec::Transaction_GetBalanceAgreementSuccess(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return OT_ERROR;
    }
    if (THE_TRANSACTION.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_TRANSACTION passed in!\n";
        return OT_ERROR;
    }

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theAccountID(ACCOUNT_ID);

    String strTransaction(THE_TRANSACTION);

    Nym* pNym = OTAPI()->GetOrLoadPrivateNym(
        theNymID, false, __FUNCTION__);  // These copiously log, and ASSERT.
    if (nullptr == pNym) return OT_ERROR;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    OTTransaction theTransaction(theNymID, theAccountID, theNotaryID);

    if (!theTransaction.LoadContractFromString(strTransaction)) {
        String strAcctID(theAccountID);
        otErr << __FUNCTION__
              << ": Error loading transaction from string. Acct ID: "
              << strAcctID << "\n";
        return OT_ERROR;
    }
    OTTransaction* pTransaction = nullptr;
    std::unique_ptr<OTTransaction> theTransAngel;

    if (theTransaction.IsAbbreviated())  // IF TRANSACTION IS ABBREVIATED
                                         // (Ledger
                                         // may only contain stubs, not full
                                         // records...)
    {
        int64_t lBoxType = 0;

        if (theTransaction.Contains("nymboxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::nymbox);
        else if (theTransaction.Contains("inboxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::inbox);
        else if (theTransaction.Contains("outboxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::outbox);
        else if (theTransaction.Contains("paymentInboxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::paymentInbox);
        else if (theTransaction.Contains("recordBoxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::recordBox);
        else if (theTransaction.Contains("expiredBoxRecord"))
            lBoxType = static_cast<int64_t>(Ledger::expiredBox);
        else {
            otErr << __FUNCTION__ << ": Error loading from abbreviated "
                                     "transaction: unknown ledger type. \n";
            return OT_ERROR;
        }
        pTransaction = LoadBoxReceipt(theTransaction, lBoxType);
        if (nullptr == pTransaction) {
            otErr << __FUNCTION__ << ": Error loading from abbreviated "
                                     "transaction: failed loading box "
                                     "receipt.\n";
            return OT_ERROR;
        }
        theTransAngel.reset(pTransaction);
    } else
        pTransaction = &theTransaction;
    // At this point, I actually have the transaction pointer, so let's return
    // its success status
    Item* pReplyItem = pTransaction->GetItem(Item::atBalanceStatement);

    if (nullptr == pReplyItem)
        pReplyItem = pTransaction->GetItem(Item::atTransactionStatement);

    if (nullptr == pReplyItem) {
        otErr << __FUNCTION__ << ": good transaction (could have been "
                                 "abbreviated though) but uncovered \"\" item "
                                 "pointer.\n";
        return OT_ERROR;  // Weird.
    }

    return (pReplyItem->GetStatus() == Item::acknowledgement) ? OT_TRUE
                                                              : OT_FALSE;
}

// GET BALANCE AGREEMENT SUCCESS (From a MESSAGE.)
//
// Returns true (1) for Success and false (0) for Failure.
// New: returns -1 (-1) for error.
//
int32_t OTAPI_Exec::Message_GetBalanceAgreementSuccess(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_MESSAGE) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return OT_ERROR;
    }
    if (THE_MESSAGE.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_MESSAGE passed in!\n";
        return OT_ERROR;
    }

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theAccountID(ACCOUNT_ID);

    String strMessage(THE_MESSAGE);

    Message theMessage;

    if (!strMessage.Exists() ||
        !theMessage.LoadContractFromString(strMessage)) {
        otOut << __FUNCTION__ << ": Unable to load message.\n";
        return OT_ERROR;
    }

    // It's not a transaction request or response, so the Payload wouldn't
    // contain a ledger. (Don't want to pass back whatever it DOES contain
    // in that case, now do I?)
    //
    if ((false ==
         theMessage.m_strCommand.Compare("notarizeTransactionResponse")) &&
        (false == theMessage.m_strCommand.Compare("processNymboxResponse")) &&
        (false == theMessage.m_strCommand.Compare("processInboxResponse"))) {
        otOut << __FUNCTION__
              << ": Wrong message type: " << theMessage.m_strCommand << "\n";
        return OT_ERROR;
    }

    // The ledger is stored in the Payload, we'll grab it into the String.
    String strLedger(theMessage.m_ascPayload);

    if (!strLedger.Exists()) {
        otOut << __FUNCTION__ << ": No ledger found on message.\n";
        return OT_ERROR;
    }

    Ledger theLedger(theNymID, theAccountID, theNotaryID);

    if (!theLedger.LoadLedgerFromString(strLedger)) {
        String strAcctID(theAccountID);
        otErr << __FUNCTION__
              << ": Error loading ledger from string. Acct ID: " << strAcctID
              << "\n";
        return OT_ERROR;
    }

    // At this point, I know theLedger loaded successfully.

    if (theLedger.GetTransactionCount() <= 0) {
        otErr << __FUNCTION__ << " bad count in message ledger: "
              << theLedger.GetTransactionCount() << "\n";
        return OT_ERROR;  // out of bounds. I'm saving from an OT_ASSERT_MSG()
                          // happening here. (Maybe I shouldn't.)
    }

    OTTransaction* pReplyTransaction = theLedger.GetTransactionByIndex(
        0);  // Right now this is a defacto standard. (only 1 transaction per
             // message ledger, excepting process inbox. <== why? That's one as
             // well I thought. And has multiple items attached.)

    if (nullptr == pReplyTransaction) {
        otErr << __FUNCTION__
              << " good index but uncovered \"\" pointer there: " << 0 << "\n";
        return OT_ERROR;  // Weird.
    }

    // At this point, I actually have the transaction pointer, so let's return
    // its success status
    Item* pReplyItem = pReplyTransaction->GetItem(Item::atBalanceStatement);

    if (nullptr == pReplyItem)
        pReplyItem = pReplyTransaction->GetItem(Item::atTransactionStatement);

    if (nullptr == pReplyItem) {
        otErr << __FUNCTION__
              << " good index but uncovered \"\" item pointer: " << 0 << "\n";
        return OT_ERROR;  // Weird.
    }

    if (pReplyItem->GetStatus() == Item::acknowledgement) {
        return OT_TRUE;
    }

    return OT_FALSE;
}

/*
std::string OTAPI_Exec::LoadPurse(const std::string& NOTARY_ID,
                                  std::string INSTRUMENT_DEFINITION_ID,
                                  std::string NYM_ID) // returns "", or a
purse.
{
    OT_ASSERT_MSG("" != NOTARY_ID, "Null NOTARY_ID passed in.");
    OT_ASSERT_MSG("" != INSTRUMENT_DEFINITION_ID, "Null INSTRUMENT_DEFINITION_ID
passed in.");
    OT_ASSERT_MSG("" != NYM_ID, "Null NYM_ID passed in.");

    const OTIdentifier theNotaryID(NOTARY_ID);
    const OTIdentifier theInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID);
    const OTIdentifier theNymID(NYM_ID);

    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.
    OTPurse * pPurse = OTAPI()->LoadPurse(theNotaryID,
theInstrumentDefinitionID, theNymID);

    // Make sure it gets cleaned up when this goes out of scope.
    OTCleanup<OTPurse>    thePurseAngel(pPurse); // I pass the pointer, in case
it's "".

    if (nullptr == pPurse)
    {
        otOut << "Failure calling OT_API::LoadPurse in
OTAPI_Exec::LoadPurse.\n Server: " << NOTARY_ID << "\n Asset Type: " <<
INSTRUMENT_DEFINITION_ID << "\n";
    }
    else // success
    {
        OTString strOutput(*pPurse); // For the output

        std::string pBuf = strOutput.Get();



        return pBuf;
    }

    return "";
}
*/

// PURSE FUNCTIONS

// Warning! This will overwrite whatever purse is there.
// The proper way to use this function is, LOAD the purse,
// then IMPORT whatever other purse you want into it, then
// SAVE it again.
bool OTAPI_Exec::SavePurse(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& NYM_ID,
    const std::string& THE_PURSE) const  // returns bool
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return false;
    }
    if (INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Null: INSTRUMENT_DEFINITION_ID passed in!\n";
        return false;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return false;
    }
    if (THE_PURSE.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_PURSE passed in!\n";
        return false;
    }

    std::string strFunc = __FUNCTION__;
    const Identifier theNotaryID(NOTARY_ID),
        theInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID), theNymID(NYM_ID);
    const String strPurse(THE_PURSE);
    bool bSuccess = false;
    Purse thePurse(theNotaryID, theInstrumentDefinitionID, theNymID);

    if (strPurse.Exists() && thePurse.LoadContractFromString(strPurse)) {
        // NOTE: no need to verify the server / instrument definition id here,
        // since the call
        // to SavePurse already does that.
        //
        //      if ((theNotaryID    != thePurse.GetNotaryID()) ||
        //          (theInstrumentDefinitionID !=
        //          thePurse.GetInstrumentDefinitionID()))
        //      {
        //          otOut << strFunc << ": Failure loading purse from string;
        // server "
        //                         "or instrument definition id didn't match
        //                         what was
        // expected:\n" << strPurse << "\n";
        //      }
        if (OTAPI()->SavePurse(
                theNotaryID, theInstrumentDefinitionID, theNymID, thePurse)) {
            bSuccess = true;
        } else {
            otOut << strFunc << ": Failure saving purse:\n" << strPurse << "\n";
        }
    } else {
        otOut << strFunc << ": Failure loading purse from string:\n"
              << strPurse << "\n";
    }

    return bSuccess;
}

// LOAD PURSE -- (from local storage)
//
// Based on Instrument Definition ID: load a purse, a public mint, or an asset
// contract
// and return it as a string -- or return "" if it wasn't found.
//
std::string OTAPI_Exec::LoadPurse(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& NYM_ID) const  // returns
                                      // "", or
                                      // a purse.
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Null: INSTRUMENT_DEFINITION_ID passed in!\n";
        return "";
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    const Identifier theNotaryID(NOTARY_ID);
    const Identifier theInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID);
    const Identifier theNymID(NYM_ID);
    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.

    std::unique_ptr<Purse> pPurse(
        OTAPI()->LoadPurse(theNotaryID, theInstrumentDefinitionID, theNymID));

    if (nullptr == pPurse) {
        otInfo << "OTAPI_Exec::LoadPurse() received null when called "
                  "OT_API::LoadPurse(). Server: "
               << NOTARY_ID << " Asset Type: " << INSTRUMENT_DEFINITION_ID
               << "\n";
    } else  // success
    {
        String strOutput(*pPurse);  // For the output
        std::string pBuf = strOutput.Get();
        return pBuf;
    }

    return "";
}

// Get Purse Total Value  (internally uses GetTotalValue().)
//
// Returns the purported sum of all the tokens within.
//
int64_t OTAPI_Exec::Purse_GetTotalValue(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& THE_PURSE) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR_AMOUNT;
    }
    if (INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Null: INSTRUMENT_DEFINITION_ID passed in!\n";
        return OT_ERROR_AMOUNT;
    }
    if (THE_PURSE.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_PURSE passed in!\n";
        return OT_ERROR_AMOUNT;
    }
    const Identifier theNotaryID(NOTARY_ID),
        theInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID);
    String strPurse(THE_PURSE);
    Purse thePurse(theNotaryID, theInstrumentDefinitionID);

    if (!thePurse.LoadContractFromString(strPurse)) {
        String strInstrumentDefinitionID(theInstrumentDefinitionID);
        otErr << __FUNCTION__
              << ": Error loading purse from string. Instrument Definition ID: "
              << strInstrumentDefinitionID << "\n";
        return OT_ERROR_AMOUNT;
    }

    int64_t lTotalValue = thePurse.GetTotalValue();
    return lTotalValue > 0 ? lTotalValue : 0;
}

// Returns a count of the tokens inside this purse. (Coins.)
// or -1 in case of error.
//
int32_t OTAPI_Exec::Purse_Count(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& THE_PURSE) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Null: INSTRUMENT_DEFINITION_ID passed in!\n";
        return OT_ERROR;
    }
    if (THE_PURSE.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_PURSE passed in!\n";
        return OT_ERROR;
    }

    const Identifier theNotaryID(NOTARY_ID),
        theInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID);
    const String strPurse(THE_PURSE);
    Purse thePurse(theNotaryID, theInstrumentDefinitionID);

    if (strPurse.Exists() && thePurse.LoadContractFromString(strPurse) &&
        (thePurse.GetNotaryID() == theNotaryID) &&
        (thePurse.GetInstrumentDefinitionID() == theInstrumentDefinitionID)) {
        return thePurse.Count();
    }

    return OT_ERROR;
}

// Some purses are encrypted to a specific Nym.
// Whereas other purses are encrypted to a passphrase.
// This function returns bool and lets you know, either way.
//
bool OTAPI_Exec::Purse_HasPassword(
    const std::string& NOTARY_ID,
    const std::string& THE_PURSE) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return false;
    }
    if (THE_PURSE.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_PURSE passed in!\n";
        return false;
    }

    const Identifier theNotaryID(NOTARY_ID);
    const String strPurse(THE_PURSE);
    Purse thePurse(theNotaryID);

    if (strPurse.Exists() && thePurse.LoadContractFromString(strPurse) &&
        (thePurse.GetNotaryID() == theNotaryID)) {
        return thePurse.IsPasswordProtected();
    }

    return false;
}

// returns "", or a purse.
//
std::string OTAPI_Exec::CreatePurse(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& OWNER_ID,
    const std::string& SIGNER_ID) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Null: INSTRUMENT_DEFINITION_ID passed in!\n";
        return "";
    }
    if (OWNER_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: OWNER_ID passed in!\n";
        return "";
    }
    if (SIGNER_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SIGNER_ID passed in!\n";
        return "";
    }

    std::string strFunc = __FUNCTION__;

    const Identifier theNotaryID(NOTARY_ID),
        theInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID),
        theOwnerID(OWNER_ID), theSignerID(SIGNER_ID);
    OTPasswordData thePWData(
        "Creating a cash purse. Enter wallet master password.");
    const Nym* pOwnerNym =
        OTAPI()->GetOrLoadNym(theOwnerID, false, strFunc.c_str(), &thePWData);
    if (nullptr == pOwnerNym) return "";
    Nym* pSignerNym = OTAPI()->GetOrLoadPrivateNym(
        theSignerID,
        false,
        strFunc.c_str(),
        &thePWData);  // These copiously log, and ASSERT.
    if (nullptr == pSignerNym) return "";
    // By this point, pSignerNym is a good pointer, and is on the wallet. (No
    // need to cleanup.)
    std::unique_ptr<Purse> pPurse(
        OTAPI()->CreatePurse(
            theNotaryID, theInstrumentDefinitionID, theOwnerID));

    if (nullptr != pPurse) {
        pPurse->SignContract(*pSignerNym, &thePWData);
        pPurse->SaveContract();
        const String strOutput(*pPurse);
        std::string pBuf = strOutput.Get();

        return pBuf;
    }
    return "";
}

// When we create a password-protected purse, OWNER_ID has changed to SIGNER_ID,
// since we still need a Nym to sign it (in order to save it to string.) But
// this
// Nym will not be the owner. There is no owner, and the tokens are not
// encrypted
// to any Nym (but instead to a passphrase.)
//
// returns "", or a purse.
//
std::string OTAPI_Exec::CreatePurse_Passphrase(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& SIGNER_ID) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Null: INSTRUMENT_DEFINITION_ID passed in!\n";
        return "";
    }
    if (SIGNER_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SIGNER_ID passed in!\n";
        return "";
    }
    const Identifier theNotaryID(NOTARY_ID),
        theInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID),
        theSignerID(SIGNER_ID);
    OTPasswordData thePWData("Creating a password-protected cash purse.");
    Nym* pNym = OTAPI()->GetOrLoadPrivateNym(
        theSignerID,
        false,
        __FUNCTION__,
        &thePWData);  // These copiously log, and ASSERT.
    if (nullptr == pNym) return "";
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    std::unique_ptr<Purse> pPurse(
        OTAPI()->CreatePurse_Passphrase(
            theNotaryID, theInstrumentDefinitionID));

    if (nullptr != pPurse) {
        pPurse->SignContract(*pNym, &thePWData);
        pPurse->SaveContract();
        const String strOutput(*pPurse);
        std::string pBuf = strOutput.Get();

        return pBuf;
    }
    return "";
}

// Returns the TOKEN on top of the stock (LEAVING it on top of the stack,
// but giving you a decrypted string copy of it.)
//
// OWNER_ID can be "", **if** the purse is password-protected.
// (It's just ignored in that case.) Otherwise MUST contain the NymID for
// the Purse owner (necessary to decrypt the token.)
//
// returns "" if failure.
//
std::string OTAPI_Exec::Purse_Peek(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& OWNER_ID,  // This should be "", **IF** purse is
                                  // password-protected. Otherwise MUST contain
                                  // the NymID for the Purse owner (necessary to
                                  // decrypt the token.)
    const std::string& THE_PURSE) const
{
    String strOutput;  // for later.

    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Null: INSTRUMENT_DEFINITION_ID passed in!\n";
        return "";
    }
    // if (OWNER_ID.empty())            { otErr << __FUNCTION__ << ": Null:
    // OWNER_ID passed
    // in!\n"; OT_FAIL; }
    if (THE_PURSE.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_PURSE passed in!\n";
        return "";
    }

    std::string strFunc = __FUNCTION__;  //"OTAPI_Exec::Purse_Peek";
    OTPasswordData thePWData("Peeking at cash purse contents.");
    const bool& bDoesOwnerIDExist =
        (("" != OWNER_ID) && ('\0' != OWNER_ID[0]));  // If bDoesOwnerIDExist is
    // not true, then the purse
    // MUST be
    // password-protected.
    Identifier theOwnerID;
    if (bDoesOwnerIDExist) {
        const String strOwnerID(OWNER_ID);
        Nym* pNym = nullptr;
        if (strOwnerID.Exists()) {
            theOwnerID.SetString(strOwnerID);
            pNym = OTAPI()->GetOrLoadPrivateNym(
                theOwnerID,
                false,
                strFunc.c_str(),
                &thePWData);  // These copiously log, and ASSERT.
        }
        if (nullptr == pNym) return "";
    }
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    const Identifier theNotaryID(NOTARY_ID),
        theInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID);
    const String strPurse(THE_PURSE);
    std::unique_ptr<Token> pToken(
        OTAPI()->Purse_Peek(
            theNotaryID,
            theInstrumentDefinitionID,
            strPurse,
            bDoesOwnerIDExist ? &theOwnerID : nullptr,
            nullptr));
    if (nullptr != pToken) {
        pToken->SaveContractRaw(strOutput);

        std::string pBuf = strOutput.Get();

        return pBuf;
    } else
        otOut << strFunc << ": Failed peeking at a token on a cash purse.\n";
    return "";
}

// Removes the token from the top of the stock and DESTROYS IT,
// and RETURNS THE UPDATED PURSE (with the token now missing from it.)
//
// WARNING: Do not call this function unless you have PEEK()d FIRST!!
// Otherwise you will lose the token and get left "holding the bag".
//
// returns "" if failure.
//
std::string OTAPI_Exec::Purse_Pop(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& OWNER_OR_SIGNER_ID,  // The purse, in order to be
                                            // changed,
                                            // must be
    // re-signed, which requires a private Nym. If the
    // purse is password-protected, then there's no
    // owner, but you still need to pass a Nym in here
    // to sign it (doesn't really matter which one, as
    // long as the private key is available, for
    // signing.) In that case, he's the signer. But if
    // the purse DOES have a Nym owner, then you MUST
    // pass the owner's Nym ID here, in order for this
    // action to be successful. (Because we must be able
    // to load the private key for that Nym, in order to
    // perform the pop. In which case we might as well
    // use the same Nym for signing...)
    const std::string& THE_PURSE) const
{
    String strOutput;  // for later.

    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Null: INSTRUMENT_DEFINITION_ID passed in!\n";
        return "";
    }
    if (OWNER_OR_SIGNER_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: OWNER_OR_SIGNER_ID passed in!\n";
        return "";
    }
    if (THE_PURSE.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_PURSE passed in!\n";
        return "";
    }

    std::string strFunc = __FUNCTION__;  //"OTAPI_Exec::Purse_Pop";
    const String strReason("Popping a token off of a cash purse.");
    OTPasswordData thePWData(strReason);
    const Identifier theNotaryID(NOTARY_ID),
        theInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID),
        theNymID(OWNER_OR_SIGNER_ID);
    const String strPurse(THE_PURSE);
    Nym* pNym = OTAPI()->GetOrLoadPrivateNym(
        theNymID,
        false,
        strFunc.c_str(),
        &thePWData);  // These copiously log, and ASSERT.
    if (nullptr == pNym) return "";
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    std::unique_ptr<Purse> pPurse(
        OTAPI()->Purse_Pop(
            theNotaryID,
            theInstrumentDefinitionID,
            strPurse,
            &theNymID,  // Note: if the purse is password-protected, then this
                        // parameter is ignored.
            &strReason));
    if (nullptr != pPurse) {
        pPurse->ReleaseSignatures();
        pPurse->SignContract(*pNym, &thePWData);
        pPurse->SaveContract();

        pPurse->SaveContractRaw(strOutput);
        std::string pBuf = strOutput.Get();

        return pBuf;
    } else
        otOut << strFunc << ": Failed popping a token off of a cash purse.\n";

    return "";
}

// Makes an exact copy of a purse, except with no tokens inside. (Value of
// zero.)
// Useful when you need to create a temporary purse for moving tokens around,
// and
// you don't want some new symmetric/master key being generated for that purse
// as
// though it were really some new "other purse."
// For example, if you have a password-protected purse, you might want to pop
// all of
// the tokens off of it, and as you iterate, re-assign half of them to new
// ownership,
// push those onto a new purse owned by that new owner. But you only want to do
// this
// with half of the tokens... the other half of the tokens, you just want to
// push onto
// a temp purse (for the loop) that's owned by the original owner, so you can
// then save
// it back over the original in storage (since it contains "all the tokens that
// WEREN'T
// deposited" or "all the tokens that WEREN'T exported" etc.
//
// The point? If the "original owner" is a password-protected purse with a
// symmetric
// key, then you can't just generate some new "temp purse" without also
// generating a
// whole new KEY, which you DO NOT want to do. You also don't want to have to
// deal with
// re-assigning ownership back and forth between the two purses -- you just want
// to
// shove some tokens into one temporarily so you can finish your loop.
// You could take the original purse and make a copy of it, and then just pop
// all the
// tokens off of it one-by-one, but that is very cumbersome and expensive. But
// that'd
// be the only way to get a copy of the original purse with the SAME symmetric
// key,
// except empty, so you can use it as a temp purse.
// Therefore, to make that easier and solve that whole dilemma, I present:
// OTAPI_Exec::Purse_Empty.
// OTAPI_Exec::Purse_Empty takes a purse and returns an empty version of it
// (except if there's
// a symmetric/master key inside, those are preserved, so you can use it as a
// temp purse.)
//
// This function is effectively the same thing as calling Pop until the purse is
// empty.
// Returns: the empty purse, or "" if failure.
//
std::string OTAPI_Exec::Purse_Empty(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& SIGNER_ID,
    const std::string& THE_PURSE) const
{
    String strOutput;  // for later.

    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Null: INSTRUMENT_DEFINITION_ID passed in!\n";
        return "";
    }
    if (SIGNER_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SIGNER_ID passed in!\n";
        return "";
    }
    if (THE_PURSE.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_PURSE passed in!\n";
        return "";
    }

    std::string strFunc = __FUNCTION__;  //"OTAPI_Exec::Purse_Empty";
    const String strReason("Creating an empty copy of a cash purse.");
    OTPasswordData thePWData(strReason);
    const Identifier theNotaryID(NOTARY_ID),
        theInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID),
        theNymID(SIGNER_ID);
    const String strPurse(THE_PURSE);
    Nym* pNym = OTAPI()->GetOrLoadPrivateNym(
        theNymID,
        false,
        strFunc.c_str(),
        &thePWData);  // These copiously log, and ASSERT.
    if (nullptr == pNym) return "";
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    std::unique_ptr<Purse> pPurse(
        OTAPI()->Purse_Empty(
            theNotaryID, theInstrumentDefinitionID, strPurse, &strReason));
    if (nullptr != pPurse) {
        pPurse->ReleaseSignatures();
        pPurse->SignContract(*pNym, &thePWData);
        pPurse->SaveContract();

        pPurse->SaveContractRaw(strOutput);
        std::string pBuf = strOutput.Get();

        return pBuf;
    } else
        otOut << strFunc << ": Failed emptying a cash purse.\n";

    return "";
}

// Pushes a token onto the stack (of the purse.)
// Returns the updated purse (now including the token.)
//
// Returns "" if failure.
//
std::string OTAPI_Exec::Purse_Push(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& SIGNER_ID,  // The purse, in order to be changed, must be
                                   // re-signed, which requires a private Nym.
                                   // Even if the purse is password-protected,
                                   // then there's no owner, but you still need
                                   // to pass a Nym in here to sign it (doesn't
                                   // really matter which one, but must have
                                   // private key for signing.)
    const std::string& OWNER_ID,   // If the purse is password-protected, then
                                   // there's no owner, and this owner parameter
                                   // should be "". However, if the purse DOES
                                   // have a Nym owner, then you MUST pass the
                                   // owner's Nym ID here, in order for this
                                   // action to be successful. Furthermore, the
                                   // public key for that Nym must be available,
                                   // in order to encrypt the token being pushed
                                   // into the purse. (Private key NOT necessary
    // for owner, in this case.) If this fails due
    // to public key not being available, just
    // download it from the server and try again.
    const std::string& THE_PURSE,
    const std::string& THE_TOKEN) const
{
    String strOutput;  // for later.

    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Null: INSTRUMENT_DEFINITION_ID passed in!\n";
        return "";
    }
    if (SIGNER_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SIGNER_ID passed in!\n";
        return "";
    }
    //    if (OWNER_ID.empty())       { otErr << __FUNCTION__ << ": Null:
    // OWNER_ID passed
    // in!\n"; OT_FAIL; }
    if (THE_PURSE.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_PURSE passed in!\n";
        return "";
    }
    if (THE_TOKEN.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_TOKEN passed in!\n";
        return "";
    }

    std::string strFunc = __FUNCTION__;  //"OTAPI_Exec::Purse_Push";
    const String strReason("Pushing a token onto a cash purse.");
    OTPasswordData thePWData(strReason);
    const bool& bDoesOwnerIDExist =
        (("" != OWNER_ID) && ('\0' != OWNER_ID[0]));  // If bDoesOwnerIDExist is
    // not true, then the purse
    // MUST be
    // password-protected.
    Identifier theOwnerID;
    if (bDoesOwnerIDExist) {
        const String strOwnerID(OWNER_ID);
        const Nym* pOwnerNym = nullptr;
        if (strOwnerID.Exists()) {
            theOwnerID.SetString(strOwnerID);
            pOwnerNym = OTAPI()->GetOrLoadNym(
                theOwnerID,
                false,
                strFunc.c_str(),
                &thePWData);  // These copiously log, and ASSERT.
        }
        if (nullptr == pOwnerNym) return "";
    }
    // By this point, pOwnerNym is a good pointer, and is on the wallet. (No
    // need to cleanup.)
    const Identifier theNotaryID(NOTARY_ID),
        theInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID);
    const String strPurse(THE_PURSE), strToken(THE_TOKEN);
    std::unique_ptr<Purse> pPurse(
        OTAPI()->Purse_Push(
            theNotaryID,
            theInstrumentDefinitionID,
            strPurse,
            strToken,
            bDoesOwnerIDExist ? &theOwnerID : nullptr,  // Note: if the purse is
            // password-protected, then this
            // parameter should be "".
            &strReason));
    if (nullptr != pPurse) {
        const Identifier theSignerID(SIGNER_ID);
        Nym* pSignerNym = OTAPI()->GetOrLoadPrivateNym(
            theSignerID,
            false,
            strFunc.c_str(),
            &thePWData);  // These copiously log, and ASSERT.
        if (nullptr == pSignerNym) return "";
        // By this point, pSignerNym is a good pointer, and is on the wallet.
        // (No need to cleanup.)
        pPurse->ReleaseSignatures();
        pPurse->SignContract(*pSignerNym, &thePWData);
        pPurse->SaveContract();

        pPurse->SaveContractRaw(strOutput);
        std::string pBuf = strOutput.Get();

        return pBuf;
    } else
        otOut << strFunc << ": Failed pushing a token onto a cash purse.\n";

    return "";
}

//
// Returns bool
// Should handle duplicates. Should load, merge, and save.
//
bool OTAPI_Exec::Wallet_ImportPurse(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& NYM_ID,
    const std::string& THE_PURSE) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return false;
    }
    if (INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Null: INSTRUMENT_DEFINITION_ID passed in!\n";
        return false;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return false;
    }
    if (THE_PURSE.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_PURSE passed in!\n";
        return false;
    }

    String strReason("Importing a cash purse into the wallet.");
    //  OTPasswordData thePWData(strReason);
    const Identifier theNotaryID(NOTARY_ID),
        theInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID), theNymID(NYM_ID);
    const String strNewPurse(THE_PURSE);
    // THE_PURSE (the new purse) either is for a Nym, or a Symmetric Key.
    // If it's for a Nym, it either has a NymID, or the ID is left blank.
    //
    // This call already logs on failure, so I won't bother logging again here.
    //
    String strDisplay("");

    const bool& bImported = OTAPI()->Wallet_ImportPurse(
        theNotaryID,
        theInstrumentDefinitionID,
        theNymID,
        strNewPurse,
        &strDisplay);
    return bImported ? true : false;
}

// TODO:!!!!!  NEW!!!!!

// Messages the server. If failure, make sure you didn't lose that purse!!
// If success, the new tokens will be returned shortly and saved into the
// appropriate purse.
// Note that an asset account isn't necessary to do this... just a nym operating
// cash-only.
// The same as exchanging a 20-dollar bill at the teller window for a
// replacement bill.
//
// You could also have a webpage operated by the transaction server, where a
// dummy nym
// performs cash exchanges using a single page with a text area (for copying and
// pasting
// cash tokens.) This way all cash token exchanges can go through the same Nym.
// (Although
// it must be stressed, that the cash is untraceable whether you use your own
// Nym or not.)
//
// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
// >0 means NO error, and the message was sent, and the request number fits into
// an integer...
//  ...and in fact the requestNum IS the return value!
//  ===> In 99% of cases, this LAST option is what actually happens!!
//
int32_t OTAPI_Exec::exchangePurse(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& NYM_ID,
    const std::string& THE_PURSE) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Null: INSTRUMENT_DEFINITION_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (THE_PURSE.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_PURSE passed in!\n";
        return OT_ERROR;
    }

    // todo:  exchange message.
    otErr << __FUNCTION__
          << ": TODO (NOT CODED YET) OTAPI_Exec::exchangePurse: NOTARY_ID: "
          << NOTARY_ID
          << "\n INSTRUMENT_DEFINITION_ID: " << INSTRUMENT_DEFINITION_ID
          << "\n NYM_ID: " << NYM_ID << "\n ";

    return OT_ERROR;
}

// the GUI needs to be able to export cash from one Nym to another,
// so a function has been provided for doing so to the actual Token IDs (not
// just the purse.
// Understand that even when you decrypt a token out of a purse, the token ID
// itself is still
// encrypted within that token. That is what this function is for.
//
// returns: the updated token itself in string form.
//

//
// ALLOW the caller to pass a symmetric key here, instead of either Nym ID.
// We'll load it up and use it instead of a Nym. Update: make that a purse.
// These tokens already beint64_t to specific purses, so just pass the purse
// here
//
// Done: Also, add a key cache with a timeout (similar to Master Key) where we
// can stash
// any already-loaded symmetric keys, and a thread wipes them out later. That
// way
// even if we have to load the key each time this func is called, we still avoid
// the
// user having to enter the passphrase more than once per timeout period.
//
// Done also: allow a "Signer ID" to be passed in here, since either Nym could
// potentially
// now be a symmetric key.

std::string OTAPI_Exec::Token_ChangeOwner(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& THE_TOKEN,
    const std::string& SIGNER_NYM_ID,
    const std::string& OLD_OWNER,        // Pass a NymID here, or a purse.
    const std::string& NEW_OWNER) const  // Pass a NymID here, or a purse.
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Null: INSTRUMENT_DEFINITION_ID passed in!\n";
        return "";
    }
    if (THE_TOKEN.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_TOKEN passed in!\n";
        return "";
    }
    if (SIGNER_NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SIGNER_NYM_ID passed in!\n";
        return "";
    }
    if (OLD_OWNER.empty()) {
        otErr << __FUNCTION__ << ": Null: OLD_OWNER passed in!\n";
        return "";
    }
    if (NEW_OWNER.empty()) {
        otErr << __FUNCTION__ << ": Null: NEW_OWNER passed in!\n";
        return "";
    }

    const Identifier theNotaryID(NOTARY_ID),
        theInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID),
        theSignerNymID(SIGNER_NYM_ID);
    const String strOldOwner(OLD_OWNER),  // Either of these MIGHT contain a
                                          // Nym ID, OR might contain a
                                          // purse...
        strNewOwner(NEW_OWNER);  // (purse is passed in cases where the token is
                                 // encrypted with a passphrase aka symmetric
                                 // crypto, versus being encrypted to a Nym's
                                 // public key.)
    String strToken(THE_TOKEN);
    std::unique_ptr<Token> pToken(
        OTAPI()->Token_ChangeOwner(
            theNotaryID,
            theInstrumentDefinitionID,
            strToken,
            theSignerNymID,
            strOldOwner,  // Pass a NymID here as a string, or a purse. (IF
                          // symmetrically encrypted, the relevant key is in the
                          // purse.)
            strNewOwner));  // Pass a NymID here as a string, or a purse. (IF
    // symmetrically encrypted, the relevant key is in the
    // purse.)
    if (nullptr != pToken)  // Success!
    {
        const String strOutput(*pToken);

        std::string pBuf = strOutput.Get();

        return pBuf;
    }

    return "";
}

// Returns an encrypted form of the actual blinded token ID.
// (There's no need to decrypt the ID until redeeming the token, when
// you re-encrypt it to the server's public key, or until spending it,
// when you re-encrypt it to the recipient's public key, or exporting
// it, when you create a dummy recipient and attach it to the purse.)
//
std::string OTAPI_Exec::Token_GetID(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& THE_TOKEN) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Null: INSTRUMENT_DEFINITION_ID passed in!\n";
        return "";
    }
    if (THE_TOKEN.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_TOKEN passed in!\n";
        return "";
    }

    const Identifier theNotaryID(NOTARY_ID),
        theInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID);

    String strOutput("0");

    String strToken(THE_TOKEN);
    std::unique_ptr<Token> pToken(
        Token::TokenFactory(strToken, theNotaryID, theInstrumentDefinitionID));

    if (nullptr != pToken)  // TokenFactory instantiates AND loads from string.
    {
        const OTASCIIArmor& ascSpendable =
            pToken->GetSpendable();  // encrypted version of Token ID, used as
                                     // an
                                     // "ID" on client side.

        strOutput.Format("%s", ascSpendable.Get());
    }

    std::string pBuf = strOutput.Get();

    return pBuf;
}

// The actual cash value of the token. Returns -1 on error.
//
int64_t OTAPI_Exec::Token_GetDenomination(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& THE_TOKEN) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return -1;
    }
    if (INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Null: INSTRUMENT_DEFINITION_ID passed in!\n";
        return -1;
    }
    if (THE_TOKEN.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_TOKEN passed in!\n";
        return -1;
    }

    const Identifier theNotaryID(NOTARY_ID),
        theInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID);

    String strOutput("0");

    String strToken(THE_TOKEN);
    std::unique_ptr<Token> pToken(
        Token::TokenFactory(strToken, theNotaryID, theInstrumentDefinitionID));

    if (nullptr != pToken)  // TokenFactory instantiates AND loads from string.
    {
        return pToken->GetDenomination();
    } else
        return -1;
}

// OTAPI_Exec::Token_GetSeries
// Returns -1 for error.
// Otherwise returns the series number of this token. (Int.)
//
int32_t OTAPI_Exec::Token_GetSeries(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& THE_TOKEN) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Null: INSTRUMENT_DEFINITION_ID passed in!\n";
        return OT_ERROR;
    }
    if (THE_TOKEN.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_TOKEN passed in!\n";
        return OT_ERROR;
    }

    const Identifier theNotaryID(NOTARY_ID),
        theInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID);

    String strOutput;

    String strToken(THE_TOKEN);
    std::unique_ptr<Token> pToken(
        Token::TokenFactory(strToken, theNotaryID, theInstrumentDefinitionID));

    if (nullptr != pToken)  // TokenFactory instantiates AND loads from string.
        return pToken->GetSeries();

    return OT_ERROR;
}

// the date is seconds since Jan 1970, but returned as a string.
// Return -1 on error;
//
time64_t OTAPI_Exec::Token_GetValidFrom(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& THE_TOKEN) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OTTimeGetTimeFromSeconds(-1);
    }
    if (INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Null: INSTRUMENT_DEFINITION_ID passed in!\n";
        return OTTimeGetTimeFromSeconds(-1);
    }
    if (THE_TOKEN.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_TOKEN passed in!\n";
        return OTTimeGetTimeFromSeconds(-1);
    }

    const Identifier theNotaryID(NOTARY_ID),
        theInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID);

    String strOutput;

    String strToken(THE_TOKEN);
    std::unique_ptr<Token> pToken(
        Token::TokenFactory(strToken, theNotaryID, theInstrumentDefinitionID));

    if (nullptr != pToken)  // TokenFactory instantiates AND loads from string.
    {
        return pToken->GetValidFrom();
    }
    return OTTimeGetTimeFromSeconds(-1);
}

// the date is seconds since Jan 1970.
// Return -1 on error;
//
time64_t OTAPI_Exec::Token_GetValidTo(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& THE_TOKEN) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OTTimeGetTimeFromSeconds(-1);
    }
    if (INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Null: INSTRUMENT_DEFINITION_ID passed in!\n";
        return OTTimeGetTimeFromSeconds(-1);
    }
    if (THE_TOKEN.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_TOKEN passed in!\n";
        return OTTimeGetTimeFromSeconds(-1);
    }

    const Identifier theNotaryID(NOTARY_ID),
        theInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID);

    String strOutput;

    String strToken(THE_TOKEN);
    std::unique_ptr<Token> pToken(
        Token::TokenFactory(strToken, theNotaryID, theInstrumentDefinitionID));

    if (nullptr != pToken)  // TokenFactory instantiates AND loads from string.
    {
        return pToken->GetValidTo();
    }
    return OTTimeGetTimeFromSeconds(-1);
}

std::string OTAPI_Exec::Token_GetInstrumentDefinitionID(
    const std::string& THE_TOKEN) const
{
    if (THE_TOKEN.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_TOKEN passed in!\n";
        return "";
    }

    String strOutput;

    String strToken(THE_TOKEN);
    std::unique_ptr<Token> pToken(Token::TokenFactory(strToken));

    if (nullptr != pToken)  // TokenFactory instantiates AND loads from string.
    {
        const Identifier& theID = pToken->GetInstrumentDefinitionID();
        theID.GetString(strOutput);
    }

    std::string pBuf = strOutput.Get();

    return pBuf;
}

std::string OTAPI_Exec::Token_GetNotaryID(const std::string& THE_TOKEN) const
{
    if (THE_TOKEN.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_TOKEN passed in!\n";
        return "";
    }

    String strOutput;

    String strToken(THE_TOKEN);
    std::unique_ptr<Token> pToken(Token::TokenFactory(strToken));

    if (nullptr != pToken)  // TokenFactory instantiates AND loads from string.
    {
        const Identifier& theID = pToken->GetNotaryID();
        theID.GetString(strOutput);
    }

    std::string pBuf = strOutput.Get();

    return pBuf;
}

// IS BASKET CURRENCY ?
//
// Tells you whether or not a given instrument definition is actually a basket
// currency.
//
// returns bool (true or false aka 1 or 0.)
//
bool OTAPI_Exec::IsBasketCurrency(
    const std::string& INSTRUMENT_DEFINITION_ID) const
{
    if (INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Null: INSTRUMENT_DEFINITION_ID passed in!\n";
        return false;
    }

    const Identifier theInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID);

    if (OTAPI()->IsBasketCurrency(theInstrumentDefinitionID))
        return true;
    else
        return false;
}

// Get Basket Count (of backing instrument definitions.)
//
// Returns the number of instrument definitions that make up this basket.
// (Or zero.)
//
int32_t OTAPI_Exec::Basket_GetMemberCount(
    const std::string& INSTRUMENT_DEFINITION_ID) const
{
    if (INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Null: INSTRUMENT_DEFINITION_ID passed in!\n";
        return 0;
    }

    const Identifier theInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID);

    return OTAPI()->GetBasketMemberCount(theInstrumentDefinitionID);
}

// Get Asset Type of a basket's member currency, by index.
//
// (Returns a string containing Instrument Definition ID, or "").
//
std::string OTAPI_Exec::Basket_GetMemberType(
    const std::string& BASKET_INSTRUMENT_DEFINITION_ID,
    const int32_t& nIndex) const
{
    if (BASKET_INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Null: BASKET_INSTRUMENT_DEFINITION_ID passed in!\n";
        return "";
    }

    if (0 > nIndex) {
        otErr << __FUNCTION__
              << ": nIndex is out of bounds (it's in the negative!)\n";
        return "";
    }

    const Identifier theInstrumentDefinitionID(BASKET_INSTRUMENT_DEFINITION_ID);

    Identifier theOutputMemberType;

    bool bGotType = OTAPI()->GetBasketMemberType(
        theInstrumentDefinitionID, nIndex, theOutputMemberType);
    if (!bGotType) return "";

    String strOutput(theOutputMemberType);

    std::string pBuf = strOutput.Get();

    return pBuf;
}

// GET BASKET MINIMUM TRANSFER AMOUNT
//
// Returns a int64_t containing the minimum transfer
// amount for the entire basket.
//
// Returns OT_ERROR_AMOUNT on error.
//
// FOR EXAMPLE:
// If the basket is defined as 10 Rands == 2 Silver, 5 Gold, 8 Euro,
// then the minimum transfer amount for the basket is 10. This function
// would return a string containing "10", in that example.
//
int64_t OTAPI_Exec::Basket_GetMinimumTransferAmount(
    const std::string& BASKET_INSTRUMENT_DEFINITION_ID) const
{
    if (BASKET_INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Null: BASKET_INSTRUMENT_DEFINITION_ID passed in!\n";
        return OT_ERROR_AMOUNT;
    }

    const Identifier theInstrumentDefinitionID(BASKET_INSTRUMENT_DEFINITION_ID);

    int64_t lMinTransAmount =
        OTAPI()->GetBasketMinimumTransferAmount(theInstrumentDefinitionID);

    if (0 >= lMinTransAmount) {
        otErr
            << __FUNCTION__
            << ": returned 0 (or negitive). Strange... what basket is this?\n";
        return OT_ERROR_AMOUNT;
    }

    return lMinTransAmount;
}

// GET BASKET MEMBER's MINIMUM TRANSFER AMOUNT
//
// Returns a int64_t containing the minimum transfer
// amount for one of the member currencies in the basket.
//
// Returns OT_ERROR_AMOUNT on error.
//
// FOR EXAMPLE:
// If the basket is defined as 10 Rands == 2 Silver, 5 Gold, 8 Euro,
// then the minimum transfer amount for the member currency at index
// 0 is 2, the minimum transfer amount for the member currency at
// index 1 is 5, and the minimum transfer amount for the member
// currency at index 2 is 8.
//
int64_t OTAPI_Exec::Basket_GetMemberMinimumTransferAmount(
    const std::string& BASKET_INSTRUMENT_DEFINITION_ID,
    const int32_t& nIndex) const
{
    if (BASKET_INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Null: BASKET_INSTRUMENT_DEFINITION_ID passed in!\n";
        return OT_ERROR_AMOUNT;
    }

    if (0 > nIndex) {
        otErr << __FUNCTION__
              << ": nIndex is out of bounds (it's in the negative!)\n";
        return OT_ERROR_AMOUNT;
    }

    const Identifier theInstrumentDefinitionID(BASKET_INSTRUMENT_DEFINITION_ID);

    int64_t lMinTransAmount = OTAPI()->GetBasketMemberMinimumTransferAmount(
        theInstrumentDefinitionID, nIndex);

    if (0 >= lMinTransAmount) {
        otErr
            << __FUNCTION__
            << ": returned 0 (or negitive). Strange... what basket is this?\n";
        return OT_ERROR_AMOUNT;
    }

    return lMinTransAmount;
}

// MESSAGES BEING SENT TO THE SERVER:

// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
// >0 means NO error, and the message was sent, and the request number fits into
// an integer...
//  ...and in fact the requestNum IS the return value!
//  ===> In 99% of cases, this LAST option is what actually happens!!
//
int32_t OTAPI_Exec::pingNotary(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }

    Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID);

    return OTAPI()->pingNotary(theNotaryID, theNymID);
}

// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
// >0 means NO error, and the message was sent, and the request number fits into
// an integer...
//  ...and in fact the requestNum IS the return value!
//  ===> In 99% of cases, this LAST option is what actually happens!!
//
int32_t OTAPI_Exec::registerNym(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }

    Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID);

    return OTAPI()->registerNym(theNotaryID, theNymID);
}

// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
// >0 means NO error, and the message was sent, and the request number fits into
// an integer...
//  ...and in fact the requestNum IS the return value!
//  ===> In 99% of cases, this LAST option is what actually happens!!
//
int32_t OTAPI_Exec::unregisterNym(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }

    Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID);

    return OTAPI()->unregisterNym(theNotaryID, theNymID);
}

// If THE_MESSAGE is of command type usageCreditsResponse, and IF it was a
// SUCCESS,
// then this function returns the usage credits BALANCE (it's a int64_t int32_t,
// but
// passed as a string). If you adjusted the balance using the usageCredits
// message (THE_MESSAGE being the server's reply to that) then you will see
// the balance AFTER the adjustment. (The "Current" Usage Credits balance.)
//
// UPDATE: Notice that we now return -2 in the case of all errors.
//         This is because the lowest possible actual value is -1.
//
//         - Basically a 0 or positive value means that usage credits are
//           actually turned on (on the server side) and that you are seeing
//           the actual usage credits value for that Nym.
//
//         - Whereas a -2 value means there was an error.
//
//         - Whereas a -1 value means that usage credits are turned off (on
//           the server side) OR that the Nym has been given the special -1
//           setting for usage credits, which enables him to operate as if he
//           has unlimited usage credits.
//
int64_t OTAPI_Exec::Message_GetUsageCredits(
    const std::string& THE_MESSAGE) const
{
    if (THE_MESSAGE.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_MESSAGE passed in!\n";
        return -2;
    }

    String strMessage(THE_MESSAGE);
    Message theMessage;

    if (!strMessage.Exists()) {
        otErr << __FUNCTION__ << ": Error: THE_MESSAGE doesn't exist.\n";
        return -2;
    }

    if (!theMessage.LoadContractFromString(strMessage)) {
        otErr << __FUNCTION__ << ": Failed loading message from string.\n";
        return -2;
    }

    if (!theMessage.m_bSuccess) {
        otErr << __FUNCTION__ << ": Message success == false, thus unable to "
                                 "report Usage Credits balance. (Returning "
                                 "-2.)\n";
        return -2;
    }

    if (!theMessage.m_strCommand.Compare("usageCreditsResponse")) {
        otErr << __FUNCTION__
              << ": THE_MESSAGE is supposed to be of command "
                 "type \"usageCreditsResponse\", but instead it's a: "
              << theMessage.m_strCommand << "\n (Failure. Returning -2.)";
        return -2;
    }

    // By this point, we know the message was a successful usageCreditsResponse,
    // loaded
    // properly from the string that was passed in. Let's return the usage
    // credits
    // balance (a int64_t int32_t, returned in string format.)

    return theMessage.m_lDepth;
}

// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
// >0 means NO error, and the message was sent, and the request number fits into
// an integer...
//  ...and in fact the requestNum IS the return value!
//  ===> In 99% of cases, this LAST option is what actually happens!!
//
int32_t OTAPI_Exec::usageCredits(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& NYM_ID_CHECK,
    const int64_t& ADJUSTMENT) const  // can be "0",
                                      // or
// "", if you just
// want to check the
// current balance
// without adjusting
// it.
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID_CHECK.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID_CHECK passed in!\n";
        return OT_ERROR;
    }

    //    OT_ASSERT_MSG("" != ADJUSTMENT, "OTAPI_Exec::usageCredits: Null
    // ADJUSTMENT passed in."); // "" is allowed here.

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theOtherNymID(NYM_ID_CHECK);

    const int64_t lAdjustment = ADJUSTMENT;  // "" resolves as 0.

    return OTAPI()->usageCredits(
        theNotaryID,
        theNymID,
        theOtherNymID,
        static_cast<int64_t>(lAdjustment));
}

// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
// >0 means NO error, and the message was sent, and the request number fits into
// an integer...
//  ...and in fact the requestNum IS the return value!
//  ===> In 99% of cases, this LAST option is what actually happens!!
//
int32_t OTAPI_Exec::checkNym(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& NYM_ID_CHECK) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID_CHECK.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID_CHECK passed in!\n";
        return OT_ERROR;
    }

    Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theOtherNymID(NYM_ID_CHECK);

    return OTAPI()->checkNym(theNotaryID, theNymID, theOtherNymID);
}

// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
// >0 means NO error, and the message was sent, and the request number fits into
// an integer...
//  ...and in fact the requestNum IS the return value!
//  ===> In 99% of cases, this LAST option is what actually happens!!
//
int32_t OTAPI_Exec::sendNymMessage(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& NYM_ID_RECIPIENT,
    const std::string& THE_MESSAGE) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID_RECIPIENT.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID_RECIPIENT passed in!\n";
        return OT_ERROR;
    }
    if (THE_MESSAGE.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_MESSAGE passed in!\n";
        return OT_ERROR;
    }

    Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theOtherNymID(NYM_ID_RECIPIENT);
    String strMessage(THE_MESSAGE);

    return OTAPI()->sendNymMessage(
        theNotaryID, theNymID, theOtherNymID, strMessage);
}

std::string OTAPI_Exec::notifyBailment(
    const std::string& serverID,
    const std::string& senderNymID,
    const std::string& recipientNymID,
    const std::string& unitID,
    const std::string& txid) const
{
    auto senderNym = App::Me().Contract().Nym(Identifier(senderNymID));
    std::unique_ptr<PeerRequest> request(
        PeerRequest::Create(
            senderNym,
            proto::PEERREQUEST_PENDINGBAILMENT,
            Identifier(unitID),
            Identifier(serverID),
            Identifier(recipientNymID),
            txid));

    if (request) {
        return proto::ProtoAsString(request->Contract());
    }

    return "";
}

std::string OTAPI_Exec::initiateBailment(
    const std::string& serverID,
    const std::string& senderNymID,
    const std::string& unitID) const
{
    auto senderNym = App::Me().Contract().Nym(Identifier(senderNymID));
    std::unique_ptr<PeerRequest> request(
        PeerRequest::Create(
            senderNym,
            proto::PEERREQUEST_BAILMENT,
            Identifier(unitID),
            Identifier(serverID)));

    if (request) {
        return proto::ProtoAsString(request->Contract());
    }

    return "";
}

std::string OTAPI_Exec::initiateOutBailment(
    const std::string& serverID,
    const std::string& senderNymID,
    const std::string& unitID,
    const std::uint64_t& amount,
    const std::string& terms) const
{
    auto senderNym = App::Me().Contract().Nym(Identifier(senderNymID));
    std::unique_ptr<PeerRequest> request(
        PeerRequest::Create(
            senderNym,
            proto::PEERREQUEST_OUTBAILMENT,
            Identifier(unitID),
            Identifier(serverID),
            amount,
            terms));

    if (request) {
        return proto::ProtoAsString(request->Contract());
    }

    return "";
}

std::string OTAPI_Exec::acknowledgeBailment(
    const std::string& senderNymID,
    const std::string& requestID,
    const std::string& terms) const
{
    auto senderNym = App::Me().Contract().Nym(Identifier(senderNymID));
    std::unique_ptr<PeerReply> reply(
        PeerReply::Create(
            senderNym,
            proto::PEERREQUEST_BAILMENT,
            Identifier(requestID),
            terms));

    if (reply) {
        return proto::ProtoAsString(reply->Contract());
    }

    return "";
}

std::string OTAPI_Exec::acknowledgeNotice(
    const std::string& senderNymID,
    const std::string& requestID,
    const bool ack) const
{
    auto senderNym = App::Me().Contract().Nym(Identifier(senderNymID));
    std::unique_ptr<PeerReply> reply(
        PeerReply::Create(
            senderNym,
            Identifier(requestID),
            ack));

    if (reply) {
        return proto::ProtoAsString(reply->Contract());
    }

    return "";
}

std::string OTAPI_Exec::acknowledgeOutBailment(
    const std::string& senderNymID,
    const std::string& requestID,
    const std::string& terms) const
{
    auto senderNym = App::Me().Contract().Nym(Identifier(senderNymID));
    std::unique_ptr<PeerReply> reply(
        PeerReply::Create(
            senderNym,
            proto::PEERREQUEST_OUTBAILMENT,
            Identifier(requestID),
            terms));

    if (reply) {
        return proto::ProtoAsString(reply->Contract());
    }

    return "";
}

int32_t OTAPI_Exec::initiatePeerRequest(
    const std::string& sender,
    const std::string& recipient,
    const std::string& server,
    const std::string& request) const
{
    if (0 == sender.size()) {
        otErr << __FUNCTION__ << ": Sender is empty." << std::endl;

        return -1;
    }

    if (0 == recipient.size()) {
        otErr << __FUNCTION__ << ": Recipent is empty." << std::endl;

        return -1;
    }

    if (0 == server.size()) {
        otErr << __FUNCTION__ << ": Server is empty." << std::endl;

        return -1;
    }

    if (0 == request.size()) {
        otErr << __FUNCTION__ << ": Request is empty." << std::endl;

        return -1;
    }

    const Identifier senderID(sender);
    auto senderNym = App::Me().Contract().Nym(senderID);

    std::unique_ptr<PeerRequest> instantiated(
        PeerRequest::Factory(
            senderNym,
            proto::TextToProto<proto::PeerRequest>(request)));

    return OTAPI()->initiatePeerRequest(
        senderID,
        Identifier(recipient),
        Identifier(server),
        instantiated);
}

int32_t OTAPI_Exec::initiatePeerReply(
    const std::string& sender,
    const std::string& recipient,
    const std::string& server,
    const std::string& request,
    const std::string& reply) const
{
    if (0 == sender.size()) {
        otErr << __FUNCTION__ << ": Sender is empty." << std::endl;

        return -1;
    }

    if (0 == recipient.size()) {
        otErr << __FUNCTION__ << ": Recipent is empty." << std::endl;

        return -1;
    }

    if (0 == server.size()) {
        otErr << __FUNCTION__ << ": Server is empty." << std::endl;

        return -1;
    }

    if (0 == request.size()) {
        otErr << __FUNCTION__ << ": Request is empty." << std::endl;

        return -1;
    }

    if (0 == reply.size()) {
        otErr << __FUNCTION__ << ": Reply is empty." << std::endl;

        return -1;
    }

    const Identifier senderID(sender);
    auto senderNym = App::Me().Contract().Nym(senderID);

    std::unique_ptr<PeerReply> instantiated(
        PeerReply::Factory(
            senderNym,
            proto::TextToProto<proto::PeerReply>(reply)));

    return OTAPI()->initiatePeerReply(
        senderID,
        Identifier(recipient),
        Identifier(server),
        Identifier(request),
        instantiated);
}

int32_t OTAPI_Exec::completePeerReply(
    const std::string& nymID,
    const std::string& replyID) const
{
    const Identifier nym(nymID);
    const Identifier reply(replyID);

    return App::Me().Contract().PeerReplyComplete(nym, reply);
}

int32_t OTAPI_Exec::completePeerRequest(
    const std::string& nymID,
    const std::string& requestID) const
{
    const Identifier nym(nymID);
    const Identifier request(requestID);

    return App::Me().Contract().PeerRequestComplete(nym, request);
}

std::list<std::string> OTAPI_Exec::getIncomingRequests(
    const std::string& nymID) const
{
    const Identifier nym(nymID);
    const auto requests = App::Me().Contract().PeerRequestIncoming(nym);
    std::list<std::string> output;

    for (auto& item : requests) {
        output.push_back(item.first);
    }

    return output;
}

std::list<std::string> OTAPI_Exec::getIncomingReplies(
    const std::string& nymID) const
{
    const Identifier nym(nymID);
    const auto requests = App::Me().Contract().PeerReplyIncoming(nym);
    std::list<std::string> output;

    for (auto& item : requests) {
        output.push_back(item.first);
    }

    return output;
}

std::string OTAPI_Exec::getRequest(
    const std::string& nymID,
    const std::string& requestID) const
{
    auto request = App::Me().Contract().PeerRequest(
        Identifier(nymID),
        Identifier(requestID),
        StorageBox::INCOMINGPEERREQUEST);

    if (request) {
        return proto::ProtoAsString(*request);
    }

    return "";
}

std::string OTAPI_Exec::getReply(
    const std::string& nymID,
    const std::string& replyID) const
{
    auto reply = App::Me().Contract().PeerReply(
        Identifier(nymID),
        Identifier(replyID),
        StorageBox::INCOMINGPEERREPLY);

    if (reply) {
        return proto::ProtoAsString(*reply);
    }

    return "";
}

// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
// >0 means NO error, and the message was sent, and the request number fits into
// an integer...
//  ...and in fact the requestNum IS the return value!
//  ===> In 99% of cases, this LAST option is what actually happens!!
//
int32_t OTAPI_Exec::sendNymInstrument(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& NYM_ID_RECIPIENT,
    const std::string& THE_INSTRUMENT,
    const std::string& INSTRUMENT_FOR_SENDER) const  // Can be empty. Special
                                                     // version
// for the sender's outpayments
// box. Only used for cash:
// THE_INSTRUMENT contains tokens
// already encrypted to the
// recipient's key. So this
// version contains tokens
// encrypted to the sender's key
// instead.
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID_RECIPIENT.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID_RECIPIENT passed in!\n";
        return OT_ERROR;
    }
    if (THE_INSTRUMENT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_INSTRUMENT passed in!\n";
        return OT_ERROR;
    }
    //    if (INSTRUMENT_FOR_SENDER.empty()) { otErr << __FUNCTION__ << ": Null:
    // " << "INSTRUMENT_FOR_SENDER" << "
    // passed in!\n"; OT_FAIL; }
    // (INSTRUMENT_FOR_SENDER is optional, so we allow it to be empty.)

    Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theOtherNymID(NYM_ID_RECIPIENT);
    String strInstrument(THE_INSTRUMENT);
    // Note: this was removed and can be deleted from the code.
    //
    // Why? Because we pass the string version of the public key,
    // not the OTASCIIArmor version. The bookends are passed intact
    // into the OpenSSL call since the key is expected that way.
    // Whereas loading it into ascii-armor is just going to strip off
    // the bookends and pass the raw base64-encoded pubkey to OpenSSL
    // which is NOT what OpenSSL is expecting. FYI.
    //
    // Todo, security: Should we do this anyway, just purely for validation
    // purposes?
    //
    //  OTASCIIArmor ascRecipPubkey;
    //  const bool bLoadedArmor = OTASCIIArmor::LoadFromString(ascRecipPubkey,
    // strRecipPubkey); // str_bookend="-----BEGIN" by default
    //
    //  if (!bLoadedArmor || !ascRecipPubkey.Exists())
    //  {
    //      otErr << __FUNCTION__ << ": Failure loading string into OTASCIIArmor
    // object:\n\n" << strRecipPubkey << "\n\n";
    //      return OT_ERROR;
    //  }
    OTPayment thePayment(strInstrument);

    if (!thePayment.IsValid() || !thePayment.SetTempValues()) {
        otOut << __FUNCTION__ << ": Failure loading payment instrument "
                                 "(intended for recipient) from string:\n\n"
              << strInstrument << "\n\n";
        return OT_ERROR;
    }
    const bool bSenderCopyIncluded = (INSTRUMENT_FOR_SENDER.size() > 0);

    if (bSenderCopyIncluded) {
        String strInstrumentForSender(INSTRUMENT_FOR_SENDER);
        OTPayment theSenderPayment(strInstrumentForSender);

        if (!theSenderPayment.IsValid() || !theSenderPayment.SetTempValues()) {
            otOut << __FUNCTION__
                  << ": Failure loading payment instrument (copy intended for "
                     "sender's records) from string:\n\n"
                  << strInstrumentForSender << "\n\n";
            return OT_ERROR;
        }
        return OTAPI()->sendNymInstrument(
            theNotaryID,
            theNymID,
            theOtherNymID,
            thePayment,
            &theSenderPayment);
    }
    return OTAPI()->sendNymInstrument(
        theNotaryID, theNymID, theOtherNymID, thePayment);
}

// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
//  1 means the "getRequestNumber" message was successfully SENT.
//
int32_t OTAPI_Exec::getRequestNumber(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }

    Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID);

    return OTAPI()->getRequestNumber(theNotaryID, theNymID);
}

// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
// >0 means NO error, and the message was sent, and the request number fits into
// an integer...
//  ...and in fact the requestNum IS the return value!
//  ===> In 99% of cases, this LAST option is what actually happens!!
//
int32_t OTAPI_Exec::registerInstrumentDefinition(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& THE_CONTRACT) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (THE_CONTRACT.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CONTRACT passed in!\n";
        return OT_ERROR;
    }

    Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID);

    String strContract(THE_CONTRACT);

    return OTAPI()->registerInstrumentDefinition(
        theNotaryID, theNymID, strContract);
}

// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
// >0 means NO error, and the message was sent, and the request number fits into
// an integer...
//  ...and in fact the requestNum IS the return value!
//  ===> In 99% of cases, this LAST option is what actually happens!!
//
int32_t OTAPI_Exec::getInstrumentDefinition(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& INSTRUMENT_DEFINITION_ID) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Null: INSTRUMENT_DEFINITION_ID passed in!\n";
        return OT_ERROR;
    }

    Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID);

    return OTAPI()->getInstrumentDefinition(
        theNotaryID, theNymID, theInstrumentDefinitionID);
}

// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
// >0 means NO error, and the message was sent, and the request number fits into
// an integer...
//  ...and in fact the requestNum IS the return value!
//  ===> In 99% of cases, this LAST option is what actually happens!!
//
int32_t OTAPI_Exec::getMint(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& INSTRUMENT_DEFINITION_ID) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Null: INSTRUMENT_DEFINITION_ID passed in!\n";
        return OT_ERROR;
    }

    Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID);

    return OTAPI()->getMint(theNotaryID, theNymID, theInstrumentDefinitionID);
}

// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
// >0 means NO error, and the message was sent, and the request number fits into
// an integer...
//  ...and in fact the requestNum IS the return value!
//  ===> In 99% of cases, this LAST option is what actually happens!!
//
int32_t OTAPI_Exec::registerAccount(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& INSTRUMENT_DEFINITION_ID) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Null: INSTRUMENT_DEFINITION_ID passed in!\n";
        return OT_ERROR;
    }

    Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID);

    return OTAPI()->registerAccount(
        theNotaryID, theNymID, theInstrumentDefinitionID);
}

// Sends a message to the server to retrieve latest copy of an asset acct.
// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
// >0 means NO error, and the message was sent, and the request number fits into
// an integer...
//  ...and in fact the requestNum IS the return value!
//  ===> In 99% of cases, this LAST option is what actually happens!!
//
int32_t OTAPI_Exec::getAccountData(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCT_ID) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (ACCT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCT_ID passed in!\n";
        return OT_ERROR;
    }

    Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID), theAcctID(ACCT_ID);

    return OTAPI()->getAccountData(theNotaryID, theNymID, theAcctID);
}

// GENERATE BASKET CREATION REQUEST
//
// (returns the basket in string form.)
//
// Call OTAPI_Exec::AddBasketCreationItem multiple times to add
// the various currencies to the basket, and then call
// OTAPI_Exec::issueBasket to send the request to the server.
//
std::string OTAPI_Exec::GenerateBasketCreation(
    const std::string& serverID,
    const std::string& shortname,
    const std::string& name,
    const std::string& symbol,
    const std::string& terms,
    const uint64_t weight) const
{
    auto serverContract = App::Me().Contract().Server(Identifier(serverID));

    if (!serverContract) {
        return "";
    }

    auto basketTemplate = UnitDefinition::Create(
        serverContract->Nym(), shortname, name, symbol, terms, weight);

    std::string str_return =
        (proto::ProtoAsArmored<proto::UnitDefinition>(
             basketTemplate->PublicContract(), "BASKET CONTRACT"))
            .Get();
    return str_return;
}

// ADD BASKET CREATION ITEM
//
// (returns the updated basket in string form.)
//
// Call OTAPI_Exec::GenerateBasketCreation first (above), then
// call this function multiple times to add the various
// currencies to the basket, and then call OTAPI_Exec::issueBasket
// to send the request to the server.
//
std::string OTAPI_Exec::AddBasketCreationItem(
    const std::string& basketTemplate,
    const std::string& currencyID,
    const uint64_t& weight) const
{

    if (basketTemplate.empty()) {
        otErr << __FUNCTION__ << ": Null basketTemplate passed in!\n";
        return "";
    }
    if (currencyID.empty()) {
        otErr << __FUNCTION__ << ": Null: currencyID passed in!\n";
        return "";
    }

    bool bAdded = false;
    auto contract =
        proto::StringToProto<proto::UnitDefinition>(String(basketTemplate));

    bAdded =
        OTAPI()->AddBasketCreationItem(contract, String(currencyID), weight);

    if (!bAdded) {
        return "";
    }

    return proto::ProtoAsArmored(contract, "BASKET CONTRACT").Get();
}

// ISSUE BASKET CURRENCY
//
// Issue a new instrument definition based on a BASKET of other instrument
// definitions!
// You cannot call this function until you first set up the BASKET_INFO object.
// I will provide functions for setting up that object, so that you can then
// call this function to actually message the server with your request.
//
// ANYONE can issue a new basket type, but they will have no control over the
// issuer account. Normally when issuing a currency, you therefore control the
// issuer account. But with baskets, that is managed internally by the server.
// This means anyone can define a basket, and all may use it -- but no one
// controls it except the server.
//
// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
// >0 means NO error, and the message was sent, and the request number fits into
// an integer...
//  ...and in fact the requestNum IS the return value!
//  ===> In 99% of cases, this LAST option is what actually happens!!
//
int32_t OTAPI_Exec::issueBasket(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& THE_BASKET) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (THE_BASKET.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_BASKET passed in!\n";
        return OT_ERROR;
    }

    Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID);

    String strBasketInfo(THE_BASKET);

    return OTAPI()->issueBasket(
        theNotaryID,
        theNymID,
        proto::StringToProto<proto::UnitDefinition>(strBasketInfo));
}

// GENERATE BASKET EXCHANGE REQUEST
//
// (Returns the new basket exchange request in string form.)
//
// Call this function first. Then call OTAPI_Exec::AddBasketExchangeItem
// multiple times, and then finally call OTAPI_Exec::exchangeBasket to
// send the request to the server.
//
std::string OTAPI_Exec::GenerateBasketExchange(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& BASKET_INSTRUMENT_DEFINITION_ID,
    const std::string& BASKET_ASSET_ACCT_ID,
    const int32_t& TRANSFER_MULTIPLE) const  // 1            2             3
// 5=2,3,4  OR  10=4,6,8  OR 15=6,9,12
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    if (BASKET_INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Null: BASKET_INSTRUMENT_DEFINITION_ID passed in!\n";
        return "";
    }
    if (BASKET_ASSET_ACCT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: BASKET_ASSET_ACCT_ID passed in!\n";
        return "";
    }

    const Identifier theNymID(NYM_ID), theNotaryID(NOTARY_ID),
        theBasketInstrumentDefinitionID(BASKET_INSTRUMENT_DEFINITION_ID),
        theBasketAssetAcctID(BASKET_ASSET_ACCT_ID);
    int32_t nTransferMultiple = 1;  // Just a default value.

    if (TRANSFER_MULTIPLE > 0) nTransferMultiple = TRANSFER_MULTIPLE;
    std::unique_ptr<Basket> pBasket(
        OTAPI()->GenerateBasketExchange(
            theNotaryID,
            theNymID,
            theBasketInstrumentDefinitionID,
            theBasketAssetAcctID,
            nTransferMultiple));  // 1            2             3
    // 5=2,3,4  OR  10=4,6,8  OR 15=6,9,12

    if (nullptr == pBasket) return "";

    // At this point, I know pBasket is good (and will be cleaned up
    // automatically.)
    String strOutput(*pBasket);  // Extract the basket to string form.

    std::string pBuf = strOutput.Get();

    return pBuf;
}

// ADD BASKET EXCHANGE ITEM
//
// Returns the updated basket exchange request in string form.
// (Or "".)
//
// Call the above function first. Then call this one multiple
// times, and then finally call OTAPI_Exec::exchangeBasket to send
// the request to the server.
//
std::string OTAPI_Exec::AddBasketExchangeItem(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& THE_BASKET,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& ASSET_ACCT_ID) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    if (THE_BASKET.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_BASKET passed in!\n";
        return "";
    }
    if (INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Null: INSTRUMENT_DEFINITION_ID passed in!\n";
        return "";
    }
    if (ASSET_ACCT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ASSET_ACCT_ID passed in!\n";
        return "";
    }

    String strBasket(THE_BASKET);
    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID),
        theAssetAcctID(ASSET_ACCT_ID);
    Basket theBasket;

    bool bAdded = false;

    // todo perhaps verify the basket here, even though I already verified the
    // asset contract itself...
    // Can't never be too sure.
    if (theBasket.LoadContractFromString(strBasket)) {
        bAdded = OTAPI()->AddBasketExchangeItem(
            theNotaryID,
            theNymID,
            theBasket,
            theInstrumentDefinitionID,
            theAssetAcctID);
    }

    if (!bAdded) return "";

    String strOutput(theBasket);  // Extract the updated basket to string form.

    std::string pBuf = strOutput.Get();

    return pBuf;
}

// EXCHANGE BASKET
//
// Send a request to the server asking to exchange in or out of a basket
// currency.
//
// For example, maybe you have 3 gold, 2 silver, and 5 dollars, and those are
// the ingredients of the "Rand" basket currency. This message allows you to
// ask the server to convert from your gold, silver, and dollar accounts into
// your Rand account. (Or convert from your Rand account back into your gold,
// silver, and dollar accounts.)
//
// Other than this conversion process, (with the reserves stored internally by
// the server) basket accounts are identical to normal asset accounts -- they
// are merely another instrument definition ID, and you can use them the same as
// you would
// use any other instrument definition (open accounts, write cheques, withdraw
// cash, trade
// on markets, etc.)
//
// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
// >0 means NO error, and the message was sent, and the request number fits into
// an integer...
//  ...and in fact the requestNum IS the return value!
//  ===> In 99% of cases, this LAST option is what actually happens!!
//

int32_t OTAPI_Exec::exchangeBasket(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& BASKET_INSTRUMENT_DEFINITION_ID,
    const std::string& THE_BASKET,
    const bool& BOOL_EXCHANGE_IN_OR_OUT) const  // exchanging in == true (1),
                                                // out
                                                // ==
                                                // false (0).
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (BASKET_INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Null: BASKET_INSTRUMENT_DEFINITION_ID passed in!\n";
        return OT_ERROR;
    }
    if (THE_BASKET.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_BASKET passed in!\n";
        return OT_ERROR;
    }

    Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theBasketInstrumentDefinitionID(BASKET_INSTRUMENT_DEFINITION_ID);

    String strBasketInfo(THE_BASKET);

    // exchanging in == true, out == false.
    const bool& bExchangeInOrOut =
        ((true == BOOL_EXCHANGE_IN_OR_OUT) ? true : false);

    return OTAPI()->exchangeBasket(
        theNotaryID,
        theNymID,
        theBasketInstrumentDefinitionID,
        strBasketInfo,
        bExchangeInOrOut);
}

// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
// >0 means NO error, and the message was sent, and the request number fits into
// an integer...
//  ...and in fact the requestNum IS the return value!
//  ===> In 99% of cases, this LAST option is what actually happens!!
//
int32_t OTAPI_Exec::getTransactionNumbers(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }

    Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID);

    return OTAPI()->getTransactionNumbers(theNotaryID, theNymID);
}

// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
// >0 means NO error, and the message was sent, and the request number fits into
// an integer...
//  ...and in fact the requestNum IS the return value!
//  ===> In 99% of cases, this LAST option is what actually happens!!
//
int32_t OTAPI_Exec::notarizeWithdrawal(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCT_ID,
    const int64_t& AMOUNT) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (ACCT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCT_ID passed in!\n";
        return OT_ERROR;
    }
    if (0 > AMOUNT) {
        otErr << __FUNCTION__ << ": Negative: AMOUNT passed in!\n";
        return OT_ERROR;
    }

    Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID), theAcctID(ACCT_ID);

    return OTAPI()->notarizeWithdrawal(
        theNotaryID, theNymID, theAcctID, static_cast<int64_t>(AMOUNT));
}

// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
// >0 means NO error, and the message was sent, and the request number fits into
// an integer...
//  ...and in fact the requestNum IS the return value!
//  ===> In 99% of cases, this LAST option is what actually happens!!
//
int32_t OTAPI_Exec::notarizeDeposit(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCT_ID,
    const std::string& THE_PURSE) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (ACCT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCT_ID passed in!\n";
        return OT_ERROR;
    }
    if (THE_PURSE.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_PURSE passed in!\n";
        return OT_ERROR;
    }

    Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID), theAcctID(ACCT_ID);
    String strPurse(THE_PURSE);

    return OTAPI()->notarizeDeposit(theNotaryID, theNymID, theAcctID, strPurse);
}

// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
// >0 means NO error, and the message was sent, and the request number fits into
// an integer...
//  ...and in fact the requestNum IS the return value!
//  ===> In 99% of cases, this LAST option is what actually happens!!
//
int32_t OTAPI_Exec::notarizeTransfer(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCT_FROM,
    const std::string& ACCT_TO,
    const int64_t& AMOUNT,
    const std::string& NOTE) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (ACCT_FROM.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCT_FROM passed in!\n";
        return OT_ERROR;
    }
    if (ACCT_TO.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCT_TO passed in!\n";
        return OT_ERROR;
    }
    if (0 > AMOUNT) {
        otErr << __FUNCTION__ << ": Negative: AMOUNT passed in!\n";
        return OT_ERROR;
    }

    Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID);
    Identifier theFromAcct(ACCT_FROM), theToAcct(ACCT_TO);

    String strNote(NOTE.empty() ? "" : NOTE);

    int64_t lAmount = AMOUNT;

    return OTAPI()->notarizeTransfer(
        theNotaryID, theNymID, theFromAcct, theToAcct, lAmount, strNote);
}

// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
// >0 means NO error, and the message was sent, and the request number fits into
// an integer...
//  ...and in fact the requestNum IS the return value!
//  ===> In 99% of cases, this LAST option is what actually happens!!
//
int32_t OTAPI_Exec::getNymbox(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }

    Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID);

    return OTAPI()->getNymbox(theNotaryID, theNymID);
}

// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
// >0 means NO error, and the message was sent, and the request number fits into
// an integer...
//  ...and in fact the requestNum IS the return value!
//  ===> In 99% of cases, this LAST option is what actually happens!!
//
int32_t OTAPI_Exec::processInbox(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCT_ID,
    const std::string& ACCT_LEDGER) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (ACCT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCT_ID passed in!\n";
        return OT_ERROR;
    }
    if (ACCT_LEDGER.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCT_LEDGER passed in!\n";
        return OT_ERROR;
    }

    //    otOut << __FUNCTION__ << ": \n"
    //        "NOTARY_ID: " << NOTARY_ID << " \n"
    //        "NYM_ID: " << NYM_ID << " \n"
    //        "ACCT_ID: " << ACCT_ID << " \n"
    //        "ACCT_LEDGER:\n" << ACCT_LEDGER << "\n\n";

    Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID), theAcctID(ACCT_ID);
    String strLedger(ACCT_LEDGER);

    //    OTString temp1(NOTARY_ID), temp2(NYM_ID), temp3(ACCT_ID),
    // temp4(ACCT_LEDGER);
    //    otOut << __FUNCTION__ << ": \n"
    //        "\n\nNOTARY_ID: " << temp1 << " \n"
    //        "NYM_ID: " << temp2 << " \n"
    //        "ACCT_ID: " << temp3 << " \n"
    //        "ACCT_LEDGER:\n" << temp4 << "\n\n";

    return OTAPI()->processInbox(theNotaryID, theNymID, theAcctID, strLedger);
}

// Returns:
// -1 if error.
//  0 if Nymbox is empty.
//  1 or more: (OLD: Count of items in Nymbox before processing.)
//  UPDATE: This now returns the request number of the message sent, if success.
//
int32_t OTAPI_Exec::processNymbox(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }

    Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID);

    return OTAPI()->processNymbox(theNotaryID, theNymID);
}

// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
// >0 means NO error, and the message was sent, and the request number fits into
// an integer...
//  ...and in fact the requestNum IS the return value!
//  ===> In 99% of cases, this LAST option is what actually happens!!
//
int32_t OTAPI_Exec::withdrawVoucher(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCT_ID,
    const std::string& RECIPIENT_NYM_ID,
    const std::string& CHEQUE_MEMO,
    const int64_t& AMOUNT) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (ACCT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCT_ID passed in!\n";
        return OT_ERROR;
    }
    if (RECIPIENT_NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: RECIPIENT_NYM_ID passed in!\n";
        return OT_ERROR;
    }
    //    if (CHEQUE_MEMO.empty())        { otErr << __FUNCTION__ << ": Null:
    // CHEQUE_MEMO passed
    // in!\n"; OT_FAIL; }
    if (0 > AMOUNT) {
        otErr << __FUNCTION__ << ": Negative: AMOUNT passed in!\n";
        return OT_ERROR;
    }

    Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID), theAcctID(ACCT_ID),
        theRecipientNymID(RECIPIENT_NYM_ID);

    String strMemo(CHEQUE_MEMO);
    int64_t lAmount = AMOUNT;

    return OTAPI()->withdrawVoucher(
        theNotaryID, theNymID, theAcctID, theRecipientNymID, strMemo, lAmount);
}

// PAY DIVIDEND -- to shareholders
//
int32_t OTAPI_Exec::payDividend(
    const std::string& NOTARY_ID,
    const std::string& ISSUER_NYM_ID,  // must be issuer of
                                       // SHARES_INSTRUMENT_DEFINITION_ID
    const std::string& DIVIDEND_FROM_ACCT_ID,  // if dollars paid for pepsi
                                               // shares, then this is the
                                               // issuer's dollars account.
    const std::string& SHARES_INSTRUMENT_DEFINITION_ID,  // if dollars paid for
                                                         // pepsi
    // shares, then this is the pepsi
    // shares instrument definition id.
    const std::string& DIVIDEND_MEMO,  // user-configurable note that's added to
                                       // the payout request message.
    const int64_t& AMOUNT_PER_SHARE) const  // number of dollars to be paid out
                                            // PER
// SHARE (multiplied by total number of
// shares issued.)
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (ISSUER_NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ISSUER_NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (DIVIDEND_FROM_ACCT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: DIVIDEND_FROM_ACCT_ID passed in!\n";
        return OT_ERROR;
    }
    if (SHARES_INSTRUMENT_DEFINITION_ID.empty()) {
        otErr << __FUNCTION__
              << ": Null: SHARES_INSTRUMENT_DEFINITION_ID passed in!\n";
        return OT_ERROR;
    }
    //    if (DIVIDEND_MEMO.empty())           { otErr << __FUNCTION__ << ":
    // Null: DIVIDEND_MEMO
    // passed in!\n"; return OT_ERROR; }
    if (0 > AMOUNT_PER_SHARE) {
        otErr << __FUNCTION__ << ": Negative: AMOUNT_PER_SHARE passed in!\n";
        return OT_ERROR;
    }

    Identifier theNotaryID(NOTARY_ID), theIssuerNymID(ISSUER_NYM_ID),
        theDividendFromAcctID(DIVIDEND_FROM_ACCT_ID),
        theSharesInstrumentDefinitionID(SHARES_INSTRUMENT_DEFINITION_ID);

    String strMemo(DIVIDEND_MEMO);
    int64_t lAmount = AMOUNT_PER_SHARE;

    return OTAPI()->payDividend(
        theNotaryID,
        theIssuerNymID,
        theDividendFromAcctID,
        theSharesInstrumentDefinitionID,
        strMemo,
        lAmount);
}

// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
// >0 means NO error, and the message was sent, and the request number fits into
// an integer...
//  ...and in fact the requestNum IS the return value!
//  ===> In 99% of cases, this LAST option is what actually happens!!
//
int32_t OTAPI_Exec::depositCheque(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCT_ID,
    const std::string& THE_CHEQUE) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (ACCT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCT_ID passed in!\n";
        return OT_ERROR;
    }
    if (THE_CHEQUE.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_CHEQUE passed in!\n";
        return OT_ERROR;
    }

    Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID), theAcctID(ACCT_ID);

    String strCheque(THE_CHEQUE);

    return OTAPI()->depositCheque(theNotaryID, theNymID, theAcctID, strCheque);
}

// DEPOSIT PAYMENT PLAN
//
// See OTAPI_Exec::WritePaymentPlan as well.
//
// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
// >0 means NO error, and the message was sent, and the request number fits into
// an integer...
//  ...and in fact the requestNum IS the return value!
//  ===> In 99% of cases, this LAST option is what actually happens!!
//
int32_t OTAPI_Exec::depositPaymentPlan(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& THE_PAYMENT_PLAN) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (THE_PAYMENT_PLAN.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_PAYMENT_PLAN passed in!\n";
        return OT_ERROR;
    }

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID);
    const String strPlan(THE_PAYMENT_PLAN);

    return OTAPI()->depositPaymentPlan(theNotaryID, theNymID, strPlan);
}

// DONE: Remove Market ID.
// DONE: Change inner call from cancelNymMarketOffer to cancelCronItem
// DONE: Make a copy of this function called cancelPaymentPlan.
//
// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
// >0 means NO error, and the message was sent, and the request number fits into
// an integer...
//  ...and in fact the requestNum IS the return value!
//  ===> In 99% of cases, this LAST option is what actually happens!!
//
int32_t OTAPI_Exec::killMarketOffer(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ASSET_ACCT_ID,
    const int64_t& TRANSACTION_NUMBER) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (ASSET_ACCT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ASSET_ACCT_ID passed in!\n";
        return OT_ERROR;
    }
    if (0 > TRANSACTION_NUMBER) {
        otErr << __FUNCTION__ << ": Negative: TRANSACTION_NUMBER passed in!\n";
        return OT_ERROR;
    }

    const int64_t lTransactionNumber = TRANSACTION_NUMBER;

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theAssetAcctID(ASSET_ACCT_ID);

    return OTAPI()->cancelCronItem(
        theNotaryID,
        theNymID,
        theAssetAcctID,
        static_cast<int64_t>(lTransactionNumber));
}

// OTAPI_Exec::cancelPaymentPlan
// Cancel a payment plan by transaction number.
//
// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
// >0 means NO error, and the message was sent, and the request number fits into
// an integer...
//  ...and in fact the requestNum IS the return value!
//  ===> In 99% of cases, this LAST option is what actually happens!!
//
int32_t OTAPI_Exec::killPaymentPlan(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& FROM_ACCT_ID,
    const int64_t& TRANSACTION_NUMBER) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (FROM_ACCT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: FROM_ACCT_ID passed in!\n";
        return OT_ERROR;
    }
    if (0 > TRANSACTION_NUMBER) {
        otErr << __FUNCTION__ << ": Negative: TRANSACTION_NUMBER passed in!\n";
        return OT_ERROR;
    }

    const int64_t lTransactionNumber = TRANSACTION_NUMBER;

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theFromAcctID(FROM_ACCT_ID);

    return OTAPI()->cancelCronItem(
        theNotaryID,
        theNymID,
        theFromAcctID,
        static_cast<int64_t>(lTransactionNumber));
}

// ISSUE MARKET OFFER
//
// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
// >0 means NO error, and the message was sent, and the request number fits into
// an integer...
//  ...and in fact the requestNum IS the return value!
//  ===> In 99% of cases, this LAST option is what actually happens!!
//
int32_t OTAPI_Exec::issueMarketOffer(
    const std::string& ASSET_ACCT_ID,     // Perhaps this is the wheat market.
    const std::string& CURRENCY_ACCT_ID,  // Perhaps I'm buying the wheat with
                                          // rubles.
    const int64_t& MARKET_SCALE,          // Defaults to minimum of 1. Market
                                          // granularity.
    const int64_t& MINIMUM_INCREMENT,  // This will be multiplied by the Scale.
                                       // Min 1.
    const int64_t& TOTAL_ASSETS_ON_OFFER,  // Total assets available for sale or
                                           // purchase. Will be multiplied by
                                           // minimum increment.
    const int64_t& PRICE_LIMIT,            // Per Minimum Increment...
    const bool& bBuyingOrSelling,          // SELLING == true, BUYING == false.
    const time64_t& LIFESPAN_IN_SECONDS,   // Pass 0 for the default behavior:
                                           // 86400 seconds aka 1 day.
    const std::string& STOP_SIGN,  // Must be "" (for market/limit orders) or
                                   // "<"
                                   // or ">"  (for stop orders.)
    const int64_t& ACTIVATION_PRICE) const  // Must be provided if STOP_SIGN is
                                            // also
// set. Determines the price threshold for
// stop orders.
{
    if (ASSET_ACCT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ASSET_ACCT_ID passed in!\n";
        return OT_ERROR;
    }
    if (CURRENCY_ACCT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: CURRENCY_ACCT_ID passed in!\n";
        return OT_ERROR;
    }
    if (0 > MARKET_SCALE) {
        otErr << __FUNCTION__ << ": Negative: MARKET_SCALE passed in!\n";
        return OT_ERROR;
    }
    if (0 > MINIMUM_INCREMENT) {
        otErr << __FUNCTION__ << ": Negative: MINIMUM_INCREMENT passed in!\n";
        return OT_ERROR;
    }
    if (0 > TOTAL_ASSETS_ON_OFFER) {
        otErr << __FUNCTION__
              << ": Negative: TOTAL_ASSETS_ON_OFFER passed in!\n";
        return OT_ERROR;
    }
    if (0 > PRICE_LIMIT) {
        otErr << __FUNCTION__ << ": Negative: PRICE_LIMIT passed in!\n";
        return OT_ERROR;
    }
    if (0 > ACTIVATION_PRICE) {
        otErr << __FUNCTION__ << ": Negative: ACTIVATION_PRICE passed in!\n";
        return OT_ERROR;
    }
    char cStopSign = 0;

    if (0 == STOP_SIGN.compare("<"))
        cStopSign = '<';
    else if (0 == STOP_SIGN.compare(">"))
        cStopSign = '>';
    if (!STOP_SIGN.empty() && ((ACTIVATION_PRICE == 0) ||
                               ((cStopSign != '<') && (cStopSign != '>')))) {
        otErr << __FUNCTION__ << ": If STOP_SIGN is provided, it must be \"<\" "
                                 "or \">\", and in that case ACTIVATION_PRICE "
                                 "must be non-zero.\n";
        return OT_ERROR;
    }
    const Identifier theAssetAcctID(ASSET_ACCT_ID),
        theCurrencyAcctID(CURRENCY_ACCT_ID);
    const std::string str_asset_notary_id =
        OTAPI_Exec::GetAccountWallet_NotaryID(ASSET_ACCT_ID);
    const std::string str_currency_notary_id =
        OTAPI_Exec::GetAccountWallet_NotaryID(CURRENCY_ACCT_ID);
    const std::string str_asset_nym_id =
        OTAPI_Exec::GetAccountWallet_NymID(ASSET_ACCT_ID);
    const std::string str_currency_nym_id =
        OTAPI_Exec::GetAccountWallet_NymID(CURRENCY_ACCT_ID);
    if (str_asset_notary_id.empty() || str_currency_notary_id.empty() ||
        str_asset_nym_id.empty() || str_currency_nym_id.empty()) {
        otErr << __FUNCTION__ << ": Failed determining server or nym ID for "
                                 "either asset or currency account.\n";
        return OT_ERROR;
    }
    const Identifier theAssetNotaryID(str_asset_notary_id),
        theAssetNymID(str_asset_nym_id),
        theCurrencyNotaryID(str_currency_notary_id),
        theCurrencyNymID(str_currency_nym_id);
    if (theAssetNotaryID != theCurrencyNotaryID) {
        otErr << __FUNCTION__
              << ": The accounts provided are on two different servers.\n";
        return OT_ERROR;
    }
    if (theAssetNymID != theCurrencyNymID) {
        otErr << __FUNCTION__
              << ": The accounts provided are owned by two different nyms.\n";
        return OT_ERROR;
    }
    // 1 is the minimum value here.
    //
    int64_t lMarketScale = (0 == MARKET_SCALE) ? 1 : MARKET_SCALE;
    int64_t lMinIncrement = (0 == MINIMUM_INCREMENT) ? 1 : MINIMUM_INCREMENT;
    int64_t lTotalAssetsOnOffer =
        (0 == TOTAL_ASSETS_ON_OFFER) ? 1 : TOTAL_ASSETS_ON_OFFER;
    int64_t lPriceLimit = PRICE_LIMIT;  // 0 is allowed now, for market orders.
    return OTAPI()->issueMarketOffer(
        theAssetNotaryID,
        theAssetNymID,
        theAssetAcctID,
        theCurrencyAcctID,
        lMarketScale,
        lMinIncrement,
        lTotalAssetsOnOffer,
        lPriceLimit,
        bBuyingOrSelling,
        LIFESPAN_IN_SECONDS,
        cStopSign,
        static_cast<int64_t>(ACTIVATION_PRICE));
}

// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
// >0 means NO error, and the message was sent, and the request number fits into
// an integer...
//  ...and in fact the requestNum IS the return value!
//  ===> In 99% of cases, this LAST option is what actually happens!!
//
int32_t OTAPI_Exec::getMarketList(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID);

    return OTAPI()->getMarketList(theNotaryID, theNymID);
}

// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
// >0 means NO error, and the message was sent, and the request number fits into
// an integer...
//  ...and in fact the requestNum IS the return value!
//  ===> In 99% of cases, this LAST option is what actually happens!!
//
int32_t OTAPI_Exec::getMarketOffers(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& MARKET_ID,
    const int64_t& MAX_DEPTH) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (0 > MAX_DEPTH) {
        otErr << __FUNCTION__ << ": Negative: MAX_DEPTH passed in!\n";
        return OT_ERROR;
    }

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theMarketID(MARKET_ID);

    const int64_t lDepth = MAX_DEPTH;
    if (0 > lDepth) {
        otErr << __FUNCTION__
              << ": lDepth is out of bounds (it's in the negative!)\n";
        return OT_ERROR;
    }

    return OTAPI()->getMarketOffers(
        theNotaryID, theNymID, theMarketID, static_cast<int64_t>(lDepth));
}

// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
// >0 means NO error, and the message was sent, and the request number fits into
// an integer...
//  ...and in fact the requestNum IS the return value!
//  ===> In 99% of cases, this LAST option is what actually happens!!
//
int32_t OTAPI_Exec::getMarketRecentTrades(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& MARKET_ID) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (MARKET_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: MARKET_ID passed in!\n";
        return OT_ERROR;
    }

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theMarketID(MARKET_ID);

    return OTAPI()->getMarketRecentTrades(theNotaryID, theNymID, theMarketID);
}

// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
// >0 means NO error, and the message was sent, and the request number fits into
// an integer...
//  ...and in fact the requestNum IS the return value!
//  ===> In 99% of cases, this LAST option is what actually happens!!
//
int32_t OTAPI_Exec::getNymMarketOffers(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID);

    return OTAPI()->getNymMarketOffers(theNotaryID, theNymID);
}

// POP MESSAGE BUFFER
//
// If there are any replies from the server, they are stored in
// the message buffer. This function will return those messages
// (and remove them from the list) one-by-one, newest first.
//
// Returns the message as a string.
//
// Update: added arguments for: NotaryID AND NymID AND request number
// NOTE: Any messages, when popping, which have the CORRECT notaryID
// and the CORRECT NymID, but the wrong Request number, will be discarded.
//
// (Why? Because the client using the OT API will have already treated
// that message as "dropped" by now, if it's already on to the next one,
// and the protocol is designed to move forward properly based specifically
// on this function returning the one EXPECTED... outgoing messages flush
// the incoming buffer anyway, so the client will have assumed the wrong
// reply was flushed by now anyway.)
//
// However, if the Notary ID and the Nym ID are wrong, this just means that
// some other code is still expecting that reply, and hasn't even popped yet!
// Therefore, we do NOT want to discard THOSE replies, but put them back if
// necessary -- only discarding the ones where the IDs match.
//
//
std::string OTAPI_Exec::PopMessageBuffer(
    const int64_t& REQUEST_NUMBER,
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const
{
    if (0 > REQUEST_NUMBER) {
        otErr << __FUNCTION__ << ": Negative: REQUEST_NUMBER passed in!\n";
        return "";
    }
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }

    const int64_t lRequestNum = REQUEST_NUMBER;
    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID);
    std::shared_ptr<Message> pMsg(
        OTAPI()->PopMessageBuffer(
            static_cast<int64_t>(lRequestNum),
            theNotaryID,
            theNymID));  // caller responsible to delete.

    if (nullptr == pMsg)  // The buffer was empty.
    {
        otErr << __FUNCTION__ << ":  Reply not found, sorry.\n";
        return "";
    }

    const String strOutput(*pMsg);

    std::string pBuf = strOutput.Get();
    return pBuf;
}

// Just flat-out empties the thing.
//
void OTAPI_Exec::FlushMessageBuffer(void) const
{
    OTAPI()->FlushMessageBuffer();
}

// Message OUT-BUFFER
//
// (for messages I--the client--have sent the server.)
/*
class OTMessageOutbuffer:
void        Clear();
void        AddSentMessage      (OTMessage& theMessage);   // Allocate theMsg
on the heap (takes ownership.) Mapped by request num.
OTMessage * GetSentMessage      (const int64_t& lRequestNum); // null == not
found. caller NOT responsible to delete.
bool        RemoveSentMessage   (const int64_t& lRequestNum); // true == it was
removed. false == it wasn't found.
*/

// GET SENT MESSAGE
//
// If there were any messages sent to the server, copies are
// stored here, so the developer using the OT API can access
// them by request number.
//
// Returns the message as a string.
//
std::string OTAPI_Exec::GetSentMessage(
    const int64_t& REQUEST_NUMBER,
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const
{
    if (0 > REQUEST_NUMBER) {
        otErr << __FUNCTION__ << ": Negative: REQUEST_NUMBER passed in!\n";
        return "";
    }
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return "";
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return "";
    }
    const int64_t lRequestNum = REQUEST_NUMBER;
    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID);
    Message* pMsg = OTAPI()->GetSentMessage(
        static_cast<int64_t>(lRequestNum), theNotaryID, theNymID);

    if (nullptr == pMsg)  // The message wasn't found with that request number.
    {
        otWarn << __FUNCTION__ << ": Message not found with request number "
               << lRequestNum << ", sorry.\n";
        return "";
    }
    const String strOutput(*pMsg);  // No need to cleanup the message since
                                    // it's still in the buffer until
                                    // explicitly removed.

    std::string pBuf = strOutput.Get();

    return pBuf;
}

// REMOVE SENT MESSAGE
//
// If there were any messages sent to the server, copies are
// stored until removed via this function.
//
// Returns bool based on whether message was found (and removed.)
//
bool OTAPI_Exec::RemoveSentMessage(
    const int64_t& REQUEST_NUMBER,
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const
{
    if (0 > REQUEST_NUMBER) {
        otErr << __FUNCTION__ << ": Negative: REQUEST_NUMBER passed in!\n";
        return false;
    }
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return false;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return false;
    }
    const int64_t lRequestNum = REQUEST_NUMBER;
    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID);
    const bool& bSuccess = OTAPI()->RemoveSentMessage(
        static_cast<int64_t>(lRequestNum), theNotaryID, theNymID);

    return bSuccess;
}

// OTAPI_Exec::FlushSentMessages
//
// Make sure to call this directly after a successful getNymboxResponse.
// (And ONLY at that time.)
//
// This empties the buffer of sent messages.
// (Harvesting any transaction numbers that are still there.)
//
// NOTE: You normally ONLY call this immediately after receiving
// a successful getNymboxResponse. It's only then that you can see which
// messages a server actually received or not -- which transactions
// it processed (success or fail) vs which transactions did NOT
// process (and thus did NOT leave any success/fail receipt in the
// nymbox.)
//
// I COULD have just flushed myself IN the getNymboxResponse code (where
// the reply is processed.) But then the developer using the OT API
// would never have the opportunity to see whether a message was
// replied to, and harvest it for himself (say, just before attempting
// a re-try, which I plan to do in the high-level Java API, which is
// why I'm coding it this way.)
//
// This way, he can do that if he wishes, THEN call this function,
// and harvesting will still occur properly, and he will also thus have
// his chance to check for his own replies to harvest before then.
// This all depends on the developer using the API being smart enough
// to call this function after a successful getNymboxResponse!
//
void OTAPI_Exec::FlushSentMessages(
    const bool& bHarvestingForRetry,
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& THE_NYMBOX) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return;
    }
    if (THE_NYMBOX.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_NYMBOX passed in!\n";
        return;
    }

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID);
    const String strLedger(THE_NYMBOX);
    Ledger theLedger(theNymID, theNymID, theNotaryID);
    if (strLedger.Exists() && theLedger.LoadContractFromString(strLedger))
        OTAPI()
            ->FlushSentMessages(
                bHarvestingForRetry, theNotaryID, theNymID, theLedger);
    else
        otErr << __FUNCTION__
              << ": Failure: Unable to load Nymbox from string:\n\n"
              << strLedger << "\n\n";
}

// Sometimes you just need a quick nap between messages.
//
void OTAPI_Exec::Sleep(const int64_t& MILLISECONDS) const
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return;
    }

    if (0 > MILLISECONDS) {
        otErr << __FUNCTION__ << ": Negative: MILLISECONDS passed in!\n";
        return;
    }

    const int64_t lMilliseconds = MILLISECONDS;

    Log::Sleep(std::chrono::milliseconds(lMilliseconds));
}

// Make sure you download your Nymbox (getNymbox) before calling this,
// so when it loads the Nymbox it will have the latest version of it.
//
// Also, call registerNym() and pass the server reply message in
// here, so that it can read theMessageNym (to sync the transaction
// numbers.)
//
bool OTAPI_Exec::ResyncNymWithServer(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& THE_MESSAGE) const
{
    bool bIsInitialized = OTAPI()->IsInitialized();
    if (!bIsInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        return false;
    }

    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return false;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return false;
    }
    if (THE_MESSAGE.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_MESSAGE passed in!\n";
        return false;
    }

    Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID);
    const String strMessage(THE_MESSAGE), strNymID(theNymID);

    Nym* pNym = OTAPI()->GetOrLoadPrivateNym(theNymID, false);
    if (nullptr == pNym) return false;
    Message theMessage;

    if (!theMessage.LoadContractFromString(strMessage)) {
        otErr << __FUNCTION__
              << ": Failed trying to load registerNymResponse() "
                 "message from string (it's a server reply.) "
                 "Contents:\n\n"
              << strMessage << "\n\n";
        return false;
    }
    if (!strNymID.Compare(theMessage.m_strNymID)) {
        otErr << __FUNCTION__
              << ": Failed. Though success loading message from string, it had "
                 "the wrong NymID. (Expected "
              << strNymID << ", but found " << theMessage.m_strNymID
              << ".) Message contents:\n\n"
              << strMessage << "\n\n";
        return false;
    }
    if (!theMessage.m_strCommand.Compare("registerNymResponse")) {
        otErr << __FUNCTION__ << ": Failed. Though success loading message "
                                 "from string, it had the wrong command type. "
                                 "(Expected registerNymResponse, but found "
              << theMessage.m_strCommand << ".) Message contents:\n\n"
              << strMessage << "\n\n";
        return false;
    }
    if (!theMessage.m_ascPayload.Exists()) {
        otErr << __FUNCTION__
              << ": Failed. Though success loading registerNymResponse() "
                 "message, the payload was empty. (Expected theMessageNym to "
                 "be there, so I could re-sync client side to server.) Message "
                 "contents:\n\n"
              << strMessage << "\n\n";
        return false;
    }
    String strMessageNym;

    if (!theMessage.m_ascPayload.GetString(strMessageNym)) {
        otErr << __FUNCTION__ << ": Failed decoding message payload in server "
                                 "reply: registerNymResponse(). (Expected "
                                 "theMessageNym to be there, so I could "
                                 "re-sync client side to server.) Message "
                                 "contents:\n\n"
              << strMessage << "\n\n";
        return false;
    }
    Nym theMessageNym;  // <====================

    if (!theMessageNym.LoadNymFromString(strMessageNym)) {
        otErr << __FUNCTION__ << ": Failed loading theMessageNym from a "
                                 "string. String contents:\n\n"
              << strMessageNym << "\n\n";
        return false;
    }
    // Based on notaryID and NymID, load the Nymbox.
    //
    Ledger theNymbox(theNymID, theNymID, theNotaryID);  // <===========

    bool bSynced = false;
    bool bLoadedNymbox =
        (theNymbox.LoadNymbox() && theNymbox.VerifyAccount(*pNym));

    if (bLoadedNymbox)
        bSynced = OTAPI()->ResyncNymWithServer(*pNym, theNymbox, theMessageNym);
    else
        otErr << __FUNCTION__
              << ": Failed while loading or verifying Nymbox for User "
              << strNymID << ", on Server " << NOTARY_ID << " \n";

    return bSynced;
}

// QUERY ASSET TYPES (server message)
//
// This way you can ask the server to confirm whether various
// instrument definitions are issued there. You must prepare the encoded
// StringMap in advance of calling this function.
//

// Returns int32_t:
// -1 means error; no message was sent.
//  0 means NO error, but also: no message was sent.
// >0 means NO error, and the message was sent, and the request number fits into
// an integer...
//  ...and in fact the requestNum IS the return value!
//  ===> In 99% of cases, this LAST option is what actually happens!!
//
int32_t OTAPI_Exec::queryInstrumentDefinitions(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ENCODED_MAP) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (ENCODED_MAP.empty()) {
        otErr << __FUNCTION__ << ": Null: ENCODED_MAP passed in!\n";
        return OT_ERROR;
    }

    Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID);
    OTASCIIArmor theArmor((String(ENCODED_MAP)));

    return OTAPI()->queryInstrumentDefinitions(theNotaryID, theNymID, theArmor);
}

// GET MESSAGE PAYLOAD
//
// This way you can retrieve the payload from any message.
// Useful, for example, for getting the encoded StringMap object
// from the queryInstrumentDefinitions and queryInstrumentDefinitionsResponse
// messages, which both
// use the m_ascPayload field to transport it.
//
std::string OTAPI_Exec::Message_GetPayload(const std::string& THE_MESSAGE) const
{
    if (THE_MESSAGE.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_MESSAGE passed in!\n";
        return "";
    }

    String strMessage(THE_MESSAGE);
    Message theMessage;

    if (!strMessage.Exists() || !theMessage.LoadContractFromString(strMessage))
        return "";

    std::string pBuf = theMessage.m_ascPayload.Get();

    return pBuf;
}

// GET MESSAGE COMMAND TYPE
//
// This way you can discover what kind of command it was.
// All server replies are pre-pended with the @ sign. For example, if
// you send a "getAccountData" message, the server reply is
// "getAccountResponse",
// and if you send "getMint" the reply is "getMintResponse", and so on.
//
std::string OTAPI_Exec::Message_GetCommand(const std::string& THE_MESSAGE) const
{
    if (THE_MESSAGE.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_MESSAGE passed in!\n";
        return "";
    }

    String strMessage(THE_MESSAGE);

    Message theMessage;

    if (!strMessage.Exists() || !theMessage.LoadContractFromString(strMessage))
        return "";

    String strOutput(theMessage.m_strCommand);

    std::string pBuf = strOutput.Get();

    return pBuf;
}

// GET MESSAGE LEDGER
//
// If you just received a server response to a transaction, and
// you want to actually iterate through the transactions in the
// response ledger for that transaction, this function will retrieve
// that ledger for you.
//
std::string OTAPI_Exec::Message_GetLedger(const std::string& THE_MESSAGE) const
{
    if (THE_MESSAGE.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_MESSAGE passed in!\n";
        return "";
    }

    String strMessage(THE_MESSAGE);

    Message theMessage;

    if (!strMessage.Exists() ||
        !theMessage.LoadContractFromString(strMessage)) {
        otOut << __FUNCTION__ << ": Unable to load message.\n";
        return "";
    }

    // It's not a transaction request or response, so the Payload wouldn't
    // contain a ledger. (Don't want to pass back whatever it DOES contain
    // in that case, now do I?)
    //
    if ((false == theMessage.m_strCommand.Compare("notarizeTransaction")) &&
        (false ==
         theMessage.m_strCommand.Compare("notarizeTransactionResponse"))) {
        otOut << __FUNCTION__
              << ": Wrong message type: " << theMessage.m_strCommand << "\n";
        return "";
    }

    // The ledger is stored in the Payload, we'll grab it into the String.
    String strOutput(theMessage.m_ascPayload);

    if (!strOutput.Exists()) {
        otOut << __FUNCTION__ << ": No ledger found on message.\n";
        return "";
    }

    std::string pBuf = strOutput.Get();

    return pBuf;
}

// GET NEW ASSET TYPE ID
//
// If you just issued a new instrument definition, you'll want to read the
// server reply and get the new instrument definition ID out of it.
// Otherwise how will you ever open accounts in that new type?
//
std::string OTAPI_Exec::Message_GetNewInstrumentDefinitionID(
    const std::string& THE_MESSAGE) const
{
    if (THE_MESSAGE.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_MESSAGE passed in!\n";
        return "";
    }

    String strMessage(THE_MESSAGE);

    Message theMessage;

    if (!strMessage.Exists() ||
        !theMessage.LoadContractFromString(strMessage)) {
        otOut << __FUNCTION__ << ": Unable to load message.\n";
        return "";
    }

    // It's not a transaction request or response, so the Payload wouldn't
    // contain a ledger. (Don't want to pass back whatever it DOES contain
    // in that case, now do I?)
    //
    if ((false ==
         theMessage.m_strCommand.Compare(
             "registerInstrumentDefinitionResponse")) &&
        (false == theMessage.m_strCommand.Compare("issueBasketResponse"))) {
        otOut << __FUNCTION__
              << ": Wrong message type: " << theMessage.m_strCommand << "\n";
        return "";
    }

    String strOutput(theMessage.m_strInstrumentDefinitionID);

    if (!strOutput.Exists()) {
        otOut << __FUNCTION__
              << ": No new instrument definition ID found on message.\n";
        return "";
    }

    std::string pBuf = strOutput.Get();

    return pBuf;
}

// GET NEW ISSUER ACCOUNT ID
//
// If you just issued a new instrument definition, you'll want to read the
// server reply and get the new issuer acct ID out of it.
// Otherwise how will you ever issue anything with it?
//
std::string OTAPI_Exec::Message_GetNewIssuerAcctID(
    const std::string& THE_MESSAGE) const
{
    if (THE_MESSAGE.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_MESSAGE passed in!\n";
        return "";
    }

    String strMessage(THE_MESSAGE);

    Message theMessage;

    if (!strMessage.Exists() ||
        !theMessage.LoadContractFromString(strMessage)) {
        otOut << __FUNCTION__ << ": Unable to load message.\n";
        return "";
    }

    // It's not an issue instrument definition response, so the m_strAcctID
    // wouldn't
    // contain an issuer account ID. (Don't want to pass back whatever it DOES
    // contain
    // in that case, now do I?)
    //
    if (!theMessage.m_strCommand.Compare(
            "registerInstrumentDefinitionResponse")) {
        otOut << __FUNCTION__
              << ": Wrong message type: " << theMessage.m_strCommand << "\n";
        return "";
    }

    String strOutput(theMessage.m_strAcctID);

    if (!strOutput.Exists()) {
        otOut << __FUNCTION__ << ": No issuer account ID found on message.\n";
        return "";
    }

    std::string pBuf = strOutput.Get();

    return pBuf;
}

// GET NEW ACCOUNT ID
//
// If you just created a new asset account, you'll want to read the
// server reply and get the new acct ID out of it.
// Otherwise how will you ever use it?
// This function allows you to get the new account ID out of the
// server reply message.
//
std::string OTAPI_Exec::Message_GetNewAcctID(
    const std::string& THE_MESSAGE) const
{
    if (THE_MESSAGE.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_MESSAGE passed in!\n";
        return "";
    }

    String strMessage(THE_MESSAGE);

    Message theMessage;

    if (!strMessage.Exists() ||
        !theMessage.LoadContractFromString(strMessage)) {
        otOut << __FUNCTION__ << ": Unable to load message.\n";
        return "";
    }

    // It's not a response to registerAccount, so the m_strAcctID wouldn't
    // contain a new account ID anyway, right? (Don't want to pass back whatever
    // it DOES contain in that case, now do I?)
    //
    if (!theMessage.m_strCommand.Compare("registerAccountResponse")) {
        otOut << __FUNCTION__
              << ": Wrong message type: " << theMessage.m_strCommand << "\n";
        return "";
    }

    String strOutput(theMessage.m_strAcctID);

    if (!strOutput.Exists()) {
        otOut << __FUNCTION__ << ": No asset account ID found on message.\n";
        return "";
    }

    std::string pBuf = strOutput.Get();

    return pBuf;
}

// GET NYMBOX HASH
//
// Some messages include a copy of the Nymbox Hash. This helps the
// server to quickly ascertain whether some messages will fail, and
// also allows the client to query the server for this information
// for syncronicity purposes.
//
std::string OTAPI_Exec::Message_GetNymboxHash(
    const std::string& THE_MESSAGE) const
{
    if (THE_MESSAGE.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_MESSAGE passed in!\n";
        return "";
    }

    String strMessage(THE_MESSAGE);

    Message theMessage;

    if (!strMessage.Exists() ||
        !theMessage.LoadContractFromString(strMessage)) {
        otOut << __FUNCTION__ << ": Unable to load message.\n";
        return "";
    }

    // So far these are the only messages that use m_strNymboxHash:
    if ((false == theMessage.m_strCommand.Compare("processNymbox")) &&
        (false == theMessage.m_strCommand.Compare("notarizeTransaction")) &&
        (false == theMessage.m_strCommand.Compare("getTransactionNumbers")) &&
        (false == theMessage.m_strCommand.Compare("processInbox")) &&
        (false == theMessage.m_strCommand.Compare("triggerClause")) &&
        (false == theMessage.m_strCommand.Compare("getNymboxResponse")) &&
        (false ==
         theMessage.m_strCommand.Compare("getRequestNumberResponse")) &&
        (false ==
         theMessage.m_strCommand.Compare("getTransactionNumbersResponse"))) {
        otOut << __FUNCTION__
              << ": Wrong message type : " << theMessage.m_strCommand
              << " \nFYI, with m_strNymboxHash : " << theMessage.m_strNymboxHash
              << "\n";
        return "";
    }

    if (!theMessage.m_strNymboxHash.Exists()) {
        otOut << __FUNCTION__
              << ": No NymboxHash found on message: " << strMessage << "\n";
        return "";
    }

    String strOutput(theMessage.m_strNymboxHash);
    std::string pBuf = strOutput.Get();

    return pBuf;
}

// GET MESSAGE SUCCESS (True or False)
//
// Returns true (1) for Success and false (0) for Failure.
//
// NEW: returns (-1) for error!
//
int32_t OTAPI_Exec::Message_GetSuccess(const std::string& THE_MESSAGE) const
{
    if (THE_MESSAGE.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_MESSAGE passed in!\n";
        return OT_ERROR;
    }

    Message theMessage;
    String strMessage(THE_MESSAGE);

    if (!strMessage.Exists()) {
        otErr << __FUNCTION__ << ": Error: THE_MESSAGE doesn't exist.\n";
        return OT_ERROR;
    }

    if (!theMessage.LoadContractFromString(strMessage)) {
        otErr << __FUNCTION__
              << ": Error: Failed loading message from string:\n\n"
              << THE_MESSAGE << "\n\n";
        return OT_ERROR;
    }
    if (true == theMessage.m_bSuccess) {
        otInfo << __FUNCTION__ << ": Server reply for RequestNum "
               << StringToLong(theMessage.m_strRequestNum.Get())
               << "(Message_GetSuccess was successful, but any transaction "
                  "inside could have failed OR succeeded. Use "
                  "Message_GetTransactionSuccess for that.)\n";  // Contents:
                                                                 // \n\n" <<
                                                                 // THE_MESSAGE
                                                                 // << "\n\n"
        return OT_TRUE;
    } else {
        otWarn << __FUNCTION__ << ": ** FYI, server reply was received, and it "
                                  "said 'No.' (Status = failed). RequestNum: "
               << StringToLong(theMessage.m_strRequestNum.Get())
               << "\n";  // Contents:\n\n" << THE_MESSAGE << "\n\n"
    }
    return OT_FALSE;
}

// GET MESSAGE "DEPTH"   (USED FOR MARKET-SPECIFIC MESSAGES.)
//
// Returns the count of relevant items, so you know whether to bother reading
// the payload.
// Returns -1 if error.
//
// The "depth" variable stores the count of items being returned.
// For example, if I call getMarketList, and 10 markets are returned,
// then depth will be set to 10. OR, if I call getNymMarketOffers, and
// the Nym has 16 offers on the various markets, then the depth will be 16.
//
// This value is important when processing server replies to market inquiries.
// If the depth is 0, then you are done. End. BUT! if it contains a number, such
// as 10,
// then that means you will want to next READ those 10 markets (or offers, or
// trades, etc)
// out of the server reply's payload.
//
// Whereas if success is TRUE, but depth is 0, that means while the message
// processed
// successfully, the list was simply empty (and thus it would be a waste of time
// trying
// to unpack the non-existent, empty list of data items from the payload of your
// successful
// reply.)
//
int32_t OTAPI_Exec::Message_GetDepth(const std::string& THE_MESSAGE) const
{
    if (THE_MESSAGE.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_MESSAGE passed in!\n";
        return OT_ERROR;
    }

    String strMessage(THE_MESSAGE);

    Message theMessage;

    if (!strMessage.Exists() || !theMessage.LoadContractFromString(strMessage))
        return OT_ERROR;

    return static_cast<int32_t>(theMessage.m_lDepth);
}

// GET MESSAGE TRANSACTION "IS CANCELLED" (True or False)
//
// Returns true (1) for Success and false (0) for Failure.
//         also returns (-1) for Error
//
int32_t OTAPI_Exec::Message_IsTransactionCanceled(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_MESSAGE) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return OT_ERROR;
    }
    if (THE_MESSAGE.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_MESSAGE passed in!\n";
        return OT_ERROR;
    }

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theAccountID(ACCOUNT_ID);

    String strMessage(THE_MESSAGE);

    Message theMessage;

    if (!strMessage.Exists() ||
        !theMessage.LoadContractFromString(strMessage)) {
        otOut << __FUNCTION__ << ": Unable to load message.\n";
        return OT_ERROR;
    }

    // It's not a transaction request or response, so the Payload wouldn't
    // contain a ledger. (Don't want to pass back whatever it DOES contain
    // in that case, now do I?)
    //
    if ((false ==
         theMessage.m_strCommand.Compare("notarizeTransactionResponse")) &&
        (false == theMessage.m_strCommand.Compare("processInboxResponse")) &&
        (false == theMessage.m_strCommand.Compare("processNymboxResponse"))) {
        otOut << __FUNCTION__
              << ": Wrong message type: " << theMessage.m_strCommand << "\n";
        return OT_ERROR;
    }

    // The ledger is stored in the Payload, we'll grab it into the String.
    String strLedger(theMessage.m_ascPayload);

    if (!strLedger.Exists()) {
        otOut << __FUNCTION__ << ": No ledger found on message.\n";
        return OT_ERROR;
    }

    Ledger theLedger(theNymID, theAccountID, theNotaryID);

    if (!theLedger.LoadContractFromString(strLedger)) {
        String strAcctID(theAccountID);
        otErr << __FUNCTION__
              << ": Error loading ledger from string. Acct ID: " << strAcctID
              << "\n";
        return OT_ERROR;
    }

    // At this point, I know theLedger loaded successfully.

    if (theLedger.GetTransactionCount() <= 0) {
        otErr << __FUNCTION__ << ": bad count in message ledger: "
              << theLedger.GetTransactionCount() << "\n";
        return OT_ERROR;  // out of bounds. I'm saving from an OT_ASSERT_MSG()
                          // happening here. (Maybe I shouldn't.)
    }

    OTTransaction* pTransaction = theLedger.GetTransactionByIndex(
        0);  // Right now this is a defacto standard. (only 1 transaction per
             // message ledger, excepting process inbox.)

    if (nullptr == pTransaction) {
        otErr << __FUNCTION__
              << ": good index but uncovered \"\" pointer: " << 0 << "\n";
        return OT_ERROR;  // Weird.
    }

    // At this point, I actually have the transaction pointer, so let's return
    // its 'canceled' status
    //
    if (pTransaction->IsCancelled()) return OT_TRUE;

    return OT_FALSE;
}

// GET MESSAGE TRANSACTION SUCCESS (True or False)
//
// Returns true (1) for Success and false (0) for Failure.
//         also returns (-1) for Error
//
int32_t OTAPI_Exec::Message_GetTransactionSuccess(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_MESSAGE) const
{
    if (NOTARY_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        return OT_ERROR;
    }
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        return OT_ERROR;
    }
    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        return OT_ERROR;
    }
    if (THE_MESSAGE.empty()) {
        otErr << __FUNCTION__ << ": Null: THE_MESSAGE passed in!\n";
        return OT_ERROR;
    }

    const Identifier theNotaryID(NOTARY_ID), theNymID(NYM_ID),
        theAccountID(ACCOUNT_ID);

    String strMessage(THE_MESSAGE);

    Message theMessage;

    if (!strMessage.Exists() ||
        !theMessage.LoadContractFromString(strMessage)) {
        otOut << __FUNCTION__ << ": Unable to load message.\n";
        return OT_ERROR;
    }

    // It's not a transaction request or response, so the Payload wouldn't
    // contain a ledger. (Don't want to pass back whatever it DOES contain
    // in that case, now do I?)
    //
    if ((false ==
         theMessage.m_strCommand.Compare("notarizeTransactionResponse")) &&
        (false == theMessage.m_strCommand.Compare("processInboxResponse")) &&
        (false == theMessage.m_strCommand.Compare("processNymboxResponse"))) {
        otOut << __FUNCTION__
              << ": Wrong message type: " << theMessage.m_strCommand << "\n";
        return OT_ERROR;
    }

    // The ledger is stored in the Payload, we'll grab it into the String.
    String strLedger(theMessage.m_ascPayload);

    if (!strLedger.Exists()) {
        otOut << __FUNCTION__ << ": No ledger found on message.\n";
        return OT_ERROR;
    }

    Ledger theLedger(theNymID, theAccountID, theNotaryID);

    if (!theLedger.LoadContractFromString(strLedger)) {
        String strAcctID(theAccountID);
        otErr << __FUNCTION__
              << ": Error loading ledger from string. Acct ID: " << strAcctID
              << "\n";
        return OT_ERROR;
    }

    // At this point, I know theLedger loaded successfully.

    if (theLedger.GetTransactionCount() <= 0) {
        otErr << __FUNCTION__ << ": bad count in message ledger: "
              << theLedger.GetTransactionCount() << "\n";
        return OT_ERROR;  // out of bounds. I'm saving from an OT_ASSERT_MSG()
                          // happening here. (Maybe I shouldn't.)
    }

    OTTransaction* pTransaction = theLedger.GetTransactionByIndex(
        0);  // Right now this is a defacto standard. (only 1 transaction per
             // message ledger, excepting process inbox.)

    if (nullptr == pTransaction) {
        otErr << __FUNCTION__
              << ": good index but uncovered \"\" pointer: " << 0 << "\n";
        return OT_ERROR;  // Weird.
    }

    // At this point, I actually have the transaction pointer, so let's return
    // its success status
    //
    if (pTransaction->GetSuccess())
        return OT_TRUE;
    else {
        const int64_t lRequestNum =
            StringToLong(theMessage.m_strRequestNum.Get());
        const int64_t lTransactionNum = pTransaction->GetTransactionNum();

        otWarn << __FUNCTION__
               << ": ** FYI, server reply was received, and it said 'No.' "
                  "(Status = failed). RequestNum: "
               << lRequestNum << ", TransNum: " << lTransactionNum
               << "\n";  // Contents: \n\n" << THE_MESSAGE << "\n\n"
    }

    return OT_FALSE;
}

std::string OTAPI_Exec::ContactAttributeName(
    const proto::ContactItemAttribute type,
    std::string lang)
{
    return App::Me().Identity().ContactAttributeName(type, lang);
}

std::set<proto::ContactSectionName> OTAPI_Exec::ContactSectionList(
    const std::uint32_t version)
{
    return App::Me().Identity().ContactSectionList(version);
}

std::string OTAPI_Exec::ContactSectionName(
    const proto::ContactSectionName section,
    std::string lang)
{
    return App::Me().Identity().ContactSectionName(section, lang);
}

std::set<proto::ContactItemType> OTAPI_Exec::ContactSectionTypeList(
    const proto::ContactSectionName section,
    const std::uint32_t version)
{
    return App::Me().Identity().ContactSectionTypeList(section, version);
}

std::string OTAPI_Exec::ContactTypeName(
    const proto::ContactItemType type,
    std::string lang)
{
    return App::Me().Identity().ContactTypeName(type, lang);
}

proto::ContactItemType OTAPI_Exec::ReciprocalRelationship(
    const proto::ContactItemType relationship)
{
    return App::Me().Identity().ReciprocalRelationship(relationship);
}

std::string OTAPI_Exec::Wallet_GetSeed() const
{
    const bool bIsInitialized = OTAPI()->IsInitialized();

    if (!bIsInitialized) {
        otErr << __FUNCTION__ << ": Not initialized; call OT_API::Init first."
              << std::endl;
        return "";
    }

    return OTAPI()->Wallet_GetSeed();
}

std::string OTAPI_Exec::Wallet_GetPassphrase() const
{
    const bool bIsInitialized = OTAPI()->IsInitialized();

    if (!bIsInitialized) {
        otErr << __FUNCTION__ << ": Not initialized; call OT_API::Init first."
              << std::endl;
        return "";
    }

    return OTAPI()->Wallet_GetPhrase();
}

std::string OTAPI_Exec::Wallet_GetWords() const
{
    const bool bIsInitialized = OTAPI()->IsInitialized();

    if (!bIsInitialized) {
        otErr << __FUNCTION__ << ": Not initialized; call OT_API::Init first."
              << std::endl;
        return "";
    }

    return OTAPI()->Wallet_GetWords();
}

bool OTAPI_Exec::AddClaim(
    const std::string& nymID,
    const std::uint32_t& section,
    const std::uint32_t& type,
    const std::string& value,
    const bool active,
    const bool primary,
    const int64_t start,
    const int64_t end) const
{
    std::unique_ptr<proto::ContactItem> claim(new proto::ContactItem);

    if (!claim) { return false; }

    claim->set_version(1);
    claim->set_type(static_cast<proto::ContactItemType>(type));
    claim->set_value(value);
    claim->set_start(start);
    claim->set_end(end);

    if (active) {
        claim->add_attribute(proto::CITEMATTR_ACTIVE);
    }

    if (primary) {
        claim->add_attribute(proto::CITEMATTR_PRIMARY);
    }

    return SetClaim(nymID, section,  proto::ProtoAsString(*claim));
}
}  // namespace opentxs
