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

#include "opentxs/stdafx.hpp"

#include "opentxs/cash/Mint.hpp"

#include "opentxs/cash/MintLucre.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/core/OTStringXML.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/OTFolders.hpp"
#include "opentxs/core/util/Tag.hpp"

#include <irrxml/irrXML.hpp>

#include <cstdint>
#include <cstdlib>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <utility>

namespace opentxs
{

// static
Mint* Mint::MintFactory()
{
    Mint* pMint = nullptr;

#if OT_CASH_USING_LUCRE
    pMint = new MintLucre;
#elif OT_CASH_USING_MAGIC_MONEY
    //  pMint = new Mint_MagicMoney;
    otErr << __FUCNTION__
          << ": Open-Transactions doesn't support Magic Money "
             "by Pr0duct Cypher (yet), "
          << "so it's impossible to instantiate a mint.\n";
#else
    otErr
        << __FUNCTION__
        << ": Open-Transactions isn't built with any digital cash algorithms, "
        << "so it's impossible to instantiate a mint.\n";
#endif
    return pMint;
}

// static
Mint* Mint::MintFactory(
    const String& strNotaryID,
    const String& strInstrumentDefinitionID)
{
    Mint* pMint = nullptr;

#if OT_CASH_USING_LUCRE
    pMint = new MintLucre(strNotaryID, strInstrumentDefinitionID);
#elif OT_CASH_USING_MAGIC_MONEY
    //  pMint = new OTMint_MagicMoney;
    otErr << __FUNCTION__
          << ": Open-Transactions doesn't support Magic Money "
             "by Pr0duct Cypher (yet), "
          << "so it's impossible to instantiate a mint.\n";
#else
    otErr
        << __FUNCTION__
        << ": Open-Transactions isn't built with any digital cash algorithms, "
        << "so it's impossible to instantiate a mint.\n";
#endif
    return pMint;
}

// static
Mint* Mint::MintFactory(
    const String& strNotaryID,
    const String& strServerNymID,
    const String& strInstrumentDefinitionID)
{
    Mint* pMint = nullptr;

#if OT_CASH_USING_LUCRE
    pMint =
        new MintLucre(strNotaryID, strServerNymID, strInstrumentDefinitionID);
#elif OT_CASH_USING_MAGIC_MONEY
    //  pMint = new OTMint_MagicMoney;
    otErr << __FUNCTION__
          << ": Open-Transactions doesn't support Magic Money "
             "by Pr0duct Cypher (yet), "
          << "so it's impossible to instantiate a mint.\n";
#else
    otErr
        << __FUNCTION__
        << ": Open-Transactions isn't built with any digital cash algorithms, "
        << "so it's impossible to instantiate a mint.\n";
#endif
    return pMint;
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
    while (!m_mapPublic.empty()) {
        OTASCIIArmor* pArmor = m_mapPublic.begin()->second;
        m_mapPublic.erase(m_mapPublic.begin());
        delete pArmor;
        pArmor = nullptr;
    }
    while (!m_mapPrivate.empty()) {
        OTASCIIArmor* pArmor = m_mapPrivate.begin()->second;
        m_mapPrivate.erase(m_mapPrivate.begin());
        delete pArmor;
        pArmor = nullptr;
    }
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

    if (m_pReserveAcct) {
        delete m_pReserveAcct;
        m_pReserveAcct = nullptr;
    }
}

void Mint::Release()
{
    Release_Mint();

    Contract::Release();  // I overrode the parent, so now I give him a chance
                          // to
                          // clean up.

    InitMint();
}

Mint::~Mint() { Release_Mint(); }

void Mint::InitMint()
{
    m_strContractType.Set("MINT");

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

    m_pReserveAcct = nullptr;
}

Mint::Mint(
    const String& strNotaryID,
    const String& strServerNymID,
    const String& strInstrumentDefinitionID)
    : Contract(strInstrumentDefinitionID)
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
    , m_pReserveAcct(nullptr)
{
    m_strFoldername.Set(OTFolders::Mint().Get());
    m_strFilename.Format(
        "%s%s%s",
        strNotaryID.Get(),
        Log::PathSeparator(),
        strInstrumentDefinitionID.Get());

    InitMint();
}

Mint::Mint(const String& strNotaryID, const String& strInstrumentDefinitionID)
    : Contract(strInstrumentDefinitionID)
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
    , m_pReserveAcct(nullptr)
{
    m_strFoldername.Set(OTFolders::Mint().Get());
    m_strFilename.Format(
        "%s%s%s",
        strNotaryID.Get(),
        Log::PathSeparator(),
        strInstrumentDefinitionID.Get());

    InitMint();
}

Mint::Mint()
    : Contract()
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
    , m_pReserveAcct(nullptr)
{
    InitMint();
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
    if (!m_strFoldername.Exists()) m_strFoldername.Set(OTFolders::Mint().Get());

    const String strNotaryID(m_NotaryID),
        strInstrumentDefinitionID(m_InstrumentDefinitionID);

    if (!m_strFilename.Exists()) {
        if (nullptr != szAppend)
            m_strFilename.Format(
                "%s%s%s%s",
                strNotaryID.Get(),
                Log::PathSeparator(),  // server appends ".1"
                                       // or ".PUBLIC" here.
                strInstrumentDefinitionID.Get(),
                szAppend);
        else
            m_strFilename.Format(
                "%s%s%s",
                strNotaryID.Get(),
                Log::PathSeparator(),
                strInstrumentDefinitionID.Get());  // client uses only
                                                   // instrument definition
                                                   // id, no append.
    }

    String strFilename;
    if (nullptr != szAppend)
        strFilename.Format(
            "%s%s",
            strInstrumentDefinitionID.Get(),
            szAppend);  // server side
    else
        strFilename = strInstrumentDefinitionID.Get();  // client side

    const char* szFolder1name = OTFolders::Mint().Get();  // "mints"
    const char* szFolder2name = strNotaryID.Get();        // "mints/NOTARY_ID"
    const char* szFilename =
        strFilename
            .Get();  // "mints/NOTARY_ID/INSTRUMENT_DEFINITION_ID<szAppend>"

    if (!OTDB::Exists(szFolder1name, szFolder2name, szFilename)) {
        otOut << "Mint::LoadMint: File does not exist: " << szFolder1name
              << Log::PathSeparator() << szFolder2name << Log::PathSeparator()
              << szFilename << "\n";
        return false;
    }

    std::string strFileContents(OTDB::QueryPlainString(
        szFolder1name,
        szFolder2name,
        szFilename));  // <=== LOADING FROM DATA STORE.

    if (strFileContents.length() < 2) {
        otErr << "Mint::LoadMint: Error reading file: " << szFolder1name
              << Log::PathSeparator() << szFolder2name << Log::PathSeparator()
              << szFilename << "\n";
        return false;
    }

    // NOTE: No need to worry about the OT ARMORED file format, since
    // LoadContractFromString already handles that internally.

    String strRawFile(strFileContents.c_str());

    bool bSuccess = LoadContractFromString(
        strRawFile);  // Note: This handles OT ARMORED file format.

    return bSuccess;
}

bool Mint::SaveMint(const char* szAppend)
{
    if (!m_strFoldername.Exists()) m_strFoldername.Set(OTFolders::Mint().Get());

    const String strNotaryID(m_NotaryID),
        strInstrumentDefinitionID(m_InstrumentDefinitionID);

    if (!m_strFilename.Exists()) {
        if (nullptr != szAppend)
            m_strFilename.Format(
                "%s%s%s%s",
                strNotaryID.Get(),
                Log::PathSeparator(),  // server side
                strInstrumentDefinitionID.Get(),
                szAppend);
        else
            m_strFilename.Format(
                "%s%s%s",
                strNotaryID.Get(),
                Log::PathSeparator(),
                strInstrumentDefinitionID.Get());  // client side
    }

    String strFilename;
    if (nullptr != szAppend)
        strFilename.Format("%s%s", strInstrumentDefinitionID.Get(), szAppend);
    else
        strFilename = strInstrumentDefinitionID.Get();

    const char* szFolder1name = OTFolders::Mint().Get();
    const char* szFolder2name = strNotaryID.Get();
    const char* szFilename = strFilename.Get();

    String strRawFile;

    if (!SaveContractRaw(strRawFile)) {
        otErr << "Mint::SaveMint: Error saving Mintfile (to string):\n"
              << szFolder1name << Log::PathSeparator() << szFolder2name
              << Log::PathSeparator() << szFilename << "\n";
        return false;
    }

    String strFinal;
    OTASCIIArmor ascTemp(strRawFile);

    if (false ==
        ascTemp.WriteArmoredString(strFinal, m_strContractType.Get())) {
        otErr << "Mint::SaveMint: Error saving mint (failed writing armored "
                 "string):\n"
              << szFolder1name << Log::PathSeparator() << szFolder2name
              << Log::PathSeparator() << szFilename << "\n";
        return false;
    }

    bool bSaved = OTDB::StorePlainString(
        strFinal.Get(),
        szFolder1name,
        szFolder2name,
        szFilename);  // <=== SAVING TO LOCAL DATA STORE.
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

    otLog3 << "\nWe now know that...\n"
              "1) The Asset Contract ID matches the Mint ID loaded from the "
              "Mint file.\n"
              "2) The SIGNATURE VERIFIED.\n\n";
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
        String str1(m_ID), str2(m_InstrumentDefinitionID);

        otErr << "\nMint ID does NOT match Instrument Definition ID in "
                 "Mint::VerifyContractID.\n"
              << str1 << "\n"
              << str2 << "\n";
        //                "\nRAW FILE:\n--->" << m_strRawFile << "<---"
        return false;
    } else {
        String str1(m_ID);
        otInfo << "\nMint ID *SUCCESSFUL* match to Asset Contract ID:\n"
               << str1 << "\n\n";
        return true;
    }
}

// The mint has a different key pair for each denomination.
// Pass in the actual denomination such as 5, 10, 20, 50, 100...
bool Mint::GetPrivate(OTASCIIArmor& theArmor, std::int64_t lDenomination)
{
    for (auto& it : m_mapPrivate) {
        OTASCIIArmor* pArmor = it.second;
        OT_ASSERT_MSG(
            nullptr != pArmor, "nullptr mint pointer in Mint::GetPrivate.\n");
        // if this denomination (say, 50) matches the one passed in
        if (it.first == lDenomination) {
            theArmor.Set(*pArmor);
            return true;
        }
    }
    return false;
}

// The mint has a different key pair for each denomination.
// Pass in the actual denomination such as 5, 10, 20, 50, 100...
bool Mint::GetPublic(OTASCIIArmor& theArmor, std::int64_t lDenomination)
{
    for (auto& it : m_mapPublic) {
        OTASCIIArmor* pArmor = it.second;
        OT_ASSERT_MSG(
            nullptr != pArmor, "nullptr mint pointer in Mint::GetPublic.\n");
        // if this denomination (say, 50) matches the one passed in
        if (it.first == lDenomination) {
            theArmor.Set(*pArmor);
            return true;
        }
    }

    return false;
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
    if (nIndex > (m_nDenominationCount - 1)) {
        return 0;
    }

    std::int32_t nIterateIndex = 0;

    for (auto it = m_mapPublic.begin(); it != m_mapPublic.end();
         ++it, nIterateIndex++) {
        OTASCIIArmor* pArmor = it->second;
        OT_ASSERT_MSG(
            nullptr != pArmor,
            "nullptr mint pointer in Mint::GetDenomination.\n");

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
    String NOTARY_ID(m_NotaryID), NOTARY_NYM_ID(m_ServerNymID),
        INSTRUMENT_DEFINITION_ID(m_InstrumentDefinitionID),
        CASH_ACCOUNT_ID(m_CashAccountID);

    // I release this because I'm about to repopulate it.
    m_xmlUnsigned.Release();

    Tag tag("mint");

    tag.add_attribute("version", m_strVersion.Get());
    tag.add_attribute("notaryID", NOTARY_ID.Get());
    tag.add_attribute("serverNymID", NOTARY_NYM_ID.Get());
    tag.add_attribute("instrumentDefinitionID", INSTRUMENT_DEFINITION_ID.Get());
    tag.add_attribute("cashAcctID", CASH_ACCOUNT_ID.Get());
    tag.add_attribute("series", formatInt(m_nSeries));
    tag.add_attribute("expiration", formatTimestamp(m_EXPIRATION));
    tag.add_attribute("validFrom", formatTimestamp(m_VALID_FROM));
    tag.add_attribute("validTo", formatTimestamp(m_VALID_TO));

    if (m_nDenominationCount) {
        if (m_bSavePrivateKeys) {
            m_bSavePrivateKeys = false;  // reset this back to false again. Use
                                         // SetSavePrivateKeys() to set it true.

            for (auto& it : m_mapPrivate) {
                OTASCIIArmor* pArmor = it.second;
                OT_ASSERT_MSG(
                    nullptr != pArmor,
                    "nullptr private mint pointer "
                    "in "
                    "Mint::UpdateContents.\n");

                TagPtr tagPrivateInfo(
                    new Tag("mintPrivateInfo", pArmor->Get()));
                tagPrivateInfo->add_attribute(
                    "denomination", formatLong(it.first));
                tag.add_tag(tagPrivateInfo);
            }
        }
        for (auto& it : m_mapPublic) {
            OTASCIIArmor* pArmor = it.second;
            OT_ASSERT_MSG(
                nullptr != pArmor,
                "nullptr public mint pointer in Mint::UpdateContents.\n");

            TagPtr tagPublicInfo(new Tag("mintPublicInfo", pArmor->Get()));
            tagPublicInfo->add_attribute("denomination", formatLong(it.first));
            tag.add_tag(tagPublicInfo);
        }
    }

    std::string str_result;
    tag.output(str_result);

    m_xmlUnsigned.Concatenate("%s", str_result.c_str());
}

// return -1 if error, 0 if nothing, and 1 if the node was processed.
std::int32_t Mint::ProcessXMLNode(irr::io::IrrXMLReader*& xml)
{
    std::int32_t nReturnVal = 0;

    const String strNodeName(xml->getNodeName());

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

    if (strNodeName.Compare("mint")) {
        String strNotaryID, strServerNymID, strInstrumentDefinitionID,
            strCashAcctID;

        m_strVersion = xml->getAttributeValue("version");
        strNotaryID = xml->getAttributeValue("notaryID");
        strServerNymID = xml->getAttributeValue("serverNymID");
        strInstrumentDefinitionID =
            xml->getAttributeValue("instrumentDefinitionID");
        strCashAcctID = xml->getAttributeValue("cashAcctID");

        m_nSeries = atoi(xml->getAttributeValue("series"));
        m_EXPIRATION = parseTimestamp(xml->getAttributeValue("expiration"));

        m_VALID_FROM = parseTimestamp(xml->getAttributeValue("validFrom"));
        m_VALID_TO = parseTimestamp(xml->getAttributeValue("validTo"));

        m_NotaryID->SetString(strNotaryID);
        m_ServerNymID->SetString(strServerNymID);
        m_InstrumentDefinitionID->SetString(strInstrumentDefinitionID);
        m_CashAccountID->SetString(strCashAcctID);

        if (m_pReserveAcct) {
            delete m_pReserveAcct;
            m_pReserveAcct = nullptr;
        }

        // Every Mint has its own cash account. Here we load ours so it's ready
        // for transactions.
        if (strCashAcctID.Exists())
            m_pReserveAcct =
                Account::LoadExistingAccount(m_CashAccountID, m_NotaryID);

        std::int64_t nValidFrom = OTTimeGetSecondsFromTime(m_VALID_FROM);
        std::int64_t nValidTo = OTTimeGetSecondsFromTime(m_VALID_TO);
        std::int64_t nExpiration = OTTimeGetSecondsFromTime(m_EXPIRATION);

        otWarn <<
            //    "\n===> Loading XML for mint into memory structures..."
            "\n\nMint version: " << m_strVersion
               << "\n Notary ID: " << strNotaryID
               << "\n Instrument Definition ID: " << strInstrumentDefinitionID
               << "\n Cash Acct ID: " << strCashAcctID
               << "\n"
                  ""
               << ((m_pReserveAcct != nullptr) ? "SUCCESS" : "FAILURE")
               << " loading Cash Account into memory for pointer: "
                  "Mint::m_pReserveAcct\n"
                  " Series: "
               << m_nSeries << "\n Expiration: " << nExpiration
               << "\n Valid From: " << nValidFrom << "\n Valid To: " << nValidTo
               << "\n";

        nReturnVal = 1;
    } else if (strNodeName.Compare("mintPrivateInfo")) {
        std::int64_t lDenomination =
            String::StringToLong(xml->getAttributeValue("denomination"));

        OTASCIIArmor* pArmor = new OTASCIIArmor;

        OT_ASSERT(nullptr != pArmor);

        if (!Contract::LoadEncodedTextField(xml, *pArmor) ||
            !pArmor->Exists()) {
            otErr << "Error in Mint::ProcessXMLNode: mintPrivateInfo field "
                     "without value.\n";

            delete pArmor;
            pArmor = nullptr;

            return (-1);  // error condition
        } else {
            m_mapPrivate[lDenomination] = pArmor;
        }

        return 1;
    } else if (strNodeName.Compare("mintPublicInfo")) {
        std::int64_t lDenomination =
            String::StringToLong(xml->getAttributeValue("denomination"));

        OTASCIIArmor* pArmor = new OTASCIIArmor;

        OT_ASSERT(nullptr != pArmor);

        if (!Contract::LoadEncodedTextField(xml, *pArmor) ||
            !pArmor->Exists()) {
            otErr << "Error in Mint::ProcessXMLNode: mintPublicInfo field "
                     "without value.\n";

            delete pArmor;
            pArmor = nullptr;

            return (-1);  // error condition
        } else {
            m_mapPublic[lDenomination] = pArmor;
            m_nDenominationCount++;  // Whether client or server, both sides
                                     // have
                                     // public. Each public denomination should
                                     // increment this count.
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
                                            const OTPseudonym & theServerNym,
 const OTMessage & theMessage,
                                            const OTAccount::AccountType
 eAcctType=OTAccount::user)


 // The above method uses this one internally...
 bool OTAccount::GenerateNewAccount(const OTPseudonym & theServer, const
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
    m_pReserveAcct = Account::GenerateNewAccount(
        NOTARY_NYM_ID,
        theNotaryID,
        theNotary,
        NOTARY_NYM_ID,
        theInstrumentDefinitionID,
        Account::mint);

    if (m_pReserveAcct) {
        m_pReserveAcct->GetIdentifier(m_CashAccountID);
        otOut << "Successfully created cash reserve account for new mint.\n";
    } else {
        otErr << "Error creating cash reserve account for new mint.\n";
    }

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

}  // namespace opentxs
