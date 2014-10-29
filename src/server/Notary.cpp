/************************************************************
 *
 *  Notary.cpp
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

#include <opentxs/server/Notary.hpp>
#include <opentxs/server/OTServer.hpp>
#include <opentxs/server/Macros.hpp>
#include <opentxs/server/ServerSettings.hpp>
#include <opentxs/server/PayDividendVisitor.hpp>
#include <opentxs/ext/OTPayment.hpp>
#include <opentxs/cash/Mint.hpp>
#include <opentxs/cash/Purse.hpp>
#include <opentxs/cash/Token.hpp>
#include <opentxs/basket/BasketItem.hpp>
#include <opentxs/basket/Basket.hpp>
#include <opentxs/core/script/OTSmartContract.hpp>
#include <opentxs/core/recurring/OTPaymentPlan.hpp>
#include <opentxs/core/crypto/OTNymOrSymmetricKey.hpp>
#include <opentxs/core/OTAssetContract.hpp>
#include <opentxs/core/OTCheque.hpp>
#include <opentxs/core/OTLedger.hpp>
#include <opentxs/core/OTAccount.hpp>
#include <opentxs/core/OTPseudonym.hpp>
#include <opentxs/core/OTTransaction.hpp>
#include <opentxs/core/OTString.hpp>
#include <opentxs/core/trade/OTOffer.hpp>
#include <opentxs/core/OTItem.hpp>
#include <opentxs/core/trade/OTTrade.hpp>
#include <opentxs/core/util/OTFolders.hpp>
#include <opentxs/core/OTLog.hpp>
#include <deque>
#include <memory>
#include <list>

namespace opentxs
{

typedef std::list<OTAccount*> listOfAccounts;
typedef std::deque<Token*> dequeOfTokenPtrs;

Notary::Notary(OTServer* server)
    : server_(server)
{
}

void Notary::NotarizeTransfer(OTPseudonym& theNym, OTAccount& theFromAccount,
                              OTTransaction& tranIn, OTTransaction& tranOut,
                              bool& bOutSuccess)
{
    // The outgoing transaction is an "atTransfer", that is, "a reply to the
    // transfer request"
    tranOut.SetType(OTTransaction::atTransfer);

    OTItem* pItem = nullptr;
    OTItem* pBalanceItem = nullptr;
    OTItem* pResponseItem = nullptr;
    OTItem* pResponseBalanceItem = nullptr;

    // The incoming transaction may be sent to inboxes and outboxes, and it
    // will probably be bundled in our reply to the user as well. Therefore,
    // let's grab it as a string.
    OTString strInReferenceTo;
    OTString strBalanceItem;

    // Grab the actual server ID from this object, and use it as the server ID
    // here.
    const OTIdentifier SERVER_ID(server_->m_strServerID), USER_ID(theNym),
        ACCOUNT_ID(theFromAccount),
        ASSET_TYPE_ID(theFromAccount.GetAssetTypeID());

    OTString strUserID(USER_ID), strAccountID(ACCOUNT_ID);
    pResponseBalanceItem =
        OTItem::CreateItemFromTransaction(tranOut, OTItem::atBalanceStatement);
    pResponseBalanceItem->SetStatus(OTItem::rejection); // the default.
    tranOut.AddItem(*pResponseBalanceItem); // the Transaction's destructor will
                                            // cleanup the item. It "owns" it
                                            // now.
    pResponseItem =
        OTItem::CreateItemFromTransaction(tranOut, OTItem::atTransfer);
    pResponseItem->SetStatus(OTItem::rejection); // the default.
    tranOut.AddItem(*pResponseItem); // the Transaction's destructor will
                                     // cleanup the item. It "owns" it now.

    if (false ==
        NYM_IS_ALLOWED(strUserID.Get(), ServerSettings::__transact_transfer)) {
        OTLog::vOutput(0, "Notary::NotarizeTransfer: User %s cannot do this "
                          "transaction (All acct-to-acct transfers are "
                          "disallowed in server.cfg)\n",
                       strUserID.Get());
    }
    else if (nullptr ==
               (pBalanceItem = tranIn.GetItem(OTItem::balanceStatement))) {
        OTString strTemp(tranIn);
        OTLog::vOutput(0, "Notary::NotarizeTransfer: Expected "
                          "OTItem::balanceStatement in trans# %lld: \n\n%s\n\n",
                       tranIn.GetTransactionNum(),
                       strTemp.Exists()
                           ? strTemp.Get()
                           : " (ERROR LOADING TRANSACTION INTO STRING) ");
    }
    // For now, there should only be one of these transfer items inside the
    // transaction.
    // So we treat it that way... I either get it successfully or not.
    else if (nullptr == (pItem = tranIn.GetItem(OTItem::transfer))) {
        OTString strTemp(tranIn);
        OTLog::vOutput(0, "Notary::NotarizeTransfer: Expected "
                          "OTItem::transfer in trans# %lld: \n\n%s\n\n",
                       tranIn.GetTransactionNum(),
                       strTemp.Exists()
                           ? strTemp.Get()
                           : " (ERROR LOADING TRANSACTION INTO STRING) ");
    }
    else if (ACCOUNT_ID == pItem->GetDestinationAcctID()) {
        OTString strTemp(tranIn);
        OTLog::vOutput(
            0,
            "Notary::NotarizeTransfer: Failed attempt by user %s in "
            "trans# %lld, to transfer money \"To the From Acct\": \n\n%s\n\n",
            strUserID.Get(), tranIn.GetTransactionNum(),
            strTemp.Exists() ? strTemp.Get()
                             : " (ERROR LOADING TRANSACTION INTO STRING) ");
    }
    else {
        // The response item, as well as the inbox and outbox items, will
        // contain a copy
        // of the request item. So I save it into a string here so they can all
        // grab a copy of it
        // into their "in reference to" fields.
        pItem->SaveContractRaw(strInReferenceTo);
        pBalanceItem->SaveContractRaw(strBalanceItem);

        // IDFromAccount is the ID on the "from" Account that was passed in.
        // IDItemFromAccount is the "from" account ID on the transaction Item we
        // are currently examining.
        // IDItemToAccount is the "to" account ID on the transaction item we are
        // currently examining.
        OTIdentifier IDFromAccount(theFromAccount);

        // Server response item being added to server response transaction
        // (tranOut)
        // They're getting SOME sort of response item.

        pResponseItem->SetReferenceString(strInReferenceTo); // the response
                                                             // item carries a
                                                             // copy of what
                                                             // it's responding
                                                             // to.
        pResponseItem->SetReferenceToNum(
            pItem->GetTransactionNum()); // This response item is IN RESPONSE to
                                         // pItem and its Owner Transaction.
        pResponseItem->SetNumberOfOrigin(*pItem);

        pResponseBalanceItem->SetReferenceString(
            strBalanceItem); // the response item carries a copy of what it's
                             // responding to.
        pResponseBalanceItem->SetReferenceToNum(
            pItem->GetTransactionNum()); // This response item is IN RESPONSE to
                                         // pItem and its Owner Transaction.
        pResponseBalanceItem->SetNumberOfOrigin(*pItem);

        // Set the ID on the To Account based on what the transaction request
        // said. (So we can load it up.)
        std::unique_ptr<OTAccount> pDestinationAcct(
            OTAccount::LoadExistingAccount(pItem->GetDestinationAcctID(),
                                           SERVER_ID));

        // Only accept transfers with positive amounts.
        if (0 > pItem->GetAmount()) {
            OTLog::Output(0, "Notary::NotarizeTransfer: Failure: Attempt to "
                             "transfer negative balance.\n");
        }

        // I'm using the operator== because it exists.
        // If the ID on the "from" account that was passed in,
        // does not match the "Acct From" ID on this transaction item
        else if (!(IDFromAccount == pItem->GetPurportedAccountID())) {
            OTLog::Output(0, "Notary::NotarizeTransfer: Error: 'From' "
                             "account ID on the transaction does not match "
                             "'from' account ID on the transaction item.\n");
        }
        // ok so the IDs match. Does the destination account exist?
        else if (nullptr == pDestinationAcct) {
            OTLog::Output(0, "Notary::NotarizeTransfer: ERROR verifying "
                             "existence of the 'to' account.\n");
        }
        // Is the destination a legitimate other user's acct, or is it just an
        // internal server account?
        // (That is, stash accounts, voucher accounts, basket accounts, etc are
        // only used internally,
        // and may not be recipients to user transfers...)
        //
        else if (pDestinationAcct->IsInternalServerAcct()) {
            OTLog::Output(0, "Notary::NotarizeTransfer: Failure: Destination "
                             "account is used internally by the server, and is "
                             "not a valid recipient for this transaction.\n");
        }
        // Are both of the accounts of the same Asset Type?
        else if (!(theFromAccount.GetAssetTypeID() ==
                   pDestinationAcct->GetAssetTypeID())) {
            OTString strFromAssetID(theFromAccount.GetAssetTypeID()),
                strDestinationAssetID(pDestinationAcct->GetAssetTypeID());
            OTLog::vOutput(
                0, "ERROR - user attempted to transfer between accounts of 2 "
                   "different "
                   "asset types in Notary::NotarizeTransfer:\n%s\n%s\n",
                strFromAssetID.Get(), strDestinationAssetID.Get());
        }
        // Does it verify?
        // I call VerifySignature here since VerifyContractID was already called
        // in LoadExistingAccount().
        else if (!pDestinationAcct->VerifySignature(server_->m_nymServer)) {
            OTLog::Output(0, "ERROR verifying signature on, the 'to' account "
                             "in Notary::NotarizeTransfer\n");
        }

        // This entire function can be divided into the top and bottom halves.
        // The top half is oriented around finding the "transfer" item (in the
        // "transfer" transaction)
        // and setting up the response item that will go into the response
        // transaction.
        // The bottom half is oriented, in the case of success, around creating
        // the necessary inbox
        // and outbox entries, and debiting the account, and basically
        // performing the actual transfer.
        else {
            // Okay then, everything checks out. Let's add this to the sender's
            // outbox and the recipient's inbox.
            // IF they can be loaded up from file, or generated, that is.

            // Load the inbox/outbox in case they already exist
            OTLedger theFromOutbox(USER_ID, IDFromAccount, SERVER_ID),
                theToInbox(pItem->GetDestinationAcctID(), SERVER_ID);

            bool bSuccessLoadingInbox = theToInbox.LoadInbox();
            bool bSuccessLoadingOutbox = theFromOutbox.LoadOutbox();
            // ...or generate them otherwise...

            // NOTE:
            // 1. Any normal user had his inbox created at the same time as his
            // asset account was created.
            // 2. If there is an error now, we don't necessarily just want to
            // re-create (and overwrite) that file.
            // 3. Therefore I do not generate the ledger for safety reasons, per
            // 2.
            // 4. Also, what if an attempt is being made to transfer to an
            // account that isn't SUPPOSED to have
            //    an inbox? For example, a server voucher account (where backing
            // funds for vouchers are stored) does
            //    not have an inbox, and should not be able to receive
            // transfers. In that case, we definitely don't want
            //    to just "generate" an inbox here! Instead, we want it to fail.
            // In fact, I'm adding a check, above, for
            //    the account type. In fact, I'm adding a new method to
            // OTAccount where we can just ask it, for each
            //    transaction type, whether it can even be used for that purpose
            // in the first place.
            //    Update: appears OTAccount::IsInternalServerAcct already
            // basically fits the bill.

            if (true == bSuccessLoadingInbox)
                bSuccessLoadingInbox =
                    theToInbox.VerifyAccount(server_->m_nymServer);
            else
                OTLog::Error(
                    "Notary::NotarizeTransfer: Error loading 'to' inbox.\n");

            if (true == bSuccessLoadingOutbox)
                bSuccessLoadingOutbox =
                    theFromOutbox.VerifyAccount(server_->m_nymServer);
            else
                OTLog::Error("Notary::NotarizeTransfer: Error loading 'from' "
                             "outbox.\n");

            std::unique_ptr<OTLedger> pInbox(
                theFromAccount.LoadInbox(server_->m_nymServer));
            std::unique_ptr<OTLedger> pOutbox(
                theFromAccount.LoadOutbox(server_->m_nymServer));

            if (nullptr == pInbox) {
                OTLog::Error("Error loading or verifying inbox.\n");
            }
            else if (nullptr == pOutbox) {
                OTLog::Error("Error loading or verifying outbox.\n");
            }
            else if (!bSuccessLoadingInbox ||
                       false == bSuccessLoadingOutbox) {
                OTLog::Error(
                    "ERROR generating ledger in Notary::NotarizeTransfer.\n");
            }
            else {
                // Generate new transaction number for these new transactions
                // todo check this generation for failure (can it fail?)
                int64_t lNewTransactionNumber = 0;

                server_->transactor_.issueNextTransactionNumber(
                    server_->m_nymServer, lNewTransactionNumber,
                    false); // bStoreTheNumber = false
                // I create TWO Outbox transactions -- one for the real outbox,
                // (theFromOutbox)
                // and one for pOutbox (used for verifying the balance
                // statement.)
                // pTEMPOutboxTransaction (here below) is that last one,
                // pOutbox.
                //
                OTTransaction* pTEMPOutboxTransaction =
                    OTTransaction::GenerateTransaction(*pOutbox,
                                                       OTTransaction::pending,
                                                       lNewTransactionNumber);
                OTTransaction* pOutboxTransaction =
                    OTTransaction::GenerateTransaction(theFromOutbox,
                                                       OTTransaction::pending,
                                                       lNewTransactionNumber);

                OTTransaction* pInboxTransaction =
                    OTTransaction::GenerateTransaction(theToInbox,
                                                       OTTransaction::pending,
                                                       lNewTransactionNumber);
                // UPDATE: I am now issuing one new transaction number above,
                // instead of two. This is to make it easy
                // for the two to cross-reference each other. Later if I want to
                // remove the transaction from the inbox
                // and need to know the corresponding transaction # for the
                // outbox, it will be the same number.

                // I have to set this one up just like the one below.
                pTEMPOutboxTransaction->SetReferenceString(strInReferenceTo);
                pTEMPOutboxTransaction->SetReferenceToNum(
                    pItem->GetTransactionNum());
                pTEMPOutboxTransaction->SetNumberOfOrigin(*pItem);
                // the new transactions store a record of the item they're
                // referring to.
                pOutboxTransaction->SetReferenceString(strInReferenceTo);
                pOutboxTransaction->SetReferenceToNum(
                    pItem->GetTransactionNum());
                pOutboxTransaction->SetNumberOfOrigin(*pItem);

                // todo put these two together in a method.
                pInboxTransaction->SetReferenceString(strInReferenceTo);
                pInboxTransaction->SetReferenceToNum(
                    pItem->GetTransactionNum());
                pInboxTransaction->SetNumberOfOrigin(*pItem);

                // Now we have created 2 new transactions from the server to the
                // users' boxes
                // Let's sign them and add to their inbox / outbox.
                pOutboxTransaction->SignContract(server_->m_nymServer);
                pInboxTransaction->SignContract(server_->m_nymServer);

                pOutboxTransaction->SaveContract();
                pInboxTransaction->SaveContract();
                // Meanwhile a copy of the outbox transaction is also added to
                // pOutbox. (It's just another copy of the outbox, but used
                // purely for verifying the balance statement, while a different
                // copy of the outbox is used for actually adding the receipt
                // and saving to the outbox file.)
                //
                pTEMPOutboxTransaction->SignContract(server_->m_nymServer);
                pTEMPOutboxTransaction->SaveContract();

                // No need to save a box receipt in this case, like we normally
                // would
                // when adding a transaction to a box.
                pOutbox->AddTransaction(
                    *pTEMPOutboxTransaction); // pOutbox will clean this up

                // The balance item from the user, for the outbox transaction,
                // will not have
                // the correct transaction number (because I just generated it
                // above, so the user
                // could not possibly have known that when he sent his message.)
                // Currently it is
                // set to "1" in the user request, but I just put the actual
                // number in the pOutbox
                // above (since I now have the actual number.)
                //
                // So when the receipt is saved (the output transaction) it will
                // show the user's
                // signed request with "1" in the outbox, included in the
                // server's signed reply
                // with lNewTransactionNumber in the outbox to correspond to it.
                // The user saves
                // a copy of the same receipt, thus he will be unable to produce
                // a receipt signed
                // by the server, without producing the exact same thing.
                // ("1" in the request and lNewTransactionNumber in the signed
                // response.)
                //
                // This all means that the below call to
                // VerifyBalanceStatement() needs to verify
                // the number "1" on the user request, as lNewTransactionNumber
                // in pOutbox, in order
                // to handle this special case, since otherwise the verification
                // would fail.
                //
                if (!(pBalanceItem->VerifyBalanceStatement(
                        pItem->GetAmount() * (-1), // My acct balance will be
                                                   // smaller as a result of
                                                   // this transfer.
                        theNym, *pInbox, *pOutbox, theFromAccount, tranIn,
                        lNewTransactionNumber))) {
                    OTLog::vOutput(0, "ERROR verifying balance statement while "
                                      "performing transfer. Acct ID:\n%s\n",
                                   strAccountID.Get());
                }
                else {
                    pResponseBalanceItem->SetStatus(
                        OTItem::acknowledgement); // the balance agreement (just
                                                  // above) was successful.
                    pResponseBalanceItem->SetNewOutboxTransNum(
                        lNewTransactionNumber); // So the receipt will show that
                                                // the client's "1" in the
                                                // outbox is now actually "34"
                                                // or whatever, issued by the
                                                // server as part of
                                                // successfully processing the
                                                // transaction.

                    // Deduct the amount from the account...
                    // TODO an issuer account here, can go negative.
                    // whereas a regular account should not be allowed to go
                    // negative.
                    if (theFromAccount.Debit(
                            pItem->GetAmount())) { // todo need to be able to
                                                   // "roll back" if anything
                                                   // inside this block fails.
                        // Here the transactions we just created are actually
                        // added to the ledgers.
                        theFromOutbox.AddTransaction(*pOutboxTransaction);
                        theToInbox.AddTransaction(*pInboxTransaction);

                        // Release any signatures that were there before (They
                        // won't
                        // verify anymore anyway, since the content has
                        // changed.)
                        theFromOutbox.ReleaseSignatures();
                        theToInbox.ReleaseSignatures();

                        // Sign them.
                        theFromOutbox.SignContract(server_->m_nymServer);
                        theToInbox.SignContract(server_->m_nymServer);

                        // Save them internally
                        theFromOutbox.SaveContract();
                        theToInbox.SaveContract();

                        // Save their internals (signatures and all) to file.
                        theFromAccount.SaveOutbox(theFromOutbox);
                        pDestinationAcct->SaveInbox(theToInbox);

                        theFromAccount.ReleaseSignatures();
                        theFromAccount.SignContract(server_->m_nymServer);
                        theFromAccount.SaveContract();
                        theFromAccount.SaveAccount();

                        pDestinationAcct->ReleaseSignatures();
                        pDestinationAcct->SignContract(server_->m_nymServer);
                        pDestinationAcct->SaveContract();
                        pDestinationAcct->SaveAccount();

                        // Now we can set the response item as an
                        // acknowledgement instead of the default (rejection)
                        // otherwise, if we never entered this block, then it
                        // would still be set to rejection, and the
                        // new items would never have been added to the
                        // inbox/outboxes, and those files, along with
                        // the account file, would never have had their
                        // signatures released, or been re-signed or
                        // re-saved back to file.  The debit failed, so all of
                        // those other actions would fail also.
                        // BUT... if the message comes back with
                        // acknowledgement--then all of these actions must have
                        // happened, and here is the server's signature to prove
                        // it.
                        // Otherwise you get no items and no signature. Just a
                        // rejection item in the response transaction.
                        pResponseItem->SetStatus(OTItem::acknowledgement);

                        bOutSuccess = true; // The transfer was successful.

                        // Any inbox/nymbox/outbox ledger will only itself
                        // contain
                        // abbreviated versions of the receipts, including their
                        // hashes.
                        //
                        // The rest is stored separately, in the box receipt,
                        // which is created
                        // whenever a receipt is added to a box, and deleted
                        // after a receipt
                        // is removed from a box.
                        //
                        pOutboxTransaction->SaveBoxReceipt(theFromOutbox);
                        pInboxTransaction->SaveBoxReceipt(theToInbox);
                    }
                    else {
                        delete pOutboxTransaction;
                        pOutboxTransaction = nullptr;
                        delete pInboxTransaction;
                        pInboxTransaction = nullptr;
                        OTLog::vOutput(0, "%s: Unable to debit account %s in "
                                          "the amount of: %lld\n",
                                       __FUNCTION__, strAccountID.Get(),
                                       pItem->GetAmount());
                    }
                }
            } // both boxes were successfully loaded or generated.
        }
    }

    // sign the response item before sending it back (it's already been added to
    // the transaction above)
    // Now, whether it was rejection or acknowledgement, it is set properly and
    // it is signed, and it
    // is owned by the transaction, who will take it from here.
    pResponseItem->SignContract(server_->m_nymServer);
    pResponseItem->SaveContract(); // the signing was of no effect because I
                                   // forgot to save.

    pResponseBalanceItem->SignContract(server_->m_nymServer);
    pResponseBalanceItem->SaveContract();
}

/// NotarizeWithdrawal supports two withdrawal types:
///
/// OTItem::withdrawVoucher    This is a bank voucher, like a cashier's check.
/// Funds are transferred to
///                            the bank, who then issues a cheque drawn on an
/// internal voucher account.
///
/// OTItem::withdrawal        This is a digital cash withdrawal, in the form of
/// untraceable, blinded
///                            tokens. Funds are transferred to the bank, who
/// blind-signs the tokens.
///
void Notary::NotarizeWithdrawal(OTPseudonym& theNym, OTAccount& theAccount,
                                OTTransaction& tranIn, OTTransaction& tranOut,
                                bool& bOutSuccess)
{
    // The outgoing transaction is an "atWithdrawal", that is, "a reply to the
    // withdrawal request"
    tranOut.SetType(OTTransaction::atWithdrawal);

    OTItem* pItem = nullptr;
    OTItem* pItemCash = nullptr;
    OTItem* pItemVoucher = nullptr;
    OTItem* pBalanceItem = nullptr;
    OTItem* pResponseItem = nullptr;
    OTItem* pResponseBalanceItem = nullptr;

    // The incoming transaction may be sent to inboxes and outboxes, and it
    // will probably be bundled in our reply to the user as well. Therefore,
    // let's grab it as a string.
    OTString strInReferenceTo;
    OTString strBalanceItem;

    // Grab the actual server ID from this object, and use it as the server ID
    // here.
    const OTIdentifier SERVER_ID(server_->m_strServerID), USER_ID(theNym),
        ACCOUNT_ID(theAccount), SERVER_USER_ID(server_->m_nymServer),
        ASSET_TYPE_ID(theAccount.GetAssetTypeID());

    const OTString strUserID(USER_ID), strAccountID(ACCOUNT_ID),
        strAssetTypeID(ASSET_TYPE_ID);

    // Here we find out if we're withdrawing cash, or a voucher
    // (A voucher is a cashier's cheque aka banker's cheque).
    //
    OTItem::itemType theReplyItemType = OTItem::error_state;

    pItemVoucher = tranIn.GetItem(OTItem::withdrawVoucher);

    if (nullptr == pItemVoucher) {
        pItemCash = tranIn.GetItem(OTItem::withdrawal);
        pItem = pItemCash;
        if (nullptr != pItem) theReplyItemType = OTItem::atWithdrawal;
    }
    else {
        pItem = pItemVoucher;
        theReplyItemType = OTItem::atWithdrawVoucher;
    }
    pResponseItem =
        OTItem::CreateItemFromTransaction(tranOut, theReplyItemType);
    pResponseItem->SetStatus(OTItem::rejection); // the default.
    tranOut.AddItem(*pResponseItem); // the Transaction's destructor will
                                     // cleanup the item. It "owns" it now.

    pResponseBalanceItem =
        OTItem::CreateItemFromTransaction(tranOut, OTItem::atBalanceStatement);
    pResponseBalanceItem->SetStatus(OTItem::rejection); // the default.
    tranOut.AddItem(*pResponseBalanceItem); // the Transaction's destructor will
                                            // cleanup the item. It "owns" it
                                            // now.
    if (nullptr == pItem) {
        OTString strTemp(tranIn);
        OTLog::vOutput(
            0, "Notary::NotarizeWithdrawal: Expected OTItem::withdrawal or "
               "OTItem::withdrawVoucher in trans# %lld: \n\n%s\n\n",
            tranIn.GetTransactionNum(),
            strTemp.Exists() ? strTemp.Get()
                             : " (ERROR LOADING TRANSACTION INTO STRING) ");
    }
    // Below this point, we know that pItem is good, and that either
    // pItemVoucher OR pItemCash is good,
    // and that pItem points to the good one. Therefore next, let's verify
    // permissions:
    // This permission has to do with ALL withdrawals (cash or voucher)
    else if (!NYM_IS_ALLOWED(strUserID.Get(),
                             ServerSettings::__transact_withdrawal)) {
        OTLog::vOutput(0, "Notary::NotarizeWithdrawal: User %s cannot do "
                          "this transaction (All withdrawals are disallowed in "
                          "server.cfg)\n",
                       strUserID.Get());
    }
    // This permission has to do with vouchers.
    else if ((nullptr != pItemVoucher) &&
             (false ==
              NYM_IS_ALLOWED(strUserID.Get(),
                             ServerSettings::__transact_withdraw_voucher))) {
        OTLog::vOutput(0, "Notary::NotarizeWithdrawal: User %s cannot do "
                          "this transaction (withdrawVoucher is disallowed in "
                          "server.cfg)\n",
                       strUserID.Get());
    }
    // This permission has to do with cash.
    else if ((nullptr != pItemCash) &&
             (false ==
              NYM_IS_ALLOWED(strUserID.Get(),
                             ServerSettings::__transact_withdraw_cash))) {
        OTLog::vOutput(0, "Notary::NotarizeWithdrawal: User %s cannot do "
                          "this transaction (withdraw cash is disallowed in "
                          "server.cfg)\n",
                       strUserID.Get());
    }
    // Check for a balance agreement...
    //
    else if (nullptr ==
             (pBalanceItem = tranIn.GetItem(OTItem::balanceStatement))) {
        OTString strTemp(tranIn);
        OTLog::vOutput(0, "Notary::NotarizeWithdrawal: Expected "
                          "OTItem::balanceStatement, but not found in trans # "
                          "%lld: \n\n%s\n\n",
                       tranIn.GetTransactionNum(),
                       strTemp.Exists()
                           ? strTemp.Get()
                           : " (ERROR LOADING TRANSACTION INTO STRING) ");
    }
    else if (pItem->GetType() == OTItem::withdrawVoucher) {
        // The response item will contain a copy of the request item. So I save
        // it into a string
        // here so they can all grab a copy of it into their "in reference to"
        // fields.
        pItem->SaveContractRaw(strInReferenceTo);
        pBalanceItem->SaveContractRaw(strBalanceItem);

        // Server response item being added to server response transaction
        // (tranOut)
        // (They're getting SOME sort of response item.)

        pResponseItem->SetReferenceString(strInReferenceTo); // the response
                                                             // item carries a
                                                             // copy of what
                                                             // it's responding
                                                             // to.
        pResponseItem->SetReferenceToNum(
            pItem->GetTransactionNum()); // This response item is IN RESPONSE to
                                         // pItem and its Owner Transaction.

        pResponseBalanceItem->SetReferenceString(
            strBalanceItem); // the response item carries a copy of what it's
                             // responding to.
        pResponseBalanceItem->SetReferenceToNum(
            pItem->GetTransactionNum()); // This response item is IN RESPONSE to
                                         // pItem and its Owner Transaction.

        //        OTAccount * pVoucherReserveAcct    = nullptr;
        // contains the server's funds to back vouchers of a specific asset type
        std::shared_ptr<OTAccount> pVoucherReserveAcct;
        std::unique_ptr<OTLedger> pInbox(
            theAccount.LoadInbox(server_->m_nymServer));
        std::unique_ptr<OTLedger> pOutbox(
            theAccount.LoadOutbox(server_->m_nymServer));

        // I'm using the operator== because it exists.
        // If the ID on the "from" account that was passed in,
        // does not match the "Acct From" ID on this transaction item.
        //
        if (!(ACCOUNT_ID ==
              pItem->GetPurportedAccountID())) { // TODO see if this is already
                                                 // verified by the caller
                                                 // function and if so, remove.
            OTLog::Output(0, "Error: Account ID does not match account ID on "
                             "the withdrawal item.\n");
        }
        else if (nullptr == pInbox) {
            OTLog::Error("Error loading or verifying inbox.\n");
        }
        else if (nullptr == pOutbox) {
            OTLog::Error("Error loading or verifying outbox.\n");
        }
        // The server will already have a special account for issuing vouchers.
        // Actually, a list of them --
        // one for each asset type. Since this is the normal way of doing
        // business, transactor_.getVoucherAccount() will
        // just create it if it doesn't already exist, and then return the
        // pointer. Therefore, a failure here
        // is a catastrophic failure!  Should never fail.
        //
        else if ((pVoucherReserveAcct = server_->transactor_.getVoucherAccount(
                      ASSET_TYPE_ID)) && // If assignment results in good
                                         // pointer...
                 pVoucherReserveAcct->VerifyAccount(
                     server_->m_nymServer)) // and if it
                                            // points to
                                            // an acct
                                            // that
                                            // verifies...
        {
            OTString strVoucherRequest, strItemNote;
            pItem->GetNote(strItemNote);
            pItem->GetAttachment(strVoucherRequest);

            OTAccount& theVoucherReserveAcct = (*pVoucherReserveAcct);
            OTIdentifier VOUCHER_ACCOUNT_ID(theVoucherReserveAcct);

            OTCheque theVoucher(SERVER_ID, ASSET_TYPE_ID),
                theVoucherRequest(SERVER_ID, ASSET_TYPE_ID);

            bool bLoadContractFromString =
                theVoucherRequest.LoadContractFromString(strVoucherRequest);

            if (!bLoadContractFromString) {
                OTLog::vError("Notary::%s: ERROR loading voucher request "
                              "from string:\n%s\n",
                              __FUNCTION__, strVoucherRequest.Get());
            }
            else if (!server_->transactor_.verifyTransactionNumber(
                           theNym, theVoucherRequest.GetTransactionNum())) {
                OTLog::vError(
                    "Notary::%s: Failed verifying transaction number on the "
                    "voucher (%lld) in withdrawal request %lld for Nym: %s\n",
                    __FUNCTION__, theVoucherRequest.GetTransactionNum(),
                    tranIn.GetTransactionNum(), strUserID.Get());
            }
            else if (ASSET_TYPE_ID != theVoucherRequest.GetAssetID()) {
                const OTString strFoundAssetID(theVoucherRequest.GetAssetID());
                OTLog::vError(
                    "Notary::%s: Failed verifying asset type ID (%s) on the "
                    "withdraw voucher request (found: %s) "
                    "for transaction %lld, voucher %lld. User: %s\n",
                    __FUNCTION__, strAssetTypeID.Get(), strFoundAssetID.Get(),
                    tranIn.GetTransactionNum(),
                    theVoucherRequest.GetTransactionNum(), strUserID.Get());
            }
            else if (!(pBalanceItem->VerifyBalanceStatement(
                           theVoucherRequest.GetAmount() *
                               (-1), // My account's balance will go down by
                                     // this much.
                           theNym,
                           *pInbox, *pOutbox, theAccount, tranIn))) {
                OTLog::vOutput(0, "ERROR verifying balance statement while "
                                  "issuing voucher. Acct ID:\n%s\n",
                               strAccountID.Get());
            }
            else // successfully loaded the voucher request from the string...
            {
                pResponseBalanceItem->SetStatus(
                    OTItem::acknowledgement); // the transaction agreement was
                                              // successful.
                OTString strChequeMemo;
                strChequeMemo.Format("%s%s", strItemNote.Get(),
                                     theVoucherRequest.GetMemo().Get());

                // 10 minutes ==    600 Seconds
                // 1 hour    ==     3600 Seconds
                // 1 day    ==    86400 Seconds
                // 30 days    ==  2592000 Seconds
                // 3 months ==  7776000 Seconds
                // 6 months == 15552000 Seconds

                const time64_t VALID_FROM =
                    OTTimeGetCurrentTime(); // This time is set to TODAY NOW
                const time64_t VALID_TO = OTTimeAddTimeInterval(
                    VALID_FROM,
                    OTTimeGetSecondsFromTime(OT_TIME_SIX_MONTHS_IN_SECONDS));

                // UPDATE: We now use a transaction number owned by the
                // remitter, instead of the transaction server.
                //
                //                int64_t lNewTransactionNumber = 0;
                //                transactor_.issueNextTransactionNumber(server_->m_nymServer,
                // lNewTransactionNumber); // bStoreTheNumber defaults to true.
                // We save the transaction
                // number on the server Nym (normally we'd discard it) because
                const int64_t lAmount =
                    theVoucherRequest.GetAmount(); // when the cheque is
                                                   // deposited, the server nym,
                                                   // as the owner of
                const OTIdentifier& RECIPIENT_ID =
                    theVoucherRequest.GetRecipientUserID(); // the voucher
                                                            // account, needs to
                                                            // verify the
                                                            // transaction # on
                                                            // the
                // cheque (to prevent double-spending of cheques.)
                bool bIssueVoucher = theVoucher.IssueCheque(
                    lAmount, // The amount of the cheque.
                    theVoucherRequest.GetTransactionNum(), // Requiring a
                                                           // transaction number
                                                           // prevents
                                                           // double-spending of
                                                           // cheques.
                    VALID_FROM, // The expiration date (valid from/to dates) of
                                // the cheque
                    VALID_TO,   // Vouchers are automatically starting today and
                                // lasting 6 months.
                    VOUCHER_ACCOUNT_ID, // The asset account the cheque is drawn
                                        // on.
                    SERVER_USER_ID, // User ID of the sender (in this case the
                                    // server.)
                    strChequeMemo.Get(), // Optional memo field. Includes item
                                         // note and request memo.
                    theVoucherRequest.HasRecipient() ? (&RECIPIENT_ID)
                                                     : nullptr);

                // IF we successfully created the voucher, AND the voucher
                // amount is greater than 0,
                // AND debited the user's account,
                // AND credited the server's voucher account,
                //
                // THEN save the accounts and return the voucher to the user.
                //
                if (bIssueVoucher && (lAmount > 0) &&
                    theAccount.Debit(theVoucherRequest.GetAmount())) {
                    if (false ==
                        pVoucherReserveAcct->Credit(
                            theVoucherRequest.GetAmount())) {
                        OTLog::Error("Notary::NotarizeWithdrawal: Failed "
                                     "crediting voucher reserve account.\n");

                        if (false ==
                            theAccount.Credit(theVoucherRequest.GetAmount()))
                            OTLog::Error("Notary::NotarizeWithdrawal "
                                         "(voucher): Failed crediting user "
                                         "account.\n");
                    }
                    else {
                        OTString strVoucher;
                        theVoucher.SetAsVoucher(
                            USER_ID, ACCOUNT_ID); // All this does is set the
                                                  // voucher's internal contract
                                                  // string to
                        // "VOUCHER" instead of "CHEQUE". Plus it saves the
                        // remitter's IDs.
                        theVoucher.SignContract(server_->m_nymServer);
                        theVoucher.SaveContract();
                        theVoucher.SaveContractRaw(strVoucher);

                        pResponseItem->SetAttachment(strVoucher);
                        pResponseItem->SetStatus(OTItem::acknowledgement);

                        bOutSuccess =
                            true; // The withdrawal of a voucher was successful.
                        // Release any signatures that were there before (They
                        // won't
                        // verify anymore anyway, since the content has
                        // changed.)
                        theAccount.ReleaseSignatures();
                        theAccount.SignContract(server_->m_nymServer); // Sign
                        theAccount.SaveContract();                     // Save
                        theAccount.SaveAccount(); // Save to file

                        // We also need to save the Voucher cash reserve
                        // account.
                        // (Any issued voucher cheque is automatically backed by
                        // this reserve account.
                        // If a cheque is deposited, the funds come back out of
                        // this account. If the
                        // cheque expires, then after the expiry period, if it
                        // remains in the account,
                        // it is now the property of the transaction server.)
                        pVoucherReserveAcct->ReleaseSignatures();
                        pVoucherReserveAcct->SignContract(server_->m_nymServer);
                        pVoucherReserveAcct->SaveContract();
                        pVoucherReserveAcct->SaveAccount();
                    }
                }
                // else{} // TODO log that there was a problem with the amount

            } // voucher request loaded successfully from string
        }     // transactor_.getVoucherAccount()
        else {
            OTLog::vError(
                "transactor_.getVoucherAccount() failed in NotarizeWithdrawal. "
                "Asset Type:\n%s\n",
                strAssetTypeID.Get());
        }
    }

    // WITHDRAW DIGITAL CASH (BLINDED TOKENS)
    //
    // For now, there should only be one of these withdrawal items inside the
    // transaction.
    // So we treat it that way... I either get it successfully or not.
    //
    else if (pItem->GetType() == OTItem::withdrawal) {
        // The response item will contain a copy of the request item. So I save
        // it into a string
        // here so they can all grab a copy of it into their "in reference to"
        // fields.
        //
        pItem->SaveContractRaw(strInReferenceTo);
        pBalanceItem->SaveContractRaw(strBalanceItem);

        // Server response item being added to server response transaction
        // (tranOut)
        // They're getting SOME sort of response item.
        //
        pResponseItem->SetReferenceString(strInReferenceTo); // the response
                                                             // item carries a
                                                             // copy of what
                                                             // it's responding
                                                             // to.
        pResponseItem->SetReferenceToNum(
            pItem->GetTransactionNum()); // This response item is IN RESPONSE to
                                         // pItem and its Owner Transaction.

        pResponseBalanceItem->SetReferenceString(
            strBalanceItem); // the response item carries a copy of what it's
                             // responding to.
        pResponseBalanceItem->SetReferenceToNum(
            pItem->GetTransactionNum()); // This response item is IN RESPONSE to
                                         // pItem and its Owner Transaction.
        std::unique_ptr<OTLedger> pInbox(
            theAccount.LoadInbox(server_->m_nymServer));
        std::unique_ptr<OTLedger> pOutbox(
            theAccount.LoadOutbox(server_->m_nymServer));

        Mint* pMint = nullptr;
        OTAccount* pMintCashReserveAcct = nullptr;

        if (0 > pItem->GetAmount()) {
            OTLog::Output(0, "Attempt to withdraw a negative amount.\n");
        }
        // If the ID on the "from" account that was passed in,
        // does not match the "Acct From" ID on this transaction item
        //
        else if (ACCOUNT_ID != pItem->GetPurportedAccountID()) {
            OTLog::Output(0, "Error: 'From' account ID on the transaction does "
                             "not match 'from' account ID on the withdrawal "
                             "item.\n");
        }
        else if (nullptr == pInbox) {
            OTLog::Error("Error loading or verifying inbox.\n");
        }
        else if (nullptr == pOutbox) {
            OTLog::Error("Error loading or verifying outbox.\n");
        }
        else {
            // The COIN REQUEST (including the prototokens) comes from the
            // client side.
            // so we assume the Token is in the payload. Now we need to
            // randomly choose one for
            // signing, and reply to the client with that number so that the
            // client can reply back
            // to us with the unblinding factors for all the other prototokens
            // (but that one.)
            //
            // In the meantime, I have to store this request somewhere --
            // presumably in the outbox or purse.
            //
            // UPDATE!!! Looks like Lucre protocol is simpler than that. The
            // request only needs to contain a
            // single blinded token, which the server signs and sends back.
            // Done.
            //
            // The amount is known to be safe (by the mint) because the User
            // asks the Mint to create
            // a denomination (say, 10) token. The Mint therefore uses the
            // "Denomination 10" key to sign
            // the token, and will later use the "Denomination 10" key to verify
            // the token. So the mint
            // obviously trusts its own keys... There is nothing else to "open
            // and verify", since only the ID
            // itself is what gets blinded and verified.  The amount on the
            // token (as well as the asset type)
            // is only there to help the bank to look up the right key, without
            // which the token will DEFINITELY
            // NOT verify. So it is in the user's interest to supply the correct
            // amount, because otherwise he'll
            // just get the wrong key and then get rejected by the bank.

            OTString strPurse;
            pItem->GetAttachment(strPurse);

            // Todo do more security checking in here, like making sure the
            // withdrawal amount matches the
            // total of the proto-tokens. Update: I think this is done, since
            // the Debits are done one-at-a-time
            // for each token and it's amount/denomination

            Purse thePurse(SERVER_ID, ASSET_TYPE_ID);
            Purse theOutputPurse(SERVER_ID, ASSET_TYPE_ID);
            Token* pToken = nullptr;
            dequeOfTokenPtrs theDeque;

            bool bSuccess = false;
            bool bLoadContractFromString =
                thePurse.LoadContractFromString(strPurse);

            if (!bLoadContractFromString) {
                OTLog::vError("ERROR loading purse from string in "
                              "Notary::NotarizeWithdrawal:\n%s\n",
                              strPurse.Get());
            }
            else if (!(pBalanceItem->VerifyBalanceStatement(
                           thePurse.GetTotalValue() * (-1), // This amount will
                                                            // be subtracted
                                                            // from my acct.
                           theNym, *pInbox, *pOutbox, theAccount, tranIn))) {
                OTLog::vOutput(0, "ERROR verifying balance statement while "
                                  "withdrawing cash. Acct ID: %s\n",
                               strAccountID.Get());
            }
            else // successfully loaded the purse from the string...
            {
                pResponseBalanceItem->SetStatus(
                    OTItem::acknowledgement); // the transaction agreement was
                                              // successful.

                // Pull the token(s) out of the purse that was received from the
                // client.
                while ((pToken = thePurse.Pop(server_->m_nymServer)) !=
                       nullptr) {
                    // We are responsible to cleanup pToken
                    // So I grab a copy here for later...
                    theDeque.push_front(pToken);

                    pMint = server_->transactor_.getMint(ASSET_TYPE_ID,
                                                         pToken->GetSeries());

                    if (nullptr == pMint) {
                        OTLog::vError("Notary::NotarizeWithdrawal: Unable to "
                                      "find Mint (series %d): %s\n",
                                      pToken->GetSeries(),
                                      strAssetTypeID.Get());
                        bSuccess = false;
                        break; // Once there's a failure, we ditch the loop.
                    }
                    else if (nullptr ==
                               (pMintCashReserveAcct =
                                    pMint->GetCashReserveAccount())) {
                        OTLog::vError(
                            "Notary::NotarizeWithdrawal: Unable to find cash "
                            "reserve account for Mint (series %d): %s\n",
                            pToken->GetSeries(), strAssetTypeID.Get());
                        bSuccess = false;
                        break; // Once there's a failure, we ditch the loop.
                    }
                    // Mints expire halfway into their token expiration period.
                    // So if a mint creates
                    // tokens valid from Jan 1 through Jun 1, then the Mint
                    // itself expires Mar 1.
                    // That's when the next series Mint is phased in to start
                    // issuing tokens, even
                    // though the server continues redeeming the first series
                    // tokens until June.
                    //
                    else if (pMint->Expired()) {
                        OTLog::vError(
                            "Notary::NotarizeWithdrawal: User attempting "
                            "withdrawal with an expired mint (series %d): %s\n",
                            pToken->GetSeries(), strAssetTypeID.Get());
                        bSuccess = false;
                        break; // Once there's a failure, we ditch the loop.
                    }
                    else {
                        OTString theStringReturnVal;

                        if (pToken->GetAssetID() != ASSET_TYPE_ID) {
                            const OTString str1(pToken->GetAssetID()),
                                str2(ASSET_TYPE_ID);
                            bSuccess = false;
                            OTLog::vError("%s: ERROR while signing token: "
                                          "Expected asset ID %s but found %s "
                                          "instead. (Failure.)\n",
                                          __FUNCTION__, str2.Get(), str1.Get());
                            break;
                        }
                        // TokenIndex is for cash systems that send multiple
                        // proto-tokens, so the Mint
                        // knows which proto-token has been chosen for signing.
                        // But Lucre only uses a single proto-token, so the
                        // token index is always 0.
                        //
                        else if (!(pMint->SignToken(server_->m_nymServer,
                                                    *pToken, theStringReturnVal,
                                                    0))) // nTokenIndex = 0 //
                        // ******************************************
                        {
                            bSuccess = false;
                            OTLog::vError(
                                "%s: Failure in call: "
                                "pMint->SignToken(server_->m_nymServer, "
                                "*pToken, theStringReturnVal, 0). "
                                "(Returning.)\n",
                                __FUNCTION__);
                            break;
                        }
                        else {
                            OTASCIIArmor theArmorReturnVal(theStringReturnVal);

                            pToken->ReleaseSignatures(); // this releases the
                                                         // normal signatures,
                                                         // not the Lucre signed
                                                         // token from the Mint,
                                                         // above.

                            pToken->SetSignature(theArmorReturnVal,
                                                 0); // nTokenIndex = 0

                            // Sign and Save the token
                            pToken->SignContract(server_->m_nymServer);
                            pToken->SaveContract();

                            // Now the token is in signedToken mode, and the
                            // other prototokens have been released.

                            // Deduct the amount from the account...
                            if (theAccount.Debit(
                                    pToken->GetDenomination())) { // todo need
                                                                  // to be able
                                                                  // to "roll
                                                                  // back" if
                                                                  // anything
                                                                  // inside this
                                                                  // block
                                                                  // fails.
                                bSuccess = true;

                                // Credit the server's cash account for this
                                // asset type in the same
                                // amount that was debited. When the token is
                                // deposited again, Debit that same
                                // server cash account and deposit in the
                                // depositor's acct.
                                // Why, you might ask? Because if the token
                                // expires, the money will stay in
                                // the bank's cash account instead of being lost
                                // (and screwing up the overall
                                // issuer balance, with the issued money
                                // disappearing forever.) The bank knows
                                // that once the series expires, whatever funds
                                // are left in that cash account are
                                // for the bank to keep. They can be transferred
                                // to another account and kept, instead
                                // of being lost.
                                if (!pMintCashReserveAcct->Credit(
                                        pToken->GetDenomination())) {
                                    OTLog::Error("Error crediting mint cash "
                                                 "reserve account...\n");

                                    // Reverse the account debit (even though
                                    // we're not going to save it anyway.)
                                    if (false ==
                                        theAccount.Credit(
                                            pToken->GetDenomination()))
                                        OTLog::vError("%s: Failed crediting "
                                                      "user account back.\n",
                                                      __FUNCTION__);

                                    bSuccess = false;
                                    break;
                                }
                            }
                            else {
                                bSuccess = false;
                                OTLog::vOutput(0, "%s: Unable to debit account "
                                                  "%s in the amount of: %lld\n",
                                               __FUNCTION__, strAccountID.Get(),
                                               pToken->GetDenomination());
                                break; // Once there's a failure, we ditch the
                                       // loop.
                            }
                        }
                    }
                } // While success popping token out of the purse...

                if (bSuccess) {
                    while (!theDeque.empty()) {
                        pToken = theDeque.front();
                        theDeque.pop_front();

                        theOutputPurse.Push(theNym, *pToken); // these were in
                                                              // reverse order.
                                                              // Fixing with
                                                              // theDeque.

                        delete pToken;
                        pToken = nullptr;
                    }

                    strPurse.Release(); // just in case it only concatenates.

                    theOutputPurse.SignContract(server_->m_nymServer);
                    theOutputPurse.SaveContract(); // todo this is probably
                                                   // unnecessary
                    theOutputPurse.SaveContractRaw(strPurse);

                    // Add the digital cash token to the response message
                    pResponseItem->SetAttachment(strPurse);
                    pResponseItem->SetStatus(OTItem::acknowledgement);

                    bOutSuccess = true; // The cash withdrawal was successful.

                    // Release any signatures that were there before (They won't
                    // verify anymore anyway, since the content has changed.)
                    theAccount.ReleaseSignatures();

                    // Sign
                    theAccount.SignContract(server_->m_nymServer);

                    // Save
                    theAccount.SaveContract();

                    // Save to file
                    theAccount.SaveAccount();

                    // We also need to save the Mint's cash reserve.
                    // (Any cash issued by the Mint is automatically backed by
                    // this reserve
                    // account. If cash is deposited, it comes back out of this
                    // account. If the
                    // cash expires, then after the expiry period, if it remains
                    // in the account,
                    // it is now the property of the transaction server.)
                    pMintCashReserveAcct->ReleaseSignatures();
                    pMintCashReserveAcct->SignContract(server_->m_nymServer);
                    pMintCashReserveAcct->SaveContract();
                    pMintCashReserveAcct->SaveAccount();

                    // Notice if there is any failure in the above loop, then we
                    // will never enter this block.
                    // Therefore the account will never be saved with the new
                    // debited balance, and the output
                    // purse will never be added to the response item.  No
                    // tokens will be returned to the user
                    // and the account will not be saved, thus retaining the
                    // original balance.
                    //
                    // Only if everything is successful do we enter this block,
                    // save the output purse onto the
                    // response, and save the newly-debitted account back to
                    // disk.
                }
                // Still need to clean up theDeque
                else {
                    while (!theDeque.empty()) {
                        pToken = theDeque.front();
                        theDeque.pop_front();

                        delete pToken;
                        pToken = nullptr;
                    }
                }

            } // purse loaded successfully from string

        } // the Account ID on the item matched properly
        // sign the response item before sending it back (it's already been
        // added to the transaction above)
        // Now, whether it was rejection or acknowledgement, it is set properly
        // and it is signed, and it
        // is owned by the transaction, who will take it from here.
        pResponseItem->SignContract(server_->m_nymServer);
        pResponseItem->SaveContract(); // the signing was of no effect because I
                                       // forgot to save.

        pResponseBalanceItem->SignContract(server_->m_nymServer);
        pResponseBalanceItem->SaveContract();
    }
    else {
        OTString strTemp(tranIn);
        OTLog::vOutput(
            0, "Notary::NotarizeWithdrawal: Expected OTItem::withdrawal or "
               "OTItem::withdrawVoucher in trans# %lld: \n\n%s\n\n",
            tranIn.GetTransactionNum(),
            strTemp.Exists() ? strTemp.Get()
                             : " (ERROR LOADING TRANSACTION INTO STRING) ");
    }

    // sign the response item before sending it back (it's already been added to
    // the transaction above)
    // Now, whether it was rejection or acknowledgement, it is set properly and
    // it is signed, and it
    // is owned by the transaction, who will take it from here.
    pResponseItem->SignContract(server_->m_nymServer);
    pResponseItem->SaveContract(); // the signing was of no effect because I
                                   // forgot to save.

    pResponseBalanceItem->SignContract(server_->m_nymServer);
    pResponseBalanceItem->SaveContract();
}

/// NotarizePayDividend
///
/// Phase 1: Only the signer on the currency contract (the issuer) can pay a
/// dividend. He
///          must pay the dividend in a currency of a DIFFERENT type. (Such as,
/// a dollar
///          dividend for shares of Pepsi.) So this transaction is a "dollar"
/// transaction,
///          using that example, and theAccount is a dollar account. But then
/// how do we know
///          those dollars are being paid to _Pepsi_ shareholders? Because the
/// asset type
///          of the shares must be attached to the OTItem::payDividend within
/// tranIn--and
///          also so must the "dividend payout amount, per share" be included,
/// for the same
///          reason. This function gets the asset contract for the shares, and
/// passes a functor
///          to it, so that it can iterate through all the Pepsi asset accounts
/// and form/send a
///          payout voucher for each one (via the functor.) This function also
/// verifies that
///          theNym is both signer on the asset contract for Pepsi shares (the
/// calling function
///          has already verified that theNym is the signer on the dollar
/// account.)
///
/// Phase 2: voting groups, hierarchical entities with agents, oversight,
/// corporate asset accounts, etc.
///
void Notary::NotarizePayDividend(OTPseudonym& theNym,
                                 OTAccount& theSourceAccount,
                                 OTTransaction& tranIn, OTTransaction& tranOut,
                                 bool& bOutSuccess)
{
    const char* szFunc = "Notary::NotarizePayDividend";

    // The outgoing transaction is an "atPayDividend", that is, "a reply to the
    // 'pay dividend' request"
    tranOut.SetType(OTTransaction::atPayDividend);

    OTItem* pItem =
        nullptr; // This pointer and the following one, are 2 pointers,
                 // as a vestige
    OTItem* pItemPayDividend =
        nullptr;                    // from the withdrawal code, which has two
                                    // forms: voucher and cash.
    OTItem* pBalanceItem = nullptr; // The balance agreement item, which must be
                                    // on any transaction.
    OTItem* pResponseItem = nullptr; // Server's response to pItem.
    OTItem* pResponseBalanceItem =
        nullptr; // Server's response to pBalanceItem.

    // The incoming transaction may be sent to inboxes and outboxes, and it
    // will probably be bundled in our reply to the user as well. Therefore,
    // let's grab it as a string.
    //
    OTString strInReferenceTo;
    OTString strBalanceItem;

    // Grab the actual server ID from this object, and use it as the server ID
    // here.
    //
    const OTIdentifier SERVER_ID(server_->m_strServerID), USER_ID(theNym),
        SOURCE_ACCT_ID(theSourceAccount), SERVER_USER_ID(server_->m_nymServer),
        PAYOUT_ASSET_ID(theSourceAccount.GetAssetTypeID()); // Ex: Pepsi shares,
                                                            // Dollar dividend.
                                                            // (PAYOUT_ASSET_ID
                                                            // is Dollars.)

    const OTString strUserID(USER_ID), strAccountID(SOURCE_ACCT_ID),
        strAssetTypeID(PAYOUT_ASSET_ID);
    // Make sure the appropriate item is attached.
    //
    OTItem::itemType theReplyItemType = OTItem::error_state;

    pItemPayDividend = tranIn.GetItem(OTItem::payDividend);

    if (nullptr != pItemPayDividend) // found it.
    {
        pItem = pItemPayDividend;
        theReplyItemType = OTItem::atPayDividend;
    }
    // Server response item being added to server response transaction (tranOut)
    // (They're getting SOME sort of response item.)
    //
    pResponseItem =
        OTItem::CreateItemFromTransaction(tranOut, theReplyItemType);
    pResponseItem->SetStatus(OTItem::rejection); // the default.
    tranOut.AddItem(*pResponseItem); // the Transaction's destructor will
                                     // cleanup the item. It "owns" it now.

    pResponseBalanceItem =
        OTItem::CreateItemFromTransaction(tranOut, OTItem::atBalanceStatement);
    pResponseBalanceItem->SetStatus(OTItem::rejection); // the default.
    tranOut.AddItem(*pResponseBalanceItem); // the Transaction's destructor will
                                            // cleanup the item. It "owns" it
                                            // now.
    if (nullptr == pItem) {
        OTString strTemp(tranIn);
        OTLog::vOutput(
            0, "%s: Expected OTItem::payDividend in trans# %lld: \n\n%s\n\n",
            szFunc, tranIn.GetTransactionNum(),
            strTemp.Exists()
                ? strTemp.Get()
                : " (ERROR SERIALIZING TRANSACTION INTO A STRING) ");
    }
    // Below this point, we know that pItem is good, and that pItemPayDividend
    // is good,
    // and that pItem points to it. Therefore next, let's verify permissions:
    //
    // This permission has to do with ALL withdrawals from an account
    // (cash / voucher / dividends)
    else if (!NYM_IS_ALLOWED(strUserID.Get(),
                             ServerSettings::__transact_withdrawal)) {
        OTLog::vOutput(
            0, "%s: User %s cannot do this transaction (All withdrawals are "
               "disallowed in server.cfg, even for paying dividends with.)\n",
            szFunc, strUserID.Get());
    }
    // This permission has to do with paying dividends.
    //
    else if ((nullptr != pItemPayDividend) &&
             (false ==
              NYM_IS_ALLOWED(strUserID.Get(),
                             ServerSettings::__transact_pay_dividend))) {
        OTLog::vOutput(0, "%s: User %s cannot do this transaction "
                          "(payDividend is disallowed in server.cfg)\n",
                       szFunc, strUserID.Get());
    }
    // Check for a balance agreement...
    //
    else if (nullptr ==
             (pBalanceItem = tranIn.GetItem(OTItem::balanceStatement))) {
        OTString strTemp(tranIn);
        OTLog::vOutput(0, "%s: Expected OTItem::balanceStatement, but not "
                          "found in trans # %lld: \n\n%s\n\n",
                       szFunc, tranIn.GetTransactionNum(),
                       strTemp.Exists()
                           ? strTemp.Get()
                           : " (ERROR SERIALIZING TRANSACTION INTO A STRING) ");
    }
    else if (pItem->GetType() == OTItem::payDividend) // Superfluous by this
                                                        // point. Artifact of
                                                        // withdrawal code.
    {
        // The response item will contain a copy of the request item. So I save
        // it into a string
        // here so they can all grab a copy of it into their "in reference to"
        // fields.
        //
        pItem->SaveContractRaw(strInReferenceTo);
        pBalanceItem->SaveContractRaw(strBalanceItem);

        // Make sure the response items know which transaction # they're in
        // response to,
        // and have a copy of the original request-transaction.
        //
        pResponseItem->SetReferenceString(strInReferenceTo); // the response
                                                             // item carries a
                                                             // copy of what
                                                             // it's responding
                                                             // to.
        pResponseItem->SetReferenceToNum(
            pItem->GetTransactionNum()); // This response item is IN RESPONSE to
                                         // pItem and its Owner Transaction.

        pResponseBalanceItem->SetReferenceString(
            strBalanceItem); // the response item carries a copy of what it's
                             // responding to.
        pResponseBalanceItem->SetReferenceToNum(
            pItem->GetTransactionNum()); // This response item is IN RESPONSE to
                                         // pItem and its Owner Transaction.
        const int64_t lTotalCostOfDividend = pItem->GetAmount();
        OTCheque theVoucherRequest;
        OTString strVoucherRequest,
            strItemNote; // When paying a dividend, you create a voucher request
                         // (the same as in withdrawVoucher). It's just for
                         // information
        pItem->GetAttachment(strVoucherRequest); // passing, since payDividend
                                                 // needs a few bits of info,
                                                 // and this is a convenient way
                                                 // of passing it.
        pItem->GetNote(strItemNote);
        const bool bLoadContractFromString =
            theVoucherRequest.LoadContractFromString(strVoucherRequest);

        if (!bLoadContractFromString) {
            OTLog::vError("%s: ERROR loading dividend payout's voucher request "
                          "from string:\n%s\n",
                          szFunc, strVoucherRequest.Get());
        }
        else if (theVoucherRequest.GetAmount() <= 0) {
            OTLog::vError("%s: ERROR expected >0 'payout per share' as "
                          "'amount' on request voucher:\n%s\n",
                          szFunc, strVoucherRequest.Get());
        }
        else {
            // the request voucher (sent from client) contains the payout amount
            // per share.
            // Whereas pItem contains lTotalCostOfDividend, which is the total
            // cost (the
            // payout multiplied by number of shares.)
            //
            const int64_t lAmountPerShare =
                theVoucherRequest.GetAmount(); // already validated, just above.
            const OTIdentifier SHARES_ISSUER_ACCT_ID =
                theVoucherRequest.GetSenderAcctID();

            const OTString strSharesIssuerAcct(SHARES_ISSUER_ACCT_ID);
            // Get the asset contract for the shares type, stored in the voucher
            // request, inside pItem.
            //       (Make sure it's NOT the same asset type as
            // theSourceAccount.)
            //
            const OTIdentifier SHARES_ASSET_ID = theVoucherRequest.GetAssetID();
            OTAssetContract* pSharesContract =
                server_->transactor_.getAssetContract(SHARES_ASSET_ID);
            OTAccount* pSharesIssuerAccount = nullptr;
            std::unique_ptr<OTAccount> theAcctAngel;

            if (nullptr != pSharesContract) {
                pSharesIssuerAccount = OTAccount::LoadExistingAccount(
                    SHARES_ISSUER_ACCT_ID, SERVER_ID);
                theAcctAngel.reset(pSharesIssuerAccount);
            }

            if (nullptr == pSharesContract) {
                const OTString strSharesType(SHARES_ASSET_ID);
                OTLog::vError("%s: ERROR unable to find shares contract based "
                              "on asset type ID: %s\n",
                              szFunc, strSharesType.Get());
            }
            else if (!pSharesContract->IsShares()) {
                const OTString strSharesType(SHARES_ASSET_ID);
                OTLog::vError("%s: FAILURE: Asset contract is not "
                              "shares-based. Asset type ID: %s\n",
                              szFunc, strSharesType.Get());
            }
            else if (!pSharesContract->VerifySignature(theNym)) {
                const OTString strSharesType(SHARES_ASSET_ID);
                OTLog::vError("%s: ERROR unable to verify signature for Nym "
                              "(%s) on shares contract "
                              "with asset ID: %s\n",
                              szFunc, strUserID.Get(), strSharesType.Get());
            }
            else if (nullptr == pSharesIssuerAccount) {
                OTLog::vError(
                    "%s: ERROR unable to find issuer account for shares: %s\n",
                    szFunc, strSharesIssuerAcct.Get());
            }
            else if (PAYOUT_ASSET_ID ==
                       SHARES_ASSET_ID) // these can't be the same
            {
                const OTString strSharesType(PAYOUT_ASSET_ID);
                OTLog::vError("%s: ERROR dividend payout attempted, using "
                              "shares asset type as payout type also. "
                              "(It's logically impossible for it to payout to "
                              "itself, using "
                              "ITSELF as the asset type for the payout): %s\n",
                              szFunc, strSharesType.Get());
            }
            else if (false ==
                       pSharesIssuerAccount->VerifyAccount(
                           server_->m_nymServer)) {
                const OTString strIssuerAcctID(SHARES_ISSUER_ACCT_ID);
                OTLog::vError(
                    "%s: ERROR failed trying to verify issuer account: %s\n",
                    szFunc, strIssuerAcctID.Get());
            }
            else if (!pSharesIssuerAccount->VerifyOwner(theNym)) {
                const OTString strIssuerAcctID(SHARES_ISSUER_ACCT_ID);
                OTLog::vOutput(
                    0, "%s: ERROR verifying signer's ownership of shares "
                       "issuer account (%s), "
                       "while trying to pay dividend from source account: %s\n",
                    szFunc, strIssuerAcctID.Get(), strAccountID.Get());
            }
            // Make sure the share issuer's account balance (number of shares
            // issued * (-1)),
            // when multiplied by the dividend "amount payout per share", equals
            // the "total cost of dividend"
            // as expected based on the value from pItem->GetAmount.
            //
            //
            else if ((pSharesIssuerAccount->GetBalance() * (-1) *
                      lAmountPerShare) != lTotalCostOfDividend) {
                const OTString strIssuerAcctID(SHARES_ISSUER_ACCT_ID);
                OTLog::vOutput(0, "%s: ERROR: total payout of dividend as "
                                  "calculated (%lld) doesn't match client's "
                                  "request (%lld) for source acct: %s\n",
                               szFunc, (pSharesIssuerAccount->GetBalance() *
                                        (-1) * lAmountPerShare),
                               lTotalCostOfDividend, strAccountID.Get());
            }
            else if (theSourceAccount.GetBalance() < lTotalCostOfDividend) {
                const OTString strIssuerAcctID(SHARES_ISSUER_ACCT_ID);
                OTLog::vOutput(0, "%s: FAILURE: not enough funds (%lld) to "
                                  "cover total dividend payout (%lld) for "
                                  "source acct: %s\n",
                               szFunc, theSourceAccount.GetBalance(),
                               lTotalCostOfDividend, strAccountID.Get());
            }
            else {
                // Remove all the funds at once (so the balance agreement
                // matches up.)
                // Then, iterate through the asset accounts and use a functor to
                // send a voucher to each one.
                // (Or back to the issuer, for any that fail.)

                // UPDATE: unfortunately the balance agreement will be a lie
                // unless the complete amount is removed.
                // Therefore, failures must be sent back to the issuer as
                // individual receipts, containing the vouchers
                // for any failures, so he can have a record of them, and so he
                // can recover the funds.
                std::unique_ptr<OTLedger> pInbox(
                    theSourceAccount.LoadInbox(server_->m_nymServer));
                std::unique_ptr<OTLedger> pOutbox(
                    theSourceAccount.LoadOutbox(server_->m_nymServer));
                // contains the server's funds to back vouchers of a specific
                // asset type.
                std::shared_ptr<OTAccount> pVoucherReserveAcct;
                //              OTAccount    *       pVoucherReserveAcct    =
                // nullptr;
                //
                // If the ID on the "from" account that was passed in, does
                // not match the "Acct From" ID on this transaction item...
                //
                if (SOURCE_ACCT_ID !=
                    pItem->GetPurportedAccountID()) { // TODO see if this is
                                                      // already verified by the
                                                      // caller function and if
                                                      // so, remove.
                    // (I believe the item would have entirely failed to load,
                    // if the account ID, and
                    // other IDs, hadn't matched up with the transaction when we
                    // loaded it.)
                    //
                    OTLog::vOutput(0, "%s: Error: Account ID does not match "
                                      "account ID on the 'pay dividend' "
                                      "item.\n",
                                   szFunc);
                }
                else if (nullptr == pInbox) {
                    OTLog::vError("%s: Error loading or verifying inbox.\n",
                                  szFunc);
                }
                else if (nullptr == pOutbox) {
                    OTLog::vError("%s: Error loading or verifying outbox.\n",
                                  szFunc);
                }
                // The server will already have a special account for issuing
                // vouchers. Actually, a list of them --
                // one for each asset type. Since this is the normal way of
                // doing business, transactor_.getVoucherAccount() will
                // just create it if it doesn't already exist, and then return
                // the pointer. Therefore, a failure here
                // is a catastrophic failure!  Should never fail.
                //
                else if ((pVoucherReserveAcct =
                              server_->transactor_.getVoucherAccount(
                                  PAYOUT_ASSET_ID)) && // If
                         // transactor_.getVoucherAccount
                         // returns a good pointer...
                         pVoucherReserveAcct->VerifyAccount(
                             server_->m_nymServer)) // ...and if it points to an
                                                    // acct
                // that verifies with the server's
                // nym...
                {
                    OTAccount& theVoucherReserveAcct = (*pVoucherReserveAcct);
                    const OTIdentifier VOUCHER_ACCOUNT_ID(
                        theVoucherReserveAcct);

                    // This amount must be the total amount based on the amount
                    // issued.
                    // For example if 1000 shares of Pepsi were issued, and the
                    // dividend is $2 per share,
                    // then loading the issuer's account will show a balance of
                    // -1000, and I must have
                    // $2000 in my source account if I am going to pay this
                    // dividend.
                    //
                    // This $2000 is entirely removed from my account at once,
                    // and the below balance agreement
                    // must be for $2000. The vouchers are sent to the owners of
                    // each account, in amounts
                    // proportionate to the number of shares in the account. For
                    // any voucher that fails to be
                    // sent (for whatever reason) it is sent back to theNym
                    // instead.
                    //
                    if (!(pBalanceItem->VerifyBalanceStatement(
                            lTotalCostOfDividend * (-1), // My account's
                                                         // balance will go
                                                         // down by this much.
                            theNym, *pInbox, *pOutbox, theSourceAccount,
                            tranIn))) {
                        OTLog::vOutput(0, "%s: ERROR verifying balance "
                                          "statement while trying to pay "
                                          "dividend. Source Acct ID: %s\n",
                                       szFunc, strAccountID.Get());
                    }
                    else // successfully verified the balance agreement.
                    {
                        pResponseBalanceItem->SetStatus(
                            OTItem::acknowledgement); // the transaction
                                                      // agreement was
                                                      // successful.
                        // IF we successfully created the voucher, AND the
                        // voucher amount is greater than 0,
                        // AND debited the user's account,
                        // AND credited the server's voucher account,
                        //
                        // THEN save the accounts and pay the dividend out to
                        // the shareholders.
                        //
                        if ((lTotalCostOfDividend > 0) &&
                            theSourceAccount.Debit(
                                lTotalCostOfDividend) // todo: failsafe: update
                                                      // this code in case of
                                                      // problems in this
                                                      // sensitive area. need
                                                      // better funds transfer
                                                      // code.
                            ) {
                            const OTString strVoucherAcctID(VOUCHER_ACCOUNT_ID);

                            if (false ==
                                pVoucherReserveAcct->Credit(
                                    lTotalCostOfDividend)) // theVoucherRequest.GetAmount()))
                            {
                                OTLog::vError("%s: Failed crediting %lld units "
                                              "to voucher reserve account: "
                                              "%s\n",
                                              szFunc, lTotalCostOfDividend,
                                              strVoucherAcctID.Get());

                                // Since pVoucherReserveAcct->Credit failed, we
                                // have to return
                                // the funds from theSourceAccount.Debit (Credit
                                // them back.)
                                //
                                if (false ==
                                    theSourceAccount.Credit(
                                        lTotalCostOfDividend))
                                    OTLog::vError(
                                        "%s: Failed crediting back the user "
                                        "account, after taking his funds "
                                        "and failing to credit them to the "
                                        "voucher reserve account.\n",
                                        szFunc);
                            }
                            else // By this point, we have taken the full
                                   // funds
                                   // and moved them to the voucher
                            { // reserve account. So now, let's iterate all the
                                // accounts for that share type,
                                // and send a voucher to the owner of each one,
                                // to payout his dividend.

                                // todo: determine whether I need to attach
                                // anything here at all...
                                pResponseItem->SetStatus(
                                    OTItem::acknowledgement);

                                bOutSuccess = true; // The paying of the
                                                    // dividends was successful.
                                //
                                //
                                // SAVE THE ACCOUNTS WITH THE NEW BALANCES
                                // (FUNDS ARE MOVED)
                                //
                                // At this point, we save the accounts, so that
                                // the funds transfer is solid before
                                // we start mailing vouchers out to people.

                                // Release any signatures that were there before
                                // (They won't
                                // verify anymore anyway, since the content has
                                // changed.)
                                theSourceAccount.ReleaseSignatures();
                                theSourceAccount.SignContract(
                                    server_->m_nymServer);       // Sign
                                theSourceAccount.SaveContract(); // Save
                                theSourceAccount.SaveAccount();  // Save to file

                                // We also need to save the Voucher cash reserve
                                // account.
                                // (Any issued voucher cheque is automatically
                                // backed by this reserve account.
                                // If a cheque is deposited, the funds come back
                                // out of this account. If the
                                // cheque expires, then after the expiry period,
                                // if it remains in the account,
                                // it is now the property of the transaction
                                // server.)
                                pVoucherReserveAcct->ReleaseSignatures();
                                pVoucherReserveAcct->SignContract(
                                    server_->m_nymServer);
                                pVoucherReserveAcct->SaveContract();
                                pVoucherReserveAcct->SaveAccount();

                                //
                                // PAY THE SHAREHOLDERS
                                //
                                // Here's where we actually loop through the
                                // asset accounts for the share type,
                                // and send a voucher to the owner of each one.
                                //
                                // This way, the actionPayDividend won't
                                // possibly load these accounts twice.
                                // We make them available here this way.
                                //
                                mapOfAccounts theAccounts;
                                theAccounts.insert(
                                    std::pair<std::string, OTAccount*>(
                                        strAccountID.Get(), &theSourceAccount));
                                theAccounts.insert(
                                    std::pair<std::string, OTAccount*>(
                                        strSharesIssuerAcct.Get(),
                                        pSharesIssuerAccount));
                                theAccounts.insert(
                                    std::pair<std::string, OTAccount*>(
                                        strVoucherAcctID.Get(),
                                        &theVoucherReserveAcct));

                                PayDividendVisitor actionPayDividend(
                                    SERVER_ID, USER_ID, PAYOUT_ASSET_ID,
                                    VOUCHER_ACCOUNT_ID,
                                    strInReferenceTo, // Memo for each voucher
                                                      // (containing original
                                                      // payout request pItem)
                                    *server_, lAmountPerShare, &theAccounts);

                                // Loops through all the accounts for a given
                                // asset type (PAYOUT_ASSET_ID), and triggers
                                // actionPayDividend for each one. This sends
                                // the owner nym for each, a voucher drawn on
                                // VOUCHER_ACCOUNT_ID. (In the amount of
                                // lAmountPerShare * number of shares in
                                // account.)
                                //
                                const bool bForEachAcct =
                                    pSharesContract->VisitAccountRecords(
                                        actionPayDividend); // <================
                                                            // pay all the
                                                            // dividends here.

                                // TODO: Since the above line of code loops
                                // through all the accounts and loads them
                                // up, transforms them, and saves them again, we
                                // cannot use our own loaded accounts below
                                // this point. (They could overwrite
                                // themselves.) theSourceAccount especially, was
                                // passed in
                                // from above -- so how can we possible warn the
                                // caller than he cannot save this account
                                // without
                                // overwriting work we have done in this
                                // function?
                                //
                                // Aside from any more elegant solution, the
                                // only way to make it work in this case would
                                // be to
                                // make a map or list of all the accounts that
                                // are already loaded in memory (such as
                                // theSourceAccount)
                                // and PASS THEM IN to the above
                                // VisitAccountRecords call. This way it would
                                // have the option to use
                                // the "already loaded" versions, where
                                // appropriate, instead of loading them twice.
                                // (As it is,
                                // theSourceAccount is not used below this
                                // point, though we couldn't preven the caller
                                // from using it.)
                                //
                                // Therefore we need to have some central system
                                // where accounts can be loaded, locked, saved,
                                // etc.
                                // So we cannot ever overwrite ourselves BY
                                // DESIGN. (And the same for other data types as
                                // well, like Nyms.)
                                // Todo.
                                //
                                if (!bForEachAcct) // todo failsafe. Handle this
                                                   // better.
                                {
                                    OTLog::vError(
                                        "%s: ERROR: After moving funds for "
                                        "dividend payment, there was some "
                                        "error when sending out the vouchers "
                                        "to the payout recipients.\n",
                                        szFunc);
                                }
                                //
                                // REFUND ANY LEFTOVERS
                                //
                                const int64_t lLeftovers =
                                    lTotalCostOfDividend -
                                    (actionPayDividend.GetAmountPaidOut() +
                                     actionPayDividend.GetAmountReturned());
                                if (lLeftovers > 0) {
                                    // Of the total amount removed from the
                                    // sender's account, and after paying all
                                    // dividends,
                                    // there was a leftover amount that wasn't
                                    // paid to anybody. Therefore, we should pay
                                    // it back
                                    // to the sender himself, now.
                                    //
                                    OTLog::vOutput(
                                        0, "%s: After dividend payout, with "
                                           "%lld units removed initially, "
                                           "there were %lld units remaining. "
                                           "(Returning them to sender...)\n",
                                        szFunc, lTotalCostOfDividend,
                                        lLeftovers);
                                    OTCheque theVoucher(SERVER_ID,
                                                        PAYOUT_ASSET_ID);

                                    // 10 minutes ==    600 Seconds
                                    // 1 hour    ==     3600 Seconds
                                    // 1 day    ==    86400 Seconds
                                    // 30 days    ==  2592000 Seconds
                                    // 3 months ==  7776000 Seconds
                                    // 6 months == 15552000 Seconds

                                    const time64_t VALID_FROM =
                                        OTTimeGetCurrentTime(); // This time is
                                                                // set to TODAY
                                                                // NOW
                                    const time64_t VALID_TO =
                                        OTTimeAddTimeInterval(
                                            VALID_FROM,
                                            OTTimeGetSecondsFromTime(
                                                OT_TIME_SIX_MONTHS_IN_SECONDS)); // This time occurs in 180 days (6 months).  Todo hardcoding.

                                    int64_t lNewTransactionNumber = 0;
                                    const bool bGotNextTransNum =
                                        server_->transactor_
                                            .issueNextTransactionNumber(
                                                server_->m_nymServer,
                                                lNewTransactionNumber); // bStoreTheNumber
                                    // defaults to
                                    // true. We save
                                    // the
                                    // transaction
                                    // number on the server Nym (normally we'd
                                    // discard it) because
                                    // when the cheque is deposited, the server
                                    // nym, as the owner of
                                    // the voucher account, needs to verify the
                                    // transaction # on the
                                    // cheque (to prevent double-spending of
                                    // cheques.)
                                    if (bGotNextTransNum) {
                                        const OTIdentifier SERVER_NYM_ID(
                                            server_->m_nymServer);
                                        const bool bIssueVoucher =
                                            theVoucher.IssueCheque(
                                                lLeftovers, // The amount of the
                                                            // cheque.
                                                lNewTransactionNumber, // Requiring
                                                                       // a
                                                // transaction
                                                // number
                                                // prevents
                                                // double-spending
                                                // of
                                                // cheques.
                                                VALID_FROM, // The expiration
                                                            // date
                                                // (valid from/to dates)
                                                // of the cheque
                                                VALID_TO, // Vouchers are
                                                // automatically starting
                                                // today and lasting 6
                                                // months.
                                                VOUCHER_ACCOUNT_ID, // The asset
                                                // account the
                                                // cheque is
                                                // drawn on.
                                                SERVER_NYM_ID, // User ID of the
                                                // sender (in this
                                                // case the server
                                                // nym.)
                                                strInReferenceTo, // Optional
                                                                  // memo
                                                // field. Includes
                                                // item note and
                                                // request memo.
                                                &USER_ID);

                                        // All account crediting / debiting
                                        // happens in the caller, in OTServer.
                                        //    (AND it happens only ONCE, to
                                        // cover ALL vouchers.)
                                        // Then in here, the voucher either gets
                                        // send to the recipient, or if error,
                                        // sent back home to
                                        // the issuer Nym. (ALL the funds are
                                        // removed, then the vouchers are sent
                                        // one way or the other.)
                                        // Any returned vouchers, obviously
                                        // serve to notify the dividend payer of
                                        // where the errors were
                                        // (as well as give him the opportunity
                                        // to get his money back.)
                                        //
                                        bool bSent = false;
                                        if (bIssueVoucher) {
                                            theVoucher.SetAsVoucher(
                                                SERVER_NYM_ID,
                                                VOUCHER_ACCOUNT_ID); // All this
                                                                     // does is
                                                                     // set the
                                            // voucher's
                                            // internal
                                            // contract
                                            // string
                                            theVoucher.SignContract(
                                                server_->m_nymServer); // to
                                            // "VOUCHER"
                                            // instead of
                                            // "CHEQUE".
                                            theVoucher.SaveContract();

                                            // Send the voucher to the payments
                                            // inbox of the recipient.
                                            //
                                            const OTString strVoucher(
                                                theVoucher);
                                            OTPayment thePayment(strVoucher);

                                            // calls DropMessageToNymbox
                                            bSent =
                                                server_->SendInstrumentToNym(
                                                    SERVER_ID,
                                                    SERVER_NYM_ID, // sender nym
                                                    USER_ID, // recipient nym
                                                             // (returning to
                                                    // original sender.)
                                                    nullptr, &thePayment,
                                                    "payDividend"); // todo:
                                            // hardcoding.
                                        }
                                        // If we didn't send it, then we need to
                                        // return the funds to where they came
                                        // from.
                                        //
                                        if (!bSent) {
                                            const OTString strPayoutAssetID(
                                                PAYOUT_ASSET_ID),
                                                strSenderUserID(USER_ID);
                                            OTLog::vError(
                                                "%s: ERROR failed issuing "
                                                "voucher (to return leftovers "
                                                "back to "
                                                "the dividend payout "
                                                "initiator.) WAS TRYING TO PAY "
                                                "%lld of asset type %s to Nym "
                                                "%s.\n",
                                                szFunc, lLeftovers,
                                                strPayoutAssetID.Get(),
                                                strSenderUserID.Get());
                                        }  // if !bSent
                                    }
                                    else // !bGotNextTransNum
                                    {
                                        const OTString strPayoutAssetID(
                                            PAYOUT_ASSET_ID),
                                            strRecipientUserID(USER_ID);
                                        OTLog::vError(
                                            "%s: ERROR!! Failed issuing next "
                                            "transaction "
                                            "number while trying to send a "
                                            "voucher (while returning leftover "
                                            "funds, after paying dividends.) "
                                            "WAS TRYING TO PAY %lld of asset "
                                            "type %s to Nym %s.\n",
                                            szFunc, lLeftovers,
                                            strPayoutAssetID.Get(),
                                            strRecipientUserID.Get());
                                    }
                                }
                            } // else
                        }
                        // else{} // TODO log that there was a problem with the
                        // amount

                    } // voucher request loaded successfully from string
                }     // server_->transactor_.getVoucherAccount()
                else {
                    OTLog::vError(
                        "%s: server_->transactor_.getVoucherAccount() failed. "
                        "Asset Type:\n%s\n",
                        szFunc, strAssetTypeID.Get());
                }
            }
        }
    }
    else {
        OTString strTemp(tranIn);
        OTLog::vOutput(
            0, "%s: Expected OTItem::payDividend in trans# %lld: \n\n%s\n\n",
            szFunc, tranIn.GetTransactionNum(),
            strTemp.Exists() ? strTemp.Get()
                             : " (ERROR LOADING TRANSACTION INTO STRING) ");
    }
    // sign the response item before sending it back (it's already been added to
    // the transaction above)
    // Now, whether it was rejection or acknowledgement, it is set properly and
    // it is signed, and it
    // is owned by the transaction, who will take it from here.
    pResponseItem->SignContract(server_->m_nymServer);
    pResponseItem->SaveContract(); // the signing was of no effect because I
                                   // forgot to save.

    pResponseBalanceItem->SignContract(server_->m_nymServer);
    pResponseBalanceItem->SaveContract();
}

/// for depositing a cheque or cash.
void Notary::NotarizeDeposit(OTPseudonym& theNym, OTAccount& theAccount,
                             OTTransaction& tranIn, OTTransaction& tranOut,
                             bool& bOutSuccess)
{
    // The outgoing transaction is an "atDeposit", that is, "a reply to the
    // deposit request"
    tranOut.SetType(OTTransaction::atDeposit);

    OTItem* pItem = nullptr;
    OTItem* pItemCheque = nullptr;
    OTItem* pItemCash = nullptr;
    OTItem* pBalanceItem = nullptr;
    OTItem* pResponseItem = nullptr;
    OTItem* pResponseBalanceItem = nullptr;

    // The incoming transaction may be sent to inboxes and outboxes, and it
    // will probably be bundled in our reply to the user as well. Therefore,
    // let's grab it as a string.
    OTString strInReferenceTo;
    OTString strBalanceItem;

    // Grab the actual server ID from this object, and use it as the server ID
    // here.
    const OTIdentifier SERVER_ID(server_->m_strServerID), USER_ID(theNym),
        ACCOUNT_ID(theAccount), SERVER_USER_ID(server_->m_nymServer),
        ASSET_TYPE_ID(theAccount.GetAssetTypeID());

    const OTString strUserID(USER_ID), strAccountID(ACCOUNT_ID);

    Mint* pMint = nullptr; // the Mint itself.
    OTAccount* pMintCashReserveAcct =
        nullptr; // the Mint's funds for cash withdrawals.
    // Here we find out if we're depositing cash, or a cheque
    //
    OTItem::itemType theReplyItemType = OTItem::error_state;

    pItemCheque = tranIn.GetItem(OTItem::depositCheque);

    if (nullptr == pItemCheque) {
        pItemCash = tranIn.GetItem(OTItem::deposit);
        pItem = pItemCash;
        theReplyItemType = OTItem::atDeposit;
    }
    else {
        pItem = pItemCheque;
        theReplyItemType = OTItem::atDepositCheque;
    }
    pResponseItem =
        OTItem::CreateItemFromTransaction(tranOut, theReplyItemType);
    pResponseItem->SetStatus(OTItem::rejection); // the default.
    tranOut.AddItem(*pResponseItem); // the Transaction's destructor will
                                     // cleanup the item. It "owns" it now.

    pResponseBalanceItem =
        OTItem::CreateItemFromTransaction(tranOut, OTItem::atBalanceStatement);
    pResponseBalanceItem->SetStatus(OTItem::rejection); // the default.
    tranOut.AddItem(*pResponseBalanceItem); // the Transaction's destructor will
                                            // cleanup the item. It "owns" it
                                            // now.
    if (nullptr == pItem) {
        OTString strTemp(tranIn);
        OTLog::vOutput(0, "Notary::NotarizeDeposit: Expected OTItem::deposit "
                          "or OTItem::depositCheque in trans# %lld: \n\n%s\n\n",
                       tranIn.GetTransactionNum(),
                       strTemp.Exists()
                           ? strTemp.Get()
                           : " (ERROR LOADING TRANSACTION INTO STRING) ");
    }
    // Below this point, we know that pItem is good, and that either pItemCheque
    // OR pItemCash is good,
    // and that pItem points to the good one. Therefore next, let's verify
    // permissions:

    // This permission has to do with ALL deposits (cash or cheque)
    else if (!NYM_IS_ALLOWED(strUserID.Get(),
                             ServerSettings::__transact_deposit)) {
        OTLog::vOutput(0, "Notary::NotarizeDeposit: User %s cannot do this "
                          "transaction (All deposits are disallowed in "
                          "server.cfg)\n",
                       strUserID.Get());
    }
    // This permission has to do with vouchers.
    else if ((nullptr != pItemCheque) &&
             (false ==
              NYM_IS_ALLOWED(strUserID.Get(),
                             ServerSettings::__transact_deposit_cheque))) {
        OTLog::vOutput(0, "Notary::NotarizeDeposit: User %s cannot do this "
                          "transaction (depositCheque is disallowed in "
                          "server.cfg)\n",
                       strUserID.Get());
    }
    // This permission has to do with cash.
    else if ((nullptr != pItemCash) &&
             (false ==
              NYM_IS_ALLOWED(strUserID.Get(),
                             ServerSettings::__transact_deposit_cash))) {
        OTLog::vOutput(0, "Notary::NotarizeDeposit: User %s cannot do this "
                          "transaction (deposit cash is disallowed in "
                          "server.cfg)\n",
                       strUserID.Get());
    }
    // Check for a balance agreement...
    //
    else if (nullptr ==
             (pBalanceItem = tranIn.GetItem(OTItem::balanceStatement))) {
        OTString strTemp(tranIn);
        OTLog::vOutput(
            0, "Notary::NotarizeDeposit: Expected OTItem::balanceStatement, "
               "but not found in trans # %lld: \n\n%s\n\n",
            tranIn.GetTransactionNum(),
            strTemp.Exists() ? strTemp.Get()
                             : " (ERROR LOADING TRANSACTION INTO STRING) ");
    }
    // DEPOSIT CHEQUE  (Deposit Cash is the bottom half of the function, deposit
    // cheque is the top half.)
    else if (pItem->GetType() == OTItem::depositCheque) {
        // The response item, as well as the sender's inbox, will soon contain a
        // copy
        // of the request item. So I save it into a string here so they can grab
        // a copy of it
        // into their "in reference to" fields.
        pItem->SaveContractRaw(strInReferenceTo);
        pBalanceItem->SaveContractRaw(strBalanceItem);

        // Server response item being added to server response transaction
        // (tranOut)
        // They're getting SOME sort of response item.

        pResponseItem->SetReferenceString(strInReferenceTo); // the response
                                                             // item carries a
                                                             // copy of what
                                                             // it's responding
                                                             // to.
        pResponseItem->SetReferenceToNum(
            pItem->GetTransactionNum()); // This response item is IN RESPONSE to
                                         // pItem and its Owner Transaction.

        pResponseBalanceItem->SetReferenceString(
            strBalanceItem); // the response item carries a copy of what it's
                             // responding to.
        pResponseBalanceItem->SetReferenceToNum(
            pItem->GetTransactionNum()); // This response item is IN RESPONSE to
                                         // pItem and its Owner Transaction.
        std::unique_ptr<OTLedger> pInbox(
            theAccount.LoadInbox(server_->m_nymServer));
        std::unique_ptr<OTLedger> pOutbox(
            theAccount.LoadOutbox(server_->m_nymServer));

        if (nullptr ==
            pInbox) // || !pInbox->VerifyAccount(server_->m_nymServer)) Verified
                    // in OTAccount::Load
        {
            OTLog::Error("Notary::NotarizeDeposit: Error loading or "
                         "verifying inbox for depositor account.\n");
        }
        else if (nullptr ==
                   pOutbox) // || !pOutbox->VerifyAccount(server_->m_nymServer))
                            // Verified in OTAccount::Load
        {
            OTLog::Error("Notary::NotarizeDeposit: Error loading or "
                         "verifying outbox for depositor account.\n");
        }
        // NOTE: Below this point, pInbox and pOutbox are both available, AND
        // will be properly
        // cleaned up automatically upon scope exit.
        // If the ID on the "from" account that was passed in,
        // does not match the "Acct From" ID on this transaction item
        else if (ACCOUNT_ID != pItem->GetPurportedAccountID()) {
            // TODO: Verify that this if block is unnecessary, and if so, remove
            // it.
            // (The item should already have been verified when the transaction
            // itself was
            // verified, before this function was even called. I think this is
            // just an oversight.)
            OTLog::Output(0, "Notary::NotarizeDeposit: Error: account ID "
                             "does not match account ID on the deposit "
                             "item.\n");
        }
        else {
            // Get the cheque from the Item and load it up into a Cheque object.
            OTString strCheque;
            pItem->GetAttachment(strCheque);
            OTCheque theCheque(SERVER_ID,
                               ASSET_TYPE_ID); // allocated on the stack :-)
            bool bLoadContractFromString =
                theCheque.LoadContractFromString(strCheque);

            if (!bLoadContractFromString) {
                OTLog::vError("%s: ERROR loading cheque from string:\n%s\n",
                              __FUNCTION__, strCheque.Get());
            }
            // See if the cheque is drawn on the same server as the deposit
            // account
            // (the server running this code right now.)
            //
            else if (SERVER_ID != theCheque.GetServerID()) {
                const OTString strSenderUserID(theCheque.GetSenderUserID());
                const OTString strRecipientUserID(
                    theCheque.GetRecipientUserID());
                OTLog::vOutput(0, "%s: Cheque rejected (%lld): "
                                  "Incorrect Server ID on cheque. Sender User "
                                  "ID: %s\nRecipient User ID is: %s\n",
                               __FUNCTION__, theCheque.GetTransactionNum(),
                               strSenderUserID.Get(), strRecipientUserID.Get());
            }
            // See if the cheque is already expired or otherwise not within it's
            // valid date range.
            else if (!theCheque.VerifyCurrentDate()) {
                const OTString strSenderUserID(theCheque.GetSenderUserID());
                const OTString strRecipientUserID(
                    theCheque.GetRecipientUserID());
                OTLog::vOutput(0, "%s: Cheque rejected (%lld): "
                                  "Not within valid date range. Sender User "
                                  "ID: %s\nRecipient User ID: %s\n",
                               __FUNCTION__, theCheque.GetTransactionNum(),
                               strSenderUserID.Get(), strRecipientUserID.Get());
            }
            // You can't deposit a cheque into the same account that it's drawn
            // on. (Otherwise, in loading
            // both accounts, I would cause one of them to  be overwritten. I'm
            // not willing to do the same
            // pointer magic that I'm doing with Nyms... instead I just disallow
            // this entirely.)
            //
            // UPDATE: New rule: If you deposit a cheque into the same account
            // it's drawn on,
            // the effect is the cancellation of the cheque. (If we are in this
            // block, it means
            // the original cheque writer is now trying to cancel the cheque
            // before the recipient
            // has a chance to deposit it.)
            //
            // CANCELLATION OF CHEQUES:
            //
            else if (ACCOUNT_ID ==
                     theCheque.GetSenderAcctID()) // Depositor ACCOUNT_ID is
                                                  // recipient's acct. (theNym.)
                                                  // But pSenderNym is normally
                                                  // someone else (the sender).
            {
                // UPDATE: This block is now for cancelling the cheque before
                // the original
                // recipient manages to deposit it.

                const OTString strSenderUserID(theCheque.GetSenderUserID());
                const OTString strRecipientUserID(
                    theCheque.GetRecipientUserID());
                if (theCheque.GetSenderUserID() != USER_ID) {
                    OTLog::vOutput(
                        0,
                        "%s: Failure verifying cheque: Strange: while the "
                        "depositing account has the "
                        "same ID as the cheque sender acct, somehow the user "
                        "IDs do not match. (Should never happen.)\n"
                        "Cheque Sender User ID: %s\n Depositor User ID: %s\n",
                        __FUNCTION__, strSenderUserID.Get(), strUserID.Get());
                }
                else if (theCheque.GetAmount() != 0) {
                    OTLog::vOutput(
                        0, "%s: Failure verifying cheque: While attempting to "
                           "perform cancellation, "
                           "the cheque has a non-zero amount.\n"
                           "Sender User ID: %s\nRecipient User ID: %s\n",
                        __FUNCTION__, strSenderUserID.Get(),
                        strRecipientUserID.Get());
                }
                // Make sure the transaction number on the cheque is still
                // available and valid for use by theNym.
                //
                else if (false ==
                         server_->transactor_.verifyTransactionNumber(
                             theNym, theCheque.GetTransactionNum())) {
                    OTLog::vOutput(
                        0, "%s: Failure verifying cheque: Bad transaction "
                           "number.\n"
                           "Sender User ID: %s\nRecipient User ID: %s\n",
                        __FUNCTION__, strSenderUserID.Get(),
                        strRecipientUserID.Get());
                }
                // Let's see if the sender's signature matches the one on the
                // cheque...
                else if (!theCheque.VerifySignature(theNym)) {
                    OTLog::vOutput(0,
                                   "%s: Failure verifying cheque signature for "
                                   "sender ID: %s\nRecipient User ID: %s\n "
                                   "Cheque:\n\n%s\n\n",
                                   __FUNCTION__, strSenderUserID.Get(),
                                   strRecipientUserID.Get(), strCheque.Get());
                }
                else if (!(pBalanceItem->VerifyBalanceStatement(
                               theCheque.GetAmount(), // This amount is always
                                                      // zero in the case of
                                                      // cheque cancellation.
                               theNym, *pInbox, *pOutbox, theAccount,
                               tranIn))) {
                    OTLog::vOutput(0, "%s: ERROR verifying balance statement "
                                      "while cancelling cheque %lld. Acct "
                                      "ID:\n%s\n",
                                   __FUNCTION__, theCheque.GetTransactionNum(),
                                   strAccountID.Get());
                }
                else {
                    pResponseBalanceItem->SetStatus(
                        OTItem::acknowledgement); // the transaction agreement
                                                  // was successful.

                    if ( // Clear the transaction number. Sender Nym was
                        // responsible for it (and still is, until
                        // he signs to accept the cheque reecipt). Still,
                        // however, he HAS used the cheque, so
                        // I'm removing his ability to use that number
                        // twice. It will remain on his issued list
                        // until he signs for the receipt.
                        //
                        (false ==
                         server_->transactor_.removeTransactionNumber(
                             theNym, theCheque.GetTransactionNum(),
                             true)) // bSave=true
                        ) {
                        OTLog::vError("%s: Failed marking the transaction "
                                      "number as in use. (Should never "
                                      "happen.)\n",
                                      __FUNCTION__);
                    }
                    else {
                        // Generate new transaction number (for putting the
                        // check receipt in the sender's inbox.)
                        // todo check this generation for failure (can it fail?)
                        int64_t lNewTransactionNumber = 0;

                        server_->transactor_.issueNextTransactionNumber(
                            server_->m_nymServer, lNewTransactionNumber,
                            false); // bStoreTheNumber = false

                        OTTransaction* pInboxTransaction =
                            OTTransaction::GenerateTransaction(
                                *pInbox, OTTransaction::chequeReceipt,
                                lNewTransactionNumber);

                        // The depositCheque request OTItem is saved as a "in
                        // reference to" field,
                        // on the inbox chequeReceipt transaction.
                        // todo put these two together in a method.
                        pInboxTransaction->SetReferenceString(strInReferenceTo);
                        pInboxTransaction->SetReferenceToNum(
                            pItem->GetTransactionNum());

                        pInboxTransaction->SetAsCancelled();

                        // Now we have created a new transaction from the server
                        // to the sender's inbox
                        // Let's sign and save it...
                        pInboxTransaction->SignContract(server_->m_nymServer);
                        pInboxTransaction->SaveContract();

                        // Here the transaction we just created is actually
                        // added to the source acct's inbox.
                        pInbox->AddTransaction(*pInboxTransaction);

                        // Release any signatures that were there before (They
                        // won't
                        // verify anymore anyway, since the content has
                        // changed.)
                        //
                        pInbox->ReleaseSignatures();
                        pInbox->SignContract(server_->m_nymServer);
                        pInbox->SaveContract();

                        theAccount.SaveInbox(*pInbox);

                        // Any inbox/nymbox/outbox ledger will only itself
                        // contain
                        // abbreviated versions of the receipts, including their
                        // hashes.
                        //
                        // The rest is stored separately, in the box receipt,
                        // which is created
                        // whenever a receipt is added to a box, and deleted
                        // after a receipt
                        // is removed from a box.
                        //
                        pInboxTransaction->SaveBoxReceipt(*pInbox);
                        // AT THIS POINT, the sender/depositor's inbox has had
                        // the cheque transaction added to it,
                        // as his receipt. (He must perform a balance agreement
                        // in order to get it out of his inbox.)
                        //
                        // THERE IS NOTHING LEFT TO DO BUT SAVE THE FILES!!

                        theAccount.ReleaseSignatures();
                        theAccount.SignContract(server_->m_nymServer);
                        theAccount.SaveContract();
                        theAccount.SaveAccount();

                        // Now we can set the response item as an
                        // acknowledgement instead of the default (rejection)
                        // otherwise, if we never entered this block, then it
                        // would still be set to rejection, and the
                        // new item would never have been added to the inbox,
                        // and the inbox file would never have had
                        // its signatures released, or been re-signed or
                        // re-saved back to file.
                        // BUT... if the message comes back with
                        // acknowledgement--then all of these actions must have
                        // happened, and here is the server's signature to prove
                        // it.
                        // Otherwise you get no items and no signature. Just a
                        // rejection item in the response transaction.
                        //
                        pResponseItem->SetStatus(OTItem::acknowledgement);

                        bOutSuccess =
                            true; // The cheque cancellation was successful.

                        OTLog::vOutput(
                            0, "%s: SUCCESS cancelling cheque %lld, which had "
                               "been drawn from account: %s\n",
                            __FUNCTION__, theCheque.GetTransactionNum(),
                            strAccountID.Get());

                        tranOut.SetAsCancelled();

                        // TODO: Our code that actually saves the new balance
                        // statement receipt should go here
                        // (that is, only after ultimate success.) Otherwise we
                        // still want to store the old receipt.
                        // For now I'm verifying it, but not storing it.  This
                        // means the security for it works, but
                        // in a dispute, I can't prove it / cover my ass.  So
                        // very soon a receipt WILL be saved here
                        // that is, a copy of the user's signed
                        // BalanceAgreement.
                        // Note: if I'm saving the entire outgoing transaction
                        // reply, or message reply, (versus just
                        // the reply to a certain item) then I think I have the
                        // balance agreement already? Double check.
                    }
                } // "else"
            }     // ABOVE: Cheque cancellation
            else  // BELOW: Cheque deposit
            {
                const OTIdentifier& SOURCE_ACCT_ID(
                    theCheque.GetSenderAcctID()); // relevant for both vouchers
                                                  // AND cheques.
                const OTIdentifier& SENDER_USER_ID(theCheque.GetSenderUserID());

                const OTIdentifier& REMITTER_ACCT_ID(
                    theCheque.GetRemitterAcctID()); // only relevant in case of
                                                    // vouchers.
                const OTIdentifier& REMITTER_USER_ID(
                    theCheque.GetRemitterUserID());

                // If the cheque has a remitter ...and the depositor IS the
                // remitter...
                // (Then the remitter is cancelling the voucher.)
                //
                const bool bHasRemitter = theCheque.HasRemitter();
                const bool bRemitterCancelling =
                    (bHasRemitter &&
                     (USER_ID == theCheque.GetRemitterUserID()));

                // The point of the logic here is to enable remitters to deposit
                // vouchers. Basically allows
                // them to "cancel" the voucher, and get their money back.
                //
                const OTIdentifier& RECIPIENT_USER_ID(
                    bRemitterCancelling ? USER_ID
                                        : theCheque.GetRecipientUserID());

                // (Note that we allow the remitter of a voucher to deposit that
                // voucher.)

                const OTString strSenderUserID(SENDER_USER_ID),
                    strSourceAcctID(SOURCE_ACCT_ID),
                    strRecipientUserID(RECIPIENT_USER_ID),
                    strRemitterUserID(REMITTER_USER_ID),
                    strRemitterAcctID(REMITTER_ACCT_ID);
                OTLedger theSenderInbox(SENDER_USER_ID, SOURCE_ACCT_ID,
                                        SERVER_ID); // chequeReceipt goes here.
                OTLedger theRemitterInbox(
                    REMITTER_USER_ID, REMITTER_ACCT_ID,
                    SERVER_ID); // voucherReceipt goes here.
                OTLedger* pSenderInbox = &theSenderInbox;
                OTLedger* pRemitterInbox = &theRemitterInbox;
                OTAccount* pRemitterAcct =
                    nullptr; // Only used in the case of vouchers.
                std::unique_ptr<OTAccount> theRemitterAcctGuardian;
                OTAccount* pSourceAcct =
                    nullptr; // We'll load this up and change
                             // its balance, save it then
                             // delete the instance.
                std::unique_ptr<OTAccount> theSourceAcctGuardian;
                // OTAccount::LoadExistingAccount().
                OTPseudonym theRemitterNym(REMITTER_USER_ID);
                OTPseudonym* pRemitterNym = &theRemitterNym;
                OTPseudonym theSenderNym(SENDER_USER_ID);
                OTPseudonym* pSenderNym = &theSenderNym;

                // Don't want to overwrite files or db records in cases where
                // the sender is also the depositor.
                // (Similar concern if the server is also the depositor, but
                // that's already partly handled
                // before we get to this point... theNym is already substituted
                // for m_nymServer,
                // if the IDs match, before any of the commands are processed.)
                //
                // The conundrum is in the multiplicity of combinations. The
                // server COULD be the sender
                // OR the depositor, or he could be BOTH, or the server might
                // NOT be the sender OR depositor,
                // yet they could still be the same person.  Normally all 3 are
                // separate entities.  That's a
                // lot of combinations. I want to make sure that I don't
                // overwrite ANY files while juggling the
                // respective nymfiles, transaction numbers, request numbers,
                // etc.
                //
                // Solution:  When commands are first processed, and the Request
                // Number is processed, theNym
                // is ALREADY replaced with m_nymServer IF the IDs match and the
                // signature verifies. It is then
                // passed that way to all of the commands (including the one we
                // are in now.)
                //
                // I THEN do the logic BELOW as additional to that. Make sure if
                // you change anything that you
                // think int64_t and hard about what you are doing!!
                //
                // Here's the logic:
                // -- theNym is the depositor (for sure.)
                // -- m_nymServer is the server nym (for sure.)
                // -- By the time we get here, IF the ServerUserID and UserID
                // match,
                //      then theNym IS ALREADY m_nymServer, literally a
                // reference to it.
                //    (This is performed before we even get to this point, so
                // that the
                //      same problem doesn't occur with request numbers.)
                // -- In cases where the "server is also the depositor", it's
                // therefore
                //      ALREADY handled, since theNym already points to
                // m_nymServer.
                // -- theSenderNym is used to load the sender nym in cases where
                // we have
                //      to load it from file or db ourselves. (Most normal
                // cases.)
                // -- In those normal cases, pSenderNym will point to
                // theSenderNym and
                //      we load it up from file or database.
                // -- In cases where the sender is also the user (the
                // depositor), then
                //      pSenderNym will point to theNym instead of to
                // theSenderNym, and we
                //      no longer bother loading it, since the user is already
                // loaded.
                // -- In cases where the sender is also the server, then
                // pSenderNym will
                //      point to m_nymServer instead of to theSenderNym, and we
                // no longer
                //      bother loading it, since the server's nym is already
                // loaded.

                bool bDepositorIsServer = (USER_ID == SERVER_USER_ID);
                bool bDepositorIsSender = (USER_ID == SENDER_USER_ID);
                bool bDepositorIsRemitter = (USER_ID == REMITTER_USER_ID);
                bool bDepositAcctIsRemitter = (ACCOUNT_ID == REMITTER_ACCT_ID);
                bool bSourceAcctIsRemitter =
                    (SOURCE_ACCT_ID == REMITTER_ACCT_ID);
                bool bSenderIsServer = (SENDER_USER_ID == SERVER_USER_ID);
                bool bRemitterIsServer = (REMITTER_USER_ID == SERVER_USER_ID);
                bool bSenderUserAlreadyLoaded = false;
                bool bSourceAcctAlreadyLoaded = false;
                bool bRemitterUserAlreadyLoaded = false;
                bool bRemitterAcctAlreadyLoaded = false;

                // The depositor is already loaded, (for sure.)

                // The server is already loaded, (for sure.)

                // If the depositor IS the server, then it already points there
                // (for sure.)
                if (bDepositorIsServer) {
                    // OTPseudonym & theNym = m_nymServer; // Already handled in
                    // the code that calls IncrementRequestNum().

                    // bSenderUserAlreadyLoaded = false; // Sender is either
                    // determined to be already loaded (directly below) or
                    // it is loaded as part of the cheque verification process
                    // below that.
                    // At this point I can't set it because I just don't know
                    // yet.
                }
                // If the depositor IS the sender, pSenderNym points to
                // depositor, and we're already loaded.
                if (bDepositorIsSender) {
                    pSenderNym = &theNym;
                    bSenderUserAlreadyLoaded = true;
                }
                // If the server IS the sender, pSenderNym points to the server,
                // and we're already loaded.
                if (bSenderIsServer) {
                    pSenderNym = &server_->m_nymServer;
                    bSenderUserAlreadyLoaded = true;
                }
                bool bSuccessLoadSenderInbox = false;
                bool bSuccessLoadRemitterInbox = false;

                if (bHasRemitter) {
                    pSenderInbox = nullptr;

                    // If the depositor IS the remitter, pRemitterNym points to
                    // depositor, and we're already loaded.
                    if (bDepositorIsRemitter) {
                        pRemitterNym = &theNym;
                        bRemitterUserAlreadyLoaded = true;
                    }
                    // If the server IS the remitter, pRemitterNym points to the
                    // server, and we're already loaded.
                    if (bRemitterIsServer) {
                        pRemitterNym = &server_->m_nymServer;
                        bRemitterUserAlreadyLoaded = true;
                    }
                    // If the account IS the remitter's account, pRemitterNym
                    // points to the server, and we're already loaded.
                    if (bDepositAcctIsRemitter) {
                        pRemitterAcct = &theAccount;
                        pRemitterInbox = pInbox.get();

                        bRemitterAcctAlreadyLoaded = true;
                        bSuccessLoadRemitterInbox = true;

                        // Just for completeness. In this case ALL THREE
                        // accounts would be the same account,
                        // which is probably disallowed later on anyway.
                        //
                        if (bSourceAcctIsRemitter) {
                            pSourceAcct = pRemitterAcct;
                            pSenderInbox = pRemitterInbox;

                            bSourceAcctAlreadyLoaded = true;
                            bSuccessLoadSenderInbox = true;
                        }
                    }
                }
                else // Definitely has no remitter. Therefore definitely NOT a
                       // voucher.
                { // (If it's not a voucher, that means we should DEFINITELY be
                    // able to load the sender's inbox.)
                    // Load source account's inbox

                    bSuccessLoadSenderInbox = pSenderInbox->LoadInbox();

                    // ...If it loads, verify it. Otherwise, generate it...
                    if (bSuccessLoadSenderInbox)
                        bSuccessLoadSenderInbox =
                            pSenderInbox->VerifyAccount(server_->m_nymServer);
                    else
                        OTLog::vOutput(0, "Notary::%s: Failed loading inbox "
                                          "for %s source account.\n",
                                       __FUNCTION__,
                                       (bHasRemitter) ? "cheque" : "voucher");
                    //                  else
                    //                      bSuccessLoadSenderInbox =
                    // pSenderInbox->GenerateLedger(SOURCE_ACCT_ID, SERVER_ID,
                    // OTLedger::inbox, true); // bGenerateFile=true
                }
                // To deposit a cheque, need to verify:  (in no special order)
                //
                // -- DONE Load the source account and verify it exists.
                // -- DONE Make sure the source acct is verified for the server
                // signature.
                // -- DONE Make sure the Asset Type of the cheque matches the
                // Asset Type of BOTH source and recipient accts.
                // -- DONE See if there is a Recipient User ID. If so, MAKE SURE
                // it matches the user depositing the cheque!
                // -- DONE See if the Sender User even exists.
                // -- DONE See if the Sender User ID matches the hash of the
                // actual public key in the sender's pubkey file.
                // -- DONE Make sure the cheque has the right server ID.
                // -- DONE Make sure the cheque has not yet EXPIRED.
                // -- DONE Make sure the cheque signature is verified with the
                // sender's pubkey.
                // -- DONE Make sure the account ID on the transaction item
                // matches the depositor account ID.
                // -- DONE Verify the funds are in the source acct.
                //
                // -- DONE Plus deal with sender's inbox.

                // If there's a remitter, then it's a voucher, and thus the
                // sender MUST be the server.
                // (If the sender on a voucher is NOT the server, then it's very
                // suspicious.)
                //
                if (bHasRemitter && !bSenderIsServer) {
                    OTLog::vOutput(0, "Notary::%s: Failure: Voucher has a "
                                      "'sender' who is not the server: %s\n",
                                   __FUNCTION__, strSenderUserID.Get());
                }

                // See if we can load the sender's public key (to verify cheque
                // signature,
                // and to remove transaction number if it's not a voucher.)
                // if !bSenderUserAlreadyLoaded since the server already had its
                // public key loaded at boot-time.
                // (also since the depositor and sender might be the same
                // person.)
                else if (!bSenderUserAlreadyLoaded &&
                         (false ==
                          theSenderNym.LoadPublicKey()) // Old style (The call
                                                        // on this line is
                                                        // deprecated.) NOTE:
                                                        // LoadPublicKey already
                                                        // calls LoadCredentials
                                                        // at its top, though
                                                        // eventually we'll just
                                                        // call it here
                                                        // directly, once
                                                        // LoadPublicKey is
                                                        // removed for good.
                         ) {
                    OTLog::vOutput(
                        0, "Notary::%s: Error loading public key for %s "
                           "signer during "
                           "deposit: %s\nRecipient User ID: %s\n",
                        __FUNCTION__, (bHasRemitter) ? "cheque" : "voucher",
                        strSenderUserID.Get(), strRecipientUserID.Get());
                }

                // See if the Nym ID (User ID) that we have matches the message
                // digest of his public key.
                else if (!pSenderNym->VerifyPseudonym()) {
                    OTLog::vOutput(
                        0, "Notary::%s: Failure verifying %s: "
                           "Bad Sender User ID (fails hash check of pubkey)"
                           ": %s\nRecipient User ID: %s\n",
                        __FUNCTION__, (bHasRemitter) ? "cheque" : "voucher",
                        strSenderUserID.Get(), strRecipientUserID.Get());
                }

                // See if we can load the sender's nym file (to verify the
                // transaction # on the cheque)
                // if !bSenderUserAlreadyLoaded since the server already had its
                // nym file loaded at boot-time.
                // (also since the depositor and sender might be the same
                // person.)
                else if (!bSenderUserAlreadyLoaded &&
                         (false ==
                          theSenderNym.LoadSignedNymfile(
                              server_->m_nymServer))) {
                    OTLog::vOutput(
                        0, "Notary::%s: Error loading nymfile for %s signer "
                           "during deposit, user ID: %s\n"
                           "Recipient User ID: %s\n",
                        __FUNCTION__, (bHasRemitter) ? "cheque" : "voucher",
                        strSenderUserID.Get(), strRecipientUserID.Get());
                }
                // See if we can load the remitter's public key (if it's a
                // voucher.)
                // if !bRemitterAlreadyLoaded since the server already had its
                // public key loaded at boot-time.
                // (also since the depositor or server, and remitter, might be
                // the same person.)
                else if (bHasRemitter && !bRemitterUserAlreadyLoaded &&
                         (false ==
                          theRemitterNym.LoadPublicKey()) // Old style (The call
                                                          // on this line is
                                                          // deprecated.) NOTE:
                                                          // LoadPublicKey
                                                          // already calls
                                                          // LoadCredentials at
                                                          // its top, though
                                                          // eventually we'll
                                                          // just call it here
                                                          // directly, once
                                                          // LoadPublicKey is
                                                          // removed for good.
                         ) {
                    OTLog::vOutput(
                        0, "Notary::%s: Error loading public key for "
                           "remitter during "
                           "voucher deposit: %s\nRecipient User ID: %s\n",
                        __FUNCTION__, strRemitterUserID.Get(),
                        strRecipientUserID.Get());
                }

                // See if the Nym ID (User ID) that we have matches the message
                // digest of his public key.
                else if (bHasRemitter && !pRemitterNym->VerifyPseudonym()) {
                    OTLog::vOutput(
                        0, "Notary::%s: Failure verifying voucher: "
                           "Bad Remitter User ID (fails hash check of pubkey)"
                           ": %s\nRecipient User ID: %s\n",
                        __FUNCTION__, strRemitterUserID.Get(),
                        strRecipientUserID.Get());
                }

                // See if we can load the remitter's nym file (to verify the
                // transaction # on the cheque)
                // if !bRemitterUserAlreadyLoaded since the server already had
                // its nym file loaded at boot-time.
                // (also since the depositor and remitter might be the same
                // person.)
                else if (bHasRemitter && !bRemitterUserAlreadyLoaded &&
                         (false ==
                          theRemitterNym.LoadSignedNymfile(
                              server_->m_nymServer))) {
                    OTLog::vOutput(0, "Notary::%s: Error loading nymfile for "
                                      "remitter during voucher deposit: %s\n"
                                      "Recipient User ID: %s\n",
                                   __FUNCTION__, strRemitterUserID.Get(),
                                   strRecipientUserID.Get());
                }
                // Make sure they're not double-spending this cheque.
                //
                else if (false ==
                         server_->transactor_.verifyTransactionNumber(
                             *(bHasRemitter ? pRemitterNym : pSenderNym),
                             theCheque.GetTransactionNum())) {
                    OTLog::vOutput(0, "Notary::%s: Failure verifying %s: Bad "
                                      "transaction number.\n"
                                      "Recipient User ID: %s\n",
                                   __FUNCTION__,
                                   (bHasRemitter) ? "cheque" : "voucher",
                                   strRecipientUserID.Get());
                }
                // Let's see if the sender's signature matches the one on the
                // cheque...
                else if (false ==
                         theCheque.VerifySignature(*pSenderNym)) // This is same
                                                                 // for cheques
                // and vouchers.
                {
                    OTLog::vOutput(
                        0, "Notary::%s: Failure verifying %s signature for "
                           "sender ID: %s Recipient User ID: %s",
                        __FUNCTION__, (bHasRemitter) ? "cheque" : "voucher",
                        strSenderUserID.Get(), strRecipientUserID.Get());
                    if (bHasRemitter)
                        OTLog::vOutput(0, " Remitter User ID: %s\n",
                                       strRemitterUserID.Get());
                    else
                        OTLog::Output(0, "\n");
                }
                // If there is a recipient user ID on the check, it must match
                // the user depositing the cheque.
                // (But if there is NO recipient user ID, then it's a blank
                // cheque and ANYONE can deposit it.)
                else if (theCheque.HasRecipient() &&
                         !(USER_ID == RECIPIENT_USER_ID)) {
                    OTLog::vOutput(0, "%s: Failure matching %s recipient to "
                                      "depositor. Depositor User ID: %s "
                                      "Recipient User ID: %s\n",
                                   __FUNCTION__,
                                   (bHasRemitter) ? "cheque" : "voucher",
                                   strUserID.Get(), strRecipientUserID.Get());
                }
                // Try to load the source account (that cheque is drawn on) up
                // into memory.
                // We'll need to debit funds from it...  Also, set the cleanup
                // object onto this pointer.
                else if (!bSourceAcctAlreadyLoaded &&
                         ((nullptr ==
                           (pSourceAcct = OTAccount::LoadExistingAccount(
                                SOURCE_ACCT_ID, SERVER_ID))) ||
                          (theSourceAcctGuardian.reset(pSourceAcct),
                           false // I want this to eval to false, but I want
                                 // SetCleanup to call.
                           )) // Also, SetCleanup() is safe even if pointer is
                              // nullptr.
                         ) {
                    OTLog::vOutput(
                        0, "%s: %s deposit failure, "
                           "trying to load source acct, ID: %s Recipient User "
                           "ID: %s",
                        __FUNCTION__, (bHasRemitter) ? "Cheque" : "Voucher",
                        strSourceAcctID.Get(), strRecipientUserID.Get());
                    if (bHasRemitter)
                        OTLog::vOutput(0, " Remitter User ID: %s\n",
                                       strRemitterUserID.Get());
                    else
                        OTLog::Output(0, "\n");
                }
                // If the cheque is a voucher (drawn on an internal server
                // account) then there is no need to
                // load any "sender inbox" (which will not exist anyway.)
                // Whereas if the source account is NOT
                // an internal server account (but rather, is a NORMAL account)
                // then it had better have
                // successfully loaded that inbox, or we have a problem.
                //
                else if (!bSuccessLoadSenderInbox &&
                         !pSourceAcct->IsInternalServerAcct()) {
                    OTLog::vError("%s: ERROR verifying inbox ledger for source "
                                  "acct ID: %s\n",
                                  __FUNCTION__, strSourceAcctID.Get());
                }
                // Does the source account verify?
                // I call VerifySignature here since VerifyContractID was
                // already called in LoadExistingAccount().
                // (Otherwise I might normally call VerifyAccount(), which does
                // both...)
                //
                // Someone can't just alter an account file, because then the
                // server's signature
                // won't verify anymore on that file and transactions will fail
                // such as right here:
                //
                else if (!pSourceAcct->VerifySignature(server_->m_nymServer)) {
                    OTLog::vOutput(
                        0, "%s: ERROR verifying signature on source account "
                           "while depositing %s. Acct ID: %s\n",
                        __FUNCTION__, (bHasRemitter) ? "cheque" : "voucher",
                        strAccountID.Get());
                }
                // Need to make sure the signer is the owner of the source
                // account...
                else if (!pSourceAcct->VerifyOwner(*pSenderNym)) {
                    OTLog::vOutput(0, "Notary::%s: ERROR verifying signer's "
                                      "ownership of source account while "
                                      "depositing %s. Source Acct ID: %s\n",
                                   __FUNCTION__,
                                   (bHasRemitter) ? "cheque" : "voucher",
                                   strAccountID.Get());
                }
                else {
                    if (bSourceAcctIsRemitter) {
                        pRemitterAcct = pSourceAcct;
                        bRemitterAcctAlreadyLoaded = true;

                        pRemitterInbox = pSenderInbox;

                        if (nullptr != pRemitterInbox) // This part is here for
                                                       // completeness only. It
                                                       // will never actually
                                                       // happen,
                            bSuccessLoadRemitterInbox =
                                true; // since a voucher source account is owned
                                      // by server and has no inbox.
                    }

                    // Try to load the remitter account (that voucher was
                    // originally financed from) up into memory.
                    // We'll need to verify it so we can drop the receipt into
                    // its inbox. Also, set the cleanup object onto this
                    // pointer.
                    //
                    if (bHasRemitter && !bRemitterAcctAlreadyLoaded &&
                        ((nullptr ==
                          (pRemitterAcct = OTAccount::LoadExistingAccount(
                               REMITTER_ACCT_ID, SERVER_ID))) ||
                         (theRemitterAcctGuardian.reset(pRemitterAcct),
                          false // I want this to eval to false, but I want
                                // SetCleanup to call.
                          )     // Also, SetCleanup() is safe even if pointer is
                                // nullptr.
                         )) {
                        OTLog::vOutput(
                            0, "%s: Voucher deposit, failure "
                               "trying to load remitter acct, ID: %s Recipient "
                               "User ID: %s Remitter User ID: %s\n",
                            __FUNCTION__, strRemitterAcctID.Get(),
                            strRecipientUserID.Get(), strRemitterUserID.Get());
                    }
                    // Does the remitter account verify?
                    // I call VerifySignature here since VerifyContractID was
                    // already called in LoadExistingAccount().
                    // (Otherwise I might normally call VerifyAccount(), which
                    // does both...)
                    //
                    else if (bHasRemitter &&
                             !pRemitterAcct->VerifySignature(
                                 server_->m_nymServer)) {
                        OTLog::vOutput(0, "%s: ERROR verifying signature on "
                                          "remitter account while depositing "
                                          "voucher. "
                                          "Remitter Acct ID: %s\n",
                                       __FUNCTION__, strRemitterAcctID.Get());
                    }
                    // Need to make sure the remitter is the owner of the
                    // remitter account...
                    else if (bHasRemitter &&
                             !pRemitterAcct->VerifyOwner(*pRemitterNym)) {
                        OTLog::vOutput(
                            0, "Notary::%s: ERROR verifying remitter's "
                               "ownership of remitter account while "
                               "depositing voucher. Remitter Acct ID: %s\n",
                            __FUNCTION__, strRemitterAcctID.Get());
                    }
                    else if (bHasRemitter && // If it's a voucher, and the
                                               // source account isn't the
                                               // remitter account...
                               !bSourceAcctIsRemitter && // (if it was, there'd
                                                         // definitely be no
                               // remitter inbox, since
                               // it'd be a server
                               // acct.)
                               !bSuccessLoadRemitterInbox && // ...and if we
                                                             // haven't
                                                             // successfully
                                                             // loaded the
                                                             // remitter's inbox
                                                             // yet...
                               (!pRemitterInbox->LoadInbox() ||
                                !pRemitterInbox->VerifyAccount(
                                    server_->m_nymServer))) // Then load and
                                                            // verify it.
                    {
                        OTLog::vError("%s: ERROR loading or verifying inbox "
                                      "ledger for remitter acct ID: %s\n",
                                      __FUNCTION__, strRemitterAcctID.Get());
                    }
                    // Are all of the accounts, AND the cheque, all of the same
                    // Asset Type?
                    else if (!(theCheque.GetAssetID() ==
                               pSourceAcct->GetAssetTypeID()) ||
                             !(theCheque.GetAssetID() ==
                               theAccount.GetAssetTypeID()) ||
                             (bHasRemitter &&
                              !(theCheque.GetAssetID() ==
                                pRemitterAcct->GetAssetTypeID()))) {
                        OTString strSourceAssetID(
                            pSourceAcct->GetAssetTypeID()),
                            strRecipientAssetID(theAccount.GetAssetTypeID());
                        OTLog::vOutput(
                            0, "Notary::%s: ERROR - user attempted to "
                               "deposit cheque between accounts of different "
                               "asset types. Source Acct: %s\nType: "
                               "%s\nRecipient Acct: %s\nType: %s\n",
                            __FUNCTION__, strSourceAcctID.Get(),
                            strSourceAssetID.Get(), strAccountID.Get(),
                            strRecipientAssetID.Get());

                        if (bHasRemitter) {
                            OTString strRemitterAssetID(
                                pRemitterAcct->GetAssetTypeID());
                            OTLog::vOutput(0, "Remitter Acct: %s\nType: %s\n",
                                           strRemitterAcctID.Get(),
                                           strRemitterAssetID.Get());
                        }
                    }

                    // The BALANCE AGREEMENT includes a signed and dated:
                    /*
                      user ID, server ID, account ID, transaction ID.

                      BY THE TIME you are ever inside the procesing for ANY
                      transaction. we know for
                      a fact that NotarizeTransaction has ALREADY checked all
                      the items on the transaction
                      (the ones in its list) to make sure they ALL have the same
                      owner, and signature,
                      and transaction number, and account ID, and server ID.
                      This happens when the items
                      first load via VerifyContractID(), and then in
                      NotarizeTransaction() with a call to
                      VerifyItems(). Therefore I can consider the above
                      variables COVERED for pItem as
                      well as pBalanceItem.

                      Balance Agreement also includes:
                      -- A copy of all the transaction numbers that should still
                      be issued to the Nym,
                      AFTER one is removed from depositing this cheque. (The
                      same number on tranIn and pItem.)
                      NEED TO VERIFY BOTH LISTS ARE THE SAME AFTER REMOVING ONE
                      ON MY SIDE.
                      -- Account balance.
                      (NEED TO VERIFY BALANCE WOULD BE THE SAME AFTER PROCESSING
                      TRANSACTION.
                      -- Inbox and Outbox reports on a single list of sub-items.
                      (NEED TO VERIFY INBOX AND OUTBOX ITEMS MATCH BY
                      RE-CREATING AND THEN COMPARING.)

                      All these are now done in VerifyBalanceStatement().

                    */
                    else if (!(pBalanceItem->VerifyBalanceStatement(
                                 theCheque.GetAmount(), theNym, *pInbox,
                                 *pOutbox, theAccount, tranIn))) {
                        OTLog::vOutput(0, "Notary::%s: ERROR verifying "
                                          "balance statement while depositing "
                                          "cheque. Acct ID:\n%s\n",
                                       __FUNCTION__, strAccountID.Get());
                    }

                    // Debit Source account, Credit Recipient Account, add to
                    // Sender's inbox.
                    //
                    // Also clear the transaction number so this cheque can't be
                    // deposited again.
                    //
                    else {
                        pResponseBalanceItem->SetStatus(
                            OTItem::acknowledgement); // the transaction
                                                      // agreement was
                                                      // successful.

                        // Deduct the amount from the source account, and add it
                        // to the recipient account...
                        if (false ==
                            pSourceAcct->Debit(theCheque.GetAmount())) {
                            OTLog::vError("Notary::%s: Failed debiting "
                                          "source account.\n",
                                          __FUNCTION__);
                        }
                        else if (false ==
                                   theAccount.Credit(theCheque.GetAmount())) {
                            OTLog::vError("Notary::%s: Failed crediting "
                                          "destination account.\n",
                                          __FUNCTION__);

                            if (false ==
                                pSourceAcct->Credit(theCheque.GetAmount()))
                                OTLog::vError("Notary::%s: Failed crediting "
                                              "back source account.\n",
                                              __FUNCTION__);
                        }
                        else if ( // Clear the transaction number. Sender Nym
                            // was responsible for it (and still is,
                            // until
                            // he signs to accept the cheque
                            // reecipt). Alternately, remitter Nym is
                            // responsible for
                            // it, until he signs to accept the
                            // voucher receipt. At this point, the
                            // cheque is USED,
                            // so I'm removing his ability to use
                            // that number twice. It will remain on
                            // his issued
                            // list until he signs for the receipt.
                            //
                            false ==
                            server_->transactor_.removeTransactionNumber(
                                *(bHasRemitter ? pRemitterNym : pSenderNym),
                                theCheque.GetTransactionNum(),
                                true) // bSave=true
                            ) {
                            OTLog::vError("%s: Strange: Failed removing "
                                          "transaction number from sender or "
                                          "remitter, even though "
                                          "it verified just earlier...\n",
                                          __FUNCTION__);
                            if (false ==
                                pSourceAcct->Credit(theCheque.GetAmount()))
                                OTLog::vError("Notary::%s: Failed "
                                              "crediting-back source "
                                              "account.\n",
                                              __FUNCTION__);

                            if (false ==
                                theAccount.Debit(theCheque.GetAmount()))
                                OTLog::vError("Notary::%s: Failed "
                                              "debiting-back destination "
                                              "account.\n",
                                              __FUNCTION__);
                        }
                        else { // need to be able to "roll back" if anything
                                 // inside this block fails.
                            // update: actually does pretty good roll-back as it
                            // is. The debits and credits
                            // don't save unless everything is a success.

                            OTAccount* pAcctWhereReceiptGoes = nullptr;
                            OTLedger* pInboxWhereReceiptGoes = nullptr;
                            if (bHasRemitter) // voucher
                            {
                                pAcctWhereReceiptGoes = pRemitterAcct;
                                pInboxWhereReceiptGoes = pRemitterInbox;
                            }
                            else // normal cheque
                            {
                                pAcctWhereReceiptGoes = pSourceAcct;
                                pInboxWhereReceiptGoes = pSenderInbox;
                            }
                            // Add the chequeReceipt to the inbox of the signer
                            // on the cheque.
                            // (Or add the voucherReceipt to the inbox of the
                            // remitter of the voucher.)
                            //
                            // The original sender (or remitter, whichever is
                            // applicable) can close out the
                            // transaction number associated with the cheque by
                            // signing-off on that chequeReceipt or
                            // voucherReceipt.
                            //
                            if (pAcctWhereReceiptGoes
                                    ->IsInternalServerAcct()) // Must be a
                                                              // voucher
                                                              // remitted by the
                                                              // server.
                                                              // (Unusual, but
                                                              // possible...
                                                              // Typically a
                                                              // voucher is
                                                              // remitted by a
                                                              // normal user,
                                                              // but this can
                                                              // happen in the
                                                              // case of
                                                              // dividends.)
                            {
                                if (!bHasRemitter || !bRemitterIsServer)
                                    OTLog::vError(
                                        "%s: Error: Apparently this is a "
                                        "voucher remitted directly by the "
                                        "server (such as a dividend payment) "
                                        "but one of these values is false: "
                                        "bHasRemitter (%s), "
                                        "bRemitterIsServer(%s)\n",
                                        __FUNCTION__,
                                        bHasRemitter ? "true" : "false",
                                        bRemitterIsServer ? "true" : "false");
                                else {
                                    // If bHasRemitter is true, which it is,
                                    // then we KNOW the server is the sender,
                                    // since
                                    // we explicitly verified that already
                                    // above. But the receipt doesn't go to the
                                    // sender's inbox,
                                    // but rather, to the remitter's inbox.
                                    // However, in this block, clearly
                                    // IsInternalServerAcct is true,
                                    // where the receipt is supposed to go,
                                    // which means the server is also the
                                    // remitter! And we verified
                                    // that also in the above 'if' statement, so
                                    // we explicitly know that
                                    // bRemitterIsServer. And in that
                                    // case, we can't just drop a receipt into
                                    // his inbox, since the server has no inbox,
                                    // and that means
                                    // he will never be able to close out the
                                    // transaction number by accepting the
                                    // receipt, like a normal
                                    // user would be able to do.
                                    //
                                    // Therefore, we explicitly remove the
                                    // issued number here from the remitter
                                    // (whom we know to actually
                                    // be the server.) He has no inbox, and he
                                    // won't get any receipt -- but the number
                                    // will be closed out
                                    // properly. Before this fix, the server was
                                    // probably keeping every voucher number
                                    // open for eternity.
                                    //
                                    if (!server_->transactor_
                                             .removeIssuedNumber(
                                                 *pRemitterNym,
                                                 theCheque.GetTransactionNum(),
                                                 true)) // bSave=true
                                        OTLog::vError(
                                            "%s: Strange: Failed removing "
                                            "issued number from remitter (the "
                                            "server nym, in this case), "
                                            "even though the number verified "
                                            "just earlier...\n",
                                            __FUNCTION__);
                                }
                            }
                            else // For normal cheques, and for normal
                                   // vouchers (where the remitter is a normal
                                   // user.)
                            {
                                // Generate new transaction number (for putting
                                // the check receipt in the sender's inbox.)
                                // todo check this generation for failure (can
                                // it fail?)
                                int64_t lNewTransactionNumber = 0;

                                server_->transactor_.issueNextTransactionNumber(
                                    server_->m_nymServer, lNewTransactionNumber,
                                    false); // bStoreTheNumber = false

                                OTTransaction* pInboxTransaction =
                                    OTTransaction::GenerateTransaction(
                                        *pInboxWhereReceiptGoes,
                                        theCheque.HasRemitter()
                                            ? OTTransaction::voucherReceipt
                                            : OTTransaction::chequeReceipt,
                                        lNewTransactionNumber);

                                // The depositCheque request OTItem is saved as
                                // a "in reference to" field,
                                // on the inbox chequeReceipt or voucherReceipt
                                // transaction.
                                // todo put these two together in a method.
                                pInboxTransaction->SetReferenceString(
                                    strInReferenceTo);
                                pInboxTransaction->SetReferenceToNum(
                                    pItem->GetTransactionNum());
                                pInboxTransaction->SetNumberOfOrigin(
                                    theCheque.GetTransactionNum());

                                if (bRemitterCancelling)
                                    pInboxTransaction->SetAsCancelled();

                                // Now we have created a new transaction from
                                // the server to the sender's inbox (or
                                // remitter's inbox.)
                                // Let's sign and save it...
                                pInboxTransaction->SignContract(
                                    server_->m_nymServer);
                                pInboxTransaction->SaveContract();

                                // Here the transaction we just created is
                                // actually added to the source acct's or
                                // remitter's inbox.
                                pInboxWhereReceiptGoes->AddTransaction(
                                    *pInboxTransaction);

                                // Release any signatures that were there before
                                // (They won't
                                // verify anymore anyway, since the content has
                                // changed.)
                                //
                                pInboxWhereReceiptGoes->ReleaseSignatures();
                                pInboxWhereReceiptGoes->SignContract(
                                    server_->m_nymServer);
                                pInboxWhereReceiptGoes->SaveContract();

                                pAcctWhereReceiptGoes->SaveInbox(
                                    *pInboxWhereReceiptGoes);

                                // if there's NOT a remitter, then the source
                                // account is ALREADY saved below this block.
                                // (Otherwise we need to save his acct here.)
                                //
                                if (bHasRemitter &&
                                    !bRemitterIsServer) // If remitter is also
                                                        // server, then no need
                                                        // to save here, since
                                                        // source acct is
                                                        // ALREADY saved below.
                                {
                                    // If there IS a remitter who is NOT the
                                    // server, then we save his
                                    // account here, so it will contain the
                                    // updated inbox hash (since
                                    // we just added a receipt to its inbox.)
                                    //
                                    pAcctWhereReceiptGoes->ReleaseSignatures();
                                    pAcctWhereReceiptGoes->SignContract(
                                        server_->m_nymServer);
                                    pAcctWhereReceiptGoes->SaveContract();
                                    pAcctWhereReceiptGoes->SaveAccount();
                                }

                                // Any inbox/nymbox/outbox ledger will only
                                // itself contain
                                // abbreviated versions of the receipts,
                                // including their hashes.
                                //
                                // The rest is stored separately, in the box
                                // receipt, which is created
                                // whenever a receipt is added to a box, and
                                // deleted after a receipt
                                // is removed from a box.
                                //
                                pInboxTransaction->SaveBoxReceipt(
                                    *pInboxWhereReceiptGoes);
                            }
                            // AT THIS POINT, the source account is debited, the
                            // recipient account is credited,
                            // and the sender's (or remitter's) inbox has had
                            // the chequeReceipt or voucherReceipt added to it.
                            // (He must perform a balance agreement in order to
                            // get it out of his inbox.)
                            //
                            // THERE IS NOTHING LEFT TO DO BUT SAVE THE FILES!!

                            pSourceAcct->ReleaseSignatures();
                            theAccount.ReleaseSignatures();

                            pSourceAcct->SignContract(server_->m_nymServer);
                            theAccount.SignContract(server_->m_nymServer);

                            pSourceAcct->SaveContract();
                            theAccount.SaveContract();

                            pSourceAcct->SaveAccount();
                            theAccount.SaveAccount();

                            // Now we can set the response item as an
                            // acknowledgement instead of the default
                            // (rejection)
                            // otherwise, if we never entered this block, then
                            // it would still be set to rejection, and the
                            // new item would never have been added to the
                            // inbox, and the inbox file, along with the
                            // account files, would never have had their
                            // signatures released, or been re-signed or
                            // re-saved back to file.  The debit failed, so all
                            // of those other actions would fail also.
                            // BUT... if the message comes back with
                            // acknowledgement--then all of these actions must
                            // have
                            // happened, and here is the server's signature to
                            // prove it.
                            // Otherwise you get no items and no signature. Just
                            // a rejection item in the response transaction.
                            //
                            pResponseItem->SetStatus(OTItem::acknowledgement);

                            bOutSuccess =
                                true; // The cheque deposit was successful.

                            if (bRemitterCancelling) {
                                tranOut.SetAsCancelled();
                                OTLog::vOutput(1, "Notary::%s: SUCCESS "
                                                  "crediting remitter account "
                                                  "from voucher "
                                                  "cancellation.\n",
                                               __FUNCTION__);
                            }
                            else
                                OTLog::vOutput(1, "Notary::%s: SUCCESS "
                                                  "crediting account from "
                                                  "cheque deposit.\n",
                                               __FUNCTION__);

                            // TODO: Our code that actually saves the new
                            // balance statement receipt should go here
                            // (that is, only after ultimate success.) Otherwise
                            // we still want to store the old receipt.
                            // For now I'm verifying it, but not storing it.
                            // This means the security for it works, but
                            // in a dispute, I can't prove it / cover my ass.
                            // So very soon a receipt WILL be saved here
                            // that is, a copy of the user's signed
                            // BalanceAgreement.
                            // Note: if I'm saving the entire outgoing
                            // transaction reply, or message reply, (versus just
                            // the reply to a certain item) then I think I have
                            // the balance agreement already? Double check.
                        }

                    } // "else"
                }     // "else"
            }         // successfully loaded cheque from string
        }             // account ID DOES match item's account ID
    }                 // deposit cheque

    // BELOW -- DEPOSIT CASH

    // For now, there should only be one of these deposit items inside the
    // transaction.
    // So we treat it that way... I either get it successfully or not.
    //
    // Deposit (the transaction) now supports deposit (the item) and
    // depositCheque (the item)
    else if (pItem->GetType() == OTItem::deposit) {
        // The response item, as well as the inbox and outbox items, will
        // contain a copy
        // of the request item. So I save it into a string here so they can all
        // grab a copy of it
        // into their "in reference to" fields.
        pItem->SaveContractRaw(strInReferenceTo);
        pBalanceItem->SaveContractRaw(strBalanceItem);

        // Server response item being added to server response transaction
        // (tranOut)
        // They're getting SOME sort of response item.

        pResponseItem->SetReferenceString(strInReferenceTo); // the response
                                                             // item carries a
                                                             // copy of what
                                                             // it's responding
                                                             // to.
        pResponseItem->SetReferenceToNum(
            pItem->GetTransactionNum()); // This response item is IN RESPONSE to
                                         // pItem and its Owner Transaction.

        pResponseBalanceItem->SetReferenceString(
            strBalanceItem); // the response item carries a copy of what it's
                             // responding to.
        pResponseBalanceItem->SetReferenceToNum(
            pItem->GetTransactionNum()); // This response item is IN RESPONSE to
                                         // pItem and its Owner Transaction.

        // If the ID on the "from" account that was passed in,
        // does not match the "Acct From" ID on this transaction item
        if (ACCOUNT_ID != pItem->GetPurportedAccountID()) {
            OTLog::vOutput(0, "Notary::NotarizeDeposit: Error: 'From' "
                              "account ID on the transaction does not match "
                              "'from' account ID on the deposit item.\n");
        }
        else {
            std::unique_ptr<OTLedger> pInbox(
                theAccount.LoadInbox(server_->m_nymServer));
            std::unique_ptr<OTLedger> pOutbox(
                theAccount.LoadOutbox(server_->m_nymServer));

            if (nullptr == pInbox) {
                OTLog::Error("Notary::NotarizeDeposit: Error loading or "
                             "verifying inbox.\n");
                OT_FAIL;
            }
            else if (nullptr == pOutbox) {
                OTLog::Error("Notary::NotarizeDeposit: Error loading or "
                             "verifying outbox.\n");
                OT_FAIL;
            }
            OTString strPurse;
            pItem->GetAttachment(strPurse);

            Purse thePurse(SERVER_ID, ASSET_TYPE_ID);

            bool bLoadContractFromString =
                thePurse.LoadContractFromString(strPurse);

            if (!bLoadContractFromString) {
                OTLog::vError("Notary::NotarizeDeposit: ERROR loading purse "
                              "from string:\n%s\n",
                              strPurse.Get());
            }
            else if (!(pBalanceItem->VerifyBalanceStatement(
                           thePurse.GetTotalValue(), theNym, *pInbox, *pOutbox,
                           theAccount, tranIn))) {
                OTLog::vOutput(0, "Notary::NotarizeDeposit: ERROR verifying "
                                  "balance statement while depositing cash. "
                                  "Acct ID:\n%s\n",
                               strAccountID.Get());
            }

            // TODO: double-check all verification stuff all around on the purse
            // and token, transaction, mint, etc.
            else // the purse loaded successfully from the string
            {
                pResponseBalanceItem->SetStatus(
                    OTItem::acknowledgement); // the transaction agreement was
                                              // successful.

                bool bSuccess = false;

                // Pull the token(s) out of the purse that was received from the
                // client.
                while (true) {
                    std::unique_ptr<Token> pToken(
                        thePurse.Pop(server_->m_nymServer));
                    if (!pToken) {
                        break;
                    }

                    pMint = server_->transactor_.getMint(ASSET_TYPE_ID,
                                                         pToken->GetSeries());

                    if (nullptr == pMint) {
                        OTLog::Error("Notary::NotarizeDeposit: Unable to get "
                                     "or load Mint.\n");
                        break;
                    }
                    else if ((pMintCashReserveAcct =
                                    pMint->GetCashReserveAccount()) !=
                               nullptr) {
                        OTString strSpendableToken;
                        bool bToken = pToken->GetSpendableString(
                            server_->m_nymServer, strSpendableToken);

                        if (!bToken) // if failure getting the spendable token
                                     // data from the token object
                        {
                            bSuccess = false;
                            OTLog::vOutput(0, "Notary::NotarizeDeposit: "
                                              "ERROR verifying token: Failure "
                                              "retrieving token data. \n");
                            break;
                        }
                        else if (!(pToken->GetAssetID() ==
                                     ASSET_TYPE_ID)) // or if failure verifying
                                                     // asset type
                        {
                            bSuccess = false;
                            OTLog::vOutput(0, "Notary::NotarizeDeposit: "
                                              "ERROR verifying token: Wrong "
                                              "asset type. \n");
                            break;
                        }
                        else if (!(pToken->GetServerID() ==
                                     SERVER_ID)) // or if failure verifying
                                                 // server ID
                        {
                            bSuccess = false;
                            OTLog::vOutput(0, "Notary::NotarizeDeposit: "
                                              "ERROR verifying token: Wrong "
                                              "server ID. \n");
                            break;
                        }
                        // This call to VerifyToken verifies the token's Series
                        // and From/To dates against the
                        // mint's, and also verifies that the CURRENT date is
                        // inside that valid date range.
                        //
                        // It also verifies the Lucre coin data itself against
                        // the key for that series and
                        // denomination. (The signed and unblinded Lucre coin is
                        // finally verified in Lucre
                        // using the appropriate Mint private key.)
                        //
                        else if (!(pMint->VerifyToken(
                                     server_->m_nymServer, strSpendableToken,
                                     pToken->GetDenomination()))) {
                            bSuccess = false;
                            OTLog::vOutput(0, "Notary::NotarizeDeposit: "
                                              "ERROR verifying token: Token "
                                              "verification failed. \n");
                            break;
                        }
                        // Lookup the token in the SPENT TOKEN DATABASE, and
                        // make sure
                        // that it hasn't already been spent...
                        else if (pToken->IsTokenAlreadySpent(
                                     strSpendableToken)) {
                            // TODO!!!! Need to store the spent token database
                            // in multiple places, on multiple media!
                            //          Furthermore need to CHECK those multiple
                            // places inside IsTokenAlreadySpent.
                            //          In fact, that should all be configurable
                            // in the server config file!
                            //          Related: make sure IsTokenAlreadySpent
                            // differentiates between ACTUALLY not finding
                            //          a token as spent (successfully), versus
                            // some error state with the storage.
                            bSuccess = false;
                            OTLog::vOutput(0, "Notary::NotarizeDeposit: "
                                              "ERROR verifying token: Token "
                                              "was already spent. \n");
                            break;
                        }
                        else {
                            OTLog::Output(3, "Notary::NotarizeDeposit: "
                                             "SUCCESS verifying token...    "
                                             "\n");

                            // need to be able to "roll back" if anything inside
                            // this block fails.
                            // so unless bSuccess is true, I don't save the
                            // account below.
                            //

                            // two defense mechanisms here:  mint cash reserve
                            // acct, and spent token database
                            //
                            if (false ==
                                pMintCashReserveAcct->Debit(
                                    pToken->GetDenomination())) {
                                OTLog::Error("Notary::NotarizeDeposit: Error "
                                             "debiting the mint cash reserve "
                                             "account. "
                                             "SHOULD NEVER HAPPEN...\n");
                                bSuccess = false;
                                break;
                            }
                            // CREDIT the amount to the account...
                            else if (false ==
                                     theAccount.Credit(
                                         pToken->GetDenomination())) {
                                OTLog::Error("Notary::NotarizeDeposit: Error "
                                             "crediting the user's asset "
                                             "account...\n");

                                if (false ==
                                    pMintCashReserveAcct->Credit(
                                        pToken->GetDenomination()))
                                    OTLog::Error("Notary::NotarizeDeposit: "
                                                 "Failure crediting-back "
                                                 "mint's cash reserve account "
                                                 "while depositing cash.\n");
                                bSuccess = false;
                                break;
                            }
                            // Spent token database. This is where the call is
                            // made to add
                            // the token to the spent token database.
                            else if (false ==
                                     pToken->RecordTokenAsSpent(
                                         strSpendableToken)) {
                                OTLog::Error("Notary::NotarizeDeposit: "
                                             "Failed recording token as "
                                             "spent...\n");

                                if (false ==
                                    pMintCashReserveAcct->Credit(
                                        pToken->GetDenomination()))
                                    OTLog::Error("Notary::NotarizeDeposit: "
                                                 "Failure crediting-back "
                                                 "mint's cash reserve account "
                                                 "while depositing cash.\n");

                                if (false ==
                                    theAccount.Debit(pToken->GetDenomination()))
                                    OTLog::Error("Notary::NotarizeDeposit: "
                                                 "Failure debiting-back user's "
                                                 "asset account while "
                                                 "depositing cash.\n");

                                bSuccess = false;
                                break;
                            }
                            else // SUCCESS!!! (this iteration)
                            {
                                OTLog::vOutput(2, "Notary::NotarizeDeposit: "
                                                  "SUCCESS crediting account "
                                                  "with cash token...\n");
                                bSuccess = true;

                                // No break here -- we allow the loop to carry
                                // on through.
                            }
                        }
                    }
                    else {
                        OTLog::Error("Notary::NotarizeDeposit: Unable to get "
                                     "cash reserve account for Mint.\n");
                        bSuccess = false;
                        break;
                    }
                } // while success popping token from purse

                if (bSuccess) {
                    // Release any signatures that were there before (They won't
                    // verify anymore anyway, since the content has changed.)
                    theAccount.ReleaseSignatures();

                    // Sign
                    theAccount.SignContract(server_->m_nymServer);

                    // Save
                    theAccount.SaveContract();

                    // Save to file
                    theAccount.SaveAccount();

                    // We also need to save the Mint's cash reserve.
                    // (Any cash issued by the Mint is automatically backed by
                    // this reserve
                    // account. If cash is deposited, it comes back out of this
                    // account. If the
                    // cash expires, then after the expiry period, if it remains
                    // in the account,
                    // it is now the property of the transaction server.)
                    pMintCashReserveAcct->ReleaseSignatures();
                    pMintCashReserveAcct->SignContract(server_->m_nymServer);
                    pMintCashReserveAcct->SaveContract();
                    pMintCashReserveAcct->SaveAccount();

                    pResponseItem->SetStatus(OTItem::acknowledgement);

                    bOutSuccess = true; // The cash deposit was successful.

                    OTLog::Output(1, "Notary::NotarizeDeposit: .....SUCCESS "
                                     "-- crediting account from cash "
                                     "deposit.\n");

                    // TODO:  Right here, again, I need to save the receipt from
                    // the new balance agreement, since we have
                    // "ultimate success".  Also need to save the Nym, since he
                    // had a transaction number removed in
                    // the above call to VerifyBalanceAgreement. If we failed
                    // here, then we wouldn't WANT to save, since
                    // that number should stay on him! Same reason we don't save
                    // the accounts if anything goes wrong.
                }
            } // the purse loaded successfully from the string
        }     // the account ID matches correctly to the acct ID on the item.
    }
    else {
        OTString strTemp(tranIn);
        OTLog::vOutput(0, "%s: Expected OTItem::deposit or "
                          "OTItem::depositCheque on trans# %lld: \n\n%s\n\n",
                       __FUNCTION__, tranIn.GetTransactionNum(),
                       strTemp.Exists()
                           ? strTemp.Get()
                           : " (ERROR CREATING STRING FROM TRANSACTION.) ");
    }

    // sign the response item before sending it back (it's already been added to
    // the transaction above)
    // Now, whether it was rejection or acknowledgement, it is set properly and
    // it is signed, and it
    // is owned by the transaction, who will take it from here.
    pResponseItem->SignContract(server_->m_nymServer);
    pResponseItem->SaveContract(); // the signing was of no effect because I
                                   // forgot to save.

    pResponseBalanceItem->SignContract(server_->m_nymServer);
    pResponseBalanceItem->SaveContract();
}

// DONE:  Need to make sure both parties have included TWO!!! transaction
// numbers, so both
// have the option to cancel later!  (And so the server can expire it later, and
// cover its own ass.)
//
// Note: still need to do something with those numbers upon closing. (cron
// expiration, and cancelCronItem.)
//

// DONE: The current version verifies that it's signed by both parties.
//  Fix it so that it loads the merchant's copy to verify recipient signature.

/// 1) The Merchant generates the payment plan, adds transaction numbers, and
/// signs. (All done via ProposePaymentPlan)
/// 2) Then the Customer uses ConfirmPaymentPlan to add his own numbers and
/// sign.
/// 3) Then the Customer must activate the payment plan. (Using a transaction
/// with the same number as the plan.)
///
///
void Notary::NotarizePaymentPlan(OTPseudonym& theNym,
                                 OTAccount& theDepositorAccount,
                                 OTTransaction& tranIn, OTTransaction& tranOut,
                                 bool& bOutSuccess)
{
    // The outgoing transaction is an "atPaymentPlan", that is, "a reply to the
    // paymentPlan request"
    tranOut.SetType(OTTransaction::atPaymentPlan);

    OTItem* pItem = nullptr;
    OTItem* pBalanceItem = nullptr;
    OTItem* pResponseItem = nullptr;
    OTItem* pResponseBalanceItem = nullptr;

    // The incoming transaction may be sent to inboxes and outboxes, and it
    // will definitely be bundled in our reply to the user as well. Therefore,
    // let's grab it as a string.
    OTString strInReferenceTo;
    OTString strBalanceItem;

    // Grab the actual server ID from this object, and use it as the server ID
    // here.
    const OTIdentifier SERVER_ID(server_->m_strServerID),
        DEPOSITOR_USER_ID(theNym), SERVER_USER_ID(server_->m_nymServer),
        DEPOSITOR_ACCT_ID(theDepositorAccount), USER_ID(theNym);

    const OTString strUserID(USER_ID);
    pItem = tranIn.GetItem(OTItem::paymentPlan);
    pBalanceItem = tranIn.GetItem(OTItem::transactionStatement);
    pResponseItem =
        OTItem::CreateItemFromTransaction(tranOut, OTItem::atPaymentPlan);
    pResponseItem->SetStatus(OTItem::rejection); // the default.
    tranOut.AddItem(*pResponseItem); // the Transaction's destructor will
                                     // cleanup the item. It "owns" it now.
    pResponseBalanceItem = OTItem::CreateItemFromTransaction(
        tranOut, OTItem::atTransactionStatement);
    pResponseBalanceItem->SetStatus(OTItem::rejection); // the default.
    tranOut.AddItem(*pResponseBalanceItem); // the Transaction's destructor will
                                            // cleanup the item. It "owns" it
                                            // now.
    if ((nullptr != pItem) &&
        (false == NYM_IS_ALLOWED(strUserID.Get(),
                                 ServerSettings::__transact_payment_plan))) {
        OTLog::vOutput(0, "%s: User %s cannot do this transaction (All payment "
                          "plans are disallowed in server.cfg)\n",
                       __FUNCTION__, strUserID.Get());
    }
    // For now, there should only be one of these paymentPlan items inside the
    // transaction.
    // So we treat it that way... I either get it successfully or not.
    else if ((nullptr == pItem) || (nullptr == pBalanceItem)) {
        OTLog::vError("%s: Error, expected OTItem::paymentPlan and "
                      "OTItem::transactionStatement.\n",
                      __FUNCTION__);
    }
    else {
        if (DEPOSITOR_ACCT_ID != pItem->GetPurportedAccountID()) {
            OTLog::vOutput(0, "%s: Error: Source account ID on the transaction "
                              "does not match sender's account ID on the "
                              "transaction item.\n",
                           __FUNCTION__);
        }
        else if (false ==
                   pBalanceItem->VerifyTransactionStatement(
                       theNym, tranIn)) // bIsRealTransaction=true
        {
            OTLog::vOutput(0, "%s: Failed verifying transaction statement.\n",
                           __FUNCTION__);
        }
        else {
            pResponseBalanceItem->SetStatus(
                OTItem::acknowledgement); // the transaction agreement was
                                          // successful.

            // The response item will contain a copy of the request item. So I
            // save it into a string
            // here so it can be saved into the "in reference to" field.
            pItem->SaveContractRaw(strInReferenceTo);
            pBalanceItem->SaveContractRaw(strBalanceItem);

            // Server response item being added to server response transaction
            // (tranOut)
            // They're getting SOME sort of response item.

            pResponseItem->SetReferenceString(
                strInReferenceTo); // the response item carries a copy of what
                                   // it's responding to.
            pResponseItem->SetReferenceToNum(
                pItem->GetTransactionNum()); // This response item is IN
                                             // RESPONSE to pItem and its Owner
                                             // Transaction.

            pResponseBalanceItem->SetReferenceString(
                strBalanceItem); // the response item carries a copy of what
                                 // it's responding to.
            pResponseBalanceItem->SetReferenceToNum(
                pItem->GetTransactionNum()); // This response item is IN
                                             // RESPONSE to pItem and its Owner
                                             // Transaction.

            // Also load up the Payment Plan from inside the transaction item.
            OTString strPaymentPlan;
            pItem->GetAttachment(strPaymentPlan);
            OTPaymentPlan* pPlan = new OTPaymentPlan();
            OT_ASSERT(nullptr != pPlan);

            // If we failed to load the plan...
            if ((false == pPlan->LoadContractFromString(strPaymentPlan))) {
                OTLog::vError(
                    "%s: ERROR loading payment plan from string:\n%s\n",
                    __FUNCTION__, strPaymentPlan.Get());
            }
            else if (pPlan->GetServerID() != SERVER_ID) {
                OTLog::vOutput(0, "%s: ERROR bad server ID on payment plan.\n",
                               __FUNCTION__);
            }
            else if (pPlan->GetAssetID() !=
                       theDepositorAccount.GetAssetTypeID()) {
                const OTString strAssetID1(pPlan->GetAssetID()),
                    strAssetID2(theDepositorAccount.GetAssetTypeID());
                OTLog::vOutput(0, "%s: ERROR wrong Asset Type ID (%s) on "
                                  "payment plan. Expected: %s\n",
                               __FUNCTION__, strAssetID1.Get(),
                               strAssetID2.Get());
            }
            else {
                // CANCELLING? OR ACTIVATING?
                // If he is cancelling the payment plan (from his outpayments
                // box, before it's even had a chance
                // to be activated by the sender) then the recipient (merchant)
                // will be the depositor. Otherwise,
                // if he is activating the payment plan (from his payments
                // inbox) then the sender (customer) will
                // be the depositor.

                OTIdentifier theCancelerNymID;
                const bool bCancelling =
                    (pPlan->IsCanceled() &&
                     pPlan->GetCancelerID(theCancelerNymID));
                const int64_t lExpectedNum =
                    bCancelling ? 0 : pItem->GetTransactionNum();
                const int64_t lFoundNum = pPlan->GetTransactionNum();
                const OTIdentifier& FOUND_USER_ID =
                    bCancelling ? pPlan->GetRecipientUserID()
                                : pPlan->GetSenderUserID();
                const OTIdentifier& FOUND_ACCT_ID =
                    bCancelling ? pPlan->GetRecipientAcctID()
                                : pPlan->GetSenderAcctID();
                const int64_t lFoundOpeningNum =
                    pPlan->GetOpeningNumber(FOUND_USER_ID);
                const int64_t lFoundClosingNum =
                    pPlan->GetClosingNumber(FOUND_ACCT_ID);
                if (lFoundNum != lExpectedNum) {
                    OTLog::vOutput(0, "%s: ERROR bad main transaction number "
                                      "while %s payment plan (%lld). Expected "
                                      "based on transaction: %lld\n",
                                   __FUNCTION__,
                                   bCancelling ? "cancelling" : "activating",
                                   lFoundNum, lExpectedNum);
                }
                if (lFoundOpeningNum != pItem->GetTransactionNum()) {
                    OTLog::vOutput(
                        0, "%s: ERROR bad transaction number while %s payment "
                           "plan (%lld). Expected based on transaction: %lld\n",
                        __FUNCTION__, bCancelling ? "cancelling" : "activating",
                        lFoundOpeningNum, pItem->GetTransactionNum());
                }
                else if (FOUND_USER_ID != DEPOSITOR_USER_ID) {
                    const OTString strIDExpected(FOUND_USER_ID),
                        strIDDepositor(DEPOSITOR_USER_ID);
                    OTLog::vOutput(
                        0, "%s: ERROR wrong user ID while %s payment plan. "
                           "Depositor: %s  Found on plan: %s\n",
                        __FUNCTION__, bCancelling ? "cancelling" : "activating",
                        strIDDepositor.Get(), strIDExpected.Get());
                }
                else if (bCancelling &&
                           (DEPOSITOR_USER_ID != theCancelerNymID)) {
                    const OTString strIDExpected(DEPOSITOR_USER_ID),
                        strIDDepositor(theCancelerNymID);
                    OTLog::vOutput(0, "%s: ERROR wrong canceler Nym ID while "
                                      "canceling payment plan. Depositor: %s  "
                                      "Canceler: %s\n",
                                   __FUNCTION__, strIDExpected.Get(),
                                   strIDDepositor.Get());
                }
                else if (FOUND_ACCT_ID != DEPOSITOR_ACCT_ID) {
                    const OTString strAcctID1(FOUND_ACCT_ID),
                        strAcctID2(DEPOSITOR_ACCT_ID);
                    OTLog::vOutput(0, "%s: ERROR wrong Acct ID (%s) while %s "
                                      "payment plan. Expected: %s\n",
                                   __FUNCTION__, strAcctID1.Get(),
                                   bCancelling ? "cancelling" : "activating",
                                   strAcctID2.Get());
                }
                // If we're activating the plan (versus cancelling) then the
                // transaction number opens
                // the payment plan, but there must also be a closing number for
                // closing it.
                else if (!bCancelling && // If activating and:
                         ((pPlan->GetCountClosingNumbers() <
                           1) || // ...if there aren't enough closing numbers...
                          !server_->transactor_.verifyTransactionNumber(
                              theNym, lFoundClosingNum))) // ...or the official
                                                          // closing # isn't
                                                          // available for use
                                                          // on theNym.
                { // We don't check opening number here, since
                    // NotarizeTransaction already did.
                    OTLog::vOutput(0, "%s: ERROR: the Closing number %lld "
                                      "wasn't available for use while "
                                      "activating a payment plan.\n",
                                   __FUNCTION__, lFoundClosingNum);
                }
                else if (bCancelling && // If cancelling and:
                           ((pPlan->GetRecipientCountClosingNumbers() < 2) ||
                            !server_->transactor_.verifyTransactionNumber(
                                theNym, lFoundClosingNum))) {
                    OTLog::vOutput(0, "%s: ERROR: the Closing number wasn't "
                                      "available for use while cancelling a "
                                      "payment plan.\n",
                                   __FUNCTION__);
                }
                else // The plan is good (so far.)
                {
                    // The RECIPIENT_ACCT_ID is the ID on the "To" Account.
                    // (When doing a transfer, normally 2nd acct is the Payee.)
                    const OTIdentifier RECIPIENT_ACCT_ID(
                        pPlan->GetRecipientAcctID()),
                        RECIPIENT_USER_ID(pPlan->GetRecipientUserID());

                    bool bRecipientNymIsServerNym =
                        ((RECIPIENT_USER_ID == SERVER_USER_ID) ? true : false);
                    bool bUsersAreSameNym =
                        ((DEPOSITOR_USER_ID == RECIPIENT_USER_ID) ? true
                                                                  : false);

                    OTPseudonym theRecipientNym; // We'll probably use this, but
                                                 // maybe not. So I use a
                                                 // pointer that will maybe
                                                 // point here.
                    OTPseudonym* pRecipientNym = nullptr; // Here's the pointer.
                                                          // (Logic explained
                                                          // directly below.)
                    // Set pRecipientNym to point to the right one so we can use
                    // it below. (Do NOT use theRecipientNym,
                    // since it won't always point to that one.)

                    bool bFoundRecipientNym = false;

                    // Find out if Recipient Nym is also the Server Nym...
                    if (bRecipientNymIsServerNym) {
                        // If the Recipient Nym is the server, then just point
                        // to that.
                        pRecipientNym = &server_->m_nymServer;
                        bFoundRecipientNym = true;

                        // (No need to verify Nym here since already done in
                        // this case.)
                    }
                    else if (bUsersAreSameNym) // Else if the participants are
                                                 // the same Nym, point to the
                                                 // one we already loaded.
                    {
                        pRecipientNym = &theNym;
                        bFoundRecipientNym = true;

                        // (No need to verify Nym here since already done in
                        // this case, before we even got here.)
                    }
                    else // Otherwise load the Recipient Nym from Disk and
                           // point to that.
                    {
                        theRecipientNym.SetIdentifier(RECIPIENT_USER_ID);

                        bool bLoadedNym =
                            theRecipientNym.LoadPublicKey(); // Old style
                                                             // (deprecated.)
                                                             // NOTE:
                                                             // LoadCredentials
                                                             // is already
                                                             // called inside
                                                             // LoadPublicKey,
                                                             // at the top, but
                                                             // eventually we'll
                                                             // be calling it
                                                             // here directly,
                                                             // once
                                                             // LoadPublicKey is
                                                             // removed.

                        if (!bLoadedNym) {
                            OTString strNymID(RECIPIENT_USER_ID);
                            OTLog::vError("%s: Failure loading Recipient Nym "
                                          "public key: %s\n",
                                          __FUNCTION__, strNymID.Get());
                            bFoundRecipientNym = false;
                        }
                        else if (!theRecipientNym.VerifyPseudonym() ||
                                   !theRecipientNym.LoadSignedNymfile(
                                       server_->m_nymServer)) {
                            OTString strNymID(RECIPIENT_USER_ID);
                            OTLog::vError("%s: Failure loading or verifying "
                                          "Recipient Nym public key: %s\n",
                                          __FUNCTION__, strNymID.Get());
                            bFoundRecipientNym = false;
                        }
                        else {
                            pRecipientNym = &theRecipientNym; //  <=====
                            bFoundRecipientNym = true;
                        }
                    }
                    // Below this point, ALWAYS use pRecipientNym, NOT
                    // theRecipientNym.
                    // pRecipientNym is always guaranteed below here to point to
                    // the right one.
                    if (!bFoundRecipientNym || (nullptr == pRecipientNym)) {
                        // (No need to log here; already logged right above.)
                        // OTLog::vOutput("Unable to load or verify Recipient
                        // Nym.()", __FUNCTION__);
                    }

                    // Below this point, we know for sure that the Recipient Nym
                    // is loaded and verified, and we know
                    // that if the Server or Sender is actually the Recipient,
                    // that the pRecipientNym pointer will
                    // always point to the right one, and no files can be
                    // overwritten. *phew*

                    // You CAN have both accounts owned by the same Nym, but you
                    // CANNOT have them both actually be the SAME ACCT.
                    else if (!bCancelling && (DEPOSITOR_ACCT_ID ==
                                              RECIPIENT_ACCT_ID)) // ACTIVATING
                    {
                        OTLog::vOutput(
                            0, "%s: Error: Source account ID matches Recipient "
                               "account ID "
                               "on attempted Payment Plan notarization.\n",
                            __FUNCTION__);
                    }
                    // Unless you are cancelling...
                    else if (bCancelling && (DEPOSITOR_ACCT_ID !=
                                             RECIPIENT_ACCT_ID)) // CANCELLING
                    {
                        OTLog::vOutput(
                            0, "%s: Error: Source account ID doesn't match "
                               "Recipient account ID "
                               "on attempted Payment Plan cancellation.\n",
                            __FUNCTION__);
                    }
                    else if (!bCancelling &&
                               !pPlan->VerifyAgreement(*pRecipientNym,
                                                       theNym)) // ACTIVATING
                    {
                        OTLog::vOutput(
                            0, "%s: ERROR verifying Sender and Recipient on "
                               "Payment Plan "
                               "(against merchant and customer copies.)\n",
                            __FUNCTION__);
                    }
                    // This is now done above, in VerifyAgreement().
                    // We only have it here now in cases of cancellation (where
                    // VerifyAgreement isn't called.)
                    //                                                                        //
                    // CANCELING
                    else if (bCancelling &&
                             !pPlan->VerifySignature(*pRecipientNym)) {
                        OTLog::Output(0, "ERROR verifying Recipient's "
                                         "signature on Payment Plan.\n");
                    }
                    else {
                        // Verify that BOTH of the Recipient's transaction
                        // numbers
                        // (opening and closing) are available for use.
                        //
                        // These three blocks are only checked if we are
                        // activating, not cancelling.
                        // Why? Because if we're canceling, then we ALREADY
                        // checked these things above.
                        // But if we're activating, that means we checked the
                        // sender above only, and
                        // thus we still need to check the recipient.
                        //
                        if (!bCancelling &&
                            pPlan->GetRecipientCountClosingNumbers() < 2) {
                            OTLog::vOutput(0, "%s: ERROR verifying Recipient's "
                                              "Opening and Closing number on a "
                                              "Payment Plan "
                                              "(he should have two numbers, "
                                              "but he doesn't.)\n",
                                           __FUNCTION__);
                        }
                        else if (!bCancelling &&
                                   !server_->transactor_
                                        .verifyTransactionNumber(
                                            *pRecipientNym,
                                            pPlan->GetRecipientOpeningNum())) {
                            OTLog::vOutput(0, "%s: ERROR verifying Recipient's "
                                              "opening transaction number on a "
                                              "payment plan.\n",
                                           __FUNCTION__);
                        }
                        else if (!bCancelling &&
                                   !server_->transactor_
                                        .verifyTransactionNumber(
                                            *pRecipientNym,
                                            pPlan->GetRecipientClosingNum())) {
                            OTLog::vOutput(0, "%s: ERROR verifying Recipient's "
                                              "Closing transaction number on a "
                                              "Payment Plan.\n",
                                           __FUNCTION__);
                        }
                        else {
                            // Load up the recipient ACCOUNT and validate it.
                            //
                            OTAccount* pRecipientAcct = nullptr;
                            std::unique_ptr<OTAccount> theRecipientAcctGuardian;
                            if (!bCancelling) // ACTIVATING
                            {
                                pRecipientAcct = OTAccount::LoadExistingAccount(
                                    RECIPIENT_ACCT_ID, SERVER_ID);
                                theRecipientAcctGuardian.reset(pRecipientAcct);
                            }
                            else // CANCELLING
                            {
                                pRecipientAcct = &theDepositorAccount;
                            }
                            //
                            if (nullptr == pRecipientAcct) {
                                OTLog::vOutput(
                                    0, "%s: ERROR loading Recipient account.\n",
                                    __FUNCTION__);
                            }
                            else if (!pRecipientAcct->VerifyOwner(
                                           *pRecipientNym)) {
                                OTLog::vOutput(0, "%s: ERROR verifying "
                                                  "ownership of the recipient "
                                                  "account.\n",
                                               __FUNCTION__);
                            }
                            else if (pRecipientAcct->IsInternalServerAcct()) {
                                OTLog::vOutput(
                                    0,
                                    "%s: Failed: recipient account is an "
                                    "internal "
                                    "server account (currently prohibited.)\n",
                                    __FUNCTION__);
                            }
                            // Are both of the accounts of the same Asset Type?
                            // VERY IMPORTANT!!
                            else if (pRecipientAcct->GetAssetTypeID() !=
                                     theDepositorAccount.GetAssetTypeID()) {
                                OTString strSourceAssetID(
                                    theDepositorAccount.GetAssetTypeID()),
                                    strRecipAssetID(
                                        pRecipientAcct->GetAssetTypeID());
                                OTLog::vOutput(
                                    0, "%s: ERROR - user attempted to %s a "
                                       "payment plan between dissimilar "
                                       "asset types:\n%s\n%s\n",
                                    __FUNCTION__,
                                    bCancelling ? "cancel" : "activate",
                                    strSourceAssetID.Get(),
                                    strRecipAssetID.Get());
                            }
                            // Does it verify?
                            // I call VerifySignature here since
                            // VerifyContractID was already called in
                            // LoadExistingAccount().
                            else if (!pRecipientAcct->VerifySignature(
                                         server_->m_nymServer)) {
                                OTLog::vOutput(0, "%s: ERROR verifying "
                                                  "signature on the Recipient "
                                                  "account.\n",
                                               __FUNCTION__);
                            }
                            // This one is superfluous, but I'm leaving it.
                            // (pPlan and pRecip are both already
                            // matches to a 3rd value: source acct asset type
                            // ID.)
                            else if (pRecipientAcct->GetAssetTypeID() !=
                                     pPlan->GetAssetID()) {
                                const OTString strAssetID1(pPlan->GetAssetID()),
                                    strAssetID2(
                                        pRecipientAcct->GetAssetTypeID());
                                OTLog::vOutput(0, "%s: ERROR wrong Asset Type "
                                                  "ID (%s) on Recipient Acct. "
                                                  "Expected per Plan: %s\n",
                                               __FUNCTION__, strAssetID2.Get(),
                                               strAssetID1.Get());
                            }
                            // At this point I feel pretty confident that the
                            // Payment Plan is a valid request from both
                            // parties.
                            // I have both users AND both accounts and validated
                            // against the Payment Plan, signatures and all.
                            // The only other possibility is that the merchant
                            // is canceling the payment plan before the customer
                            // had a chance to confirm/activate it.
                            else {
                                // If activating, add it to Cron...
                                //
                                // We add the payment plan to the server's Cron
                                // object, which does regular processing.
                                // That object will take care of processing the
                                // payment plan according to its terms.
                                //
                                // NOTE: FYI, inside AddCronItem, since this is
                                // a new CronItem, a Cron Receipt will
                                // be saved with the User's signature on it,
                                // containing the Cron Item from the user's
                                // original request. After that, the item is
                                // stored internally to Cron itself, and
                                // signed by the server--and changes over time
                                // as cron processes. (The original receipt
                                // can always be loaded when necessary.)
                                //
                                if (!bCancelling &&
                                    server_->m_Cron.AddCronItem(
                                        *pPlan, &theNym, true,
                                        OTTimeGetCurrentTime())) // bSaveReceipt=true
                                {
                                    // todo need to be able to "roll back" if
                                    // anything inside this block fails.

                                    // Now we can set the response item as an
                                    // acknowledgement instead of the default
                                    // (rejection)
                                    pResponseItem->SetStatus(
                                        OTItem::acknowledgement);

                                    bOutSuccess = true; // The payment plan
                                                        // activation was
                                                        // successful.

                                    OTLog::vOutput(2, "%s: Successfully added "
                                                      "payment plan to Cron "
                                                      "object.\n",
                                                   __FUNCTION__);

                                    // Server side, the Nym stores a list of all
                                    // open cron item numbers.
                                    // (So we know if there is still stuff open
                                    // on Cron for that Nym, and we know what it
                                    // is.)
                                    //
                                    std::set<int64_t>& theIDSet =
                                        theNym.GetSetOpenCronItems();
                                    theIDSet.insert(pPlan->GetTransactionNum());
                                    theIDSet.insert(pPlan->GetClosingNum());
                                    // // saved below

                                    // This just marks the Closing number so I
                                    // can't USE it again. (Since I'm using it
                                    // as the closing
                                    // number for this cron item now.) I'm still
                                    // RESPONSIBLE for the number until
                                    // RemoveIssuedNumber()
                                    // is called. If we didn't call this here,
                                    // then I could come back later and USE THE
                                    // NUMBER AGAIN!
                                    // (Bad!)
                                    // server_->transactor_.removeTransactionNumber
                                    // was
                                    // already
                                    // called for tranIn->GetTransactionNum()
                                    // (That's the opening number.)
                                    //
                                    // Here's the closing number:
                                    server_->transactor_
                                        .removeTransactionNumber(
                                            theNym, pPlan->GetClosingNum(),
                                            true); // bSave=true
                                    // RemoveIssuedNum will be called for that
                                    // original transaction number when the
                                    // finalReceipt is created.
                                    // RemoveIssuedNum will be called for the
                                    // Closing number when the finalReceipt is
                                    // accepted.

                                    std::set<int64_t>& theIDSet2 =
                                        pRecipientNym->GetSetOpenCronItems();
                                    theIDSet2.insert(
                                        pPlan->GetRecipientOpeningNum());
                                    theIDSet2.insert(
                                        pPlan->GetRecipientClosingNum());
                                    // // saved below

                                    // For recipient, I also remove the opening
                                    // and closing numbers as AVAILABLE FOR USE.
                                    // But they aren't removed as ISSUED until
                                    // later...
                                    // RemoveIssuedNum is called for the
                                    // Recipient's opening number
                                    // onFinalReceipt,
                                    // and it's called for the Recipient's
                                    // closing number when that final receipt is
                                    // closed out.
                                    server_->transactor_
                                        .removeTransactionNumber(
                                            *pRecipientNym,
                                            pPlan->GetRecipientOpeningNum(),
                                            false); // bSave=true
                                    server_->transactor_
                                        .removeTransactionNumber(
                                            *pRecipientNym,
                                            pPlan->GetRecipientClosingNum(),
                                            true); // bSave=true

                                    // Send success notice to other parties.
                                    // (So they can deal with their payments
                                    // inbox and outpayments box,
                                    // where pending copies of the instrument
                                    // may still be waiting.)
                                    //
                                    int64_t lOtherNewTransNumber = 0;
                                    server_->transactor_
                                        .issueNextTransactionNumber(
                                            server_->m_nymServer,
                                            lOtherNewTransNumber,
                                            false); // bStoreTheNumber = false

                                    if (false ==
                                        pPlan->SendNoticeToAllParties(
                                            true, // bSuccessMsg=true
                                            server_->m_nymServer, SERVER_ID,
                                            lOtherNewTransNumber,
                                            // // Each party has its own opening
                                            // number. Handled internally.
                                            strPaymentPlan, nullptr, nullptr,
                                            &theNym)) {
                                        OTLog::vOutput(
                                            0, "%s: Failed notifying parties "
                                               "while trying to activate "
                                               "payment plan: %lld.\n",
                                            __FUNCTION__,
                                            pPlan->GetOpeningNum());
                                    }
                                }
                                else {
                                    if (bCancelling) {
                                        tranOut.SetAsCancelled();

                                        OTLog::vOutput(
                                            0, "%s: Canceling a payment plan "
                                               "before it was ever activated. "
                                               "(At user's request.)\n",
                                            __FUNCTION__);
                                    }
                                    else
                                        OTLog::vOutput(
                                            0, "%s: Unable to add payment plan "
                                               "to Cron. (Failed activating "
                                               "payment plan.)\n",
                                            __FUNCTION__);

                                    // Send a failure notice to the other
                                    // parties.
                                    //
                                    // DROP REJECTION NOTICE HERE TO ALL
                                    // PARTIES....
                                    // SO THEY CAN CLAW BACK THEIR TRANSACTION
                                    // #s....
                                    //
                                    int64_t lOtherNewTransNumber = 0;
                                    server_->transactor_
                                        .issueNextTransactionNumber(
                                            server_->m_nymServer,
                                            lOtherNewTransNumber,
                                            false); // bStoreTheNumber = false

                                    if (false ==
                                        pPlan->SendNoticeToAllParties(
                                            false, server_->m_nymServer,
                                            SERVER_ID, lOtherNewTransNumber,
                                            // // Each party has its own opening
                                            // number. Handled internally.
                                            strPaymentPlan, nullptr, nullptr,
                                            &theNym)) {
                                        // NOTE: A party may deliberately try to
                                        // activate a payment plan without
                                        // signing it.
                                        // (As a way of rejecting it.) This will
                                        // cause rejection notices to go to all
                                        // the other
                                        // parties, allowing them to harvest
                                        // back their closing numbers.
                                        // Since that is expected to happen,
                                        // that means if you have 2 parties, and
                                        // the 2nd one
                                        // "activates" it (without signing),
                                        // then this piece of code here will
                                        // DEFINITELY fail to
                                        // send the rejection notice to the
                                        // first party (since the 2nd one hadn't
                                        // even signed the
                                        // thing yet.)
                                        //
                                        // (Since we expect that to normally
                                        // happen, we don't log an error here.)

                                        //                                  OTLog::vOutput(0,
                                        // "%s: Failed notifying all parties
                                        // about failed activation of payment
                                        // plan: %lld.\n",
                                        //                                                 __FUNCTION__,
                                        // pPlan->GetTransactionNum());
                                    }
                                } // Failure adding Cron Item.
                            }
                        }
                    } // If recipient Nym successfully loaded from storage.
                } // If Payment Plan successfully loaded from Transaction Item.
            }     // else

            // If the payment plan WAS successfully added to Cron, then we don't
            // need to
            // delete it here, since Cron owns it now, and will deal with
            // cleaning it up at the right time.
            if ((nullptr != pPlan) &&
                (pResponseItem->GetStatus() != OTItem::acknowledgement)) {
                delete pPlan;
                pPlan = nullptr;
            }
        }
    }

    // sign the response item before sending it back (it's already been added to
    // the transaction above)
    // Now, whether it was rejection or acknowledgement, it is set properly and
    // it is signed, and it
    // is owned by the transaction, who will take it from here.
    pResponseItem->SignContract(server_->m_nymServer);
    pResponseItem->SaveContract(); // the signing was of no effect because I
                                   // forgot to save.

    pResponseBalanceItem->SignContract(server_->m_nymServer);
    pResponseBalanceItem->SaveContract();
}

void Notary::NotarizeSmartContract(OTPseudonym& theNym,
                                   OTAccount& theActivatingAccount,
                                   OTTransaction& tranIn,
                                   OTTransaction& tranOut, bool& bOutSuccess)
{
    // The outgoing transaction is an "atSmartContract", that is, "a reply to
    // the smartContract request"
    tranOut.SetType(OTTransaction::atSmartContract);

    OTItem* pItem = nullptr;
    OTItem* pBalanceItem = nullptr;
    OTItem* pResponseItem = nullptr;
    OTItem* pResponseBalanceItem = nullptr;

    // The incoming transaction may be sent to inboxes and outboxes, and it
    // will definitely be bundled in our reply to the user as well. Therefore,
    // let's grab it as a string.
    OTString strInReferenceTo;
    OTString strBalanceItem;

    // Grab the actual server ID from this object, and use it as the server ID
    // here.
    const OTIdentifier SERVER_ID(server_->m_strServerID),
        ACTIVATOR_USER_ID(theNym), SERVER_USER_ID(server_->m_nymServer),
        ACTIVATOR_ACCT_ID(theActivatingAccount), USER_ID(theNym);
    const OTString strUserID(USER_ID);
    pItem = tranIn.GetItem(OTItem::smartContract);
    pBalanceItem = tranIn.GetItem(OTItem::transactionStatement);
    pResponseItem =
        OTItem::CreateItemFromTransaction(tranOut, OTItem::atSmartContract);
    pResponseItem->SetStatus(OTItem::rejection); // the default.
    tranOut.AddItem(*pResponseItem); // the Transaction's destructor will
                                     // cleanup the item. It "owns" it now.
    pResponseBalanceItem = OTItem::CreateItemFromTransaction(
        tranOut, OTItem::atTransactionStatement);
    pResponseBalanceItem->SetStatus(OTItem::rejection); // the default.
    tranOut.AddItem(*pResponseBalanceItem); // the Transaction's destructor will
                                            // cleanup the item. It "owns" it
                                            // now.
    if ((nullptr != pItem) &&
        (false == NYM_IS_ALLOWED(strUserID.Get(),
                                 ServerSettings::__transact_smart_contract))) {
        OTLog::vOutput(0, "%s: User %s cannot do this transaction (All smart "
                          "contracts are disallowed in server.cfg)\n",
                       __FUNCTION__, strUserID.Get());
    }
    // For now, there should only be one of these smartContract items inside the
    // transaction.
    // So we treat it that way... I either get it successfully or not.
    else if ((nullptr == pItem) || (nullptr == pBalanceItem)) {
        OTLog::vError("%s: Error, expected OTItem::smartContract and "
                      "OTItem::transactionStatement.\n",
                      __FUNCTION__);
    }
    else {
        if (ACTIVATOR_ACCT_ID != pItem->GetPurportedAccountID()) {
            OTLog::vOutput(0, "%s: Error: Source account ID on the transaction "
                              "does not match activator's account ID on the "
                              "transaction item.\n",
                           __FUNCTION__);
        }
        else if (false ==
                   pBalanceItem->VerifyTransactionStatement(theNym, tranIn)) {
            OTLog::vOutput(0, "%s: Failed verifying transaction statement.\n",
                           __FUNCTION__);
        }
        else {
            pResponseBalanceItem->SetStatus(
                OTItem::acknowledgement); // the transaction agreement was
                                          // successful.

            // The response item will contain a copy of the request item. So I
            // save it into a string
            // here so it can be saved into the "in reference to" field.
            pItem->SaveContractRaw(strInReferenceTo);
            pBalanceItem->SaveContractRaw(strBalanceItem);

            // Server response item being added to server response transaction
            // (tranOut)
            // They're getting SOME sort of response item.

            pResponseItem->SetReferenceString(
                strInReferenceTo); // the response item carries a copy of what
                                   // it's responding to.
            pResponseItem->SetReferenceToNum(
                pItem->GetTransactionNum()); // This response item is IN
                                             // RESPONSE to pItem and its Owner
                                             // Transaction.

            pResponseBalanceItem->SetReferenceString(
                strBalanceItem); // the response item carries a copy of what
                                 // it's responding to.
            pResponseBalanceItem->SetReferenceToNum(
                pItem->GetTransactionNum()); // This response item is IN
                                             // RESPONSE to pItem and its Owner
                                             // Transaction.

            // Also load up the smart contract from inside the transaction item.
            OTString strContract;
            pItem->GetAttachment(strContract);
            OTSmartContract* pContract = new OTSmartContract(SERVER_ID);
            OT_ASSERT(nullptr != pContract);

            // If we failed to load the smart contract...
            if ((false == pContract->LoadContractFromString(strContract))) {
                OTLog::vError(
                    "%s: ERROR loading smart contract from string:\n\n%s\n\n",
                    __FUNCTION__, strContract.Get());
            }
            else if (pContract->GetServerID() != SERVER_ID) {
                const OTString strWrongID(pContract->GetServerID());
                OTLog::vOutput(0, "%s: ERROR bad server ID (%s) on smart "
                                  "contract. Expected %s\n",
                               __FUNCTION__, strWrongID.Get(),
                               server_->m_strServerID.Get());
            }
            else {
                // CANCELING, or ACTIVATING?
                //
                OTIdentifier theCancelerNymID;
                const bool bCancelling =
                    (pContract->IsCanceled() &&
                     pContract->GetCancelerID(theCancelerNymID));
                const int64_t lFoundNum = pContract->GetTransactionNum();
                const int64_t lExpectedNum = pItem->GetTransactionNum();
                int64_t lFoundOpeningNum = 0;
                int64_t lFoundClosingNum = 0;

                OTIdentifier FOUND_USER_ID;
                OTIdentifier FOUND_ACCT_ID;

                if (!bCancelling) // ACTIVATING
                {
                    OTLog::vOutput(
                        0, "Attempting to activate smart contract...\n");

                    lFoundOpeningNum = pContract->GetOpeningNum();
                    lFoundClosingNum = pContract->GetClosingNum();

                    FOUND_USER_ID = pContract->GetSenderUserID();
                    FOUND_ACCT_ID = pContract->GetSenderAcctID();
                }
                else // CANCELING
                {
                    OTLog::vOutput(0,
                                   "Attempting to cancel smart contract...\n");

                    lFoundOpeningNum = pContract->GetOpeningNumber(
                        theCancelerNymID); // See if there's an opening number
                                           // for the canceling Nym.
                    lFoundClosingNum = pContract->GetClosingNumber(
                        ACTIVATOR_ACCT_ID); // See if there's a closing number
                                            // for the current account.

                    if (lFoundOpeningNum > 0) FOUND_USER_ID = theCancelerNymID;
                    if (lFoundClosingNum > 0) FOUND_ACCT_ID = ACTIVATOR_ACCT_ID;
                }

                if (lFoundNum != lExpectedNum) {
                    OTLog::vOutput(
                        0, "%s: ERROR bad main opening transaction number on "
                           "smart contract. Found: %lld  Expected: %lld\n"
                           "FYI, pItem->GetTransactionNum() is %lld.\n",
                        __FUNCTION__, lFoundNum, lExpectedNum,
                        pItem->GetTransactionNum());
                }
                else if (lFoundOpeningNum != lExpectedNum) {
                    OTLog::vOutput(
                        0, "%s: ERROR bad opening transaction number on smart "
                           "contract. Found: %lld  Expected: %lld\n",
                        __FUNCTION__, lFoundOpeningNum, lExpectedNum);
                }
                else if (FOUND_USER_ID != ACTIVATOR_USER_ID) {
                    const OTString strWrongID(ACTIVATOR_USER_ID);
                    const OTString strRightID(FOUND_USER_ID);
                    OTLog::vOutput(0, "%s: ERROR wrong user ID (%s) used while "
                                      "%s smart contract. Expected from "
                                      "contract: %s\n",
                                   __FUNCTION__, strWrongID.Get(),
                                   bCancelling ? "canceling" : "activating",
                                   strRightID.Get());
                }
                else if (FOUND_ACCT_ID != ACTIVATOR_ACCT_ID) {
                    const OTString strSenderAcctID(FOUND_ACCT_ID),
                        strActivatorAcctID(ACTIVATOR_ACCT_ID);
                    OTLog::vOutput(0, "%s: ERROR wrong asset Acct ID used (%s) "
                                      "to %s smart contract. Expected from "
                                      "contract: %s\n",
                                   __FUNCTION__, strActivatorAcctID.Get(),
                                   bCancelling ? "cancel" : "activate",
                                   strSenderAcctID.Get());
                }
                // The transaction number opens the smart contract, but there
                // must also be a closing number for closing it.
                else if ((pContract->GetCountClosingNumbers() <
                          1) || // the transaction number was verified before we
                                // entered this function, so only the closing #
                                // is left...
                         !server_->transactor_.verifyTransactionNumber(
                             theNym, lFoundClosingNum)) // Verify that it can
                                                        // still be USED (not
                                                        // closed... that's
                                                        // VerifyIssuedNum())
                {
                    OTLog::vOutput(0, "%s: ERROR: the Closing number %lld "
                                      "wasn't available for use while %s a "
                                      "smart contract.\n",
                                   __FUNCTION__, lFoundClosingNum,
                                   bCancelling ? "canceling" : "activating");
                }
                // NOTE: since theNym has ALREADY been substituted for the
                // Server's Nym by this point, if indeed they are the same Nym,
                // then I could probably just ALLOW the server to be a party to
                // a smart contract. It will definitely be on the "list of
                // nyms that are already loaded" due to the substitution. So
                // really it's just a matter of security review, and the below
                // block could be commented out (or not.)  ALSO: If I'm going to
                // enforce this, then I need to do it for ALL parties, not just
                // the activator!
                else if ((pContract->GetSenderUserID() == SERVER_USER_ID) ||
                         (nullptr !=
                          pContract->FindPartyBasedOnNymAsAgent(
                              server_->m_nymServer))) {
                    OTLog::vOutput(
                        0,
                        "%s: ** SORRY ** but the server itself is NOT ALLOWED "
                        "to be a party "
                        "to any smart contracts. (Pending security review.)\n",
                        __FUNCTION__);
                }
                //
                // VERIFY SMART CONTRACT
                /*
                  -- Loop through all parties and load up the authorizing
                  agent's Nym, if not already loaded, for each.
                  -- Verify each party, that the authorizing agent is good, and
                  verify his signature on the party's copy
                  of the contract.
                  -- Definitely during this, need to make sure that the contents
                  of the signed version match the contents
                  of the main version, for each signer.
                  -- Verify that the authorizing agent actually has the opening
                  transaction # for the party issued to him.
                  -- EVEN IF VERIFICATION FAILS HALFWAY THOUGH, REMOVE that
                  opening transaction # for each-and-every agent.
                  (So he can't use it twice--leaving it as issued, but no longer
                  as "available to be used on another
                  transaction".) Otherwise, if verification failed halfway
                  through, with half of the parties having their
                  opening numbers already burned, and the other half not, then
                  it would be impossible to tell, based on
                  the failed message itself, which group YOU are in, and
                  therefore whether YOU need to harvest that number
                  back or not (in order to avoid going out-of-sync.) THEREFORE
                  WE BURN ALL OPENING NUMBERS so the client API
                  can just assume the opening number is burned, if the
                  transaction ran at all. (And, as normal, if the
                  transaction did NOT run at all, e.g. if the message failed
                  before the transaction had a chance to run,
                  then all opening numbers are still good, for all
                  parties--including the activator.)

                  -- NOTE: this means, if it succeeds, the opening numbers are
                  marked as IN USE
                  (RemoveTransactionNum but NOT RemoveIssuedNum.) But if it
                  FAILS, then we also need to RemoveIssuedNum...
                  So I'm adding that to VerifySmartContract.

                  -- Next, loop through all the asset accounts...
                  -- For each, get a pointer to the authorized agent and verify
                  the CLOSING number for that asset acct.
                  (AND mark that number as "used but still issued.") Again, do
                  this for ALL asset accounts on the smart
                  contract, even if some of them fail the verification process.
                  (It's also okay to skip the accounts for
                  parties who failed verification.) If anything fails, then at
                  the very end, add the closing numbers back
                  again as "available for use" on those nyms.

                  -- Since we're looping through all the agents, and looping
                  through all the asset accounts, and
                  checking the agent for each asset account, then we might as
                  well make sure that each agent is
                  a legit agent for the party, and that each account has a legit
                  agent lording over it.
                */
                else if (bCancelling && !pContract->VerifySignature(theNym)) {
                    OTLog::vOutput(0, "%s: Failed verifying canceler signature "
                                      "while canceling smart contract.\n",
                                   __FUNCTION__);
                }

                // We let it run through the verifier here, even if we are
                // cancelling.
                // The reason is because this is where the various
                // opening/closing numbers
                // are burned/reserved/etc. So even cancellation needs this part
                // done.
                //
                else if (!pContract->VerifySmartContract(
                             theNym, theActivatingAccount, server_->m_nymServer,
                             true)) // bBurnTransNo=false by default, but here
                                    // we pass TRUE.
                {
                    if (bCancelling) {
                        tranOut.SetAsCancelled();

                        OTLog::vOutput(0, "%s: Canceling a smart contract "
                                          "before it was ever even activated "
                                          "(at user's request.)\n",
                                       __FUNCTION__);
                    }
                    else
                        OTLog::vOutput(
                            0,
                            "%s: This smart contract has FAILED to verify.\n",
                            __FUNCTION__);

                    /*

                      ------ TODO: Smart Contracts -----------

                      Done:  Whenever a party confirms a smart contract (sending
                      it on to the next party) then a copy of
                      the smart contract should go into that party's
                      paymentOutbox. Same thing if the party is the last
                      one in the chain, and has activated it on to the server. A
                      copy sits in the paymentOutbox until
                      that smart contract is either successfully activated, or
                      FAILS to activate.

                      If a smart contract activates,
                      OTAgreement::DropServerNoticeToNymbox already sends an
                      'acknowledgment' notice to all parties.

                      Done: If a smart contract fails to activate, it should
                      ALSO send a notice ('rejection') to
                      all parties.

                      TODO: When a party receives a rejection notice in his
                      Nymbox for a certain smart contract,
                      he looks up that same smart contract in his paymentOutbox,
                      HARVESTS THE CLOSING NUMBERS, and
                      then moves the notice from his outpayments box to his
                      recordBox.
                      NOTE: the notice might be in his payments inbox
                      (sometimes) instead of his outpayments box.
                      Possibly even both. How so? See below. Point being: Need
                      to check both, at this point.

                      Until this is added, then clients will go out of sync on
                      rejected smart contracts. (Not the kind
                      of out-of-sync where they can't do any transactions, but
                      rather, the kind where they have certain
                      numbers signed out forever but then never use them on
                      anything because their client thinks those
                      numbers were already used on a smart contract somewhere,
                      and without the above code they would
                      never have clawed back those numbers.)

                      MORE DETAILS:

                      *** When I send a smart contract on to the next party,
                      remember it's sitting in my payments inbox
                      at first. When I confirm it, a copy goes into my
                      outpayments box. Then when I actually SEND it, a
                      copy goes into my outpayments box AGAIN. (This is already
                      smart enough to remove this first copy,
                      when this happens.) If I activate it (rather than sending
                      it on, perhaps I'm the last one) then it's
                      already in my outpayments box from the confirmation.

                      BUT WHEN DO I REMOVE IT FROM THE payments *INBOX* ?
                      Answer: when the successful server reply is
                      received from the sendUserInstrument. What if I don't send
                      it to another user? Perhaps I activate it.
                      In that case, whether the activation succeeds or fails, I
                      will get an acknowledgment (or rejection)
                      notice in my Nymbox. Therefore I can harvest the numbers
                      back when that notice is received (or not.)
                      That will be from my outpayments box. But removing it from
                      my INBOX should happen when I get the server
                      response to the activation (just as when I get the server
                      response to sendUserInstrument.)
                      If I never tried to activate it, and never tried to send
                      it to the next party, and never discarded it,
                      then it should remain in my inbox, until I choose to do
                      one of those things.


                      *** The sent contract remains in the outPayments box
                      until:
                      A. Activated. When: When the acknowledgment of activation
                      is received through the Nymbox.
                      B. Failed activation. When: Rejection of activation
                      received through the Nymbox.
                      C. Expiration. Expired notices may be harvested from the
                      outpayments box. After all,
                      they were apparently never activated or even attempted,
                      since either of those actions
                      would have resulted in a rejection notice which would have
                      already removed the outpayments
                      box. So the transaction numbers can be harvested. BUT make
                      sure you have latest version of
                      files first, so you know for sure that the contract never
                      really was activated or attempted.

                      *** What if the incoming smart contract is discarded,
                      instead of confirmed?
                      This means it never goes into my outbox in the first
                      place. It's in my inbox, then I discard it.
                      Then what? In one scenario, the user simply throws it
                      away. He removes it from the box and never
                      notifies anyone. This is physically possible so we must
                      consider it. In that case, it's still sitting
                      in other people's outboxes, and will eventually expire,
                      and then those people will just harvest back
                      their transaction numbers. It's kind of rude not to notify
                      them, but everything will still be OKAY.
                      They also still have the power, since it hasn't been
                      activated, to "false activate" it, which will fail
                      since it's not fully-confirmed yet, and then the rejection
                      notice will come through and remove it from
                      their outboxes. All parties are notified in that case.
                      The polite thing to do, instead of just discarding it,
                      would be for me to do the same (false-activate it)
                      meaning I activate it, but without signing it. And
                      possibly putting some other "This must fail" indicator
                      on the message, so the server doesn't waste a lot of time
                      figuring that out. Then the failure causes all
                      the parties who DID sign it, to get a rejection
                      notification, and I can remove it from my payments inbox
                      at
                      that time, when they are all removing it from their
                      outboxes.

                      *** What if the incoming contract is discarded AFTER it
                      was confirmed? From the outbox, meaning it hasn't
                      been activated yet. Perhaps I sent someone a cheque that
                      hasn't been cashed yet. Perhaps I sent someone
                      a signed smart contract but they haven't activated it yet.
                      Therefore I still have a chance to cancel it.
                      I can't just discard it, since they can still deposit
                      their copy whenever they want. But if I RUN IT THROUGH,
                      then it will be INVALIDATED thereafter -- and if I beat
                      them to the punch, then it will work. Of course, if
                      they activate it, then I will get an activation notice,
                      which will automatically remove it from my outbox.
                      So I beat them to the punch, by activating / depositing it
                      myself, which fails, and then we both get rejection
                      notices. That removes it from my outbox, as well as the
                      inbox of the guy who I had been stuck waiting on in
                      the first place.


                      WHY WAS IT in whichever box it was in? (Just curious.)
                      Well...
                      If inbox, because I discarded it without confirming, yet
                      wanted to be nice and let people who
                      had, harvest their numbers back. (Otherwise they'd still
                      eventually expire.) If outpayments box,
                      because I activated it (so it's in that box) and it's just
                      legitimately a failed attempt on my
                      part, or because I confirmed it and sent to the next guy,
                      and he hasn't activated it yet, and
                      I've changed my mind and wish to cancel it. Either way,
                      once I do, I will get the notice (as will
                      any other parties) and then it will be removed from that
                      box (and placed in the records box.)
                      Another scenario: It's removed from my inbox when some
                      other confirming party "false activates" it
                      in order to cancel it and remove it from his outbox. He
                      HAD been sitting there waiting on me, while
                      the notice sat in my inbox. But now that it's been
                      invalidated at the server, I will get a rejection
                      notice from the server which should remove the one that
                      was sitting in my inbox, to the record box.


                      ACTIONS:

                      -- When successful "sendUserInstrument" server reply is
                      received,
                      remove that instrument from payments inbox. (If it's there
                      -- it can be.)

                      -- When party receives notice that smart contract has been
                      activated,
                      remove the instrument from outpayments box. (If it's there
                      -- it can be.)

                      -- When party receives notice that smart contract has
                      failed activation attempt, then remove
                      the instrument from payments inbox AND outpayments box.
                      (If there -- could be for either.)

                      Does this cover all cases?

                      -- Any _sent_ instrument will properly be removed from the
                      payments inbox.
                      -- It will go into the outpayments box. Once it activates,
                      it will be removed again from that box. (For all parties.)
                      -- If it fails to activate, or if a party discards it from
                      inbox (through a deliberate failed activation) or if a
                      party
                      discards it from the outbox (through a deliberate failed
                      activation) either way, it will be removed from both
                      boxes.
                      -- If it expires while sitting in my inbox, my high-level
                      API is responsible to remove it and harvest the numbers.
                      -- It if expires while sitting in my outbox, my high-level
                      API is responsible to remove it and harvest the numbers.

                      It can be sent, discarded (from outbox, as a
                      scramble-to-discard-it-before-next-guy-deposits it),
                      discarded (from inbox,
                      when I decide I won't sign it), it can be ignored until
                      expiration (either box), and it can legitimately activate
                      or fail
                      to activate, and either way, all the parties who confirmed
                      it will get a notice and harvest (if necessary.)

                      THIS SEEMS TO COVER ALL CASES!

                      One more thing, just noticed: Whether success or fail, the
                      opening AND closing numbers are marked as "used but not
                      closed"
                      on the Nym's record. We do this above for all Nyms just
                      doing verification, since we can't fail halfway through
                      and have
                      inconsistent results between them. (All or nothing.)

                      THERFORE: 1. When the Nyms receive "SUCCESS" activating
                      the smart contract, they ALL know that the opening AND
                      closing
                      numbers are marked off as used, and can only be closed
                      thereafter through the final receipt. 2. When the Nyms
                      receive
                      FAILED activating the smart contract... Do they harvest
                      the numbers back? NOT if they were all marked off already!
                      So
                      next, if failure here I need to mark them all as closed,
                      right? Since we failed? And then the client side, when he
                      gets
                      the notice, he needs to mark them as closed as well. (NOT
                      harvest them.) We could alternately mark them all as
                      "still
                      available" on the server side (or all closing numbers
                      anyway) and mark all the opening numbers as closed. But
                      whatever we
                      do, the client side needs to do the same thing.
                      The only time we harvest ALL the numbers is then when we
                      haven't even sent it to the server yet? Otherwise we mark
                      them
                      as "used and not closed" (if success) or if failure, we
                      mark them as:  ????? Perhaps all still open except the
                      main opening
                      one? Or perhaps all the closing numbers are still
                      available and the opening numbers are burned? Or just ALL
                      numbers are
                      burned? Which? Why?

                      NOTE: I found the answer in the comments in
                      OTSmartContract::VerifySmartContract. (And there are very
                      good reasons
                      involved for why I went the way that I did. Read it for
                      those reasons.) Conclusion:

                      If there is a failed activation attempt, then all parties
                      get a notice, and all parties can CLOSE the opening
                      number,
                      which was burned, and they can HARVEST the closing
                      numbers, which were made new again.

                      But if the activation attempt succeeded, then all parties
                      get a notice, and all parties will continue as they were:
                      with the opening AND closing numbers marked as "Still
                      issued but in use." Their opening numbers will not close
                      until
                      the smart contract is deactivated, and their closing
                      numbers will not close until their final receipts have
                      been closed.
                      You might ask, "Then why send the notice, if the
                      transaction numbers are already set up correctly on the
                      client side?"
                      The answer is, because the client still does things based
                      on that notice. Like for example, it removes the confirmed
                      copy of that smart contract from its outpayments box.

                    */

                    // DROP REJECTION NOTICE HERE TO ALL PARTIES....
                    // SO THEY CAN CLAW BACK THEIR TRANSACTION #s....
                    //
                    int64_t lNewTransactionNumber = 0;
                    server_->transactor_.issueNextTransactionNumber(
                        server_->m_nymServer, lNewTransactionNumber, false);

                    if (false ==
                        pContract->SendNoticeToAllParties(
                            false, server_->m_nymServer, SERVER_ID,
                            lNewTransactionNumber,
                            // // Each party has its own opening number. Handled
                            // internally.
                            strContract, nullptr, nullptr, &theNym)) {
                        // NOTE: A party may deliberately try to activate a
                        // smart contract without signing it.
                        // (As a way of rejecting it.) This will cause rejection
                        // notices to go to all the other
                        // parties, allowing them to harvest back their closing
                        // numbers.
                        // Since that is expected to happen, that means if you
                        // have 5 parties, and the 3rd one
                        // "activates" the contract, then this piece of code
                        // here will DEFINITELY fail to send
                        // the rejection notice to the last 2 parties (since
                        // they hadn't even signed the contract
                        // yet.)
                        //
                        // (Since we expect that to normally happen, we don't
                        // log an error here.)
                    }
                } // smart contract is no good.

                // The smart contract is good...
                //
                // NOTIFY ALL PARTIES and ACTIVATE.
                //
                // This is important to notify first, because the hooks in
                // OTSmartContract::onActivate() could very
                // potentially trigger MORE receipts, and we want to make sure
                // the activation receipt comes first.
                //
                else {
                    int64_t lNewTransactionNumber = 0;
                    server_->transactor_.issueNextTransactionNumber(
                        server_->m_nymServer, lNewTransactionNumber, false);

                    if (false ==
                        pContract->SendNoticeToAllParties(
                            true, server_->m_nymServer, SERVER_ID,
                            lNewTransactionNumber,
                            // // Each party has its own opening number. Handled
                            // internally.
                            strContract, nullptr, nullptr, &theNym)) {
                        OTLog::vOutput(
                            0, "%s: Failed notifying parties while trying to "
                               "activate smart contract: %lld.\n",
                            __FUNCTION__, pContract->GetTransactionNum());
                    }
                    // Add it to Cron...
                    else if (server_->m_Cron.AddCronItem(
                                 *pContract, &theNym, true,
                                 OTTimeGetCurrentTime())) {
                        // We add the smart contract to the server's Cron
                        // object, which does regular processing.
                        // That object will take care of processing the smart
                        // contract according to its terms.
                        //
                        // NOTE: FYI, inside AddCronItem, since this is a new
                        // CronItem, a Cron Receipt will
                        // be saved with the User's signature on it, containing
                        // the Cron Item from the user's
                        // original request. After that, the item is stored
                        // internally to Cron itself, and
                        // signed by the server--and changes over time as cron
                        // processes. (The original receipt
                        // can always be loaded when necessary.)
                        //

                        // Now we can set the response item as an
                        // acknowledgement instead of rejection (the default)
                        pResponseItem->SetStatus(OTItem::acknowledgement);
                        bOutSuccess = true; // The smart contract activation was
                                            // successful.
                        OTLog::vOutput(0, "%s: Successfully added smart "
                                          "contract to Cron object.\n",
                                       __FUNCTION__);
                    } // If smart contract verified.
                    else {
                        OTLog::vOutput(0, "%s: Unable to add smart contract to "
                                          "Cron object.\n",
                                       __FUNCTION__);
                    }
                } // contract verifies, activate it.
            }     // else
            // If the smart contract WAS successfully added to Cron, then we
            // don't need to
            // delete it here, since Cron owns it now, and will deal with
            // cleaning it up at the right time.
            if ((nullptr != pContract) &&
                (pResponseItem->GetStatus() != OTItem::acknowledgement)) {
                delete pContract;
                pContract = nullptr;
            }
        }
    }

    // sign the response item before sending it back (it's already been added to
    // the transaction above)
    // Now, whether it was rejection or acknowledgement, it is set properly and
    // it is signed, and it
    // is owned by the transaction, who will take it from here.
    pResponseItem->SignContract(server_->m_nymServer);
    pResponseItem->SaveContract(); // the signing was of no effect because I
                                   // forgot to save.

    pResponseBalanceItem->SignContract(server_->m_nymServer);
    pResponseBalanceItem->SaveContract();
}

// DONE: The code inside here is just a copy of payment plan.
// Make it into a REAL notarizeCancelCronItem so it actually works.
//
// Cancel a market offer.
// (DONE:  NEED TO CHANGE THIS INTO A TRANSACTION, INSTEAD OF A MESSAGE...)
// Will become "Cancel Cron Item"
//
// DONE: This needs to be "CANCEL CRON ITEM" and it should make use of CLOSING
// NUMBERS that should
// SHOULD ALREADY be available in the CRON ITEMS!
//
// Basically it allows you to cancel payment plans OR market offers, and places
// the appropriate
// cancellation receipts (preferably through polymorphism, versus some huge 'if'
// block here...
//
// When cancelling it uses the closing numbers provided in the cron items.
// Then code the expiration part in OTCron Item or wherever, which should use
// the SAME closing numbers.
//
void Notary::NotarizeCancelCronItem(OTPseudonym& theNym,
                                    OTAccount& theAssetAccount,
                                    OTTransaction& tranIn,
                                    OTTransaction& tranOut, bool& bOutSuccess)
{
    // The outgoing transaction is an "atCancelCronItem", that is, "a reply to
    // the cancelCronItem request"
    tranOut.SetType(OTTransaction::atCancelCronItem);

    OTItem* pItem = nullptr;
    OTItem* pBalanceItem = nullptr;
    OTItem* pResponseItem = nullptr;
    OTItem* pResponseBalanceItem = nullptr;

    // The incoming transaction may be sent to inboxes and outboxes, and it
    // will definitely be bundled in our reply to the user as well. Therefore,
    // let's grab it as a string.
    OTString strInReferenceTo;
    OTString strBalanceItem;

    // Grab the actual server ID from this object, and use it as the server ID
    // here.
    const OTIdentifier SERVER_ID(server_->m_strServerID), USER_ID(theNym);

    const OTString strUserID(USER_ID);

    pBalanceItem = tranIn.GetItem(OTItem::transactionStatement);
    pResponseItem =
        OTItem::CreateItemFromTransaction(tranOut, OTItem::atCancelCronItem);
    pResponseItem->SetStatus(OTItem::rejection); // the default.
    tranOut.AddItem(*pResponseItem); // the Transaction's destructor will
                                     // cleanup the item. It "owns" it now.

    pResponseBalanceItem = OTItem::CreateItemFromTransaction(
        tranOut, OTItem::atTransactionStatement);
    pResponseBalanceItem->SetStatus(OTItem::rejection); // the default.
    tranOut.AddItem(*pResponseBalanceItem); // the Transaction's destructor will
                                            // cleanup the item. It "owns" it
                                            // now.
    if (!NYM_IS_ALLOWED(strUserID.Get(),
                        ServerSettings::__transact_cancel_cron_item)) {
        OTLog::vOutput(
            0, "%s: User %s cannot do this transaction "
               "(CancelCronItem messages are disallowed in server.cfg)\n",
            __FUNCTION__, strUserID.Get());
    }
    else if (nullptr == pBalanceItem) {
        OTString strTemp(tranIn);
        OTLog::vOutput(
            0, "%s: Expected transaction statement in trans# %lld: \n\n%s\n\n",
            __FUNCTION__, tranIn.GetTransactionNum(),
            strTemp.Exists() ? strTemp.Get()
                             : " (ERROR LOADING TRANSACTION INTO STRING) ");
    }
    // For now, there should only be one of these cancelCronItem items inside
    // the transaction.
    // So we treat it that way... I either get it successfully or not.
    else if (nullptr != (pItem = tranIn.GetItem(OTItem::cancelCronItem))) {
        // The response item will contain a copy of the request item. So I save
        // it into a string
        // here so it can be saved into the "in reference to" field.
        pItem->SaveContractRaw(strInReferenceTo);
        pBalanceItem->SaveContractRaw(strBalanceItem);

        // ASSET_ACCT_ID is the ID on the "from" Account that was passed in.
        //
        const OTIdentifier ASSET_ACCT_ID(theAssetAccount);

        // Server response item being added to server response transaction
        // (tranOut)
        // They're getting SOME sort of response item.

        pResponseItem->SetReferenceString(strInReferenceTo); // the response
                                                             // item carries a
                                                             // copy of what
                                                             // it's responding
                                                             // to.
        pResponseItem->SetReferenceToNum(
            pItem->GetTransactionNum()); // This response item is IN RESPONSE to
                                         // pItem and its Owner Transaction.

        pResponseBalanceItem->SetReferenceString(
            strBalanceItem); // the response item carries a copy of what it's
                             // responding to.
        pResponseBalanceItem->SetReferenceToNum(
            pItem->GetTransactionNum()); // This response item is IN RESPONSE to
                                         // pItem and its Owner Transaction.

        if (!(pBalanceItem->VerifyTransactionStatement(
                theNym, tranIn))) // isRealTransaction=true
        {
            OTLog::vOutput(0, "ERROR verifying transaction statement in "
                              "NotarizeCancelCronItem.\n");
        }
        else {
            pResponseBalanceItem->SetStatus(
                OTItem::acknowledgement); // the transaction agreement was
                                          // successful.

            const int64_t lReferenceToNum = pItem->GetReferenceToNum();

            // I'm using the operator== because it exists. (Although now I
            // believe != exists also)
            // If the ID on the "from" account that was passed in,
            // does not match the "Acct From" ID on this transaction item
            if (!(ASSET_ACCT_ID == pItem->GetPurportedAccountID())) {
                OTLog::Output(0, "Error: Asset account ID on the transaction "
                                 "does not match asset account "
                                 "ID on the transaction item.\n");
            }
            else // LET'S SEE IF WE CAN REMOVE IT THEN...
            {
                OTCronItem* pCronItem =
                    server_->m_Cron.GetItemByValidOpeningNum(lReferenceToNum);

                // Check for the closing number here (that happens in
                // OTCronItem, since it's polymorphic.)

                bool bSuccess = false;

                if ((nullptr != pCronItem) &&
                    (pCronItem->CanRemoveItemFromCron(theNym))) // see if theNym
                                                                // has right to
                                                                // remove the
                                                                // cronItem from
                                                                // processing.
                {
                    bSuccess = server_->m_Cron.RemoveCronItem(
                        pCronItem->GetTransactionNum(), theNym); // <=====
                }

                // If we were just successful in removing the offer from the
                // market, that means a finalReceipt was
                // just dropped into the inboxes for the relevant asset
                // accounts. Once I process that receipt out of my
                // inbox, (which will require my processing out all related
                // marketReceipts) then the closing number
                // will be removed from my list of responsibility.

                if (bSuccess) {
                    // Now we can set the response item as an acknowledgement
                    // instead of the default (rejection)
                    pResponseItem->SetStatus(OTItem::acknowledgement);

                    bOutSuccess =
                        true; // The "cancel cron item" was successful.

                    OTLog::vOutput(2, "Successfully removed Cron Item from "
                                      "Cron object, based on ID: %lld\n",
                                   (nullptr != pCronItem)
                                       ? pCronItem->GetTransactionNum()
                                       : lReferenceToNum);

                    // Any transaction numbers that need to be cleared, happens
                    // inside RemoveCronItem().
                }
                else {
                    OTLog::Output(0, "Unable to remove Cron Item from Cron "
                                     "object "
                                     "Notary::NotarizeCancelCronItem\n");
                }
            }
        } // transaction statement verified.
    }
    else {
        OTString strTemp(tranIn);
        OTLog::vOutput(
            0, "Error, expected OTItem::cancelCronItem "
               "in Notary::NotarizeCancelCronItem for trans# %lld:\n\n%s\n\n",
            tranIn.GetTransactionNum(),
            strTemp.Exists() ? strTemp.Get()
                             : " (ERROR LOADING TRANSACTION FROM STRING) ");
    }
    // sign the response item before sending it back (it's already been added to
    // the transaction above)
    // Now, whether it was rejection or acknowledgement, it is set properly and
    // it is signed, and it
    // is owned by the transaction, who will take it from here.
    pResponseItem->SignContract(server_->m_nymServer);
    pResponseItem->SaveContract(); // the signing was of no effect because I
                                   // forgot to save.

    pResponseBalanceItem->SignContract(server_->m_nymServer);
    pResponseBalanceItem->SaveContract();
}

/// a user is exchanging in or out of a basket.  (Ex. He's trading 2 gold and 3
/// silver for 10 baskets, or vice-versa.)
void Notary::NotarizeExchangeBasket(OTPseudonym& theNym, OTAccount& theAccount,
                                    OTTransaction& tranIn,
                                    OTTransaction& tranOut, bool& bOutSuccess)
{
    // The outgoing transaction is an "atExchangeBasket", that is, "a reply to
    // the exchange basket request"
    tranOut.SetType(OTTransaction::atExchangeBasket);

    OTItem* pItem = tranIn.GetItem(OTItem::exchangeBasket);
    OTItem* pBalanceItem = tranIn.GetItem(OTItem::balanceStatement);
    OTItem* pResponseItem = nullptr;
    OTItem* pResponseBalanceItem = nullptr;

    // The incoming transaction may be sent to inboxes and outboxes, and it
    // will probably be bundled in our reply to the user as well. Therefore,
    // let's grab it as a string.
    OTString strInReferenceTo;
    OTString strBalanceItem;

    const OTIdentifier USER_ID(theNym), SERVER_ID(server_->m_strServerID),
        BASKET_CONTRACT_ID(theAccount.GetAssetTypeID()), ACCOUNT_ID(theAccount);

    const OTString strUserID(USER_ID);

    std::unique_ptr<OTLedger> pInbox(
        theAccount.LoadInbox(server_->m_nymServer));
    std::unique_ptr<OTLedger> pOutbox(
        theAccount.LoadOutbox(server_->m_nymServer));

    pResponseItem =
        OTItem::CreateItemFromTransaction(tranOut, OTItem::atExchangeBasket);
    pResponseItem->SetStatus(OTItem::rejection); // the default.
    tranOut.AddItem(*pResponseItem); // the Transaction's destructor will
                                     // cleanup the item. It "owns" it now.

    pResponseBalanceItem =
        OTItem::CreateItemFromTransaction(tranOut, OTItem::atBalanceStatement);
    pResponseBalanceItem->SetStatus(OTItem::rejection); // the default.
    tranOut.AddItem(*pResponseBalanceItem); // the Transaction's destructor will
                                            // cleanup the item. It "owns" it
                                            // now.
    bool bSuccess = false;

    if (!NYM_IS_ALLOWED(strUserID.Get(),
                        ServerSettings::__transact_exchange_basket)) {
        OTLog::vOutput(0, "Notary::NotarizeExchangeBasket: User %s cannot do "
                          "this transaction (All basket exchanges are "
                          "disallowed in server.cfg)\n",
                       strUserID.Get());
    }
    else if (nullptr == pItem) {
        OTLog::Output(0, "Notary::NotarizeExchangeBasket: No exchangeBasket "
                         "item found on this transaction.\n");
    }
    else if (nullptr == pBalanceItem) {
        OTLog::Output(0, "Notary::NotarizeExchangeBasket: No Balance "
                         "Agreement item found on this transaction.\n");
    }
    else if ((nullptr == pInbox)) {
        OTLog::Error("Error loading or verifying inbox.\n");
    }
    else if ((nullptr == pOutbox)) {
        OTLog::Error("Error loading or verifying outbox.\n");
    }
    else {
        pItem->SaveContractRaw(strInReferenceTo);
        pBalanceItem->SaveContractRaw(strBalanceItem);

        pResponseItem->SetReferenceString(strInReferenceTo); // the response
                                                             // item carries a
                                                             // copy of what
                                                             // it's responding
                                                             // to.
        pResponseItem->SetReferenceToNum(
            pItem->GetTransactionNum()); // This response item is IN RESPONSE to
                                         // pItem and its Owner Transaction.

        pResponseBalanceItem->SetReferenceString(
            strBalanceItem); // the response item carries a copy of what it's
                             // responding to.
        pResponseBalanceItem->SetReferenceToNum(
            pBalanceItem->GetTransactionNum()); // This response item is IN
                                                // RESPONSE to tranIn's balance
                                                // agreement
        // Now after all that setup, we do the balance agreement!
        if (false ==
            pBalanceItem->VerifyBalanceStatement(
                0,       // the one balance agreement that doesn't change any
                         // balances.
                theNym,  // Could have been a transaction agreement.
                *pInbox, // Still could be, in fact....
                *pOutbox, theAccount, tranIn)) {
            OTLog::vOutput(0, "Notary::NotarizeExchangeBasket: ERROR "
                              "verifying balance statement.\n");

        }
        else // BALANCE AGREEMENT WAS SUCCESSFUL.......
        {
            pResponseBalanceItem->SetStatus(
                OTItem::acknowledgement); // the balance agreement was
                                          // successful.

            // Set up some account pointer lists for later...
            listOfAccounts listUserAccounts, listServerAccounts;
            std::list<OTLedger*> listInboxes;

            // Here's the request from the user.
            OTString strBasket;
            Basket theBasket, theRequestBasket;

            pItem->GetAttachment(strBasket);

            int64_t lTransferAmount = 0;

            // Now we have the Contract ID from the basket account,
            // we can get a pointer to its asset contract...

            OTIdentifier BASKET_ACCOUNT_ID;

            OTAccount* pBasketAcct = nullptr;
            std::unique_ptr<OTAccount> theBasketAcctGuardian;

            bool bLookup =
                server_->transactor_.lookupBasketAccountIDByContractID(
                    BASKET_CONTRACT_ID, BASKET_ACCOUNT_ID);
            if (!bLookup) {
                OTLog::Error("Notary::NotarizeExchangeBasket: Asset type is "
                             "not a basket currency.\n");
            }
            else if (!strBasket.Exists() ||
                       !theRequestBasket.LoadContractFromString(strBasket) ||
                       !theRequestBasket.VerifySignature(theNym)) {
                OTLog::Error("Notary::NotarizeExchangeBasket: Expected "
                             "verifiable basket object to be attached to "
                             "exchangeBasket item.\n");
            }
            else if (theRequestBasket.GetRequestAccountID() !=
                       theAccount.GetPurportedAccountID()) {
                OTLog::Error("Notary::NotarizeExchangeBasket: User's main "
                             "account ID according to request basket doesn't "
                             "match theAccount.\n");
            }
            else if (false ==
                       server_->transactor_.verifyTransactionNumber(
                           theNym, theRequestBasket.GetClosingNum())) {
                OTLog::Error("Notary::NotarizeExchangeBasket: Closing number "
                             "used for User's main account receipt was not "
                             "available for use...\n");
            }
            else { // Load the basket account and make sure it exists.
                pBasketAcct = OTAccount::LoadExistingAccount(BASKET_ACCOUNT_ID,
                                                             SERVER_ID);

                // If the pointer is nullptr, that works too. Otherwise it
                // cleans
                // up the object at the end of this function.
                theBasketAcctGuardian.reset(pBasketAcct);

                if (nullptr == pBasketAcct) {
                    OTLog::Error("ERROR loading the basket account in "
                                 "Notary::NotarizeExchangeBasket\n");
                }
                // Does it verify?
                // I call VerifySignature here since VerifyContractID was
                // already called in LoadExistingAccount().
                else if (!pBasketAcct->VerifySignature(server_->m_nymServer)) {
                    OTLog::Error("ERROR verifying signature on the basket "
                                 "account in "
                                 "Notary::NotarizeExchangeBasket\n");
                }
                else {
                    // Now we get a pointer to its asset contract...
                    OTAssetContract* pContract =
                        server_->transactor_.getAssetContract(
                            BASKET_CONTRACT_ID);

                    // Now let's load up the actual basket, from the actual
                    // asset contract.
                    if (pContract &&
                        theBasket.LoadContractFromString(
                            pContract->GetBasketInfo()) &&
                        theBasket.VerifySignature(server_->m_nymServer) &&
                        theBasket.Count() == theRequestBasket.Count() &&
                        theBasket.GetMinimumTransfer() ==
                            theRequestBasket.GetMinimumTransfer()) {
                        // Let's make sure that the same asset account doesn't
                        // appear twice on the request.
                        //
                        std::set<OTIdentifier> setOfAccounts;
                        setOfAccounts.insert(
                            theRequestBasket.GetRequestAccountID());

                        bool bFoundSameAcctTwice = false;

                        for (int32_t i = 0; i < theRequestBasket.Count(); i++) {
                            BasketItem* item = theRequestBasket.At(i);
                            OT_ASSERT(nullptr != item);
                            std::set<OTIdentifier>::iterator it_account =
                                setOfAccounts.find(item->SUB_ACCOUNT_ID);

                            if (setOfAccounts.end() !=
                                it_account) // The account appears twice!!
                            {
                                const OTString strSubID(item->SUB_ACCOUNT_ID);
                                OTLog::vError("%s: Failed: Sub-account ID "
                                              "found TWICE on same basket "
                                              "exchange request: %s\n",
                                              __FUNCTION__, strSubID.Get());
                                bFoundSameAcctTwice = true;
                                break;
                            }
                            setOfAccounts.insert(item->SUB_ACCOUNT_ID);
                        }
                        if (!bFoundSameAcctTwice) // Let's do it!
                        {
                            // Loop through the request AND the actual basket
                            // TOGETHER...
                            for (int32_t i = 0; i < theBasket.Count(); i++) {
                                BasketItem* pBasketItem = theBasket.At(i);
                                BasketItem* pRequestItem =
                                    theRequestBasket.At(i); // we already know
                                                            // these are the
                                                            // same length

                                // if not equal
                                if (!(pBasketItem->SUB_CONTRACT_ID ==
                                      pRequestItem->SUB_CONTRACT_ID)) {
                                    OTLog::Error("Error: expected asset type "
                                                 "IDs to match in "
                                                 "Notary::"
                                                 "NotarizeExchangeBasket\n");
                                    bSuccess = false;
                                    break;
                                }
                                // if accounts are equal (should never happen --
                                // why would the user be trying to use the
                                // server's account as his own?)
                                // Furthermore, loading both at the same time,
                                // with same ID, then saving again, can screw up
                                // the balance.
                                //
                                else if (pBasketItem->SUB_ACCOUNT_ID ==
                                         pRequestItem->SUB_ACCOUNT_ID) {
                                    OTLog::Error("Error: VERY strange to have "
                                                 "these account ID's match. "
                                                 "Notary::"
                                                 "NotarizeExchangeBasket.\n");
                                    bSuccess = false;
                                    break;
                                }
                                else if (false ==
                                           server_->transactor_
                                               .verifyTransactionNumber(
                                                   theNym,
                                                   pRequestItem
                                                       ->lClosingTransactionNo)) {
                                    OTLog::Error(
                                        "Error: Basket sub-currency closing "
                                        "number didn't verify . "
                                        "Notary::NotarizeExchangeBasket.\n");
                                    bSuccess = false;
                                    break;
                                }
                                else // if equal
                                {
                                    bSuccess = true;

                                    // Load up the two accounts and perform the
                                    // exchange...
                                    OTAccount* pUserAcct =
                                        OTAccount::LoadExistingAccount(
                                            pRequestItem->SUB_ACCOUNT_ID,
                                            SERVER_ID);

                                    if (nullptr == pUserAcct) {
                                        OTLog::Error("ERROR loading a user's "
                                                     "asset account in "
                                                     "Notary::"
                                                     "NotarizeExchangeBasket"
                                                     "\n");
                                        bSuccess = false;
                                        break;
                                    }
                                    OTAccount* pServerAcct =
                                        OTAccount::LoadExistingAccount(
                                            pBasketItem->SUB_ACCOUNT_ID,
                                            SERVER_ID);

                                    if (nullptr == pServerAcct) {
                                        OTLog::Error("ERROR loading a basket "
                                                     "sub-account in "
                                                     "Notary::"
                                                     "NotarizeExchangeBasket"
                                                     "\n");
                                        bSuccess = false;
                                        break;
                                    }
                                    // Load up the inbox for the user's sub
                                    // account, so we can drop the receipt.
                                    //
                                    OTLedger* pSubInbox = pUserAcct->LoadInbox(
                                        server_->m_nymServer);

                                    if (nullptr == pSubInbox) {
                                        OTLog::Error("Error loading or "
                                                     "verifying sub-inbox in "
                                                     "Notary::"
                                                     "NotarizeExchangeBasket."
                                                     "\n");
                                        bSuccess = false;
                                        break;
                                    }

                                    // I'm preserving these points, to be
                                    // deleted at the end.
                                    // They won't be saved until after ALL
                                    // debits/credits were successful.
                                    // Once ALL exchanges are done, THEN it
                                    // loops through and saves / deletes
                                    // all the accounts.
                                    listUserAccounts.push_back(pUserAcct);
                                    listServerAccounts.push_back(pServerAcct);
                                    listInboxes.push_back(pSubInbox);

                                    // Do they verify?
                                    // I call VerifySignature here since
                                    // VerifyContractID was already called in
                                    // LoadExistingAccount().
                                    if (pUserAcct->GetAssetTypeID() !=
                                        pBasketItem->SUB_CONTRACT_ID) {
                                        OTLog::Error(
                                            "ERROR verifying asset type on a "
                                            "user's account in "
                                            "Notary::"
                                            "NotarizeExchangeBasket\n");
                                        bSuccess = false;
                                        break;
                                    }
                                    else if (!pUserAcct->VerifySignature(
                                                   server_->m_nymServer)) {
                                        OTLog::Error(
                                            "ERROR verifying signature on a "
                                            "user's asset account in "
                                            "Notary::"
                                            "NotarizeExchangeBasket\n");
                                        bSuccess = false;
                                        break;
                                    }
                                    else if (!pServerAcct->VerifySignature(
                                                   server_->m_nymServer)) {
                                        OTLog::Error(
                                            "ERROR verifying signature on a "
                                            "basket sub-account in "
                                            "Notary::"
                                            "NotarizeExchangeBasket\n");
                                        bSuccess = false;
                                        break;
                                    }
                                    else {
                                        // the amount being transferred between
                                        // these two accounts is the minimum
                                        // transfer amount
                                        // for the sub-account on the basket,
                                        // multiplied by
                                        lTransferAmount =
                                            (pBasketItem
                                                 ->lMinimumTransferAmount *
                                             theRequestBasket
                                                 .GetTransferMultiple());

                                        // user is performing exchange IN
                                        if (theRequestBasket
                                                .GetExchangingIn()) {
                                            if (pUserAcct->Debit(
                                                    lTransferAmount)) {
                                                if (pServerAcct->Credit(
                                                        lTransferAmount))
                                                    bSuccess = true;
                                                else { // the server credit
                                                       // failed.
                                                    OTLog::Error(
                                                        " Notary::"
                                                        "NotarizeExchangeBasket"
                                                        ": Failure crediting "
                                                        "server acct.\n");

                                                    // Since we debited the
                                                    // user's acct already,
                                                    // let's put that back.
                                                    if (false ==
                                                        pUserAcct->Credit(
                                                            lTransferAmount))
                                                        OTLog::Error(
                                                            " Notary::"
                                                            "NotarizeExchangeBa"
                                                            "sket: Failure "
                                                            "crediting back "
                                                            "user "
                                                            "account.\n");
                                                    bSuccess = false;
                                                    break;
                                                }
                                            }
                                            else {
                                                OTLog::Output(
                                                    0, "Notary::"
                                                       "NotarizeExchangeBasket:"
                                                       " Unable to Debit user "
                                                       "account.\n");
                                                bSuccess = false;
                                                break;
                                            }
                                        }
                                        else // user is peforming exchange OUT
                                        {
                                            if (pServerAcct->Debit(
                                                    lTransferAmount)) {
                                                if (pUserAcct->Credit(
                                                        lTransferAmount))
                                                    bSuccess = true;
                                                else { // the user credit
                                                       // failed.
                                                    OTLog::Error(
                                                        " Notary::"
                                                        "NotarizeExchangeBasket"
                                                        ": Failure crediting "
                                                        "user acct.\n");

                                                    // Since we debited the
                                                    // server's acct already,
                                                    // let's put that back.
                                                    if (false ==
                                                        pServerAcct->Credit(
                                                            lTransferAmount))
                                                        OTLog::Error(
                                                            " Notary::"
                                                            "NotarizeExchangeBa"
                                                            "sket: Failure "
                                                            "crediting back "
                                                            "server "
                                                            "account.\n");
                                                    bSuccess = false;
                                                    break;
                                                }
                                            }
                                            else {
                                                OTLog::Output(
                                                    0, " Notary::"
                                                       "NotarizeExchangeBasket:"
                                                       " Unable to Debit "
                                                       "server account.\n");
                                                bSuccess = false;
                                                break;
                                            }
                                        }
                                        // Drop the receipt -- accounts were
                                        // debited and credited properly.
                                        //
                                        if (bSuccess) { // need to be able to
                                                        // "roll back" if
                                                        // anything inside this
                                                        // block fails.
                                            // update: actually does pretty good
                                            // roll-back as it is. The debits
                                            // and credits
                                            // don't save unless everything is a
                                            // success.

                                            // Generate new transaction number
                                            // (for putting the basketReceipt in
                                            // the exchanger's inbox.)
                                            // todo check this generation for
                                            // failure (can it fail?)
                                            int64_t lNewTransactionNumber = 0;

                                            server_->transactor_
                                                .issueNextTransactionNumber(
                                                    server_->m_nymServer,
                                                    lNewTransactionNumber,
                                                    false);

                                            OTTransaction* pInboxTransaction =
                                                OTTransaction::
                                                    GenerateTransaction(
                                                        *pSubInbox,
                                                        OTTransaction::
                                                            basketReceipt,
                                                        lNewTransactionNumber);

                                            OTItem* pItemInbox = OTItem::
                                                CreateItemFromTransaction(
                                                    *pInboxTransaction,
                                                    OTItem::basketReceipt);

                                            // these may be unnecessary, I'll
                                            // have to check
                                            // CreateItemFromTransaction. I'll
                                            // leave em.
                                            OT_ASSERT(nullptr != pItemInbox);

                                            pItemInbox->SetStatus(
                                                OTItem::acknowledgement);
                                            pItemInbox->SetAmount(
                                                theRequestBasket
                                                        .GetExchangingIn()
                                                    ? lTransferAmount * (-1)
                                                    : lTransferAmount);

                                            pItemInbox->SignContract(
                                                server_->m_nymServer);
                                            pItemInbox->SaveContract();

                                            pInboxTransaction->AddItem(
                                                *pItemInbox); // Add the inbox
                                                              // item to the
                                                              // inbox
                                                              // transaction, so
                                                              // we can add to
                                                              // the inbox
                                                              // ledger.

                                            pInboxTransaction
                                                ->SetNumberOfOrigin(*pItem);

                                            // The "exchangeBasket request"
                                            // OTItem is saved as the "In
                                            // Reference To" field
                                            // on the inbox basketReceipt
                                            // transaction.
                                            // todo put these two together in a
                                            // method.
                                            pInboxTransaction
                                                ->SetReferenceString(
                                                    strInReferenceTo);
                                            pInboxTransaction
                                                ->SetReferenceToNum(
                                                    pItem->GetTransactionNum());
                                            // Here is the number the user
                                            // wishes
                                            // to sign-off by accepting this
                                            // receipt.
                                            pInboxTransaction->SetClosingNum(
                                                pRequestItem
                                                    ->lClosingTransactionNo);

                                            // Now we have created a new
                                            // transaction from the server to
                                            // the sender's inbox (for a
                                            // receipt).
                                            // Let's sign and save it...
                                            pInboxTransaction->SignContract(
                                                server_->m_nymServer);
                                            pInboxTransaction->SaveContract();

                                            // Here the transaction we just
                                            // created is actually added to the
                                            // exchanger's inbox.
                                            pSubInbox->AddTransaction(
                                                *pInboxTransaction);
                                            pInboxTransaction->SaveBoxReceipt(
                                                *pSubInbox);
                                        }
                                    } // User and Server sub-accounts are good.
                                }     // pBasketItem and pRequestItem are good.
                            }         // for (loop through basketitems)
                            // Load up the two main accounts and perform the
                            // exchange...
                            // (Above we did the sub-accounts for server and
                            // user. Now we do the main accounts for server and
                            // user.)
                            //

                            // At this point, if we have successfully debited /
                            // credited the sub-accounts.
                            // then we need to debit and credit the user's main
                            // basket account and the server's basket issuer
                            // account.
                            if ((true == bSuccess) &&
                                (nullptr != pBasketAcct)) {
                                lTransferAmount =
                                    (theRequestBasket.GetMinimumTransfer() *
                                     theRequestBasket.GetTransferMultiple());

                                // Load up the two accounts and perform the
                                // exchange...
                                // user is performing exchange IN
                                if (theRequestBasket.GetExchangingIn()) {
                                    if (pBasketAcct->Debit(lTransferAmount)) {
                                        if (theAccount.Credit(lTransferAmount))
                                            bSuccess = true;
                                        else {
                                            OTLog::Error("Notary::"
                                                         "NotarizeExchangeBaske"
                                                         "t: Failed crediting "
                                                         "user basket "
                                                         "account.\n");

                                            if (false ==
                                                pBasketAcct->Credit(
                                                    lTransferAmount))
                                                OTLog::Error(
                                                    "Notary::"
                                                    "NotarizeExchangeBasket: "
                                                    "Failed crediting back "
                                                    "basket issuer account.\n");

                                            bSuccess = false;
                                        }
                                    }
                                    else {
                                        bSuccess = false;
                                        OTLog::Output(
                                            0, "Unable to Debit basket issuer "
                                               "account, in "
                                               "Notary::"
                                               "NotarizeExchangeBasket\n");
                                    }
                                }
                                else // user is peforming exchange OUT
                                {
                                    if (theAccount.Debit(lTransferAmount)) {
                                        if (pBasketAcct->Credit(
                                                lTransferAmount))
                                            bSuccess = true;
                                        else {
                                            OTLog::Error("Notary::"
                                                         "NotarizeExchangeBaske"
                                                         "t: Failed crediting "
                                                         "basket issuer "
                                                         "account.\n");

                                            if (false ==
                                                theAccount.Credit(
                                                    lTransferAmount))
                                                OTLog::Error(
                                                    "Notary::"
                                                    "NotarizeExchangeBasket: "
                                                    "Failed crediting back "
                                                    "user basket account.\n");

                                            bSuccess = false;
                                        }
                                    }
                                    else {
                                        bSuccess = false;
                                        OTLog::Output(0, "Unable to Debit user "
                                                         "basket account in "
                                                         "Notary::"
                                                         "NotarizeExchangeBaske"
                                                         "t\n");
                                    }
                                }

                                // Drop the receipt -- accounts were debited and
                                // credited properly.
                                //
                                if (bSuccess) { // need to be able to "roll
                                                // back" if anything inside this
                                                // block fails.
                                    // update: actually does pretty good
                                    // roll-back as it is. The debits and
                                    // credits
                                    // don't save unless everything is a
                                    // success.

                                    // Generate new transaction number (for
                                    // putting the basketReceipt in the
                                    // exchanger's inbox.)
                                    // todo check this generation for failure
                                    // (can it fail?)
                                    int64_t lNewTransactionNumber = 0;

                                    server_->transactor_
                                        .issueNextTransactionNumber(
                                            server_->m_nymServer,
                                            lNewTransactionNumber,
                                            false); // bStoreTheNumber = false

                                    OTTransaction* pInboxTransaction =
                                        OTTransaction::GenerateTransaction(
                                            *pInbox,
                                            OTTransaction::basketReceipt,
                                            lNewTransactionNumber);

                                    OTItem* pItemInbox =
                                        OTItem::CreateItemFromTransaction(
                                            *pInboxTransaction,
                                            OTItem::basketReceipt);

                                    // these may be unnecessary, I'll have to
                                    // check CreateItemFromTransaction. I'll
                                    // leave em.
                                    OT_ASSERT(nullptr != pItemInbox);

                                    pItemInbox->SetStatus(
                                        OTItem::acknowledgement); // the
                                                                  // default.
                                    pItemInbox->SetAmount(
                                        theRequestBasket.GetExchangingIn()
                                            ? lTransferAmount
                                            : lTransferAmount * (-1));

                                    pItemInbox->SignContract(
                                        server_->m_nymServer);
                                    pItemInbox->SaveContract();

                                    pInboxTransaction->AddItem(
                                        *pItemInbox); // Add the inbox item to
                                                      // the inbox transaction,
                                                      // so we can add to the
                                                      // inbox ledger.

                                    pInboxTransaction->SetNumberOfOrigin(
                                        *pItem);

                                    // The exchangeBasket request OTItem is
                                    // saved as a "in reference to" field,
                                    // on the inbox basketReceipt transaction.
                                    // todo put these two together in a method.
                                    pInboxTransaction->SetReferenceString(
                                        strInReferenceTo);
                                    pInboxTransaction->SetReferenceToNum(
                                        pItem->GetTransactionNum());
                                    pInboxTransaction->SetClosingNum(
                                        theRequestBasket
                                            .GetClosingNum()); // So the
                                                               // exchanger can
                                                               // sign-off on
                                                               // this closing
                                                               // num by
                                                               // accepting the
                                                               // basket receipt
                                                               // on his main
                                                               // basket
                                                               // account.

                                    // Now we have created a new transaction
                                    // from the server to the sender's inbox
                                    // Let's sign and save it...
                                    pInboxTransaction->SignContract(
                                        server_->m_nymServer);
                                    pInboxTransaction->SaveContract();

                                    // Here the transaction we just created is
                                    // actually added to the source acct's
                                    // inbox.
                                    pInbox->AddTransaction(*pInboxTransaction);
                                    pInboxTransaction->SaveBoxReceipt(*pInbox);
                                }
                            }
                            else {
                                OTLog::Error("Error loading or verifying "
                                             "user's main basket account in "
                                             "Notary::"
                                             "NotarizeExchangeBasket\n");
                                bSuccess = false;
                            }

                            // At this point, we have hopefully credited/debited
                            // ALL the relevant accounts.
                            // So now, let's Save them ALL to disk.. (and clean
                            // them up.)
                            OTAccount* pAccount = nullptr;

                            // empty the list of USER accounts (and save to
                            // disk, if everything was successful.)
                            while (!listUserAccounts.empty()) {
                                pAccount = listUserAccounts.front();
                                if (nullptr == pAccount) OT_FAIL;
                                listUserAccounts.pop_front();

                                if (true == bSuccess) {
                                    pAccount->ReleaseSignatures();
                                    pAccount->SignContract(
                                        server_->m_nymServer);
                                    pAccount->SaveContract();
                                    pAccount->SaveAccount();
                                }

                                delete pAccount;
                                pAccount = nullptr;
                            }
                            // empty the list of SERVER accounts (and save to
                            // disk, if everything was successful.)
                            while (!listServerAccounts.empty()) {
                                pAccount = listServerAccounts.front();
                                if (nullptr == pAccount) OT_FAIL;

                                listServerAccounts.pop_front();

                                if (true == bSuccess) {
                                    pAccount->ReleaseSignatures();
                                    pAccount->SignContract(
                                        server_->m_nymServer);
                                    pAccount->SaveContract();
                                    pAccount->SaveAccount();
                                }

                                delete pAccount;
                                pAccount = nullptr;
                            }
                            // empty the list of inboxes (and save to disk, if
                            // everything was successful.)
                            while (!listInboxes.empty()) {
                                OTLedger* pTempInbox = listInboxes.front();
                                if (nullptr == pTempInbox) OT_FAIL;
                                listInboxes.pop_front();

                                if (true == bSuccess) {
                                    pTempInbox->ReleaseSignatures();
                                    pTempInbox->SignContract(
                                        server_->m_nymServer);
                                    pTempInbox->SaveContract();
                                    pTempInbox->SaveInbox();
                                }

                                delete pTempInbox;
                                pTempInbox = nullptr;
                            }
                            if (true == bSuccess) {
                                pInbox->ReleaseSignatures();
                                pInbox->SignContract(server_->m_nymServer);
                                pInbox->SaveContract();
                                theAccount.SaveInbox(*pInbox);

                                theAccount.ReleaseSignatures();
                                theAccount.SignContract(server_->m_nymServer);
                                theAccount.SaveContract();
                                theAccount.SaveAccount();

                                pBasketAcct->ReleaseSignatures();
                                pBasketAcct->SignContract(server_->m_nymServer);
                                pBasketAcct->SaveContract();
                                pBasketAcct->SaveAccount();

                                // Remove my ability to use the "closing"
                                // numbers in the future.
                                // (Since I'm using them to do this exchange...)
                                //
                                for (int32_t i = 0;
                                     i < theRequestBasket.Count(); i++) {
                                    BasketItem* pRequestItem =
                                        theRequestBasket.At(i);

                                    OT_ASSERT(nullptr != pRequestItem);

                                    // This just removes the number so I can't
                                    // USE it.
                                    // I'm still RESPONSIBLE for the number
                                    // until RemoveIssuedNumber() is called.
                                    //
                                    server_->transactor_
                                        .removeTransactionNumber(
                                            theNym,
                                            pRequestItem->lClosingTransactionNo,
                                            false);
                                }
                                server_->transactor_.removeTransactionNumber(
                                    theNym, theRequestBasket.GetClosingNum(),
                                    true);
                                pResponseItem->SetStatus(
                                    OTItem::acknowledgement); // the
                                                              // exchangeBasket
                                                              // was successful.

                                bOutSuccess =
                                    true; // The exchangeBasket was successful.
                            }
                        } // Let's do it!
                    }
                    else {
                        OTLog::Error(
                            "Error finding asset contract for basket, or "
                            "loading the basket from it, or verifying\n"
                            "the signature on that basket, or the request "
                            "basket didn't match actual basket.\n");
                    }
                } // pBasket exists and signature verifies
            }     // theRequestBasket loaded properly.
        }         // else (balance agreement verified.)
    }             // Balance Agreement item found.

    // I put this here so it's signed/saved whether the balance agreement itself
    // was successful OR NOT.
    pResponseItem->SignContract(server_->m_nymServer);
    pResponseItem->SaveContract();

    pResponseBalanceItem->SignContract(server_->m_nymServer);
    pResponseBalanceItem->SaveContract();
}

// DONE:  Make sure a CLOSING TRANSACTION number is provided, and recorded for
// use later in cron!

void Notary::NotarizeMarketOffer(OTPseudonym& theNym,
                                 OTAccount& theAssetAccount,
                                 OTTransaction& tranIn, OTTransaction& tranOut,
                                 bool& bOutSuccess)
{
    // The outgoing transaction is an "atMarketOffer", that is, "a reply to the
    // marketOffer request"
    tranOut.SetType(OTTransaction::atMarketOffer);

    OTItem* pItem = nullptr;
    OTItem* pBalanceItem = nullptr;
    OTItem* pResponseItem = nullptr;
    OTItem* pResponseBalanceItem = nullptr;

    // The incoming transaction may be sent to inboxes and outboxes, and it
    // will definitely be bundled in our reply to the user as well. Therefore,
    // let's grab it as a string.
    OTString strInReferenceTo;
    OTString strBalanceItem;

    // Grab the actual server ID from this object, and use it as the server ID
    // here.
    const OTIdentifier SERVER_ID(server_->m_strServerID), USER_ID(theNym);
    const OTString strUserID(USER_ID);

    pItem = tranIn.GetItem(OTItem::marketOffer);
    pBalanceItem = tranIn.GetItem(OTItem::transactionStatement);
    pResponseItem =
        OTItem::CreateItemFromTransaction(tranOut, OTItem::atMarketOffer);
    pResponseItem->SetStatus(OTItem::rejection); // the default.
    tranOut.AddItem(*pResponseItem); // the Transaction's destructor will
                                     // cleanup the item. It "owns" it now.

    pResponseBalanceItem = OTItem::CreateItemFromTransaction(
        tranOut, OTItem::atTransactionStatement);
    pResponseBalanceItem->SetStatus(OTItem::rejection); // the default.
    tranOut.AddItem(*pResponseBalanceItem); // the Transaction's destructor will
                                            // cleanup the item. It "owns" it
                                            // now.
    if (!NYM_IS_ALLOWED(strUserID.Get(),
                        ServerSettings::__transact_market_offer)) {
        OTLog::vOutput(
            0,
            "Notary::NotarizeMarketOffer: User %s cannot do this transaction "
            "(All market offers are disallowed in server.cfg)\n",
            strUserID.Get());
    }
    else if (nullptr == pBalanceItem) {
        OTString strTemp(tranIn);
        OTLog::vOutput(0, "Notary::NotarizeMarketOffer: Expected transaction "
                          "statement in trans # %lld: \n\n%s\n\n",
                       tranIn.GetTransactionNum(),
                       strTemp.Exists()
                           ? strTemp.Get()
                           : " (ERROR LOADING TRANSACTION INTO STRING) ");
    }
    else if (nullptr == pItem) {
        OTString strTemp(tranIn);
        OTLog::vOutput(0, "Notary::NotarizeMarketOffer: Expected "
                          "OTItem::marketOffer in trans# %lld:\n\n%s\n\n",
                       tranIn.GetTransactionNum(),
                       strTemp.Exists()
                           ? strTemp.Get()
                           : " (ERROR LOADING TRANSACTION INTO STRING) ");
    }
    // For now, there should only be one of these marketOffer items inside the
    // transaction.
    // So we treat it that way... I either get it successfully or not.
    else {
        // The response item will contain a copy of the request item. So I save
        // it into a string
        // here so it can be saved into the "in reference to" field.
        pItem->SaveContractRaw(strInReferenceTo);
        pBalanceItem->SaveContractRaw(strBalanceItem);

        // ASSET_ACCT_ID is the ID on the "from" Account that was passed in.
        // The CURRENCY_ACCT_ID is the ID on the "To" Account. (When doing a
        // transfer, normally 2nd acct is the Payee.)
        const OTIdentifier ASSET_ACCT_ID(theAssetAccount),
            CURRENCY_ACCT_ID(pItem->GetDestinationAcctID());

        // Server response item being added to server response transaction
        // (tranOut)
        // They're getting SOME sort of response item.

        pResponseItem->SetReferenceString(strInReferenceTo); // the response
                                                             // item carries a
                                                             // copy of what
                                                             // it's responding
                                                             // to.
        pResponseItem->SetReferenceToNum(
            pItem->GetTransactionNum()); // This response item is IN RESPONSE to
                                         // pItem and its Owner Transaction.

        pResponseBalanceItem->SetReferenceString(
            strBalanceItem); // the response item carries a copy of what it's
                             // responding to.
        pResponseBalanceItem->SetReferenceToNum(
            pItem->GetTransactionNum()); // This response item is IN RESPONSE to
                                         // pItem and its Owner Transaction.

        if (!(pBalanceItem->VerifyTransactionStatement(
                theNym, tranIn))) // bIsRealTransaction = true;
        {
            OTLog::vOutput(0, "ERROR verifying transaction statement in "
                              "NotarizeMarketOffer.\n");
        }
        else {
            pResponseBalanceItem->SetStatus(
                OTItem::acknowledgement); // the transaction agreement was
                                          // successful.

            // Load up the currency account and validate it.
            std::unique_ptr<OTAccount> pCurrencyAcct(
                OTAccount::LoadExistingAccount(CURRENCY_ACCT_ID, SERVER_ID));

            // Also load up the Trade from inside the transaction item.
            OTString strOffer;
            OTOffer theOffer;

            OTString strTrade;
            pItem->GetAttachment(strTrade);

            OTTrade* pTrade = new OTTrade();

            OT_ASSERT(nullptr != pTrade);

            // First load the Trade up (from the string that was passed in on
            // the transaction item.)
            bool bLoadContractFromString =
                pTrade->LoadContractFromString(strTrade);

            // If failed to load the trade...
            if (!bLoadContractFromString) {
                OTLog::vError("ERROR loading trade from string in "
                              "Notary::NotarizeMarketOffer:\n%s\n",
                              strTrade.Get());
            }
            // I'm using the operator== because it exists. (Although now I
            // believe != exists also)
            // If the ID on the "from" account that was passed in,
            // does not match the "Acct From" ID on this transaction item
            else if (!(ASSET_ACCT_ID == pItem->GetPurportedAccountID())) {
                OTLog::Output(0, "Error: Asset account ID on the transaction "
                                 "does not match asset account ID on the "
                                 "transaction item.\n");
            }
            // ok so the IDs match. Does the currency account exist?
            else if (nullptr == pCurrencyAcct) {
                OTLog::Output(0, "ERROR verifying existence of the currency "
                                 "account in Notary::NotarizeMarketOffer\n");
            }
            else if (!pCurrencyAcct->VerifyContractID()) {
                OTLog::Output(0, "ERROR verifying Contract ID on the currency "
                                 "account in Notary::NotarizeMarketOffer\n");
            }
            else if (!pCurrencyAcct->VerifyOwner(theNym)) {
                OTLog::Output(0, "ERROR verifying ownership of the currency "
                                 "account in Notary::NotarizeMarketOffer\n");
            }
            // Are both of the accounts of the same Asset Type?
            else if (theAssetAccount.GetAssetTypeID() ==
                     pCurrencyAcct->GetAssetTypeID()) {
                OTString strAssetTypeID(theAssetAccount.GetAssetTypeID()),
                    strCurrencyTypeID(pCurrencyAcct->GetAssetTypeID());
                OTLog::vOutput(
                    0, "ERROR - user attempted to trade between identical "
                       "asset types in Notary::NotarizeMarketOffer:\n%s\n%s\n",
                    strAssetTypeID.Get(), strCurrencyTypeID.Get());
            }
            // Does it verify?
            // I call VerifySignature here since VerifyContractID was already
            // called in LoadExistingAccount().
            else if (!pCurrencyAcct->VerifySignature(server_->m_nymServer)) {
                OTLog::Output(0, "ERROR verifying signature on the Currency "
                                 "account in Notary::NotarizeMarketOffer\n");
            }
            else if (!pTrade->VerifySignature(theNym)) {
                OTLog::Output(0, "ERROR verifying signature on the Trade in "
                                 "Notary::NotarizeMarketOffer\n");
            }
            else if (pTrade->GetTransactionNum() !=
                       pItem->GetTransactionNum()) {
                OTLog::Output(0, "ERROR bad transaction number on trade in "
                                 "Notary::NotarizeMarketOffer\n");
            }
            // The transaction number opens the market offer, but there must
            // also be a closing number for closing it.
            else if ((pTrade->GetCountClosingNumbers() < 2) ||
                     !server_->transactor_.verifyTransactionNumber(
                         theNym,
                         pTrade->GetAssetAcctClosingNum()) || // Verify that it
                                                              // can still be
                                                              // USED
                     !server_->transactor_.verifyTransactionNumber(
                         theNym, pTrade->GetCurrencyAcctClosingNum())) {
                OTLog::Output(0, "ERROR needed 2 valid closing transaction "
                                 "numbers in Notary::NotarizeMarketOffer\n");
            }
            else if (pTrade->GetServerID() != SERVER_ID) {
                const OTString strID1(pTrade->GetServerID()), strID2(SERVER_ID);
                OTLog::vOutput(0, "Notary::NotarizeMarketOffer: ERROR wrong "
                                  "Server ID (%s) on trade. Expected: %s\n",
                               strID1.Get(), strID2.Get());
            }
            else if (pTrade->GetSenderUserID() != USER_ID) {
                const OTString strID1(pTrade->GetSenderUserID()),
                    strID2(USER_ID);
                OTLog::vOutput(0, "Notary::NotarizeMarketOffer: ERROR wrong "
                                  "Nym ID (%s) on trade. Expected: %s\n",
                               strID1.Get(), strID2.Get());
            }
            else if (pTrade->GetAssetID() !=
                       theAssetAccount.GetAssetTypeID()) {
                const OTString strAssetID1(pTrade->GetAssetID()),
                    strAssetID2(theAssetAccount.GetAssetTypeID());
                OTLog::vOutput(0, "Notary::NotarizeMarketOffer: ERROR wrong "
                                  "Asset Type ID (%s) on trade. Expected: %s\n",
                               strAssetID1.Get(), strAssetID2.Get());
            }
            else if (pTrade->GetSenderAcctID() != ASSET_ACCT_ID) {
                const OTString strAcctID1(pTrade->GetSenderAcctID()),
                    strAcctID2(ASSET_ACCT_ID);
                OTLog::vOutput(0, "Notary::NotarizeMarketOffer: ERROR wrong "
                                  "asset Acct ID (%s) on trade. Expected: %s\n",
                               strAcctID1.Get(), strAcctID2.Get());
            }
            else if (pTrade->GetCurrencyID() !=
                       pCurrencyAcct->GetAssetTypeID()) {
                const OTString strID1(pTrade->GetCurrencyID()),
                    strID2(pCurrencyAcct->GetAssetTypeID());
                OTLog::vOutput(0, "Notary::NotarizeMarketOffer: ERROR wrong "
                                  "Currency Type ID (%s) on trade. Expected: "
                                  "%s\n",
                               strID1.Get(), strID2.Get());
            }
            else if (pTrade->GetCurrencyAcctID() != CURRENCY_ACCT_ID) {
                const OTString strID1(pTrade->GetCurrencyAcctID()),
                    strID2(CURRENCY_ACCT_ID);
                OTLog::vOutput(0, "Notary::NotarizeMarketOffer: ERROR wrong "
                                  "Currency Acct ID (%s) on trade. Expected: "
                                  "%s\n",
                               strID1.Get(), strID2.Get());
            }
            // If the Trade successfully verified, but I couldn't get the offer
            // out of it, then it
            // actually DIDN'T successfully load still.  :-(
            else if (!pTrade->GetOfferString(strOffer)) {
                OTLog::vError("ERROR getting offer string from trade in "
                              "Notary::NotarizeMarketOffer:\n%s\n",
                              strTrade.Get());
            }
            else if (!theOffer.LoadContractFromString(strOffer)) {
                OTLog::vError("ERROR loading offer from string in "
                              "Notary::NotarizeMarketOffer:\n%s\n",
                              strTrade.Get());
            }
            // ...And then we use that same Nym to verify the signature on the
            // offer.
            else if (!theOffer.VerifySignature(theNym)) {
                OTLog::Error("ERROR verifying offer signature in "
                             "Notary::NotarizeMarketOffer.\n");
            }
            else if (!pTrade->VerifyOffer(theOffer)) {
                OTLog::Output(0, "FAILED verifying offer for Trade in "
                                 "Notary::NotarizeMarketOffer\n");
            }
            else if (theOffer.GetScale() <
                       ServerSettings::GetMinMarketScale()) {
                OTLog::vOutput(0, "Notary::NotarizeMarketOffer: FAILED "
                                  "verifying Offer, SCALE: %lld. (Minimum is "
                                  "%lld.) \n",
                               theOffer.GetScale(),
                               ServerSettings::GetMinMarketScale());
            }
            else if (static_cast<int64_t>(
                           (theNym.GetSetOpenCronItems().size() / 3)) >=
                       OTCron::GetCronMaxItemsPerNym()) {
                // NOTE:
                // We divided by 3 since this set contains THREE numbers for
                // each active market offer.
                // It's kind of a hack, since it may NOT be three numbers for
                // other cron items such as
                // payment plans and smart contracts. But it's a good enough
                // approximation for now.
                //
                OTLog::Output(0, "Notary::NotarizeMarketOffer: FAILED adding "
                                 "offer to market: "
                                 "NYM HAS TOO MANY ACTIVE OFFERS ALREADY. See "
                                 "'max_items_per_nym' setting in the config "
                                 "file.\n");
            }
            // At this point I feel pretty confident that the Trade is a valid
            // request from the user.
            // The top half of this function is oriented around finding the
            // "marketOffer" item (in the "marketOffer"
            // transaction) and setting up the response item that will go into
            // the response transaction. It also
            // retrieves the Trade object and fully validates it.
            //
            // Next all we need to do is add it to the market...
            else {
                // We don't actually add the trade to a market here. Instead, we
                // add it to the server's Cron object.
                // That object will take care of processing the offer on and off
                // of any market.
                //
                // NOTE: FYI, inside AddCronItem, since this is a new CronItem,
                // a Cron Receipt will
                // be saved with the User's signature on it, containing the Cron
                // Item from the user's
                // original request. After that, the item is stored internally
                // to Cron itself, and
                // signed by the server--and changes over time as cron
                // processes. (The original receipt
                // can always be loaded when necessary.)
                //
                if (server_->m_Cron.AddCronItem(
                        *pTrade, &theNym, true,
                        OTTimeGetCurrentTime())) // bSaveReceipt=true
                {
                    // todo need to be able to "roll back" if anything inside
                    // this block fails.

                    // Now we can set the response item as an acknowledgement
                    // instead of the default (rejection)
                    pResponseItem->SetStatus(OTItem::acknowledgement);

                    bOutSuccess = true; // The offer was successfully placed on
                                        // the market.

                    OTLog::Output(2,
                                  "Successfully added Trade to Cron object.\n");

                    // Server side, the Nym stores a list of all open cron item
                    // numbers.
                    // (So we know if there is still stuff open on Cron for that
                    // Nym, and we know what it is.)
                    //
                    std::set<int64_t>& theIDSet = theNym.GetSetOpenCronItems();
                    theIDSet.insert(pTrade->GetTransactionNum());
                    theIDSet.insert(pTrade->GetAssetAcctClosingNum());
                    theIDSet.insert(pTrade->GetCurrencyAcctClosingNum());
                    // This just removes the Closing number so he can't USE it
                    // again. (Since he's using it as the closing
                    // number for this cron item now.) He's still RESPONSIBLE
                    // for the number until RemoveIssuedNumber()
                    // is called. If we didn't call this here, then he could
                    // come back later and USE THE NUMBER AGAIN!
                    // (Bad!) You might ask, why not remove the Opening number
                    // as well as the Closing numbers? The answer
                    // is, we already did, before we got here. (Otherwise we
                    // wouldn't have even gotten this far.)
                    //
                    server_->transactor_.removeTransactionNumber(
                        theNym, pTrade->GetAssetAcctClosingNum(), false);
                    server_->transactor_.removeTransactionNumber(
                        theNym, pTrade->GetCurrencyAcctClosingNum(),
                        false); // (Saved below.)
                    // RemoveIssuedNum will be called for the original
                    // transaction number when the finalReceipt is created.
                    // RemoveIssuedNum will be called for the Closing number
                    // when the finalReceipt is accepted.

                    theNym.SaveSignedNymfile(
                        server_->m_nymServer); // <===== SAVED HERE.
                }
                else {
                    OTLog::Output(0, "Unable to add trade to Cron object "
                                     "Notary::NotarizeMarketOffer\n");
                }
            }

            // If the trade WAS successfully added to Cron, then we don't need
            // to
            // delete it here, since Cron owns it now, and will deal with
            // cleaning
            // it up at the right time.
            if ((nullptr != pTrade) &&
                pResponseItem->GetStatus() != OTItem::acknowledgement) {
                delete pTrade;
                pTrade = nullptr;
            }
        } // transaction statement verified.
    }

    // sign the response item before sending it back (it's already been added to
    // the transaction above)
    // Now, whether it was rejection or acknowledgement, it is set properly and
    // it is signed, and it
    // is owned by the transaction, who will take it from here.
    pResponseItem->SignContract(server_->m_nymServer);
    pResponseItem->SaveContract(); // the signing was of no effect because I
                                   // forgot to save. (fixed.)

    pResponseBalanceItem->SignContract(server_->m_nymServer);
    pResponseBalanceItem->SaveContract();
}

/// If the server receives a notarizeTransactions command, it will be
/// accompanied by a payload
/// containing a ledger to be notarized.  UserCmdNotarizeTransactions will loop
/// through that ledger,
/// and for each transaction within, it calls THIS method.
/// TODO think about error reporting here and sending a message back to user.
void Notary::NotarizeTransaction(OTPseudonym& theNym, OTTransaction& tranIn,
                                 OTTransaction& tranOut, bool& bOutSuccess)
{
    const int64_t lTransactionNumber = tranIn.GetTransactionNum();
    const OTIdentifier SERVER_ID(server_->m_strServerID);
    OTIdentifier USER_ID;
    theNym.GetIdentifier(USER_ID);
    const OTString strIDNym(USER_ID);

    OTAccount theFromAccount(USER_ID, tranIn.GetPurportedAccountID(),
                             SERVER_ID);

    // Make sure the "from" account even exists...
    if (!theFromAccount.LoadContract()) {
        const OTString strIDAcct(tranIn.GetPurportedAccountID());
        OTLog::vOutput(0, "%s: Error loading asset account: %s\n", __FUNCTION__,
                       strIDAcct.Get());
    }
    // Make sure the "from" account isn't marked for deletion.
    else if (theFromAccount.IsMarkedForDeletion()) {
        const OTString strIDAcct(tranIn.GetPurportedAccountID());
        OTLog::vOutput(0, "%s: Failed attempt to use an Asset account that was "
                          "marked for deletion: %s\n",
                       __FUNCTION__, strIDAcct.Get());
    }
    // Make sure the Account ID loaded from the file matches the one we just set
    // and used as the filename.
    else if (!theFromAccount.VerifyContractID()) {
        // this should never happen. How did the wrong ID get into the account
        // file, if the right
        // ID is on the filename itself? and vice versa.
        const OTString strIDAcct(tranIn.GetPurportedAccountID());
        OTLog::vError("%s: Error verifying account ID: %s\n", __FUNCTION__,
                      strIDAcct.Get());
    }
    // Make sure the nymID loaded up in the account as its actual owner matches
    // the nym who was
    // passed in to this function requesting a transaction on this account...
    // otherwise any asshole
    // could do transactions on your account, no?
    else if (!theFromAccount.VerifyOwner(theNym)) {
        const OTIdentifier idAcct(theFromAccount);
        const OTString strIDAcct(idAcct);
        OTLog::vOutput(
            0, "%s: Error verifying account ownership... Nym: %s  Acct: %s\n",
            __FUNCTION__, strIDNym.Get(), strIDAcct.Get());
    }
    // Make sure I, the server, have signed this file.
    else if (!theFromAccount.VerifySignature(server_->m_nymServer)) {
        const OTIdentifier idAcct(theFromAccount);
        const OTString strIDAcct(idAcct);
        OTLog::vError(
            "%s: Error verifying server signature on account: %s for Nym: %s\n",
            __FUNCTION__, strIDAcct.Get(), strIDNym.Get());
    }
    // No need to call VerifyAccount() here since the above calls go above and
    // beyond that method.
    else if (!server_->transactor_.verifyTransactionNumber(
                 theNym, lTransactionNumber)) {
        const OTIdentifier idAcct(theFromAccount);
        const OTString strIDAcct(idAcct);
        // The user may not submit a transaction using a number he's already
        // used before.
        OTLog::vOutput(0, "%s: Error verifying transaction number %lld on user "
                          "Nym: %s Account: %s\n",
                       __FUNCTION__, lTransactionNumber, strIDNym.Get(),
                       strIDAcct.Get());
    }

    // The items' acct and server ID were already checked in VerifyContractID()
    // when they were loaded.
    // Now this checks a little deeper, to verify ownership, signatures, and
    // transaction number
    // on each item.  That way those things don't have to be checked for
    // security over and over
    // again in the subsequent calls.
    //
    else if (!tranIn.VerifyItems(theNym)) {
        const OTIdentifier idAcct(theFromAccount);
        const OTString strIDAcct(idAcct);
        OTLog::vOutput(0, "%s: Error verifying transaction items. Trans: %lld "
                          "Nym: %s  Account: %s\n",
                       __FUNCTION__, lTransactionNumber, strIDNym.Get(),
                       strIDAcct.Get());
    }

    // any other security stuff?
    // Todo do I need to verify the server ID here as well?
    else {
        // We don't want any transaction number being used twice.
        // (The number, at this point, is STILL issued to the user, who is still
        // responsible
        // for that number and must continue signing for it. All this means here
        // is that the
        // user no longer has the number on his AVAILABLE list. Removal from
        // issued list happens separately.)
        //
        if (false ==
            server_->transactor_.removeTransactionNumber(
                theNym, lTransactionNumber, true)) // bSave=true
        {
            OTLog::Error("Error removing transaction number (as available) "
                         "from user nym in Notary::NotarizeTransaction\n");
        }
        else {
            OTItem::itemType theReplyItemType = OTItem::error_state;

            switch (tranIn.GetType()) {
            // TRANSFER (account to account)
            // Alice sends a signed request to the server asking it to
            // transfer from her account ABC to the inbox of account DEF.
            // A copy will also remain in her outbox until canceled or accepted.
            case OTTransaction::transfer:
                OTLog::Output(0, "NotarizeTransaction type: Transfer\n");
                NotarizeTransfer(theNym, theFromAccount, tranIn, tranOut,
                                 bOutSuccess);
                theReplyItemType = OTItem::atTransfer;
                break;

            // PROCESS INBOX (currently, all incoming transfers must be
            // accepted.)
            // Bob sends a signed request to the server asking it to reject
            // some of his inbox items and/or accept some into his account DEF.
            case OTTransaction::processInbox:
                OTLog::Output(0, "NotarizeTransaction type: Process Inbox\n");
                NotarizeProcessInbox(theNym, theFromAccount, tranIn, tranOut,
                                     bOutSuccess);
                //                    theReplyItemType = OTItem::atProcessInbox;
                // // Nonexistent, and here, unused.
                // (There is a processInbox message that carries that
                // transaction...)
                break;

            // WITHDRAWAL (cash or voucher)
            // Alice sends a signed request to the server asking it to debit her
            // account ABC and then issue her a purse full of blinded cash
            // tokens
            // --OR-- a voucher (a cashier's cheque, made out to any recipient's
            // User ID, or made out to a blank recipient, just like a blank
            // cheque.)
            case OTTransaction::withdrawal: {
                OTItem* pItemVoucher = tranIn.GetItem(OTItem::withdrawVoucher);
                OTItem* pItemCash = tranIn.GetItem(OTItem::withdrawal);

                if (nullptr != pItemCash) {
                    theReplyItemType = OTItem::atWithdrawal;
                    OTLog::Output(
                        0, "NotarizeTransaction type: Withdrawal (cash)\n");
                }
                else if (nullptr != pItemVoucher) {
                    theReplyItemType = OTItem::atWithdrawVoucher;
                    OTLog::Output(
                        0, "NotarizeTransaction type: Withdrawal (voucher)\n");
                }
                NotarizeWithdrawal(theNym, theFromAccount, tranIn, tranOut,
                                   bOutSuccess);
            } break;

            // DEPOSIT    (cash or cheque)
            // Bob sends a signed request to the server asking it to deposit
            // into his
            // account ABC. He includes with his request a signed cheque made
            // out to
            // Bob's user ID (or blank), --OR-- a purse full of tokens.
            case OTTransaction::deposit:
                OTLog::Output(0, "NotarizeTransaction type: Deposit\n");
                NotarizeDeposit(theNym, theFromAccount, tranIn, tranOut,
                                bOutSuccess);
                theReplyItemType = OTItem::atDeposit;
                break;

            // PAY DIVIDEND
            // Bob sends a signed request to the server asking it to pay all
            // shareholders
            // of a given asset type at the rate of $X per share, where X and $
            // are both
            // configurable.
            case OTTransaction::payDividend:
                OTLog::Output(0, "NotarizeTransaction type: Pay Dividend\n");
                NotarizePayDividend(theNym, theFromAccount, tranIn, tranOut,
                                    bOutSuccess);
                theReplyItemType = OTItem::atPayDividend;
                break;

            // MARKET OFFER
            // Bob sends a signed request to the server asking it to put an
            // offer
            // on the market. He includes with his request a signed trade
            // listing
            // the relevant information, asset types and account IDs.
            case OTTransaction::marketOffer:
                OTLog::Output(0, "NotarizeTransaction type: Market Offer\n");
                NotarizeMarketOffer(theNym, theFromAccount, tranIn, tranOut,
                                    bOutSuccess);
                theReplyItemType = OTItem::atMarketOffer;
                break;

            // PAYMENT PLAN
            // Bob sends a signed request to the server asking it to make
            // regular
            // payments to Alice. (BOTH Alice AND Bob must have signed the same
            // contract.)
            case OTTransaction::paymentPlan:
                OTLog::Output(0, "NotarizeTransaction type: Payment Plan\n");
                NotarizePaymentPlan(theNym, theFromAccount, tranIn, tranOut,
                                    bOutSuccess);
                theReplyItemType = OTItem::atPaymentPlan;
                break;

            // SMART CONTRACT
            // Bob sends a signed request to the server asking it to activate a
            // smart contract.
            // Bob is the authorizing agent for one of the parties, all of whom
            // have signed it,
            // and have provided transaction #s for it.
            case OTTransaction::smartContract: {
                OTLog::Output(0, "NotarizeTransaction type: Smart Contract\n");

                // For all transaction numbers used on cron items, we keep track
                // of them in
                // the GetSetOpenCronItems. This will be removed again below, if
                // the transaction
                // fails.
                std::set<int64_t>& theIDSet = theNym.GetSetOpenCronItems();
                theIDSet.insert(lTransactionNumber);
                NotarizeSmartContract(theNym, theFromAccount, tranIn, tranOut,
                                      bOutSuccess);
                theReplyItemType = OTItem::atSmartContract;
            } break;

            // CANCEL CRON ITEM
            // (Cron items: market offers, payment plans...)
            // Bob sends a signed request to the server asking it to cancel a
            // REGULARLY PROCESSING CONTRACT that he had previously created.
            case OTTransaction::cancelCronItem:
                OTLog::Output(0, "NotarizeTransaction type: cancelCronItem\n");
                NotarizeCancelCronItem(theNym, theFromAccount, tranIn, tranOut,
                                       bOutSuccess);
                theReplyItemType = OTItem::atCancelCronItem;
                break;

            // EXCHANGE BASKET
            // Bob sends a signed request to the server asking it to exchange
            // funds
            // in or out of a basket currency. (From-or-to his main basket
            // account and his
            // various sub-accounts for each member currency in the basket.)
            case OTTransaction::exchangeBasket:
                OTLog::Output(0, "NotarizeTransaction type: Exchange Basket\n");
                NotarizeExchangeBasket(theNym, theFromAccount, tranIn, tranOut,
                                       bOutSuccess);
                theReplyItemType = OTItem::atExchangeBasket;
                break;

            default:
                OTLog::vError("%s: Error, unexpected type: %s\n", __FUNCTION__,
                              tranIn.GetTypeString());
                break;
            }

            // Where appropriate, remove a transaction number from my issued
            // list
            // (the list of numbers I must sign for in every balance agreement.)
            bool bIsCronItem = false;

            switch (tranIn.GetType()) {
            case OTTransaction::marketOffer:
            case OTTransaction::paymentPlan:
            case OTTransaction::smartContract:
                bIsCronItem = true; // Falls through...

            case OTTransaction::transfer:
                // If success, then Issued number stays on Nym's issued list
                // until the transfer, paymentPlan, marketOffer, or smart
                // contract is entirely closed and removed. In the case of
                // transfer, that's when the transfer receipt is accepted.
                // In the case of markets and paymentplans, that's when they've
                // been entirely removed from Cron (many
                // intermediary receipts might occur before that happens.) At
                // that time, a final receipt is issued with
                // a closing transaction number (to make sure the user closes
                // all of the related market receipts.)
                //
                // But if failure, then Issued number is immediately removed.
                // (It already can't be used again, and there's no receipt to
                // clear later, thus no reason to save it...)
                {
                    OTItem* pItem = tranOut.GetItem(theReplyItemType);

                    if ((nullptr != pItem)) {
                        if (OTItem::rejection == pItem->GetStatus()) {
                            // If this is a cron item, then we need to remove it
                            // from the
                            // list of open cron items as well.
                            if (bIsCronItem) {
                                std::set<int64_t>& theIDSet =
                                    theNym.GetSetOpenCronItems();
                                std::set<int64_t>::iterator theSetIT =
                                    theIDSet.find(lTransactionNumber);
                                if (theSetIT != theIDSet.end()) // Found it.
                                    theIDSet.erase(lTransactionNumber);
                            }
                            if (!server_->transactor_.removeIssuedNumber(
                                    theNym, lTransactionNumber,
                                    true)) // bSave=true
                            {
                                const OTString strNymID(USER_ID);
                                OTLog::vError("%s: Error removing issued "
                                              "number %lld from user nym: %s\n",
                                              __FUNCTION__, lTransactionNumber,
                                              strNymID.Get());
                            }
                        }
                    }
                }
                break;
            // In the case of the below transaction types, the transaction
            // number is removed from the Nym's
            // issued list SUCCESS OR FAIL. (It's closed either way.)
            //
            case OTTransaction::processInbox:
            case OTTransaction::payDividend:
            case OTTransaction::withdrawal:
            case OTTransaction::deposit:
            case OTTransaction::cancelCronItem:
            case OTTransaction::exchangeBasket:
                if (!server_->transactor_.removeIssuedNumber(
                        theNym, lTransactionNumber, true)) // bSave=true
                {
                    const OTString strNymID(USER_ID);
                    OTLog::vError("%s: Error removing issued number %lld from "
                                  "user nym: %s\n",
                                  __FUNCTION__, lTransactionNumber,
                                  strNymID.Get());
                }
                break;
            default:
                OTLog::vError("%s: Error, unexpected type: %s\n", __FUNCTION__,
                              tranIn.GetTypeString());
                break;
            }
        }
    }

    // sign the outoing transaction
    tranOut.SignContract(server_->m_nymServer);
    tranOut.SaveContract(); // don't forget to save (to internal raw file
                            // member)

    // Contracts store an internal member that contains the "Raw File" contents
    // That is, the unsigned XML portion, plus the signatures, attached in a
    // standard
    // PGP-compatible format. It's not enough to sign it, you must also save it
    // into
    // that Raw file member variable (using SaveContract) and then you must
    // sometimes
    // THEN save it into a file (or a string or wherever you want to put it.)
}

/// The client may send multiple transactions in the ledger when he calls
/// processNymbox.
/// This function will be called for each of those.  Each processNymbox
/// transaction may
/// contain multiple items accepting or rejecting certain transactions. The
/// server
/// acknowledges and notarizes those transactions accordingly.
/// (And each of those transactions must be accepted or rejected in whole.)
//
// The processNymbox TRANSACTION has a series of TRANSACTION ITEMS. One is the
// transaction
// statement (which is like a balance agreement, except there's no balance,
// since there's no
// asset account.) The rest are *items* IN REFERENCE TO some *transaction* in my
// Nymbox (signing
// to accept it.) At this point you can't really reject Nymbox receipts, just
// like you can't
// reject inbox receipts. Why not? Haven't coded it yet. So your items on your
// processNymbox
// transaction can only accept things (notices, new transaction numbers,
void Notary::NotarizeProcessNymbox(OTPseudonym& theNym, OTTransaction& tranIn,
                                   OTTransaction& tranOut, bool& bOutSuccess)
{
    // The outgoing transaction is an "atProcessNymbox", that is, "a reply to
    // the process nymbox request"
    tranOut.SetType(OTTransaction::atProcessNymbox);
    OTItem* pItem = nullptr;
    OTItem* pBalanceItem = tranIn.GetItem(OTItem::transactionStatement);
    OTItem* pResponseItem = nullptr;
    OTItem* pResponseBalanceItem = nullptr;

    // Grab the actual server ID from this object, and use it as the server ID
    // here.
    const OTIdentifier SERVER_ID(server_->m_strServerID), USER_ID(theNym);
    OTPseudonym theTempNym;

    OTLedger theNymbox(USER_ID, USER_ID, SERVER_ID);
    OTString strNymID(USER_ID);

    bool bSuccessLoadingNymbox = theNymbox.LoadNymbox();

    if (true == bSuccessLoadingNymbox)
        bSuccessLoadingNymbox = theNymbox.VerifyAccount(
            server_->m_nymServer); // make sure it's all good.
    pResponseBalanceItem = OTItem::CreateItemFromTransaction(
        tranOut, OTItem::atTransactionStatement);
    pResponseBalanceItem->SetStatus(OTItem::rejection); // the default.
    tranOut.AddItem(*pResponseBalanceItem); // the Transaction's destructor will
                                            // cleanup the item. It "owns" it
                                            // now.
    bool bNymboxHashRegenerated = false;
    OTIdentifier NYMBOX_HASH; // In case the Nymbox hash is updated, we will
                              // have the updated version here.
    if (!bSuccessLoadingNymbox) {
        OTLog::vOutput(
            0, "Notary::%s: Failed loading or verifying Nymbox for user:\n%s\n",
            __FUNCTION__, strNymID.Get());
    }
    else if (nullptr == pBalanceItem) {
        const OTString strTransaction(tranIn);
        OTLog::vOutput(0, "Notary::%s: No Transaction Agreement item found "
                          "on this transaction %lld (required):\n\n%s\n\n",
                       __FUNCTION__, tranIn.GetTransactionNum(),
                       strTransaction.Get());
    }
    else {
        OTString strBalanceItem;

        pBalanceItem->SaveContractRaw(strBalanceItem);

        pResponseBalanceItem->SetReferenceString(
            strBalanceItem); // the response item carries a copy of what it's
                             // responding to.
        pResponseBalanceItem->SetReferenceToNum(
            pBalanceItem->GetTransactionNum()); // This response item is IN
                                                // RESPONSE to tranIn's balance
                                                // agreement

        // The incoming transaction accepts various messages and transaction
        // numbers.
        // So when it's all finished, my list of transaction numbers will be
        // higher.
        //
        // I would like to not even process the whole giant loop below,
        // if I can verify here now that the transaction agreement is wrong.
        //
        // Thus I will actually loop through the acceptTransaction items in
        // tranIn, and then for each one, I'll
        // lookup the ACTUAL transaction in the nymbox, and get its ACTUAL
        // value. (And store them all up on a temp nym.)
        //
        // The ones being accepted will therefore be added to my Nym, so the
        // Transaction Statement will be signed
        // as if that is already the case. (So they'll match.)
        //
        // I need to add them all to the Nym, verify the transaction statement,
        // and then remove them again.
        // (which is why I stored them on a temp Nym :-) Then if it succeeds for
        // real, at the bottom of this function,
        // I'll go ahead and add them properly (so it adds them to both lists.)

        bool bSuccessFindingAllTransactions = true;

        for (auto& it : tranIn.GetItemList()) {
            pItem = it;
            OT_ASSERT_MSG(nullptr != pItem,
                          "Pointer should not have been nullptr.");

            if (pItem->GetType() == OTItem::acceptTransaction) {
                OTTransaction* pTransaction =
                    theNymbox.GetTransaction(pItem->GetReferenceToNum());

                if ((nullptr != pTransaction) &&
                    (pTransaction->GetType() ==
                     OTTransaction::blank)) // The user is referencing a blank
                                            // in the nymbox, which indeed is
                                            // actually there.
                {
                    bSuccessFindingAllTransactions = true;

                    OTNumList listNumbersNymbox, listNumbersUserItem;

                    pItem->GetNumList(listNumbersUserItem);
                    pTransaction->GetNumList(listNumbersNymbox);

                    // MAKE SURE THEY MATCH. (Otherwise user could be signing
                    // numbers that differ from the
                    // actual ones in the Nymbox.)
                    //
                    if (!listNumbersNymbox.Verify(listNumbersUserItem))
                        OTLog::Error(
                            "Notary::NotarizeProcessNymbox: Failed "
                            "verifying: The numbers on the actual blank "
                            "transaction in the nymbox do not match the list "
                            "of numbers sent over by the user.\n");
                    else // INSTEAD of merely adding the TRANSACTION NUMBER of
                         // the blank to the Nym,
                    {    // we actually add an entire list of numbers retrieved
                         // from the blank, including
                        // its main number.
                        std::set<int64_t> theNumbers;
                        listNumbersNymbox.Output(theNumbers);

                        // Looping through the transaction numbers on the Nymbox
                        // blank transaction.
                        // (There's probably 20 of them.)
                        //
                        for (const auto& lTransactionNumber : theNumbers) {
                            // (We don't add it if it's already there.)
                            //
                            if (false ==
                                theNym.VerifyIssuedNum(server_->m_strServerID,
                                                       lTransactionNumber)) {
                                theNym.AddIssuedNum(server_->m_strServerID,
                                                    lTransactionNumber);
                                theTempNym.AddIssuedNum(
                                    server_->m_strServerID,
                                    lTransactionNumber); // so I can remove from
                                                         // theNym after
                                // VerifyTransactionStatement
                                // call
                            }
                            else
                                OTLog::vError("Notary::NotarizeProcessNymbox:"
                                              " tried to add an issued trans# "
                                              "(%lld) to a nym who "
                                              "ALREADY had that number...\n",
                                              lTransactionNumber);
                        }
                    }
                }
                else {
                    bSuccessFindingAllTransactions = false;
                    break;
                }
            }
        }

        // NOTICE: We're adding up all the new transaction numbers being added.
        // (OTItem::acceptTransaction)...
        // but we're NOT bothering with the ones being REMOVED
        // (OTItem::acceptFinalReceipt) here in NotarizeProecessNymbox.
        // Why not? BECAUSE THEY WERE ALREADY REMOVED. They were removed when
        // the Cron Item expired, or was canceled.
        // The finalReceipt notice that went into the Nymbox was ONLY A COURTESY
        // -- the NUMBER was ALREADY REMOVED.
        // Thus, we don't need to remove it now, although we DO need to add the
        // new transaction numbers (acceptTransaction).
        //
        // (Of course, I will still remove the finalReceipt from the Nymbox. I
        // just don't have to juggle any
        // transaction numbers on the NYM as a result of this.)
        //

        if (!bSuccessFindingAllTransactions) {
            OTLog::vOutput(0, "%s: transactions in processNymbox message do "
                              "not match actual nymbox.\n",
                           __FUNCTION__);

            // Remove all issued nums from theNym that are stored on theTempNym
            // HERE.
            for (int32_t i = 0; i < theTempNym.GetIssuedNumCount(SERVER_ID);
                 i++) {
                int64_t lTemp = theTempNym.GetIssuedNum(SERVER_ID, i);
                theNym.RemoveIssuedNum(server_->m_strServerID, lTemp);
            }
        }
        // (else true == success finding all transaction...)
        //
        // VERIFY TRANSACTION STATEMENT!
        //
        else if (false ==
                 pBalanceItem->VerifyTransactionStatement(
                     theNym, tranIn, false)) // bIsRealTransaction=false (since
                                             // we're doing Nymbox) // <========
        {
            OTLog::vOutput(0, "%s: ERROR verifying transaction statement.\n",
                           __FUNCTION__);

            // Remove all issued nums from theNym that are stored on theTempNym
            // HERE.
            for (int32_t i = 0; i < theTempNym.GetIssuedNumCount(SERVER_ID);
                 i++) {
                int64_t lTemp = theTempNym.GetIssuedNum(SERVER_ID, i);
                theNym.RemoveIssuedNum(server_->m_strServerID, lTemp);
            }
        }
        else // TRANSACTION AGREEMENT WAS SUCCESSFUL.......
        {
            // Remove all issued nums from theNym that are stored on theTempNym
            // HERE.
            for (int32_t i = 0; i < theTempNym.GetIssuedNumCount(SERVER_ID);
                 i++) {
                int64_t lTemp = theTempNym.GetIssuedNum(SERVER_ID, i);
                theNym.RemoveIssuedNum(server_->m_strServerID, lTemp);
            }

            pResponseBalanceItem->SetStatus(
                OTItem::acknowledgement); // the transaction statement was
                                          // successful.

            // THE ABOVE LOOP WAS JUST A TEST RUN
            //
            // (TO **VERIFY TRANSACTION STATEMENT** BEFORE WE BOTHERED TO RUN
            // THIS LOOP BELOW...)
            // (AND ALSO SO WE COULD GET THE LIST OF NUMBERS FOR THE STATEMENT
            // ONTO TEMP NYM.)

            // loop through the items that make up the incoming transaction, and
            // add them
            // to the Nym, and remove them from the Nymbox, as appropriate.
            //
            for (auto& it : tranIn.GetItemList()) {
                pItem = it;
                OT_ASSERT_MSG(nullptr != pItem,
                              "Pointer should not have been nullptr.");

                // We already handled this one (if we're even in this block in
                // the first place.)
                //
                if (OTItem::transactionStatement == pItem->GetType()) {
                    continue;
                }

                // If the client sent an accept item then let's process it.
                if ((OTItem::request == pItem->GetStatus()) &&
                    ((OTItem::acceptFinalReceipt ==
                      pItem->GetType()) || // Clearing out a finalReceipt
                                           // notice.
                     (OTItem::acceptTransaction ==
                      pItem->GetType()) || // Accepting new transaction number.
                     (OTItem::acceptMessage ==
                      pItem->GetType()) || // Accepted message.
                     (OTItem::acceptNotice ==
                      pItem->GetType()) // Accepted server notification.
                     )) {
                    OTString strInReferenceTo;

                    // The response item will contain a copy of the "accept"
                    // request.
                    // So I'm just setting aside a copy now for those purposes
                    // later.
                    pItem->SaveContractRaw(strInReferenceTo);

                    OTItem::itemType theReplyItemType;
                    switch (pItem->GetType()) {
                    case OTItem::acceptFinalReceipt:
                        theReplyItemType = OTItem::atAcceptFinalReceipt;
                        break;
                    case OTItem::acceptTransaction:
                        theReplyItemType = OTItem::atAcceptTransaction;
                        break;
                    case OTItem::acceptMessage:
                        theReplyItemType = OTItem::atAcceptMessage;
                        break;
                    case OTItem::acceptNotice:
                        theReplyItemType = OTItem::atAcceptNotice;
                        break;
                    default:
                        OTLog::Error("Should never happen.\n");
                        theReplyItemType =
                            OTItem::error_state; // should never happen based on
                                                 // above 'if' statement.
                        continue; // saving this anyway just cause it's cleaner.
                    }

                    // Server response item being added to server response
                    // transaction (tranOut)
                    // They're getting SOME sort of response item.

                    pResponseItem = OTItem::CreateItemFromTransaction(
                        tranOut, theReplyItemType);
                    pResponseItem->SetStatus(OTItem::rejection); // the default.
                    pResponseItem->SetReferenceString(
                        strInReferenceTo); // the response item carries a copy
                                           // of what it's responding to.
                                           //                    pResponseItem->SetReferenceToNum(pItem->GetTransactionNum());
                    // // This was just 0 every time, since Nymbox needs no
                    // transaction numbers.
                    pResponseItem->SetReferenceToNum(
                        pItem->GetReferenceToNum()); // So the reference was
                                                     // useless. I'm hoping to
                                                     // change it to this and
                                                     // make sure nothing
                                                     // breaks.
                    // ReferenceNum actually means you can match it up against
                    // the request items, and also, that is where THEY store it.
                    tranOut.AddItem(*pResponseItem); // the Transaction's
                                                     // destructor will cleanup
                                                     // the item. It "owns" it
                                                     // now.

                    OTTransaction* pServerTransaction = nullptr;

                    if ((nullptr !=
                         (pServerTransaction = theNymbox.GetTransaction(
                              pItem->GetReferenceToNum()))) &&
                        ((OTTransaction::finalReceipt ==
                          pServerTransaction->GetType()) || // finalReceipt
                                                            // (notice that an
                                                            // opening num was
                                                            // closed.)
                         (OTTransaction::blank ==
                          pServerTransaction->GetType()) || // new transaction
                                                            // number waiting to
                                                            // be picked up.
                         (OTTransaction::message ==
                          pServerTransaction->GetType()) || // message in the
                                                            // nymbox
                         (OTTransaction::replyNotice ==
                          pServerTransaction->GetType()) || // replyNotice
                                                            // containing a
                                                            // server
                         // reply to a previous request.
                         // (Some replies are so important,
                         // this is used to make sure users
                         // get them.)
                         (OTTransaction::successNotice ==
                          pServerTransaction->GetType()) || // successNotice
                                                            // that you signed
                                                            // out a
                                                            // transaction#.
                         (OTTransaction::notice ==
                          pServerTransaction->GetType()) || // server
                                                            // notification, in
                                                            // the nymbox
                         (OTTransaction::instrumentNotice ==
                          pServerTransaction->GetType()) // A financial
                                                         // instrument sent from
                                                         // another user.
                         // (Nymbox=>PaymentInbox)
                         )) {
                        // the accept item will come with the transaction number
                        // that
                        // it's referring to. So we'll just look up that
                        // transaction
                        // in the nymbox, and now that it's been accepted, we'll
                        // process it.

                        // At this point, pItem points to the client's attempt
                        // to accept pServerTransaction
                        // and pServerTransaction is the server's created
                        // transaction in my nymbox that might
                        // have a message or transaction number on it I might
                        // find useful.

                        // What are we doing in this code?
                        //
                        // I need to accept various items that are sitting in my
                        // nymbox, such as:
                        //
                        // -- transaction numbers waiting to be accepted (they
                        // cannot be rejected.)
                        //
                        // -- messages waiting to be accepted (they cannot be
                        // rejected.)
                        //

                        // The below block only executes for ACCEPTING a MESSAGE
                        if ((OTItem::acceptMessage == pItem->GetType()) &&
                            (OTTransaction::message ==
                             pServerTransaction->GetType())) {
                            // pItem contains the current user's attempt to
                            // accept the
                            // ['message'] located in pServerTransaction.
                            // Now we have the user's item and the item he is
                            // trying to accept.
                            pServerTransaction->DeleteBoxReceipt(
                                theNymbox); // faster.
                            theNymbox.RemoveTransaction(
                                pServerTransaction->GetTransactionNum());

                            theNymbox.ReleaseSignatures();
                            theNymbox.SignContract(server_->m_nymServer);
                            theNymbox.SaveContract();
                            theNymbox.SaveNymbox();

                            // Now we can set the response item as an
                            // acknowledgement instead of the default
                            // (rejection)
                            pResponseItem->SetStatus(OTItem::acknowledgement);
                        } // its type is OTItem::aacceptMessage

                        // The below block only executes for ACCEPTING a NOTICE
                        else if ((OTItem::acceptNotice == pItem->GetType()) &&
                                 ((OTTransaction::notice ==
                                   pServerTransaction->GetType()) ||
                                  (OTTransaction::replyNotice ==
                                   pServerTransaction->GetType()) ||
                                  (OTTransaction::successNotice ==
                                   pServerTransaction->GetType()) ||
                                  (OTTransaction::instrumentNotice ==
                                   pServerTransaction->GetType()))) {
                            // pItem contains the current user's attempt to
                            // accept the
                            // ['notice'] or replyNotice or successNotice or
                            // instrumentNotice
                            // located in pServerTransaction.
                            // Now we have the user's item and the item he is
                            // trying to accept.

                            pServerTransaction->DeleteBoxReceipt(
                                theNymbox); // faster.
                            theNymbox.RemoveTransaction(
                                pServerTransaction->GetTransactionNum());

                            theNymbox.ReleaseSignatures();
                            theNymbox.SignContract(server_->m_nymServer);
                            theNymbox.SaveContract();
                            theNymbox.SaveNymbox();

                            // Now we can set the response item as an
                            // acknowledgement instead of the default
                            // (rejection)
                            pResponseItem->SetStatus(OTItem::acknowledgement);

                        } // its type is OTItem::acceptNotice

                        // The below block only executes for ACCEPTING a
                        // TRANSACTION NUMBER
                        // It also places a success notice into the Nymbox, to
                        // solve sync issues.
                        // (We'll make SURE the client got the notice! Probably
                        // should do this
                        // for cash withdrawals as well...)
                        else if ((OTItem::acceptTransaction ==
                                  pItem->GetType()) &&
                                 (OTTransaction::blank ==
                                  pServerTransaction->GetType())) {
                            // Add the success notice to the Nymbox, so if the
                            // Nym fails to see the server reply, he can still
                            // get his
                            // transaction # later, from the notice, instead of
                            // going out of sync.
                            //
                            int64_t lSuccessNoticeTransNum = 0;
                            bool bGotNextTransNum =
                                server_->transactor_.issueNextTransactionNumber(
                                    server_->m_nymServer,
                                    lSuccessNoticeTransNum,
                                    false); // bool bStoreTheNumber = false

                            if (!bGotNextTransNum) {
                                lSuccessNoticeTransNum = 0;
                                OTLog::Error("Error getting next transaction "
                                             "number in "
                                             "Notary::NotarizeProcessNymbox "
                                             "for OTTransaction::blank (for "
                                             "the successNotice)\n");
                            }
                            else {
                                // Drop SUCCESS NOTICE in the Nymbox
                                //
                                OTTransaction* pSuccessNotice =
                                    OTTransaction::GenerateTransaction(
                                        theNymbox, OTTransaction::successNotice,
                                        lSuccessNoticeTransNum);

                                if (nullptr !=
                                    pSuccessNotice) // The above has an
                                                    // OT_ASSERT within,
                                                    // but I just like
                                                    // to check my
                                                    // pointers.
                                {
                                    // If I accepted blank trans#10, then this
                                    // successNotice is in reference to #10.
                                    //
                                    pSuccessNotice->SetReferenceToNum(
                                        pServerTransaction
                                            ->GetTransactionNum());

                                    // Contains a copy of the OTItem where I
                                    // actually accepted the blank transaction
                                    // #.
                                    // (which generated the notice in the first
                                    // place...)
                                    //
                                    pSuccessNotice->SetReferenceString(
                                        strInReferenceTo);

                                    OTNumList theOutput;
                                    pServerTransaction->GetNumList(
                                        theOutput); // now theOutput contains
                                                    // the numlist from the
                                                    // server-side nymbox's copy
                                                    // of the blank. (containing
                                                    // 20 transaction #s)

                                    pSuccessNotice->AddNumbersToTransaction(
                                        theOutput); // Now we add those numbers
                                                    // to the success notice.
                                                    // That way client can add
                                                    // those numbers to his
                                                    // issued and transaction
                                                    // lists.

                                    pSuccessNotice->SignContract(
                                        server_->m_nymServer);
                                    pSuccessNotice->SaveContract();

                                    theNymbox.AddTransaction(
                                        *pSuccessNotice); // Add the
                                                          // successNotice to
                                                          // the nymbox. It
                                                          // takes ownership.

                                    pSuccessNotice->SaveBoxReceipt(theNymbox);
                                }
                            }
                            // pItem contains the current user's attempt to
                            // accept the
                            // transaction number located in pServerTransaction.
                            // Now we have the user's item and the item he is
                            // trying to accept.

                            // Here we remove the blank transaction that was
                            // just accepted.
                            //
                            pServerTransaction->DeleteBoxReceipt(
                                theNymbox); // faster.
                            theNymbox.RemoveTransaction(
                                pServerTransaction->GetTransactionNum());

                            theNymbox.ReleaseSignatures();
                            theNymbox.SignContract(server_->m_nymServer);
                            theNymbox.SaveContract();
                            theNymbox.SaveNymbox(&NYMBOX_HASH);

                            bNymboxHashRegenerated = true;

                            // Now we can set the response item as an
                            // acknowledgement instead of the default
                            // (rejection)
                            pResponseItem->SetStatus(OTItem::acknowledgement);
                        }

                        // The below block only executes for CLEARING a
                        // finalReceipt
                        // (an OPENING TRANSACTION NUMBER was already removed),
                        // and this was
                        // a notice that that had occurred. The client has seen
                        // the notice and is
                        // now clearing it from the box.
                        else if ((OTItem::acceptFinalReceipt ==
                                  pItem->GetType()) &&
                                 (OTTransaction::finalReceipt ==
                                  pServerTransaction->GetType())) {
                            // pItem contains the current user's attempt to
                            // clear the
                            // finalReceipt located in pServerTransaction.
                            // Now we have the user's item and the item he is
                            // trying to accept.

                            pServerTransaction->DeleteBoxReceipt(
                                theNymbox); // faster.
                            theNymbox.RemoveTransaction(
                                pServerTransaction->GetTransactionNum());

                            theNymbox.ReleaseSignatures();
                            theNymbox.SignContract(server_->m_nymServer);
                            theNymbox.SaveContract();
                            theNymbox.SaveNymbox(&NYMBOX_HASH);

                            bNymboxHashRegenerated = true;

                            // Now we can set the response item as an
                            // acknowledgement instead of the default
                            // (rejection)
                            pResponseItem->SetStatus(OTItem::acknowledgement);
                        }
                    }
                    else {
                        OTLog::vError("Error finding original Nymbox "
                                      "transaction that client is trying to "
                                      "accept: %lld\n",
                                      pItem->GetReferenceToNum());
                    }

                    // sign the response item before sending it back (it's
                    // already been added to the transaction above)
                    // Now, whether it was rejection or acknowledgement, it is
                    // set properly and it is signed, and it
                    // is owned by the transaction, who will take it from here.
                    pResponseItem->ReleaseSignatures();
                    pResponseItem->SignContract(server_->m_nymServer);
                    pResponseItem->SaveContract();
                }
                else {
                    const int32_t nStatus = pItem->GetStatus();
                    OTString strItemType;
                    pItem->GetTypeString(strItemType);

                    OTLog::vError("Error, unexpected item type (%s) and/or "
                                  "status (%d) in "
                                  "Notary::NotarizeProcessNymbox\n",
                                  strItemType.Get(), nStatus);
                }
            }
        }
    }

    pResponseBalanceItem->ReleaseSignatures();
    pResponseBalanceItem->SignContract(server_->m_nymServer);
    pResponseBalanceItem->SaveContract();
    tranOut.ReleaseSignatures();
    tranOut.SignContract(server_->m_nymServer);
    tranOut.SaveContract();

    if (bNymboxHashRegenerated) {
        theNym.SetNymboxHashServerSide(NYMBOX_HASH); // server-side
        theNym.SaveSignedNymfile(
            server_->m_nymServer); // todo: make objects like nym
                                   // saveable and dirty-able, so
                                   // they are only saved once and
                                   // not multiple times
                                   // redundantly.
    }

    OTString strPath;

    // On the server side, response will only have chance to succeed if balance
    // agreement succeeds first.
    // Therefore, you will never see successful response but failed balance,
    // since it would stop at the
    // balance and response itself would remain failed with no chance of
    // changing.
    //
    // Thus, "success" must be when balance succeeded and transaction succeeded,
    // and "failure" must be when balance succeeded but transaction failed.
    //
    // If NEITHER succeeded, then there is no point recording it to a file, now
    // is there?

    if ((nullptr != pResponseBalanceItem) &&
        (OTItem::acknowledgement == pResponseBalanceItem->GetStatus())) {
        if (tranOut.GetSuccess()) {
            // Transaction agreement was a success, AND process nymbox was a
            // success.
            // Therefore, add any new issued numbers to theNym, and save.

            theNym.HarvestIssuedNumbers(SERVER_ID, server_->m_nymServer,
                                        theTempNym, true); // bSave=true

            bOutSuccess = true; // the processNymbox was successful.

            strPath.Format(const_cast<char*>("%s.success"), strNymID.Get());
        }
        else
            strPath.Format(const_cast<char*>("%s.fail"), strNymID.Get());

        const char* szFoldername = OTFolders::Receipt().Get();

        tranOut.SaveContract(szFoldername, strPath.Get());
    }
}

/// The client may send multiple transactions in the ledger when he calls
/// processInbox.
/// This function will be called for each of those.  Each may contain multiple
/// items accepting
/// or rejecting certain transactions. The server acknowledges and notarizes
/// those transactions
/// accordingly.
/// (And each of those transactions must be accepted or rejected in whole.)
void Notary::NotarizeProcessInbox(OTPseudonym& theNym, OTAccount& theAccount,
                                  OTTransaction& tranIn, OTTransaction& tranOut,
                                  bool& bOutSuccess)
{
    // The outgoing transaction is an "atProcessInbox", that is, "a reply to the
    // process inbox request"
    tranOut.SetType(OTTransaction::atProcessInbox);

    OTItem* pItem = nullptr;
    OTItem* pBalanceItem = tranIn.GetItem(OTItem::balanceStatement);
    OTItem* pResponseItem = nullptr;
    OTItem* pResponseBalanceItem = nullptr;

    // The incoming transaction may be sent to inboxes and outboxes, and it
    // will probably be bundled in our reply to the user as well. Therefore,
    // let's grab it as a string.
    OTString strInReferenceTo;
    OTString strBalanceItem;

    // Grab the actual server ID from this object, and use it as the server ID
    // here.
    const OTIdentifier SERVER_ID(server_->m_strServerID),
        ACCOUNT_ID(theAccount), USER_ID(theNym);

    const OTString strUserID(USER_ID);

    OTPseudonym theTempNym, theTempClosingNumNym;
    std::unique_ptr<OTLedger> pInbox(
        theAccount.LoadInbox(server_->m_nymServer));
    std::unique_ptr<OTLedger> pOutbox(
        theAccount.LoadOutbox(server_->m_nymServer));

    pResponseBalanceItem =
        OTItem::CreateItemFromTransaction(tranOut, OTItem::atBalanceStatement);
    pResponseBalanceItem->SetStatus(OTItem::rejection); // the default.
    tranOut.AddItem(*pResponseBalanceItem); // the Transaction's destructor will
                                            // cleanup the item. It "owns" it
                                            // now.
    if (!NYM_IS_ALLOWED(strUserID.Get(),
                        ServerSettings::__transact_process_inbox)) {
        OTLog::vOutput(
            0, "%s: User %s cannot do this transaction (All \"process inbox\" "
               "transactions are disallowed in server.cfg)\n",
            __FUNCTION__, strUserID.Get());
    }
    else if (nullptr == pBalanceItem) {
        OTLog::vOutput(
            0, "%s: No Balance Agreement item found on this transaction.\n",
            __FUNCTION__);
    }
    else if (nullptr == pInbox) {
        OTLog::vError("%s: Error loading or verifying inbox.\n", __FUNCTION__);
    }
    else if (nullptr == pOutbox) {
        OTLog::vError("%s: Error loading or verifying outbox.\n", __FUNCTION__);
    }
    else {
        pBalanceItem->SaveContractRaw(strBalanceItem);

        pResponseBalanceItem->SetReferenceString(
            strBalanceItem); // the response item carries a copy of what it's
                             // responding to.
        pResponseBalanceItem->SetReferenceToNum(
            pBalanceItem->GetTransactionNum()); // This response item is IN
                                                // RESPONSE to tranIn's balance
                                                // agreement
        pResponseBalanceItem->SetNumberOfOrigin(*pBalanceItem);

        // This transaction accepts various incoming pending transfers.
        // So when it's all done, my balance will be higher.
        // AND pending inbox items will be removed from my inbox.
        //
        // I would like to not even process the whole giant loop below,
        // if I can verify here now that the balance agreement is wrong.
        //
        // Thus I will actually loop through the acceptPending items in tranIn,
        // and then for each one, I'll
        // lookup the ACTUAL transaction in the inbox, and get its ACTUAL value.
        // (And total them all up.)
        //
        // The total of those, (WITHOUT the user having to tell me what it will
        // be, since I'm looking them all up),
        // should equal the difference in the account balance! Meaning the
        // current balance plus that total will be
        // the expected NEW balance, according to this balance agreement -- if
        // it wants to be approved, that is.
        //
        //

        std::list<int64_t> theListOfInboxReceiptsBeingRemoved;
        // To make sure each
        // inbox item refers to a different number. (If two of them refer to the
        // same number, that's bad and is not allowed. You can't process the
        // same inbox item twice simultaneously! Or even at all.)

        bool bSuccessFindingAllTransactions = true;
        int64_t lTotalBeingAccepted = 0;

        for (auto& it_bigloop : tranIn.GetItemList()) {
            pItem = it_bigloop;
            OT_ASSERT_MSG(nullptr != pItem,
                          "Pointer should not have been nullptr.");
            OTTransaction* pServerTransaction = nullptr;

            switch (pItem->GetType()) {
            case OTItem::balanceStatement:
                pServerTransaction = nullptr;
                continue;

            case OTItem::acceptCronReceipt:
            case OTItem::acceptFinalReceipt:
            case OTItem::acceptBasketReceipt:
            case OTItem::disputeCronReceipt:
            case OTItem::disputeFinalReceipt:
            case OTItem::disputeBasketReceipt:
                pServerTransaction =
                    pInbox->GetTransaction(pItem->GetReferenceToNum());
                break;

            case OTItem::acceptPending:     // Accept an incoming (pending)
                                            // transfer.
            case OTItem::acceptItemReceipt: // Accept a chequeReceipt,
                                            // voucherReceipt, or
                                            // transferReceipt.
            case OTItem::rejectPending:
            case OTItem::disputeItemReceipt:
                pServerTransaction =
                    pInbox->GetTransaction(pItem->GetReferenceToNum());
                break;

            default: {
                OTString strItemType;
                pItem->GetTypeString(strItemType);
                int32_t nItemType = pItem->GetType();

                pServerTransaction = nullptr;
                bSuccessFindingAllTransactions = false;

                OTLog::vError("%s: Wrong item type: %s (%d).\n", __FUNCTION__,
                              strItemType.Exists() ? strItemType.Get() : "",
                              nItemType);
                break;
            }
            }
            if (nullptr == pServerTransaction) {
                const OTString strAccountID(ACCOUNT_ID);
                OTLog::vError("%s: Unable to find or process inbox transaction "
                              "being accepted by user: %s for account: %s\n",
                              __FUNCTION__, strUserID.Get(),
                              strAccountID.Get());
                bSuccessFindingAllTransactions = false;
                break;
            }
            else if (pServerTransaction->GetReceiptAmount() !=
                       pItem->GetAmount()) {
                OTLog::vError(
                    "%s: Receipt amounts don't match: %lld and %lld. Nym: %s\n",
                    __FUNCTION__, pServerTransaction->GetReceiptAmount(),
                    pItem->GetAmount(), strUserID.Get());
                bSuccessFindingAllTransactions = false;
                break;
            }

            // BELOW THIS POINT, WE KNOW THAT pServerTransaction was FOUND (and
            // validated.)

            switch (pItem->GetType()) {
            case OTItem::acceptCronReceipt:
                bSuccessFindingAllTransactions = true;
                break;
            case OTItem::acceptFinalReceipt:
                bSuccessFindingAllTransactions = true;

                // Need to ERROR OUT here, if the number of cron receipts
                // (related to
                // this finalReceipt) in the inbox isn't equal to the number
                // being accepted
                // in this processInbox transaction. (You can't close the final
                // receipt unless
                // you close all the others as well.)
                //

                // IN THIS CASE: If user is accepting a finalReceipt, that means
                // all the OTHER receipts related to it
                // (sharing the same "in reference to") must ALSO be cleared
                // from the inbox along with it! That's the
                // whole point of the finalReceipt -- to make sure all related
                // receipts are cleared, when IT is.
                //
                // So let's see if the number of related receipts on this
                // process inbox (tranIn) matches
                // the number of related receipts in the actual inbox (pInbox),
                // as found by the finalReceipt's
                // (pServerTransaction) "in reference to" value, which should be
                // the same as on the related receipts.

                // (Below) tranIn is the processInbox transaction. Each item on
                // it is "in ref to" a DIFFERENT receipt,
                // even though, if they are marketReceipts, all of THOSE
                // receipts are "in ref to" the original transaction#.
                // I need to loop through all items on tranIn (processInbox
                // request)
                // For each, look it up on the inbox. (Each item will be "in
                // reference to" the original transaction.)
                // ONCE THE INBOX RECEIPT IS FOUND, if *IT* is "in reference to"
                // pServerTransaction->GetReferenceToNum(),
                // Then increment the count for the transaction.  COMPARE *THAT*
                // to theInbox.GetCount and we're golden!!
                {
                    std::set<int64_t> setOfRefNumbers; // we'll store them here,
                                                       // and disallow
                                                       // duplicates, to make
                                                       // sure they are all
                                                       // unique IDs (no
                                                       // repeats.)

                    for (auto& it : tranIn.GetItemList()) {
                        OTItem* pItemPointer = it;
                        OT_ASSERT_MSG(nullptr != pItemPointer,
                                      "Pointer should not have been nullptr.");

                        OTTransaction* pTransPointer = pInbox->GetTransaction(
                            pItemPointer->GetReferenceToNum());

                        if ((nullptr != pTransPointer) &&
                            (pTransPointer->GetReferenceToNum() ==
                             pServerTransaction->GetReferenceToNum())) {
                            setOfRefNumbers.insert(
                                pItemPointer->GetReferenceToNum());
                        }
                    }

                    if (pInbox->GetTransactionCountInRefTo(
                            pServerTransaction->GetReferenceToNum()) !=
                        static_cast<int32_t>(setOfRefNumbers.size())) {
                        OTLog::vOutput(
                            0, "%s: User tried to close a finalReceipt, "
                               "without also closing all related receipts. "
                               "(Those that share the IN REF TO number.)\n",
                            __FUNCTION__);
                        bSuccessFindingAllTransactions = false;
                        break;
                    }
                    // Upon success, these numbers will be removed from the
                    // Nym's additional record of "cron item IDs".
                    //

                    // Server side stores a list of open cron items on each Nym.
                    // The closing transaction number on the final receipt
                    // SHOULD be on that list.
                    //
                    std::set<int64_t>& theIDSet = theNym.GetSetOpenCronItems();

                    std::set<int64_t>::iterator theSetIT =
                        theIDSet.find(pServerTransaction->GetClosingNum());

                    // If we FOUND it on the Nym, then we add it to the list to
                    // be removed from Nym's open cron items.
                    // (If it wasn't there before, then we wouldn't want to
                    // "re-add" it, now would we?)
                    //
                    if (theIDSet.end() != theSetIT) // FOUND IT!
                        theTempClosingNumNym.AddIssuedNum(
                            server_->m_strServerID,
                            pServerTransaction->GetClosingNum()); // Schedule to
                                                                  // remove
                    // GetClosingNum() from
                    // server-side list of Nym's
                    // open cron items. (By
                    // adding it to
                    // theTempClosingNumNym.)
                    else
                        OTLog::vOutput(
                            1, "%s: expected to find "
                               "pServerTransaction->GetClosingNum() (%lld) on "
                               "Nym's (%s) "
                               "list of open cron items. (Maybe he didn't see "
                               "the notice in his Nymbox yet.)\n",
                            __FUNCTION__, pServerTransaction->GetClosingNum(),
                            strUserID.Get());
                    // else error log.
                }

            // ---- COUNT is correct and closing num is on list of open cron
            // items. (FINAL RECEIPT FALLS THROUGH HERE!!! no break)

            case OTItem::acceptBasketReceipt:
                // IF it's actually there on theNym, then schedule it for
                // removal.
                // (Otherwise we'd end up improperly re-adding it.)
                //
                if (theNym.VerifyIssuedNum(server_->m_strServerID,
                                           pServerTransaction->GetClosingNum()))
                    theTempNym.AddIssuedNum(
                        server_->m_strServerID,
                        pServerTransaction->GetClosingNum());
                else {
                    bSuccessFindingAllTransactions = false;

                    OTLog::vError("%s: basket or final receipt, trying to "
                                  "'remove' an issued "
                                  "number (%lld) that already wasn't on Nym's "
                                  "issued list. (So what is this in the inbox, "
                                  "then?)\n",
                                  __FUNCTION__,
                                  pServerTransaction->GetClosingNum());
                }

                break;
            case OTItem::acceptPending:
                // IF I'm accepting a pending transfer, then add the amount to
                // my counter of total amount being accepted.
                //
                lTotalBeingAccepted += pServerTransaction->GetReceiptAmount();
                bSuccessFindingAllTransactions = true;
                break;
            case OTItem::acceptItemReceipt:
                bSuccessFindingAllTransactions = true;
                {
                    // If I'm accepting an item receipt (which will remove my
                    // responsibility for that item) then add it
                    // to the temp Nym (which is a list of transaction numbers
                    // that will be removed from my responsibility if
                    // all is successful.)  Also remove all the Temp Nym numbers
                    // from theNym, so we can verify the Balance
                    // Statement AS IF they were already removed.
                    //
                    // What number do I remove here? the user is accepting a
                    // transfer receipt, which
                    // is in reference to the recipient's acceptPending. THAT
                    // item is in reference to
                    // my original transfer (or contains a cheque with my
                    // original number.) (THAT's the # I need.)
                    //
                    OTString strOriginalItem;
                    pServerTransaction->GetReferenceString(strOriginalItem);

                    std::unique_ptr<OTItem> pOriginalItem(
                        OTItem::CreateItemFromString(
                            strOriginalItem, SERVER_ID,
                            pServerTransaction->GetReferenceToNum()));

                    if (nullptr != pOriginalItem) {
                        // If pOriginalItem is acceptPending, that means the
                        // client is accepting the transfer receipt from the
                        // server, (from his inbox),
                        // which has the recipient's acceptance inside of the
                        // client's transfer as the original item. This means
                        // the transfer that
                        // the client originally sent is now finally closed!
                        //
                        // If it's a depositCheque, that means the client is
                        // accepting the cheque receipt from the server, (from
                        // his inbox)
                        // which has the recipient's deposit inside of it as the
                        // original item. This means that the cheque that
                        // the client originally wrote is now finally closed!
                        //
                        // In both cases, the "original item" itself is not from
                        // the client, but from the recipient! Therefore,
                        // the number on that item is useless for removing
                        // numbers from the client's list of issued numbers.
                        // Rather, I need to load that original cheque, or
                        // pending transfer, from WITHIN the original item,
                        // in order to get THAT number, to remove it from the
                        // client's issued list. (Whether for real, or for
                        // setting up dummy data in order to verify the balance
                        // agreement.) *sigh*
                        //
                        if (OTItem::depositCheque ==
                            pOriginalItem->GetType()) // client is accepting a
                                                      // cheque receipt, which
                                                      // has a depositCheque
                                                      // (from the recipient) as
                                                      // the original item
                                                      // within.
                        {
                            // Get the cheque from the Item and load it up into
                            // a Cheque object.
                            OTString strCheque;
                            pOriginalItem->GetAttachment(strCheque);

                            OTCheque theCheque; // allocated on the stack :-)

                            if (false ==
                                ((strCheque.GetLength() > 2) &&
                                 theCheque.LoadContractFromString(strCheque))) {
                                OTLog::vError("%s: ERROR loading cheque from "
                                              "string:\n%s\n",
                                              __FUNCTION__, strCheque.Get());
                                bSuccessFindingAllTransactions = false;
                            }
                            else // Since the client wrote the cheque, and he
                                   // is now accepting the cheque receipt, he
                                   // can be cleared for that transaction
                                   // number...
                            {
                                // IF it's actually there on theNym, then
                                // schedule it for removal.
                                // (Otherwise we'd end up improperly re-adding
                                // it.)
                                //
                                if (theNym.VerifyIssuedNum(
                                        server_->m_strServerID,
                                        theCheque.GetTransactionNum()))
                                    theTempNym.AddIssuedNum(
                                        server_->m_strServerID,
                                        theCheque.GetTransactionNum());
                                else {
                                    bSuccessFindingAllTransactions = false;

                                    OTLog::vError(
                                        "%s: cheque receipt, trying to "
                                        "'remove' an issued "
                                        "number (%lld) that already wasn't on "
                                        "Nym's issued list. (So what is this "
                                        "in the inbox, "
                                        "then?)\n",
                                        __FUNCTION__,
                                        theCheque.GetTransactionNum());
                                }
                            }
                        }
                        // client is accepting a transfer receipt, which has an
                        // acceptPending from the recipient as the original item
                        // within,
                        else if (OTItem::acceptPending ==
                                 pOriginalItem->GetType()) // (which is in
                                                           // reference to the
                                                           // client's outoing
                                                           // original
                                                           // transfer.)
                        {
                            // IF it's actually there on theNym, then schedule
                            // it for removal.
                            // (Otherwise we'd end up improperly re-adding it.)
                            //
                            if (theNym.VerifyIssuedNum(
                                    server_->m_strServerID,
                                    pOriginalItem->GetNumberOfOrigin()))
                                theTempNym.AddIssuedNum(
                                    server_->m_strServerID,
                                    pOriginalItem->GetNumberOfOrigin());
                            else {
                                bSuccessFindingAllTransactions = false;

                                OTLog::vError(
                                    "%s: transfer receipt, trying to 'remove' "
                                    "an issued "
                                    "number (%lld) that already wasn't on "
                                    "Nym's "
                                    "issued list. (So what is this in the "
                                    "inbox, "
                                    "then?)\n",
                                    __FUNCTION__,
                                    pOriginalItem->GetReferenceToNum());
                            }
                        }
                        else {
                            OTString strOriginalItemType;
                            pOriginalItem->GetTypeString(strOriginalItemType);
                            OTLog::vError("%s: Original item has wrong type, "
                                          "while accepting item receipt:\n%s\n",
                                          __FUNCTION__,
                                          strOriginalItemType.Get());
                            bSuccessFindingAllTransactions = false;
                        }
                    }
                    else {
                        OTLog::vError("%s: Unable to load original item from "
                                      "string while accepting item "
                                      "receipt:\n%s\n",
                                      __FUNCTION__, strOriginalItem.Get());
                        bSuccessFindingAllTransactions = false;
                    }
                } // pItem is acceptItemReceipt --------------------

                break;
            default:
                OTLog::Error("Wrong item type in "
                             "Notary::NotarizeProcessInbox. (2nd notice.)\n");
                bSuccessFindingAllTransactions = false;
                break;
            } // switch --------------------------------------------

            // I'll also go ahead and remove each transaction from pInbox, and
            // pass said inbox into the VerifyBalanceAgreement call...
            // (So it can simulate as if the inbox was already processed, and
            // the total is already calculated, and if it succeeds,
            // then we can allow the giant loop below to do it all for real.)
            // (I'm not saving this copy of the inbox anyway--there's another
            // one below.)
            //
            if (bSuccessFindingAllTransactions) {
                // WE'RE REMOVING THE TRANSACTIONS FROM AN INBOX COPY, IN ORDER
                // TO VERIFY THE
                // BALANCE AGREEMENT (WITH THAT INBOX COPY SET UP AS THOUGH THE
                // TRANSACTION HAD
                // ALREADY BEEN A SUCCESS.)
                // I'm not ACTUALLY removing though, until AFTER the loop (in
                // case the rest of the
                // loop needs the data still, in that inbox.) So we save in a
                // list, and remove AFTER the loop.
                //
                theListOfInboxReceiptsBeingRemoved.push_back(
                    pServerTransaction->GetTransactionNum());
            }
            else // If there was an error above, then we don't want to keep
                   // looping. We want the below error block.
                break;
        } // for loop (list of "process inbox" items)

        if (!bSuccessFindingAllTransactions) {
            OTLog::vOutput(0, "%s: transactions in processInbox message do not "
                              "match actual inbox.\n",
                           __FUNCTION__);
        }
        else { // Remove certain receipts (determined in the big loop above)
                 // from the inbox copy,
            // to see if it will verify in the balance agreement.
            //
            while (!theListOfInboxReceiptsBeingRemoved.empty()) {
                int64_t lTemp = theListOfInboxReceiptsBeingRemoved.front();
                theListOfInboxReceiptsBeingRemoved.pop_front();

                // Notice I don't call DeleteBoxReceipt(lTemp) here like I
                // normally would when calling
                // RemoveTransaction(lTemp), since this is only a copy of my
                // inbox and not the real thing.
                //
                if (false ==
                    pInbox->RemoveTransaction(lTemp)) // <================
                    OTLog::vError(
                        "%s: Failed removing receipt from Inbox copy: %lld \n"
                        "Meaning the client probably has an old copy of his "
                        "inbox. "
                        "We don't even see the receipt that he still thinks he "
                        "has.\n",
                        __FUNCTION__, lTemp);
            }
            // Remove certain issued numbers (determined in the big loop above)
            // from the Nym,
            // to see if it will verify in the balance agreement (we'll re-add
            // them after.)
            // Also, we're ONLY removing these because we verified above that
            // they were really there.
            // Otherwise it'd be pretty stupid to "re-add" them, eh?
            //
            for (int32_t i = 0; i < theTempNym.GetIssuedNumCount(SERVER_ID);
                 i++) {
                int64_t lTemp = theTempNym.GetIssuedNum(SERVER_ID, i);
                theNym.RemoveIssuedNum(server_->m_strServerID, lTemp);
            }

            // FINALLY after all that setup, we can do the balance agreement!!
            //
            const bool bVerifiedBalanceStatement =
                pBalanceItem->VerifyBalanceStatement(
                    lTotalBeingAccepted, // <========================
                    theNym, *pInbox, *pOutbox, theAccount, tranIn);

            // Here, add all the issued nums back (that had been temporarily
            // removed from theNym) that were stored on theTempNym for
            // safe-keeping.
            for (int32_t i = 0; i < theTempNym.GetIssuedNumCount(SERVER_ID);
                 i++) {
                int64_t lTemp = theTempNym.GetIssuedNum(SERVER_ID, i);
                theNym.AddIssuedNum(server_->m_strServerID, lTemp);
            }
            // (They are removed for real at the bottom of this function, IF
            // everything is successful between now and then.)

            if (!bVerifiedBalanceStatement) {
                OTLog::vOutput(0, "Notary::NotarizeProcessInbox: ERROR "
                                  "verifying balance statement for transaction "
                                  "%lld.\n",
                               tranIn.GetTransactionNum());
            }
            else // BALANCE AGREEMENT WAS SUCCESSFUL.......
            {
                pResponseBalanceItem->SetStatus(
                    OTItem::acknowledgement); // the transaction agreement was
                                              // successful.

                // THE ABOVE LOOP WAS JUST A TEST RUN
                //
                // (TO VERIFY BALANCE AGREEMENT BEFORE WE BOTHERED TO RUN THIS
                // LOOP BELOW...)

                // loop through the items that make up the incoming transaction
                //
                for (auto& it_bigloop_two : tranIn.GetItemList()) {
                    pItem = it_bigloop_two;
                    OT_ASSERT_MSG(nullptr != pItem,
                                  "Pointer should not have been nullptr.");

                    // We already handled this one (if we're even in this block
                    // in the first place.)
                    //
                    if (OTItem::balanceStatement == pItem->GetType()) {
                        continue;
                    }

                    // If the client sent an accept item, (or reject/dispute)
                    // then let's process it.
                    if ((OTItem::request == pItem->GetStatus()) &&
                        ((OTItem::acceptCronReceipt ==
                          pItem->GetType()) || // Accepting notice of market
                                               // trade or payment processing.
                                               // (Original in Cron Receipt.)
                         //                       (OTItem::disputeCronReceipt
                         (OTItem::acceptItemReceipt ==
                          pItem->GetType()) || // Accepted item receipt (cheque,
                                               // transfer)
                         (OTItem::acceptPending ==
                          pItem->GetType()) || // Accepting notice of pending
                                               // transfer
                         (OTItem::acceptFinalReceipt ==
                          pItem->GetType()) || // Accepting finalReceipt
                         (OTItem::acceptBasketReceipt ==
                          pItem->GetType()) // Accepting basketReceipt
                         )) {
                        // The response item will contain a copy of the "accept"
                        // request.
                        // So I'm just setting aside a copy now for those
                        // purposes later.
                        strInReferenceTo.Release();
                        pItem->SaveContractRaw(strInReferenceTo);

                        OTItem::itemType theReplyItemType;
                        switch (pItem->GetType()) {
                        case OTItem::acceptPending:
                            theReplyItemType = OTItem::atAcceptPending;
                            break;
                        case OTItem::rejectPending:
                            theReplyItemType = OTItem::atRejectPending;
                            break;
                        case OTItem::acceptCronReceipt:
                            theReplyItemType = OTItem::atAcceptCronReceipt;
                            break;
                        case OTItem::disputeCronReceipt:
                            theReplyItemType = OTItem::atDisputeCronReceipt;
                            break;
                        case OTItem::acceptItemReceipt:
                            theReplyItemType = OTItem::atAcceptItemReceipt;
                            break;
                        case OTItem::disputeItemReceipt:
                            theReplyItemType = OTItem::atDisputeItemReceipt;
                            break;
                        case OTItem::acceptFinalReceipt:
                            theReplyItemType = OTItem::atAcceptFinalReceipt;
                            break;
                        case OTItem::disputeFinalReceipt:
                            theReplyItemType = OTItem::atDisputeFinalReceipt;
                            break;
                        case OTItem::acceptBasketReceipt:
                            theReplyItemType = OTItem::atAcceptBasketReceipt;
                            break;
                        case OTItem::disputeBasketReceipt:
                            theReplyItemType = OTItem::atDisputeBasketReceipt;
                            break;
                        default:
                            OTLog::Error("Should never happen.\n");
                            theReplyItemType =
                                OTItem::error_state; // should never happen
                                                     // based on above 'if'
                                                     // statement.
                            break; // saving this anyway just cause it's
                                   // cleaner.
                        }

                        // Server response item being added to server response
                        // transaction (tranOut)
                        // They're getting SOME sort of response item.

                        pResponseItem = OTItem::CreateItemFromTransaction(
                            tranOut, theReplyItemType);
                        pResponseItem->SetStatus(
                            OTItem::rejection); // the default.
                        pResponseItem->SetReferenceString(
                            strInReferenceTo); // the response item carries a
                                               // copy of what it's responding
                                               // to.
                        pResponseItem->SetReferenceToNum(
                            pItem->GetTransactionNum());
                        pResponseItem->SetNumberOfOrigin(*pItem);

                        tranOut.AddItem(*pResponseItem); // the Transaction's
                                                         // destructor will
                                                         // cleanup the item. It
                                                         // "owns" it now.

                        // Need to load the Inbox first, in order to look up the
                        // transaction that
                        // the client is accepting. This is possible because the
                        // client has included
                        // the transaction number.  I'll just look it up in his
                        // inbox and then
                        // process it.
                        // theAcctID is the ID on the client Account that was
                        // passed in.
                        OTLedger theInbox(USER_ID, ACCOUNT_ID, SERVER_ID);

                        OTTransaction* pServerTransaction = nullptr;

                        if (!theInbox.LoadInbox()) {
                            OTLog::Error(
                                "Error loading inbox during processInbox\n");
                        }
                        else if (false ==
                                   theInbox.VerifyAccount(
                                       server_->m_nymServer)) {
                            OTLog::Error(
                                "Error verifying inbox during processInbox\n");
                        }
                        //
                        // Warning! In the case of a
                        // OTTransaction::paymentReceipt or
                        // OTTransaction::marketReceipt,
                        // the "in reference to" string will NOT contain an
                        // OTItem at all, but an OTPaymentPlan or
                        // an OTTrade!! Also, a paymentReceipt might be for a
                        // smart contract, in which case there's
                        // a smartcontract inside, instead of a payment plan! I
                        // handle these cases first, here:
                        else if ( // MARKET RECEIPT, or PAYMENT RECEIPT.....
                            ((OTItem::acceptCronReceipt ==
                              pItem->GetType()) // This is checked
                                                // above, but just
                                                // keeping this safe.
                             ) // especially in case this block moves
                               // or is used elsewhere.
                            &&
                            (nullptr !=
                             (pServerTransaction = theInbox.GetTransaction(
                                  pItem->GetReferenceToNum()))) &&
                            ((OTTransaction::paymentReceipt ==
                              pServerTransaction->GetType()) ||
                             (OTTransaction::marketReceipt ==
                              pServerTransaction->GetType()))) {
                            // pItem contains the current user's attempt to
                            // accept the Receipt
                            // represented by pServerTransaction. Therefore we
                            // have the user's
                            // item AND the receipt he is trying to accept.

                            pServerTransaction->DeleteBoxReceipt(
                                theInbox); // faster.
                            theInbox.RemoveTransaction(
                                pServerTransaction->GetTransactionNum());

                            theInbox.ReleaseSignatures();
                            theInbox.SignContract(server_->m_nymServer);
                            theInbox.SaveContract();
                            theAccount.SaveInbox(theInbox);
                            theAccount.ReleaseSignatures();
                            theAccount.SignContract(server_->m_nymServer);
                            theAccount.SaveContract();
                            theAccount.SaveAccount();

                            // Now we can set the response item as an
                            // acknowledgement instead of the default
                            // (rejection)
                            pResponseItem->SetStatus(OTItem::acknowledgement);
                        }
                        else if ( // FINAL RECEIPT
                            ((OTItem::acceptFinalReceipt ==
                              pItem->GetType()) // This is checked
                                                // above, but just
                                                // keeping this safe.
                             ) // especially in case this block moves
                               // or is used elsewhere.
                            &&
                            (nullptr !=
                             (pServerTransaction = theInbox.GetTransaction(
                                  pItem->GetReferenceToNum()))) &&
                            ((OTTransaction::finalReceipt ==
                              pServerTransaction->GetType()))) {
                            // pItem contains the current user's attempt to
                            // accept the Receipt
                            // represented by pServerTransaction. Therefore we
                            // have the user's
                            // item AND the receipt he is trying to accept.

                            pServerTransaction->DeleteBoxReceipt(
                                theInbox); // faster.
                            theInbox.RemoveTransaction(
                                pServerTransaction->GetTransactionNum());

                            theInbox.ReleaseSignatures();
                            theInbox.SignContract(server_->m_nymServer);
                            theInbox.SaveContract();
                            theAccount.SaveInbox(theInbox);
                            theAccount.ReleaseSignatures();
                            theAccount.SignContract(server_->m_nymServer);
                            theAccount.SaveContract();
                            theAccount.SaveAccount();

                            // Now we can set the response item as an
                            // acknowledgement instead of the default
                            // (rejection)
                            pResponseItem->SetStatus(OTItem::acknowledgement);
                        }
                        else if ( // BASKET RECEIPT
                            ((OTItem::acceptBasketReceipt ==
                              pItem->GetType()) // This is checked
                                                // above, but just
                                                // keeping this safe.
                             ) // especially in case this block moves
                               // or is used elsewhere.
                            &&
                            (nullptr !=
                             (pServerTransaction = theInbox.GetTransaction(
                                  pItem->GetReferenceToNum()))) &&
                            ((OTTransaction::basketReceipt ==
                              pServerTransaction->GetType()))) {
                            // pItem contains the current user's attempt to
                            // accept the Receipt
                            // represented by pServerTransaction. Therefore we
                            // have the user's
                            // item AND the receipt he is trying to accept.

                            pServerTransaction->DeleteBoxReceipt(
                                theInbox); // faster.
                            theInbox.RemoveTransaction(
                                pServerTransaction->GetTransactionNum());

                            theInbox.ReleaseSignatures();
                            theInbox.SignContract(server_->m_nymServer);
                            theInbox.SaveContract();
                            theAccount.SaveInbox(theInbox);
                            theAccount.ReleaseSignatures();
                            theAccount.SignContract(server_->m_nymServer);
                            theAccount.SaveContract();
                            theAccount.SaveAccount();

                            // Now we can set the response item as an
                            // acknowledgement instead of the default
                            // (rejection)
                            pResponseItem->SetStatus(OTItem::acknowledgement);
                        }

                        // Careful here.  I'm looking up the original
                        // transaction number (1, say) which is stored
                        // in my inbox as a "in reference to" on transaction
                        // number 41. (Which is a pending transaction
                        // or receipt
                        // that the server created in my inbox, and only REFERS
                        // to the original transaction, but is not
                        // the original transaction in and of itself.)
                        //
                        // In other words, in this case below, I am looking for
                        // the transaction in the Inbox
                        // that REFERS to the same transaction that the accept
                        // item REFERS to. That process, necessary
                        // for pending transactions and cheque receipts, is NOT
                        // the case above, with receipts from cron.
                        else if (((OTItem::acceptItemReceipt ==
                                   pItem->GetType()) // acceptItemReceipt
                                                     // includes checkReceipt
                                                     // and transferReceipts.
                                  ||
                                  (OTItem::acceptPending ==
                                   pItem->GetType()) // acceptPending includes
                                                     // checkReceipts. Because
                                                     // they are
                                  ) &&
                                 (nullptr !=
                                  (pServerTransaction = theInbox.GetTransaction(
                                       pItem->GetReferenceToNum()))) &&
                                 ((OTTransaction::pending ==
                                   pServerTransaction
                                       ->GetType()) || // pending transfer.
                                  (OTTransaction::transferReceipt ==
                                   pServerTransaction->GetType()) || // transfer
                                                                     // receipt.
                                  (OTTransaction::voucherReceipt ==
                                   pServerTransaction->GetType()) || // voucher
                                                                     // receipt.
                                  (OTTransaction::chequeReceipt ==
                                   pServerTransaction->GetType()) // cheque
                                                                  // receipt is
                                                                  // down here
                                                                  // in the
                                                                  // pending
                                                                  // section,
                                  ) // because this is where an OTItem is loaded
                                    // up (since it
                                 ) // originated with a deposit transaction, not
                                   // a cron receipt.)
                        {
                            // The accept item will come with the transaction
                            // number that
                            // it's referring to. So we'll just look up that
                            // transaction
                            // in the inbox, and now that it's been accepted,
                            // we'll process it.

                            // At this point, pItem points to the client's
                            // attempt to accept pServerTransaction
                            // and pServerTransaction is the server's created
                            // transaction in my inbox that contains
                            // the original item (from the sender) as the
                            // "referenced to" object. So let's extract
                            // it.
                            OTString strOriginalItem;
                            pServerTransaction->GetReferenceString(
                                strOriginalItem);

                            std::unique_ptr<OTItem> pOriginalItem(
                                OTItem::CreateItemFromString(
                                    strOriginalItem, SERVER_ID,
                                    pServerTransaction->GetReferenceToNum()));

                            if (nullptr != pOriginalItem) {

                                // What are we doing in this code?
                                //
                                // I need to accept various items that are
                                // sitting in my inbox, such as:
                                //
                                // -- transfers waiting to be accepted (or
                                // rejected.)
                                //
                                // -- cheque deposit receipts waiting to be
                                // accepted (they cannot be rejected.)
                                //
                                // -- transfer receipts waiting to be accepted
                                // (they cannot be rejected.)

                                //
                                // ONLY in the case of pending transfers also do
                                // I need to mess around with my account,
                                // and the sender's inbox and outbox. In the
                                // other cases, I merely need to remove
                                // the item from my inbox.
                                // Although when 'accepting the reject', I do
                                // need to take the money back into
                                // my inbox...

                                // The depositCheque request OTItem is saved as
                                // a "in reference to" field
                                // on the inbox chequeReceipt transaction.

                                // Therefore, if I am processing an
                                // acceptPending item from the client,
                                // for accepting a chequeReceipt Transaction
                                // that's in his inbox, and
                                // the original item (that the receipt is for)
                                // is a depositCheque,
                                // then I can go ahead and clear it from his
                                // inbox.

                                // The below block only executes for ACCEPTING a
                                // CHEQUE deposit receipt, or
                                // for 'Accepting an ACCEPT.'
                                //
                                // I can't 'Accept a REJECT' without also
                                // transferring the rejected money back into
                                // my own account. And that means fiddling with
                                // my account, and that means it will
                                // be in a different block of code than this
                                // one.
                                //
                                // Whereas with accepting a cheque deposit
                                // receipt, or accepting an accepted transfer
                                // notice,
                                // in both of those cases, my account balance
                                // doesn't change at all. I just need to accept
                                // those notices in order to get them out of my
                                // inbox. So that's the simplest case, and it's
                                // handled by THIS block of code:
                                //
                                if ((OTItem::acceptItemReceipt ==
                                     pItem->GetType()) &&
                                    (((OTTransaction::transferReceipt ==
                                       pServerTransaction->GetType()) &&
                                      (OTItem::acceptPending ==
                                       pOriginalItem->GetType())) ||
                                     (((OTTransaction::chequeReceipt ==
                                        pServerTransaction->GetType()) ||
                                       (OTTransaction::voucherReceipt ==
                                        pServerTransaction->GetType())) &&
                                      (OTItem::depositCheque ==
                                       pOriginalItem->GetType())))) { // (The
                                                                      // funds
                                                                      // are
                                                                      // already
                                                                      // paid
                                                                      // out...)
                                    // pItem contains the current user's attempt
                                    // to accept the
                                    // ['depositCheque' OR 'acceptPending']
                                    // located in theOriginalItem.
                                    // Now we have the user's item and the item
                                    // he is trying to accept.

                                    pServerTransaction->DeleteBoxReceipt(
                                        theInbox); // faster.
                                    theInbox.RemoveTransaction(
                                        pServerTransaction
                                            ->GetTransactionNum());
                                    theInbox.ReleaseSignatures();
                                    theInbox.SignContract(server_->m_nymServer);
                                    theInbox.SaveContract();
                                    theAccount.SaveInbox(theInbox);
                                    theAccount.ReleaseSignatures();
                                    theAccount.SignContract(
                                        server_->m_nymServer);
                                    theAccount.SaveContract();
                                    theAccount.SaveAccount();

                                    // Now we can set the response item as an
                                    // acknowledgement instead of the default
                                    // (rejection)
                                    pResponseItem->SetStatus(
                                        OTItem::acknowledgement);

                                    // Don't I need to remove from
                                    // responsibility list?
                                    // No, because that is done at the bottom of
                                    // the function.
                                    //
                                } // its type is OTItem::acceptPending or
                                  // OTItem::depositCheque

                                // TODO: 'Accept a REJECT' -- NEED TO PERFORM
                                // THE TRANSFER OF FUNDS BACK TO THE SENDER'S
                                // ACCOUNT WHEN TRANSFER IS REJECTED.

                                // The below block only executes for ACCEPTING a
                                // TRANSFER
                                else if ((OTTransaction::pending ==
                                          pServerTransaction->GetType()) &&
                                         (OTItem::transfer ==
                                          pOriginalItem->GetType())) {
                                    // pItem contains the current user's attempt
                                    // to accept the transfer located in
                                    // theOriginalItem.
                                    // Now we have both items.
                                    OTIdentifier IDFromAccount(
                                        pOriginalItem->GetPurportedAccountID());
                                    OTIdentifier IDToAccount(
                                        pOriginalItem->GetDestinationAcctID());

                                    // I'm using the operator== because it
                                    // exists.
                                    // If the ID on the "To" account from the
                                    // original transaction does not
                                    // match the Acct ID of the client trying to
                                    // accept the transaction...
                                    if (!(ACCOUNT_ID == IDToAccount)) {
                                        OTLog::Error(
                                            "Error: Destination account ID on "
                                            "the transaction does not match "
                                            "account ID of client transaction "
                                            "item.\n");
                                    }

                                    // The 'from' outbox is loaded to remove the
                                    // outgoing transfer, since it has been
                                    // accepted.
                                    // The 'from' inbox is loaded in order to
                                    // put a notice of this acceptance for the
                                    // sender's records.
                                    OTLedger theFromOutbox(
                                        IDFromAccount,
                                        SERVER_ID), // Sender's *OUTBOX*
                                        theFromInbox(
                                            IDFromAccount,
                                            SERVER_ID); // Sender's *INBOX*

                                    bool bSuccessLoadingInbox =
                                        theFromInbox.LoadInbox();
                                    bool bSuccessLoadingOutbox =
                                        theFromOutbox.LoadOutbox();

                                    // THE FROM INBOX -- We are adding an item
                                    // here (acceptance of transfer),
                                    // so we will create this inbox if we have
                                    // to, so we can add that record to it.

                                    if (true == bSuccessLoadingInbox)
                                        bSuccessLoadingInbox =
                                            theFromInbox.VerifyAccount(
                                                server_->m_nymServer);
                                    else
                                        OTLog::Error("ERROR missing 'from' "
                                                     "inbox in "
                                                     "Notary::"
                                                     "NotarizeProcessInbox.\n");
                                    // THE FROM OUTBOX -- We are removing an
                                    // item, so this outbox SHOULD already
                                    // exist.

                                    if (true == bSuccessLoadingOutbox)
                                        bSuccessLoadingOutbox =
                                            theFromOutbox.VerifyAccount(
                                                server_->m_nymServer);
                                    else // If it does not already exist, that
                                         // is an error condition. For now, log
                                         // and fail.
                                        OTLog::Error("ERROR missing 'from' "
                                                     "outbox in "
                                                     "Notary::"
                                                     "NotarizeProcessInbox.\n");
                                    if (!bSuccessLoadingInbox ||
                                        false == bSuccessLoadingOutbox) {
                                        OTLog::Error("ERROR loading 'from' "
                                                     "inbox or outbox in "
                                                     "Notary::"
                                                     "NotarizeProcessInbox.\n");
                                    }
                                    else {
                                        // Generate a new transaction number for
                                        // the sender's inbox (to notice him of
                                        // acceptance.)
                                        int64_t lNewTransactionNumber = 0;
                                        server_->transactor_
                                            .issueNextTransactionNumber(
                                                server_->m_nymServer,
                                                lNewTransactionNumber,
                                                false); // bStoreTheNumber =
                                                        // false

                                        // Generate a new transaction... (to
                                        // notice the sender of acceptance.)
                                        OTTransaction* pInboxTransaction =
                                            OTTransaction::GenerateTransaction(
                                                theFromInbox,
                                                OTTransaction::transferReceipt,
                                                lNewTransactionNumber);

                                        if (nullptr == pInboxTransaction)
                                            OT_FAIL;

                                        // Here we give the sender (by dropping
                                        // into his inbox) a copy of my
                                        // acceptItem (for
                                        // his transfer), including the
                                        // transaction number of my acceptance
                                        // of his transfer.
                                        //
                                        pInboxTransaction->SetReferenceString(
                                            strInReferenceTo);
                                        pInboxTransaction->SetReferenceToNum(
                                            pItem
                                                ->GetTransactionNum()); // Right
                                                                        // now
                                                                        // this
                                                                        // has
                                                                        // the
                                        // 'accept
                                        // the
                                        // transfer'
                                        // transaction
                                        // number.
                                        // It could be changed to the original
                                        // transaction number, as a better
                                        // receipt for the original sender.
                                        // TODO? Decisions....

                                        pInboxTransaction->SetNumberOfOrigin(
                                            *pItem);

                                        // Now we have created a new transaction
                                        // from the server to the sender's inbox
                                        // Let's sign it and add to his inbox.
                                        pInboxTransaction->ReleaseSignatures();
                                        pInboxTransaction->SignContract(
                                            server_->m_nymServer);
                                        pInboxTransaction->SaveContract();

                                        // At this point I have theInbox ledger,
                                        // theFromOutbox ledger, theFromINBOX
                                        // ledger,
                                        // and theAccount.  So I should remove
                                        // the appropriate item from each
                                        // ledger, and
                                        // add the acceptance to the sender's
                                        // inbox, and credit the account....

                                        // First try to credit the amount to the
                                        // account...
                                        if (theAccount.Credit(
                                                pOriginalItem->GetAmount())) {
                                            // Add a transfer receipt to the
                                            // sender's inbox, containing the
                                            // "accept" transaction as the ref
                                            // string.
                                            // (to notify him that his transfer
                                            // was accepted; once he accepts it,
                                            // the trans# can be removed from
                                            // his issued list.)
                                            //
                                            theFromInbox.AddTransaction(
                                                *pInboxTransaction);

                                            // The original item carries the
                                            // transaction number that the
                                            // original
                                            // sender used to generate the
                                            // transfer in the first place. This
                                            // is the number
                                            // by which that transaction is
                                            // available in the sender's outbox.
                                            //
                                            // Then ANOTHER transaction was
                                            // created, by the server, in order
                                            // to put
                                            // a pending transfer into the
                                            // recipient's inbox. This has its
                                            // own transaction
                                            // number, generated by the server
                                            // at that time.
                                            //
                                            // So we remove the original
                                            // transfer from the sender's outbox
                                            // using the
                                            // transaction number on the
                                            // original item, and we remove the
                                            // pending transfer
                                            // from the recipient's inbox using
                                            // the transaction number from the
                                            // pending
                                            // transaction.

                                            // UPDATE: These two transactions
                                            // correspond to each other, so I am
                                            // now creating
                                            // them with the same transaction
                                            // number. As you can see, this
                                            // makes them easy
                                            // to remove as well.

                                            pServerTransaction
                                                ->DeleteBoxReceipt(
                                                    theFromOutbox); // faster.
                                            theFromOutbox.RemoveTransaction(
                                                pServerTransaction
                                                    ->GetTransactionNum());

                                            pServerTransaction
                                                ->DeleteBoxReceipt(
                                                    theInbox); // faster.
                                            theInbox.RemoveTransaction(
                                                pServerTransaction
                                                    ->GetTransactionNum());

                                            // NOTICE BTW, warning: Notice that
                                            // the box receipts are marked for
                                            // deletion
                                            // the instant they are removed from
                                            // their respective boxes.
                                            // Meanwhile, the client
                                            // may not have actually DOWNLOADED
                                            // those box receipts. Once they are
                                            // ACTUALLY
                                            // deleted, then client will never
                                            // have the chance. It's assumed
                                            // that client doesn't
                                            // care, since the receipts are
                                            // already out of his box.

                                            theFromInbox.ReleaseSignatures();
                                            theFromOutbox.ReleaseSignatures();

                                            theFromInbox.SignContract(
                                                server_->m_nymServer);
                                            theFromOutbox.SignContract(
                                                server_->m_nymServer);

                                            theFromInbox.SaveContract();
                                            theFromOutbox.SaveContract();

                                            theFromInbox.SaveInbox();
                                            theFromOutbox.SaveOutbox();

                                            // Release any signatures that were
                                            // there before (Old ones won't
                                            // verify anymore anyway, since the
                                            // content has changed.)
                                            theInbox.ReleaseSignatures();
                                            theInbox.SignContract(
                                                server_->m_nymServer);
                                            theInbox.SaveContract();
                                            theAccount.SaveInbox(theInbox);
                                            theAccount.ReleaseSignatures();
                                            theAccount.SignContract(
                                                server_->m_nymServer);
                                            theAccount.SaveContract();
                                            theAccount.SaveAccount();

                                            // Now we can set the response item
                                            // as an acknowledgement instead of
                                            // the default (rejection)
                                            // otherwise, if we never entered
                                            // this block, then it would still
                                            // be set to rejection, and the
                                            // new items would never have been
                                            // added to the inbox/outboxes, and
                                            // those files, along with
                                            // the account file, would never
                                            // have had their signatures
                                            // released, or been re-signed or
                                            // re-saved back to file.  The debit
                                            // failed, so all of those other
                                            // actions would fail also.
                                            // BUT... if the message comes back
                                            // with acknowledgement--then all of
                                            // these actions must have
                                            // happened, and here is the
                                            // server's signature to prove it.
                                            // Otherwise you get no items and no
                                            // signature. Just a rejection item
                                            // in the response transaction.
                                            pResponseItem->SetStatus(
                                                OTItem::acknowledgement);

                                            // This goes with the call above to
                                            // theFromInbox.AddTransaction().
                                            // Adding a receipt to any box, for
                                            // real, requires saving the box
                                            // receipt
                                            // as well. (Which is stored in a
                                            // separate file.)
                                            //
                                            pInboxTransaction->SaveBoxReceipt(
                                                theFromInbox);
                                        }
                                        else {
                                            delete pInboxTransaction;
                                            pInboxTransaction = nullptr;
                                            OTLog::Error(
                                                "Unable to credit account in "
                                                "Notary::"
                                                "NotarizeProcessInbox.\n");
                                        }
                                    } // outbox was successfully loaded
                                }     // its type is OTItem::transfer
                            }         // loaded original item from string
                            else {
                                OTLog::Error("Error loading original item from "
                                             "inbox transaction.\n");
                            }
                        }
                        else {
                            OTLog::vError("Error finding original receipt or "
                                          "transfer that client is trying to "
                                          "accept: %lld\n",
                                          pItem->GetReferenceToNum());
                        }

                        // sign the response item before sending it back (it's
                        // already been added to the transaction above)
                        // Now, whether it was rejection or acknowledgement, it
                        // is set properly and it is signed, and it
                        // is owned by the transaction, who will take it from
                        // here.
                        pResponseItem->SignContract(server_->m_nymServer);
                        pResponseItem->SaveContract();
                    }
                    else {
                        OTString strItemType;
                        pItem->GetTypeString(strItemType);

                        OTLog::vError("Notary::%s: Error, unexpected "
                                      "OTItem::itemType: %s\n",
                                      __FUNCTION__, strItemType.Get());
                    } // if type == ACCEPT, REJECT, DISPUTE
                }     // for LOOP (each item)
            }         // else (balance agreement verified.)
        }             // else bSuccessFindingAllTransactions = true
    }                 // Balance Agreement item found.

    // I put this here so it's signed/saved whether the balance agreement itself
    // was successful OR NOT.
    // (Or whether it even existed or not.)
    //
    pResponseBalanceItem->ReleaseSignatures();
    pResponseBalanceItem->SignContract(server_->m_nymServer);
    pResponseBalanceItem->SaveContract();
    tranOut.ReleaseSignatures();
    tranOut.SignContract(server_->m_nymServer);
    tranOut.SaveContract();
    OTString strPath; // SAVE THE RECEIPT TO LOCAL STORAGE (for dispute
                      // resolution.)

    // On the server side, response will only have chance to succeed if balance
    // agreement succeeds first.
    // Therefore, you will never see successful response but failed balance,
    // since it would stop at the
    // balance and response itself would remain failed with no chance of
    // changing.
    //
    // Thus, "success" must be when balance succeeded and transaction succeeded,
    // and "failure" must be when balance succeeded but transaction failed.
    //
    // If NEITHER succeeded, then there is no point recording it to a file, now
    // is there?

    const OTString strAcctID(ACCOUNT_ID);

    if (tranOut.GetSuccess()) {
        // Balance agreement was a success, AND process inbox was a success.
        // Therefore, remove any relevant issued numbers from theNym (those he's
        // now officially no longer responsible for), and save.
        //
        for (int32_t i = 0; i < theTempNym.GetIssuedNumCount(SERVER_ID); i++) {
            int64_t lTemp = theTempNym.GetIssuedNum(SERVER_ID, i);
            theNym.RemoveIssuedNum(server_->m_nymServer, server_->m_strServerID,
                                   lTemp, false); // bSave = false (saved below)
        }
        // The Nym (server side) stores a list of all opening and closing cron
        // #s.
        // So when the number is released from the Nym, we also take it off that
        // list.
        //
        std::set<int64_t>& theIDSet = theNym.GetSetOpenCronItems();
        for (int32_t i = 0;
             i < theTempClosingNumNym.GetIssuedNumCount(SERVER_ID); i++) {
            int64_t lTemp = theTempClosingNumNym.GetIssuedNum(SERVER_ID, i);
            theIDSet.erase(lTemp); // now it's erased from within the Nym.
        }
        theNym.SaveSignedNymfile(server_->m_nymServer);
        bOutSuccess = true; // the processInbox was successful.
        strPath.Format(const_cast<char*>("%s.success"), strAcctID.Get());
    }
    else
        strPath.Format(const_cast<char*>("%s.fail"), strAcctID.Get());

    const char* szFoldername = OTFolders::Receipt().Get();

    // Save the receipt. (My outgoing transaction including the client's signed
    // request that triggered it.)
    tranOut.SaveContract(szFoldername, strPath.Get());
}

} // namespace opentxs
