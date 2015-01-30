/************************************************************
 *
 *  OTMessage.cpp
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

#include <opentxs/core/Message.hpp>

#include <opentxs/core/Ledger.hpp>
#include <opentxs/core/Log.hpp>
#include <opentxs/core/Nym.hpp>
#include <opentxs/core/OTStorage.hpp>

#include <fstream>
#include <cstring>

#include <irrxml/irrXML.hpp>

// PROTOCOL DOCUMENT

// --- This is the file that implements the entire message protocol.
// (Transactions are in a different file.)

// true  == success (even if nothing harvested.)
// false == error.
//

namespace opentxs
{

OTMessageStrategyManager Message::messageStrategyManager;

bool Message::HarvestTransactionNumbers(
    Nym& theNym,
    bool bHarvestingForRetry,          // false until positively asserted.
    bool bReplyWasSuccess,             // false until positively asserted.
    bool bReplyWasFailure,             // false until positively asserted.
    bool bTransactionWasSuccess,       // false until positively asserted.
    bool bTransactionWasFailure) const // false until positively asserted.
{

    const Identifier MSG_NYM_ID(m_strNymID), NOTARY_ID(m_strNotaryID),
        ACCOUNT_ID(m_strAcctID.Exists() ? m_strAcctID
                                        : m_strNymID); // This may be
                                                       // unnecessary, but just
                                                       // in case.

    const String strLedger(m_ascPayload);

    Ledger theLedger(MSG_NYM_ID, ACCOUNT_ID, NOTARY_ID); // We're going to
                                                         // load a messsage
                                                         // ledger from *this.

    if (!strLedger.Exists() || !theLedger.LoadLedgerFromString(strLedger)) {
        otErr << __FUNCTION__
              << ": ERROR: Failed trying to load message ledger:\n\n"
              << strLedger << "\n\n";
        return false;
    }
    // theLedger is loaded up!
    else {
        // Let's iterate through the transactions inside, and harvest whatever
        // we can...
        //
        for (auto& it : theLedger.GetTransactionMap()) {
            OTTransaction* pTransaction = it.second;
            OT_ASSERT(nullptr != pTransaction);

            // NOTE: You would ONLY harvest the transaction numbers if your
            // request failed.
            // Clearly you would never bother harvesting the numbers from a
            // SUCCESSFUL request,
            // because doing so would only put you out of sync. (This is the
            // same reason why
            // we DO harvest numbers from UNSUCCESSFUL requests--in order to
            // stay in sync.)
            //
            // That having been said, an important distinction must be made
            // between failed
            // requests where "the message succeeded but the TRANSACTION
            // failed", versus requests
            // where the MESSAGE ITSELF failed (meaning the transaction itself
            // never got a
            // chance to run, and thus never had a chance to fail.)
            //
            // In the first case, you don't want to harvest the opening
            // transaction number
            // (the primary transaction number for that transaction) because
            // that number was
            // already burned when the transaction failed. Instead, you want to
            // harvest "all
            // the others" (the "closing" numbers.)
            // But in the second case, you want to harvest the opening
            // transaction number as well,
            // since it is still good (because the transaction never ran.)
            //
            // (Therefore the below logic turns on whether or not the message
            // was a success.)
            //
            // UPDATE: The logic is now all inside
            // OTTransaction::Harvest...Numbers, you just have to tell it,
            // when you call it, the state of certain things (message success,
            // transaction success, etc.)
            //

            pTransaction->HarvestOpeningNumber(
                theNym, bHarvestingForRetry, bReplyWasSuccess, bReplyWasFailure,
                bTransactionWasSuccess, bTransactionWasFailure);

            // We grab the closing numbers no matter what (whether message
            // succeeded or failed.)
            // It bears mentioning one more time that you would NEVER harvest in
            // the first place unless
            // your original request somehow failed. So this is more about WHERE
            // the failure occurred (at
            // the message level or the transaction level), not WHETHER one
            // occurred.
            //
            pTransaction->HarvestClosingNumbers(
                theNym, bHarvestingForRetry, bReplyWasSuccess, bReplyWasFailure,
                bTransactionWasSuccess, bTransactionWasFailure);
        }
    } // else (ledger is loaded up.)

    return true;
}

// So the message can get the list of numbers from the Nym, before sending,
// that should be listed as acknowledged that the server reply has already been
// seen for those request numbers.
// IMPORTANT NOTE: The Notary ID is used to lookup the numbers from the Nym.
// Therefore,
// make sure that OTMessage::m_strNotaryID is set BEFORE calling this function.
// (It will
// ASSERT if you don't...)
//
void Message::SetAcknowledgments(Nym& theNym)
{
    m_AcknowledgedReplies.Release();

    const Identifier theNotaryID(m_strNotaryID);

    for (auto& it : theNym.GetMapAcknowledgedNum()) {
        std::string strNotaryID = it.first;
        dequeOfTransNums* pDeque = it.second;
        OT_ASSERT(nullptr != pDeque);

        String OTstrNotaryID = strNotaryID.c_str();
        const Identifier theTempID(OTstrNotaryID);

        if (!(pDeque->empty()) &&
            (theNotaryID == theTempID)) // only for the matching notaryID.
        {
            for (uint32_t i = 0; i < pDeque->size(); i++) {
                const int64_t lAckRequestNumber = pDeque->at(i);

                m_AcknowledgedReplies.Add(lAckRequestNumber);
            }
            break; // We found it! Might as well break out.
        }
    } // for
}

// The framework (OTContract) will call this function at the appropriate time.
// OTMessage is special because it actually does something here, when most
// contracts
// are read-only and thus never update their contents.
// Messages, obviously, are different every time, and this function will be
// called
// just prior to the signing of the message, in OTContract::SignContract.
void Message::UpdateContents()
{
    // I release this because I'm about to repopulate it.
    m_xmlUnsigned.Release();

    m_xmlUnsigned.Concatenate(
        "<notaryMessage\n version=\"%s\"\n dateSigned=\"%s\">\n\n",
        m_strVersion.Get(), getTimestamp().c_str());

    m_lTime = parseTimestamp(getTimestamp());

    if (!updateContentsByType()) {
        m_xmlUnsigned.Concatenate("<%s\n" // Command
                                  " requestNum=\"%s\"\n"
                                  " success=\"false\"\n"
                                  " acctID=\"%s\"\n"
                                  " nymID=\"%s\"\n"
                                  " notaryID=\"%s\""
                                  " ><!-- THIS IS AN INVALID MESSAGE -->\n\n",
                                  m_strCommand.Get(), m_strRequestNum.Get(),
                                  m_strAcctID.Get(), m_strNymID.Get(),
                                  m_strNotaryID.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
    }

    // ACKNOWLEDGED REQUEST NUMBERS
    //
    // (For reducing the number of box receipts for replyNotices that must be
    // downloaded.)
    //
    // Client keeps a list of server replies he's already seen.
    // Server keeps a list of numbers the client has provided on HIS list
    // (server has removed those from Nymbox).
    //
    // (Each sends his respective list in every message.)
    //
    // Client removes any number he sees on the server's list.
    // Server removes any number he sees the client has also removed.
    //

    if (m_AcknowledgedReplies.Count() > 0) {
        String strAck;
        if (m_AcknowledgedReplies.Output(strAck) && strAck.Exists()) {
            const OTASCIIArmor ascTemp(strAck);

            if (ascTemp.Exists())
                m_xmlUnsigned.Concatenate("<ackReplies>\n%s</ackReplies>\n\n",
                                          ascTemp.Get());
        }
    }

    m_xmlUnsigned.Concatenate("</notaryMessage>\n");
}

bool Message::updateContentsByType()
{
    OTMessageStrategy* strategy =
        messageStrategyManager.findStrategy(m_strCommand.Get());
    if (!strategy) return false;
    m_xmlUnsigned.Concatenate(strategy->writeXml(*this));
    return true;
}

// Todo: consider leaving the request # inside all the server REPLIES, so they
// are easier to match up to the requests. (Duh.)

// return -1 if error, 0 if nothing, and 1 if the node was processed.
int32_t Message::ProcessXMLNode(irr::io::IrrXMLReader*& xml)
{
    // Here we call the parent class first.
    // If the node is found there, or there is some error,
    // then we just return either way.  But if it comes back
    // as '0', then nothing happened, and we'll continue executing.
    //
    // -- Note you can choose not to call the parent if
    // you don't want to use any of those xml tags.
    // As I do below, in the case of OTAccount.
    //
    // if (nReturnVal = OTContract::ProcessXMLNode(xml))
    //      return nReturnVal;

    const String strNodeName(xml->getNodeName());
    if (strNodeName.Compare("ackReplies")) {
        return processXmlNodeAckReplies(*this, xml);
    }
    else if (strNodeName.Compare("acknowledgedReplies")) {
        return processXmlNodeAcknowledgedReplies(*this, xml);
    }
    else if (strNodeName.Compare("notaryMessage")) {
        return processXmlNodeNotaryMessage(*this, xml);
    }

    OTMessageStrategy* strategy =
        messageStrategyManager.findStrategy(xml->getNodeName());
    if (!strategy) return 0;
    return strategy->processXml(*this, xml);
}

int32_t Message::processXmlNodeAckReplies(Message& m,
                                          irr::io::IrrXMLReader*& xml)
{
    String strDepth;
    if (!Contract::LoadEncodedTextField(xml, strDepth)) {
        otErr << "Error in OTMessage::ProcessXMLNode: ackReplies field "
                 "without value.\n";
        return (-1); // error condition
    }

    m_AcknowledgedReplies.Release();

    if (strDepth.Exists()) m_AcknowledgedReplies.Add(strDepth);

    return 1;
}

int32_t Message::processXmlNodeAcknowledgedReplies(Message& m,
                                                   irr::io::IrrXMLReader*& xml)
{
    otErr << "OTMessage::ProcessXMLNode: SKIPPING DEPRECATED FIELD: "
             "acknowledgedReplies\n";

    while (xml->getNodeType() != irr::io::EXN_ELEMENT_END) {
        xml->read();
    }

    return 1;
}

int32_t Message::processXmlNodeNotaryMessage(Message& m,
                                             irr::io::IrrXMLReader*& xml)
{
    m_strVersion = xml->getAttributeValue("version");

    String strDateSigned = xml->getAttributeValue("dateSigned");

    if (strDateSigned.Exists()) m_lTime = parseTimestamp(strDateSigned.Get());

    otInfo << "\n===> Loading XML for Message into memory structures...\n";

    return 1;
}

// OTString StrategyGetMarketListResponse::writeXml(OTMessage &message)

// int32_t StrategyGetMarketListResponse::processXml(OTMessage &message,
// irr::io::IrrXMLReader*& xml)

// Most contracts do not override this function...
// But OTMessage does, because every request sent to the server needs to be
// signed.
// And each new request is a new message, that requires a new signature, unlike
// most
// contracts, (that always stay the same after they are signed.)
//
// We need to update the m_xmlUnsigned member with the message members before
// the
// actual signing occurs. (Presumably this is the whole reason why the account
// is being re-signed.)
//
// Normally, in other OTContract and derived classes, m_xmlUnsigned is read
// from the file and then kept read-only, since contracts do not normally
// change.
// But as new messages are sent, they must be signed. This function insures that
// the most up-to-date member contents are included in the request before it is
// signed.
//
// Note: Above comment is slightly old. This override is now here only for the
// purpose
// of releasing the signatures.  The other functionality is now handled by the
// UpdateContents member, which is called by the framework, and otherwise empty
// in
// default, but child classes such as OTMessage and OTAccount override it to
// save
// their contents just before signing.
// See OTMessage::UpdateContents near the top of this file for an example.
//
bool Message::SignContract(const Nym& theNym, const OTPasswordData* pPWData)
{
    // I release these, I assume, because a message only has one signer.
    ReleaseSignatures(); // Note: this might change with credentials. We might
                         // require multiple signatures.

    // Use the authentication key instead of the signing key.
    //
    m_bIsSigned = Contract::SignContractAuthent(theNym, pPWData);

    if (m_bIsSigned) {
        //        otErr <<
        // "\n******************************************************\n"
        //                "Contents of signed
        // message:\n\n%s******************************************************\n\n",
        // m_xmlUnsigned.Get());
    }
    else
        otWarn << "Failure signing message:\n" << m_xmlUnsigned << "";

    return m_bIsSigned;
}

// virtual (OTContract)
bool Message::VerifySignature(const Nym& theNym,
                              const OTPasswordData* pPWData) const
{
    // Messages, unlike many contracts, use the authentication key instead of
    // the signing key. This is because signing keys are meant for signing
    // legally
    // binding agreements, whereas authentication keys are used for message
    // transport
    // and for file storage. Since this is OTMessage specifically, used for
    // transport,
    // we have overridden sign and verify contract methods, to explicitly use
    // the
    // authentication key instead of the signing key. OTSignedFile should
    // probably be
    // the same way. (Maybe it already is, by the time you are reading this.)
    //
    return VerifySigAuthent(theNym, pPWData);
}

// Unlike other contracts, which do not change over time, and thus calculate
// their ID
// from a hash of the file itself, OTMessage objects are different every time.
// Thus, we
// cannot use a hash of the file to produce the Message ID.
//
// Message ID will probably become an important part of the protocol (to prevent
// replay attacks..)
// So I will end up using it. But for now, VerifyContractID will always return
// true.
//
bool Message::VerifyContractID() const
{
    return true;
}

Message::Message()
    : Contract()
    , m_bIsSigned(false)
    , m_lNewRequestNum(0)
    , m_lDepth(0)
    , m_lTransactionNum(0)
    , m_bSuccess(false)
    , m_bBool(false)
    , m_lTime(0)

{
    Contract::m_strContractType.Set("MESSAGE");
}

Message::~Message()
{
}

void OTMessageStrategy::processXmlSuccess(Message& m,
                                          irr::io::IrrXMLReader*& xml)
{
    m.m_bSuccess = String(xml->getAttributeValue("success")).Compare("true");
}

void Message::registerStrategy(std::string name, OTMessageStrategy* strategy)
{
    messageStrategyManager.registerStrategy(name, strategy);
}

OTMessageStrategy::~OTMessageStrategy()
{
}

class StrategyGetMarketOffers : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate("<%s\n"
                           " requestNum=\"%s\"\n"
                           " nymID=\"%s\"\n"
                           " notaryID=\"%s\"\n"
                           " marketID=\"%s\"\n" // stored in NymID2
                           " depth=\"%" PRId64 "\""
                           ">\n\n",
                           m.m_strCommand.Get(), m.m_strRequestNum.Get(),
                           m.m_strNymID.Get(), m.m_strNotaryID.Get(),
                           m.m_strNymID2.Get(), // Storing Market ID
                           m.m_lDepth);

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    virtual int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID2 = xml->getAttributeValue("marketID");

        String strDepth = xml->getAttributeValue("depth");

        if (strDepth.GetLength() > 0) m.m_lDepth = strDepth.ToLong();

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID
               << "\n Market ID: " << m.m_strNymID2
               << "\n Request #: " << m.m_strRequestNum << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetMarketOffers::reg("getMarketOffers",
                                              new StrategyGetMarketOffers());

class StrategyGetMarketOffersResponse : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate("<%s\n"
                           " requestNum=\"%s\"\n"
                           " success=\"%s\"\n"
                           " nymID=\"%s\"\n"
                           " notaryID=\"%s\"\n"
                           " marketID=\"%s\"\n" // stored in NymID2
                           " depth=\"%" PRId64 "\""
                           ">\n\n",
                           m.m_strCommand.Get(), m.m_strRequestNum.Get(),
                           (m.m_bSuccess ? "true" : "false"),
                           m.m_strNymID.Get(), m.m_strNotaryID.Get(),
                           m.m_strNymID2.Get(), // Storing Market ID
                           m.m_lDepth);

        if (m.m_bSuccess && (m.m_ascPayload.GetLength() > 2) &&
            (m.m_lDepth > 0))
            result.Concatenate("<messagePayload>\n%s</messagePayload>\n\n",
                               m.m_ascPayload.Get());
        else if (!m.m_bSuccess && (m.m_ascInReferenceTo.GetLength() > 2))
            result.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                               m.m_ascInReferenceTo.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    virtual int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strNymID2 = xml->getAttributeValue("marketID");

        String strDepth = xml->getAttributeValue("depth");

        if (strDepth.GetLength() > 0) m.m_lDepth = strDepth.ToLong();

        const char* pElementExpected = nullptr;
        if (m.m_bSuccess && (m.m_lDepth > 0))
            pElementExpected = "messagePayload";
        else if (!m.m_bSuccess)
            pElementExpected = "inReferenceTo";

        if (nullptr != pElementExpected) {
            OTASCIIArmor ascTextExpected;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }

            if (m.m_bSuccess)
                m.m_ascPayload.Set(ascTextExpected);
            else
                m.m_ascInReferenceTo = ascTextExpected;
        }

        if (m.m_bSuccess)
            otWarn << "\nCommand: " << m.m_strCommand << "   "
                   << (m.m_bSuccess ? "SUCCESS" : "FAILED")
                   << "\nNymID:    " << m.m_strNymID
                   << "\n NotaryID: " << m.m_strNotaryID
                   << "\n MarketID: " << m.m_strNymID2
                   << "\n\n"; // m_ascPayload.Get()
        else
            otWarn << "\nCommand: " << m.m_strCommand << "   "
                   << (m.m_bSuccess ? "SUCCESS" : "FAILED")
                   << "\nNymID:    " << m.m_strNymID
                   << "\n NotaryID: " << m.m_strNotaryID
                   << "\n MarketID: " << m.m_strNymID2
                   << "\n\n"; // m_ascInReferenceTo.Get()

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetMarketOffersResponse::reg(
    "getMarketOffersResponse", new StrategyGetMarketOffersResponse());

class StrategyGetMarketRecentTrades : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate("<%s\n"
                           " requestNum=\"%s\"\n"
                           " nymID=\"%s\"\n"
                           " notaryID=\"%s\"\n"
                           " marketID=\"%s\"" // stored in NymID2
                           ">\n\n",
                           m.m_strCommand.Get(), m.m_strRequestNum.Get(),
                           m.m_strNymID.Get(), m.m_strNotaryID.Get(),
                           m.m_strNymID2.Get() // Storing Market ID
                           );

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID2 = xml->getAttributeValue("marketID");

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID
               << "\n Market ID: " << m.m_strNymID2
               << "\n Request #: " << m.m_strRequestNum << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetMarketRecentTrades::reg(
    "getMarketRecentTrades", new StrategyGetMarketRecentTrades());

class StrategyGetMarketRecentTradesResponse : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate("<%s\n"
                           " requestNum=\"%s\"\n"
                           " success=\"%s\"\n"
                           " nymID=\"%s\"\n"
                           " notaryID=\"%s\"\n"
                           " marketID=\"%s\"\n" // stored in NymID2
                           " depth=\"%" PRId64 "\""
                           ">\n\n",
                           m.m_strCommand.Get(), m.m_strRequestNum.Get(),
                           (m.m_bSuccess ? "true" : "false"),
                           m.m_strNymID.Get(), m.m_strNotaryID.Get(),
                           m.m_strNymID2.Get(), // Storing Market ID
                           m.m_lDepth);

        if (m.m_bSuccess && (m.m_ascPayload.GetLength() > 2) &&
            (m.m_lDepth > 0))
            result.Concatenate("<messagePayload>\n%s</messagePayload>\n\n",
                               m.m_ascPayload.Get());
        else if (!m.m_bSuccess && (m.m_ascInReferenceTo.GetLength() > 2))
            result.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                               m.m_ascInReferenceTo.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    virtual int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strNymID2 = xml->getAttributeValue("marketID");

        String strDepth = xml->getAttributeValue("depth");

        if (strDepth.GetLength() > 0) m.m_lDepth = strDepth.ToLong();

        const char* pElementExpected = nullptr;
        if (m.m_bSuccess && (m.m_lDepth > 0))
            pElementExpected = "messagePayload";
        else if (!m.m_bSuccess)
            pElementExpected = "inReferenceTo";

        if (nullptr != pElementExpected) {
            OTASCIIArmor ascTextExpected;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }

            if (m.m_bSuccess)
                m.m_ascPayload.Set(ascTextExpected);
            else
                m.m_ascInReferenceTo = ascTextExpected;
        }

        if (m.m_bSuccess)
            otWarn << "\nCommand: " << m.m_strCommand << "   "
                   << (m.m_bSuccess ? "SUCCESS" : "FAILED")
                   << "\nNymID:    " << m.m_strNymID
                   << "\n NotaryID: " << m.m_strNotaryID
                   << "\n MarketID: " << m.m_strNymID2
                   << "\n\n"; // m_ascPayload.Get()
        else
            otWarn << "\nCommand: " << m.m_strCommand << "   "
                   << (m.m_bSuccess ? "SUCCESS" : "FAILED")
                   << "\nNymID:    " << m.m_strNymID
                   << "\n NotaryID: " << m.m_strNotaryID
                   << "\n MarketID: " << m.m_strNymID2
                   << "\n\n"; // m_ascInReferenceTo.Get()

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetMarketRecentTradesResponse::reg(
    "getMarketRecentTradesResponse",
    new StrategyGetMarketRecentTradesResponse());

class StrategyGetNymMarketOffers : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate("<%s\n"
                           " requestNum=\"%s\"\n"
                           " nymID=\"%s\"\n"
                           " notaryID=\"%s\""
                           ">\n\n",
                           m.m_strCommand.Get(), m.m_strRequestNum.Get(),
                           m.m_strNymID.Get(), m.m_strNotaryID.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    virtual int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID
               << "\nRequest #: " << m.m_strRequestNum << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetNymMarketOffers::reg(
    "getNymMarketOffers", new StrategyGetNymMarketOffers());

class StrategyGetNymMarketOffersResponse : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate("<%s\n"
                           " requestNum=\"%s\"\n"
                           " success=\"%s\"\n"
                           " nymID=\"%s\"\n"
                           " notaryID=\"%s\"\n"
                           " depth=\"%" PRId64 "\""
                           ">\n\n",
                           m.m_strCommand.Get(), m.m_strRequestNum.Get(),
                           (m.m_bSuccess ? "true" : "false"),
                           m.m_strNymID.Get(), m.m_strNotaryID.Get(),
                           m.m_lDepth);

        if (m.m_bSuccess && (m.m_ascPayload.GetLength() > 2) &&
            (m.m_lDepth > 0))
            result.Concatenate("<messagePayload>\n%s</messagePayload>\n\n",
                               m.m_ascPayload.Get());
        else if (!m.m_bSuccess && (m.m_ascInReferenceTo.GetLength() > 2))
            result.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                               m.m_ascInReferenceTo.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    virtual int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");

        String strDepth = xml->getAttributeValue("depth");

        if (strDepth.GetLength() > 0) m.m_lDepth = strDepth.ToLong();

        const char* pElementExpected = nullptr;
        if (m.m_bSuccess && (m.m_lDepth > 0))
            pElementExpected = "messagePayload";
        else if (!m.m_bSuccess)
            pElementExpected = "inReferenceTo";

        if (nullptr != pElementExpected) {
            OTASCIIArmor ascTextExpected;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }

            if (m.m_bSuccess)
                m.m_ascPayload.Set(ascTextExpected);
            else
                m.m_ascInReferenceTo = ascTextExpected;
        }

        if (m.m_bSuccess)
            otWarn << "\nCommand: " << m.m_strCommand << "   "
                   << (m.m_bSuccess ? "SUCCESS" : "FAILED")
                   << "\nNymID:    " << m.m_strNymID
                   << "\n NotaryID: " << m.m_strNotaryID
                   << "\n\n"; // m_ascPayload.Get()
        else
            otWarn << "\nCommand: " << m.m_strCommand << "   "
                   << (m.m_bSuccess ? "SUCCESS" : "FAILED")
                   << "\nNymID:    " << m.m_strNymID
                   << "\n NotaryID: " << m.m_strNotaryID
                   << "\n\n"; // m_ascInReferenceTo.Get()

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetNymMarketOffersResponse::reg(
    "getNymMarketOffersResponse", new StrategyGetNymMarketOffersResponse());

class StrategyPingNotary : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate("<%s\n"
                           " requestNum=\"%s\"\n"
                           " nymID=\"%s\"\n"
                           " notaryID=\"%s\""
                           ">\n\n",
                           m.m_strCommand.Get(), m.m_strRequestNum.Get(),
                           m.m_strNymID.Get(), m.m_strNotaryID.Get());

        result.Concatenate("<publicAuthentKey>\n%s</publicAuthentKey>\n\n",
                           m.m_strNymPublicKey.Get());
        result.Concatenate(
            "<publicEncryptionKey>\n%s</publicEncryptionKey>\n\n",
            m.m_strNymID2.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");

        const char* pElementExpected = "publicAuthentKey";
        OTASCIIArmor ascTextExpected;

        if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                  pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m.m_strCommand
                  << ".\n";
            return (-1); // error condition
        }

        m.m_strNymPublicKey.Set(ascTextExpected);

        pElementExpected = "publicEncryptionKey";
        ascTextExpected.Release();

        if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                  pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m.m_strCommand
                  << ".\n";
            return (-1); // error condition
        }

        m.m_strNymID2.Set(ascTextExpected);

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID
               << "\n\n Public signing key:\n" << m.m_strNymPublicKey
               << "\nPublic encryption key:\n" << m.m_strNymID2 << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyPingNotary::reg("pingNotary",
                                         new StrategyPingNotary());

class StrategyPingNotaryResponse : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate("<%s\n"
                           " requestNum=\"%s\"\n"
                           " success=\"%s\"\n"
                           " nymID=\"%s\"\n"
                           " notaryID=\"%s\""
                           ">\n\n",
                           m.m_strCommand.Get(), m.m_strRequestNum.Get(),
                           (m.m_bSuccess ? "true" : "false"),
                           m.m_strNymID.Get(), m.m_strNotaryID.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nSuccess: " << (m.m_bSuccess ? "true" : "false")
               << "\nNymID:    " << m.m_strNymID
               << "\n"
                  "NotaryID: " << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyPingNotaryResponse::reg(
    "pingNotaryResponse", new StrategyPingNotaryResponse());

class StrategyRegisterNym : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate("<%s\n"
                           " requestNum=\"%s\"\n"
                           " nymID=\"%s\"\n"
                           " notaryID=\"%s\""
                           ">\n\n",
                           m.m_strCommand.Get(), m.m_strRequestNum.Get(),
                           m.m_strNymID.Get(), m.m_strNotaryID.Get());

        Contract::saveCredentialsToXml(result, m.m_ascPayload, m.credentials);

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");

        if (!Contract::loadCredentialsFromXml(xml, m.m_ascPayload,
                                              m.credentials)) {
            otErr << "Error in loading credentials, for " << m.m_strCommand
                  << ".\n";
            return -1;
        }

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyRegisterNym::reg("registerNym",
                                          new StrategyRegisterNym());

class StrategyRegisterNymResponse : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate("<%s\n"
                           " requestNum=\"%s\"\n"
                           " success=\"%s\"\n"
                           " nymID=\"%s\"\n"
                           " notaryID=\"%s\""
                           ">\n\n",
                           m.m_strCommand.Get(), m.m_strRequestNum.Get(),
                           (m.m_bSuccess ? "true" : "false"),
                           m.m_strNymID.Get(), m.m_strNotaryID.Get());

        if (m.m_bSuccess && (m.m_ascPayload.GetLength() > 2))
            result.Concatenate("<nymfile>\n%s</nymfile>\n\n",
                               m.m_ascPayload.Get());

        if (m.m_ascInReferenceTo.GetLength() > 2)
            result.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                               m.m_ascInReferenceTo.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");

        if (m.m_bSuccess) {
            const char* pElementExpected = "nymfile";
            OTASCIIArmor& ascTextExpected = m.m_ascPayload;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        const char* pElementExpected = "inReferenceTo";
        OTASCIIArmor& ascTextExpected = m.m_ascInReferenceTo;

        if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                  pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m.m_strCommand
                  << ".\n";
            return (-1); // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand << "  "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID << "\n\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyRegisterNymResponse::reg(
    "registerNymResponse", new StrategyRegisterNymResponse());

class StrategyUnregisterNym : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate("<%s\n"
                           " requestNum=\"%s\"\n"
                           " nymID=\"%s\"\n"
                           " notaryID=\"%s\""
                           ">\n\n",
                           m.m_strCommand.Get(), m.m_strRequestNum.Get(),
                           m.m_strNymID.Get(), m.m_strNotaryID.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyUnregisterNym::reg("unregisterNym",
                                            new StrategyUnregisterNym());

class StrategyUnregisterNymResponse : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate("<%s\n"
                           " requestNum=\"%s\"\n"
                           " success=\"%s\"\n"
                           " nymID=\"%s\"\n"
                           " notaryID=\"%s\""
                           ">\n\n",
                           m.m_strCommand.Get(), m.m_strRequestNum.Get(),
                           (m.m_bSuccess ? "true" : "false"),
                           m.m_strNymID.Get(), m.m_strNotaryID.Get());

        if (m.m_ascInReferenceTo.GetLength() > 2)
            result.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                               m.m_ascInReferenceTo.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");

        const char* pElementExpected = "inReferenceTo";
        OTASCIIArmor& ascTextExpected = m.m_ascInReferenceTo;

        if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                  pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m.m_strCommand
                  << ".\n";
            return (-1); // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand << "  "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID << "\n\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyUnregisterNymResponse::reg(
    "unregisterNymResponse", new StrategyUnregisterNymResponse());

class StrategyCheckNym : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate("<%s\n"
                           " nymID=\"%s\"\n"
                           " nymID2=\"%s\"\n"
                           " requestNum=\"%s\"\n"
                           " notaryID=\"%s\""
                           ">\n\n",
                           m.m_strCommand.Get(), m.m_strNymID.Get(),
                           m.m_strNymID2.Get(), m.m_strRequestNum.Get(),
                           m.m_strNotaryID.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNymID2 = xml->getAttributeValue("nymID2");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNymID2:    " << m.m_strNymID2
               << "\nNotaryID: " << m.m_strNotaryID
               << "\nRequest #: " << m.m_strRequestNum << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyCheckNym::reg("checkNym", new StrategyCheckNym());

class StrategyCheckNymResponse : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        // This means new-style credentials are being sent, not just the public
        // key as before.
        const bool bCredentials =
            (m.m_ascPayload.Exists() && m.m_ascPayload2.Exists());

        result.Concatenate("<%s\n"
                           " requestNum=\"%s\"\n"
                           " success=\"%s\"\n"
                           " nymID=\"%s\"\n"
                           " nymID2=\"%s\"\n"
                           " hasCredentials=\"%s\"\n"
                           " notaryID=\"%s\""
                           ">\n\n",
                           m.m_strCommand.Get(), m.m_strRequestNum.Get(),
                           (m.m_bSuccess ? "true" : "false"),
                           m.m_strNymID.Get(), m.m_strNymID2.Get(),
                           (bCredentials ? "true" : "false"),
                           m.m_strNotaryID.Get());

        if (m.m_bSuccess) {
            // Old style. (Deprecated.)
            if (m.m_strNymPublicKey.Exists())
                result.Concatenate("<nymPublicKey>\n%s</nymPublicKey>\n\n",
                                   m.m_strNymPublicKey.Get());

            // New style:
            if (bCredentials) {
                result.Concatenate("<credentialIDs>\n%s</credentialIDs>\n\n",
                                   m.m_ascPayload.Get());
                result.Concatenate("<credentials>\n%s</credentials>\n\n",
                                   m.m_ascPayload2.Get());
            }
        }
        else
            result.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                               m.m_ascInReferenceTo.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());

        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNymID2 = xml->getAttributeValue("nymID2");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");

        const String strHasCredentials(
            xml->getAttributeValue("hasCredentials"));
        const bool bHasCredentials = strHasCredentials.Compare("true");

        const char* pElementExpected = nullptr;
        if (m.m_bSuccess)
            pElementExpected = "nymPublicKey";
        else
            pElementExpected = "inReferenceTo";

        OTASCIIArmor ascTextExpected;
        if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                  pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m.m_strCommand
                  << ".\n";
            return (-1); // error condition
        }
        if (m.m_bSuccess)
            m.m_strNymPublicKey.Set(ascTextExpected);
        else
            m.m_ascInReferenceTo = ascTextExpected;

        if (bHasCredentials) {
            pElementExpected = "credentialIDs";
            ascTextExpected.Release();

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
            m.m_ascPayload = ascTextExpected;

            pElementExpected = "credentials";
            ascTextExpected.Release();

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
            m.m_ascPayload2 = ascTextExpected;
        }

        if (m.m_bSuccess)
            otWarn << "\nCommand: " << m.m_strCommand << "   "
                   << (m.m_bSuccess ? "SUCCESS" : "FAILED")
                   << "\nNymID:    " << m.m_strNymID
                   << "\nNymID2:    " << m.m_strNymID2
                   << "\n"
                      "NotaryID: " << m.m_strNotaryID << "\nNym2 Public Key:\n"
                   << m.m_strNymPublicKey << "\n\n";
        else
            otWarn << "\nCommand: " << m.m_strCommand << "   "
                   << (m.m_bSuccess ? "SUCCESS" : "FAILED")
                   << "\nNymID:    " << m.m_strNymID
                   << "\nNymID2:    " << m.m_strNymID2
                   << "\n"
                      "NotaryID: " << m.m_strNotaryID
                   << "\n\n"; // m.m_ascInReferenceTo.Get()

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyCheckNymResponse::reg("checkNymResponse",
                                               new StrategyCheckNymResponse());

class StrategyUsageCredits : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate("<%s\n"
                           " nymID=\"%s\"\n"
                           " nymID2=\"%s\"\n"
                           " requestNum=\"%s\"\n"
                           " adjustment=\"%" PRId64 "\"\n"
                           " notaryID=\"%s\""
                           ">\n\n",
                           m.m_strCommand.Get(), m.m_strNymID.Get(),
                           m.m_strNymID2.Get(), m.m_strRequestNum.Get(),
                           m.m_lDepth, m.m_strNotaryID.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNymID2 = xml->getAttributeValue("nymID2");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        String strAdjustment = xml->getAttributeValue("adjustment");

        if (strAdjustment.GetLength() > 0) m.m_lDepth = strAdjustment.ToLong();

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNymID2:    " << m.m_strNymID2
               << "\nNotaryID: " << m.m_strNotaryID
               << "\nRequest #: " << m.m_strRequestNum
               << "\nAdjustment: " << m.m_lDepth << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyUsageCredits::reg("usageCredits",
                                           new StrategyUsageCredits());

class StrategyUsageCreditsResponse : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate("<%s\n"
                           " requestNum=\"%s\"\n"
                           " success=\"%s\"\n"
                           " nymID=\"%s\"\n"
                           " nymID2=\"%s\"\n"
                           " totalCredits=\"%" PRId64 "\"\n"
                           " notaryID=\"%s\""
                           ">\n\n",
                           m.m_strCommand.Get(), m.m_strRequestNum.Get(),
                           (m.m_bSuccess ? "true" : "false"),
                           m.m_strNymID.Get(), m.m_strNymID2.Get(), m.m_lDepth,
                           m.m_strNotaryID.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNymID2 = xml->getAttributeValue("nymID2");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");

        String strTotalCredits = xml->getAttributeValue("totalCredits");

        if (strTotalCredits.GetLength() > 0)
            m.m_lDepth = strTotalCredits.ToLong();

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nNymID2:    " << m.m_strNymID2
               << "\n"
                  "NotaryID: " << m.m_strNotaryID
               << "\nTotal Credits: " << m.m_lDepth << " \n\n";
        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyUsageCreditsResponse::reg(
    "usageCreditsResponse", new StrategyUsageCreditsResponse());

class StrategyOutpaymentsMessageOrOutmailMessage : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        // This one isn't part of the message protocol, but is used for outmail
        // storage.
        // (Because outmail isn't encrypted like the inmail is, since the
        // Nymfile
        // itself
        // will soon be encrypted, and there's no need to be redundant also as
        // well
        // in addition on top of that.
        //
        result.Concatenate("<%s\n"
                           " nymID=\"%s\"\n"
                           " nymID2=\"%s\"\n"
                           " requestNum=\"%s\"\n"
                           " notaryID=\"%s\""
                           ">\n\n",
                           m.m_strCommand.Get(), m.m_strNymID.Get(),
                           m.m_strNymID2.Get(), m.m_strRequestNum.Get(),
                           m.m_strNotaryID.Get());

        if (m.m_ascPayload.GetLength() > 2)
            result.Concatenate("<messagePayload>\n%s</messagePayload>\n\n",
                               m.m_ascPayload.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNymID2 = xml->getAttributeValue("nymID2");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        const char* pElementExpected = "messagePayload";
        OTASCIIArmor& ascTextExpected = m.m_ascPayload;

        if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                  pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m.m_strCommand
                  << ".\n";
            return (-1); // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNymID2:    " << m.m_strNymID2
               << "\nNotaryID: " << m.m_strNotaryID
               << "\nRequest #: " << m.m_strRequestNum << "\n";

        return 1;
    }
    static RegisterStrategy reg;
    static RegisterStrategy reg2;
};
RegisterStrategy StrategyOutpaymentsMessageOrOutmailMessage::reg(
    "outpaymentsMessage", new StrategyOutpaymentsMessageOrOutmailMessage());
RegisterStrategy StrategyOutpaymentsMessageOrOutmailMessage::reg2(
    "outmailMessage", new StrategyOutpaymentsMessageOrOutmailMessage());

class StrategySendNymMessage : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate("<%s\n"
                           " nymID=\"%s\"\n"
                           " nymID2=\"%s\"\n"
                           " requestNum=\"%s\"\n"
                           " notaryID=\"%s\""
                           ">\n\n",
                           m.m_strCommand.Get(), m.m_strNymID.Get(),
                           m.m_strNymID2.Get(), m.m_strRequestNum.Get(),
                           m.m_strNotaryID.Get());

        if (m.m_ascPayload.GetLength() > 2)
            result.Concatenate("<messagePayload>\n%s</messagePayload>\n\n",
                               m.m_ascPayload.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNymID2 = xml->getAttributeValue("nymID2");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        const char* pElementExpected = "messagePayload";
        OTASCIIArmor& ascTextExpected = m.m_ascPayload;

        if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                  pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m.m_strCommand
                  << ".\n";
            return (-1); // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNymID2:    " << m.m_strNymID2
               << "\nNotaryID: " << m.m_strNotaryID
               << "\nRequest #: " << m.m_strRequestNum << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategySendNymMessage::reg("sendNymMessage",
                                             new StrategySendNymMessage());

class StrategySendNymMessageResponse : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate("<%s\n"
                           " requestNum=\"%s\"\n"
                           " success=\"%s\"\n"
                           " nymID=\"%s\"\n"
                           " nymID2=\"%s\"\n"
                           " notaryID=\"%s\""
                           ">\n\n",
                           m.m_strCommand.Get(), m.m_strRequestNum.Get(),
                           (m.m_bSuccess ? "true" : "false"),
                           m.m_strNymID.Get(), m.m_strNymID2.Get(),
                           m.m_strNotaryID.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNymID2 = xml->getAttributeValue("nymID2");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nNymID2:    " << m.m_strNymID2
               << "\n"
                  "NotaryID: " << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategySendNymMessageResponse::reg(
    "sendNymMessageResponse", new StrategySendNymMessageResponse());

class StrategySendNymInstrumentOrPayDividend : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        // sendNymInstrument is sent from one user
        // to the server, which then attaches that
        // message as a payment, onto a transaction
        // on the Nymbox of the recipient.

        // payDividend is not a normal user
        // message. Rather, the sender uses
        // notarizeTransaction to do a
        // payDividend transaction. On the
        // server side, this creates a new
        // message of type "payDividend"
        // for each recipient, in order to
        // attach a voucher to it (for each
        // recipient) and then that
        // (artificially created
        // payDividend msg) is added to the
        // Nymbox of each recipient.
        result.Concatenate("<%s\n"
                           " nymID=\"%s\"\n"
                           " nymID2=\"%s\"\n"
                           " requestNum=\"%s\"\n"
                           " notaryID=\"%s\""
                           ">\n\n",
                           m.m_strCommand.Get(), m.m_strNymID.Get(),
                           m.m_strNymID2.Get(), m.m_strRequestNum.Get(),
                           m.m_strNotaryID.Get());

        if (m.m_ascPayload.GetLength() > 2)
            result.Concatenate("<messagePayload>\n%s</messagePayload>\n\n",
                               m.m_ascPayload.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNymID2 = xml->getAttributeValue("nymID2");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        const char* pElementExpected = "messagePayload";
        OTASCIIArmor& ascTextExpected = m.m_ascPayload;

        if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                  pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m.m_strCommand
                  << ".\n";
            return (-1); // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNymID2:    " << m.m_strNymID2
               << "\nNotaryID: " << m.m_strNotaryID
               << "\nRequest #: " << m.m_strRequestNum << "\n";

        return 1;
    }
    static RegisterStrategy reg;
    static RegisterStrategy reg2;
};
RegisterStrategy StrategySendNymInstrumentOrPayDividend::reg(
    "sendNymInstrument", new StrategySendNymInstrumentOrPayDividend());
RegisterStrategy StrategySendNymInstrumentOrPayDividend::reg2(
    "payDividend", new StrategySendNymInstrumentOrPayDividend());

class StrategySendNymInstrumentResponse : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate("<%s\n"
                           " requestNum=\"%s\"\n"
                           " success=\"%s\"\n"
                           " nymID=\"%s\"\n"
                           " nymID2=\"%s\"\n"
                           " notaryID=\"%s\""
                           ">\n\n",
                           m.m_strCommand.Get(), m.m_strRequestNum.Get(),
                           (m.m_bSuccess ? "true" : "false"),
                           m.m_strNymID.Get(), m.m_strNymID2.Get(),
                           m.m_strNotaryID.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNymID2 = xml->getAttributeValue("nymID2");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nNymID2:    " << m.m_strNymID2
               << "\n"
                  "NotaryID: " << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategySendNymInstrumentResponse::reg(
    "sendNymInstrumentResponse", new StrategySendNymInstrumentResponse());

class StrategyGetRequestNumber : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate("<%s\n"
                           " nymID=\"%s\"\n"
                           " notaryID=\"%s\"\n"
                           " requestNum=\"%s\""
                           ">\n\n",
                           m.m_strCommand.Get(), m.m_strNymID.Get(),
                           m.m_strNotaryID.Get(), m.m_strRequestNum.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetRequestNumber::reg("getRequestNumber",
                                               new StrategyGetRequestNumber());

class StrategyGetRequestResponse : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        // This is the ONE command where you see a request number coming back
        // from
        // the server.
        // In all the other commands, it should be SENT to the server, not
        // received
        // from the server.
        result.Concatenate(
            "<%s\n"             // command
            " success=\"%s\"\n" // m.m_bSuccess
            " nymID=\"%s\"\n"
            " nymboxHash=\"%s\"\n"
            " notaryID=\"%s\"\n"
            " newRequestNum=\"%" PRId64 "\"\n"
            " requestNum=\"%s\""
            ">\n\n",
            m.m_strCommand.Get(), (m.m_bSuccess ? "true" : "false"),
            m.m_strNymID.Get(), m.m_strNymboxHash.Get(), m.m_strNotaryID.Get(),
            m.m_lNewRequestNum, m.m_strRequestNum.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNymboxHash = xml->getAttributeValue("nymboxHash");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");

        const String strNewRequestNum = xml->getAttributeValue("newRequestNum");
        m.m_lNewRequestNum =
            strNewRequestNum.Exists() ? strNewRequestNum.ToLong() : 0;

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\n"
                  "NotaryID: " << m.m_strNotaryID
               << "\nRequest Number:    " << m.m_strRequestNum
               << "  New Number: " << m.m_lNewRequestNum << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetRequestResponse::reg(
    "getRequestNumberResponse", new StrategyGetRequestResponse());

class StrategyRegisterInstrumentDefinition : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate("<%s\n" // Command
                           " nymID=\"%s\"\n"
                           " notaryID=\"%s\"\n"
                           " requestNum=\"%s\"\n"
                           " instrumentDefinitionID=\"%s\""
                           ">\n\n",
                           m.m_strCommand.Get(), m.m_strNymID.Get(),
                           m.m_strNotaryID.Get(), m.m_strRequestNum.Get(),
                           m.m_strInstrumentDefinitionID.Get());

        if (m.m_ascPayload.GetLength())
            result.Concatenate(
                "<instrumentDefinition>\n%s</instrumentDefinition>\n\n",
                m.m_ascPayload.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strInstrumentDefinitionID =
            xml->getAttributeValue("instrumentDefinitionID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        const char* pElementExpected = "instrumentDefinition";
        OTASCIIArmor& ascTextExpected = m.m_ascPayload;

        if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                  pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m.m_strCommand
                  << ".\n";
            return (-1); // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand
               << " \nNymID:    " << m.m_strNymID
               << "\n"
                  "NotaryID: " << m.m_strNotaryID
               << "\nRequest#: " << m.m_strRequestNum << "\nAsset Type:\n"
               << m.m_strInstrumentDefinitionID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyRegisterInstrumentDefinition::reg(
    "registerInstrumentDefinition", new StrategyRegisterInstrumentDefinition());

class StrategyRegisterInstrumentDefinitionResponse : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate("<%s\n" // Command
                           " requestNum=\"%s\"\n"
                           " success=\"%s\"\n"
                           " accountID=\"%s\"\n" // the new issuer account ID
                           " nymID=\"%s\"\n"
                           " instrumentDefinitionID=\"%s\"\n"
                           " notaryID=\"%s\""
                           ">\n\n",
                           m.m_strCommand.Get(), m.m_strRequestNum.Get(),
                           (m.m_bSuccess ? "true" : "false"),
                           m.m_strAcctID.Get(), m.m_strNymID.Get(),
                           m.m_strInstrumentDefinitionID.Get(),
                           m.m_strNotaryID.Get());

        if (m.m_ascInReferenceTo.GetLength())
            result.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                               m.m_ascInReferenceTo.Get());

        if (m.m_bSuccess && m.m_ascPayload.GetLength())
            result.Concatenate("<issuerAccount>\n%s</issuerAccount>\n\n",
                               m.m_ascPayload.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strInstrumentDefinitionID =
            xml->getAttributeValue("instrumentDefinitionID");
        m.m_strAcctID = xml->getAttributeValue("accountID");

        // If successful, we need to read 2 more things: inReferenceTo and
        // issuerAccount payload.
        // If failure, then we only need to read 1 thing: inReferenceTo
        // At this point, we do not send the REASON WHY if it failed.

        {
            const char* pElementExpected = "inReferenceTo";
            OTASCIIArmor& ascTextExpected = m.m_ascInReferenceTo;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        if (m.m_bSuccess) {
            const char* pElementExpected = "issuerAccount";
            OTASCIIArmor& ascTextExpected = m.m_ascPayload;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there,
        // OR if it was successful but the Payload isn't there, then failure.
        if (!m.m_ascInReferenceTo.GetLength() ||
            (m.m_bSuccess && !m.m_ascPayload.GetLength())) {
            otErr << "Error in OTMessage::ProcessXMLNode:\n"
                     "Expected issuerAccount and/or inReferenceTo elements "
                     "with text fields in "
                     "registerInstrumentDefinitionResponse reply\n";
            return (-1); // error condition
        }

        String acctContents(m.m_ascPayload);
        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nAccountID: " << m.m_strAcctID
               << "\nInstrument Definition ID: "
               << m.m_strInstrumentDefinitionID
               << "\n"
                  "NotaryID: " << m.m_strNotaryID << "\n\n";
        //    "****New Account****:\n%s\n",
        //    m.m_ascInReferenceTo.Get(),
        // acctContents.Get()

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyRegisterInstrumentDefinitionResponse::reg(
    "registerInstrumentDefinitionResponse",
    new StrategyRegisterInstrumentDefinitionResponse());

class StrategyQueryInstrumentDefinitions : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate("<%s\n" // Command
                           " nymID=\"%s\"\n"
                           " notaryID=\"%s\"\n"
                           " requestNum=\"%s\""
                           ">\n\n",
                           m.m_strCommand.Get(), m.m_strNymID.Get(),
                           m.m_strNotaryID.Get(), m.m_strRequestNum.Get());

        if (m.m_ascPayload.GetLength())
            result.Concatenate("<stringMap>\n%s</stringMap>\n\n",
                               m.m_ascPayload.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        const char* pElementExpected = "stringMap";
        OTASCIIArmor& ascTextExpected = m.m_ascPayload;

        if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                  pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m.m_strCommand
                  << ".\n";
            return (-1); // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand
               << " \nNymID:    " << m.m_strNymID
               << "\n"
                  "NotaryID: " << m.m_strNotaryID
               << "\nRequest#: " << m.m_strRequestNum << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyQueryInstrumentDefinitions::reg(
    "queryInstrumentDefinitions", new StrategyQueryInstrumentDefinitions());

class StrategyQueryInstrumentDefinitionsResponse : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate("<%s\n" // Command
                           " requestNum=\"%s\"\n"
                           " success=\"%s\"\n"
                           " nymID=\"%s\"\n"
                           " notaryID=\"%s\""
                           ">\n\n",
                           m.m_strCommand.Get(), m.m_strRequestNum.Get(),
                           (m.m_bSuccess ? "true" : "false"),
                           m.m_strNymID.Get(), m.m_strNotaryID.Get());

        if (m.m_ascInReferenceTo.GetLength())
            result.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                               m.m_ascInReferenceTo.Get());

        if (m.m_bSuccess && m.m_ascPayload.GetLength())
            result.Concatenate("<stringMap>\n%s</stringMap>\n\n",
                               m.m_ascPayload.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");

        // If successful, we need to read 2 more things: inReferenceTo and
        // issuerAccount payload.
        // If failure, then we only need to read 1 thing: inReferenceTo
        // At this point, we do not send the REASON WHY if it failed.

        {
            const char* pElementExpected = "inReferenceTo";
            OTASCIIArmor& ascTextExpected = m.m_ascInReferenceTo;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        if (m.m_bSuccess) {
            const char* pElementExpected = "stringMap";
            OTASCIIArmor& ascTextExpected = m.m_ascPayload;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there,
        // OR if it was successful but the Payload isn't there, then failure.
        if (!m.m_ascInReferenceTo.GetLength() ||
            (m.m_bSuccess && !m.m_ascPayload.GetLength())) {
            otErr << "Error in OTMessage::ProcessXMLNode:\n"
                     "Expected stringMap and/or inReferenceTo elements with "
                     "text fields in "
                     "queryInstrumentDefinitionsResponse reply\n";
            return (-1); // error condition
        }

        otWarn << "\n Command: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\n NymID:    " << m.m_strNymID
               << "\n NotaryID: " << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyQueryInstrumentDefinitionsResponse::reg(
    "queryInstrumentDefinitionsResponse",
    new StrategyQueryInstrumentDefinitionsResponse());

class StrategyIssueBasket : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate("<%s\n" // Command
                           " nymID=\"%s\"\n"
                           " notaryID=\"%s\"\n"
                           " requestNum=\"%s\""
                           ">\n\n",
                           m.m_strCommand.Get(), m.m_strNymID.Get(),
                           m.m_strNotaryID.Get(), m.m_strRequestNum.Get());

        if (m.m_ascPayload.GetLength())
            result.Concatenate("<currencyBasket>\n%s</currencyBasket>\n\n",
                               m.m_ascPayload.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        {
            const char* pElementExpected = "currencyBasket";
            OTASCIIArmor& ascTextExpected = m.m_ascPayload;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the Payload isn't there, then failure.
        if (!m.m_ascPayload.GetLength()) {
            otErr << "Error in OTMessage::ProcessXMLNode:\n"
                     "Expected currencyBasket element with text fields in "
                     "issueBasket message\n";
            return (-1); // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand
               << " \nNymID:    " << m.m_strNymID
               << "\n"
                  "NotaryID: " << m.m_strNotaryID
               << "\nRequest#: " << m.m_strRequestNum << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyIssueBasket::reg("issueBasket",
                                          new StrategyIssueBasket());

class StrategyIssueBasketResponse : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate(
            "<%s\n" // Command
            " requestNum=\"%s\"\n"
            " success=\"%s\"\n"
            " accountID=\"%s\"\n" // the new basket issuer account ID
            " nymID=\"%s\"\n"
            " instrumentDefinitionID=\"%s\"\n" // the new Asset Type
            " notaryID=\"%s\""
            ">\n\n",
            m.m_strCommand.Get(), m.m_strRequestNum.Get(),
            (m.m_bSuccess ? "true" : "false"), m.m_strAcctID.Get(),
            m.m_strNymID.Get(), m.m_strInstrumentDefinitionID.Get(),
            m.m_strNotaryID.Get());

        if (m.m_ascInReferenceTo.GetLength())
            result.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                               m.m_ascInReferenceTo.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strInstrumentDefinitionID =
            xml->getAttributeValue("instrumentDefinitionID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strAcctID = xml->getAttributeValue("accountID");

        {
            const char* pElementExpected = "inReferenceTo";
            OTASCIIArmor& ascTextExpected = m.m_ascInReferenceTo;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there,
        // OR if it was successful but the Payload isn't there, then failure.
        if (!m.m_ascInReferenceTo.GetLength()) {
            otErr << "Error in OTMessage::ProcessXMLNode:\n"
                     "Expected inReferenceTo element with text fields in "
                     "issueBasketResponse reply\n";
            return (-1); // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nAccountID: " << m.m_strAcctID
               << "\nInstrumentDefinitionID: " << m.m_strInstrumentDefinitionID
               << "\n"
                  "NotaryID: " << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyIssueBasketResponse::reg(
    "issueBasketResponse", new StrategyIssueBasketResponse());

class StrategyRegisterAccount : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate("<%s\n" // Command
                           " nymID=\"%s\"\n"
                           " notaryID=\"%s\"\n"
                           " requestNum=\"%s\"\n"
                           " instrumentDefinitionID=\"%s\""
                           ">\n\n",
                           m.m_strCommand.Get(), m.m_strNymID.Get(),
                           m.m_strNotaryID.Get(), m.m_strRequestNum.Get(),
                           m.m_strInstrumentDefinitionID.Get());

        //        otErr << "DEBUG: Asset Type length: %d, Value:\n%s\n",
        // m.m_strInstrumentDefinitionID.GetLength(),
        // m.m_strInstrumentDefinitionID.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strInstrumentDefinitionID =
            xml->getAttributeValue("instrumentDefinitionID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        otWarn << "\nCommand: " << m.m_strCommand
               << " \nNymID:    " << m.m_strNymID
               << "\n"
                  "NotaryID: " << m.m_strNotaryID
               << "\nRequest#: " << m.m_strRequestNum << "\nAsset Type:\n"
               << m.m_strInstrumentDefinitionID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyRegisterAccount::reg("registerAccount",
                                              new StrategyRegisterAccount());

class StrategyRegisterAccountResponse : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate("<%s\n" // Command
                           " requestNum=\"%s\"\n"
                           " success=\"%s\"\n"
                           " accountID=\"%s\"\n"
                           " nymID=\"%s\"\n"
                           " notaryID=\"%s\""
                           ">\n\n",
                           m.m_strCommand.Get(), m.m_strRequestNum.Get(),
                           (m.m_bSuccess ? "true" : "false"),
                           m.m_strAcctID.Get(), m.m_strNymID.Get(),
                           m.m_strNotaryID.Get());

        if (m.m_ascInReferenceTo.Exists())
            result.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                               m.m_ascInReferenceTo.Get());

        if (m.m_bSuccess && m.m_ascPayload.Exists())
            result.Concatenate("<newAccount>\n%s</newAccount>\n\n",
                               m.m_ascPayload.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strAcctID = xml->getAttributeValue("accountID");

        // If successful, we need to read 2 more things: inReferenceTo and
        // issuerAccount payload.
        // If failure, then we only need to read 1 thing: inReferenceTo
        // At this point, we do not send the REASON WHY if it failed.

        {
            const char* pElementExpected = "inReferenceTo";
            OTASCIIArmor& ascTextExpected = m.m_ascInReferenceTo;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                //                return (-1); // error condition
            }
        }

        if (m.m_bSuccess) {
            const char* pElementExpected = "newAccount";
            OTASCIIArmor& ascTextExpected = m.m_ascPayload;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there,
        // OR if it was successful but the Payload isn't there, then failure.
        //
        if (m.m_bSuccess && !m.m_ascPayload.GetLength()) {
            otErr << "Error in OTMessage::ProcessXMLNode:\n"
                     "Expected newAccount element with text field, in "
                     "registerAccountResponse reply\n";
            return (-1); // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nAccountID: " << m.m_strAcctID
               << "\n"
                  "NotaryID: " << m.m_strNotaryID << "\n\n";
        //    "****New Account****:\n%s\n",
        //    m.m_ascInReferenceTo.Get(),
        // acctContents.Get()

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyRegisterAccountResponse::reg(
    "registerAccountResponse", new StrategyRegisterAccountResponse());

class StrategyGetBoxReceipt : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate(
            "<%s\n" // Command
            " nymID=\"%s\"\n"
            " notaryID=\"%s\"\n"
            " requestNum=\"%s\"\n"
            " transactionNum=\"%" PRId64 "\"\n"
            " boxType=\"%s\"\n"
            " accountID=\"%s\"" // If retrieving box receipt for Nymbox, NymID
                                // will appear in this variable.
            ">\n\n",
            m.m_strCommand.Get(), m.m_strNymID.Get(), m.m_strNotaryID.Get(),
            m.m_strRequestNum.Get(), m.m_lTransactionNum,
            (m.m_lDepth == 0)
                ? "nymbox"
                : ((m.m_lDepth == 1) ? "inbox" : "outbox"), // outbox is 2.
            m.m_strAcctID.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strAcctID = xml->getAttributeValue("accountID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        String strTransactionNum = xml->getAttributeValue("transactionNum");
        m.m_lTransactionNum =
            strTransactionNum.Exists() ? strTransactionNum.ToLong() : 0;

        const String strBoxType = xml->getAttributeValue("boxType");

        if (strBoxType.Compare("nymbox"))
            m.m_lDepth = 0;
        else if (strBoxType.Compare("inbox"))
            m.m_lDepth = 1;
        else if (strBoxType.Compare("outbox"))
            m.m_lDepth = 2;
        else {
            m.m_lDepth = 0;
            otErr << "Error in OTMessage::ProcessXMLNode:\n"
                     "Expected boxType to be inbox, outbox, or nymbox, in "
                     "getBoxReceipt\n";
            return (-1);
        }

        otWarn << "\n Command: " << m.m_strCommand
               << " \n NymID:    " << m.m_strNymID
               << "\n AccountID:    " << m.m_strAcctID
               << "\n"
                  " NotaryID: " << m.m_strNotaryID
               << "\n Request#: " << m.m_strRequestNum
               << "  Transaction#: " << m.m_lTransactionNum << "   boxType: "
               << ((m.m_lDepth == 0) ? "nymbox" : (m.m_lDepth == 1) ? "inbox"
                                                                    : "outbox")
               << "\n\n"; // outbox is 2.);

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetBoxReceipt::reg("getBoxReceipt",
                                            new StrategyGetBoxReceipt());

class StrategyGetBoxReceiptResponse : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate(
            "<%s\n" // Command
            " requestNum=\"%s\"\n"
            " success=\"%s\"\n"
            " accountID=\"%s\"\n"
            " transactionNum=\"%" PRId64 "\"\n"
            " boxType=\"%s\"\n"
            " nymID=\"%s\"\n"
            " notaryID=\"%s\""
            ">\n\n",
            m.m_strCommand.Get(), m.m_strRequestNum.Get(),
            (m.m_bSuccess ? "true" : "false"), m.m_strAcctID.Get(),
            m.m_lTransactionNum,
            (m.m_lDepth == 0)
                ? "nymbox"
                : ((m.m_lDepth == 1) ? "inbox" : "outbox"), // outbox is 2.
            m.m_strNymID.Get(),
            m.m_strNotaryID.Get());

        if (m.m_ascInReferenceTo.GetLength())
            result.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                               m.m_ascInReferenceTo.Get());

        if (m.m_bSuccess && m.m_ascPayload.GetLength())
            result.Concatenate("<boxReceipt>\n%s</boxReceipt>\n\n",
                               m.m_ascPayload.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strAcctID = xml->getAttributeValue("accountID");

        String strTransactionNum = xml->getAttributeValue("transactionNum");
        m.m_lTransactionNum =
            strTransactionNum.Exists() ? strTransactionNum.ToLong() : 0;

        const String strBoxType = xml->getAttributeValue("boxType");

        if (strBoxType.Compare("nymbox"))
            m.m_lDepth = 0;
        else if (strBoxType.Compare("inbox"))
            m.m_lDepth = 1;
        else if (strBoxType.Compare("outbox"))
            m.m_lDepth = 2;
        else {
            m.m_lDepth = 0;
            otErr << "Error in OTMessage::ProcessXMLNode:\n"
                     "Expected boxType to be inbox, outbox, or nymbox, in "
                     "getBoxReceiptResponse reply\n";
            return (-1);
        }

        // inReferenceTo contains the getBoxReceipt (original request)
        // At this point, we do not send the REASON WHY if it failed.

        {
            const char* pElementExpected = "inReferenceTo";
            OTASCIIArmor& ascTextExpected = m.m_ascInReferenceTo;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        if (m.m_bSuccess) {
            const char* pElementExpected = "boxReceipt";
            OTASCIIArmor& ascTextExpected = m.m_ascPayload;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there,
        // OR if it was successful but the Payload isn't there, then failure.
        if (!m.m_ascInReferenceTo.GetLength() ||
            (m.m_bSuccess && !m.m_ascPayload.GetLength())) {
            otErr << "Error in OTMessage::ProcessXMLNode:\n"
                     "Expected boxReceipt and/or inReferenceTo elements with "
                     "text fields in "
                     "getBoxReceiptResponse reply\n";
            return (-1); // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nAccountID: " << m.m_strAcctID
               << "\n"
                  "NotaryID: " << m.m_strNotaryID << "\n\n";
        //    "****New Account****:\n%s\n",

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetBoxReceiptResponse::reg(
    "getBoxReceiptResponse", new StrategyGetBoxReceiptResponse());

class StrategyUnregisterAccount : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate("<%s\n" // Command
                           " nymID=\"%s\"\n"
                           " notaryID=\"%s\"\n"
                           " requestNum=\"%s\"\n"
                           " accountID=\"%s\""
                           ">\n\n",
                           m.m_strCommand.Get(), m.m_strNymID.Get(),
                           m.m_strNotaryID.Get(), m.m_strRequestNum.Get(),
                           m.m_strAcctID.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strAcctID = xml->getAttributeValue("accountID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        otWarn << "\n Command: " << m.m_strCommand
               << " \n NymID:    " << m.m_strNymID
               << "\n AccountID:    " << m.m_strAcctID
               << "\n"
                  " NotaryID: " << m.m_strNotaryID
               << "\n Request#: " << m.m_strRequestNum << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyUnregisterAccount::reg(
    "unregisterAccount", new StrategyUnregisterAccount());

class StrategyUnregisterAccountResponse : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate("<%s\n" // Command
                           " requestNum=\"%s\"\n"
                           " success=\"%s\"\n"
                           " accountID=\"%s\"\n"
                           " nymID=\"%s\"\n"
                           " notaryID=\"%s\""
                           ">\n\n",
                           m.m_strCommand.Get(), m.m_strRequestNum.Get(),
                           (m.m_bSuccess ? "true" : "false"),
                           m.m_strAcctID.Get(), m.m_strNymID.Get(),
                           m.m_strNotaryID.Get());

        if (m.m_ascInReferenceTo.GetLength())
            result.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                               m.m_ascInReferenceTo.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strAcctID = xml->getAttributeValue("accountID");

        // inReferenceTo contains the unregisterAccount (original request)
        // At this point, we do not send the REASON WHY if it failed.

        {
            const char* pElementExpected = "inReferenceTo";
            OTASCIIArmor& ascTextExpected = m.m_ascInReferenceTo;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there, then failure.
        if (!m.m_ascInReferenceTo.GetLength()) {
            otErr << "Error in OTMessage::ProcessXMLNode:\n"
                     "Expected inReferenceTo element with text fields in "
                     "unregisterAccountResponse reply\n";
            return (-1); // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nAccountID: " << m.m_strAcctID
               << "\n"
                  "NotaryID: " << m.m_strNotaryID << "\n\n";
        //    "****New Account****:\n%s\n",
        //    m.m_ascInReferenceTo.Get(),
        // acctContents.Get()

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyUnregisterAccountResponse::reg(
    "unregisterAccountResponse", new StrategyUnregisterAccountResponse());

class StrategyNotarizeTransaction : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        // the Payload contains an ascii-armored OTLedger object.
        result.Concatenate("<%s\n" // Command
                           " nymID=\"%s\"\n"
                           " nymboxHash=\"%s\"\n"
                           " notaryID=\"%s\"\n"
                           " accountID=\"%s\"\n"
                           " requestNum=\"%s\""
                           " >\n\n",
                           m.m_strCommand.Get(), m.m_strNymID.Get(),
                           m.m_strNymboxHash.Get(), m.m_strNotaryID.Get(),
                           m.m_strAcctID.Get(), m.m_strRequestNum.Get());

        // I would check if this was empty, but it should never be empty...
        // famous last words.
        if (m.m_ascPayload.GetLength())
            result.Concatenate("<accountLedger>\n%s</accountLedger>\n\n",
                               m.m_ascPayload.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNymboxHash = xml->getAttributeValue("nymboxHash");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strAcctID = xml->getAttributeValue("accountID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        {
            const char* pElementExpected = "accountLedger";
            OTASCIIArmor& ascTextExpected = m.m_ascPayload;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        otWarn << "\n Command: " << m.m_strCommand
               << " \n NymID:    " << m.m_strNymID
               << "\n AccountID:    " << m.m_strAcctID
               << "\n"
                  " NotaryID: " << m.m_strNotaryID
               << "\n Request#: " << m.m_strRequestNum << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyNotarizeTransaction::reg(
    "notarizeTransaction", new StrategyNotarizeTransaction());

class StrategyNotarizeTransactionResponse : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        // the Payload contains an ascii-armored OTLedger object.
        result.Concatenate("<%s\n" // Command
                           " requestNum=\"%s\"\n"
                           " success=\"%s\"\n"
                           " nymID=\"%s\"\n"
                           " notaryID=\"%s\"\n"
                           " accountID=\"%s\""
                           " >\n\n",
                           m.m_strCommand.Get(), m.m_strRequestNum.Get(),
                           (m.m_bSuccess ? "true" : "false"),
                           m.m_strNymID.Get(), m.m_strNotaryID.Get(),
                           m.m_strAcctID.Get());

        if (m.m_ascInReferenceTo.GetLength())
            result.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                               m.m_ascInReferenceTo.Get());

        // I would check if this was empty, but it should never be empty...
        // famous last words.
        if (m.m_ascPayload.GetLength())
            result.Concatenate("<responseLedger>\n%s</responseLedger>\n\n",
                               m.m_ascPayload.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strAcctID = xml->getAttributeValue("accountID");

        // If successful or failure, we need to read 2 more things:
        // inReferenceTo and the responseLedger payload.
        // At this point, we do not send the REASON WHY if it failed.

        {
            const char* pElementExpected = "inReferenceTo";
            OTASCIIArmor& ascTextExpected = m.m_ascInReferenceTo;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        {
            const char* pElementExpected = "responseLedger";
            OTASCIIArmor& ascTextExpected = m.m_ascPayload;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there, or the Payload isn't
        // there, then failure.
        if (!m.m_ascInReferenceTo.GetLength() ||
            (!m.m_ascPayload.GetLength())) {
            otErr << "Error in OTMessage::ProcessXMLNode:\n"
                     "Expected responseLedger and/or inReferenceTo elements "
                     "with text fields in "
                     "notarizeTransactionResponse reply\n";
            return (-1); // error condition
        }

        //        OTString acctContents(m.m_ascPayload);
        otWarn << "\n Command: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\n NymID:    " << m.m_strNymID
               << "\n AccountID: " << m.m_strAcctID
               << "\n NotaryID: " << m.m_strNotaryID << "\n\n";
        //    "****New Account****:\n%s\n",
        //    m.m_ascInReferenceTo.Get(),
        // acctContents.Get()

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyNotarizeTransactionResponse::reg(
    "notarizeTransactionResponse", new StrategyNotarizeTransactionResponse());

class StrategyGetTransactionNumbers : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate("<%s\n" // Command
                           " nymID=\"%s\"\n"
                           " nymboxHash=\"%s\"\n"
                           " notaryID=\"%s\"\n"
                           " requestNum=\"%s\""
                           " >\n\n",
                           m.m_strCommand.Get(), m.m_strNymID.Get(),
                           m.m_strNymboxHash.Get(), m.m_strNotaryID.Get(),
                           m.m_strRequestNum.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNymboxHash = xml->getAttributeValue("nymboxHash");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        otWarn << "\n Command: " << m.m_strCommand
               << " \n NymID:    " << m.m_strNymID
               << "\n"
                  " NotaryID: " << m.m_strNotaryID
               << "\n Request#: " << m.m_strRequestNum << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetTransactionNumbers::reg(
    "getTransactionNumbers", new StrategyGetTransactionNumbers());

class StrategyGetTransactionNumbersResponse : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate("<%s\n" // Command
                           " requestNum=\"%s\"\n"
                           " success=\"%s\"\n"
                           " nymID=\"%s\"\n"
                           " nymboxHash=\"%s\"\n"
                           " notaryID=\"%s\""
                           " >\n\n",
                           m.m_strCommand.Get(), m.m_strRequestNum.Get(),
                           (m.m_bSuccess ? "true" : "false"),
                           m.m_strNymID.Get(), m.m_strNymboxHash.Get(),
                           m.m_strNotaryID.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNymboxHash = xml->getAttributeValue("nymboxHash");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");

        otWarn << "\n Command: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\n NymID:    " << m.m_strNymID
               << "\n"
                  " NotaryID: " << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetTransactionNumbersResponse::reg(
    "getTransactionNumbersResponse",
    new StrategyGetTransactionNumbersResponse());

class StrategyGetNymbox : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate("<%s\n" // Command
                           " nymID=\"%s\"\n"
                           " notaryID=\"%s\"\n"
                           " requestNum=\"%s\""
                           " >\n\n",
                           m.m_strCommand.Get(), m.m_strNymID.Get(),
                           m.m_strNotaryID.Get(), m.m_strRequestNum.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID
               << "\nRequest #: " << m.m_strRequestNum << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetNymbox::reg("getNymbox", new StrategyGetNymbox());

class StrategyGetNymboxResponse : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        // the Payload contains an ascii-armored OTLedger object.
        result.Concatenate("<%s\n" // Command
                           " requestNum=\"%s\"\n"
                           " success=\"%s\"\n"
                           " nymID=\"%s\"\n"
                           " nymboxHash=\"%s\"\n"
                           " notaryID=\"%s\""
                           " >\n\n",
                           m.m_strCommand.Get(), m.m_strRequestNum.Get(),
                           (m.m_bSuccess ? "true" : "false"),
                           m.m_strNymID.Get(), m.m_strNymboxHash.Get(),
                           m.m_strNotaryID.Get());

        if (!m.m_bSuccess && m.m_ascInReferenceTo.GetLength())
            result.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                               m.m_ascInReferenceTo.Get());

        // I would check if this was empty, but it should never be empty...
        // famous last words.
        if (m.m_bSuccess && m.m_ascPayload.GetLength())
            result.Concatenate("<nymboxLedger>\n%s</nymboxLedger>\n\n",
                               m.m_ascPayload.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNymboxHash = xml->getAttributeValue("nymboxHash");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");

        const char* pElementExpected;
        if (m.m_bSuccess)
            pElementExpected = "nymboxLedger";
        else
            pElementExpected = "inReferenceTo";

        OTASCIIArmor ascTextExpected;

        if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                  pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m.m_strCommand
                  << ".\n";
            return (-1); // error condition
        }

        if (m.m_bSuccess)
            m.m_ascPayload = ascTextExpected;
        else
            m.m_ascInReferenceTo = ascTextExpected;

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\n"
                  "NotaryID: " << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetNymboxResponse::reg(
    "getNymboxResponse", new StrategyGetNymboxResponse());

class StrategyGetAccountData : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate("<%s\n" // Command
                           " nymID=\"%s\"\n"
                           " notaryID=\"%s\"\n"
                           " accountID=\"%s\"\n"
                           " requestNum=\"%s\""
                           " >\n\n",
                           m.m_strCommand.Get(), m.m_strNymID.Get(),
                           m.m_strNotaryID.Get(), m.m_strAcctID.Get(),
                           m.m_strRequestNum.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strAcctID = xml->getAttributeValue("accountID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID
               << "\nAccountID:    " << m.m_strAcctID
               << "\nRequest #: " << m.m_strRequestNum << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetAccountData::reg("getAccountData",
                                             new StrategyGetAccountData());

class StrategyGetAccountDataResponse : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        // the Payload contains a STRING_MAP containing the OTAccount,
        // plus the inbox and outbox for that acct..
        //
        result.Concatenate("<%s\n" // Command
                           " requestNum=\"%s\"\n"
                           " success=\"%s\"\n"
                           " inboxHash=\"%s\"\n"
                           " outboxHash=\"%s\"\n"
                           " nymID=\"%s\"\n"
                           " notaryID=\"%s\"\n"
                           " accountID=\"%s\""
                           " >\n\n",
                           m.m_strCommand.Get(), m.m_strRequestNum.Get(),
                           (m.m_bSuccess ? "true" : "false"),
                           m.m_strInboxHash.Get(), m.m_strOutboxHash.Get(),
                           m.m_strNymID.Get(), m.m_strNotaryID.Get(),
                           m.m_strAcctID.Get());

        if (!m.m_bSuccess && m.m_ascInReferenceTo.GetLength())
            result.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                               m.m_ascInReferenceTo.Get());

        if (m.m_bSuccess) {
            if (m.m_ascPayload.GetLength())
                result.Concatenate("<account>\n%s</account>\n\n",
                                   m.m_ascPayload.Get());
            if (m.m_ascPayload2.GetLength())
                result.Concatenate("<inbox>\n%s</inbox>\n\n",
                                   m.m_ascPayload2.Get());
            if (m.m_ascPayload3.GetLength())
                result.Concatenate("<outbox>\n%s</outbox>\n\n",
                                   m.m_ascPayload3.Get());
        }

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strAcctID = xml->getAttributeValue("accountID");
        m.m_strInboxHash = xml->getAttributeValue("inboxHash");
        m.m_strOutboxHash = xml->getAttributeValue("outboxHash");

        if (m.m_bSuccess) {
            if (!Contract::LoadEncodedTextFieldByName(xml, m.m_ascPayload,
                                                      "account")) {
                otErr << "Error in OTMessage::ProcessXMLNode: Expected account "
                         "element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }

            if (!Contract::LoadEncodedTextFieldByName(xml, m.m_ascPayload2,
                                                      "inbox")) {
                otErr << "Error in OTMessage::ProcessXMLNode: Expected inbox"
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }

            if (!Contract::LoadEncodedTextFieldByName(xml, m.m_ascPayload3,
                                                      "outbox")) {
                otErr << "Error in OTMessage::ProcessXMLNode: Expected outbox"
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        if (!m.m_bSuccess) {
            if (!Contract::LoadEncodedTextFieldByName(xml, m.m_ascInReferenceTo,
                                                      "inReferenceTo")) {
                otErr << "Error in OTMessage::ProcessXMLNode: Expected "
                         "inReferenceTo element with text field, for "
                      << m.m_strCommand << ".\n";
                return (-1); // error condition
            }
        }

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nAccountID:    " << m.m_strAcctID
               << "\n"
                  "NotaryID: " << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetAccountDataResponse::reg(
    "getAccountDataResponse", new StrategyGetAccountDataResponse());

class StrategyGetInstrumentDefinition : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate(
            "<%s\n" // Command
            " nymID=\"%s\"\n"
            " notaryID=\"%s\"\n"
            " instrumentDefinitionID=\"%s\"\n"
            " requestNum=\"%s\""
            " >\n\n",
            m.m_strCommand.Get(), m.m_strNymID.Get(), m.m_strNotaryID.Get(),
            m.m_strInstrumentDefinitionID.Get(), m.m_strRequestNum.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strInstrumentDefinitionID =
            xml->getAttributeValue("instrumentDefinitionID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID
               << "\nAsset Type:    " << m.m_strInstrumentDefinitionID
               << "\nRequest #: " << m.m_strRequestNum << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetInstrumentDefinition::reg(
    "getInstrumentDefinition", new StrategyGetInstrumentDefinition());

class StrategyGetInstrumentDefinitionResponse : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        // the Payload contains an ascii-armored OTAssetContract object.
        result.Concatenate("<%s\n" // Command
                           " requestNum=\"%s\"\n"
                           " success=\"%s\"\n"
                           " nymID=\"%s\"\n"
                           " notaryID=\"%s\"\n"
                           " instrumentDefinitionID=\"%s\""
                           " >\n\n",
                           m.m_strCommand.Get(), m.m_strRequestNum.Get(),
                           (m.m_bSuccess ? "true" : "false"),
                           m.m_strNymID.Get(), m.m_strNotaryID.Get(),
                           m.m_strInstrumentDefinitionID.Get());

        if (!m.m_bSuccess && m.m_ascInReferenceTo.GetLength())
            result.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                               m.m_ascInReferenceTo.Get());

        // I would check if this was empty, but it should never be empty...
        // famous last words.
        if (m.m_bSuccess && m.m_ascPayload.GetLength())
            result.Concatenate(
                "<instrumentDefinition>\n%s</instrumentDefinition>\n\n",
                m.m_ascPayload.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strInstrumentDefinitionID =
            xml->getAttributeValue("instrumentDefinitionID");

        const char* pElementExpected;
        if (m.m_bSuccess)
            pElementExpected = "instrumentDefinition";
        else
            pElementExpected = "inReferenceTo";

        OTASCIIArmor ascTextExpected;

        if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                  pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m.m_strCommand
                  << ".\n";
            return (-1); // error condition
        }

        if (m.m_bSuccess)
            m.m_ascPayload = ascTextExpected;
        else
            m.m_ascInReferenceTo = ascTextExpected;

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nInstrument Definition ID:    "
               << m.m_strInstrumentDefinitionID
               << "\n"
                  "NotaryID: " << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetInstrumentDefinitionResponse::reg(
    "getInstrumentDefinitionResponse",
    new StrategyGetInstrumentDefinitionResponse());

class StrategyGetMint : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate(
            "<%s\n" // Command
            " nymID=\"%s\"\n"
            " notaryID=\"%s\"\n"
            " instrumentDefinitionID=\"%s\"\n"
            " requestNum=\"%s\""
            " >\n\n",
            m.m_strCommand.Get(), m.m_strNymID.Get(), m.m_strNotaryID.Get(),
            m.m_strInstrumentDefinitionID.Get(), m.m_strRequestNum.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strInstrumentDefinitionID =
            xml->getAttributeValue("instrumentDefinitionID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID
               << "\nAsset Type:    " << m.m_strInstrumentDefinitionID
               << "\nRequest #: " << m.m_strRequestNum << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetMint::reg("getMint", new StrategyGetMint());

class StrategyGetMintResponse : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        // the Payload contains an ascii-armored OTMint object.
        result.Concatenate("<%s\n" // Command
                           " requestNum=\"%s\"\n"
                           " success=\"%s\"\n"
                           " nymID=\"%s\"\n"
                           " notaryID=\"%s\"\n"
                           " instrumentDefinitionID=\"%s\""
                           " >\n\n",
                           m.m_strCommand.Get(), m.m_strRequestNum.Get(),
                           (m.m_bSuccess ? "true" : "false"),
                           m.m_strNymID.Get(), m.m_strNotaryID.Get(),
                           m.m_strInstrumentDefinitionID.Get());

        if (!m.m_bSuccess && m.m_ascInReferenceTo.GetLength())
            result.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                               m.m_ascInReferenceTo.Get());

        // I would check if this was empty, but it should never be empty...
        // famous last words.
        if (m.m_bSuccess && m.m_ascPayload.GetLength())
            result.Concatenate("<mint>\n%s</mint>\n\n", m.m_ascPayload.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strInstrumentDefinitionID =
            xml->getAttributeValue("instrumentDefinitionID");

        const char* pElementExpected;
        if (m.m_bSuccess)
            pElementExpected = "mint";
        else
            pElementExpected = "inReferenceTo";

        OTASCIIArmor ascTextExpected;

        if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                  pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m.m_strCommand
                  << ".\n";
            return (-1); // error condition
        }

        if (m.m_bSuccess)
            m.m_ascPayload = ascTextExpected;
        else
            m.m_ascInReferenceTo = ascTextExpected;

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nInstrument Definition ID:    "
               << m.m_strInstrumentDefinitionID
               << "\n"
                  "NotaryID: " << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetMintResponse::reg("getMintResponse",
                                              new StrategyGetMintResponse());

class StrategyProcessInbox : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        // the Payload contains an ascii-armored OTLedger object.
        result.Concatenate("<%s\n" // Command
                           " nymID=\"%s\"\n"
                           " nymboxHash=\"%s\"\n"
                           " notaryID=\"%s\"\n"
                           " accountID=\"%s\"\n"
                           " requestNum=\"%s\""
                           " >\n\n",
                           m.m_strCommand.Get(), m.m_strNymID.Get(),
                           m.m_strNymboxHash.Get(), m.m_strNotaryID.Get(),
                           m.m_strAcctID.Get(), m.m_strRequestNum.Get());

        // I would check if this was empty, but it should never be empty...
        // famous last words.
        if (m.m_ascPayload.GetLength())
            result.Concatenate("<processLedger>\n%s</processLedger>\n\n",
                               m.m_ascPayload.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNymboxHash = xml->getAttributeValue("nymboxHash");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strAcctID = xml->getAttributeValue("accountID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        {
            const char* pElementExpected = "processLedger";
            OTASCIIArmor& ascTextExpected = m.m_ascPayload;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        otWarn << "\n Command: " << m.m_strCommand
               << " \n NymID:    " << m.m_strNymID
               << "\n AccountID:    " << m.m_strAcctID
               << "\n"
                  " NotaryID: " << m.m_strNotaryID
               << "\n Request#: " << m.m_strRequestNum << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyProcessInbox::reg("processInbox",
                                           new StrategyProcessInbox());

class StrategyProcessInboxResponse : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        // the Payload contains an ascii-armored OTLedger object.
        result.Concatenate("<%s\n" // Command
                           " requestNum=\"%s\"\n"
                           " success=\"%s\"\n"
                           " nymID=\"%s\"\n"
                           " notaryID=\"%s\"\n"
                           " accountID=\"%s\""
                           " >\n\n",
                           m.m_strCommand.Get(), m.m_strRequestNum.Get(),
                           (m.m_bSuccess ? "true" : "false"),
                           m.m_strNymID.Get(), m.m_strNotaryID.Get(),
                           m.m_strAcctID.Get());

        if (m.m_ascInReferenceTo.GetLength())
            result.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                               m.m_ascInReferenceTo.Get());

        // I would check if this was empty, but it should never be empty...
        // famous last words.
        if (m.m_ascPayload.GetLength())
            result.Concatenate("<responseLedger>\n%s</responseLedger>\n\n",
                               m.m_ascPayload.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strAcctID = xml->getAttributeValue("accountID");

        // If successful or failure, we need to read 2 more things:
        // inReferenceTo and the responseLedger payload.
        // At this point, we do not send the REASON WHY if it failed.

        {
            const char* pElementExpected = "inReferenceTo";
            OTASCIIArmor& ascTextExpected = m.m_ascInReferenceTo;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        {
            const char* pElementExpected = "responseLedger";
            OTASCIIArmor& ascTextExpected = m.m_ascPayload;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there, or the Payload isn't
        // there, then failure.
        if (!m.m_ascInReferenceTo.GetLength() ||
            (!m.m_ascPayload.GetLength())) {
            otErr << "Error in OTMessage::ProcessXMLNode:\n"
                     "Expected responseLedger and/or inReferenceTo elements "
                     "with text fields in "
                     "processInboxResponse reply\n";
            return (-1); // error condition
        }

        otWarn << "\n Command: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\n NymID:    " << m.m_strNymID
               << "\n AccountID: " << m.m_strAcctID
               << "\n NotaryID: " << m.m_strNotaryID << "\n\n";
        //    "****New Account****:\n%s\n",

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyProcessInboxResponse::reg(
    "processInboxResponse", new StrategyProcessInboxResponse());

class StrategyProcessNymbox : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        // the Payload contains an ascii-armored OTLedger object.
        result.Concatenate("<%s\n" // Command
                           " nymID=\"%s\"\n"
                           " nymboxHash=\"%s\"\n"
                           " notaryID=\"%s\"\n"
                           " requestNum=\"%s\""
                           " >\n\n",
                           m.m_strCommand.Get(), m.m_strNymID.Get(),
                           m.m_strNymboxHash.Get(), m.m_strNotaryID.Get(),
                           m.m_strRequestNum.Get());

        // I would check if this was empty, but it should never be empty...
        // famous last words.
        if (m.m_ascPayload.GetLength())
            result.Concatenate("<processLedger>\n%s</processLedger>\n\n",
                               m.m_ascPayload.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNymboxHash = xml->getAttributeValue("nymboxHash");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        {
            const char* pElementExpected = "processLedger";
            OTASCIIArmor& ascTextExpected = m.m_ascPayload;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        otWarn << "\n Command: " << m.m_strCommand
               << " \n NymID:    " << m.m_strNymID
               << "\n"
                  " NotaryID: " << m.m_strNotaryID
               << "\n Request#: " << m.m_strRequestNum << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyProcessNymbox::reg("processNymbox",
                                            new StrategyProcessNymbox());

class StrategyProcessNymboxResponse : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        // the Payload contains an ascii-armored OTLedger object.
        result.Concatenate("<%s\n" // Command
                           " requestNum=\"%s\"\n"
                           " success=\"%s\"\n"
                           " nymID=\"%s\"\n"
                           " notaryID=\"%s\""
                           " >\n\n",
                           m.m_strCommand.Get(), m.m_strRequestNum.Get(),
                           (m.m_bSuccess ? "true" : "false"),
                           m.m_strNymID.Get(), m.m_strNotaryID.Get());

        if (m.m_ascInReferenceTo.GetLength())
            result.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                               m.m_ascInReferenceTo.Get());

        // I would check if this was empty, but it should never be empty...
        // famous last words.
        if (m.m_ascPayload.GetLength())
            result.Concatenate("<responseLedger>\n%s</responseLedger>\n\n",
                               m.m_ascPayload.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");

        // If successful or failure, we need to read 2 more things:
        // inReferenceTo and the responseLedger payload.
        // At this point, we do not send the REASON WHY if it failed.

        {
            const char* pElementExpected = "inReferenceTo";
            OTASCIIArmor& ascTextExpected = m.m_ascInReferenceTo;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        {
            const char* pElementExpected = "responseLedger";
            OTASCIIArmor& ascTextExpected = m.m_ascPayload;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there, or the Payload isn't
        // there, then failure.
        if (!m.m_ascInReferenceTo.GetLength() ||
            (!m.m_ascPayload.GetLength())) {
            otErr << "Error in OTMessage::ProcessXMLNode:\n"
                     "Expected responseLedger and/or inReferenceTo elements "
                     "with text fields in "
                     "processNymboxResponse reply\n";
            return (-1); // error condition
        }

        otWarn << "\n Command: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\n NymID:    " << m.m_strNymID
               << "\n"
                  " NotaryID: " << m.m_strNotaryID << "\n\n";
        //    "****New Account****:\n%s\n",

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyProcessNymboxResponse::reg(
    "processNymboxResponse", new StrategyProcessNymboxResponse());

class StrategyTriggerClause : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate(
            "<%s\n" // Command
            " nymID=\"%s\"\n"
            " nymboxHash=\"%s\"\n"
            " notaryID=\"%s\"\n"
            " smartContractID=\"%" PRId64 "\"\n"
            " clauseName=\"%s\"\n"
            " hasParam=\"%s\"\n"
            " requestNum=\"%s\""
            " >\n\n",
            m.m_strCommand.Get(), m.m_strNymID.Get(), m.m_strNymboxHash.Get(),
            m.m_strNotaryID.Get(), m.m_lTransactionNum,
            m.m_strNymID2.Get(), // clause name is stored here for this message.
            (m.m_ascPayload.Exists()) ? "true" : "false",
            m.m_strRequestNum.Get());

        if (m.m_ascPayload.Exists())
            result.Concatenate("<parameter>\n%s</parameter>\n\n",
                               m.m_ascPayload.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNymboxHash = xml->getAttributeValue("nymboxHash");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strNymID2 = xml->getAttributeValue("clauseName");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        const String strHasParam = xml->getAttributeValue("hasParam");

        String strTransactionNum = xml->getAttributeValue("smartContractID");
        if (strTransactionNum.Exists())
            m.m_lTransactionNum = strTransactionNum.ToLong();

        if (strHasParam.Compare("true")) {
            const char* pElementExpected = "parameter";
            OTASCIIArmor ascTextExpected;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
            else
                m.m_ascPayload = ascTextExpected;
        }

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID
               << "\nClause TransNum and Name:  " << m.m_lTransactionNum
               << "  /  " << m.m_strNymID2 << " \n"
                                              "Request #: " << m.m_strRequestNum
               << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyTriggerClause::reg("triggerClause",
                                            new StrategyTriggerClause());

class StrategyTriggerClauseResponse : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        // the Payload contains an ascii-armored OTMint object.
        result.Concatenate("<%s\n" // Command
                           " requestNum=\"%s\"\n"
                           " success=\"%s\"\n"
                           " nymID=\"%s\"\n"
                           " notaryID=\"%s\""
                           " >\n\n",
                           m.m_strCommand.Get(), m.m_strRequestNum.Get(),
                           (m.m_bSuccess ? "true" : "false"),
                           m.m_strNymID.Get(), m.m_strNotaryID.Get());

        if (m.m_ascInReferenceTo.GetLength())
            result.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                               m.m_ascInReferenceTo.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");

        const char* pElementExpected = "inReferenceTo";

        OTASCIIArmor ascTextExpected;

        if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                  pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m.m_strCommand
                  << ".\n";
            return (-1); // error condition
        }

        m.m_ascInReferenceTo = ascTextExpected;

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "   NotaryID: " << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyTriggerClauseResponse::reg(
    "triggerClauseResponse", new StrategyTriggerClauseResponse());

class StrategyGetMarketList : public OTMessageStrategy
{
public:
    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate("<%s\n"
                           " requestNum=\"%s\"\n"
                           " nymID=\"%s\"\n"
                           " notaryID=\"%s\""
                           ">\n\n",
                           m.m_strCommand.Get(), m.m_strRequestNum.Get(),
                           m.m_strNymID.Get(), m.m_strNotaryID.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    virtual int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID
               << "\nRequest #: " << m.m_strRequestNum << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetMarketList::reg("getMarketList",
                                            new StrategyGetMarketList());

class StrategyGetMarketListResponse : public OTMessageStrategy
{
public:
    virtual int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        //      std::cerr << m_xmlUnsigned << std::endl;
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");

        String strDepth = xml->getAttributeValue("depth");

        if (strDepth.GetLength() > 0) m.m_lDepth = strDepth.ToLong();

        const char* pElementExpected = nullptr;
        if (m.m_bSuccess && (m.m_lDepth > 0))
            pElementExpected = "messagePayload";
        else if (!m.m_bSuccess)
            pElementExpected = "inReferenceTo";

        if (nullptr != pElementExpected) {
            OTASCIIArmor ascTextExpected;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }

            if (m.m_bSuccess)
                m.m_ascPayload.Set(ascTextExpected);
            else
                m.m_ascInReferenceTo.Set(ascTextExpected);
        }

        if (m.m_bSuccess)
            otWarn << "\nCommand: " << m.m_strCommand << "   "
                   << (m.m_bSuccess ? "SUCCESS" : "FAILED")
                   << "\nNymID:    " << m.m_strNymID
                   << "\n NotaryID: " << m.m_strNotaryID
                   << "\n\n"; // m_ascPayload.Get()
        else
            otWarn << "\nCommand: " << m.m_strCommand << "   "
                   << (m.m_bSuccess ? "SUCCESS" : "FAILED")
                   << "\nNymID:    " << m.m_strNymID
                   << "\n NotaryID: " << m.m_strNotaryID
                   << "\n\n"; // m_ascInReferenceTo.Get()

        return 1;
    }

    virtual String writeXml(Message& m)
    {
        String result;
        result.Concatenate("<%s\n"
                           " requestNum=\"%s\"\n"
                           " success=\"%s\"\n"
                           " nymID=\"%s\"\n"
                           " notaryID=\"%s\"\n"
                           " depth=\"%" PRId64 "\""
                           ">\n\n",
                           m.m_strCommand.Get(), m.m_strRequestNum.Get(),
                           (m.m_bSuccess ? "true" : "false"),
                           m.m_strNymID.Get(), m.m_strNotaryID.Get(),
                           m.m_lDepth);

        if (m.m_bSuccess && (m.m_ascPayload.GetLength() > 2) &&
            (m.m_lDepth > 0))
            result.Concatenate("<messagePayload>\n%s</messagePayload>\n\n",
                               m.m_ascPayload.Get());
        else if (!m.m_bSuccess && (m.m_ascInReferenceTo.GetLength() > 2))
            result.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                               m.m_ascInReferenceTo.Get());

        result.Concatenate("</%s>\n\n", m.m_strCommand.Get());
        return result;
    }

    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetMarketListResponse::reg(
    "getMarketListResponse", new StrategyGetMarketListResponse());

} // namespace opentxs
