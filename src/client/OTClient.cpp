/************************************************************
 *
 *  OTClient.cpp
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

#include <opentxs/client/OTClient.hpp>
#include <opentxs/client/OTWallet.hpp>
#include "Helpers.hpp"

#include <opentxs/ext/OTPayment.hpp>
#include <opentxs/cash/Mint.hpp>
#include <opentxs/cash/Purse.hpp>
#include <opentxs/cash/Token.hpp>
#include <opentxs/basket/Basket.hpp>
#include <opentxs/core/recurring/OTPaymentPlan.hpp>
#include <opentxs/core/Account.hpp>
#include <opentxs/core/AssetContract.hpp>
#include <opentxs/core/crypto/OTAsymmetricKey.hpp>
#include <opentxs/core/Cheque.hpp>
#include <opentxs/core/crypto/OTEnvelope.hpp>
#include <opentxs/core/util/OTFolders.hpp>
#include <opentxs/core/Identifier.hpp>
#include <opentxs/core/Ledger.hpp>
#include <opentxs/core/Log.hpp>
#include <opentxs/core/Message.hpp>
#include <opentxs/core/crypto/OTNymOrSymmetricKey.hpp>
#include <opentxs/core/OTData.hpp>
#include <opentxs/core/Nym.hpp>
#include <opentxs/core/OTServerContract.hpp>
#include <opentxs/core/OTStorage.hpp>
#include <opentxs/core/String.hpp>
#include <opentxs/core/trade/OTOffer.hpp>
#include <opentxs/core/trade/OTTrade.hpp>
#include <opentxs/core/util/StringUtils.hpp>

#include <memory>
#include <cstdio>

namespace opentxs
{

OTClient::OTClient(OTWallet* theWallet)
    : m_pConnection()
    , m_pWallet(theWallet)
    , m_MessageBuffer()
    , m_MessageOutbuffer()
{
}

bool OTClient::connect(OTServerContract* contract, Nym* nym)
{
    m_pConnection.reset(new OTServerConnection(this, contract, nym));
    return true;
}

void OTClient::ProcessMessageOut(OTServerContract* pServerContract, Nym* pNym,
                                 const Message& theMessage)
{
    String strMessage(theMessage);

    // WHAT DOES THIS MEAN?

    // It means that later, if a message with a certain request number
    // fails to reply, or show its face in the replies box, then I will
    // have the option to look it up in the Outbuffer, based on that
    // same request number, and send a re-try, or claw back any transaction
    // numbers that might be on that message.

    // Should probably add an API call for specifically doing this, agnostic
    // to whatever kind of transaction it actually is. Something like,
    // OT_API_Message_HarvestClosingNumbers, and
    // OT_API_Message_HarvestAllNumbers

    // So I can save the request number when sending a message, check for it
    // later in the Nymbox, and then worst case, look it up in the Outbuffer and
    // get my fucking transaction numbers back again!

    std::unique_ptr<Message> pMsg(new Message());
    if (pMsg->LoadContractFromString(strMessage))
        m_MessageOutbuffer.AddSentMessage(*(pMsg.release()));

    if (!m_pConnection) {
        connect(pServerContract, pNym);
    }

    bool sent = m_pConnection->send(theMessage);
    if (!sent) {
        otErr << "Error sending message!\n";
        OT_FAIL;
    }

    bool received = m_pConnection->receive();
    if (!received) {
        otErr << "Error receiving message!\n";
        OT_FAIL;
    }
}

/// This is standard behavior for the Nymbox (NOT the inbox.)
/// That is, to just accept everything there.
//
bool OTClient::AcceptEntireNymbox(Ledger& theNymbox,
                                  const Identifier& theNotaryID,
                                  const OTServerContract& theServerContract,
                                  Nym& theNym, Message& theMessage)
{
    if (theNymbox.GetTransactionCount() < 1) {
        // If there aren't any notices in the nymbox, no point wasting a # to
        // process an empty box.
        otLog4 << __FUNCTION__ << ": Nymbox is empty.\n";

        return false;
    }
    else if (!theNymbox.VerifyAccount(theNym)) {
        // If there aren't any notices in the nymbox, no point wasting a # to
        // process an empty box.
        otErr << __FUNCTION__ << ": Error: VerifyAccount() failed.\n";
        return false;
    }
    Nym* pNym = &theNym;

    const Identifier theNymID(*pNym);
    const String strNotaryID(theNotaryID), strNymID(theNymID);

    int64_t lHighestNum = 0;
    // get the last/current highest transaction number for the notaryID.
    // (making sure we're not being slipped any new ones with a lower value
    // than this.)
    const bool bGotHighestNum = pNym->GetHighestNum(strNotaryID, lHighestNum);

    // Contrasting Inbox and Nymbox.
    //
    // In "AcceptEntireInbox", I have to have a transaction number in order to
    // accept the inbox.
    // But I ALSO need to RECEIVE my transaction number THROUGH an inbox, so the
    // server can get
    // my signature on that number (that's the only way to hold me responsible
    // for it, AND to
    // later prove I'm NOT responsible for it when it's spent, without having to
    // worry about
    // saving account history forever, via so-called "destruction of account
    // history.")
    //
    // So how can I receive a number, if I don't have anymore?  My solution is
    // to receive all
    // transaction numbers through the NYMBOX, which is associated with Nym
    // instead of asset account.
    // That is: you RECEIVE numbers through the Nymbox, and you SPEND numbers
    // through the Inbox.
    //
    // (You can also receive messages through your nymbox.)  This way, I can
    // require a transaction
    // number for an INBOX (since asset accounts can have changing balances) but
    // I do NOT have
    // to require one for processing the NYMBOX (since users HAVE NO balances.)
    // I can still
    // get the signed receipt during this time in order to satisfy destruction
    // of acct history.
    // Perfect!
    //
    // Due to all this, lStoredTransactionNumber will be 0 for now.  If I have
    // to assign a number
    // to it, then I will (probably the request number) but I will NOT be using
    // a real
    // transaction number here, since this is the NYMBOX.
    //
    int64_t lStoredTransactionNumber = 0;

    // the message to the server will contain a ledger to be processed for a
    // specific acct. (in this case no acct, but user ID used twice instead.)
    Ledger processLedger(theNymbox.GetNymID(), theNymbox.GetNymID(),
                         theNotaryID);

    // bGenerateFile defaults to false on GenerateLedger call, so I left out the
    // false.
    processLedger.GenerateLedger(theNymbox.GetNymID(), theNotaryID,
                                 Ledger::message); // Can't just use one of
                                                   // these. It either has to
                                                   // be read out of a file or
    // a string, or it has to be generated. So you construct it, then you either
    // call GenerateLedger or LoadInbox, then you call VerifyContractID to make
    // sure
    // it loaded securely. (No need to verify if you just generated it.)

    OTTransaction* pAcceptTransaction = OTTransaction::GenerateTransaction(
        theNymbox.GetNymID(), theNymbox.GetNymID(), theNotaryID,
        OTTransaction::processNymbox, lStoredTransactionNumber);

    // This insures that the ledger will handle cleaning up the transaction, so
    // I don't have to delete it later.
    processLedger.AddTransaction(*pAcceptTransaction);

    // loop through the transactions in theNymbox, and create corresponding
    // "accept" items
    // for each one of the transfer requests. Each of those items will go into a
    // single
    // "process nymbox" transaction that I will add to the processledger and
    // thus to the
    // outgoing message.

    // theIssuedNym     == transaction numbers being added.
    // theRemovedNym    == transaction numbers being removed. (finalReceipt
    // notices about opening numbers for cron items.)
    // theTentativeNym  == transaction numbers being tentatively added. (I keep
    // a )
    //
    Nym theIssuedNym, theRemovedNym;

    std::set<int64_t> setNoticeNumbers; // Trans#s I've successfully signed for,
                                        // and have a notice of this from the
                                        // server.

    // For each transaction in the nymbox, if it's in reference to a transaction
    // request,
    // then create an "accept" item for that blank transaction, and add it to my
    // own, new,
    // "process nymbox" transaction that I'm sending out.
    //
    for (auto& it : theNymbox.GetTransactionMap()) {
        OTTransaction* pTransaction = it.second;
        OT_ASSERT(nullptr != pTransaction);
        // ------------------------------------------------------------
        // This is now possible (abbreviated notices in the box), since we try
        // to avoid
        // downloading replyNotices if we can help it. So we only error if it's
        // abbreviated
        // but NOT a replyNotice.
        //
        if (pTransaction->IsAbbreviated() &&
            (pTransaction->GetType() != OTTransaction::replyNotice)) {
            otErr << __FUNCTION__ << ": Error: Unexpected abbreviated receipt "
                                     "in Nymbox, even after supposedly loading "
                                     "all box receipts. (And it's not a "
                                     "replyNotice, either!)\n";
        }

        String strRespTo;
        pTransaction->GetReferenceString(strRespTo);

        if ((OTTransaction::message == pTransaction->GetType())) {
            Item* pAcceptItem = Item::CreateItemFromTransaction(
                *pAcceptTransaction, Item::acceptMessage);

            // The above already has OT_ASSERT so, no need to check the pointer
            // for nullptr.

            // the transaction will handle cleaning up the transaction item.
            pAcceptTransaction->AddItem(*pAcceptItem);

            pAcceptItem->SetReferenceToNum(
                pTransaction->GetTransactionNum()); // This is critical. Server
                                                    // needs this to look up the
                                                    // receipt in my nymbox.
            // Don't need to set transaction num on item since the constructor
            // already got it off the owner transaction.

            // sign the item
            pAcceptItem->SignContract(*pNym);
            pAcceptItem->SaveContract();

            otInfo << __FUNCTION__
                   << ": Received an encrypted message in your Nymbox:\n"
                   << strRespTo << "\n";

            // Todo: really shouldn't do this until we get a successful REPLY
            // from the server.
            // That's when I do a lot of other things. But this is a no-biggie
            // thing. It will almost
            // always succeed and in the odd-event that it fails, I'll end up
            // with a duplicate message
            // in my mail. So what?
            Message* pMessage = new Message;

            OT_ASSERT(nullptr != pMessage);

            // The original message that was sent to me (with an encrypted
            // envelope in the payload,
            // and with the sender's ID and recipient IDs as m_strNymID and
            // m_strNymID2) is stored
            // within strRespTo. Let's load it up into an OTMessage instance,
            // and add it to pNym's mail.
            //
            if (pMessage->LoadContractFromString(strRespTo)) {
                pNym->AddMail(*pMessage); // Now the Nym is responsible to
                                          // delete it. It's in his "mail".
                Nym* pSignerNym = pNym;
                pNym->SaveSignedNymfile(*pSignerNym);
            }
            else {
                delete pMessage; // Don't want to leak otherwise.
                pMessage = nullptr;
            }
        }

        // INSTRUMENT (From Another Nym)
        //
        if ((OTTransaction::instrumentNotice == pTransaction->GetType())) {
            Item* pAcceptItem = Item::CreateItemFromTransaction(
                *pAcceptTransaction, Item::acceptNotice);

            // The above already has OT_ASSERT so, no need to check the pointer
            // for nullptr.

            // the transaction will handle cleaning up the transaction item.
            pAcceptTransaction->AddItem(*pAcceptItem);

            pAcceptItem->SetReferenceToNum(
                pTransaction->GetTransactionNum()); // This is critical. Server
                                                    // needs this to look up the
                                                    // receipt in my nymbox.
            // Don't need to set transaction num on item since the constructor
            // already got it off the owner transaction.

            // sign the item
            pAcceptItem->SignContract(*pNym);
            pAcceptItem->SaveContract();

            otInfo << __FUNCTION__
                   << ": Received an encrypted instrument in your Nymbox:\n"
                   << strRespTo << "\n";
        }

        // SERVER NOTIFICATION
        //
        else if ((OTTransaction::notice == pTransaction->GetType())) {
            Item* pAcceptItem = Item::CreateItemFromTransaction(
                *pAcceptTransaction, Item::acceptNotice);

            // The above already has OT_ASSERT so, no need to check the pointer
            // for nullptr.

            // the transaction will handle cleaning up the transaction item.
            pAcceptTransaction->AddItem(*pAcceptItem);

            pAcceptItem->SetReferenceToNum(
                pTransaction->GetTransactionNum()); // This is critical. Server
                                                    // needs this to look up the
                                                    // receipt in my nymbox.
            // FYI, we don't need to set transaction num on item, since the
            // constructor already got it off the owner transaction.

            // sign the item
            pAcceptItem->SignContract(*pNym);
            pAcceptItem->SaveContract();

            // Todo: stash these somewhere, just like messages are in the
            // pNym->AddMail() feature.
            // NOTE: Most likely we still stash these in the paymentInbox just
            // the same as instrumentNotice (above)

        }

        // It's a NEW Transaction Number that I ALREADY signed for, and this
        // notice means it was a success.
        // The server puts these in the Nymbox just in case -- helps to prevent
        // synchronization issues.
        //
        // This means the new number was successfully already added to me.
        // Therefore I need to add it to my side also, so my balance agreements
        // will work.
        // However, ONLY if I find the number on my tentative list, where I
        // stored when I
        // first signed for the number, in order to make sure the server
        // couldn't lie to me
        // later by slipping me a successNotice for one I never really signed
        // for.
        //
        else if ((OTTransaction::successNotice ==
                  pTransaction->GetType()) // if successNotice (new; ALREADY
                                           // just added) transaction number.
                 ) {
            // The numbers on this set were (1) received in a successNotice, (2)
            // found on my Tentative list,
            // and (3) Therefore have ALREADY been added as numbers in the past.
            // Therefore I need to REMOVE
            // them from my tentative list, and add them as actual transactions.
            // I also need to update my
            // "most recent" highest trans # to reflect these new numbers.
            //
            NumList theOutput;
            pTransaction->GetNumList(theOutput); // Get the numlist from the
                                                 // successNotice transaction
            std::set<int64_t> theNumbers;        //
            theOutput.Output(theNumbers); // Get the actual set of numbers from
                                          // the numlist object.
            // Iterate through those numbers...
            //
            for (const auto& lValue : theNumbers) {

                if (!pNym->VerifyTentativeNum(strNotaryID, lValue))
                    otWarn << __FUNCTION__
                           << ": OTTransaction::successNotice: This wasn't on "
                              "my tentative list (" << lValue
                           << "), I must have already processed it. (Or there "
                              "was dropped message when I did, or the server "
                              "is trying to slip me an old number.\n)";
                else
                    setNoticeNumbers.insert(lValue); // I only take the numbers
                                                     // that I had been
                                                     // expecting, as tentative
                                                     // numbers,
            }
            Item* pAcceptItem = Item::CreateItemFromTransaction(
                *pAcceptTransaction, Item::acceptNotice);

            // the transaction will handle cleaning up the transaction item.
            pAcceptTransaction->AddItem(*pAcceptItem);

            pAcceptItem->SetReferenceToNum(
                pTransaction->GetTransactionNum()); // This is critical. Server
                                                    // needs this to look up the
                                                    // original.
            // Don't need to set transaction num on item since the constructor
            // already got it off the owner transaction.

            // sign the item
            pAcceptItem->SignContract(*pNym);
            pAcceptItem->SaveContract();

        }         // else if successNotice
        else if ( // if replyNotice -- notice of a server reply I should have
            // already received when I first sent the request.
            // (Some server replies are important enough that they have
            // a copy dropped into your Nymbox to make SURE you
            // receive and process them.) I'll accept the notice (clear
            // it from my nymbox) and also I'll process the
            // original server reply message inside of it, in case due
            // to some network issue, I've never seen it before.
            //
            (OTTransaction::replyNotice == pTransaction->GetType()))
        // UPDATE: Clearly if I ALREADY processed the server reply, then I
        // don't need to process it AGAIN, right?
        // This replyNotice is only here JUST IN CASE. (In case I missed the
        // reply originally.) Well, guess what?
        // Now I have a list of request numbers stored on the Nym, that
        // tells me definitively whether or not that
        // Nym has seen the reply. (Clearly if the Nym has processed the
        // reply already, he doesn't have to do it
        // AGAIN, now does he? This notice was "just in case.")
        //
        // Therefore I will check to see if the request number for this
        // replyNotice is in my list of "replies I've
        // already seen." If it is, I can entirely skip this step, which
        // would otherwise end up trying erroneously
        // to process a server reply even though I had already processed it
        // before.
        {

            const bool bAlreadySeenIt = pNym->VerifyAcknowledgedNum(
                strNotaryID, pTransaction->GetRequestNum()); // Client verifies
                                                             // it has already
                                                             // seen a server
                                                             // reply.

            if (bAlreadySeenIt) // if we've already seen the reply, then we're
                                // already signalling the server to remove this
                continue;       // replyNotice on its side anyway, since the
                                // notification is clearly accomplished.
            //
            else // But if we HAVEN'T already seen the server's reply, then
                 // lucky for us he dropped a copy into the Nymbox! Now we can
                 // process it!
            {
                Item* pAcceptItem = Item::CreateItemFromTransaction(
                    *pAcceptTransaction, Item::acceptNotice);
                OT_ASSERT_MSG(nullptr != pAcceptItem,
                              "OTItem * pAcceptItem = "
                              "OTItem::CreateItemFromTransaction(*"
                              "pAcceptTransaction, OTItem::acceptNotice); for "
                              "replyNotice.");

                // the transaction will handle cleaning up the transaction item.
                pAcceptTransaction->AddItem(*pAcceptItem);

                pAcceptItem->SetReferenceToNum(
                    pTransaction->GetTransactionNum()); // This is critical.
                                                        // Server needs this to
                                                        // look up the original.
                // Don't need to set transaction num on item since the
                // constructor already got it off the owner transaction.

                // Load up the server's original reply message (from the
                // server's transaction item, on the receipt from my Nymbox.)
                // The whole reason that notice was placed in the Nymbox is so
                // we would be guaranteed to receive and process it, in
                // case the original reply was lost due to network problems.
                // Some messages are too important to just "get lost."
                // Therefore, even though we most likely ALREADY processed this
                // server reply, we're still going to give it a shot
                // to process right here and now, just as we're also telling the
                // server to go ahead and clear it out of the Nymbox.
                // The server's conscience is clear: he knows for SURE that I
                // DID receive notice.

                Item* pItem = pTransaction->GetItem(Item::replyNotice);

                if ((nullptr != pItem) &&
                    Item::acknowledgement == pItem->GetStatus()) {
                    String strOriginalReply;
                    pItem->GetAttachment(strOriginalReply);

                    if (!strOriginalReply.Exists()) {
                        otErr << __FUNCTION__ << ": Error loading original "
                                                 "server reply message from "
                                                 "replyNotice. (It appears to "
                                                 "be zero length.)\n";
                    }
                    else // strOriginalReply.Exists() == true.
                    {
                        std::shared_ptr<Message> pMessage(new Message);
                        OT_ASSERT_MSG(pMessage != nullptr,
                                      "OTClient::AcceptEntireNymbox: OTMessage "
                                      "* pMessage = new OTMessage;");

                        if (false ==
                            pMessage->LoadContractFromString(
                                strOriginalReply)) {
                            otErr << __FUNCTION__
                                  << ": Failed loading original server reply "
                                     "message from replyNotice:\n\n"
                                  << strOriginalReply << "\n\n";
                        }
                        else // Success loading the server's original reply up
                               // into an OTMessage, from a string.
                        {
                            //
                            // pMessage needs to be allocated on the heap since
                            // ProcessServerReply takes ownership of it.
                            // theNymbox is passed in as a pointer because it's
                            // an optional parameter, precisely meant for this
                            // situation, where theNymbox happens to be already
                            // loaded and we don't want it loading it again,
                            // with one copy ending up overwriting the other.
                            //
                            //                          const bool bProcessed =
                            // ProcessServerReply  sometimes has to load
                            // the Nymbox. Since we  already have it loaded
                            // here, we pass it in so it won't get loaded twice.
                            processServerReply(pMessage, &theNymbox);

                            pMessage = nullptr; // We're done with it now.

                            // By this point, I KNOW FOR A FACT that IF there
                            // was some network problem that caused a Nym to
                            // lose an important server message, that by now,
                            // the Nym HAS received and processed that server
                            // reply as appropriate, using the exact same
                            // function that would have been called, had the
                            // reply
                            // been properly received in the first place. It's
                            // as if it was never lost. (Vital for syncing.)
                        } // if success loading original reply message from
                          // server.
                    }     // if strOriginalReply.Exists()
                }      // if the replyNotice item is not-nullptr and status is
                       // "success"
                else { // nullptr or "rejected"
                    otOut << __FUNCTION__
                          << ": the replyNotice item was either nullptr, or "
                             "rejected. (Unexpectedly on either count.)\n";
                }
                //
                // sign the item
                pAcceptItem->SignContract(*pNym);
                pAcceptItem->SaveContract();
            } // If we haven't "already seen it" then we loaded it up (above)
              // and processed the server reply.

            // Todo: notice that we remove the replyNotice from the Nymbox,
            // whether we are actually able to successfully
            // load the original message or not. But what if that fails? We have
            // now just discarded the message. In the
            // future, perhaps have a place where "failed messages go to die" so
            // that vital data isn't lost in the event
            // of some unanticipated future bug.

        } // else if replyNotice

        // It's a NEW Transaction Number (I need to sign for it.)
        //
        else if ((OTTransaction::blank ==
                  pTransaction->GetType()) // if blank (new; just added)
                                           // transaction number.
                 ) {
            // My new transaction agreement needs to reflect all these new
            // transaction numbers
            // that I'm signing for (or at least this one in this block) so I
            // add them to this
            // temp nym, and then harvest the ones onto it from theNym, and then
            // send those
            // numbers in the new transaction agreement. (Removing them
            // immediately after, and
            // then only adding them for real if we get a server
            // acknowledgment.)
            //
            NumList theNumlist, theBlankList;
            pTransaction->GetNumList(theNumlist);
            std::set<int64_t> theNumbers;
            theNumlist.Output(theNumbers);

            for (auto& it : theNumbers) {
                const int64_t lTransactionNumber = it;
                // Loop FOR EACH TRANSACTION NUMBER in the "blank" (there could
                // be 20 of them...)
                //
                if (pNym->VerifyIssuedNum(
                        strNotaryID, lTransactionNumber)) // Trans number is
                                                          // already issued to
                                                          // this nym (must be
                                                          // an old notice.)
                    otOut << __FUNCTION__ << ": Attempted to accept a blank "
                                             "transaction number that I "
                                             "ALREADY HAD...(Skipping.)\n";
                else if (pNym->VerifyTentativeNum(
                             strNotaryID, lTransactionNumber)) // Trans number
                                                               // is already on
                                                               // the tentative
                                                               // list (meaning
                                                               // it's already
                                                               // been
                                                               // accepted.)
                    otOut << __FUNCTION__
                          << ": Attempted to accept a blank transaction number "
                             "that I ALREADY ACCEPTED (it's on my tentative "
                             "list already; Skipping.)\n";
                else if (bGotHighestNum &&
                         (lTransactionNumber <= lHighestNum)) // Man, this is
                                                              // old numbers
                                                              // we've already
                                                              // HAD before!
                    otOut << __FUNCTION__
                          << ": Attempted to accept a blank transaction number "
                             "that I've HAD BEFORE, or at least, is <= to ones "
                             "I've had before. (Skipping...)\n";
                else {
                    theIssuedNym.AddIssuedNum(strNotaryID, lTransactionNumber);
                    theBlankList.Add(lTransactionNumber);
                }
            } // for-each
            Item* pAcceptItem = Item::CreateItemFromTransaction(
                *pAcceptTransaction, Item::acceptTransaction);

            pAcceptItem->AddBlankNumbersToItem(theBlankList);

            // the transaction will handle cleaning up the transaction item.
            pAcceptTransaction->AddItem(*pAcceptItem);

            pAcceptItem->SetReferenceToNum(
                pTransaction->GetTransactionNum()); // This is critical. Server
                                                    // needs this to look up the
                                                    // original.
            // Don't need to set transaction num on item since the constructor
            // already got it off the owner transaction.

            // sign the item
            pAcceptItem->SignContract(*pNym);
            pAcceptItem->SaveContract();
        } // else if blank

        // It's a Final Receipt (In the Nymbox, this means an opening
        // transaction
        // number has been removed from my issued list on the server side.)
        //
        else if ((OTTransaction::finalReceipt ==
                  pTransaction->GetType()) // if finalReceipt (just removed)
                                           // transaction number.
                 ) {
            // Todo security: make sure this is only possible for finalReceipts,
            // in case of abuse.
            // Not only for finalReceipts, but for specific finalReceipt #s that
            // I store a local list of, perhaps
            // in my Nym, to track until they are closed. No other number should
            // get through here.
            // Otherwise the server could trick you into removing your issued
            // numbers, simply by dropping
            // a final receipt for the appropriate number!
            // The server already keeps a list on its side to protect it from
            // this possibility, but now it
            // appears that the client-side will have to do a similar thing.
            // Sigh.

            // Since the "in reference to" (the original "opening" transaction#)
            // is supposedly
            // already closed, then let's just MAKE SURE of that, since
            // otherwise it'll screw up
            // my future balance agreements. (The instant a finalReceipt
            // appears, the "in ref to" # is already gone..)
            //
            if (pNym->RemoveIssuedNum(*pNym, strNotaryID,
                                      pTransaction->GetReferenceToNum(),
                                      true)) // bool bSave=true
                otWarn << __FUNCTION__
                       << ": **** Due to finding a finalReceipt, REMOVING "
                          "OPENING NUMBER FROM NYM:  "
                       << pTransaction->GetReferenceToNum() << " \n";
            else
                otWarn << __FUNCTION__
                       << ": **** Noticed a finalReceipt, but Opening Number "
                       << pTransaction->GetReferenceToNum()
                       << " had ALREADY been removed from nym. \n";

            //
            // pNym won't actually save unless it actually removes that #. If
            // the #'s already NOT THERE,
            // then the removal will fail, and thus it won't bother saving here.

            // The client side keeps a list of active (recurring) transactions.
            // That is, smart contracts and payment plans. I don't think it
            // keeps
            // market offers in that list, since we already have a list of
            // active
            // market offers separately. And market offers produce final
            // receipts,
            // so basically this piece of code will be executed for all final
            // receipts.
            // It's not really necessary that it be called for market offers,
            // but whatever.
            // It is for the others.
            //
            // Notice even though the final receipt hasn't yet been cleared out
            // of the box,
            // we are already removing the record of the active cron receipt.
            // Why?
            // Because regardless of when the user processes the finalReceipt,
            // we know for
            // a fact the transaction is no longer actively running on Cron. So
            // we don't want
            // to keep it on our list of "active" cron items if we know it's
            // already inactive.
            //
            OTCronItem::EraseActiveCronReceipt(
                pTransaction->GetReferenceToNum(), pNym->GetConstID(),
                pTransaction->GetPurportedNotaryID());
            Item* pAcceptItem = Item::CreateItemFromTransaction(
                *pAcceptTransaction, Item::acceptFinalReceipt);

            // the transaction will handle cleaning up the transaction item.
            pAcceptTransaction->AddItem(*pAcceptItem);

            pAcceptItem->SetReferenceToNum(
                pTransaction->GetTransactionNum()); // This is critical. Server
                                                    // needs this to look up the
                                                    // original.
            // Don't need to set transaction num on item since the constructor
            // already got it off the owner transaction.

            pAcceptItem->SignContract(*pNym);
            pAcceptItem->SaveContract();
        } // else if finalReceipt (in Nymbox, this signals that an OPENING
          // number has closed ALREADY. Thus no need to have a "closing
          // process.")
    }

    // If the above processing resulted in us actually accepting certain
    // specific items,
    // then let's process the message out to the server.
    //
    if (pAcceptTransaction->GetItemCount()) {
        // IF there were transactions that were approved for me, (and I have
        // notice of them in my nymbox)
        // then they will be in this set. Also, they'll only be here IF they
        // were verified as ACTUALLY being
        // on my tentative list.
        // Therefore need to REMOVE from Tentative list, and add to actual
        // issued/available lists.
        //
        if (!setNoticeNumbers.empty()) {
            //
            // Note: No need to update highest num here, since that should have
            // already been done when they were
            // added to my issued list in the first place. (Removed from
            // tentative.)
            //
            //            int64_t lViolator = pNym->UpdateHighestNum(*pNym,
            // strNotaryID, setNoticeNumbers); // bSave=false (saved below if
            // necessary)
            //
            //            if (lViolator != 0)
            //                otErr << "OTClient::AcceptEntireNymbox:
            // ERROR: Tried to update highest trans # for a server, with lower
            // numbers!\n"
            //                              "This should NEVER HAPPEN, since
            // these numbers are supposedly verified already before even getting
            // this far.\n"
            //                              "Violating number (too low): " <<
            // lViolator << ",
            // Nym ID: " << strNymID << " \n";
            //            else
            {
                for (auto& it : setNoticeNumbers) {
                    const int64_t lNoticeNum = it;

                    if (pNym->RemoveTentativeNum(
                            strNotaryID,
                            lNoticeNum)) // doesn't save (but saved below)
                        pNym->AddTransactionNum(
                            *pNym, strNotaryID, lNoticeNum,
                            false); // bSave = false (but saved below...)
                }

                // The notice means it already happened in the past. I already
                // accepted the transaction # in my past,
                // and now there is a notice of that fact sitting in my Nymbox.
                // Until I recognize it, all my transaction
                // statements will fail. (Like the one a few lines below
                // here...)
                //
                pNym->SaveSignedNymfile(*pNym);
            }
        }

        if (ProcessUserCommand(OTClient::processNymbox, theMessage, *pNym,
                               theServerContract, nullptr) > 0) {
            // the message is all set up and ready to go out... it's even
            // signed.
            // Except the ledger we're sending, still needs to be added, and
            // then the
            // message needs to be re-signed as a result of that.

            theNymbox.ReleaseTransactions(); // Since this function accepts them
                                             // ALL, the new balance agreement
                                             // needs to show it as empty.

            // By this point, theIssuedNym contains a list of all the
            // transaction numbers that are in my
            // nymbox, and that WILL be ADDED to me once this processNymbox is
            // processed.
            // Therefore I need to ADD those items to my issued list (at least
            // temporarily) in order to
            // calculate the transaction agreement properly. So I used
            // theIssueNym as a temp variable to store those
            // numbers, so I can add them to my Nym and them remove them again
            // after generating the statement.
            //
            for (int32_t i = 0; i < theIssuedNym.GetIssuedNumCount(theNotaryID);
                 i++) {
                int64_t lTemp = theIssuedNym.GetIssuedNum(theNotaryID, i);
                // We know it's not already issued on the Nym, or it wouldn't
                // have even gotten
                // set inside theIssuedNym in the first place (further up
                // above.) That's why
                // we are confident now that we can add it, generate the
                // transaction statement,
                // and then remove it again.
                //
                pNym->AddIssuedNum(strNotaryID, lTemp); // doesn't save.
            }

            // TRANSACTION STATEMENT
            // The item is signed and saved within this call as well. No need to
            // do that again.
            //
            Item* pBalanceItem =
                pNym->GenerateTransactionStatement(*pAcceptTransaction);

            // Here I am removing the new numbers again, now that the statement
            // has been generated.
            // If the message is successful, then I will need to add them for
            // real.
            //
            bool bAddedTentative = false;
            for (int32_t i = 0; i < theIssuedNym.GetIssuedNumCount(theNotaryID);
                 i++) {
                int64_t lTemp = theIssuedNym.GetIssuedNum(theNotaryID, i);
                pNym->RemoveIssuedNum(strNotaryID, lTemp);
                pNym->AddTentativeNum(strNotaryID,
                                      lTemp); // So when I see the success
                                              // notice later, I'll know the
                                              // server isn't lying. (Store a
                                              // copy here until then.)
                bAddedTentative = true;
            }

            if (bAddedTentative) pNym->SaveSignedNymfile(*pNym);

            if (nullptr !=
                pBalanceItem) // This can't be nullptr BTW, since there is
                              // an OT_ASSERT in Generate call. But I
                              // hate to use a pointer without checking
                              // it.
                pAcceptTransaction->AddItem(*pBalanceItem); // Better not be
            // nullptr... message
            // will fail... But
            // better check
            // anyway.
            else
                otErr << __FUNCTION__ << ": This should never happen.\n";

            // Sign the accept transaction, as well as the message ledger
            // that we've just constructed containing it.
            pAcceptTransaction->SignContract(*pNym);
            pAcceptTransaction->SaveContract();

            processLedger.SignContract(*pNym);
            processLedger.SaveContract();

            // Extract the ledger into string form and add it as the payload on
            // the message.
            String strLedger(processLedger);
            theMessage.m_ascPayload.SetString(strLedger);

            // Release any other signatures from the message, since I know it
            // was signed already in the above call to ProcessUserCommand.
            theMessage.ReleaseSignatures();

            return true;
        }
        else
            otErr << __FUNCTION__
                  << ": Error processing processNymbox command.\n";
    }

    return false;
}

// I'm doing this so I can declare a local function, INSIDE this function :-)
// (To avoid duplicating code.)  Watch and learn...
//
void OTClient::load_str_trans_add_to_ledger(const Identifier& the_nym_id,
                                            const String& str_trans,
                                            String str_box_type,
                                            const int64_t& lTransNum,
                                            Nym& the_nym, Ledger& ledger) const
{
    if (nullptr == ledger.GetTransaction(lTransNum)) // (Only add it if it's not
                                                     // already there.)
    {
        OTTransactionType* pTransType =
            OTTransactionType::TransactionFactory(str_trans);

        if (nullptr == pTransType)
            otErr << __FUNCTION__ << ": Error instantiating transaction type "
                                     "based on str_trans:\n" << str_trans
                  << "\n";
        else {
            OTTransaction* pCopy = dynamic_cast<OTTransaction*>(pTransType);

            if (nullptr ==
                pCopy) // it's a transaction type but not a transaction.
            {
                const String strNymID(the_nym_id), strAcctID(the_nym_id);
                otOut
                    << __FUNCTION__
                    << ": it's a transaction type but not a transaction: (for "
                    << str_box_type << "):\n\n" << str_trans << "\n\n";
                delete pTransType;
                pTransType = nullptr;
            }
            else // The copy transaction is now loaded from the string. Add it
                   // to the ledger...
            {
                if (!ledger.AddTransaction(*pCopy)) // if unable to add that
                                                    // transaction, once loaded,
                                                    // signed, and saved, to the
                                                    // paymentInbox or recordBox
                                                    // ledger...
                {
                    String strNymID(the_nym_id), strAcctID(the_nym_id);
                    otOut << __FUNCTION__
                          << ": Unable to add the transaction to the "
                          << str_box_type << " with user/acct IDs: " << strNymID
                          << " / " << strAcctID
                          << ", and loading from string:\n\n" << str_trans
                          << "\n\n";
                    delete pCopy;
                    pCopy = nullptr;
                }
                else // We were able to add it, so now let's save the
                       // paymentInbox (or recordBox.)
                {
                    ledger.ReleaseSignatures();
                    ledger.SignContract(the_nym);
                    ledger.SaveContract();

                    if (Ledger::paymentInbox == ledger.GetType())
                        ledger.SavePaymentInbox();
                    else if (Ledger::recordBox == ledger.GetType())
                        ledger.SaveRecordBox();
                    else if (Ledger::expiredBox == ledger.GetType())
                        ledger.SaveExpiredBox();

                    if (!pCopy->SaveBoxReceipt(ledger))
                        otErr
                            << __FUNCTION__ << ": " << str_box_type
                            << " Failed trying to SaveBoxReceipt. Contents:\n\n"
                            << str_trans << "\n\n";
                }
            }
        }
    } // if this transaction wasn't already in the paymentInbox / recordBox
      // (whichever was passed in)...
      // else it WAS already there, so do nothing. (No need to add it twice.)
} // void load_str_trans_add_to_ledger

/// We have received the server reply (ProcessServerReply) which has vetted it
/// and determined that it
/// is legitimate and safe, and that it is a reply to a transaction request.
///
/// At that point, this function is called to open the reply, go through the
/// transaction responses,
/// and potentially grab any bearer certificates that are inside and save them
/// in a purse somewhere.
/// (And do any other necessary processing on that reply.)
///
/// Also: Need to call this function after Nymbox notices of old server replies
/// (to prevent sync issues).
/// But what if already processed? Call pNym-VerifyIssuedNum(strNotaryID,
/// pTrans->GetTransactionNum()
/// and if you discover that it's already been removed, then discard it (or save
/// it in your auto-receipt
/// storage. But if you discover that the number is STILL issued to you, the
/// simply call the below
/// function, the same as you would have normally if you had received the server
/// reply in the first
/// place!  That way transaction sync issues become impossible.
/// SOLUTION: bool OTClient::processServerReply(OTMessage& theReply)
/// Any message deemed important enough to have a notice containing the reply
/// dropped into my nymbox,
/// I will just take that message and pass it to processServerReply(), which
/// will then call THIS function
/// (ProcessIncomingTransactions) where appropriate, and THIS function should
/// therefore then be smart
/// enough to ignore transactions that aren't VERIFIED as issued numbers on this
/// Nym still! (An easy
/// enough test for determining whether it's already been processed...) If it's
/// not on the issued list,
/// then skip it! I must have processed it already.
///
void OTClient::ProcessIncomingTransactions(OTServerConnection& theConnection,
                                           const Message& theReply) const
{
    const Identifier ACCOUNT_ID(theReply.m_strAcctID);
    Identifier NOTARY_ID;
    theConnection.GetNotaryID(NOTARY_ID);
    Nym* pNym = theConnection.GetNym();
    Identifier NYM_ID;
    pNym->GetIdentifier(NYM_ID);
    const String strNymID(NYM_ID);
    String strNotaryID(NOTARY_ID),
        strReceiptID("ID_NOT_SET_YET"); // This will be user ID or acct ID
                                        // depending on whether trans statement
                                        // or balance statement.

    // todo fix cast.
    Nym* pServerNym = const_cast<Nym*>(
        theConnection.GetServerContract()->GetContractPublicNym());
    // The only incoming transactions that we actually care about are responses
    // to cash
    // WITHDRAWALS.  (Cause we want to get that money off of the response, not
    // lose it.)
    // So let's just check to see if it's a withdrawal...
    Ledger theLedger(NYM_ID, ACCOUNT_ID, NOTARY_ID);
    String strLedger(theReply.m_ascPayload);

    // The ledger we received from the server was generated there, so we don't
    // have to call GenerateLedger. We just load it.
    bool bSuccess =
        theLedger.LoadLedgerFromString(strLedger); // This is a MESSAGE ledger.

    if (bSuccess) {
        bSuccess = theLedger.VerifyAccount(*pServerNym);
    }
    if (!bSuccess) {
        otErr << "ERROR loading ledger from message payload in "
                 "OTClient::ProcessIncomingTransactions.\n";
        return;
    }

    otLog3 << "Loaded ledger out of message payload.\n";

    // Loop through ledger transactions,

    for (auto& it : theLedger.GetTransactionMap()) {
        OTTransaction* pTransaction = it.second;
        OT_ASSERT_MSG(nullptr != pTransaction, "nullptr transaction pointer in "
                                               "OTServer::"
                                               "UserCmdNotarizeTransaction\n");

        // See note above function. In this loop, it's possible that we've
        // already processed these
        // transactions. Therefore we ignore the ones that are already released
        // from our issued list.
        //
        if (false ==
            pNym->VerifyIssuedNum(strNotaryID,
                                  pTransaction->GetTransactionNum())) {
            otInfo << "OTClient::ProcessIncomingTransactions: Skipping "
                      "processing of server reply to transaction number "
                   << pTransaction->GetTransactionNum()
                   << " since the number isn't even issued to me. Usually this "
                      "means that I ALREADY processed it, and we are now "
                      "processing the nymbox notice for the same "
                      "transaction.\n";
            continue; // If this trans# isn't even signed out to me anymore,
                      // then skip it. It's already closed.
        }

        // Each transaction in the ledger is a server reply to our original
        // transaction request.
        //
        if (pTransaction->VerifyAccount(*pServerNym)) // if not null && valid
                                                      // transaction reply from
                                                      // server
        {
            // We had to burn a transaction number to run the transaction that
            // the server has now replied to,
            // so let's remove that number from our list of responsibility.
            // Whether it was successful or not,
            // the server has removed it from our list of responsibility, so we
            // need to remove it on our side as well.
            // so that we can properly calculate our balance agreements in the
            // future.
            //
            // NOTE: not for all types! See the switch statements:

            Item::itemType theItemType = Item::error_state;

            switch (pTransaction->GetType()) {
            case OTTransaction::atDeposit:
                theItemType = Item::atDeposit;
                break;
            case OTTransaction::atWithdrawal: {
                Item* pItemCash = pTransaction->GetItem(Item::atWithdrawal);
                Item* pItemVoucher =
                    pTransaction->GetItem(Item::atWithdrawVoucher);

                if (nullptr != pItemCash)
                    theItemType = Item::atWithdrawal;
                else if (nullptr != pItemVoucher)
                    theItemType = Item::atWithdrawVoucher;
            } break;
            case OTTransaction::atPayDividend:
                theItemType = Item::atPayDividend;
                break;
            case OTTransaction::atTransfer:
                theItemType = Item::atTransfer;
                break;
            case OTTransaction::atMarketOffer:
                theItemType = Item::atMarketOffer;
                break;
            case OTTransaction::atPaymentPlan:
                theItemType = Item::atPaymentPlan;
                break;
            case OTTransaction::atSmartContract:
                theItemType = Item::atSmartContract;
                break;
            case OTTransaction::atCancelCronItem:
                theItemType = Item::atCancelCronItem;
                break;
            case OTTransaction::atExchangeBasket:
                theItemType = Item::atExchangeBasket;
                break;
            default:
            case OTTransaction::atProcessInbox: // not handled here...
                continue;
            }

            switch (pTransaction->GetType()) {
            case OTTransaction::atDeposit:
                ProcessDepositResponse(*pTransaction, theConnection, theReply);
                pNym->RemoveIssuedNum(*pNym, strNotaryID,
                                      pTransaction->GetTransactionNum(),
                                      true); // bool bSave=true
                break;

            case OTTransaction::atPayDividend:
                ProcessPayDividendResponse(*pTransaction, theConnection,
                                           theReply);
                pNym->RemoveIssuedNum(*pNym, strNotaryID,
                                      pTransaction->GetTransactionNum(),
                                      true); // bool bSave=true
                break;

            case OTTransaction::atExchangeBasket:
                pNym->RemoveIssuedNum(*pNym, strNotaryID,
                                      pTransaction->GetTransactionNum(),
                                      true); // bool bSave=true
                // If the exchangeBasket FAILS, then I put all the transaction
                // numbers BACK on the Nym,
                // that had been taken for the exchange (for all the
                // basketReceipts.)
                {
                    Item* pItem = pTransaction->GetItem(theItemType);

                    if ((nullptr != pItem) &&
                        Item::rejection == pItem->GetStatus()) // REJECTION
                    {
                        String strOriginalItem;
                        pItem->GetReferenceString(strOriginalItem);

                        OTTransactionType* pTempTransType =
                            strOriginalItem.Exists()
                                ? OTTransactionType::TransactionFactory(
                                      strOriginalItem)
                                : nullptr;

                        std::unique_ptr<Item> pOriginalItem(
                            (nullptr == pTempTransType)
                                ? nullptr
                                : dynamic_cast<Item*>(pTempTransType));

                        if (nullptr != pOriginalItem) {
                            String strBasket;
                            Basket theRequestBasket;

                            pOriginalItem->GetAttachment(strBasket);

                            if (strBasket.Exists() &&
                                theRequestBasket.LoadContractFromString(
                                    strBasket))
                                theRequestBasket.HarvestClosingNumbers(
                                    *pNym, NOTARY_ID, true); // bSave=true
                            else
                                otErr << "(atExchangeBasket) Error loading "
                                         "original basket request in "
                                         "OTClient::"
                                         "ProcessIncomingTransactions\n";
                        } // if success loading original item.
                        else {
                            otErr << "(atExchangeBasket) Error loading "
                                     "original item from string in "
                                     "OTClient::ProcessIncomingTransactions\n";
                        }
                    } // if exchangeBasket was a failure
                }
                break;

            case OTTransaction::atCancelCronItem:
                pNym->RemoveIssuedNum(*pNym, strNotaryID,
                                      pTransaction->GetTransactionNum(),
                                      true); // bool bSave=true
                // Just above, we remove the issued number that was used to
                // initiate the cancelCronItem. (Regardless of success.)
                // Below, we remove the issued number that was ON that Cron Item
                // (IF SUCCESS.)
                {
                    Item* pItem = pTransaction->GetItem(theItemType);

                    if ((nullptr != pItem) &&
                        Item::acknowledgement ==
                            pItem->GetStatus()) { // If it was a success
                                                  // cancelling the cron item,
                                                  // then the final receipt has
                                                  // been created, and
                        // the transaction number is closed out, and only the
                        // closing number is left. If that is the case
                        // then I can remove the transaction number from my
                        // issued list, presumably the server already has.
                        //
                        String strOriginalItem;
                        pItem->GetReferenceString(strOriginalItem);

                        OTTransactionType* pTempTransType =
                            strOriginalItem.Exists()
                                ? OTTransactionType::TransactionFactory(
                                      strOriginalItem)
                                : nullptr;

                        std::unique_ptr<Item> pOriginalItem(
                            (nullptr == pTempTransType)
                                ? nullptr
                                : dynamic_cast<Item*>(pTempTransType));

                        if (nullptr != pOriginalItem) {
                            if (false ==
                                pNym->RemoveIssuedNum(
                                    *pNym, strNotaryID,
                                    pOriginalItem->GetReferenceToNum(),
                                    true)) // bool bSave=true
                            {
                                otErr << "(atCancelCronItem) Error removing "
                                         "issued number from user nym in "
                                         "OTClient::"
                                         "ProcessIncomingTransactions\n";
                            }
                            // I don't have to call RemoveTransactionNum for the
                            // closing number (though the server does.) Why not?
                            // Because I already called GetNextTransactionNum()
                            // to use it in the first place, so it's already off
                            // my
                            // list of usable transaction numbers here on the
                            // client side.
                        }
                        else {
                            otErr << __FUNCTION__ << ": (atCancelCronItem) "
                                                     "Error loading original "
                                                     "item from string.\n";
                        }
                    }
                }
                break;

            case OTTransaction::atWithdrawal:
                ProcessWithdrawalResponse(*pTransaction, theConnection,
                                          theReply);
                pNym->RemoveIssuedNum(*pNym, strNotaryID,
                                      pTransaction->GetTransactionNum(),
                                      true); // bool bSave=true
                break;

            case OTTransaction::atTransfer:
                // Nothing removed here since the transaction number is still in
                // play, in this cases.
                // ACTUALLY, if this is a failure, we need to REMOVE from issued
                // list. (It's burned.)
                // But if success, the number stays in play until a later time.
                // (So we leave it issued.)
                {
                    Item* pItem = pTransaction->GetItem(theItemType);

                    if ((nullptr != pItem) &&
                        Item::rejection == pItem->GetStatus()) {
                        // Why do this? Oh I see, this number either gets burned
                        // from the attempt,
                        // or it stays open for a while if success. So here what
                        // do we see? The rejection
                        // burning the transaction number, but leaving it open
                        // if success. Perfect.
                        //
                        if (false ==
                            pNym->RemoveIssuedNum(
                                *pNym, strNotaryID,
                                pTransaction->GetTransactionNum(),
                                true)) // bool bSave=true
                        {
                            otErr << __FUNCTION__ << ": Error removing issued "
                                                     "number from user nym "
                                                     "(for a transfer.)\n";
                        }
                    }
                }
                break;

            case OTTransaction::atMarketOffer:
            case OTTransaction::atPaymentPlan:
            case OTTransaction::atSmartContract:

                // Nothing removed here since the transaction number is still in
                // play, in these cases.
                // ACTUALLY, if these are a failure, we need to REMOVE from
                // issued list.
                // But if success, the number stays in play until a later time.
                {
                    const int64_t lNymOpeningNumber =
                        pTransaction->GetTransactionNum();
                    Item* pReplyItem = pTransaction->GetItem(theItemType);
                    if (nullptr != pReplyItem) {
                        String strOriginalItem;
                        pReplyItem->GetReferenceString(strOriginalItem);

                        OTTransactionType* pTempTransType =
                            strOriginalItem.Exists()
                                ? OTTransactionType::TransactionFactory(
                                      strOriginalItem)
                                : nullptr;

                        std::unique_ptr<Item> pOriginalItem(
                            (nullptr == pTempTransType)
                                ? nullptr
                                : dynamic_cast<Item*>(pTempTransType));

                        if (nullptr != pOriginalItem) {
                            String strCronItem;
                            pOriginalItem->GetAttachment(strCronItem);

                            // What kind of cron item is it?
                            // Well (todo) we should probably double-check, but
                            // the only cron items we
                            // send notices for are payment plans and smart
                            // contracts. Market offers don't
                            // need notices, since anyone activating a market
                            // offer is already getting the
                            // reply. (AND getting a copy of that reply,
                            // already, inside a replyNotice in
                            // his Nymbox...) So he can't possibly miss the
                            // server's reply, and there aren't
                            // any other parties to notify (re: successful
                            // activation), besides the Nym himself.
                            //
                            // Only payment plans and smart contracts could
                            // potentially have some other signer, who
                            // would want to get notified, and to whom the
                            // notice is send.
                            //
                            std::unique_ptr<OTCronItem> pCronItem(
                                strCronItem.Exists()
                                    ? OTCronItem::NewCronItem(strCronItem)
                                    : nullptr);

                            if (nullptr != pCronItem) // the original smart
                            // contract or payment plan
                            // object.
                            {
                                if (Item::rejection ==
                                    pReplyItem->GetStatus()) // REJECTION (This
                                                             // is where we
                                                             // remove the
                                                             // opening number,
                                                             // and harvest the
                                                             // closing
                                                             // numbers.)
                                {
                                    // Why do this? Oh I see, this number either
                                    // gets burned from the attempt,
                                    // or it stays open for a while if success.
                                    // So here what do we see? The rejection
                                    // burning the transaction number, but
                                    // leaving it open if success. Perfect.
                                    //
                                    if (false ==
                                        pNym->RemoveIssuedNum(
                                            *pNym, strNotaryID,
                                            lNymOpeningNumber,
                                            true)) // bool bSave=true
                                    {
                                        otErr << __FUNCTION__
                                              << ": Error removing issued "
                                                 "number from user nym (for a "
                                                 "cron item.)\n";
                                    }
                                    // If the activation was a failure, we can
                                    // add all the extra transaction numbers
                                    // BACK to the
                                    // Nym, that were being used as CLOSING
                                    // numbers, and use them later. (They aren't
                                    // burned.)
                                    // They're still all signed-out, so we
                                    // should harvest them so we can still use
                                    // them on something.
                                    // (Whereas if it had been a success, then
                                    // we would have left them in their existing
                                    // state, since
                                    // the transaction would then be in play,
                                    // and the numbers could not be used again,
                                    // nor removed as
                                    // issued numbers until the transaction
                                    // itself had finished and its receipts had
                                    // been signed-off.)
                                    //
                                    pCronItem->HarvestClosingNumbers(
                                        *pNym); // saves.
                                }
                                // When party receives notice that smart
                                // contract has been activated,
                                // remove the instrument from outpayments box.
                                // (If it's there -- it can be.)
                                //
                                // (This happens for acknowledged AND rejected
                                // smart contracts.)
                                //

                                String strInstrument; // If the instrument is
                                                      // in the outpayments
                                                      // box, we put a copy of
                                                      // it here.

                                if ((OTTransaction::atPaymentPlan ==
                                     pTransaction->GetType()) || // No need to
                                                                 // do this for
                                                                 // market
                                                                 // offers.
                                                                 // (Because
                                                                 // they don't
                                    (OTTransaction::atSmartContract ==
                                     pTransaction->GetType())) // go into the
                                                               // outpayments
                                                               // box in the
                                                               // first place.)
                                {
                                    // If success, save a copy in my "active
                                    // cron items" folder.
                                    //
                                    if (Item::acknowledgement ==
                                        pReplyItem->GetStatus()) {
                                        pCronItem->SaveActiveCronReceipt(
                                            pNym->GetConstID());
                                    }
                                    NumList numlistOutpayment(
                                        lNymOpeningNumber);
                                    const int32_t nOutpaymentIndex =
                                        GetOutpaymentsIndexByTransNum(
                                            *pNym, lNymOpeningNumber);
                                    std::unique_ptr<Message> theMessageAngel;

                                    if (nOutpaymentIndex >= 0) {
                                        Message* pMsg =
                                            pNym->GetOutpaymentsByIndex(
                                                nOutpaymentIndex);

                                        if (nullptr == pMsg) {
                                            otErr << __FUNCTION__
                                                  << ": Unable to find payment "
                                                     "message in outpayment "
                                                     "box based on index "
                                                  << nOutpaymentIndex << ".\n";
                                        }
                                        else {
                                            const bool bRemovedOutpayment =
                                                pNym->RemoveOutpaymentsByIndex(
                                                    nOutpaymentIndex,
                                                    false); // bDeleteIt=false
                                                            // (deleted later
                                                            // on.)
                                            theMessageAngel.reset(pMsg);
                                            if (bRemovedOutpayment)
                                                pNym->SaveSignedNymfile(*pNym);
                                            else
                                                otErr << __FUNCTION__
                                                      << ": Failed trying to "
                                                         "remove outpayment at "
                                                         "index: "
                                                      << nOutpaymentIndex
                                                      << "\n";
                                            if (!pMsg->m_ascPayload.GetString(
                                                    strInstrument)) {
                                                otErr << __FUNCTION__
                                                      << ": Unable to find "
                                                         "payment instrument "
                                                         "in outpayment "
                                                         "message at index "
                                                      << nOutpaymentIndex
                                                      << ".\n";
                                            }
                                            else {
                                                // At this point, we've removed
                                                // the outpayment already, and
                                                // it will be deleted
                                                // when it goes out of scope
                                                // already. And we've got a copy
                                                // of the original financial
                                                // instrument that was SENT in
                                                // that outpayment.
                                                //
                                                // But what for? Why did I want
                                                // that instrument here in a
                                                // string, in strInstrument?
                                                // Do I still need to do
                                                // something with it? Yes: I
                                                // need to drop a copy of it
                                                // into
                                                // the record box!

                                                // NOTE: strInstrument is added
                                                // to the RecordBox below. So
                                                // there's no need to
                                                // do that here, ATM.
                                            }
                                        }
                                    } // if (nOutpaymentIndex >= 0)
                                    // When party receives notice that smart
                                    // contract has failed activation attempt,
                                    // then remove
                                    // the instrument from payments inbox AND
                                    // outpayments box. (If there -- could be
                                    // for either.)
                                    // (Outbox is done just above, so now let's
                                    // do inbox...)
                                    //

                                    // Why only rejected items? Why not remove
                                    // it from the payments inbox on success as
                                    // well?
                                    // Normally wouldn't we expect that a
                                    // successful activation of an inbox item,
                                    // should remove
                                    // that inbox item? Especially if there's
                                    // already a copy in the outbox as well...
                                    //
                                    //                                  if
                                    // (OTItem::rejection ==
                                    // pReplyItem->GetStatus()) // REJECTION
                                    {
                                        const bool bExists1 = OTDB::Exists(
                                            OTFolders::PaymentInbox().Get(),
                                            strNotaryID.Get(), strNymID.Get());
                                        const bool bExists2 = OTDB::Exists(
                                            OTFolders::RecordBox().Get(),
                                            strNotaryID.Get(), strNymID.Get());
                                        Ledger thePmntInbox(
                                            NYM_ID, NYM_ID,
                                            NOTARY_ID); // payment inbox
                                        Ledger theRecordBox(
                                            NYM_ID, NYM_ID,
                                            NOTARY_ID); // record box
                                        bool bSuccessLoading1 =
                                            (bExists1 &&
                                             thePmntInbox.LoadPaymentInbox());
                                        bool bSuccessLoading2 =
                                            (bExists2 &&
                                             theRecordBox.LoadRecordBox());
                                        if (bExists1 && bSuccessLoading1)
                                            bSuccessLoading1 =
                                                (thePmntInbox
                                                     .VerifyContractID() &&
                                                 thePmntInbox.VerifySignature(
                                                     *pNym));
                                        // =
                                        // (thePmntInbox.VerifyAccount(*pNym));
                                        // // (No need to load all the Box
                                        // Receipts using VerifyAccount)
                                        else if (!bExists1)
                                            bSuccessLoading1 =
                                                thePmntInbox.GenerateLedger(
                                                    NYM_ID, NOTARY_ID,
                                                    Ledger::paymentInbox,
                                                    true); // bGenerateFile=true
                                        if (bExists2 && bSuccessLoading2)
                                            bSuccessLoading2 =
                                                (theRecordBox
                                                     .VerifyContractID() &&
                                                 theRecordBox.VerifySignature(
                                                     *pNym));
                                        // =
                                        // (theRecordBox.VerifyAccount(*pNym));
                                        // // (No need to load all the Box
                                        // Receipts using VerifyAccount)
                                        else if (!bExists2)
                                            bSuccessLoading2 =
                                                theRecordBox.GenerateLedger(
                                                    NYM_ID, NOTARY_ID,
                                                    Ledger::recordBox,
                                                    true); // bGenerateFile=true
                                        // by this point, the boxes DEFINITELY
                                        // exist -- or not. (generation might
                                        // have failed, or verification.)
                                        //
                                        if (!bSuccessLoading1 ||
                                            !bSuccessLoading2) {
                                            otOut
                                                << __FUNCTION__
                                                << ": while processing server "
                                                   "reply containing rejection "
                                                   "of cron item: WARNING: "
                                                   "Unable to load, verify, or "
                                                   "generate paymentInbox or "
                                                   "recordBox, with IDs: "
                                                << strNymID << " / " << strNymID
                                                << "\n";
                                        }
                                        else // --- ELSE --- Success loading
                                               // the payment inbox and
                                               // recordBox and verifying
                                        { // their contractID and signature, (OR
                                            // success generating the ledger.)
                                            // See if there's a receipt in the
                                            // payments inbox.
                                            // If so, remove it.
                                            //
                                            // What's going on here?
                                            //
                                            // Well let's say Alice sends Bob a
                                            // payment plan. (This applies to
                                            // smart contracts, too.)
                                            // This means Bob has a payment plan
                                            // in his PAYMENTS INBOX, with the
                                            // recipient's (Alice)
                                            // transaction number set to X, and
                                            // the sender's transaction number
                                            // set to 0. It's 0 because
                                            // the instrument is still in Bob's
                                            // inbox -- he hasn't signed it yet
                                            // -- so his transaction
                                            // number isn't on it yet. It's
                                            // blank (0).
                                            //
                                            // Next, let's say Bob
                                            // signs/confirms the contract,
                                            // which puts a copy of it into his
                                            // PAYMENTS
                                            // OUTBOX. On the outbox version,
                                            // Alice's transaction number is X,
                                            // and Bob's transaction number
                                            // is Y.
                                            //
                                            // Later on, Bob needs to lookup the
                                            // payment plan in his PAYMENTS
                                            // INBOX (for example, to remove
                                            // it, AS YOU SEE IN THE BELOW
                                            // LOOP.) Remember, Bob's
                                            // transaction number is Y. But he
                                            // can't use
                                            // that number (Y) to lookup the
                                            // payment plan in his inbox, since
                                            // it's set to ZERO in his inbox!
                                            // The inbox version simply doesn't
                                            // HAVE Y set onto it yet -- only
                                            // the outbox version does.
                                            //
                                            // So how in the fuck does Bob
                                            // lookup the inbox version, if the
                                            // transaction number isn't SET on
                                            // it yet??
                                            //
                                            // The solution:
                                            // 1. Bob grabs an OTNumList
                                            // containing all the transaction
                                            // numbers from the OUTBOX VERSION,
                                            //    which ends up containing "X,Y"
                                            // (that happens in this block.)
                                            // 2. Bob loops through the payments
                                            // INBOX, and for each, he grabs an
                                            // OTNumList containing all
                                            //    the transaction numbers. One
                                            // of those (the matching one) will
                                            // contain "X,0". (Except it
                                            //    will actually only contain
                                            // "X", since 0 is ignored in the
                                            // call to
                                            // GetAllTransactionNumbers.)
                                            // 3. Bob then checks like this:
                                            // if
                                            // (numlistOutpayment.VerifyAny(numlistIncomingPayment))
                                            //    This is equivalent to saying:
                                            // if ("X,Y".VerifyAny("X")) which
                                            // RETURNS TRUE -- and we have
                                            //    found the instrument!

                                            OTPayment theOutpayment;

                                            if (strInstrument.Exists() &&
                                                theOutpayment.SetPayment(
                                                    strInstrument) &&
                                                theOutpayment.SetTempValues()) {
                                                theOutpayment
                                                    .GetAllTransactionNumbers(
                                                        numlistOutpayment);
                                            }
                                            const int32_t nTransCount =
                                                thePmntInbox
                                                    .GetTransactionCount();

                                            for (int32_t ii = (nTransCount - 1);
                                                 ii >= 0;
                                                 --ii) // Count backwards since
                                                       // we are removing
                                                       // things.
                                            {
                                                std::unique_ptr<OTPayment>
                                                    pPayment(GetInstrument(
                                                        *pNym, ii,
                                                        thePmntInbox));

                                                if (nullptr == pPayment) {
                                                    otOut
                                                        << __FUNCTION__
                                                        << ": While looping "
                                                           "payments inbox to "
                                                           "remove a payment, "
                                                           "unable to retrieve "
                                                           "payment at index "
                                                        << ii
                                                        << " (skipping.)\n";
                                                    continue;
                                                }
                                                else if (false ==
                                                           pPayment
                                                               ->SetTempValues()) {
                                                    otOut
                                                        << __FUNCTION__
                                                        << ": While looping "
                                                           "payments inbox to "
                                                           "remove a payment, "
                                                           "unable to set temp "
                                                           "values for payment "
                                                           "at index " << ii
                                                        << " (skipping.)\n";
                                                    continue;
                                                }

                                                NumList numlistIncomingPayment;

                                                pPayment
                                                    ->GetAllTransactionNumbers(
                                                        numlistIncomingPayment);

                                                if (numlistOutpayment.VerifyAny(
                                                        numlistIncomingPayment)) {
                                                    // ** It's the same
                                                    // instrument.**
                                                    // Remove it from the
                                                    // payments inbox, and save.
                                                    //
                                                    OTTransaction*
                                                        pTransPaymentInbox =
                                                            thePmntInbox
                                                                .GetTransactionByIndex(
                                                                    ii);
                                                    OT_ASSERT(
                                                        nullptr !=
                                                        pTransPaymentInbox); // It DEFINITELY should be there. (Assert otherwise.)
                                                    int64_t lPaymentTransNum =
                                                        pTransPaymentInbox
                                                            ->GetTransactionNum();

                                                    // DON'T I NEED to call
                                                    // DeleteBoxReceipt at this
                                                    // point?
                                                    // Since that needs to be
                                                    // called now whenever
                                                    // removing something from
                                                    // any box?
                                                    //
                                                    // NOTE: might need to just
                                                    // MOVE this box receipt to
                                                    // the record box, instead
                                                    // of
                                                    // deleting it.
                                                    //
                                                    // Probably I need to do
                                                    // that ONLY if the version
                                                    // in the payments outbox
                                                    // doesn't exist.
                                                    // For example, if
                                                    // strInstrument doesn't
                                                    // exist, then there was
                                                    // nothing in the payments
                                                    // outbox, and therefore the
                                                    // version in the payment
                                                    // INBOX is the ONLY version
                                                    // I have,
                                                    // and therefore I should
                                                    // stick it in the Record
                                                    // Box.
                                                    //
                                                    // HOWEVER, if strInstrument
                                                    // DOES exist, then I should
                                                    // create its own
                                                    // transaction to add
                                                    // to the record box, and
                                                    // delete the one that was
                                                    // in the payment inbox. Why
                                                    // delete it? Because
                                                    // otherwise I would be
                                                    // adding the same thing
                                                    // TWICE to the record box,
                                                    // which I don't really
                                                    // need to do. And if I'm
                                                    // going to choose one of
                                                    // the two, the one in the
                                                    // outpayments box will
                                                    // be the more recent / more
                                                    // relevant one of the two.
                                                    // So I favor that one,
                                                    // unless it doesn't
                                                    // exist, in which case I
                                                    // should add the other one
                                                    // instead. (Todo.)
                                                    //
                                                    // NOTE: Until the above is
                                                    // completed, the current
                                                    // behavior is that the
                                                    // outpayments box item
                                                    // will be moved to the
                                                    // record box if it exists,
                                                    // and otherwise nothing
                                                    // will be, since any
                                                    // payments
                                                    // inbox item will be
                                                    // deleted.

                                                    if (false ==
                                                        thePmntInbox
                                                            .DeleteBoxReceipt(
                                                                lPaymentTransNum)) {
                                                        otErr
                                                            << __FUNCTION__
                                                            << ": Failed "
                                                               "trying to "
                                                               "delete the box "
                                                               "receipt for a "
                                                               "transaction "
                                                               "being removed "
                                                               "from the "
                                                               "payment "
                                                               "inbox.\n";
                                                    }
                                                    if (thePmntInbox
                                                            .RemoveTransaction(
                                                                lPaymentTransNum)) {
                                                        thePmntInbox
                                                            .ReleaseSignatures();
                                                        thePmntInbox
                                                            .SignContract(
                                                                *pNym);
                                                        thePmntInbox
                                                            .SaveContract();

                                                        if (!thePmntInbox
                                                                 .SavePaymentInbox()) {
                                                            otErr
                                                                << __FUNCTION__
                                                                << ": Failure "
                                                                   "while "
                                                                   "trying to "
                                                                   "save "
                                                                   "payment "
                                                                   "inbox.\n";
                                                        }
                                                        else {
                                                            otOut
                                                                << __FUNCTION__
                                                                << ": Removed "
                                                                   "instrument "
                                                                   "from "
                                                                   "payment "
                                                                   "inbox."
                                                                   "\nSaved "
                                                                   "payment "
                                                                   "inbox.\n";
                                                        }
                                                    }
                                                    else {
                                                        otErr
                                                            << __FUNCTION__
                                                            << ": Failed "
                                                               "trying to "
                                                               "remove "
                                                               "transaction "
                                                               "from payment "
                                                               "inbox. (Should "
                                                               "never "
                                                               "happen.)\n";
                                                    }
                                                    // Note: I could break right
                                                    // here, if this is the only
                                                    // transaction in the
                                                    // payment inbox which
                                                    // contains the instrument
                                                    // in question. Which I
                                                    // believe
                                                    // it is.  Todo: if that's
                                                    // true, which I think it
                                                    // is, then call break here.
                                                    // After all, you wouldn't
                                                    // send me the SAME
                                                    // instrument TWICE, would
                                                    // you?
                                                    // But it still seems
                                                    // theoretically possible
                                                    // (albeit stupid.)
                                                }
                                            }
                                            // for (int32_t ii = 0; ii <
                                            // nTransCount; ++ii)
                                            // Also, if there was a message in
                                            // the outpayments box (which we
                                            // already removed
                                            // a bit above), go ahead and add a
                                            // receipt for it into the record
                                            // box.
                                            //
                                            if (strInstrument.Exists()) // Found
                                                                        // the
                                            // instrument in
                                            // the
                                            // outpayments
                                            // box.
                                            {
                                                OTTransaction* pNewTransaction =
                                                    OTTransaction::
                                                        GenerateTransaction(
                                                            theRecordBox, // recordbox.
                                                            OTTransaction::
                                                                notice,
                                                            lNymOpeningNumber);
                                                std::unique_ptr<OTTransaction>
                                                    theTransactionAngel(
                                                        pNewTransaction);

                                                if (nullptr !=
                                                    pNewTransaction) // The
                                                                     // above
                                                                     // has an
                                                // OT_ASSERT
                                                // within,
                                                // but I
                                                // oust
                                                // like to
                                                // check my
                                                // pointers.
                                                {
                                                    pNewTransaction
                                                        ->SetReferenceToNum(
                                                            lNymOpeningNumber); // Referencing myself here. We'll see how it works out.
                                                    pNewTransaction
                                                        ->SetReferenceString(
                                                            strInstrument); // The
                                                    // cheque,
                                                    // invoice,
                                                    // etc
                                                    // that
                                                    // used
                                                    // to be
                                                    // in
                                                    // the
                                                    // outpayments
                                                    // box.
                                                    pNewTransaction
                                                        ->SignContract(*pNym);
                                                    pNewTransaction
                                                        ->SaveContract();
                                                    const bool bAdded =
                                                        theRecordBox
                                                            .AddTransaction(
                                                                *pNewTransaction);

                                                    if (!bAdded) {
                                                        otErr
                                                            << __FUNCTION__
                                                            << ": Unable to "
                                                               "add "
                                                               "transaction "
                                                            << pNewTransaction
                                                                   ->GetTransactionNum()
                                                            << " to record box "
                                                               "(after "
                                                               "tentatively "
                                                               "removing from "
                                                               "payment "
                                                               "outbox, an "
                                                               "action that is "
                                                               "now "
                                                               "canceled.)\n";
                                                    }
                                                    else {
                                                        theTransactionAngel
                                                            .release();

                                                        theRecordBox
                                                            .ReleaseSignatures();
                                                        theRecordBox
                                                            .SignContract(
                                                                *pNym);
                                                        theRecordBox
                                                            .SaveContract();
                                                        theRecordBox
                                                            .SaveRecordBox(); // todo log failure.

                                                        // Any
                                                        // inbox/nymbox/outbox
                                                        // ledger will only
                                                        // itself contain
                                                        // abbreviated versions
                                                        // of the receipts,
                                                        // including their
                                                        // hashes.
                                                        //
                                                        // The rest is stored
                                                        // separately, in the
                                                        // box receipt, which is
                                                        // created
                                                        // whenever a receipt is
                                                        // added to a box, and
                                                        // deleted after a
                                                        // receipt
                                                        // is removed from a
                                                        // box.
                                                        //
                                                        if (!pNewTransaction
                                                                 ->SaveBoxReceipt(
                                                                     theRecordBox)) // <===================
                                                        {
                                                            String strNewTransaction(
                                                                *pNewTransaction);
                                                            otErr
                                                                << __FUNCTION__
                                                                << ": for "
                                                                   "Record "
                                                                   "Box... "
                                                                   "Failed "
                                                                   "trying to "
                                                                   "SaveBoxRece"
                                                                   "ipt. "
                                                                   "Contents:"
                                                                   "\n\n"
                                                                << strNewTransaction
                                                                << "\n\n";
                                                        }
                                                    }
                                                }    // if (nullptr !=
                                                     // pNewTransaction)
                                                else // should never happen
                                                {
                                                    otErr
                                                        << __FUNCTION__
                                                        << ": Failed while "
                                                           "trying to generate "
                                                           "transaction in "
                                                           "order to add a new "
                                                           "transaction to "
                                                           "record box (for a "
                                                           "payment instrument "
                                                           "we just removed "
                                                           "from the "
                                                           "outpayments box): "
                                                        << strNymID << "\n";
                                                }
                                            } // if (strInstrument.Exists())
                                              // (then add a copy to record
                                              // box.)
                                        } // else (Success loading the payment
                                          // inbox and recordBox)
                                    }     // (OTItem::rejection ==
                                          // pReplyItem->GetStatus()) (loading
                                          // payment inbox and record box.)
                                }         // if payment plan or smart contract.
                            }             // if (nullptr != pCronItem)
                            else {
                                otErr << __FUNCTION__
                                      << ": Error loading cronitem from "
                                         "original item, from string:\n"
                                      << strOriginalItem << "\n";
                            }
                        } // if (nullptr != pOriginalItem)
                        else {
                            otErr << __FUNCTION__ << ": Error loading original "
                                                     "item from string:\n"
                                  << strOriginalItem << "\n\n";
                        }
                    } // if (nullptr != pReplyItem)
                }     // Case market offer, payment plan, or smart contract.
                break;

            default:
                // Error
                otErr << __FUNCTION__ << ": wrong transaction type: "
                      << pTransaction->GetTypeString() << "\n";
                break;
            } // switch
            // -----------------------------------------------------------------
            // atTransfer:        If success, KEEP the number on my list of
            // responsibility. If fail, REMOVE it.
            //                    (Do the same for atMarketOffer, atPaymentPlan,
            // and atSmartContract.)
            // atDeposit:        Whether success or fail, remove the number from
            // my list of responsibility.
            // atWithdrawal:    Whether success or fail, remove the number from
            // my list of responsibility.
            // atAcceptPending:    Whether success or fail, remove the number
            // from my list of responsibility.
            //
            // SAVE THE RECEIPT....
            //
            // OTFolders::Receipt().Get()
            const String strNotaryID(NOTARY_ID);
            String strReceiptFilename; // contains: strReceiptID .success,
                                       // fail, or error.
            Item* pItem = pTransaction->GetItem(Item::atBalanceStatement);

            if (nullptr == pItem) {
                pItem = pTransaction->GetItem(Item::atTransactionStatement);

                if (nullptr != pItem)
                    pNym->GetIdentifier(strReceiptID); // In this case, the
                                                       // receipt ID is the Nym
                                                       // ID
            }
            else {
                strReceiptID = theReply.m_strAcctID; // If a balance statement,
                                                     // then the receipt ID is
                                                     // the Account ID.
            }
            // Try to save the transaction receipt to local storage.
            //
            String strTransaction;
            pTransaction->SaveContractRaw(strTransaction);
            String strFinal;
            OTASCIIArmor ascTemp(strTransaction);

            if (false ==
                ascTemp.WriteArmoredString(strFinal,
                                           "TRANSACTION")) // todo hardcoding.
            {
                otErr << __FUNCTION__ << ": Error saving transaction receipt "
                                         "(failed writing armored string):\n"
                      << OTFolders::Receipt() << Log::PathSeparator()
                      << strNotaryID << Log::PathSeparator()
                      << strReceiptFilename << "\n";
                return;
            }
            if (nullptr != pItem) {
                // Filename is based on transaction success/failure.
                //
                if (pTransaction->GetSuccess())
                    strReceiptFilename.Format("%s.success", strReceiptID.Get());
                else
                    strReceiptFilename.Format("%s.fail", strReceiptID.Get());

                OTDB::StorePlainString(
                    strFinal.Get(), OTFolders::Receipt().Get(),
                    strNotaryID.Get(), strReceiptFilename.Get());
            }
            else // This should never happen...
            {
                strReceiptFilename.Format("%s.error", strReceiptID.Get());

                otErr << __FUNCTION__
                      << ": Error saving transaction receipt, since pItem was "
                         "nullptr: " << strReceiptFilename << "\n";

                OTDB::StorePlainString(
                    strFinal.Get(), OTFolders::Receipt().Get(),
                    strNotaryID.Get(), strReceiptFilename.Get());
            }

            // No matter what kind of transaction it is,
            // let's see if the server gave us some new transaction numbers with
            // it...
            // UPDATE: the server will not give me transaction numbers unless I
            // have SIGNED FOR THEM.
            // Therefore, they are now dropped into the Nymbox, and that is
            // where they will be.
            //            HarvestTransactionNumbers(*pTransaction, *pNym);
        }
        else {
            otOut
                << __FUNCTION__
                << ": Failed verifying server ownership of this transaction.\n";
        }
    }
}

void OTClient::ProcessPayDividendResponse(
    OTTransaction& theTransaction, const OTServerConnection& theConnection,
    const Message& theReply) const
{
    const Identifier ACCOUNT_ID(theReply.m_strAcctID);
    Identifier NOTARY_ID;
    theConnection.GetNotaryID(NOTARY_ID);
    Nym* pNym = theConnection.GetNym();
    Identifier NYM_ID;
    pNym->GetIdentifier(NYM_ID);

    // loop through the ALL items that make up this transaction and check to see
    // if a response to pay dividend.

    for (auto& it : theTransaction.GetItemList()) {
        Item* pItem = it;
        OT_ASSERT(nullptr != pItem);

        // if pointer not null, and it's a dividend payout, and it's an
        // acknowledgement (not a rejection or error)

        if (Item::atPayDividend == pItem->GetType()) {
            if (Item::acknowledgement == pItem->GetStatus()) {
                otOut << "TRANSACTION SUCCESS -- Server acknowledges dividend "
                         "payout.\n";
            }
            else {
                otOut << "TRANSACTION FAILURE -- Server rejects dividend "
                         "payout.\n";
            }
        }
    }
}

void OTClient::ProcessDepositResponse(OTTransaction& theTransaction,
                                      const OTServerConnection& theConnection,
                                      const Message& theReply) const
{
    const Identifier ACCOUNT_ID(theReply.m_strAcctID);
    Identifier NOTARY_ID;
    theConnection.GetNotaryID(NOTARY_ID);
    Nym* pNym = theConnection.GetNym();
    Identifier NYM_ID;
    pNym->GetIdentifier(NYM_ID);

    // loop through the ALL items that make up this transaction and check to see
    // if a response to deposit.

    for (auto& it : theTransaction.GetItemList()) {
        Item* pReplyItem = it;
        OT_ASSERT(nullptr != pReplyItem);

        // if pointer not null, and it's a deposit, and it's an acknowledgement
        // (not a rejection or error)

        if ((Item::atDeposit == pReplyItem->GetType()) ||
            (Item::atDepositCheque == pReplyItem->GetType())) {
            if (Item::acknowledgement == pReplyItem->GetStatus()) {
                otOut
                    << "TRANSACTION SUCCESS -- Server acknowledges deposit.\n";

                if (Item::atDepositCheque == pReplyItem->GetType()) {
                    // Inside OT, when processing a successful server reply to a
                    // depositCheque request,
                    // and if that cheque is found inside the Payments Inbox,
                    // ==> move it to the record box.
                    //
                    std::unique_ptr<Ledger> pLedger(Ledger::GenerateLedger(
                        NYM_ID, NYM_ID, NOTARY_ID, Ledger::paymentInbox));
                    // Beyond this point, I know that pLedger will need to be
                    // deleted or returned.
                    if ((nullptr != pLedger) && pLedger->LoadPaymentInbox() &&
                        pLedger->VerifyAccount(*pNym)) {
                        // If an incoming payment exists that matches the
                        // instrument inside the server's deposit response,
                        // then remove it from the payments inbox and save. Save
                        // a copy to the records box.
                        //
                        // Response item contains a copy of the original item,
                        // as reference string.
                        //
                        String strOriginalDepositItem;
                        Item* pOriginalItem = nullptr;
                        pReplyItem->GetReferenceString(strOriginalDepositItem);

                        std::unique_ptr<OTTransactionType> pTransType(
                            OTTransactionType::TransactionFactory(
                                strOriginalDepositItem));

                        if (nullptr != pTransType) {
                            pOriginalItem =
                                dynamic_cast<Item*>(pTransType.get());
                        }
                        if (nullptr != pOriginalItem) {
                            String strCheque;
                            pOriginalItem->GetAttachment(strCheque);

                            Cheque theCheque;
                            bool bLoadContractFromString =
                                theCheque.LoadContractFromString(strCheque);

                            if (!bLoadContractFromString) {
                                otErr << __FUNCTION__
                                      << ": ERROR loading cheque from string:\n"
                                      << strCheque << "\n";
                            }
                            else // Okay, we've got the cheque!
                            {
                                // Let's loop through the payment inbox and see
                                // if there's a matching cheque.
                                //
                                const int64_t lChequeTransNum =
                                    theCheque.GetTransactionNum();
                                const int32_t nTransCount =
                                    pLedger->GetTransactionCount();

                                for (int32_t ii = (nTransCount - 1); ii >= 0;
                                     --ii) // going backwards since we are
                                           // deleting something. (Probably only
                                           // one thing, but still...)
                                {
                                    std::unique_ptr<OTPayment> pPayment(
                                        GetInstrument(*pNym, ii, *pLedger));

                                    int64_t lPaymentTransNum = 0;

                                    if ((nullptr != pPayment) &&
                                        pPayment->SetTempValues() &&
                                        pPayment->GetTransactionNum(
                                            lPaymentTransNum) &&
                                        (lPaymentTransNum == lChequeTransNum)) {
                                        // It's the same cheque.
                                        // Remove it from the payments inbox,
                                        // and save.
                                        //
                                        OTTransaction* pTransaction =
                                            pLedger->GetTransactionByIndex(ii);
                                        String strPmntInboxTransaction;
                                        int64_t lRemoveTransaction = 0;

                                        if (nullptr != pTransaction) {
                                            pTransaction->SaveContractRaw(
                                                strPmntInboxTransaction);
                                            lRemoveTransaction =
                                                pTransaction
                                                    ->GetTransactionNum();

                                            if (false ==
                                                pLedger->DeleteBoxReceipt(
                                                    lRemoveTransaction)) {
                                                otErr << __FUNCTION__
                                                      << ": Failed trying to "
                                                         "delete the box "
                                                         "receipt for a cheque "
                                                         "being removed from a "
                                                         "payments inbox: "
                                                      << lRemoveTransaction
                                                      << "\n";
                                            }
                                            if (pLedger->RemoveTransaction(
                                                    lRemoveTransaction)) {
                                                pLedger->ReleaseSignatures();
                                                pLedger->SignContract(*pNym);
                                                pLedger->SaveContract();

                                                if (!pLedger
                                                         ->SavePaymentInbox()) {
                                                    otErr << __FUNCTION__
                                                          << ": Failure while "
                                                             "trying to save "
                                                             "payment inbox.\n";
                                                }
                                                else {
                                                    otOut
                                                        << __FUNCTION__
                                                        << ": Removed cheque "
                                                           "from payments "
                                                           "inbox. (Deposited "
                                                           "successfully.)"
                                                           "\nSaved payments "
                                                           "inbox.\n";
                                                }
                                            }
                                        } // if (nullptr != pTransaction)
                                        // We're still in the loop backwards
                                        // through the paymentInbox, checking
                                        // each for
                                        // a payment instrument. Specifically,
                                        // theCheque's cheque. That's because
                                        // this is
                                        // processChequeResponse. If there was a
                                        // cheque in my payments inbox, and I
                                        // just
                                        // successfully deposited the cheque,
                                        // then I want to remove it from my
                                        // payments
                                        // inbox. We already just did that -- so
                                        // now we want to drop a copy of it into
                                        // the record box.
                                        //
                                        // Save a copy to the record box.
                                        //
                                        if (strPmntInboxTransaction.Exists()) {
                                            const String strNymID(NYM_ID);
                                            const String strNotaryID(NOTARY_ID);
                                            const bool bExists = OTDB::Exists(
                                                OTFolders::RecordBox().Get(),
                                                strNotaryID.Get(),
                                                strNymID.Get());
                                            Ledger theRecordBox(
                                                NYM_ID, NYM_ID,
                                                NOTARY_ID); // record box
                                            bool bSuccessLoading =
                                                (bExists &&
                                                 theRecordBox.LoadRecordBox());
                                            if (bExists && bSuccessLoading)
                                                bSuccessLoading =
                                                    (theRecordBox
                                                         .VerifyContractID() &&
                                                     theRecordBox
                                                         .VerifySignature(
                                                             *pNym));
                                            //                                              bSuccessLoading
                                            // =
                                            // (theRecordBox.VerifyAccount(*pNym));
                                            // // (No need here to load all the
                                            // Box Receipts by using
                                            // VerifyAccount)
                                            else if (!bExists)
                                                bSuccessLoading =
                                                    theRecordBox.GenerateLedger(
                                                        NYM_ID, NOTARY_ID,
                                                        Ledger::recordBox,
                                                        true); // bGenerateFile=true
                                            // by this point, the nymbox
                                            // DEFINITELY exists -- or not.
                                            // (generation might have failed, or
                                            // verification.)
                                            //
                                            if (!bSuccessLoading) {
                                                String strNymID(NYM_ID),
                                                    strAcctID(NYM_ID);
                                                otOut
                                                    << __FUNCTION__
                                                    << ": WARNING: Unable to "
                                                       "load, verify, or "
                                                       "generate recordBox, "
                                                       "with IDs: " << strNymID
                                                    << " / " << strAcctID
                                                    << "\n";
                                            }
                                            else // --- ELSE --- Success
                                                   // loading the recordBox and
                                                   // verifying its contractID
                                                   // and signature, (OR success
                                                   // generating the ledger.)
                                            {
                                                // Currently in
                                                // getBoxReceiptResponse,
                                                // we are taking an incoming
                                                // cheque from the nymbox
                                                // and adding it to the payments
                                                // inbox. From there the user
                                                // might choose to deposit it.
                                                // When he does that, he'll
                                                // receive a server reply, which
                                                // is what we're processing here
                                                // in this function. So now that
                                                // we've got that reply, we want
                                                // to move the cheque notice
                                                // from the payments inbox, and
                                                // into the record box at this
                                                // point HERE, when we've just
                                                // above removed it from the
                                                // payments inbox (on successful
                                                // deposit.)
                                                //
                                                load_str_trans_add_to_ledger(
                                                    NYM_ID,
                                                    strPmntInboxTransaction,
                                                    "recordBox",
                                                    lRemoveTransaction, *pNym,
                                                    theRecordBox);
                                            }
                                        }
                                    } // pPayment
                                }
                                // for (payments inbox)
                            }
                        } // if nullptr != pOriginalItem
                    }
                    else {
                        String strNymID(NYM_ID), strAcctID(NYM_ID);
                        otWarn << __FUNCTION__ << ": Unable to load or verify "
                                                  "payments inbox: User "
                               << strNymID << " / Acct " << strAcctID << "\n";
                    }
                }
            }
            else {
                otOut << __FUNCTION__
                      << ": TRANSACTION FAILURE -- Server rejects deposit.\n";
            }
        }
    }
}

/// It's definitely a withdrawal, we just need to iterate through the items in
/// the transaction and
/// grab any cash tokens that are inside, to save inside a purse.  Also want to
/// display any vouchers.
void OTClient::ProcessWithdrawalResponse(
    OTTransaction& theTransaction, const OTServerConnection& theConnection,
    const Message& theReply) const
{
    const Identifier ACCOUNT_ID(theReply.m_strAcctID);
    Identifier NOTARY_ID;
    theConnection.GetNotaryID(NOTARY_ID);

    String strNotaryID(NOTARY_ID);

    Nym* pNym = theConnection.GetNym();
    Identifier NYM_ID;
    pNym->GetIdentifier(NYM_ID);

    const String strNymID(NYM_ID);

    Nym* pServerNym = const_cast<Nym*>(
        theConnection.GetServerContract()->GetContractPublicNym());

    // loop through the ALL items that make up this transaction and check to see
    // if a response to withdrawal.

    // if pointer not null, and it's a withdrawal, and it's an acknowledgement
    // (not a rejection or error)
    for (auto& it : theTransaction.GetItemList()) {
        Item* pItem = it;
        OT_ASSERT(nullptr != pItem);
        // VOUCHER WITHDRAWAL
        //
        // If we got a reply to a voucher withdrawal, we'll just display the
        // voucher
        // on the screen (if the server sent us one...)
        //
        if ((Item::atWithdrawVoucher == pItem->GetType()) &&
            (Item::acknowledgement == pItem->GetStatus())) {
            String strVoucher;
            Cheque theVoucher;

            pItem->GetAttachment(strVoucher);

            if (theVoucher.LoadContractFromString(strVoucher)) {
                otOut << "\nReceived voucher from server:\n\n" << strVoucher
                      << "\n\n";
            }
        }
        // CASH WITHDRAWAL
        //
        // If the item is a response to a cash withdrawal, we want to save the
        // coins into a purse
        // somewhere on the computer. That's cash! Gotta keep it safe.
        //
        else if ((Item::atWithdrawal == pItem->GetType()) &&
                 (Item::acknowledgement == pItem->GetStatus())) {
            String strPurse;
            pItem->GetAttachment(strPurse);

            Purse thePurse(NOTARY_ID);

            if (thePurse.LoadContractFromString(strPurse)) {
                // When we made the withdrawal request, we saved that purse
                // pointer in the
                // wallet so that we could get to the private coin unblinding
                // data when we
                // needed it (now).
                Purse* pRequestPurse = m_pWallet->GetPendingWithdrawal();

                String strInstrumentDefinitionID(
                    thePurse.GetInstrumentDefinitionID());
                std::unique_ptr<Mint> pMint(
                    Mint::MintFactory(strNotaryID, strInstrumentDefinitionID));
                OT_ASSERT(nullptr != pMint);
                // Unlike the purse which we read out of a message,
                // now we try to open a purse as a file on the client side,
                // keyed by Instrument Definition Id.  (The client should
                // already have one
                // purse file for each instrument definition, if he already has
                // cash.)
                //
                // We don't want to just overwrite that file. So instead, we
                // try to load that purse first, then add the token, then save
                // it
                // again.
                Purse theWalletPurse(thePurse);
                // TODO verify the wallet purse when loaded. My signature should
                // be the last thing on it.

                // TODO: I don't check this for failure. If the file doesn't
                // exist,
                // we are still going to save the purse there regardless.
                // HOWEVER need to make sure the wallet software has good backup
                // strategy.  In the event that tokens are overwritten here, it
                // shouldn't be a problem since they would be in the archive
                // somewhere.

                theWalletPurse.LoadPurse(strNotaryID.Get(), strNymID.Get(),
                                         strInstrumentDefinitionID.Get());

                bool bSuccess = false;

                if ((nullptr != pRequestPurse) && (nullptr != pServerNym) &&
                    pMint->LoadMint() && pMint->VerifyMint(*pServerNym)) {
                    Token* pToken = nullptr;
                    while ((pToken = thePurse.Pop(*pNym)) != nullptr) {
                        OT_ASSERT(nullptr != pToken);

                        Token* pOriginalToken = pRequestPurse->Pop(*pNym);

                        if (nullptr == pOriginalToken) {
                            otErr << "ERROR, processing withdrawal response, "
                                     "but couldn't find original token:"
                                  << strPurse << "\n";
                        }
                        else if (Token::signedToken == pToken->GetState()) {
                            otWarn << "Retrieved signed token from purse, and "
                                      "have corresponding withdrawal request "
                                      "in wallet. Unblinding...\n\n";

                            if (pToken->ProcessToken(*pNym, *pMint,
                                                     *pOriginalToken)) {
                                // Now that it's processed, let's save it again.
                                pToken->ReleaseSignatures();
                                pToken->SignContract(*pNym);
                                pToken->SaveContract();

                                bSuccess = true;

                                // add it to the existing client-side purse for
                                // storing tokens of that instrument definition
                                theWalletPurse.Push(*pNym, *pToken);
                            }
                            else {
                                bSuccess = false;
                                if (pToken) {
                                    delete pToken;
                                    pToken = nullptr;
                                }
                                // The while loop starts by allocating a
                                // pOriginalToken, so I want to
                                // delete it for each iteration and keep things
                                // clean.
                                if (pOriginalToken) {
                                    delete pOriginalToken;
                                    pOriginalToken = nullptr;
                                }
                                break;
                            }
                        }

                        // The while loop starts by allocating a pToken, so I
                        // want to
                        // delete it for each iteration and keep things clean.
                        if (nullptr != pToken) {
                            delete pToken;
                            pToken = nullptr;
                        }
                        // The while loop starts by allocating a pOriginalToken,
                        // so I want to
                        // delete it for each iteration and keep things clean.
                        if (pOriginalToken) {
                            delete pOriginalToken;
                            pOriginalToken = nullptr;
                        }
                    }
                }

                if (bSuccess) {
                    // Sign it, save it.
                    theWalletPurse.ReleaseSignatures(); // Might as well,
                                                        // they're no good
                                                        // anyway once the data
                                                        // has changed.
                    theWalletPurse.SignContract(*pNym);
                    theWalletPurse.SaveContract();
                    theWalletPurse.SavePurse(strNotaryID.Get(), strNymID.Get(),
                                             strInstrumentDefinitionID.Get());

                    otOut << "SUCCESSFULLY UNBLINDED token, and added the cash "
                             "to the local purse, and saved.\n";
                }
            }
        }
    }
}

struct OTClient::ProcessServerReplyArgs
{
    Identifier ACCOUNT_ID, NOTARY_ID;
    Nym* pNym;
    Identifier NYM_ID;
    String strNotaryID, strNymID;
    Nym* pServerNym;
};

void OTClient::setRecentHash(const Message& theReply, const String& strNotaryID,
                             Nym* pNym, bool setNymboxHash)
{
    Identifier NYMBOX_HASH, RECENT_HASH;
    const std::string str_server(strNotaryID.Get());

    if (theReply.m_strNymboxHash.Exists()) {
        if (setNymboxHash) {
            NYMBOX_HASH.SetString(theReply.m_strNymboxHash);
        }
        RECENT_HASH.SetString(theReply.m_strNymboxHash);

        bool bNymboxHash = false;
        if (setNymboxHash) {
            bNymboxHash = pNym->SetNymboxHash(str_server, NYMBOX_HASH);
        }
        bool bRecentHash = pNym->SetRecentHash(str_server, RECENT_HASH);

        if (!bRecentHash) {
            otErr << theReply.m_strCommand
                  << ": Failed getting NymboxHash (to store as 'recent "
                     "hash') from Nym for server: " << str_server << "\n";
        }
        if (setNymboxHash && !bNymboxHash) {
            otErr << "Failed setting NymboxHash on Nym for server: "
                  << str_server << "\n";
        }
        if (bRecentHash || (setNymboxHash && bNymboxHash)) {
            Nym* pSignerNym = pNym;
            pNym->SaveSignedNymfile(*pSignerNym);
        }
    }
}

bool OTClient::processServerReplyTriggerClause(const Message& theReply,
                                               ProcessServerReplyArgs& args)
{
    setRecentHash(theReply, args.strNotaryID, args.pNym, false);

    return true;
}

bool OTClient::processServerReplyGetRequestNumber(const Message& theReply,
                                                  ProcessServerReplyArgs& args)
{

    int64_t lNewRequestNumber = theReply.m_lNewRequestNum;

    // so the proper request number is sent next time, we take the one that
    // the server just sent us, and we ask the wallet to save it somewhere
    // safe
    // (like in the nymfile)

    // In the future, I will have to write a function on the wallet that
    // actually
    // takes the reply, looks up the associated nym in the wallet, verifies
    // that it was EXPECTING a response to GetRequestNumber, (cause otherwise it
    // won't
    // know which one to update) and then updates the request number there.
    // In the meantime there is only one connection, and it already has a
    // pointer to
    // the Nym,  so I'll just tell it to update the request number that way
    // for now.

    OTServerConnection& theConnection = *m_pConnection;
    theConnection.OnServerResponseToGetRequestNumber(lNewRequestNumber);

    setRecentHash(theReply, args.strNotaryID, args.pNym, false);

    return true;
}

bool OTClient::processServerReplyCheckNym(const Message& theReply,
                                          ProcessServerReplyArgs& args)
{
    const String strNymID2(theReply.m_strNymID2),
        strPubkey(theReply.m_strNymPublicKey.Get()); // Old style (It's
                                                     // deprecated to pass a
                                                     // pubkey directly like
                                                     // this.)

    // First try to get Credentials, if there are any.
    //
    const OTASCIIArmor& ascArmor =
        theReply.m_ascPayload; // credentialList  (New style! Credentials.)
    const OTASCIIArmor& ascArmor2 = theReply.m_ascPayload2; // credentials
    const bool bHasCredentials = (ascArmor.Exists() && ascArmor2.Exists());
    if (bHasCredentials) // New style of doing things, for Nym keys.
                         // Credentials!
    {
        String strCredentialList;
        ascArmor.GetString(strCredentialList);

        if (strCredentialList.Exists()) {
            std::unique_ptr<OTDB::Storable> pStorable(OTDB::DecodeObject(
                OTDB::STORED_OBJ_STRING_MAP, ascArmor2.Get()));
            OTDB::StringMap* pMap =
                dynamic_cast<OTDB::StringMap*>(pStorable.get());
            if (nullptr == pMap)
                otOut << __FUNCTION__ << ": Failed decoding StringMap "
                                         "object in checkNymResponse.\n";
            else // IF the list saved, then we save the credentials
                 // themselves...
            {
                String::Map& theMap = pMap->the_map;
                Nym theTargetNym;
                theTargetNym.SetIdentifier(strNymID2);

                if (false ==
                    theTargetNym.LoadFromString(strCredentialList, &theMap)) {
                    otErr << __FUNCTION__
                          << ": checkNymResponse: Failure loading nym "
                          << strNymID2 << " from credential string.\n";
                }
                // Now that the Nym has been loaded up from the message
                // parameters,
                // including the list of credential IDs, and the map
                // containing the
                // credentials themselves, let's try to Verify the
                // pseudonym. If we
                // verify, then we're safe to save the credentials to
                // storage.
                //
                else if (!theTargetNym.VerifyPseudonym()) {
                    otErr << __FUNCTION__ << ": checkNymResponse: Loaded nym "
                          << strNymID2 << " from credentials, but then it "
                                          "failed verifying.\n";
                }
                else // Okay, we loaded the Nym up from the credentials in
                       // the message, AND
                {      // verified the Nym (including the credentials.)
                    // So let's save it to local storage...
                    //
                    std::string str_nym_id = strNymID2.Get();
                    String strFilename;
                    strFilename.Format("%s.cred", str_nym_id.c_str());

                    bool bStoredList = false;
                    String strOutput;
                    if (ascArmor.Exists() &&
                        ascArmor.WriteArmoredString(
                            strOutput,
                            "CREDENTIAL LIST") && // bEscaped=false by
                                                  // default.
                        strOutput.Exists())
                        bStoredList = OTDB::StorePlainString(
                            strOutput.Get(), OTFolders::Pubcred().Get(),
                            strFilename.Get());
                    if (!bStoredList)
                        otErr << __FUNCTION__
                              << ": Failed trying to armor or store "
                              << strFilename << ".\n";
                    else {
                        otOut << "checkNymResponse: Success saving public "
                                 "credential list for Nym: " << strNymID2
                              << "\n";
                        for (auto& it : theMap) {
                            std::string str_cred_id = it.first;
                            String strCredential(it.second);
                            bool bStoredCredential = false;
                            strOutput.Release();
                            OTASCIIArmor ascLoopArmor(strCredential);
                            if (ascLoopArmor.Exists() &&
                                ascLoopArmor.WriteArmoredString(
                                    strOutput,
                                    "CREDENTIAL") && // bEscaped=false by
                                                     // default.
                                strOutput.Exists())
                                bStoredCredential = OTDB::StorePlainString(
                                    strOutput.Get(), OTFolders::Pubcred().Get(),
                                    str_nym_id, str_cred_id);
                            if (!bStoredCredential)
                                otErr << __FUNCTION__
                                      << ": Failed trying to store "
                                         "credential " << str_cred_id
                                      << " for nym " << str_nym_id << ".\n";
                            else
                                otOut << "checkNymResponse: Success saving "
                                         "public "
                                         "credential ID: " << str_cred_id
                                      << "\n";
                        }
                    } // Success decoding string map of credential contents.
                }
            }
        } // credential list exists, after base64-decoding.
    }     // Has Credentials.
    // Old-style (deprecated.)
    //
    else if (strPubkey.Exists()) {
        String strPath = strNymID2.Get();
        // Next we save the public key in the pubkeys folder...
        //
        Nym thePubkeyNym(strNymID2);

        if (thePubkeyNym.SetPublicKey(strPubkey) &&
            thePubkeyNym.VerifyPseudonym()) {
            if (thePubkeyNym.SavePublicKey(strPath))
                otOut
                    << "checkNymResponse: (Deprecated.) Success saving public "
                       "key file for Nym: " << strNymID2 << "\n";
        }
    }

    return true;
}

bool OTClient::processServerReplyNotarizeTransaction(
    const Message& theReply, ProcessServerReplyArgs& args)
{
    otOut << "Received server response to notarize Transactions message.\n";

    setRecentHash(theReply, args.strNotaryID, args.pNym, false);
    ProcessIncomingTransactions(*m_pConnection, theReply);

    // todo (gui):
    // This block assumes that the above "notarizeTransactionResponse", being
    // successful, probably changed
    // the account balance. A nice GUI would probably interpret the reply
    // and edit the local files
    // to update them to match (since it was successful). In fact, the above
    // call to ProcessIncomingTransactions
    // does some of that sort of stuff already, at least for issued numbers
    // on the nym.
    //
    // (For now we just re-download the files.)

    return true;
}

bool OTClient::processServerReplyGetTransactionNumbers(
    const Message& theReply, ProcessServerReplyArgs& args)
{
    otOut << "Received server response to Get Transaction Num message.\n";

    setRecentHash(theReply, args.strNotaryID, args.pNym, false);
    return true;
}

bool OTClient::processServerReplyGetNymBox(const Message& theReply,
                                           Ledger* pNymbox,
                                           ProcessServerReplyArgs& args)
{
    const auto& pNym = args.pNym;
    const auto& NOTARY_ID = args.NOTARY_ID;
    const auto& NYM_ID = args.NYM_ID;

    String strReply(theReply);

    otOut << "Received getNymboxResponse server response ("
          << (theReply.m_bSuccess ? "success" : "failure") << ")\n";

    // base64-Decode the server reply's payload into strInbox
    String strNymbox(theReply.m_ascPayload);

    // IF pNymbox NOT nullptr, THEN USE IT INSTEAD OF LOADING MY OWN.
    // Except... getNymboxResponse isn't dropped as a replyNotice into the
    // Nymbox,
    // so we'll never end up here except in cases where it needs to be
    // loaded. I can even ASSERT here, that the pointer is actually nullptr!
    //
    OT_ASSERT_MSG(nullptr == pNymbox, "Nymbox pointer is expected to be "
                                      "nullptr here, since getNymboxResponse "
                                      "isn't dropped as a server "
                                      "replyNotice into the nymbox.");

    // Load the ledger object from that string.
    Ledger theNymbox(NYM_ID, NYM_ID, NOTARY_ID);

    setRecentHash(theReply, args.strNotaryID, args.pNym, true);

    // I receive the nymbox, verify the server's signature, then RE-SIGN IT
    // WITH MY OWN
    // SIGNATURE, then SAVE it to local storage.  So any FUTURE checks of
    // this nymbox
    // would require MY signature, not the server's, to verify. But in this
    // one spot,
    // just before saving, I need to verify the server's first.
    // UPDATE: Keeping the server's signature, and just adding my own.
    //
    if (theNymbox.LoadNymboxFromString(
            strNymbox)) // && theNymbox.VerifyAccount(*pServerNym)) No point
                        // doing this, since the client hasn't even had a
                        // chance to download the box receipts yet.
                        // (VerifyAccount will fail before then...)
    {

        //
        // UPDATE: We will have to rely on the Developer using the OT API to
        // call
        // OT_API_FlushSentMessages IMMEDIATELY after calling getNymbox and
        // receiving
        // a successful reply. Why? Because that's the only way to give him
        // the chance
        // to see if certain replies are there or not (before they get
        // removed.) That way
        // he can do his own harvesting, do a re-try, etc and then finally
        // when he is done
        // with that, do the flush.
        //

        theNymbox.ReleaseSignatures(); // Now I'm keeping the server
                                       // signature, and just adding my own.
        theNymbox.SignContract(*pNym); // UPDATE: Releasing the signature
                                       // again, since Receipts are now
                                       // fully functional.
        theNymbox.SaveContract();      // Thus we can prove the Nymbox using the
                                       // last signed transaction receipt. This
                                       // means
        theNymbox.SaveNymbox(); // the receipt is our proof, and the nymbox
                                // becomes just an intermediary file that is
        // downloaded occasionally (like checking for new email) but no
        // trust is risked since
        // the downloaded file is always verified against the receipt!
    }
    else {
        otErr << "OTClient::ProcessServerReply: Error loading or verifying "
                 "nymbox during getNymboxResponse:\n\n" << strNymbox << "\n";
    }

    return true;
}

bool OTClient::processServerReplyGetBoxReceipt(const Message& theReply,
                                               Ledger* pNymbox,
                                               ProcessServerReplyArgs& args)
{
    const auto& pNym = args.pNym;
    const auto& NOTARY_ID = args.NOTARY_ID;
    const auto& NYM_ID = args.NYM_ID;
    const auto& pServerNym = args.pServerNym;
    const auto& strNymID = args.strNymID;
    const auto& strNotaryID = args.strNotaryID;

    otOut << "Received server response to getBoxReceipt request ("
          << (theReply.m_bSuccess ? "success" : "failure") << ")\n";

    // IF pNymbox NOT nullptr, THEN USE IT INSTEAD OF LOADING MY OWN.
    // Except... getNymboxResponse isn't dropped as a replyNotice into the
    // Nymbox,
    // so we'll never end up here except in cases where it needs to be
    // loaded. I can even ASSERT here, that the pointer is actually nullptr!
    //
    OT_ASSERT_MSG(nullptr == pNymbox,
                  "Nymbox pointer is expected to be "
                  "nullptr here, since getBoxReceiptResponse "
                  "isn't dropped as a server "
                  "replyNotice into the nymbox.");

    // Note: I don't HAVE to load the ledger, and what if there are 500000
    // receipts in it?
    // Do I want to reload it EVERY time? Therefore
    bool bErrorCondition = false;
    bool bSuccessLoading =
        true; // We don't need to load the ledger, so that's commented out.

    switch (theReply.m_lDepth) { // No need to load the ledger at this
                                 // point...  plus, it would slow things
                                 // down.
    case 0: // bSuccessLoading = pLedger->LoadNymbox();    break;
    case 1: // bSuccessLoading = pLedger->LoadInbox();    break;
    case 2: // bSuccessLoading = pLedger->LoadOutbox();    break;
        break;
    default:
        otErr << __FUNCTION__ << ": getBoxReceiptResponse: Unknown box type: "
              << theReply.m_lDepth << "\n";
        bErrorCondition = true;
        break;
    }

    if (bSuccessLoading && !bErrorCondition) {
        // At this point, the ledger is loaded. Now let's use it for what we
        // really
        // wanted: To save the Box Receipt!
        // Update: not loading ledger -- it would slow things down. Added a
        // method that allowed me to circumvent loading it.

        // base64-Decode the server reply's payload into strTransaction
        //
        const String strTransType(theReply.m_ascPayload);
        std::unique_ptr<OTTransactionType> pTransType;

        if (strTransType.Exists())
            pTransType.reset(
                OTTransactionType::TransactionFactory(strTransType));

        if (nullptr == pTransType)
            otErr << __FUNCTION__
                  << ": getBoxReceiptResponse: Error instantiating transaction "
                     "type based on decoded theReply.m_ascPayload:\n\n"
                  << strTransType << "\n";
        else {
            OTTransaction* pBoxReceipt =
                dynamic_cast<OTTransaction*>(pTransType.get());

            if (nullptr == pBoxReceipt)
                otErr << __FUNCTION__
                      << ": getBoxReceiptResponse: Error dynamic_cast from "
                         "transaction type to transaction, based on "
                         "decoded theReply.m_ascPayload:\n\n" << strTransType
                      << "\n\n";
            else if (!pBoxReceipt->VerifyAccount(*pServerNym))
                otErr << __FUNCTION__
                      << ": getBoxReceiptResponse: Error: Box Receipt "
                      << pBoxReceipt->GetTransactionNum() << " in "
                      << ((theReply.m_lDepth == 0)
                              ? "nymbox"
                              : ((theReply.m_lDepth == 1) ? "inbox" : "outbox"))
                      << " fails VerifyAccount().\n"; // outbox is 2.);
            else if (pBoxReceipt->GetTransactionNum() !=
                     theReply.m_lTransactionNum)
                otErr << __FUNCTION__
                      << ": getBoxReceiptResponse: Error: Transaction Number "
                         "doesn't match on the box receipt itself ("
                      << pBoxReceipt->GetTransactionNum()
                      << "), versus the one listed in the reply message ("
                      << theReply.m_lTransactionNum << ").\n";
            // Note: Account ID and Notary ID were already verified, in
            // VerifyAccount().
            else if (pBoxReceipt->GetNymID() != NYM_ID) {
                const String strPurportedNymID(pBoxReceipt->GetNymID());
                otErr
                    << __FUNCTION__
                    << ": getBoxReceiptResponse: Error: NymID doesn't match on "
                       "the box receipt itself (" << strPurportedNymID
                    << "), versus the one listed in the reply message ("
                    << theReply.m_strNymID << ").\n";
            }
            else // FINALLY we have the Ledger AND the Box Receipt both
                   // loaded at the same time.
            {      // UPDATE: Not loading the ledger at this point. Not
                   // necessary. Faster without it.

                // UPDATE: We will ASSUME the abbreviated receipt is in the
                // NYMBOX, which is WHY
                // we are now downloading the FULL BOX RECEIPT. We will SAVE
                // it for the Nymbox,
                // which finishes the Nymbox (already in box as abbreviated,
                // and already saved in full
                // in box receipts folder). Next we will also add it to the
                // PAYMENT INBOX and RECORD BOX,
                // if it's the right sort of receipt. We will also save
                // THEIR versions of the FULL BOX RECEIPT,
                // just as we did for the Nymbox here.

                if ((OTTransaction::instrumentNotice ==
                     pBoxReceipt->GetType()) ||
                    (OTTransaction::instrumentRejection ==
                     pBoxReceipt->GetType())) {
                    // Just make sure not to add it if it's already there...
                    if (!strNotaryID.Exists()) {
                        otErr << __FUNCTION__
                              << ": strNotaryID doesn't Exist!\n";
                        OT_FAIL;
                    }
                    if (!strNymID.Exists()) {
                        otErr << __FUNCTION__ << ": strNymID dosn't Exist!\n";
                        OT_FAIL;
                    }
                    const bool bExists =
                        OTDB::Exists(OTFolders::PaymentInbox().Get(),
                                     strNotaryID.Get(), strNymID.Get());
                    Ledger thePmntInbox(NYM_ID, NYM_ID,
                                        NOTARY_ID); // payment inbox
                    bool bSuccessLoading =
                        (bExists && thePmntInbox.LoadPaymentInbox());
                    if (bExists && bSuccessLoading)
                        bSuccessLoading = (thePmntInbox.VerifyContractID() &&
                                           thePmntInbox.VerifySignature(*pNym));
                    //                          bSuccessLoading    =
                    // (thePmntInbox.VerifyAccount(*pNym)); // (No need here
                    // to load all the Box Receipts by using VerifyAccount)
                    else if (!bExists)
                        bSuccessLoading = thePmntInbox.GenerateLedger(
                            NYM_ID, NOTARY_ID, Ledger::paymentInbox,
                            true); // bGenerateFile=true
                    // by this point, the nymbox DEFINITELY exists -- or
                    // not. (generation might have failed, or verification.)

                    if (!bSuccessLoading) {
                        String strNymID(NYM_ID), strAcctID(NYM_ID);
                        otOut << __FUNCTION__
                              << ": getBoxReceiptResponse: WARNING: Unable to "
                                 "load, verify, or generate paymentInbox, "
                                 "with IDs: " << strNymID << " / " << strAcctID
                              << "\n";
                    }
                    else // --- ELSE --- Success loading the payment inbox
                           // and recordBox and verifying their contractID
                           // and signature, (OR success generating the
                           // ledger.)
                    {
                        // The transaction (which we are putting into the
                        // payment inbox) will not
                        // be removed from the nymbox until we receive the
                        // server's success reply to
                        // this "process Nymbox" message. That's why you see
                        // me adding it here to
                        // the payment inbox, while not removing it from the
                        // Nymbox (because that
                        // will happen once the reply is received.) NOTE:
                        // Need to make sure the
                        // associated box receipt doesn't get MARKED FOR
                        // DELETION when being removed
                        // at that time.
                        //
                        //                          void
                        // load_str_trans_add_to_ledger(const OTIdentifier&
                        // the_nym_id, const OTString& str_trans, const
                        // OTString str_box_type, const int64_t& lTransNum,
                        // OTPseudonym& the_nym, OTLedger& ledger);

                        // Basically we are taking this receipt from the
                        // Nymbox, and also adding copies of it
                        // to the paymentInbox and the recordBox.
                        //
                        // QUESTION: what if I ERASE it out of my recordBox.
                        // Won't it pop back up again?
                        // ANSWER: YES, but not if I do this instead at
                        // getBoxReceiptResponse which will only happen once.
                        //         UPDATE: which I now AM (see our location
                        // here...)
                        // HOWEVER: Most likely not, because this notice
                        // will no longer BE in my Nymbox...
                        //
                        // QUESTION: What if I ERASE it out of my
                        // paymentInbox? Won't this pop back there again?
                        // ANSWER: I can't erase it out of there. I can
                        // either accept it or reject it. Either way,
                        // it is removed from my paymentInbox at that time
                        // by OT. Like above, if a copy were still
                        // in the Nymbox, I would get a duplicate here when
                        // processing Nymbox again. But MOST TIMES,
                        // there will be no duplicate, because it will
                        // already be cleaned out of my Nymbox anyway.
                        //
                        //
                        const int64_t lTransNum =
                            pBoxReceipt->GetTransactionNum();

                        // If pBoxReceipt->GetType() is instrument notice,
                        // add to the payments inbox.
                        // (It will be moved to record box after the
                        // incoming payment is deposited or discarded.)
                        //
                        load_str_trans_add_to_ledger(NYM_ID, strTransType,
                                                     "paymentInbox", lTransNum,
                                                     *pNym, thePmntInbox);
                        //                          load_str_trans_add_to_ledger(NYM_ID,
                        // strTransType, "recordBox",    lTransNum, *pNym,
                        // theRecordBox); // No longer here. Moved to
                        // processDepositResponse

                    } // --- ELSE --- Success loading the payment inbox and
                      // verifying its contractID and signature, OR success
                      // generating the ledger.
                }     // if pBoxReceipt is instrumentNotice or
                      // instrumentRejection...

                //                    pBoxReceipt->ReleaseSignatures();

                // I don't release the server's signature, so later on I can
                // verify either
                // signature -- the server's or pNym's. Both should be on
                // the receipt.
                // UPDATE: We're not changing the content of the Box Receipt
                // AT ALL
                // because we don't want to already its message digest,
                // which will be
                // compared to the hash stored in the abbreviated version of
                // the same receipt.
                //
                //                    pBoxReceipt->SignContract(*pNym);
                //                    pBoxReceipt->SaveContract();

                //                    if
                // (!pBoxReceipt->SaveBoxReceipt(*pLedger))
                // // <===================
                if (!pBoxReceipt->SaveBoxReceipt(
                        theReply.m_lDepth)) // <===================
                    otErr << __FUNCTION__
                          << ": getBoxReceiptResponse(): Failed trying to "
                             "SaveBoxReceipt. Contents:\n\n" << strTransType
                          << "\n\n";
                /* theReply.m_lDepth in this context stores boxType. Value
                 * can be: 0/nymbox,1/inbox,2/outbox*/

            } // We can save the box receipt.
        }     // Success loading the boxReceipt from the server reply
    }         // No error condition.
    else {
        otErr
            << __FUNCTION__
            << ": SHOULD NEVER HAPPEN: getBoxReceiptResponse: failure loading "
               "box, or verifying it. NymID: " << theReply.m_strNymID
            << "  AcctID: " << theReply.m_strAcctID << " \n";
    }

    return true;
}

bool OTClient::processServerReplyProcessInbox(const Message& theReply,
                                              Ledger* pNymbox,
                                              ProcessServerReplyArgs& args)
{
    // IN EITHER of these cases (processInboxResponse, processNymboxResponse),
    // the
    // number of
    // transaction numbers on my Nym has
    // probably changed.
    // But the server acknowledgment here confirms it, so I should remove any
    // issued numbers,
    // save the nym, etc.
    //
    const auto& pNym = args.pNym;
    // todo: make this const and fix the error -> why is ACCOUNT_ID being
    // overwritten?
    auto& ACCOUNT_ID = args.ACCOUNT_ID;
    const auto& NOTARY_ID = args.NOTARY_ID;
    const auto& NYM_ID = args.NYM_ID;
    const auto& pServerNym = args.pServerNym;
    const auto& strNymID = args.strNymID;

    String strNotaryID(NOTARY_ID), strReply(theReply);

    otOut << "Received server response: " << theReply.m_strCommand << " \n";

    setRecentHash(theReply, args.strNotaryID, args.pNym, false);
    // If the server acknowledges either of the above commands, then my
    // transaction
    // numbers have changed. I need to read the numbers from my last
    // transaction agreement
    // (which should be saved in this server reply) and make sure to update
    // my nym accordingly.
    //
    String strOriginalMessage;
    if (theReply.m_ascInReferenceTo.Exists())
        theReply.m_ascInReferenceTo.GetString(strOriginalMessage);

    Message theOriginalMessage;

    if (strOriginalMessage.Exists() &&
        theOriginalMessage.LoadContractFromString(strOriginalMessage) &&
        theOriginalMessage.VerifySignature(*pNym)) {
        String strLedger, strReplyLedger;

        // todo: we are already in the function which is called
        // for processNymboxResponse and processInboxResponse. Maybe call this
        // func.
        // with a flag so we do not have to compare again?
        if (theReply.m_strCommand.Compare("processNymboxResponse"))
            ACCOUNT_ID = NYM_ID; // For Nymbox, NymID *is* AcctID.

        Ledger theLedger(NYM_ID, ACCOUNT_ID, NOTARY_ID),
            theReplyLedger(NYM_ID, ACCOUNT_ID, NOTARY_ID);

        theOriginalMessage.m_ascPayload.GetString(strLedger);
        theReply.m_ascPayload.GetString(strReplyLedger);

        if (!strLedger.Exists()) {
            String strLogData(theOriginalMessage);
            otErr << "Strange: Received server acknowledgment ("
                  << theReply.m_strCommand
                  << "), but found no request ledger within your original "
                     "message:\n\n" << strLogData << "\n\n";
        }
        else if (!strReplyLedger.Exists()) {
            String strReply(theReply);
            otOut << "Strange... received server acknowledgment ("
                  << theReply.m_strCommand
                  << "), but found no reply ledger within:\n\n" << strReply
                  << "\n\n";
        }
        else if (!theLedger.LoadLedgerFromString(strLedger)) {
            otErr << "Strange: Received server acknowledgment ("
                  << theReply.m_strCommand
                  << "), but unable to load original request ledger from "
                     "string:\n\n" << strLedger << "\n\n";
        }
        else if (!theLedger.VerifySignature(*pNym)) {
            otErr << "Strange: Received server acknowledgment ("
                  << theReply.m_strCommand
                  << "), but unable to verify your signature on the "
                     "original request ledger:\n\n" << strLedger << "\n\n";
        }
        else if (!theReplyLedger.LoadLedgerFromString(strReplyLedger)) {
            otErr << "Strange: Received server acknowledgment ("
                  << theReply.m_strCommand
                  << "), but unable to load the reply ledger from string:\n\n"
                  << strReplyLedger << "\n\n";
        }
        else if (!theReplyLedger.VerifySignature(*pServerNym)) {
            otErr << "Strange: Received server acknowledgment ("
                  << theReply.m_strCommand
                  << "), but unable to verify server's signature on the "
                     "reply ledger within:\n\n" << strReplyLedger << "\n\n";
        }
        else {
            // atAcceptItemReceipt: Whether success or fail, remove the
            // number used from list of responsibility.
            //                      ALSO, if success, remove the number from
            // the original cheque or the original transfer request.
            //
            // Other options are not handled here, but they ARE handled
            // elsewhere (above). They are:
            //
            // atDeposit:        Whether success or fail, remove the number
            // from my list of responsibility.
            // atWithdrawal:    Whether success or fail, remove the number
            // from my list of responsibility.
            // atAcceptPending:    Whether success or fail, remove the
            // number from my list of responsibility.
            // atTransfer:        If success, KEEP the number on my issued
            // list. (Remove when transfer receipt is accepted.)
            //                    If failure, REMOVE the number from my
            // issued list. (Use a new one next time.)
            // atMarketOffer:   If success, KEEP the number on my issued
            // list. (Removed when final receipt is created.)
            //                    If failure, REMOVE the number from my
            // issued list. (Use a new one next time.)
            // atCancelCronItem: Whether success or fail, remove the number
            // from my list of responsibility.
            // atExchangeBasket: Whether success or fail, remove the number
            // from my list of responsibility.

            OTTransaction* pTransaction = nullptr;
            OTTransaction* pReplyTransaction = nullptr;

            // todo: we are already in the function which is called
            // for processNymboxResponse and processInboxResponse. Maybe call
            // this
            // func.
            // with a flag so we do not have to compare again?
            if (theReply.m_strCommand.Compare(
                    "processInboxResponse")) // We're processing the SERVER's
                                             // REPLY
                                             // to our processInbox request.
            {
                pTransaction =
                    theLedger.GetTransaction(OTTransaction::processInbox);
                pReplyTransaction = theReplyLedger.GetTransaction(
                    OTTransaction::atProcessInbox);

                if (nullptr != pTransaction) {
                    // pNym->RemoveTransactionNum() happened whenever I
                    // first fired off the processInbox request.
                    // Now let's remove that number from our ISSUED list of
                    // responsibility, since we got a server reply...
                    //  <====> Whatever trans num I used to process inbox is
                    // now OFF my issued list on server side!
                    // (Therefore remove here too, to match..)
                    //
                    const bool bIsSignedOut = pNym->VerifyIssuedNum(
                        strNotaryID, pTransaction->GetTransactionNum());

                    // Why? Because we might have already processed this,
                    // when it first happened, and now we're just
                    // seeing a repeat of the message from a nymbox notice.
                    // (Some messages are so important, you get
                    // a nymbox notice including a copy of the message, so
                    // the server can make SURE you have processed
                    // the reply. This was added to prevent syncing issues
                    // between client and server.)
                    //
                    if (bIsSignedOut)
                        pNym->RemoveIssuedNum(*pNym, strNotaryID,
                                              pTransaction->GetTransactionNum(),
                                              true); // bool bSave=true
                    if (bIsSignedOut && (nullptr != pReplyTransaction)) {
                        // Load the inbox.
                        Ledger theInbox(NYM_ID, ACCOUNT_ID, NOTARY_ID);
                        Ledger theRecordBox(NYM_ID, ACCOUNT_ID, NOTARY_ID);

                        bool bInbox = OTDB::Exists(OTFolders::Inbox().Get(),
                                                   strNotaryID.Get(),
                                                   theReply.m_strAcctID.Get());

                        if (bInbox && theInbox.LoadInbox())
                            bInbox = theInbox.VerifyAccount(*pNym);

                        // I JUST had this loaded if I sent acceptWhatever
                        // just instants ago, (which I am now processing the
                        // reply for.)
                        // Therefore I'm just ASSUMING here that it loads
                        // successfully here, since it worked an instant
                        // ago. Todo.
                        OT_ASSERT_MSG(bInbox,
                                      "Was trying to load / verify Inbox.");
                        bool bLoadedRecordBox = false;
                        bool bRecordBoxExists = OTDB::Exists(
                            OTFolders::RecordBox().Get(), strNotaryID.Get(),
                            theReply.m_strAcctID.Get());
                        // Next, loop through the reply items for each
                        // "process inbox" item that I must have previously
                        // sent.
                        // For each, if successful, remove from inbox.
                        // For item receipts, if successful, also remove the
                        // appropriate trans#
                        // from my issued list of transaction numbers (like
                        // above.)

                        for (auto& it_bigloop :
                             pReplyTransaction->GetItemList()) {
                            Item* pReplyItem = it_bigloop;
                            OT_ASSERT_MSG(nullptr != pReplyItem,
                                          "OTClient::ProcessServerReply: "
                                          "Pointer should not have been "
                                          "nullptr.");

                            // otErr << " *** TOP OF LOOP of Reply items,
                            // one presumably for each processInbox that I
                            // sent previously.\n";

                            Item::itemType theItemType = Item::error_state;

                            switch (pReplyItem->GetType()) {
                            case Item::atAcceptPending:
                                theItemType = Item::acceptPending;
                                break;
                            case Item::atAcceptCronReceipt:
                                theItemType = Item::acceptCronReceipt;
                                break;
                            case Item::atAcceptItemReceipt:
                                theItemType = Item::acceptItemReceipt;
                                break;

                            case Item::atRejectPending: // turn down the
                                                        // money!
                                theItemType = Item::rejectPending;
                                continue;                    // unused
                            case Item::atDisputeCronReceipt: // dispute a
                                                             // market
                                                             // trade or
                                                             // payment
                                                             // for a
                                // payment plan
                                theItemType = Item::disputeCronReceipt;
                                continue;                    // unused
                            case Item::atDisputeItemReceipt: // dispute a
                                                             // cheque
                                                             // receipt or
                                                             // transfer
                                                             // receipt.
                                theItemType = Item::disputeItemReceipt;
                                continue; // unused

                            case Item::atAcceptFinalReceipt:
                                theItemType = Item::acceptFinalReceipt;
                                break;

                            case Item::atAcceptBasketReceipt:
                                theItemType = Item::acceptBasketReceipt;
                                break;

                            case Item::atDisputeFinalReceipt:
                                theItemType = Item::disputeFinalReceipt;
                                continue; // unused
                            case Item::atDisputeBasketReceipt:
                                theItemType = Item::disputeBasketReceipt;
                                continue; // unused

                            // We don't care about these here.
                            //
                            case Item::atBalanceStatement:
                                theItemType = Item::balanceStatement;
                                continue;
                            case Item::atTransactionStatement:
                                theItemType = Item::transactionStatement;
                                continue;

                            // FYI, on server side, it does not bother to
                            // process an item,
                            // if the balance statement or transaction
                            // statement has not succeeded.
                            //
                            // Thus, if the ITEM ITSELF has succeeded, that
                            // means the balance or
                            // transaction statement MUST have succeeded!
                            // Because server wouldn't have
                            // even bothered to process the item otherwise.
                            //
                            // There still might be some future application
                            // in doing something with these
                            // statements when they come in.

                            default: {
                                const int32_t nReplyItemType =
                                    pReplyItem->GetType();

                                String strTheType;
                                pReplyItem->GetTypeString(strTheType);

                                otErr
                                    << "*** Unexpected reply item type ("
                                    << nReplyItemType
                                    << ") in processInboxResponse, while "
                                       "processing server reply: " << strTheType
                                    << " \n";
                                continue;
                            }
                            } // SWITCH

                            // The below actions are only necessary if
                            // pReplyItem was a SUCCESS.
                            // (Otherwise we skip them...)
                            //
                            String strTempTypeString;
                            pReplyItem->GetTypeString(strTempTypeString);

                            if (Item::acknowledgement !=
                                pReplyItem->GetStatus()) {
                                otWarn << "processInboxResponse reply item "
                                       << strTempTypeString
                                       << ": status == FAILED\n";
                                continue;
                            }
                            // else
                            otWarn << "processInboxResponse reply item "
                                   << strTempTypeString
                                   << ": status == SUCCESS\n";

                            // WTF IS THIS? There could be 3 acceptPendings,
                            // 5 acceptCronReceipts, 3 acceptFinalReceipts,
                            // etc
                            // in a single ProcessInbox transaction.
                            // Therefore this "get by type" will NOT fly in
                            // this case.
                            // (Fixing this now to look it up by ID instead
                            // of type.)
                            //
                            // OTItem * pItem =
                            // pTransaction->GetItem(theItemType);
                            //
                            // Can't do this either:
                            // OTItem * pItem =
                            // pTransaction->GetItemInRefTo(pReplyItem->GetReferenceToNum());
                            //
                            // (pReplyItem->GetReferenceToNum() contains the
                            // processInbox transaction# of pItem, not
                            //  the inbox receipt # that pItem is in
                            // reference to.)
                            //
                            // pTransaction is the processInbox transaction
                            // request that I sent.
                            // (The items within it all share its same
                            // transaction number, but they are IN REFERENCE
                            // TO
                            //  the inbox receipts that they accept/reject.)
                            // pReplyTransaction is the server's reply to
                            // that.
                            // pReplyItem is the current item when iterating
                            // through pReplyTransaction.
                            // pItem is the corresponding REQUEST item from
                            // pTransaction, that pReplyItem is responding
                            // to.
                            //
                            // Therefore: I need to load the original item
                            // from pReplyItem's reference string (it's
                            // bundled in there).
                            // THEN I will get the "in reference to" number
                            // from THAT (which is the inbox Receipt #).
                            // THEN I will use that number to look up the
                            // SAME original item from pTransaction.
                            // The last step isn't technically necessary,
                            // but may be useful for security.
                            //
                            // Sheesh!

                            String strProcessInboxItem;
                            pReplyItem->GetReferenceString(strProcessInboxItem);

                            std::unique_ptr<Item> pProcessInboxItem(
                                Item::CreateItemFromString(
                                    strProcessInboxItem, NOTARY_ID,
                                    pReplyItem->GetReferenceToNum()));

                            // pProcessInboxItem is already a copy of the
                            // correct processInbox item that I need. But
                            // still, it's a copy that the SERVER
                            // sent me. So I'm going to use it to get the
                            // reference number that I need, in order to
                            // look up MY copy of the item.
                            // So pItem is my original request, inside a
                            // processInbox transaction, to accept some
                            // receipt from my inbox.
                            //
                            Item* pItem = (pProcessInboxItem != nullptr)
                                              ? pTransaction->GetItemInRefTo(
                                                    pProcessInboxItem
                                                        ->GetReferenceToNum())
                                              : nullptr;

                            if (nullptr == pItem) {
                                otErr << "Unable to find original item in "
                                         "original processInbox "
                                         "transaction request, based on "
                                         "reply item.\n";
                                continue;
                            }

                            // If this happens, it means the item we found
                            // in our original process inbox transaction,
                            // which matched the
                            // "in reference to" number that we expected
                            // from the copy of that original item we loaded
                            // from within the
                            // pReplyItem that's supposedly responding to
                            // it, does not have the same TYPE that we would
                            // have expected it to
                            // have, based on the intelligence in the above
                            // switch statement.
                            //
                            if (pItem->GetType() !=
                                theItemType) { // (Possible types for pItem:
                                               // acceptItemReceipt,
                                               // acceptPending,
                                               // acceptCronReceipt,
                                               // acceptFinalReceipt,
                                               // acceptBasketReceipt.)
                                otErr << "Wrong original item TYPE, on "
                                         "reply item's copy of original "
                                         "item, than what was expected "
                                         "based on reply item's type.\n";
                                continue;
                            }

                            // Todo here: any other verification of pItem
                            // against pProcessInboxItem, which are
                            // supposedly copies of the same item.

                            // FYI, pItem->GetReferenceToNum() is the ID of
                            // the receipt that's in the inbox.
                            //

                            OTTransaction* pServerTransaction = nullptr;

                            otWarn
                                << "Checking client-side inbox for "
                                   "expected pending or receipt "
                                   "transaction: " << pItem->GetReferenceToNum()
                                << "... \n"; // temp remove

                            switch (pReplyItem->GetType()) {
                            case Item::atAcceptPending: // Server reply to
                                                        // my acceptance
                                                        // of
                            // pending transfer.
                            case Item::atAcceptItemReceipt: // Server
                                                            // reply to my
                                                            // acceptance
                                                            // of
                                // chequeReceipt, voucherReceipt or
                                // transferReceipt.

                                pServerTransaction = theInbox.GetTransaction(
                                    pItem->GetReferenceToNum());
                                break;
                            case Item::atAcceptCronReceipt:
                            case Item::atAcceptFinalReceipt:
                            case Item::atAcceptBasketReceipt:
                                pServerTransaction = theInbox.GetTransaction(
                                    pItem->GetReferenceToNum());
                                break;

                            default: {
                                const int32_t nReplyItemType =
                                    pReplyItem->GetType();

                                String strTheType;
                                pReplyItem->GetTypeString(strTheType);

                                otErr
                                    << "*** Unexpected reply item type ("
                                    << nReplyItemType
                                    << ") in processInboxResponse, while "
                                       "processing server reply: " << strTheType
                                    << "\n";
                                break; // will return just below, where it
                                       // checks pServerTransaction for
                                       // nullptr.
                            }
                            }

                            if (nullptr == pServerTransaction) {
                                otErr << "Unable to find the server's "
                                         "receipt, in my inbox, that my "
                                         "original processInbox's item was "
                                         "referring to.\n";
                                break; // We must've processed this already,
                                       // and it came through again cause a
                                       // copy was in a nymbox notice.
                            }

                            bool bAddToRecordBox = true;

                            switch (pReplyItem->GetType()) // All of these
                                                           // need to remove
                                                           // something from
                                                           // the
                                                           // client-side
                                                           // inbox. (Which
                                                           // happens below
                                                           // this switch.)
                            { // Some also need to remove an issued
                              // transaction number from pNym.
                            case Item::atAcceptPending:

                                break;

                            // In the case of item receipt (not cron receipt
                            // or pending) I need to
                            // remove the issued num from my list of
                            // responsibility. (Since I finally
                            // accepted the receipt and closed it out.)
                            //
                            // (Basically closing out the original transfer
                            // I must have sent, or cheque I must have
                            // written.)
                            case Item::
                                atAcceptItemReceipt: // <==================================================
                            {
                                // What number do I remove here? the user is
                                // accepting a transfer receipt, which
                                // is in reference to the recipient's
                                // acceptPending. THAT item is in reference
                                // to
                                // my original transfer (or contains a
                                // cheque with my original number.) (THAT's
                                // the # I need.)
                                //
                                String strOriginalItem;
                                pServerTransaction->GetReferenceString(
                                    strOriginalItem);

                                std::unique_ptr<Item> pOriginalItem(
                                    Item::CreateItemFromString(
                                        strOriginalItem, NOTARY_ID,
                                        pServerTransaction
                                            ->GetReferenceToNum()));

                                if (nullptr != pOriginalItem) {
                                    // If pOriginalItem is acceptPending,
                                    // that means I am accepting the
                                    // transfer receipt from the server,
                                    // (from my inbox),
                                    // which has the recipient's acceptance
                                    // inside of my transfer as the original
                                    // item. This means the transfer that
                                    // I originally sent is now finally
                                    // closed!
                                    //
                                    // If it's a depositCheque, that means I
                                    // am accepting the cheque receipt from
                                    // the server, (from my inbox)
                                    // which has the recipient's deposit
                                    // inside of it as the original item.
                                    // This means that the cheque that
                                    // I originally wrote is now finally
                                    // closed!
                                    //
                                    // In both cases, the "original item"
                                    // itself is not from me, but from the
                                    // recipient! Therefore,
                                    // the number on that item is useless
                                    // for removing numbers from my list of
                                    // issued numbers.
                                    // Rather, I need to load that original
                                    // cheque, or pending transfer, from
                                    // WITHIN the original item,
                                    // in order to get THAT number, to
                                    // remove it from my issued list.
                                    //
                                    if (Item::depositCheque ==
                                        pOriginalItem->GetType()) // I am
                                                                  // accepting a
                                    // CHEQUE RECEIPT,
                                    // which has a
                                    // depositCheque
                                    // request (from
                                    // the recipient)
                                    // as the original
                                    // item within.
                                    {
                                        // Get the cheque from the Item and
                                        // load it up into a Cheque object.
                                        String strCheque;
                                        pOriginalItem->GetAttachment(strCheque);

                                        Cheque theCheque; // allocated on
                                                          // the stack :-)

                                        if (false ==
                                            ((strCheque.GetLength() > 2) &&
                                             theCheque.LoadContractFromString(
                                                 strCheque))) {
                                            otErr << "ERROR loading cheque "
                                                     "from string in "
                                                     "OTClient::"
                                                     "ProcessServerReply:\n"
                                                  << strCheque << "\n";
                                        }
                                        else // Since I wrote the cheque,
                                               // and I am now accepting the
                                               // cheque receipt, I can now
                                               // be cleared
                                               // for that issued number.
                                               // (Because the server reply
                                               // said SUCCESS accepting the
                                        // chequeReceipt/voucherReceipt.)
                                        {
                                            pNym->RemoveIssuedNum(
                                                *pNym, strNotaryID,
                                                theCheque.GetTransactionNum(),
                                                true); // bool bSave=true
                                            /* Inside OT, when processing
                                            successful server reply to
                                            processInbox request, if a
                                            chequeReceipt
                                            was processed out successfully
                                            (here: YES), and if that cheque
                                            is found inside the outpayments,
                                            then move it at that time to the
                                            record box. */

                                            int32_t lOutpaymentsIndex =
                                                GetOutpaymentsIndexByTransNum(
                                                    *pNym,
                                                    theCheque
                                                        .GetTransactionNum());

                                            if (lOutpaymentsIndex >
                                                (-1)) // found something
                                                      // that matches...
                                            {
                                                // Remove it from
                                                // Outpayments box. We're
                                                // done with it -- we
                                                // accepted the
                                                // chequeReceipt now.
                                                // (Dump it in records for
                                                // your app, but OT itself
                                                // is done with it.)
                                                //
                                                if (pNym->RemoveOutpaymentsByIndex(
                                                        lOutpaymentsIndex)) {
                                                    if (!pNym->SaveSignedNymfile(
                                                            *pNym)) // <==
                                                                    // save
                                                                    // Nym
                                                                    // to
                                                        // local
                                                        // storage,
                                                        // since
                                                        // an
                                                        // outpayment
                                                        // was
                                                        // erased.
                                                        otErr << __FUNCTION__
                                                              << ": Error "
                                                                 "saving "
                                                                 "Nym: "
                                                              << strNymID
                                                              << "\n";
                                                }
                                            }
                                        }
                                    }
                                    // I am accepting a TRANSFER RECEIPT,
                                    // which has an acceptPending inside
                                    // FROM THE RECIPIENT,
                                    // as the original item within, (which
                                    // is in reference to my outoing
                                    // original transfer.)
                                    else if (Item::acceptPending ==
                                             pOriginalItem->GetType()) {
                                        pNym->RemoveIssuedNum(
                                            *pNym, strNotaryID,
                                            pOriginalItem->GetNumberOfOrigin(),
                                            true); // bool bSave=true
                                    }
                                    else {
                                        String strOriginalItemType;
                                        pOriginalItem->GetTypeString(
                                            strOriginalItemType);
                                        otErr << "OTClient::"
                                                 "ProcessServerReply: "
                                                 "Original item has wrong "
                                                 "type, while accepting "
                                                 "item receipt:\n"
                                              << strOriginalItemType << "\n";
                                    }
                                }
                                else {
                                    otErr << "OTClient::ProcessServerReply:"
                                             " Unable to load original "
                                             "item from string while "
                                             "accepting item receipt:\n"
                                          << strOriginalItem << "\n";
                                }
                            } // OTItem::atAcceptItemReceipt.
                            break;

                            // Cron Receipt: We do not remove the original
                            // trans# until the Cron job is entirely
                            // complete. (Many Cron receipts may breeze
                            // through here before that happens.)
                            //
                            case Item::atAcceptCronReceipt: {
                                // If it's a CRON receipt, find out if it's
                                // from a MARKET TRADE, and if so,
                                // add it to my local list of Market Trades,
                                // for the GUI to use on the market panel.
                                //
                                // Todo security: add the actual sale price
                                // to boths receipts, along with both
                                // amounts,
                                // in order to verify the amount moved is in
                                // keeping with the terms of the original
                                // offer.
                                //
                                Item* pServerItem = pServerTransaction->GetItem(
                                    Item::marketReceipt); // paymentPlan
                                                          // and
                                                          // smartContract
                                                          // are also
                                                          // POSSIBLE
                                                          // here.

                                if (nullptr != pServerItem) {
                                    String strOffer, strTrade;
                                    pServerItem->GetAttachment(
                                        strOffer); // contains updated
                                                   // offer.
                                    pServerItem->GetNote(
                                        strTrade); // contains updated
                                                   // trade.

                                    OTOffer theOffer;
                                    OTTrade theTrade;

                                    bool bLoadOfferFromString =
                                        theOffer.LoadContractFromString(
                                            strOffer);
                                    bool bLoadTradeFromString =
                                        theTrade.LoadContractFromString(
                                            strTrade);
                                    if (bLoadOfferFromString &&
                                        bLoadTradeFromString) {
                                        std::unique_ptr<OTDB::TradeDataNym>
                                        pData(dynamic_cast<OTDB::TradeDataNym*>(
                                            OTDB::CreateObject(
                                                OTDB::
                                                    STORED_OBJ_TRADE_DATA_NYM)));
                                        OT_ASSERT(nullptr != pData);

                                        /*
                                        std::stringstream ss;
                                        ss << theTrade.GetTransactionNum();
                                        pData->transaction_id = ss.str();
                                        ss.str(""); */
                                        pData->transaction_id = to_string<
                                            int64_t>(
                                            theTrade
                                                .GetTransactionNum()); // TransID
                                        // for
                                        // original
                                        // offer.
                                        // (Offer
                                        // may
                                        // trade
                                        // many
                                        // times.)
                                        pData->updated_id = to_string<int64_t>(
                                            pServerItem
                                                ->GetTransactionNum()); // TransID
                                                                        // for
                                                                        // BOTH
                                        // receipts
                                        // for
                                        // current
                                        // trade.
                                        // (Asset/Currency.)
                                        pData->completed_count =
                                            to_string<int32_t>(
                                                theTrade.GetCompletedCount());
                                        std::unique_ptr<Account> pAccount(
                                            Account::LoadExistingAccount(
                                                ACCOUNT_ID, NOTARY_ID));

                                        bool bIsAsset =
                                            (theTrade
                                                 .GetInstrumentDefinitionID() ==
                                             pAccount
                                                 ->GetInstrumentDefinitionID());
                                        bool bIsCurrency =
                                            (theTrade.GetCurrencyID() ==
                                             pAccount
                                                 ->GetInstrumentDefinitionID());

                                        if (bIsAsset) {
                                            //                                                  pServerItem->GetAmount()
                                            // contains:  (lAmountSold); //
                                            // asset

                                            const String strInstrumentDefinitionID(
                                                theTrade
                                                    .GetInstrumentDefinitionID());
                                            int64_t lAssetsThisTrade =
                                                pServerItem->GetAmount();
                                            pData->instrument_definition_id =
                                                strInstrumentDefinitionID.Get();
                                            pData->amount_sold =
                                                to_string<int64_t>(
                                                    lAssetsThisTrade); // The
                                                                       // amount
                                                                       // of
                                                                       // ASSETS
                                                                       // moved,
                                                                       // this
                                                                       // trade.
                                        }
                                        else if (bIsCurrency) {
                                            //                                                  pServerItem->GetAmount()
                                            // contains:  (lTotalPaidOut);
                                            // // currency

                                            const String strCurrencyID(
                                                theTrade.GetCurrencyID());
                                            int64_t lCurrencyThisTrade =
                                                pServerItem->GetAmount();
                                            pData->currency_id =
                                                strCurrencyID.Get();
                                            pData->currency_paid =
                                                to_string<int64_t>(
                                                    lCurrencyThisTrade);
                                        }
                                        const time64_t& tProcessDate =
                                            theTrade.GetLastProcessDate();
                                        pData->date =
                                            to_string<time64_t>(tProcessDate);
                                        // The original offer price. (Might
                                        // be 0, if it's a market order.)
                                        //
                                        const int64_t& lPriceLimit =
                                            theOffer.GetPriceLimit();
                                        pData->offer_price =
                                            to_string<int64_t>(lPriceLimit);
                                        const int64_t& lFinishedSoFar =
                                            theOffer.GetFinishedSoFar();
                                        pData->finished_so_far =
                                            to_string<int64_t>(lFinishedSoFar);
                                        // save to local storage...
                                        //
                                        String strNymID(NYM_ID);

                                        std::unique_ptr<OTDB::TradeListNym>
                                            pList;

                                        if (OTDB::Exists(OTFolders::Nym().Get(),
                                                         "trades", // todo stop
                                                         // hardcoding.
                                                         strNotaryID.Get(),
                                                         strNymID.Get()))
                                            pList.reset(dynamic_cast<
                                                OTDB::TradeListNym*>(
                                                OTDB::QueryObject(
                                                    OTDB::
                                                        STORED_OBJ_TRADE_LIST_NYM,
                                                    OTFolders::Nym().Get(),
                                                    "trades", // todo stop
                                                              // hardcoding.
                                                    strNotaryID.Get(),
                                                    strNymID.Get())));
                                        if (nullptr == pList) {
                                            otInfo << "Creating storage list "
                                                      "of trade receipts for "
                                                      "Nym: " << strNymID
                                                   << "\n";
                                            pList.reset(dynamic_cast<
                                                OTDB::
                                                    TradeListNym*>(OTDB::CreateObject(
                                                OTDB::
                                                    STORED_OBJ_TRADE_LIST_NYM)));
                                        }
                                        OT_ASSERT(nullptr != pList);
                                        // Loop through and see if we can
                                        // find one that's ALREADY there.
                                        // We can match the asset receipt
                                        // and currency receipt.
                                        // This way we insure there is only
                                        // one in the end, which combines
                                        // info from both.
                                        // This also enables us to calculate
                                        // the sale price!
                                        //
                                        bool bWeFoundIt = false;

                                        size_t nTradeDataNymCount =
                                            pList->GetTradeDataNymCount();

                                        for (size_t nym_count = 0;
                                             nym_count < nTradeDataNymCount;
                                             ++nym_count) {
                                            OTDB::TradeDataNym* pTradeData =
                                                pList->GetTradeDataNym(
                                                    nym_count);

                                            if (nullptr ==
                                                pTradeData) // Should never
                                                            // happen.
                                                continue;
                                            if (0 ==
                                                pTradeData->updated_id.compare(
                                                    pData->updated_id)) // Found
                                            // it!
                                            {
                                                // It's a repeat of the same
                                                // one. (Discard.)
                                                if ((!pTradeData
                                                          ->instrument_definition_id
                                                          .empty() &&
                                                     !pData
                                                          ->instrument_definition_id
                                                          .empty()) ||
                                                    (!pTradeData->currency_id
                                                          .empty() &&
                                                     !pData->currency_id
                                                          .empty()))
                                                    break;
                                                // Okay looks like one is
                                                // the asset receipt, and
                                                // the other is the currency
                                                // receipt.
                                                // Therefore let's combine
                                                // them into pTradeData!
                                                //
                                                if (pTradeData
                                                        ->instrument_definition_id
                                                        .empty()) {
                                                    pTradeData
                                                        ->instrument_definition_id =
                                                        pData
                                                            ->instrument_definition_id;
                                                    pTradeData->amount_sold =
                                                        pData->amount_sold;
                                                }
                                                else if (pTradeData
                                                               ->currency_id
                                                               .empty()) {
                                                    pTradeData->currency_id =
                                                        pData->currency_id;
                                                    pTradeData->currency_paid =
                                                        pData->currency_paid;
                                                }
                                                if (!pTradeData->amount_sold
                                                         .empty() &&
                                                    !pTradeData->currency_paid
                                                         .empty()) {
                                                    const int64_t lAmountSold =
                                                        String::StringToLong(
                                                            pTradeData
                                                                ->amount_sold);
                                                    const int64_t lCurrencyPaid =
                                                        String::StringToLong(
                                                            pTradeData
                                                                ->currency_paid);

                                                    if (lAmountSold !=
                                                        0) // just in case
                                                           // (divide by 0.)
                                                    {
                                                        const int64_t
                                                            lSalePrice =
                                                                (lCurrencyPaid /
                                                                 lAmountSold);

                                                        String strSalePrice;
                                                        strSalePrice.Format(
                                                            "%" PRId64 "",
                                                            lSalePrice);

                                                        pTradeData->price =
                                                            strSalePrice.Get();
                                                    }
                                                }

                                                bWeFoundIt = true;

                                                break;

                                            }            // if we found it.
                                        }                // for
                                        if (!bWeFoundIt) // We didn't find
                                                         // it. So let's add
                                                         // it.
                                        {
                                            pList->AddTradeDataNym(*pData);
                                        }
                                        if (false == OTDB::StoreObject(
                                                         *pList,
                                                         OTFolders::Nym().Get(),
                                                         "trades", // todo stop
                                                         // hardcoding.
                                                         strNotaryID.Get(),
                                                         strNymID.Get()))
                                            otErr
                                                << "OTClient::" << __FUNCTION__
                                                << ": Failed storing list "
                                                   "of trades for Nym. "
                                                   "Notary ID: " << strNotaryID
                                                << " Nym ID: " << strNymID
                                                << " \n";
                                    }
                                }
                                // else
                                //    otErr <<
                                // "OTClient::ProcessServerReply:
                                // "
                                //                 "Expected marketReceipt
                                // item in transaction in inbox.");
                            } // OTItem::atAcceptCronReceipt
                            break;

                            case Item::atAcceptFinalReceipt: {
                                otWarn << "OTClient::ProcessServerReply: "
                                          "Successfully removed finalReceipt "
                                          "with closing num: "
                                       << pServerTransaction->GetClosingNum()
                                       << "\n";
                                pNym->RemoveIssuedNum(
                                    *pNym, strNotaryID,
                                    pServerTransaction->GetClosingNum(),
                                    true); // bool bSave=true

                                // This should have already been done by
                                // this point, but I'm putting it here just
                                // in case,
                                // while debugging:
                                //
                                if (pNym->RemoveIssuedNum(
                                        *pNym, strNotaryID,
                                        pServerTransaction->GetReferenceToNum(),
                                        true)) // bool bSave=true
                                    otWarn << "**** Due to finding a "
                                              "finalReceipt, REMOVING "
                                              "OPENING NUMBER FROM NYM:  "
                                           << pServerTransaction
                                                  ->GetReferenceToNum()
                                           << " \n";
                                else
                                    otWarn << "**** Noticed a finalReceipt, "
                                              "but Opening Number "
                                           << pServerTransaction
                                                  ->GetReferenceToNum()
                                           << " had ALREADY been removed from "
                                              "nym. \n";
                                // The client side keeps a list of active
                                // (recurring) transactions.
                                // That is, smart contracts and payment
                                // plans. I don't think it keeps
                                // market offers in that list, since we
                                // already have a list of active
                                // market offers separately. And market
                                // offers produce final receipts,
                                // so basically this piece of code will be
                                // executed for all final receipts.
                                // It's not really necessary that it be
                                // called for market offers, but whatever.
                                // It is for the others.
                                //
                                OTCronItem::EraseActiveCronReceipt(
                                    pServerTransaction->GetReferenceToNum(),
                                    pNym->GetConstID(),
                                    pServerTransaction->GetPurportedNotaryID());
                            } // OTItem::atAcceptFinalReceipt
                            break;

                            case Item::atAcceptBasketReceipt: {
                                otInfo << "OTClient::ProcessServerReply: "
                                          "Successfully removed basketReceipt "
                                          "with closing num: "
                                       << pServerTransaction->GetClosingNum()
                                       << "\n";
                                pNym->RemoveIssuedNum(
                                    *pNym, strNotaryID,
                                    pServerTransaction->GetClosingNum(),
                                    true); // bool bSave=true
                            }              // OTItem::atAcceptBasketReceipt
                            break;

                            default: // Error
                            {
                                bAddToRecordBox = false;
                                pReplyItem->GetTypeString(strTempTypeString);
                                otErr << "OTClient::ProcessServerReply: "
                                         "wrong reply item transaction "
                                         "type: " << strTempTypeString << "\n";
                            } break;
                            } // switch replyItem type
                            if (bAddToRecordBox) {
                                if (!bLoadedRecordBox) // We haven't loaded
                                                       // / created it yet.
                                {
                                    bLoadedRecordBox =
                                        (bRecordBoxExists &&
                                         theRecordBox.LoadRecordBox());
                                    if (bRecordBoxExists && bLoadedRecordBox)
                                        bLoadedRecordBox =
                                            (theRecordBox.VerifyContractID() &&
                                             theRecordBox.VerifySignature(
                                                 *pNym));
                                    //                                          bLoadedRecordBox
                                    // =
                                    // (theRecordBox.VerifyAccount(*pNym));
                                    // // (No need to load all the Box
                                    // Receipts using VerifyAccount)
                                    else if (!bLoadedRecordBox)
                                        bLoadedRecordBox =
                                            theRecordBox.GenerateLedger(
                                                ACCOUNT_ID, NOTARY_ID,
                                                Ledger::recordBox,
                                                true); // bGenerateFile=true
                                    // by this point, the box DEFINITELY
                                    // exists -- or not. (generation might
                                    // have failed, or verification.)
                                    //
                                    if (!bLoadedRecordBox) {
                                        otOut << __FUNCTION__
                                              << ": while processing server "
                                                 "reply to processInbox: "
                                                 "WARNING: Unable to load, "
                                                 "verify, or generate "
                                                 "recordBox, with IDs: "
                                              << strNymID << " / "
                                              << theReply.m_strAcctID << "\n";
                                    }
                                }
                                if (bLoadedRecordBox) {
                                    const String strServerTransaction(
                                        *pServerTransaction);
                                    OTTransaction* pNewTransaction = nullptr;
                                    std::unique_ptr<OTTransactionType>
                                        pTransType(
                                            OTTransactionType::
                                                TransactionFactory(
                                                    strServerTransaction));

                                    pNewTransaction =
                                        dynamic_cast<OTTransaction*>(
                                            pTransType.get());
                                    if (nullptr != pNewTransaction) {
                                        const bool bAdded =
                                            theRecordBox.AddTransaction(
                                                *pNewTransaction);

                                        if (!bAdded) {
                                            otErr << __FUNCTION__
                                                  << ": Unable to add "
                                                     "transaction "
                                                  << pNewTransaction
                                                         ->GetTransactionNum()
                                                  << " to record box (still "
                                                     "removing it from asset "
                                                     "account inbox, "
                                                     "however.)\n";
                                        }
                                        else // Success adding it to the
                                               // record box (let's save
                                               // it.)
                                        {
                                            // If successfully added to the
                                            // record box, then no need
                                            // anymore to clean it up
                                            // ourselves. The record box
                                            // owns it now.
                                            pTransType.release();

                                            theRecordBox.ReleaseSignatures();
                                            theRecordBox.SignContract(*pNym);
                                            theRecordBox.SaveContract();
                                            theRecordBox
                                                .SaveRecordBox(); // todo
                                                                  // log
                                            // failure.

                                            // Any inbox/nymbox/outbox
                                            // ledger will only itself
                                            // contain
                                            // abbreviated versions of the
                                            // receipts, including their
                                            // hashes.
                                            //
                                            // The rest is stored
                                            // separately, in the box
                                            // receipt, which is created
                                            // whenever a receipt is added
                                            // to a box (here), and deleted
                                            // after a receipt
                                            // is removed from a box.
                                            //
                                            if (!pNewTransaction->SaveBoxReceipt(
                                                    theRecordBox)) // <===================
                                                otErr << __FUNCTION__
                                                      << ": for Record "
                                                         "Box... Failed "
                                                         "trying to "
                                                         "SaveBoxReceipt. "
                                                         "Contents:\n\n"
                                                      << strServerTransaction
                                                      << "\n\n";
                                        }
                                    } // if (nullptr != pNewTransaction)
                                }     // if (bLoadedRecordBox)
                            }         // if (bAddToRecordBox)
                            // REMOVE IT FROM THE INBOX.
                            //
                            // This removal happens for ALL of the above
                            // cases.
                            // Update: Now when removing receipts from any
                            // box, we have to
                            // also delete the box receipt, which is stored
                            // as a separate file.
                            //
                            pServerTransaction->DeleteBoxReceipt(
                                theInbox); // faster
                            //                                theInbox.DeleteBoxReceipt(pServerTransaction->GetTransactionNum());
                            theInbox.RemoveTransaction(
                                pServerTransaction->GetTransactionNum());

                        } // for loop (reply items)
                        // Save the Inbox
                        //
                        theInbox.ReleaseSignatures();
                        theInbox.SignContract(*pNym);
                        theInbox.SaveContract();
                        theInbox.SaveInbox();
                    } // if pReplyTransaction
                }     // if pTransaction
            }
            //
            else // processNymboxResponse.  // We're processing the SERVER's
                 // REPLY
                 // to our processNymbox request.
            {
                pTransaction =
                    theLedger.GetTransaction(OTTransaction::processNymbox);
                pReplyTransaction = theReplyLedger.GetTransaction(
                    OTTransaction::atProcessNymbox);

                // If I have already processed this reply,

                // We did NOT have to burn a transaction number to process
                // the Nymbox, so we don't
                // have to remove it from the list of responsibility, like
                // we do above.
                // The reason is because the Nymbox cannot be used for
                // financial transactions, since
                // it is associated with a user acct (instead of asset
                // account.)
                // THIS IS ACTUALLY the WHOLE POINT of the Nymbox: If it
                // required a transaction number
                // to process the Nymbox, and you use the Nymbox to get
                // transaction numbers, then how
                // can you ever get a new number if you run out?  You need a
                // number to get a number?
                //
                // That makes no logical sense.  Therefore, the Nymbox
                // provides a way to get new transaction
                // numbers WITHOUT HAVING TO BURN ONE TO DO IT.  You still
                // have to do a transaction statement
                // to do it (sign off on the ones that you actually do
                // have), but you can still process
                // the Nymbox even if you have zero transaction numbers,
                // whereas with the inbox for an asset
                // account, you cannot process it until you burn a
                // transaction number to do so. And if you
                // don't have any transaction numbers to do that with,
                // that's fine: you just get a new one
                // via your nymbox.  This is the original reason that I
                // added nymboxes in the first place.
                //
                // SIMILARLY, when a transaction number is REMOVED from our
                // list via the Nymbox, it's only
                // a NOTIFICATION. The Nymbox cannot actually REMOVE your
                // transaction numbers, but it CAN
                // be used to drop a notice informing you that one was
                // removed. (Usually by a recurring
                // transaction, such as a market offer, where you had
                // already provided the closing number in
                // advance, and you expected that it could be closed at
                // anytime.)
                //
                //
                if ((nullptr != pTransaction) &&
                    (nullptr != pReplyTransaction)) {
                    // HARVEST TRANSACTION NUMBERS (Nymbox only)
                    //
                    Item* pStatementItem =
                        pTransaction->GetItem(Item::transactionStatement);

                    // We found it!
                    if (nullptr == pStatementItem) {
                        otOut << "Strange... found transaction in ledger in "
                              << theReply.m_strCommand
                              << ", but didn't find a transactionStatement "
                                 "item within.\n";
                    }
                    else if (!pReplyTransaction->GetSuccess()) {
                        otOut << "Found the receipt you're talking about, "
                                 "in ledger in " << theReply.m_strCommand
                              << ", but the Server's Reply transaction "
                                 "says FAILED.\n";
                    }
                    else {
                        String strMessageNym;
                        Nym theMessageNym;

                        pStatementItem->GetAttachment(strMessageNym);

                        if (strMessageNym.Exists() &&
                            theMessageNym.LoadFromString(strMessageNym)) {
                            // Success!
                            // Whatever Trans#'s I accepted when I processed
                            // my nymbox, I now
                            // harvest them onto my Nym for use. (Couldn't
                            // be sure until server replied "success".)
                            //
                            // Contrast this with the numbers removed. In
                            // the case of Nymbox, I cannot
                            // remove numbers, only receive notice that a
                            // number was already removed.
                            // Therefore, I might as well remove it on my
                            // side also, as soon as I see that
                            // notice (and approve of it.) There's no need
                            // juggling it in that case -- it's
                            // already gone. (Therefore it's already been
                            // done by the time we're in this
                            // function reading the server's reply. Removals
                            // for Nymbox happen in Finalize for
                            // processNymbox, and in AcceptEntireNymbox.)
                            // Below however, are additions, not removals,
                            // so we don't add them until the
                            // server has DEFINITELY responded in the
                            // affirmative (here):
                            //
                            pNym->HarvestTransactionNumbers(
                                pStatementItem->GetPurportedNotaryID(), *pNym,
                                theMessageNym, true); // bSave=true
                            // New version now takes tentative numbers into
                            // account, to reduce sync issues.
                            //                                pNym->HarvestIssuedNumbers(pStatementItem->GetPurportedNotaryID(),
                            //                                                           *pNym,
                            // theMessageNym, true); // bSave=true
                        }
                        else {
                            otOut << "Strange... found transaction item in "
                                     "ledger in " << theReply.m_strCommand
                                  << ", but didn't find theMessageNym "
                                     "within.\n";
                        }
                    }

                    //
                    // REMOVE VARIOUS ITEMS FROM THE LOCAL NYMBOX (THEIR
                    // TIME IS DONE.)

                    // Load the Nymbox.
                    Ledger theNymbox(NYM_ID, NYM_ID, NOTARY_ID);
                    bool bLoadedNymbox = false;
                    if (nullptr != pNymbox) // If a pointer was passed in, then
                                            // we'll just use it.
                    {
                        bLoadedNymbox = true;
                    }
                    else // Otherwise, we have to load it ourselves. (And
                           // point the pointer to it.)
                    {
                        pNymbox = &theNymbox;
                        bLoadedNymbox = (pNymbox->LoadNymbox() &&
                                         pNymbox->VerifyAccount(*pNym));
                    }
                    // I JUST had this loaded if I sent acceptWhatever just
                    // instants ago, (which I am now processing the reply
                    // for.)
                    // Therefore I'm just ASSUMING here that it loads
                    // successfully here, since it worked an instant ago.
                    // Todo.
                    //
                    OT_ASSERT_MSG(bLoadedNymbox, "Was trying to load Nymbox.");
                    // Next, loop through the reply items for each "process
                    // nymbox" item that I must have previously sent.
                    // For each, if successful, remove from inbox.
                    // For item receipts, if successful, also remove the
                    // appropriate trans#
                    // from my issued list of transaction numbers (like
                    // above.)
                    //
                    for (auto& it : pReplyTransaction->GetItemList()) {
                        Item* pReplyItem = it;
                        OT_ASSERT_MSG(nullptr != pReplyItem,
                                      "OTClient::ProcessServerReply: "
                                      "Pointer should not have been "
                                      "nullptr.");

                        Item::itemType theItemType = Item::error_state;

                        switch (pReplyItem->GetType()) {
                        case Item::atAcceptFinalReceipt: // for inbox this
                                                         // is a closing
                                                         // issued number
                            // being removed from your list.
                            theItemType = Item::acceptFinalReceipt; // but for
                            // Nymbox, this
                            // is only a
                            // notification
                            // that it
                            // already
                            // happened
                            // previously.
                            break;
                        case Item::atAcceptMessage:
                            theItemType = Item::acceptMessage;
                            break;
                        case Item::atAcceptNotice:
                            theItemType = Item::acceptNotice;
                            break;
                        case Item::atAcceptTransaction:
                            theItemType = Item::acceptTransaction;
                            break;
                        // FYI, on server side, it does not bother to
                        // process an item,
                        // if the balance statement or transaction statement
                        // has not succeeded.
                        //
                        // Thus, if the ITEM ITSELF has succeeded, that
                        // means the balance or
                        // transaction statement MUST have succeeded!
                        // Because server wouldn't have
                        // even bothered to process the item otherwise.
                        //
                        // There still might be some future application in
                        // doing something with these
                        // statements when they come in.
                        case Item::atTransactionStatement:
                            theItemType = Item::transactionStatement; // We just
                            // continue;
                            // when this
                            // happens,
                            // and skip
                            // this one.
                            continue; // (The transaction statement itself
                                      // is already handled before this
                                      // "for" loop.)

                        default: {
                            String strTempTypeString;
                            pReplyItem->GetTypeString(strTempTypeString);
                            otErr << __FUNCTION__
                                  << ": Unexpected replyItem:type while "
                                     "processing Nymbox: " << strTempTypeString
                                  << " \n";
                            continue;
                        }
                        } // SWITCH
                        // The below actions are only necessary if
                        // pReplyItem was a SUCCESS.
                        // (Otherwise we skip them...)
                        //
                        String strTempTypeString;
                        pReplyItem->GetTypeString(strTempTypeString);

                        if (Item::acknowledgement != pReplyItem->GetStatus()) {
                            otWarn << "processNymboxResponse reply item "
                                   << strTempTypeString
                                   << ": status == FAILED\n";
                            continue;
                        }
                        // else
                        otWarn << "processNymboxResponse reply item "
                               << strTempTypeString << ": status == SUCCESS\n";

                        // pReplyItem->GetReferenceToNum() contains the
                        // process transaction# of pItem (0, in
                        // a transaction statement, since it usually has no
                        // transaction number of its own), not
                        // the inbox receipt # that pItem is in reference
                        // to.
                        //
                        // pTransaction is the processNymbox transaction
                        // request that I sent.
                        // (The items within it all share its same
                        // transaction number, but they are IN REFERENCE TO
                        //  the Nymbox receipts that they accept/reject.)
                        // pReplyTransaction is the server's reply to that.
                        // pReplyItem is the current item when iterating
                        // through pReplyTransaction.
                        // pItem is the corresponding REQUEST item from
                        // pTransaction, that pReplyItem is responding to.
                        //
                        // Therefore: I need to load the original item from
                        // pReplyItem's reference string (it's bundled in
                        // there).
                        // THEN I will get the "in reference to" number from
                        // THAT (which is the nymbox Receipt #).
                        // THEN I will use that number to look up the SAME
                        // original item from pTransaction.
                        // The last step isn't technically necessary, but
                        // may be useful for security.
                        //
                        // Sheesh!

                        String strProcessNymboxItem;
                        pReplyItem->GetReferenceString(strProcessNymboxItem);

                        std::unique_ptr<Item> pProcessNymboxItem(
                            Item::CreateItemFromString(
                                strProcessNymboxItem, NOTARY_ID,
                                0 /* 0 is the "transaction number"*/)); // todo
                        // stop
                        // hardcoding.

                        // pProcessNymboxItem is already a copy of the
                        // correct processNymbox item that I need.
                        // But still, it's a copy that the SERVER sent me.
                        // So I'm going to use it to get the
                        // reference number that I need, in order to look up
                        // MY copy of the item.
                        //
                        Item* pItem =
                            (pProcessNymboxItem != nullptr)
                                ? pTransaction->GetItemInRefTo(
                                      pProcessNymboxItem->GetReferenceToNum())
                                : nullptr;

                        if (nullptr == pItem) {
                            otErr << __FUNCTION__
                                  << ": Unable to find original item in "
                                     "original processNymbox transaction "
                                     "request, based on reply item.\n";
                            continue;
                        }
                        // If this happens, it means the item we found in
                        // our original process Nymbox transaction, which
                        // matched the
                        // "in reference to" number that we expected from
                        // the copy of that original item we loaded from
                        // within the
                        // pReplyItem that's supposedly responding to it,
                        // does not have the same TYPE that we would have
                        // expected it to
                        // have, based on the intelligence in the above
                        // switch statement.
                        //
                        if (pItem->GetType() !=
                            theItemType) { // (Possible types for pItem:
                                           // acceptMessage, acceptNotice,
                                           // acceptTransactions,
                                           // acceptFinalReceipt.)
                            otErr << __FUNCTION__
                                  << ": Wrong original item TYPE, on reply "
                                     "item's copy of original item, than "
                                     "what was expected based on reply "
                                     "item's type.\n";
                            continue;
                        }

                        // Todo here: any other verification of pItem
                        // against pProcessNymboxItem, which are supposedly
                        // copies of the same item.
                        // (Potentially todo security.)

                        // FYI, pItem->GetReferenceToNum() is the ID of the
                        // receipt that's in the Nymbox.
                        //
                        OTTransaction* pServerTransaction = nullptr;

                        otWarn << __FUNCTION__
                               << ": Checking client-side Nymbox for "
                                  "expected Nymbox item: "
                               << pItem->GetReferenceToNum()
                               << "... \n"; // temp remove

                        switch (pReplyItem->GetType()) {
                        case Item::atAcceptNotice:
                        case Item::atAcceptMessage:
                        case Item::atAcceptTransaction:
                        case Item::atAcceptFinalReceipt:
                            pServerTransaction = pNymbox->GetTransaction(
                                pItem->GetReferenceToNum());
                            break;

                        default: {
                            String strTempTypeString;
                            pReplyItem->GetTypeString(strTempTypeString);
                            otErr << __FUNCTION__
                                  << ": Unexpected replyItem::type while "
                                     "processing Nymbox: " << strTempTypeString
                                  << " \n";
                            break;
                        }
                        }
                        if (nullptr == pServerTransaction) {
                            otWarn << __FUNCTION__
                                   << ": The original processNymbox item "
                                      "referred to trans number "
                                   << pItem->GetReferenceToNum()
                                   << ", but that receipt wasn't in my "
                                      "Nymbox. (We probably processed this "
                                      "server reply ALREADY, and now we're "
                                      "just seeing it again, since an "
                                      "extra copy was dropped into the "
                                      "Nymbox originally. It happens. "
                                      "Skipping.)";
                            break; // We must have processed this reply
                                   // already, and it just came through
                                   // again cause a copy was in a nymbox
                                   // notice.
                        }
                        // All of these need to remove something from the
                        // client-side Nymbox. (Which happens below this
                        // switch.)
                        //
                        switch (pReplyItem->GetType()) { // Some also need to
                                                         // remove an issued
                                                         // transaction number
                                                         // from pNym.

                        case Item::atAcceptNotice:

                            // There are many different types of notices.
                            // We just indiscriminately accept them all from
                            // the Nymbox.
                            // The replyNotice tells you that a transaction
                            // was processed.
                            // (We put a copy of the server reply into your
                            // Nymbox, to make sure
                            // you get it, so you stay in sync with which
                            // transaction numbers are signed out.)
                            // The successNotice tells you that you
                            // successfully signed out new transaction
                            // numbers
                            // (to use on transactions.)
                            // The "plain-ole" OTTransaction::notice is used
                            // to notice the parties to a smart
                            // contract that it has activated (or failed to
                            // activate.)

                            // if pReplyItem is atAcceptNotice, then pItem
                            // is acceptNotice.
                            // Then pItem is accepting (IN REFERENCE TO) the
                            // original OTItem::notice that's
                            // sitting in the Nymbox!

                            if (OTTransaction::notice ==
                                pServerTransaction->GetType()) {

                                if ((Item::rejection ==
                                     pReplyItem->GetStatus()) || // REJECTION
                                    (Item::acknowledgement ==
                                     pReplyItem->GetStatus())) // ACKNOWLEDGMENT
                                {
                                    // NOTE: NORMALLY we do this sort of
                                    // thing in the server reply to the
                                    // actual
                                    // transaction request (by the
                                    // activating party.)
                                    //
                                    // For example, if you tried to activate
                                    // a smart contract, and that failed,
                                    // then
                                    // the atSmartContract server reply will
                                    // be processed, and the opening issued#
                                    // will
                                    // be removed at that time, and the
                                    // closing numbers will be harvested. So
                                    // then, why
                                    // this additional notice in my Nymbox?
                                    // If that will already happen?
                                    //
                                    // ===> Because of ALL THE OTHER PARTIES
                                    // to the smart contract! (This may be
                                    // necessary
                                    // for payment plans, too.) The
                                    // activating party got his reply (he
                                    // even had a back-up
                                    // reply stuffed into his Nymbox to make
                                    // SURE he got it.) But all the other
                                    // parties will
                                    // only know, if they are sent a notice!
                                    // Therefore a notice is sent by the
                                    // server, to all
                                    // parties.
                                    //
                                    // ===> This also means that the
                                    // ACTIVATING party himself will ALSO
                                    // get this same notice!
                                    // But since we've already established
                                    // above that the activating party
                                    // ALREADY processes
                                    // his activation reply, we don't want
                                    // him to process it TWICE!
                                    //
                                    // Therefore, we will process the notice
                                    // like normal, UNLESS pNym is the
                                    // activating Nym for
                                    // the smart contract, in which case we
                                    // skip it, since we assume he already
                                    // processed the
                                    // reply directly when he activated the
                                    // smart contract.
                                    //
                                    // You might ask, then why not just let
                                    // the activating party, process this
                                    // notice here the
                                    // same as all the other parties, and
                                    // just NOT have him process it on the
                                    // direct reply, as
                                    // he is now? The answer is, because he
                                    // will stay in sync better if we just
                                    // give him that
                                    // info as soon as he's able to receive
                                    // it, which is preferably RIGHT when he
                                    // performs the
                                    // activation. The other parties are not
                                    // currently present, so they HAVE to be
                                    // informed by
                                    // notices. But the ACTIVATING party
                                    // might as well be informed instantly.
                                    // Otherwise he will
                                    // just be out of sync until the next
                                    // time he processes his Nymbox, which
                                    // causes unnecessary
                                    // delays as it will result in
                                    // unnecessary server messages to resync
                                    // the situation.
                                    //
                                    // THEREFORE: We will skip this step if
                                    // pNym is the activating Nym, since
                                    // he's assumed to
                                    // have done this already. Otherwise,
                                    // pNym is NOT the activating Nym, and
                                    // he's one of the
                                    // other parties receiving this notice,
                                    // and therefore he needs to process it
                                    // accordingly
                                    // (He, in fact, processes it here
                                    // IDENTICALLY as the activating Nym
                                    // does when he receives
                                    // the reply to his transaction request:
                                    // by removing the issued opening
                                    // number, and by
                                    // harvesting the closing numbers.)
                                    // If it was a failure, harvest the
                                    // extra transaction numbers that were
                                    // used as
                                    // CLOSING numbers. They can go back on
                                    // my Nym and be used another day!
                                    // Remove
                                    // the opening number and harvest the
                                    // closing ones, basically.
                                    //

                                    String strCronItem;
                                    pServerTransaction->GetReferenceString(
                                        strCronItem);

                                    // What kind of cron item is it?
                                    // Well (todo) we should probably
                                    // double-check, but the only cron items
                                    // we
                                    // send notices for are payment plans
                                    // and smart contracts. Market offers
                                    // don't
                                    // need notices, since anyone activating
                                    // a market offer is already getting the
                                    // reply. (AND getting a copy of that
                                    // reply, already, inside a replyNotice
                                    // in
                                    // his Nymbox...) So he can't possibly
                                    // miss the server's reply, and there
                                    // aren't
                                    // any other parties to notify (re:
                                    // successful activation), besides the
                                    // Nym himself.
                                    //
                                    // Only payment plans and smart
                                    // contracts could potentially have some
                                    // other signer, who
                                    // would want to get notified, and to
                                    // whom the notice is send.
                                    //
                                    std::unique_ptr<OTCronItem> pCronItem(
                                        (strCronItem.Exists()
                                             ? OTCronItem::NewCronItem(
                                                   strCronItem)
                                             : nullptr));

                                    if (nullptr != pCronItem) // the original
                                                              // smart contract
                                                              // or payment
                                                              // plan object.
                                    {
                                        Identifier theCancelerNymID;
                                        const int64_t lNymOpeningNumber =
                                            pCronItem->GetOpeningNumber(
                                                pNym->GetConstID());
                                        const bool bCancelling =
                                            (pCronItem->IsCanceled() &&
                                             pCronItem->GetCancelerID(
                                                 theCancelerNymID));
                                        const bool bIsCancelerNym =
                                            (bCancelling &&
                                             (pNym->GetConstID() ==
                                              theCancelerNymID));
                                        const bool bIsActivatingNym =
                                            (pCronItem->GetOpeningNum() ==
                                             lNymOpeningNumber); // If the
                                                                 // opening
                                                                 // number
                                                                 // for the
                                                                 // cron
                                                                 // item is
                                                                 // the SAME
                                                                 // as Nym's
                                                                 // opening
                                                                 // number,
                                                                 // then Nym
                                                                 // is the
                                        // ACTIVATING
                                        // NYM
                                        // (Skip
                                        // him,
                                        // since he
                                        // does
                                        // this
                                        // same
                                        // stuff
                                        // when he
                                        // receives
                                        // the
                                        // actual
                                        // server
                                        // reply.
                                        // The
                                        // notices
                                        // are for
                                        // the
                                        // OTHER
                                        // parties)...

                                        // Canceler (if cancelling) or
                                        // activator (if activating) are
                                        // handled already elsewhere, when
                                        // they receive
                                        // the server reply. A notice is
                                        // also sent to all the parties (and
                                        // we're processing that notice now)
                                        // so here
                                        // we just need to handle everyone
                                        // else but him.
                                        //
                                        if ((bCancelling &&
                                             !bIsCancelerNym) || // If
                                            // canceling,
                                            // and Nym
                                            // is not
                                            // the
                                            // canceler...
                                            (!bCancelling &&
                                             !bIsActivatingNym) // or if
                                            // activating,
                                            // and Nym
                                            // is not
                                            // the
                                            // activator...
                                            ) {
                                            if (Item::rejection ==
                                                pReplyItem
                                                    ->GetStatus()) // REJECTION
                                                                   // (This
                                                                   // is
                                                                   // where
                                                                   // we
                                                                   // remove
                                                                   // the
                                            // opening
                                            // number,
                                            // and
                                            // harvest
                                            // the
                                            // closing
                                            // numbers.)
                                            {
                                                // Why do this? Oh I see,
                                                // this number either gets
                                                // burned from the attempt,
                                                // or it stays open for a
                                                // while if success. So here
                                                // what do we see? The
                                                // rejection
                                                // burning the transaction
                                                // number, but leaving it
                                                // open if success. Perfect.
                                                //
                                                if (false ==
                                                    pNym->RemoveIssuedNum(
                                                        *pNym, strNotaryID,
                                                        lNymOpeningNumber,
                                                        true)) // bool
                                                               // bSave=true
                                                {
                                                    otErr << __FUNCTION__
                                                          << ": Error "
                                                             "removing "
                                                             "issued number "
                                                             "from user nym "
                                                             "(for a cron "
                                                             "item.)\n";
                                                }
                                                // If the activation was a
                                                // failure, we can add all
                                                // the extra transaction
                                                // numbers BACK to the
                                                // Nym, that were being used
                                                // as CLOSING numbers, and
                                                // use them later. (They
                                                // aren't burned.)
                                                // They're still all
                                                // signed-out, so we should
                                                // harvest them so we can
                                                // still use them on
                                                // something.
                                                // (Whereas if it had been a
                                                // success, then we would
                                                // have left them in their
                                                // existing state, since
                                                // the transaction would
                                                // then be in play, and the
                                                // numbers could not be used
                                                // again, nor removed as
                                                // issued numbers until the
                                                // transaction itself had
                                                // finished and its receipts
                                                // had been signed-off.)
                                                //
                                                pCronItem
                                                    ->HarvestClosingNumbers(
                                                        *pNym); // saves.
                                            }
                                            // If success, save a copy in my
                                            // "active cron items" folder.
                                            //
                                            else // if (OTItem::acknowledged
                                                 // ==
                                                 // pReplyItem->GetStatus())
                                            {
                                                pCronItem
                                                    ->SaveActiveCronReceipt(
                                                        pNym->GetConstID());
                                            }
                                            // When party receives notice
                                            // that smart contract has been
                                            // activated,
                                            // remove the instrument from
                                            // outpayments box. (If it's
                                            // there -- it can be.)
                                            //
                                            // (This happens for
                                            // acknowledged AND rejected
                                            // smart contracts.)
                                            //
                                            NumList numlistOutpayment(
                                                lNymOpeningNumber);
                                            String strInstrument; // If
                                                                  // the
                                            // instrument is
                                            // in the
                                            // outpayments
                                            // box, we put a
                                            // copy of it
                                            // here.
                                            const int32_t nOutpaymentIndex =
                                                GetOutpaymentsIndexByTransNum(
                                                    *pNym, lNymOpeningNumber);
                                            std::unique_ptr<Message>
                                                theMessageAngel;

                                            if (nOutpaymentIndex >= 0) {
                                                Message* pMsg =
                                                    pNym->GetOutpaymentsByIndex(
                                                        nOutpaymentIndex);

                                                if (nullptr == pMsg) {
                                                    otErr << __FUNCTION__
                                                          << ": Unable to "
                                                             "find payment "
                                                             "message in "
                                                             "outpayment box "
                                                             "based on index "
                                                          << nOutpaymentIndex
                                                          << ".\n";
                                                }
                                                else {
                                                    const bool bRemovedOutpayment =
                                                        pNym->RemoveOutpaymentsByIndex(
                                                            nOutpaymentIndex,
                                                            false); // bDeleteIt=false
                                                    // (deleted
                                                    // later
                                                    // on.)
                                                    theMessageAngel.reset(pMsg);
                                                    // Since we
                                                    // chose to
                                                    // keep pMsg
                                                    // alive and
                                                    // undeleted,
                                                    // after
                                                    // removing
                                                    // it from
                                                    // the
                                                    // outpayments
                                                    // box, we
                                                    // set the
                                                    // angel here
                                                    // to make
                                                    // sure it
                                                    // gets
                                                    // cleaned up
                                                    // later,
                                                    // whenever
                                                    // we return
                                                    // out of
                                                    // this
                                                    // godforsaken
                                                    // function.
                                                    if (bRemovedOutpayment)
                                                        pNym->SaveSignedNymfile(
                                                            *pNym);
                                                    else
                                                        otErr
                                                            << __FUNCTION__
                                                            << ": Failed "
                                                               "trying to "
                                                               "remove "
                                                               "outpayment "
                                                               "at index: "
                                                            << nOutpaymentIndex
                                                            << "\n";
                                                    if (!pMsg->m_ascPayload
                                                             .GetString(
                                                                 strInstrument)) {
                                                        otErr
                                                            << __FUNCTION__
                                                            << ": Unable "
                                                               "to find "
                                                               "payment "
                                                               "instrument "
                                                               "in "
                                                               "outpayment "
                                                               "message at "
                                                               "index "
                                                            << nOutpaymentIndex
                                                            << ".\n";
                                                    }
                                                    else {
                                                        // At this point,
                                                        // we've removed the
                                                        // outpayment
                                                        // already, and it
                                                        // will be deleted
                                                        // when it goes out
                                                        // of scope already.
                                                        // And we've got a
                                                        // copy of the
                                                        // original
                                                        // financial
                                                        // instrument that
                                                        // was SENT in that
                                                        // outpayment.
                                                        //
                                                        // But what for? Why
                                                        // did I want that
                                                        // instrument here
                                                        // in a string, in
                                                        // strInstrument?
                                                        // Do I still need
                                                        // to do something
                                                        // with it? Yes: I
                                                        // need to drop a
                                                        // copy of it into
                                                        // the record box!

                                                        // NOTE:
                                                        // strInstrument is
                                                        // added to the
                                                        // RecordBox below.
                                                        // So there's no
                                                        // need to
                                                        // do that here,
                                                        // ATM.
                                                    }
                                                }
                                            }
                                            // When party receives notice
                                            // that smart contract has
                                            // failed activation attempt,
                                            // then remove
                                            // the instrument from payments
                                            // inbox AND outpayments box.
                                            // (If there -- could be for
                                            // either.)
                                            // (Outbox is done just above,
                                            // so now let's do inbox...)
                                            //

                                            // Why only rejected items? Why
                                            // not remove it from the
                                            // payments inbox on success as
                                            // well?
                                            // Normally wouldn't we expect
                                            // that a successful activation
                                            // of an inbox item, should
                                            // remove
                                            // that inbox item? Especially
                                            // if there's already a copy in
                                            // the outbox as well...
                                            //
                                            //                                              if
                                            // (OTItem::rejection ==
                                            // pReplyItem->GetStatus()) //
                                            // REJECTION
                                            {
                                                const bool bExists1 =
                                                    OTDB::Exists(
                                                        OTFolders::
                                                            PaymentInbox()
                                                                .Get(),
                                                        strNotaryID.Get(),
                                                        strNymID.Get());
                                                const bool bExists2 =
                                                    OTDB::Exists(
                                                        OTFolders::RecordBox()
                                                            .Get(),
                                                        strNotaryID.Get(),
                                                        strNymID.Get());
                                                Ledger thePmntInbox(
                                                    NYM_ID, NYM_ID,
                                                    NOTARY_ID); // payment
                                                                // inbox
                                                Ledger theRecordBox(
                                                    NYM_ID, NYM_ID,
                                                    NOTARY_ID); // record
                                                                // box
                                                bool bSuccessLoading1 =
                                                    (bExists1 &&
                                                     thePmntInbox
                                                         .LoadPaymentInbox());
                                                bool bSuccessLoading2 =
                                                    (bExists2 &&
                                                     theRecordBox
                                                         .LoadRecordBox());
                                                if (bExists1 &&
                                                    bSuccessLoading1)
                                                    bSuccessLoading1 =
                                                        (thePmntInbox
                                                             .VerifyContractID() &&
                                                         thePmntInbox
                                                             .VerifySignature(
                                                                 *pNym));
                                                //                                                      bSuccessLoading1
                                                // =
                                                // (thePmntInbox.VerifyAccount(*pNym));
                                                // // (No need to load all
                                                // the Box Receipts using
                                                // VerifyAccount)
                                                else if (!bExists1)
                                                    bSuccessLoading1 =
                                                        thePmntInbox
                                                            .GenerateLedger(
                                                                NYM_ID,
                                                                NOTARY_ID,
                                                                Ledger::
                                                                    paymentInbox,
                                                                true); // bGenerateFile=true
                                                if (bExists2 &&
                                                    bSuccessLoading2)
                                                    bSuccessLoading2 =
                                                        (theRecordBox
                                                             .VerifyContractID() &&
                                                         theRecordBox
                                                             .VerifySignature(
                                                                 *pNym));
                                                //                                                      bSuccessLoading2
                                                // =
                                                // (theRecordBox.VerifyAccount(*pNym));
                                                // // (No need to load all
                                                // the Box Receipts using
                                                // VerifyAccount)
                                                else if (!bExists2)
                                                    bSuccessLoading2 =
                                                        theRecordBox
                                                            .GenerateLedger(
                                                                NYM_ID,
                                                                NOTARY_ID,
                                                                Ledger::
                                                                    recordBox,
                                                                true); // bGenerateFile=true
                                                // by this point, the boxes
                                                // DEFINITELY exist -- or
                                                // not. (generation might
                                                // have failed, or
                                                // verification.)
                                                //
                                                if (!bSuccessLoading1 ||
                                                    !bSuccessLoading2) {
                                                    otOut << __FUNCTION__
                                                          << ": while "
                                                             "processing "
                                                             "server "
                                                             "rejection of "
                                                             "cron item: "
                                                             "WARNING: "
                                                             "Unable to "
                                                             "load, verify, "
                                                             "or generate "
                                                             "paymentInbox "
                                                             "or recordBox, "
                                                             "with IDs: "
                                                          << strNymID << " / "
                                                          << strNymID << "\n";
                                                }
                                                else // --- ELSE ---
                                                       // Success loading
                                                       // the payment inbox
                                                       // and recordBox and
                                                       // verifying
                                                {      // their contractID and
                                                    // signature, (OR
                                                    // success
                                                    // generating the
                                                    // ledger.)
                                                    // See if there's a
                                                    // receipt in the
                                                    // payments inbox.
                                                    // If so, remove it.
                                                    //
                                                    // What's going on here?
                                                    //
                                                    // Well let's say Alice
                                                    // sends Bob a payment
                                                    // plan. (This applies
                                                    // to smart contracts,
                                                    // too.)
                                                    // This means Bob has a
                                                    // payment plan in his
                                                    // PAYMENTS INBOX, with
                                                    // the recipient's
                                                    // (Alice)
                                                    // transaction number
                                                    // set to X, and the
                                                    // sender's transaction
                                                    // number set to 0. It's
                                                    // 0 because
                                                    // the instrument is
                                                    // still in Bob's inbox
                                                    // -- he hasn't signed
                                                    // it yet -- so his
                                                    // transaction
                                                    // number isn't on it
                                                    // yet. It's blank (0).
                                                    //
                                                    // Next, let's say Bob
                                                    // signs/confirms the
                                                    // contract, which puts
                                                    // a copy of it into his
                                                    // PAYMENTS
                                                    // OUTBOX. On the outbox
                                                    // version, Alice's
                                                    // transaction number is
                                                    // X, and Bob's
                                                    // transaction number
                                                    // is Y.
                                                    //
                                                    // Later on, Bob needs
                                                    // to lookup the payment
                                                    // plan in his PAYMENTS
                                                    // INBOX (for example,
                                                    // to remove
                                                    // it, AS YOU SEE IN THE
                                                    // BELOW LOOP.)
                                                    // Remember, Bob's
                                                    // transaction number is
                                                    // Y. But he can't use
                                                    // that number (Y) to
                                                    // lookup the payment
                                                    // plan in his inbox,
                                                    // since it's set to
                                                    // ZERO in his inbox!
                                                    // The inbox version
                                                    // simply doesn't HAVE Y
                                                    // set onto it yet --
                                                    // only the outbox
                                                    // version does.
                                                    //
                                                    // So how in the fuck
                                                    // does Bob lookup the
                                                    // inbox version, if the
                                                    // transaction number
                                                    // isn't SET on
                                                    // it yet??
                                                    //
                                                    // The solution:
                                                    // 1. Bob grabs an
                                                    // OTNumList containing
                                                    // all the transaction
                                                    // numbers from the
                                                    // OUTBOX VERSION,
                                                    //    which ends up
                                                    // containing "X,Y"
                                                    // (that happens in this
                                                    // block.)
                                                    // 2. Bob loops through
                                                    // the payments INBOX,
                                                    // and for each, he
                                                    // grabs an OTNumList
                                                    // containing all
                                                    //    the transaction
                                                    // numbers. One of those
                                                    // (the matching one)
                                                    // will contain "X,0".
                                                    // (Except it
                                                    //    will actually only
                                                    // contain "X", since 0
                                                    // is ignored in the
                                                    // call to
                                                    // GetAllTransactionNumbers.)
                                                    // 3. Bob then checks
                                                    // like this:    if
                                                    // (numlistOutpayment.VerifyAny(numlistIncomingPayment))
                                                    //    This is equivalent
                                                    // to saying: if
                                                    // ("X,Y".VerifyAny("X"))
                                                    // which RETURNS TRUE --
                                                    // and we have
                                                    //    found the
                                                    // instrument!

                                                    OTPayment theOutpayment;

                                                    if (strInstrument
                                                            .Exists() &&
                                                        theOutpayment.SetPayment(
                                                            strInstrument) &&
                                                        theOutpayment
                                                            .SetTempValues()) {
                                                        theOutpayment
                                                            .GetAllTransactionNumbers(
                                                                numlistOutpayment);
                                                    }

                                                    const int32_t nTransCount =
                                                        thePmntInbox
                                                            .GetTransactionCount();

                                                    for (int32_t ii =
                                                             (nTransCount - 1);
                                                         ii >= 0; --ii) // Count
                                                    // backwards
                                                    // since we
                                                    // are
                                                    // removing
                                                    // things.
                                                    {
                                                        std::unique_ptr<
                                                            OTPayment>
                                                            pPayment(GetInstrument(
                                                                *pNym, ii,
                                                                thePmntInbox));

                                                        if (nullptr ==
                                                            pPayment) {
                                                            otOut
                                                                << __FUNCTION__
                                                                << ": "
                                                                   "(Upon "
                                                                   "receivi"
                                                                   "ng "
                                                                   "notice)"
                                                                   " While "
                                                                   "looping"
                                                                   " paymen"
                                                                   "ts "
                                                                   "inbox "
                                                                   "to "
                                                                   "remove "
                                                                   "a "
                                                                   "payment"
                                                                   ", "
                                                                   "unable "
                                                                   "to "
                                                                   "retriev"
                                                                   "e "
                                                                   "payment"
                                                                   " at "
                                                                   "index "
                                                                << ii
                                                                << " ("
                                                                   "skippin"
                                                                   "g.)\n";
                                                            continue;
                                                        }
                                                        else if (false ==
                                                                   pPayment
                                                                       ->SetTempValues()) {
                                                            otOut
                                                                << __FUNCTION__
                                                                << ": "
                                                                   "(Upon "
                                                                   "receivi"
                                                                   "ng "
                                                                   "notice)"
                                                                   " While "
                                                                   "looping"
                                                                   " paymen"
                                                                   "ts "
                                                                   "inbox "
                                                                   "to "
                                                                   "remove "
                                                                   "a "
                                                                   "payment"
                                                                   ", "
                                                                   "unable "
                                                                   "to set "
                                                                   "temp "
                                                                   "values "
                                                                   "for "
                                                                   "payment"
                                                                   " at "
                                                                   "index "
                                                                << ii
                                                                << " ("
                                                                   "skippin"
                                                                   "g.)\n";
                                                            continue;
                                                        }

                                                        NumList
                                                            numlistIncomingPayment;

                                                        pPayment->GetAllTransactionNumbers(
                                                            numlistIncomingPayment);

                                                        if (numlistOutpayment.VerifyAny(
                                                                numlistIncomingPayment)) // Found it.
                                                        {
                                                            // ** It's the
                                                            // same
                                                            // instrument.**
                                                            // Remove it
                                                            // from the
                                                            // payments
                                                            // inbox, and
                                                            // save.
                                                            //
                                                            OTTransaction*
                                                                pTransPaymentInbox =
                                                                    thePmntInbox
                                                                        .GetTransactionByIndex(
                                                                            ii);
                                                            OT_ASSERT(
                                                                nullptr !=
                                                                pTransPaymentInbox); // It DEFINITELY should be there. (Assert otherwise.)
                                                            int64_t lPaymentTransNum =
                                                                pTransPaymentInbox
                                                                    ->GetTransactionNum();

                                                            // DON'T I NEED
                                                            // to call
                                                            // DeleteBoxReceipt
                                                            // at this
                                                            // point?
                                                            // Since that
                                                            // needs to be
                                                            // called now
                                                            // whenever
                                                            // removing
                                                            // something
                                                            // from any box?
                                                            //
                                                            // NOTE: might
                                                            // need to just
                                                            // MOVE this box
                                                            // receipt to
                                                            // the record
                                                            // box, instead
                                                            // of
                                                            // deleting it.
                                                            //
                                                            // Probably I
                                                            // need to do
                                                            // that ONLY if
                                                            // the version
                                                            // in the
                                                            // payments
                                                            // outbox
                                                            // doesn't
                                                            // exist.
                                                            // For example,
                                                            // if
                                                            // strInstrument
                                                            // doesn't
                                                            // exist, then
                                                            // there was
                                                            // nothing in
                                                            // the payments
                                                            // outbox, and
                                                            // therefore the
                                                            // version in
                                                            // the payment
                                                            // INBOX is the
                                                            // ONLY version
                                                            // I have,
                                                            // and therefore
                                                            // I should
                                                            // stick it in
                                                            // the Record
                                                            // Box.
                                                            //
                                                            // HOWEVER, if
                                                            // strInstrument
                                                            // DOES exist,
                                                            // then I should
                                                            // create its
                                                            // own
                                                            // transaction
                                                            // to add
                                                            // to the record
                                                            // box, and
                                                            // delete the
                                                            // one that was
                                                            // in the
                                                            // payment
                                                            // inbox. Why
                                                            // delete it?
                                                            // Because
                                                            // otherwise I
                                                            // would be
                                                            // adding the
                                                            // same thing
                                                            // TWICE to the
                                                            // record box,
                                                            // which I don't
                                                            // really
                                                            // need to do.
                                                            // And if I'm
                                                            // going to
                                                            // choose one of
                                                            // the two, the
                                                            // one in the
                                                            // outpayments
                                                            // box will
                                                            // be the more
                                                            // recent / more
                                                            // relevant one
                                                            // of the two.
                                                            // So I favor
                                                            // that one,
                                                            // unless it
                                                            // doesn't
                                                            // exist, in
                                                            // which case I
                                                            // should add
                                                            // the other one
                                                            // instead.
                                                            // (Todo.)
                                                            //
                                                            // NOTE: Until
                                                            // the above is
                                                            // completed,
                                                            // the current
                                                            // behavior is
                                                            // that the
                                                            // outpayments
                                                            // box item
                                                            // will be moved
                                                            // to the record
                                                            // box if it
                                                            // exists, and
                                                            // otherwise
                                                            // nothing will
                                                            // be, since any
                                                            // payments
                                                            // inbox item
                                                            // will be
                                                            // deleted.

                                                            if (false ==
                                                                thePmntInbox.DeleteBoxReceipt(
                                                                    lPaymentTransNum)) {
                                                                otErr
                                                                    << __FUNCTION__
                                                                    << ": "
                                                                       "Fai"
                                                                       "led"
                                                                       " tr"
                                                                       "yin"
                                                                       "g "
                                                                       "to "
                                                                       "del"
                                                                       "ete"
                                                                       " th"
                                                                       "e "
                                                                       "box"
                                                                       " re"
                                                                       "cei"
                                                                       "pt "
                                                                       "for"
                                                                       " a "
                                                                       "tra"
                                                                       "nsa"
                                                                       "cti"
                                                                       "on "
                                                                       "bei"
                                                                       "ng "
                                                                       "rem"
                                                                       "ove"
                                                                       "d "
                                                                       "fro"
                                                                       "m "
                                                                       "the"
                                                                       " pa"
                                                                       "yme"
                                                                       "nt "
                                                                       "inb"
                                                                       "ox."
                                                                       "\n";
                                                            }
                                                            if (thePmntInbox.RemoveTransaction(
                                                                    lPaymentTransNum)) {
                                                                thePmntInbox
                                                                    .ReleaseSignatures();
                                                                thePmntInbox
                                                                    .SignContract(
                                                                        *pNym);
                                                                thePmntInbox
                                                                    .SaveContract();

                                                                if (!thePmntInbox
                                                                         .SavePaymentInbox()) {
                                                                    otErr
                                                                        << __FUNCTION__
                                                                        << ": "
                                                                           "Fai"
                                                                           "lur"
                                                                           "e "
                                                                           "whi"
                                                                           "le "
                                                                           "try"
                                                                           "ing"
                                                                           " to"
                                                                           " sa"
                                                                           "ve "
                                                                           "pay"
                                                                           "men"
                                                                           "t "
                                                                           "inb"
                                                                           "ox."
                                                                           "\n";
                                                                }
                                                                else {
                                                                    otOut
                                                                        << __FUNCTION__
                                                                        << ": "
                                                                           "Rem"
                                                                           "ove"
                                                                           "d "
                                                                           "ins"
                                                                           "tru"
                                                                           "men"
                                                                           "t "
                                                                           "fro"
                                                                           "m "
                                                                           "pay"
                                                                           "men"
                                                                           "t "
                                                                           "inb"
                                                                           "ox."
                                                                           "\nS"
                                                                           "ave"
                                                                           "d "
                                                                           "pay"
                                                                           "men"
                                                                           "t "
                                                                           "inb"
                                                                           "ox."
                                                                           "\n";
                                                                }
                                                            }
                                                            else {
                                                                otErr
                                                                    << __FUNCTION__
                                                                    << ": "
                                                                       "Fai"
                                                                       "led"
                                                                       " tr"
                                                                       "yin"
                                                                       "g "
                                                                       "to "
                                                                       "rem"
                                                                       "ove"
                                                                       " tr"
                                                                       "ans"
                                                                       "act"
                                                                       "ion"
                                                                       " fr"
                                                                       "om "
                                                                       "pay"
                                                                       "men"
                                                                       "t "
                                                                       "inb"
                                                                       "ox."
                                                                       " ("
                                                                       "Sho"
                                                                       "uld"
                                                                       " ne"
                                                                       "ver"
                                                                       " ha"
                                                                       "ppe"
                                                                       "n.)"
                                                                       "\n";
                                                            }
                                                            // Todo: save a
                                                            // copy to the
                                                            // record box.
                                                            // Note: I could
                                                            // break right
                                                            // here, if this
                                                            // is the only
                                                            // transaction
                                                            // in the
                                                            // payment inbox
                                                            // which
                                                            // contains the
                                                            // instrument in
                                                            // question.
                                                            // Which I
                                                            // believe
                                                            // it is.  Todo:
                                                            // if that's
                                                            // true, which I
                                                            // think it is,
                                                            // then call
                                                            // break here.
                                                            // After all,
                                                            // you wouldn't
                                                            // send me the
                                                            // SAME
                                                            // instrument
                                                            // TWICE, would
                                                            // you?
                                                            // But it still
                                                            // seems
                                                            // theoretically
                                                            // possible
                                                            // (albeit
                                                            // stupid.)
                                                        }
                                                    }
                                                    // for (int32_t ii = 0;
                                                    // ii < nTransCount;
                                                    // ++ii)
                                                    // Also, if there was a
                                                    // message in the
                                                    // outpayments box
                                                    // (which we already
                                                    // removed
                                                    // a bit above), go
                                                    // ahead and add a
                                                    // receipt for it into
                                                    // the record box.
                                                    //
                                                    if (strInstrument
                                                            .Exists()) // Found
                                                    // the
                                                    // instrument
                                                    // in
                                                    // the
                                                    // outpayments
                                                    // box.
                                                    {
                                                        OTTransaction* pNewTransaction =
                                                            OTTransaction::
                                                                GenerateTransaction(
                                                                    theRecordBox, // recordbox.
                                                                    OTTransaction::
                                                                        notice,
                                                                    lNymOpeningNumber);
                                                        std::unique_ptr<
                                                            OTTransaction>
                                                            theTransactionAngel(
                                                                pNewTransaction);

                                                        if (nullptr !=
                                                            pNewTransaction) // The above has an OT_ASSERT within, but I just like to check my pointers.
                                                        {
                                                            pNewTransaction
                                                                ->SetReferenceToNum(
                                                                    lNymOpeningNumber); // Referencing myself here. We'll see how it works out.
                                                            pNewTransaction
                                                                ->SetReferenceString(
                                                                    strInstrument); // The cheque, invoice, etc that used to be in the outpayments box.
                                                            pNewTransaction
                                                                ->SignContract(
                                                                    *pNym);
                                                            pNewTransaction
                                                                ->SaveContract();
                                                            const bool bAdded =
                                                                theRecordBox
                                                                    .AddTransaction(
                                                                        *pNewTransaction);

                                                            if (!bAdded) {
                                                                otErr
                                                                    << __FUNCTION__
                                                                    << ": "
                                                                       "Una"
                                                                       "ble"
                                                                       " to"
                                                                       " ad"
                                                                       "d "
                                                                       "tra"
                                                                       "nsa"
                                                                       "cti"
                                                                       "on "
                                                                    << pNewTransaction
                                                                           ->GetTransactionNum()
                                                                    << " to"
                                                                       " re"
                                                                       "cor"
                                                                       "d "
                                                                       "box"
                                                                       " ("
                                                                       "aft"
                                                                       "er "
                                                                       "ten"
                                                                       "tat"
                                                                       "ive"
                                                                       "ly "
                                                                       "rem"
                                                                       "ovi"
                                                                       "ng "
                                                                       "fro"
                                                                       "m "
                                                                       "pay"
                                                                       "men"
                                                                       "t "
                                                                       "out"
                                                                       "box"
                                                                       ", "
                                                                       "an "
                                                                       "act"
                                                                       "ion"
                                                                       " th"
                                                                       "at "
                                                                       "is "
                                                                       "now"
                                                                       " ca"
                                                                       "nce"
                                                                       "led"
                                                                       ".)"
                                                                       "\n";
                                                                return false;
                                                            }
                                                            else
                                                                theTransactionAngel
                                                                    .release(); // If successfully added to the record box, then no need anymore to clean it up ourselves. The record box owns it now.

                                                            theRecordBox
                                                                .ReleaseSignatures();
                                                            theRecordBox
                                                                .SignContract(
                                                                    *pNym);
                                                            theRecordBox
                                                                .SaveContract();
                                                            theRecordBox
                                                                .SaveRecordBox(); // todo log failure.

                                                            // Any
                                                            // inbox/nymbox/outbox
                                                            // ledger will
                                                            // only itself
                                                            // contain
                                                            // abbreviated
                                                            // versions of
                                                            // the receipts,
                                                            // including
                                                            // their hashes.
                                                            //
                                                            // The rest is
                                                            // stored
                                                            // separately,
                                                            // in the box
                                                            // receipt,
                                                            // which is
                                                            // created
                                                            // whenever a
                                                            // receipt is
                                                            // added to a
                                                            // box, and
                                                            // deleted after
                                                            // a receipt
                                                            // is removed
                                                            // from a box.
                                                            //
                                                            if (!pNewTransaction
                                                                     ->SaveBoxReceipt(
                                                                         theRecordBox)) // <===================
                                                            {
                                                                String strNewTransaction(
                                                                    *pNewTransaction);
                                                                otErr
                                                                    << __FUNCTION__
                                                                    << ": "
                                                                       "for"
                                                                       " Re"
                                                                       "cor"
                                                                       "d "
                                                                       "Box"
                                                                       "..."
                                                                       " Fa"
                                                                       "ile"
                                                                       "d "
                                                                       "try"
                                                                       "ing"
                                                                       " to"
                                                                       " Sa"
                                                                       "veB"
                                                                       "oxR"
                                                                       "ece"
                                                                       "ipt"
                                                                       ". "
                                                                       "Con"
                                                                       "ten"
                                                                       "ts:"
                                                                       "\n"
                                                                       "\n"
                                                                    << strNewTransaction
                                                                    << "\n"
                                                                       "\n";
                                                            }
                                                        }
                                                        else // should
                                                               // never
                                                               // happen
                                                        {
                                                            otErr
                                                                << __FUNCTION__
                                                                << ": "
                                                                   "Failed "
                                                                   "while "
                                                                   "trying "
                                                                   "to "
                                                                   "generat"
                                                                   "e "
                                                                   "transac"
                                                                   "tion "
                                                                   "in "
                                                                   "order "
                                                                   "to add "
                                                                   "a new "
                                                                   "transac"
                                                                   "tion "
                                                                   "to "
                                                                   "record "
                                                                   "box "
                                                                   "(for a "
                                                                   "payment"
                                                                   " instru"
                                                                   "ment "
                                                                   "we "
                                                                   "just "
                                                                   "removed"
                                                                   " from "
                                                                   "the "
                                                                   "outpaym"
                                                                   "ents "
                                                                   "box): "
                                                                << strNymID
                                                                << "\n";
                                                        }
                                                    } // if
                                                    // (strInstrument.Exists())
                                                    // (then add a copy to
                                                    // record box.)
                                                } // else (Success loading
                                                  // the payment inbox and
                                                  // recordBox)
                                            }     // (OTItem::rejection ==
                                                  // pReplyItem->GetStatus())
                                        }         // if (!bIsActivatingNym)
                                    }             // if (nullptr != pCronItem)
                                    else {
                                        otErr << __FUNCTION__
                                              << ": Error loading cronitem "
                                                 "from Nymbox receipt, "
                                                 "from string:\n" << strCronItem
                                              << "\n";
                                    }

                                } // pReplyItem is a rejection.
                            }     // pServerTransaction (the Nymbox receipt we
                                  // just accepted / removed) is a notice.

                            break;

                        case Item::atAcceptMessage:
                        case Item::atAcceptTransaction:
                            break;
                        // I don't think we need to do anything here...

                        case Item::atAcceptFinalReceipt:
                            otInfo << __FUNCTION__
                                   << ": Successfully removed finalReceipt "
                                      "from Nymbox with opening num: "
                                   << pServerTransaction->GetReferenceToNum()
                                   << "\n";

                            if (pNym->RemoveIssuedNum(
                                    *pNym, strNotaryID,
                                    pServerTransaction->GetReferenceToNum(),
                                    true)) // bool bSave=true
                                otWarn
                                    << "**** Due to finding a "
                                       "finalReceipt, REMOVING OPENING "
                                       "NUMBER FROM NYM:  "
                                    << pServerTransaction->GetReferenceToNum()
                                    << " \n";
                            else
                                otWarn
                                    << "**** Noticed a finalReceipt, "
                                       "but Opening Number "
                                    << pServerTransaction->GetReferenceToNum()
                                    << " had ALREADY been removed from "
                                       "nym. \n";

                            // BUG: RemoveIssuedNum shouldn't be here. In
                            // Nymbox, finalReceipt is only a notice, and I
                            // shoulda
                            // removed the number the instant that I saw it.
                            // (Back when processing the Nymbox, before even
                            // calculating the request.) Therefore, this is
                            // moved to AcceptEntireNymbox and Finalize for
                            // Process Inbox.
                            //
                            //                              pNym->RemoveIssuedNum(*pNym,
                            // strNotaryID,
                            // pServerTransaction->GetReferenceToNum(),
                            // true); // bool bSave=true
                            // The client side keeps a list of active
                            // (recurring) transactions.
                            // That is, smart contracts and payment plans. I
                            // don't think it keeps
                            // market offers in that list, since we already
                            // have a list of active
                            // market offers separately. And market offers
                            // produce final receipts,
                            // so basically this piece of code will be
                            // executed for all final receipts.
                            // It's not really necessary that it be called
                            // for market offers, but whatever.
                            // It is for the others.
                            //
                            // Notice even though the final receipt hasn't
                            // yet been cleared out of the box,
                            // we are already removing the record of the
                            // active cron receipt. Why?
                            // Because regardless of when the user processes
                            // the finalReceipt, we know for
                            // a fact the transaction is no longer actively
                            // running on Cron. So we don't want
                            // to keep it on our list of "active" cron items
                            // if we know it's already inactive.
                            //
                            OTCronItem::EraseActiveCronReceipt(
                                pServerTransaction->GetReferenceToNum(),
                                pNym->GetConstID(),
                                pServerTransaction->GetPurportedNotaryID());

                            break;

                        default: {
                            String strTempTypeString;
                            pReplyItem->GetTypeString(strTempTypeString);
                            otErr << "Unexpected replyItem:type while "
                                     "processing Nymbox: " << strTempTypeString
                                  << " \n";
                            continue;
                        }
                        } // switch replyItem type

                        // Remove from pNymbox
                        // This happens for ALL of the above cases.
                        // Update: Now whenever removing a receipt from any
                        // box, we also have
                        // to delete the box receipt, which is stored as a
                        // separate file.
                        //
                        pServerTransaction->DeleteBoxReceipt(
                            *pNymbox); // faster.
                                       //                            pNymbox->DeleteBoxReceipt(pServerTransaction->GetTransactionNum());
                        pNymbox->RemoveTransaction(
                            pServerTransaction->GetTransactionNum());

                    } // for loop (reply items)
                    // All done? Let's save up...
                    //
                    pNymbox->ReleaseSignatures();
                    pNymbox->SignContract(*pNym);
                    pNymbox->SaveContract();
                    pNymbox->SaveNymbox();

                    pNymbox = nullptr; // Since it could be pointing to a
                                       // variable that's in this block (now
                                       // out of scope) then we clear the
                                       // pointer.
                } // pTransaction and pReplyTransaction are both NOT
                  // nullptr.
            }

            //
            // The below happens BOTH for Inbox AND Nymbox.

            if ((nullptr != pTransaction) && (nullptr != pReplyTransaction)) {
                //
                // SAVE THE RECEIPT....

                String strNotaryID(NOTARY_ID);

                String strReceiptID("NOT_SET_YET");

                Item* pReplyItem =
                    pReplyTransaction->GetItem(Item::atBalanceStatement);

                if (nullptr == pReplyItem) {
                    pReplyItem = pReplyTransaction->GetItem(
                        Item::atTransactionStatement);

                    if (nullptr != pReplyItem)
                        pNym->GetIdentifier(strReceiptID); // In this case,
                                                           // the receipt ID
                                                           // is the Nym ID
                }
                else {
                    strReceiptID =
                        theReply.m_strAcctID; // If a balance statement,
                                              // then the receipt ID is the
                                              // Account ID.
                }
                String strTransaction;
                pReplyTransaction->SaveContractRaw(
                    strTransaction); // <=========== Save that receipt!
                String strReceiptFilename;

                if (pReplyTransaction->GetSuccess())
                    strReceiptFilename.Format("%s.success", strReceiptID.Get());
                else
                    strReceiptFilename.Format("%s.fail", strReceiptID.Get());
                String strFinal;
                OTASCIIArmor ascTemp(strTransaction);

                if (false ==
                    ascTemp.WriteArmoredString(
                        strFinal, "TRANSACTION")) // todo hardcoding.
                {
                    otErr << "OTClient::ProcessServerReply: Error saving "
                             "transaction receipt (failed writing armored "
                             "string):\n" << OTFolders::Receipt()
                          << Log::PathSeparator() << strNotaryID
                          << Log::PathSeparator() << strReceiptFilename
                          << "\n Contents:\n" << strTransaction << "\n";
                }
                else // success writing armored string
                {
                    if (nullptr != pReplyItem) {
                        OTDB::StorePlainString(
                            strFinal.Get(), OTFolders::Receipt().Get(),
                            strNotaryID.Get(), strReceiptFilename.Get());
                    }
                    else // This should never happen...
                    {
                        strReceiptFilename.Format("%s.error",
                                                  strReceiptID.Get());

                        otErr << "OTClient::ProcessServerReply: Error "
                                 "saving transaction receipt:  " << strNotaryID
                              << Log::PathSeparator() << strReceiptFilename
                              << "\n";

                        OTDB::StorePlainString(
                            strFinal.Get(), OTFolders::Receipt().Get(),
                            strNotaryID.Get(), strReceiptFilename.Get());
                    }
                } // success writing armored string
            }
            else {
                const String strTheLedger(theLedger),
                    strTheReplyLedger(theReplyLedger);
                otOut << "Strange... found ledger in " << theReply.m_strCommand
                      << ", but didn't find the right transaction type "
                         "within.\n(pTransaction == "
                      << ((nullptr != pTransaction) ? "NOT nullptr" : "nullptr")
                      << ") && (pReplyTransaction == "
                      << ((nullptr != pReplyTransaction) ? "NOT nullptr"
                                                         : "nullptr")
                      << ")\ntheLedger: \n\n" << strTheLedger
                      << "\n\ntheReplyLedger:\n\n" << strTheReplyLedger
                      << "\n\n";
            }
        }
    }
    else {
        otOut << "Strange... received server acknowledgment but 'in "
                 "reference to' message was blank.\n";
    }

    return true;
}

bool OTClient::processServerReplyGetAccountData(const Message& theReply,
                                                Ledger* pNymbox,
                                                ProcessServerReplyArgs& args)
{

    const auto& ACCOUNT_ID = args.ACCOUNT_ID;
    const auto& NOTARY_ID = args.NOTARY_ID;
    const auto& NYM_ID = args.NYM_ID;
    const auto& pServerNym = args.pServerNym;
    const auto& pNym = args.pNym;

    otOut << "Received server response to getAccountData message.\n";

    String strAccount, strInbox, strOutbox;
    if (!theReply.m_ascPayload.GetString(strAccount) ||
        !theReply.m_ascPayload2.GetString(strInbox) ||
        !theReply.m_ascPayload3.GetString(strOutbox)) {
        otErr << __FUNCTION__ << ": Failed to decode armored reponse\n";
    }

    if (strAccount.Exists()) {
        // Load the account object from that string.
        std::unique_ptr<Account> pAccount(
            new Account(NYM_ID, ACCOUNT_ID, NOTARY_ID));

        if (pAccount && pAccount->LoadContractFromString(strAccount) &&
            pAccount->VerifyAccount(*pServerNym)) {
            otInfo << "Saving updated account file to disk...\n";
            pAccount->ReleaseSignatures(); // So I don't get the
                                           // annoying failure to
                                           // verify message from
                                           // the server's
                                           // signature.
            // Will eventually end up keeping the signature,
            // however, just for reasons of proof.
            // UPDATE (above) I now release signatures again since
            // we have receipts functional. As long as receipt has
            // server's signature, it can prove the others.
            pAccount->SignContract(*pNym);
            pAccount->SaveContract();
            pAccount->SaveAccount();

            m_pWallet->AddAccount(*(pAccount.release()));
            m_pWallet->SaveWallet();
        }
    }

    const String strAcctID(ACCOUNT_ID);
    const std::string str_acct_id(strAcctID.Get());

    if (strInbox.Exists()) {
        const String strNotaryID(NOTARY_ID);

        // Load the ledger object from strInbox
        Ledger theInbox(NYM_ID, ACCOUNT_ID, NOTARY_ID);

        // I receive the inbox, verify the server's signature, then
        // RE-SIGN IT WITH MY OWN
        // SIGNATURE, then SAVE it to local storage.  So any FUTURE
        // checks of this inbox
        // would require MY signature, not the server's, to verify.
        // But in this one spot,
        // just before saving, I need to verify the server's first.
        // UPDATE: Keeping the server's signature, and just adding
        // my own.
        if (theInbox.LoadInboxFromString(strInbox) &&
            theInbox.VerifySignature(*pServerNym)) // No VerifyAccount.
                                                   // Can't, because
        // client hasn't had a chance yet to
        // download the box receipts that go
        // with this inbox -- and
        // VerifyAccount() tries to load
        // those, which would fail here...
        {
            Identifier THE_HASH;

            if (theReply.m_strInboxHash.Exists()) {
                THE_HASH.SetString(theReply.m_strInboxHash);

                const bool bHash = pNym->SetInboxHash(str_acct_id, THE_HASH);

                if (!bHash)
                    otErr << __FUNCTION__
                          << ": Failed setting InboxHash on Nym "
                             "for account: " << str_acct_id << "\n";
                else {
                    Nym* pSignerNym = pNym;
                    pNym->SaveSignedNymfile(*pSignerNym);
                }
            }

            // If I have Transaction #35 signed out, and I use it to
            // start a market offer (or any other cron item)
            // then it's always possible that a finalReceipt will
            // pop into my Inbox while I'm asleep, closing
            // that transaction #. The server officially believes 35
            // is closed. Unfortunately, I still have it signed
            // out, on my side anyway, because I didn't know the
            // finalReceipt came in.
            //
            // THEREFORE, WHEN A FINAL RECEIPT COMES IN, I NEED TO
            // REMOVE ITS "in reference to" NUMBER FROM MY
            // ISSUED LIST. Here is clearly the best place for that:
            //
            for (auto& it : theInbox.GetTransactionMap()) {
                OTTransaction* pTempTrans = it.second;
                OT_ASSERT(nullptr != pTempTrans);

                // TODO security: Keep a client-side list of issued
                // #s for finalReceipts. That way,
                // I'll be smart enough here not to actually remove
                // just any number, unless it's actually
                // on my list of final receipts.  (The server does a
                // similar thing already.)
                //
                if (OTTransaction::finalReceipt == pTempTrans->GetType()) {
                    otInfo << "*** Removing opening issued number ("
                           << pTempTrans->GetReferenceToNum()
                           << "), since finalReceipt found when "
                              "retrieving asset account inbox. "
                              "***\n";

                    if (pNym->RemoveIssuedNum(*pNym, strNotaryID,
                                              pTempTrans->GetReferenceToNum(),
                                              true)) // bool bSave=true
                        otWarn << "**** Due to finding a "
                                  "finalReceipt, REMOVING OPENING "
                                  "NUMBER FROM NYM:  "
                               << pTempTrans->GetReferenceToNum() << " \n";
                    else
                        otWarn << "**** Noticed a finalReceipt, "
                                  "but Opening Number "
                               << pTempTrans->GetReferenceToNum()
                               << " had ALREADY been removed from "
                                  "nym. \n";

                    //                              pNym->RemoveIssuedNum(*pNym,
                    // strNotaryID, pTempTrans->GetReferenceToNum(),
                    // true); // bSave = true;
                    // The client side keeps a list of active
                    // (recurring) transactions.
                    // That is, smart contracts and payment plans. I
                    // don't think it keeps
                    // market offers in that list, since we already
                    // have a list of active
                    // market offers separately. And market offers
                    // produce final receipts,
                    // so basically this piece of code will be
                    // executed for all final receipts.
                    // It's not really necessary that it be called
                    // for market offers, but whatever.
                    // It is for the others.
                    //
                    OTCronItem::EraseActiveCronReceipt(
                        pTempTrans->GetReferenceToNum(), pNym->GetConstID(),
                        pTempTrans->GetPurportedNotaryID());

                } // We also do this in AcceptEntireNymbox
            }

            // Now I'm keeping the server signature, and just adding
            // my own.
            theInbox.ReleaseSignatures(); // This is back. Why?
                                          // Because we have
                                          // receipts functional
                                          // now.
            theInbox.SignContract(*pNym);
            theInbox.SaveContract();
            theInbox.SaveInbox();
        }
        else {
            otErr << __FUNCTION__
                  << ": Error loading (from string) or verifying "
                     "inbox:\n\n" << strInbox << "\n";
        }
    }
    if (strOutbox.Exists()) {
        // Load the ledger object from strOutbox.
        Ledger theOutbox(NYM_ID, ACCOUNT_ID, NOTARY_ID);

        // I receive the outbox, verify the server's signature, then
        // RE-SIGN IT WITH MY OWN
        // SIGNATURE, then SAVE it to local storage.  So any FUTURE
        // checks of this outbox
        // would require MY signature, not the server's, to verify.
        // But in this one spot,
        // just before saving, I need to verify the server's first.
        // UPDATE: keeping the server's signature, and just adding
        // my own.
        if (theOutbox.LoadOutboxFromString(strOutbox) &&
            theOutbox.VerifySignature(*pServerNym)) // No point calling
                                                    // VerifyAccount
        // since the client hasn't even had a
        // chance to download the box receipts
        // yet...
        {
            Identifier THE_HASH;

            if (theReply.m_strOutboxHash.Exists()) {
                THE_HASH.SetString(theReply.m_strOutboxHash);

                const bool bHash = pNym->SetOutboxHash(str_acct_id, THE_HASH);

                if (!bHash)
                    otErr << __FUNCTION__
                          << ": Failed setting OutboxHash on Nym "
                             "for account: " << str_acct_id << "\n";
                else {
                    Nym* pSignerNym = pNym;
                    pNym->SaveSignedNymfile(*pSignerNym);
                }
            }
            theOutbox.ReleaseSignatures(); // UPDATE: keeping the
                                           // server's signature,
                                           // and just adding my
                                           // own.
            theOutbox.SignContract(*pNym); // ANOTHER UPDATE:
                                           // Removing signature
                                           // again, since we have
                                           // receipts functional
                                           // now.
            theOutbox.SaveContract();
            theOutbox.SaveOutbox();
        }
        else {
            otErr << __FUNCTION__
                  << ": Error loading (from string) or verifying "
                     "outbox:\n\n" << strOutbox << "\n";
        }
    }

    return true;
}

bool OTClient::processServerReplyGetInstrumentDefinition(
    const Message& theReply, ProcessServerReplyArgs& args)
{
    // base64-Decode the server reply's payload into strContract
    String strContract(theReply.m_ascPayload);

    String strFoldername(OTFolders::Contract().Get());
    String strFilename; // In this case the filename isn't actually used,
                        // since SaveToContractFolder will
    // handle setting up the filename and overwrite it anyway. But I still
    // prefer to set it
    // up correctly, rather than pass a blank. I'm just funny like that.
    strFilename = theReply.m_strInstrumentDefinitionID.Get();

    AssetContract* pContract =
        new AssetContract(theReply.m_strInstrumentDefinitionID, strFoldername,
                          strFilename, theReply.m_strInstrumentDefinitionID);

    OT_ASSERT(nullptr != pContract);

    // Check the server signature on the contract here. (Perhaps the message
    // is good enough?
    // After all, the message IS signed by the server and contains the
    // Account.
    if (pContract->LoadContractFromString(strContract) &&
        pContract->VerifyContract()) {

        m_pWallet->AddAssetContract(*pContract);
        pContract = nullptr; // Success. The wallet "owns" it now, no need
                             // to clean it up.
    }

    if (pContract) {
        delete pContract;
        pContract = nullptr;
    }
    return true;
}

bool OTClient::processServerReplyGetMint(const Message& theReply)
{
    // base64-Decode the server reply's payload into strMint
    String strMint(theReply.m_ascPayload);
    // Load the mint object from that string...
    std::unique_ptr<Mint> pMint(Mint::MintFactory(
        theReply.m_strNotaryID, theReply.m_strInstrumentDefinitionID));
    OT_ASSERT(nullptr != pMint);
    // TODO check the server signature on the mint here...
    if (pMint->LoadContractFromString(strMint)) {
        otOut << "Saving mint file to disk...\n";
        pMint->SaveMint();
    }
    return true;
}

bool OTClient::processServerReplyGetMarketList(const Message& theReply)
{
    String strMarketDatafile;
    strMarketDatafile.Format("%s", "market_data.bin");

    OTDB::Storage* pStorage = OTDB::GetDefaultStorage();
    OT_ASSERT(nullptr != pStorage);

    // The reply is a SUCCESS, and the COUNT is 0 (empty list was returned.)
    // Since it was a success, but the list was empty, then we need to erase
    // the data file. (So when the file is loaded from storage, it will
    // correctly
    // display an empty list on the screen, instead of a list of outdated
    // items.)
    //
    if (theReply.m_lDepth == 0) {
        bool bSuccessErase = pStorage->EraseValueByKey(
            OTFolders::Market().Get(),    // "markets"
            theReply.m_strNotaryID.Get(), // "markets/<notaryID>"
            strMarketDatafile.Get()); // "markets/<notaryID>/market_data.bin"
        if (!bSuccessErase)
            otErr << "Error erasing market list from market folder: "
                  << strMarketDatafile << " \n";

        return true;
    }

    OTData thePayload;

    if ((theReply.m_ascPayload.GetLength() <= 2) ||
        (false == theReply.m_ascPayload.GetData(thePayload))) {
        otErr << "ProcessServerReply: unable to decode ascii-armored "
                 "payload in getMarketListResponse reply.\n";
        return true;
    }

    // Unpack the market list...

    OTDB::OTPacker* pPacker =
        pStorage->GetPacker(); // No need to check for failure, since this
                               // already ASSERTS. No need to cleanup
                               // either.

    std::unique_ptr<OTDB::PackedBuffer> pBuffer(pPacker->CreateBuffer());
    OT_ASSERT(nullptr != pBuffer);

    pBuffer->SetData(static_cast<const uint8_t*>(thePayload.GetPointer()),
                     thePayload.GetSize());

    std::unique_ptr<OTDB::MarketList> pMarketList(
        dynamic_cast<OTDB::MarketList*>(
            OTDB::CreateObject(OTDB::STORED_OBJ_MARKET_LIST)));

    bool bUnpacked = pPacker->Unpack(*pBuffer, *pMarketList);

    if (!bUnpacked) {
        otErr << "Process Server Reply: Failed unpacking data for "
                 "getMarketListResponse.\n";
        return true;
    }

    bool bSuccessStore = pStorage->StoreObject(
        *pMarketList, OTFolders::Market().Get(), // "markets"
        theReply.m_strNotaryID.Get(),            // "markets/<notaryID>"
        strMarketDatafile.Get()); // "markets/<notaryID>/market_data.bin"
    if (!bSuccessStore)
        otErr << "Error storing market list to market folder: "
              << strMarketDatafile << " \n";

    return true;
}

bool OTClient::processServerReplyGetMarketOffers(const Message& theReply)
{

    const String& strMarketID = theReply.m_strNymID2; // market ID stored here.

    String strOfferDatafile;
    strOfferDatafile.Format("%s.bin", strMarketID.Get());

    OTDB::Storage* pStorage = OTDB::GetDefaultStorage();
    OT_ASSERT(nullptr != pStorage);

    // The reply is a SUCCESS, and the COUNT is 0 (empty list was returned.)
    // Since it was a success, but the list was empty, then we need to erase
    // the data file. (So when the file is loaded from storage, it will
    // correctly
    // display an empty list on the screen, instead of a list of outdated
    // items.)
    //
    if (theReply.m_lDepth == 0) {
        bool bSuccessErase = pStorage->EraseValueByKey(
            OTFolders::Market().Get(),    // "markets"
            theReply.m_strNotaryID.Get(), // "markets/<notaryID>offers", //
                                          // "markets/<notaryID>/offers"
                                          // // todo stop
                                          // hardcoding.
            strOfferDatafile
                .Get()); // "markets/<notaryID>/offers/<marketID>.bin"
        if (!bSuccessErase)
            otErr << "Error erasing offers list from market folder: "
                  << strOfferDatafile << " \n";

        return true;
    }

    OTData thePayload;

    if ((theReply.m_ascPayload.GetLength() <= 2) ||
        (false == theReply.m_ascPayload.GetData(thePayload))) {
        otErr << "ProcessServerReply: unable to decode ascii-armored "
                 "payload in getMarketOffersResponse reply.\n";
        return true;
    }

    // Unpack the market list...

    OTDB::OTPacker* pPacker =
        pStorage->GetPacker(); // No need to check for failure, since this
                               // already ASSERTS. No need to cleanup
                               // either.

    std::unique_ptr<OTDB::PackedBuffer> pBuffer(pPacker->CreateBuffer());
    OT_ASSERT(nullptr != pBuffer);

    pBuffer->SetData(static_cast<const uint8_t*>(thePayload.GetPointer()),
                     thePayload.GetSize());

    std::unique_ptr<OTDB::OfferListMarket> pOfferList(
        dynamic_cast<OTDB::OfferListMarket*>(
            OTDB::CreateObject(OTDB::STORED_OBJ_OFFER_LIST_MARKET)));

    bool bUnpacked = pPacker->Unpack(*pBuffer, *pOfferList);

    if (!bUnpacked) {
        otErr << "Failed unpacking data for process server reply, "
                 "getMarketOffersResponse.\n";
        return true;
    }

    bool bSuccessStore = pStorage->StoreObject(
        *pOfferList, OTFolders::Market().Get(), // "markets"
        theReply.m_strNotaryID.Get(), // "markets/<notaryID>offers", //
                                      // "markets/<notaryID>/offers"   //
                                      // todo stop hardcoding.
        strOfferDatafile.Get()); // "markets/<notaryID>/offers/<marketID>.bin"
    if (!bSuccessStore)
        otErr << "Error storing " << strOfferDatafile << " to market folder.\n";

    return true;
}

bool OTClient::processServerReplyGetMarketRecentTrades(const Message& theReply)
{
    const String& strMarketID = theReply.m_strNymID2; // market ID stored here.

    String strTradeDatafile;
    strTradeDatafile.Format("%s.bin", strMarketID.Get());

    OTDB::Storage* pStorage = OTDB::GetDefaultStorage();
    OT_ASSERT(nullptr != pStorage);

    // The reply is a SUCCESS, and the COUNT is 0 (empty list was returned.)
    // Since it was a success, but the list was empty, then we need to erase
    // the data file. (So when the file is loaded from storage, it will
    // correctly
    // display an empty list on the screen, instead of a list of outdated
    // items.)
    //
    if (theReply.m_lDepth == 0) {
        bool bSuccessErase = pStorage->EraseValueByKey(
            OTFolders::Market().Get(),    // "markets"
            theReply.m_strNotaryID.Get(), // "markets/<notaryID>recent", //
                                          // "markets/<notaryID>/recent"
                                          // // todo stop
                                          // hardcoding.
            strTradeDatafile
                .Get()); // "markets/<notaryID>/recent/<marketID>.bin"
        if (!bSuccessErase)
            otErr << "Error erasing recent trades list from market folder: "
                  << strTradeDatafile << " \n";

        return true;
    }

    OTData thePayload;

    if ((theReply.m_ascPayload.GetLength() <= 2) ||
        (false == theReply.m_ascPayload.GetData(thePayload))) {
        otErr << "ProcessServerReply: unable to decode ascii-armored "
                 "payload in getMarketRecentTradesResponse reply.\n";
        return true;
    }

    // Unpack the market list...

    OTDB::OTPacker* pPacker =
        pStorage->GetPacker(); // No need to check for failure, since this
                               // already ASSERTS. No need to cleanup
                               // either.

    std::unique_ptr<OTDB::PackedBuffer> pBuffer(pPacker->CreateBuffer());
    OT_ASSERT(nullptr != pBuffer);

    pBuffer->SetData(static_cast<const uint8_t*>(thePayload.GetPointer()),
                     thePayload.GetSize());

    std::unique_ptr<OTDB::TradeListMarket> pTradeList(
        dynamic_cast<OTDB::TradeListMarket*>(
            OTDB::CreateObject(OTDB::STORED_OBJ_TRADE_LIST_MARKET)));

    bool bUnpacked = pPacker->Unpack(*pBuffer, *pTradeList);

    if (!bUnpacked) {
        otErr << "Failed unpacking data for process server reply, "
                 "getMarketRecentTradesResponse.\n";
        return true;
    }

    bool bSuccessStore = pStorage->StoreObject(
        *pTradeList, OTFolders::Market().Get(), // "markets"
        theReply.m_strNotaryID.Get(), // "markets/<notaryID>recent", //
                                      // "markets/<notaryID>/recent"   //
                                      // todo stop hardcoding.
        strTradeDatafile.Get()); // "markets/<notaryID>/recent/<marketID>.bin"
    if (!bSuccessStore)
        otErr << "Error storing " << strTradeDatafile << " to market folder.\n";

    return true;
}

bool OTClient::processServerReplyGetNymMarketOffers(const Message& theReply)
{
    String strOfferDatafile;
    strOfferDatafile.Format("%s.bin", theReply.m_strNymID.Get());

    OTDB::Storage* pStorage = OTDB::GetDefaultStorage();
    OT_ASSERT(nullptr != pStorage);

    // The reply is a SUCCESS, and the COUNT is 0 (empty list was returned.)
    // Since it was a success, but the list was empty, then we need to erase
    // the data file. (So when the file is loaded from storage, it will
    // correctly
    // display an empty list on the screen, instead of a list of outdated
    // items.)
    //
    if (theReply.m_lDepth == 0) {
        bool bSuccessErase = pStorage->EraseValueByKey(
            OTFolders::Nym().Get(),       // "nyms"
            theReply.m_strNotaryID.Get(), // "nyms/<notaryID>offers", //
                                          // "nyms/<notaryID>/offers"   //
                                          // todo stop hardcoding.
            strOfferDatafile.Get()); // "nyms/<notaryID>/offers/<NymID>.bin"
        if (!bSuccessErase)
            otErr << "Error erasing offers list from nyms folder: "
                  << strOfferDatafile << " \n";

        return true;
    }

    OTData thePayload;

    if ((theReply.m_ascPayload.GetLength() <= 2) ||
        (false == theReply.m_ascPayload.GetData(thePayload))) {
        otErr << "ProcessServerReply: unable to decode ascii-armored "
                 "payload in getNymMarketOffersResponse reply.\n";
        return true;
    }

    // Unpack the nym's offer list...

    OTDB::OTPacker* pPacker =
        pStorage->GetPacker(); // No need to check for failure, since this
                               // already ASSERTS. No need to cleanup
                               // either.

    std::unique_ptr<OTDB::PackedBuffer> pBuffer(pPacker->CreateBuffer());
    OT_ASSERT(nullptr != pBuffer);

    pBuffer->SetData(static_cast<const uint8_t*>(thePayload.GetPointer()),
                     thePayload.GetSize());

    std::unique_ptr<OTDB::OfferListNym> pOfferList(
        dynamic_cast<OTDB::OfferListNym*>(
            OTDB::CreateObject(OTDB::STORED_OBJ_OFFER_LIST_NYM)));

    bool bUnpacked = pPacker->Unpack(*pBuffer, *pOfferList);

    if (!bUnpacked) {
        otErr << "Failed unpacking data for process server reply, "
                 "getNymMarketOffersResponse.\n";
        return true;
    }

    bool bSuccessStore = pStorage->StoreObject(
        *pOfferList, OTFolders::Nym().Get(), // "nyms"
        theReply.m_strNotaryID.Get(),        // "nyms/<notaryID>offers", //
        // "nyms/<notaryID>/offers"   // todo
        // stop hardcoding.
        strOfferDatafile.Get()); // "nyms/<notaryID>/offers/<NymID>.bin"
    if (!bSuccessStore)
        otErr << "Error storing " << strOfferDatafile << " to nyms folder.\n";

    return true;
}

bool OTClient::processServerReplyUnregisterNym(const Message& theReply,
                                               ProcessServerReplyArgs& args)
{
    const auto& pNym = args.pNym;
    const auto& NOTARY_ID = args.NOTARY_ID;

    String strOriginalMessage;
    if (theReply.m_ascInReferenceTo.Exists())
        theReply.m_ascInReferenceTo.GetString(strOriginalMessage);

    Message theOriginalMessage;

    const String strNotaryID(NOTARY_ID);

    if (strOriginalMessage.Exists() &&
        theOriginalMessage.LoadContractFromString(strOriginalMessage) &&
        theOriginalMessage.VerifySignature(*pNym) &&
        theOriginalMessage.m_strNymID.Compare(theReply.m_strNymID) &&
        theOriginalMessage.m_strCommand.Compare("unregisterNym")) {

        while (pNym->GetTransactionNumCount(NOTARY_ID) > 0) {
            int64_t lTemp = pNym->GetTransactionNum(NOTARY_ID, 0); // index 0
            pNym->RemoveTransactionNum(strNotaryID, lTemp); // doesn't save.
        }
        while (pNym->GetIssuedNumCount(NOTARY_ID) > 0) {
            int64_t lTemp = pNym->GetIssuedNum(NOTARY_ID, 0); // index 0
            pNym->RemoveIssuedNum(strNotaryID, lTemp);        // doesn't save.
        }
        pNym->UnRegisterAtServer(
            strNotaryID); // Remove request number for that server.

        // SAVE the updated Nym to local storage.
        //
        Nym& extraNym = *pNym;
        pNym->SaveSignedNymfile(extraNym);

        otOut << "Successfully DELETED Nym from Server: removed request "
                 "number, plus all issued and transaction numbers for Nym "
              << theReply.m_strNymID << " for Server " << strNotaryID << ".\n";
    }
    else
        otErr << "The server just for some reason tried to trick me into "
                 "erasing my issued and transaction numbers for Nym "
              << theReply.m_strNymID << ", Server " << strNotaryID << ".\n";

    return true;
}

bool OTClient::processServerReplyUnregisterAccount(const Message& theReply,
                                                   ProcessServerReplyArgs& args)
{
    const auto& NOTARY_ID = args.NOTARY_ID;
    const auto& pNym = args.pNym;

    String strOriginalMessage;
    if (theReply.m_ascInReferenceTo.Exists())
        theReply.m_ascInReferenceTo.GetString(strOriginalMessage);

    Message theOriginalMessage;

    const String strNotaryID(NOTARY_ID);

    if (strOriginalMessage.Exists() &&
        theOriginalMessage.LoadContractFromString(strOriginalMessage) &&
        theOriginalMessage.VerifySignature(*pNym) &&
        theOriginalMessage.m_strNymID.Compare(theReply.m_strNymID) &&
        theOriginalMessage.m_strAcctID.Compare(theReply.m_strAcctID) &&
        theOriginalMessage.m_strCommand.Compare("unregisterAccount")) {

        const Identifier theAccountID(theReply.m_strAcctID);

        Account* pDeletedAcct = m_pWallet->GetAccount(theAccountID);

        if (nullptr != pDeletedAcct) {
            pDeletedAcct->MarkForDeletion();
            pDeletedAcct->ReleaseSignatures();
            pDeletedAcct->SignContract(*pNym);
            pDeletedAcct->SaveContract();
            pDeletedAcct->SaveAccount();
            // (The account still exists in storage, but has been MARKED FOR
            // DELETION.)

            // Remove the account from the wallet:
            //
            if (m_pWallet->RemoveAccount(theAccountID)) {
                m_pWallet->SaveWallet();
            }
        }

        otOut << "Successfully DELETED Asset Acct " << theReply.m_strAcctID
              << " from Server: " << strNotaryID << ".\n";
    }
    else
        otErr << "The server just for some reason tried to trick me into "
                 "erasing my account " << theReply.m_strAcctID << " on Server "
              << strNotaryID << ".\n";

    return true;
}

bool OTClient::processServerReplyRegisterInstrumentDefinition(
    const Message& theReply, ProcessServerReplyArgs& args)
{
    const auto& ACCOUNT_ID = args.ACCOUNT_ID;
    const auto& NOTARY_ID = args.NOTARY_ID;
    const auto& NYM_ID = args.NYM_ID;
    const auto& pServerNym = args.pServerNym;
    const auto& pNym = args.pNym;
    if (theReply.m_ascPayload.GetLength()) {
        Account* pAccount = nullptr;

        // this decodes the ascii-armor payload where the new account file
        // is stored, and returns a normal string in strAcctContents.
        String strAcctContents(theReply.m_ascPayload);

        // TODO check return value
        pAccount = new Account(NYM_ID, ACCOUNT_ID, NOTARY_ID);

        if (pAccount->LoadContractFromString(strAcctContents) &&
            pAccount->VerifyAccount(*pServerNym)) {
            // (2) Sign the Account
            pAccount->SignContract(*pNym);
            pAccount->SaveContract();

            // (3) Save the Account to file
            pAccount->SaveAccount();

            // Need to consider other security considerations.
            // What if I wasn't EXPECTING a registerInstrumentDefinitionResponse
            // message?
            // Well actually, in that case, the server wouldn't have a
            // copy of my request to send back to me, would he? So I should
            // check that request to make sure it's good.
            // Also maybe should check to see if I was expecting this
            // message
            // in the first place.

            m_pWallet->AddAccount(*pAccount);
            m_pWallet->SaveWallet();

            return true;
        }
        else {
            delete pAccount;
            pAccount = nullptr;
        }
    }
    return false;
}

bool OTClient::processServerReplyRegisterAccount(const Message& theReply,
                                                 ProcessServerReplyArgs& args)
{
    const auto& ACCOUNT_ID = args.ACCOUNT_ID;
    const auto& NOTARY_ID = args.NOTARY_ID;
    const auto& NYM_ID = args.NYM_ID;
    const auto& pServerNym = args.pServerNym;
    const auto& pNym = args.pNym;
    if (theReply.m_ascPayload.GetLength()) {
        Account* pAccount = nullptr;

        // this decodes the ascii-armor payload where the new account file
        // is stored, and returns a normal string in strAcctContents.
        String strAcctContents(theReply.m_ascPayload);

        pAccount = new Account(NYM_ID, ACCOUNT_ID, NOTARY_ID);

        if (pAccount && pAccount->LoadContractFromString(strAcctContents) &&
            pAccount->VerifyAccount(*pServerNym)) {
            // (2) Sign the Account
            pAccount->ReleaseSignatures(); // So I don't get the annoying
                                           // failure to verify message from
                                           // the server's signature.
            // Will eventually end up keeping the signature, however, just
            // for reasons of proof.
            // UPDATE (above) we are releasing these now, for good, since
            // server's signature is not needed. Receipts are functional
            // now,
            pAccount->SignContract(*pNym); // and the last receipt IS signed
                                           // by the server, and it can be
                                           // used to verify the nym,
                                           // account, inbox, and outbox.
                                           // Nifty!
            pAccount->SaveContract();

            // (3) Save the Account to file
            pAccount->SaveAccount();

            // Need to consider other security considerations.
            // What if I wasn't EXPECTING a registerAccountResponse message?
            // Well actually, in that case, the server wouldn't have a
            // copy of my request to send back to me, would he? So I should
            // check that request to make sure it's good.
            // Also maybe should check to see if I was expecting this
            // message
            // in the first place.

            m_pWallet->AddAccount(*pAccount);
            m_pWallet->SaveWallet();

            return true;
        }
        else {
            delete pAccount;
            pAccount = nullptr;
        }
    }
    return false;
}

/// We have just received a message from the server.
/// Find out what it is and do the appropriate processing.
/// Perhaps we just tried to create an account -- this could be
/// our new account! Let's make sure we receive it and save it
/// to disk somewhere.
///
/// PS... The Client TAKES OWNERSHIP of this message (adding it
/// to a message buffer) and will store it until the buffer is
/// flushed, or until the messages are popped back off later for
/// processing by the client API.
/// THEREFORE -- theReply MUST be allocated on the heap, and is
/// only passed in as a reference here in order to make sure it's real.
///
/// returns true/false on whether or not the reply was actually
/// verified and processed, versus whether
///
bool OTClient::processServerReply(std::shared_ptr<Message> reply,
                                  Ledger* pNymbox) // IF the Nymbox
                                                   // is passed in,
                                                   // then use that
                                                   // one, where
                                                   // appropriate,
                                                   // instead of
                                                   // loading it
                                                   // internally.
{
    Message& theReply = *reply;
    OT_ASSERT(nullptr != m_pConnection);

    OTServerConnection& theConnection = *m_pConnection;

    ProcessServerReplyArgs args;
    args.ACCOUNT_ID = Identifier(theReply.m_strAcctID);
    theConnection.GetNotaryID(args.NOTARY_ID);
    args.pNym = theConnection.GetNym();
    args.NYM_ID = Identifier(*args.pNym);
    args.strNotaryID = args.NOTARY_ID;
    args.strNymID = args.NYM_ID;
    args.pServerNym = const_cast<Nym*>(
        theConnection.GetServerContract()->GetContractPublicNym());

    Nym* pNym = args.pNym;
    const String& strNotaryID = args.strNotaryID;
    const String& strNymID = args.strNymID;
    Nym* pServerNym = args.pServerNym;

    // Just like the server verifies all messages before processing them,
    // so does the client need to verify the signatures against each message
    // and verify the various contract IDs and signatures.
    if (!theReply.VerifySignature(*pServerNym)) {
        otErr << __FUNCTION__
              << ": Error: Server reply signature failed to verify.\n";
        return false;
    }
    Message* pSentMsg = GetMessageOutbuffer().GetSentMessage(
        theReply.m_strRequestNum.ToLong(), strNotaryID,
        strNymID); // doesn't delete.
    // We couldn't find it in the "sent message" outbuffer (todo: persist this
    // buffer on the Nym.)
    // That means we must have missed the original server reply, even though it
    // DID happen. Then we
    // downloaded the Nymbox to re-sync after that failure occurred, and found
    // the reply there, and
    // processed it--removing it from the sent messages outbuffer at the same
    // time, since it was now
    // definitely handled.
    // FINALLY the network comes through with the server reply, and here we are
    // trying to process it
    // twice? But this time, it's NOT in the sent buffer, because we already
    // processed it -- so we
    // discard it! (todo: in a nice future version, save all of these in the
    // recordbox or something.)
    //
    // Here's another plausible scenario:  You RECEIVE the server's reply
    // properly the first time, and
    // you process it. Of course, you STILL get the Nymbox copy of that same
    // message ("just in case") and
    // thus ProcessServerReply gets called a second time, again leading us to
    // this block of code right here...
    //
    if (nullptr == pSentMsg) //
    {
        const String strReply(theReply);
        otLog3 << __FUNCTION__
               << ": FYI: no record of server reply in sent messages buffer. "
                  "We must have already processed it, and then removed it, "
                  "earlier. (Discarding.) Reply message:\n\n" << strReply
               << "\n\n";
        return false;
    }
    // Below this point, we know we found the original sent message--still
    // cached as though its reply
    // hasn't been processed yet. We haven't processed it yet! We are now
    // supposedly, processing it for
    // the first and proper time! Therefore, let's remove it from the "sent
    // messages" outbuffer, so we
    // are able to tell, next time around, that this has already happened.
    // (After all, we don't want
    // the next FlushSentMessages call to claw back any transaction numbers when
    // we clearly had a proper
    // reply come through!)
    //
    const int64_t lReplyRequestNum = theReply.m_strRequestNum.ToLong();

    //  bool bRemoved =
    GetMessageOutbuffer().RemoveSentMessage(lReplyRequestNum, strNotaryID,
                                            strNymID); // deletes.
    bool bDirtyNym = false;

    // Similarly we keep a client side list of all the request numbers that we
    // KNOW we have
    // a server reply for. (Each ID is maintained until we see a mirror of it
    // appear in the server's
    // copy of that same list, and then we go ahead and remove it. This is
    // basically an optimization
    // trick that enables us to avoid downloading many box receipts -- the
    // replyNotices, specifically.)
    //
    if (pNym->AddAcknowledgedNum(strNotaryID,
                                 lReplyRequestNum)) // doesn't save (here).
    {
        bDirtyNym = true;
    }

    // Okay, we received a reply, so we added its request number to our list of
    // "replies we have definitely received."
    // But what about when the server sees that, and mirrors our list? It will
    // send its own list, containing that mirror.
    // Any number that appears there, can be removed from the local list
    // (confirmation is total by that point.)
    // Clearly the server KNOWS I saw his reply, since he copied my ack into his
    // ack mirror list. Therefore I have no
    // more reason to continue telling him that I got the reply -- he already
    // knows it!  So I can remove the number from
    // my ack list, which will cause the server to do the same to match, once he
    // gets my next message.
    //
    // So next step: Loop through the ack list on the server reply, and any
    // numbers there can be REMOVED from the local
    // list...
    //
    std::set<int64_t> numlist_ack_reply;
    if (theReply.m_AcknowledgedReplies.Output(
            numlist_ack_reply)) // returns false if the numlist was empty.
    {
        for (auto& it : numlist_ack_reply) {
            const int64_t lTempRequestNum = it;
            Nym* pSignerNym = pNym;

            if (pNym->RemoveAcknowledgedNum(*pSignerNym, strNotaryID,
                                            lTempRequestNum,
                                            false)) // bSave=false
                bDirtyNym = true;
        }
    }

    if (bDirtyNym) {
        Nym* pSignerNym = pNym;
        pNym->SaveSignedNymfile(*pSignerNym);
    }

    // Done:  Do a Get Sent Message based on request number. If we find the
    // sent message, then process the reply and Remove the sent message.
    // But if we do NOT find the sent message, then we must have processed it
    // already -- in which case discard it and return.

    // Here, the Client takes ownership of the message (so make sure it's
    // heap-allocated.)
    m_MessageBuffer.Push(reply);

    // Once that process is done, everything below that line, in this function,
    // will be able to assume there is a verified Nym available, and a Server
    // Contract,
    // and an asset contract where applicable, and an account where applicable.
    //
    // Until that code is written, I do not have those things available to me.
    //
    // Furthermore also need to verify the payloads...
    // If "Command Responding To" was not actually signed by me, and I wasn't
    // expecting the new account request, then I do NOT want to sign it.
    //
    // Also if the new account is not signed by the server, I don't want to sign
    // it either. Need to check for all these things. Right now just proof of
    // concept.

    // Also, assuming all the verification shit is done here, I will have the
    // Nym
    // Wait a second, I think I have the Nym already cause there's a pointer on
    // the server connection that was passed in here...

    if (!theReply.m_bSuccess) {
        return false;
    }
    if (theReply.m_strCommand.Compare("triggerClauseResponse")) {
        return processServerReplyTriggerClause(theReply, args);
    }
    if (theReply.m_strCommand.Compare("getRequestNumberResponse")) {
        return processServerReplyGetRequestNumber(theReply, args);
    }
    if (theReply.m_strCommand.Compare("checkNymResponse")) {
        return processServerReplyCheckNym(theReply, args);
    }
    if (theReply.m_strCommand.Compare("notarizeTransactionResponse")) {
        return processServerReplyNotarizeTransaction(theReply, args);
    }
    if (theReply.m_strCommand.Compare("getTransactionNumbersResponse")) {
        return processServerReplyGetTransactionNumbers(theReply, args);
    }
    if (theReply.m_strCommand.Compare("getNymboxResponse")) {
        return processServerReplyGetNymBox(theReply, pNymbox, args);
    }
    if (theReply.m_strCommand.Compare("getBoxReceiptResponse")) {
        return processServerReplyGetBoxReceipt(theReply, pNymbox, args);
    }
    if ((theReply.m_strCommand.Compare("processInboxResponse") ||
         theReply.m_strCommand.Compare("processNymboxResponse"))) {
        return processServerReplyProcessInbox(theReply, pNymbox, args);
    }
    if (theReply.m_strCommand.Compare("getAccountDataResponse")) // Replaces
                                                                 // getAccount,
                                                                 // getInbox,
                                                                 // and
                                                                 // getOutbox
    {
        return processServerReplyGetAccountData(theReply, pNymbox, args);
    }
    if (theReply.m_strCommand.Compare("getInstrumentDefinitionResponse")) {
        return processServerReplyGetInstrumentDefinition(theReply, args);
    }
    if (theReply.m_strCommand.Compare("getMintResponse")) {
        return processServerReplyGetMint(theReply);
    }
    if (theReply.m_strCommand.Compare("getMarketListResponse")) {
        return processServerReplyGetMarketList(theReply);
    }
    if (theReply.m_strCommand.Compare("getMarketOffersResponse")) {
        return processServerReplyGetMarketOffers(theReply);
    }
    if (theReply.m_strCommand.Compare("getMarketRecentTradesResponse")) {
        return processServerReplyGetMarketRecentTrades(theReply);
    }
    if (theReply.m_strCommand.Compare("getNymMarketOffersResponse")) {
        return processServerReplyGetNymMarketOffers(theReply);
    }
    if (theReply.m_strCommand.Compare("unregisterNymResponse")) {
        return processServerReplyUnregisterNym(theReply, args);
    }
    if (theReply.m_strCommand.Compare("unregisterAccountResponse")) {
        return processServerReplyUnregisterAccount(theReply, args);
    }
    if (theReply.m_strCommand.Compare("registerInstrumentDefinitionResponse")) {
        return processServerReplyRegisterInstrumentDefinition(theReply, args);
    }
    if (theReply.m_strCommand.Compare("registerAccountResponse")) {
        return processServerReplyRegisterAccount(theReply, args);
    }
    return false;
}

/// NOTE: the functionality of this function was largely moved to OT_API in
/// OpenTransactions.cpp. The rest of the switch cases below should also be
/// moved there, and this function eliminated.

/// This function sets up "theMessage" so that it is ready to be sent out to the
/// server.
/// If you want to set up a pingNotary command and send it to the server,
/// then
/// you just call this to get the OTMessage object all set up and ready to be
/// sent.
//
/// returns -1 if error, don't send message.
/// returns  0 if NO error, but still, don't send message.
/// returns 1 if message is sent but there's not request number
/// returns >0 for processInbox, containing the number that was there before
/// processing.
/// returns >0 for nearly everything else, containing the request number itself.
///
int32_t OTClient::ProcessUserCommand(
    OTClient::OT_CLIENT_CMD_TYPE requestedCommand, Message& theMessage,
    Nym& theNym,
    // OTAssetContract& theContract,
    const OTServerContract& theServer, const Account* pAccount,
    int64_t lTransactionAmount, AssetContract* pMyAssetContract,
    const Identifier* pHisNymID, const Identifier* pHisAcctID)
{
    // This is all preparatory work to get the various pieces of data together
    // -- only
    // then can we put those pieces into a message.
    Identifier CONTRACT_ID;
    String strNymID, strContractID, strNotaryID, strNymPublicKey, strAccountID;
    int64_t lRequestNumber = 0;

    theNym.GetIdentifier(strNymID);
    theServer.GetIdentifier(strNotaryID);

    const Identifier NOTARY_ID(strNotaryID);

    if (nullptr != pAccount) {
        pAccount->GetIdentifier(strAccountID);

        if (pAccount->GetPurportedNotaryID() != NOTARY_ID) {
            otErr << "OTClient::ProcessUserCommand: "
                     "pAccount->GetPurportedNotaryID() doesn't match "
                     "NOTARY_ID.\n(Try adding:  --server NOTARY_ID)\n";

            return -1;
        }
    }

    int64_t lReturnValue = 0;

    switch (requestedCommand) {

    case (OTClient::pingNotary): {
        String strAuthentKey, strEncryptionKey;

        theNym.GetPublicAuthKey().GetPublicKey(strAuthentKey);
        theNym.GetPublicEncrKey().GetPublicKey(strEncryptionKey);

        // (1) set up member variables
        theMessage.m_strCommand = "pingNotary";
        theMessage.m_strNymID = strNymID; // Not expected to verify in any way
                                          // (for this message.) Just mirrored
                                          // back in the reply.
        theMessage.m_strNotaryID = strNotaryID;
        theMessage.m_strNymPublicKey =
            strAuthentKey; // Authentication public key for this Nym. (That he's
                           // signing this message with...)
        theMessage.m_strNymID2 = strEncryptionKey; // Encryption public key for
                                                   // this Nym (to send an
                                                   // encrypted reply back.)

        theMessage.m_strRequestNum.Format(
            "%d", 1); // Request Number, if unused, should be set to 1.

        // (2) Sign the Message
        // When a message is signed, it updates its m_xmlUnsigned contents to
        // the values in the member variables
        theMessage.SignContract(theNym);

        // (3) Save the Message (with signatures and all, back to its internal
        // member m_strRawFile.)
        //
        // FYI, SaveContract takes m_xmlUnsigned and wraps it with the
        // signatures and ------- BEGIN  bookends
        // If you don't pass a string in, then SaveContract saves the new
        // version to its member, m_strRawFile
        theMessage.SaveContract();

        lReturnValue = 1;

    } break;

    case (OTClient::registerNym): {
        String strCredList;
        String::Map theMap;

        // Credentials exist already.
        if (theNym.GetMasterCredentialCount() > 0) {
            theNym.GetPublicCredentials(strCredList, &theMap);
        }
        else // No credentials? Create them, then.
        {
            String strMasterCredID;
            const bool bAddedMaster =
                theNym.AddNewMasterCredential(strMasterCredID);

            if (bAddedMaster && strMasterCredID.Exists() &&
                (theNym.GetMasterCredentialCount() > 0)) {
                otOut << __FUNCTION__
                      << ": Adding new keyCredential to master credential: "
                      << strMasterCredID << "\n";

                const Identifier theMasterCredID(strMasterCredID);

                const bool bAddedSubkey = theNym.AddNewSubkey(theMasterCredID);

                if (bAddedSubkey) {
                    theNym.SaveCredentialList();
                    theNym.GetPublicCredentials(strCredList, &theMap);
                }
                else
                    otErr << __FUNCTION__ << ": Failed trying to add new "
                                             "keyCredential to new Master "
                                             "credential.\n";
            }
            else
                otErr << __FUNCTION__ << ": Failed trying to add new "
                                         "master credential (for Nym who "
                                         "doesn't have one yet.)\n";
        }

        // Won't bother if there are zero credentials somehow.
        if (strCredList.Exists() && (!theMap.empty())) {
            theMessage.m_ascPayload.SetString(strCredList);
            theMessage.credentials.swap(theMap);
        }
        if (!theMessage.m_ascPayload.Exists() ||
            theMessage.credentials.empty()) {
            otErr << __FUNCTION__ << ": Failed trying to assemble a "
                                     "registerNym message: This Nym has "
                                     "no credentials to use for registration. "
                                     "Convert this Nym first to the new "
                                     "credential system, then try again.\n";
        }
        else {
            // (1) set up member variables
            theMessage.m_strCommand = "registerNym";
            theMessage.m_strNymID = strNymID;
            theMessage.m_strNotaryID = strNotaryID;

            //          theNym.GetPublicKey().GetPublicKey(strNymPublicKey);
            //          theMessage.m_strNymPublicKey    = strNymPublicKey; //
            // Deprecated. (Credentials are new.)

            // THIS APPEARS SLIGHTLY ABOVE. Just leaving as a comment
            // here so it's not forgotten that this is also happening.
            //
            //          theMessage.m_ascPayload.SetString(strCredList);   //
            // <========== Success
            //          theMessage.m_ascPayload2.Set(str_Encoded.c_str());  //
            // Payload contains credentials list, payload2 contains actual
            // credentials.

            theMessage.m_strRequestNum.Format(
                "%d", 1); // Request Number, if unused, should be set to 1.

            // (2) Sign the Message
            theMessage.SignContract(theNym);

            // (3) Save the Message (with signatures and all, back to its
            // internal member m_strRawFile.)
            theMessage.SaveContract();

            lReturnValue = 1;
        }
    } break;
    case (OTClient::getRequestNumber): {
        //        otOut << "(User has instructed to send a getRequestNumber
        //        command to
        // the server...)\n";

        // (1) set up member variables
        theMessage.m_strCommand = "getRequestNumber";
        theMessage.m_strNymID = strNymID;
        theMessage.m_strNotaryID = strNotaryID;

        theMessage.m_strRequestNum.Format(
            "%d", 1); // Request Number, if unused, should be set to 1.

        // (2) Sign the Message
        theMessage.SignContract(theNym);

        // (3) Save the Message (with signatures and all, back to its internal
        // member m_strRawFile.)
        theMessage.SaveContract();

        lReturnValue = 1;
    }

    // EVERY COMMAND BELOW THIS POINT (THEY ARE ALL OUTGOING TO THE SERVER) MUST
    // INCLUDE THE
    // CORRECT REQUEST NUMBER, OR BE REJECTED BY THE SERVER.
    //
    // The same commands must also increment the local counter of the request
    // number by calling theNym.IncrementRequestNum
    // Otherwise it will get out of sync, and future commands will start failing
    // (until it is resynchronized with
    // a getRequestNumber message to the server, which replies with the latest
    // number.
    // The code on this side that processes
    // that server reply is already smart enough to update the local nym's copy
    // of the request number when it is received.
    // In this way, the client becomes resynchronized and the next command will
    // work again. But it's better to increment the
    // counter properly.
    // PROPERLY == every time you actually get the request number from a nym and
    // use it to make a server request,
    // then you should therefore also increment that counter. If you call
    // GetCurrentRequestNum AND USE IT WITH THE SERVER,
    // then make sure you call IncrementRequestNum immediately after. Otherwise
    // future commands will start failing.
    //
    // This is all because the server requres a new request number (last one +1)
    // with each request. This is in
    // order to thwart would-be attackers who cannot break the crypto, but try
    // to capture encrypted messages and
    // send them to the server twice. Better that new requests requre new
    // request numbers :-)
    break;
    case OTClient::unregisterNym: {
        // (0) Set up the REQUEST NUMBER and then INCREMENT IT
        theNym.GetCurrentRequestNum(strNotaryID, lRequestNumber);
        theMessage.m_strRequestNum.Format(
            "%" PRId64 "", lRequestNumber); // Always have to send this.
        theNym.IncrementRequestNum(theNym, strNotaryID); // since I used it for
                                                         // a server request, I
                                                         // have to increment it

        // (1) set up member variables
        theMessage.m_strCommand = "unregisterNym";
        theMessage.m_strNymID = strNymID;
        theMessage.m_strNotaryID = strNotaryID;
        theMessage.SetAcknowledgments(theNym); // Must be called AFTER
                                               // theMessage.m_strNotaryID is
                                               // already set. (It uses it.)

        // (2) Sign the Message
        theMessage.SignContract(theNym);

        // (3) Save the Message (with signatures and all, back to its internal
        // member m_strRawFile.)
        theMessage.SaveContract();

        lReturnValue = lRequestNumber;
    } break;
    case OTClient::processNymbox: // PROCESS NYMBOX
    {
        // (0) Set up the REQUEST NUMBER and then INCREMENT IT
        theNym.GetCurrentRequestNum(strNotaryID, lRequestNumber);
        theMessage.m_strRequestNum.Format(
            "%" PRId64 "", lRequestNumber); // Always have to send this.
        theNym.IncrementRequestNum(theNym, strNotaryID); // since I used it for
                                                         // a server request, I
                                                         // have to increment it

        // (1) Set up member variables
        theMessage.m_strCommand = "processNymbox";
        theMessage.m_strNymID = strNymID;
        theMessage.m_strNotaryID = strNotaryID;
        theMessage.SetAcknowledgments(theNym); // Must be called AFTER
                                               // theMessage.m_strNotaryID is
                                               // already set. (It uses it.)

        Identifier EXISTING_NYMBOX_HASH;
        const std::string str_notary_id(strNotaryID.Get());

        const bool bSuccess =
            theNym.GetNymboxHash(str_notary_id, EXISTING_NYMBOX_HASH);

        if (bSuccess)
            EXISTING_NYMBOX_HASH.GetString(theMessage.m_strNymboxHash);

        // (2) Sign the Message
        theMessage.SignContract(theNym);

        // (3) Save the Message (with signatures and all, back to its internal
        // member m_strRawFile.)
        theMessage.SaveContract();

        lReturnValue = lRequestNumber;
    }

    // This is called by the user of the command line utility.
    //
    break;
    case OTClient::notarizePurse: // NOTARIZE PURSE (deposit)
    {
        String strFromAcct;

        if (nullptr == pAccount) {
            otOut << "Please enter an asset Account ID: ";
            // User input.
            // I need a from account
            strFromAcct.OTfgets(std::cin);

            if (strFromAcct.GetLength() < 2) return (-1);

            const Identifier ACCOUNT_ID(strFromAcct);

            if ((pAccount = m_pWallet->GetAccount(ACCOUNT_ID)) != nullptr) {
                pAccount->GetIdentifier(strFromAcct);
                CONTRACT_ID = pAccount->GetInstrumentDefinitionID();
                CONTRACT_ID.GetString(strContractID);
            }
            else if ((pAccount = m_pWallet->GetAccountPartialMatch(
                            strFromAcct.Get())) != nullptr) {
                pAccount->GetIdentifier(strFromAcct);
                CONTRACT_ID = pAccount->GetInstrumentDefinitionID();
                CONTRACT_ID.GetString(strContractID);
            }
            else {
                otErr << "Unable to deposit without an account. Try adding:  "
                         "--myacct ACCOUNT_ID\n";
                return (-1);
            }
        }
        else {
            pAccount->GetIdentifier(strFromAcct);
            CONTRACT_ID = pAccount->GetInstrumentDefinitionID();
            CONTRACT_ID.GetString(strContractID);
        }

        if (pAccount->GetPurportedNotaryID() != NOTARY_ID) {
            otErr << "OTClient::ProcessUserCommand: "
                     "pAccount->GetPurportedNotaryID() doesn't match "
                     "NOTARY_ID.\n(Try adding:  --server NOTARY_ID)\n";
            return (-1);
        }

        // "from acct" is the acct we are depositing this cash to. aka MyAcct.
        const Identifier ACCT_FROM_ID(strFromAcct), NYM_ID(theNym);

        Purse thePurse(NOTARY_ID, CONTRACT_ID);

        const Nym* pServerNym = theServer.GetContractPublicNym();

        Purse theSourcePurse(thePurse);

        String strInstrumentDefinitionID;

        // If no asset contract was passed in, then --mypurse was not specified.
        // Therefore,
        // we can get the purse from the user, and verify that it has the same
        // instrument definition ID
        // as pAccount does. (No need to ask for the type.)
        //
        // But if an asset contract WAS passed in, then we will assume (for now)
        // that the purse
        // from local storage will be used, of that same type, and thus no need
        // to ask for the type.
        //
        if (nullptr == pMyAssetContract) {
            String strSourcePurse;

            otOut << "Please enter a plaintext purse (of the same instrument "
                     "definition "
                     "as the account), \nand terminate with a ~ (tilde "
                     "character) on a new line:\n> ";
            char decode_buffer[200]; // Safe since we only read
                                     // sizeof(decode_buffer)-1

            do {
                decode_buffer[0] = 0; // Make it fresh.

                if (!fgets(decode_buffer, sizeof(decode_buffer) - 1, stdin) &&
                    (decode_buffer[0] != '~')) {
                    strSourcePurse.Concatenate("%s", decode_buffer);
                    otOut << "> ";
                }
                else {
                    break;
                }

            } while (decode_buffer[0] != '~');

            if (false ==
                theSourcePurse.LoadContractFromString(strSourcePurse)) {
                otOut << "Failure trying to load purse from string provided by "
                         "user.\n";
                return (-1);
            }

            // todo verify signature?

            theSourcePurse.GetInstrumentDefinitionID().GetString(
                strInstrumentDefinitionID);
        }
        else {
            pMyAssetContract->GetIdentifier(strInstrumentDefinitionID);

            bool bLoadedSourcePurse =
                theSourcePurse.LoadPurse(strNotaryID.Get(), strNymID.Get(),
                                         strInstrumentDefinitionID.Get());

            if (!bLoadedSourcePurse) {
                otOut << "Deposit purse: Failure trying to load purse from "
                         "local storage:\nServer " << strNotaryID << "  Nym "
                      << strNymID << "  Asset Type "
                      << strInstrumentDefinitionID << "\n";
                return (-1);
            }
            else
                otOut << "WARNING: This operation is very low-level. Once you "
                         "deposit the purse in local storage,\nyou need to "
                         "erase the purse file from local storage, since the "
                         "tokens within it are\nall spent. (Otherwise, when "
                         "you withdraw again, good tokens would be mixed in "
                         "with\nthe spent ones, and then you'll have to sit "
                         "there depositing them one-by-one, in order\nto sort "
                         "it all out.\n (So just use the GUI and save yourself "
                         "the trouble.)\n\nDeposit purse: using purse from "
                         "local storage.\n Server " << strNotaryID << "  Nym "
                      << strNymID << "  Asset Type "
                      << strInstrumentDefinitionID << "\n";

            theSourcePurse.GetInstrumentDefinitionID().GetString(
                strInstrumentDefinitionID);
        }

        // By this point, theSourcePurse is DEFINITELY good,
        // and strInstrumentDefinitionID contains its ID.
        const Identifier INSTRUMENT_DEFINITION_ID(strInstrumentDefinitionID);

        if (INSTRUMENT_DEFINITION_ID != CONTRACT_ID) {
            otOut << "Instrument Definition Id on purse didn't match "
                     "instrument definition id on account. "
                     "\nTry: --myacct ACCT_ID  (to specify a different "
                     "account.)\nTo use the purse in local storage, try:  "
                     "--mypurse INSTRUMENT_DEFINITION_ID\nFYI, if you PREFER "
                     "to provide "
                     "the purse from user input, OT *will* ask you to\ninput a "
                     "purse when doing this, just as long as --mypurse is NOT "
                     "provided. (And\nthat includes the defaultmypurse value "
                     "stored in ~/.ot/command-line-ot.opt)\n\n";
            return (-1);
        }

        // By this point, I have theSourcePurse loaded, whether from local
        // storage or from
        // the command-line, and I know that it has the same instrument
        // definition as
        // pAccount.
        //

        int64_t lStoredTransactionNumber = 0;
        bool bGotTransNum = false;

        std::unique_ptr<Ledger> pInbox(pAccount->LoadInbox(theNym));
        std::unique_ptr<Ledger> pOutbox(pAccount->LoadOutbox(theNym));

        if (nullptr == pInbox) {
            otOut << "Failed loading inbox!\n";
            break;
        }

        if (nullptr == pOutbox) {
            otOut << "Failed loading outbox!\n";
            break;
        }

        bGotTransNum = theNym.GetNextTransactionNum(theNym, strNotaryID,
                                                    lStoredTransactionNumber);
        if (!bGotTransNum) {
            otOut << "No Transaction Numbers were available. Try requesting "
                     "the server for a new one.\n";
            break;
        }

        if (!pServerNym) {
            // IF FAILED, ADD TRANSACTION NUMBER BACK TO LIST OF AVAILABLE
            // NUMBERS.
            theNym.AddTransactionNum(theNym, strNotaryID,
                                     lStoredTransactionNumber,
                                     true); // bSave=true
            break;
        }

        bool bSuccess = false;

        // Create a transaction
        OTTransaction* pTransaction = OTTransaction::GenerateTransaction(
            NYM_ID, ACCT_FROM_ID, NOTARY_ID, OTTransaction::deposit,
            lStoredTransactionNumber);

        // set up the transaction item (each transaction may have multiple
        // items...)
        Item* pItem =
            Item::CreateItemFromTransaction(*pTransaction, Item::deposit);

        String strNote("Deposit this cash, please!");
        pItem->SetNote(strNote);

        OTNym_or_SymmetricKey theNymAsOwner(theNym),
            theServerNymAsOwner(*pServerNym);

        while (!theSourcePurse.IsEmpty()) {
            std::unique_ptr<Token> pToken(theSourcePurse.Pop(theNym));

            if (pToken) {
                // TODO need 2-recipient envelopes. My request to the server is
                // encrypted to the server's nym,
                // but it should be encrypted to my Nym also, so both have
                // access to decrypt it.
                // TODO: AHH OR I could just put a copy of everything into a
                // SINGLE-recipient envelope made out to ME!!
                //

                // Now the token is ready, let's add it to a purse
                // By pushing theToken into thePurse with *pServerNym, I encrypt
                // it to pServerNym.
                // So now only the server Nym can decrypt that token and pop it
                // out of that purse.
                if (false ==
                    pToken->ReassignOwnership(theNymAsOwner,
                                              theServerNymAsOwner)) {
                    otErr << "Error re-assigning ownership of token (to "
                             "server.)\n";
                    bSuccess = false;
                    break;
                }
                else {
                    otLog3 << "Success re-assigning ownership of token (to "
                              "server.)\n";

                    bSuccess = true;

                    pToken->ReleaseSignatures();
                    pToken->SignContract(theNym);
                    pToken->SaveContract();

                    thePurse.Push(theServerNymAsOwner,
                                  *pToken); // <================

                    int64_t lTemp = pItem->GetAmount();
                    pItem->SetAmount(
                        lTemp +
                        pToken->GetDenomination()); // <==================
                }
            }
            else {
                otErr << "Error loading token from purse.\n";
                bSuccess = false;
                break;
            }
        }

        if (bSuccess) {
            thePurse.SignContract(theNym);
            thePurse.SaveContract(); // I think this one is unnecessary. UPDATE:
                                     // WRONG. It's necessary.

            // Save the purse into a string...
            String strPurse;
            thePurse.SaveContractRaw(strPurse);

            // Add the purse string as the attachment on the transaction item.
            pItem->SetAttachment(
                strPurse); // The purse is contained in the reference string.

            // sign the item
            pItem->SignContract(theNym);
            pItem->SaveContract();

            // the Transaction "owns" the item now and will handle cleaning it
            // up.
            pTransaction->AddItem(*pItem); // the Transaction's destructor will
                                           // cleanup the item. It "owns" it
                                           // now.

            // BALANCE AGREEMENT

            // pBalanceItem is signed and saved within this call. No need to do
            // that again.
            Item* pBalanceItem = pInbox->GenerateBalanceStatement(
                pItem->GetAmount(), *pTransaction, theNym, *pAccount, *pOutbox);

            if (nullptr !=
                pBalanceItem) // will never be nullptr. Will assert above
                              // before it gets here.
                pTransaction->AddItem(
                    *pBalanceItem); // Better not be nullptr...
                                    // message will fail...
                                    // But better check
                                    // anyway.

            // sign the transaction
            pTransaction->SignContract(theNym);
            pTransaction->SaveContract();

            // set up the ledger
            Ledger theLedger(NYM_ID, ACCT_FROM_ID, NOTARY_ID);
            theLedger.GenerateLedger(ACCT_FROM_ID, NOTARY_ID,
                                     Ledger::message); // bGenerateLedger
                                                       // defaults to false,
                                                       // which is correct.
            theLedger.AddTransaction(*pTransaction);   // now the ledger "owns"
            // and will handle cleaning
            // up the transaction.

            // sign the ledger
            theLedger.SignContract(theNym);
            theLedger.SaveContract();

            // extract the ledger in ascii-armored form... encoding...
            String strLedger(theLedger);
            OTASCIIArmor ascLedger(strLedger); // I can't pass strLedger into
                                               // this constructor because I
                                               // want to encode it

            // (0) Set up the REQUEST NUMBER and then INCREMENT IT
            theNym.GetCurrentRequestNum(strNotaryID, lRequestNumber);
            theMessage.m_strRequestNum.Format(
                "%" PRId64 "", lRequestNumber); // Always have to send this.
            theNym.IncrementRequestNum(theNym, strNotaryID); // since I used it
                                                             // for a server
                                                             // request, I have
                                                             // to increment it

            // (1) Set up member variables
            theMessage.m_strCommand = "notarizeTransaction";
            theMessage.m_strNymID = strNymID;
            theMessage.m_strNotaryID = strNotaryID;
            theMessage.SetAcknowledgments(theNym); // Must be called AFTER
                                                   // theMessage.m_strNotaryID
                                                   // is already set. (It uses
                                                   // it.)

            theMessage.m_strAcctID = strFromAcct;
            theMessage.m_ascPayload = ascLedger;

            Identifier NYMBOX_HASH;
            const std::string str_server(strNotaryID.Get());
            const bool bNymboxHash =
                theNym.GetNymboxHash(str_server, NYMBOX_HASH);

            if (bNymboxHash)
                NYMBOX_HASH.GetString(theMessage.m_strNymboxHash);
            else
                otErr << "Failed getting NymboxHash from Nym for server: "
                      << str_server << "\n";

            // (2) Sign the Message
            theMessage.SignContract(theNym);

            // (3) Save the Message (with signatures and all, back to its
            // internal member m_strRawFile.)
            theMessage.SaveContract();

            lReturnValue = lRequestNumber;
        } // bSuccess
        else {
            // IF FAILED, ADD TRANSACTION NUMBER BACK TO LIST OF AVAILABLE
            // NUMBERS.
            theNym.AddTransactionNum(theNym, strNotaryID,
                                     lStoredTransactionNumber,
                                     true); // bSave=true

            delete pItem;
            pItem = nullptr;
            delete pTransaction;
            pTransaction = nullptr;
        }
    } break;
    case OTClient::notarizeCheque: // DEPOSIT CHEQUE
    {
        String strFromAcct;

        if (nullptr == pAccount) {
            otOut << "Please enter an asset Account ID (to deposit to): ";
            // User input.
            // I need a from account
            strFromAcct.OTfgets(std::cin);

            if (strFromAcct.GetLength() < 2) return (-1);

            const Identifier ACCOUNT_ID(strFromAcct);

            if ((pAccount = m_pWallet->GetAccount(ACCOUNT_ID)) != nullptr) {
                pAccount->GetIdentifier(strFromAcct);
                CONTRACT_ID = pAccount->GetInstrumentDefinitionID();
                CONTRACT_ID.GetString(strContractID);
            }
            else if ((pAccount = m_pWallet->GetAccountPartialMatch(
                            strFromAcct.Get())) != nullptr) {
                pAccount->GetIdentifier(strFromAcct);
                CONTRACT_ID = pAccount->GetInstrumentDefinitionID();
                CONTRACT_ID.GetString(strContractID);
            }
            else {
                otErr << "Unable to deposit without an account. Try adding:  "
                         "--myacct ACCOUNT_ID\n";
                return (-1);
            }
        }
        else {
            pAccount->GetIdentifier(strFromAcct);
            CONTRACT_ID = pAccount->GetInstrumentDefinitionID();
            CONTRACT_ID.GetString(strContractID);
        }

        if (pAccount->GetPurportedNotaryID() != NOTARY_ID) {
            otErr << "OTClient::ProcessUserCommand: "
                     "pAccount->GetPurportedNotaryID() doesn't match "
                     "NOTARY_ID.\n(Try adding:  --server NOTARY_ID)\n";
            return (-1);
        }

        const Identifier ACCT_FROM_ID(strFromAcct), NYM_ID(theNym);

        Cheque theCheque(NOTARY_ID, CONTRACT_ID);

        otOut << "Please enter plaintext cheque, terminate with ~ on a new "
                 "line:\n> ";
        String strCheque;
        char decode_buffer[200]; // Safe since we only read
                                 // sizeof(decode_buffer) - 1

        do {
            decode_buffer[0] = 0; // Make sure it's starting out fresh.

            if (!fgets(decode_buffer, sizeof(decode_buffer) - 1, stdin) &&
                (decode_buffer[0] != '~')) {
                strCheque.Concatenate("%s", decode_buffer);
                otOut << "> ";
            }
            else {
                break;
            }

        } while (decode_buffer[0] != '~');

        int64_t lStoredTransactionNumber = 0;
        bool bGotTransNum = theNym.GetNextTransactionNum(
            theNym, strNotaryID, lStoredTransactionNumber);

        if (!bGotTransNum) {
            otOut << "No Transaction Numbers were available. Try requesting "
                     "the server for a new one.\n";
        }
        else if (theCheque.LoadContractFromString(strCheque)) {
            if (theCheque.HasRecipient() &&
                (theCheque.GetRecipientNymID() != NYM_ID)) {
                const String strRecipientNym(theCheque.GetRecipientNymID());
                otOut << "This cheque is made out to the Nym: "
                      << strRecipientNym
                      << " (and that is NOT you, so you can't deposit it!)\n "
                         "You are: " << strNymID << " \n";
                // IF FAILED, ADD TRANSACTION NUMBER BACK TO LIST OF AVAILABLE
                // NUMBERS.
                theNym.AddTransactionNum(theNym, strNotaryID,
                                         lStoredTransactionNumber,
                                         true); // bSave=true
            }
            else // the cheque is blank, or is made out to me.
            {
                // Create a transaction
                OTTransaction* pTransaction =
                    OTTransaction::GenerateTransaction(
                        NYM_ID, ACCT_FROM_ID, NOTARY_ID, OTTransaction::deposit,
                        lStoredTransactionNumber);

                // set up the transaction item (each transaction may have
                // multiple items...)
                Item* pItem = Item::CreateItemFromTransaction(
                    *pTransaction, Item::depositCheque);

                String strNote("Deposit this cheque, please!");
                pItem->SetNote(strNote);

                strCheque.Release();
                theCheque.SaveContractRaw(strCheque);

                // Add the cheque string as the attachment on the transaction
                // item.
                pItem->SetAttachment(strCheque); // The cheque is contained in
                                                 // the reference string.

                // sign the item
                pItem->SignContract(theNym);
                pItem->SaveContract();

                // the Transaction "owns" the item now and will handle cleaning
                // it up.
                pTransaction->AddItem(*pItem); // the Transaction's destructor
                                               // will cleanup the item. It
                                               // "owns" it now.

                std::unique_ptr<Ledger> pInbox(pAccount->LoadInbox(theNym));
                std::unique_ptr<Ledger> pOutbox(pAccount->LoadOutbox(theNym));

                if (nullptr == pInbox) {
                    otOut << "Failed loading inbox!\n";

                    // IF FAILED, ADD TRANSACTION NUMBER BACK TO LIST OF
                    // AVAILABLE NUMBERS.
                    theNym.AddTransactionNum(theNym, strNotaryID,
                                             lStoredTransactionNumber,
                                             true); // bSave=true
                }
                else if (nullptr == pOutbox) {
                    otOut << "Failed loading outbox!\n";

                    // IF FAILED, ADD TRANSACTION NUMBER BACK TO LIST OF
                    // AVAILABLE NUMBERS.
                    theNym.AddTransactionNum(theNym, strNotaryID,
                                             lStoredTransactionNumber,
                                             true); // bSave=true
                }
                else {
                    // BALANCE AGREEMENT

                    // pBalanceItem is signed and saved within this call. No
                    // need to do that again.
                    Item* pBalanceItem = pInbox->GenerateBalanceStatement(
                        theCheque.GetAmount(), *pTransaction, theNym, *pAccount,
                        *pOutbox);

                    if (nullptr !=
                        pBalanceItem) // will never be nullptr. Will assert
                                      // above before it gets here.
                        pTransaction->AddItem(*pBalanceItem); // Better not be
                    // nullptr... message
                    // will fail...
                    // But better
                    // check anyway.

                    // sign the transaction
                    pTransaction->SignContract(theNym);
                    pTransaction->SaveContract();

                    // set up the ledger
                    Ledger theLedger(NYM_ID, ACCT_FROM_ID, NOTARY_ID);
                    theLedger.GenerateLedger(
                        ACCT_FROM_ID, NOTARY_ID,
                        Ledger::message); // bGenerateLedger defaults to
                                          // false, which is correct.
                    theLedger.AddTransaction(*pTransaction); // now the ledger
                                                             // "owns" and will
                                                             // handle cleaning
                                                             // up the
                                                             // transaction.

                    // sign the ledger
                    theLedger.SignContract(theNym);
                    theLedger.SaveContract();

                    // extract the ledger in ascii-armored form... encoding...
                    String strLedger(theLedger);
                    OTASCIIArmor ascLedger(strLedger);

                    // (0) Set up the REQUEST NUMBER and then INCREMENT IT
                    theNym.GetCurrentRequestNum(strNotaryID, lRequestNumber);
                    theMessage.m_strRequestNum.Format(
                        "%" PRId64 "",
                        lRequestNumber); // Always have to send this.
                    theNym.IncrementRequestNum(
                        theNym, strNotaryID); // since I used it for a server
                                              // request, I have to increment it

                    // (1) Set up member variables
                    theMessage.m_strCommand = "notarizeTransaction";
                    theMessage.m_strNymID = strNymID;
                    theMessage.m_strNotaryID = strNotaryID;
                    theMessage.SetAcknowledgments(
                        theNym); // Must be called AFTER
                                 // theMessage.m_strNotaryID is already set. (It
                                 // uses it.)

                    theMessage.m_strAcctID = strFromAcct;
                    theMessage.m_ascPayload = ascLedger;

                    Identifier NYMBOX_HASH;
                    const std::string str_server(strNotaryID.Get());
                    const bool bNymboxHash =
                        theNym.GetNymboxHash(str_server, NYMBOX_HASH);

                    if (bNymboxHash)
                        NYMBOX_HASH.GetString(theMessage.m_strNymboxHash);
                    else
                        otErr
                            << "Failed getting NymboxHash from Nym for server: "
                            << str_server << "\n";

                    // (2) Sign the Message
                    theMessage.SignContract(theNym);

                    // (3) Save the Message (with signatures and all, back to
                    // its internal member m_strRawFile.)
                    theMessage.SaveContract();

                    lReturnValue = lRequestNumber;

                } // inbox/outbox loaded
            }     // the cheque is blank, or is made out to me
        }         // cheque loaded
        else      // cheque failed to load
        {
            // IF FAILED, ADD TRANSACTION NUMBER BACK TO LIST OF AVAILABLE
            // NUMBERS.
            theNym.AddTransactionNum(theNym, strNotaryID,
                                     lStoredTransactionNumber,
                                     true); // bSave=true
        }
    } break;
    case OTClient::getTransactionNumbers: // GET TRANSACTION NUM
    {
        // (0) Set up the REQUEST NUMBER and then INCREMENT IT
        theNym.GetCurrentRequestNum(strNotaryID, lRequestNumber);
        theMessage.m_strRequestNum.Format(
            "%" PRId64 "", lRequestNumber); // Always have to send this.
        theNym.IncrementRequestNum(theNym, strNotaryID); // since I used it for
                                                         // a server request, I
                                                         // have to increment it

        // (1) Set up member variables
        theMessage.m_strCommand = "getTransactionNumbers";
        theMessage.m_strNymID = strNymID;
        theMessage.m_strNotaryID = strNotaryID;
        theMessage.SetAcknowledgments(theNym); // Must be called AFTER
                                               // theMessage.m_strNotaryID is
                                               // already set. (It uses it.)

        Identifier NYMBOX_HASH;
        const std::string str_server(strNotaryID.Get());
        const bool bNymboxHash = theNym.GetNymboxHash(str_server, NYMBOX_HASH);

        if (bNymboxHash)
            NYMBOX_HASH.GetString(theMessage.m_strNymboxHash);
        else
            otErr << "Failed getting NymboxHash from Nym for server: "
                  << str_server << "\n";

        // (2) Sign the Message
        theMessage.SignContract(theNym);

        // (3) Save the Message (with signatures and all, back to its internal
        // member m_strRawFile.)
        theMessage.SaveContract();

        lReturnValue = lRequestNumber;
    } break;
    case OTClient::writeCheque: // Write a CHEQUE. (Sends no message to server.)
    {
        String strFromAcct;

        if (nullptr == pAccount) {
            otOut << "Please enter an Asset Account ID (to draw the cheque "
                     "from): ";
            // User input.
            // I need a from account
            strFromAcct.OTfgets(std::cin);

            if (strFromAcct.GetLength() < 2) return (-1);

            const Identifier ACCOUNT_ID(strFromAcct);

            if ((pAccount = m_pWallet->GetAccount(ACCOUNT_ID)) != nullptr) {
                pAccount->GetIdentifier(strFromAcct);
                CONTRACT_ID = pAccount->GetInstrumentDefinitionID();
                CONTRACT_ID.GetString(strContractID);
            }
            else if ((pAccount = m_pWallet->GetAccountPartialMatch(
                            strFromAcct.Get())) != nullptr) {
                pAccount->GetIdentifier(strFromAcct);
                CONTRACT_ID = pAccount->GetInstrumentDefinitionID();
                CONTRACT_ID.GetString(strContractID);
            }
            else {
                otErr << "Unable to write cheque without account to draw from. "
                         "On comand line, try adding:  --myacct ACCOUNT_ID\n";
                return (-1);
            }
        }
        else {
            pAccount->GetIdentifier(strFromAcct);
            CONTRACT_ID = pAccount->GetInstrumentDefinitionID();
            CONTRACT_ID.GetString(strContractID);
        }

        const Identifier ACCOUNT_ID(strFromAcct);

        if (pAccount->GetPurportedNotaryID() != NOTARY_ID) {
            otErr << "OTClient::ProcessUserCommand: "
                     "pAccount->GetPurportedNotaryID() doesn't match "
                     "NOTARY_ID.\n(Try adding:  --server NOTARY_ID)\n";
            return (-1);
        }

        String strRecipientNym;

        if (nullptr == pHisNymID) {
            otOut << "Enter Recipient's Nym ID (full ID -- no partials here.) "
                     "Blank IS allowed: ";

            // User input.
            // I need a from account
            strRecipientNym.OTfgets(std::cin);
        }
        else {
            pHisNymID->GetString(strRecipientNym);
        }

        // Todo add partial lookups here from wallet and/or address book.

        const Identifier MY_NYM_ID(theNym);

        const Identifier HIS_NYM_ID(strRecipientNym);

        String strAmount;
        if (0 == lTransactionAmount) {
            otOut << "Please enter an amount: ";
            // User input.
            // I need an amount
            strAmount.OTfgets(std::cin);
        }

        const int64_t lTotalAmount =
            (0 == lTransactionAmount)
                ? // If nothing was passed in, then use strAmount,
                (strAmount.Exists() ? strAmount.ToLong() : 0)
                : lTransactionAmount; // otherwise lTransactionAmount.

        // To write a cheque, we need to burn one of our transaction numbers.
        // (Presumably the wallet
        // is also storing a couple of these, since they are needed to perform
        // any transaction.)
        //
        // I don't have to contact the server to write a cheque -- as long as I
        // already have a transaction
        // number I can use to write it. Otherwise I'd have to ask the server to
        // send me one first.

        int64_t lTransactionNumber = 0;

        if (false ==
            theNym.GetNextTransactionNum(theNym, strNotaryID,
                                         lTransactionNumber)) {
            otOut << "Cheques are written offline, but you still need a "
                     "transaction number\n(and you have none, currently.) Try "
                     "using 'n' to request another transaction number.\n";
            return (-1);
        }

        Cheque theCheque(pAccount->GetRealNotaryID(),
                         pAccount->GetInstrumentDefinitionID());

        // Memo
        otOut << "Enter a memo for your check: ";
        String strChequeMemo;
        strChequeMemo.OTfgets(std::cin);

        // Valid date range (in seconds)
        otOut << " 6 minutes    ==      360 Seconds\n10 minutes    ==      600 "
                 "Seconds\n1 hour        ==     3600 Seconds\n1 day        ==  "
                 "  86400 Seconds\n30 days    ==  2592000 Seconds\n3 months    "
                 "==  7776000 Seconds\n6 months    == 15552000 Seconds\n\n";

        int64_t lExpirationInSeconds =
            OTTimeGetSecondsFromTime(OT_TIME_HOUR_IN_SECONDS);
        otOut << "How many seconds before cheque expires? (defaults to 1 hour: "
              << lExpirationInSeconds << "): ";
        String strTemp;
        strTemp.OTfgets(std::cin);

        if (strTemp.GetLength() > 1) lExpirationInSeconds = strTemp.ToLong();

        time64_t VALID_FROM =
            OTTimeGetCurrentTime(); // This time is set to TODAY NOW

        otOut << "Cheque may be cashed STARTING date (defaults to now, in "
                 "seconds) [" << VALID_FROM << "]: ";
        strTemp.Release();
        strTemp.OTfgets(std::cin);

        if (strTemp.GetLength() > 2)
            VALID_FROM = OTTimeGetTimeFromSeconds(strTemp.Get());

        const time64_t VALID_TO = OTTimeAddTimeInterval(
            VALID_FROM, lExpirationInSeconds); // now + 3600

        bool bIssueCheque = theCheque.IssueCheque(
            lTotalAmount, lTransactionNumber, VALID_FROM, VALID_TO, ACCOUNT_ID,
            MY_NYM_ID, strChequeMemo,
            (strRecipientNym.GetLength() > 2)
                ? &(HIS_NYM_ID)
                : nullptr); // blank cheques are allowed.

        if (bIssueCheque) {
            theCheque.SignContract(theNym);
            theCheque.SaveContract();

            String strCheque(theCheque);

            otOut << "\n\nOUTPUT (writeCheque):\n\n\n";
            // otOut actually goes to stderr, whereas the cout below is
            // actually sent to standard output.
            std::cout << strCheque << std::endl;
        }
        else {
            otOut << "Failed trying to issue the cheque!\n";

            // IF FAILED, ADD TRANSACTION NUMBER BACK TO LIST OF AVAILABLE
            // NUMBERS.
            theNym.AddTransactionNum(theNym, strNotaryID, lTransactionNumber,
                                     true); // bSave=true
        }

        return -1;
    } break;
    case OTClient::paymentPlan: // Activate a PAYMENT PLAN
    {
        const Identifier NYM_ID(theNym);

        OTPaymentPlan thePlan;

        otOut << "Please enter plaintext payment plan. Terminate with ~ on a "
                 "new line:\n> ";
        String strPlan;
        char decode_buffer[200]; // Safe since we only read
                                 // sizeof(decode_buffer)-1

        do {
            decode_buffer[0] = 0; // Make it fresh.

            if (!fgets(decode_buffer, sizeof(decode_buffer) - 1, stdin) &&
                (decode_buffer[0] != '~')) {
                strPlan.Concatenate("%s", decode_buffer);
                otOut << "> ";
            }
            else {
                break;
            }

        } while (decode_buffer[0] != '~');

        if (thePlan.LoadContractFromString(strPlan)) {
            const Identifier ACCOUNT_ID(thePlan.GetSenderAcctID());

            Account* pSenderAccount = m_pWallet->GetAccount(ACCOUNT_ID);

            if (nullptr == pSenderAccount) {
                otOut << "There is no account loaded on this wallet with that "
                         "sender acct ID, sorry.\n";
            }
            if ((nullptr != pAccount) && (pSenderAccount != pAccount)) {
                otOut << "This Payment Plan is already confirmed, yet now you "
                         "try to activate it, and you \nhave supplied a "
                         "different account ID than the one that originally "
                         "confirmed it.\nPerhaps it's just an unfortunate "
                         "default in your ~/.ot/command-line-ot.opt file.\nBe "
                         "explicit, and use:  --myacct <acct_id>\n";
            }
            else {
                String strFromAcct(ACCOUNT_ID);

                // Create a transaction
                OTTransaction* pTransaction =
                    OTTransaction::GenerateTransaction(
                        NYM_ID, ACCOUNT_ID, NOTARY_ID,
                        OTTransaction::paymentPlan,
                        thePlan.GetTransactionNum());

                // set up the transaction item (each transaction may have
                // multiple items...)
                Item* pItem = Item::CreateItemFromTransaction(
                    *pTransaction, Item::paymentPlan);

                strPlan.Release();
                thePlan.SaveContractRaw(strPlan);

                // Add the payment plan string as the attachment on the
                // transaction item.
                pItem->SetAttachment(strPlan); // The payment plan is contained
                                               // in the reference string.

                // sign the item
                pItem->SignContract(theNym);
                pItem->SaveContract();

                // the Transaction "owns" the item now and will handle cleaning
                // it up.
                pTransaction->AddItem(*pItem); // the Transaction's destructor
                                               // will cleanup the item. It
                                               // "owns" it now.

                // TRANSACTION AGREEMENT

                // pBalanceItem is signed and saved within this call. No need to
                // do that again.
                Item* pStatementItem =
                    theNym.GenerateTransactionStatement(*pTransaction);

                if (nullptr !=
                    pStatementItem) // will never be nullptr. Will assert
                                    // above before it gets here.
                    pTransaction->AddItem(*pStatementItem); // Better not be
                // nullptr... message
                // will fail... But
                // better check
                // anyway.

                // sign the transaction
                pTransaction->SignContract(theNym);
                pTransaction->SaveContract();

                // set up the ledger
                Ledger theLedger(NYM_ID, ACCOUNT_ID, NOTARY_ID);
                theLedger.GenerateLedger(ACCOUNT_ID, NOTARY_ID,
                                         Ledger::message); // bGenerateLedger
                                                           // defaults to
                                                           // false, which is
                                                           // correct.
                theLedger.AddTransaction(*pTransaction);   // now the ledger
                                                           // "owns" and will
                // handle cleaning up
                // the transaction.

                // sign the ledger
                theLedger.SignContract(theNym);
                theLedger.SaveContract();

                // extract the ledger in ascii-armored form... encoding...
                String strLedger(theLedger);
                OTASCIIArmor ascLedger(strLedger);

                // (0) Set up the REQUEST NUMBER and then INCREMENT IT
                theNym.GetCurrentRequestNum(strNotaryID, lRequestNumber);
                theMessage.m_strRequestNum.Format(
                    "%" PRId64 "", lRequestNumber); // Always have to send this.
                theNym.IncrementRequestNum(
                    theNym, strNotaryID); // since I used it for a server
                                          // request, I have to increment it

                // (1) Set up member variables
                theMessage.m_strCommand = "notarizeTransaction";
                theMessage.m_strNymID = strNymID;
                theMessage.m_strNotaryID = strNotaryID;
                theMessage.SetAcknowledgments(
                    theNym); // Must be called AFTER theMessage.m_strNotaryID is
                             // already set. (It uses it.)

                theMessage.m_strAcctID = strFromAcct;
                theMessage.m_ascPayload = ascLedger;

                Identifier NYMBOX_HASH;
                const std::string str_server(strNotaryID.Get());
                const bool bNymboxHash =
                    theNym.GetNymboxHash(str_server, NYMBOX_HASH);

                if (bNymboxHash)
                    NYMBOX_HASH.GetString(theMessage.m_strNymboxHash);
                else
                    otErr << "Failed getting NymboxHash from Nym for server: "
                          << str_server << "\n";

                // (2) Sign the Message
                theMessage.SignContract(theNym);

                // (3) Save the Message (with signatures and all, back to its
                // internal member m_strRawFile.)
                theMessage.SaveContract();

                lReturnValue = lRequestNumber;

            } // pAccount not nullptr
        }     // thePlan.LoadContractFromString()
        else {
            otOut << "Unable to load payment plan from string. Sorry.\n";
        }

    } break;

    default: {
        otOut << "\n";
    }
    }

    return static_cast<int32_t>(lReturnValue);
}

} // namespace opentxs
