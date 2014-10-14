/************************************************************
 *
 *  OTContract.cpp
 *
 *  Base class for many of the OT classes.
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

#include "stdafx.hpp"

#include "OTContract.hpp"
#include "crypto/OTAsymmetricKey.hpp"
#include "crypto/OTCrypto.hpp"
#include "util/OTFolders.hpp"
#include "OTLog.hpp"
#include "crypto/OTPasswordData.hpp"
#include "OTPseudonym.hpp"
#include "crypto/OTSignature.hpp"
#include "OTStorage.hpp"

#include <irrxml/irrXML.hpp>

#include <fstream>
#include <memory>

using namespace irr;
using namespace io;

namespace opentxs
{

// static
bool OTContract::DearmorAndTrim(const OTString& strInput, OTString& strOutput,
                                OTString& strFirstLine)
{

    if (!strInput.Exists()) {
        otErr << __FUNCTION__ << ": Input string is empty.\n";
        return false;
    }

    strOutput.Set(strInput);

    if (false ==
        strOutput.DecodeIfArmored(false)) // bEscapedIsAllowed=true by default.
    {
        otErr << __FUNCTION__ << ": Input string apparently was encoded and "
                                 "then failed decoding. Contents: \n"
              << strInput << "\n";
        return false;
    }

    strOutput.reset(); // for sgets

    // At this point, strOutput contains the actual contents, whether they
    // were originally ascii-armored OR NOT. (And they are also now trimmed,
    // either way.)

    static char buf[75] = "";
    buf[0] = 0; // probably unnecessary.
    bool bGotLine = strOutput.sgets(buf, 70);

    if (!bGotLine) return false;

    strFirstLine.Set(buf);
    strOutput.reset(); // set the "file" pointer within this string back to
                       // index 0.

    // Now I feel pretty safe -- the string I'm examining is within
    // the first 70 characters of the beginning of the contract, and
    // it will NOT contain the escape "- " sequence. From there, if
    // it contains the proper sequence, I will instantiate that type.
    if (!strFirstLine.Exists() || strFirstLine.Contains("- -")) return false;

    return true;
}

OTContract::OTContract()
{
    Initialize();
}

OTContract::OTContract(const OTString& name, const OTString& foldername,
                       const OTString& filename, const OTString& strID)
{
    Initialize();

    m_strName = name;
    m_strFoldername = foldername;
    m_strFilename = filename;

    m_ID.SetString(strID);
}

OTContract::OTContract(const OTString& strID)
{
    Initialize();

    m_ID.SetString(strID);
}

OTContract::OTContract(const OTIdentifier& theID)
{
    Initialize();

    m_ID = theID;
}

void OTContract::Initialize()
{
    m_strContractType =
        "CONTRACT"; // CONTRACT, MESSAGE, TRANSACTION, LEDGER, TRANSACTION ITEM
    // make sure subclasses set this in their own initialization routine.

    m_strSigHashType = OTIdentifier::DefaultHashAlgorithm;
    m_strVersion = "2.0"; // since new credentials system.
}

// The name, filename, version, and ID loaded by the wallet
// are NOT released here, since they are used immediately after
// the Release() call in LoadContract(). Really I just want to
// "Release" the stuff that is about to be loaded, not the stuff
// that I need to load it!
void OTContract::Release_Contract()
{
    // !! Notice I don't release the m_strFilename here!!
    // Because in LoadContract, we want to release all the members, and then
    // load up from the file.
    // So if I release the filename, now I can't load up from the file cause I
    // just blanked it. DUh.
    //
    // m_strFilename.Release();

    m_strSigHashType = OTIdentifier::DefaultHashAlgorithm;
    m_xmlUnsigned.Release();
    m_strRawFile.Release();

    ReleaseSignatures();

    m_mapConditions.clear();

    // Go through the existing list of nyms at this point, and delete them all.
    while (!m_mapNyms.empty()) {
        OTPseudonym* pNym = m_mapNyms.begin()->second;
        OT_ASSERT(nullptr != pNym);
        delete pNym;
        pNym = nullptr;
        m_mapNyms.erase(m_mapNyms.begin());
    }
}

void OTContract::Release()
{
    Release_Contract();

    // No call to ot_super::Release() here, since OTContract
    // is the base class.
}

OTContract::~OTContract()
{

    Release_Contract();
}

bool OTContract::SaveToContractFolder()
{
    OTString strFoldername(OTFolders::Contract().Get()), strFilename;

    GetIdentifier(strFilename);

    // These are already set in SaveContract(), called below.
    //    m_strFoldername    = strFoldername;
    //    m_strFilename    = strFilename;

    otInfo << "OTContract::SaveToContractFolder: Saving asset contract to "
              "disk...\n";

    return SaveContract(strFoldername.Get(), strFilename.Get());
}

void OTContract::GetFilename(OTString& strFilename) const
{
    strFilename = m_strFilename;
}

void OTContract::GetFoldername(OTString& strFoldername) const
{
    strFoldername = m_strFoldername;
}

void OTContract::GetIdentifier(OTIdentifier& theIdentifier) const
{
    theIdentifier = m_ID;
}

void OTContract::GetIdentifier(OTString& theIdentifier) const
{
    m_ID.GetString(theIdentifier);
}

// Make sure this contract checks out. Very high level.
// Verifies ID, existence of public key, and signature.
//
bool OTContract::VerifyContract()
{
    // Make sure that the supposed Contract ID that was set is actually
    // a hash of the contract file, signatures and all.
    if (!VerifyContractID()) {
        otWarn << __FUNCTION__ << ": Failed verifying contract ID.\n";
        return false;
    }

    // Make sure we are able to read the official "contract" public key out of
    // this contract.
    const OTPseudonym* pNym = GetContractPublicNym();

    if (nullptr == pNym) {
        otOut << __FUNCTION__
              << ": Failed retrieving public nym from contract.\n";
        return false;
    }

    if (!VerifySignature(*pNym)) {
        const OTIdentifier theNymID(*pNym);
        const OTString strNymID(theNymID);
        otOut << __FUNCTION__ << ": Failed verifying the contract's signature "
                                 "against the public key that was retrieved "
                                 "from the contract, with key ID: " << strNymID
              << "\n";
        return false;
    }

    otWarn << "\nVerified -- The Contract ID from the wallet matches the "
              "newly-calculated hash of the contract file.\n"
              "Verified -- A standard \"contract\" Public Key or x509 Cert WAS "
              "found inside the contract.\n"
              "Verified -- And the **SIGNATURE VERIFIED** with THAT key.\n\n";
    return true;
}

void OTContract::CalculateContractID(OTIdentifier& newID) const
{
    // may be redundant...
    std::string str_Trim(m_strRawFile.Get());
    std::string str_Trim2 = OTString::trim(str_Trim);

    OTString strTemp(str_Trim2.c_str());

    if (!newID.CalculateDigest(strTemp))
        otErr << __FUNCTION__ << ": Error calculating Contract digest.\n";
}

bool OTContract::VerifyContractID() const
{
    OTIdentifier newID;
    CalculateContractID(newID);

    // newID now contains the Hash aka Message Digest aka Fingerprint
    // aka thumbprint aka "IDENTIFIER" of the Contract.
    //
    // Now let's compare that identifier to the one already loaded by the wallet
    // for this contract and make sure they MATCH.

    // I use the == operator here because there is no != operator at this time.
    // That's why you see the ! outside the parenthesis.
    //
    if (!(m_ID == newID)) {
        OTString str1(m_ID), str2(newID);

        otOut << "\nHashes do NOT match in OTContract::VerifyContractID.\n "
                 "Expected: " << str1 << "\n   Actual: " << str2
              << "\n"
                 //                "\nRAW FILE:\n--->" << m_strRawFile << "<---"
                 "\n";
        return false;
    }
    else {
        OTString str1;
        newID.GetString(str1);
        otWarn << "\nContract ID *SUCCESSFUL* match to "
               << OTIdentifier::DefaultHashAlgorithm
               << " hash of contract file: " << str1 << "\n\n";
        return true;
    }
}

const OTPseudonym* OTContract::GetContractPublicNym() const
{
    for (auto& it : m_mapNyms) {
        OTPseudonym* pNym = it.second;
        OT_ASSERT_MSG(
            nullptr != pNym,
            "nullptr pseudonym pointer in OTContract::GetContractPublicNym.\n");

        // We favor the new "credential" system over the old "public key"
        // system.
        // No one will ever actually put BOTH in a single contract. But if they
        // do,
        // we favor the new version over the old.
        if (it.first == "signer") {
            return pNym;
        }
        // TODO have a place for hardcoded values like this.
        else if (it.first == "contract") {
            // We're saying here that every contract has to have a key tag
            // called "contract"
            // where the official public key can be found for it and for any
            // contract.
            return pNym;
        }
    }

    return nullptr;
}

// If there is a public key I can find for this contract, then
// I will return it here -- or nullptr.
const OTAsymmetricKey* OTContract::GetContractPublicKey() const
{
    for (auto& it : m_mapNyms) {
        OTPseudonym* pNym = it.second;
        OT_ASSERT_MSG(
            nullptr != pNym,
            "nullptr pseudonym pointer in OTContract::GetContractPublicKey.\n");

        if (it.first == "signer" || it.first == "contract") {
            return &(pNym->GetPublicSignKey());
        }
    }

    return nullptr;
}

// This is the one that you will most likely want to call.
// It actually attaches the resulting signature to this contract.
// If you want the signature to remain on the contract and be handled
// internally, then this is what you should call.
//
bool OTContract::SignContract(const OTPseudonym& theNym,
                              const OTPasswordData* pPWData)
{
    OTSignature* pSig = new OTSignature();
    OT_ASSERT_MSG(
        nullptr != pSig,
        "OTContract::SignContract: Error allocating memory for Signature.\n");

    bool bSigned = SignContract(theNym, *pSig, pPWData);

    if (bSigned)
        m_listSignatures.push_back(pSig);
    else {
        otErr << __FUNCTION__ << ": Failure while calling "
                                 "SignContract(theNym, *pSig, pPWData)\n";
        delete pSig;
        pSig = nullptr;
    }

    return bSigned;
}

// Signs using authentication key instead of signing key.
//
bool OTContract::SignContractAuthent(const OTPseudonym& theNym,
                                     const OTPasswordData* pPWData)
{
    OTSignature* pSig = new OTSignature();
    OT_ASSERT_MSG(nullptr != pSig, "OTContract::SignContractAuthent: Error "
                                   "allocating memory for Signature.\n");

    bool bSigned = SignContractAuthent(theNym, *pSig, pPWData);

    if (bSigned)
        m_listSignatures.push_back(pSig);
    else {
        otErr << __FUNCTION__ << ": Failure while calling "
                                 "SignContractAuthent(theNym, *pSig, "
                                 "pPWData)\n";
        delete pSig;
        pSig = nullptr;
    }

    return bSigned;
}

// The output signature will be in theSignature.
// It is NOT attached to the contract.  This is just a utility function.
bool OTContract::SignContract(const OTPseudonym& theNym,
                              OTSignature& theSignature,
                              const OTPasswordData* pPWData)
{
    return SignContract(theNym.GetPrivateSignKey(), theSignature,
                        m_strSigHashType, pPWData);
}

// Uses authentication key instead of signing key.
bool OTContract::SignContractAuthent(const OTPseudonym& theNym,
                                     OTSignature& theSignature,
                                     const OTPasswordData* pPWData)
{
    return SignContract(theNym.GetPrivateAuthKey(), theSignature,
                        m_strSigHashType, pPWData);
}

// Normally you'd use OTContract::SignContract(const OTPseudonym& theNym)...
// Normally you WOULDN'T use this function SignWithKey.
// But this is here anyway for those peculiar places where you need it. For
// example,
// when first creating a Nym, you generate the master credential as part of
// creating
// the Nym, and the master credential has to sign itself, and it therefore needs
// to be
// able to "sign a contract" at a high level using purely the key, without
// having the Nym
// ready yet to signing anything with.
//
bool OTContract::SignWithKey(const OTAsymmetricKey& theKey,
                             const OTPasswordData* pPWData)
{
    OTSignature* pSig = new OTSignature();
    OT_ASSERT_MSG(
        nullptr != pSig,
        "OTContract::SignWithKey: Error allocating memory for Signature.\n");

    bool bSigned = SignContract(theKey, *pSig, m_strSigHashType, pPWData);

    if (bSigned)
        m_listSignatures.push_back(pSig);
    else {
        otErr << __FUNCTION__
              << ": Failure while calling SignContract(theNym, *pSig).\n";
        delete pSig;
        pSig = nullptr;
    }

    return bSigned;
}

// Done: When signing a contract, need to record the metadata into the signature
// object here.

// We will know if the key is signing, authentication, or encryption key
// because?
// Because we used the Nym to choose it! In which case we should have a default
// option,
// and also some other function with a new name that calls SignContract and
// CHANGES that default
// option.
// For example, SignContract(bool bUseAuthenticationKey=false)
// Then: SignContractAuthentication() { return SignContract(true); }
//
// In most cases we actually WILL want the signing key, since we are actually
// signing contracts
// such as cash withdrawals, etc. But when the Nym stores something for himself
// locally, or when
// sending messages, those will use the authentication key.
//
// We'll also have the ability to SWITCH the key which is important because it
// raises the
// question, how do we CHOOSE the key? On my phone I might use a different key
// than on my iPad.
// theNym should either know already (GetPrivateKey being intelligent) or it
// must be passed in
// (Into the below versions of SignContract.)
//
// If theKey knows its type (A|E|S) the next question is, does it know its other
// metadata?
// It certainly CAN know, can't it? Especially if it's being loaded from
// credentials in the
// first place. And if not, well then the data's not there and it's not added to
// the signature.
// (Simple.) So I will put the Signature Metadata into its own class, so not
// only a signature
// can use it, but also the OTAsymmetricKey class can use it and also
// OTSubcredential can use it.
// Then OTContract just uses it if it's there. Also we don't have to pass it in
// here as separate
// parameters. At most we have to figure out which private key to get above, in
// theNym.GetPrivateKey()
// Worst case maybe put a loop, and see which of the private keys inside that
// Nym, in its credentials,
// is actually loaded and available. Then just have GetPrivateKey return THAT
// one. Similarly, later
// on, in VerifySignature, we'll pass the signature itself into the Nym so that
// the Nym can use it
// to help search for the proper public key to use for verifying, based on that
// metadata.
//
// This is all great because it means the only real change I need to do here now
// is to see if
// theKey.HasMetadata and if so, just copy it directly over to theSignature's
// Metadata.
//

// The output signature will be in theSignature.
// It is NOT attached to the contract.  This is just a utility function.
//
bool OTContract::SignContract(const OTAsymmetricKey& theKey,
                              OTSignature& theSignature,
                              const OTString& strHashType,
                              const OTPasswordData* pPWData)
{
    // We assume if there's any important metadata, it will already
    // be on the key, so we just copy it over to the signature.
    //
    if (nullptr != theKey.m_pMetadata) {
        theSignature.getMetaData() = *(theKey.m_pMetadata);
    }

    // Update the contents, (not always necessary, many contracts are read-only)
    // This is where we provide an overridable function for the child classes
    // that
    // need to update their contents at this point.
    // But the OTContract version of this function is actually empty, since the
    // default behavior is that contract contents don't change.
    // (Accounts and Messages being two big exceptions.)
    //
    UpdateContents();

    if (false ==
        OTCrypto::It()->SignContract(m_xmlUnsigned, theKey, theSignature,
                                     strHashType, pPWData)) {
        otErr << "OTContract::SignContract: "
                 "OTCrypto::It()->SignContract returned false.\n";
        return false;
    }

    return true;
}

bool OTContract::VerifySigAuthent(const OTPseudonym& theNym,
                                  const OTPasswordData* pPWData) const
{
    OTString strNymID;
    theNym.GetIdentifier(strNymID);
    char cNymID = '0';
    uint32_t nIndex = 0;
    const bool bNymID = strNymID.At(nIndex, cNymID);

    for (auto& it : m_listSignatures) {
        OTSignature* pSig = it;
        OT_ASSERT(nullptr != pSig);

        if (bNymID && pSig->getMetaData().HasMetadata()) {
            // If the signature has metadata, then it knows the first character
            // of the NymID that signed it. We know the first character of the
            // NymID
            // who's trying to verify it. Thus, if they don't match, we can skip
            // this
            // signature without having to try to verify it at all.
            //
            if (pSig->getMetaData().FirstCharNymID() != cNymID) continue;
        }

        if (VerifySigAuthent(theNym, *pSig, pPWData)) return true;
    }

    return false;
}

bool OTContract::VerifySignature(const OTPseudonym& theNym,
                                 const OTPasswordData* pPWData) const
{
    OTString strNymID;
    theNym.GetIdentifier(strNymID);
    char cNymID = '0';
    uint32_t nIndex = 0;
    const bool bNymID = strNymID.At(nIndex, cNymID);

    for (auto& it : m_listSignatures) {
        OTSignature* pSig = it;
        OT_ASSERT(nullptr != pSig);

        if (bNymID && pSig->getMetaData().HasMetadata()) {
            // If the signature has metadata, then it knows the first character
            // of the NymID that signed it. We know the first character of the
            // NymID
            // who's trying to verify it. Thus, if they don't match, we can skip
            // this
            // signature without having to try to verify it at all.
            //
            if (pSig->getMetaData().FirstCharNymID() != cNymID) continue;
        }

        if (VerifySignature(theNym, *pSig, pPWData)) return true;
    }

    return false;
}

bool OTContract::VerifyWithKey(const OTAsymmetricKey& theKey,
                               const OTPasswordData* pPWData) const
{
    for (auto& it : m_listSignatures) {
        OTSignature* pSig = it;
        OT_ASSERT(nullptr != pSig);

        if (theKey.m_pMetadata && theKey.m_pMetadata->HasMetadata() &&
            pSig->getMetaData().HasMetadata()) {
            // Since key and signature both have metadata, we can use it
            // to skip signatures which don't match this key.
            //
            if (pSig->getMetaData() != *(theKey.m_pMetadata)) continue;
        }

        OTPasswordData thePWData("OTContract::VerifyWithKey");
        if (VerifySignature(theKey, *pSig, m_strSigHashType,
                            (nullptr != pPWData) ? pPWData : &thePWData))
            return true;
    }

    return false;
}

// Like VerifySignature, except it uses the authentication key instead of the
// signing key.
// (Like for sent messages or stored files, where you want a signature but you
// don't want
// a legally binding signature, just a technically secure signature.)
//
bool OTContract::VerifySigAuthent(const OTPseudonym& theNym,
                                  const OTSignature& theSignature,
                                  const OTPasswordData* pPWData) const
{

    OTPasswordData thePWData("OTContract::VerifySigAuthent 1");
    listOfAsymmetricKeys listOutput;

    const int32_t nCount = theNym.GetPublicKeysBySignature(
        listOutput, theSignature, 'A'); // 'A' for authentication key.

    if (nCount > 0) // Found some (potentially) matching keys...
    {
        for (auto& it : listOutput) {
            OTAsymmetricKey* pKey = it;
            OT_ASSERT(nullptr != pKey);

            if (VerifySignature(*pKey, theSignature, m_strSigHashType,
                                (nullptr != pPWData) ? pPWData : &thePWData))
                return true;
        }
    }
    else {
        OTString strNymID;
        theNym.GetIdentifier(strNymID);
        otWarn << __FUNCTION__
               << ": Tried to grab a list of keys from this Nym (" << strNymID
               << ") which might match this signature, "
                  "but recovered none. Therefore, will attempt to verify using "
                  "the Nym's default public "
                  "AUTHENTICATION key.\n";
    }
    // else found no keys.

    return VerifySignature(theNym.GetPublicAuthKey(), theSignature,
                           m_strSigHashType,
                           (nullptr != pPWData) ? pPWData : &thePWData);
}

// The only different between calling this with a Nym and calling it with an
// Asymmetric Key is that
// the key gives you the choice of hash algorithm, whereas the nym version uses
// m_strHashType to decide
// for you.  Choose the function you prefer, you can do it either way.
//
bool OTContract::VerifySignature(const OTPseudonym& theNym,
                                 const OTSignature& theSignature,
                                 const OTPasswordData* pPWData) const
{

    OTPasswordData thePWData("OTContract::VerifySignature 1");
    listOfAsymmetricKeys listOutput;

    const int32_t nCount = theNym.GetPublicKeysBySignature(
        listOutput, theSignature, 'S'); // 'S' for signing key.

    if (nCount > 0) // Found some (potentially) matching keys...
    {
        for (auto& it : listOutput) {
            OTAsymmetricKey* pKey = it;
            OT_ASSERT(nullptr != pKey);

            if (VerifySignature(*pKey, theSignature, m_strSigHashType,
                                (nullptr != pPWData) ? pPWData : &thePWData))
                return true;
        }
    }
    else {
        OTString strNymID;
        theNym.GetIdentifier(strNymID);
        otWarn << __FUNCTION__
               << ": Tried to grab a list of keys from this Nym (" << strNymID
               << ") which might match this signature, "
                  "but recovered none. Therefore, will attempt to verify using "
                  "the Nym's default public "
                  "SIGNING key.\n";
    }
    // else found no keys.

    return VerifySignature(theNym.GetPublicSignKey(), theSignature,
                           m_strSigHashType,
                           (nullptr != pPWData) ? pPWData : &thePWData);
}

bool OTContract::VerifySignature(const OTAsymmetricKey& theKey,
                                 const OTSignature& theSignature,
                                 const OTString& strHashType,
                                 const OTPasswordData* pPWData) const
{
    // See if this key could possibly have even signed this signature.
    // (The metadata may eliminate it as a possibility.)
    //
    if ((nullptr != theKey.m_pMetadata) && theKey.m_pMetadata->HasMetadata() &&
        theSignature.getMetaData().HasMetadata()) {
        if (theSignature.getMetaData() != *(theKey.m_pMetadata)) return false;
    }

    OTPasswordData thePWData("OTContract::VerifySignature 2");

    if (false ==
        OTCrypto::It()->VerifySignature(
            m_xmlUnsigned, theKey, theSignature, strHashType,
            (nullptr != pPWData) ? pPWData : &thePWData)) {
        otLog4 << __FUNCTION__
               << ": OTCrypto::It()->VerifySignature returned false.\n";
        return false;
    }

    return true;
}

void OTContract::ReleaseSignatures()
{

    while (!m_listSignatures.empty()) {
        OTSignature* pSig = m_listSignatures.front();
        m_listSignatures.pop_front();
        delete pSig;
    }
}

bool OTContract::DisplayStatistics(OTString& strContents) const
{
    // Subclasses may override this.
    strContents.Concatenate(
        const_cast<char*>("ERROR:  OTContract::DisplayStatistics was called "
                          "instead of a subclass...\n"));

    return false;
}

bool OTContract::SaveContractWallet(OTString&) const
{
    // Subclasses may use this.

    return false;
}

bool OTContract::SaveContents(std::ofstream& ofs) const
{
    ofs << m_xmlUnsigned;

    return true;
}

// Saves the unsigned XML contents to a string
bool OTContract::SaveContents(OTString& strContents) const
{
    strContents.Concatenate(m_xmlUnsigned.Get());

    return true;
}

// Save the contract member variables into the m_strRawFile variable
bool OTContract::SaveContract()
{
    OTString strTemp;
    bool bSuccess = RewriteContract(strTemp);

    if (bSuccess) {
        m_strRawFile.Set(strTemp);

        // RewriteContract() already does this.
        //
        //        std::string str_Trim(strTemp.Get());
        //        std::string str_Trim2 = OTString::trim(str_Trim);
        //        m_strRawFile.Set(str_Trim2.c_str());
    }

    return bSuccess;
}

void OTContract::UpdateContents()
{
    // Deliberately left blank.
    //
    // Some child classes may need to perform work here
    // (OTAccount and OTMessage, for example.)
    //
    // This function is called just prior to the signing of a contract.

    // Update: MOST child classes actually use this.
    // The server and asset contracts are not meant to ever change after
    // they are signed. However, many other contracts are meant to change
    // and be re-signed. (You cannot change something without signing it.)
    // (So most child classes override this method.)
}

// CreateContract is great if you already know what kind of contract to
// instantiate
// and have already done so. Otherwise this function will take ANY flat text and
// use
// a generic OTContract instance to sign it and then write it to strOutput. This
// is
// due to the fact that OT was never really designed for signing flat text, only
// contracts.
//
// static
bool OTContract::SignFlatText(OTString& strFlatText,
                              const OTString& strContractType,
                              const OTPseudonym& theSigner, OTString& strOutput)
{
    const char* szFunc = "OTContract::SignFlatText";

    // Trim the input to remove any extraneous whitespace
    //
    std::string str_Trim(strFlatText.Get());
    std::string str_Trim2 = OTString::trim(str_Trim);

    strFlatText.Set(str_Trim2.c_str());

    char cNewline = 0;
    const uint32_t lLength = strFlatText.GetLength();

    if ((3 > lLength) || !strFlatText.At(lLength - 1, cNewline)) {
        otErr << szFunc
              << ": Invalid input: text is less than 3 bytes "
                 "int64_t, or unable to read a byte from the end where "
                 "a newline is meant to be.\n";
        return false;
    }

    // ADD a newline, if necessary.
    // (The -----BEGIN part needs to start on its OWN LINE...)
    //
    // If length is 10, then string goes from 0..9.
    // Null terminator will be at 10.
    // Therefore the final newline should be at 9.
    // Therefore if char_at_index[lLength-1] != '\n'
    // Concatenate one!

    OTString strInput;
    if ('\n' == cNewline) // It already has a newline
        strInput = strFlatText;
    else
        strInput.Format("%s\n", strFlatText.Get());

    OTSignature theSignature;
    OTPasswordData thePWData("Signing flat text (need private key)");

    if (false ==
        OTCrypto::It()->SignContract(strInput, theSigner.GetPrivateSignKey(),
                                     theSignature, // the output
                                     OTIdentifier::DefaultHashAlgorithm,
                                     &thePWData)) {
        otErr << szFunc << ": SignContract failed. Contents:\n\n" << strInput
              << "\n\n\n";
        return false;
    }

    listOfSignatures listSignatures;
    listSignatures.push_back(&theSignature);

    const bool bBookends = OTContract::AddBookendsAroundContent(
        strOutput, // the output (other params are input.)
        strInput, strContractType, OTIdentifier::DefaultHashAlgorithm,
        listSignatures);

    return bBookends;
}

// Saves the raw (pre-existing) contract text to any string you want to pass in.
bool OTContract::SaveContractRaw(OTString& strOutput) const
{
    strOutput.Concatenate("%s", m_strRawFile.Get());

    return true;
}

// static
bool OTContract::AddBookendsAroundContent(
    OTString& strOutput, const OTString& strContents,
    const OTString& strContractType, const OTString& strHashType,
    const listOfSignatures& listSignatures)
{
    OTString strTemp;

    strTemp.Concatenate("-----BEGIN SIGNED %s-----\nHash: %s\n\n",
                        strContractType.Get(), strHashType.Get());

    strTemp.Concatenate("%s", strContents.Get());

    for (const auto& it : listSignatures) {
        OTSignature* pSig = it;
        OT_ASSERT(nullptr != pSig);

        strTemp.Concatenate("-----BEGIN %s SIGNATURE-----\n"
                            "Version: Open Transactions %s\n"
                            "Comment: "
                            "http://github.com/FellowTraveler/"
                            "Open-Transactions/wiki\n",
                            strContractType.Get(), OTLog::Version());

        if (pSig->getMetaData().HasMetadata())
            strTemp.Concatenate("Meta:    %c%c%c%c\n",
                                pSig->getMetaData().GetKeyType(),
                                pSig->getMetaData().FirstCharNymID(),
                                pSig->getMetaData().FirstCharMasterCredID(),
                                pSig->getMetaData().FirstCharSubCredID());

        strTemp.Concatenate("\n%s",
                            pSig->Get()); // <=== *** THE SIGNATURE ITSELF ***
        strTemp.Concatenate("-----END %s SIGNATURE-----\n\n",
                            strContractType.Get());
    }

    std::string str_Trim(strTemp.Get());
    std::string str_Trim2 = OTString::trim(str_Trim);
    strOutput.Set(str_Trim2.c_str());

    return true;
}

// Takes the pre-existing XML contents (WITHOUT signatures) and re-writes
// into strOutput the appearance of m_strRawData, adding the pre-existing
// signatures along with new signature bookends.. (The caller actually passes
// m_strRawData into this function...)
//
bool OTContract::RewriteContract(OTString& strOutput) const
{
    OTString strContents;
    SaveContents(strContents);

    return OTContract::AddBookendsAroundContent(
        strOutput, strContents, m_strContractType, m_strSigHashType,
        m_listSignatures);
}

bool OTContract::SaveContract(const char* szFoldername, const char* szFilename)
{
    OT_ASSERT_MSG(nullptr != szFilename,
                  "Null filename sent to OTContract::SaveContract\n");
    OT_ASSERT_MSG(nullptr != szFoldername,
                  "Null foldername sent to OTContract::SaveContract\n");

    m_strFoldername.Set(szFoldername);
    m_strFilename.Set(szFilename);

    OT_ASSERT(m_strFoldername.GetLength() > 2);
    OT_ASSERT(m_strFilename.GetLength() > 2);

    if (!m_strRawFile.Exists()) {
        otErr << "OTContract::SaveContract: Error saving file (contract "
                 "contents are empty): " << szFoldername
              << OTLog::PathSeparator() << szFilename << "\n";
        return false;
    }

    OTString strFinal;

    OTASCIIArmor ascTemp(m_strRawFile);

    if (false ==
        ascTemp.WriteArmoredString(strFinal, m_strContractType.Get())) {
        otErr << "OTContract::SaveContract: Error saving file (failed writing "
                 "armored string): " << szFoldername << OTLog::PathSeparator()
              << szFilename << "\n";
        return false;
    }

    bool bSaved =
        OTDB::StorePlainString(strFinal.Get(), szFoldername, szFilename);

    if (!bSaved) {
        otErr << "OTContract::SaveContract: Error saving file: " << szFoldername
              << OTLog::PathSeparator() << szFilename << "\n";
        return false;
    }

    return true;
}

// assumes m_strFilename is already set.
// Then it reads that file into a string.
// Then it parses that string into the object.
bool OTContract::LoadContract()
{
    Release();
    LoadContractRawFile(); // opens m_strFilename and reads into m_strRawFile

    return ParseRawFile(); // Parses m_strRawFile into the various member
                           // variables.
}

// The entire Raw File, signatures and all, is used to calculate the hash
// value that becomes the ID of the contract. If you change even one letter,
// then you get a different ID.
// This applies to all contracts except accounts, since their contents must
// change periodically, their ID is not calculated from a hash of the file,
// but instead is chosen at random when the account is created.
bool OTContract::LoadContractRawFile()
{
    const char* szFoldername = m_strFoldername.Get();
    const char* szFilename = m_strFilename.Get();

    if (!m_strFoldername.Exists() || !m_strFilename.Exists()) return false;

    if (!OTDB::Exists(szFoldername, szFilename)) {
        otErr << __FUNCTION__ << ": File does not exist: " << szFoldername
              << OTLog::PathSeparator() << szFilename << "\n";
        return false;
    }

    OTString strFileContents(OTDB::QueryPlainString(
        szFoldername, szFilename)); // <=== LOADING FROM DATA STORE.

    if (!strFileContents.Exists()) {
        otErr << __FUNCTION__ << ": Error reading file: " << szFoldername
              << OTLog::PathSeparator() << szFilename << "\n";
        return false;
    }

    if (false ==
        strFileContents.DecodeIfArmored()) // bEscapedIsAllowed=true by default.
    {
        otErr << __FUNCTION__ << ": Input string apparently was encoded and "
                                 "then failed decoding. Contents: \n"
              << strFileContents << "\n";
        return false;
    }

    // At this point, strFileContents contains the actual contents, whether they
    // were originally ascii-armored OR NOT. (And they are also now trimmed,
    // either way.)
    //
    m_strRawFile.Set(strFileContents);

    return m_strRawFile.Exists();
}

bool OTContract::LoadContract(const char* szFoldername, const char* szFilename)
{
    Release();

    m_strFoldername.Set(szFoldername);
    m_strFilename.Set(szFilename);

    // opens m_strFilename and reads into m_strRawFile
    if (LoadContractRawFile())
        return ParseRawFile(); // Parses m_strRawFile into the various member
                               // variables.
    else {
        otErr << "Failed loading raw contract file: " << m_strFoldername
              << OTLog::PathSeparator() << m_strFilename << "\n";
    }
    return false;
}

// Just like it says. If you have a contract in string form, pass it in
// here to import it.
bool OTContract::LoadContractFromString(const OTString& theStr)
{
    Release();

    if (!theStr.Exists()) {
        otErr << __FUNCTION__ << ": ERROR: Empty string passed in...\n";
        return false;
    }

    OTString strContract(theStr);

    if (false ==
        strContract.DecodeIfArmored()) // bEscapedIsAllowed=true by default.
    {
        otErr << __FUNCTION__ << ": ERROR: Input string apparently was encoded "
                                 "and then failed decoding. "
                                 "Contents: \n" << theStr << "\n";
        return false;
    }

    m_strRawFile.Set(strContract);

    // This populates m_xmlUnsigned with the contents of m_strRawFile (minus
    // bookends, signatures, etc. JUST the XML.)
    bool bSuccess =
        ParseRawFile(); // It also parses into the various member variables.

    // Removed:
    // This was the bug where the version changed from 75 to 75c, and suddenly
    // contract ID was wrong...
    //
    // If it was a success, save back to m_strRawFile again so
    // the format is consistent and hashes will calculate properly.
    //    if (bSuccess)
    //    {
    //        // Basically we take the m_xmlUnsigned that we parsed out of the
    // raw file before,
    //        // then we use that to generate the raw file again, re-attaching
    // the signatures.
    //        // This function does that.
    //        SaveContract();
    //    }

    return bSuccess;
}

bool OTContract::ParseRawFile()
{
    char buffer1[2100]; // a bit bigger than 2048, just for safety reasons.
    OTSignature* pSig = nullptr;

    std::string line;

    bool bSignatureMode = false;          // "currently in signature mode"
    bool bContentMode = false;            // "currently in content mode"
    bool bHaveEnteredContentMode = false; // "have yet to enter content mode"

    if (!m_strRawFile.GetLength()) {
        otErr << "Empty m_strRawFile in OTContract::ParseRawFile. Filename: "
              << m_strFoldername << OTLog::PathSeparator() << m_strFilename
              << ".\n";
        return false;
    }

    // This is redundant (I thought) but the problem hasn't cleared up yet.. so
    // trying to really nail it now.
    std::string str_Trim(m_strRawFile.Get());
    std::string str_Trim2 = OTString::trim(str_Trim);
    m_strRawFile.Set(str_Trim2.c_str());

    bool bIsEOF = false;
    m_strRawFile.reset();

    do {
        // Just a fresh start at the top of the loop block... probably
        // unnecessary.
        memset(buffer1, 0, 2100); // todo remove this in optimization. (might be
                                  // removed already...)

        // the call returns true if there's more to read, and false if there
        // isn't.
        bIsEOF = !(m_strRawFile.sgets(buffer1, 2048));

        line = buffer1;
        const char* pBuf = line.c_str();

        if (line.length() < 2) {
            if (bSignatureMode) continue;
        }

        // if we're on a dashed line...
        else if (line.at(0) == '-') {
            if (bSignatureMode) {
                // we just reached the end of a signature
                //    otErr << "%s\n", pSig->Get());
                pSig = nullptr;
                bSignatureMode = false;
                continue;
            }

            // if I'm NOT in signature mode, and I just hit a dash, that means
            // there
            // are only four options:

            // a. I have not yet even entered content mode, and just now
            // entering it for the first time.
            if (!bHaveEnteredContentMode) {
                if ((line.length() > 3) &&
                    (line.find("BEGIN") != std::string::npos) &&
                    line.at(1) == '-' && line.at(2) == '-' &&
                    line.at(3) == '-') {
                    //                    otErr << "\nProcessing contract...
                    // \n";
                    bHaveEnteredContentMode = true;
                    bContentMode = true;
                    continue;
                }
                else {
                    continue;
                }

            }

            // b. I am now entering signature mode!
            else if (line.length() > 3 &&
                     line.find("SIGNATURE") != std::string::npos &&
                     line.at(1) == '-' && line.at(2) == '-' &&
                     line.at(3) == '-') {
                // if (bContentMode)
                //    otLog3 << "Finished reading contract.\n\nReading a
                // signature at the bottom of the contract...\n");
                // else
                //    otLog3 << "Reading another signature...\n");

                bSignatureMode = true;
                bContentMode = false;

                pSig = new OTSignature();

                OT_ASSERT_MSG(nullptr != pSig, "Error allocating memory for "
                                               "Signature in "
                                               "OTContract::ParseRawFile\n");

                m_listSignatures.push_back(pSig);

                continue;
            }
            // c. There is an error in the file!
            else if (line.length() < 3 || line.at(1) != ' ' ||
                     line.at(2) != '-') {
                otOut
                    << "Error in contract " << m_strFilename
                    << ": a dash at the beginning of the "
                       "line should be followed by a space and another dash:\n"
                    << m_strRawFile << "\n";
                return false;
            }
            // d. It is an escaped dash, and therefore kosher, so I merely
            // remove the escape and add it.
            // I've decided not to remove the dashes but to keep them as part of
            // the signed content.
            // It's just much easier to deal with that way. The input code will
            // insert the extra dashes.
            // pBuf += 2;
        }

        // Else we're on a normal line, not a dashed line.
        else {
            if (bHaveEnteredContentMode) {
                if (bSignatureMode) {
                    if (line.length() < 2) {
                        otLog3 << "Skipping short line...\n";

                        if (bIsEOF || !m_strRawFile.sgets(buffer1, 2048)) {
                            otOut << "Error in signature for contract "
                                  << m_strFilename
                                  << ": Unexpected EOF after short line.\n";
                            return false;
                        }

                        continue;
                    }
                    else if (line.compare(0, 8, "Version:") == 0) {
                        otLog3 << "Skipping version section...\n";

                        if (bIsEOF || !m_strRawFile.sgets(buffer1, 2048)) {
                            otOut << "Error in signature for contract "
                                  << m_strFilename
                                  << ": Unexpected EOF after \"Version:\"\n";
                            return false;
                        }

                        continue;
                    }
                    else if (line.compare(0, 8, "Comment:") == 0) {
                        otLog3 << "Skipping comment section...\n";

                        if (bIsEOF || !m_strRawFile.sgets(buffer1, 2048)) {
                            otOut << "Error in signature for contract "
                                  << m_strFilename
                                  << ": Unexpected EOF after \"Comment:\"\n";
                            return false;
                        }

                        continue;
                    }
                    if (line.compare(0, 5, "Meta:") == 0) {
                        otLog3 << "Collecting signature metadata...\n";

                        if (line.length() !=
                            13) // "Meta:    knms" (It will always be exactly 13
                                // characters int64_t.) knms represents the
                                // first characters of the Key type, NymID,
                                // Master Cred ID, and Subcred ID. Key type is
                                // (A|E|S) and the others are base62.
                        {
                            otOut << "Error in signature for contract "
                                  << m_strFilename << ": Unexpected length for "
                                                      "\"Meta:\" comment.\n";
                            return false;
                        }

                        OT_ASSERT(nullptr != pSig);
                        if (false ==
                            pSig->getMetaData().SetMetadata(
                                line.at(9), line.at(10), line.at(11),
                                line.at(12))) // "knms" from "Meta:    knms"
                        {
                            otOut << "Error in signature for contract "
                                  << m_strFilename
                                  << ": Unexpected metadata in the \"Meta:\" "
                                     "comment.\nLine: " << line << "\n";
                            return false;
                        }

                        if (bIsEOF || !m_strRawFile.sgets(buffer1, 2048)) {
                            otOut << "Error in signature for contract "
                                  << m_strFilename
                                  << ": Unexpected EOF after \"Meta:\"\n";
                            return false;
                        }

                        continue;
                    }
                }
                if (bContentMode) {
                    if (line.compare(0, 6, "Hash: ") == 0) {
                        otLog3 << "Collecting message digest algorithm from "
                                  "contract header...\n";

                        std::string strTemp = line.substr(6);
                        m_strSigHashType = strTemp.c_str();
                        m_strSigHashType.ConvertToUpperCase();

                        if (bIsEOF || !m_strRawFile.sgets(buffer1, 2048)) {
                            otOut << "Error in contract " << m_strFilename
                                  << ": Unexpected EOF after \"Hash:\"\n";
                            return false;
                        }
                        continue;
                    }
                }
            }
        }

        if (bSignatureMode) {
            OT_ASSERT_MSG(nullptr != pSig,
                          "Error: Null Signature pointer WHILE "
                          "processing signature, in "
                          "OTContract::ParseRawFile");

            pSig->Concatenate("%s\n", pBuf);
        }
        else if (bContentMode)
            m_xmlUnsigned.Concatenate("%s\n", pBuf);
    } while (!bIsEOF);
    //    while(!bIsEOF && (!bHaveEnteredContentMode || bContentMode ||
    // bSignatureMode));

    if (!bHaveEnteredContentMode) {
        otErr << "Error in OTContract::ParseRawFile: Found no BEGIN for signed "
                 "content.\n";
        return false;
    }
    else if (bContentMode) {
        otErr << "Error in OTContract::ParseRawFile: EOF while reading xml "
                 "content.\n";
        return false;
    }
    else if (bSignatureMode) {
        otErr << "Error in OTContract::ParseRawFile: EOF while reading "
                 "signature.\n";
        return false;
    }
    else if (!LoadContractXML()) {
        otErr << "Error in OTContract::ParseRawFile: unable to load XML "
                 "portion of contract into memory.\n";
        return false;
    }
    // Verification code and loading code are now called separately.
    //    else if (!VerifyContractID())
    //    {
    //        otErr << "Error in OTContract::ParseRawFile: Contract ID does not
    // match hashed contract file.\n";
    //        return false;
    //    }
    else {
        return true;
    }
}

// This function assumes that m_xmlUnsigned is ready to be processed.
// This function only processes that portion of the contract.
bool OTContract::LoadContractXML()
{
    int32_t retProcess = 0;

    if (!m_xmlUnsigned.Exists()) {
        return false;
    }

    m_xmlUnsigned.reset();

    IrrXMLReader* xml = irr::io::createIrrXMLReader(m_xmlUnsigned);
    OT_ASSERT_MSG(nullptr != xml, "Memory allocation issue with xml reader in "
                                  "OTContract::LoadContractXML()\n");
    std::unique_ptr<IrrXMLReader> xmlAngel(xml);

    // parse the file until end reached
    while (xml->read()) {
        OTString strNodeType;

        switch (xml->getNodeType()) {
        case EXN_NONE:
            strNodeType.Set("EXN_NONE");
            goto switch_log;
        case EXN_COMMENT:
            strNodeType.Set("EXN_COMMENT");
            goto switch_log;
        case EXN_ELEMENT_END:
            strNodeType.Set("EXN_ELEMENT_END");
            goto switch_log;
        case EXN_CDATA:
            strNodeType.Set("EXN_CDATA");
            goto switch_log;

        switch_log:
            //                otErr << "SKIPPING %s element in
            // OTContract::LoadContractXML: "
            //                              "type: %d, name: %s, value: %s\n",
            //                              strNodeType.Get(),
            // xml->getNodeType(), xml->getNodeName(), xml->getNodeData());

            break;

        case EXN_TEXT: {
            // unknown element type
            //                otErr << "SKIPPING unknown text element type in
            // OTContract::LoadContractXML: %s, value: %s\n",
            //                              xml->getNodeName(),
            // xml->getNodeData());
        } break;
        case EXN_ELEMENT: {
            retProcess = ProcessXMLNode(xml);

            // an error was returned. file format or whatever.
            if ((-1) == retProcess) {
                otErr << "OTContract::LoadContractXML: (Cancelling this "
                         "contract load; an error occurred.)\n";
                return false;
            }
            // No error, but also the node wasn't found...
            else if (0 == retProcess) {
                // unknown element type
                otErr << "UNKNOWN element type in OTContract::LoadContractXML: "
                      << xml->getNodeName() << ", value: " << xml->getNodeData()
                      << "\n";
            }
            // else if 1 was returned, that means the node was processed.
        } break;
        default: {
            //                otErr << "SKIPPING (default case) element in
            // OTContract::LoadContractXML: %d, value: %s\n",
            //                              xml->getNodeType(),
            // xml->getNodeData());
        }
            continue;
        }
    }

    return true;
}

// static
bool OTContract::SkipToElement(IrrXMLReader*& xml)
{
    OT_ASSERT_MSG(nullptr != xml,
                  "OTContract::SkipToElement -- assert: nullptr != xml");

    const char* szFunc = "OTContract::SkipToElement";

    while (xml->read() && (xml->getNodeType() != EXN_ELEMENT)) {
        //      otOut << szFunc << ": Looping to skip non-elements: currently
        // on: " << xml->getNodeName() << " \n";

        if (xml->getNodeType() == EXN_NONE) {
            otOut << "*** " << szFunc << ": EXN_NONE  (skipping)\n";
            continue;
        } // SKIP
        else if (xml->getNodeType() == EXN_COMMENT) {
            otOut << "*** " << szFunc << ": EXN_COMMENT  (skipping)\n";
            continue;
        } // SKIP
        else if (xml->getNodeType() == EXN_ELEMENT_END)
        //        { otOut << "*** OTContract::SkipToElement: EXN_ELEMENT_END
        // (ERROR)\n";  return false; }
        {
            otWarn << "*** " << szFunc << ": EXN_ELEMENT_END  (skipping "
                   << xml->getNodeName() << ")\n";
            continue;
        }
        else if (xml->getNodeType() == EXN_CDATA) {
            otOut << "*** " << szFunc
                  << ": EXN_CDATA (ERROR -- unexpected CData)\n";
            return false;
        }
        else if (xml->getNodeType() == EXN_TEXT) {
            otErr << "*** " << szFunc << ": EXN_TEXT\n";
            return false;
        }
        else if (xml->getNodeType() == EXN_ELEMENT) {
            otOut << "*** " << szFunc << ": EXN_ELEMENT\n";
            break;
        } // (Should never happen due to while() second condition.) Still
          // returns true.
        else {
            otErr << "*** " << szFunc
                  << ": SHOULD NEVER HAPPEN  (Unknown element type!)\n";
            return false;
        } // Failure / Error
    }

    return true;
}

// static
bool OTContract::SkipToTextField(IrrXMLReader*& xml)
{
    OT_ASSERT_MSG(nullptr != xml,
                  "OTContract::SkipToTextField -- assert: nullptr != xml");

    const char* szFunc = "OTContract::SkipToTextField";

    while (xml->read() && (xml->getNodeType() != EXN_TEXT)) {
        if (xml->getNodeType() == EXN_NONE) {
            otOut << "*** " << szFunc << ": EXN_NONE  (skipping)\n";
            continue;
        } // SKIP
        else if (xml->getNodeType() == EXN_COMMENT) {
            otOut << "*** " << szFunc << ": EXN_COMMENT  (skipping)\n";
            continue;
        } // SKIP
        else if (xml->getNodeType() == EXN_ELEMENT_END)
        //        { otOut << "*** OTContract::SkipToTextField:
        // EXN_ELEMENT_END  (skipping)\n";  continue; }     // SKIP
        // (debugging...)
        {
            otOut << "*** " << szFunc << ": EXN_ELEMENT_END  (ERROR)\n";
            return false;
        }
        else if (xml->getNodeType() == EXN_CDATA) {
            otOut << "*** " << szFunc
                  << ": EXN_CDATA (ERROR -- unexpected CData)\n";
            return false;
        }
        else if (xml->getNodeType() == EXN_ELEMENT) {
            otOut << "*** " << szFunc << ": EXN_ELEMENT\n";
            return false;
        }
        else if (xml->getNodeType() == EXN_TEXT) {
            otErr << "*** " << szFunc << ": EXN_TEXT\n";
            break;
        } // (Should never happen due to while() second condition.) Still
          // returns true.
        else {
            otErr << "*** " << szFunc
                  << ": SHOULD NEVER HAPPEN  (Unknown element type!)\n";
            return false;
        } // Failure / Error
    }

    return true;
}

// AFTER you read an element or text field, there is some whitespace, and you
// just want to bring your cursor back to wherever it should be for the next
// guy.
// So you call this function..
//
// static
bool OTContract::SkipAfterLoadingField(IrrXMLReader*& xml)
{
    OT_ASSERT_MSG(
        nullptr != xml,
        "OTContract::SkipAfterLoadingField -- assert: nullptr != xml");

    if (EXN_ELEMENT_END != xml->getNodeType()) // If we're not ALREADY on the
                                               // ending element, then go there.
    {
        const char* szFunc = "OTContract::SkipAfterLoadingField";
        // move to the next node which SHOULD be the expected element_end.
        //
        while (xml->read()) {
            if (xml->getNodeType() == EXN_NONE) {
                otOut << "*** " << szFunc << ": EXN_NONE  (skipping)\n";
                continue;
            } // SKIP
            else if (xml->getNodeType() == EXN_COMMENT) {
                otOut << "*** " << szFunc << ": EXN_COMMENT  (skipping)\n";
                continue;
            } // SKIP
            else if (xml->getNodeType() == EXN_ELEMENT_END) {
                otLog5 << "*** " << szFunc << ": EXN_ELEMENT_END  (success)\n";
                break;
            } // Success...
            else if (xml->getNodeType() == EXN_CDATA) {
                otOut << "*** " << szFunc << ": EXN_CDATA  (Unexpected!)\n";
                return false;
            } // Failure / Error
            else if (xml->getNodeType() == EXN_ELEMENT) {
                otOut << "*** " << szFunc << ": EXN_ELEMENT  (Unexpected!)\n";
                return false;
            } // Failure / Error
            else if (xml->getNodeType() == EXN_TEXT) {
                otErr << "*** " << szFunc << ": EXN_TEXT  (Unexpected!)\n";
                return false;
            } // Failure / Error
            else {
                otErr << "*** " << szFunc
                      << ": SHOULD NEVER HAPPEN  (Unknown element type!)\n";
                return false;
            } // Failure / Error
        }
    }

    // else ... (already on the ending element.)
    //

    return true;
}

// Loads it up and also decodes it to a string.
//
// static
bool OTContract::LoadEncodedTextField(IrrXMLReader*& xml, OTString& strOutput)
{
    OTASCIIArmor ascOutput;

    if (OTContract::LoadEncodedTextField(xml, ascOutput) &&
        ascOutput.GetLength() > 2) {
        return ascOutput.GetString(strOutput, true); // linebreaks = true
    }

    return false;
}

// static
bool OTContract::LoadEncodedTextField(IrrXMLReader*& xml,
                                      OTASCIIArmor& ascOutput)
{
    OT_ASSERT_MSG(nullptr != xml,
                  "OTContract::LoadEncodedTextField -- assert: nullptr != xml");

    const char* szFunc = "OTContract::LoadEncodedTextField";

    // If we're not ALREADY on a text field, maybe there is some whitespace, so
    // let's skip ahead...
    //
    if (EXN_TEXT != xml->getNodeType()) {
        otLog4 << szFunc << ": Skipping non-text field... \n";

        // move to the next node which SHOULD be the expected text field.
        //
        if (!SkipToTextField(xml)) {
            otOut << szFunc
                  << ": Failure: Unable to find expected text field.\n";
            return false;
        }
        otLog4 << szFunc
               << ": Finished skipping non-text field. (Successfully.)\n";
    }

    if (EXN_TEXT == xml->getNodeType()) // SHOULD always be true, in fact this
                                        // could be an assert().
    {
        OTString strNodeData = xml->getNodeData();

        // Sometimes the XML reads up the data with a prepended newline.
        // This screws up my own objects which expect a consistent in/out
        // So I'm checking here for that prepended newline, and removing it.
        //
        char cNewline;
        if (strNodeData.Exists() && strNodeData.GetLength() > 2 &&
            strNodeData.At(0, cNewline)) {
            if ('\n' == cNewline) {
                ascOutput.Set(strNodeData.Get() + 1);
            }
            else {
                ascOutput.Set(strNodeData.Get());
            }

            // SkipAfterLoadingField() only skips ahead if it's not ALREADY
            // sitting on an element_end node.
            //
            xml->read(); // THIS PUTS us on the CLOSING TAG.
                         // <========================

            // The below call won't advance any further if it's ALREADY on the
            // closing tag (e.g. from the above xml->read() call.)
            if (!SkipAfterLoadingField(xml)) {
                otOut << "*** " << szFunc
                      << ": Bad data? Expected EXN_ELEMENT_END here, but "
                         "didn't get it. Returning false.\n";
                return false;
            }

            return true;
        }
    }
    else
        otOut << szFunc << ": Failure: Unable to find expected text field. 2\n";

    return false;
}

// Loads it up and also decodes it to a string.
// static
bool OTContract::LoadEncodedTextFieldByName(IrrXMLReader*& xml,
                                            OTString& strOutput,
                                            const char*& szName,
                                            OTString::Map* pmapExtraVars)
{
    OT_ASSERT(nullptr != szName);

    OTASCIIArmor ascOutput;

    if (OTContract::LoadEncodedTextFieldByName(xml, ascOutput, szName,
                                               pmapExtraVars) &&
        ascOutput.GetLength() > 2) {
        return ascOutput.GetString(strOutput, true); // linebreaks = true
    }

    return false;
}

// Loads it up and keeps it encoded in an ascii-armored object.
// static
bool OTContract::LoadEncodedTextFieldByName(IrrXMLReader*& xml,
                                            OTASCIIArmor& ascOutput,
                                            const char*& szName,
                                            OTString::Map* pmapExtraVars)
{
    OT_ASSERT(nullptr != szName);

    const char* pElementExpected = szName;

    // If we're not ALREADY on an element, maybe there is some whitespace, so
    // let's skip ahead...
    //
    if ((EXN_ELEMENT != xml->getNodeType()) || // If we're not already on a
                                               // node, OR if the node's
        !(strcmp(pElementExpected, xml->getNodeName()) ==
          0)) // name doesn't match the one expected.
    {
        // move to the next node which SHOULD be the expected name.
        //
        if (!SkipToElement(xml)) {
            otOut << __FUNCTION__
                  << ": Failure: Unable to find expected element: " << szName
                  << ". \n";
            return false;
        }
    }

    if (EXN_ELEMENT == xml->getNodeType()) // SHOULD always be true...
    {
        if (!strcmp(pElementExpected, xml->getNodeName())) {

            if (nullptr !=
                pmapExtraVars) // If the caller wants values for certain
                               // names expected to be on this node.
            {
                OTString::Map& mapExtraVars = (*pmapExtraVars);

                for (auto& it : mapExtraVars) {
                    std::string first = it.first;
                    OTString strTemp = xml->getAttributeValue(first.c_str());

                    if (strTemp.Exists()) {
                        mapExtraVars[first] = strTemp.Get();

                        //                        mapExtraVars.erase(first);
                        //                        mapExtraVars.insert(std::pair<std::string,
                        // std::string>(first, strTemp.Get()));
                    }
                }
            } // Any attribute names passed in, now have their corresponding
              // values set on mapExtraVars (for caller.)

            if (false ==
                OTContract::LoadEncodedTextField(
                    xml,
                    ascOutput)) // <====================================================
            {
                otErr << __FUNCTION__ << ": Error loading " << pElementExpected
                      << " field.\n";
                return false; // error condition
            }
            else {

                // SkipAfterLoadingField() only skips ahead if it's not ALREADY
                // sitting on an element_end node.
                //
                // Update: Above, LoadEncodedTextField() already does this
                // (below).
                //
                //                if (!SkipAfterLoadingField(xml))
                //                { otOut << "*** %s: Bad data? Expected
                // EXN_ELEMENT_END here, but "
                //                                "didn't get it. Returning
                // false.\n", __FUNCTION__); return false; }

                return true; // <============ SUCCESS!!!!
            }
        }
        else {
            otErr << __FUNCTION__ << ": Error: missing " << pElementExpected
                  << " element.\n";
            return false; // error condition
        }
    }
    else {
        otErr << __FUNCTION__ << ": Error: Expected " << pElementExpected
              << " element with text field.\n";
        return false; // error condition
    }
}

// Make sure you escape any lines that begin with dashes using "- "
// So "---BEGIN " at the beginning of a line would change to: "- ---BEGIN"
// This function expects that's already been done.
// This function assumes there is only unsigned contents, and not a signed
// contract.
// This function is intended to PRODUCE said signed contract.
// NOTE: This function also assumes you already instantiated a contract
// of the proper type. For example, if it's an OTServerContract, then you
// INSTANTIATED an OTServerContract. Because if you tried to do this using
// an OTContract but then the strContract was for an OTServerContract, then
// this function will fail when it tries to "LoadContractFromString()" since it
// is not actually the proper type to handle those data members.
//
// Therefore I need to make an entirely different (but similar) function which
// can be used for signing a piece of unsigned XML where the actual contract
// type
// is unknown.
//
bool OTContract::CreateContract(const OTString& strContract,
                                const OTPseudonym& theSigner)
{
    Release();

    char cNewline =
        0; // this is about to contain a byte read from the end of the contract.
    const uint32_t lLength = strContract.GetLength();

    if ((3 > lLength) || !strContract.At(lLength - 1, cNewline)) {
        otErr << __FUNCTION__
              << ": Invalid input: contract is less than 3 bytes "
                 "int64_t, or unable to read a byte from the end where a "
                 "newline is meant to be.\n";
        return false;
    }

    // ADD a newline, if necessary.
    // (The -----BEGIN part needs to start on its OWN LINE...)
    //
    // If length is 10, then string goes from 0..9.
    // Null terminator will be at 10.
    // Therefore the final newline should be at 9.
    // Therefore if char_at_index[lLength-1] != '\n'
    // Concatenate one!

    if ('\n' == cNewline) // It already has a newline
        m_xmlUnsigned = strContract;
    else
        m_xmlUnsigned.Format("%s\n", strContract.Get());

    // This function assumes that m_xmlUnsigned is ready to be processed.
    // This function only processes that portion of the contract.
    //
    bool bLoaded = LoadContractXML();

    if (bLoaded) {

        // Add theSigner to the contract, if he's not already there.
        //
        if (nullptr == GetContractPublicNym()) {
            const bool bHasCredentials =
                (theSigner.GetMasterCredentialCount() > 0);

            if (!bHasCredentials) {
                OTString strPubkey;
                if (theSigner.GetPublicSignKey().GetPublicKey(
                        strPubkey) && // bEscaped=true by default.
                    strPubkey.Exists()) {
                    InsertNym("contract", strPubkey);
                }
            }
            else // theSigner has Credentials, so we'll add him to the
                   // contract.
            {
                OTString strCredList, strSignerNymID;
                OTString::Map mapCredFiles;
                theSigner.GetIdentifier(strSignerNymID);
                theSigner.GetPublicCredentials(strCredList, &mapCredFiles);

                std::unique_ptr<OTPseudonym> pNym(new OTPseudonym);

                pNym->SetIdentifier(strSignerNymID);
                pNym->SetNymIDSource(theSigner.GetNymIDSource());
                pNym->SetAltLocation(theSigner.GetAltLocation());

                if (!pNym->LoadFromString(strCredList, &mapCredFiles)) {
                    otErr << __FUNCTION__ << ": Failure loading nym "
                          << strSignerNymID << " from credential string.\n";
                }
                // Now that the Nym has been loaded up from the two strings,
                // including the list of credential IDs, and the map containing
                // the
                // credentials themselves, let's try to Verify the pseudonym. If
                // we
                // verify, then we're safe to add the Nym to the contract.
                //
                else if (!pNym->VerifyPseudonym()) {
                    otErr
                        << __FUNCTION__ << ": Loaded nym " << strSignerNymID
                        << " from credentials, but then it failed verifying.\n";
                }
                else // Okay, we loaded the Nym up from the credentials, AND
                {      // verified the Nym (including the credentials.)
                    // So let's add it to the contract...
                    // Add pNym to the contract's
                    m_mapNyms["signer"] = pNym.release();
                    // internal list of nyms.
                }
            }
        }
        // This re-writes the contract internally based on its data members,
        // similar to UpdateContents. (Except specifically intended for the
        // initial creation of the contract.)
        // Since theSigner was just added, he will be included here now as well,
        // just prior to the actual signing below.
        //
        CreateContents();

        OTPasswordData thePWData("OTContract::CreateContract needs the private "
                                 "key to sign the contract...");

        if (!SignContract(theSigner, &thePWData)) {
            otErr << __FUNCTION__ << ": SignContract failed.\n";
            return false;
        }

        SaveContract();

        OTString strTemp;
        SaveContractRaw(strTemp);

        Release();
        LoadContractFromString(strTemp); // The ultimate test is, once
                                         // we've created the serialized
                                         // string for this contract, is
                                         // to then load it up from that
                                         // string.

        OTIdentifier NEW_ID;
        CalculateContractID(NEW_ID);
        m_ID = NEW_ID;

        return true;
    }
    else
        otErr << __FUNCTION__
              << ": LoadContractXML failed. strContract contents:\n\n"
              << strContract << "\n\n";

    return false;
}

// Overrides of CreateContents call this in order to add some common internals.
//
void OTContract::CreateInnerContents()
{
    // CONDITIONS
    //
    if (!m_mapConditions.empty()) {
        m_xmlUnsigned.Concatenate("<!-- CONDITIONS -->\n\n");

        for (auto& it : m_mapConditions) {
            std::string str_condition_name = it.first;
            std::string str_condition_value = it.second;

            m_xmlUnsigned.Concatenate(
                "<condition name=\"%s\">%s</condition>\n\n",
                str_condition_name.c_str(), str_condition_value.c_str());
        }
    }

    // KEYS (old) / CREDENTIALS (new)
    //
    if (!m_mapNyms.empty()) {
        OTString strTemp;

        // KEYS  (Note: deprecated in favor of NymID source and credentials.)
        for (auto& it : m_mapNyms) {
            std::string str_name = it.first;
            OTPseudonym* pNym = it.second;
            OT_ASSERT_MSG(nullptr != pNym,
                          "1: nullptr pseudonym pointer in "
                          "OTContract::CreateInnerContents.\n");

            if (("contract" == str_name) || ("certification" == str_name) ||
                ("serverCertification" == str_name)) {
                OTString strPubkey;
                if (pNym->GetPublicSignKey().GetPublicKey(
                        strPubkey) && // bEscaped=true by default.
                    strPubkey.Exists()) {
                    strTemp.Concatenate("<key name=\"%s\">\n%s</key>\n\n",
                                        str_name.c_str(), strPubkey.Get());
                }
            }
        }

        if (strTemp.Exists()) {
            m_xmlUnsigned.Concatenate("<!-- KEYS -->\n\n%s", strTemp.Get());
            strTemp.Release();
        }

        // NEW CREDENTIALS, based on NymID and Source, and credential IDs.
        for (auto& it : m_mapNyms) {
            std::string str_name = it.first;
            OTPseudonym* pNym = it.second;
            OT_ASSERT_MSG(nullptr != pNym,
                          "2: nullptr pseudonym pointer in "
                          "OTContract::CreateInnerContents.\n");

            if ("signer" == str_name) {
                const bool bHasCredentials =
                    (pNym->GetMasterCredentialCount() > 0);

                OTString strNymID;
                pNym->GetIdentifier(strNymID);

                OTASCIIArmor ascAltLocation;
                if (pNym->GetAltLocation().Exists())
                    ascAltLocation.SetString(pNym->GetAltLocation(),
                                             false); // bLineBreaks=true by
                                                     // default. But here, no
                                                     // line breaks.

                strTemp.Concatenate("<%s hasCredentials=\"%s\"\n"
                                    " nymID=\"%s\"\n"
                                    " altLocation=\"%s\""
                                    ">\n\n",
                                    str_name.c_str(), //"signer"
                                    bHasCredentials ? "true" : "false",
                                    strNymID.Get(), ascAltLocation.Get());

                if (pNym->GetNymIDSource().Exists()) {
                    OTASCIIArmor ascNymIDSource(pNym->GetNymIDSource());
                    strTemp.Concatenate("<nymIDSource>\n%s</nymIDSource>\n\n",
                                        ascNymIDSource.Get());
                }

                // credentialList
                // credentials
                //
                if (bHasCredentials) {

                    // Create a new OTDB::StringMap object.
                    //
                    std::unique_ptr<OTDB::Storable> pStorable(
                        OTDB::CreateObject(OTDB::STORED_OBJ_STRING_MAP));
                    OTDB::StringMap* pMap = nullptr;

                    pMap =
                        (nullptr == pStorable)
                            ? nullptr
                            : dynamic_cast<OTDB::StringMap*>(pStorable.get());

                    if (nullptr == pMap)
                        otErr << __FUNCTION__ << ": Error: failed trying to "
                                                 "load or create a "
                                                 "STORED_OBJ_STRING_MAP.\n";
                    else // It instantiated.
                    {
                        OTString strCredList;
                        OTString::Map& theMap = pMap->the_map;

                        pNym->GetPublicCredentials(strCredList, &theMap);

                        // Serialize the StringMap to a string...
                        //
                        if (strCredList.Exists() &&
                            (!theMap.empty())) // Won't bother if there are
                                               // zero credentials somehow.
                        {
                            std::string str_Encoded = OTDB::EncodeObject(*pMap);
                            const bool bSuccessEncoding =
                                (str_Encoded.size() > 0);
                            if (bSuccessEncoding) {
                                OTASCIIArmor armor1(strCredList), armor2;
                                armor2.Set(str_Encoded.c_str());
                                if (armor1.Exists())
                                    strTemp.Concatenate("<credentialList>\n%s</"
                                                        "credentialList>\n\n",
                                                        armor1.Get());
                                if (armor2.Exists())
                                    strTemp.Concatenate(
                                        "<credentials>\n%s</credentials>\n\n",
                                        armor2.Get());
                            }
                        }
                    }
                }

                strTemp.Concatenate("</%s>\n\n", str_name.c_str()); //"signer"

            } // "signer"
        }

        if (strTemp.Exists()) {
            m_xmlUnsigned.Concatenate("<!-- NYMS -->\n\n%s", strTemp.Get());
            strTemp.Release();
        }

    } // if (m_mapNyms.size() > 0)
}

// Only used when first generating an asset or server contract.
// Meant for contracts which never change after that point.
// Otherwise does the same thing as UpdateContents.
// (But meant for a different purpose.)
// See OTServerContract.cpp and OTAssetContract.cpp
//
void OTContract::CreateContents()
{
    OT_FAIL_MSG("ASSERT: OTContract::CreateContents should never be called, "
                "but should be overrided. (In this case, it wasn't.)");
}

// return -1 if error, 0 if nothing, and 1 if the node was processed.
int32_t OTContract::ProcessXMLNode(IrrXMLReader*& xml)
{
    const OTString strNodeName(xml->getNodeName());

    if (strNodeName.Compare("entity")) {
        m_strEntityShortName = xml->getAttributeValue("shortname");
        if (!m_strName.Exists()) // only set it if it's not already set, since
                                 // the wallet may have already had a user label
                                 // set.
            m_strName = m_strEntityShortName; // m_strName may later be changed
                                              // again in
                                              // OTAssetContract::ProcessXMLNode

        m_strEntityLongName = xml->getAttributeValue("longname");
        m_strEntityEmail = xml->getAttributeValue("email");

        otWarn << "Loaded Entity, shortname: " << m_strEntityShortName
               << "\nLongname: " << m_strEntityLongName
               << ", email: " << m_strEntityEmail << "\n----------\n";

        return 1;
    }
    else if (strNodeName.Compare("condition")) {
        // todo security: potentially start ascii-encoding these.
        // (Are they still "human readable" if you can easily decode them?)
        //
        OTString strConditionName;
        OTString strConditionValue;

        strConditionName = xml->getAttributeValue("name");

        if (!SkipToTextField(xml)) {
            otOut << "OTContract::ProcessXMLNode: Failure: Unable to find "
                     "expected text field for xml node named: "
                  << xml->getNodeName() << "\n";
            return (-1); // error condition
        }

        if (EXN_TEXT == xml->getNodeType()) {
            strConditionValue = xml->getNodeData();
        }
        else {
            otErr << "Error in OTContract::ProcessXMLNode: Condition without "
                     "value: " << strConditionName << "\n";
            return (-1); // error condition
        }

        // Add the conditions to a list in memory on this object.
        //
        m_mapConditions.insert(std::pair<std::string, std::string>(
            strConditionName.Get(), strConditionValue.Get()));

        otWarn << "---- Loaded condition \"" << strConditionName << "\"\n";
        //        otWarn << "Loading condition \"%s\": %s----------(END
        // DATA)----------\n", strConditionName.Get(),
        //                strConditionValue.Get());

        return 1;
    }
    else if (strNodeName.Compare("signer")) {
        const OTString strSignerNymID = xml->getAttributeValue("nymID");
        const OTString strHasCredentials =
            xml->getAttributeValue("hasCredentials");
        const OTASCIIArmor ascAltLocation =
            xml->getAttributeValue("altLocation");
        OTString strAltLocation, strSignerSource;

        if (ascAltLocation.Exists())
            ascAltLocation.GetString(strAltLocation,
                                     false); // bLineBreaks=true by default.

        bool bHasCredentials = strHasCredentials.Compare("true");
        const bool bHasAltLocation = strAltLocation.Exists();

        if (!strSignerNymID.Exists()) {
            otErr << "Error in " << __FUNCTION__
                  << ": "
                     "Expected nymID attribute on signer element.\n";
            return (-1); // error condition
        }
        OTASCIIArmor ascArmor;  // For credential list.
        OTASCIIArmor ascArmor2; // For credentials.

        const char* pElementExpected = "nymIDSource";
        otWarn << __FUNCTION__ << ": Loading " << pElementExpected << "...\n";
        if (!OTContract::LoadEncodedTextFieldByName(xml, strSignerSource,
                                                    pElementExpected)) {
            otErr << "Error in " << __FILE__ << " line " << __LINE__
                  << ": failed loading expected " << pElementExpected
                  << " field:\n\n" << m_xmlUnsigned << "\n\n\n";
            return (-1); // error condition
        }
        // TODO: hash the source right here and compare it to the NymID, just to
        // be safe.

        if (!bHasCredentials) {
            // If there are no credentials provided (which is proper) then we
            // should
            // just download them from the source.
            // ...Unless it's one of those where you can't discover such things
            // from the source,
            // in which case an alternate location must be provided.
            //
            if (bHasAltLocation) {
                otErr << __FUNCTION__
                      << ": WARNING: No credentials provided. An alternate "
                         "location is "
                         "listed, but that's not yet supported in the "
                         "code.\nLocation: " << strAltLocation << "\n";

                // A signer ideally just has a NymID and source.
                // Then we can directly just download the credentials from the
                // source.
                // But let's say the source doesn't include download info (like
                // if it contains DN info.)
                // We can have this optional attribute "altLocation" for the
                // alternate download location.
                // We can also optionally allow people to just directly put the
                // credentials inside the
                // contract (credentialList, and credentials). That's why
                // hasCredentials can be true or false.
                // Ideally, people will not do that. Instead, we can download
                // them from the source, or from
                // the alternate location, if the source cannot supply. But
                // worst case, they can directly embed
                // the credentials, though it's not best practice for a real
                // contract, it can be useful for
                // testing.
                //
                // If we eventually add the download code here, put the
                // credential list into ascArmor,
                // and the credentials into ascArmor2.
            }
            else // There's no alternate location, and no credentials
                   // provided,
            { // Therefore we be must expected to download them based on the
                // source
                // string, and if we can't, then we've failed to load.
                //
                otErr << __FUNCTION__
                      << ": WARNING: Alternate location not listed, and no "
                         "credentials provided, so we need to download"
                         " them from the source--but that's not yet supported "
                         "in the code.\nNymID Source String: "
                      << strSignerSource << "\n";
                //
                // If we eventually add the download code here, put the
                // credential list into ascArmor,
                // and the credentials into ascArmor2.
            }
            return (-1); // for now, since this block is incomplete.
        }
        else           // (bHasCredentials)
        {
            pElementExpected = "credentialList";

            if (!OTContract::LoadEncodedTextFieldByName(xml, ascArmor,
                                                        pElementExpected)) {
                otErr << "Error in " << __FUNCTION__ << ": "
                                                        "Expected "
                      << pElementExpected << " element with text field.\n";
                return (-1); // error condition
            }

            pElementExpected = "credentials";

            if (!OTContract::LoadEncodedTextFieldByName(xml, ascArmor2,
                                                        pElementExpected)) {
                otErr << "Error in " << __FUNCTION__ << ": "
                                                        "Expected "
                      << pElementExpected << " element with text field.\n";
                return (-1); // error condition
            }
        }

        bHasCredentials = (ascArmor.Exists() && ascArmor2.Exists());

        // bHasCredentials might have gotten set to true in the block above the
        // above block,
        // after downloading, checking alternate location, etc. Otherwise, in
        // the above block,
        // it was loaded from the contract itself.
        if (bHasCredentials) {
            // credentialList
            //
            OTString strCredentialList;
            ascArmor.GetString(strCredentialList);

            if (strCredentialList.Exists()) {
                std::unique_ptr<OTDB::Storable> pStorable(OTDB::DecodeObject(
                    OTDB::STORED_OBJ_STRING_MAP, ascArmor2.Get()));
                OTDB::StringMap* pMap =
                    (nullptr == pStorable)
                        ? nullptr
                        : dynamic_cast<OTDB::StringMap*>(pStorable.get());

                if (nullptr == pMap)
                    otOut << __FUNCTION__
                          << ": Failed decoding StringMap object.\n";
                else // IF the list saved, then we save the credentials
                     // themselves...
                {
                    OTString::Map& theMap = pMap->the_map;

                    std::unique_ptr<OTPseudonym> pNym(new OTPseudonym);
                    pNym->SetIdentifier(strSignerNymID);

                    if (false ==
                        pNym->LoadFromString(strCredentialList, &theMap)) {
                        otErr << __FUNCTION__ << ": Failure loading nym "
                              << strSignerNymID << " from credential string.\n";
                    }
                    // Now that the Nym has been loaded up from the two strings,
                    // including the list of credential IDs, and the map
                    // containing the
                    // credentials themselves, let's try to Verify the
                    // pseudonym. If we
                    // verify, then we're safe to add the Nym to the contract.
                    //
                    else if (!pNym->VerifyPseudonym()) {
                        otErr << __FUNCTION__ << ": Loaded nym "
                              << strSignerNymID
                              << " from credentials, but then it failed "
                                 "verifying.\n";
                    }
                    else // Okay, we loaded the Nym up from the credentials in
                           // the contract, AND
                    {      // verified the Nym (including the credentials.)
                        // So let's add it to the contract...
                        //

                        m_mapNyms[strNodeName.Get() /*"signer"*/] =
                            pNym.release();
                        // Add pNym to the contract's internal list of nyms.

                        return 1; // <==== Success!
                    }
                }
            } // credential list exists, after base64-decoding.

        } // Has Credentials.

        return (-1);
    }
    else if (strNodeName.Compare("key")) {
        OTString strKeyName;
        OTString strKeyValue;

        strKeyName = xml->getAttributeValue("name");

        if (!SkipToTextField(xml)) {
            otOut << "OTContract::ProcessXMLNode: Failure: Unable to find "
                     "expected text "
                     "field for xml node named: " << xml->getNodeName() << "\n";
            return (-1); // error condition
        }

        if (EXN_TEXT == xml->getNodeType()) {
            strKeyValue = xml->getNodeData();
        }
        else {
            otErr << "Error in OTContract::ProcessXMLNode: Key without value: "
                  << strKeyName << "\n";
            return (-1); // error condition
        }
        // Create a new nym for this public key (or x509 cert, it works either
        // way)
        // and add it to the contract's internal list of nyms.
        //
        // Later on, if someone needs to get the Public Key for this contract
        // and
        // use it to verify the signature on the contract, or to verify a
        // message
        // from the server, the contract can iterate the list of Nyms and get
        // the public contract key based on a standard name such as the
        // "contract" key.
        // (Versus the "server" key or the "certification" key, etc.

        if (!InsertNym(strKeyName, strKeyValue))
            otErr << "Error performing OTContract::InsertNym.\n";

        return 1;
    }

    return 0;
}

// If you have a Public Key or Cert that you would like to add as one of the
// keys on this contract,
// just call this function. Usually you'd never want to do that because you
// would never want to actually
// change the text of the contract (or the signatures will stop verifying.)
// But in unique situations, for example when first creating a contract, you
// might want to insert some
// keys into it. You might also call this function when LOADING the contract, to
// populate it.
//
bool OTContract::InsertNym(const OTString& strKeyName,
                           const OTString& strKeyValue)
{
    bool bResult = false;
    OTPseudonym* pNym = new OTPseudonym;

    OT_ASSERT_MSG(
        nullptr != pNym,
        "Error allocating memory for new Nym in OTContract::InsertNym\n");

    // This is the version of SetCertificate that handles escaped bookends. ( -
    // -----BEGIN CERTIFICATE-----)
    if (strKeyValue.Contains("CERTIFICATE") &&
        pNym->SetCertificate(strKeyValue,
                             true)) // it also defaults to true, FYI.
    {
        m_mapNyms[strKeyName.Get()] = pNym;
        pNym->SetIdentifierByPubkey();
        otWarn << "---- Loaded certificate \"" << strKeyName << "\"\n";
        bResult = true;
    }
    else if (strKeyValue.Contains("PUBLIC KEY") &&
               pNym->SetPublicKey(strKeyValue,
                                  true)) // it also defaults to true, FYI.
    {
        m_mapNyms[strKeyName.Get()] = pNym;
        pNym->SetIdentifierByPubkey();
        otWarn << "---- Loaded public key \"" << strKeyName << "\"\n";
        bResult = true;
    }
    else {
        delete pNym;
        pNym = nullptr;
        otOut << "\nLoaded key \"" << strKeyName
              << "\" but FAILED adding the"
                 " Nym to the Contract:\n--->" << strKeyValue << "<---\n";
    }

    return bResult;
}

} // namespace opentxs
