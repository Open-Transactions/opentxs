/************************************************************
 *
 *  PayDividendVisitor.cpp
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

#include <opentxs/server/PayDividendVisitor.hpp>
#include <opentxs/server/OTServer.hpp>

#include <opentxs/core/OTAccount.hpp>
#include <opentxs/core/OTCheque.hpp>
#include <opentxs/core/OTLog.hpp>
#include <opentxs/core/OTString.hpp>
#include <opentxs/ext/OTPayment.hpp>

namespace opentxs
{

PayDividendVisitor::PayDividendVisitor(
    const OTIdentifier& theServerID, const OTIdentifier& theUserID,
    const OTIdentifier& thePayoutAssetID, const OTIdentifier& theVoucherAcctID,
    const OTString& strMemo, OTServer& theServer, int64_t lPayoutPerShare,
    mapOfAccounts* pLoadedAccounts)
    : AccountVisitor(theServerID, pLoadedAccounts)
    , m_pUserID(new OTIdentifier(theUserID))
    , m_pPayoutAssetID(new OTIdentifier(thePayoutAssetID))
    , m_pVoucherAcctID(new OTIdentifier(theVoucherAcctID))
    , m_pstrMemo(new OTString(strMemo))
    , m_pServer(&theServer)
    , m_lPayoutPerShare(lPayoutPerShare)
    , m_lAmountPaidOut(0)
    , m_lAmountReturned(0)
{
}

PayDividendVisitor::~PayDividendVisitor()
{
    if (nullptr != m_pUserID) delete m_pUserID;
    m_pUserID = nullptr;
    if (nullptr != m_pPayoutAssetID) delete m_pPayoutAssetID;
    m_pPayoutAssetID = nullptr;
    if (nullptr != m_pVoucherAcctID) delete m_pVoucherAcctID;
    m_pVoucherAcctID = nullptr;
    if (nullptr != m_pstrMemo) delete m_pstrMemo;
    m_pstrMemo = nullptr;
    m_pServer = nullptr; // don't delete this one (I don't own it.)
    m_lPayoutPerShare = 0;
    m_lAmountPaidOut = 0;
    m_lAmountReturned = 0;
}

// For each "simple" account of a specific asset type, this function
// is called in order to pay a dividend to the Nym who owns that account.

// PayDividendVisitor::Trigger() is used in
// OTAssetContract::VisitAccountRecords()
// cppcheck-suppress unusedFunction
bool PayDividendVisitor::Trigger(
    OTAccount& theSharesAccount) // theSharesAccount is, say, a Pepsi shares
                                 // account.  Here, we'll send a dollars voucher
                                 // to its owner.
{
    const int64_t lPayoutAmount =
        (theSharesAccount.GetBalance() * GetPayoutPerShare());

    if (lPayoutAmount <= 0) {
        OTLog::Output(0, "PayDividendVisitor::Trigger: nothing to pay, "
                         "since this account owns no shares. (Returning "
                         "true.)");
        return true; // nothing to pay, since this account owns no shares.
                     // Success!
    }
    OT_ASSERT(nullptr != GetServerID());
    const OTIdentifier& theServerID = *(GetServerID());
    OT_ASSERT(nullptr != GetPayoutAssetID());
    const OTIdentifier& thePayoutAssetID = *(GetPayoutAssetID());
    OT_ASSERT(nullptr != GetVoucherAcctID());
    const OTIdentifier& theVoucherAcctID = *(GetVoucherAcctID());
    OT_ASSERT(nullptr != GetServer());
    OTServer& theServer = *(GetServer());
    OTPseudonym& theServerNym =
        const_cast<OTPseudonym&>(theServer.GetServerNym());
    const OTIdentifier theServerNymID(theServerNym);
    const OTIdentifier& RECIPIENT_ID = theSharesAccount.GetUserID();
    OT_ASSERT(nullptr != GetUserID());
    const OTIdentifier& theSenderUserID = *(GetUserID());
    OT_ASSERT(nullptr != GetMemo());
    const OTString& strMemo = *(GetMemo());
    // Note: theSenderUserID is the originator of the Dividend Payout.
    // However, all the actual vouchers will be from "the server Nym" and
    // not from theSenderUserID. So then why is it even here? Because anytime
    // there's an error, the server will send to theSenderUserID instead of
    // RECIPIENT_ID (so the original sender can have his money back, instead of
    // just having it get lost in the ether.)
    bool bReturnValue = false;

    OTCheque theVoucher(theServerID, thePayoutAssetID);

    // 10 minutes ==    600 Seconds
    // 1 hour    ==     3600 Seconds
    // 1 day    ==    86400 Seconds
    // 30 days    ==  2592000 Seconds
    // 3 months ==  7776000 Seconds
    // 6 months == 15552000 Seconds

    const time64_t VALID_FROM =
        OTTimeGetCurrentTime(); // This time is set to TODAY NOW
    const time64_t VALID_TO = OTTimeAddTimeInterval(
        VALID_FROM, OTTimeGetSecondsFromTime(
                        OT_TIME_SIX_MONTHS_IN_SECONDS)); // This time occurs in
                                                         // 180 days (6 months).
                                                         // Todo hardcoding.

    int64_t lNewTransactionNumber = 0;

    bool bGotNextTransNum = theServer.transactor_.issueNextTransactionNumber(
        theServerNym, lNewTransactionNumber); // bStoreTheNumber defaults to
                                              // true. We save the transaction
    // number on the server Nym (normally we'd discard it) because
    // when the cheque is deposited, the server nym, as the owner of
    // the voucher account, needs to verify the transaction # on the
    // cheque (to prevent double-spending of cheques.)
    if (bGotNextTransNum) {
        const bool bIssueVoucher = theVoucher.IssueCheque(
            lPayoutAmount,         // The amount of the cheque.
            lNewTransactionNumber, // Requiring a transaction number prevents
                                   // double-spending of cheques.
            VALID_FROM, // The expiration date (valid from/to dates) of the
                        // cheque
            VALID_TO, // Vouchers are automatically starting today and lasting 6
                      // months.
            theVoucherAcctID, // The asset account the cheque is drawn on.
            theServerNymID,   // User ID of the sender (in this case the server
                              // nym.)
            strMemo, // Optional memo field. Includes item note and request
                     // memo.
            &RECIPIENT_ID);

        // All account crediting / debiting happens in the caller, in OTServer.
        //    (AND it happens only ONCE, to cover ALL vouchers.)
        // Then in here, the voucher either gets send to the recipient, or if
        // error, sent back home to
        // the issuer Nym. (ALL the funds are removed, then the vouchers are
        // sent one way or the other.)
        // Any returned vouchers, obviously serve to notify the dividend payer
        // of where the errors were
        // (as well as give him the opportunity to get his money back.)
        //
        bool bSent = false;
        if (bIssueVoucher) {
            // All this does is set the voucher's internal contract string to
            // "VOUCHER" instead of "CHEQUE". We also set the server itself as
            // the remitter, which is unusual for vouchers, but necessary in the
            // case of dividends.
            //
            theVoucher.SetAsVoucher(theServerNymID, theVoucherAcctID);
            theVoucher.SignContract(theServerNym);
            theVoucher.SaveContract();

            // Send the voucher to the payments inbox of the recipient.
            //
            const OTString strVoucher(theVoucher);
            OTPayment thePayment(strVoucher);

            // calls DropMessageToNymbox
            bSent = theServer.SendInstrumentToNym(
                theServerID, theServerNymID,          // sender nym
                RECIPIENT_ID,                         // recipient nym
                nullptr, &thePayment, "payDividend"); // todo: hardcoding.
            bReturnValue = bSent;                     // <======= RETURN VALUE.
            if (bSent)
                m_lAmountPaidOut +=
                    lPayoutAmount; // At the end of iterating all accounts, if
                                   // m_lAmountPaidOut is less than
                                   // lTotalPayoutAmount, then we return to rest
                                   // to the sender.
        }
        else {
            const OTString strPayoutAssetID(thePayoutAssetID),
                strRecipientUserID(RECIPIENT_ID);
            OTLog::vError(
                "PayDividendVisitor::Trigger: ERROR failed "
                "issuing voucher (to send to dividend payout "
                "recipient.) "
                "WAS TRYING TO PAY %lld of asset type %s to Nym %s.\n",
                lPayoutAmount, strPayoutAssetID.Get(),
                strRecipientUserID.Get());
        }
        // If we didn't send it, then we need to return the funds to where they
        // came from.
        //
        if (!bSent) {
            OTCheque theReturnVoucher(theServerID, thePayoutAssetID);

            const bool bIssueReturnVoucher = theReturnVoucher.IssueCheque(
                lPayoutAmount,         // The amount of the cheque.
                lNewTransactionNumber, // Requiring a transaction number
                                       // prevents double-spending of cheques.
                VALID_FROM, // The expiration date (valid from/to dates) of the
                            // cheque
                VALID_TO,   // Vouchers are automatically starting today and
                            // lasting 6 months.
                theVoucherAcctID, // The asset account the cheque is drawn on.
                theServerNymID,   // User ID of the sender (in this case the
                                  // server nym.)
                strMemo, // Optional memo field. Includes item note and request
                         // memo.
                &theSenderUserID); // We're returning the money to its original
                                   // sender.
            if (bIssueReturnVoucher) {
                // All this does is set the voucher's internal contract string
                // to
                // "VOUCHER" instead of "CHEQUE".
                //
                theReturnVoucher.SetAsVoucher(theServerNymID, theVoucherAcctID);
                theReturnVoucher.SignContract(theServerNym);
                theReturnVoucher.SaveContract();

                // Return the voucher back to the payments inbox of the original
                // sender.
                //
                const OTString strReturnVoucher(theReturnVoucher);
                OTPayment theReturnPayment(strReturnVoucher);

                // calls DropMessageToNymbox
                bSent = theServer.SendInstrumentToNym(
                    theServerID, theServerNymID, // sender nym
                    theSenderUserID, // recipient nym (original sender.)
                    nullptr, &theReturnPayment,
                    "payDividend"); // todo: hardcoding.
                if (bSent)
                    m_lAmountReturned +=
                        lPayoutAmount; // At the end of iterating all accounts,
                                       // if m_lAmountPaidOut+m_lAmountReturned
                                       // is less than lTotalPayoutAmount, then
                                       // we return the rest to the sender.
            }
            else {
                const OTString strPayoutAssetID(thePayoutAssetID),
                    strSenderUserID(theSenderUserID);
                OTLog::vError(
                    "PayDividendVisitor::Trigger: ERROR "
                    "failed issuing voucher (to return back to "
                    "the dividend payout initiator, after a failed "
                    "payment attempt to the originally intended "
                    "recipient.) WAS TRYING TO PAY %lld of asset type "
                    "%s to Nym %s.\n",
                    lPayoutAmount, strPayoutAssetID.Get(),
                    strSenderUserID.Get());
            }
        }  // if !bSent
    }
    else // !bGotNextTransNum
    {
        const OTString strPayoutAssetID(thePayoutAssetID),
            strRecipientUserID(RECIPIENT_ID);
        OTLog::vError(
            "PayDividendVisitor::Trigger: ERROR!! Failed issuing next "
            "transaction "
            "number while trying to send a voucher (while paying dividends.) "
            "WAS TRYING TO PAY %lld of asset type %s to Nym %s.\n",
            lPayoutAmount, strPayoutAssetID.Get(), strRecipientUserID.Get());
    }

    return bReturnValue;
}

} // namespace opentxs
