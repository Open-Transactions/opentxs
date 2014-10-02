/************************************************************
 *
 *  Token.cpp
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
#include <opentxs/cash/Token.hpp>
#include <opentxs/cash/Mint.hpp>
#include <opentxs/cash/Purse.hpp>
#include <opentxs/cash/TokenLucre.hpp>
#include <opentxs/core/crypto/OTEnvelope.hpp>
#include <opentxs/core/crypto/OTNymOrSymmetricKey.hpp>
#include <opentxs/core/util/OTFolders.hpp>
#include <opentxs/core/OTLog.hpp>
#include <opentxs/core/OTStorage.hpp>

#include <irrxml/irrXML.hpp>

namespace opentxs
{

// The current implementation for withdrawals (using Lucre) requires only a
// single proto-token
// to be sent, signed, and returned. Only the ID of the token is blinded.
//
// But this library supports sending up to N proto-tokens. Even though only 1 is
// required, this
// lib supports sending 5 or 100 or 1000, if other protocols (such as Cham) are
// later added.

// todo: make this configurable. Or configured in the contract.
// Then the server can be configured with the contract parameters that it is
// willing to accept.
// Each server operator may have different standards about the contracts they
// are willing to
// process, and the prices for notarizing each.
//
// You may want to set it up as 1 out of 100
// or 1 out of 500
// or 1 out of 5
// Basically this number determines how many blinded prototokens must be sent to
// the
// server in order for the server to accept the withdrawal request and sign one
// of them.
// (more prototokens == more resource cost, but more security.)
const int32_t Token__nMinimumPrototokenCount = 1;

int32_t Token::GetMinimumPrototokenCount()
{
    return Token__nMinimumPrototokenCount;
}

// Lucre, in fact, only sends a single blinded token, and the bank signs it
// blind and returns it.
// With Chaum, I thought the bank had to open some of the proto-tokens to verify
// the amount was
// correct, etc. But now I realize that was probably just a metaphor used for
// news interviews.
//
// With Lucre, only the ID is blinded. The bank can already see the amount--it's
// not blinded. So
// there's no need to verify it.  The client can send an ill-formed token if he
// wishes, but only
// hurts himself.
//
// Problem is, the bank can still falsely refuse a coin. So I have wrapped Lucre
// in my own protocol
// which includes signed receipts from the bank. Also, since the bank must store
// the spent tokens
// (at least until they expire) then the bank can be asked to produce the
// deposit receipt which
// has the customer's signature on it who deposited that cash, if it indeed has
// already been spent.

void Token::InitToken()
{
    //    m_lDenomination    = 0;
    //    m_nTokenCount    = 0;
    //    m_nChosenIndex    = 0;
    //    m_nSeries        = 0;
    //    m_State            = blankToken;
    //    m_bSavePrivateKeys = false;

    m_strContractType.Set("CASH TOKEN"); // todo internationalization.
}

Token::Token()
    : ot_super()
    , m_bPasswordProtected(false)
    , m_lDenomination(0)
    , m_nTokenCount(0)
    , m_nChosenIndex(0)
    , m_nSeries(0)
    , m_State(blankToken)
    , m_bSavePrivateKeys(false)
{
    InitToken();
}

// int64_t                m_lDenomination;
// int32_t                m_nTokenCount;
// int32_t                m_nChosenIndex;
// int32_t                m_nSeries;
// tokenState        m_State;
// bool                m_bSavePrivateKeys; // Determines whether it serializes
// private keys 1 time (yes if true)

Token::Token(const OTIdentifier& SERVER_ID, const OTIdentifier& ASSET_ID)
    : ot_super(SERVER_ID, ASSET_ID)
    , m_bPasswordProtected(false)
    , m_lDenomination(0)
    , m_nTokenCount(0)
    , m_nChosenIndex(0)
    , m_nSeries(0)
    , m_State(blankToken)
    , m_bSavePrivateKeys(false)
{
    InitToken();

    // m_ServerID and m_AssetTypeID are now in the parent class (OTInstrument)
    // So they are initialized there now.
}

Token::Token(const Purse& thePurse)
    : ot_super()
    , m_bPasswordProtected(false)
    , m_lDenomination(0)
    , m_nTokenCount(0)
    , m_nChosenIndex(0)
    , m_nSeries(0)
    , m_State(blankToken)
    , m_bSavePrivateKeys(false)
{
    InitToken();

    // These are in the parent class, OTInstrument.
    // I set them here because the "Purse" argument only exists
    // in this subclass constructor, not the base.
    m_ServerID = thePurse.GetServerID();
    m_AssetTypeID = thePurse.GetAssetID();
}

void Token::Release_Token()
{

    m_Signature.Release();
    m_ascSpendable.Release();

    //  InitToken();

    ReleasePrototokens();
}

void Token::Release()
{
    Release_Token();

    ot_super::Release(); // since I've overridden the base class, I call it
                         // now...
}

Token::~Token()
{
    Release_Token();

    m_bPasswordProtected = false;

    // todo: optimization (probably just remove these here.)
    m_lDenomination = 0;
    //    m_nTokenCount    = 0;  // this happens in ReleasePrototokens. (Called
    // by Release_Token above.)
    m_nChosenIndex = 0;
    m_nSeries = 0;
    m_State = blankToken;
    m_bSavePrivateKeys = false;
}

void Token::ReleasePrototokens()
{
    for (auto& it : m_mapPublic) {
        OTASCIIArmor* pPrototoken = it.second;
        OT_ASSERT_MSG(
            nullptr != pPrototoken,
            "nullptr OTASCIIArmor pointer in Token::ReleasePrototokens.");

        delete pPrototoken;
        pPrototoken = nullptr;
    }

    for (auto& it : m_mapPrivate) {
        OTASCIIArmor* pPrototoken = it.second;
        OT_ASSERT_MSG(
            nullptr != pPrototoken,
            "nullptr OTASCIIArmor pointer in Token::ReleasePrototokens.");

        delete pPrototoken;
        pPrototoken = nullptr;
    }

    m_mapPublic.clear();
    m_mapPrivate.clear();

    m_nTokenCount = 0;
}

// static -- class factory.
//
Token* Token::LowLevelInstantiate(const OTString& strFirstLine,
                                  const OTIdentifier& SERVER_ID,
                                  const OTIdentifier& ASSET_ID)
{
    Token* pToken = nullptr;

    if (strFirstLine.Contains("-----BEGIN SIGNED CASH-----")) // this string is
                                                              // 27 chars long.
    {
        pToken = new Token_Lucre(SERVER_ID, ASSET_ID);
        OT_ASSERT(nullptr != pToken);
    }
    else if (strFirstLine.Contains(
                   "-----BEGIN SIGNED CASH TOKEN-----")) // this string is 33
                                                         // chars long.
    {
        pToken = new Token_Lucre(SERVER_ID, ASSET_ID);
        OT_ASSERT(nullptr != pToken);
    }
    else if (strFirstLine.Contains(
                   "-----BEGIN SIGNED LUCRE CASH TOKEN-----")) // this string is
                                                               // 39 chars long.
    {
        pToken = new Token_Lucre(SERVER_ID, ASSET_ID);
        OT_ASSERT(nullptr != pToken);
    }

    return pToken;
}

Token* Token::LowLevelInstantiate(const OTString& strFirstLine,
                                  const Purse& thePurse)
{
    Token* pToken = nullptr;

    if (strFirstLine.Contains("-----BEGIN SIGNED CASH-----")) // this string is
                                                              // 27 chars long.
    {
        pToken = new Token_Lucre(thePurse);
        OT_ASSERT(nullptr != pToken);
    }
    else if (strFirstLine.Contains(
                   "-----BEGIN SIGNED CASH TOKEN-----")) // this string is 33
                                                         // chars long.
    {
        pToken = new Token_Lucre(thePurse);
        OT_ASSERT(nullptr != pToken);
    }
    else if (strFirstLine.Contains(
                   "-----BEGIN SIGNED LUCRE CASH TOKEN-----")) // this string is
                                                               // 39 chars long.
    {
        pToken = new Token_Lucre(thePurse);
        OT_ASSERT(nullptr != pToken);
    }
    otErr << __FUNCTION__ << ": Open-Transactions is not built for any digital "
                             "cash algorithms. (Failure.)";

    return pToken;
}

Token* Token::LowLevelInstantiate(const Purse& thePurse)
{
    Token* pToken = nullptr;

    pToken = new Token_Lucre(thePurse);
    OT_ASSERT(nullptr != pToken);
    return pToken;
}

Token* Token::LowLevelInstantiate(const OTString& strFirstLine)
{
    Token* pToken = nullptr;

    if (strFirstLine.Contains("-----BEGIN SIGNED CASH-----")) // this string is
                                                              // 27 chars long.
    {
        pToken = new Token_Lucre;
        OT_ASSERT(nullptr != pToken);
    }
    else if (strFirstLine.Contains(
                   "-----BEGIN SIGNED CASH TOKEN-----")) // this string is 33
                                                         // chars long.
    {
        pToken = new Token_Lucre;
        OT_ASSERT(nullptr != pToken);
    }
    else if (strFirstLine.Contains(
                   "-----BEGIN SIGNED LUCRE CASH TOKEN-----")) // this string is
                                                               // 39 chars long.
    {
        pToken = new Token_Lucre;
        OT_ASSERT(nullptr != pToken);
    }
    otErr << __FUNCTION__ << ": Open-Transactions is not built for any digital "
                             "cash algorithms. (Failure.)";

    return pToken;
}

// static -- class factory.
//
Token* Token::TokenFactory(OTString strInput, const OTIdentifier& SERVER_ID,
                           const OTIdentifier& ASSET_ID)
{
    //  const char * szFunc = "Token::TokenFactory";

    OTString strContract, strFirstLine; // output for the below function.
    const bool bProcessed =
        OTContract::DearmorAndTrim(strInput, strContract, strFirstLine);

    if (bProcessed) {
        Token* pToken =
            Token::LowLevelInstantiate(strFirstLine, SERVER_ID, ASSET_ID);

        // The string didn't match any of the options in the factory.
        if (nullptr == pToken) return nullptr;

        // Does the contract successfully load from the string passed in?
        if (pToken->LoadContractFromString(strContract))
            return pToken;
        else
            delete pToken;
    }

    return nullptr;
}

Token* Token::TokenFactory(OTString strInput, const Purse& thePurse)
{
    //  const char * szFunc = "Token::TokenFactory";

    OTString strContract, strFirstLine; // output for the below function.
    const bool bProcessed =
        OTContract::DearmorAndTrim(strInput, strContract, strFirstLine);

    if (bProcessed) {
        Token* pToken = Token::LowLevelInstantiate(strFirstLine, thePurse);

        // The string didn't match any of the options in the factory.
        if (nullptr == pToken) return nullptr;

        // Does the contract successfully load from the string passed in?
        if (pToken->LoadContractFromString(strContract))
            return pToken;
        else
            delete pToken;
    }

    return nullptr;
}

Token* Token::TokenFactory(OTString strInput)
{
    //  const char * szFunc = "Token::TokenFactory";

    OTString strContract, strFirstLine; // output for the below function.
    const bool bProcessed =
        OTContract::DearmorAndTrim(strInput, strContract, strFirstLine);

    if (bProcessed) {
        Token* pToken = Token::LowLevelInstantiate(strFirstLine);

        // The string didn't match any of the options in the factory.
        if (nullptr == pToken) return nullptr;

        // Does the contract successfully load from the string passed in?
        if (pToken->LoadContractFromString(strContract))
            return pToken;
        else
            delete pToken;
    }

    return nullptr;
}

// Note: ALL failures will return true, even if the token has NOT already been
// spent, and the failure was actually due to a directory creation error. Why,
// you might ask? Because no matter WHAT is causing the failure, any return of
// false is a signal that the token is SAFE TO ACCEPT AS TENDER. If there was a
// temporary file system error, someone could suddenly deposit the same token
// over and over again and this method would return "false" (Token is "not
// already
// spent.")
//
// We simply cannot risk that, so false is not returned unless execution reaches
// the very bottom of this method. Every other error acts as if the token is
// no good, for security reasons. If the token really IS good, the user can
// submit
// it again later and it will work.
//
bool Token::IsTokenAlreadySpent(OTString& theCleartextToken)
{
    OTString strAssetID(GetAssetID());

    // Calculate the filename (a hash of the Lucre cleartext token ID)
    OTIdentifier theTokenHash;
    theTokenHash.CalculateDigest(theCleartextToken);

    // Grab the new hash into a string (for use as a filename)
    OTString strTokenHash(theTokenHash);

    OTString strAssetFolder;
    strAssetFolder.Format("%s.%d", strAssetID.Get(), GetSeries());

    bool bTokenIsPresent = OTDB::Exists(
        OTFolders::Spent().Get(), strAssetFolder.Get(), strTokenHash.Get());

    if (bTokenIsPresent) {
        otOut << "\nToken::IsTokenAlreadySpent: Token was already spent: "
              << OTFolders::Spent() << OTLog::PathSeparator() << strAssetFolder
              << OTLog::PathSeparator() << strTokenHash << "\n";
        return true; // all errors must return true in this function.
                     // But this is not an error. Token really WAS already
    }                // spent, and this true is for real. The others are just
                     // for security reasons because of this one.

    // This is the ideal case: the token was NOT already spent, it was good,
    // so we can return false and the depositor can be credited appropriately.
    // IsTokenAlreadySpent?  NO-it was NOT already spent. You can only POSSIBLY
    // get a false out of this method if you actually reached the bottom (here.)
    return false;
}

bool Token::RecordTokenAsSpent(OTString& theCleartextToken)
{
    OTString strAssetID(GetAssetID());

    // Calculate the filename (a hash of the Lucre cleartext token ID)
    OTIdentifier theTokenHash;
    theTokenHash.CalculateDigest(theCleartextToken);

    // Grab the new hash into a string (for use as a filename)
    OTString strTokenHash(theTokenHash);

    OTString strAssetFolder;
    strAssetFolder.Format("%s.%d", strAssetID.Get(), GetSeries());

    // See if the spent token file ALREADY EXISTS...
    bool bTokenIsPresent = OTDB::Exists(
        OTFolders::Spent().Get(), strAssetFolder.Get(), strTokenHash.Get());

    // If so, we're trying to record a token that was already recorded...
    if (bTokenIsPresent) {
        otErr << "Token::RecordTokenAsSpent: Trying to record token as spent,"
                 " but it was already recorded: " << OTFolders::Spent()
              << OTLog::PathSeparator() << strAssetFolder
              << OTLog::PathSeparator() << strTokenHash << "\n";
        return false;
    }

    // FINISHED:
    //
    // We actually save the token itself into the file, which is named based
    // on a hash of the Lucre data.
    // The success of that operation is also now the success of this one.

    OTString strFinal;
    OTASCIIArmor ascTemp(m_strRawFile);

    if (false ==
        ascTemp.WriteArmoredString(strFinal, m_strContractType.Get())) {
        otErr << "Token::RecordTokenAsSpent: Error recording token as "
                 "spent (failed writing armored string):\n"
              << OTFolders::Spent() << OTLog::PathSeparator() << strAssetFolder
              << OTLog::PathSeparator() << strTokenHash << "\n";
        return false;
    }

    const bool bSaved =
        OTDB::StorePlainString(strFinal.Get(), OTFolders::Spent().Get(),
                               strAssetFolder.Get(), strTokenHash.Get());
    if (!bSaved) {
        otErr << "Token::RecordTokenAsSpent: Error saving file: "
              << OTFolders::Spent() << OTLog::PathSeparator() << strAssetFolder
              << OTLog::PathSeparator() << strTokenHash << "\n";
    }

    return bSaved;
}

// OTSymmetricKey:
// static bool CreateNewKey(OTString & strOutput, const OTString *
// pstrDisplay=nullptr, const OTPassword * pAlreadyHavePW=nullptr);
//
// static bool Encrypt(const OTString & strKey,
//                    const OTString & strPlaintext, OTString & strOutput, const
// OTString * pstrDisplay=nullptr,
//                    const bool       bBookends=true, const OTPassword *
// pAlreadyHavePW=nullptr);
//
// static bool Decrypt(const OTString & strKey, OTString & strCiphertext,
//                    OTString & strOutput, const OTString *
// pstrDisplay=nullptr,
// const OTPassword * pAlreadyHavePW=nullptr);

// OTEnvelope:
// bool Encrypt(const OTString & theInput,        OTSymmetricKey & theKey, const
// OTPassword & thePassword);
// bool Decrypt(      OTString & theOutput, const OTSymmetricKey & theKey, const
// OTPassword & thePassword);

// OTNym_or_SymmetricKey:
// const OTPseudonym    * GetNym()      const { return m_pNym;      }
// const OTSymmetricKey * GetKey()      const { return m_pKey;      }
// const OTPassword     * GetPassword() const { return m_pPassword; } // for
// symmetric key (optional)
// bool  IsNym()       const { return (nullptr != m_pNym);      }
// bool  IsKey()       const { return (nullptr != m_pKey);      }
// bool  HasPassword() const { return (nullptr != m_pPassword); } // for
// symmetric
// key (optional)

/*
 NOTE: OTNym_or_SymmetricKey is passed in here as a reference.
 Normally, you might pass in a Nym, or a SymmetricKey, and OTNym_or_SymmetricKey
 is able to construct itself from either one. This can be convenient. However,
 if
 you don't use an OTPassword when you construct the OTNym_or_SymmetricKey, and
 it needs
 one internally for its symmetric key, then it will create one and store it, and
 delete it
 upon destruction.
 Therefore it can be useful to pass the SAME OTNym_or_SymmetricKey into a
 function multiple
 times (say, during a loop) since it is storing its password internally, and
 this makes that PW
 available to every call, without having to create it EACH TIME (forcing user to
 enter passphrase
 EACH TIME as well...)
 So... what if you would rather just instantiate the passphrase at a higher
 level, and then just
 pass it in to this function each time? That way you get the same functionality,
 but WITHOUT forcing
 the caller to instantiate the OTNym_or_SymmetricKey himself unless he has to do
 so. Otherwise he could
 just pass a Nym. (This isn't currently possible since I'm passing a reference.)
 You can still actually instantiate the passphrase at a higher level, and then
 just use that each time
 you call Token::ReassignOwnership (instantiating a OTNym_or_SymmetricKey to
 call it, and passing in
 the existing passphrase pointer to it on construction.)
 Therefore: I'm leaving the reference. In most cases, I would remove it. But
 since ReassignOwnership has
 such a specific "doing these in a loop" use-case, then might as well just
 instantiate the OTNym_or_SymmetricKey
 once (in the caller) and then just pass the same one in here a bunch of times,
 without having to construct
 anything each time.
 */
bool Token::ReassignOwnership(
    OTNym_or_SymmetricKey& oldOwner, // must be private, if a Nym.
    OTNym_or_SymmetricKey& newOwner) // can be public, if a Nym.
{
    const char* szFunc = "Token::ReassignOwnership";
    const OTString strDisplay(szFunc);

    bool bSuccess = true;

    if (!oldOwner.CompareID(newOwner)) // only re-assign if they don't ALREADY
                                       // have the same owner.
    {
        OTEnvelope theEnvelope(m_ascSpendable);
        OTString theString; // output from opening/decrypting (and eventually
                            // input for sealing/encrypting) envelope.

        // Remember, OTPurse can store its own internal symmetric key, for cases
        // where the purse is "password protected" instead of belonging to a
        // specific Nym.
        // Therefore the old or new "owner" might actually be a symmetric key.
        // Decrypt/Open the Envelope into theString
        //
        bSuccess =
            oldOwner.Open_or_Decrypt(theEnvelope, theString, &strDisplay);
        if (bSuccess) {
            OTEnvelope theNewEnvelope;
            bSuccess = newOwner.Seal_or_Encrypt(theNewEnvelope, theString,
                                                &strDisplay);
            if (bSuccess)
                bSuccess = theNewEnvelope.GetAsciiArmoredData(m_ascSpendable);
        }
    }
    return bSuccess;
}

bool Token::GetSpendableString(OTNym_or_SymmetricKey theOwner,
                               OTString& theString) const
{
    const char* szFunc = "Token::GetSpendableString";

    if (m_ascSpendable.Exists()) {
        OTEnvelope theEnvelope(m_ascSpendable);

        // Decrypt the Envelope into strContents
        const OTString strDisplay(szFunc);

        if (theOwner.Open_or_Decrypt(theEnvelope, theString, &strDisplay))
            return true;
    }
    else
        otErr << szFunc << ": m_ascSpendable is empty... (failure.)\n";

    return false;
}

void Token::UpdateContents()
{
    if (m_State == Token::spendableToken) m_strContractType.Set("CASH TOKEN");

    OTString ASSET_TYPE_ID(m_AssetTypeID), SERVER_ID(m_ServerID);

    OTString strState;
    switch (m_State) {
    case Token::blankToken:
        strState.Set("blankToken");
        break;
    case Token::protoToken:
        strState.Set("protoToken");
        break;
    case Token::signedToken:
        strState.Set("signedToken");
        break;
    case Token::spendableToken:
        strState.Set("spendableToken");
        break;
    case Token::verifiedToken:
        strState.Set("verifiedToken");
        break;
    default:
        strState.Set("errorToken");
        break;
    }

    int64_t lFrom = OTTimeGetSecondsFromTime(m_VALID_FROM);
    int64_t lTo = OTTimeGetSecondsFromTime(m_VALID_TO);

    // I release this because I'm about to repopulate it.
    m_xmlUnsigned.Release();

    m_xmlUnsigned.Concatenate("<?xml version=\"%s\"?>\n\n", "1.0");

    m_xmlUnsigned.Concatenate(
        "<token\n version=\"%s\"\n state=\"%s\"\n denomination=\"%lld\"\n"
        " assetTypeID=\"%s\"\n"
        " serverID=\"%s\"\n"
        " series=\"%d\"\n"
        " validFrom=\"%" PRId64 "\"\n"
        " validTo=\"%" PRId64 "\""
        " >\n\n",
        m_strVersion.Get(), strState.Get(), GetDenomination(),
        ASSET_TYPE_ID.Get(), SERVER_ID.Get(), m_nSeries, lFrom, lTo);

    // signed tokens, as well as spendable tokens, both carry a TokenID
    // (The spendable token contains the unblinded version.)
    if (Token::signedToken == m_State || Token::spendableToken == m_State) {
        m_xmlUnsigned.Concatenate("<tokenID>\n%s</tokenID>\n\n",
                                  m_ascSpendable.Get());
    }

    // Only signedTokens carry the signature, which is discarded in spendable
    // tokens.
    // (Because it is not used past the unblinding stage anyway, and because it
    // could
    // be used to track the token.)
    if (Token::signedToken == m_State) {
        m_xmlUnsigned.Concatenate("<tokenSignature>\n%s</tokenSignature>\n\n",
                                  m_Signature.Get());
    }

    if ((Token::protoToken == m_State || Token::signedToken == m_State) &&
        m_nTokenCount) {
        m_xmlUnsigned.Concatenate(
            "<protopurse count=\"%d\" chosenIndex=\"%d\">\n\n", m_nTokenCount,
            m_nChosenIndex);

        for (auto& it : m_mapPublic) {
            OTASCIIArmor* pPrototoken = it.second;
            OT_ASSERT(nullptr != pPrototoken);

            m_xmlUnsigned.Concatenate("<prototoken>\n%s</prototoken>\n\n",
                                      pPrototoken->Get());
        }
        m_xmlUnsigned.Concatenate("</protopurse>\n\n");
    }

    if (m_bSavePrivateKeys) {
        m_bSavePrivateKeys = false; // set it back to false;

        m_xmlUnsigned.Concatenate("<privateProtopurse>\n\n");

        for (auto& it : m_mapPrivate) {
            OTASCIIArmor* pPrototoken = it.second;
            OT_ASSERT(nullptr != pPrototoken);

            m_xmlUnsigned.Concatenate(
                "<privatePrototoken>\n%s</privatePrototoken>\n\n",
                pPrototoken->Get());
        }
        m_xmlUnsigned.Concatenate("</privateProtopurse>\n\n");
    }

    m_xmlUnsigned.Concatenate("</token>\n");
}

// return -1 if error, 0 if nothing, and 1 if the node was processed.
int32_t Token::ProcessXMLNode(irr::io::IrrXMLReader*& xml)
{
    static int32_t nPublicTokenCount = 0;
    static int32_t nPrivateTokenCount = 0;

    int32_t nReturnVal = 0;

    const OTString strNodeName(xml->getNodeName());

    // Here we call the parent class first.
    // If the node is found there, or there is some error,
    // then we just return either way.  But if it comes back
    // as '0', then nothing happened, and we'll continue executing.
    //
    // -- Note you can choose not to call the parent if
    // you don't want to use any of those xml tags.
    // As I do below, in the case of OTAccount.
    // if (nReturnVal = OTContract::ProcessXMLNode(xml))
    //    return nReturnVal;

    if (strNodeName.Compare("token")) {
        OTString strState;

        m_strVersion = xml->getAttributeValue("version");
        strState = xml->getAttributeValue("state");

        m_nSeries = atoi(xml->getAttributeValue("series"));

        const OTString str_from = xml->getAttributeValue("validFrom");
        const OTString str_to = xml->getAttributeValue("validTo");

        int64_t tFrom = str_from.ToLong();
        int64_t tTo = str_to.ToLong();

        m_VALID_FROM = OTTimeGetTimeFromSeconds(tFrom);
        m_VALID_TO = OTTimeGetTimeFromSeconds(tTo);

        SetDenomination(atol(xml->getAttributeValue("denomination")));

        if (strState.Compare("blankToken"))
            m_State = Token::blankToken;
        else if (strState.Compare("protoToken"))
            m_State = Token::protoToken;
        else if (strState.Compare("signedToken"))
            m_State = Token::signedToken;
        else if (strState.Compare("spendableToken"))
            m_State = Token::spendableToken;
        else if (strState.Compare("verifiedToken"))
            m_State = Token::verifiedToken;
        else
            m_State = Token::errorToken;

        if (m_State == Token::spendableToken)
            m_strContractType.Set("CASH TOKEN");

        OTString strAssetTypeID(xml->getAttributeValue("assetTypeID")),
            strServerID(xml->getAttributeValue("serverID"));

        m_AssetTypeID.SetString(strAssetTypeID);
        m_ServerID.SetString(strServerID);

        otLog4 <<
            //    "\n===> Loading XML for token into memory structures..."
            "\n\nToken State: " << strState
               << "\n Denomination: " << GetDenomination()
               << "\n"
                  " AssetTypeID: " << strAssetTypeID
               << "\nServerID: " << strServerID << "\n";

        nReturnVal = 1;
    }
    else if (strNodeName.Compare("tokenID")) {
        if (!OTContract::LoadEncodedTextField(xml, m_ascSpendable)) {
            otErr << "Error in Token::ProcessXMLNode: token ID without "
                     "value.\n";
            return (-1); // error condition
        }

        return 1;
    }
    else if (strNodeName.Compare("tokenSignature")) {
        if (!OTContract::LoadEncodedTextField(xml, m_Signature)) {
            otErr << "Error in Token::ProcessXMLNode: token Signature "
                     "without value.\n";
            return (-1); // error condition
        }

        return 1;
    }
    else if (strNodeName.Compare("protopurse")) { // TODO for security, if the
                                                    // count here doesn't match
                                                    // what's loaded up, that
                                                    // should be part of
        // what is verified in each token when it's verified..
        m_nTokenCount = atoi(xml->getAttributeValue("count"));
        m_nChosenIndex = atoi(xml->getAttributeValue("chosenIndex"));

        nPublicTokenCount = 0;

        return 1;
    }
    else if (strNodeName.Compare("prototoken")) {
        OTASCIIArmor* pArmoredPrototoken = new OTASCIIArmor;
        OT_ASSERT(nullptr != pArmoredPrototoken);

        if (!OTContract::LoadEncodedTextField(xml, *pArmoredPrototoken) ||
            !pArmoredPrototoken->Exists()) {
            otErr << "Error in Token::ProcessXMLNode: prototoken field "
                     "without value.\n";

            delete pArmoredPrototoken;
            pArmoredPrototoken = nullptr;

            return (-1); // error condition
        }
        else {
            m_mapPublic[nPublicTokenCount] = pArmoredPrototoken;
            nPublicTokenCount++;
        }

        return 1;
    }
    else if (strNodeName.Compare("privateProtopurse")) {
        nPrivateTokenCount = 0;

        return 1;
    }
    else if (strNodeName.Compare("privatePrototoken")) {
        OTASCIIArmor* pArmoredPrototoken = new OTASCIIArmor;
        OT_ASSERT(nullptr != pArmoredPrototoken);

        if (!OTContract::LoadEncodedTextField(xml, *pArmoredPrototoken) ||
            !pArmoredPrototoken->Exists()) {
            otErr << "Error in Token::ProcessXMLNode: privatePrototoken "
                     "field without value.\n";

            delete pArmoredPrototoken;
            pArmoredPrototoken = nullptr;

            return (-1); // error condition
        }
        else {
            m_mapPrivate[nPrivateTokenCount] = pArmoredPrototoken;
            nPrivateTokenCount++;

            otLog4 << "Loaded prototoken and adding to m_mapPrivate at index: "
                   << nPrivateTokenCount - 1 << "\n";
        }

        return 1;
    }

    return nReturnVal;
}

/*

 enum tokenState {
     blankToken,
     protoToken,
     signedToken,
     spendableToken,
     verifiedToken,
     errorToken
 };

 */

bool Token::GetPrototoken(OTASCIIArmor& ascPrototoken, int32_t nTokenIndex)
{
    // out of bounds. For a count 10 element array, index 10 is out of bounds.
    // thus if attempted index is equal or larger to the count, out of bounds.
    if (nTokenIndex >= m_nTokenCount) {
        return false;
    }

    for (auto& it : m_mapPublic) {
        OTASCIIArmor* pPrototoken = it.second;
        OT_ASSERT(nullptr != pPrototoken);

        const bool bSuccess = (nTokenIndex == it.first);

        if (bSuccess) {
            ascPrototoken.Set(*pPrototoken);

            return true;
        }
    }
    return false;
}

bool Token::GetPrivatePrototoken(OTASCIIArmor& ascPrototoken,
                                 int32_t nTokenIndex)
{
    // out of bounds. For a count 10 element array, index 10 is out of bounds.
    // thus if attempted index is equal or larger to the count, out of bounds.
    if (nTokenIndex >= m_nTokenCount) {
        return false;
    }

    for (auto& it : m_mapPrivate) {
        OTASCIIArmor* pPrototoken = it.second;
        OT_ASSERT(nullptr != pPrototoken);

        bool bSuccess = (nTokenIndex == it.first);

        if (bSuccess) {
            ascPrototoken.Set(*pPrototoken);
            return true;
        }
    }
    return false;
}

// static
Token* Token::InstantiateAndGenerateTokenRequest(const Purse& thePurse,
                                                 const OTPseudonym& theNym,
                                                 Mint& theMint,
                                                 int64_t lDenomination,
                                                 int32_t nTokenCount)
{
    Token* pToken = Token::LowLevelInstantiate(thePurse); // already asserts.
    OT_ASSERT(nullptr != pToken); // Just for good measure.

    const bool bGeneratedRequest = pToken->GenerateTokenRequest(
        theNym, theMint, lDenomination, nTokenCount);

    if (!bGeneratedRequest) {
        otErr << __FUNCTION__ << ": Failed trying to generate token request.\n";
        delete pToken;
        pToken = nullptr;
    }

    return pToken;
}

inline bool Token::ChooseIndex(const int32_t nIndex)
{
    if (nIndex > (m_nTokenCount - 1) || nIndex < 0)
        return false;
    else {
        m_nChosenIndex = nIndex;
        return true;
    }
}

// The Mint has signed the token, and is sending it back to the client.
// (we're near Lucre step 3 with this function)
void Token::SetSignature(const OTASCIIArmor& theSignature, int32_t nTokenIndex)
{
    // The server sets the signature, and then sends the token back to the
    // client. We release all these prototokens before doing so, because there's
    // no point in sending them all back to the client again, who already has
    // them anyway.
    // This is important because otherwise I wouldn't release, because the
    // client
    // still has to look up the private coin in order to unblind. But we're not
    // on the client if we're signing -- we're on the server -- who doesn't have
    // those private coins anyway.
    ReleasePrototokens();

    // We now officially have the bank's signature on this token.
    m_Signature.Set(theSignature);

    //    otErr << "DEBUG Token::SetSignature. nTokenIndex is %d.\nm_Signature
    // is:\n%s\n"
    //            "-------------------------------------\n",
    //            nTokenIndex, m_Signature.Get());

    // We have to flag which index was signed by the mint, so that
    // the client knows which private coin to use for unblinding.
    // (Once the coin is unblinded, it will be ready to spend.)
    ChooseIndex(nTokenIndex);

    m_State = Token::signedToken;
}

bool Token::GetSignature(OTASCIIArmor& theSignature) const
{
    theSignature = m_Signature;

    return true;
}

// **** VERIFY THE TOKEN WHEN REDEEMED AT THE SERVER
// Lucre step 5: token verifies when it is redeemed by merchant.
// IMPORTANT: while stored on the client side, the tokens are
// encrypted to the client side nym. But when he redeems them to
// the server, he re-encrypts them first to the SERVER's public nym.
// So by the time it comes to verify, we are opening this envelope
// with the Server's Nym.
bool Token::VerifyToken(OTPseudonym& theNotary, Mint& theMint)
{
    // otErr << "%s <bank info> <coin>\n",argv[0]);

    if (Token::spendableToken != m_State) {
        otErr << "Expected spendable token in Token::VerifyToken\n";

        return false;
    }

    //  _OT_Lucre_Dumper setDumper; // OTMint::VerifyToken already does this.
    // Unnecessary here?

    // load the bank and coin info into the bios
    // The Mint private info is encrypted in m_ascPrivate. So I need to extract
    // that
    // first before I can use it.
    OTEnvelope theEnvelope(m_ascSpendable);

    OTString strContents; // output from opening the envelope.
    // Decrypt the Envelope into strContents
    if (!theEnvelope.Open(theNotary, strContents))
        return false; // todo log error, etc.

    // Verify that the series is correct...
    // (Otherwise, someone passed us the wrong Mint and the
    // thing won't verify anyway, since we'd have the wrong keys.)
    if (m_nSeries != theMint.GetSeries() ||
        // Someone might, however, in a clever attack, choose to leave
        // the series intact, but change the expiration dates, so that the
        // mint keys continue to work properly for this token, but then
        // when we check the date, it APPEARS good, when really the dates
        // were altered! To prevent this, we explicitly verify the series
        // information on the token against the same info on the mint,
        // BEFORE checking the date.
        m_VALID_FROM != theMint.GetValidFrom() ||
        m_VALID_TO != theMint.GetValidTo()) {
        otOut << "Token series information doesn't match Mint series "
                 "information!\n";
        return false;
    }

    // Verify whether token has expired...expiration date is validated here.
    // We know the series is correct or the key wouldn't verify below... and
    // we know that the dates are correct because we compared them against the
    // mint of that series above. So now we just make sure that the CURRENT date
    // and time is within the range described on the token.
    if (!VerifyCurrentDate()) {
        otOut << "Token is expired!\n";
        return false;
    }

    // pass the cleartext Lucre spendable coin data to the Mint to be verified.
    if (theMint.VerifyToken(theNotary, strContents,
                            GetDenomination())) // Here's the boolean output:
                                                // coin is verified!
    {
        otOut << "Token verified!\n";
        return true;
    }
    else {
        otOut << "Bad coin!\n";
        return false;
    }
}

} // namespace opentxs
