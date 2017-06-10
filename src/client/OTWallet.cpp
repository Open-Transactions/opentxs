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

#include "opentxs/client/OTWallet.hpp"

#include "opentxs/api/OT.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/cash/Purse.hpp"
#include "opentxs/core/crypto/Bip32.hpp"
#include "opentxs/core/crypto/Bip39.hpp"
#include "opentxs/core/crypto/CryptoEngine.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/crypto/OTCachedKey.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/crypto/OTSymmetricKey.hpp"
#ifdef ANDROID
#include "opentxs/core/util/android_string.hpp"
#endif // ANDROID
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/OTDataFolder.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/core/OTStringXML.hpp"
#include "opentxs/core/Proto.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/Types.hpp"

#include <stdint.h>
#include <irrxml/irrXML.hpp>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <utility>

namespace opentxs
{

OTWallet::OTWallet()
    : m_strDataFolder(OTDataFolder::Get())
{
    m_pWithdrawalPurse = nullptr;
}
//
OTWallet::~OTWallet() { Release(); }

void OTWallet::Release()
{
    m_mapPrivateNyms.clear();

    // Go through the map of Accounts and delete them. (They were dynamically
    // allocated.)
    while (!m_mapAccounts.empty()) {
        Account* pAccount = m_mapAccounts.begin()->second;

        OT_ASSERT(nullptr != pAccount);

        delete pAccount;
        pAccount = nullptr;

        m_mapAccounts.erase(m_mapAccounts.begin());
    }

    // Watch how much prettier this one is, since we used smart pointers!
    //
    m_mapExtraKeys.clear();
}

// While waiting on server response to a withdrawal, we keep the private coin
// data here so we can unblind the response.
// This information is so important (as important as the digital cash token
// itself, until the unblinding is done) that we need to save the file right
// away.
void OTWallet::AddPendingWithdrawal(const Purse& thePurse)
{
    // TODO maintain a list here (I don't know why, the server response is
    // nearly
    // instant and then it's done.)

    // TODO notice I don't check the pointer here to see if it's already set, I
    // just start using it.. Fix that.
    m_pWithdrawalPurse = const_cast<Purse*>(&thePurse);
}  // TODO WARNING: If this data is lost before the transaction is completed,
   // the user will be unable to unblind his tokens and make them spendable.
   // So this data MUST be SAVED until the successful withdrawal is verified!

void OTWallet::RemovePendingWithdrawal()
{
    if (m_pWithdrawalPurse) delete m_pWithdrawalPurse;

    m_pWithdrawalPurse = nullptr;
}

bool OTWallet::SignContractWithFirstNymOnList(Contract& theContract)
{
    if (GetNymCount() > 0) {
        Identifier NYM_ID;
        String NYM_NAME;

        if (GetNym(
                0,  // index 0
                NYM_ID,
                NYM_NAME)) {
            Nym* pNym = GetPrivateNymByID(NYM_ID);

            if (nullptr != pNym) {
                theContract.SignContract(*pNym);
                return true;
            }
        }
    }

    return false;
}

std::string OTWallet::GetPhrase()
{
#if OT_CRYPTO_SUPPORTED_KEY_HD
    const std::string defaultFingerprint = OT::App().DB().DefaultSeed();
    const bool firstTime = defaultFingerprint.empty();

    if (firstTime) {
        SaveWallet();
    }

    return OT::App().Crypto().BIP39().Passphrase(defaultFingerprint);
#else
    return "";
#endif
}

std::string OTWallet::GetSeed()
{
#if OT_CRYPTO_SUPPORTED_KEY_HD
    const std::string defaultFingerprint = OT::App().DB().DefaultSeed();
    const bool firstTime = defaultFingerprint.empty();

    if (firstTime) {
        SaveWallet();
    }

    return OT::App().Crypto().BIP32().Seed(defaultFingerprint);
#else
    return "";
#endif
}

std::string OTWallet::GetWords()
{
#if OT_CRYPTO_SUPPORTED_KEY_HD
    const std::string defaultFingerprint = OT::App().DB().DefaultSeed();
    const bool firstTime = defaultFingerprint.empty();

    if (firstTime) {
        SaveWallet();
    }

    return OT::App().Crypto().BIP39().Words(defaultFingerprint);
#else
    return "";
#endif
}

std::string OTWallet::ImportSeed(
    const OTPassword& words,
    const OTPassword& passphrase) const
{
#if OT_CRYPTO_WITH_BIP39
    return OT::App().Crypto().BIP39().ImportSeed(words, passphrase);
#else
    return "";
#endif
}

// No need to delete Nym returned by this function.
// (Wallet stores it in RAM and will delete when it destructs.)
Nym* OTWallet::CreateNym(const NymParameters& nymParameters)
{
    std::unique_ptr<Nym> pNym(new Nym(nymParameters));

    OT_ASSERT(pNym);

    if (pNym->VerifyPseudonym()) {
        // Takes ownership
        this->AddPrivateNym(*pNym);

        // NOTE: It's already on the master key. To prevent that, we would have
        // had to PAUSE the master key before calling GenerateNym above. So the
        // below call is less about the Nym's encryption, and more about the
        // wallet KNOWING. Because OTWallet::ConvertNymToCachedKey is what adds
        // this nym to the wallet's list of "master key nyms". Until that
        // happens, the wallet has no idea.
        if (!this->ConvertNymToCachedKey(*pNym))
            otErr << __FUNCTION__
                  << ": Error: Failed in OTWallet::ConvertNymToCachedKey.\n";

        this->SaveWallet();  // Since it just changed.

        // By this point, pNym is a good pointer, and is on the wallet.
        //  (No need to cleanup.)
        return pNym.release();
    } else {

        return nullptr;
    }
}

// The wallet presumably has multiple Nyms listed within.
// I should be able to pass in a Nym ID and, if the Nym is there,
// the wallet returns a pointer to that nym.
Nym* OTWallet::GetPrivateNymByID(const Identifier& NYM_ID)
{
    Nym* output = nullptr;

    try {
        auto& it = m_mapPrivateNyms.at(String(NYM_ID).Get());

        if (it) {
            output = it.get();
        }
    }
    catch (const std::out_of_range) {
        output = nullptr;
    }

    return output;
}

Nym* OTWallet::GetNymByIDPartialMatch(std::string PARTIAL_ID)  // works
                                                               // with
                                                               // name as
                                                               // well.
{
    for (auto& it : m_mapPrivateNyms) {
        String strTemp;
        it.second->GetIdentifier(strTemp);
        std::string strIdentifier = strTemp.Get();

        if (strIdentifier.compare(0, PARTIAL_ID.length(), PARTIAL_ID) == 0)

            return it.second.get();
    }

    // OK, let's try it by the name, then...
    //
    for (auto& it : m_mapPrivateNyms) {
        std::string str_NymName = it.second->Alias();

        if (str_NymName.compare(0, PARTIAL_ID.length(), PARTIAL_ID) == 0)

            return it.second.get();
    }

    return nullptr;
}

// used by high-level wrapper.
int32_t OTWallet::GetNymCount()
{
    return static_cast<int32_t>(m_mapPrivateNyms.size());
}

int32_t OTWallet::GetAccountCount()
{
    return static_cast<int32_t>(m_mapAccounts.size());
}

// used by high-level wrapper.
bool OTWallet::GetNym(int32_t iIndex, Identifier& NYM_ID, String& NYM_NAME)
{
    // if iIndex is within proper bounds (0 through count minus 1)
    if (iIndex < GetNymCount() && iIndex >= 0) {
        int32_t iCurrentIndex = (-1);

        for (auto& it : m_mapPrivateNyms) {
            iCurrentIndex++;  // On first iteration, this becomes 0 here. (For 0
                              // index.) Increments thereafter.

            if (iIndex == iCurrentIndex) {
                it.second->GetIdentifier(NYM_ID);
                NYM_NAME.Set(String(it.second->Alias()));

                return true;
            }
        }
    }

    return false;
}

// used by high-level wrapper.
bool OTWallet::GetAccount(int32_t iIndex, Identifier& THE_ID, String& THE_NAME)
{
    // if iIndex is within proper bounds (0 through count minus 1)
    if (iIndex < GetAccountCount() && iIndex >= 0) {
        int32_t iCurrentIndex = (-1);

        for (auto& it : m_mapAccounts) {
            Account* pAccount = it.second;
            OT_ASSERT(nullptr != pAccount);

            iCurrentIndex++;  // On first iteration, this becomes 0 here. (For 0
                              // index.) Increments thereafter.

            if (iIndex == iCurrentIndex)  // if not null
            {
                pAccount->GetIdentifier(THE_ID);
                pAccount->GetName(THE_NAME);
                return true;
            }
        }
    }

    return false;
}

void OTWallet::DisplayStatistics(String& strOutput)
{
    strOutput.Concatenate(
        "\n-------------------------------------------------\n");
    strOutput.Concatenate("WALLET STATISTICS:\n");

    strOutput.Concatenate("\nPSEUDONYM(s):\n\n");

    for (auto& it : m_mapPrivateNyms) {
        const auto& pNym = it.second;

        if (pNym) {
            pNym->DisplayStatistics(strOutput);
        }
    }

    strOutput.Concatenate(
        "-------------------------------------------------\n");
    strOutput.Concatenate("ACCOUNTS:\n\n");

    for (auto& it : m_mapAccounts) {
        Account* pAccount = it.second;
        OT_ASSERT_MSG(
            nullptr != pAccount,
            "nullptr account pointer in "
            "OTWallet::m_mapAccounts, "
            "OTWallet::DisplayStatistics");

        pAccount->DisplayStatistics(strOutput);

        strOutput.Concatenate(
            "-------------------------------------------------\n\n");
    }
}

// Wallet takes ownership and will delete.
// theNym is passed as reference only to prove that it's real.
//
// This function assumes the Nym has already been loaded, verified, etc.
// AND that it's been dynamically allocated.
//
void OTWallet::AddPrivateNym(const Nym& theNym)
{
    AddNym(theNym, m_mapPrivateNyms);
}

void OTWallet::AddNym(const Nym& theNym, mapOfNymsSP& map)
{
    const std::string id = String(Identifier(theNym)).Get();
    auto& it = map[id];
    const bool haveExisting = bool(it);
    Nym* input = const_cast<Nym*>(&theNym);

    if (haveExisting) {
        if (input != it.get()) {
            const std::string oldAlias = it->Alias();
            it.reset(input);

            if (it->Alias().empty() && (!oldAlias.empty())) {
                it->SetAlias(oldAlias);
            }
        }
    } else {
        it.reset(input);
    }
}

void OTWallet::AddAccount(const Account& theAcct)
{
    const Identifier ACCOUNT_ID(theAcct);

    // See if there is already an account object on this wallet with the same ID
    // (Otherwise if we don't delete it, this would be a memory leak.)
    // Should use a smart pointer.
    Identifier anAccountID;

    for (auto it(m_mapAccounts.begin()); it != m_mapAccounts.end(); ++it) {
        Account* pAccount = it->second;
        OT_ASSERT(nullptr != pAccount);

        pAccount->GetIdentifier(anAccountID);

        if (anAccountID == ACCOUNT_ID) {
            String strName;
            pAccount->GetName(strName);

            if (strName.Exists()) {
                const_cast<Account&>(theAcct).SetName(strName);
            }

            m_mapAccounts.erase(it);
            delete pAccount;
            pAccount = nullptr;

            break;
        }
    }

    const String strAcctID(ACCOUNT_ID);
    m_mapAccounts[strAcctID.Get()] = const_cast<Account*>(&theAcct);
}

// Look up an account by ID and see if it is in the wallet.
// If it is, return a pointer to it, otherwise return nullptr.
Account* OTWallet::GetAccount(const Identifier& theAccountID)
{
    // loop through the accounts and find one with a specific ID.
    //
    for (auto& it : m_mapAccounts) {
        Account* pAccount = it.second;
        OT_ASSERT(nullptr != pAccount);

        Identifier anAccountID;
        pAccount->GetIdentifier(anAccountID);

        if (anAccountID == theAccountID) return pAccount;
    }

    return nullptr;
}

Account* OTWallet::GetAccountPartialMatch(std::string PARTIAL_ID)  // works
                                                                   // with the
                                                                   // name,
                                                                   // too.
{
    // loop through the accounts and find one with a specific ID.
    for (auto& it : m_mapAccounts) {
        Account* pAccount = it.second;
        OT_ASSERT(nullptr != pAccount);

        Identifier anAccountID;
        pAccount->GetIdentifier(anAccountID);
        String strTemp(anAccountID);
        std::string strIdentifier = strTemp.Get();

        if (strIdentifier.compare(0, PARTIAL_ID.length(), PARTIAL_ID) == 0)
            return pAccount;
    }

    // Okay, let's try it by name, then...
    //
    for (auto& it : m_mapAccounts) {
        Account* pAccount = it.second;
        OT_ASSERT(nullptr != pAccount);

        String strName;
        pAccount->GetName(strName);
        std::string str_Name = strName.Get();

        if (str_Name.compare(0, PARTIAL_ID.length(), PARTIAL_ID) == 0)
            return pAccount;
    }

    return nullptr;
}

Account* OTWallet::GetIssuerAccount(const Identifier& theInstrumentDefinitionID)
{
    // loop through the accounts and find one with a specific instrument
    // definition ID.
    // (And with the issuer type set.)
    //
    for (auto& it : m_mapAccounts) {
        Account* pIssuerAccount = it.second;
        OT_ASSERT(nullptr != pIssuerAccount);

        if ((pIssuerAccount->GetInstrumentDefinitionID() ==
             theInstrumentDefinitionID) &&
            (pIssuerAccount->IsIssuer()))
            return pIssuerAccount;
    }

    return nullptr;
}

bool OTWallet::VerifyAssetAccount(
    const Nym& theNym,
    Account& theAcct,
    const Identifier& NOTARY_ID,
    const String& strAcctID,
    const char* szFuncName)
{
    const char* szFunc =
        (nullptr != szFuncName) ? szFuncName : "OTWallet::VerifyAssetAccount";

    if (NOTARY_ID != theAcct.GetRealNotaryID()) {
        const String s1(NOTARY_ID), s2(theAcct.GetRealNotaryID());
        otOut << "OTWallet::VerifyAssetAccount " << szFunc
              << ": Notary ID passed in (" << s1 << ") didn't match the one "
                                                    "on the account ("
              << s2 << "). Acct ID: " << strAcctID << "\n";
        return false;
    }

    const Identifier theNymID(theNym);
    const String strNymID(theNymID);

    if (!theAcct.VerifyOwner(theNym))  // Verifies Ownership.
    {
        otOut << "OTWallet::VerifyAssetAccount " << szFunc
              << ": Nym (ID: " << strNymID
              << ") is not the owner of the account: " << strAcctID << "\n";
        return false;
    }

    if (false ==
        theAcct.VerifyAccount(theNym))  // Verifies ContractID and Signature.
    {
        otOut << "OTWallet::VerifyAssetAccount " << szFunc
              << ": Account signature or AccountID fails to verify. "
                 "NymID: "
              << strNymID << "  AcctID: " << strAcctID << "\n";
        return false;
    }
    // By this point, I know that everything checks out. Signature and Account
    // ID,
    // Nym is owner, etc.

    return true;
}

// No need to cleanup the account returned, it's owned by the wallet.
//
Account* OTWallet::GetOrLoadAccount(
    const Nym& theNym,
    const Identifier& ACCT_ID,
    const Identifier& NOTARY_ID,
    const char* szFuncName)
{
    const char* szFunc =
        (nullptr != szFuncName) ? szFuncName : "OTWallet::GetOrLoadAccount";

    const String strAcctID(ACCT_ID);

    Account* pAccount = GetAccount(ACCT_ID);

    if (nullptr ==
        pAccount)  // It wasn't there already, so we'll have to load it...
    {
        otOut << "OTWallet::GetOrLoadAccount " << szFunc
              << ": There's no asset account in the wallet with that ID ("
              << strAcctID << "). "
                              "Attempting to load it from storage...\n";
        pAccount = LoadAccount(theNym, ACCT_ID, NOTARY_ID, szFuncName);
    }  // pAccount == nullptr.

    // It either was already there, or it loaded successfully...
    //
    if (nullptr == pAccount)  // pAccount EXISTS...
    {
        otErr << "OTWallet::GetOrLoadAccount " << szFunc
              << ": Error loading Asset Account: " << strAcctID << "\n";
        return nullptr;
    }

    return pAccount;
}

// No need to cleanup the account returned, it's owned by the wallet.
//
// We don't care if this asset account is already loaded in the wallet.
// Presumably, the user has just download the latest copy of the account
// from the server, and the one in the wallet is old, so now this function
// is being called to load the new one from storage and update the wallet.
//
Account* OTWallet::LoadAccount(
    const Nym& theNym,
    const Identifier& ACCT_ID,
    const Identifier& NOTARY_ID,
    const char* szFuncName)
{
    const char* szFunc =
        (nullptr != szFuncName) ? szFuncName : "OTWallet::LoadAccount";

    const String strAcctID(ACCT_ID);
    Account* pAccount = Account::LoadExistingAccount(ACCT_ID, NOTARY_ID);

    // It loaded successfully...
    //
    if (nullptr != pAccount)  // pAccount EXISTS...
    {
        bool bVerified =
            VerifyAssetAccount(theNym, *pAccount, NOTARY_ID, strAcctID, szFunc);

        if (!bVerified) {
            delete pAccount;
            pAccount = nullptr;
            return nullptr;  // No need to log, since VerifyAssetAccount()
                             // already
                             // logs.
        }

        // If I had to load it myself, that means I need to add it to the
        // wallet.
        // (Whereas if GetAccount() had worked, then it would ALREADY be in the
        // wallet,
        // and thus I shouldn't add it twice...)
        //
        AddAccount(*pAccount);

    } else {
        otErr << "OTWallet::LoadAccount " << szFunc
              << ": Failed loading Asset Account: " << strAcctID << "\n";
        return nullptr;
    }

    return pAccount;
}

// This function only tries to load as a private Nym.
// No need to cleanup, since it adds the Nym to the wallet.
//
// It is smart enough to Get the Nym from the wallet, and if it
// sees that it's only a public nym (no private key) then it
// reloads it as a private nym at that time.
//
Nym* OTWallet::GetOrLoadPrivateNym(
    const Identifier& NYM_ID,
    bool bChecking,
    const char* szFuncName,
    const OTPasswordData* pPWData,
    const OTPassword* pImportPassword)
{
    if (NYM_ID.IsEmpty()) {
        otErr << __FUNCTION__ << ":" << szFuncName
              << ": Error: NYM_ID passed in empty, returning null";
        return nullptr;
    }

    const String strNymID(NYM_ID);
    String strNymName;
    OTPasswordData thePWData(OT_PW_DISPLAY);
    if (nullptr == pPWData) pPWData = &thePWData;

    szFuncName = (szFuncName == nullptr) ? "" : szFuncName;

    // See if it's already there. (Could be the public version
    // though :P Still might have to reload it.)
    Nym* pNym = GetPrivateNymByID(NYM_ID);
    if (nullptr != pNym) return pNym;  // Found.

    // Wasn't already in the wallet. Let's try loading it...
    otWarn << __FUNCTION__ << " " << szFuncName
           << ": There's no Nym already loaded with that ID. "
              "Attempting to load private key...\n";
    pNym = Nym::LoadPrivateNym(
        NYM_ID,
        bChecking,
        &strNymName,
        szFuncName,  // <===========
        pPWData,
        pImportPassword);

    // LoadPrivateNym has plenty of error logging already.
    if (nullptr == pNym) {
        OTLogStream& otLog = bChecking ? otWarn : otOut;
        otLog << __FUNCTION__ << ": " << szFuncName << ": ("
              << "bChecking"
              << ": is " << (bChecking ? "true" : "false")
              << ").  Unable to load Private Nym for: " << strNymID << "\n";
        return nullptr;
    }

    AddPrivateNym(*pNym);
    return pNym;
}

Nym* OTWallet::reloadAndGetPrivateNym(
    const Identifier& NYM_ID,
    bool bChecking /*=false*/,
    const char* szFuncName /*=nullptr*/,
    const OTPasswordData* pPWData /*=nullptr*/,
    const OTPassword* pImportPassword /*=nullptr*/)
{
    szFuncName = (szFuncName == nullptr) ? "" : szFuncName;

    if (NYM_ID.IsEmpty()) {
        otErr << __FUNCTION__ << ":" << szFuncName
              << ": Error: NYM_ID passed in empty, returning null";
        return nullptr;
    }
    const String strNymID(NYM_ID);
    String strNymName, strFirstName, strSecondName;

    OTPasswordData thePWData(OT_PW_DISPLAY);
    if (nullptr == pPWData) pPWData = &thePWData;
    // --------------------------------------------
    // Unload if the nym is already loaded.
    RemovePrivateNym(NYM_ID, false, &strFirstName);

    if (strFirstName.Exists())
        strNymName = strFirstName;
    else if (strSecondName.Exists())
        strNymName = strSecondName;
    // --------------------------------------------
    Nym* pNym = Nym::LoadPrivateNym(
        NYM_ID, bChecking, &strNymName, szFuncName, pPWData, pImportPassword);
    // --------------------------------------------
    // LoadPrivateNym has plenty of error logging already.
    if (nullptr == pNym) {
        OTLogStream& otLog = bChecking ? otWarn : otOut;
        otLog << __FUNCTION__ << ": " << szFuncName << ": ("
              << "bChecking"
              << ": is " << (bChecking ? "true" : "false")
              << ").  Unable to load Private Nym for: " << strNymID << "\n";
        return nullptr;
    }

    AddPrivateNym(*pNym);
    return pNym;
}

// These functions are low-level. They don't check for dependent data before
// deleting,
// and they don't save the wallet after they do.
//
// You have to handle that at a higher level.

// higher level version of this will require a server message, in addition to
// removing from wallet.
bool OTWallet::RemovePrivateNym(
    const Identifier& theTargetID,
    bool bRemoveFromCachedKey /*=true*/,
    String* pStrOutputName /*=nullptr*/)
{
    return RemoveNym(
        theTargetID, m_mapPrivateNyms, bRemoveFromCachedKey, pStrOutputName);
}

bool OTWallet::RemoveNym(
    const Identifier& theTargetID,
    mapOfNymsSP& map,
    bool bRemoveFromCachedKey /*=true*/,
    String* pStrOutputName /*=nullptr*/)
{
    const std::string id = String(theTargetID).Get();
    try {
        auto& it = map.at(id);

        if (nullptr != pStrOutputName) {
            *pStrOutputName = String(it->Alias());
        }

        map.erase(id);

        if (bRemoveFromCachedKey) {
            m_setNymsOnCachedKey.erase(theTargetID);
        }

        return true;
    }
    catch (std::out_of_range) {

        return false;
    }
}

// higher level version of this will require a server message, in addition to
// removing from wallet.
bool OTWallet::RemoveAccount(const Identifier& theTargetID)
{
    // loop through the accounts and find one with a specific ID.
    Identifier anAccountID;

    for (auto it(m_mapAccounts.begin()); it != m_mapAccounts.end(); ++it) {
        Account* pAccount = it->second;
        OT_ASSERT(nullptr != pAccount);

        pAccount->GetIdentifier(anAccountID);

        if (anAccountID == theTargetID) {
            m_mapAccounts.erase(it);
            delete pAccount;
            return true;
        }
    }

    return false;
}

bool OTWallet::SaveContract(String& strContract)
{
    Tag tag("wallet");

    // Name is in the clear in memory,
    // and base64 in storage.
    OTASCIIArmor ascName;
    if (m_strName.Exists()) {
        ascName.SetString(m_strName, false);  // linebreaks == false
    }

    tag.add_attribute("name", m_strName.Exists() ? ascName.Get() : "");
    tag.add_attribute(
        "version",
        OTCachedKey::It()->IsGenerated() ? "2.0" : m_strVersion.Get());

    if (OTCachedKey::It()->IsGenerated())  // If it exists, then serialize it.
    {
        OTASCIIArmor ascMasterContents;

        if (OTCachedKey::It()->SerializeTo(ascMasterContents)) {
            tag.add_tag("cachedKey", ascMasterContents.Get());
        } else
            otErr << "OTWallet::SaveContract: Failed trying to write master "
                     "key to wallet.\n";
    }

    // Save the extra symmetric keys. (The ones the client app might use to
    // encrypt his local sql-lite DB's record of his Bitmessage connect string,
    // or any other local data.)
    //
    for (auto& it : m_mapExtraKeys) {
        const std::string str_id = it.first;
        std::shared_ptr<OTSymmetricKey> pKey = it.second;

        String strKeyID(str_id.c_str());
        OTASCIIArmor ascKeyID;

        ascKeyID.SetString(
            strKeyID,
            false);  // linebreaks=false (true by default.)

        OTASCIIArmor ascKeyContents;

        if (pKey && pKey->SerializeTo(ascKeyContents)) {
            TagPtr pTag(new Tag("symmetricKey", ascKeyContents.Get()));
            pTag->add_attribute("id", ascKeyID.Get());
            tag.add_tag(pTag);
        } else
            otErr << "OTWallet::SaveContract: Failed trying to serialize "
                     "symmetric keys to wallet.\n";
    }

    //
    // We want to save the NymIDs for the Nyms on the master key.
    // I save those before the Nyms themselves, so that they are
    // all loaded up and available in LoadWallet before the Nyms
    // themselves are loaded.
    //
    for (const auto& it : m_setNymsOnCachedKey) {
        const Identifier& theNymID = it;
        String strNymID(theNymID);

        TagPtr pTag(new Tag("nymUsingCachedKey"));
        pTag->add_attribute("id", strNymID.Get());
        tag.add_tag(pTag);
    }

    for (auto& it : m_mapPrivateNyms) {
        auto& nym = it.second;

        if (nym) {
            nym->SavePseudonymWallet(tag);
        }
    }

    for (auto& it : m_mapAccounts) {
        Contract* pAccount = it.second;
        OT_ASSERT_MSG(
            nullptr != pAccount,
            "nullptr account pointer in "
            "OTWallet::m_mapAccounts, "
            "OTWallet::SaveContract");

        pAccount->SaveContractWallet(tag);
    }

    std::string str_result;
    tag.output(str_result);

    strContract.Concatenate("%s", str_result.c_str());

    return true;
}

// Let's say you have client-app data that you want to keep in encrypted form.
// Well, use this function to create/retrieve a symmetric key based on an ID.
// For example, "mc_sql_lite" might be the name of the symmetric key that I use
// to encrypt sensitive contents in the sql*lite DB.
// This function will find or create the key and return it to you. The key is
// encrypted to the master key in the wallet, so you never actually have to type
// a password to use it, except when the master key itself has expired.
//
std::shared_ptr<OTSymmetricKey> OTWallet::getOrCreateExtraKey(
    const std::string& str_KeyID,
    const std::string* pReason)
{
    //  const std::string str_KeyID("mc_sql_lite");

    // Get the appropriate symmetric key from the wallet.
    // (Which we will decrypt using pMaster.)
    // Once it's decrypted, we'll use this key for encrypting/decrypting
    // the sql*lite DB data on the client side.
    //
    std::shared_ptr<OTSymmetricKey> pExtraKey = getExtraKey(str_KeyID);

    // (If it doesn't exist, let's just create it here.)
    //
    if (!pExtraKey) {
        // The extra keys, like the Nyms, are all encrypted to the master key
        // for the wallet.
        // Thus, to create a new extra symmetrical key, we need to get the
        // master key from OTCachedKey...
        //
        std::shared_ptr<OTCachedKey> pMasterCredential(OTCachedKey::It());

        if (pMasterCredential) {
            OTPassword master_password;
            const bool bGotMasterPW = pMasterCredential->GetMasterPassword(
                pMasterCredential,
                master_password,
                (nullptr == pReason) ? "" : pReason->c_str());
            String strNewKeyOutput;

            if (bGotMasterPW &&
                OTSymmetricKey::CreateNewKey(
                    strNewKeyOutput, nullptr, &master_password)) {
                std::shared_ptr<OTSymmetricKey> pNewExtraKey(
                    new OTSymmetricKey);

                if (pNewExtraKey &&
                    pNewExtraKey->SerializeFrom(strNewKeyOutput) &&
                    addExtraKey(str_KeyID, pNewExtraKey)) {

                    pExtraKey = pNewExtraKey;

                    SaveWallet();
                }
            }  // if (bGotMasterPW)
        }      // if (pMasterCredential)
    }

    // Then:
    //
    if (pExtraKey) return pExtraKey;  // <======== SUCCESS.

    return std::shared_ptr<OTSymmetricKey>();
}

// The "extra" symmetric keys in the wallet are all, like the Nyms, encrypted
// to the wallet's master key. So whenever the wallet's master key is changed,
// this method needs to be called as well, to update those extra symmetric keys
// to the new master key. (Otherwise they'll stop working.)
//
bool OTWallet::ChangePassphrasesOnExtraKeys(
    const OTPassword& oldPassphrase,
    const OTPassword& newPassphrase)
{
    // First we copy all the keys over to a new map, since we aren't going
    // to copy the changed ones back to the actual map unless EVERYTHING
    // succeeds.
    //
    mapOfSymmetricKeys mapChanged;

    for (auto& it : m_mapExtraKeys) {
        const std::string str_id = it.first;
        std::shared_ptr<OTSymmetricKey> pOldKey = it.second;

        Data thePayload;

        if (pOldKey && pOldKey->SerializeTo(thePayload)) {
            std::shared_ptr<OTSymmetricKey> pNewKey(new OTSymmetricKey);

            if (pNewKey && pNewKey->SerializeFrom(thePayload))
                mapChanged.insert(
                    std::pair<std::string, std::shared_ptr<OTSymmetricKey>>(
                        str_id, pNewKey));
            else
                return false;
        } else
            return false;
    }

    // We're still here? Must have been a success so far.
    // Next we'll loop through mapChanged, and change the passphrase
    // on each key in there. If they all succeed, we'll clear the old
    // map and copy mapChanged into it.
    //
    for (auto& it : mapChanged) {
        std::shared_ptr<OTSymmetricKey> pNewKey = it.second;

        if (pNewKey) {
            if (!pNewKey->ChangePassphrase(oldPassphrase, newPassphrase))
                return false;
        } else
            return false;
    }

    // Still here? Must have been successful changing the passphrases
    // on all the various extra symmetric keys. So let's clear the main
    // map and copy the changed map into it.
    //
    m_mapExtraKeys.clear();
    m_mapExtraKeys = mapChanged;

    return true;
}

bool OTWallet::Encrypt_ByKeyID(
    const std::string& key_id,
    const String& strPlaintext,
    String& strOutput,
    const String* pstrDisplay,
    bool bBookends)
{
    if (key_id.empty() || !strPlaintext.Exists()) return false;

    std::string str_Reason((nullptr != pstrDisplay) ? pstrDisplay->Get() : "");

    std::shared_ptr<OTSymmetricKey> pKey =
        OTWallet::getOrCreateExtraKey(key_id, &str_Reason);

    if (pKey) {
        std::shared_ptr<OTCachedKey> pMasterCredential(OTCachedKey::It());

        if (pMasterCredential) {
            OTPassword master_password;

            if (pMasterCredential->GetMasterPassword(
                    pMasterCredential, master_password))
                return OTSymmetricKey::Encrypt(
                    *pKey,
                    strPlaintext,
                    strOutput,
                    pstrDisplay,
                    bBookends,
                    &master_password);
        }
    }
    return false;
}
bool OTWallet::Decrypt_ByKeyID(
    const std::string& key_id,
    const String& strCiphertext,
    String& strOutput,
    const String* pstrDisplay)
{
    if (key_id.empty() || !strCiphertext.Exists()) return false;

    std::shared_ptr<OTSymmetricKey> pKey = OTWallet::getExtraKey(key_id);

    if (pKey) {
        std::shared_ptr<OTCachedKey> pMasterCredential(OTCachedKey::It());

        if (pMasterCredential) {
            OTPassword master_password;

            if (pMasterCredential->GetMasterPassword(
                    pMasterCredential, master_password))
                return OTSymmetricKey::Decrypt(
                    *pKey,
                    strCiphertext,
                    strOutput,
                    pstrDisplay,
                    &master_password);
        }
    }
    return false;
}

std::shared_ptr<OTSymmetricKey> OTWallet::getExtraKey(
    const std::string& str_id) const
{
    if (str_id.empty()) return std::shared_ptr<OTSymmetricKey>();

    auto it = m_mapExtraKeys.find(str_id);

    if (it != m_mapExtraKeys.end())  // It's already there (can't add it.)
    {
        std::shared_ptr<OTSymmetricKey> pKey = it->second;

        return pKey;
    }

    return std::shared_ptr<OTSymmetricKey>();
}

bool OTWallet::addExtraKey(
    const std::string& str_id,
    std::shared_ptr<OTSymmetricKey> pKey)
{
    if (str_id.empty() || !pKey) return false;

    auto it = m_mapExtraKeys.find(str_id);

    if (it != m_mapExtraKeys.end())  // It's already there (can't add it.)
        return false;

    m_mapExtraKeys.insert(
        std::pair<std::string, std::shared_ptr<OTSymmetricKey>>(str_id, pKey));

    return true;
}

// Pass in the name only, NOT the full path.
// If you pass nullptr, it remembers full path from last time.
// (Better to do that.)
//
bool OTWallet::SaveWallet(const char* szFilename)
{
    if (nullptr != szFilename) m_strFilename.Set(szFilename);

    if (!m_strFilename.Exists()) {
        otErr << __FUNCTION__ << ": Filename Dosn't Exist!\n";
        OT_FAIL;
    }

    bool bSuccess = false;
    String strContract;

    if (SaveContract(strContract)) {

        // Try to save the wallet to local storage.
        //
        String strFinal;
        OTASCIIArmor ascTemp(strContract);

        if (false ==
            ascTemp.WriteArmoredString(strFinal, "WALLET"))  // todo hardcoding.
        {
            otErr << "OTWallet::SaveWallet: Error saving wallet (failed "
                     "writing armored string):\n"
                  << m_strDataFolder << Log::PathSeparator() << m_strFilename
                  << "\n";
            return false;
        }

        // Wallet file is the only one in data_folder (".") and not a subfolder
        // of that.
        bSuccess = OTDB::StorePlainString(
            strFinal.Get(),
            ".",
            m_strFilename.Get());  // <==== Store Plain String
    }

    return bSuccess;
}

/*

<wallet name="" version="2.0">

<cachedKey>
CkwAAQCAAAD//wAAAAhVRpwTzc+1NAAAABCKe14aROG8v/ite3un3bBCAAAAINyw
HXTM/x449Al2z8zBHBTRF77jhHkYLj8MIgqrJ2Ep
</cachedKey>

</wallet>

 */
bool OTWallet::LoadWallet(const char* szFilename)
{
    OT_ASSERT_MSG(
        m_strFilename.Exists() || (nullptr != szFilename),
        "OTWallet::LoadWallet: nullptr filename.\n");

    Release();

    // The directory is "." because unlike every other OT file, the wallet file
    // doesn't go into a subdirectory, but it goes into the main data_folder
    // itself.
    // Every other file, however, needs to specify its folder AND filename (and
    // both
    // of those will be appended to the local path to form the complete file
    // path.)
    //
    if (!m_strFilename.Exists())  // If it's not already set, then set it.
        m_strFilename.Set(
            szFilename);  // (We know nullptr wasn't passed in, in this case.)

    if (nullptr ==
        szFilename)  // If nullptr was passed in, then set the pointer to
                     // existing string.
        szFilename = m_strFilename.Get();  // (We know existing string is there,
                                           // in this case.)

    if (!OTDB::Exists(".", szFilename)) {
        otErr << __FUNCTION__ << ": Wallet file does not exist: " << szFilename
              << ". Creating...\n";

        const char* szContents = "<wallet name=\"\" version=\"1.0\">\n"
                                 "\n"
                                 "</wallet>\n";

        if (!OTDB::StorePlainString(szContents, ".", szFilename)) {
            otErr << __FUNCTION__
                  << ": Error: Unable to create blank wallet file.\n";
            OT_FAIL;
        }
    }

    String strFileContents(OTDB::QueryPlainString(".", szFilename));  // <===
                                                                      // LOADING
                                                                      // FROM
                                                                      // DATA
                                                                      // STORE.

    if (!strFileContents.Exists()) {
        otErr << __FUNCTION__ << ": Error reading wallet file: " << szFilename
              << "\n";
        return false;
    }

    bool bNeedToSaveAgain = false;

    {
        OTStringXML xmlFileContents(strFileContents);

        if (!xmlFileContents.DecodeIfArmored()) {
            otErr << __FUNCTION__
                  << ": Input string apparently was encoded and then failed "
                     "decoding. Filename: "
                  << szFilename << " \n"
                                   "Contents: \n"
                  << strFileContents << "\n";
            return false;
        }

        irr::io::IrrXMLReader* xml =
            irr::io::createIrrXMLReader(xmlFileContents);

        // parse the file until end reached
        while (xml && xml->read()) {
            // strings for storing the data that we want to read out of the file
            String NymName;
            String NymID;

            String AssetName;
            String InstrumentDefinitionID;

            String ServerName;
            String NotaryID;

            String AcctName;
            String AcctID;

            const String strNodeName(xml->getNodeName());

            switch (xml->getNodeType()) {
                case irr::io::EXN_NONE:
                case irr::io::EXN_TEXT:
                case irr::io::EXN_COMMENT:
                case irr::io::EXN_ELEMENT_END:
                case irr::io::EXN_CDATA:
                    // in this xml file, the only text which occurs is the
                    // messageText
                    // messageText = xml->getNodeData();
                    break;
                case irr::io::EXN_ELEMENT: {
                    if (strNodeName.Compare("wallet")) {
                        OTASCIIArmor ascWalletName =
                            xml->getAttributeValue("name");

                        if (ascWalletName.Exists())
                            ascWalletName.GetString(
                                m_strName,
                                false);  // linebreaks == false

                        //                      m_strName            =
                        // xml->getAttributeValue("name");
                        //                      OTLog::OTPath        =
                        // xml->getAttributeValue("path");
                        m_strVersion = xml->getAttributeValue("version");

                        otWarn << "\nLoading wallet: " << m_strName
                               << ", version: " << m_strVersion << "\n";
                    } else if (strNodeName.Compare("cachedKey")) {
                        OTASCIIArmor ascCachedKey;

                        if (Contract::LoadEncodedTextField(xml, ascCachedKey)) {
                            // We successfully loaded the cachedKey from file,
                            // so
                            // let's SET it as the cached key globally...
                            //
                            OTCachedKey::It()->SetCachedKey(ascCachedKey);

                            if (!OTCachedKey::It()->HasHashCheck()) {
                                OTPassword tempPassword;
                                tempPassword.zeroMemory();

                                std::shared_ptr<OTCachedKey> sharedPtr(
                                    OTCachedKey::It());

                                bNeedToSaveAgain = sharedPtr->GetMasterPassword(
                                    sharedPtr,
                                    tempPassword,
                                    "We do not have a check hash yet for this "
                                    "password, please enter your password",
                                    true);
                            }
                        }

                        otWarn << "Loading cachedKey:\n"
                               << ascCachedKey << "\n";
                    } else if (strNodeName.Compare("nymUsingCachedKey")) {
                        NymID = xml->getAttributeValue(
                            "id");  // message digest from
                                    // hash of x.509 cert
                                    // or public key.

                        otWarn << "NymID using Cached Key: " << NymID << "\n";
                        if (!NymID.Exists()) {
                            otErr << __FUNCTION__
                                  << ": NymID using Cached Key was "
                                     "empty when loading wallet!\n";
                            OT_FAIL;
                        }

                        const Identifier theNymID(NymID);

                        m_setNymsOnCachedKey.insert(theNymID);
                    } else if (strNodeName.Compare("symmetricKey")) {
                        String strKeyID;
                        OTASCIIArmor ascKeyID = xml->getAttributeValue("id");
                        OTASCIIArmor ascSymmetricKey;

                        if (!ascKeyID.Exists() ||
                            !ascKeyID.GetString(strKeyID, false))  // linebreaks
                                                                   // ==
                            // false (true by
                            // default.)
                            otErr << __FUNCTION__ << ": Failed loading "
                                                     "symmetricKey ID (it was "
                                                     "blank.)\n";

                        else if (
                            Contract::LoadEncodedTextField(
                                xml, ascSymmetricKey)) {
                            std::shared_ptr<OTSymmetricKey> pKey(
                                new OTSymmetricKey);

                            if (!pKey || !pKey->SerializeFrom(ascSymmetricKey))
                                otErr
                                    << __FUNCTION__
                                    << ": Failed serializing symmetricKey from "
                                       "string (id: "
                                    << strKeyID << ")\n";
                            else {
                                const std::string str_id(strKeyID.Get());

                                if (!addExtraKey(str_id, pKey))
                                    otErr << __FUNCTION__
                                          << ": Failed adding serialized "
                                             "symmetricKey to wallet (id: "
                                          << strKeyID << ")\n";
                            }
                        }
                    } else if (strNodeName.Compare("pseudonym")) {
                        OTASCIIArmor ascNymName =
                            xml->getAttributeValue("name");
                        if (ascNymName.Exists())
                            ascNymName.GetString(
                                NymName,
                                false);  // linebreaks == false

                        NymID =
                            xml->getAttributeValue("nymID");  // message digest
                                                              // from hash of
                                                              // x.509 cert or
                                                              // public key.

                        otInfo << "\n\n** Pseudonym ** (wallet listing): "
                               << NymName << "\nID: " << NymID << "\n";
                        if (!NymID.Exists()) {
                            otErr << __FUNCTION__ << ": NymID doesn't Exist!\n";
                            OT_FAIL;
                        }

                        const Identifier theNymID(NymID);

                        // What's going on here? We need to see if the MASTER
                        // KEY
                        // exists at this point. If it's GENERATED.
                        // If not, that means the Nyms are all still encrypted
                        // to
                        // their own passphrases, not to the master key.
                        // In which case we need to generate one and re-encrypt
                        // each
                        // private key to that new master key.
                        //
                        //                  bool
                        //                  OTWallet::IsNymOnCachedKey(const
                        // OTIdentifier& needle) const // needle and haystack.

                        const bool bIsOldStyleNym =
                            (false == IsNymOnCachedKey(theNymID));

                        if (bIsOldStyleNym && !(OTCachedKey::It()->isPaused()))
                        //                  if (m_strVersion.Compare("1.0")) //
                        // This means this Nym has not been converted yet to
                        // master password.
                        {
                            OTCachedKey::It()->Pause();
                        }

                        Nym* pNym =
                            Nym::LoadPrivateNym(theNymID, false, &NymName);

                        if (nullptr == pNym)
                            otOut << __FUNCTION__ << ": Failed loading Nym ("
                                  << NymName << ") with ID: " << NymID << "\n";
                        else
                            AddPrivateNym(
                                *pNym);  // Nym loaded. Insert to wallet's
                                         // list of Nyms.

                        if (bIsOldStyleNym && OTCachedKey::It()->isPaused()) {
                            OTCachedKey::It()->Unpause();
                        }
                        // (Here we set it back again, so any new-style Nyms
                        // will
                        // still load properly, when they come around.)
                    }

                    else if (strNodeName.Compare("account")) {
                        OTASCIIArmor ascAcctName =
                            xml->getAttributeValue("name");

                        if (ascAcctName.Exists())
                            ascAcctName.GetString(
                                AcctName,
                                false);  // linebreaks == false

                        AcctID = xml->getAttributeValue("accountID");
                        NotaryID = xml->getAttributeValue("notaryID");

                        otInfo << "\n------------------------------------------"
                                  "----"
                                  "----------------------------\n"
                                  "****Account**** (wallet listing)\n"
                                  " Account Name: "
                               << AcctName << "\n   Account ID: " << AcctID
                               << "\n    Notary ID: " << NotaryID << "\n";

                        const Identifier ACCOUNT_ID(AcctID),
                            NOTARY_ID(NotaryID);

                        Account* pAccount =
                            Account::LoadExistingAccount(ACCOUNT_ID, NOTARY_ID);

                        if (pAccount) {
                            pAccount->SetName(AcctName);
                            AddAccount(*pAccount);
                        } else {
                            otErr
                                << __FUNCTION__
                                << ": Error loading existing Asset Account.\n";
                        }
                    }
                    // This tag is no longer saved in the wallet, but it is
                    // still parsed to allow conversion of existing wallets.
                    // From now on, the BIP39 class tracks the last used index
                    // individually for each seed rather than globally in the
                    // wallet (which assumed only one seed existed).
                    else if (strNodeName.Compare("hd")) {
                        std::uint32_t index =
                            String::StringToUint(
                                xml->getAttributeValue("index"));
                        // An empty string will load the default seed
                        std::string seed = "";
                        OT::App().Crypto().BIP39().UpdateIndex(seed, index);
                    } else {
                        // unknown element type
                        otErr << __FUNCTION__ << ": unknown element type: "
                              << xml->getNodeName() << "\n";
                    }
                } break;
                default:
                    otLog5 << __FUNCTION__
                           << ": Unknown XML type: " << xml->getNodeName()
                           << "\n";
                    break;
            }
        }  // while xml->read()

        // After we've loaded all the old-format Nyms that don't use the master
        // key,
        // NOW we can go through and convert them all, now that they're all
        // loaded.

        for (auto& it : m_mapPrivateNyms) {
            auto& pNym = it.second;

            if (pNym &&
                pNym->hasCapability(NymCapability::SIGN_MESSAGE) &&
                ConvertNymToCachedKey(*pNym))  // Internally this is smart
                                               // enough to only convert
                                               // the unconverted.
                bNeedToSaveAgain = true;
        }

        //
        // delete the xml parser after usage
        if (xml) delete xml;
    }

    // In case we converted any of the Nyms to the new "master key" encryption.
    if (bNeedToSaveAgain) SaveWallet(szFilename);

    return true;
}

bool OTWallet::ConvertNymToCachedKey(Nym& theNym)
{
    // If he's not ALREADY on the master key...
    //
    if (!IsNymOnCachedKey(theNym.GetConstID())) {

        // The Nym has credentials.
        //
        OT_ASSERT(theNym.GetMasterCredentialCount() > 0);

        String strNymID;
        theNym.GetIdentifier(strNymID);

        const bool bConverted = theNym.WriteCredentials();

        if (bConverted) {
            m_setNymsOnCachedKey.insert(theNym.GetConstID());
        } else {
            otErr << __FUNCTION__ << ": Failure trying to store "
                  << "private credential list for Nym: " << strNymID
                  << std::endl;

            return false;
        }

        return bConverted;
    }  // This block only occurs if Nym is not ALREADY on the wallet's list of
       // Nym using the wallet's cached master key.

    return false;
}

//     setOfIdentifiers m_setNymsOnCachedKey;  // All the Nyms that use the
// Master key are listed here (makes it easy to see which ones are converted
// already.)
// Todo: serialize?
//
bool OTWallet::IsNymOnCachedKey(const Identifier& needle) const  // needle and
                                                                 // haystack.
{
    for (const auto& it : m_setNymsOnCachedKey) {
        if (needle == it) return true;
    }
    return false;
}

}  // namespace opentxs
