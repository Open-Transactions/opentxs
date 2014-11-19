/************************************************************
 *
 *  Purse.cpp
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

#include <opentxs/cash/Purse.hpp>
#include <opentxs/cash/Token.hpp>

#include <opentxs/core/crypto/OTSymmetricKey.hpp>
#include <opentxs/core/crypto/OTCachedKey.hpp>
#include <opentxs/core/crypto/OTEnvelope.hpp>
#include <opentxs/core/crypto/OTNymOrSymmetricKey.hpp>
#include <opentxs/core/crypto/OTPassword.hpp>
#include <opentxs/core/util/OTFolders.hpp>
#include <opentxs/core/OTLog.hpp>
#include <opentxs/core/OTStorage.hpp>

#include <irrxml/irrXML.hpp>

namespace opentxs
{

typedef std::map<std::string, Token*> mapOfTokenPointers;

bool Purse::GetNymID(Identifier& theOutput) const
{
    bool bSuccess = false;
    theOutput.clear();

    if (IsPasswordProtected()) {
        bSuccess = false; // optimizer will remove automatically anyway, I
                          // assume. Might as well have it here for clarity.
    }
    else if (IsNymIDIncluded() && !m_UserID.empty()) {
        bSuccess = true;
        theOutput = m_UserID;
    }
    else if (!m_UserID.empty()) {
        bSuccess = true;
        theOutput = m_UserID;
    }

    return bSuccess;
}

// Retrieves the passphrase for this purse (which is cached by its master key.)
// Prompts the user to enter his actual passphrase, if necessary to unlock it.
// (May not need unlocking yet -- there is a timeout.)
//
bool Purse::GetPassphrase(OTPassword& theOutput, const char* szDisplay)
{
    const char* szFunc = "Purse::GetPassphrase";

    if (!IsPasswordProtected()) {
        otOut << szFunc
              << ": Failed: this purse isn't even password-protected.\n";
        return false;
    }

    std::shared_ptr<OTCachedKey> pCachedKey(GetInternalMaster());
    if (!pCachedKey) OT_FAIL;

    const String strReason((nullptr == szDisplay) ? szFunc : szDisplay);

    const bool bGotMasterPassword = pCachedKey->GetMasterPassword(
        pCachedKey, theOutput, strReason.Get()); // bVerifyTwice=false
    return bGotMasterPassword;
}

// Don't ever deal with m_pCachedKey directly (except before it's been created /
// loaded.)
// When you actually USE m_pCachedKey, you want to use this function instead.
// (It will save the user from having the type the password, for example, 50
// times in 1 minute,
// by using the cached one.)

// stores the passphrase for the symmetric key.
std::shared_ptr<OTCachedKey> Purse::GetInternalMaster()
{

    if (!IsPasswordProtected() ||
        (!m_pCachedKey)) // this second half of the logic should never happen.
    {
        otOut << __FUNCTION__
              << ": Failed: no internal master key exists, in this purse.\n";
        return std::shared_ptr<OTCachedKey>();
    }

    if (!m_pCachedKey->IsGenerated()) // should never happen, since the purse IS
                                      // password-protected... then where's the
                                      // master key?
    {
        otOut << __FUNCTION__
              << ": Error: internal master key has not yet been generated.\n";
        return std::shared_ptr<OTCachedKey>();
    }

    // By this point we know the purse is password protected, the internal
    // master key
    // exists (not nullptr) and it's been properly generated, so we won't be
    // inadvertantly sticking
    // a copy of it on the CachedKey map indexed to some nonexistent ID for an
    // ungenerated key.
    // The caller will be forced to make sure the master key is real and
    // generated, before passing
    // it in here where it could get copied.
    //
    // Why is that important? BECAUSE THE COPY is all the caller will ever
    // actually use! So if it's
    // not ENTIRELY loaded up properly BEFORE it's copied, the caller will never
    // see the properly
    // loaded version of that master key.
    //
    return OTCachedKey::It(*m_pCachedKey); // here we return a cached copy of
                                           // the master key (so it's available
                                           // between instances of this purse.)
}

// INTERNAL KEY: For adding a PASSPHRASE to a PURSE.
//
// What if you DON'T want to encrypt the purse to your Nym??
// What if you just want to use a passphrase instead?
// That's what these functions are for. OT just generates
// a symmetric key and stores it INSIDE THE PURSE. You set the
// passphrase for the symmetric key, and thereafter your
// experience is one of a password-protected purse.
//
bool Purse::GenerateInternalKey()
{
    if (IsPasswordProtected() ||
        (nullptr != m_pSymmetricKey) || // GetInternalKey())
        (m_pCachedKey)) {
        otOut << __FUNCTION__
              << ": Failed: internal Key  or master key already exists. "
                 "Or IsPasswordProtected was true.\n";
        return false;
    }

    if (!IsEmpty()) {
        otOut << __FUNCTION__
              << ": Failed: The purse must be EMPTY before you create a "
                 "new symmetric key, internal to that purse. (For the purposes "
                 "of "
                 "adding a passphrase to the purse, normally.) Otherwise I "
                 "would have "
                 "to loop through all the tokens and re-assign ownership of "
                 "each one. "
                 "Instead, I'm just going to return false. That's easier.\n";
        return false;
    }

    //  OTSymmetricKey *   m_pSymmetricKey;    // If this purse contains its own
    // symmetric key (instead of using an owner Nym)...
    //  OTCachedKey    *   m_pCachedKey;       // ...then it will have a master
    // key as well, for unlocking that symmetric key, and managing timeouts.

    // m_pSymmetricKey and m_pCachedKey are both explicitly checked for nullptr
    // (above.)
    // Therefore we have to instantiate them both now.
    //
    // We'll do the Master key first, since we need the passphrase from that, in
    // order to
    // create the symmetric key.
    //
    OTPassword thePassphrase;
    const String strDisplay(
        "Enter the new passphrase for this new password-protected "
        "purse."); // todo internationalization / hardcoding.

    // thePassphrase and m_pCachedKey are BOTH output from the below function.
    //
    m_pCachedKey = OTCachedKey::CreateMasterPassword(
        thePassphrase,
        strDisplay.Get()); // int32_t nTimeoutSeconds=OT_MASTER_KEY_TIMEOUT)

    if ((!m_pCachedKey) || !m_pCachedKey->IsGenerated()) // This one is
                                                         // unnecessary because
                                                         // CreateMasterPassword
                                                         // already checks it.
                                                         // todo optimize.
    {
        otOut << __FUNCTION__
              << ": Failed: While calling OTCachedKey::CreateMasterPassword.\n";
        return false;
    }

    m_pSymmetricKey =
        new OTSymmetricKey(thePassphrase); // Creates the symmetric key here
                                           // based on the passphrase from
                                           // purse's master key.
    OT_ASSERT(nullptr != m_pSymmetricKey);

    if (!m_pSymmetricKey->IsGenerated()) {
        otOut << __FUNCTION__ << ": Failed: generating m_pSymmetricKey.\n";
        delete m_pSymmetricKey;
        m_pSymmetricKey = nullptr;
        m_pCachedKey.reset();
        return false;
    }

    m_UserID.clear();
    m_bIsNymIDIncluded = false;

    otWarn << __FUNCTION__
           << ": Successfully created a purse's internal key.\n";

    m_bPasswordProtected = true;

    std::shared_ptr<OTCachedKey> pCachedMaster(Purse::GetInternalMaster());
    if (!pCachedMaster)
        otErr << __FUNCTION__
              << ": Failed trying to cache the master key for this purse.\n";

    return true;
}

// Take all the tokens from a purse and add them to this purse.
// Don't allow duplicates.
//
bool Purse::Merge(const OTPseudonym& theSigner,
                  OTNym_or_SymmetricKey theOldNym, // must be private, if a nym.
                  OTNym_or_SymmetricKey theNewNym, // must be private, if a nym.
                  Purse& theNewPurse)
{
    const char* szFunc = "Purse::Merge";

    mapOfTokenPointers theMap;

    while (Count() > 0) {
        Token* pToken = Pop(theOldNym); // must be private, if a Nym.
        OT_ASSERT_MSG(nullptr != pToken,
                      "Purse::Merge: Assert: nullptr != Pop(theOldNym) \n");

        const OTASCIIArmor& ascTokenID = pToken->GetSpendable();

        std::list<mapOfTokenPointers::iterator> listOfTokenMapIterators;

        // I just popped a Token off of *this. Let's see if it's in my temporary
        // map...
        // If it's already there, then just delete it (duplicate).
        //
        for (auto it(theMap.begin()); it != theMap.end(); ++it) {
            Token* pTempToken = it->second;
            OT_ASSERT(nullptr != pTempToken);

            const OTASCIIArmor& ascTempTokenID = pTempToken->GetSpendable();

            // It's already there. Delete the one that's already there.
            // (That way we can add it after, whether it was there originally or
            // not.)
            if (ascTempTokenID == ascTokenID) {
                listOfTokenMapIterators.push_back(it);
                //                theMap.erase(it);
                //                delete pTempToken;
                //              pTempToken = nullptr;
                // break; // In case there are multiple duplicates, not just
                // one.
            }
        }
        while (!listOfTokenMapIterators.empty()) {
            Token* pTempToken = (listOfTokenMapIterators.back())->second;
            theMap.erase(listOfTokenMapIterators.back());
            delete pTempToken;
            pTempToken = nullptr;
            listOfTokenMapIterators.pop_back();
        }

        // Now we know there aren't any duplicates on the temporary map, let's
        // add the token to it.
        std::string theKey = ascTokenID.Get();
        theMap.insert(std::pair<std::string, Token*>(theKey, pToken));
    }
    // At this point, all of the tokens on *this have been popped, and added
    // to the temporary map as token pointers, with any duplicates removed.

    // Basically now I just want to do the exact same thing with the other
    // purse...
    //
    while (theNewPurse.Count() > 0) {
        Token* pToken = theNewPurse.Pop(theNewNym);
        OT_ASSERT_MSG(
            nullptr != pToken,
            "Purse::Merge: Assert: nullptr != theNewPurse.Pop(theNewNym) \n");

        const OTASCIIArmor& ascTokenID = pToken->GetSpendable();

        std::list<mapOfTokenPointers::iterator> listOfTokenMapIterators;

        // I just popped a Token off of theNewPurse. Let's see if it's in my
        // temporary map...
        // If it's already there, then just delete it (it's a duplicate.)
        for (auto it(theMap.begin()); it != theMap.end(); ++it) {
            Token* pTempToken = it->second;
            OT_ASSERT(nullptr != pTempToken);

            const OTASCIIArmor& ascTempTokenID = pTempToken->GetSpendable();

            // It's already there. Delete the one that's already there.
            // (That way we can add it after, whether it was there originally or
            // not.)
            if (ascTempTokenID == ascTokenID) {
                listOfTokenMapIterators.push_back(it);
                //                theMap.erase(it);
                //                delete pTempToken;
                //              pTempToken = nullptr;
                // break; // In case there are multiple duplicates, not just
                // one.
            }
        }
        while (!listOfTokenMapIterators.empty()) {
            Token* pTempToken = (listOfTokenMapIterators.back())->second;
            theMap.erase(listOfTokenMapIterators.back());
            delete pTempToken;
            pTempToken = nullptr;
            listOfTokenMapIterators.pop_back();
        }
        // Now we KNOW there aren't any duplicates on the temporary map, so
        // let's
        // add the token to it...
        //
        std::string theKey = ascTokenID.Get();
        theMap.insert(std::pair<std::string, Token*>(theKey, pToken));

        //
        // SINCE THE new purse is being MERGED into the old purse, we don't have
        // to re-assign ownership
        // of any of the old tokens. But we DO need to re-assign ownership of
        // the NEW tokens that are being
        // merged in. We reassign them from New ==> TO OLD. (And we only bother
        // if they aren't the same Nym.)
        //
        //      if (!theNewNym.CompareID(theOldNym)) // Not the same
        // Nym!!
        //
        // UPDATE: the above line was moved INSIDE OTToken::ReassignOwnership,
        // FYI.
        //
        if (false ==
            pToken->ReassignOwnership(theNewNym,  // must be private, if a Nym.
                                      theOldNym)) // can be public, if a Nym.
        {
            otErr << szFunc << ": Error: Failed while attempting to re-assign "
                               "ownership of token during purse merge.\n";
        }
        else {
            otWarn << szFunc << ": FYI: Success re-assigning ownership of "
                                "token during purse merge.\n";

            pToken->ReleaseSignatures();
            pToken->SignContract(theSigner);
            pToken->SaveContract();
        }
    }

    // At this point, all of the tokens on *this (old purse) AND theNewPurse
    // have been popped, and added
    // to the temporary map as token pointers, with any duplicates removed.
    // Also, the tokens on the New Purse have been reassigned (from theNewNym as
    // owner, to theOldNym as
    // owner) and each has been signed and saved properly, using the old Nym.

    // Next, we loop through theMap, and Push ALL of those tokens back onto
    // *this. (The old purse.)

    bool bSuccess = true;

    for (auto& it : theMap) {
        Token* pToken = it.second;
        OT_ASSERT(nullptr != pToken);

        bool bPush = Push(theOldNym, // can be public, if a Nym.
                          *pToken);  // The purse makes it's own copy of
                                     // the token, into string form.

        if (!bPush) {
            otErr << szFunc << ": Error: Failure pushing token into purse.\n";
            bSuccess = false;
        }
        // Notice we don't break here if 1 token fails -- we loop through them
        // all.
        // Maybe shouldn't? Seems right somehow.
    }

    // Next I clean up all the tokens out of the temporary map, since they will
    // leak otherwise.
    //
    while (!theMap.empty()) {
        Token* pToken = theMap.begin()->second;
        OT_ASSERT(nullptr != pToken);

        delete pToken;
        pToken = nullptr;

        theMap.erase(theMap.begin());
    }

    // Note: Caller needs to re-sign and re-save this purse, since we aren't
    // doing it
    // internally here.

    return bSuccess;
}

// static -- class factory.
//
Purse* Purse::LowLevelInstantiate(const String& strFirstLine,
                                  const Identifier& SERVER_ID,
                                  const Identifier& ASSET_ID)
{
    Purse* pPurse = nullptr;
    if (strFirstLine.Contains("-----BEGIN SIGNED PURSE-----")) // this string is
                                                               // 28 chars long.
                                                               // todo
                                                               // hardcoding.
    {
        pPurse = new Purse(SERVER_ID, ASSET_ID);
        OT_ASSERT(nullptr != pPurse);
    }
    return pPurse;
}

Purse* Purse::LowLevelInstantiate(const String& strFirstLine,
                                  const Identifier& SERVER_ID)
{
    Purse* pPurse = nullptr;
    if (strFirstLine.Contains("-----BEGIN SIGNED PURSE-----")) // this string is
                                                               // 28 chars long.
                                                               // todo
                                                               // hardcoding.
    {
        pPurse = new Purse(SERVER_ID);
        OT_ASSERT(nullptr != pPurse);
    }
    return pPurse;
}

Purse* Purse::LowLevelInstantiate(const String& strFirstLine)
{
    Purse* pPurse = nullptr;
    if (strFirstLine.Contains("-----BEGIN SIGNED PURSE-----")) // this string is
                                                               // 28 chars long.
                                                               // todo
                                                               // hardcoding.
    {
        pPurse = new Purse();
        OT_ASSERT(nullptr != pPurse);
    }
    return pPurse;
}

// static -- class factory.
//
// Checks the serverID / AssetID, so you don't have to.
//
Purse* Purse::PurseFactory(String strInput, const Identifier& SERVER_ID,
                           const Identifier& ASSET_ID)
{
    String strContract, strFirstLine; // output for the below function.
    const bool bProcessed =
        Contract::DearmorAndTrim(strInput, strContract, strFirstLine);

    if (bProcessed) {
        Purse* pPurse =
            Purse::LowLevelInstantiate(strFirstLine, SERVER_ID, ASSET_ID);

        // The string didn't match any of the options in the factory.
        if (nullptr == pPurse) return nullptr;

        // Does the contract successfully load from the string passed in?
        if (pPurse->LoadContractFromString(strContract)) {
            const char* szFunc = "Purse::PurseFactory";
            if (SERVER_ID != pPurse->GetServerID()) {
                const String strServerID(SERVER_ID),
                    strPurseServerID(pPurse->GetServerID());
                otErr << szFunc << ": Failure: ServerID on purse ("
                      << strPurseServerID << ") doesn't match expected "
                                             "server ID (" << strServerID
                      << ").\n";
                delete pPurse;
                pPurse = nullptr;
            }
            else if (ASSET_ID != pPurse->GetAssetID()) {
                const String strAssetID(ASSET_ID),
                    strPurseAssetID(pPurse->GetAssetID());
                otErr << szFunc << ": Failure: AssetID on purse ("
                      << strPurseAssetID << ") doesn't match expected "
                                            "asset ID (" << strAssetID
                      << ").\n";
                delete pPurse;
                pPurse = nullptr;
            }
            else
                return pPurse;
        }
        else {
            delete pPurse;
            pPurse = nullptr;
        }
    }

    return nullptr;
}

// Checks the serverID, so you don't have to.
//
Purse* Purse::PurseFactory(String strInput, const Identifier& SERVER_ID)
{
    String strContract, strFirstLine; // output for the below function.
    const bool bProcessed =
        Contract::DearmorAndTrim(strInput, strContract, strFirstLine);

    if (bProcessed) {
        Purse* pPurse = Purse::LowLevelInstantiate(strFirstLine, SERVER_ID);

        // The string didn't match any of the options in the factory.
        if (nullptr == pPurse) return nullptr;

        // Does the contract successfully load from the string passed in?
        if (pPurse->LoadContractFromString(strContract)) {
            if (SERVER_ID != pPurse->GetServerID()) {
                const String strServerID(SERVER_ID),
                    strPurseServerID(pPurse->GetServerID());
                otErr << "Purse::PurseFactory"
                      << ": Failure: ServerID on purse (" << strPurseServerID
                      << ") doesn't match expected server ID (" << strServerID
                      << ").\n";
                delete pPurse;
                pPurse = nullptr;
            }
            else
                return pPurse;
        }
        else {
            delete pPurse;
            pPurse = nullptr;
        }
    }

    return nullptr;
}

Purse* Purse::PurseFactory(String strInput)
{
    //  const char * szFunc = "Purse::PurseFactory";

    String strContract, strFirstLine; // output for the below function.
    const bool bProcessed =
        Contract::DearmorAndTrim(strInput, strContract, strFirstLine);

    if (bProcessed) {
        Purse* pPurse = Purse::LowLevelInstantiate(strFirstLine);

        // The string didn't match any of the options in the factory.
        if (nullptr == pPurse) return nullptr;

        // Does the contract successfully load from the string passed in?
        if (pPurse->LoadContractFromString(strContract))
            return pPurse;
        else
            delete pPurse;
    }

    return nullptr;
}

// private, used by factory.
Purse::Purse()
    : Contract()
    ,
    //    m_ServerID(),
    //    m_AssetID(),
    m_lTotalValue(0)
    , m_bPasswordProtected(false)
    , m_bIsNymIDIncluded(false)
    , m_pSymmetricKey(nullptr)
    , m_tLatestValidFrom(OT_TIME_ZERO)
    , m_tEarliestValidTo(OT_TIME_ZERO)
{
    InitPurse();
}

Purse::Purse(const Purse& thePurse)
    : Contract()
    , m_UserID()
    , m_ServerID(thePurse.GetServerID())
    , m_AssetID(thePurse.GetAssetID())
    , m_lTotalValue(0)
    , m_bPasswordProtected(false)
    , m_bIsNymIDIncluded(false)
    , m_pSymmetricKey(nullptr)
    , m_tLatestValidFrom(OT_TIME_ZERO)
    , m_tEarliestValidTo(OT_TIME_ZERO)
{
    InitPurse();
}

// Don't use this unless you really don't have the asset type handy.
// Perhaps you know you're about to read this purse from a string and you
// know the asset type is in there anyway. So you use this constructor.
Purse::Purse(const Identifier& SERVER_ID)
    : Contract()
    , m_ServerID(SERVER_ID)
    , m_lTotalValue(0)
    , m_bPasswordProtected(false)
    , m_bIsNymIDIncluded(false)
    , m_pSymmetricKey(nullptr)
    , m_tLatestValidFrom(OT_TIME_ZERO)
    , m_tEarliestValidTo(OT_TIME_ZERO)
{
    InitPurse();
}

Purse::Purse(const Identifier& SERVER_ID, const Identifier& ASSET_ID)
    : Contract()
    , m_ServerID(SERVER_ID)
    , m_AssetID(ASSET_ID)
    , m_lTotalValue(0)
    , m_bPasswordProtected(false)
    , m_bIsNymIDIncluded(false)
    , m_pSymmetricKey(nullptr)
    , m_tLatestValidFrom(OT_TIME_ZERO)
    , m_tEarliestValidTo(OT_TIME_ZERO)
{
    InitPurse();
}

Purse::Purse(const Identifier& SERVER_ID, const Identifier& ASSET_ID,
             const Identifier& USER_ID)
    : Contract()
    , m_UserID(USER_ID)
    , m_ServerID(SERVER_ID)
    , m_AssetID(ASSET_ID)
    , m_lTotalValue(0)
    , m_bPasswordProtected(false)
    , m_bIsNymIDIncluded(false)
    , m_pSymmetricKey(nullptr)
    , m_tLatestValidFrom(OT_TIME_ZERO)
    , m_tEarliestValidTo(OT_TIME_ZERO)
{
    InitPurse();
}

void Purse::InitPurse()
{
    m_strContractType.Set("PURSE");

    m_lTotalValue = 0;

    m_bPasswordProtected = false;
    m_bIsNymIDIncluded = false;
}

Purse::~Purse()
{
    Release_Purse();
}

void Purse::Release_Purse()
{
    // This sets m_lTotalValue to 0 already.
    ReleaseTokens();
    //    m_lTotalValue = 0;

    m_bPasswordProtected = false;
    m_bIsNymIDIncluded = false;

    // the Temp Nym is when a purse contains its own Nym, created just
    // for that purse, so that it can be password protected instead of using
    // one of the real Nyms in your wallet.
    //
    if (nullptr != m_pSymmetricKey) {
        delete m_pSymmetricKey;
        m_pSymmetricKey = nullptr;
    }

    //    if (m_pCachedKey)
    //    {
    //        delete m_pCachedKey;
    //        m_pCachedKey = nullptr;
    //    }
}

void Purse::Release()
{
    Release_Purse();

    Contract::Release();

    InitPurse();
}

/*
 OTIdentifier    m_UserID;    // Optional
 OTIdentifier    m_ServerID;    // Mandatory
 OTIdentifier    m_AssetID;    // Mandatory
 */

bool Purse::LoadContract()
{
    return LoadPurse();
}

bool Purse::LoadPurse(const char* szServerID, const char* szUserID,
                      const char* szAssetTypeID)
{
    OT_ASSERT(!IsPasswordProtected());

    if (!m_strFoldername.Exists())
        m_strFoldername.Set(OTFolders::Purse().Get());

    String strServerID(m_ServerID), strUserID(m_UserID),
        strAssetTypeID(m_AssetID);

    if (nullptr != szServerID) strServerID = szServerID;
    if (nullptr != szUserID) strUserID = szUserID;
    if (nullptr != szAssetTypeID) strAssetTypeID = szAssetTypeID;

    if (!m_strFilename.Exists()) {
        m_strFilename.Format("%s%s%s%s%s", strServerID.Get(),
                             OTLog::PathSeparator(), strUserID.Get(),
                             OTLog::PathSeparator(), strAssetTypeID.Get());
    }

    const char* szFolder1name = OTFolders::Purse().Get(); // purse
    const char* szFolder2name = strServerID.Get();        // purse/SERVER_ID
    const char* szFolder3name = strUserID.Get(); // purse/SERVER_ID/USER_ID
    const char* szFilename =
        strAssetTypeID.Get(); // purse/SERVER_ID/USER_ID/ASSET_TYPE_ID

    if (false ==
        OTDB::Exists(szFolder1name, szFolder2name, szFolder3name, szFilename)) {
        otInfo << "Purse::LoadPurse: File does not exist: " << szFolder1name
               << OTLog::PathSeparator() << szFolder2name
               << OTLog::PathSeparator() << szFolder3name
               << OTLog::PathSeparator() << szFilename << "\n";
        return false;
    }

    std::string strFileContents(
        OTDB::QueryPlainString(szFolder1name, szFolder2name, szFolder3name,
                               szFilename)); // <=== LOADING FROM DATA STORE.

    if (strFileContents.length() < 2) {
        otErr << "Purse::LoadPurse: Error reading file: " << szFolder1name
              << OTLog::PathSeparator() << szFolder2name
              << OTLog::PathSeparator() << szFolder3name
              << OTLog::PathSeparator() << szFilename << "\n";
        return false;
    }

    // NOTE: No need here to deal with OT ARMORED file format, since
    // LoadContractFromString
    // already handles it internally.

    String strRawFile(strFileContents.c_str());

    return LoadContractFromString(strRawFile);
}

bool Purse::SavePurse(const char* szServerID, const char* szUserID,
                      const char* szAssetTypeID)
{
    OT_ASSERT(!IsPasswordProtected());

    if (!m_strFoldername.Exists())
        m_strFoldername.Set(OTFolders::Purse().Get());

    String strServerID(m_ServerID), strUserID(m_UserID),
        strAssetTypeID(m_AssetID);

    if (nullptr != szServerID) strServerID = szServerID;
    if (nullptr != szUserID) strUserID = szUserID;
    if (nullptr != szAssetTypeID) strAssetTypeID = szAssetTypeID;

    if (!m_strFilename.Exists()) {
        m_strFilename.Format("%s%s%s%s%s", strServerID.Get(),
                             OTLog::PathSeparator(), strUserID.Get(),
                             OTLog::PathSeparator(), strAssetTypeID.Get());
    }

    const char* szFolder1name = OTFolders::Purse().Get(); // purse
    const char* szFolder2name = strServerID.Get();        // purse/SERVER_ID
    const char* szFolder3name = strUserID.Get(); // purse/SERVER_ID/USER_ID
    const char* szFilename =
        strAssetTypeID.Get(); // purse/SERVER_ID/USER_ID/ASSET_TYPE_ID

    String strRawFile;

    if (!SaveContractRaw(strRawFile)) {
        otErr << "Purse::SavePurse: Error saving Pursefile (to string):\n"
              << szFolder1name << OTLog::PathSeparator() << szFolder2name
              << OTLog::PathSeparator() << szFolder3name
              << OTLog::PathSeparator() << szFilename << "\n";
        return false;
    }

    String strFinal;
    OTASCIIArmor ascTemp(strRawFile);

    if (false ==
        ascTemp.WriteArmoredString(strFinal, m_strContractType.Get())) {
        otErr << "Purse::SavePurse: Error saving Pursefile (failed writing "
                 "armored string):\n" << szFolder1name << OTLog::PathSeparator()
              << szFolder2name << OTLog::PathSeparator() << szFolder3name
              << OTLog::PathSeparator() << szFilename << "\n";
        return false;
    }

    bool bSaved = OTDB::StorePlainString(
        strFinal.Get(), szFolder1name, szFolder2name, szFolder3name,
        szFilename); // <=== SAVING TO DATA STORE.
    if (!bSaved) {
        otErr << "Purse::SavePurse: Error writing to file: " << szFolder1name
              << OTLog::PathSeparator() << szFolder2name
              << OTLog::PathSeparator() << szFolder3name
              << OTLog::PathSeparator() << szFilename << "\n";
        return false;
    }

    return true;
}

void Purse::UpdateContents() // Before transmission or serialization, this is
                             // where the Purse saves its contents
{
    const String SERVER_ID(m_ServerID), USER_ID(m_UserID),
        ASSET_TYPE_ID(m_AssetID);

    int64_t lValidFrom = OTTimeGetSecondsFromTime(m_tLatestValidFrom);
    int64_t lValidTo = OTTimeGetSecondsFromTime(m_tEarliestValidTo);

    // I release this because I'm about to repopulate it.
    m_xmlUnsigned.Release();
    m_xmlUnsigned.Concatenate("<?xml version=\"%s\"?>\n\n", "1.0");
    m_xmlUnsigned.Concatenate(
        "<purse version=\"%s\"\n"
        " totalValue=\"%" PRId64 "\"\n" // Total value of all the tokens within.
        " validFrom=\"%" PRId64
        "\"\n" // Latest "valid from" date of all tokens contained.
        " validTo=\"%" PRId64
        "\"\n" // Earliest "valid to" date of all tokens contained.
        " isPasswordProtected=\"%s\"\n"
        " isNymIDIncluded=\"%s\"\n"
        " userID=\"%s\"\n"       // UserID   is optional.
        " assetTypeID=\"%s\"\n"  // assetTypeID required.
        " serverID=\"%s\">\n\n", // serverID is required.
        m_strVersion.Get(),
        m_lTotalValue, lValidFrom, lValidTo,
        m_bPasswordProtected ? "true" : "false",
        m_bIsNymIDIncluded ? "true" : "false",

        // USER_ID / NYM_ID of purse owner.
        // IF a real NymID (from the user's wallet) is listed in the purse
        // (which is
        // optional--user's choice) we attach that NymID here...
        //
        (m_bIsNymIDIncluded &&
         !m_UserID.empty()) // (Provided that the ID even exists, of course.)
            ?               // =====>
            USER_ID.Get()
            : "", // Then print the ID (otherwise print an empty string.)
        (!m_AssetID.empty()) ? ASSET_TYPE_ID.Get()
                             : "", // (Should never actually be empty.) todo:
                                   // Change this to just the Get()
        (!m_ServerID.empty()) ? SERVER_ID.Get()
                              : "" // (Should never actually be empty.) todo:
                                   // Change this to just the Get()
        );

    // Save the Internal Symmetric Key here (if there IS one.)
    // (Some Purses own their own internal Symmetric Key, in order to "password
    // protect" the purse.)
    //
    if (m_bPasswordProtected) {
        if (!m_pCachedKey)
            otErr
                << __FUNCTION__
                << ": Error: m_pCachedKey is unexpectedly nullptr, even though "
                   "m_bPasswordProtected is true!\n";
        else if (nullptr == m_pSymmetricKey)
            otErr << __FUNCTION__ << ": Error: m_pSymmetricKey is unexpectedly "
                                     "nullptr, even though "
                                     "m_bPasswordProtected is true!\n";
        else // m_pCachedKey and m_pSymmetricKey are good pointers. (Or at
             // least, not-null.)
        {
            if (!m_pCachedKey->IsGenerated())
                otErr << __FUNCTION__ << ": Error: m_pCachedKey wasn't a "
                                         "generated key! Even though "
                                         "m_bPasswordProtected is true.\n";
            else if (!m_pSymmetricKey->IsGenerated())
                otErr << __FUNCTION__ << ": Error: m_pSymmetricKey wasn't a "
                                         "generated key! Even though "
                                         "m_bPasswordProtected is true.\n";
            else {
                OTASCIIArmor ascCachedKey, ascSymmetricKey;

                if (!m_pCachedKey->SerializeTo(ascCachedKey) ||
                    !ascCachedKey.Exists() ||
                    !m_pSymmetricKey->SerializeTo(ascSymmetricKey) ||
                    !ascSymmetricKey.Exists())
                    otErr << __FUNCTION__
                          << ": Error: m_pCachedKey or m_pSymmetricKey failed "
                             "trying to serialize to OTASCIIArmor.\n";
                else {
                    // ascInternalKey is good by this point.
                    // Therefore, let's serialize it...

                    // By this point, ascInternalKey contains the Key itself.
                    //
                    m_xmlUnsigned.Concatenate(
                        "<cachedKey>\n%s</cachedKey>\n\n",
                        ascCachedKey.Get()); // The "password" for the internal
                                             // symmetric key.

                    m_xmlUnsigned.Concatenate(
                        "<internalKey>\n%s</internalKey>\n\n",
                        ascSymmetricKey.Get()); // The internal symmetric key,
                                                // owned by the purse.
                                                // ascii-armored.
                }
            }
        }
    }

    for (int32_t i = 0; i < Count(); i++) {
        m_xmlUnsigned.Concatenate("<token>\n%s</token>\n\n",
                                  m_dequeTokens[i]->Get());
    }

    m_xmlUnsigned.Concatenate("</purse>\n");
}

int32_t Purse::ProcessXMLNode(irr::io::IrrXMLReader*& xml)
{
    const char* szFunc = "Purse::ProcessXMLNode";

    const String strNodeName(xml->getNodeName());

    if (strNodeName.Compare("purse")) {
        m_strVersion = xml->getAttributeValue("version");

        const String strTotalValue = xml->getAttributeValue("totalValue");

        if (strTotalValue.Exists() && (strTotalValue.ToLong() > 0))
            m_lTotalValue = strTotalValue.ToLong();
        else
            m_lTotalValue = 0;

        const String str_valid_from = xml->getAttributeValue("validFrom");
        const String str_valid_to = xml->getAttributeValue("validTo");

        if (str_valid_from.Exists()) {
            int64_t lValidFrom = str_valid_from.ToLong();

            m_tLatestValidFrom = OTTimeGetTimeFromSeconds(lValidFrom);
        }
        if (str_valid_to.Exists()) {
            int64_t lValidTo = str_valid_to.ToLong();

            m_tEarliestValidTo = OTTimeGetTimeFromSeconds(lValidTo);
        }

        const String strPasswdProtected =
            xml->getAttributeValue("isPasswordProtected");
        m_bPasswordProtected = strPasswdProtected.Compare("true");

        const String strNymIDIncluded =
            xml->getAttributeValue("isNymIDIncluded");
        m_bIsNymIDIncluded = strNymIDIncluded.Compare("true");

        // TODO security: Might want to verify the server ID here, if it's
        // already set.
        // Just to make sure it's the one we were expecting.
        const String strServerID = xml->getAttributeValue("serverID");
        if (strServerID.Exists())
            m_ServerID.SetString(strServerID);
        else {
            m_ServerID.clear();
            otErr << szFunc
                  << ": Failed loading serverID, when one was expected.\n";
            return (-1);
        }

        // TODO security: Might want to verify the asset ID here, if it's
        // already set.
        // Just to make sure it's the one we were expecting.
        const String strAssetTypeID = xml->getAttributeValue("assetTypeID");
        if (strAssetTypeID.Exists())
            m_AssetID.SetString(strAssetTypeID);
        else {
            m_AssetID.clear();
            otErr << szFunc
                  << ": Failed loading assetTypeID, when one was expected.\n";
            return (-1);
        }

        const String strUserID =
            xml->getAttributeValue("userID"); // (May not exist.)
        if (m_bIsNymIDIncluded) // Nym ID **is** included.  (It's optional. Even
                                // if you use one, you don't have to list it.)
        {
            if (strUserID.Exists())
                m_UserID.SetString(strUserID);
            else {
                otErr << szFunc
                      << ": Failed loading userID, when one was expected. "
                         "(isNymIDIncluded was true.)\n";
                m_UserID.clear();
                return (-1);
            }
        }
        else // UserID SUPPOSED to be blank here. (Thus the Release.) Maybe
               // later,
            // we might consider trying to read it, in order to validate this.
            //
            m_UserID.clear(); // For now, just assume it's not there to be
                              // read, and Release my own value to match it.

        otLog4 << szFunc << ": Loaded purse... ("
               << (m_bPasswordProtected ? "Password-protected"
                                        : "NOT password-protected")
               << ")\n ServerID: " << strServerID
               << "\n UserID: " << (m_bIsNymIDIncluded ? strUserID.Get() : "")
               << "\n Asset ID: " << strAssetTypeID << "\n----------\n";

        return 1;
    }

    // Sometimes you want the purse to have a passphrase on it, without being
    // attached to
    // one of your actual Nyms in your wallet. To accommodate this, OT creates a
    // symmetric key
    // and stashes it INSIDE the purse. This symmetric key can have whatever
    // passphrase you want.
    // There is also a master key attached, which allows for passphrase timeouts
    // on the symmetric key.
    // Therefore internalKey and cachedKey will both be attached to the purse
    // (or neither will be.)
    //
    else if (strNodeName.Compare("internalKey")) {
        if (!m_bPasswordProtected) // If we're NOT using the internal key, then
                                   // why am I in the middle of loading one
                                   // here?
        {
            otErr << szFunc << ": Error: Unexpected 'internalKey' data, "
                               "since m_bPasswordProtected is set to false!\n";
            return (-1); // error condition
        }

        if (!m_UserID.empty()) // If the UserID isn't empty, then why am I in
                               // the middle of loading an internal Key?
        {
            otErr << szFunc << ": Error: Unexpected 'internalKey' data, since "
                               "m_UserID is not blank! "
                               "(The UserID should have loaded before THIS "
                               "node ever popped up...)\n";
            return (-1); // error condition
        }

        OTASCIIArmor ascValue;

        if (!Contract::LoadEncodedTextField(xml, ascValue) ||
            !ascValue.Exists()) {
            otErr << szFunc << ": Error: Expected "
                  << "internalKey"
                  << " element to have text field.\n";
            return (-1); // error condition
        }

        // Let's see if the internal key is already loaded somehow... (Shouldn't
        // be...)
        //
        if (nullptr != m_pSymmetricKey) {
            otErr << szFunc
                  << ": WARNING: While loading internal Key for a purse, "
                     "noticed the pointer was ALREADY set! (I'm deleting old "
                     "one to make room, "
                     "and then allowing this one to load instead...)\n";
            //          return (-1); // error condition

            delete m_pSymmetricKey;
            m_pSymmetricKey = nullptr;
        }

        // By this point, I've loaded up the string containing the encrypted
        // symmetric key.
        // I also know that m_bPasswordProtected is set to true, and I know that
        // m_pSymmetricKey is nullptr.
        //
        // (It's only now that I bother instantiating.)
        //
        OTSymmetricKey* pSymmetricKey = new OTSymmetricKey();
        OT_ASSERT_MSG(nullptr != pSymmetricKey, "Purse::ProcessXMLNode: "
                                                "Assert: nullptr != new "
                                                "OTSymmetricKey \n");

        // NOTE: In the event of any error, need to delete pSymmetricKey before
        // returning.
        // (Or it will leak.)
        //
        if (!pSymmetricKey->SerializeFrom(ascValue)) {
            otErr
                << szFunc
                << ": Error: While loading internal Key for a purse, failed "
                   "serializing from stored string! (Failed loading purse.)\n";
            delete pSymmetricKey;
            pSymmetricKey = nullptr;
            return (-1);
        }

        // By this point, the symmetric key has loaded successfully from
        // storage.

        otWarn << szFunc << ": Successfully loaded a purse's internal key.\n";

        // No more worry about pSymmetricKey cleanup, now that this pointer is
        // set.

        m_pSymmetricKey = pSymmetricKey;

        return 1;
    }
    else if (strNodeName.Compare("cachedKey")) {
        if (!m_bPasswordProtected) // If we're NOT using the internal and master
                                   // keys, then why am I in the middle of
                                   // loading one here?
        {
            otErr << szFunc << ": Error: Unexpected 'cachedKey' data, "
                               "since m_bPasswordProtected is set to false!\n";
            return (-1); // error condition
        }

        if (!m_UserID.empty()) // If the UserID isn't empty, then why am I in
                               // the middle of loading an internal Key?
        {
            otErr << szFunc << ": Error: Unexpected 'cachedKey' data, since "
                               "m_UserID is not blank!\n";
            return (-1); // error condition
        }

        OTASCIIArmor ascValue;

        if (!Contract::LoadEncodedTextField(xml, ascValue) ||
            !ascValue.Exists()) {
            otErr << szFunc << ": Error: Expected "
                  << "cachedKey"
                  << " element to have text field.\n";
            return (-1); // error condition
        }

        // Let's see if the master key is already loaded somehow... (Shouldn't
        // be...)
        //
        if (m_pCachedKey) {
            otErr << szFunc
                  << ": WARNING: While loading master Key for a purse, "
                     "noticed the pointer was ALREADY set! (I'm deleting old "
                     "one to make room, "
                     "and then allowing this one to load instead...)\n";
            //          return (-1); // error condition

            m_pCachedKey.reset();
        }

        // By this point, I've loaded up the string containing the encrypted
        // symmetric key.
        // I also know that m_bPasswordProtected is set to true, and I know that
        // m_pSymmetricKey is nullptr.
        //
        // (It's only now that I bother instantiating.)
        //
        std::shared_ptr<OTCachedKey> pCachedKey(new OTCachedKey(ascValue));
        //        OT_ASSERT_MSG(nullptr != pCachedKey, "Purse::ProcessXMLNode:
        // Assert: nullptr != new OTCachedKey \n");

        // NOTE: In the event of any error, need to delete pCachedKey before
        // returning.
        // (Or it will leak.)
        //
        if (!pCachedKey->SerializeFrom(ascValue)) {
            otErr
                << szFunc
                << ": Error: While loading master Key for a purse, failed "
                   "serializing from stored string! (Failed loading purse.)\n";
            //            delete pCachedKey; pCachedKey = nullptr;
            return (-1);
        }

        // By this point, the symmetric key has loaded successfully from
        // storage.

        otWarn << szFunc << ": Successfully loaded a purse's master key.\n";

        // No more worry about pSymmetricKey cleanup, now that this pointer is
        // set.

        m_pCachedKey = pCachedKey;

        // NOTE: Hereafter, do NOT use m_pCachedKey directly.
        // Instead, use OTCachedKey::It(*m_pCachedKey) (So you deal with the
        // cached
        // version, and avoid forcing the user to re-type his passphrase more
        // than
        // necessary according to timeouts designed in OTCachedKey class.)
        //
        // In fact, don't even use that. Instead, I'll add an
        // Purse::GetPassphrase
        // method, which handles that for you.

        return 1;
    }
    else if (strNodeName.Compare("token")) {
        OTASCIIArmor* pArmor = new OTASCIIArmor;
        OT_ASSERT(nullptr != pArmor);

        if (!Contract::LoadEncodedTextField(xml, *pArmor) ||
            !pArmor->Exists()) {
            otErr << szFunc << ": Error: token field without value.\n";

            delete pArmor;
            pArmor = nullptr;

            return (-1); // error condition
        }
        else {
            m_dequeTokens.push_front(pArmor);
        }

        return 1;
    }

    return 0;
}

time64_t Purse::GetLatestValidFrom() const
{
    return m_tLatestValidFrom;
}

time64_t Purse::GetEarliestValidTo() const
{
    return m_tEarliestValidTo;
}

// Verify whether the CURRENT date is AFTER the the VALID TO date.
// Notice, this will return false, if the instrument is NOT YET VALID.
// You have to use VerifyCurrentDate() to make sure you're within the
// valid date range to use this instrument. But sometimes you only want
// to know if it's expired, regardless of whether it's valid yet. So this
// function answers that for you.
//
bool Purse::IsExpired()
{
    const time64_t CURRENT_TIME = OTTimeGetCurrentTime();

    // If the current time is AFTER the valid-TO date,
    // AND the valid_to is a nonzero number (0 means "doesn't expire")
    // THEN return true (it's expired.)
    //
    if ((CURRENT_TIME >= m_tEarliestValidTo) &&
        (m_tEarliestValidTo > OT_TIME_ZERO))
        return true;
    else
        return false;
}

// Verify whether the CURRENT date is WITHIN the VALID FROM / TO dates.
//
bool Purse::VerifyCurrentDate()
{
    const time64_t CURRENT_TIME = OTTimeGetCurrentTime();

    if ((CURRENT_TIME >= m_tLatestValidFrom) &&
        ((CURRENT_TIME <= m_tEarliestValidTo) ||
         (OT_TIME_ZERO == m_tEarliestValidTo)))
        return true;
    else
        return false;
}

// Caller IS responsible to delete. (Peek returns an instance of the
// actual token, which is stored in encrypted form inside the purse.)
//
Token* Purse::Peek(OTNym_or_SymmetricKey theOwner) const
{
    if (m_dequeTokens.empty()) return nullptr;

    // Grab a pointer to the first armored token on the deque.
    //
    const OTASCIIArmor* pArmor = m_dequeTokens.front();
    // ---------------
    // Copy the token contents into an Envelope.
    OTEnvelope theEnvelope(*pArmor);

    // Open the envelope into a string.
    //
    String strToken;
    const String strDisplay(__FUNCTION__); // this is the passphrase string
                                           // that will display if theOwner
                                           // doesn't have one already.

    const bool bSuccess =
        theOwner.Open_or_Decrypt(theEnvelope, strToken, &strDisplay);

    if (bSuccess) {
        // Create a new token with the same server and asset IDs as this purse.
        Token* pToken = Token::TokenFactory(strToken, *this);
        OT_ASSERT(nullptr != pToken);

        if (pToken->GetAssetID() != m_AssetID ||
            pToken->GetServerID() != m_ServerID) {
            delete pToken;
            pToken = nullptr;

            otErr << __FUNCTION__
                  << ": ERROR: Cash token with wrong server or asset type.\n";
        }
        else {
            // CALLER is responsible to delete this token.
            return pToken;
        }
    }
    else
        otErr << __FUNCTION__ << ": Failure: theOwner.Open_or_Decrypt.\n";

    return nullptr;
}

// Hypocritically (compared to Push) in the case of Pop(), we DO
// allocate a OTToken and return the pointer. The caller IS
// responsible to delete it when he's done with it.
//
// The apparent discrepancy is due to the fact that internally,
// we aren't storing the token object but an encrypted string of it.
// But this is hidden from the user of the purse, who perceives only
// that he is passing tokens in and getting them back out again.
//
Token* Purse::Pop(OTNym_or_SymmetricKey theOwner)
{
    if (m_dequeTokens.empty()) return nullptr;

    Token* pToken = Peek(theOwner);

    if (nullptr == pToken) {
        otErr << __FUNCTION__ << ": Failure: Peek(theOwner) "
                                 "(And m_dequeTokens isn't empty, either.)\n";
        return nullptr;
    }

    // Grab a pointer to the ascii-armored token, and remove it from the deque.
    // (And delete it.)
    //
    OTASCIIArmor* pArmor = m_dequeTokens.front();
    m_dequeTokens.pop_front();
    delete pArmor;
    pArmor = nullptr;

    // We keep track of the purse's total value.
    m_lTotalValue -= pToken->GetDenomination();

    // We keep track of the purse's expiration dates, based on the tokens
    // within.
    //
    //    OT_ASSERT(pToken->GetValidFrom() <= m_tLatestValidFrom); // If the
    // token's was larger, then the purse's should match it already.
    //    OT_ASSERT(pToken->GetValidTo()   >= m_tEarliestValidTo); // If the
    // token's was smaller, then the purse's should match it already.

    // NOTE: the above asserts were commented out because the below call to
    // RecalculateExpirationDates
    // was commented out (because without recalculating those dates when tokens
    // are removed, these asserts
    // would get triggered.)

    if ((pToken->GetValidFrom() == m_tLatestValidFrom) ||
        (pToken->GetValidTo() == m_tEarliestValidTo)) {
        //      RecalculateExpirationDates(theOwner);
    }

    // CALLER is responsible to delete this token.
    return pToken;
}

void Purse::RecalculateExpirationDates(OTNym_or_SymmetricKey& theOwner)
{
    m_tLatestValidFrom = OT_TIME_ZERO;
    m_tEarliestValidTo = OT_TIME_ZERO;

    for (auto& it : m_dequeTokens) {
        OTASCIIArmor* pArmor = it;
        OT_ASSERT(nullptr != pArmor);

        OTEnvelope theEnvelope(*pArmor);

        // Open the envelope into a string.
        //
        String strToken;
        const String strDisplay(__FUNCTION__); // this is the passphrase
                                               // string that will display if
                                               // theOwner doesn't have one
                                               // already.

        const bool bSuccess =
            theOwner.Open_or_Decrypt(theEnvelope, strToken, &strDisplay);

        if (bSuccess) {
            // Create a new token with the same server and asset IDs as this
            // purse.
            Token* pToken = Token::TokenFactory(strToken, *this);
            OT_ASSERT(nullptr != pToken);

            if (m_tLatestValidFrom < pToken->GetValidFrom()) {
                m_tLatestValidFrom = pToken->GetValidFrom();
            }

            if ((OT_TIME_ZERO == m_tEarliestValidTo) ||
                (m_tEarliestValidTo > pToken->GetValidTo())) {
                m_tEarliestValidTo = pToken->GetValidTo();
            }

            if (m_tLatestValidFrom > m_tEarliestValidTo)
                otErr << __FUNCTION__
                      << ": WARNING: This purse has a 'valid from' date LATER "
                         "than the 'valid to' date. "
                         "(due to different tokens with different date "
                         "ranges...)\n";

        }
        else
            otErr << __FUNCTION__
                  << ": Failure while trying to decrypt a token.\n";
    }
}

// Use a local variable for theToken, do NOT allocate it on the heap
// unless you are going to delete it yourself.
// Repeat: Purse is NOT responsible to delete it. We create our OWN internal
// variable here, new that, and add it to the stack. We do not add the one
// passed in.
bool Purse::Push(OTNym_or_SymmetricKey theOwner, const Token& theToken)
{
    if (theToken.GetAssetID() == m_AssetID) {
        const String strDisplay(__FUNCTION__); // this is the passphrase
                                               // string that will display if
                                               // theOwner doesn't have one
                                               // already.

        String strToken(theToken);
        OTEnvelope theEnvelope;
        const bool bSuccess =
            theOwner.Seal_or_Encrypt(theEnvelope, strToken, &strDisplay);

        if (bSuccess) {
            OTASCIIArmor* pArmor = new OTASCIIArmor(theEnvelope);

            m_dequeTokens.push_front(pArmor);

            // We keep track of the purse's total value.
            m_lTotalValue += theToken.GetDenomination();

            // We keep track of the expiration dates for the purse, based on the
            // tokens within.
            //
            if (m_tLatestValidFrom < theToken.GetValidFrom()) {
                m_tLatestValidFrom = theToken.GetValidFrom();
            }

            if ((OT_TIME_ZERO == m_tEarliestValidTo) ||
                (m_tEarliestValidTo > theToken.GetValidTo())) {
                m_tEarliestValidTo = theToken.GetValidTo();
            }

            if (m_tLatestValidFrom > m_tEarliestValidTo)
                otErr << __FUNCTION__
                      << ": WARNING: This purse has a 'valid from' date LATER "
                         "than the 'valid to' date. "
                         "(due to different tokens with different date "
                         "ranges...)\n";

            return true;
        }
        else {
            String strPurseAssetType(m_AssetID),
                strTokenAssetType(theToken.GetAssetID());
            otErr << __FUNCTION__ << ": Failed while calling: "
                                     "theOwner.Seal_or_Encrypt(theEnvelope, "
                                     "strToken)\nPurse Asset Type:\n"
                  << strPurseAssetType << "\n"
                                          "Token Asset Type:\n"
                  << strTokenAssetType << "\n";
        }
    }
    else {
        String strPurseAssetType(m_AssetID),
            strTokenAssetType(theToken.GetAssetID());
        otErr << __FUNCTION__ << ": ERROR: Tried to push token with wrong "
                                 "asset type.\nPurse Asset Type:\n"
              << strPurseAssetType << "\n"
                                      "Token Asset Type:\n" << strTokenAssetType
              << "\n";
    }

    return false;
}

int32_t Purse::Count() const
{
    return static_cast<int32_t>(m_dequeTokens.size());
}

bool Purse::IsEmpty() const
{
    return m_dequeTokens.empty();
}

void Purse::ReleaseTokens()
{
    while (!m_dequeTokens.empty()) {
        OTASCIIArmor* pArmor = m_dequeTokens.front();
        m_dequeTokens.pop_front();
        delete pArmor;
    }

    m_lTotalValue = 0;
}

} // namespace opentxs
