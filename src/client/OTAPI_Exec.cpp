// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/client/OTAPI_Exec.hpp"

#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Identity.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/api/Wallet.hpp"
#if OT_CASH
#include "opentxs/cash/Purse.hpp"
#endif  // OT_CASH
#include "opentxs/client/Helpers.hpp"
#include "opentxs/client/NymData.hpp"
#include "opentxs/client/OTWallet.hpp"
#include "opentxs/client/OT_API.hpp"
#include "opentxs/cash/Mint.hpp"
#include "opentxs/cash/Token.hpp"
#include "opentxs/consensus/ServerContext.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/core/contract/basket/Basket.hpp"
#include "opentxs/core/contract/peer/PeerObject.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/cron/OTCronItem.hpp"
#include "opentxs/core/crypto/CredentialSet.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/OTEnvelope.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/recurring/OTPaymentPlan.hpp"
#include "opentxs/core/script/OTAgent.hpp"
#include "opentxs/core/script/OTBylaw.hpp"
#include "opentxs/core/script/OTClause.hpp"
#include "opentxs/core/script/OTParty.hpp"
#include "opentxs/core/script/OTPartyAccount.hpp"
#include "opentxs/core/script/OTScriptable.hpp"
#include "opentxs/core/script/OTVariable.hpp"
#include "opentxs/core/transaction/Helpers.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/OTPaths.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Item.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/NumList.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/NymIDSource.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/LegacySymmetric.hpp"
#include "opentxs/ext/OTPayment.hpp"
#include "opentxs/Types.hpp"

#include <cstdint>
#include <memory>
#include <sstream>
#include <string>

#define OT_METHOD "opentxs::OTAPI_Exec::"

namespace opentxs
{

#ifndef OT_FALSE
const std::int32_t OT_FALSE = 0;
#endif

#ifndef OT_TRUE
const std::int32_t OT_TRUE = 1;
#endif

#ifndef OT_ERROR
const std::int32_t OT_ERROR = (-1);
#endif

OTAPI_Exec::OTAPI_Exec(
    const api::Core& api,
    const api::client::Activity& activity,
    const api::client::Contacts& contacts,
    const api::network::ZMQ& zeromq,
    const api::Identity& identity,
    const OT_API& otapi,
    const ContextLockCallback& lockCallback)
    : api_(api)
    , activity_(activity)
    , contacts_(contacts)
    , zeromq_(zeromq)
    , identity_(identity)
    , ot_api_(otapi)
    , lock_callback_(lockCallback)
{
    // WARNING: do not access api_.Wallet() during construction
}

void OTAPI_Exec::SetAppBinaryFolder(const std::string& strFolder)
{
    OTPaths::SetAppBinaryFolder(String::Factory(strFolder.c_str()));
}

void OTAPI_Exec::SetHomeFolder(const std::string& strFolder)
{
    OTPaths::SetHomeFolder(String::Factory(strFolder.c_str()));
}

std::int64_t OTAPI_Exec::StringToLong(const std::string& strNumber) const
{
    return String::StringToLong(strNumber);
}

std::string OTAPI_Exec::LongToString(const std::int64_t& lNumber) const
{
    return String::LongToString(lNumber);
}

std::uint64_t OTAPI_Exec::StringToUlong(const std::string& strNumber) const
{
    return String::StringToUlong(strNumber);
}

std::string OTAPI_Exec::UlongToString(const std::uint64_t& lNumber) const
{
    return String::UlongToString(lNumber);
}

bool OTAPI_Exec::CheckSetConfigSection(
    const std::string& strSection,
    const std::string& strComment) const
{
    bool b_isNewSection = false;

    const bool bSuccess = api_.Config().CheckSetSection(
        String::Factory(strSection.c_str()),
        String::Factory(strComment.c_str()),
        b_isNewSection);
    if (bSuccess && b_isNewSection) {
        if (!api_.Config().Save()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error: Unable to save updated config file.")
                .Flush();
            OT_FAIL;
        }
    }

    return bSuccess;
}

bool OTAPI_Exec::SetConfig_str(
    const std::string& strSection,
    const std::string& strKey,
    const std::string& strValue) const
{
    bool b_isNew = false;
    const bool bSuccess = api_.Config().Set_str(
        String::Factory(strSection.c_str()),
        String::Factory(strKey.c_str()),
        String::Factory(strValue.c_str()),
        b_isNew);

    if (bSuccess && b_isNew) {
        if (!api_.Config().Save()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error: Unable to save updated config file.")
                .Flush();
            OT_FAIL;
        }
    }

    return bSuccess;
}

bool OTAPI_Exec::SetConfig_long(
    const std::string& strSection,
    const std::string& strKey,
    const std::int64_t& lValue) const
{
    bool b_isNew = false;
    const bool bSuccess = api_.Config().Set_long(
        String::Factory(strSection.c_str()),
        String::Factory(strKey.c_str()),
        lValue,
        b_isNew);

    if (bSuccess && b_isNew) {
        if (!api_.Config().Save()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error: Unable to save updated config file.")
                .Flush();
            OT_FAIL;
        }
    }

    return bSuccess;
}

bool OTAPI_Exec::SetConfig_bool(
    const std::string& strSection,
    const std::string& strKey,
    const bool bValue) const
{
    bool b_isNew = false;
    const bool bSuccess = api_.Config().Set_bool(
        String::Factory(strSection.c_str()),
        String::Factory(strKey.c_str()),
        bValue,
        b_isNew);

    if (bSuccess && b_isNew) {
        if (!api_.Config().Save()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error: Unable to save updated config file.")
                .Flush();
            OT_FAIL;
        }
    }

    return bSuccess;
}

std::string OTAPI_Exec::GetConfig_str(
    const std::string& strSection,
    const std::string& strKey) const
{
    auto strOutput = String::Factory();
    bool bKeyExists = false;
    const bool bSuccess = api_.Config().Check_str(
        String::Factory(strSection.c_str()),
        String::Factory(strKey.c_str()),
        strOutput,
        bKeyExists);
    std::string str_result = "";

    if (bSuccess && bKeyExists) str_result = strOutput->Get();

    return str_result;
}

std::int64_t OTAPI_Exec::GetConfig_long(
    const std::string& strSection,
    const std::string& strKey) const
{
    std::int64_t lOutput = 0;
    bool bKeyExists = false;
    const bool bSuccess = api_.Config().Check_long(
        String::Factory(strSection.c_str()),
        String::Factory(strKey.c_str()),
        lOutput,
        bKeyExists);

    if (bSuccess && bKeyExists) return lOutput;

    return 0;
}

bool OTAPI_Exec::GetConfig_bool(
    const std::string& strSection,
    const std::string& strKey) const
{
    bool bOutput = false;
    bool bKeyExists = false;
    const bool bSuccess = api_.Config().Check_bool(
        String::Factory(strSection.c_str()),
        String::Factory(strKey.c_str()),
        bOutput,
        bKeyExists);

    if (bSuccess && bKeyExists) return bOutput;

    return false;
}

bool OTAPI_Exec::SetWallet(const std::string& strWalletFilename) const
{
    auto sWalletFilename = String::Factory(strWalletFilename);

    if (sWalletFilename->Exists()) {

        return ot_api_.SetWalletFilename(sWalletFilename);
    }

    LogOutput(OT_METHOD)(__FUNCTION__)(": Error: Wallet Filename is Null!")
        .Flush();

    return false;
}

/**
WALLET EXISTS

Just Checks if the m_pWallet pointer is not null.

WalletExists();

*/
bool OTAPI_Exec::WalletExists() const { return ot_api_.WalletExists(); }

bool OTAPI_Exec::LoadWallet() const { return ot_api_.LoadWallet(); }

bool OTAPI_Exec::SwitchWallet() const { return ot_api_.LoadWallet(); }

// Returns new list if ALL strNumbers are successfully added to strNumList.
// Otherwise returns "" and doesn't change anything.
//
std::string OTAPI_Exec::NumList_Add(
    const std::string& strNumList,
    const std::string& strNumbers) const
{
    if (strNumbers.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: strNumbers passed in!")
            .Flush();
        return {};
    }

    // strNumList can be null, (create a new one).
    NumList theList, theNewNumbers(strNumbers);

    if ("" != strNumList) {
        const auto otstrNumList = String::Factory(strNumList);
        theList.Add(otstrNumList);
    }

    const bool bAdded = ot_api_.NumList_Add(theList, theNewNumbers);
    auto strOutput = String::Factory();

    if (bAdded && theList.Output(strOutput)) { return strOutput->Get(); }

    return {};
}

// Returns new list if ALL strNumbers are successfully removed from strNumList.
// Otherwise returns "" and doesn't change anything.
//
std::string OTAPI_Exec::NumList_Remove(
    const std::string& strNumList,
    const std::string& strNumbers) const
{
    if (strNumList.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: strNumList passed in!")
            .Flush();
        return {};
    }
    if (strNumbers.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: strNumbers passed in!")
            .Flush();
        return {};
    }

    NumList theList(strNumList), theNewNumbers(strNumbers);

    const bool bRemoved = ot_api_.NumList_Remove(theList, theNewNumbers);

    auto strOutput = String::Factory();

    if (bRemoved && theList.Output(strOutput)) { return strOutput->Get(); }

    return {};
}

// Verifies strNumbers as a SUBSET of strNumList.
//
bool OTAPI_Exec::NumList_VerifyQuery(
    const std::string& strNumList,
    const std::string& strNumbers) const
{
    if (strNumList.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: strNumList passed in!")
            .Flush();
        return false;
    }
    if (strNumbers.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: strNumbers passed in!")
            .Flush();
        return false;
    }

    NumList theList(strNumList), theNewNumbers(strNumbers);

    return ot_api_.NumList_VerifyQuery(theList, theNewNumbers);
}

// Verifies COUNT and CONTENT but NOT ORDER.
//
bool OTAPI_Exec::NumList_VerifyAll(
    const std::string& strNumList,
    const std::string& strNumbers) const
{
    if (strNumList.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: strNumList passed in!")
            .Flush();
        return false;
    }
    if (strNumbers.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: strNumbers passed in!")
            .Flush();
        return false;
    }
    NumList theList(strNumList), theNewNumbers(strNumbers);

    return ot_api_.NumList_VerifyAll(theList, theNewNumbers);
}

std::int32_t OTAPI_Exec::NumList_Count(const std::string& strNumList) const
{
    if (strNumList.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: strNumList passed in!")
            .Flush();
        return OT_ERROR;
    }

    NumList theList(strNumList);

    return ot_api_.NumList_Count(theList);
}

bool OTAPI_Exec::IsValidID(const std::string& strPurportedID) const
{
    return Identifier::Validate(strPurportedID);
}

std::string OTAPI_Exec::NymIDFromPaymentCode(
    const std::string& paymentCode) const
{
    return ot_api_.NymIDFromPaymentCode(paymentCode);
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
    const std::int32_t& nKeySize,  // must be 1024, 2048, 4096, or 8192
    __attribute__((unused))
    const std::string& NYM_ID_SOURCE) const  // Can be empty.
{
    if (0 >= nKeySize) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Keysize is 0 or less, will fail! Try 1024.")
            .Flush();
        return {};
    }
    // ---------------------------------------
    switch (nKeySize) {
        case 1024:
        case 2048:
        case 4096:
        case 8192:
            break;
        default:
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failure: nKeySize must be one of: "
                "1024, 2048, 4096, 8192. (")(nKeySize)(" was passed...).")
                .Flush();
            return {};
    }

    std::shared_ptr<NymParameters> nymParameters;
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    nymParameters = std::make_shared<NymParameters>(nKeySize);
#else
    nymParameters = std::make_shared<NymParameters>(proto::CREDTYPE_LEGACY);
#endif

    ConstNym pNym = api_.Wallet().Nym(*nymParameters);

    if (false == bool(pNym))  // Creation failed.
    {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Failed trying to create Nym.")
            .Flush();
        return {};
    }
    // -----------------------------------------------------}
    return pNym->ID().str();  // We're returning the new Nym ID.
}

std::string OTAPI_Exec::CreateNymHD(
    const proto::ContactItemType type,
    const std::string& name,
    const std::string& fingerprint,
    const std::int32_t index) const
{
#if OT_CRYPTO_SUPPORTED_KEY_HD
    switch (type) {
        case proto::CITEMTYPE_INDIVIDUAL:
        case proto::CITEMTYPE_ORGANIZATION:
        case proto::CITEMTYPE_BUSINESS:
        case proto::CITEMTYPE_GOVERNMENT:
        case proto::CITEMTYPE_SERVER:
        case proto::CITEMTYPE_BOT: {
            break;
        }
        default: {
            LogNormal(OT_METHOD)(__FUNCTION__)(": Invalid nym type.").Flush();

            return {};
        }
    }

    OTWallet* pWallet = ot_api_.GetWallet(__FUNCTION__);

    if (nullptr == pWallet) { return {}; }

    NymParameters nymParameters(proto::CREDTYPE_HD);

    if (0 < fingerprint.size()) { nymParameters.SetSeed(fingerprint); }

    if (0 > index) {
        nymParameters.SetUseAutoIndex(true);
    } else {
        nymParameters.SetNym(index);
    }

    auto nym = api_.Wallet().Nym(nymParameters, type, name);

    if (nullptr == nym) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Failed trying to create Nym.")
            .Flush();

        return {};
    }

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    auto code = api_.Factory().PaymentCode(nym->PaymentCode());
#endif
    contacts_.NewContact(
        name,
        nym->ID()
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
            ,
        code
#endif
    );

    return nym->ID().str();
#else
    LogNormal(OT_METHOD)(__FUNCTION__)(": No support for HD key derivation.")
        .Flush();

    return {};
#endif
}

std::string OTAPI_Exec::GetNym_ActiveCronItemIDs(
    const std::string& NYM_ID,
    const std::string& NOTARY_ID) const
{
    rLock lock(lock_callback_({NYM_ID, NOTARY_ID}));

    if (NYM_ID.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": nullptr NYM_ID passed in!")
            .Flush();
        return {};
    }
    if (NOTARY_ID.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": nullptr NOTARY_ID passed in!")
            .Flush();
        return {};
    }
    const auto nymId = Identifier::Factory(NYM_ID),
               notaryID = Identifier::Factory(NOTARY_ID);
    NumList numlist;
    std::string str_return;

    if (OTCronItem::GetActiveCronTransNums(
            numlist, api_.DataFolder(), nymId, notaryID)) {
        auto strOutput = String::Factory();
        numlist.Output(strOutput);
        str_return = strOutput->Get();
    }

    return str_return;
}

std::string OTAPI_Exec::GetActiveCronItem(
    const std::string& NOTARY_ID,
    std::int64_t lTransNum) const
{
    Lock lock(lock_);

    if (NOTARY_ID.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": nullptr NOTARY_ID passed in!")
            .Flush();
        return {};
    }
    if (0 > lTransNum) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Negative: lTransNum passed in!")
            .Flush();
        return {};
    }
    const auto notaryID = Identifier::Factory(NOTARY_ID);
    std::string str_return;
    const std::int64_t lTransactionNum = lTransNum;

    const auto pCronItem{
        OTCronItem::LoadActiveCronReceipt(api_, lTransactionNum, notaryID)};
    if (false != bool(pCronItem)) {
        const auto strCronItem = String::Factory(*pCronItem);

        str_return = strCronItem->Get();
    }
    return str_return;
}

std::string OTAPI_Exec::GetNym_SourceForID(const std::string& NYM_ID) const
{
    if (NYM_ID.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": nullptr NYM_ID passed in!")
            .Flush();
        return {};
    }
    OTPasswordData thePWData(OT_PW_DISPLAY);
    auto nym_id = Identifier::Factory(NYM_ID);
    auto pNym = api_.Wallet().Nym(nym_id);
    if (false == bool(pNym)) return {};
    const std::string str_return(pNym->Source().asString()->Get());
    return str_return;
}

std::string OTAPI_Exec::GetNym_Description(const std::string& NYM_ID) const
{
    if (NYM_ID.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": nullptr NYM_ID passed in!")
            .Flush();
        return {};
    }
    OTPasswordData thePWData(OT_PW_DISPLAY);
    auto nym_id = Identifier::Factory(NYM_ID);
    auto pNym = api_.Wallet().Nym(nym_id);
    if (false == bool(pNym)) return {};
    const std::string str_return(pNym->GetDescription().Get());
    return str_return;
}

std::string OTAPI_Exec::GetNym_MasterCredentialContents(
    const std::string& NYM_ID,
    const std::string& CREDENTIAL_ID) const
{
    if (NYM_ID.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": nullptr NYM_ID passed in!")
            .Flush();
        return {};
    }
    OTPasswordData thePWData(OT_PW_DISPLAY);
    auto nym_id = Identifier::Factory(NYM_ID);
    auto pNym = api_.Wallet().Nym(nym_id);
    if (false == bool(pNym)) return {};

    auto serialized = pNym->MasterCredentialContents(CREDENTIAL_ID);

    if (serialized) { return proto::ProtoAsString(*serialized); }

    return {};
}

std::string OTAPI_Exec::GetNym_RevokedCredContents(
    const std::string& NYM_ID,
    const std::string& CREDENTIAL_ID) const
{
    if (NYM_ID.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": nullptr NYM_ID passed in!")
            .Flush();
        return {};
    }
    OTPasswordData thePWData(OT_PW_DISPLAY);
    auto nym_id = Identifier::Factory(NYM_ID);
    auto pNym = api_.Wallet().Nym(nym_id);
    if (false == bool(pNym)) { return {}; }

    auto serialized = pNym->RevokedCredentialContents(CREDENTIAL_ID);

    if (serialized) { return proto::ProtoAsString(*serialized); }

    return {};
}

std::string OTAPI_Exec::GetNym_ChildCredentialContents(
    const std::string& NYM_ID,
    const std::string& MASTER_CRED_ID,
    const std::string& SUB_CRED_ID) const
{
    if (NYM_ID.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": nullptr NYM_ID passed in!")
            .Flush();
        return {};
    }
    if (MASTER_CRED_ID.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": nullptr MASTER_CRED_ID passed in!")
            .Flush();
        return {};
    }
    if (SUB_CRED_ID.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": nullptr SUB_CRED_ID passed in!")
            .Flush();
        return {};
    }
    OTPasswordData thePWData(OT_PW_DISPLAY);
    auto nym_id = Identifier::Factory(NYM_ID);
    auto pNym = api_.Wallet().Nym(nym_id);
    if (false == bool(pNym)) return {};

    auto serialized =
        pNym->ChildCredentialContents(MASTER_CRED_ID, SUB_CRED_ID);

    if (serialized) { return proto::ProtoAsString(*serialized); }

    return {};
}

bool OTAPI_Exec::RevokeChildCredential(
    const std::string& NYM_ID,
    const std::string& MASTER_CRED_ID,
    const std::string& SUB_CRED_ID) const
{
    if (NYM_ID.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": nullptr NYM_ID passed in!")
            .Flush();
        return false;
    }
    if (MASTER_CRED_ID.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": nullptr MASTER_CRED_ID passed in!")
            .Flush();
        return false;
    }
    if (SUB_CRED_ID.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": nullptr SUB_CRED_ID passed in!")
            .Flush();
        return false;
    }
    OTPasswordData thePWData(OT_PW_DISPLAY);
    auto nym_id = Identifier::Factory(NYM_ID);
    auto pNym = api_.Wallet().Nym(nym_id);

    if (false == bool(pNym)) { return false; }

    const auto strCredID = String::Factory(MASTER_CRED_ID);

    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": TODO: REVOKING IS NOT YET CODED. ADD FUNCTION CALL "
        "HERE TO REVOKE SUB-CREDENTIAL!")
        .Flush();

    /* Revokation is not implemented yet
    CredentialSet* pCredential = pNym->GetMasterCredential(strCredID);

    if (nullptr == pCredential)
        otOut << OT_METHOD << __FUNCTION__ << ": Sorry, (Nym " << NYM_ID
              << ") no master credential found with the ID : " << strCredID
              << "\n";
    else // Found the master credential...
    {
        const String strSubID(SUB_CRED_ID);
        const Credential* pSub = pCredential->GetChildCredential(strSubID);

        if (nullptr == pSub)
            otOut << OT_METHOD << __FUNCTION__ << ": Found master credential ("
<< strCredID
                  << "), but unable to "
                     "find child credential with ID: " << strSubID << "\n";
        else {

            // TODO: Okay we found master AND child credential. Now let's revoke
            // it...
            //

            otErr << "\n\n\nSwigWrap::" << __FUNCTION__
                  << ": TODO: REVOKING IS NOT YET CODED. ADD FUNCTION CALL "
                     "HERE TO REVOKE SUB-CREDENTIAL!\n\n\n";

//          return true;
        }
    }*/
    return false;
}

/// Base64-encodes the result. Otherwise identical to GetContactData.
std::string OTAPI_Exec::GetContactData_Base64(const std::string& NYM_ID) const
{
    std::string str_result = GetContactData(NYM_ID);

    if (str_result.empty()) return {};

    return api_.Crypto().Encode().DataEncode(str_result);
}

/// Returns a serialized protobuf (binary) stored in a std::string.
/// (Courtesy of Google's Protobuf.)
std::string OTAPI_Exec::GetContactData(const std::string& NYM_ID) const
{
    auto claims = ot_api_.GetContactData(Identifier::Factory(NYM_ID));

    if (!claims) { return {}; }

    return proto::ProtoAsString(*claims);
}

std::string OTAPI_Exec::DumpContactData(const std::string& NYM_ID) const
{
    auto claims = ot_api_.GetContactData(Identifier::Factory(NYM_ID));

    if (!claims) { return {}; }

    return ContactData::PrintContactData(*claims);
}

/// Expects a Base64-encoded data parameter.
/// Otherwise identical to SetContactData.
bool OTAPI_Exec::SetContactData_Base64(
    const std::string& NYM_ID,
    const std::string& THE_DATA) const
{
    if (THE_DATA.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unexpectedly got a blank data parameter. "
            "Should assert here. (The UI developer has a "
            "bug in his UI code, if you are seeing this "
            "log).")
            .Flush();
        return false;
    }

    std::string str_decoded = api_.Crypto().Encode().DataDecode(THE_DATA);

    return SetContactData(NYM_ID, str_decoded);
}

/// For the data parameter, expects a std::string containing
/// binary data. (A serialized protobuf.)
bool OTAPI_Exec::SetContactData(
    const std::string& NYM_ID,
    const std::string& THE_DATA) const
{
    if (NYM_ID.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": nullptr NYM_ID passed in!")
            .Flush();
        return false;
    }
    if (THE_DATA.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": nullptr THE_DATA passed in!")
            .Flush();
        return false;
    }
    opentxs::OTIdentifier nymID = Identifier::Factory(NYM_ID);
    NymData pNym = api_.Wallet().mutable_Nym(nymID);

    // ------------------------------
    auto contactData =
        proto::StringToProto<proto::ContactData>(String::Factory(THE_DATA));
    // ------------------------------
    if (pNym.SetContactData(contactData)) {
        return bool(api_.Wallet().Nym(pNym.asPublicNym()));
    }

    return false;
}

/// Identical to SetClaim except the claim param is expected
/// to be base64-encoded (and must be decoded here.)
bool OTAPI_Exec::SetClaim_Base64(
    const std::string& nymID,
    const std::uint32_t& section,
    const std::string& claim) const
{
    if (claim.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unexpectedly got a blank claim parameter. "
            "Should assert here. (The UI developer has a "
            "bug in his UI code, if you are seeing this "
            "log).")
            .Flush();
        return false;
    }

    std::string str_decoded = api_.Crypto().Encode().DataDecode(claim);

    return SetClaim(nymID, section, str_decoded);
}

bool OTAPI_Exec::SetClaim(
    const std::string& nymID,
    const std::uint32_t& section,
    const std::string& claim) const
{
    if (nymID.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": nullptr nymID passed in!")
            .Flush();
        return false;
    }
    NymData pNym = api_.Wallet().mutable_Nym(Identifier::Factory(nymID));

    // ------------------------------
    const auto item = proto::DataToProto<proto::ContactItem>(
        Data::Factory(claim.c_str(), claim.length()));
    std::set<std::uint32_t> attribute;

    for (const auto& it : item.attribute()) { attribute.insert(it); }

    const Claim input{item.id(),
                      section,
                      item.type(),
                      item.value(),
                      item.start(),
                      item.end(),
                      attribute};
    pNym.AddClaim(input);

    return true;
}

bool OTAPI_Exec::DeleteClaim(
    const std::string& nymID,
    const std::string& claimID) const
{
    if (nymID.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": nullptr nymID passed in!")
            .Flush();
        return false;
    }
    NymData pNym = api_.Wallet().mutable_Nym(Identifier::Factory(nymID));

    return pNym.DeleteClaim(Identifier::Factory(claimID));
}

/// Identical to GetVerificationSet except it returns
/// base64-encoded data.
std::string OTAPI_Exec::GetVerificationSet_Base64(
    const std::string& nymID) const
{
    std::string str_result = GetVerificationSet(nymID);

    if (str_result.empty()) return {};

    return api_.Crypto().Encode().DataEncode(str_result);
}

std::string OTAPI_Exec::GetVerificationSet(const std::string& nymID) const
{
    const auto pNym = api_.Wallet().Nym(Identifier::Factory(nymID));

    if (!pNym) { return {}; }

    auto verifications = identity_.Verifications(*pNym);

    if (verifications) { return proto::ProtoAsString(*verifications); }

    return {};
}

/// Identical to SetVerification except it returns
/// base64-encoded data.
std::string OTAPI_Exec::SetVerification_Base64(
    bool& changed,
    const std::string& onNym,
    const std::string& claimantNymID,
    const std::string& claimID,
    const ClaimPolarity polarity,
    const std::int64_t start,
    const std::int64_t end) const
{
    std::string str_result = SetVerification(
        changed, onNym, claimantNymID, claimID, polarity, start, end);
    if (str_result.empty()) return {};

    return api_.Crypto().Encode().DataEncode(str_result);
}

std::string OTAPI_Exec::SetVerification(
    bool& changed,
    const std::string& onNym,
    const std::string& claimantNymID,
    const std::string& claimID,
    const ClaimPolarity polarity,
    const std::int64_t start,
    const std::int64_t end) const
{
    if (onNym.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Empty onNym passed in!").Flush();
        return {};
    }
    NymData pNym = api_.Wallet().mutable_Nym(Identifier::Factory(onNym));

    // ------------------------------
    auto verifications = identity_.Verify(
        pNym, changed, claimantNymID, claimID, polarity, start, end);

    if (verifications) { return proto::ProtoAsString(*verifications); }

    return {};
}

std::string OTAPI_Exec::GetSignerNymID(const std::string& str_Contract) const
{
    if (str_Contract.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: str_Contract passed in!")
            .Flush();
        return {};
    }
    std::string str_Trim(str_Contract);
    std::string str_Trim2 = String::trim(str_Trim);
    auto strContract = String::Factory(str_Trim2.c_str());

    if (strContract->GetLength() < 2) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Empty contract passed in!")
            .Flush();
        return {};
    }

    auto pContract{api_.Factory().Contract(strContract)};

    if (false != bool(pContract)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to instantiate contract.")
            .Flush();

        return {};
    }

    ConstNym pNym = pContract->GetContractPublicNym();

    if (false == bool(pNym)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to retrieve signer nym from contract.")
            .Flush();

        return {};
    }
    //-----------------------------------
    return pNym->ID().str();
}

std::string OTAPI_Exec::CalculateContractID(
    const std::string& str_Contract) const
{
    if (str_Contract.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: str_Contract passed in!")
            .Flush();
        return {};
    }
    std::string str_Trim(str_Contract);
    std::string str_Trim2 = String::trim(str_Trim);
    auto strContract = String::Factory(str_Trim2.c_str());

    if (strContract->GetLength() < 2) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Empty contract passed in!")
            .Flush();
        return {};
    }

    auto pContract{api_.Factory().Contract(strContract)};

    if (false != bool(pContract)) {
        auto idOutput = Identifier::Factory();
        pContract->CalculateContractID(idOutput);
        const auto strOutput = String::Factory(idOutput);
        std::string pBuf = strOutput->Get();
        return pBuf;
    } else {
        // ----------------------------------------------------------
        {
            auto serialized =
                proto::StringToProto<proto::ServerContract>(strContract);

            auto id(serialized.id());

            if (id.size() > 0) return id;
        }
        // ----------------------------------------------------------
        {
            auto serialized =
                proto::StringToProto<proto::UnitDefinition>(strContract);

            auto id(serialized.id());

            if (id.size() > 0) return id;
        }
        // ----------------------------------------------------------
    }
    return {};
}

std::string OTAPI_Exec::CreateCurrencyContract(
    const std::string& NYM_ID,
    const std::string& shortname,
    const std::string& terms,
    const std::string& name,
    const std::string& symbol,
    const std::string& tla,
    const std::uint32_t power,
    const std::string& fraction) const
{
    std::string output = "";

    if (NYM_ID.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: NYM_ID passed in!").Flush();
        return output;
    }
    if (terms.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: terms passed in!").Flush();
        return output;
    }
    if (name.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: name passed in!").Flush();
        return output;
    }
    if (symbol.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: symbol passed in!").Flush();
        return output;
    }

    auto pContract = api_.Wallet().UnitDefinition(
        NYM_ID, shortname, name, symbol, terms, tla, power, fraction);

    if (pContract) {
        output = pContract->ID()->str();
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to create currency contract.")
            .Flush();
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

    if (NYM_ID.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: NYM_ID passed in!").Flush();
        return output;
    }
    if (terms.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: terms passed in!").Flush();
        return output;
    }
    if (name.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: name passed in!").Flush();
        return output;
    }
    if (symbol.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: symbol passed in!").Flush();
        return output;
    }

    auto pContract =
        api_.Wallet().UnitDefinition(NYM_ID, shortname, name, symbol, terms);

    if (pContract) {
        output = pContract->ID()->str();
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to create currency contract.")
            .Flush();
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

std::string OTAPI_Exec::GetServer_Contract(
    const std::string& NOTARY_ID) const  // Return's Server's contract (based on
                                         // server ID)
{
    auto pServer = api_.Wallet().Server(Identifier::Factory(NOTARY_ID));

    if (!pServer) { return {}; }

    auto serialized = pServer->Serialize();
    auto armored = Armored::Factory(serialized);
    auto strOutput = String::Factory();
    armored->WriteArmoredString(strOutput, "SERVER CONTRACT");

    return strOutput->Get();
}

std::int32_t OTAPI_Exec::GetCurrencyDecimalPower(
    const std::string& INSTRUMENT_DEFINITION_ID) const
{
    auto unit = api_.Wallet().UnitDefinition(
        Identifier::Factory(INSTRUMENT_DEFINITION_ID));

    if (!unit) return -1;

    return unit->DecimalPower();
}

std::string OTAPI_Exec::GetCurrencyTLA(
    const std::string& INSTRUMENT_DEFINITION_ID) const
{
    auto unit = api_.Wallet().UnitDefinition(
        Identifier::Factory(INSTRUMENT_DEFINITION_ID));

    if (!unit) return {};

    return unit->TLA();
}

std::string OTAPI_Exec::GetCurrencySymbol(
    const std::string& INSTRUMENT_DEFINITION_ID) const
{
    auto pContract = api_.Wallet().UnitDefinition(
        Identifier::Factory(INSTRUMENT_DEFINITION_ID));

    if (!pContract) { return {}; }

    return pContract->GetCurrencySymbol();
}

// Returns amount from formatted string, based on currency contract and locale.
//
std::int64_t OTAPI_Exec::StringToAmount(
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& str_input) const
{
    const std::string str_thousand(OT_THOUSANDS_SEP);
    const std::string str_decimal(OT_DECIMAL_POINT);

    return StringToAmountLocale(
        INSTRUMENT_DEFINITION_ID, str_input, str_thousand, str_decimal);
}

std::int64_t OTAPI_Exec::StringToAmountLocale(
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& str_input,
    const std::string& THOUSANDS_SEP,
    const std::string& DECIMAL_POINT) const
{
    auto unit = api_.Wallet().UnitDefinition(
        Identifier::Factory(INSTRUMENT_DEFINITION_ID));

    if (!unit) { return -1; }

    std::int64_t theResult;
    bool bParsed = unit->StringToAmountLocale(
        theResult, str_input, THOUSANDS_SEP, DECIMAL_POINT);

    return bParsed ? theResult : StringToLong(str_input);
}

// Returns formatted string for output, for a given amount, based on currency
// contract and locale.
//
std::string OTAPI_Exec::FormatAmount(
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::int64_t& THE_AMOUNT) const
{
    const std::string str_thousand(OT_THOUSANDS_SEP);
    const std::string str_decimal(OT_DECIMAL_POINT);

    return FormatAmountLocale(
        INSTRUMENT_DEFINITION_ID, THE_AMOUNT, str_thousand, str_decimal);
}

std::string OTAPI_Exec::FormatAmountLocale(
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::int64_t& THE_AMOUNT,
    const std::string& THOUSANDS_SEP,
    const std::string& DECIMAL_POINT) const
{
    auto unit = api_.Wallet().UnitDefinition(
        Identifier::Factory(INSTRUMENT_DEFINITION_ID));

    if (!unit) return {};

    // By this point, pContract is a good pointer.  (No need to cleanup.)
    const std::int64_t lAmount = THE_AMOUNT;
    std::int64_t theAmount(lAmount);
    auto strBackup = String::Factory(LongToString(THE_AMOUNT));
    std::string str_result;
    const bool bFormatted = unit->FormatAmountLocale(  // Convert 545 to $5.45.
        theAmount,
        str_result,
        THOUSANDS_SEP,
        DECIMAL_POINT);
    const auto strOutput =
        String::Factory(bFormatted ? str_result.c_str() : strBackup->Get());

    return (bFormatted ? str_result : strBackup->Get());
}

// Returns formatted string for output, for a given amount, based on currency
// contract and locale.
std::string OTAPI_Exec::FormatAmountWithoutSymbol(
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::int64_t& THE_AMOUNT) const
{
    const std::string str_thousand(OT_THOUSANDS_SEP);
    const std::string str_decimal(OT_DECIMAL_POINT);

    return FormatAmountWithoutSymbolLocale(
        INSTRUMENT_DEFINITION_ID, THE_AMOUNT, str_thousand, str_decimal);
}

std::string OTAPI_Exec::FormatAmountWithoutSymbolLocale(
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::int64_t& THE_AMOUNT,
    const std::string& THOUSANDS_SEP,
    const std::string& DECIMAL_POINT) const
{
    auto unit = api_.Wallet().UnitDefinition(
        Identifier::Factory(INSTRUMENT_DEFINITION_ID));

    if (!unit) return {};

    auto strBackup = String::Factory(LongToString(THE_AMOUNT));
    std::string str_result;  // Convert 545 to $5.45.
    const bool bFormatted = unit->FormatAmountWithoutSymbolLocale(
        THE_AMOUNT, str_result, THOUSANDS_SEP, DECIMAL_POINT);

    return (bFormatted ? str_result : strBackup->Get());
}

/// Returns currency contract based on Instrument Definition ID
std::string OTAPI_Exec::GetAssetType_Contract(
    const std::string& INSTRUMENT_DEFINITION_ID) const
{
    auto pContract = api_.Wallet().UnitDefinition(
        Identifier::Factory(INSTRUMENT_DEFINITION_ID));

    if (!pContract) { return {}; }

    return proto::ProtoAsArmored<proto::UnitDefinition>(
               pContract->PublicContract(), String::Factory("UNIT DEFINITION"))
        ->Get();
}

// If you have a server contract that you'd like to add
// to your wallet, call this function.
std::string OTAPI_Exec::AddServerContract(const std::string& strContract) const
{
    if (strContract.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: strContract passed in!")
            .Flush();
    } else {
        auto serialized = proto::StringToProto<proto::ServerContract>(
            String::Factory(strContract));
        auto contract = api_.Wallet().Server(serialized);

        if (contract) { return contract->ID()->str(); }
    }

    return {};
}

// If you have an asset contract that you'd like to add
// to your wallet, call this function.
//
std::string OTAPI_Exec::AddUnitDefinition(const std::string& strContract) const
{
    if (strContract.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: strContract passed in!")
            .Flush();
    } else {
        auto serialized = proto::StringToProto<proto::UnitDefinition>(
            String::Factory(strContract));
        auto contract = api_.Wallet().UnitDefinition(serialized);

        if (contract) { return contract->ID()->str(); }
    }

    return {};
}

std::int32_t OTAPI_Exec::GetNymCount(void) const
{
    return ot_api_.GetNymCount();
}

std::int32_t OTAPI_Exec::GetServerCount(void) const
{
    const auto servers = api_.Wallet().ServerList();
    return servers.size();
}

std::int32_t OTAPI_Exec::GetAssetTypeCount(void) const
{
    const auto units = api_.Wallet().UnitDefinitionList();
    return units.size();
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
    return ot_api_.Wallet_CanRemoveServer(Identifier::Factory(NOTARY_ID));
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
    Lock lock(lock_);

    OT_ASSERT_MSG(
        !NOTARY_ID.empty(),
        "OTAPI_Exec::Wallet_RemoveServer: Null NOTARY_ID passed in.");

    // Make sure there aren't any dependent accounts..
    if (!Wallet_CanRemoveServer(NOTARY_ID)) return false;

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
    auto theID = Identifier::Factory(NOTARY_ID);

    if (api_.Wallet().RemoveServer(theID)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Removed server contract from the wallet: ")(NOTARY_ID)
            .Flush();
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
    return ot_api_.Wallet_CanRemoveAssetType(
        Identifier::Factory(INSTRUMENT_DEFINITION_ID));
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
    Lock lock(lock_);

    OT_ASSERT_MSG(
        !INSTRUMENT_DEFINITION_ID.empty(),
        "OTAPI_Exec::Wallet_"
        "RemoveAssetType: Null "
        "INSTRUMENT_DEFINITION_ID "
        "passed in.");

    // Make sure there aren't any dependent accounts..
    if (!OTAPI_Exec::Wallet_CanRemoveAssetType(INSTRUMENT_DEFINITION_ID))
        return false;

    auto theID = Identifier::Factory(INSTRUMENT_DEFINITION_ID);

    if (api_.Wallet().RemoveUnitDefinition(theID)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Removed unit definition contract from the wallet: ")(
            INSTRUMENT_DEFINITION_ID)
            .Flush();
        return true;
    }
    return false;
}

bool OTAPI_Exec::Wallet_CanRemoveNym(const std::string& NYM_ID) const
{
    return ot_api_.Wallet_CanRemoveNym(Identifier::Factory(NYM_ID));
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
    OT_ASSERT_MSG(
        !NYM_ID.empty(),
        "OTAPI_Exec::Wallet_RemoveNym: Null NYM_ID passed in.");

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
    // cleaner for the server side, who otherwise has to expire unused nyms
    // based on some rule
    // presumably to be found in the server contract.
    if (!OTAPI_Exec::Wallet_CanRemoveNym(NYM_ID)) return false;

    OTWallet* pWallet = ot_api_.GetWallet(__FUNCTION__);

    if (nullptr == pWallet) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": No wallet found...!").Flush();
        return false;
    }

    auto theID = Identifier::Factory(NYM_ID);

    // Nyms can't be removed.  May be changed to hide the nyms.
    //
    //    if (pWallet->RemovePrivateNym(theID)) {
    //        otOut << OT_METHOD << __FUNCTION__
    //              << ": Success erasing Nym from wallet: " << NYM_ID << "\n";
    //        pWallet->SaveWallet();
    //        return true;
    //    } else
    LogNormal(OT_METHOD)(__FUNCTION__)(": Failure erasing Nym from wallet: ")(
        NYM_ID)(".")
        .Flush();

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
    Lock lock(lock_);

    OT_ASSERT_MSG(
        !ACCOUNT_ID.empty(),
        "OTAPI_Exec::Wallet_CanRemoveAccount: Null ACCOUNT_ID passed in.");

    const auto theAccountID = Identifier::Factory(ACCOUNT_ID);
    auto account = api_.Wallet().Account(theAccountID);

    if (false == bool(account)) {

        return false;
        // Balance must be zero in order to close an account!
    } else if (account.get().GetBalance() != 0) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Account balance MUST be zero in order to "
            "close an asset account: ")(ACCOUNT_ID)(".")
            .Flush();
        return false;
    }
    bool BOOL_RETURN_VALUE = false;

    const Identifier& theNotaryID = account.get().GetPurportedNotaryID();
    const Identifier& theNymID = account.get().GetNymID();

    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.
    std::unique_ptr<Ledger> pInbox(
        ot_api_.LoadInbox(theNotaryID, theNymID, theAccountID));
    std::unique_ptr<Ledger> pOutbox(
        ot_api_.LoadOutbox(theNotaryID, theNymID, theAccountID));

    if (nullptr == pInbox) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failure calling OT_API::LoadInbox. Account ID: ")(ACCOUNT_ID)(
            ".")
            .Flush();
    } else if (nullptr == pOutbox) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failure calling OT_API::LoadOutbox. Account ID: ")(ACCOUNT_ID)(
            ".")
            .Flush();
    } else if (
        (pInbox->GetTransactionCount() > 0) ||
        (pOutbox->GetTransactionCount() > 0)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failure: You cannot remove an asset "
            "account if there are inbox/outbox items "
            "still waiting to be processed.")
            .Flush();
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
    const std::int32_t& nBoxType,   // 0/nymbox, 1/inbox, 2/outbox
    const std::int64_t& TRANSACTION_NUMBER) const
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(ACCOUNT_ID);

    if (!((0 == nBoxType) || (1 == nBoxType) || (2 == nBoxType))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": nBoxType is of wrong type: value: ")(nBoxType)(".")
            .Flush();
        return false;
    }
    if (0 > TRANSACTION_NUMBER) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Negative: TRANSACTION_NUMBER passed in!")
            .Flush();
        return false;
    }
    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theNymID = Identifier::Factory(NYM_ID),
               theAccountID = Identifier::Factory(ACCOUNT_ID);
    const std::int64_t lTransactionNum = TRANSACTION_NUMBER;
    switch (nBoxType) {
        case 0:  // nymbox
        case 1:  // inbox
        case 2:  // outbox
            break;
        default:
            LogOutput(OT_METHOD)(__FUNCTION__)(": Error: bad nBoxType: ")(
                nBoxType)(" for NymID: ")(NYM_ID)(" AcctID: ")(ACCOUNT_ID)(
                " (expected one of: 0/nymbox, 1/inbox, 2/outbox).")
                .Flush();
            return false;
    }
    return ot_api_.DoesBoxReceiptExist(
        theNotaryID,
        theNymID,
        theAccountID,  // If for Nymbox (vs inbox/outbox)
                       // then pass NYM_ID in this field
                       // also.
        nBoxType,      // 0/nymbox, 1/inbox, 2/outbox
        static_cast<std::int64_t>(lTransactionNum));
}

// OT has the capability to export a Nym (normally stored in several files) as
// an encoded
// object (in base64-encoded form) and then import it again.
// Returns: Exported Nym in String Form.
//
std::string OTAPI_Exec::Wallet_ExportNym(const std::string& NYM_ID) const
{
    OT_ASSERT_MSG(
        !NYM_ID.empty(),
        "OTAPI_Exec::Wallet_ExportNym: Null NYM_ID passed in.");

    const auto theNymID = Identifier::Factory(NYM_ID);

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
    auto strOutput = String::Factory();

    if (ot_api_.Wallet_ExportNym(theNymID, strOutput)) {

        return strOutput->Get();
    }

    return {};
}

// OT has the capability to export a Nym (normally stored in several files) as
// an encoded
// object (in base64-encoded form) and then import it again.
//
// Returns: Nym ID of newly-imported Nym (or "".)
//
std::string OTAPI_Exec::Wallet_ImportNym(const std::string& FILE_CONTENTS) const
{
    if (FILE_CONTENTS.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: FILE_CONTENTS passed in!")
            .Flush();
        return {};
    }

    // Pause the master key, since this Nym is coming from outside
    // the wallet.
    const auto strFileContents = String::Factory(FILE_CONTENTS);
    auto theNymID = Identifier::Factory();

    if (ot_api_.Wallet_ImportNym(strFileContents, theNymID)) {

        return theNymID->str();
    }

    return {};
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
    return ot_api_.Wallet_ChangePassphrase();
}

// bool Nym::Savex509CertAndPrivateKeyToString(OTString& strOutput,
// OTString * pstrReason)

// bool Nym::Savex509CertAndPrivateKey(bool       bCreateFile,
//                                            OTString * pstrReason)

// Attempts to find a full ID in the wallet, based on a partial of the same ID.
// Returns "" on failure, otherwise returns the full ID.
//
std::string OTAPI_Exec::Wallet_GetNymIDFromPartial(
    const std::string& PARTIAL_ID) const
{
    if (PARTIAL_ID.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Empty PARTIAL_ID passed in!")
            .Flush();
        return {};
    }

    const Nym* pObject = nullptr;

    auto thePartialID = Identifier::Factory(PARTIAL_ID);

    // In this case, the user maybe passed in the FULL ID.
    // (We STILL confirm whether he's found in the wallet...)
    //
    if (!thePartialID->empty()) {
        auto constNym = api_.Wallet().Nym(thePartialID);
        if (constNym) { pObject = constNym.get(); }
    }

    if (nullptr != pObject)  // Found it (as full ID.)
    {
        return pObject->ID().str();
    }
    // Below this point, it definitely wasn't a FULL ID, at least one that
    // we know about, so now we can go ahead and search for it as a PARTIAL
    // ID...
    //
    auto constNym = api_.Wallet().NymByIDPartialMatch(PARTIAL_ID);
    if (constNym) { pObject = constNym.get(); }

    if (nullptr != pObject)  // Found it (as partial ID.)
    {
        return pObject->ID().str();
    }

    return {};
}

// Attempts to find a full ID in the wallet, based on a partial of the same ID.
// Returns "" on failure, otherwise returns the full ID.
//
std::string OTAPI_Exec::Wallet_GetNotaryIDFromPartial(
    const std::string& PARTIAL_ID) const
{
    if (PARTIAL_ID.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: PARTIAL_ID passed in!")
            .Flush();
        return {};
    }

    const auto servers = api_.Wallet().ServerList();
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
    if (PARTIAL_ID.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: PARTIAL_ID passed in!")
            .Flush();
        return {};
    }

    auto theID = Identifier::Factory(PARTIAL_ID);
    ConstUnitDefinition pUnit;  // shared_ptr to const.

    // See if it's available using the full length ID.
    if (!theID->empty()) pUnit = api_.Wallet().UnitDefinition(theID);

    if (!pUnit) {
        const auto units = api_.Wallet().UnitDefinitionList();

        // See if it's available using the partial length ID.
        for (auto& it : units) {
            if (0 == it.first.compare(0, PARTIAL_ID.length(), PARTIAL_ID)) {
                pUnit =
                    api_.Wallet().UnitDefinition(Identifier::Factory(it.first));
                break;
            }
        }
        if (!pUnit) {
            // See if it's available using the full length name.
            for (auto& it : units) {
                if (0 == it.second.compare(0, it.second.length(), PARTIAL_ID)) {
                    pUnit = api_.Wallet().UnitDefinition(
                        Identifier::Factory(it.first));
                    break;
                }
            }

            if (!pUnit) {
                // See if it's available using the partial name.
                for (auto& it : units) {
                    if (0 ==
                        it.second.compare(0, PARTIAL_ID.length(), PARTIAL_ID)) {
                        pUnit = api_.Wallet().UnitDefinition(
                            Identifier::Factory(it.first));
                        break;
                    }
                }
            }
        }
    }

    if (pUnit)  // Found it (as partial ID.)
    {
        return proto::ProtoAsArmored<proto::UnitDefinition>(
                   pUnit->PublicContract(), String::Factory("UNIT DEFINITION"))
            ->Get();
    }

    return {};
}

std::string OTAPI_Exec::Wallet_GetAccountIDFromPartial(
    const std::string& PARTIAL_ID) const
{
    return api_.Wallet().AccountPartialMatch(PARTIAL_ID)->str();
}

/// based on Index this returns the Nym's ID
std::string OTAPI_Exec::GetNym_ID(const std::int32_t& nIndex) const
{
    if (0 > nIndex) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": nIndex is out of bounds (it's in the negative!).")
            .Flush();
        return {};
    }

    auto theNymID = Identifier::Factory();
    auto strName = String::Factory();
    bool bGetNym = ot_api_.GetNym(nIndex, theNymID, strName);

    if (bGetNym) { return theNymID->str(); }

    return {};
}

/// Returns Nym Name (based on NymID)
std::string OTAPI_Exec::GetNym_Name(const std::string& NYM_ID) const
{
    auto nym = api_.Wallet().Nym(Identifier::Factory(NYM_ID));

    if (!nym) { return {}; }

    return nym->Alias();
}

bool OTAPI_Exec::IsNym_RegisteredAtServer(
    const std::string& NYM_ID,
    const std::string& NOTARY_ID) const
{
    OT_ASSERT_MSG(
        !NYM_ID.empty(),
        "OTAPI_Exec::IsNym_RegisteredAtServer: Null NYM_ID passed in.");
    OT_ASSERT_MSG(
        !NOTARY_ID.empty(),
        "OTAPI_Exec::IsNym_RegisteredAtServer: Null NOTARY_ID passed in.");

    const auto theNymID = Identifier::Factory(NYM_ID),
               theNotaryID = Identifier::Factory(NOTARY_ID);

    return ot_api_.IsNym_RegisteredAtServer(theNymID, theNotaryID);
}

// Returns Nym data (based on NymID)
//
std::string OTAPI_Exec::GetNym_Stats(const std::string& NYM_ID) const
{
    OT_ASSERT_MSG(
        !NYM_ID.empty(), "OTAPI_Exec::GetNym_Stats: Null NYM_ID passed in.");

    auto theNymID = Identifier::Factory(NYM_ID);
    std::unique_ptr<const class NymFile> pNym =
        api_.Wallet().Nymfile(theNymID, __FUNCTION__);

    if (nullptr != pNym) {
        auto strOutput = String::Factory();

        pNym->DisplayStatistics(strOutput);

        std::string pBuf = strOutput->Get();

        return pBuf;
    }

    return {};
}

// Returns NymboxHash (based on NotaryID)
//
std::string OTAPI_Exec::GetNym_NymboxHash(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const  // Returns NymboxHash (based on NotaryID)
{
    OT_ASSERT_MSG(
        !NOTARY_ID.empty(),
        "OTAPI_Exec::GetNym_NymboxHash: Null NOTARY_ID passed in.");
    OT_ASSERT_MSG(
        !NYM_ID.empty(),
        "OTAPI_Exec::GetNym_NymboxHash: Null NYM_ID passed in.");

    auto context = api_.Wallet().ServerContext(
        Identifier::Factory(NYM_ID), Identifier::Factory(NOTARY_ID));

    if (context) { return context->LocalNymboxHash()->str(); }

    return "";
}

// Returns RecentHash (based on NotaryID)
//
std::string OTAPI_Exec::GetNym_RecentHash(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const  // Returns RecentHash (based on NotaryID)
{
    OT_ASSERT_MSG(
        !NOTARY_ID.empty(),
        "OTAPI_Exec::GetNym_RecentHash: Null NOTARY_ID passed in.");
    OT_ASSERT_MSG(
        !NYM_ID.empty(),
        "OTAPI_Exec::GetNym_RecentHash: Null NYM_ID passed in.");

    auto context = api_.Wallet().ServerContext(
        Identifier::Factory(NYM_ID), Identifier::Factory(NOTARY_ID));

    if (context) { return context->RemoteNymboxHash()->str(); }

    return "";
}

std::string OTAPI_Exec::GetNym_InboxHash(
    const std::string& ACCOUNT_ID,
    const std::string& NYM_ID) const  // InboxHash for "most recently
                                      // DOWNLOADED" Inbox
                                      // (by AccountID)
{
    OT_ASSERT_MSG(
        !ACCOUNT_ID.empty(),
        "OTAPI_Exec::GetNym_InboxHash: Null ACCOUNT_ID passed in.");
    OT_ASSERT_MSG(
        !NYM_ID.empty(),
        "OTAPI_Exec::GetNym_InboxHash: Null NYM_ID passed in.");

    auto theNymID = Identifier::Factory(NYM_ID);
    auto pNym = api_.Wallet().Nymfile(theNymID, __FUNCTION__);

    if (nullptr != pNym) {
        auto theHash = Identifier::Factory();
        const std::string str_acct_id(ACCOUNT_ID);
        const bool bGothash =
            pNym->GetInboxHash(str_acct_id, theHash);  // (theHash is output.)

        if (!bGothash) {
            const auto strNymID =
                String::Factory(theNymID);  // You might ask, why create this
                                            // string and not just use
                                            // NYM_ID?
            // The answer is because I'm looking forward to a day soon when we
            // don't passconst std::string& in the first
            // place, and thus I can't always expect that variable will be
            // there.
            //
            LogDetail(OT_METHOD)(__FUNCTION__)(
                ": InboxHash not found, on client side, for account ")(
                str_acct_id)(" and nym ")(strNymID)(". (Returning.) ")
                .Flush();
        } else  // Success: the hash was there, for that Nym, for that server
                // ID.
        {

            return String::Factory(theHash)->Get();
        }
    }

    return {};
}

std::string OTAPI_Exec::GetNym_OutboxHash(
    const std::string& ACCOUNT_ID,
    const std::string& NYM_ID) const  // OutboxHash for "most recently
                                      // DOWNLOADED"
                                      // Outbox (by AccountID)
{
    OT_ASSERT_MSG(
        !ACCOUNT_ID.empty(),
        "OTAPI_Exec::GetNym_OutboxHash: Null ACCOUNT_ID passed in.");
    OT_ASSERT_MSG(
        !NYM_ID.empty(),
        "OTAPI_Exec::GetNym_OutboxHash: Null NYM_ID passed in.");

    auto theNymID = Identifier::Factory(NYM_ID);
    auto pNym = api_.Wallet().Nymfile(theNymID, __FUNCTION__);

    if (nullptr != pNym) {
        auto theHash = Identifier::Factory();
        const std::string str_acct_id(ACCOUNT_ID);
        const bool bGothash =
            pNym->GetOutboxHash(str_acct_id, theHash);  // (theHash is output.)

        if (!bGothash) {
            const auto strNymID =
                String::Factory(theNymID);  // You might ask, why create this
                                            // string and not just use
                                            // NYM_ID?
            // The answer is because I'm looking forward to a day soon when we
            // don't passconst std::string& in the first
            // place, and thus I can't always expect that variable will be
            // there.
            //
            LogDetail(OT_METHOD)(__FUNCTION__)(
                ": OutboxHash not found, on client side, for account ")(
                str_acct_id)(" and nym ")(strNymID)(". (Returning.) ")
                .Flush();
        } else  // Success: the hash was there, for that Nym, for that server
                // ID.
        {
            return String::Factory(theHash)->Get();
        }
    }

    return {};
}

std::list<std::string> OTAPI_Exec::GetNym_MailThreads(
    const std::string& NYM_ID,
    const bool unreadOnly) const
{
    const auto nym = Identifier::Factory(NYM_ID);
    const auto threads = activity_.Threads(nym, unreadOnly);
    std::list<std::string> output;

    for (auto& item : threads) { output.push_back(item.first); }

    return output;
}

std::list<std::string> OTAPI_Exec::GetNym_MailCount(
    const std::string& NYM_ID) const
{
    return ot_api_.BoxItemCount(
        Identifier::Factory(NYM_ID), StorageBox::MAILINBOX);
}

std::string OTAPI_Exec::GetNym_MailContentsByIndex(
    const std::string& nym,
    const std::string& nIndex) const
{
    return ot_api_.BoxContents(
        Identifier::Factory(nym),
        Identifier::Factory(nIndex),
        StorageBox::MAILINBOX);
}

std::string OTAPI_Exec::GetNym_MailSenderIDByIndex(
    const std::string& NYM_ID,
    const std::string& nIndex) const
{
    const auto message = activity_.Mail(
        Identifier::Factory(NYM_ID),
        Identifier::Factory(nIndex),
        StorageBox::MAILINBOX);

    if (!message) { return {}; }

    return message->m_strNymID->Get();
}

std::string OTAPI_Exec::GetNym_MailNotaryIDByIndex(
    const std::string& NYM_ID,
    const std::string& nIndex) const
{
    const auto message = activity_.Mail(
        Identifier::Factory(NYM_ID),
        Identifier::Factory(nIndex),
        StorageBox::MAILINBOX);

    if (!message) { return {}; }

    return message->m_strNotaryID->Get();
}

bool OTAPI_Exec::Nym_RemoveMailByIndex(
    const std::string& NYM_ID,
    const std::string& nIndex) const
{
    return activity_.MailRemove(
        Identifier::Factory(NYM_ID),
        Identifier::Factory(nIndex),
        StorageBox::MAILINBOX);
}

bool OTAPI_Exec::Nym_VerifyMailByIndex(
    const std::string& NYM_ID,
    const std::string& nIndex) const
{
    const auto message = activity_.Mail(
        Identifier::Factory(NYM_ID),
        Identifier::Factory(nIndex),
        StorageBox::MAILINBOX);

    if (!message) { return false; }

    auto senderNym =
        api_.Wallet().Nym(Identifier::Factory(message->m_strNymID));

    if (!senderNym) { return false; }

    return message->VerifySignature(*senderNym);
}

std::list<std::string> OTAPI_Exec::GetNym_OutmailCount(
    const std::string& NYM_ID) const
{
    return ot_api_.BoxItemCount(
        Identifier::Factory(NYM_ID), StorageBox::MAILOUTBOX);
}

std::string OTAPI_Exec::GetNym_OutmailContentsByIndex(
    const std::string& nym,
    const std::string& nIndex) const
{
    return ot_api_.BoxContents(
        Identifier::Factory(nym),
        Identifier::Factory(nIndex),
        StorageBox::MAILOUTBOX);
}

std::string OTAPI_Exec::GetNym_OutmailRecipientIDByIndex(
    const std::string& NYM_ID,
    const std::string& nIndex) const
{
    const auto message = activity_.Mail(
        Identifier::Factory(NYM_ID),
        Identifier::Factory(nIndex),
        StorageBox::MAILOUTBOX);

    if (!message) { return {}; }

    return message->m_strNymID2->Get();
}

std::string OTAPI_Exec::GetNym_OutmailNotaryIDByIndex(
    const std::string& NYM_ID,
    const std::string& nIndex) const
{
    const auto message = activity_.Mail(
        Identifier::Factory(NYM_ID),
        Identifier::Factory(nIndex),
        StorageBox::MAILOUTBOX);

    if (!message) { return {}; }

    return message->m_strNotaryID->Get();
}

bool OTAPI_Exec::Nym_RemoveOutmailByIndex(
    const std::string& NYM_ID,
    const std::string& nIndex) const
{
    return activity_.MailRemove(
        Identifier::Factory(NYM_ID),
        Identifier::Factory(nIndex),
        StorageBox::MAILOUTBOX);
}

bool OTAPI_Exec::Nym_VerifyOutmailByIndex(
    const std::string& NYM_ID,
    const std::string& nIndex) const
{
    const auto message = activity_.Mail(
        Identifier::Factory(NYM_ID),
        Identifier::Factory(nIndex),
        StorageBox::MAILOUTBOX);

    if (!message) { return false; }

    auto senderNym =
        api_.Wallet().Nym(Identifier::Factory(message->m_strNymID));

    if (!senderNym) { return false; }

    return message->VerifySignature(*senderNym);
}

//
// OUTPAYMENTS!!
//
// (Outbox on payments screen.)
//
// Todo: Move these and all functions to OpenTransactions.cpp.  This should ONLY
// be a wrapper for that class.  That way we can eventually phase this file out
// entirely and replace it with SwigWrapper.cpp directly on
// OpenTransactions.cpp

std::int32_t OTAPI_Exec::GetNym_OutpaymentsCount(
    const std::string& NYM_ID) const
{
    OT_ASSERT_MSG(
        !NYM_ID.empty(),
        "OTAPI_Exec::GetNym_OutpaymentsCount: Null NYM_ID passed in.");

    auto theNymID = Identifier::Factory(NYM_ID);
    auto pNym = api_.Wallet().Nymfile(theNymID, __FUNCTION__);

    if (false == bool(pNym)) return OT_ERROR;

    return pNym->GetOutpaymentsCount();
}

// Returns the payment instrument that was sent.
//
std::string OTAPI_Exec::GetNym_OutpaymentsContentsByIndex(
    const std::string& NYM_ID,
    const std::int32_t& nIndex) const
{
    OT_ASSERT_MSG(
        !NYM_ID.empty(),
        "OTAPI_Exec::GetNym_"
        "OutpaymentsContentsByIndex: Null NYM_ID "
        "passed in.");

    if (0 > nIndex) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": nIndex is out of bounds (it's in the negative)!")
            .Flush();
        return {};
    }
    auto theNymID = Identifier::Factory(NYM_ID);
    auto pNym = api_.Wallet().Nymfile(theNymID, __FUNCTION__);

    if (false == bool(pNym)) return {};

    auto pMessage = pNym->GetOutpaymentsByIndex(nIndex);
    if (false != bool(pMessage)) {
        // SENDER:     pMessage->m_strNymID
        // RECIPIENT:  pMessage->m_strNymID2
        // INSTRUMENT: pMessage->m_ascPayload (in an OTEnvelope)
        auto strPayment = String::Factory();

        // There isn't any encrypted envelope this time, since it's my
        // outPayments box.
        //
        if (pMessage->m_ascPayload->Exists() &&
            pMessage->m_ascPayload->GetString(strPayment) &&
            strPayment->Exists()) {
            auto thePayment{api_.Factory().Payment(strPayment)};

            if (thePayment->IsValid()) {
                std::string pBuf = strPayment->Get();
                return pBuf;
            }
        }
    }
    return {};
}

// returns the recipient ID for a piece of payments outmail. (NymID).
//
std::string OTAPI_Exec::GetNym_OutpaymentsRecipientIDByIndex(
    const std::string& NYM_ID,
    const std::int32_t& nIndex) const
{
    OT_ASSERT_MSG(
        !NYM_ID.empty(),
        "OTAPI_Exec::GetNym_"
        "OutpaymentsRecipientIDByIndex: Null NYM_ID "
        "passed in.");

    if (0 > nIndex) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": nIndex is out of bounds (it's in the negative)!")
            .Flush();
        return {};
    }
    auto theNymID = Identifier::Factory(NYM_ID);
    std::unique_ptr<const class NymFile> pNym =
        api_.Wallet().Nymfile(theNymID, __FUNCTION__);
    if (false == bool(pNym)) return {};
    auto pMessage = pNym->GetOutpaymentsByIndex(nIndex);
    if (false != bool(pMessage)) {
        // SENDER:    pMessage->m_strNymID
        // SERVER:    pMessage->m_strNotaryID
        // RECIPIENT: pMessage->m_strNymID2
        // MESSAGE:   pMessage->m_ascPayload

        std::string pBuf = pMessage->m_strNymID2->Get();
        return pBuf;
    }
    return {};
}

// returns the server ID that a piece of outgoing payments went thru.
//
std::string OTAPI_Exec::GetNym_OutpaymentsNotaryIDByIndex(
    const std::string& NYM_ID,
    const std::int32_t& nIndex) const
{
    OT_ASSERT_MSG(
        !NYM_ID.empty(),
        "OTAPI_Exec::GetNym_"
        "OutpaymentsNotaryIDByIndex: Null NYM_ID "
        "passed in.");

    if (0 > nIndex) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": nIndex is out of bounds (it's in the negative)!")
            .Flush();
        return {};
    }
    auto theNymID = Identifier::Factory(NYM_ID);
    std::unique_ptr<const class NymFile> pNym =
        api_.Wallet().Nymfile(theNymID, __FUNCTION__);
    if (false == bool(pNym)) return {};
    auto pMessage = pNym->GetOutpaymentsByIndex(nIndex);

    if (false != bool(pMessage)) {
        // SENDER:    pMessage->m_strNymID
        // SERVER:    pMessage->m_strNotaryID
        // RECIPIENT: pMessage->m_strNymID2
        // MESSAGE:   pMessage->m_ascPayload

        std::int32_t bNotaryIDLength = pMessage->m_strNotaryID->GetLength();
        if (1 >= bNotaryIDLength) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": m_strNotaryID Length is 1 or less!")
                .Flush();
            return {};
        }

        std::string pBuf = pMessage->m_strNotaryID->Get();
        return pBuf;
    }
    return {};
}

bool OTAPI_Exec::Nym_RemoveOutpaymentsByIndex(
    const std::string& NYM_ID,
    const std::int32_t& nIndex) const
{
    OT_ASSERT_MSG(
        !NYM_ID.empty(),
        "OTAPI_Exec::Nym_RemoveOutpaymentsByIndex: Null NYM_ID passed in.");

    if (0 > nIndex) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": nIndex is out of bounds (it's in the negative)!")
            .Flush();
        return false;
    }

    auto theNymID = Identifier::Factory(NYM_ID);
    auto pNym = api_.Wallet().mutable_Nymfile(theNymID, __FUNCTION__);

    if (pNym.It().RemoveOutpaymentsByIndex(nIndex)) {
        // save Nym to local storage, since a payment outmail was erased.
        return true;
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
    const std::int32_t& nIndex) const
{
    OT_ASSERT_MSG(
        !NYM_ID.empty(),
        "OTAPI_Exec::Nym_VerifyOutpaymentsByIndex: Null NYM_ID passed in.");

    if (0 > nIndex) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": nIndex is out of bounds (it's in the negative)!")
            .Flush();
        return false;
    }
    auto theNymID = Identifier::Factory(NYM_ID);
    std::unique_ptr<const class NymFile> pNym =
        api_.Wallet().Nymfile(theNymID, __FUNCTION__);
    if (false == bool(pNym)) return false;
    auto pMessage = pNym->GetOutpaymentsByIndex(nIndex);
    if (false != bool(pMessage)) {
        // Grab the NymID of the sender.
        const auto theSenderNymID = Identifier::Factory(pMessage->m_strNymID);

        // Grab a pointer to that Nym (if its public key is in my wallet.)
        ConstNym pSenderNym = api_.Wallet().Nym(theSenderNymID);

        // If it's there, use it to verify the signature on the message.
        // return true if successful signature verification.
        //
        if (false != bool(pSenderNym)) {
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

std::int64_t OTAPI_Exec::Instrmnt_GetAmount(
    const std::string& THE_INSTRUMENT) const
{
    if (THE_INSTRUMENT.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: THE_INSTRUMENT passed in!")
            .Flush();
        return -1;
    }
    const auto strInstrument = String::Factory(THE_INSTRUMENT);
    auto thePayment{api_.Factory().Payment(strInstrument)};

    if (!thePayment->IsValid()) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to parse instrument: ")(
            strInstrument)(".")
            .Flush();
        return -1;
    }
    const bool bSetValues = thePayment->SetTempValues();
    if (!bSetValues) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to load instrument: ")(
            strInstrument)(".")
            .Flush();
        return -1;
    }
    // BY THIS POINT, we have definitely loaded up all the values of the
    // instrument
    // into the OTPayment object. (Meaning we can now return the requested
    // data...)

    std::int64_t lOutput = 0;
    return thePayment->GetAmount(lOutput) ? lOutput : OT_ERROR_AMOUNT;
}

std::int64_t OTAPI_Exec::Instrmnt_GetTransNum(
    const std::string& THE_INSTRUMENT) const
{
    if (THE_INSTRUMENT.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: THE_INSTRUMENT passed in!")
            .Flush();
        return -1;
    }
    const auto strInstrument = String::Factory(THE_INSTRUMENT);
    auto thePayment{api_.Factory().Payment(strInstrument)};

    if (!thePayment->IsValid()) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to parse instrument: ")(
            strInstrument)(".")
            .Flush();
        return -1;
    }
    const bool bSetValues = thePayment->SetTempValues();
    if (!bSetValues) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to load instrument: ")(
            strInstrument)(".")
            .Flush();
        return -1;
    }
    // BY THIS POINT, we have definitely loaded up all the values of the
    // instrument
    // into the OTPayment object. (Meaning we can now return the requested
    // data...)
    auto strOutput = String::Factory();
    std::int64_t lOutput = 0;
    const bool bGotData = thePayment->GetTransactionNum(lOutput);  // <========

    return bGotData ? lOutput : -1;
}

time64_t OTAPI_Exec::Instrmnt_GetValidFrom(
    const std::string& THE_INSTRUMENT) const
{
    if (THE_INSTRUMENT.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: THE_INSTRUMENT passed in!")
            .Flush();
        return OTTimeGetTimeFromSeconds(-1);
    }
    const auto strInstrument = String::Factory(THE_INSTRUMENT);
    auto thePayment{api_.Factory().Payment(strInstrument)};

    if (!thePayment->IsValid()) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to parse instrument: ")(
            strInstrument)(".")
            .Flush();
        return OTTimeGetTimeFromSeconds(-1);
    }
    const bool bSetValues = thePayment->SetTempValues();
    if (!bSetValues) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to load instrument: ")(
            strInstrument)(".")
            .Flush();
        return OTTimeGetTimeFromSeconds(-1);
    }

    // BY THIS POINT, we have definitely loaded up all the values of the
    // instrument
    // into the OTPayment object. (Meaning we can now return the requested
    // data...)

    auto strOutput = String::Factory();
    time64_t tOutput = OT_TIME_ZERO;
    const bool bGotData = thePayment->GetValidFrom(tOutput);  // <========

    return bGotData ? tOutput : OTTimeGetTimeFromSeconds(-1);
}

time64_t OTAPI_Exec::Instrmnt_GetValidTo(
    const std::string& THE_INSTRUMENT) const
{
    if (THE_INSTRUMENT.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: THE_INSTRUMENT passed in!")
            .Flush();
        return OTTimeGetTimeFromSeconds(-1);
    }
    const auto strInstrument = String::Factory(THE_INSTRUMENT);
    auto thePayment{api_.Factory().Payment(strInstrument)};

    if (!thePayment->IsValid()) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to parse instrument: ")(
            strInstrument)(".")
            .Flush();
        return OTTimeGetTimeFromSeconds(-1);
    }
    const bool bSetValues = thePayment->SetTempValues();
    if (!bSetValues) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to load instrument: ")(
            strInstrument)(".")
            .Flush();
        return OTTimeGetTimeFromSeconds(-1);
    }

    // BY THIS POINT, we have definitely loaded up all the values of the
    // instrument
    // into the OTPayment object. (Meaning we can now return the requested
    // data...)

    auto strOutput = String::Factory();
    time64_t tOutput = OT_TIME_ZERO;
    const bool bGotData = thePayment->GetValidTo(tOutput);  // <========

    return bGotData ? tOutput : OTTimeGetTimeFromSeconds(-1);
}

std::string OTAPI_Exec::Instrmnt_GetType(
    const std::string& THE_INSTRUMENT) const
{
    if (THE_INSTRUMENT.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: THE_INSTRUMENT passed in!")
            .Flush();
        return {};
    }
    const auto strInstrument = String::Factory(THE_INSTRUMENT);
    auto thePayment{api_.Factory().Payment(strInstrument)};

    if (!thePayment->IsValid()) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to parse instrument: ")(
            strInstrument)(".")
            .Flush();
        return {};
    }
    const bool bSetValues = thePayment->SetTempValues();
    if (!bSetValues) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to load instrument: ")(
            strInstrument)(".")
            .Flush();
        return {};
    }
    // BY THIS POINT, we have definitely loaded up all the values of the
    // instrument
    // into the OTPayment object. (Meaning we can now return the requested
    // data...)

    const auto strOutput = String::Factory(thePayment->GetTypeString());

    if (strOutput->Exists()) {
        std::string pBuf = strOutput->Get();

        return pBuf;
    }

    return {};
}

std::string OTAPI_Exec::Instrmnt_GetMemo(
    const std::string& THE_INSTRUMENT) const
{
    if (THE_INSTRUMENT.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: THE_INSTRUMENT passed in!")
            .Flush();
        return {};
    }
    const auto strInstrument = String::Factory(THE_INSTRUMENT);
    auto thePayment{api_.Factory().Payment(strInstrument)};

    if (!thePayment->IsValid()) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to parse instrument: ")(
            strInstrument)(".")
            .Flush();
        return {};
    }
    const bool bSetValues = thePayment->SetTempValues();
    if (!bSetValues) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to load instrument: ")(
            strInstrument)(".")
            .Flush();
        return {};
    }

    // BY THIS POINT, we have definitely loaded up all the values of the
    // instrument
    // into the OTPayment object. (Meaning we can now return the requested
    // data...)

    auto strOutput = String::Factory();
    const bool bGotData = thePayment->GetMemo(strOutput);  // <========

    if (bGotData) {
        std::string pBuf = strOutput->Get();
        return pBuf;
    }

    return {};
}

std::string OTAPI_Exec::Instrmnt_GetNotaryID(
    const std::string& THE_INSTRUMENT) const
{
    if (THE_INSTRUMENT.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: THE_INSTRUMENT passed in!")
            .Flush();
        return {};
    }
    const auto strInstrument = String::Factory(THE_INSTRUMENT);
    auto thePayment{api_.Factory().Payment(strInstrument)};

    if (!thePayment->IsValid()) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to parse instrument: ")(
            strInstrument)(".")
            .Flush();
        return {};
    }
    const bool bSetValues = thePayment->SetTempValues();
    if (!bSetValues) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to load instrument: ")(
            strInstrument)(".")
            .Flush();
        return {};
    }

    // BY THIS POINT, we have definitely loaded up all the values of the
    // instrument
    // into the OTPayment object. (Meaning we can now return the requested
    // data...)

    auto theOutput = Identifier::Factory();
    const bool bGotData = thePayment->GetNotaryID(theOutput);  // <========

    if (bGotData) {
        const auto strOutput = String::Factory(theOutput);
        std::string pBuf = strOutput->Get();
        return pBuf;
    }

    return {};
}

std::string OTAPI_Exec::Instrmnt_GetInstrumentDefinitionID(
    const std::string& THE_INSTRUMENT) const
{
    if (THE_INSTRUMENT.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: THE_INSTRUMENT passed in!")
            .Flush();
        return {};
    }
    const auto strInstrument = String::Factory(THE_INSTRUMENT);
    auto thePayment{api_.Factory().Payment(strInstrument)};

    if (!thePayment->IsValid()) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to parse instrument: ")(
            strInstrument)(".")
            .Flush();
        return {};
    }
    const bool bSetValues = thePayment->SetTempValues();
    if (!bSetValues) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to load instrument: ")(
            strInstrument)(".")
            .Flush();
        return {};
    }

    // BY THIS POINT, we have definitely loaded up all the values of the
    // instrument
    // into the OTPayment object. (Meaning we can now return the requested
    // data...)

    auto theOutput = Identifier::Factory();
    const bool bGotData =
        thePayment->GetInstrumentDefinitionID(theOutput);  // <========

    if (bGotData) {
        const auto strOutput = String::Factory(theOutput);
        std::string pBuf = strOutput->Get();
        return pBuf;
    }

    return {};
}

std::string OTAPI_Exec::Instrmnt_GetRemitterNymID(
    const std::string& THE_INSTRUMENT) const
{
    if (THE_INSTRUMENT.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: THE_INSTRUMENT passed in!")
            .Flush();
        return {};
    }
    const auto strInstrument = String::Factory(THE_INSTRUMENT);
    auto thePayment{api_.Factory().Payment(strInstrument)};

    if (!thePayment->IsValid()) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to parse instrument: ")(
            strInstrument)(".")
            .Flush();
        return {};
    }
    const bool bSetValues = thePayment->SetTempValues();
    if (!bSetValues) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to load instrument: ")(
            strInstrument)(".")
            .Flush();
        return {};
    }

    // BY THIS POINT, we have definitely loaded up all the values of the
    // instrument
    // into the OTPayment object. (Meaning we can now return the requested
    // data...)

    auto theOutput = Identifier::Factory();
    const bool bGotData = thePayment->GetRemitterNymID(theOutput);  // <========

    if (bGotData) {
        const auto strOutput = String::Factory(theOutput);
        std::string pBuf = strOutput->Get();
        return pBuf;
    }

    return {};
}

std::string OTAPI_Exec::Instrmnt_GetRemitterAcctID(
    const std::string& THE_INSTRUMENT) const
{
    if (THE_INSTRUMENT.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: THE_INSTRUMENT passed in!")
            .Flush();
        return {};
    }
    const auto strInstrument = String::Factory(THE_INSTRUMENT);
    auto thePayment{api_.Factory().Payment(strInstrument)};

    if (!thePayment->IsValid()) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to parse instrument: ")(
            strInstrument)(".")
            .Flush();
        return {};
    }
    const bool bSetValues = thePayment->SetTempValues();
    if (!bSetValues) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to load instrument: ")(
            strInstrument)(".")
            .Flush();
        return {};
    }

    // BY THIS POINT, we have definitely loaded up all the values of the
    // instrument
    // into the OTPayment object. (Meaning we can now return the requested
    // data...)

    auto theOutput = Identifier::Factory();
    const bool bGotData =
        thePayment->GetRemitterAcctID(theOutput);  // <========

    if (bGotData) {
        const auto strOutput = String::Factory(theOutput);
        std::string pBuf = strOutput->Get();
        return pBuf;
    }

    return {};
}

std::string OTAPI_Exec::Instrmnt_GetSenderNymID(
    const std::string& THE_INSTRUMENT) const
{
    if (THE_INSTRUMENT.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: THE_INSTRUMENT passed in!")
            .Flush();
        return {};
    }
    const auto strInstrument = String::Factory(THE_INSTRUMENT);
    auto thePayment{api_.Factory().Payment(strInstrument)};

    if (!thePayment->IsValid()) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to parse instrument: ")(
            strInstrument)(".")
            .Flush();
        return {};
    }
    const bool bSetValues = thePayment->SetTempValues();
    if (!bSetValues) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to load instrument: ")(
            strInstrument)(".")
            .Flush();
        return {};
    }

    // BY THIS POINT, we have definitely loaded up all the values of the
    // instrument
    // into the OTPayment object. (Meaning we can now return the requested
    // data...)

    auto theOutput = Identifier::Factory();
    const bool bGotData = thePayment->GetSenderNymID(theOutput);  // <========

    if (bGotData) {
        const auto strOutput = String::Factory(theOutput);
        std::string pBuf = strOutput->Get();
        return pBuf;
    }

    return {};
}

std::string OTAPI_Exec::Instrmnt_GetSenderAcctID(
    const std::string& THE_INSTRUMENT) const
{
    if (THE_INSTRUMENT.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: THE_INSTRUMENT passed in!")
            .Flush();
        return {};
    }
    const auto strInstrument = String::Factory(THE_INSTRUMENT);
    auto thePayment{api_.Factory().Payment(strInstrument)};

    if (!thePayment->IsValid()) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to parse instrument: ")(
            strInstrument)(".")
            .Flush();
        return {};
    }
    const bool bSetValues = thePayment->SetTempValues();
    if (!bSetValues) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to load instrument: ")(
            strInstrument)(".")
            .Flush();
        return {};
    }

    // BY THIS POINT, we have definitely loaded up all the values of the
    // instrument
    // into the OTPayment object. (Meaning we can now return the requested
    // data...)

    auto theOutput = Identifier::Factory();
    const bool bGotData = thePayment->GetSenderAcctID(theOutput);  // <========

    if (bGotData) {
        const auto strOutput = String::Factory(theOutput);
        std::string pBuf = strOutput->Get();
        return pBuf;
    }

    return {};
}

std::string OTAPI_Exec::Instrmnt_GetRecipientNymID(
    const std::string& THE_INSTRUMENT) const
{
    if (THE_INSTRUMENT.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: THE_INSTRUMENT passed in!")
            .Flush();
        return {};
    }
    const auto strInstrument = String::Factory(THE_INSTRUMENT);
    auto thePayment{api_.Factory().Payment(strInstrument)};

    if (!thePayment->IsValid()) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to parse instrument: ")(
            strInstrument)(".")
            .Flush();
        return {};
    }
    const bool bSetValues = thePayment->SetTempValues();
    if (!bSetValues) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to load instrument: ")(
            strInstrument)(".")
            .Flush();
        return {};
    }

    // BY THIS POINT, we have definitely loaded up all the values of the
    // instrument
    // into the OTPayment object. (Meaning we can now return the requested
    // data...)

    auto theOutput = Identifier::Factory();
    const bool bGotData =
        thePayment->GetRecipientNymID(theOutput);  // <========

    if (bGotData) {
        const auto strOutput = String::Factory(theOutput);
        std::string pBuf = strOutput->Get();
        return pBuf;
    }

    return {};
}

std::string OTAPI_Exec::Instrmnt_GetRecipientAcctID(
    const std::string& THE_INSTRUMENT) const
{
    if (THE_INSTRUMENT.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: THE_INSTRUMENT passed in!")
            .Flush();
        return {};
    }
    const auto strInstrument = String::Factory(THE_INSTRUMENT);
    auto thePayment{api_.Factory().Payment(strInstrument)};

    if (!thePayment->IsValid()) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to parse instrument: ")(
            strInstrument)(".")
            .Flush();
        return {};
    }
    const bool bSetValues = thePayment->SetTempValues();
    if (!bSetValues) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to load instrument: ")(
            strInstrument)(".")
            .Flush();
        return {};
    }
    // BY THIS POINT, we have definitely loaded up all the values of the
    // instrument
    // into the OTPayment object. (Meaning we can now return the requested
    // data...)

    auto theOutput = Identifier::Factory();
    const bool bGotData =
        thePayment->GetRecipientAcctID(theOutput);  // <========

    if (bGotData) {
        const auto strOutput = String::Factory(theOutput);
        std::string pBuf = strOutput->Get();
        return pBuf;
    }

    return {};
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
bool OTAPI_Exec::SetNym_Alias(
    const std::string& targetNymID,
    const std::string& walletNymID,
    const std::string& name) const
{
    if (targetNymID.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: targetNymID passed in!")
            .Flush();
        return false;
    }
    if (walletNymID.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: walletNymID passed in!")
            .Flush();
        return false;
    }
    if (name.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: name passed in!").Flush();
        return false;
    }

    const bool bSuccess = ot_api_.SetNym_Alias(
        Identifier::Factory(targetNymID),
        Identifier::Factory(walletNymID),
        String::Factory(name));

    return bSuccess;
}

bool OTAPI_Exec::Rename_Nym(
    const std::string& nymID,
    const std::string& name,
    const proto::ContactItemType type,
    const bool primary) const
{
    if (nymID.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: targetNymID passed in!")
            .Flush();
        return false;
    }

    if (name.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: walletNymID passed in!")
            .Flush();
        return false;
    }

    return ot_api_.Rename_Nym(Identifier::Factory(nymID), name, type, primary);
}

// Merely a client-side label
bool OTAPI_Exec::SetServer_Name(
    const std::string& NOTARY_ID,
    const std::string& STR_NEW_NAME) const
{
    OT_ASSERT_MSG(
        !NOTARY_ID.empty(),
        "OTAPI_Exec::SetServer_Name: Null NOTARY_ID passed in.");
    if (STR_NEW_NAME.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null STR_NEW_NAME passed in!")
            .Flush();
        return false;
    }

    const bool bSuccess = api_.Wallet().SetServerAlias(
        Identifier::Factory(NOTARY_ID), STR_NEW_NAME);

    return bSuccess;
}

// Merely a client-side label
bool OTAPI_Exec::SetAssetType_Name(
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& STR_NEW_NAME) const
{
    OT_ASSERT_MSG(
        !INSTRUMENT_DEFINITION_ID.empty(),
        "OTAPI_Exec::SetAssetType_"
        "Name: Null "
        "INSTRUMENT_DEFINITION_ID "
        "passed in.");
    if (STR_NEW_NAME.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: STR_NEW_NAME passed in!")
            .Flush();
        return false;
    }

    const bool bSuccess = api_.Wallet().SetUnitDefinitionAlias(
        Identifier::Factory(INSTRUMENT_DEFINITION_ID), STR_NEW_NAME);

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
std::int32_t OTAPI_Exec::GetNym_TransactionNumCount(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const
{
    OT_ASSERT_MSG(
        !NOTARY_ID.empty(),
        "OTAPI_Exec::GetNym_TransactionNumCount: Null NOTARY_ID passed in.");
    OT_ASSERT_MSG(
        !NYM_ID.empty(),
        "OTAPI_Exec::GetNym_TransactionNumCount: Null NYM_ID passed in.");

    auto theNotaryID = Identifier::Factory(NOTARY_ID);
    auto theNymID = Identifier::Factory(NYM_ID);

    auto context = api_.Wallet().ServerContext(theNymID, theNotaryID);

    if (context) { return context->AvailableNumbers(); }

    return -1;
}

// based on Index (above 4 functions) this returns the Server's ID
std::string OTAPI_Exec::GetServer_ID(const std::int32_t& nIndex) const
{
    if (0 > nIndex) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": nIndex is out of bounds (it's in the negative)!")
            .Flush();
        return {};
    }

    std::uint32_t index(nIndex);
    auto servers = api_.Wallet().ServerList();

    if (index <= servers.size()) {
        ObjectList::iterator it = servers.begin();
        std::advance(it, index);
        return it->first;
    }

    return {};
}

// Return's Server's name (based on server ID)
std::string OTAPI_Exec::GetServer_Name(const std::string& THE_ID) const
{
    auto pServer = api_.Wallet().Server(Identifier::Factory(THE_ID));

    if (!pServer) { return {}; }

    return pServer->Alias();
}

// returns Instrument Definition ID (based on index from GetAssetTypeCount)
std::string OTAPI_Exec::GetAssetType_ID(const std::int32_t& nIndex) const
{
    if (0 > nIndex) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": nIndex is out of bounds (it's in the negative)!")
            .Flush();
        return {};
    }

    std::uint32_t index(nIndex);
    auto units = api_.Wallet().UnitDefinitionList();

    if (index <= units.size()) {
        ObjectList::iterator it = units.begin();
        std::advance(it, index);
        return it->first;
    }

    return {};
}

// Returns instrument definition Name based on Instrument Definition ID
std::string OTAPI_Exec::GetAssetType_Name(const std::string& THE_ID) const
{
    auto pContract = api_.Wallet().UnitDefinition(Identifier::Factory(THE_ID));

    if (!pContract) { return {}; }

    return pContract->Alias();
}

// Returns instrument definition TLA based on Instrument Definition ID
std::string OTAPI_Exec::GetAssetType_TLA(const std::string& THE_ID) const
{
    auto unit = api_.Wallet().UnitDefinition(Identifier::Factory(THE_ID));

    if (!unit) return {};

    return unit->TLA();
}

// returns the account name, based on account ID.
std::string OTAPI_Exec::GetAccountWallet_Name(const std::string& THE_ID) const
{
    OT_ASSERT_MSG(
        !THE_ID.empty(),
        "OTAPI_Exec::GetAccountWallet_Name: Null THE_ID passed in.");

    auto theID = Identifier::Factory(THE_ID);
    auto account = api_.Wallet().Account(theID);

    if (false == bool(account)) { return {}; }

    auto strName = String::Factory();
    account.get().GetName(strName);

    return strName->Get();
}

/** TIME (in seconds, as string)

This will return the current time in seconds, as a string.
Returns "" if failure.

Todo:  consider making this available on the server side as well,
so the smart contracts can see what time it is.

*/
time64_t OTAPI_Exec::GetTime(void) const { return ot_api_.GetTime(); }

/** OT-encode a plaintext string.  (NOT ENCRYPT)

std::string OTAPI_Exec::Encode(const std::string& strPlaintext);

This will pack, compress, and base64-encode a plain string.
Returns the base64-encoded string, or "".

Internally:
OTString        strPlain(strPlaintext);
Armored    ascEncoded(thePlaintext);    // ascEncoded now contains the
OT-encoded string.
return            ascEncoded.Get();            // We return it.
*/
std::string OTAPI_Exec::Encode(
    const std::string& strPlaintext,
    const bool& bLineBreaks) const  // bLineBreaks should
                                    // usually be set to true.
{
    if (strPlaintext.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: strPlaintext passed in!")
            .Flush();
        return {};
    }

    const auto otstrPlaintext = String::Factory(strPlaintext);
    auto strOutput = String::Factory();

    bool bEncoded = ot_api_.Encode(
        otstrPlaintext, strOutput, (true == bLineBreaks) ? true : false);

    if (!bEncoded) return {};

    std::string pBuf = strOutput->Get();

    return pBuf;
}

/** Decode an OT-encoded string (back to plaintext.)  (NOT DECRYPT)

std::string OTAPI_Exec::Decode(const std::string& strEncoded);

This will base64-decode, uncompress, and unpack an OT-encoded string.
Returns the plaintext string, or "".

Internally:
Armored    ascEncoded(strEncoded);
OTString        strPlain(ascEncoded);    // strPlain now contains the decoded
plaintext string.
return            strPlain.Get();            // We return it.
*/
std::string OTAPI_Exec::Decode(
    const std::string& strEncoded,
    const bool& bLineBreaks) const
{
    if (strEncoded.empty()) { return {}; }

    const auto otstrEncoded = String::Factory(strEncoded);
    auto strOutput = String::Factory();

    bool bDecoded = ot_api_.Decode(
        otstrEncoded, strOutput, (true == bLineBreaks) ? true : false);

    if (!bDecoded) return {};

    std::string pBuf = strOutput->Get();

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
Armored    ascCiphertext(theEnvelope);        // ascCiphertext now contains
the base64-encoded ciphertext (as a string.)
return ascCiphertext.Get();
}
*/
std::string OTAPI_Exec::Encrypt(
    const std::string& RECIPIENT_NYM_ID,
    const std::string& strPlaintext) const
{
    OT_ASSERT_MSG(
        !RECIPIENT_NYM_ID.empty(),
        "OTAPI_Exec::Encrypt: Null RECIPIENT_NYM_ID passed in.");
    OT_ASSERT_MSG(
        !strPlaintext.empty(),
        "OTAPI_Exec::Encrypt: Null strPlaintext passed in.");

    const auto otstrPlaintext = String::Factory(strPlaintext);
    const auto theRecipientNymID = Identifier::Factory(RECIPIENT_NYM_ID);
    auto strOutput = String::Factory();

    bool bEncrypted =
        ot_api_.Encrypt(theRecipientNymID, otstrPlaintext, strOutput);

    if (!bEncrypted || !strOutput->Exists()) return {};

    std::string pBuf = strOutput->Get();

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
Armored    ascCiphertext(strCiphertext);    // The base64-encoded
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
    OT_ASSERT_MSG(
        !RECIPIENT_NYM_ID.empty(),
        "OTAPI_Exec::Decrypt: Null RECIPIENT_NYM_ID passed in.");
    OT_ASSERT_MSG(
        !strCiphertext.empty(),
        "OTAPI_Exec::Decrypt: Null strCiphertext passed in.");

    const auto otstrCiphertext = String::Factory(strCiphertext);
    const auto theRecipientNymID = Identifier::Factory(RECIPIENT_NYM_ID);
    auto strOutput = String::Factory();

    bool bDecrypted =
        ot_api_.Decrypt(theRecipientNymID, otstrCiphertext, strOutput);

    if (!bDecrypted || !strOutput->Exists()) return {};

    std::string pBuf = strOutput->Get();

    return pBuf;
}

// SYMMETRIC

// Generates a new symmetric key, based on a passphrase,
// and returns it (or "".)
//
std::string OTAPI_Exec::CreateSymmetricKey() const
{
    auto strOutput = String::Factory();
    std::string strDisplay = "OTAPI: Creating a new symmetric key.";
    const auto otstrDisplay = String::Factory(strDisplay);
    const bool bSuccess = crypto::key::LegacySymmetric::CreateNewKey(
        api_.Crypto(), strOutput, otstrDisplay);  // pAlreadyHavePW=""

    if (!bSuccess) return {};

    std::string pBuf = strOutput->Get();

    return pBuf;
}

// OTEnvelope:
//     bool Encrypt(const OTString& theInput, crypto::key::LegacySymmetric&
//     theKey,
// const OTPassword& thePassword);
//     bool Decrypt(OTString& theOutput, const crypto::key::LegacySymmetric&
//     theKey,
// const OTPassword& thePassword);

// Returns the CIPHERTEXT_ENVELOPE (the Envelope encrypted with the Symmetric
// Key.)
//
std::string OTAPI_Exec::SymmetricEncrypt(
    const std::string& SYMMETRIC_KEY,
    const std::string& PLAINTEXT) const
{
    OT_ASSERT_MSG(
        !SYMMETRIC_KEY.empty(),
        "OTAPI_Exec::SymmetricEncrypt: Null SYMMETRIC_KEY passed in.");
    OT_ASSERT_MSG(
        !PLAINTEXT.empty(),
        "OTAPI_Exec::SymmetricEncrypt: Null PLAINTEXT passed in.");

    const auto strKey = String::Factory(SYMMETRIC_KEY);
    const auto strPlaintext = String::Factory(PLAINTEXT);
    auto strOutput = String::Factory();
    std::string strDisplay = "OTAPI: Password-protecting a plaintext.";
    const auto otstrDisplay = String::Factory(strDisplay);
    const bool bSuccess = crypto::key::LegacySymmetric::Encrypt(
        api_.Crypto(),
        strKey,
        strPlaintext,
        strOutput,
        otstrDisplay);  // bBookends=true, pAlreadyHavePW=""

    if (!bSuccess) return {};

    std::string pBuf = strOutput->Get();

    return pBuf;
}

// Returns the PLAINTEXT.
//
std::string OTAPI_Exec::SymmetricDecrypt(
    const std::string& SYMMETRIC_KEY,
    const std::string& CIPHERTEXT_ENVELOPE) const
{
    OT_ASSERT_MSG(
        !SYMMETRIC_KEY.empty(),
        "OTAPI_Exec::SymmetricDecrypt: Null SYMMETRIC_KEY passed in.");
    OT_ASSERT_MSG(
        !CIPHERTEXT_ENVELOPE.empty(),
        "OTAPI_Exec::SymmetricDecrypt: Null CIPHERTEXT_ENVELOPE passed in.");

    const auto strKey = String::Factory(SYMMETRIC_KEY);
    auto strCiphertext = String::Factory(CIPHERTEXT_ENVELOPE);
    auto strOutput = String::Factory();
    std::string strDisplay =
        "OTAPI: Decrypting a password-protected ciphertext.";
    const auto otstrDisplay = String::Factory(strDisplay);
    const bool bSuccess = crypto::key::LegacySymmetric::Decrypt(
        api_.Crypto(),
        strKey,
        strCiphertext,
        strOutput,
        otstrDisplay);  // pAlreadyHavePW=""

    if (!bSuccess) return {};

    std::string pBuf = strOutput->Get();

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
    OT_ASSERT_MSG(
        !SIGNER_NYM_ID.empty(),
        "OTAPI_Exec::SignContract: Null SIGNER_NYM_ID passed in.");
    OT_ASSERT_MSG(
        !THE_CONTRACT.empty(),
        "OTAPI_Exec::SignContract: Null THE_CONTRACT passed in.");

    const auto strContract = String::Factory(THE_CONTRACT);
    const auto theSignerNymID = Identifier::Factory(SIGNER_NYM_ID);
    auto strOutput = String::Factory();

    const bool bSigned =
        ot_api_.SignContract(theSignerNymID, strContract, strOutput);

    if (!bSigned || !strOutput->Exists()) return {};

    std::string pBuf = strOutput->Get();

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
    OT_ASSERT_MSG(
        !SIGNER_NYM_ID.empty(),
        "OTAPI_Exec::FlatSign: Null SIGNER_NYM_ID passed in.");
    OT_ASSERT_MSG(
        !THE_INPUT.empty(), "OTAPI_Exec::FlatSign: Null THE_INPUT passed in.");
    OT_ASSERT_MSG(
        !CONTRACT_TYPE.empty(),
        "OTAPI_Exec::FlatSign: Null CONTRACT_TYPE passed in.");

    const auto strContract = String::Factory(THE_INPUT);
    const auto strContractType = String::Factory(CONTRACT_TYPE);
    const auto theSignerNymID = Identifier::Factory(SIGNER_NYM_ID);
    auto strOutput = String::Factory();

    const bool bSigned = ot_api_.FlatSign(
        theSignerNymID, strContract, strContractType, strOutput);

    if (!bSigned || !strOutput->Exists()) return {};

    std::string pBuf = strOutput->Get();

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
    OT_ASSERT_MSG(
        !SIGNER_NYM_ID.empty(),
        "OTAPI_Exec::AddSignature: Null SIGNER_NYM_ID passed in.");
    OT_ASSERT_MSG(
        !THE_CONTRACT.empty(),
        "OTAPI_Exec::AddSignature: Null THE_CONTRACT passed in.");

    const auto strContract = String::Factory(THE_CONTRACT);
    const auto theSignerNymID = Identifier::Factory(SIGNER_NYM_ID);
    auto strOutput = String::Factory();

    const bool bSigned =
        ot_api_.AddSignature(theSignerNymID, strContract, strOutput);

    if (!bSigned || !strOutput->Exists()) return {};

    std::string pBuf = strOutput->Get();

    return pBuf;
}

/** OT-Verify the signature on a CONTRACT stored in a string.

Returns bool -- true (1) or false (0)
*/
bool OTAPI_Exec::VerifySignature(
    const std::string& SIGNER_NYM_ID,
    const std::string& THE_CONTRACT) const
{
    OT_ASSERT_MSG(
        !SIGNER_NYM_ID.empty(),
        "OTAPI_Exec::VerifySignature: Null SIGNER_NYM_ID passed in.");
    OT_ASSERT_MSG(
        !THE_CONTRACT.empty(),
        "OTAPI_Exec::VerifySignature: Null THE_CONTRACT passed in.");

    const auto strContract = String::Factory(THE_CONTRACT);
    const auto theNymID = Identifier::Factory(SIGNER_NYM_ID);
    const bool bVerified = ot_api_.VerifySignature(
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
    OT_ASSERT_MSG(
        !THE_CONTRACT.empty(),
        "OTAPI_Exec::"
        "VerifyAndRetrieveXMLContents: Null "
        "THE_CONTRACT passed in.");
    OT_ASSERT_MSG(
        !SIGNER_ID.empty(),
        "OTAPI_Exec::VerifyAndRetrieveXMLContents: Null SIGNER_ID passed in.");

    const auto strContract = String::Factory(THE_CONTRACT);
    const auto theSignerID = Identifier::Factory(SIGNER_ID);
    auto strOutput = String::Factory();

    if (false == ot_api_.VerifyAndRetrieveXMLContents(
                     strContract, theSignerID, strOutput)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failure: "
            "ot_api_.VerifyAndRetrieveXMLContents() "
            "returned false.")
            .Flush();
        return {};
    }
    std::string pBuf = strOutput->Get();

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
    OT_ASSERT_MSG(
        !NOTARY_ID.empty(),
        "OTAPI_Exec::VerifyAccountReceipt: Null NOTARY_ID passed in.");
    OT_ASSERT_MSG(
        !NYM_ID.empty(),
        "OTAPI_Exec::VerifyAccountReceipt: Null NYM_ID passed in.");
    OT_ASSERT_MSG(
        !ACCT_ID.empty(),
        "OTAPI_Exec::VerifyAccountReceipt: Null ACCT_ID passed in.");

    return ot_api_.VerifyAccountReceipt(
        Identifier::Factory(NOTARY_ID),
        Identifier::Factory(NYM_ID),
        Identifier::Factory(ACCT_ID));
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
    OT_ASSERT_MSG(
        !ACCT_ID.empty(),
        "OTAPI_Exec::SetAccountWallet_Name: Null ACCT_ID passed in.");
    OT_ASSERT_MSG(
        !SIGNER_NYM_ID.empty(),
        "OTAPI_Exec::SetAccountWallet_Name: Null SIGNER_NYM_ID passed in.");

    return ot_api_.SetAccount_Name(
        Identifier::Factory(ACCT_ID),
        Identifier::Factory(SIGNER_NYM_ID),
        String::Factory(ACCT_NEW_NAME));
}

// returns the account balance, based on account ID.
std::int64_t OTAPI_Exec::GetAccountWallet_Balance(
    const std::string& THE_ID) const
{
    OT_ASSERT_MSG(
        !THE_ID.empty(),
        "OTAPI_Exec::GetAccountWallet_Balance: Null THE_ID passed in.");

    auto theID = Identifier::Factory(THE_ID);
    auto account = api_.Wallet().Account(theID);
    return (account) ? account.get().GetBalance() : OT_ERROR_AMOUNT;
}

// returns an account's "account type", (simple, issuer, etc.)
std::string OTAPI_Exec::GetAccountWallet_Type(const std::string& THE_ID) const
{
    OT_ASSERT_MSG(
        !THE_ID.empty(),
        "OTAPI_Exec::GetAccountWallet_Type: Null THE_ID passed in.");

    auto theID = Identifier::Factory(THE_ID);
    auto account = api_.Wallet().Account(theID);

    if (false == bool(account)) return {};

    return account.get().GetTypeString();
}

// Returns an account's instrument definition ID.
// (Which is a hash of the contract used to issue the instrument definition.)
std::string OTAPI_Exec::GetAccountWallet_InstrumentDefinitionID(
    const std::string& THE_ID) const
{
    OT_VERIFY_ID_STR(THE_ID);

    auto theID = Identifier::Factory(THE_ID);
    auto account = api_.Wallet().Account(theID);

    if (false == bool(account)) return {};

    auto theInstrumentDefinitionID =
        Identifier::Factory(account.get().GetInstrumentDefinitionID());
    LogDetail(OT_METHOD)(__FUNCTION__)(": Returning instrument definition ")(
        theInstrumentDefinitionID)(" for account ")(THE_ID)
        .Flush();

    return theInstrumentDefinitionID->str();
}

// Returns an account's Notary ID.
// (Which is a hash of the server contract.)
std::string OTAPI_Exec::GetAccountWallet_NotaryID(
    const std::string& THE_ID) const
{
    OT_VERIFY_ID_STR(THE_ID);

    auto theID = Identifier::Factory(THE_ID);
    auto account = api_.Wallet().Account(theID);

    if (false == bool(account)) return {};

    auto theNotaryID =
        Identifier::Factory(account.get().GetPurportedNotaryID());

    return theNotaryID->str();
}

// Returns an account's Nym ID.
// (Which is a hash of the Nym's public key for the owner of this account.)
std::string OTAPI_Exec::GetAccountWallet_NymID(const std::string& THE_ID) const
{
    OT_VERIFY_ID_STR(THE_ID);

    const auto theID = Identifier::Factory(THE_ID);
    auto account = api_.Wallet().Account(theID);

    if (false == bool(account)) return {};

    auto theNymID = Identifier::Factory(account.get().GetNymID());

    return theNymID->str();
}

/*

WRITE A CHEQUE  --- (Returns the cheque in string form.)

==> OTAPI_Exec::WriteCheque() internally constructs an Cheque and issues it,
like so:

Cheque theCheque( NOTARY_ID, INSTRUMENT_DEFINITION_ID );

theCheque.IssueCheque( AMOUNT // The amount of the cheque, in string form, which
OTAPI
// will convert to a std::int64_t integer. Negative amounts
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
    const std::int64_t& CHEQUE_AMOUNT,
    const time64_t& VALID_FROM,
    const time64_t& VALID_TO,
    const std::string& SENDER_ACCT_ID,
    const std::string& SENDER_NYM_ID,
    const std::string& CHEQUE_MEMO,
    const std::string& RECIPIENT_NYM_ID) const
{

    OT_VERIFY_ID_STR(NOTARY_ID);
    // Positive and negative numbers are both allowed.
    // (Negative amount on a cheque makes it an invoice).
    // Only zero is disallowed.
    // (Why would you pay zero other than to spam somebody?)
    //
    if (0 == CHEQUE_AMOUNT) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: CHEQUE_AMOUNT passed in!")
            .Flush();
        return {};
    }
    OT_VERIFY_MIN_BOUND(VALID_FROM, OT_TIME_ZERO);
    OT_VERIFY_MIN_BOUND(VALID_TO, OT_TIME_ZERO);
    OT_VERIFY_ID_STR(SENDER_ACCT_ID);
    OT_VERIFY_ID_STR(SENDER_NYM_ID);
    //  OT_VERIFY_STD_STR(CHEQUE_MEMO.empty); // Optional
    //  OT_VERIFY_ID_STR(RECIPIENT_NYM_ID); // Optional.

    const std::int64_t lAmount = CHEQUE_AMOUNT;
    const time64_t time_From = static_cast<time64_t>(VALID_FROM),
                   time_To = static_cast<time64_t>(VALID_TO);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID);
    const auto theSenderAcctID = Identifier::Factory(SENDER_ACCT_ID);
    const auto theSenderNymID = Identifier::Factory(SENDER_NYM_ID);
    auto theRecipientNymID = Identifier::Factory();
    bool bHasRecipient = !RECIPIENT_NYM_ID.empty();
    if (bHasRecipient)
        theRecipientNymID->SetString(String::Factory(RECIPIENT_NYM_ID));
    auto strMemo = String::Factory();

    if (!CHEQUE_MEMO.empty()) strMemo->Set(String::Factory(CHEQUE_MEMO));

    OTIdentifier idForRecipient(
        bHasRecipient ? Identifier::Factory(theRecipientNymID)
                      : Identifier::Factory());

    std::unique_ptr<Cheque> pCheque(ot_api_.WriteCheque(
        theNotaryID,
        static_cast<std::int64_t>(lAmount),
        time_From,
        time_To,
        theSenderAcctID,
        theSenderNymID,
        strMemo,
        idForRecipient));

    if (!pCheque) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": OT_API::WriteCheque failed.")
            .Flush();
        return {};
    }
    // At this point, I know pCheque is good (and will be cleaned up
    // automatically.)

    auto strCheque =
        String::Factory(*pCheque);  // Extract the cheque to string form.
    std::string pBuf = strCheque->Get();

    return pBuf;
}

bool OTAPI_Exec::DiscardCheque(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCT_ID,
    const std::string& THE_CHEQUE) const
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(ACCT_ID);
    OT_VERIFY_STD_STR(THE_CHEQUE);

    auto theNotaryID = Identifier::Factory(NOTARY_ID),
         theNymID = Identifier::Factory(NYM_ID),
         theAcctID = Identifier::Factory(ACCT_ID);
    auto strCheque = String::Factory(THE_CHEQUE);

    return ot_api_.DiscardCheque(theNotaryID, theNymID, theAcctID, strCheque);
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
    const std::int64_t& INITIAL_PAYMENT_AMOUNT,  // zero or "" is no initial
                                                 // payment.
    const time64_t& INITIAL_PAYMENT_DELAY,       // seconds from creation date.
                                                 // Default is zero or "".
    const std::int64_t& PAYMENT_PLAN_AMOUNT,     // Zero or "" is no regular
                                                 // payments.
    const time64_t& PAYMENT_PLAN_DELAY,   // No. of seconds from creation date.
                                          // Default is zero or "".
    const time64_t& PAYMENT_PLAN_PERIOD,  // No. of seconds between payments.
                                          // Default is zero or "".
    const time64_t& PAYMENT_PLAN_LENGTH,  // In seconds. Defaults to 0 or "" (no
                                          // maximum length.)
    const std::int32_t& PAYMENT_PLAN_MAX_PAYMENTS  // Integer. Defaults to 0 or
                                                   // ""
                                                   // (no
                                                   // maximum payments.)
    ) const
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(SENDER_NYM_ID);
    // NOTE: Making this optional for this step. (Since sender account may
    // not be known until the customer receives / confirms the payment plan.)
    //  OT_VERIFY_ID_STR(SENDER_ACCT_ID); // Optional parameter.
    OT_VERIFY_ID_STR(RECIPIENT_NYM_ID);
    OT_VERIFY_ID_STR(RECIPIENT_ACCT_ID);
    OT_VERIFY_STD_STR(PLAN_CONSIDERATION);
    OT_VERIFY_MIN_BOUND(VALID_FROM, OT_TIME_ZERO);
    OT_VERIFY_MIN_BOUND(VALID_TO, OT_TIME_ZERO);
    OT_VERIFY_MIN_BOUND(INITIAL_PAYMENT_AMOUNT, 0);
    OT_VERIFY_MIN_BOUND(INITIAL_PAYMENT_DELAY, OT_TIME_ZERO);
    OT_VERIFY_MIN_BOUND(PAYMENT_PLAN_AMOUNT, 0);
    OT_VERIFY_MIN_BOUND(PAYMENT_PLAN_DELAY, OT_TIME_ZERO);
    OT_VERIFY_MIN_BOUND(PAYMENT_PLAN_PERIOD, OT_TIME_ZERO);
    OT_VERIFY_MIN_BOUND(PAYMENT_PLAN_LENGTH, OT_TIME_ZERO);
    OT_VERIFY_MIN_BOUND(PAYMENT_PLAN_MAX_PAYMENTS, 0);

    OTIdentifier angelSenderAcctId = Identifier::Factory();

    if (!SENDER_ACCT_ID.empty())
        angelSenderAcctId = Identifier::Factory(SENDER_ACCT_ID);

    std::unique_ptr<OTPaymentPlan> pPlan(ot_api_.ProposePaymentPlan(
        Identifier::Factory(NOTARY_ID),
        VALID_FROM,  // Default (0) == NOW
        VALID_TO,    // Default (0) == no expiry / cancel anytime
                     // We're making this acct optional here.
        // (Customer acct is unknown until confirmation by customer.)
        angelSenderAcctId.get(),
        Identifier::Factory(SENDER_NYM_ID),
        PLAN_CONSIDERATION.empty()
            ? String::Factory("(Consideration for the agreement between "
                              "the parties is meant to be recorded "
                              "here.)")
            // Like a memo.
            : String::Factory(PLAN_CONSIDERATION),
        Identifier::Factory(RECIPIENT_ACCT_ID),
        Identifier::Factory(RECIPIENT_NYM_ID),
        static_cast<std::int64_t>(INITIAL_PAYMENT_AMOUNT),
        INITIAL_PAYMENT_DELAY,
        static_cast<std::int64_t>(PAYMENT_PLAN_AMOUNT),
        PAYMENT_PLAN_DELAY,
        PAYMENT_PLAN_PERIOD,
        PAYMENT_PLAN_LENGTH,
        static_cast<std::int32_t>(PAYMENT_PLAN_MAX_PAYMENTS)));

    if (!pPlan) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed in OTAPI_Exec::ProposePaymentPlan.")
            .Flush();
        return {};
    }

    return String::Factory(*pPlan)->Get();
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
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(SENDER_NYM_ID);
    //  OT_VERIFY_ID_STR(SENDER_ACCT_ID); // Optional parameter.
    OT_VERIFY_ID_STR(RECIPIENT_NYM_ID);
    OT_VERIFY_ID_STR(RECIPIENT_ACCT_ID);
    OT_VERIFY_STD_STR(PLAN_CONSIDERATION);

    time64_t VALID_FROM = OT_TIME_ZERO;
    time64_t VALID_TO = OT_TIME_ZERO;
    std::int64_t INITIAL_PAYMENT_AMOUNT = 0;
    time64_t INITIAL_PAYMENT_DELAY = OT_TIME_ZERO;
    std::int64_t PAYMENT_PLAN_AMOUNT = 0;
    time64_t PAYMENT_PLAN_DELAY = OT_TIME_ZERO;
    time64_t PAYMENT_PLAN_PERIOD = OT_TIME_ZERO;
    time64_t PAYMENT_PLAN_LENGTH = OT_TIME_ZERO;
    std::int32_t PAYMENT_PLAN_MAX_PAYMENTS = 0;
    if (!DATE_RANGE.empty()) {
        NumList theList;
        const auto otstrNumList = String::Factory(DATE_RANGE);
        theList.Add(otstrNumList);
        // VALID_FROM
        if (theList.Count() > 0) {
            std::int64_t lVal = 0;
            if (theList.Peek(lVal)) VALID_FROM = OTTimeGetTimeFromSeconds(lVal);
            theList.Pop();
        }
        // VALID_TO
        if (theList.Count() > 0) {
            std::int64_t lVal = 0;
            if (theList.Peek(lVal)) VALID_TO = OTTimeGetTimeFromSeconds(lVal);
            theList.Pop();
        }
    }
    if (!INITIAL_PAYMENT.empty()) {
        NumList theList;
        const auto otstrNumList = String::Factory(INITIAL_PAYMENT);
        theList.Add(otstrNumList);
        // INITIAL_PAYMENT_AMOUNT
        if (theList.Count() > 0) {
            std::int64_t lVal = 0;
            if (theList.Peek(lVal)) INITIAL_PAYMENT_AMOUNT = lVal;
            theList.Pop();
        }
        // INITIAL_PAYMENT_DELAY
        if (theList.Count() > 0) {
            std::int64_t lVal = 0;
            if (theList.Peek(lVal))
                INITIAL_PAYMENT_DELAY = OTTimeGetTimeFromSeconds(lVal);
            theList.Pop();
        }
    }
    if (!PAYMENT_PLAN.empty()) {
        NumList theList;
        const auto otstrNumList = String::Factory(PAYMENT_PLAN);
        theList.Add(otstrNumList);
        // PAYMENT_PLAN_AMOUNT
        if (theList.Count() > 0) {
            std::int64_t lVal = 0;
            if (theList.Peek(lVal)) PAYMENT_PLAN_AMOUNT = lVal;
            theList.Pop();
        }
        // PAYMENT_PLAN_DELAY
        if (theList.Count() > 0) {
            std::int64_t lVal = 0;
            if (theList.Peek(lVal))
                PAYMENT_PLAN_DELAY = OTTimeGetTimeFromSeconds(lVal);
            theList.Pop();
        }
        // PAYMENT_PLAN_PERIOD
        if (theList.Count() > 0) {
            std::int64_t lVal = 0;
            if (theList.Peek(lVal))
                PAYMENT_PLAN_PERIOD = OTTimeGetTimeFromSeconds(lVal);
            theList.Pop();
        }
    }
    if (!PLAN_EXPIRY.empty()) {
        NumList theList;
        const auto otstrNumList = String::Factory(PLAN_EXPIRY);
        theList.Add(otstrNumList);
        // PAYMENT_PLAN_LENGTH
        if (theList.Count() > 0) {
            std::int64_t lVal = 0;
            if (theList.Peek(lVal))
                PAYMENT_PLAN_LENGTH = OTTimeGetTimeFromSeconds(lVal);
            theList.Pop();
        }
        // PAYMENT_PLAN_MAX_PAYMENTS
        if (theList.Count() > 0) {
            std::int64_t lVal = 0;
            if (theList.Peek(lVal))
                PAYMENT_PLAN_MAX_PAYMENTS = static_cast<std::int32_t>(lVal);
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
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(SENDER_NYM_ID);
    OT_VERIFY_ID_STR(SENDER_ACCT_ID);
    OT_VERIFY_ID_STR(RECIPIENT_NYM_ID);
    OT_VERIFY_STD_STR(PAYMENT_PLAN);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID);
    const auto theSenderNymID = Identifier::Factory(SENDER_NYM_ID);
    const auto theSenderAcctID = Identifier::Factory(SENDER_ACCT_ID);
    const auto theRecipientNymID = Identifier::Factory(RECIPIENT_NYM_ID);

    auto thePlan{api_.Factory().PaymentPlan()};

    OT_ASSERT(false != bool(thePlan));

    const auto strPlan = String::Factory(PAYMENT_PLAN);

    if (!strPlan->Exists() ||
        (false == thePlan->LoadContractFromString(strPlan))) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failure loading payment plan from string.")
            .Flush();
        return {};
    }
    bool bConfirmed = ot_api_.ConfirmPaymentPlan(
        theNotaryID,
        theSenderNymID,
        theSenderAcctID,
        theRecipientNymID,
        *thePlan);
    if (!bConfirmed) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": failed in OTAPI_Exec::ConfirmPaymentPlan().")
            .Flush();
        return {};
    }

    auto strOutput =
        String::Factory(*thePlan);  // Extract the payment plan to string form.

    return strOutput->Get();
}

// RETURNS:  the Smart Contract itself. (Or "".)
//
std::string OTAPI_Exec::Create_SmartContract(
    const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                       // signing
    // at this postd::int32_t is only to cause a
    // save.)
    const time64_t& VALID_FROM,  // Default (0 or "") == NOW
    const time64_t& VALID_TO,    // Default (0 or "") == no expiry / cancel
                                 // anytime
    bool SPECIFY_ASSETS,  // This means asset type IDs must be provided for
                          // every named account.
    bool SPECIFY_PARTIES  // This means Nym IDs must be provided for every
                          // party.
    ) const
{
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);

    if (OT_TIME_ZERO > VALID_FROM) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Negative: VALID_FROM passed in!")
            .Flush();
        return {};
    }
    if (OT_TIME_ZERO > VALID_TO) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Negative: VALID_TO passed in!")
            .Flush();
        return {};
    }
    const auto theSignerNymID = Identifier::Factory(SIGNER_NYM_ID);
    time64_t tValidFrom = VALID_FROM;
    time64_t tValidTo = VALID_TO;
    auto strOutput = String::Factory();

    const bool bCreated = ot_api_.Create_SmartContract(
        theSignerNymID,
        tValidFrom,      // Default (0 or "") == NOW
        tValidTo,        // Default (0 or "") == no expiry / cancel anytime
        SPECIFY_ASSETS,  // This means asset type IDs must be provided for every
                         // named account.
        SPECIFY_PARTIES,  // This means Nym IDs must be provided for every
                          // party.
        strOutput);
    if (!bCreated || !strOutput->Exists()) return {};
    // Success!
    //
    return strOutput->Get();
}

bool OTAPI_Exec::Smart_ArePartiesSpecified(
    const std::string& THE_CONTRACT) const
{
    OT_VERIFY_STD_STR(THE_CONTRACT);

    const auto strContract = String::Factory(THE_CONTRACT);

    return ot_api_.Smart_ArePartiesSpecified(strContract);
}

bool OTAPI_Exec::Smart_AreAssetTypesSpecified(
    const std::string& THE_CONTRACT) const
{
    OT_VERIFY_STD_STR(THE_CONTRACT);

    const auto strContract = String::Factory(THE_CONTRACT);

    return ot_api_.Smart_AreAssetTypesSpecified(strContract);
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
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);

    if (OT_TIME_ZERO > VALID_FROM) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Negative: VALID_FROM passed in!")
            .Flush();
        return {};
    }
    if (OT_TIME_ZERO > VALID_TO) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Negative: VALID_TO passed in!")
            .Flush();
        return {};
    }
    const auto strContract = String::Factory(THE_CONTRACT);
    const auto theSignerNymID = Identifier::Factory(SIGNER_NYM_ID);
    time64_t tValidFrom = VALID_FROM;
    time64_t tValidTo = VALID_TO;
    auto strOutput = String::Factory();

    const bool bAdded = ot_api_.SmartContract_SetDates(
        strContract,     // The contract, about to have the dates changed on it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // point is only to cause a save.)
        tValidFrom,      // Default (0 or "") == NOW
        tValidTo,        // Default (0 or "") == no expiry / cancel anytime
        strOutput);
    if (!bAdded || !strOutput->Exists()) return {};
    // Success!
    //
    return strOutput->Get();
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
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);
    OT_VERIFY_STD_STR(BYLAW_NAME);

    const auto strContract = String::Factory(THE_CONTRACT),
               strBylawName = String::Factory(BYLAW_NAME);
    const auto theSignerNymID = Identifier::Factory(SIGNER_NYM_ID);
    auto strOutput = String::Factory();

    const bool bAdded = ot_api_.SmartContract_AddBylaw(
        strContract,     // The contract, about to have the bylaw added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // postd::int32_t is only to cause a save.)
        strBylawName,  // The Bylaw's NAME as referenced in the smart contract.
                       // (And the scripts...)
        strOutput);
    if (!bAdded || !strOutput->Exists()) return {};
    // Success!
    //
    return strOutput->Get();
}

// returns: the updated smart contract (or "")
std::string OTAPI_Exec::SmartContract_RemoveBylaw(
    const std::string& THE_CONTRACT,   // The contract, about to have the bylaw
                                       // removed from it.
    const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                       // signing
    // at this postd::int32_t is only to cause a
    // save.)
    const std::string& BYLAW_NAME) const  // The Bylaw's NAME as referenced in
                                          // the
// smart contract. (And the scripts...)
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);
    OT_VERIFY_STD_STR(BYLAW_NAME);

    const auto strContract = String::Factory(THE_CONTRACT),
               strBylawName = String::Factory(BYLAW_NAME);
    const auto theSignerNymID = Identifier::Factory(SIGNER_NYM_ID);
    auto strOutput = String::Factory();

    const bool bAdded = ot_api_.SmartContract_RemoveBylaw(
        strContract,     // The contract, about to have the bylaw added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // postd::int32_t is only to cause a save.)
        strBylawName,  // The Bylaw's NAME as referenced in the smart contract.
                       // (And the scripts...)
        strOutput);
    if (!bAdded || !strOutput->Exists()) return {};
    // Success!
    //
    return strOutput->Get();
}

// returns: the updated smart contract (or "")
std::string OTAPI_Exec::SmartContract_AddClause(
    const std::string& THE_CONTRACT,   // The contract, about to have the clause
                                       // added to it.
    const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                       // signing
    // at this postd::int32_t is only to cause a
    // save.)
    const std::string& BYLAW_NAME,   // Should already be on the contract. (This
                                     // way we can find it.)
    const std::string& CLAUSE_NAME,  // The Clause's name as referenced in the
                                     // smart contract. (And the scripts...)
    const std::string& SOURCE_CODE) const  // The actual source code for the
                                           // clause.
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);
    OT_VERIFY_STD_STR(BYLAW_NAME);
    OT_VERIFY_STD_STR(CLAUSE_NAME);

    const auto strContract = String::Factory(THE_CONTRACT),
               strBylawName = String::Factory(BYLAW_NAME),
               strClauseName = String::Factory(CLAUSE_NAME),
               strSourceCode = String::Factory(SOURCE_CODE);
    const auto theSignerNymID = Identifier::Factory(SIGNER_NYM_ID);
    auto strOutput = String::Factory();

    const bool bAdded = ot_api_.SmartContract_AddClause(
        strContract,     // The contract, about to have the clause added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // postd::int32_t is only to cause a save.)
        strBylawName,    // Should already be on the contract. (This way we can
                         // find it.)
        strClauseName,   // The Clause's name as referenced in the smart
                         // contract.
                         // (And the scripts...)
        strSourceCode,   // The actual source code for the clause.
        strOutput);
    if (!bAdded || !strOutput->Exists()) return {};
    // Success!
    //
    return strOutput->Get();
}

// returns: the updated smart contract (or "")
std::string OTAPI_Exec::SmartContract_UpdateClause(
    const std::string& THE_CONTRACT,   // The contract, about to have the clause
                                       // updated on it.
    const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                       // signing
    // at this postd::int32_t is only to cause a
    // save.)
    const std::string& BYLAW_NAME,   // Should already be on the contract. (This
                                     // way we can find it.)
    const std::string& CLAUSE_NAME,  // The Clause's name as referenced in the
                                     // smart contract. (And the scripts...)
    const std::string& SOURCE_CODE) const  // The actual source code for the
                                           // clause.
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);
    OT_VERIFY_STD_STR(BYLAW_NAME);
    OT_VERIFY_STD_STR(CLAUSE_NAME);
    //  OT_VERIFY_STD_STR(SOURCE_CODE);

    const auto strContract = String::Factory(THE_CONTRACT),
               strBylawName = String::Factory(BYLAW_NAME),
               strClauseName = String::Factory(CLAUSE_NAME),
               strSourceCode = String::Factory(SOURCE_CODE);
    const auto theSignerNymID = Identifier::Factory(SIGNER_NYM_ID);
    auto strOutput = String::Factory();

    const bool bAdded = ot_api_.SmartContract_UpdateClause(
        strContract,     // The contract, about to have the clause added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // postd::int32_t is only to cause a save.)
        strBylawName,    // Should already be on the contract. (This way we can
                         // find it.)
        strClauseName,   // The Clause's name as referenced in the smart
                         // contract.
                         // (And the scripts...)
        strSourceCode,   // The actual source code for the clause.
        strOutput);
    if (!bAdded || !strOutput->Exists()) return {};
    // Success!
    //
    return strOutput->Get();
}

// returns: the updated smart contract (or "")
std::string OTAPI_Exec::SmartContract_RemoveClause(
    const std::string& THE_CONTRACT,   // The contract, about to have the clause
                                       // added to it.
    const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                       // signing
    // at this postd::int32_t is only to cause a
    // save.)
    const std::string& BYLAW_NAME,  // Should already be on the contract. (This
                                    // way we can find it.)
    const std::string& CLAUSE_NAME) const
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);
    OT_VERIFY_STD_STR(BYLAW_NAME);
    OT_VERIFY_STD_STR(CLAUSE_NAME);

    const auto strContract = String::Factory(THE_CONTRACT),
               strBylawName = String::Factory(BYLAW_NAME),
               strClauseName = String::Factory(CLAUSE_NAME);
    const auto theSignerNymID = Identifier::Factory(SIGNER_NYM_ID);
    auto strOutput = String::Factory();

    const bool bAdded = ot_api_.SmartContract_RemoveClause(
        strContract,     // The contract, about to have the clause added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // postd::int32_t is only to cause a save.)
        strBylawName,    // Should already be on the contract. (This way we can
                         // find it.)
        strClauseName,   // The Clause's name as referenced in the smart
                         // contract.
                         // (And the scripts...)
        strOutput);
    if (!bAdded || !strOutput->Exists()) return {};
    // Success!
    //
    return strOutput->Get();
}

// returns: the updated smart contract (or "")
std::string OTAPI_Exec::SmartContract_AddVariable(
    const std::string& THE_CONTRACT,   // The contract, about to have the
                                       // variable
                                       // added to it.
    const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                       // signing
    // at this postd::int32_t is only to cause a
    // save.)
    const std::string& BYLAW_NAME,  // Should already be on the contract. (This
                                    // way we can find it.)
    const std::string& VAR_NAME,    // The Variable's name as referenced in the
                                    // smart contract. (And the scripts...)
    const std::string& VAR_ACCESS,  // "constant", "persistent", or "important".
    const std::string& VAR_TYPE,    // "string", "std::int64_t", or "bool"
    const std::string& VAR_VALUE) const  // Contains a string. If type is
                                         // std::int64_t,
// StringToLong() will be used to convert
// value to a std::int64_t. If type is bool, the
// strings "true" or "false" are expected here
// in order to convert to a bool.
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);
    OT_VERIFY_STD_STR(BYLAW_NAME);
    OT_VERIFY_STD_STR(VAR_NAME);
    OT_VERIFY_STD_STR(VAR_ACCESS);
    OT_VERIFY_STD_STR(VAR_TYPE);

    const auto strContract = String::Factory(THE_CONTRACT),
               strBylawName = String::Factory(BYLAW_NAME),
               strVarName = String::Factory(VAR_NAME),
               strVarAccess = String::Factory(VAR_ACCESS),
               strVarType = String::Factory(VAR_TYPE),
               strVarValue = String::Factory(VAR_VALUE);
    const auto theSignerNymID = Identifier::Factory(SIGNER_NYM_ID);
    auto strOutput = String::Factory();

    const bool bAdded = ot_api_.SmartContract_AddVariable(
        strContract,     // The contract, about to have the clause added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // postd::int32_t is only to cause a save.)
        strBylawName,    // Should already be on the contract. (This way we can
                         // find it.)
        strVarName,  // The Variable's name as referenced in the smart contract.
                     // (And the scripts...)
        strVarAccess,  // "constant", "persistent", or "important".
        strVarType,    // "string", "std::int64_t", or "bool"
        strVarValue,   // Contains a string. If type is std::int64_t,
                       // StringToLong()
        // will be used to convert value to a std::int64_t. If type is
        // bool, the strings "true" or "false" are expected here in
        // order to convert to a bool.
        strOutput);
    if (!bAdded || !strOutput->Exists()) return {};
    // Success!
    //
    return strOutput->Get();
}

// returns: the updated smart contract (or "")
std::string OTAPI_Exec::SmartContract_RemoveVariable(
    const std::string& THE_CONTRACT,   // The contract, about to have the
                                       // variable
                                       // added to it.
    const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                       // signing
    // at this postd::int32_t is only to cause a
    // save.)
    const std::string& BYLAW_NAME,  // Should already be on the contract. (This
                                    // way we can find it.)
    const std::string& VAR_NAME     // The Variable's name as referenced in the
                                    // smart contract. (And the scripts...)
    ) const
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);
    OT_VERIFY_STD_STR(BYLAW_NAME);
    OT_VERIFY_STD_STR(VAR_NAME);

    const auto strContract = String::Factory(THE_CONTRACT),
               strBylawName = String::Factory(BYLAW_NAME),
               strVarName = String::Factory(VAR_NAME);
    const auto theSignerNymID = Identifier::Factory(SIGNER_NYM_ID);
    auto strOutput = String::Factory();

    const bool bAdded = ot_api_.SmartContract_RemoveVariable(
        strContract,     // The contract, about to have the clause added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // postd::int32_t is only to cause a save.)
        strBylawName,    // Should already be on the contract. (This way we can
                         // find it.)
        strVarName,  // The Variable's name as referenced in the smart contract.
                     // (And the scripts...)
        strOutput);
    if (!bAdded || !strOutput->Exists()) return {};
    // Success!
    //
    return strOutput->Get();
}

// returns: the updated smart contract (or "")
std::string OTAPI_Exec::SmartContract_AddCallback(
    const std::string& THE_CONTRACT,   // The contract, about to have the
                                       // callback
                                       // added to it.
    const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                       // signing
    // at this postd::int32_t is only to cause a
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
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);
    OT_VERIFY_STD_STR(BYLAW_NAME);
    OT_VERIFY_STD_STR(CALLBACK_NAME);
    OT_VERIFY_STD_STR(CLAUSE_NAME);

    const auto strContract = String::Factory(THE_CONTRACT),
               strBylawName = String::Factory(BYLAW_NAME),
               strCallbackName = String::Factory(CALLBACK_NAME),
               strClauseName = String::Factory(CLAUSE_NAME);
    const auto theSignerNymID = Identifier::Factory(SIGNER_NYM_ID);
    auto strOutput = String::Factory();

    const bool bAdded = ot_api_.SmartContract_AddCallback(
        strContract,      // The contract, about to have the clause added to it.
        theSignerNymID,   // Use any Nym you wish here. (The signing at this
                          // postd::int32_t is only to cause a save.)
        strBylawName,     // Should already be on the contract. (This way we can
                          // find it.)
        strCallbackName,  // The Callback's name as referenced in the smart
                          // contract. (And the scripts...)
        strClauseName,    // The actual clause that will be triggered by the
                          // callback. (Must exist.)
        strOutput);
    if (!bAdded || !strOutput->Exists()) return {};
    // Success!
    //
    return strOutput->Get();
}

// returns: the updated smart contract (or "")
std::string OTAPI_Exec::SmartContract_RemoveCallback(
    const std::string& THE_CONTRACT,   // The contract, about to have the
                                       // callback
                                       // removed from it.
    const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                       // signing
    // at this postd::int32_t is only to cause a
    // save.)
    const std::string& BYLAW_NAME,  // Should already be on the contract. (This
                                    // way we can find it.)
    const std::string& CALLBACK_NAME  // The Callback's name as referenced in
                                      // the smart contract. (And the
                                      // scripts...)
    ) const
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);
    OT_VERIFY_STD_STR(BYLAW_NAME);
    OT_VERIFY_STD_STR(CALLBACK_NAME);

    const auto strContract = String::Factory(THE_CONTRACT),
               strBylawName = String::Factory(BYLAW_NAME),
               strCallbackName = String::Factory(CALLBACK_NAME);
    const auto theSignerNymID = Identifier::Factory(SIGNER_NYM_ID);
    auto strOutput = String::Factory();

    const bool bAdded = ot_api_.SmartContract_RemoveCallback(
        strContract,      // The contract, about to have the clause added to it.
        theSignerNymID,   // Use any Nym you wish here. (The signing at this
                          // postd::int32_t is only to cause a save.)
        strBylawName,     // Should already be on the contract. (This way we can
                          // find it.)
        strCallbackName,  // The Callback's name as referenced in the smart
                          // contract. (And the scripts...)
        strOutput);
    if (!bAdded || !strOutput->Exists()) return {};
    // Success!
    //
    return strOutput->Get();
}

// returns: the updated smart contract (or "")
std::string OTAPI_Exec::SmartContract_AddHook(
    const std::string& THE_CONTRACT,   // The contract, about to have the hook
                                       // added to it.
    const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                       // signing
    // at this postd::int32_t is only to cause a
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
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);
    OT_VERIFY_STD_STR(BYLAW_NAME);
    OT_VERIFY_STD_STR(HOOK_NAME);
    OT_VERIFY_STD_STR(CLAUSE_NAME);

    const auto strContract = String::Factory(THE_CONTRACT),
               strBylawName = String::Factory(BYLAW_NAME),
               strHookName = String::Factory(HOOK_NAME),
               strClauseName = String::Factory(CLAUSE_NAME);
    const auto theSignerNymID = Identifier::Factory(SIGNER_NYM_ID);
    auto strOutput = String::Factory();

    const bool bAdded = ot_api_.SmartContract_AddHook(
        strContract,     // The contract, about to have the clause added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // postd::int32_t is only to cause a save.)
        strBylawName,    // Should already be on the contract. (This way we can
                         // find it.)
        strHookName,     // The Hook's name as referenced in the smart contract.
                         // (And the scripts...)
        strClauseName,  // The actual clause that will be triggered by the hook.
                        // (You can call this multiple times, and have multiple
                        // clauses trigger on the same hook.)
        strOutput);
    if (!bAdded || !strOutput->Exists()) return {};
    // Success!
    //
    return strOutput->Get();
}

// returns: the updated smart contract (or "")
std::string OTAPI_Exec::SmartContract_RemoveHook(
    const std::string& THE_CONTRACT,   // The contract, about to have the hook
                                       // removed from it.
    const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                       // signing
    // at this postd::int32_t is only to cause a
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
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);
    OT_VERIFY_STD_STR(BYLAW_NAME);
    OT_VERIFY_STD_STR(HOOK_NAME);
    OT_VERIFY_STD_STR(CLAUSE_NAME);

    const auto strContract = String::Factory(THE_CONTRACT),
               strBylawName = String::Factory(BYLAW_NAME),
               strHookName = String::Factory(HOOK_NAME),
               strClauseName = String::Factory(CLAUSE_NAME);
    const auto theSignerNymID = Identifier::Factory(SIGNER_NYM_ID);
    auto strOutput = String::Factory();

    const bool bAdded = ot_api_.SmartContract_RemoveHook(
        strContract,     // The contract, about to have the clause added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // postd::int32_t is only to cause a save.)
        strBylawName,    // Should already be on the contract. (This way we can
                         // find it.)
        strHookName,     // The Hook's name as referenced in the smart contract.
                         // (And the scripts...)
        strClauseName,  // The actual clause that will be triggered by the hook.
                        // (You can call this multiple times, and have multiple
                        // clauses trigger on the same hook.)
        strOutput);
    if (!bAdded || !strOutput->Exists()) return {};
    // Success!
    //
    return strOutput->Get();
}

// RETURNS: Updated version of THE_CONTRACT. (Or "".)
std::string OTAPI_Exec::SmartContract_AddParty(
    const std::string& THE_CONTRACT,   // The contract, about to have the party
                                       // added to it.
    const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                       // signing
    // at this postd::int32_t is only to cause a
    // save.)
    const std::string& PARTY_NYM_ID,  // Required when the smart contract is
                                      // configured to require parties to be
                                      // specified. Otherwise must be empty.
    const std::string& PARTY_NAME,    // The Party's NAME as referenced in the
                                      // smart contract. (And the scripts...)
    const std::string& AGENT_NAME) const  // An AGENT will be added by default
                                          // for this
                                          // party. Need Agent NAME.
// (FYI, that is basically the only option, until I code Entities and Roles.
// Until then, a party can ONLY be
// a Nym, with himself as the agent representing that same party. Nym ID is
// supplied on ConfirmParty() below.)
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);
    OT_VERIFY_STD_STR(PARTY_NAME);
    OT_VERIFY_STD_STR(AGENT_NAME);

    const auto strContract = String::Factory(THE_CONTRACT),
               strPartyName = String::Factory(PARTY_NAME),
               strAgentName = String::Factory(AGENT_NAME),
               strPartyNymID = String::Factory(PARTY_NYM_ID);
    const auto theSignerNymID = Identifier::Factory(SIGNER_NYM_ID);
    auto strOutput = String::Factory();

    const bool bAdded = ot_api_.SmartContract_AddParty(
        strContract,     // The contract, about to have the bylaw added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // postd::int32_t is only to cause a save.)
        strPartyNymID,
        strPartyName,  // The Party's NAME as referenced in the smart contract.
                       // (And the scripts...)
        strAgentName,  // An AGENT will be added by default for this party. Need
                       // Agent NAME.
        strOutput);
    if (!bAdded || !strOutput->Exists()) return {};
    // Success!
    //
    return strOutput->Get();
}

// RETURNS: Updated version of THE_CONTRACT. (Or "".)
std::string OTAPI_Exec::SmartContract_RemoveParty(
    const std::string& THE_CONTRACT,   // The contract, about to have the party
                                       // added to it.
    const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                       // signing
    // at this postd::int32_t is only to cause a
    // save.)
    const std::string& PARTY_NAME  // The Party's NAME as referenced in the
                                   // smart contract. (And the scripts...)
    ) const
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);
    OT_VERIFY_STD_STR(PARTY_NAME);

    const auto strContract = String::Factory(THE_CONTRACT),
               strPartyName = String::Factory(PARTY_NAME);
    const auto theSignerNymID = Identifier::Factory(SIGNER_NYM_ID);
    auto strOutput = String::Factory();

    const bool bAdded = ot_api_.SmartContract_RemoveParty(
        strContract,     // The contract, about to have the bylaw added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // postd::int32_t is only to cause a save.)
        strPartyName,  // The Party's NAME as referenced in the smart contract.
                       // (And the scripts...)
        strOutput);
    if (!bAdded || !strOutput->Exists()) return {};
    // Success!
    //
    return strOutput->Get();
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
    // at this postd::int32_t is only to cause a
    // save.)
    const std::string& PARTY_NAME,  // The Party's NAME as referenced in the
                                    // smart contract. (And the scripts...)
    const std::string& ACCT_NAME,   // The Account's name as referenced in the
                                    // smart contract
    const std::string& INSTRUMENT_DEFINITION_ID) const  // Instrument Definition
// ID for the Account. (Optional.)
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);
    OT_VERIFY_STD_STR(PARTY_NAME);
    OT_VERIFY_STD_STR(ACCT_NAME);

    //  if (INSTRUMENT_DEFINITION_ID.empty()) {
    //      otErr << OT_METHOD << __FUNCTION__ << ": Null:
    //        INSTRUMENT_DEFINITION_ID passed
    //        in!\n"; OT_FAIL; }

    const auto strContract = String::Factory(THE_CONTRACT),
               strPartyName = String::Factory(PARTY_NAME),
               strAcctName = String::Factory(ACCT_NAME),
               strInstrumentDefinitionID =
                   String::Factory(INSTRUMENT_DEFINITION_ID);
    const auto theSignerNymID = Identifier::Factory(SIGNER_NYM_ID);
    auto strOutput = String::Factory();

    const bool bAdded = ot_api_.SmartContract_AddAccount(
        strContract,     // The contract, about to have the clause added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // postd::int32_t is only to cause a save.)
        strPartyName,  // The Party's NAME as referenced in the smart contract.
                       // (And the scripts...)
        strAcctName,   // The Account's name as referenced in the smart contract
        strInstrumentDefinitionID,  // Instrument Definition ID for the Account.
        strOutput);
    if (!bAdded || !strOutput->Exists()) return {};
    // Success!
    //
    return strOutput->Get();
}

// returns: the updated smart contract (or "")
std::string OTAPI_Exec::SmartContract_RemoveAccount(
    const std::string& THE_CONTRACT,  // The contract, about to have the account
                                      // removed from it.
    const std::string& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                       // signing
    // at this postd::int32_t is only to cause a
    // save.)
    const std::string& PARTY_NAME,  // The Party's NAME as referenced in the
                                    // smart contract. (And the scripts...)
    const std::string& ACCT_NAME    // The Account's name as referenced in the
                                    // smart contract
    ) const
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);
    OT_VERIFY_STD_STR(PARTY_NAME);
    OT_VERIFY_STD_STR(ACCT_NAME);

    const auto strContract = String::Factory(THE_CONTRACT),
               strPartyName = String::Factory(PARTY_NAME),
               strAcctName = String::Factory(ACCT_NAME);
    const auto theSignerNymID = Identifier::Factory(SIGNER_NYM_ID);
    auto strOutput = String::Factory();

    const bool bAdded = ot_api_.SmartContract_RemoveAccount(
        strContract,     // The contract, about to have the clause added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // postd::int32_t is only to cause a save.)
        strPartyName,  // The Party's NAME as referenced in the smart contract.
                       // (And the scripts...)
        strAcctName,   // The Account's name as referenced in the smart contract
        strOutput);
    if (!bAdded || !strOutput->Exists()) return {};
    // Success!
    //
    return strOutput->Get();
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
std::int32_t OTAPI_Exec::SmartContract_CountNumsNeeded(
    const std::string& THE_CONTRACT,  // The smart contract, about to be queried
                                      // by this function.
    const std::string& AGENT_NAME) const
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(AGENT_NAME);

    const auto strContract = String::Factory(THE_CONTRACT),
               strAgentName = String::Factory(AGENT_NAME);
    return ot_api_.SmartContract_CountNumsNeeded(strContract, strAgentName);
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
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);
    OT_VERIFY_STD_STR(PARTY_NAME);
    OT_VERIFY_STD_STR(ACCT_NAME);
    OT_VERIFY_STD_STR(AGENT_NAME);
    OT_VERIFY_ID_STR(ACCT_ID);

    const auto strContract = String::Factory(THE_CONTRACT),
               strPartyName = String::Factory(PARTY_NAME);
    const auto strAccountID = String::Factory(ACCT_ID),
               strAcctName = String::Factory(ACCT_NAME),
               strAgentName = String::Factory(AGENT_NAME);
    const auto theSignerNymID = Identifier::Factory(SIGNER_NYM_ID),
               theAcctID = Identifier::Factory(strAccountID);
    auto strOutput = String::Factory();

    const bool bConfirmed = ot_api_.SmartContract_ConfirmAccount(
        strContract,
        theSignerNymID,
        strPartyName,
        strAcctName,
        strAgentName,
        strAccountID,
        strOutput);
    if (!bConfirmed || !strOutput->Exists()) return {};
    // Success!
    return strOutput->Get();
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
    const std::string& NYM_ID,
    const std::string& NOTARY_ID) const
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(PARTY_NAME);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(NOTARY_ID);

    const auto theNymID = Identifier::Factory(NYM_ID);
    const auto strContract = String::Factory(THE_CONTRACT),
               strPartyName = String::Factory(PARTY_NAME);
    auto strOutput = String::Factory();

    const bool bConfirmed = ot_api_.SmartContract_ConfirmParty(
        strContract,   // The smart contract, about to be changed by this
                       // function.
        strPartyName,  // Should already be on the contract. This way we can
                       // find it.
        theNymID,      // Nym ID for the party, the actual owner,
        Identifier::Factory(NOTARY_ID),
        strOutput);
    if (!bConfirmed || !strOutput->Exists()) return {};
    // Success!
    return strOutput->Get();
}

bool OTAPI_Exec::Smart_AreAllPartiesConfirmed(
    const std::string& THE_CONTRACT) const  // true or false?
{
    OT_VERIFY_STD_STR(THE_CONTRACT);

    auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to load smart contract from string : ")(
            strContract)(".")
            .Flush();
    } else {
        const bool bConfirmed =
            pScriptable->AllPartiesHaveSupposedlyConfirmed();
        const bool bVerified =
            pScriptable->VerifyThisAgainstAllPartiesSignedCopies();
        if (!bConfirmed) {
            //          otOut << OT_METHOD << __FUNCTION__ << ": Smart contract
            //          loaded up,
            // but all
            // parties are NOT confirmed:\n\n" << strContract << "\n\n";
            LogDetail(OT_METHOD)(__FUNCTION__)(
                ": Smart contract loaded up, but all ")(
                "parties are NOT confirmed.")
                .Flush();
            return false;
        } else if (bVerified) {
            //          otOut << OT_METHOD << __FUNCTION__ << ": Success: Smart
            //          contract
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
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Suspicious: Smart contract loaded up, and is supposedly "
            "confirmed by all parties, but failed to verify: ")(strContract)(
            ".")
            .Flush();
    }
    return false;
}

bool OTAPI_Exec::Smart_IsPartyConfirmed(
    const std::string& THE_CONTRACT,
    const std::string& PARTY_NAME) const  // true
                                          // or
                                          // false?
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(PARTY_NAME);

    const auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to load smart contract from string : ")(
            strContract)(".")
            .Flush();
        return false;
    }

    OTParty* pParty = pScriptable->GetParty(PARTY_NAME);
    if (nullptr == pParty) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Smart contract loaded up, but failed to find a party "
            "with the name: ")(PARTY_NAME)(".")
            .Flush();
        return false;
    }

    // We found the party...
    //...is he confirmed?
    //
    if (!pParty->GetMySignedCopy().Exists()) {
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": Smart contract loaded up, and party ")(PARTY_NAME)(
            " was found, but didn't find a signed copy of the ")(
            "agreement for that party.")
            .Flush();
        return false;
    }

    // FYI, this block comes from
    // OTScriptable::VerifyThisAgainstAllPartiesSignedCopies.
    auto pPartySignedCopy(api_.Factory().Scriptable(pParty->GetMySignedCopy()));

    if (nullptr == pPartySignedCopy) {
        const std::string current_party_name(pParty->GetPartyName());
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error loading party's (")(
            current_party_name)(") signed copy of agreement. Has it been "
                                "executed?")
            .Flush();
        return false;
    }

    if (!pScriptable->Compare(*pPartySignedCopy)) {
        const std::string current_party_name(pParty->GetPartyName());
        LogOutput(OT_METHOD)(__FUNCTION__)(": Suspicious: Party's (")(
            current_party_name)(") signed copy of agreement doesn't match the "
                                "contract.")
            .Flush();
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

std::int32_t OTAPI_Exec::Smart_GetPartyCount(
    const std::string& THE_CONTRACT) const
{
    OT_VERIFY_STD_STR(THE_CONTRACT);

    auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to load smart contract from string: ")(
            strContract)(".")
            .Flush();
        return OT_ERROR;  // Error condition.
    }

    return pScriptable->GetPartyCount();
}

std::int32_t OTAPI_Exec::Smart_GetBylawCount(
    const std::string& THE_CONTRACT) const
{
    OT_VERIFY_STD_STR(THE_CONTRACT);

    auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to load smart contract from string : ")(
            strContract)(".")
            .Flush();
        return OT_ERROR;  // Error condition.
    }

    return pScriptable->GetBylawCount();
}

/// returns the name of the party.
std::string OTAPI_Exec::Smart_GetPartyByIndex(
    const std::string& THE_CONTRACT,
    const std::int32_t& nIndex) const
{
    OT_VERIFY_STD_STR(THE_CONTRACT);

    const auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to load smart contract from string: ")(
            strContract)(".")
            .Flush();
        return {};
    }

    const std::int32_t nTempIndex = nIndex;
    OTParty* pParty = pScriptable->GetPartyByIndex(
        nTempIndex);  // has range-checking built-in.
    if (nullptr == pParty) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Smart contract loaded up, but failed to retrieve the "
            "party using index: ")(nTempIndex)(".")
            .Flush();
        return {};
    }

    return pParty->GetPartyName();
}

/// returns the name of the bylaw.
std::string OTAPI_Exec::Smart_GetBylawByIndex(
    const std::string& THE_CONTRACT,
    const std::int32_t& nIndex) const
{
    OT_VERIFY_STD_STR(THE_CONTRACT);

    const auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to load smart contract from string : ")(
            strContract)(".")
            .Flush();
        return {};
    }

    const std::int32_t nTempIndex = nIndex;
    OTBylaw* pBylaw = pScriptable->GetBylawByIndex(
        nTempIndex);  // has range-checking built-in.
    if (nullptr == pBylaw) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Smart contract loaded up, but failed to retrieve the "
            "bylaw using index: ")(nTempIndex)(".")
            .Flush();
        return {};
    }

    // We found the bylaw...
    return pBylaw->GetName().Get();
}

std::string OTAPI_Exec::Bylaw_GetLanguage(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME) const
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(BYLAW_NAME);

    auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to load smart contract from string : ")(
            strContract)(".")
            .Flush();
        return {};
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Smart contract loaded up, but failed to find a bylaw "
            "with the name: ")(BYLAW_NAME)(".")
            .Flush();
        return {};
    }
    // We found the bylaw...
    if (nullptr == pBylaw->GetLanguage()) return "error_no_language";
    return pBylaw->GetLanguage();
}

std::int32_t OTAPI_Exec::Bylaw_GetClauseCount(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME) const
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(BYLAW_NAME);

    auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to load smart contract from string : ")(
            strContract)(".")
            .Flush();
        return OT_ERROR;
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Smart contract loaded up, but failed to find a bylaw "
            "with the name: ")(BYLAW_NAME)(".")
            .Flush();
        return OT_ERROR;
    }

    return pBylaw->GetClauseCount();
}

std::int32_t OTAPI_Exec::Bylaw_GetVariableCount(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME) const
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(BYLAW_NAME);

    auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to load smart contract from string : ")(
            strContract)(".")
            .Flush();
        return OT_ERROR;
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Smart contract loaded up, but failed to find a bylaw "
            "with the name: ")(BYLAW_NAME)(".")
            .Flush();
        return OT_ERROR;
    }

    return pBylaw->GetVariableCount();
}

std::int32_t OTAPI_Exec::Bylaw_GetHookCount(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME) const
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(BYLAW_NAME);

    auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to load smart contract from string : ")(
            strContract)(".")
            .Flush();
        return OT_ERROR;
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Smart contract loaded up, but failed to find a bylaw "
            "with the name: ")(BYLAW_NAME)(".")
            .Flush();
        return OT_ERROR;
    }

    return pBylaw->GetHookCount();
}

std::int32_t OTAPI_Exec::Bylaw_GetCallbackCount(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME) const
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(BYLAW_NAME);

    auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to load smart contract from string: ")(
            strContract)(".")
            .Flush();
        return OT_ERROR;
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Smart contract loaded up, but failed to find a bylaw "
            "with the name: ")(BYLAW_NAME)(".")
            .Flush();
        return OT_ERROR;
    }

    return pBylaw->GetCallbackCount();
}

std::string OTAPI_Exec::Clause_GetNameByIndex(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME,
    const std::int32_t& nIndex) const  // returns the name of the clause.
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(BYLAW_NAME);

    const auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to load smart contract from string: ")(
            strContract)(".")
            .Flush();
        return {};
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Smart contract loaded up, but failed to retrieve the "
            "bylaw with name: ")(BYLAW_NAME)(".")
            .Flush();
        return {};
    }

    const std::int32_t nTempIndex = nIndex;
    OTClause* pClause = pBylaw->GetClauseByIndex(nTempIndex);
    if (nullptr == pClause) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Smart contract loaded up, and "
            "bylaw found, but failed to retrieve "
            "the clause at index: ")(nTempIndex)(".")
            .Flush();
        return {};
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
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(BYLAW_NAME);
    OT_VERIFY_STD_STR(CLAUSE_NAME);

    const auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to load smart contract from string: ")(
            strContract)(".")
            .Flush();
        return {};
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Smart contract loaded up, but failed to retrieve the "
            "bylaw with name: ")(BYLAW_NAME)(".")
            .Flush();
        return {};
    }

    OTClause* pClause = pBylaw->GetClause(CLAUSE_NAME);

    if (nullptr == pClause) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Smart contract loaded up, and "
            "bylaw found, but failed to retrieve "
            "the clause with name: ")(CLAUSE_NAME)(".")
            .Flush();
        return {};
    }
    // Success.
    return pClause->GetCode();
}

std::string OTAPI_Exec::Variable_GetNameByIndex(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME,
    const std::int32_t& nIndex) const  // returns the name of the variable.
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(BYLAW_NAME);

    const auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to load smart contract from string: ")(
            strContract)(".")
            .Flush();
        return {};
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Smart contract loaded up, but failed to retrieve the "
            "bylaw with name: ")(BYLAW_NAME)(".")
            .Flush();
        return {};
    }

    const std::int32_t nTempIndex = nIndex;
    OTVariable* pVar = pBylaw->GetVariableByIndex(nTempIndex);
    if (nullptr == pVar) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Smart contract loaded up, and "
            "bylaw found, but failed to retrieve "
            "the variable at index: ")(nTempIndex)(".")
            .Flush();
        return {};
    }

    return pVar->GetName().Get();
}

std::string OTAPI_Exec::Variable_GetType(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME,
    const std::string& VARIABLE_NAME) const  // returns the type of the
                                             // variable.
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(BYLAW_NAME);
    OT_VERIFY_STD_STR(VARIABLE_NAME);

    const auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to load smart contract from string: ")(
            strContract)(".")
            .Flush();
        return {};
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Smart contract loaded up, but failed to retrieve the "
            "bylaw with name: ")(BYLAW_NAME)(".")
            .Flush();
        return {};
    }

    OTVariable* pVar = pBylaw->GetVariable(VARIABLE_NAME);
    if (nullptr == pVar) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Smart contract loaded up, and bylaw found, but "
            "failed to retrieve the variable with name: ")(VARIABLE_NAME)(".")
            .Flush();
        return {};
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
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(BYLAW_NAME);
    OT_VERIFY_STD_STR(VARIABLE_NAME);

    const auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to load smart contract from string: ")(
            strContract)(".")
            .Flush();
        return {};
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Smart contract loaded up, but failed to retrieve the "
            "bylaw with name: ")(BYLAW_NAME)(".")
            .Flush();
        return {};
    }

    OTVariable* pVar = pBylaw->GetVariable(VARIABLE_NAME);
    if (nullptr == pVar) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Smart contract loaded up, and bylaw found, but "
            "failed to retrieve the variable with name: ")(VARIABLE_NAME)(".")
            .Flush();
        return {};
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
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(BYLAW_NAME);
    OT_VERIFY_STD_STR(VARIABLE_NAME);

    const auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to load smart contract from string: ")(
            strContract)(".")
            .Flush();
        return {};
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Smart contract loaded up, but failed to retrieve the "
            "bylaw with name: ")(BYLAW_NAME)(".")
            .Flush();
        return {};
    }

    OTVariable* pVar = pBylaw->GetVariable(VARIABLE_NAME);
    if (nullptr == pVar) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Smart contract loaded up, and bylaw found, but "
            "failed to retrieve the variable with name: ")(VARIABLE_NAME)(".")
            .Flush();
        return {};
    }

    switch (pVar->GetType()) {
        case OTVariable::Var_String:
            return pVar->GetValueString();
        case OTVariable::Var_Integer:
            return OTAPI_Exec::LongToString(
                static_cast<std::int64_t>(pVar->GetValueInteger()));
        case OTVariable::Var_Bool:
            return pVar->GetValueBool() ? "true" : "false";
        default:
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error: Unknown variable type.")
                .Flush();
            return {};
    }
}

std::string OTAPI_Exec::Hook_GetNameByIndex(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME,
    const std::int32_t& nIndex) const  // returns the name of the hook.
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(BYLAW_NAME);

    const auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to load smart contract from string: ")(
            strContract)(".")
            .Flush();
        return {};
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Smart contract loaded up, but failed to retrieve the "
            "bylaw with name: ")(BYLAW_NAME)(".")
            .Flush();
        return {};
    }

    const std::int32_t nTempIndex = nIndex;
    return pBylaw->GetHookNameByIndex(nTempIndex);
}

/// Returns the number of clauses attached to a specific hook.
std::int32_t OTAPI_Exec::Hook_GetClauseCount(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME,
    const std::string& HOOK_NAME) const
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(BYLAW_NAME);
    OT_VERIFY_STD_STR(HOOK_NAME);

    const auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to load smart contract from string: ")(
            strContract)(".")
            .Flush();
        return OT_ERROR;
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Smart contract loaded up, but failed to retrieve the "
            "bylaw with name: ")(BYLAW_NAME)(".")
            .Flush();
        return OT_ERROR;
    }

    mapOfClauses theResults;
    // Look up all clauses matching a specific hook.
    if (!pBylaw->GetHooks(HOOK_NAME, theResults)) return OT_ERROR;

    return static_cast<std::int32_t>(theResults.size());
}

/// Multiple clauses can trigger from the same hook.
/// Hook_GetClauseCount and Hook_GetClauseAtIndex allow you to
/// iterate through them.
/// This function returns the name for the clause at the specified index.
///
std::string OTAPI_Exec::Hook_GetClauseAtIndex(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME,
    const std::string& HOOK_NAME,
    const std::int32_t& nIndex) const
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(BYLAW_NAME);
    OT_VERIFY_STD_STR(HOOK_NAME);

    const auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to load smart contract from string: ")(
            strContract)(".")
            .Flush();
        return {};
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Smart contract loaded up, but failed to retrieve the "
            "bylaw with name: ")(BYLAW_NAME)(".")
            .Flush();
        return {};
    }

    mapOfClauses theResults;

    // Look up all clauses matching a specific hook.
    if (!pBylaw->GetHooks(HOOK_NAME, theResults)) return {};

    if ((nIndex < 0) ||
        (nIndex >= static_cast<std::int64_t>(theResults.size())))
        return {};

    std::int32_t nLoopIndex = -1;
    for (auto& it : theResults) {
        OTClause* pClause = it.second;
        OT_ASSERT(nullptr != pClause);
        ++nLoopIndex;  // on first iteration, this is now 0.

        if (nLoopIndex == nIndex) return pClause->GetName().Get();
    }
    return {};
}

std::string OTAPI_Exec::Callback_GetNameByIndex(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME,
    const std::int32_t& nIndex) const  // returns the name of the callback.
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(BYLAW_NAME);

    const auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to load smart contract from string: ")(
            strContract)(".")
            .Flush();
        return {};
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Smart contract loaded up, but failed to retrieve the "
            "bylaw with name: ")(BYLAW_NAME)(".")
            .Flush();
        return {};
    }

    const std::int32_t nTempIndex = nIndex;
    return pBylaw->GetCallbackNameByIndex(nTempIndex);
}

std::string OTAPI_Exec::Callback_GetClause(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME,
    const std::string& CALLBACK_NAME) const  // returns name of clause attached
                                             // to callback.
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(BYLAW_NAME);
    OT_VERIFY_STD_STR(CALLBACK_NAME);

    const auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to load smart contract from string: ")(
            strContract)(".")
            .Flush();
        return {};
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Smart contract loaded up, but failed to retrieve the "
            "bylaw with name: ")(BYLAW_NAME)(".")
            .Flush();
        return {};
    }

    OTClause* pClause = pBylaw->GetCallback(CALLBACK_NAME);
    if (nullptr == pClause) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Smart contract loaded up, and bylaw found, but "
            "failed to retrieve the clause for callback: ")(CALLBACK_NAME)(".")
            .Flush();
        return {};
    }

    return pClause->GetName().Get();
}

std::int32_t OTAPI_Exec::Party_GetAcctCount(
    const std::string& THE_CONTRACT,
    const std::string& PARTY_NAME) const
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(PARTY_NAME);

    auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().Scriptable(strContract));
    if (nullptr == pScriptable) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to load smart contract from string: ")(
            strContract)(".")
            .Flush();
        return OT_ERROR;
    }

    OTParty* pParty = pScriptable->GetParty(PARTY_NAME);
    if (nullptr == pParty) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Smart contract loaded up, but failed to find a party "
            "with the name: ")(PARTY_NAME)(".")
            .Flush();
        return OT_ERROR;
    }

    return pParty->GetAccountCount();
}

std::int32_t OTAPI_Exec::Party_GetAgentCount(
    const std::string& THE_CONTRACT,
    const std::string& PARTY_NAME) const
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(PARTY_NAME);

    auto strContract = String::Factory(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        api_.Factory().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to load smart contract from string: ")(
            strContract)(".")
            .Flush();
        return OT_ERROR;
    }

    OTParty* pParty = pScriptable->GetParty(PARTY_NAME);
    if (nullptr == pParty) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Smart contract loaded up, but failed to find a party "
            "with the name: ")(PARTY_NAME)(".")
            .Flush();
        return OT_ERROR;
    }

    return pParty->GetAgentCount();
}

// returns either NymID or Entity ID.
// (If there is one... Contract might not be
// signed yet.)
std::string OTAPI_Exec::Party_GetID(
    const std::string& THE_CONTRACT,
    const std::string& PARTY_NAME) const
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(PARTY_NAME);

    auto strContract = String::Factory(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        api_.Factory().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to load smart contract from string: ")(
            strContract)(".")
            .Flush();
        return {};
    }

    OTParty* pParty = pScriptable->GetParty(PARTY_NAME);
    if (nullptr == pParty) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Smart contract loaded up, but failed to find a party "
            "with the name: ")(PARTY_NAME)(".")
            .Flush();
        return {};
    }

    return pParty->GetPartyID();
}

std::string OTAPI_Exec::Party_GetAcctNameByIndex(
    const std::string& THE_CONTRACT,
    const std::string& PARTY_NAME,
    const std::int32_t& nIndex) const  // returns the name of the clause.
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(PARTY_NAME);

    const auto strContract = String::Factory(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        api_.Factory().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to load smart contract from string: ")(
            strContract)(".")
            .Flush();
        return {};
    }

    OTParty* pParty = pScriptable->GetParty(PARTY_NAME);
    if (nullptr == pParty) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Smart contract loaded up, but failed to retrieve the "
            "party with name: ")(PARTY_NAME)(".")
            .Flush();
        return {};
    }

    const std::int32_t nTempIndex = nIndex;
    OTPartyAccount* pAcct = pParty->GetAccountByIndex(nTempIndex);
    if (nullptr == pAcct) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Smart contract loaded up, and "
            "party found, but failed to retrieve "
            "the account at index: ")(nTempIndex)(".")
            .Flush();
        return {};
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
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(PARTY_NAME);
    OT_VERIFY_STD_STR(ACCT_NAME);

    const auto strContract = String::Factory(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        api_.Factory().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to load smart contract from string: ")(
            strContract)(".")
            .Flush();
        return {};
    }

    OTParty* pParty = pScriptable->GetParty(PARTY_NAME);
    if (nullptr == pParty) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Smart contract loaded up, but failed to retrieve the "
            "party with name: ")(PARTY_NAME)(".")
            .Flush();
        return {};
    }

    const OTPartyAccount* pAcct = pParty->GetAccount(ACCT_NAME);
    if (nullptr == pAcct) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Smart contract loaded up, and "
            "party found, but failed to retrieve "
            "party's account named: ")(ACCT_NAME)(".")
            .Flush();
        return {};
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
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(PARTY_NAME);
    OT_VERIFY_STD_STR(ACCT_NAME);

    const auto strContract = String::Factory(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        api_.Factory().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to load smart contract from string: ")(
            strContract)(".")
            .Flush();
    } else {
        OTParty* pParty = pScriptable->GetParty(PARTY_NAME);
        if (nullptr == pParty) {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Smart contract loaded up, but failed to retrieve the "
                "party with name: ")(PARTY_NAME)(".")
                .Flush();
        } else  // We found the party...
        {
            const OTPartyAccount* pAcct = pParty->GetAccount(ACCT_NAME);

            if (nullptr == pAcct) {
                LogNormal(OT_METHOD)(__FUNCTION__)(
                    ": Smart contract loaded up, and "
                    "party found, but failed to retrieve "
                    "party's account named: ")(ACCT_NAME)(".")
                    .Flush();
            } else  // We found the account...
            {
                const std::string str_return(
                    pAcct->GetInstrumentDefinitionID().Get());  // Success.
                return str_return;
            }
        }
    }
    return {};
}

std::string OTAPI_Exec::Party_GetAcctAgentName(
    const std::string& THE_CONTRACT,
    const std::string& PARTY_NAME,
    const std::string& ACCT_NAME) const  // returns the authorized agent for the
                                         // named
                                         // account.
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(PARTY_NAME);
    OT_VERIFY_STD_STR(ACCT_NAME);

    const auto strContract = String::Factory(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        api_.Factory().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to load smart contract from string: ")(
            strContract)(".")
            .Flush();
    } else {
        OTParty* pParty = pScriptable->GetParty(PARTY_NAME);
        if (nullptr == pParty) {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Smart contract loaded up, but failed to retrieve the "
                "party with name: ")(PARTY_NAME)(".")
                .Flush();
        } else  // We found the party...
        {
            const OTPartyAccount* pAcct = pParty->GetAccount(ACCT_NAME);

            if (nullptr == pAcct) {
                LogNormal(OT_METHOD)(__FUNCTION__)(
                    ": Smart contract loaded up, and "
                    "party found, but failed to retrieve "
                    "party's account named: ")(ACCT_NAME)(".")
                    .Flush();
            } else  // We found the account...
            {
                const std::string str_return(
                    pAcct->GetAgentName().Get());  // Success.
                return str_return;
            }
        }
    }
    return {};
}

std::string OTAPI_Exec::Party_GetAgentNameByIndex(
    const std::string& THE_CONTRACT,
    const std::string& PARTY_NAME,
    const std::int32_t& nIndex) const  // returns the name of the agent.
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(PARTY_NAME);

    const auto strContract = String::Factory(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        api_.Factory().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to load smart contract from string: ")(
            strContract)(".")
            .Flush();
    } else {
        OTParty* pParty = pScriptable->GetParty(PARTY_NAME);
        if (nullptr == pParty) {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Smart contract loaded up, but failed to retrieve the "
                "party with name: ")(PARTY_NAME)(".")
                .Flush();
        } else  // We found the party...
        {
            const std::int32_t nTempIndex = nIndex;
            OTAgent* pAgent = pParty->GetAgentByIndex(nTempIndex);

            if (nullptr == pAgent) {
                LogNormal(OT_METHOD)(__FUNCTION__)(
                    ": Smart contract loaded up, and party found, but "
                    "failed to retrieve the agent at index: ")(nTempIndex)(".")
                    .Flush();
            } else  // We found the agent...
            {
                const std::string str_name(
                    pAgent->GetName().Get());  // Success.
                return str_name;
            }
        }
    }
    return {};
}

std::string OTAPI_Exec::Party_GetAgentID(
    const std::string& THE_CONTRACT,
    const std::string& PARTY_NAME,
    const std::string& AGENT_NAME) const  // returns ID of the agent. (If
                                          // there is one...)
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(PARTY_NAME);
    OT_VERIFY_STD_STR(AGENT_NAME);

    const auto strContract = String::Factory(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        api_.Factory().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to load smart contract from string: ")(
            strContract)(".")
            .Flush();
    } else {
        OTParty* pParty = pScriptable->GetParty(PARTY_NAME);
        if (nullptr == pParty) {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Smart contract loaded up, but failed to retrieve the "
                "party with name: ")(PARTY_NAME)(".")
                .Flush();
        } else  // We found the party...
        {
            OTAgent* pAgent = pParty->GetAgent(AGENT_NAME);

            if (nullptr == pAgent) {
                LogNormal(OT_METHOD)(__FUNCTION__)(
                    ": Smart contract loaded up, and "
                    "party found, but failed to retrieve "
                    "party's agent named: ")(AGENT_NAME)(".")
                    .Flush();
            } else  // We found the agent...
            {
                auto theAgentID = Identifier::Factory();
                if (pAgent->IsAnIndividual() && pAgent->GetNymID(theAgentID)) {
                    return theAgentID->str();
                }
            }
        }
    }
    return {};
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
    Lock lock(lock_);

    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_STD_STR(THE_MESSAGE);

    const auto theNymID = Identifier::Factory(NYM_ID);
    auto theMessage{api_.Factory().Message()};
    const auto strMsg = String::Factory(THE_MESSAGE);

    if (!strMsg->Exists()) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to load message from empty string.")
            .Flush();
        return false;
    }

    // Maybe it's not a message at all. Maybe it's a cron item
    // (smart contract... payment plan...)
    //
    if (strMsg->Contains("PAYMENT PLAN") || strMsg->Contains("SMARTCONTRACT")) {
        const String& strCronItem = strMsg;

        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Attempting to harvest transaction numbers "
            "from cron item...")
            .Flush();
        // Unfortunately the ONLY reason we are loading up this cron item here,
        // is so we can get the server ID off of it.
        //
        auto pCronItem{api_.Factory().CronItem(strCronItem)};
        if (false == bool(pCronItem)) {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Failed trying to load message from string.")
                .Flush();

            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Error trying to load the cron item from string (a "
                "cron "
                "item is a smart contract, or some other recurring "
                "transaction such as a market offer, or a payment "
                "plan.) "
                "Contents: ")(strCronItem)(".")
                .Flush();
            return false;
        }

        // NOTE:
        // If a CronItem is passed in here instead of a Message, that means
        // the client didn't even TRY to send the message. He failed before
        // reaching that point. Therefore in this one, strange case, we
        // don't really care about all the bools that were passed in here.
        // We're just going to harvest ALL the numbers, and ASSUME all the
        // bools were false. Here goes...
        //
        return ot_api_.HarvestAllNumbers(
            pCronItem->GetNotaryID(), theNymID, strCronItem);
    }
    // Maybe it's not a message at all. Maybe it's a basket exchange request
    // that never
    // even got sent as a message...
    //
    if (strMsg->Contains("currencyBasket")) {
        const String& strBasket = strMsg;

        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Attempting to harvest transaction numbers "
            "from a basket currency exchange request...")
            .Flush();
        if (!api_.Wallet().IsLocalNym(NYM_ID)) { return false; }

        auto theRequestBasket{api_.Factory().Basket()};

        if (theRequestBasket->LoadContractFromString(strBasket)) {
            if (!theRequestBasket->IsExchanging()) {
                LogNormal(OT_METHOD)(__FUNCTION__)(
                    ": Error: This is apparently NOT a "
                    "basket exchange request!")("Contents: ")(strBasket)(".")
                    .Flush();
                return false;
            }
            // Now we need to find the account ID (so we can find the server
            // ID...)
            //
            auto account =
                api_.Wallet().Account(theRequestBasket->GetRequestAccountID());

            if (false == bool(account)) {
                const auto strAcctID =
                    String::Factory(theRequestBasket->GetRequestAccountID());
                LogNormal(OT_METHOD)(__FUNCTION__)(
                    ": Error: Unable to find the main account based on "
                    "the ID from the exchange request: ")(strAcctID)(".")
                    .Flush();
                return false;
            }
            // Now let's get the server ID...
            const auto serverID =
                Identifier::Factory(account.get().GetPurportedNotaryID());
            auto pServer = api_.Wallet().Server(serverID);

            if (!pServer) {
                const auto strNotaryID = String::Factory(serverID);
                LogNormal(OT_METHOD)(__FUNCTION__)(
                    ": Error: Unable to find the server based on the "
                    "exchange request: ")(strNotaryID)(".")
                    .Flush();

                return false;
            }

            auto context =
                api_.Wallet().mutable_ServerContext(theNymID, serverID);
            theRequestBasket->HarvestClosingNumbers(
                context.It(), serverID, true);

            return true;
        } else {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Error loading original basket request.")
                .Flush();
        }

        return false;
    } else if (!theMessage->LoadContractFromString(strMsg)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to load message from string.")
            .Flush();
        return false;
    }

    // By this point, we have the actual message loaded up.
    //
    const bool bSuccess = ot_api_.Msg_HarvestTransactionNumbers(
        *theMessage,
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
//    OT_ASSERT_MSG("" != NOTARY_ID, "OTAPI_Exec::HarvestClosingNumbers:
//    Null
// NOTARY_ID passed in.");
//    OT_ASSERT_MSG("" != NYM_ID, "OTAPI_Exec::HarvestClosingNumbers: Null
// NYM_ID passed in.");
//    OT_ASSERT_MSG("" != THE_CRON_ITEM, "OTAPI_Exec::HarvestClosingNumbers:
// Null THE_CRON_ITEM passed in.");
//
//    const Identifier&    theNymID(NYM_ID), theNotaryID(NOTARY_ID);
//    const OTString        strContract(THE_CRON_ITEM);
//
//    const bool& bHarvested = ot_api_.HarvestClosingNumbers(theNotaryID,
// theNymID, strContract);
//
//    return bHarvested ? true : false;
//}
//
//
// NOTE: usually you will want to call OTAPI_Exec::HarvestClosingNumbers,
// since the Opening number is usually burned up from some failed
// transaction or whatever. You don't want to harvest the opening number
// usually, just the closing numbers. (If the opening number is burned up,
// yet you still harvest it, then your OT wallet will end up using that
// number again on some other transaction, which will obviously then fail
// since the number isn't good anymore.) This function is only for those
// cases where you are sure that the opening transaction # hasn't been
// burned yet, such as when the message failed and the transaction wasn't
// attempted (because it never got that far) or such as when the contract
// simply never got signed or activated by one of the other parties, and so
// you want to claw your
// #'s back, and since in that case your opening number is still good, you
// would use the below function to get it back.
//
// bool OTAPI_Exec::HarvestAllNumbers(const std::string& NOTARY_ID,
//                                const std::string& NYM_ID,
//                                const std::string& THE_CRON_ITEM)
//{
//    OT_ASSERT_MSG("" != NOTARY_ID, "OTAPI_Exec::HarvestAllNumbers: Null
// NOTARY_ID passed in.");
//    OT_ASSERT_MSG("" != NYM_ID, "OTAPI_Exec::HarvestAllNumbers: Null
//    NYM_ID
// passed in.");
//    OT_ASSERT_MSG("" != THE_CRON_ITEM, "OTAPI_Exec::HarvestAllNumbers:
//    Null
// THE_CRON_ITEM passed in.");
//
//    const Identifier&    theNymID(NYM_ID), theNotaryID(NOTARY_ID);
//    const OTString        strContract(THE_CRON_ITEM);
//
//    const bool& bHarvested = ot_api_.HarvestAllNumbers(theNotaryID,
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
// Users will most likely store public keys of OTHER users, and they will
// need to load those from time to time, especially to verify signatures,
// etc.
//
std::string OTAPI_Exec::LoadPubkey_Encryption(
    const std::string& NYM_ID) const  // returns "", or a public key.
{
    OT_VERIFY_ID_STR(NYM_ID);

    auto strPubkey = String::Factory();  // For the output
    OTPasswordData thePWData(OT_PW_DISPLAY);
    auto nym_id = Identifier::Factory(NYM_ID);
    auto pNym = api_.Wallet().Nym(nym_id);
    if (false == bool(pNym)) return {};
    if (false == pNym->GetPublicEncrKey().GetPublicKey(strPubkey))  // bEscaped
                                                                    // defaults
                                                                    // to true.
                                                                    // 6/13/12
    {
        auto strNymID = String::Factory(nym_id);
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failure retrieving encryption pubkey."
            " From Nym: ")(strNymID)(".")
            .Flush();
    } else  // success
    {
        std::string pBuf = strPubkey->Get();
        return pBuf;
    }
    return {};
}

std::string OTAPI_Exec::LoadPubkey_Signing(
    const std::string& NYM_ID) const  // returns "", or a public key.
{
    OT_VERIFY_ID_STR(NYM_ID);

    auto strPubkey = String::Factory();  // For the output
    OTPasswordData thePWData(OT_PW_DISPLAY);
    auto nym_id = Identifier::Factory(NYM_ID);
    auto pNym = api_.Wallet().Nym(nym_id);
    if (false == bool(pNym)) return {};
    if (false == pNym->GetPublicSignKey().GetPublicKey(strPubkey)) {
        auto strNymID = String::Factory(nym_id);
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failure retrieving signing pubkey."
            " From Nym: ")(strNymID)(".")
            .Flush();
    } else  // success
    {
        std::string pBuf = strPubkey->Get();
        return pBuf;
    }
    return {};
}

// LOAD USER PUBLIC KEY  -- from local storage
//
// (return as STRING)
//
std::string OTAPI_Exec::LoadUserPubkey_Encryption(
    const std::string& NYM_ID) const  // returns "", or a public key.
{
    OT_VERIFY_ID_STR(NYM_ID);

    auto strPubkey = String::Factory();  // For the output
    auto nym_id = Identifier::Factory(NYM_ID);
    ConstNym pNym = api_.Wallet().Nym(nym_id);

    if (false == bool(pNym)) { return {}; }

    if (!pNym->GetPublicEncrKey().GetPublicKey(strPubkey)) {
        auto strNymID = String::Factory(nym_id);
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failure retrieving encryption pubkey from Nym: ")(strNymID)(".")
            .Flush();
    } else  // success
    {
        std::string pBuf = strPubkey->Get();
        return pBuf;
    }
    return {};
}

std::string OTAPI_Exec::LoadUserPubkey_Signing(
    const std::string& NYM_ID) const  // returns "", or a public key.
{
    OT_VERIFY_ID_STR(NYM_ID);

    auto strPubkey = String::Factory();  // For the output
    auto nym_id = Identifier::Factory(NYM_ID);
    // No need to cleanup.
    ConstNym pNym = api_.Wallet().Nym(nym_id);

    if (false == bool(pNym)) { return {}; }

    if (!pNym->GetPublicSignKey().GetPublicKey(strPubkey)) {
        auto strNymID = String::Factory(nym_id);
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failure retrieving signing pubkey from Nym: ")(strNymID)(".")
            .Flush();
    } else  // success
    {
        std::string pBuf = strPubkey->Get();
        return pBuf;
    }
    return {};
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
    OT_VERIFY_ID_STR(NYM_ID);

    if (!api_.Wallet().IsLocalNym(NYM_ID)) { return false; }

    return true;
}

#if OT_CASH
//
// Is Mstd::int32_t Still Good ?   true  (1) == Yes, this mint32_t is still
// good.
//                        false (0) == No: expired or other error.
//
bool OTAPI_Exec::Mint_IsStillGood(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID) const
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(INSTRUMENT_DEFINITION_ID);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theInstrumentDefinitionID =
                   Identifier::Factory(INSTRUMENT_DEFINITION_ID);
    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.
    std::unique_ptr<Mint> pMint(
        ot_api_.LoadMint(theNotaryID, theInstrumentDefinitionID));

    if (!pMint)
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failure calling OT_API::LoadMint. ")("Server: ")(NOTARY_ID)(
            " Asset Type: ")(INSTRUMENT_DEFINITION_ID)(".")
            .Flush();
    else  // success
    {
        const bool bExpired = pMint->Expired();

        if (!bExpired) { return true; }
    }
    return false;
}

std::string OTAPI_Exec::LoadMint(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID) const  // returns
                                                        // "", or a
                                                        // mint
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(INSTRUMENT_DEFINITION_ID);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID);
    const auto theInstrumentDefinitionID =
        Identifier::Factory(INSTRUMENT_DEFINITION_ID);

    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.
    std::unique_ptr<Mint> pMint(
        ot_api_.LoadMint(theNotaryID, theInstrumentDefinitionID));

    if (!pMint)
        LogNormal(OT_METHOD)(__FUNCTION__)(
            "Failure calling OT_API::LoadMint. ")("Server: ")(NOTARY_ID)(
            " Asset Type: ")(INSTRUMENT_DEFINITION_ID)(".")
            .Flush();
    else  // success
    {
        auto strOutput = String::Factory(*pMint);  // For the output
        std::string pBuf = strOutput->Get();
        return pBuf;
    }
    return {};
}
#endif  // OT_CASH

std::string OTAPI_Exec::LoadServerContract(
    const std::string& NOTARY_ID) const  // returns "", or an asset contract
{
    OT_VERIFY_ID_STR(NOTARY_ID);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID);

    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.
    auto pContract = api_.Wallet().Server(theNotaryID);

    if (!pContract) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failure calling OT_API::LoadServerContract. ")("Notary ID: ")(
            NOTARY_ID)(".")
            .Flush();
    } else  // success
    {
        auto armored = Armored::Factory(pContract->Serialize());
        auto strOutput = String::Factory();
        armored->WriteArmoredString(strOutput, "SERVER CONTRACT");
        return strOutput->Get();
    }
    return {};
}

// Some server replies (to your messages) are so important that a notice is
// dropped
// into your Nymbox with a copy of the server's reply. It's called a
// replyNotice.
// Since the server is usually replying to a message, I've added this
// function for quickly looking up the message reply, if it's there, based
// on the requestNumber. This is the only example in the entire OT API where
// a Transaction is looked-up from a ledger, based on a REQUEST NUMBER.
// (Normally transactions use transaction numbers, and messages use request
// numbers. But in this case, it's a transaction that carries a copy of a
// message.) Note: Make sure you call this AFTER you download the box
// receipts, but BEFORE you process the Nymbox (because the reply notice
// will have disappeared.) Basically this function will be used for cases
// where you missed a server reply, and you need to search your Nymbox and
// see if a copy of the missed reply is still there. (Which it won't be,
// once you process the box.)
//
std::string OTAPI_Exec::Nymbox_GetReplyNotice(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::int64_t& REQUEST_NUMBER) const  // returns replyNotice
                                               // transaction by
                                               // requestNumber.
{
    rLock lock(lock_callback_({NYM_ID, NOTARY_ID}));

    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);

    OT_VERIFY_MIN_BOUND(REQUEST_NUMBER, 0);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID);
    const auto theNymID = Identifier::Factory(NYM_ID);

    const std::int64_t lRequestNumber = REQUEST_NUMBER;
    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.

    std::unique_ptr<Ledger> pLedger(
        ot_api_.LoadNymboxNoVerify(theNotaryID, theNymID));

    if (nullptr == pLedger) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failure calling OT_API::LoadNymboxNoVerify.")
            .Flush();
        return {};
    }
    auto pTransaction =
        pLedger->GetReplyNotice(static_cast<std::int64_t>(lRequestNumber));
    // No need to cleanup this transaction, the ledger owns it already.

    if (false == bool(pTransaction)) {
        LogTrace(OT_METHOD)(__FUNCTION__)(
            ": No replyNotice transactions found "
            "in ledger with request number: ")(lRequestNumber)
            .Flush();

        return {};  // Maybe he was just looking; this isn't necessarily an
                    // error.
    }
    // At this point, I actually have the transaction pointer to the
    // replyNotice,
    // so let's return it in string form...
    //
    const std::int64_t lTransactionNum = pTransaction->GetTransactionNum();

    // Update: for transactions in ABBREVIATED form, the string is empty,
    // since it has never actually been signed (in fact the whole point
    // with abbreviated transactions in a ledger is that they take up very
    // little room, and have no signature of their own, but exist merely
    // as XML tags on their parent ledger.)
    //
    // THEREFORE I must check to see if this transaction is abbreviated and
    // if so, sign it in order to force the UpdateContents() call, so the
    // programmatic user of this API will be able to load it up.
    //

    auto strOutput =
        String::Factory(*pTransaction);  // we only have the Abbreviated, so we
                                         // have to use this one.

    if (pTransaction->IsAbbreviated()) {
        pLedger->LoadBoxReceipt(static_cast<std::int64_t>(lTransactionNum));
        auto pFullTransaction =
            pLedger->GetTransaction(static_cast<std::int64_t>(lTransactionNum));

        if (false != bool(pFullTransaction)) {
            strOutput->Release();
            strOutput->Set(
                String::Factory(*pFullTransaction));  // we have the
                                                      // FullTransaction,
                                                      // lets use
                                                      // that one.
        } else {  // nullptr == pFullTransaction
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Good index but uncovered pointer "
                "after trying to load full version of "
                "receipt (from abbreviated). Thus, saving "
                "abbreviated version instead, so I can "
                "still return SOMETHING.")
                .Flush();
            ConstNym pNym = api_.Wallet().Nym(theNymID);
            if (false == bool(pNym)) { return {}; }
            pTransaction->ReleaseSignatures();
            pTransaction->SignContract(*pNym);
            pTransaction->SaveContract();
        }
        strOutput->Release();
        pTransaction->SaveContractRaw(strOutput);  // if it was abbreviated
                                                   // before, now it either IS
                                                   // the box receipt, or it's
                                                   // the abbreviated version.
    }
    // We return the abbreviated version because the developer using the OT
    // API needs to know if that receipt is there, whether it's abbreviated
    // or not. So rather than passing "" when it's abbreviated, and thus
    // leading him astray, we give him the next-best thing: the abbreviated
    // version. That way at least he knows for sure that the receipt is
    // there, the one he is asking about.

    std::string pBuf = strOutput->Get();

    return pBuf;
}

// If the client-side has ALREADY seen the server's reply to a specific
// request number, he stores that number in a list which can be queried
// using this function.  A copy of that list is sent with nearly every
// request message sent to the server.  This way the server can see which
// replies you have already received. The server will mirror every number it
// sees (it sends its own list in all its replies.) Whenever you see a
// number mirrored in the server's reply, that means the server now knows
// you got its original reply (the one referenced by the number) and the
// server removed any replyNotice of that from your Nymbox (so you don't
// have to download it.) Basically that means you can go ahead and remove it
// from your list, and once you do, the server will remove its matching copy
// as well.
//
// *** When you are downloading your box receipts, you can skip any receipts
// where
// you have ALREADY seen the reply. So you can use this function to see if
// you already saw it, and if you did, then you can skip downloading that
// box receipt. Warning: this function isn't "perfect", in the sense that it
// cannot tell you definitively whether you have actually seen a server
// reply, but it CAN tell you if you have seen one until it finishes the
// above-described protocol (it will work in that way, which is how it was
// intended.) But after that, it will no std::int64_ter know if you got the
// reply since it has removed it from its list.
//
bool OTAPI_Exec::HaveAlreadySeenReply(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::int64_t& REQUEST_NUMBER) const  // returns
                                               // bool
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);

    OT_VERIFY_MIN_BOUND(REQUEST_NUMBER, 0);

    auto theNotaryID = Identifier::Factory(NOTARY_ID);
    auto theNymID = Identifier::Factory(NYM_ID);

    const std::int64_t lRequestNumber = REQUEST_NUMBER;

    // const std::string& strFunc = "OTAPI_Exec::HaveAlreadySeenReply";
    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.

    return ot_api_.HaveAlreadySeenReply(
        theNotaryID, theNymID, static_cast<std::int64_t>(lRequestNumber));
}

std::string OTAPI_Exec::LoadNymbox(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const  // Returns
                                      // "", or nymbox.
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID);
    const auto theNymID = Identifier::Factory(NYM_ID);

    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.
    std::unique_ptr<Ledger> pLedger(ot_api_.LoadNymbox(theNotaryID, theNymID));

    if (!pLedger) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failure calling OT_API::LoadNymbox.")
            .Flush();
    } else  // success
    {
        auto strOutput = String::Factory(*pLedger);  // For the output
        std::string pBuf = strOutput->Get();
        return pBuf;
    }

    return {};
}

std::string OTAPI_Exec::LoadNymboxNoVerify(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const  // Returns "", or nymbox.
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID);
    const auto theNymID = Identifier::Factory(NYM_ID);

    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.
    std::unique_ptr<Ledger> pLedger(
        ot_api_.LoadNymboxNoVerify(theNotaryID, theNymID));

    if (!pLedger) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failure calling OT_API::LoadNymboxNoVerify.")
            .Flush();
    } else  // success
    {
        auto strOutput = String::Factory(*pLedger);  // For the output
        std::string pBuf = strOutput->Get();
        return pBuf;
    }

    return {};
}

std::string OTAPI_Exec::LoadInbox(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID) const  // Returns "",
                                          // or an inbox.
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(ACCOUNT_ID);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID);
    const auto theNymID = Identifier::Factory(NYM_ID);
    const auto theAccountID = Identifier::Factory(ACCOUNT_ID);

    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.
    std::unique_ptr<Ledger> pLedger(
        ot_api_.LoadInbox(theNotaryID, theNymID, theAccountID));

    if (!pLedger) {
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": Failure calling OT_API::LoadInbox. Account ID: ")(ACCOUNT_ID)
            .Flush();
    } else {                                         // success
        auto strOutput = String::Factory(*pLedger);  // For the output
        std::string pBuf = strOutput->Get();
        return pBuf;
    }

    return {};
}

std::string OTAPI_Exec::LoadInboxNoVerify(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID) const  // Returns "", or an inbox.
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(ACCOUNT_ID);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID);
    const auto theNymID(NYM_ID);
    const auto theAccountID = Identifier::Factory(ACCOUNT_ID);

    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.
    std::unique_ptr<Ledger> pLedger(ot_api_.LoadInboxNoVerify(
        theNotaryID, Identifier::Factory(theNymID), theAccountID));

    if (!pLedger) {
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": Failure calling OT_API::LoadInboxNoVerify. Account ID: ")(
            ACCOUNT_ID)
            .Flush();
    } else  // success
    {
        auto strOutput = String::Factory(*pLedger);  // For the output
        std::string pBuf = strOutput->Get();
        return pBuf;
    }

    return {};
}

std::string OTAPI_Exec::LoadOutbox(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID) const
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(ACCOUNT_ID);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID);
    const auto theNymID = Identifier::Factory(NYM_ID);
    const auto theAccountID = Identifier::Factory(ACCOUNT_ID);

    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.
    std::unique_ptr<Ledger> pLedger(
        ot_api_.LoadOutbox(theNotaryID, theNymID, theAccountID));

    if (!pLedger) {
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": Failure calling OT_API::LoadOutbox(). Account ID : ")(ACCOUNT_ID)
            .Flush();
    } else  // success
    {
        auto strOutput = String::Factory(*pLedger);  // For the output
        std::string pBuf = strOutput->Get();
        return pBuf;
    }

    return {};
}

std::string OTAPI_Exec::LoadOutboxNoVerify(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID) const
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(ACCOUNT_ID);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID);
    const auto theNymID = Identifier::Factory(NYM_ID);
    const auto theAccountID = Identifier::Factory(ACCOUNT_ID);

    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.
    std::unique_ptr<Ledger> pLedger(
        ot_api_.LoadOutboxNoVerify(theNotaryID, theNymID, theAccountID));

    if (!pLedger) {
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": Failure calling OT_API::LoadOutboxNoVerify. Account ID: ")(
            ACCOUNT_ID)
            .Flush();
    } else  // success
    {
        auto strOutput = String::Factory(*pLedger);  // For the output
        std::string pBuf = strOutput->Get();
        return pBuf;
    }

    return {};
}

std::string OTAPI_Exec::LoadPaymentInbox(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const  // Returns
                                      // "", or
                                      // an
                                      // inbox.
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID);
    const auto theNymID = Identifier::Factory(NYM_ID);

    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.
    std::unique_ptr<Ledger> pLedger(
        ot_api_.LoadPaymentInbox(theNotaryID, theNymID));

    if (!pLedger) {
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": Failure calling OT_API::LoadPaymentInbox. Nym ID: ")(NYM_ID)
            .Flush();
    } else  // success
    {
        auto strOutput = String::Factory(*pLedger);  // For the output
        std::string pBuf = strOutput->Get();
        return pBuf;
    }

    return {};
}

std::string OTAPI_Exec::LoadPaymentInboxNoVerify(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const  // Returns "", or a paymentInbox.
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID);
    const auto theNymID = Identifier::Factory(NYM_ID);

    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.
    std::unique_ptr<Ledger> pLedger(
        ot_api_.LoadPaymentInboxNoVerify(theNotaryID, theNymID));

    if (!pLedger) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Failure calling ")(
            " OT_API::LoadPaymentInboxNoVerify. Nym ID: ")(NYM_ID)
            .Flush();
    } else  // success
    {
        auto strOutput = String::Factory(*pLedger);  // For the output
        std::string pBuf = strOutput->Get();
        return pBuf;
    }

    return {};
}

std::string OTAPI_Exec::LoadRecordBox(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID) const
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(ACCOUNT_ID);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID);
    const auto theNymID = Identifier::Factory(NYM_ID);
    const auto theAccountID = Identifier::Factory(ACCOUNT_ID);

    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.
    std::unique_ptr<Ledger> pLedger(
        ot_api_.LoadRecordBox(theNotaryID, theNymID, theAccountID));

    if (!pLedger) {
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": Failure calling OT_API::LoadRecordBox.")
            .Flush();
    } else  // success
    {
        auto strOutput = String::Factory(*pLedger);  // For the output
        std::string pBuf = strOutput->Get();
        return pBuf;
    }
    return {};
}

std::string OTAPI_Exec::LoadRecordBoxNoVerify(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID) const
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(ACCOUNT_ID);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID);
    const auto theNymID = Identifier::Factory(NYM_ID);
    const auto theAccountID = Identifier::Factory(ACCOUNT_ID);

    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.
    std::unique_ptr<Ledger> pLedger(
        ot_api_.LoadRecordBoxNoVerify(theNotaryID, theNymID, theAccountID));

    if (!pLedger) {
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": Failure calling OT_API::LoadRecordBoxNoVerify.")
            .Flush();
    } else  // success
    {
        const auto strOutput = String::Factory(*pLedger);  // For the output
        std::string pBuf = strOutput->Get();
        return pBuf;
    }
    return {};
}

std::string OTAPI_Exec::LoadExpiredBox(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID);
    const auto theNymID = Identifier::Factory(NYM_ID);

    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.
    std::unique_ptr<Ledger> pLedger(
        ot_api_.LoadExpiredBox(theNotaryID, theNymID));

    if (!pLedger) {
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": Failure calling OT_API::LoadExpiredBox.")
            .Flush();
    } else  // success
    {
        auto strOutput = String::Factory(*pLedger);  // For the output
        std::string pBuf = strOutput->Get();
        return pBuf;
    }
    return {};
}

std::string OTAPI_Exec::LoadExpiredBoxNoVerify(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const  // Returns nullptr, or a ExpiredBox.
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID);
    const auto theNymID = Identifier::Factory(NYM_ID);

    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.
    std::unique_ptr<Ledger> pLedger(
        ot_api_.LoadExpiredBoxNoVerify(theNotaryID, theNymID));

    if (!pLedger) {
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": Failure calling OT_API::LoadExpiredBoxNoVerify.")
            .Flush();
    } else  // success
    {
        const auto strOutput = String::Factory(*pLedger);  // For the output
        std::string pBuf = strOutput->Get();
        return pBuf;
    }
    return {};
}

bool OTAPI_Exec::RecordPayment(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const bool& bIsInbox,         // true == payments inbox. false == payments
                                  // outbox.
    const std::int32_t& nIndex,   // removes payment instrument (from
                                  // payments in or out box) and moves to
                                  // record box.
    const bool& bSaveCopy) const  // If false, then will NOT save a copy to
                                  // record box.
{
    OT_VERIFY_MIN_BOUND(nIndex, 0);
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID);
    const auto theNymID = Identifier::Factory(NYM_ID);

    return ot_api_.RecordPayment(
        theNotaryID, theNymID, bIsInbox, nIndex, bSaveCopy);
}

bool OTAPI_Exec::ClearRecord(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,  // NYM_ID can be passed here as well.
    const std::int32_t& nIndex,
    const bool& bClearAll) const  // if true, nIndex is ignored.
{
    OT_VERIFY_MIN_BOUND(nIndex, 0);
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(ACCOUNT_ID);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID);
    const auto theNymID = Identifier::Factory(NYM_ID);
    const auto theAcctID = Identifier::Factory(ACCOUNT_ID);

    return ot_api_.ClearRecord(
        theNotaryID, theNymID, theAcctID, nIndex, bClearAll);
}

bool OTAPI_Exec::ClearExpired(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::int32_t& nIndex,
    const bool& bClearAll) const  // if true, nIndex is
                                  // ignored.
{
    OT_VERIFY_MIN_BOUND(nIndex, 0);
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID);
    const auto theNymID = Identifier::Factory(NYM_ID);

    return ot_api_.ClearExpired(theNotaryID, theNymID, nIndex, bClearAll);
}

// Returns number of transactions within, or -1 for error.
std::int32_t OTAPI_Exec::Ledger_GetCount(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_LEDGER) const
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(ACCOUNT_ID);
    OT_VERIFY_STD_STR(THE_LEDGER);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theNymID = Identifier::Factory(NYM_ID),
               theAccountID = Identifier::Factory(ACCOUNT_ID);
    auto strLedger = String::Factory(THE_LEDGER);
    auto theLedger{api_.Factory().Ledger(theNymID, theAccountID, theNotaryID)};

    if (!theLedger->LoadLedgerFromString(strLedger)) {
        auto strAcctID = String::Factory(theAccountID);
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error loading ledger from string."
                                           " Acct ID: ")(strAcctID)(".")
            .Flush();
        return OT_ERROR;
    }

    return ot_api_.Ledger_GetCount(*theLedger);
}

// Returns a comma-separated list of transaction numbers
// (one for each of the receipts in this ledger).
//
std::string OTAPI_Exec::Ledger_GetTransactionNums(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_LEDGER) const
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(ACCOUNT_ID);
    OT_VERIFY_STD_STR(THE_LEDGER);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theNymID = Identifier::Factory(NYM_ID),
               theAccountID = Identifier::Factory(ACCOUNT_ID);
    auto strLedger = String::Factory(THE_LEDGER);
    auto theLedger{api_.Factory().Ledger(theNymID, theAccountID, theNotaryID)};

    if (!theLedger->LoadLedgerFromString(strLedger)) {
        auto strAcctID = String::Factory(theAccountID);
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error loading ledger from string."
                                           " Acct ID: ")(strAcctID)(".")
            .Flush();
        return {};
    }

    NumList numList{ot_api_.Ledger_GetTransactionNums(*theLedger)};

    if (numList.Count() <= 0) return {};
    // -------------------------------------------
    auto strOutput = String::Factory();
    if (numList.Output(strOutput)) return std::string{strOutput->Get()};

    return {};
}

// Creates a new 'response' ledger, set up with the right Notary ID, etc, so
// you can add the 'response' items to it, one by one.
//
// The original ledger is your inbox. Inbox receipts are the only things you
// would ever create a "response" to, as part of your "process inbox"
// process.
//
std::string OTAPI_Exec::Ledger_CreateResponse(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID) const
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(ACCOUNT_ID);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theNymID = Identifier::Factory(NYM_ID),
               theAccountID = Identifier::Factory(ACCOUNT_ID);
    // -------------------------------------------------------
    OT_API::ProcessInbox response{
        ot_api_.Ledger_CreateResponse(theNotaryID, theNymID, theAccountID)};
    // -------------------------------------------------------
    const auto& processInbox = std::get<0>(response);
    const auto& inbox = std::get<1>(response);
    // -------------------------------------------------------
    auto nym = api_.Wallet().Nym(theNymID);  // Sanity check.
    OT_ASSERT(nym);

    bool success{(bool(processInbox) && bool(inbox))};
    success &= (success && processInbox->SignContract(*nym));
    success &= (success && processInbox->SaveContract());

    if (success) {

        return String::Factory(*processInbox)->Get();
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed in the call to ot_api_.Ledger_CreateResponse.")
            .Flush();
    }
    // -------------------------------------------------------
    return {};
}

// Lookup a transaction or its ID (from within a ledger) based on index or
// transaction number.
//
// The transaction number is returned as a string, because I return ALL
// std::int64_t
// ints as a string (in the API.)  In C, you can just call StringToLong to
// convert it back.
// This makes it easier for the guys who use scripting languages. (This file
// is primarily for them. If you are lower-level, you should use
// OpenTransactions.h/.cpp
// and then use this one as a model for how to do it. Or use this one if you
// insist on straight C, since all these functions are extern "C".)
//
std::string OTAPI_Exec::Ledger_GetTransactionByIndex(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_LEDGER,
    const std::int32_t& nIndex) const  // returns transaction by index (from
                                       // ledger)
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(ACCOUNT_ID);
    OT_VERIFY_STD_STR(THE_LEDGER);
    OT_VERIFY_MIN_BOUND(nIndex, 0);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theNymID = Identifier::Factory(NYM_ID),
               theAccountID = Identifier::Factory(ACCOUNT_ID);
    // -------------------------------------------------------
    auto strLedger = String::Factory(THE_LEDGER);
    auto theLedger{api_.Factory().Ledger(theNymID, theAccountID, theNotaryID)};
    //  std::set<std::int64_t> setUnloaded;

    if (!theLedger->LoadLedgerFromString(strLedger)
        //      || !theLedger->LoadBoxReceipts(&setUnloaded)
        // This is done below, for the individual transaction,
        // for better optimization.
    ) {
        auto strAcctID = String::Factory(theAccountID);
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading ledger from string, or loading box "
            "receipts subsequently. Acct ID: ")(strAcctID)(".")
            .Flush();
        return {};
    }
    // At this point, I know theLedger loaded successfully.
    // ------------------------------------------------------
    OTTransaction* pTransaction =
        ot_api_.Ledger_GetTransactionByIndex(*theLedger, nIndex);

    if (nullptr == pTransaction) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failure: Uncovered nullptr transaction at index: ")(nIndex)(".")
            .Flush();
        return {};  // Weird.
    }
    // ----------------------------------------------
    if (pTransaction->IsAbbreviated()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Returning empty string, since box receipt wasn't "
            "downloaded yet. The caller of this function should have "
            "first called DoesBoxReceiptExist() if he didn't want to "
            "see this log.")
            .Flush();

        return {};
    }
    // ----------------------------------------------
    // Below this point I know the string won't be empty and the
    // box receipt was there. So I return it in string form.
    //
    const auto strOutput = String::Factory(*pTransaction);  // For the output
    return strOutput->Get();
}

// Returns transaction by ID (transaction numbers are std::int64_t ints, and
// thus
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
    const std::int64_t& TRANSACTION_NUMBER) const
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(ACCOUNT_ID);
    OT_VERIFY_STD_STR(THE_LEDGER);
    OT_VERIFY_MIN_BOUND(TRANSACTION_NUMBER, 1);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theNymID = Identifier::Factory(NYM_ID),
               theAccountID = Identifier::Factory(ACCOUNT_ID);

    auto strLedger = String::Factory(THE_LEDGER);
    auto theLedger{api_.Factory().Ledger(theNymID, theAccountID, theNotaryID)};

    if (!theLedger->LoadLedgerFromString(strLedger)) {
        auto strAcctID = String::Factory(theAccountID);
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading ledger from string. "
            "Acct ID: ")(strAcctID)(".")
            .Flush();

        return {};
    }
    // At this point, I know theLedger loaded successfully.
    // ----------------------------------------------
    // This may return the full version (from a loaded box receipt)
    // or an abbreviated version (if the full one wasn't available yet,
    // such as, for example, if it hasn't been downloaded yet...)
    //
    OTTransaction* pTransaction =
        ot_api_.Ledger_GetTransactionByID(*theLedger, TRANSACTION_NUMBER);
    // No need to cleanup this transaction, the ledger owns it already.

    if (nullptr == pTransaction) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": No transaction found in ledger with that number: ")(
            TRANSACTION_NUMBER)(".")
            .Flush();

        return {};  // Maybe he was just looking; this isn't
                    // necessarily an error.
    }
    // ----------------------------------------------
    // At this point, I actually have the transaction pointer, and we're
    // supposed to return it in string form. However, let's remember that
    // if it's abbreviated (because box receipt hasn't been downloaded yet)
    // then the "string contents" of the transaction are EMPTY.
    // But I don't think we want to return an empty string in that case.
    // Rather, we'd want to return the abbreviated -- ah. But we can't,
    // without signing/saving it, when the inbox items are expected to be
    // signed by the server, not the client.
    // Therefore we WILL return a blank string, if it's abbreviated, and
    // it's up to the caller to use DoesBoxReceiptExist() or use a lower-
    // level call like OT_API::Ledger_GetTransactionByID (which we just
    // used above...)
    // ----------------------------------------------
    if (pTransaction->IsAbbreviated()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Returning empty string, since box receipt wasn't "
            "downloaded yet. The caller of this function should have "
            "first called DoesBoxReceiptExist() if he didn't want to "
            "see this log.")
            .Flush();

        return {};
    }
    // ----------------------------------------------
    // Below this point I know the string won't be empty and the
    // box receipt was there. So I return it in string form.
    //
    const auto strOutput = String::Factory(*pTransaction);  // For the output
    return strOutput->Get();
}

// OTAPI_Exec::Ledger_GetInstrument (by index)
//
// Lookup a financial instrument (from within a transaction that is inside
// a paymentInbox ledger) based on index.
/*
sendNymInstrument does this:
-- Puts instrument (a contract string) as encrypted Payload on an
OTMessage(1).
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
-- Each transaction contains (1) OTMessage in the "in ref to" field, which
in turn contains an encrypted instrument in the messagePayload field.
-- *** Therefore, this function, based purely on ledger index (as we
iterate) extracts the OTMessage from the Transaction "in ref to" field (for
the transaction at that index), then decrypts the payload on that message
and returns the decrypted cleartext.
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
    const std::int32_t& nIndex) const  // returns financial instrument by index.
{
    return Ledger_GetInstrument_lowlevel(
        true,  // index_or_receipt
        NOTARY_ID,
        NYM_ID,
        ACCOUNT_ID,
        THE_LEDGER,
        nIndex,
        0);  // lReceiptId is 0 in this case. (Unused in this case).
}

std::string OTAPI_Exec::Ledger_GetInstrumentByReceiptID(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_LEDGER,
    const std::int64_t& lReceiptId) const  // returns financial instrument by
                                           // receiptID.
{
    return Ledger_GetInstrument_lowlevel(
        false,  // index_or_receipt
        NOTARY_ID,
        NYM_ID,
        ACCOUNT_ID,
        THE_LEDGER,
        -1,  // nIndex is -1 in this case. (Unused in this case).
        lReceiptId);
}

std::string OTAPI_Exec::Ledger_GetInstrument_lowlevel(
    const bool index_or_receipt,
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_LEDGER,
    const std::int32_t& nIndex,
    const std::int64_t& lReceiptId) const
{
    const bool bUsingIndex{index_or_receipt};
    const bool bUsingReceipt{!index_or_receipt};

    if (bUsingIndex) {  // nIndex
        OT_VERIFY_MIN_BOUND(nIndex, 0);
    } else {  // bUsingReceipt
        OT_VERIFY_MIN_BOUND(lReceiptId, 1);
    }
    // ----------------------------------------------------
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(ACCOUNT_ID);
    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theNymID = Identifier::Factory(NYM_ID),
               theAccountID = Identifier::Factory(ACCOUNT_ID);
    // ----------------------------------------------------
    OT_VERIFY_STD_STR(THE_LEDGER);
    auto strLedger = String::Factory(THE_LEDGER);
    auto theLedger{api_.Factory().Ledger(theNymID, theAccountID, theNotaryID)};

    if (!theLedger->LoadLedgerFromString(strLedger)
        //      || !theLedger->LoadBoxReceipts(&setUnloaded)    // This is
        // now done below, for the individual transaction, for better
        // optimization.
        )  // Update: now in the theLedger.GetInstrument call.
    {
        auto strNymID = String::Factory(theNymID);
        auto strAcctID = String::Factory(theAccountID);
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading ledger from string. NymID / Acct ID: ")(strNymID)(
            " / ")(strAcctID)(".")
            .Flush();
        return {};
    }
    // At this point, I know theLedger loaded successfully.
    // ----------------------------------------------------
    std::shared_ptr<OTPayment> pPayment =
        (bUsingReceipt
             ? ot_api_.Ledger_GetInstrumentByReceiptID(
                   theNymID, *theLedger, lReceiptId)
             : ot_api_.Ledger_GetInstrument(theNymID, *theLedger, nIndex));

    if (false == bool(pPayment)) { return {}; }
    // NOTE: instead of loading up an OTPayment, and then loading a
    // cheque/purse/etc from it, we just send the cheque/purse/etc
    // directly and use it to construct the OTPayment.
    // (Saves a step.)
    //
    auto strPaymentContents = String::Factory();
    if (!pPayment->GetPaymentContents(strPaymentContents)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed retrieving payment instrument "
            "from OTPayment object.")
            .Flush();
        return {};
    }
    return strPaymentContents->Get();
}

/*

// returns the message, optionally with Subject: as first line.

 std::string OTAPI_Exec::GetNym_MailContentsByIndex(const std::string&
NYM_ID, const std::int32_t& nIndex)
{
    OT_ASSERT_MSG("" != NYM_ID, "Null NYM_ID passed to
OTAPI_Exec::GetNym_MailContentsByIndex");

    std::string strFunc = "OTAPI_Exec::GetNym_MailContentsByIndex";
    OTIdentifier    theNymID(NYM_ID);
    Nym * pNym = ot_api_.GetNym(theNymID, strFunc);
    if (nullptr == pNym) return {};
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
    return {};
}

*/

// Returns a transaction number, or -1 for error.
std::int64_t OTAPI_Exec::Ledger_GetTransactionIDByIndex(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_LEDGER,
    const std::int32_t& nIndex) const  // returns transaction number by index.
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(ACCOUNT_ID);
    OT_VERIFY_STD_STR(THE_LEDGER);
    OT_VERIFY_MIN_BOUND(nIndex, 0);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theNymID = Identifier::Factory(NYM_ID),
               theAccountID = Identifier::Factory(ACCOUNT_ID);

    auto theLedger{api_.Factory().Ledger(theNymID, theAccountID, theNotaryID)};
    auto strLedger = String::Factory(THE_LEDGER);
    if (!theLedger->LoadLedgerFromString(strLedger)) {
        auto strAcctID = String::Factory(theAccountID);
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading ledger from string. Acct ID: ")(strAcctID)(".")
            .Flush();
        return -1;
    }
    // At this point, I know theLedger loaded successfully.
    // -------------------------------------------------------
    const std::int64_t lTransactionNumber =
        ot_api_.Ledger_GetTransactionIDByIndex(*theLedger, nIndex);
    // -------------------------------------------------------
    return lTransactionNumber;
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
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(ACCOUNT_ID);
    // ----------------------------------------------
    OT_VERIFY_STD_STR(THE_LEDGER);
    OT_VERIFY_STD_STR(THE_TRANSACTION);
    // ----------------------------------------------
    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theNymID = Identifier::Factory(NYM_ID),
               theAccountID = Identifier::Factory(ACCOUNT_ID);

    auto strLedger = String::Factory(THE_LEDGER);
    auto strTransaction = String::Factory(THE_TRANSACTION);
    // ----------------------------------------------
    auto theLedger{api_.Factory().Ledger(theNymID, theAccountID, theNotaryID)};

    if (!theLedger->LoadLedgerFromString(strLedger)) {
        auto strAcctID = String::Factory(theAccountID);
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading ledger from string. "
            "Acct ID: ")(strAcctID)(".")
            .Flush();
        return {};
    }
    // We don't do theLedger.VerifyAccount since our call to
    // OT_API::Ledger_AddTransaction will handle that.
    // ----------------------------------------------
    // At this point, I know theLedger loaded successfully.
    //
    auto pTransaction{
        api_.Factory().Transaction(theNymID, theAccountID, theNotaryID)};

    if (false == bool(pTransaction)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error allocating memory: "
                                           "pTransaction!")
            .Flush();
        return {};
    }
    // ----------------------------------------------
    if (!pTransaction->LoadContractFromString(strTransaction)) {
        auto strAcctID = String::Factory(theAccountID);
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading transaction from string. Acct ID: ")(strAcctID)(
            ".")
            .Flush();
        return {};
    }
    // We don't do pTransaction->VerifyAccount since our call to
    // OT_API::Ledger_AddTransaction will handle that.
    // ----------------------------------------------
    // theLedger takes ownership of pTransaction.
    //
    const bool bAdded =
        ot_api_.Ledger_AddTransaction(theNymID, *theLedger, pTransaction);

    if (!bAdded) return {};
    // ----------------------------------------------
    auto strOutput = String::Factory(*theLedger);  // For the output
    std::string pBuf = strOutput->Get();
    return pBuf;
}

// Create a 'response' transaction, that will be used to indicate my
// acceptance or rejection of another transaction. Usually an entire ledger
// full of these is sent to the server as I process the various transactions
// in my inbox.
//
// The original transaction is passed in, and I generate a response
// transaction based on it. Also, the response ledger is passed in, so I
// load it, add the response transaction, save it back to string, and return
// the string. (So the return string is the updated response ledger).
//
// This way, users can call this function multiple times, adding
// transactions until done.

std::string OTAPI_Exec::Transaction_CreateResponse(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_LEDGER,       // 'Response' ledger be sent to the
                                         // server...
    const std::string& THE_TRANSACTION,  // Responding to...?
    const bool& BOOL_DO_I_ACCEPT) const  // 0 or 1  (true or false.)
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(ACCOUNT_ID);
    // ----------------------------------------------
    OT_VERIFY_STD_STR(THE_LEDGER);
    OT_VERIFY_STD_STR(THE_TRANSACTION);
    // ----------------------------------------------
    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theNymID = Identifier::Factory(NYM_ID),
               theAcctID = Identifier::Factory(ACCOUNT_ID);

    auto strLedger = String::Factory(THE_LEDGER);
    auto responseLedger{
        api_.Factory().Ledger(theNymID, theAcctID, theNotaryID)};

    OT_ASSERT(false != bool(responseLedger));

    if (!responseLedger->LoadLedgerFromString(strLedger)) {
        auto strAcctID = String::Factory(theAcctID);
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading response ledger from string. "
            "Acct ID: ")(strAcctID)(".")
            .Flush();
        return {};
    }
    // ----------------------------------------------
    // At this point, I know responseLedger loaded and verified
    // successfully. (This is the 'response' ledger that the user previously
    // generated, and now he is loading it up with responses that this
    // function will generate on his behalf.)
    //
    auto strTransaction = String::Factory(THE_TRANSACTION);
    auto originalTransaction{
        api_.Factory().Transaction(theNymID, theAcctID, theNotaryID)};

    if (!originalTransaction->LoadContractFromString(strTransaction)) {
        auto strAcctID = String::Factory(theAcctID);
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading transaction from string. Acct ID: ")(strAcctID)(
            ".")
            .Flush();
        return {};
    }
    // ----------------------------------------------
    const bool bCreatedResponse =

        ot_api_.Transaction_CreateResponse(
            theNotaryID,
            theNymID,
            theAcctID,
            *responseLedger,
            *originalTransaction,
            BOOL_DO_I_ACCEPT);

    if (!bCreatedResponse) {
        auto strAcctID = String::Factory(theAcctID);
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed creating response to inbox transaction."
            " Acct ID: ")(strAcctID)(".")
            .Flush();

        return {};
    }
    // ----------------------------------------------
    // Get the processInbox transaction from the response
    // ledger (should be there...)
    // Sign and save it, as well as the response ledger.
    // Note that we do this here because we have to return it
    // in string form. But the lower-level version of this call
    // (above), ot_api_.Transaction_CreateResponse, does NOT
    // sign and save, since really it doesn't have to be done
    // until the finalize step (where it's done anyway).
    // So it goes without saying that this "stringified" higher
    // level call we're in now is alot less optimized.
    //
    auto processInbox =
        responseLedger->GetTransaction(transactionType::processInbox);
    bool output = (nullptr != processInbox);

    if (output) {
        // ----------------------------------------------
        auto context = api_.Wallet().ServerContext(theNymID, theNotaryID);
        auto& nym = *context->Nym();
        // ----------------------------------------------
        processInbox->ReleaseSignatures();
        output &= (output && processInbox->SignContract(nym));
        output &= (output && processInbox->SaveContract());

        if (output) {
            responseLedger->ReleaseSignatures();
            output &= (output && responseLedger->SignContract(nym));
            output &= (output && responseLedger->SaveContract());
            if (output) { return String::Factory(*responseLedger)->Get(); }
        }
    }
    return {};
}

// (Response Ledger) LEDGER FINALIZE RESPONSE
//
// AFTER you have set up all the transaction responses, call THIS function
// to finalize them by adding a BALANCE AGREEMENT.
//
// MAKE SURE you have the latest copy of the account file, inbox file, and
// outbox file, since we will need those in here to create the balance
// statement properly.
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
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(ACCOUNT_ID);
    OT_VERIFY_STD_STR(THE_LEDGER);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theNymID = Identifier::Factory(NYM_ID),
               theAcctID = Identifier::Factory(ACCOUNT_ID);
    auto strLedger = String::Factory(THE_LEDGER);
    // -------------------------------------------------------
    // By this point, pNym is a good pointer, and is on the wallet.
    // (No need to cleanup.)
    auto responseLedger{
        api_.Factory().Ledger(theNymID, theAcctID, theNotaryID)};

    if (!responseLedger->LoadLedgerFromString(strLedger)) {
        auto strAcctID = String::Factory(theAcctID);
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading ledger from string. Acct ID: ")(strAcctID)(".")
            .Flush();

        return {};
    }
    // -------------------------------------------------------
    const bool finalized = ot_api_.Ledger_FinalizeResponse(
        theNotaryID, theNymID, theAcctID, *responseLedger);

    if (false == finalized) {
        auto strAcctID = String::Factory(theAcctID);
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed finalizing message ledger for processInbox."
            " Acct ID: ")(strAcctID)(".")
            .Flush();

        return {};
    }
    // ----------------------------------------------
    return String::Factory(*responseLedger)->Get();
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
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(ACCOUNT_ID);
    OT_VERIFY_STD_STR(THE_TRANSACTION);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theNymID = Identifier::Factory(NYM_ID),
               theAccountID = Identifier::Factory(ACCOUNT_ID);
    const auto strTransaction = String::Factory(THE_TRANSACTION);
    auto strOutput = String::Factory();

    if (!api_.Wallet().IsLocalNym(NYM_ID)) { return {}; }

    auto theTransaction{
        api_.Factory().Transaction(theNymID, theAccountID, theNotaryID)};

    if (!theTransaction->LoadContractFromString(strTransaction)) {
        auto strAcctID = String::Factory(theAccountID);
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading transaction from string. Acct ID: ")(strAcctID)(
            ".")
            .Flush();
        return {};
    }
    // No need to check if transaction is abbreviated, since it's coming
    // from a message ledger. (Those always contain the full version of the
    // transactions, automatically.)

    if (transactionType::atWithdrawal != theTransaction->GetType()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error: tried to retrieve voucher from "
            "wrong transaction (not atWithdrawal).")
            .Flush();
        return {};
    }

    // loop through the ALL items that make up this transaction and check to
    // see if a response to withdrawal.

    // if pointer not null, and it's a withdrawal, and it's an
    // acknowledgement (not a rejection or error)
    for (auto& it : theTransaction->GetItemList()) {
        auto pItem = it;
        if (false == bool(pItem)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Pointer: "
                                               "pItem should not have been.")
                .Flush();
            return {};
        }

        if ((itemType::atWithdrawVoucher == pItem->GetType()) &&
            (Item::acknowledgement == pItem->GetStatus())) {
            auto strVoucher = String::Factory();
            pItem->GetAttachment(strVoucher);
            auto theVoucher{api_.Factory().Cheque()};

            if (theVoucher->LoadContractFromString(strVoucher))  // Todo
                                                                 // additional
                                                                 // verification
                                                                 // here on the
                                                                 // cheque.
            {
                theVoucher->SaveContractRaw(strOutput);
                break;
            }
        }
    }

    // Didn't find one.
    if (!strOutput->Exists()) return {};

    // We found a voucher -- let's return it!
    //
    std::string pBuf = strOutput->Get();

    return pBuf;
}

std::string OTAPI_Exec::Transaction_GetSenderNymID(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION) const
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(ACCOUNT_ID);
    OT_VERIFY_STD_STR(THE_TRANSACTION);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theNymID = Identifier::Factory(NYM_ID),
               theAccountID = Identifier::Factory(ACCOUNT_ID);
    const auto strTransaction = String::Factory(THE_TRANSACTION);

    if (!api_.Wallet().IsLocalNym(NYM_ID)) { return {}; }

    auto theTransaction{
        api_.Factory().Transaction(theNymID, theAccountID, theNotaryID)};

    if (!theTransaction->LoadContractFromString(strTransaction)) {
        auto strAcctID = String::Factory(theAccountID);
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading transaction from string. Acct ID: ")(strAcctID)(
            ".")
            .Flush();
        return {};
    }
    std::unique_ptr<OTTransaction> pTransaction = nullptr;

    if (theTransaction->IsAbbreviated()) {
        std::int64_t lBoxType = 0;

        if (theTransaction->Contains("nymboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::nymbox);
        else if (theTransaction->Contains("inboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::inbox);
        else if (theTransaction->Contains("outboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::outbox);
        else if (theTransaction->Contains("paymentInboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::paymentInbox);
        else if (theTransaction->Contains("recordBoxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::recordBox);
        else if (theTransaction->Contains("expiredBoxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::expiredBox);
        else {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error loading from abbreviated "
                "transaction: Unknown ledger type.")
                .Flush();
            return {};
        }
        pTransaction = LoadBoxReceipt(*theTransaction, lBoxType);
        if (false == bool(pTransaction)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error loading from abbreviated "
                "transaction: Failed loading box "
                "receipt.")
                .Flush();
            return {};
        }
    } else
        pTransaction.reset(theTransaction.release());
    auto theOutput = Identifier::Factory();

    bool bSuccess = pTransaction->GetSenderNymIDForDisplay(theOutput);

    if (bSuccess) {
        auto strOutput = String::Factory(theOutput);

        // Didn't find one.
        if (!strOutput->Exists()) return {};

        // We found it -- let's return the user ID
        //
        return theOutput->str();
    } else
        return {};
}

std::string OTAPI_Exec::Transaction_GetRecipientNymID(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION) const
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(ACCOUNT_ID);
    OT_VERIFY_STD_STR(THE_TRANSACTION);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theNymID = Identifier::Factory(NYM_ID),
               theAccountID = Identifier::Factory(ACCOUNT_ID);

    auto strTransaction = String::Factory(THE_TRANSACTION);

    if (!api_.Wallet().IsLocalNym(NYM_ID)) { return {}; }

    // By this point, pNym is a good pointer, and is on the wallet. (No need
    // to cleanup.)
    auto theTransaction{
        api_.Factory().Transaction(theNymID, theAccountID, theNotaryID)};

    if (!theTransaction->LoadContractFromString(strTransaction)) {
        auto strAcctID = String::Factory(theAccountID);
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading transaction from string. Acct ID: ")(strAcctID)(
            ".")
            .Flush();
        return {};
    }
    std::unique_ptr<OTTransaction> pTransaction = nullptr;

    if (theTransaction->IsAbbreviated()) {
        std::int64_t lBoxType = 0;

        if (theTransaction->Contains("nymboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::nymbox);
        else if (theTransaction->Contains("inboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::inbox);
        else if (theTransaction->Contains("outboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::outbox);
        else if (theTransaction->Contains("paymentInboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::paymentInbox);
        else if (theTransaction->Contains("recordBoxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::recordBox);
        else if (theTransaction->Contains("expiredBoxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::expiredBox);
        else {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error loading from abbreviated "
                "transaction: Unknown ledger type.")
                .Flush();
            return {};
        }
        pTransaction = LoadBoxReceipt(*theTransaction, lBoxType);
        if (false == bool(pTransaction)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error loading from abbreviated "
                "transaction: Failed loading box receipt.")
                .Flush();
            return {};
        }
    } else
        pTransaction.reset(theTransaction.release());

    auto theOutput = Identifier::Factory();

    bool bSuccess = pTransaction->GetRecipientNymIDForDisplay(theOutput);

    // Normally, there IS NO recipient user ID for a transfer (only a
    // recipient acct ID.) But here, as a special trick, I'll see if the
    // account is in my address book. THIS MEANS THE ADDRESS BOOK needs to
    // store nyms (for other people, their public nym) as well as a list of
    // acct IDs that I have associated with each Nym. That way, I can later
    // look up the Nym ID based on the acct ID, and then look up my display
    // label for that Nym.
    //
    //    if (!bSuccess && (theTransaction->GetType() ==
    //    transactionType::pending))
    //    {
    //        // AS SOON AS ADDRESS BOOK FEATURE IS ADDED,
    //        // THEN THIS CAN BE COMPLETED HERE.
    //    }

    if (bSuccess) {
        auto strOutput = String::Factory(theOutput);

        // Didn't find one.
        if (!strOutput->Exists()) return {};

        // We found it -- let's return the user ID
        //
        return theOutput->str();
    } else
        return {};
}

std::string OTAPI_Exec::Transaction_GetSenderAcctID(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION) const
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(ACCOUNT_ID);
    OT_VERIFY_STD_STR(THE_TRANSACTION);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theNymID = Identifier::Factory(NYM_ID),
               theAccountID = Identifier::Factory(ACCOUNT_ID);

    auto strTransaction = String::Factory(THE_TRANSACTION);

    if (!api_.Wallet().IsLocalNym(NYM_ID)) { return {}; }

    auto theTransaction{
        api_.Factory().Transaction(theNymID, theAccountID, theNotaryID)};

    if (!theTransaction->LoadContractFromString(strTransaction)) {
        auto strAcctID = String::Factory(theAccountID);
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading transaction from string. Acct ID: ")(strAcctID)(
            ".")
            .Flush();
        return {};
    }
    std::unique_ptr<OTTransaction> pTransaction = nullptr;

    if (theTransaction->IsAbbreviated()) {
        std::int64_t lBoxType = 0;

        if (theTransaction->Contains("nymboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::nymbox);
        else if (theTransaction->Contains("inboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::inbox);
        else if (theTransaction->Contains("outboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::outbox);
        else if (theTransaction->Contains("paymentInboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::paymentInbox);
        else if (theTransaction->Contains("recordBoxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::recordBox);
        else if (theTransaction->Contains("expiredBoxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::expiredBox);
        else {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error loading from abbreviated "
                "transaction: Unknown ledger type.")
                .Flush();
            return {};
        }
        pTransaction = LoadBoxReceipt(*theTransaction, lBoxType);
        if (false == bool(pTransaction)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error loading from abbreviated "
                "transaction: Failed loading box receipt.")
                .Flush();
            return {};
        }
    } else
        pTransaction.reset(theTransaction.release());

    auto theOutput = Identifier::Factory();

    bool bSuccess = pTransaction->GetSenderAcctIDForDisplay(theOutput);

    if (bSuccess) {
        auto strOutput = String::Factory(theOutput);

        // Didn't find one.
        if (!strOutput->Exists()) return {};

        // We found it -- let's return the user ID
        //
        std::string pBuf = strOutput->Get();

        return pBuf;
    } else
        return {};
}

std::string OTAPI_Exec::Transaction_GetRecipientAcctID(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION) const
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(ACCOUNT_ID);
    OT_VERIFY_STD_STR(THE_TRANSACTION);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theNymID = Identifier::Factory(NYM_ID),
               theAccountID = Identifier::Factory(ACCOUNT_ID);

    auto strTransaction = String::Factory(THE_TRANSACTION);

    if (!api_.Wallet().IsLocalNym(NYM_ID)) { return {}; }

    auto theTransaction{
        api_.Factory().Transaction(theNymID, theAccountID, theNotaryID)};

    if (!theTransaction->LoadContractFromString(strTransaction)) {
        auto strAcctID = String::Factory(theAccountID);
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading transaction from string in "
            "OTAPI_Exec::Transaction_GetRecipientAcctID. "
            "Acct ID: ")(strAcctID)(".")
            .Flush();
        return {};
    }

    std::unique_ptr<OTTransaction> pTransaction = nullptr;

    if (theTransaction->IsAbbreviated()) {
        std::int64_t lBoxType = 0;

        if (theTransaction->Contains("nymboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::nymbox);
        else if (theTransaction->Contains("inboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::inbox);
        else if (theTransaction->Contains("outboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::outbox);
        else if (theTransaction->Contains("paymentInboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::paymentInbox);
        else if (theTransaction->Contains("recordBoxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::recordBox);
        else if (theTransaction->Contains("expiredBoxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::expiredBox);
        else {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error loading from abbreviated "
                "transaction: Unknown ledger type.")
                .Flush();
            return {};
        }
        pTransaction = LoadBoxReceipt(*theTransaction, lBoxType);
        if (false == bool(pTransaction)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error loading from abbreviated "
                "transaction: Failed loading box "
                "receipt.")
                .Flush();
            return {};
        }
    } else
        pTransaction.reset(theTransaction.release());
    auto theOutput = Identifier::Factory();

    bool bSuccess = pTransaction->GetRecipientAcctIDForDisplay(theOutput);

    if (bSuccess) {
        auto strOutput = String::Factory(theOutput);

        // Didn't find one.
        if (!strOutput->Exists()) return {};

        // We found it -- let's return the user ID
        //
        return theOutput->str();
    } else
        return {};
}

//
// PENDING TRANSFER (various functions)
//
// When someone has sent you a PENDING TRANSFER (a form of transaction
// that will be sitting in your inbox waiting for you to accept/reject it)
// then, as you are reading the inbox, you can use these functions in
// order to get data from the pending transfer.
std::string OTAPI_Exec::Pending_GetNote(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION) const
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(ACCOUNT_ID);
    OT_VERIFY_STD_STR(THE_TRANSACTION);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theNymID = Identifier::Factory(NYM_ID),
               theAccountID = Identifier::Factory(ACCOUNT_ID);

    auto strTransaction = String::Factory(THE_TRANSACTION);
    if (!api_.Wallet().IsLocalNym(NYM_ID)) { return {}; }

    auto theTransaction{
        api_.Factory().Transaction(theNymID, theAccountID, theNotaryID)};

    if (!theTransaction->LoadContractFromString(strTransaction)) {
        auto strAcctID = String::Factory(theAccountID);
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading transaction from string. "
            "Acct ID: ")(strAcctID)(".")
            .Flush();
        return {};
    }
    std::unique_ptr<OTTransaction> pTransaction = nullptr;

    if (theTransaction->IsAbbreviated()) {
        std::int64_t lBoxType = 0;

        if (theTransaction->Contains("nymboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::nymbox);
        else if (theTransaction->Contains("inboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::inbox);
        else if (theTransaction->Contains("outboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::outbox);
        else if (theTransaction->Contains("paymentInboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::paymentInbox);
        else if (theTransaction->Contains("recordBoxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::recordBox);
        else if (theTransaction->Contains("expiredBoxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::expiredBox);
        else {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                " Error loading from abbreviated "
                "transaction: Unknown ledger type.")
                .Flush();
            return {};
        }
        pTransaction = LoadBoxReceipt(*theTransaction, lBoxType);
        if (false == bool(pTransaction)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error loading from abbreviated "
                "transaction: Failed loading box receipt.")
                .Flush();
            return {};
        }
    } else
        pTransaction.reset(theTransaction.release());
    if (transactionType::pending != pTransaction->GetType()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Wrong transaction type: ")(
            pTransaction->GetTypeString())(". (Expected pending).")
            .Flush();
        return {};
    }
    auto strReference = String::Factory();
    pTransaction->GetReferenceString(strReference);

    if (!strReference->Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": No reference string found on transaction.")
            .Flush();
        return {};
    }
    auto pItem{api_.Factory().Item(
        strReference, theNotaryID, pTransaction->GetReferenceToNum())};

    if (false == bool(pItem)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed loading transaction item from string.")
            .Flush();
        return {};
    }

    // pItem will be automatically cleaned up when it goes out of scope.
    if ((itemType::transfer != pItem->GetType()) ||
        (Item::request != pItem->GetStatus())) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Wrong item type or status attached as "
            "reference on transaction.")
            .Flush();
        return {};
    }
    auto strOutput = String::Factory();

    pItem->GetNote(strOutput);
    // Didn't find one.
    if (!strOutput->Exists()) return {};

    // We found a note -- let's return it!
    //
    std::string pBuf = strOutput->Get();

    return pBuf;
}

std::int64_t OTAPI_Exec::Transaction_GetAmount(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION) const
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(ACCOUNT_ID);
    OT_VERIFY_STD_STR(THE_TRANSACTION);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theNymID = Identifier::Factory(NYM_ID),
               theAccountID = Identifier::Factory(ACCOUNT_ID);

    auto strTransaction = String::Factory(THE_TRANSACTION);

    if (!api_.Wallet().IsLocalNym(NYM_ID)) { return OT_ERROR_AMOUNT; }

    auto theTransaction{
        api_.Factory().Transaction(theNymID, theAccountID, theNotaryID)};

    if (!theTransaction->LoadContractFromString(strTransaction)) {
        auto strAcctID = String::Factory(theAccountID);
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading transaction from string. Acct ID: ")(strAcctID)(
            ".")
            .Flush();
        return -1;
    }

    std::unique_ptr<OTTransaction> pTransaction = nullptr;

    if (theTransaction->IsAbbreviated()) {
        std::int64_t lBoxType = 0;

        if (theTransaction->Contains("nymboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::nymbox);
        else if (theTransaction->Contains("inboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::inbox);
        else if (theTransaction->Contains("outboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::outbox);
        else if (theTransaction->Contains("paymentInboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::paymentInbox);
        else if (theTransaction->Contains("recordBoxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::recordBox);
        else if (theTransaction->Contains("expiredBoxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::expiredBox);
        else {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error loading from abbreviated "
                "transaction: unknown ledger type.")
                .Flush();
            return -1;
        }
        pTransaction = LoadBoxReceipt(*theTransaction, lBoxType);
        if (false == bool(pTransaction)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error loading from abbreviated "
                "transaction: Failed loading box receipt.")
                .Flush();
            return -1;
        }
    } else
        pTransaction.reset(theTransaction.release());

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
// Returns cheque #, or market offer #, or payment plan #, or pending
// transfer # (Meant to be used on inbox items.)
std::int64_t OTAPI_Exec::Transaction_GetDisplayReferenceToNum(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION) const
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(ACCOUNT_ID);
    OT_VERIFY_STD_STR(THE_TRANSACTION);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theNymID = Identifier::Factory(NYM_ID),
               theAccountID = Identifier::Factory(ACCOUNT_ID);

    auto strTransaction = String::Factory(THE_TRANSACTION);

    if (!api_.Wallet().IsLocalNym(NYM_ID)) { return -1; }

    auto theTransaction{
        api_.Factory().Transaction(theNymID, theAccountID, theNotaryID)};

    if (!theTransaction->LoadContractFromString(strTransaction)) {
        auto strAcctID = String::Factory(theAccountID);
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading transaction from string. Acct ID: ")(strAcctID)(
            ".")
            .Flush();
        return -1;
    }

    // NO need to load abbreviated version here, since it already stores
    // this number.
    //
    const std::int64_t lDisplayNum =
        theTransaction->GetReferenceNumForDisplay();
    return lDisplayNum;
}

//
// Get Transaction Type  (internally uses GetTransactionTypeString().)
std::string OTAPI_Exec::Transaction_GetType(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION) const
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(ACCOUNT_ID);
    OT_VERIFY_STD_STR(THE_TRANSACTION);

    const auto theNotaryIDv = Identifier::Factory(NOTARY_ID),
               theNymID = Identifier::Factory(NYM_ID),
               theAccountID = Identifier::Factory(ACCOUNT_ID);

    auto strTransaction = String::Factory(THE_TRANSACTION);

    if (!api_.Wallet().IsLocalNym(NYM_ID)) { return {}; }

    auto theTransaction{
        api_.Factory().Transaction(theNymID, theAccountID, theNotaryIDv)};

    if (!theTransaction->LoadContractFromString(strTransaction)) {
        auto strAcctID = String::Factory(theAccountID);
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading transaction from string. Acct ID: ")(strAcctID)(
            ".")
            .Flush();
        return {};
    }
    // NO need to load abbreviated version, since it already stores this
    // number.

    std::string pBuf = theTransaction->GetTypeString();

    return pBuf;
}

// Transactions do not have request numbers. However, if you have a
// replyNotice in your Nymbox, which is an OTTransaction object, it will
// CONTAIN a server reply to some previous message. This function will only
// work on a replyNotice, and it returns the actual request number of the
// server reply inside that notice. Used for calling
// OTAPI_Exec::HaveAlreadySeenReply() in order to see if we've already
// processed the reply for that message.
// Returns -1 on Error.
std::int64_t OTAPI_Exec::ReplyNotice_GetRequestNum(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& THE_TRANSACTION) const
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_STD_STR(THE_TRANSACTION);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theNymID = Identifier::Factory(NYM_ID),
               theAccountID =
                   Identifier::Factory(NYM_ID);  // account IS user, for
                                                 // Nymbox (the only box
                                                 // that carries
                                                 // replyNotices...)

    auto strTransaction = String::Factory(THE_TRANSACTION);

    if (!api_.Wallet().IsLocalNym(NYM_ID)) { return -1; }

    auto theTransaction{
        api_.Factory().Transaction(theNymID, theAccountID, theNotaryID)};

    if (!theTransaction->LoadContractFromString(strTransaction)) {
        auto strNymID = String::Factory(theNymID);
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading transaction from string. Nym ID: ")(strNymID)(".")
            .Flush();
        return -1;
    }

    if (transactionType::replyNotice != theTransaction->GetType()) {
        auto strNymID = String::Factory(theNymID);
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unexpected transaction type: ")(
            theTransaction->GetTypeString())(
            ". (Expected: replyNotice) User: ")(strNymID)(".")
            .Flush();
        return -1;
    }
    // NO need to load abbreviated version, since it already stores this
    // number.

    const std::int64_t lRequestNumber = theTransaction->GetRequestNum();

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
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(ACCOUNT_ID);
    OT_VERIFY_STD_STR(THE_TRANSACTION);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theNymID = Identifier::Factory(NYM_ID),
               theAccountID = Identifier::Factory(ACCOUNT_ID);

    auto strTransaction = String::Factory(THE_TRANSACTION);

    if (!api_.Wallet().IsLocalNym(NYM_ID)) {
        return OTTimeGetTimeFromSeconds(-1);
    }

    auto theTransaction{
        api_.Factory().Transaction(theNymID, theAccountID, theNotaryID)};

    if (!theTransaction->LoadContractFromString(strTransaction)) {
        auto strAcctID = String::Factory(theAccountID);
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading transaction from string. Acct ID: ")(strAcctID)(
            ".")
            .Flush();
        return OTTimeGetTimeFromSeconds(-1);
    }
    // NO need to load abbreviated version here, since it already stores the
    // date.

    auto strOutput = String::Factory();
    return theTransaction->GetDateSigned();
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
std::int32_t OTAPI_Exec::Transaction_GetSuccess(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION) const
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(ACCOUNT_ID);
    OT_VERIFY_STD_STR(THE_TRANSACTION);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theNymID = Identifier::Factory(NYM_ID),
               theAccountID = Identifier::Factory(ACCOUNT_ID);

    auto strTransaction = String::Factory(THE_TRANSACTION);
    if (!api_.Wallet().IsLocalNym(NYM_ID)) { return OT_ERROR; }

    // By this point, pNym is a good pointer, and is on the wallet. (No need
    // to cleanup.)
    auto theTransaction{
        api_.Factory().Transaction(theNymID, theAccountID, theNotaryID)};

    if (!theTransaction->LoadContractFromString(strTransaction)) {
        auto strAcctID = String::Factory(theAccountID);
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading transaction from string. Acct ID: ")(strAcctID)(
            ".")
            .Flush();
        return OT_ERROR;
    }
    std::unique_ptr<OTTransaction> pTransaction = nullptr;

    if (theTransaction->IsAbbreviated())  // Abbreviated.
    {
        std::int64_t lBoxType = 0;

        if (theTransaction->Contains("nymboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::nymbox);
        else if (theTransaction->Contains("inboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::inbox);
        else if (theTransaction->Contains("outboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::outbox);
        else if (theTransaction->Contains("paymentInboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::paymentInbox);
        else if (theTransaction->Contains("recordBoxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::recordBox);
        else if (theTransaction->Contains("expiredBoxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::expiredBox);
        else {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error loading from abbreviated "
                "transaction: Unknown ledger type.")
                .Flush();
            return OT_ERROR;
        }
        pTransaction = LoadBoxReceipt(*theTransaction, lBoxType);
        if (false == bool(pTransaction)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error loading from abbreviated "
                "transaction: Failed loading box receipt.")
                .Flush();
            return OT_ERROR;
        }
    } else  // NOT abbreviated.
        pTransaction.reset(theTransaction.release());

    if (pTransaction->GetSuccess()) {
        return OT_TRUE;
    } else {
        const std::int64_t lTransactionNum = pTransaction->GetTransactionNum();

        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": ** FYI, this transaction has a 'failure' status from the "
            "server. TransNum: ")(lTransactionNum)(".")
            .Flush();
    }

    return OT_FALSE;
}

std::int32_t OTAPI_Exec::Transaction_IsCanceled(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION) const
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(ACCOUNT_ID);
    OT_VERIFY_STD_STR(THE_TRANSACTION);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theNymID = Identifier::Factory(NYM_ID),
               theAccountID = Identifier::Factory(ACCOUNT_ID);

    auto strTransaction = String::Factory(THE_TRANSACTION);
    if (!api_.Wallet().IsLocalNym(NYM_ID)) { return OT_ERROR; }

    // By this point, pNym is a good pointer, and is on the wallet. (No need
    // to cleanup.)
    auto theTransaction{
        api_.Factory().Transaction(theNymID, theAccountID, theNotaryID)};

    if (!theTransaction->LoadContractFromString(strTransaction)) {
        auto strAcctID = String::Factory(theAccountID);
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading transaction from string. Acct ID: ")(strAcctID)(
            ".")
            .Flush();
        return OT_ERROR;
    }
    std::unique_ptr<OTTransaction> pTransaction = nullptr;

    // Note: This is an artifact from Transaction_GetSuccess, whose code was
    // copied to make
    // this function. In reality, a cancelled transaction will never be
    // abbreviated, since it
    // won't be used as any kind of box receipt in the first place. Rather,
    // a canceled transaction will only occur as a server reply to a
    // transaction request. For example, if you cancel a cheque, or cancel a
    // payment plan, and that cancellation is successful, then the server
    // will return a "reply transaction" to that request, which has
    // 'cancelled' set to true. So why am I leaving this code here for now?
    // We can trim it down later I suppose.
    //
    if (theTransaction->IsAbbreviated())  // Abbreviated.
    {
        std::int64_t lBoxType = 0;

        if (theTransaction->Contains("nymboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::nymbox);
        else if (theTransaction->Contains("inboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::inbox);
        else if (theTransaction->Contains("outboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::outbox);
        else if (theTransaction->Contains("paymentInboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::paymentInbox);
        else if (theTransaction->Contains("recordBoxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::recordBox);
        else if (theTransaction->Contains("expiredBoxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::expiredBox);
        else {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error loading from abbreviated "
                "transaction: Unknown ledger type.")
                .Flush();
            return OT_ERROR;
        }
        pTransaction = LoadBoxReceipt(*theTransaction, lBoxType);
        if (nullptr == pTransaction) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error loading from abbreviated "
                "transaction: Failed loading box receipt.")
                .Flush();
            return OT_ERROR;
        }
    } else  // NOT abbreviated.
        pTransaction.reset(theTransaction.release());
    if (pTransaction->IsCancelled()) { return OT_TRUE; }

    return OT_FALSE;
}

//
// Get Balance Agreement Success
// (from a TRANSACTION.)
//                              true  (1) == acknowledgment
//                              false (0) == rejection
// NEW: -1 (-1) for error
//
std::int32_t OTAPI_Exec::Transaction_GetBalanceAgreementSuccess(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION) const
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(ACCOUNT_ID);
    OT_VERIFY_STD_STR(THE_TRANSACTION);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theNymID = Identifier::Factory(NYM_ID),
               theAccountID = Identifier::Factory(ACCOUNT_ID);

    auto strTransaction = String::Factory(THE_TRANSACTION);

    if (!api_.Wallet().IsLocalNym(NYM_ID)) { return OT_ERROR; }

    // By this point, pNym is a good pointer, and is on the wallet. (No need
    // to cleanup.)
    auto theTransaction{
        api_.Factory().Transaction(theNymID, theAccountID, theNotaryID)};

    if (!theTransaction->LoadContractFromString(strTransaction)) {
        auto strAcctID = String::Factory(theAccountID);
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading transaction from string. Acct ID: ")(strAcctID)(
            ".")
            .Flush();
        return OT_ERROR;
    }
    std::unique_ptr<OTTransaction> pTransaction = nullptr;

    if (theTransaction->IsAbbreviated())  // IF TRANSACTION IS ABBREVIATED
                                          // (Ledger
                                          // may only contain stubs, not full
                                          // records...)
    {
        std::int64_t lBoxType = 0;

        if (theTransaction->Contains("nymboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::nymbox);
        else if (theTransaction->Contains("inboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::inbox);
        else if (theTransaction->Contains("outboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::outbox);
        else if (theTransaction->Contains("paymentInboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::paymentInbox);
        else if (theTransaction->Contains("recordBoxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::recordBox);
        else if (theTransaction->Contains("expiredBoxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::expiredBox);
        else {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error loading from abbreviated "
                "transaction: Unknown ledger type.")
                .Flush();
            return OT_ERROR;
        }
        pTransaction = LoadBoxReceipt(*theTransaction, lBoxType);
        if (nullptr == pTransaction) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error loading from abbreviated "
                "transaction: Failed loading box "
                "receipt.")
                .Flush();
            return OT_ERROR;
        }
    } else
        pTransaction.reset(theTransaction.release());
    // At this point, I actually have the transaction pointer, so let's
    // return its success status
    auto pReplyItem = pTransaction->GetItem(itemType::atBalanceStatement);

    if (false == bool(pReplyItem))
        pReplyItem = pTransaction->GetItem(itemType::atTransactionStatement);

    if (nullptr == pReplyItem) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Good transaction (could have been "
            "abbreviated though), but uncovered item "
            "pointer.")
            .Flush();
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
std::int32_t OTAPI_Exec::Message_GetBalanceAgreementSuccess(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_MESSAGE) const
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(ACCOUNT_ID);
    OT_VERIFY_STD_STR(THE_MESSAGE);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theNymID = Identifier::Factory(NYM_ID),
               theAccountID = Identifier::Factory(ACCOUNT_ID);
    auto strMessage = String::Factory(THE_MESSAGE);
    auto theMessage{api_.Factory().Message()};

    if (!strMessage->Exists() ||
        !theMessage->LoadContractFromString(strMessage)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to load message.").Flush();
        return OT_ERROR;
    }

    // It's not a transaction request or response, so the Payload wouldn't
    // contain a ledger. (Don't want to pass back whatever it DOES contain
    // in that case, now do I?)
    //
    if ((false ==
         theMessage->m_strCommand->Compare("notarizeTransactionResponse")) &&
        (false == theMessage->m_strCommand->Compare("processNymboxResponse")) &&
        (false == theMessage->m_strCommand->Compare("processInboxResponse"))) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Wrong message type: ")(
            theMessage->m_strCommand)(".")
            .Flush();
        return OT_ERROR;
    }

    // The ledger is stored in the Payload, we'll grab it into the String.
    auto strLedger = String::Factory(theMessage->m_ascPayload);

    if (!strLedger->Exists()) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": No ledger found on message.")
            .Flush();
        return OT_ERROR;
    }

    auto theLedger{api_.Factory().Ledger(theNymID, theAccountID, theNotaryID)};

    if (!theLedger->LoadLedgerFromString(strLedger)) {
        auto strAcctID = String::Factory(theAccountID);
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading ledger from string. Acct ID: ")(strAcctID)(".")
            .Flush();
        return OT_ERROR;
    }

    // At this point, I know theLedger loaded successfully.

    if (theLedger->GetTransactionCount() <= 0) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Bad count in message ledger: ")(
            theLedger->GetTransactionCount())(".")
            .Flush();
        return OT_ERROR;  // out of bounds. I'm saving from an
                          // OT_ASSERT_MSG() happening here. (Maybe I
                          // shouldn't.)
    }

    auto pReplyTransaction = theLedger->GetTransactionByIndex(
        0);  // Right now this is a defacto standard. (only 1 transaction
             // per message ledger, excepting process inbox. <== why? That's
             // one as well I thought. And has multiple items attached.)

    if (false == bool(pReplyTransaction)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Good index but uncovered pointer there: ")(0)(".")
            .Flush();
        return OT_ERROR;  // Weird.
    }

    // At this point, I actually have the transaction pointer, so let's
    // return its success status
    auto pReplyItem = pReplyTransaction->GetItem(itemType::atBalanceStatement);

    if (false == bool(pReplyItem))
        pReplyItem =
            pReplyTransaction->GetItem(itemType::atTransactionStatement);

    if (false == bool(pReplyItem)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Good index but uncovered item pointer: ")(0)(".")
            .Flush();
        return OT_ERROR;  // Weird.
    }

    if (pReplyItem->GetStatus() == Item::acknowledgement) { return OT_TRUE; }

    return OT_FALSE;
}

#if OT_CASH
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
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(INSTRUMENT_DEFINITION_ID);
    OT_VERIFY_STD_STR(THE_PURSE);

    std::string strFunc = std::string(OT_METHOD) + __FUNCTION__;
    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theInstrumentDefinitionID =
                   Identifier::Factory(INSTRUMENT_DEFINITION_ID),
               theNymID = Identifier::Factory(NYM_ID);
    const auto strPurse = String::Factory(THE_PURSE);
    bool bSuccess = false;
    auto thePurse{
        api_.Factory().Purse(theNotaryID, theInstrumentDefinitionID, theNymID)};

    if (strPurse->Exists() && thePurse->LoadContractFromString(strPurse)) {
        // NOTE: no need to verify the server / instrument definition id
        // here, since the call to SavePurse already does that.
        //
        //      if ((theNotaryID    != thePurse->GetNotaryID()) ||
        //          (theInstrumentDefinitionID !=
        //          thePurse->GetInstrumentDefinitionID()))
        //      {
        //          otOut << strFunc
        //                << ": Failure loading purse from string; server "
        //                   "or instrument definition id didn't match what
        //                   " "was expected:\n" << strPurse << "\n";
        //      }
        if (ot_api_.SavePurse(
                theNotaryID, theInstrumentDefinitionID, theNymID, *thePurse)) {
            bSuccess = true;
        } else {
            LogNormal(OT_METHOD)(__FUNCTION__)(": Failure saving purse: ")(
                strPurse)(".")
                .Flush();
        }
    } else {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failure loading purse from string: ")(strPurse)(".")
            .Flush();
    }

    return bSuccess;
}

// LOAD PURSE -- (from local storage)
//
// Based on Instrument Definition ID: load a purse, a public mint, or an
// asset contract and return it as a string -- or return "" if it wasn't
// found.
//
std::string OTAPI_Exec::LoadPurse(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& NYM_ID) const  // returns
                                      // "", or
                                      // a purse.
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(INSTRUMENT_DEFINITION_ID);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID);
    const auto theInstrumentDefinitionID =
        Identifier::Factory(INSTRUMENT_DEFINITION_ID);
    const auto theNymID = Identifier::Factory(NYM_ID);
    // There is an OT_ASSERT in here for memory failure,
    // but it still might return "" if various verification fails.

    std::unique_ptr<Purse> pPurse(
        ot_api_.LoadPurse(theNotaryID, theInstrumentDefinitionID, theNymID));

    if (nullptr == pPurse) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(
            "OTAPI_Exec::LoadPurse() received null when called ")(
            "OT_API::LoadPurse(). Server: ")(NOTARY_ID)(" Asset Type: ")(
            INSTRUMENT_DEFINITION_ID)
            .Flush();
    } else  // success
    {
        auto strOutput = String::Factory(*pPurse);  // For the output
        std::string pBuf = strOutput->Get();
        return pBuf;
    }

    return {};
}

// Get Purse Total Value  (internally uses GetTotalValue().)
//
// Returns the purported sum of all the tokens within.
//
std::int64_t OTAPI_Exec::Purse_GetTotalValue(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& THE_PURSE) const
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(INSTRUMENT_DEFINITION_ID);
    OT_VERIFY_STD_STR(THE_PURSE);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theInstrumentDefinitionID =
                   Identifier::Factory(INSTRUMENT_DEFINITION_ID);
    auto strPurse = String::Factory(THE_PURSE);
    auto thePurse{api_.Factory().Purse(theNotaryID, theInstrumentDefinitionID)};

    if (!thePurse->LoadContractFromString(strPurse)) {
        auto strInstrumentDefinitionID =
            String::Factory(theInstrumentDefinitionID);
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading purse from string. Instrument Definition "
            "ID: ")(strInstrumentDefinitionID)(".")
            .Flush();
        return OT_ERROR_AMOUNT;
    }

    std::int64_t lTotalValue = thePurse->GetTotalValue();
    return lTotalValue > 0 ? lTotalValue : 0;
}

// Returns a count of the tokens inside this purse. (Coins.)
// or -1 in case of error.
//
std::int32_t OTAPI_Exec::Purse_Count(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& THE_PURSE) const
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(INSTRUMENT_DEFINITION_ID);
    OT_VERIFY_STD_STR(THE_PURSE);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theInstrumentDefinitionID =
                   Identifier::Factory(INSTRUMENT_DEFINITION_ID);
    const auto strPurse = String::Factory(THE_PURSE);
    auto thePurse{api_.Factory().Purse(theNotaryID, theInstrumentDefinitionID)};

    if (strPurse->Exists() && thePurse->LoadContractFromString(strPurse) &&
        (thePurse->GetNotaryID() == theNotaryID) &&
        (thePurse->GetInstrumentDefinitionID() == theInstrumentDefinitionID)) {
        return thePurse->Count();
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
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_STD_STR(THE_PURSE);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID);
    const auto strPurse = String::Factory(THE_PURSE);
    auto thePurse{api_.Factory().Purse(theNotaryID)};

    if (strPurse->Exists() && thePurse->LoadContractFromString(strPurse) &&
        (thePurse->GetNotaryID() == theNotaryID)) {
        return thePurse->IsPasswordProtected();
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
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(INSTRUMENT_DEFINITION_ID);
    OT_VERIFY_ID_STR(OWNER_ID);
    OT_VERIFY_ID_STR(SIGNER_ID);

    std::string strFunc = std::string(OT_METHOD) + __FUNCTION__;

    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theInstrumentDefinitionID =
                   Identifier::Factory(INSTRUMENT_DEFINITION_ID),
               theOwnerID = Identifier::Factory(OWNER_ID),
               theSignerID = Identifier::Factory(SIGNER_ID);
    OTPasswordData thePWData(
        "Creating a cash purse. Enter wallet master password.");
    auto context = api_.Wallet().ServerContext(theOwnerID, theNotaryID);
    auto& signerNym = *context->Nym();

    std::unique_ptr<Purse> pPurse(ot_api_.CreatePurse(
        theNotaryID, theInstrumentDefinitionID, theOwnerID));

    if (pPurse) {
        pPurse->SignContract(signerNym, &thePWData);
        pPurse->SaveContract();
        const auto strOutput = String::Factory(*pPurse);
        std::string pBuf = strOutput->Get();

        return pBuf;
    }
    return {};
}

// When we create a password-protected purse, OWNER_ID has changed to
// SIGNER_ID, since we still need a Nym to sign it (in order to save it to
// string.) But this Nym will not be the owner. There is no owner, and the
// tokens are not encrypted to any Nym (but instead to a passphrase.)
//
// returns "", or a purse.
//
std::string OTAPI_Exec::CreatePurse_Passphrase(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& SIGNER_ID) const
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(INSTRUMENT_DEFINITION_ID);
    OT_VERIFY_ID_STR(SIGNER_ID);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theInstrumentDefinitionID =
                   Identifier::Factory(INSTRUMENT_DEFINITION_ID),
               theSignerID = Identifier::Factory(SIGNER_ID);
    OTPasswordData thePWData("Creating a password-protected cash purse.");
    auto context = api_.Wallet().ServerContext(theSignerID, theNotaryID);
    auto& nym = *context->Nym();

    std::unique_ptr<Purse> pPurse(
        ot_api_.CreatePurse_Passphrase(theNotaryID, theInstrumentDefinitionID));

    if (pPurse) {
        pPurse->SignContract(nym, &thePWData);
        pPurse->SaveContract();
        const auto strOutput = String::Factory(*pPurse);
        std::string pBuf = strOutput->Get();

        return pBuf;
    }
    return {};
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
                                  // password-protected. Otherwise MUST
                                  // contain the NymID for the Purse owner
                                  // (necessary to decrypt the token.)
    const std::string& THE_PURSE) const
{
    auto strOutput = String::Factory();  // for later.

    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(INSTRUMENT_DEFINITION_ID);
    OT_VERIFY_STD_STR(THE_PURSE);

    std::string strFunc =
        std::string(OT_METHOD) + __FUNCTION__;  //"OTAPI_Exec::Purse_Peek";
    // If false, then the purse MUST be password-protected.
    if (false == OWNER_ID.empty()) {
        if (!api_.Wallet().IsLocalNym(OWNER_ID)) { return {}; }
    }

    auto theOwnerID = Identifier::Factory(OWNER_ID);
    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theInstrumentDefinitionID =
                   Identifier::Factory(INSTRUMENT_DEFINITION_ID);
    const auto strPurse = String::Factory(THE_PURSE);
    std::unique_ptr<Token> pToken(ot_api_.Purse_Peek(
        theNotaryID,
        theInstrumentDefinitionID,
        strPurse,
        OWNER_ID.empty() ? Identifier::Factory()
                         : Identifier::Factory(theOwnerID.get())));
    if (pToken) {
        pToken->SaveContractRaw(strOutput);
        std::string pBuf = strOutput->Get();
        return pBuf;
    } else
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed peeking at a token on a cash purse.")
            .Flush();
    return {};
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
    auto strOutput = String::Factory();  // for later.

    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(INSTRUMENT_DEFINITION_ID);
    OT_VERIFY_ID_STR(OWNER_OR_SIGNER_ID);
    OT_VERIFY_STD_STR(THE_PURSE);

    std::string strFunc =
        std::string(OT_METHOD) + __FUNCTION__;  //"OTAPI_Exec::Purse_Pop";
    const auto strReason =
        String::Factory("Popping a token off of a cash purse.");
    OTPasswordData thePWData(strReason);
    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theInstrumentDefinitionID =
                   Identifier::Factory(INSTRUMENT_DEFINITION_ID),
               theNymID = Identifier::Factory(OWNER_OR_SIGNER_ID);
    const auto strPurse = String::Factory(THE_PURSE);
    auto context = api_.Wallet().ServerContext(theNymID, theNotaryID);

    auto& nym = *context->Nym();

    // By this point, pNym is a good pointer, and is on the wallet. (No need
    // to cleanup.)
    std::unique_ptr<Purse> pPurse(ot_api_.Purse_Pop(
        theNotaryID,
        theInstrumentDefinitionID,
        strPurse,
        theNymID.get(),  // Note: if the purse is password-protected, then
                         // this parameter is ignored.
        strReason));
    if (pPurse) {
        pPurse->ReleaseSignatures();
        pPurse->SignContract(nym, &thePWData);
        pPurse->SaveContract();

        pPurse->SaveContractRaw(strOutput);
        std::string pBuf = strOutput->Get();
        return pBuf;
    } else
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed popping a token off of a cash purse.")
            .Flush();

    return {};
}

// Makes an exact copy of a purse, except with no tokens inside. (Value of
// zero.)
// Useful when you need to create a temporary purse for moving tokens
// around, and you don't want some new symmetric/master key being generated
// for that purse as though it were really some new "other purse." For
// example, if you have a password-protected purse, you might want to pop
// all of
// the tokens off of it, and as you iterate, re-assign half of them to new
// ownership,
// push those onto a new purse owned by that new owner. But you only want to
// do this with half of the tokens... the other half of the tokens, you just
// want to push onto a temp purse (for the loop) that's owned by the
// original owner, so you can then save it back over the original in storage
// (since it contains "all the tokens that WEREN'T deposited" or "all the
// tokens that WEREN'T exported" etc.
//
// The point? If the "original owner" is a password-protected purse with a
// symmetric
// key, then you can't just generate some new "temp purse" without also
// generating a
// whole new KEY, which you DO NOT want to do. You also don't want to have
// to deal with re-assigning ownership back and forth between the two purses
// -- you just want to shove some tokens into one temporarily so you can
// finish your loop. You could take the original purse and make a copy of
// it, and then just pop all the tokens off of it one-by-one, but that is
// very cumbersome and expensive. But that'd be the only way to get a copy
// of the original purse with the SAME symmetric key, except empty, so you
// can use it as a temp purse. Therefore, to make that easier and solve that
// whole dilemma, I present: OTAPI_Exec::Purse_Empty.
// OTAPI_Exec::Purse_Empty takes a purse and returns an empty version of it
// (except if there's
// a symmetric/master key inside, those are preserved, so you can use it as
// a temp purse.)
//
// This function is effectively the same thing as calling Pop until the
// purse is empty. Returns: the empty purse, or "" if failure.
//
std::string OTAPI_Exec::Purse_Empty(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& SIGNER_ID,
    const std::string& THE_PURSE) const
{
    auto strOutput = String::Factory();  // for later.

    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(INSTRUMENT_DEFINITION_ID);
    OT_VERIFY_ID_STR(SIGNER_ID);
    OT_VERIFY_STD_STR(THE_PURSE);

    std::string strFunc =
        std::string(OT_METHOD) + __FUNCTION__;  //"OTAPI_Exec::Purse_Empty";
    const auto strReason =
        String::Factory("Creating an empty copy of a cash purse.");
    OTPasswordData thePWData(strReason);
    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theInstrumentDefinitionID =
                   Identifier::Factory(INSTRUMENT_DEFINITION_ID),
               theNymID = Identifier::Factory(SIGNER_ID);
    const auto strPurse = String::Factory(THE_PURSE);
    auto context = api_.Wallet().ServerContext(theNymID, theNotaryID);

    auto& nym = *context->Nym();

    // By this point, pNym is a good pointer, and is on the wallet. (No need
    // to cleanup.)
    std::unique_ptr<Purse> pPurse(ot_api_.Purse_Empty(
        theNotaryID, theInstrumentDefinitionID, strPurse, strReason));
    if (pPurse) {
        pPurse->ReleaseSignatures();
        pPurse->SignContract(nym, &thePWData);
        pPurse->SaveContract();

        pPurse->SaveContractRaw(strOutput);
        std::string pBuf = strOutput->Get();

        return pBuf;
    } else
        LogNormal(OT_METHOD)(__FUNCTION__)(": Failed emptying a cash purse.")
            .Flush();

    return {};
}

// Pushes a token onto the stack (of the purse.)
// Returns the updated purse (now including the token.)
//
// Returns "" if failure.
//
std::string OTAPI_Exec::Purse_Push(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& SIGNER_ID,  // The purse, in order to be changed,
                                   // must be re-signed, which requires a
                                   // private Nym. Even if the purse is
                                   // password-protected, then there's no
                                   // owner, but you still need to pass a
                                   // Nym in here to sign it (doesn't really
                                   // matter which one, but must have
                                   // private key for signing.)
    const std::string& OWNER_ID,   // If the purse is password-protected,
                                   // then there's no owner, and this owner
                                   // parameter should be "". However, if the
                                   // purse DOES have a Nym owner, then you
                                   // MUST pass the owner's Nym ID here, in
                                   // order for this action to be successful.
                                   // Furthermore, the public key for that
                                   // Nym must be available, in order to
                                   // encrypt the token being pushed into the
                                   // purse. (Private key NOT necessary
    // for owner, in this case.) If this fails due
    // to public key not being available, just
    // download it from the server and try again.
    const std::string& THE_PURSE,
    const std::string& THE_TOKEN) const
{
    auto strOutput = String::Factory();  // for later.

    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(INSTRUMENT_DEFINITION_ID);
    OT_VERIFY_ID_STR(SIGNER_ID);
    //  OT_VERIFY_ID_STR(OWNER_ID);
    OT_VERIFY_STD_STR(THE_PURSE);
    OT_VERIFY_STD_STR(THE_TOKEN);

    std::string strFunc =
        std::string(OT_METHOD) + __FUNCTION__;  //"OTAPI_Exec::Purse_Push";
    const auto strReason =
        String::Factory("Pushing a token onto a cash purse.");
    OTPasswordData thePWData(strReason);
    auto theOwnerID = Identifier::Factory(OWNER_ID);

    // If false, then the purse MUST be password-protected.
    if (false == OWNER_ID.empty()) {
        const auto exists = bool(api_.Wallet().Nym(theOwnerID));

        if (false == exists) { return {}; }
    }

    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theInstrumentDefinitionID =
                   Identifier::Factory(INSTRUMENT_DEFINITION_ID);
    const auto strPurse = String::Factory(THE_PURSE),
               strToken = String::Factory(THE_TOKEN);
    std::unique_ptr<Purse> pPurse(ot_api_.Purse_Push(
        theNotaryID,
        theInstrumentDefinitionID,
        strPurse,
        strToken,
        OWNER_ID.empty() ? Identifier::Factory()
                         : Identifier::Factory(theOwnerID.get())));  // Note: if
                                                                     // the
                                                                     // purse is
    // password-protected, then this
    // parameter should be "".

    if (pPurse) {
        const auto theSignerID = Identifier::Factory(SIGNER_ID);
        auto context = api_.Wallet().ServerContext(
            theSignerID, Identifier::Factory(NOTARY_ID));

        auto& signerNym = *context->Nym();

        pPurse->ReleaseSignatures();
        pPurse->SignContract(signerNym, &thePWData);
        pPurse->SaveContract();

        pPurse->SaveContractRaw(strOutput);
        std::string pBuf = strOutput->Get();

        return pBuf;
    } else
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed pushing a token onto a cash purse.")
            .Flush();

    return {};
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
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(INSTRUMENT_DEFINITION_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_STD_STR(THE_PURSE);

    auto strReason = String::Factory("Importing a cash purse into the wallet.");
    //  OTPasswordData thePWData(strReason);
    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theInstrumentDefinitionID =
                   Identifier::Factory(INSTRUMENT_DEFINITION_ID),
               theNymID = Identifier::Factory(NYM_ID);
    const auto strNewPurse = String::Factory(THE_PURSE);
    // THE_PURSE (the new purse) either is for a Nym, or a Symmetric Key.
    // If it's for a Nym, it either has a NymID, or the ID is left blank.
    //
    // This call already logs on failure, so I won't bother logging again
    // here.
    //
    auto strDisplay = String::Factory();

    return ot_api_.Wallet_ImportPurse(
        theNotaryID,
        theInstrumentDefinitionID,
        theNymID,
        strNewPurse,
        strDisplay);
}

// the GUI needs to be able to export cash from one Nym to another,
// so a function has been provided for doing so to the actual Token IDs (not
// just the purse.
// Understand that even when you decrypt a token out of a purse, the token
// ID itself is still encrypted within that token. That is what this
// function is for.
//
// returns: the updated token itself in string form.
//

//
// ALLOW the caller to pass a symmetric key here, instead of either Nym ID.
// We'll load it up and use it instead of a Nym. Update: make that a purse.
// These tokens already bestd::int64_t to specific purses, so just pass the
// purse
// here
//
// Done: Also, add a key cache with a timeout (similar to Master Key) where
// we can stash any already-loaded symmetric keys, and a thread wipes them
// out later. That way even if we have to load the key each time this func
// is called, we still avoid the user having to enter the passphrase more
// than once per timeout period.
//
// Done also: allow a "Signer ID" to be passed in here, since either Nym
// could potentially now be a symmetric key.

std::string OTAPI_Exec::Token_ChangeOwner(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& THE_TOKEN,
    const std::string& SIGNER_NYM_ID,
    const std::string& OLD_OWNER,        // Pass a NymID here, or a purse.
    const std::string& NEW_OWNER) const  // Pass a NymID here, or a purse.
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(INSTRUMENT_DEFINITION_ID);
    OT_VERIFY_STD_STR(THE_TOKEN);
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);
    const bool bOldOwnerIsPurse = String::Factory(OLD_OWNER)->Contains("PURSE");
    const bool bNewOwnerIsPurse = String::Factory(NEW_OWNER)->Contains("PURSE");
    if (!bOldOwnerIsPurse) {
        OT_VERIFY_ID_STR(OLD_OWNER);
    } else {
        OT_VERIFY_STD_STR(OLD_OWNER);
    }
    if (!bNewOwnerIsPurse) {
        OT_VERIFY_ID_STR(NEW_OWNER);
    } else {
        OT_VERIFY_STD_STR(NEW_OWNER);
    }
    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theInstrumentDefinitionID =
                   Identifier::Factory(INSTRUMENT_DEFINITION_ID),
               theSignerNymID = Identifier::Factory(SIGNER_NYM_ID);
    const auto strOldOwner =
                   String::Factory(OLD_OWNER),  // Either of these MIGHT contain
                                                // a Nym ID, OR might contain a
                                                // purse...
        strNewOwner =
            String::Factory(NEW_OWNER);  // (purse is passed in cases where the
    // token is encrypted with a passphrase
    // aka symmetric crypto, versus being
    // encrypted to a Nym's public key.)
    auto strToken = String::Factory(THE_TOKEN);
    std::unique_ptr<Token> pToken(ot_api_.Token_ChangeOwner(
        theNotaryID,
        theInstrumentDefinitionID,
        strToken,
        theSignerNymID,
        strOldOwner,    // Pass a NymID here as a string, or a purse. (IF
                        // symmetrically encrypted, the relevant key is in the
                        // purse.)
        strNewOwner));  // Pass a NymID here as a string, or a purse. (IF
                        // symmetrically encrypted, the relevant key is in
                        // the purse.)
    if (nullptr != pToken)  // Success!
    {
        const auto strOutput = String::Factory(*pToken);
        std::string pBuf = strOutput->Get();
        return pBuf;
    }
    return {};
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
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(INSTRUMENT_DEFINITION_ID);
    OT_VERIFY_STD_STR(THE_TOKEN);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theInstrumentDefinitionID =
                   Identifier::Factory(INSTRUMENT_DEFINITION_ID);

    auto strOutput = String::Factory("0");

    auto strToken = String::Factory(THE_TOKEN);
    auto pToken{
        api_.Factory().Token(strToken, theNotaryID, theInstrumentDefinitionID)};

    if (false != bool(pToken))  // TokenFactory instantiates AND loads from
                                // string.
    {
        const Armored& ascSpendable =
            pToken->GetSpendable();  // encrypted version of Token ID, used
                                     // as an "ID" on client side.

        strOutput->Format("%s", ascSpendable.Get());
    }

    std::string pBuf = strOutput->Get();

    return pBuf;
}

// The actual cash value of the token. Returns -1 on error.
//
std::int64_t OTAPI_Exec::Token_GetDenomination(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& THE_TOKEN) const
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(INSTRUMENT_DEFINITION_ID);
    OT_VERIFY_STD_STR(THE_TOKEN);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theInstrumentDefinitionID =
                   Identifier::Factory(INSTRUMENT_DEFINITION_ID);

    auto strOutput = String::Factory("0");

    auto strToken = String::Factory(THE_TOKEN);
    auto pToken{
        api_.Factory().Token(strToken, theNotaryID, theInstrumentDefinitionID)};

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
std::int32_t OTAPI_Exec::Token_GetSeries(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& THE_TOKEN) const
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(INSTRUMENT_DEFINITION_ID);
    OT_VERIFY_STD_STR(THE_TOKEN);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theInstrumentDefinitionID =
                   Identifier::Factory(INSTRUMENT_DEFINITION_ID);

    auto strOutput = String::Factory();

    auto strToken = String::Factory(THE_TOKEN);
    auto pToken{
        api_.Factory().Token(strToken, theNotaryID, theInstrumentDefinitionID)};

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
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(INSTRUMENT_DEFINITION_ID);
    OT_VERIFY_STD_STR(THE_TOKEN);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theInstrumentDefinitionID =
                   Identifier::Factory(INSTRUMENT_DEFINITION_ID);

    auto strOutput = String::Factory();

    auto strToken = String::Factory(THE_TOKEN);
    auto pToken{
        api_.Factory().Token(strToken, theNotaryID, theInstrumentDefinitionID)};

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
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(INSTRUMENT_DEFINITION_ID);
    OT_VERIFY_STD_STR(THE_TOKEN);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theInstrumentDefinitionID =
                   Identifier::Factory(INSTRUMENT_DEFINITION_ID);

    auto strOutput = String::Factory();

    auto strToken = String::Factory(THE_TOKEN);
    auto pToken{
        api_.Factory().Token(strToken, theNotaryID, theInstrumentDefinitionID)};

    if (pToken)  // TokenFactory instantiates AND loads from string.
    {
        return pToken->GetValidTo();
    }
    return OTTimeGetTimeFromSeconds(-1);
}

std::string OTAPI_Exec::Token_GetInstrumentDefinitionID(
    const std::string& THE_TOKEN) const
{
    OT_VERIFY_STD_STR(THE_TOKEN);

    auto strOutput = String::Factory();

    auto strToken = String::Factory(THE_TOKEN);
    auto pToken{api_.Factory().Token(strToken)};

    if (false != bool(pToken))  // TokenFactory instantiates AND loads from
                                // string.
    {
        const Identifier& theID = pToken->GetInstrumentDefinitionID();
        theID.GetString(strOutput);
    }

    std::string pBuf = strOutput->Get();
    return pBuf;
}

std::string OTAPI_Exec::Token_GetNotaryID(const std::string& THE_TOKEN) const
{
    OT_VERIFY_STD_STR(THE_TOKEN);

    auto strOutput = String::Factory();

    auto strToken = String::Factory(THE_TOKEN);
    auto pToken{api_.Factory().Token(strToken)};

    if (false != bool(pToken))  // TokenFactory instantiates AND loads from
                                // string.
    {
        const Identifier& theID = pToken->GetNotaryID();
        theID.GetString(strOutput);
    }

    std::string pBuf = strOutput->Get();
    return pBuf;
}
#endif  // OT_CASH

// IS BASKET CURRENCY ?
//
// Tells you whether or not a given instrument definition is actually a
// basket currency.
//
// returns bool (true or false aka 1 or 0.)
//
bool OTAPI_Exec::IsBasketCurrency(
    const std::string& INSTRUMENT_DEFINITION_ID) const
{
    OT_VERIFY_ID_STR(INSTRUMENT_DEFINITION_ID);

    const auto theInstrumentDefinitionID =
        Identifier::Factory(INSTRUMENT_DEFINITION_ID);

    if (ot_api_.IsBasketCurrency(theInstrumentDefinitionID))
        return true;
    else
        return false;
}

// Get Basket Count (of backing instrument definitions.)
//
// Returns the number of instrument definitions that make up this basket.
// (Or zero.)
//
std::int32_t OTAPI_Exec::Basket_GetMemberCount(
    const std::string& INSTRUMENT_DEFINITION_ID) const
{
    OT_VERIFY_ID_STR(INSTRUMENT_DEFINITION_ID);

    const auto theInstrumentDefinitionID =
        Identifier::Factory(INSTRUMENT_DEFINITION_ID);

    return ot_api_.GetBasketMemberCount(theInstrumentDefinitionID);
}

// Get Asset Type of a basket's member currency, by index.
//
// (Returns a string containing Instrument Definition ID, or "").
//
std::string OTAPI_Exec::Basket_GetMemberType(
    const std::string& BASKET_INSTRUMENT_DEFINITION_ID,
    const std::int32_t& nIndex) const
{
    OT_VERIFY_ID_STR(BASKET_INSTRUMENT_DEFINITION_ID);
    OT_VERIFY_MIN_BOUND(nIndex, 0);

    const auto theInstrumentDefinitionID =
        Identifier::Factory(BASKET_INSTRUMENT_DEFINITION_ID);

    auto theOutputMemberType = Identifier::Factory();

    bool bGotType = ot_api_.GetBasketMemberType(
        theInstrumentDefinitionID, nIndex, theOutputMemberType);
    if (!bGotType) return {};

    return theOutputMemberType->str();
}

// GET BASKET MINIMUM TRANSFER AMOUNT
//
// Returns a std::int64_t containing the minimum transfer
// amount for the entire basket.
//
// Returns OT_ERROR_AMOUNT on error.
//
// FOR EXAMPLE:
// If the basket is defined as 10 Rands == 2 Silver, 5 Gold, 8 Euro,
// then the minimum transfer amount for the basket is 10. This function
// would return a string containing "10", in that example.
//
std::int64_t OTAPI_Exec::Basket_GetMinimumTransferAmount(
    const std::string& BASKET_INSTRUMENT_DEFINITION_ID) const
{
    OT_VERIFY_ID_STR(BASKET_INSTRUMENT_DEFINITION_ID);

    const auto theInstrumentDefinitionID =
        Identifier::Factory(BASKET_INSTRUMENT_DEFINITION_ID);

    std::int64_t lMinTransAmount =
        ot_api_.GetBasketMinimumTransferAmount(theInstrumentDefinitionID);

    if (0 >= lMinTransAmount) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Returned 0 (or negative). Strange! What basket is "
            "this?")
            .Flush();
        return OT_ERROR_AMOUNT;
    }

    return lMinTransAmount;
}

// GET BASKET MEMBER's MINIMUM TRANSFER AMOUNT
//
// Returns a std::int64_t containing the minimum transfer
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
std::int64_t OTAPI_Exec::Basket_GetMemberMinimumTransferAmount(
    const std::string& BASKET_INSTRUMENT_DEFINITION_ID,
    const std::int32_t& nIndex) const
{
    OT_VERIFY_ID_STR(BASKET_INSTRUMENT_DEFINITION_ID);
    OT_VERIFY_MIN_BOUND(nIndex, 0);

    const auto theInstrumentDefinitionID =
        Identifier::Factory(BASKET_INSTRUMENT_DEFINITION_ID);

    std::int64_t lMinTransAmount = ot_api_.GetBasketMemberMinimumTransferAmount(
        theInstrumentDefinitionID, nIndex);

    if (0 >= lMinTransAmount) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Returned 0 (or negative). Strange! What basket is "
            "this?")
            .Flush();
        return OT_ERROR_AMOUNT;
    }

    return lMinTransAmount;
}

// If THE_MESSAGE is of command type usageCreditsResponse, and IF it was a
// SUCCESS,
// then this function returns the usage credits BALANCE (it's a std::int64_t
// std::int32_t,
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
//           setting for usage credits, which enables him to operate as if
//           he has unlimited usage credits.
//
std::int64_t OTAPI_Exec::Message_GetUsageCredits(
    const std::string& THE_MESSAGE) const
{
    if (THE_MESSAGE.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: THE_MESSAGE passed in!")
            .Flush();
        return -2;
    }

    auto strMessage = String::Factory(THE_MESSAGE);
    auto theMessage{api_.Factory().Message()};

    if (!strMessage->Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error: THE_MESSAGE doesn't exist.")
            .Flush();
        return -2;
    }

    if (!theMessage->LoadContractFromString(strMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed loading message from string.")
            .Flush();
        return -2;
    }

    if (!theMessage->m_bSuccess) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Message success == false, thus unable to "
            "report Usage Credits balance. (Returning "
            "-2).")
            .Flush();
        return -2;
    }

    if (!theMessage->m_strCommand->Compare("usageCreditsResponse")) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": THE_MESSAGE is supposed to be of command "
            "type usageCreditsResponse, but instead it's a: ")(
            theMessage->m_strCommand)(". (Failure! Returning -2).")
            .Flush();
        return -2;
    }

    // By this point, we know the message was a successful
    // usageCreditsResponse, loaded properly from the string that was passed
    // in. Let's return the usage credits balance (a std::int64_t
    // std::int32_t, returned in string format.)

    return theMessage->m_lDepth;
}

std::int32_t OTAPI_Exec::completePeerReply(
    const std::string& nymID,
    const std::string& replyID) const
{
    const auto nym = Identifier::Factory(nymID);
    const auto reply = Identifier::Factory(replyID);

    return api_.Wallet().PeerReplyComplete(nym, reply);
}

std::int32_t OTAPI_Exec::completePeerRequest(
    const std::string& nymID,
    const std::string& requestID) const
{
    const auto nym = Identifier::Factory(nymID);
    const auto request = Identifier::Factory(requestID);

    return api_.Wallet().PeerRequestComplete(nym, request);
}

std::list<std::string> OTAPI_Exec::getSentRequests(
    const std::string& nymID) const
{
    const auto nym = Identifier::Factory(nymID);
    const auto requests = api_.Wallet().PeerRequestSent(nym);
    std::list<std::string> output;

    for (auto& item : requests) { output.push_back(item.first); }

    return output;
}

std::list<std::string> OTAPI_Exec::getIncomingRequests(
    const std::string& nymID) const
{
    const auto nym = Identifier::Factory(nymID);
    const auto requests = api_.Wallet().PeerRequestIncoming(nym);
    std::list<std::string> output;

    for (auto& item : requests) { output.push_back(item.first); }

    return output;
}

std::list<std::string> OTAPI_Exec::getFinishedRequests(
    const std::string& nymID) const
{
    const auto nym = Identifier::Factory(nymID);
    const auto requests = api_.Wallet().PeerRequestFinished(nym);
    std::list<std::string> output;

    for (auto& item : requests) { output.push_back(item.first); }

    return output;
}

std::list<std::string> OTAPI_Exec::getProcessedRequests(
    const std::string& nymID) const
{
    const auto nym = Identifier::Factory(nymID);
    const auto requests = api_.Wallet().PeerRequestProcessed(nym);
    std::list<std::string> output;

    for (auto& item : requests) { output.push_back(item.first); }

    return output;
}

std::list<std::string> OTAPI_Exec::getSentReplies(
    const std::string& nymID) const
{
    const auto nym = Identifier::Factory(nymID);
    const auto requests = api_.Wallet().PeerReplySent(nym);
    std::list<std::string> output;

    for (auto& item : requests) { output.push_back(item.first); }

    return output;
}

std::list<std::string> OTAPI_Exec::getIncomingReplies(
    const std::string& nymID) const
{
    const auto nym = Identifier::Factory(nymID);
    const auto requests = api_.Wallet().PeerReplyIncoming(nym);
    std::list<std::string> output;

    for (auto& item : requests) { output.push_back(item.first); }

    return output;
}

std::list<std::string> OTAPI_Exec::getFinishedReplies(
    const std::string& nymID) const
{
    const auto nym = Identifier::Factory(nymID);
    const auto requests = api_.Wallet().PeerReplyFinished(nym);
    std::list<std::string> output;

    for (auto& item : requests) { output.push_back(item.first); }

    return output;
}

std::list<std::string> OTAPI_Exec::getProcessedReplies(
    const std::string& nymID) const
{
    const auto nym = Identifier::Factory(nymID);
    const auto requests = api_.Wallet().PeerReplyProcessed(nym);
    std::list<std::string> output;

    for (auto& item : requests) { output.push_back(item.first); }

    return output;
}

std::string OTAPI_Exec::getRequest(
    const std::string& nymID,
    const std::string& requestID,
    const StorageBox box) const
{
    std::time_t notUsed = 0;

    auto request = api_.Wallet().PeerRequest(
        Identifier::Factory(nymID),
        Identifier::Factory(requestID),
        box,
        notUsed);

    if (request) { return proto::ProtoAsString(*request); }

    return {};
}

/// Base64-encodes the result. Otherwise identical to getRequest.
std::string OTAPI_Exec::getRequest_Base64(
    const std::string& nymID,
    const std::string& requestID) const
{
    auto output = getRequest(nymID, requestID, StorageBox::SENTPEERREQUEST);

    if (output.empty()) {
        output = getRequest(nymID, requestID, StorageBox::INCOMINGPEERREQUEST);

        if (output.empty()) {
            output =
                getRequest(nymID, requestID, StorageBox::FINISHEDPEERREQUEST);

            if (output.empty()) {
                output = getRequest(
                    nymID, requestID, StorageBox::PROCESSEDPEERREQUEST);

                if (output.empty()) { return {}; }
            }
        }
    }

    return api_.Crypto().Encode().DataEncode(output);
}

std::string OTAPI_Exec::getReply(
    const std::string& nymID,
    const std::string& replyID,
    const StorageBox box) const
{
    auto reply = api_.Wallet().PeerReply(
        Identifier::Factory(nymID), Identifier::Factory(replyID), box);

    if (reply) { return proto::ProtoAsString(*reply); }

    return {};
}

/// Base64-encodes the result. Otherwise identical to getReply.
std::string OTAPI_Exec::getReply_Base64(
    const std::string& nymID,
    const std::string& replyID) const
{
    auto output = getReply(nymID, replyID, StorageBox::SENTPEERREPLY);

    if (output.empty()) {
        output = getReply(nymID, replyID, StorageBox::INCOMINGPEERREPLY);

        if (output.empty()) {
            output = getReply(nymID, replyID, StorageBox::FINISHEDPEERREPLY);

            if (output.empty()) {
                output =
                    getReply(nymID, replyID, StorageBox::PROCESSEDPEERREPLY);

                if (output.empty()) { return {}; }
            }
        }
    }

    return api_.Crypto().Encode().DataEncode(output);
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
    const std::uint64_t weight) const
{
    auto serverContract = api_.Wallet().Server(Identifier::Factory(serverID));

    if (!serverContract) { return {}; }

    auto basketTemplate = UnitDefinition::Create(
        api_.Wallet(),
        serverContract->Nym(),
        shortname,
        name,
        symbol,
        terms,
        weight);

    std::string str_return = (proto::ProtoAsArmored<proto::UnitDefinition>(
                                  basketTemplate->PublicContract(),
                                  String::Factory("BASKET CONTRACT")))
                                 ->Get();
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
    const std::uint64_t& weight) const
{
    OT_ASSERT_MSG(
        !basketTemplate.empty(),
        "OTAPI_Exec::AddBasketCreationItem: Null basketTemplate passed "
        "in.");
    OT_ASSERT_MSG(
        !currencyID.empty(),
        "OTAPI_Exec::AddBasketCreationItem: Null currencyID passed in.");

    bool bAdded = false;
    auto contract = proto::StringToProto<proto::UnitDefinition>(
        String::Factory(basketTemplate));

    bAdded = ot_api_.AddBasketCreationItem(
        contract, String::Factory(currencyID), weight);

    if (!bAdded) { return {}; }

    return proto::ProtoAsArmored(contract, String::Factory("BASKET CONTRACT"))
        ->Get();
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
    const std::int32_t& TRANSFER_MULTIPLE) const
// 1            2            3
// 5=2,3,4  OR  10=4,6,8  OR 15=6,9,12
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(BASKET_INSTRUMENT_DEFINITION_ID);
    OT_VERIFY_ID_STR(BASKET_ASSET_ACCT_ID);

    const auto theNymID = Identifier::Factory(NYM_ID),
               theNotaryID = Identifier::Factory(NOTARY_ID),
               theBasketInstrumentDefinitionID =
                   Identifier::Factory(BASKET_INSTRUMENT_DEFINITION_ID),
               theBasketAssetAcctID = Identifier::Factory(BASKET_ASSET_ACCT_ID);
    std::int32_t nTransferMultiple = 1;  // Just a default value.

    if (TRANSFER_MULTIPLE > 0) nTransferMultiple = TRANSFER_MULTIPLE;
    std::unique_ptr<Basket> pBasket(ot_api_.GenerateBasketExchange(
        theNotaryID,
        theNymID,
        theBasketInstrumentDefinitionID,
        theBasketAssetAcctID,
        nTransferMultiple));
    // 1            2            3
    // 5=2,3,4  OR  10=4,6,8  OR 15=6,9,12

    if (nullptr == pBasket) return {};

    // At this point, I know pBasket is good (and will be cleaned up
    // automatically.)
    auto strOutput =
        String::Factory(*pBasket);  // Extract the basket to string form.
    std::string pBuf = strOutput->Get();
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
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_STD_STR(THE_BASKET);
    OT_VERIFY_ID_STR(INSTRUMENT_DEFINITION_ID);
    OT_VERIFY_ID_STR(ASSET_ACCT_ID);

    auto strBasket = String::Factory(THE_BASKET);
    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theNymID = Identifier::Factory(NYM_ID),
               theInstrumentDefinitionID =
                   Identifier::Factory(INSTRUMENT_DEFINITION_ID),
               theAssetAcctID = Identifier::Factory(ASSET_ACCT_ID);
    auto theBasket{api_.Factory().Basket()};

    OT_ASSERT(false != bool(theBasket));

    bool bAdded = false;

    // todo perhaps verify the basket here, even though I already verified
    // the asset contract itself... Can't never be too sure.
    if (theBasket->LoadContractFromString(strBasket)) {
        bAdded = ot_api_.AddBasketExchangeItem(
            theNotaryID,
            theNymID,
            *theBasket,
            theInstrumentDefinitionID,
            theAssetAcctID);
    }

    if (!bAdded) return {};

    auto strOutput = String::Factory(*theBasket);  // Extract the updated basket
                                                   // to string form.
    std::string pBuf = strOutput->Get();
    return pBuf;
}

// Message OUT-BUFFER
//
// (for messages I--the client--have sent the server.)
/*
class OTMessageOutbuffer:
void        Clear();
void        AddSentMessage      (OTMessage& theMessage);   // Allocate
theMsg on the heap (takes ownership.) Mapped by request num. OTMessage *
GetSentMessage      (const std::int64_t& lRequestNum); // null == not found.
caller NOT responsible to delete. bool        RemoveSentMessage   (const
std::int64_t& lRequestNum); // true == it was removed. false == it wasn't
found.
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
    const std::int64_t& REQUEST_NUMBER,
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_MIN_BOUND(REQUEST_NUMBER, 0);

    const std::int64_t lRequestNum = REQUEST_NUMBER;
    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theNymID = Identifier::Factory(NYM_ID);
    Message* pMsg = ot_api_.GetSentMessage(
        static_cast<std::int64_t>(lRequestNum), theNotaryID, theNymID);

    if (nullptr == pMsg)  // The message wasn't found with that request number.
    {
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": Message not found with request number ")(lRequestNum)(", sorry.")
            .Flush();
        return {};
    }
    const auto strOutput =
        String::Factory(*pMsg);  // No need to cleanup the message since
                                 // it's still in the buffer until
                                 // explicitly removed.

    std::string pBuf = strOutput->Get();
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
    const std::int64_t& REQUEST_NUMBER,
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_MIN_BOUND(REQUEST_NUMBER, 0);

    const std::int64_t lRequestNum = REQUEST_NUMBER;
    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theNymID = Identifier::Factory(NYM_ID);
    const bool bSuccess = ot_api_.RemoveSentMessage(
        static_cast<std::int64_t>(lRequestNum), theNotaryID, theNymID);

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
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_STD_STR(THE_NYMBOX);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theNymID = Identifier::Factory(NYM_ID);
    const auto strLedger = String::Factory(THE_NYMBOX);
    auto theLedger{api_.Factory().Ledger(theNymID, theNymID, theNotaryID)};

    OT_ASSERT(false != bool(theLedger));

    if (strLedger->Exists() && theLedger->LoadContractFromString(strLedger))
        ot_api_.FlushSentMessages(
            bHarvestingForRetry, theNotaryID, theNymID, *theLedger);
    else
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failure: Unable to load Nymbox from string: ")(strLedger)(".")
            .Flush();
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
    /*rLock lock(lock_callback_({NYM_ID, NOTARY_ID}));

    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_STD_STR(THE_MESSAGE);

    auto theNotaryID = Identifier::Factory(NOTARY_ID),
         theNymID = Identifier::Factory(NYM_ID);
    const String strMessage(THE_MESSAGE), strNymID(theNymID);

    auto context = api_.Wallet().mutable_ServerContext(theNymID,
    theNotaryID);

    auto nymfile = context.It().mutable_Nymfile(legacy_.ClientDataFolder(),
    __FUNCTION__);

    ConstNym pNym = context.It().Nym();
    if (false == bool(pNym)) { return false; }

    Message theMessage{legacy_.ClientDataFolder()};

    if (!theMessage.LoadContractFromString(strMessage)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed trying to load registerNymResponse() "
                 "message from string (it's a server reply.) "
                 "Contents:\n\n"
              << strMessage << "\n\n";
        return false;
    }
    if (!strNymID.Compare(theMessage.m_strNymID)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed. Though success loading message from string, it
    had " "the wrong NymID. (Expected "
              << strNymID << ", but found " << theMessage.m_strNymID
              << ".) Message contents:\n\n"
              << strMessage << "\n\n";
        return false;
    }
    if (!theMessage.m_strCommand.Compare("registerNymResponse")) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed. Though success loading message "
                 "from string, it had the wrong command type. "
                 "(Expected registerNymResponse, but found "
              << theMessage.m_strCommand << ".) Message contents:\n\n"
              << strMessage << "\n\n";
        return false;
    }
    if (!theMessage.m_ascPayload.Exists()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed. Though success loading registerNymResponse() "
                 "message, the payload was empty. (Expected theMessageNym to
    " "be there, so I could re-sync client side to server.) Message "
                 "contents:\n\n"
              << strMessage << "\n\n";
        return false;
    }
    String strMessageNym;

    if (!theMessage.m_ascPayload.GetString(strMessageNym)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed decoding message payload in server "
                 "reply: registerNymResponse(). (Expected "
                 "theMessageNym to be there, so I could "
                 "re-sync client side to server.) Message "
                 "contents:\n\n"
              << strMessage << "\n\n";
        return false;
    }

    bool unused;
    std::unique_ptr<class Nym> theMessageNym =
        Nym::DeserializeNymfile(legacy_.ClientDataFolder(), strMessageNym,
    unused); if (nullptr == theMessageNym) { otErr << OT_METHOD <<
    __FUNCTION__
              << ": Failed loading theMessageNym from a "
                 "string. String contents:\n\n"
              << strMessageNym << "\n\n";
        return false;
    }

    // Based on notaryID and NymID, load the Nymbox.
    //
    Ledger theNymbox(theNymID, theNymID, theNotaryID);

    bool bSynced = false;
    bool bLoadedNymbox =
        (theNymbox.LoadNymbox() && theNymbox.VerifyAccount(*pNym));

    if (bLoadedNymbox)
        bSynced = ot_api_.ResyncNymWithServer(
            nymfile.It(), theNymbox, *theMessageNym);
    else
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed while loading or verifying Nymbox for User "
              << strNymID << ", on Server " << NOTARY_ID << " \n";

    return bSynced;
*/
    return false;
}

// GET MESSAGE PAYLOAD
//
// This way you can retrieve the payload from any message.
// Useful, for example, for getting the encoded StringMap object
// from the queryInstrumentDefinitions and
// queryInstrumentDefinitionsResponse messages, which both use the
// m_ascPayload field to transport it.
//
std::string OTAPI_Exec::Message_GetPayload(const std::string& THE_MESSAGE) const
{
    OT_VERIFY_STD_STR(THE_MESSAGE);

    auto strMessage = String::Factory(THE_MESSAGE);
    auto theMessage{api_.Factory().Message()};

    if (!strMessage->Exists() ||
        !theMessage->LoadContractFromString(strMessage))
        return {};

    std::string pBuf = theMessage->m_ascPayload->Get();
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
    OT_VERIFY_STD_STR(THE_MESSAGE);

    auto strMessage = String::Factory(THE_MESSAGE);
    auto theMessage{api_.Factory().Message()};

    if (!strMessage->Exists() ||
        !theMessage->LoadContractFromString(strMessage))
        return {};

    auto strOutput = String::Factory(theMessage->m_strCommand->Get());
    std::string pBuf = strOutput->Get();
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
    OT_VERIFY_STD_STR(THE_MESSAGE);

    auto strMessage = String::Factory(THE_MESSAGE);
    auto theMessage{api_.Factory().Message()};

    if (!strMessage->Exists() ||
        !theMessage->LoadContractFromString(strMessage)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to load message.").Flush();
        return {};
    }

    // It's not a transaction request or response, so the Payload wouldn't
    // contain a ledger. (Don't want to pass back whatever it DOES contain
    // in that case, now do I?)
    //
    if ((false == theMessage->m_strCommand->Compare("notarizeTransaction")) &&
        (false ==
         theMessage->m_strCommand->Compare("notarizeTransactionResponse"))) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Wrong message type: ")(
            theMessage->m_strCommand)(".")
            .Flush();
        return {};
    }

    // The ledger is stored in the Payload, we'll grab it into the String.
    auto strOutput = String::Factory(theMessage->m_ascPayload);

    if (!strOutput->Exists()) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": No ledger found on message.")
            .Flush();
        return {};
    }

    std::string pBuf = strOutput->Get();

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
    OT_VERIFY_STD_STR(THE_MESSAGE);

    auto strMessage = String::Factory(THE_MESSAGE);
    auto theMessage{api_.Factory().Message()};

    if (!strMessage->Exists() ||
        !theMessage->LoadContractFromString(strMessage)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to load message.").Flush();
        return {};
    }

    // It's not a transaction request or response, so the Payload wouldn't
    // contain a ledger. (Don't want to pass back whatever it DOES contain
    // in that case, now do I?)
    //
    if ((false == theMessage->m_strCommand->Compare(
                      "registerInstrumentDefinitionResponse")) &&
        (false == theMessage->m_strCommand->Compare("issueBasketResponse"))) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Wrong message type: ")(
            theMessage->m_strCommand)(".")
            .Flush();
        return {};
    }

    auto strOutput =
        String::Factory(theMessage->m_strInstrumentDefinitionID->Get());

    if (!strOutput->Exists()) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": No new instrument definition ID found on message.")
            .Flush();
        return {};
    }

    std::string pBuf = strOutput->Get();

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
    OT_VERIFY_STD_STR(THE_MESSAGE);

    auto strMessage = String::Factory(THE_MESSAGE);
    auto theMessage{api_.Factory().Message()};

    if (!strMessage->Exists() ||
        !theMessage->LoadContractFromString(strMessage)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to load message.").Flush();
        return {};
    }

    // It's not an issue instrument definition response, so the m_strAcctID
    // wouldn't
    // contain an issuer account ID. (Don't want to pass back whatever it
    // DOES contain in that case, now do I?)
    //
    if (!theMessage->m_strCommand->Compare(
            "registerInstrumentDefinitionResponse")) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Wrong message type: ")(
            theMessage->m_strCommand)(".")
            .Flush();
        return {};
    }

    auto strOutput = String::Factory(theMessage->m_strAcctID->Get());

    if (!strOutput->Exists()) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": No issuer account ID found on message.")
            .Flush();
        return {};
    }

    std::string pBuf = strOutput->Get();

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
    OT_VERIFY_STD_STR(THE_MESSAGE);

    auto strMessage = String::Factory(THE_MESSAGE);

    auto theMessage{api_.Factory().Message()};

    if (!strMessage->Exists() ||
        !theMessage->LoadContractFromString(strMessage)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to load message.").Flush();
        return {};
    }

    // It's not a response to registerAccount, so the m_strAcctID wouldn't
    // contain a new account ID anyway, right? (Don't want to pass back
    // whatever it DOES contain in that case, now do I?)
    //
    if (!theMessage->m_strCommand->Compare("registerAccountResponse")) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Wrong message type: ")(
            theMessage->m_strCommand)(".")
            .Flush();
        return {};
    }

    auto strOutput = String::Factory(theMessage->m_strAcctID->Get());

    if (!strOutput->Exists()) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": No asset account ID found on message.")
            .Flush();
        return {};
    }

    std::string pBuf = strOutput->Get();

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
    OT_VERIFY_STD_STR(THE_MESSAGE);

    auto strMessage = String::Factory(THE_MESSAGE);
    auto theMessage{api_.Factory().Message()};

    if (!strMessage->Exists() ||
        !theMessage->LoadContractFromString(strMessage)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to load message.").Flush();
        return {};
    }

    // So far these are the only messages that use m_strNymboxHash:
    if ((false == theMessage->m_strCommand->Compare("processNymbox")) &&
        (false == theMessage->m_strCommand->Compare("notarizeTransaction")) &&
        (false == theMessage->m_strCommand->Compare("getTransactionNumbers")) &&
        (false == theMessage->m_strCommand->Compare("processInbox")) &&
        (false == theMessage->m_strCommand->Compare("triggerClause")) &&
        (false == theMessage->m_strCommand->Compare("getNymboxResponse")) &&
        (false ==
         theMessage->m_strCommand->Compare("getRequestNumberResponse")) &&
        (false ==
         theMessage->m_strCommand->Compare("getTransactionNumbersResponse"))) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Wrong message type : ")(
            theMessage->m_strCommand)(".")(" FYI, with m_strNymboxHash: ")(
            theMessage->m_strNymboxHash)(".")
            .Flush();
        return {};
    }

    if (!theMessage->m_strNymboxHash->Exists()) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": No NymboxHash found on message: ")(strMessage)(".")
            .Flush();
        return {};
    }

    auto strOutput = String::Factory(theMessage->m_strNymboxHash->Get());
    std::string pBuf = strOutput->Get();

    return pBuf;
}

// GET MESSAGE SUCCESS (True or False)
//
// Returns true (1) for Success and false (0) for Failure.
//
// NEW: returns (-1) for error!
//
std::int32_t OTAPI_Exec::Message_GetSuccess(
    const std::string& THE_MESSAGE) const
{
    OT_VERIFY_STD_STR(THE_MESSAGE);

    auto theMessage{api_.Factory().Message()};
    auto strMessage = String::Factory(THE_MESSAGE);

    if (!strMessage->Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error: THE_MESSAGE doesn't exist.")
            .Flush();
        return OT_ERROR;
    }

    if (!theMessage->LoadContractFromString(strMessage)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error: Failed loading message from string: ")(THE_MESSAGE)(".")
            .Flush();
        return OT_ERROR;
    }
    if (true == theMessage->m_bSuccess) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Server reply for RequestNum ")(
            theMessage->m_strRequestNum)(
            "(Message_GetSuccess was successful, but any transaction ")(
            "inside could have failed OR succeeded. Use "
            "Message_GetTransactionSuccess for that).")
            .Flush();  // Contents:
                       // \n\n" <<
                       // THE_MESSAGE
                       // << "\n\n"
        return OT_TRUE;
    } else {
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": ** FYI, server reply was received, and it ")(
            "said 'No.' (Status = failed). RequestNum: ")(
            StringToLong(theMessage->m_strRequestNum->Get()))
            .Flush();  // Contents:\n\n" << THE_MESSAGE << "\n\n"
    }
    return OT_FALSE;
}

// GET MESSAGE "DEPTH"   (USED FOR MARKET-SPECIFIC MESSAGES.)
//
// Returns the count of relevant items, so you know whether to bother
// reading the payload. Returns -1 if error.
//
// The "depth" variable stores the count of items being returned.
// For example, if I call getMarketList, and 10 markets are returned,
// then depth will be set to 10. OR, if I call getNymMarketOffers, and
// the Nym has 16 offers on the various markets, then the depth will be 16.
//
// This value is important when processing server replies to market
// inquiries. If the depth is 0, then you are done. End. BUT! if it contains
// a number, such as 10, then that means you will want to next READ those 10
// markets (or offers, or trades, etc) out of the server reply's payload.
//
// Whereas if success is TRUE, but depth is 0, that means while the message
// processed
// successfully, the list was simply empty (and thus it would be a waste of
// time trying to unpack the non-existent, empty list of data items from the
// payload of your successful reply.)
//
std::int32_t OTAPI_Exec::Message_GetDepth(const std::string& THE_MESSAGE) const
{
    OT_VERIFY_STD_STR(THE_MESSAGE);

    auto strMessage = String::Factory(THE_MESSAGE);
    auto theMessage{api_.Factory().Message()};

    if (!strMessage->Exists() ||
        !theMessage->LoadContractFromString(strMessage))
        return OT_ERROR;

    return static_cast<std::int32_t>(theMessage->m_lDepth);
}

// GET MESSAGE TRANSACTION "IS CANCELLED" (True or False)
//
// Returns true (1) for Success and false (0) for Failure.
//         also returns (-1) for Error
//
std::int32_t OTAPI_Exec::Message_IsTransactionCanceled(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_MESSAGE) const
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(ACCOUNT_ID);
    OT_VERIFY_STD_STR(THE_MESSAGE);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theNymID = Identifier::Factory(NYM_ID),
               theAccountID = Identifier::Factory(ACCOUNT_ID);

    auto strMessage = String::Factory(THE_MESSAGE);

    auto theMessage{api_.Factory().Message()};

    if (!strMessage->Exists() ||
        !theMessage->LoadContractFromString(strMessage)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to load message.").Flush();
        return OT_ERROR;
    }

    // It's not a transaction request or response, so the Payload wouldn't
    // contain a ledger. (Don't want to pass back whatever it DOES contain
    // in that case, now do I?)
    //
    if ((false ==
         theMessage->m_strCommand->Compare("notarizeTransactionResponse")) &&
        (false == theMessage->m_strCommand->Compare("processInboxResponse")) &&
        (false == theMessage->m_strCommand->Compare("processNymboxResponse"))) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Wrong message type: ")(
            theMessage->m_strCommand)(".")
            .Flush();
        return OT_ERROR;
    }

    // The ledger is stored in the Payload, we'll grab it into the String.
    auto strLedger = String::Factory(theMessage->m_ascPayload);

    if (!strLedger->Exists()) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": No ledger found on message.")
            .Flush();
        return OT_ERROR;
    }

    auto theLedger{api_.Factory().Ledger(theNymID, theAccountID, theNotaryID)};

    if (!theLedger->LoadContractFromString(strLedger)) {
        auto strAcctID = String::Factory(theAccountID);
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading ledger from string. Acct ID: ")(strAcctID)(".")
            .Flush();
        return OT_ERROR;
    }

    // At this point, I know theLedger loaded successfully.

    if (theLedger->GetTransactionCount() <= 0) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Bad count in message ledger: ")(
            theLedger->GetTransactionCount())(".")
            .Flush();
        return OT_ERROR;  // out of bounds. I'm saving from an
                          // OT_ASSERT_MSG() happening here. (Maybe I
                          // shouldn't.)
    }

    auto pTransaction = theLedger->GetTransactionByIndex(
        0);  // Right now this is a defacto standard. (only 1 transaction
             // per message ledger, excepting process inbox.)

    if (false == bool(pTransaction)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Good index but uncovered pointer: ")(0)(".")
            .Flush();
        return OT_ERROR;  // Weird.
    }

    // At this point, I actually have the transaction pointer, so let's
    // return its 'canceled' status
    //
    if (pTransaction->IsCancelled()) return OT_TRUE;

    return OT_FALSE;
}

// GET MESSAGE TRANSACTION SUCCESS (True or False)
//
// Returns true (1) for Success and false (0) for Failure.
//         also returns (-1) for Error
//
std::int32_t OTAPI_Exec::Message_GetTransactionSuccess(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_MESSAGE) const
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(ACCOUNT_ID);
    OT_VERIFY_STD_STR(THE_MESSAGE);

    const auto theNotaryID = Identifier::Factory(NOTARY_ID),
               theNymID = Identifier::Factory(NYM_ID),
               theAccountID = Identifier::Factory(ACCOUNT_ID);

    auto strMessage = String::Factory(THE_MESSAGE);

    auto theMessage{api_.Factory().Message()};

    if (!strMessage->Exists() ||
        !theMessage->LoadContractFromString(strMessage)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to load message.").Flush();
        return OT_ERROR;
    }

    // It's not a transaction request or response, so the Payload wouldn't
    // contain a ledger. (Don't want to pass back whatever it DOES contain
    // in that case, now do I?)
    //
    if ((false ==
         theMessage->m_strCommand->Compare("notarizeTransactionResponse")) &&
        (false == theMessage->m_strCommand->Compare("processInboxResponse")) &&
        (false == theMessage->m_strCommand->Compare("processNymboxResponse"))) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Wrong message type: ")(
            theMessage->m_strCommand)(".")
            .Flush();
        return OT_ERROR;
    }

    // The ledger is stored in the Payload, we'll grab it into the String.
    auto strLedger = String::Factory(theMessage->m_ascPayload);

    if (!strLedger->Exists()) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": No ledger found on message.")
            .Flush();
        return OT_ERROR;
    }

    auto theLedger{api_.Factory().Ledger(theNymID, theAccountID, theNotaryID)};

    if (!theLedger->LoadContractFromString(strLedger)) {
        auto strAcctID = String::Factory(theAccountID);
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading ledger from string. Acct ID: ")(strAcctID)(".")
            .Flush();
        return OT_ERROR;
    }

    // At this point, I know theLedger loaded successfully.

    if (theLedger->GetTransactionCount() <= 0) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Bad count in message ledger: ")(
            theLedger->GetTransactionCount())(".")
            .Flush();
        return OT_ERROR;  // out of bounds. I'm saving from an
                          // OT_ASSERT_MSG() happening here. (Maybe I
                          // shouldn't.)
    }

    auto pTransaction = theLedger->GetTransactionByIndex(
        0);  // Right now this is a defacto standard. (only 1 transaction
             // per message ledger, excepting process inbox.)

    if (false == bool(pTransaction)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Good index but uncovered pointer: ")(0)(".")
            .Flush();
        return OT_ERROR;  // Weird.
    }

    // At this point, I actually have the transaction pointer, so let's
    // return its success status
    //
    if (pTransaction->GetSuccess())
        return OT_TRUE;
    else {
        const std::int64_t lRequestNum =
            StringToLong(theMessage->m_strRequestNum->Get());
        const std::int64_t lTransactionNum = pTransaction->GetTransactionNum();

        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": ** FYI, server reply was received, and it said 'No.' ")(
            "(Status = failed). RequestNum: ")(lRequestNum)(", TransNum: ")(
            lTransactionNum)
            .Flush();
        // Contents: \n\n" << THE_MESSAGE << "\n\n"
    }

    return OT_FALSE;
}

std::string OTAPI_Exec::ContactAttributeName(
    const proto::ContactItemAttribute type,
    std::string lang) const
{
    return proto::TranslateItemAttributes(type, lang);
}

std::set<proto::ContactSectionName> OTAPI_Exec::ContactSectionList(
    const std::uint32_t version) const
{
    return proto::AllowedSectionNames.at(version);
}

std::string OTAPI_Exec::ContactSectionName(
    const proto::ContactSectionName section,
    std::string lang) const
{
    return proto::TranslateSectionName(section, lang);
}

std::set<proto::ContactItemType> OTAPI_Exec::ContactSectionTypeList(
    const proto::ContactSectionName section,
    const std::uint32_t version) const
{
    proto::ContactSectionVersion contactVersion{version, section};

    return proto::AllowedItemTypes.at(contactVersion);
}

std::string OTAPI_Exec::ContactTypeName(
    const proto::ContactItemType type,
    std::string lang) const
{
    return proto::TranslateItemType(type, lang);
}

proto::ContactItemType OTAPI_Exec::ReciprocalRelationship(
    const proto::ContactItemType relationship) const
{
    return static_cast<proto::ContactItemType>(
        proto::ReciprocalRelationship(relationship));
}

std::string OTAPI_Exec::Wallet_GetSeed() const
{
    return ot_api_.Wallet_GetSeed();
}

std::string OTAPI_Exec::Wallet_GetPassphrase() const
{
    return ot_api_.Wallet_GetPhrase();
}

std::string OTAPI_Exec::Wallet_GetWords() const
{
    return ot_api_.Wallet_GetWords();
}

std::string OTAPI_Exec::Wallet_ImportSeed(
    const std::string& words,
    const std::string& passphrase) const
{
    OTPassword secureWords, securePassphrase;
    secureWords.setPassword(words);
    securePassphrase.setPassword(passphrase);

    return ot_api_.Wallet_ImportSeed(secureWords, securePassphrase);
}

bool OTAPI_Exec::AddClaim(
    const std::string& nymID,
    const std::uint32_t& section,
    const std::uint32_t& type,
    const std::string& value,
    const bool active,
    const bool primary,
    const std::int64_t start,
    const std::int64_t end) const
{
    auto nym = api_.Wallet().mutable_Nym(Identifier::Factory(nymID));

    return ot_api_.AddClaim(
        nym,
        static_cast<proto::ContactSectionName>(section),
        static_cast<proto::ContactItemType>(type),
        value,
        primary,
        active,
        start,
        end);
}

void OTAPI_Exec::SetZMQKeepAlive(const std::uint64_t seconds) const
{
    zeromq_.KeepAlive(std::chrono::seconds(seconds));
}

bool OTAPI_Exec::CheckConnection(const std::string& server) const
{
    return ConnectionState::ACTIVE == ot_api_.CheckConnection(server);
}

std::string OTAPI_Exec::AddChildEd25519Credential(
    const Identifier& nymID,
    const Identifier& masterID) const
{
    std::string output;
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
#if OT_CRYPTO_SUPPORTED_KEY_HD
    NymParameters nymParameters(proto::CREDTYPE_HD);
#else
    NymParameters nymParameters(proto::CREDTYPE_LEGACY);
#endif
    nymParameters.setNymParameterType(NymParameterType::ED25519);
    output = ot_api_.AddChildKeyCredential(nymID, masterID, nymParameters);
#endif

    return output;
}

std::string OTAPI_Exec::AddChildSecp256k1Credential(
    const Identifier& nymID,
    const Identifier& masterID) const
{
    std::string output;
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#if OT_CRYPTO_SUPPORTED_KEY_HD
    NymParameters nymParameters(proto::CREDTYPE_HD);
#else
    NymParameters nymParameters(proto::CREDTYPE_LEGACY);
#endif
    nymParameters.setNymParameterType(NymParameterType::SECP256K1);
    output = ot_api_.AddChildKeyCredential(nymID, masterID, nymParameters);
#endif

    return output;
}

std::string OTAPI_Exec::AddChildRSACredential(
    const Identifier& nymID,
    const Identifier& masterID,
    const std::uint32_t keysize) const
{
    std::string output;
#if OT_CRYPTO_SUPPORTED_KEY_RSA

    if (0 >= keysize) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Keysize is 0 or less, will fail! Try 1024.")
            .Flush();

        return output;
    }

    switch (keysize) {
        case 1024:
        case 2048:
        case 4096:
        case 8192:
            break;
        default:
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failure: keysize must be one of: "
                "1024, 2048, 4096, 8192. (")(keysize)(" was passed...).")
                .Flush();

            return output;
    }

    NymParameters nymParameters(keysize);
    output = ot_api_.AddChildKeyCredential(nymID, masterID, nymParameters);
#endif

    return output;
}
}  // namespace opentxs
