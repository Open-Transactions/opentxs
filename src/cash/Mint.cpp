// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/cash/Mint.hpp"

#include "opentxs/api/Core.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/cash/MintLucre.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/OTFolders.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/core/StringXML.hpp"
#include "opentxs/core/String.hpp"

#include <irrxml/irrXML.hpp>

#include <cstdint>
#include <cstdlib>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <utility>

#define OT_METHOD "opentxs::Mint"

namespace opentxs
{
Mint::Mint(
    const api::Core& core,
    const String& strNotaryID,
    const String& strServerNymID,
    const String& strInstrumentDefinitionID)
    : Contract(core, strInstrumentDefinitionID)
    , m_mapPrivate()
    , m_mapPublic()
    , m_NotaryID(Identifier::Factory(strNotaryID))
    , m_ServerNymID(Identifier::Factory(strServerNymID))
    , m_InstrumentDefinitionID(Identifier::Factory(strInstrumentDefinitionID))
    , m_nDenominationCount(0)
    , m_bSavePrivateKeys(false)
    , m_nSeries(0)
    , m_VALID_FROM(OT_TIME_ZERO)
    , m_VALID_TO(OT_TIME_ZERO)
    , m_EXPIRATION(OT_TIME_ZERO)
    , m_CashAccountID(Identifier::Factory())
{
    m_strFoldername->Set(OTFolders::Mint().Get());
    m_strFilename->Format(
        "%s%s%s",
        strNotaryID.Get(),
        Log::PathSeparator(),
        strInstrumentDefinitionID.Get());

    InitMint();
}

Mint::Mint(
    const api::Core& core,
    const String& strNotaryID,
    const String& strInstrumentDefinitionID)
    : Contract(core, strInstrumentDefinitionID)
    , m_mapPrivate()
    , m_mapPublic()
    , m_NotaryID(Identifier::Factory(strNotaryID))
    , m_ServerNymID(Identifier::Factory())
    , m_InstrumentDefinitionID(Identifier::Factory(strInstrumentDefinitionID))
    , m_nDenominationCount(0)
    , m_bSavePrivateKeys(false)
    , m_nSeries(0)
    , m_VALID_FROM(OT_TIME_ZERO)
    , m_VALID_TO(OT_TIME_ZERO)
    , m_EXPIRATION(OT_TIME_ZERO)
    , m_CashAccountID(Identifier::Factory())
{
    m_strFoldername->Set(OTFolders::Mint().Get());
    m_strFilename->Format(
        "%s%s%s",
        strNotaryID.Get(),
        Log::PathSeparator(),
        strInstrumentDefinitionID.Get());

    InitMint();
}

Mint::Mint(const api::Core& core)
    : Contract(core)
    , m_mapPrivate()
    , m_mapPublic()
    , m_NotaryID(Identifier::Factory())
    , m_ServerNymID(Identifier::Factory())
    , m_InstrumentDefinitionID(Identifier::Factory())
    , m_nDenominationCount(0)
    , m_bSavePrivateKeys(false)
    , m_nSeries(0)
    , m_VALID_FROM(OT_TIME_ZERO)
    , m_VALID_TO(OT_TIME_ZERO)
    , m_EXPIRATION(OT_TIME_ZERO)
    , m_CashAccountID(Identifier::Factory())
{
    InitMint();
}

// SUBCLASSES OF OTMINT FOR EACH DIGITAL CASH ALGORITHM.

#if OT_CASH_USING_MAGIC_MONEY
// Todo:  Someday...
#endif  // Magic Money

// Verify the current date against the VALID FROM / EXPIRATION dates.
// (As opposed to tokens, which are verified against the valid from/to dates.)
bool Mint::Expired() const
{
    const time64_t CURRENT_TIME = OTTimeGetCurrentTime();

    if ((CURRENT_TIME >= m_VALID_FROM) && (CURRENT_TIME <= m_EXPIRATION))
        return false;
    else
        return true;
}

void Mint::ReleaseDenominations()
{
    m_mapPublic.clear();
    m_mapPrivate.clear();
}

// If you want to load a certain Mint from string, then
// you will call LoadContractFromString() (say). Well this Releases the
// contract,
// before loading it, which calls InitMint() to zero out all the important
// pieces of
// data.
//
void Mint::Release_Mint()
{
    ReleaseDenominations();
    m_CashAccountID->Release();
}

void Mint::Release()
{
    Release_Mint();
    // I overrode the parent, so now I give him a chance to clean up.
    Contract::Release();
    InitMint();
}

void Mint::InitMint()
{
    m_strContractType->Set("MINT");

    m_nDenominationCount = 0;

    m_bSavePrivateKeys =
        false;  // Determines whether it serializes private keys (no if false)

    // Mints expire and new ones are rotated in.
    // All tokens have the same series, and validity dates,
    // of the mint that created them.
    m_nSeries = 0;
    m_VALID_FROM = OT_TIME_ZERO;
    m_VALID_TO = OT_TIME_ZERO;
    m_EXPIRATION = OT_TIME_ZERO;
}

bool Mint::LoadContract()
{
    otOut << "Mint::LoadContract OVERRIDE.\n";
    return LoadMint();
}

bool Mint::LoadMint(const char* szAppend)  // todo: server should
                                           // always pass something
                                           // here. client never
                                           // should. Enforcement?
{
    if (!m_strFoldername->Exists())
        m_strFoldername->Set(OTFolders::Mint().Get());

    const auto strNotaryID = String::Factory(m_NotaryID),
               strInstrumentDefinitionID =
                   String::Factory(m_InstrumentDefinitionID);

    if (!m_strFilename->Exists()) {
        if (nullptr != szAppend)
            m_strFilename->Format(
                "%s%s%s%s",
                strNotaryID->Get(),
                Log::PathSeparator(),  // server appends ".1"
                                       // or ".PUBLIC" here.
                strInstrumentDefinitionID->Get(),
                szAppend);
        else
            m_strFilename->Format(
                "%s%s%s",
                strNotaryID->Get(),
                Log::PathSeparator(),
                strInstrumentDefinitionID->Get());  // client uses only
                                                    // instrument definition
                                                    // id, no append.
    }

    auto strFilename = String::Factory();
    if (nullptr != szAppend)
        strFilename->Format(
            "%s%s",
            strInstrumentDefinitionID->Get(),
            szAppend);  // server side
    else
        strFilename =
            String::Factory(strInstrumentDefinitionID->Get());  // client side

    const char* szFolder1name = OTFolders::Mint().Get();  // "mints"
    const char* szFolder2name = strNotaryID->Get();       // "mints/NOTARY_ID"
    const char* szFilename =
        strFilename
            ->Get();  // "mints/NOTARY_ID/INSTRUMENT_DEFINITION_ID<szAppend>"

    if (!OTDB::Exists(
            api_.DataFolder(), szFolder1name, szFolder2name, szFilename, "")) {
        otOut << "Mint::LoadMint: File does not exist: " << szFolder1name
              << Log::PathSeparator() << szFolder2name << Log::PathSeparator()
              << szFilename << "\n";
        return false;
    }

    std::string strFileContents(OTDB::QueryPlainString(
        api_.DataFolder(),
        szFolder1name,
        szFolder2name,
        szFilename,
        ""));  // <=== LOADING FROM
               // DATA STORE.

    if (strFileContents.length() < 2) {
        otErr << "Mint::LoadMint: Error reading file: " << szFolder1name
              << Log::PathSeparator() << szFolder2name << Log::PathSeparator()
              << szFilename << "\n";
        return false;
    }

    // NOTE: No need to worry about the OT ARMORED file format, since
    // LoadContractFromString already handles that internally.

    auto strRawFile = String::Factory(strFileContents.c_str());

    bool bSuccess = LoadContractFromString(
        strRawFile);  // Note: This handles OT ARMORED file format.

    return bSuccess;
}

bool Mint::SaveMint(const char* szAppend)
{
    if (!m_strFoldername->Exists())
        m_strFoldername->Set(OTFolders::Mint().Get());

    const auto strNotaryID = String::Factory(m_NotaryID),
               strInstrumentDefinitionID =
                   String::Factory(m_InstrumentDefinitionID);

    if (!m_strFilename->Exists()) {
        if (nullptr != szAppend)
            m_strFilename->Format(
                "%s%s%s%s",
                strNotaryID->Get(),
                Log::PathSeparator(),  // server side
                strInstrumentDefinitionID->Get(),
                szAppend);
        else
            m_strFilename->Format(
                "%s%s%s",
                strNotaryID->Get(),
                Log::PathSeparator(),
                strInstrumentDefinitionID->Get());  // client side
    }

    auto strFilename = String::Factory();
    if (nullptr != szAppend)
        strFilename->Format("%s%s", strInstrumentDefinitionID->Get(), szAppend);
    else
        strFilename = String::Factory(strInstrumentDefinitionID->Get());

    const char* szFolder1name = OTFolders::Mint().Get();
    const char* szFolder2name = strNotaryID->Get();
    const char* szFilename = strFilename->Get();

    auto strRawFile = String::Factory();

    if (!SaveContractRaw(strRawFile)) {
        otErr << "Mint::SaveMint: Error saving Mintfile (to string):\n"
              << szFolder1name << Log::PathSeparator() << szFolder2name
              << Log::PathSeparator() << szFilename << "\n";
        return false;
    }

    auto strFinal = String::Factory();
    auto ascTemp = Armored::Factory(strRawFile);

    if (false ==
        ascTemp->WriteArmoredString(strFinal, m_strContractType->Get())) {
        otErr << "Mint::SaveMint: Error saving mint (failed writing armored "
                 "string):\n"
              << szFolder1name << Log::PathSeparator() << szFolder2name
              << Log::PathSeparator() << szFilename << "\n";
        return false;
    }

    bool bSaved = OTDB::StorePlainString(
        strFinal->Get(),
        api_.DataFolder(),
        szFolder1name,
        szFolder2name,
        szFilename,
        "");  // <=== SAVING TO LOCAL DATA STORE.
    if (!bSaved) {
        if (nullptr != szAppend)
            otErr << "Mint::SaveMint: Error writing to file: " << szFolder1name
                  << Log::PathSeparator() << szFolder2name
                  << Log::PathSeparator() << szFilename << szAppend << "\n";
        else
            otErr << "Mint::SaveMint: Error writing to file: " << szFolder1name
                  << Log::PathSeparator() << szFolder2name
                  << Log::PathSeparator() << szFilename << "\n";

        return false;
    }

    return true;
}

// Make sure this contract checks out. Very high level.
// Verifies ID and signature.
bool Mint::VerifyMint(const Nym& theOperator)
{
    // Make sure that the supposed Contract ID that was set is actually
    // a hash of the contract file, signatures and all.
    if (!VerifyContractID()) {
        otErr << "Error comparing Mint ID to Asset Contract ID in "
                 "Mint::VerifyMint\n";
        return false;
    } else if (!VerifySignature(theOperator)) {
        otErr << "Error verifying signature on mint in Mint::VerifyMint.\n";
        return false;
    }

    LogDebug(OT_METHOD)(__FUNCTION__)(": We now know that...").Flush();
    LogDebug(OT_METHOD)(__FUNCTION__)(": 1. The Asset Contract ID matches the "
                                      "Mint ID loaded from the Mint file.")
        .Flush();
    LogDebug(OT_METHOD)(__FUNCTION__)(": 2. The SIGNATURE VERIFIED.").Flush();
    return true;
}

// Unlike other contracts, which calculate their ID from a hash of the file
// itself, a mint has
// the same ID as its Asset Contract.  When we open the Mint file, we read the
// Instrument Definition ID
// from it and then verify that it matches what we were expecting from the asset
// type.
bool Mint::VerifyContractID() const
{
    // I use the == operator here because there is no != operator at this time.
    // That's why you see the ! outside the parenthesis.
    if (!(m_ID == m_InstrumentDefinitionID)) {
        auto str1 = String::Factory(m_ID),
             str2 = String::Factory(m_InstrumentDefinitionID);

        otErr << "\nMint ID does NOT match Instrument Definition ID in "
                 "Mint::VerifyContractID.\n"
              << str1 << "\n"
              << str2 << "\n";
        //                "\nRAW FILE:\n--->" << m_strRawFile << "<---"
        return false;
    } else {
        auto str1 = String::Factory(m_ID);
        LogVerbose(OT_METHOD)(__FUNCTION__)(
            ": Mint ID *SUCCESSFUL* match to Asset Contract ID: ")(str1)
            .Flush();
        return true;
    }
}

// The mint has a different key pair for each denomination.
// Pass in the actual denomination such as 5, 10, 20, 50, 100...
bool Mint::GetPrivate(Armored& theArmor, std::int64_t lDenomination)
{
    try {
        theArmor.Set(m_mapPrivate.at(lDenomination));

        return true;
    } catch (...) {

        return false;
    }
}

// The mint has a different key pair for each denomination.
// Pass in the actual denomination such as 5, 10, 20, 50, 100...
bool Mint::GetPublic(Armored& theArmor, std::int64_t lDenomination)
{
    try {
        theArmor.Set(m_mapPublic.at(lDenomination));

        return true;
    } catch (...) {

        return false;
    }
}

// If you need to withdraw a specific amount, pass it in here and the
// mint will return the largest denomination that is equal to or smaller
// than the amount.
// Then you can subtract the denomination from the amount and call this method
// again, and again, until it reaches 0, in order to create all the necessary
// tokens to reach the full withdrawal amount.
std::int64_t Mint::GetLargestDenomination(int64_t lAmount)
{
    for (std::int32_t nIndex = GetDenominationCount() - 1; nIndex >= 0;
         nIndex--) {
        std::int64_t lDenom = GetDenomination(nIndex);

        if (lDenom <= lAmount) return lDenom;
    }

    return 0;
}

// If you call GetDenominationCount, you can then use this method
// to look up a denomination by index.
// You could also iterate through them by index.
std::int64_t Mint::GetDenomination(std::int32_t nIndex)
{
    // index out of bounds.
    if (nIndex > (m_nDenominationCount - 1)) { return 0; }

    std::int32_t nIterateIndex = 0;

    for (auto it = m_mapPublic.begin(); it != m_mapPublic.end();
         ++it, nIterateIndex++) {

        if (nIndex == nIterateIndex) return it->first;
    }

    return 0;
}

// The default behavior of this function does NOT save the private keys. It only
// serializes the public keys, and it is safe to send the object to the client.
// If the server needs to save the private keys, then call SetSavePrivateKeys()
// first.
void Mint::UpdateContents()
{
    auto NOTARY_ID = String::Factory(m_NotaryID),
         NOTARY_NYM_ID = String::Factory(m_ServerNymID),
         INSTRUMENT_DEFINITION_ID = String::Factory(m_InstrumentDefinitionID),
         CASH_ACCOUNT_ID = String::Factory(m_CashAccountID);

    // I release this because I'm about to repopulate it.
    m_xmlUnsigned->Release();

    Tag tag("mint");

    tag.add_attribute("version", m_strVersion->Get());
    tag.add_attribute("notaryID", NOTARY_ID->Get());
    tag.add_attribute("serverNymID", NOTARY_NYM_ID->Get());
    tag.add_attribute(
        "instrumentDefinitionID", INSTRUMENT_DEFINITION_ID->Get());
    tag.add_attribute("cashAcctID", CASH_ACCOUNT_ID->Get());
    tag.add_attribute("series", formatInt(m_nSeries));
    tag.add_attribute("expiration", formatTimestamp(m_EXPIRATION));
    tag.add_attribute("validFrom", formatTimestamp(m_VALID_FROM));
    tag.add_attribute("validTo", formatTimestamp(m_VALID_TO));

    if (m_nDenominationCount) {
        if (m_bSavePrivateKeys) {
            m_bSavePrivateKeys = false;  // reset this back to false again. Use
                                         // SetSavePrivateKeys() to set it true.

            for (auto& it : m_mapPrivate) {
                TagPtr tagPrivateInfo(
                    new Tag("mintPrivateInfo", it.second->Get()));
                tagPrivateInfo->add_attribute(
                    "denomination", formatLong(it.first));
                tag.add_tag(tagPrivateInfo);
            }
        }
        for (auto& it : m_mapPublic) {
            TagPtr tagPublicInfo(new Tag("mintPublicInfo", it.second->Get()));
            tagPublicInfo->add_attribute("denomination", formatLong(it.first));
            tag.add_tag(tagPublicInfo);
        }
    }

    std::string str_result;
    tag.output(str_result);

    m_xmlUnsigned->Concatenate("%s", str_result.c_str());
}

// return -1 if error, 0 if nothing, and 1 if the node was processed.
std::int32_t Mint::ProcessXMLNode(irr::io::IrrXMLReader*& xml)
{
    std::int32_t nReturnVal = 0;

    const auto strNodeName = String::Factory(xml->getNodeName());

    // Here we call the parent class first.
    // If the node is found there, or there is some error,
    // then we just return either way.  But if it comes back
    // as '0', then nothing happened, and we'll continue executing.
    //
    // -- Note you can choose not to call the parent if
    // you don't want to use any of those xml tags.
    // As I do below, in the case of OTAccount.
    // if (nReturnVal = ot_super::ProcessXMLNode(xml))
    //    return nReturnVal;

    if (strNodeName->Compare("mint")) {
        auto strNotaryID = String::Factory(),
             strServerNymID = String::Factory(),
             strInstrumentDefinitionID = String::Factory(),
             strCashAcctID = String::Factory();

        m_strVersion = String::Factory(xml->getAttributeValue("version"));
        strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        strServerNymID = String::Factory(xml->getAttributeValue("serverNymID"));
        strInstrumentDefinitionID =
            String::Factory(xml->getAttributeValue("instrumentDefinitionID"));
        strCashAcctID = String::Factory(xml->getAttributeValue("cashAcctID"));

        m_nSeries = atoi(xml->getAttributeValue("series"));
        m_EXPIRATION = parseTimestamp(xml->getAttributeValue("expiration"));

        m_VALID_FROM = parseTimestamp(xml->getAttributeValue("validFrom"));
        m_VALID_TO = parseTimestamp(xml->getAttributeValue("validTo"));

        m_NotaryID->SetString(strNotaryID);
        m_ServerNymID->SetString(strServerNymID);
        m_InstrumentDefinitionID->SetString(strInstrumentDefinitionID);
        m_CashAccountID->SetString(strCashAcctID);

        std::int64_t nValidFrom = OTTimeGetSecondsFromTime(m_VALID_FROM);
        std::int64_t nValidTo = OTTimeGetSecondsFromTime(m_VALID_TO);
        std::int64_t nExpiration = OTTimeGetSecondsFromTime(m_EXPIRATION);

        LogDetail(OT_METHOD)(__FUNCTION__)
            //    "\n===> Loading XML for mint into memory structures..."
            (": Mint version: ")(m_strVersion)(" Notary ID: ")(strNotaryID)(
                " Instrument Definition ID: ")(strInstrumentDefinitionID)(
                " Cash Acct ID: ")(strCashAcctID)(
                (false == m_CashAccountID->empty()) ? "SUCCESS" : "FAILURE")(
                " loading Cash Account into memory for pointer: ")(
                "Mint::m_pReserveAcct ")(" Series: ")(m_nSeries)(
                " Expiration: ")(nExpiration)(" Valid From: ")(nValidFrom)(
                " Valid To: ")(nValidTo)
                .Flush();

        nReturnVal = 1;
    } else if (strNodeName->Compare("mintPrivateInfo")) {
        std::int64_t lDenomination =
            String::StringToLong(xml->getAttributeValue("denomination"));
        auto pArmor = Armored::Factory();

        if (!Contract::LoadEncodedTextField(xml, pArmor) || !pArmor->Exists()) {
            otErr << "Error in Mint::ProcessXMLNode: mintPrivateInfo field "
                     "without value.\n";

            return (-1);  // error condition
        } else {
            m_mapPrivate.emplace(lDenomination, std::move(pArmor));
        }

        return 1;
    } else if (strNodeName->Compare("mintPublicInfo")) {
        std::int64_t lDenomination =
            String::StringToLong(xml->getAttributeValue("denomination"));

        auto pArmor = Armored::Factory();

        if (!Contract::LoadEncodedTextField(xml, pArmor) || !pArmor->Exists()) {
            otErr << "Error in Mint::ProcessXMLNode: mintPublicInfo field "
                     "without value.\n";

            return (-1);  // error condition
        } else {
            m_mapPublic.emplace(lDenomination, std::move(pArmor));
            // Whether client or server, both sides have public. Each public
            // denomination should increment this count.
            m_nDenominationCount++;
        }

        return 1;
    }

    return nReturnVal;
}

/*
 // Just make sure theMessage has these members populated:
 //
 // theMessage.m_strNymID;
 // theMessage.m_strInstrumentDefinitionID;
 // theMessage.m_strNotaryID;

 // static method (call it without an instance, using notation:
 OTAccount::GenerateNewAccount)
 OTAccount * OTAccount::GenerateNewAccount(    const Identifier& theNymID,
 const Identifier& theNotaryID,
                                            const Nym & theServerNym,
 const OTMessage & theMessage,
                                            const OTAccount::AccountType
 eAcctType=OTAccount::user)


 // The above method uses this one internally...
 bool OTAccount::GenerateNewAccount(const Nym & theServer, const
 OTMessage & theMessage,
                                    const OTAccount::AccountType
 eAcctType=OTAccount::user)


 OTAccount * pAcct = nullptr;
 pAcct = OTAccount::LoadExistingAccount(ACCOUNT_ID, NOTARY_ID);
 */

// Lucre step 1: generate new mint
// Make sure the issuer here has a private key
// theMint.GenerateNewMint(nSeries, VALID_FROM, VALID_TO,
// INSTRUMENT_DEFINITION_ID, m_nymServer,
// 1, 5, 10, 20, 50, 100, 500, 1000, 10000, 100000);
void Mint::GenerateNewMint(
    const api::Wallet& wallet,
    std::int32_t nSeries,
    time64_t VALID_FROM,
    time64_t VALID_TO,
    time64_t MINT_EXPIRATION,
    const Identifier& theInstrumentDefinitionID,
    const Identifier& theNotaryID,
    const Nym& theNotary,
    std::int64_t nDenom1,
    std::int64_t nDenom2,
    std::int64_t nDenom3,
    std::int64_t nDenom4,
    std::int64_t nDenom5,
    std::int64_t nDenom6,
    std::int64_t nDenom7,
    std::int64_t nDenom8,
    std::int64_t nDenom9,
    std::int64_t nDenom10)
{
    Release();

    m_InstrumentDefinitionID = theInstrumentDefinitionID;
    m_NotaryID = theNotaryID;

    auto NOTARY_NYM_ID = Identifier::Factory(theNotary);
    m_ServerNymID = NOTARY_NYM_ID;

    m_nSeries = nSeries;
    m_VALID_FROM = VALID_FROM;
    m_VALID_TO = VALID_TO;
    m_EXPIRATION = MINT_EXPIRATION;
    auto account = wallet.CreateAccount(
        NOTARY_NYM_ID,
        theNotaryID,
        theInstrumentDefinitionID,
        theNotary,
        Account::mint,
        0);

    if (account) {
        account.get().GetIdentifier(m_CashAccountID);
        otOut << "Successfully created cash reserve account for new mint.\n";
    } else {
        otErr << "Error creating cash reserve account for new mint.\n";
    }

    account.Release();

    if (nDenom1) {
        AddDenomination(
            theNotary,
            nDenom1);  // std::int32_t nPrimeLength default = 1024
    }
    if (nDenom2) {
        AddDenomination(
            theNotary,
            nDenom2);  // std::int32_t nPrimeLength default = 1024
    }
    if (nDenom3) {
        AddDenomination(
            theNotary,
            nDenom3);  // std::int32_t nPrimeLength default = 1024
    }
    if (nDenom4) {
        AddDenomination(
            theNotary,
            nDenom4);  // std::int32_t nPrimeLength default = 1024
    }
    if (nDenom5) {
        AddDenomination(
            theNotary,
            nDenom5);  // std::int32_t nPrimeLength default = 1024
    }
    if (nDenom6) {
        AddDenomination(
            theNotary,
            nDenom6);  // std::int32_t nPrimeLength default = 1024
    }
    if (nDenom7) {
        AddDenomination(
            theNotary,
            nDenom7);  // std::int32_t nPrimeLength default = 1024
    }
    if (nDenom8) {
        AddDenomination(
            theNotary,
            nDenom8);  // std::int32_t nPrimeLength default = 1024
    }
    if (nDenom9) {
        AddDenomination(
            theNotary,
            nDenom9);  // std::int32_t nPrimeLength default = 1024
    }
    if (nDenom10) {
        AddDenomination(
            theNotary,
            nDenom10);  // std::int32_t nPrimeLength default = 1024
    }
}

Mint::~Mint() { Release_Mint(); }
}  // namespace opentxs
