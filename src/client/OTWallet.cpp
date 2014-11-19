/************************************************************
 *
 *  OTWallet.cpp
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

#include <opentxs/client/OTWallet.hpp>

#include <opentxs/cash/Purse.hpp>

#include <opentxs/core/Account.hpp>
#include <opentxs/core/AssetContract.hpp>
#include <opentxs/core/crypto/OTCachedKey.hpp>
#include <opentxs/core/util/OTDataFolder.hpp>
#include <opentxs/core/util/OTFolders.hpp>
#include <opentxs/core/OTLog.hpp>
#include <opentxs/core/crypto/OTPassword.hpp>
#include <opentxs/core/crypto/OTPasswordData.hpp>
#include <opentxs/core/OTPseudonym.hpp>
#include <opentxs/core/OTServerContract.hpp>
#include <opentxs/core/OTStorage.hpp>
#include <opentxs/core/crypto/OTSymmetricKey.hpp>

#include <irrxml/irrXML.hpp>

namespace opentxs
{

OTWallet::OTWallet()
    : m_strDataFolder(OTDataFolder::Get())
{
    m_pWithdrawalPurse = nullptr;
}

OTWallet::~OTWallet()
{
    Release();
}

void OTWallet::Release()
{
    // 1) Go through the map of Nyms and delete them. (They were dynamically
    // allocated.)
    while (!m_mapNyms.empty()) {
        OTPseudonym* pNym = m_mapNyms.begin()->second;

        OT_ASSERT(nullptr != pNym);

        delete pNym;
        pNym = nullptr;

        m_mapNyms.erase(m_mapNyms.begin());
    }

    // 2) Go through the map of Contracts and delete them. (They were
    // dynamically allocated.)
    while (!m_mapContracts.empty()) {
        AssetContract* pContract = m_mapContracts.begin()->second;

        OT_ASSERT(nullptr != pContract);

        delete pContract;
        pContract = nullptr;

        m_mapContracts.erase(m_mapContracts.begin());
    }

    // 3) Go through the map of Servers and delete them. (They were dynamically
    // allocated.)
    while (!m_mapServers.empty()) {
        OTServerContract* pContract = m_mapServers.begin()->second;

        OT_ASSERT(nullptr != pContract);

        delete pContract;
        pContract = nullptr;

        m_mapServers.erase(m_mapServers.begin());
    }

    // 4) Go through the map of Accounts and delete them. (They were dynamically
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
} // TODO WARNING: If this data is lost before the transaction is completed,
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

        if (GetNym(0, // index 0
                   NYM_ID, NYM_NAME)) {
            OTPseudonym* pNym = GetNymByID(NYM_ID);

            if (nullptr != pNym) {
                theContract.SignContract(*pNym);
                return true;
            }
        }
    }

    return false;
}

// The wallet presumably has multiple Nyms listed within.
// I should be able to pass in a Nym ID and, if the Nym is there,
// the wallet returns a pointer to that nym.
OTPseudonym* OTWallet::GetNymByID(const Identifier& NYM_ID)
{
    for (auto& it : m_mapNyms) {
        OTPseudonym* pNym = it.second;
        OT_ASSERT_MSG((nullptr != pNym),
                      "nullptr pseudonym pointer in OTWallet::GetNymByID.");

        Identifier id_CurrentNym;
        pNym->GetIdentifier(id_CurrentNym);

        if (id_CurrentNym == NYM_ID) return pNym;
    }

    return nullptr;
}

OTPseudonym* OTWallet::GetNymByIDPartialMatch(std::string PARTIAL_ID) // works
                                                                      // with
                                                                      // name as
                                                                      // well.
{
    for (auto& it : m_mapNyms) {
        OTPseudonym* pNym = it.second;
        OT_ASSERT_MSG(
            (nullptr != pNym),
            "nullptr pseudonym pointer in OTWallet::GetNymByIDPartialMatch.");

        String strTemp;
        pNym->GetIdentifier(strTemp);

        std::string strIdentifier = strTemp.Get();

        if (strIdentifier.compare(0, PARTIAL_ID.length(), PARTIAL_ID) == 0)
            return pNym;
    }

    // OK, let's try it by the name, then...
    //
    for (auto& it : m_mapNyms) {
        OTPseudonym* pNym = it.second;
        OT_ASSERT_MSG(
            (nullptr != pNym),
            "nullptr pseudonym pointer in OTWallet::GetNymByIDPartialMatch.");

        String strNymName;
        strNymName.Set(pNym->GetNymName());
        std::string str_NymName = strNymName.Get();

        if (str_NymName.compare(0, PARTIAL_ID.length(), PARTIAL_ID) == 0)
            return pNym;
    }

    return nullptr;
}

// used by high-level wrapper.
int32_t OTWallet::GetNymCount()
{
    return static_cast<int32_t>(m_mapNyms.size());
}

int32_t OTWallet::GetServerCount()
{
    return static_cast<int32_t>(m_mapServers.size());
}

int32_t OTWallet::GetAssetTypeCount()
{
    return static_cast<int32_t>(m_mapContracts.size());
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

        for (auto& it : m_mapNyms) {
            OTPseudonym* pNym = it.second;
            OT_ASSERT(nullptr != pNym);

            iCurrentIndex++; // On first iteration, this becomes 0 here. (For 0
                             // index.) Increments thereafter.

            if (iIndex == iCurrentIndex) {
                pNym->GetIdentifier(NYM_ID);
                NYM_NAME.Set(pNym->GetNymName());
                return true;
            }
        }
    }

    return false;
}

// used by high-level wrapper.
bool OTWallet::GetServer(int32_t iIndex, Identifier& THE_ID, String& THE_NAME)
{
    // if iIndex is within proper bounds (0 through count minus 1)
    if (iIndex < GetServerCount() && iIndex >= 0) {
        int32_t iCurrentIndex = (-1);

        for (auto& it : m_mapServers) {
            OTServerContract* pServer = it.second;
            OT_ASSERT(nullptr != pServer);

            iCurrentIndex++; // On first iteration, this becomes 0 here. (For 0
                             // index.) Increments thereafter.

            if (iIndex == iCurrentIndex) // if not null
            {
                pServer->GetIdentifier(THE_ID);
                pServer->GetName(THE_NAME);
                return true;
            }
        }
    }

    return false;
}

// used by high-level wrapper.
bool OTWallet::GetAssetType(int32_t iIndex, Identifier& THE_ID,
                            String& THE_NAME)
{
    // if iIndex is within proper bounds (0 through count minus 1)
    if (iIndex < GetAssetTypeCount() && iIndex >= 0) {
        int32_t iCurrentIndex = (-1);

        for (auto& it : m_mapContracts) {
            AssetContract* pAssetType = it.second;
            OT_ASSERT(nullptr != pAssetType);

            iCurrentIndex++; // On first iteration, this becomes 0 here. (For 0
                             // index.) Increments thereafter.

            if (iIndex == iCurrentIndex) // if not null
            {
                pAssetType->GetIdentifier(THE_ID);
                pAssetType->GetName(THE_NAME);
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

            iCurrentIndex++; // On first iteration, this becomes 0 here. (For 0
                             // index.) Increments thereafter.

            if (iIndex == iCurrentIndex) // if not null
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

    for (auto& it : m_mapNyms) {
        OTPseudonym* pNym = it.second;
        OT_ASSERT_MSG((nullptr != pNym), "nullptr pseudonym pointer in "
                                         "OTWallet::m_mapNyms, "
                                         "OTWallet::DisplayStatistics.");

        pNym->DisplayStatistics(strOutput);
    }

    strOutput.Concatenate(
        "\n-------------------------------------------------\n");
    strOutput.Concatenate("ASSET CONTRACTS:\n\n");

    for (auto& it : m_mapContracts) {
        Contract* pContract = it.second;
        OT_ASSERT_MSG(nullptr != pContract, "nullptr contract pointer in "
                                            "OTWallet::m_mapContracts, "
                                            "OTWallet::DisplayStatistics");

        pContract->DisplayStatistics(strOutput);
    }

    strOutput.Concatenate(
        "-------------------------------------------------\n");
    strOutput.Concatenate("SERVER CONTRACTS:\n\n");

    for (auto& it : m_mapServers) {
        Contract* pServer = it.second;
        OT_ASSERT_MSG(nullptr != pServer, "nullptr server pointer in "
                                          "OTWallet::m_mapServers, "
                                          "OTWallet::DisplayStatistics");

        pServer->DisplayStatistics(strOutput);
    }

    strOutput.Concatenate(
        "-------------------------------------------------\n");
    strOutput.Concatenate("ACCOUNTS:\n\n");

    for (auto& it : m_mapAccounts) {
        Account* pAccount = it.second;
        OT_ASSERT_MSG(nullptr != pAccount, "nullptr account pointer in "
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
void OTWallet::AddNym(const OTPseudonym& theNym)
{
    const Identifier NYM_ID(theNym);
    Identifier aNymID;

    String strName;

    for (auto it(m_mapNyms.begin()); it != m_mapNyms.end(); ++it) {
        OTPseudonym* pNym = it->second;
        OT_ASSERT(nullptr != pNym);

        pNym->GetIdentifier(aNymID);

        if (aNymID == NYM_ID) {
            String strTemp(pNym->GetNymName());
            strName = strTemp; // todo optimize. currently am fixing "blank nym
                               // name" bug.

            m_mapNyms.erase(it);

            // Don't delete it if they are physically the same object.
            // (Versus each being separate copies of the same object.)
            //
            if (&theNym != pNym) delete pNym;
            pNym = nullptr;

            break;
        }
    }

    const String strNymID(NYM_ID);
    m_mapNyms[strNymID.Get()] = const_cast<OTPseudonym*>(&theNym);

    if (strName.Exists())
        (const_cast<OTPseudonym&>(theNym)).SetNymName(strName);
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

Account* OTWallet::GetAccountPartialMatch(std::string PARTIAL_ID) // works
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

Account* OTWallet::GetIssuerAccount(const Identifier& theAssetTypeID)
{
    // loop through the accounts and find one with a specific asset type ID.
    // (And with the issuer type set.)
    //
    for (auto& it : m_mapAccounts) {
        Account* pIssuerAccount = it.second;
        OT_ASSERT(nullptr != pIssuerAccount);

        if ((pIssuerAccount->GetAssetTypeID() == theAssetTypeID) &&
            (pIssuerAccount->IsIssuer()))
            return pIssuerAccount;
    }

    return nullptr;
}

// Pass in the Server ID and get the pointer back.
OTServerContract* OTWallet::GetServerContract(const Identifier& SERVER_ID)
{
    for (auto& it : m_mapServers) {
        Contract* pServer = it.second;
        OT_ASSERT_MSG((nullptr != pServer), "nullptr server pointer in "
                                            "OTWallet::m_mapServers, "
                                            "OTWallet::GetServerContract");

        Identifier id_CurrentContract;
        pServer->GetIdentifier(id_CurrentContract);

        if (id_CurrentContract == SERVER_ID)
            return dynamic_cast<OTServerContract*>(pServer);
    }

    return nullptr;
}

OTServerContract* OTWallet::GetServerContractPartialMatch(
    std::string PARTIAL_ID)
{
    for (auto& it : m_mapServers) {
        Contract* pServer = it.second;
        OT_ASSERT_MSG((nullptr != pServer), "nullptr server pointer in "
                                            "OTWallet::m_mapServers, "
                                            "OTWallet::GetServerContract");

        Identifier id_CurrentContract;
        pServer->GetIdentifier(id_CurrentContract);

        String strTemp(id_CurrentContract);
        std::string strIdentifier = strTemp.Get();

        if (strIdentifier.compare(0, PARTIAL_ID.length(), PARTIAL_ID) == 0)
            return dynamic_cast<OTServerContract*>(pServer);
    }

    // Okay, let's try it by the name, then.
    //
    for (auto& it : m_mapServers) {
        Contract* pServer = it.second;
        OT_ASSERT_MSG((nullptr != pServer), "nullptr server pointer in "
                                            "OTWallet::m_mapServers, "
                                            "OTWallet::GetServerContract");

        String strName;
        pServer->GetName(strName);
        std::string str_Name = strName.Get();

        if (str_Name.compare(0, PARTIAL_ID.length(), PARTIAL_ID) == 0)
            return dynamic_cast<OTServerContract*>(pServer);
    }

    return nullptr;
}

// The wallet "owns" theContract and will handle cleaning it up.
// So make SURE you allocate it on the heap.
void OTWallet::AddServerContract(const OTServerContract& theContract)
{
    Identifier CONTRACT_ID(theContract);
    String STR_CONTRACT_ID(CONTRACT_ID);

    OTServerContract* pContract = GetServerContract(CONTRACT_ID);

    if (pContract) {
        otErr << "Error: Attempt to add Server Contract but it is already in "
                 "the wallet.\n";

        delete &theContract; // I have to do this, since the return value is
                             // void, the caller MUST assume I took ownership.
    }
    else {
        m_mapServers[STR_CONTRACT_ID.Get()] =
            &(const_cast<OTServerContract&>(theContract));

        otInfo << "Saving server contract to disk...\n";
        (const_cast<OTServerContract&>(theContract)).SaveToContractFolder();

        SaveWallet();
    }
}

// The wallet "owns" theContract and will handle cleaning it up.
// So make SURE you allocate it on the heap.
void OTWallet::AddAssetContract(const AssetContract& theContract)
{
    Identifier CONTRACT_ID(theContract);
    String STR_CONTRACT_ID(CONTRACT_ID);

    AssetContract* pContract = GetAssetContract(CONTRACT_ID);

    if (pContract) {
        otErr << "Error: Attempt to add Asset Contract but it is already in "
                 "the wallet.\n";

        delete &theContract; // I have to do this, since the return value is
                             // void, the caller MUST assume I took ownership.
    }
    else {
        m_mapContracts[STR_CONTRACT_ID.Get()] =
            &(const_cast<AssetContract&>(theContract));

        otInfo << "Saving asset contract to disk...\n";
        (const_cast<AssetContract&>(theContract)).SaveToContractFolder();

        SaveWallet();
    }
}

bool OTWallet::VerifyAssetAccount(const OTPseudonym& theNym, Account& theAcct,
                                  const Identifier& SERVER_ID,
                                  const String& strAcctID,
                                  const char* szFuncName)
{
    const char* szFunc =
        (nullptr != szFuncName) ? szFuncName : "OTWallet::VerifyAssetAccount";

    if (SERVER_ID != theAcct.GetRealServerID()) {
        const String s1(SERVER_ID), s2(theAcct.GetRealServerID());
        otOut << "OTWallet::VerifyAssetAccount " << szFunc
              << ": Server ID passed in (" << s1 << ") didn't match the one "
                                                    "on the account (" << s2
              << "). Acct ID: " << strAcctID << "\n";
        return false;
    }

    const Identifier theNymID(theNym);
    const String strNymID(theNymID);

    if (!theAcct.VerifyOwner(theNym)) // Verifies Ownership.
    {
        otOut << "OTWallet::VerifyAssetAccount " << szFunc
              << ": Nym (ID: " << strNymID
              << ") is not the owner of the account: " << strAcctID << "\n";
        return false;
    }

    if (false ==
        theAcct.VerifyAccount(theNym)) // Verifies ContractID and Signature.
    {
        otOut << "OTWallet::VerifyAssetAccount " << szFunc
              << ": Account signature or AccountID fails to verify. "
                 "NymID: " << strNymID << "  AcctID: " << strAcctID << "\n";
        return false;
    }
    // By this point, I know that everything checks out. Signature and Account
    // ID,
    // Nym is owner, etc.

    return true;
}

// No need to cleanup the account returned, it's owned by the wallet.
//
Account* OTWallet::GetOrLoadAccount(const OTPseudonym& theNym,
                                    const Identifier& ACCT_ID,
                                    const Identifier& SERVER_ID,
                                    const char* szFuncName)
{
    const char* szFunc =
        (nullptr != szFuncName) ? szFuncName : "OTWallet::GetOrLoadAccount";

    const String strAcctID(ACCT_ID);

    Account* pAccount = GetAccount(ACCT_ID);

    if (nullptr ==
        pAccount) // It wasn't there already, so we'll have to load it...
    {
        otOut << "OTWallet::GetOrLoadAccount " << szFunc
              << ": There's no asset account in the wallet with that ID ("
              << strAcctID << "). "
                              "Attempting to load it from storage...\n";
        pAccount = LoadAccount(theNym, ACCT_ID, SERVER_ID, szFuncName);
    } // pAccount == nullptr.

    // It either was already there, or it loaded successfully...
    //
    if (nullptr == pAccount) // pAccount EXISTS...
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
Account* OTWallet::LoadAccount(const OTPseudonym& theNym,
                               const Identifier& ACCT_ID,
                               const Identifier& SERVER_ID,
                               const char* szFuncName)
{
    const char* szFunc =
        (nullptr != szFuncName) ? szFuncName : "OTWallet::LoadAccount";

    const String strAcctID(ACCT_ID);
    Account* pAccount = Account::LoadExistingAccount(ACCT_ID, SERVER_ID);

    // It loaded successfully...
    //
    if (nullptr != pAccount) // pAccount EXISTS...
    {
        bool bVerified =
            VerifyAssetAccount(theNym, *pAccount, SERVER_ID, strAcctID, szFunc);

        if (!bVerified) {
            delete pAccount;
            pAccount = nullptr;
            return nullptr; // No need to log, since VerifyAssetAccount()
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

    }
    else {
        otErr << "OTWallet::LoadAccount " << szFunc
              << ": Failed loading Asset Account: " << strAcctID << "\n";
        return nullptr;
    }

    return pAccount;
}

// This function only tries to load as a public Nym.
// No need to cleanup, since it adds the Nym to the wallet.
//
OTPseudonym* OTWallet::GetOrLoadPublicNym(const Identifier& NYM_ID,
                                          const char* szFuncName)
{
    const String strNymID(NYM_ID);
    const char* szFunc = "OTWallet::GetOrLoadPublicNym";

    szFuncName = (szFuncName == nullptr) ? "" : szFuncName;

    OTPseudonym* pNym = GetNymByID(NYM_ID); // <===========

    if (nullptr == pNym) // Wasn't already in the wallet. Try loading it.
    {
        otWarn << szFunc << " " << szFuncName
               << ": There's no Nym already loaded with that ID. "
                  "Attempting to load public key...\n";
        pNym = OTPseudonym::LoadPublicNym(NYM_ID); // <===========
        // It worked!
        if (nullptr !=
            pNym) // LoadPublicNym has plenty of error logging already.
        {
            if (pNym->HasPrivateKey()) // We don't auto-add public Nyms -- only
                                       // private ones.
                AddNym(*pNym);         // <===========
        }
        else
            otOut << szFunc << " " << szFuncName
                  << ": Unable to load public Nym for: " << strNymID << " \n";
    }

    // If pNym exists, yet he doesn't have a public key (weird!)
    // Though we log the error, we still return pNym, since it exists.
    //
    if ((nullptr != pNym) && (false == pNym->HasPublicKey()))
        otErr << szFunc << " " << szFuncName << ": Found nym (" << strNymID
              << "), but he has no public key. "
                 "(Still returning the Nym, since it exists.)\n";
    return pNym;
}

// This function only tries to load as a private Nym.
// No need to cleanup, since it adds the Nym to the wallet.
//
// It is smart enough to Get the Nym from the wallet, and if it
// sees that it's only a public nym (no private key) then it
// reloads it as a private nym at that time.
//
OTPseudonym* OTWallet::GetOrLoadPrivateNym(const Identifier& NYM_ID,
                                           bool bChecking,
                                           const char* szFuncName,
                                           const OTPasswordData* pPWData,
                                           const OTPassword* pImportPassword)
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ":" << szFuncName
              << ": Error: NYM_ID passed in empty, returning null";
        return nullptr;
    }

    const String strNymID(NYM_ID);
    OTPasswordData thePWData(OT_PW_DISPLAY);
    if (nullptr == pPWData) pPWData = &thePWData;

    szFuncName = (szFuncName == nullptr) ? "" : szFuncName;

    // See if it's already there. (Could be the public version
    // though :P Still might have to reload it.)
    //
    OTPseudonym* pNym = GetNymByID(NYM_ID); // <===========

    if (nullptr ==
        pNym) // Wasn't already in the wallet. Let's try loading it...
    {
        otWarn << __FUNCTION__ << " " << szFuncName
               << ": There's no Nym already loaded with that ID. "
                  "Attempting to load private key...\n";
        pNym = OTPseudonym::LoadPrivateNym(NYM_ID, bChecking, nullptr,
                                           szFuncName, // <===========
                                           pPWData, pImportPassword);
        // It worked!
        if (nullptr !=
            pNym)          // LoadPublicNym has plenty of error logging already.
            AddNym(*pNym); // <===========
        else {
            OTLogStream& otLog = bChecking ? otWarn : otOut;
            otLog << __FUNCTION__ << ": " << szFuncName << ": ("
                  << "bChecking"
                  << ": is " << (bChecking ? "true" : "false")
                  << ").  Unable to load Private Nym for: " << strNymID << "\n";
        }
    }

    // If pNym EXISTS, then let's make sure he has a public AND a
    // private key, as he should. (He might be already loaded on the
    // wallet, without his private key, necessitating a reload.)
    //
    if (nullptr != pNym) // pNym definitely NOT nullptr (it exists)...
    {

        // ...yet he doesn't have a public key (Weird!)
        if (!pNym->HasPublicKey())
            otErr << __FUNCTION__ << " " << szFuncName
                  << ": Found nym, but he has no public key: " << strNymID
                  << "\n";

        // ...hmm, he doesn't have a private key. Possible! If the wallet
        // already had
        // my public key loaded (without the private one) from some earlier
        // action.
        //
        if (!pNym->HasPrivateKey()) {
            otWarn
                << __FUNCTION__ << " " << szFuncName
                << ": Found nym in wallet (" << strNymID
                << "), "
                   "but he currently has no private key loaded. Reloading...\n";

            // ASSUMPTION: The Nym is always saved right after some important
            // change,
            // to avoid the risk of losing it. Therefore I don't have to save
            // the
            // current Nym here--I can just remove it from the wallet now, and
            // then
            // reload it (as a private key this time, of course.)
            //

            // Let's save the Name, in case that is already set, so we don't
            // blank it out...
            //
            String strName =
                pNym->GetNymName().Get(); // Get returns "" if string is empty.

            if (RemoveNym(NYM_ID)) {
                pNym = OTPseudonym::LoadPrivateNym(NYM_ID, false, &strName,
                                                   szFuncName, // <===========
                                                   pPWData, pImportPassword);
                // It worked!
                if (nullptr !=
                    pNym) // LoadPrivateNym has plenty of error logging already.
                    AddNym(*pNym); // <===========
                else
                    otOut << __FUNCTION__ << " " << szFuncName
                          << ": Unable to load private Nym for: " << strNymID
                          << " \n";
            }
            else
                otErr << __FUNCTION__ << " " << szFuncName << ": Found nym ("
                      << strNymID << "), but he had no private key. Then tried "
                                     "to remove him from wallet (in order "
                                     "to reload him with private key) and then "
                                     "the removal failed. Sorry.\n";
        }
    }
    return pNym;
}

// This function tries to load as public Nym first, then if it fails,
// it tries the private one next. (So as to avoid unnecessarily asking
// users for their passphrase.) Be sure to use GetOrLoadPublicNym() or
// GetOrLoadPrivateNym() if you want to force it one way or the other.
//
// No need to cleanup, since either function called will add the loaded
// Nym to the wallet, which will take ownership.
//
OTPseudonym* OTWallet::GetOrLoadNym(const Identifier& NYM_ID, bool bChecking,
                                    const char* szFuncName,
                                    const OTPasswordData* pPWData)
{
    OTPseudonym* pNym = GetOrLoadPublicNym(NYM_ID, szFuncName);

    // It tries to load as public Nym first, so as not to force the user to
    // enter his passphrase unnecessarily.
    // However, if this fails, then it tries the private one, just to see
    // if it can be found.
    //
    OTPasswordData thePWData(OT_PW_DISPLAY);

    if (nullptr == pNym)
        pNym = GetOrLoadPrivateNym(NYM_ID, bChecking, szFuncName,
                                   nullptr == pPWData ? &thePWData : pPWData);

    return pNym;
}

// These functions are low-level. They don't check for dependent data before
// deleting,
// and they don't save the wallet after they do.
//
// You have to handle that at a higher level.

// higher level version of this will require a server message, in addition to
// removing from wallet.
bool OTWallet::RemoveNym(const Identifier& theTargetID)
{
    for (auto it(m_mapNyms.begin()); it != m_mapNyms.end(); ++it) {
        OTPseudonym* pNym = it->second;
        OT_ASSERT_MSG((nullptr != pNym),
                      "nullptr pseudonym pointer in OTWallet::RemoveNym.");

        if (pNym->CompareID(theTargetID)) {

            // We have a set of NymIDs for Nyms in the wallet who are using the
            // Master key.
            // So if we're removing the Nym from the wallet, we also remove its
            // ID from that set.
            //
            for (const auto& it_master : m_setNymsOnCachedKey) {
                const Identifier& theNymID = it_master;
                if (theTargetID == theNymID) {
                    m_setNymsOnCachedKey.erase(it_master);
                    break;
                }
            }

            m_mapNyms.erase(it);
            delete pNym;
            return true;
        }
    }
    return false;
}

bool OTWallet::RemoveAssetContract(const Identifier& theTargetID)
{
    // loop through the items that make up this transaction and print them out
    // here, base64-encoded, of course.
    Identifier aContractID;

    for (auto it(m_mapContracts.begin()); it != m_mapContracts.end(); ++it) {
        AssetContract* pContract = it->second;
        OT_ASSERT(nullptr != pContract);

        pContract->GetIdentifier(aContractID);

        if (aContractID == theTargetID) {
            m_mapContracts.erase(it);

            delete pContract;

            return true;
        }
    }

    return false;
}

bool OTWallet::RemoveServerContract(const Identifier& theTargetID)
{
    for (auto it(m_mapServers.begin()); it != m_mapServers.end(); ++it) {
        Contract* pServer = it->second;
        OT_ASSERT_MSG((nullptr != pServer), "nullptr server pointer in "
                                            "OTWallet::m_mapServers, "
                                            "OTWallet::RemoveServerContract");

        Identifier id_CurrentContract;
        pServer->GetIdentifier(id_CurrentContract);

        if (id_CurrentContract == theTargetID) {
            m_mapServers.erase(it);

            OTServerContract* pServerContract =
                static_cast<OTServerContract*>(pServer);
            delete pServerContract;

            return true;
        }
    }

    return false;
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

AssetContract* OTWallet::GetAssetContract(const Identifier& theContractID)
{
    for (auto& it : m_mapContracts) {
        AssetContract* pContract = it.second;
        OT_ASSERT(nullptr != pContract);

        Identifier aContractID;
        pContract->GetIdentifier(aContractID);

        if (aContractID == theContractID) return pContract;
    }

    return nullptr;
}

AssetContract* OTWallet::GetAssetContractPartialMatch(
    std::string PARTIAL_ID) // works with name, too.
{
    for (auto& it : m_mapContracts) {
        AssetContract* pContract = it.second;
        OT_ASSERT(nullptr != pContract);

        Identifier aContractID;
        pContract->GetIdentifier(aContractID);

        String strTemp(aContractID);
        std::string strIdentifier = strTemp.Get();

        if (strIdentifier.compare(0, PARTIAL_ID.length(), PARTIAL_ID) == 0)
            return pContract;
    }

    // Okay, let's try it by the name, then...
    //
    for (auto& it : m_mapContracts) {
        AssetContract* pContract = it.second;
        OT_ASSERT(nullptr != pContract);

        String strName;
        pContract->GetName(strName);
        std::string str_Name = strName.Get();

        if (str_Name.compare(0, PARTIAL_ID.length(), PARTIAL_ID) == 0)
            return pContract;
    }

    return nullptr;
}

bool OTWallet::SaveContract(String& strContract)
{
    OTASCIIArmor ascName;

    if (m_strName.Exists()) // name is in the clear in memory, and base64 in
                            // storage.
    {
        ascName.SetString(m_strName, false); // linebreaks == false
    }

    strContract.Concatenate(
        "<?xml version=\"1.0\"?>\n<wallet name=\"%s\" version=\"%s\">\n\n",
        ascName.Get(),
        OTCachedKey::It()->IsGenerated() ? "2.0" : m_strVersion.Get());

    if (OTCachedKey::It()->IsGenerated()) // If it exists, then serialize it.
    {
        OTASCIIArmor ascMasterContents;

        if (OTCachedKey::It()->SerializeTo(ascMasterContents)) {
            strContract.Concatenate("<cachedKey>\n%s</cachedKey>\n\n",
                                    ascMasterContents.Get());
        }
        else
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

        ascKeyID.SetString(strKeyID,
                           false); // linebreaks=false (true by default.)

        OTASCIIArmor ascKeyContents;

        if (pKey && pKey->SerializeTo(ascKeyContents)) {
            strContract.Concatenate(
                "<symmetricKey id=\"%s\">\n%s</symmetricKey>\n\n",
                ascKeyID.Get(), ascKeyContents.Get());
        }
        else
            otErr << "OTWallet::SaveContract: Failed trying to serialize "
                     "symmetric keys to wallet.\n";
    }

    //
    // We want to save the NymIDs for the Nyms on the master key. I save those
    // before the Nyms themselves, so that they are all loaded up and available
    // in LoadWallet before the Nyms themselves are loaded.
    //
    for (const auto& it : m_setNymsOnCachedKey) {
        const Identifier& theNymID = it;
        String strNymID(theNymID);

        strContract.Concatenate("<nymUsingCachedKey id=\"%s\" />\n\n",
                                strNymID.Get());
    }

    for (auto& it : m_mapNyms) {
        OTPseudonym* pNym = it.second;
        OT_ASSERT_MSG(nullptr != pNym, "nullptr pseudonym pointer in "
                                       "OTWallet::m_mapNyms, "
                                       "OTWallet::SaveContract");

        pNym->SavePseudonymWallet(strContract);
    }

    for (auto& it : m_mapContracts) {
        Contract* pContract = it.second;
        OT_ASSERT_MSG(nullptr != pContract, "nullptr contract pointer in "
                                            "OTWallet::m_mapContracts, "
                                            "OTWallet::SaveContract");

        pContract->SaveContractWallet(strContract);
    }

    for (auto& it : m_mapServers) {
        Contract* pServer = it.second;
        OT_ASSERT_MSG(nullptr != pServer, "nullptr server pointer in "
                                          "OTWallet::m_mapServers, "
                                          "OTWallet::SaveContract");

        pServer->SaveContractWallet(strContract);
    }

    for (auto& it : m_mapAccounts) {
        Contract* pAccount = it.second;
        OT_ASSERT_MSG(nullptr != pAccount, "nullptr account pointer in "
                                           "OTWallet::m_mapAccounts, "
                                           "OTWallet::SaveContract");

        pAccount->SaveContractWallet(strContract);
    }

    strContract.Concatenate("%s", "</wallet>\n");

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
    const std::string& str_KeyID, const std::string* pReason)
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
        std::shared_ptr<OTCachedKey> pMasterKey(OTCachedKey::It());

        if (pMasterKey) {
            OTPassword master_password;
            const bool bGotMasterPW = pMasterKey->GetMasterPassword(
                pMasterKey, master_password,
                (nullptr == pReason) ? "" : pReason->c_str());
            String strNewKeyOutput;

            if (bGotMasterPW &&
                OTSymmetricKey::CreateNewKey(strNewKeyOutput, nullptr,
                                             &master_password)) {
                std::shared_ptr<OTSymmetricKey> pNewExtraKey(
                    new OTSymmetricKey);

                if (pNewExtraKey &&
                    pNewExtraKey->SerializeFrom(strNewKeyOutput) &&
                    addExtraKey(str_KeyID, pNewExtraKey)) {

                    pExtraKey = pNewExtraKey;

                    SaveWallet();
                }
            } // if (bGotMasterPW)
        }     // if (pMasterKey)
    }

    // Then:
    //
    if (pExtraKey) return pExtraKey; // <======== SUCCESS.

    return std::shared_ptr<OTSymmetricKey>();
}

// The "extra" symmetric keys in the wallet are all, like the Nyms, encrypted
// to the wallet's master key. So whenever the wallet's master key is changed,
// this method needs to be called as well, to update those extra symmetric keys
// to the new master key. (Otherwise they'll stop working.)
//
bool OTWallet::ChangePassphrasesOnExtraKeys(const OTPassword& oldPassphrase,
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

        ot_data_t thePayload;

        if (pOldKey && pOldKey->SerializeTo(thePayload)) {
            std::shared_ptr<OTSymmetricKey> pNewKey(new OTSymmetricKey);

            if (pNewKey && pNewKey->SerializeFrom(thePayload))
                mapChanged.insert(
                    std::pair<std::string, std::shared_ptr<OTSymmetricKey>>(
                        str_id, pNewKey));
            else
                return false;
        }
        else
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
        }
        else
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

bool OTWallet::Encrypt_ByKeyID(const std::string& key_id,
                               const String& strPlaintext, String& strOutput,
                               const String* pstrDisplay, bool bBookends)
{
    if (key_id.empty() || !strPlaintext.Exists()) return false;

    std::string str_Reason((nullptr != pstrDisplay) ? pstrDisplay->Get() : "");

    std::shared_ptr<OTSymmetricKey> pKey =
        OTWallet::getOrCreateExtraKey(key_id, &str_Reason);

    if (pKey) {
        std::shared_ptr<OTCachedKey> pMasterKey(OTCachedKey::It());

        if (pMasterKey) {
            OTPassword master_password;

            if (pMasterKey->GetMasterPassword(pMasterKey, master_password))
                return OTSymmetricKey::Encrypt(*pKey, strPlaintext, strOutput,
                                               pstrDisplay, bBookends,
                                               &master_password);
        }
    }
    return false;
}
bool OTWallet::Decrypt_ByKeyID(const std::string& key_id,
                               const String& strCiphertext, String& strOutput,
                               const String* pstrDisplay)
{
    if (key_id.empty() || !strCiphertext.Exists()) return false;

    std::shared_ptr<OTSymmetricKey> pKey = OTWallet::getExtraKey(key_id);

    if (pKey) {
        std::shared_ptr<OTCachedKey> pMasterKey(OTCachedKey::It());

        if (pMasterKey) {
            OTPassword master_password;

            if (pMasterKey->GetMasterPassword(pMasterKey, master_password))
                return OTSymmetricKey::Decrypt(*pKey, strCiphertext, strOutput,
                                               pstrDisplay, &master_password);
        }
    }
    return false;
}

std::shared_ptr<OTSymmetricKey> OTWallet::getExtraKey(const std::string& str_id)
{
    if (str_id.empty()) return std::shared_ptr<OTSymmetricKey>();

    auto it = m_mapExtraKeys.find(str_id);

    if (it != m_mapExtraKeys.end()) // It's already there (can't add it.)
    {
        std::shared_ptr<OTSymmetricKey> pKey = it->second;

        return pKey;
    }

    return std::shared_ptr<OTSymmetricKey>();
}

bool OTWallet::addExtraKey(const std::string& str_id,
                           std::shared_ptr<OTSymmetricKey> pKey)
{
    if (str_id.empty() || !pKey) return false;

    auto it = m_mapExtraKeys.find(str_id);

    if (it != m_mapExtraKeys.end()) // It's already there (can't add it.)
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
            ascTemp.WriteArmoredString(strFinal, "WALLET")) // todo hardcoding.
        {
            otErr << "OTWallet::SaveWallet: Error saving wallet (failed "
                     "writing armored string):\n" << m_strDataFolder
                  << OTLog::PathSeparator() << m_strFilename << "\n";
            return false;
        }

        // Wallet file is the only one in data_folder (".") and not a subfolder
        // of that.
        bSuccess = OTDB::StorePlainString(
            strFinal.Get(), ".",
            m_strFilename.Get()); // <==== Store Plain String
    }

    return bSuccess;
}

/*

<?xml version="1.0"?>
<wallet name="" version="2.0">

<cachedKey>
CkwAAQCAAAD//wAAAAhVRpwTzc+1NAAAABCKe14aROG8v/ite3un3bBCAAAAINyw
HXTM/x449Al2z8zBHBTRF77jhHkYLj8MIgqrJ2Ep
</cachedKey>

</wallet>

 */
bool OTWallet::LoadWallet(const char* szFilename)
{
    OT_ASSERT_MSG(m_strFilename.Exists() || (nullptr != szFilename),
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
    if (!m_strFilename.Exists()) // If it's not already set, then set it.
        m_strFilename.Set(
            szFilename); // (We know nullptr wasn't passed in, in this case.)

    if (nullptr ==
        szFilename) // If nullptr was passed in, then set the pointer to
                    // existing string.
        szFilename = m_strFilename.Get(); // (We know existing string is there,
                                          // in this case.)

    if (!OTDB::Exists(".", szFilename)) {
        otErr << __FUNCTION__ << ": Wallet file does not exist: " << szFilename
              << ". Creating...\n";

        const char* szContents = "<?xml version=\"1.0\"?>\n"
                                 "<wallet name=\"\" version=\"1.0\">\n"
                                 "\n"
                                 "</wallet>\n";

        if (!OTDB::StorePlainString(szContents, ".", szFilename)) {
            otErr << __FUNCTION__
                  << ": Error: Unable to create blank wallet file.\n";
            OT_FAIL;
        }
    }

    String strFileContents(OTDB::QueryPlainString(
        ".", szFilename)); // <=== LOADING FROM DATA STORE.

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
                     "decoding. Filename: " << szFilename << " \n"
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
            String AssetID;

            String ServerName;
            String ServerID;

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
                    OTASCIIArmor ascWalletName = xml->getAttributeValue("name");

                    if (ascWalletName.Exists())
                        ascWalletName.GetString(m_strName,
                                                false); // linebreaks == false

                    //                      m_strName            =
                    // xml->getAttributeValue("name");
                    //                      OTLog::OTPath        =
                    // xml->getAttributeValue("path");
                    m_strVersion = xml->getAttributeValue("version");

                    otWarn << "\nLoading wallet: " << m_strName
                           << ", version: " << m_strVersion << "\n";
                }

                // todo: Remove the masterKey after a while. It's here for now
                // so people's data files can get
                // converted over. After a while, just remove it.
                else if (strNodeName.Compare("masterKey") ||
                         strNodeName.Compare("cachedKey")) {
                    OTASCIIArmor ascCachedKey;

                    if (Contract::LoadEncodedTextField(xml, ascCachedKey)) {
                        // We successfully loaded the cachedKey from file, so
                        // let's SET it
                        // as the cached key globally...
                        //
                        OTCachedKey::It()->SetCachedKey(ascCachedKey);

                        if (!OTCachedKey::It()->HasHashCheck()) {
                            OTPassword tempPassword;
                            tempPassword.zeroMemory();

                            std::shared_ptr<OTCachedKey> sharedPtr(
                                OTCachedKey::It());

                            bNeedToSaveAgain = sharedPtr->GetMasterPassword(
                                sharedPtr, tempPassword,
                                "We do not have a check hash yet for this "
                                "password, please enter your password",
                                true);
                        }
                    }

                    otWarn << "Loading cachedKey:\n" << ascCachedKey << "\n";
                }

                // todo: Remove the nymUsingMasterKey after a while. It's here
                // for now so people's data files can get
                // converted over. After a while, just remove it.
                else if (strNodeName.Compare("nymUsingMasterKey") ||
                         strNodeName.Compare("nymUsingCachedKey")) {
                    NymID = xml->getAttributeValue("id"); // message digest from
                                                          // hash of x.509 cert
                                                          // or public key.

                    otWarn << "NymID using Cached Key: " << NymID << "\n";
                    if (!NymID.Exists()) {
                        otErr << __FUNCTION__ << ": NymID using Cached Key was "
                                                 "empty when loading wallet!\n";
                        OT_FAIL;
                    }

                    const Identifier theNymID(NymID);

                    m_setNymsOnCachedKey.insert(theNymID);
                }
                else if (strNodeName.Compare("symmetricKey")) {
                    String strKeyID;
                    OTASCIIArmor ascKeyID = xml->getAttributeValue("id");
                    OTASCIIArmor ascSymmetricKey;

                    if (!ascKeyID.Exists() ||
                        !ascKeyID.GetString(strKeyID, false)) // linebreaks ==
                                                              // false (true by
                                                              // default.)
                        otErr << __FUNCTION__ << ": Failed loading "
                                                 "symmetricKey ID (it was "
                                                 "blank.)\n";

                    else if (Contract::LoadEncodedTextField(xml,
                                                            ascSymmetricKey)) {
                        std::shared_ptr<OTSymmetricKey> pKey(
                            new OTSymmetricKey);

                        if (!pKey || !pKey->SerializeFrom(ascSymmetricKey))
                            otErr << __FUNCTION__
                                  << ": Failed serializing symmetricKey from "
                                     "string (id: " << strKeyID << ")\n";
                        else {
                            const std::string str_id(strKeyID.Get());

                            if (!addExtraKey(str_id, pKey))
                                otErr << __FUNCTION__
                                      << ": Failed adding serialized "
                                         "symmetricKey to wallet (id: "
                                      << strKeyID << ")\n";
                        }
                    }
                }
                else if (strNodeName.Compare("pseudonym")) {
                    OTASCIIArmor ascNymName = xml->getAttributeValue("name");
                    if (ascNymName.Exists())
                        ascNymName.GetString(NymName,
                                             false); // linebreaks == false

                    NymID = xml->getAttributeValue("nymID"); // message digest
                                                             // from hash of
                                                             // x.509 cert or
                                                             // public key.

                    otInfo << "\n\n** Pseudonym ** (wallet listing): "
                           << NymName << "\nID: " << NymID << "\n";
                    if (!NymID.Exists()) {
                        otErr << __FUNCTION__ << ": NymID dosn't Exist!\n";
                        OT_FAIL;
                    }

                    const Identifier theNymID(NymID);

                    // What's going on here? We need to see if the MASTER KEY
                    // exists at this point. If it's GENERATED.
                    // If not, that means the Nyms are all still encrypted to
                    // their own passphrases, not to the master key.
                    // In which case we need to generate one and re-encrypt each
                    // private key to that new master key.
                    //
                    //                  bool OTWallet::IsNymOnCachedKey(const
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

                    OTPseudonym* pNym =
                        OTPseudonym::LoadPrivateNym(theNymID, false, &NymName);
                    // If it fails loading as a private Nym, then maybe it's a
                    // public one...
                    if (nullptr == pNym)
                        pNym = OTPseudonym::LoadPublicNym(theNymID, &NymName);

                    if (nullptr == pNym) // STILL null ??
                        otOut << __FUNCTION__ << ": Failed loading Nym ("
                              << NymName << ") with ID: " << NymID << "\n";
                    else
                        AddNym(*pNym); // Nym loaded. Insert to wallet's
                                       // list of Nyms.

                    if (bIsOldStyleNym && OTCachedKey::It()->isPaused()) {
                        OTCachedKey::It()->Unpause();
                    }
                    // (Here we set it back again, so any new-style Nyms will
                    // still load properly, when they come around.)
                }

                // NOTE: It's only by THIS point (assetType) that we KNOW we
                // loaded all the Nyms.
                // If we are version 1.0, NOW we should convert them all to the
                // new master key!!
                else if (strNodeName.Compare("assetType")) {

                    OTASCIIArmor ascAssetName = xml->getAttributeValue("name");

                    if (ascAssetName.Exists())
                        ascAssetName.GetString(AssetName,
                                               false); // linebreaks == false

                    //                    AssetName        =
                    // xml->getAttributeValue("name");
                    AssetID = xml->getAttributeValue(
                        "assetTypeID"); // hash of contract itself

                    otInfo << "\n\n****Asset Contract**** (wallet listing)\n  "
                              "Asset Name: " << AssetName
                           << "\n Contract ID: " << AssetID << "\n";

                    String strContractPath(OTFolders::Contract());
                    AssetContract* pContract = new AssetContract(
                        AssetName, strContractPath, AssetID, AssetID);

                    OT_ASSERT_MSG(nullptr != pContract,
                                  "Error allocating memory "
                                  "for Asset Contract in "
                                  "OTWallet::LoadWallet\n");

                    if (pContract->LoadContract()) {
                        if (pContract->VerifyContract()) {
                            otWarn << "** Asset Contract Verified "
                                      "**\n------------------------------------"
                                      "----------------------------------------"
                                      "-\n\n";

                            pContract->SetName(AssetName);

                            m_mapContracts[AssetID.Get()] = pContract;
                        }
                        else {
                            delete pContract;
                            pContract = nullptr;
                            otOut << "Contract FAILED to verify.\n";
                        }
                    }
                    else {
                        delete pContract;
                        pContract = nullptr;
                        otErr << __FUNCTION__
                              << ": Error reading file for Asset Contract.\n";
                    }

                }
                else if (strNodeName.Compare("notaryProvider")) {
                    OTASCIIArmor ascServerName = xml->getAttributeValue("name");

                    if (ascServerName.Exists())
                        ascServerName.GetString(ServerName,
                                                false); // linebreaks == false

                    //                    ServerName =
                    // xml->getAttributeValue("name");
                    ServerID =
                        xml->getAttributeValue("serverID"); // hash of contract

                    otInfo << "\n\n\n****Server Contract**** (wallet "
                              "listing):\n Server Name: " << ServerName
                           << "\n   Server ID: " << ServerID << "\n";

                    String strContractPath(OTFolders::Contract().Get());

                    OTServerContract* pContract = new OTServerContract(
                        ServerName, strContractPath, ServerID, ServerID);

                    OT_ASSERT_MSG(nullptr != pContract,
                                  "Error allocating memory "
                                  "for Server Contract in "
                                  "OTWallet::LoadWallet\n");

                    if (pContract->LoadContract()) {
                        if (pContract->VerifyContract()) {
                            pContract->SetName(ServerName); // This isn't
                                                            // needed, but it's
                                                            // proper.

                            otWarn << "** Server Contract Verified "
                                      "**\n------------------------------------"
                                      "----------------------------------------"
                                      "-\n\n";
                            // Uncomment : Move these lines back above the 'if'
                            // block to regenerate some newly-signed contracts.
                            // (for testing only.) Otherwise leave here where it
                            // belongs.
                            m_mapServers[ServerID.Get()] = pContract;
                        }
                        else {
                            delete pContract;
                            pContract = nullptr;
                            otOut << __FUNCTION__
                                  << ": Server contract failed to verify.\n";
                        }
                    }
                    else {
                        delete pContract;
                        pContract = nullptr;
                        otErr
                            << __FUNCTION__
                            << ": Error reading file for Transaction Server.\n";
                    }
                }
                else if (strNodeName.Compare("assetAccount")) {
                    OTASCIIArmor ascAcctName = xml->getAttributeValue("name");

                    if (ascAcctName.Exists())
                        ascAcctName.GetString(AcctName,
                                              false); // linebreaks == false

                    AcctID = xml->getAttributeValue("accountID");
                    ServerID = xml->getAttributeValue("serverID");

                    otInfo << "\n----------------------------------------------"
                              "----------------------------\n"
                              "****Account**** (wallet listing)\n"
                              " Account Name: " << AcctName
                           << "\n   Account ID: " << AcctID
                           << "\n    Server ID: " << ServerID << "\n";

                    const Identifier ACCOUNT_ID(AcctID), SERVER_ID(ServerID);

                    Account* pAccount =
                        Account::LoadExistingAccount(ACCOUNT_ID, SERVER_ID);

                    if (pAccount) {
                        pAccount->SetName(AcctName);
                        AddAccount(*pAccount);
                    }
                    else {
                        otErr << __FUNCTION__
                              << ": Error loading existing Asset Account.\n";
                    }
                }
                else {
                    // unknown element type
                    otErr << __FUNCTION__
                          << ": unknown element type: " << xml->getNodeName()
                          << "\n";
                }
            } break;
            default:
                otLog5 << __FUNCTION__
                       << ": Unknown XML type: " << xml->getNodeName() << "\n";
                break;
            }
        } // while xml->read()

        // After we've loaded all the old-format Nyms that don't use the master
        // key,
        // NOW we can go through and convert them all, now that they're all
        // loaded.

        for (auto& it : m_mapNyms) {
            OTPseudonym* pNym = it.second;
            OT_ASSERT_MSG(
                (nullptr != pNym),
                "ASSERT: OTWallet::LoadWallet: nullptr pseudonym pointer.");

            if (pNym->HasPrivateKey() &&
                ConvertNymToCachedKey(*pNym)) // Internally this is smart
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

bool OTWallet::ConvertNymToCachedKey(OTPseudonym& theNym)
{
    // If he's not ALREADY on the master key...
    //
    if (!IsNymOnCachedKey(theNym.GetConstID())) {
        bool bConverted = false;
        // The Nym has credentials.
        //
        if (theNym.GetMasterCredentialCount() > 0) {
            String strNymID, strCredList, strOutput;
            String::Map mapCredFiles;

            theNym.GetIdentifier(strNymID);
            theNym.GetPrivateCredentials(strCredList, &mapCredFiles);

            String strFilename;
            strFilename.Format("%s.cred", strNymID.Get());

            OTASCIIArmor ascArmor(strCredList);
            if (ascArmor.Exists() &&
                ascArmor.WriteArmoredString(
                    strOutput,
                    "CREDENTIAL LIST") && // bEscaped=false by default.
                strOutput.Exists()) {
                if (!OTDB::StorePlainString(strOutput.Get(),
                                            OTFolders::Credential().Get(),
                                            strFilename.Get())) {
                    otErr << __FUNCTION__ << ": Failure trying to store "
                          << (theNym.HasPrivateKey() ? "private" : "public")
                          << " credential list for Nym: " << strNymID << "\n";
                    return false;
                }
            }

            // Here we do the actual credentials.
            for (auto& it : mapCredFiles) {
                std::string str_cred_id = it.first;
                String strCredential(it.second);

                strOutput.Release();
                OTASCIIArmor ascLoopArmor(strCredential);
                if (ascLoopArmor.Exists() &&
                    ascLoopArmor.WriteArmoredString(
                        strOutput,
                        "CREDENTIAL") && // bEscaped=false by default.
                    strOutput.Exists()) {
                    if (false ==
                        OTDB::StorePlainString(strOutput.Get(),
                                               OTFolders::Credential().Get(),
                                               strNymID.Get(), str_cred_id)) {
                        otErr << __FUNCTION__ << ": Failure trying to store "
                              << (theNym.HasPrivateKey() ? "private" : "public")
                              << " credential for Nym: " << strNymID << "\n";
                        return false;
                    }
                }
            }
            bConverted = true;
        }
        else // Kicking it old-school. (No credentials.)
        {
            String strReason("Converting Nym to cached master key.");
            bConverted = theNym.Savex509CertAndPrivateKey(true, &strReason);
        }

        if (bConverted) {
            m_setNymsOnCachedKey.insert(theNym.GetConstID());
        }

        return bConverted;
    } // This block only occurs if Nym is not ALREADY on the wallet's list of
      // Nym using the wallet's cached master key.

    return false;
}

//     setOfIdentifiers m_setNymsOnCachedKey;  // All the Nyms that use the
// Master key are listed here (makes it easy to see which ones are converted
// already.)
// Todo: serialize?
//
bool OTWallet::IsNymOnCachedKey(const Identifier& needle) const // needle and
                                                                // haystack.
{
    for (const auto& it : m_setNymsOnCachedKey) {
        if (needle == it) return true;
    }
    return false;
}

} // namespace opentxs
