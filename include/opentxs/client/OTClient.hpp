/************************************************************
 *
 *  OTClient.hpp
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

#ifndef OPENTXS_CLIENT_OTCLIENT_HPP
#define OPENTXS_CLIENT_OTCLIENT_HPP

#include <opentxs/client/OTMessageBuffer.hpp>
#include <opentxs/client/OTMessageOutbuffer.hpp>
#include <opentxs/client/OTServerConnection.hpp>
#include <memory>

namespace opentxs
{

class Account;
class AssetContract;
class Ledger;
class OTServerContract;
class OTWallet;

class OTClient
{
public:
    enum OT_CLIENT_CMD_TYPE {
        pingNotary, // Your public key is sent along with this message so the
                    // server can reply to
        // you even without your being a registered user. Other than these top
        // two commands,
        // all other commands can only be executed by registered users.
        //
        // The server ID is a hash of the server contract. The signature on the
        // contract
        // can be verified by a public key that appears in a standard section of
        // any server
        // contract. The URL/port information is also derived from the contract.
        //
        // Simply by importing the server contract into your wallet, you are
        // able to connect
        // to it and encrypt all of your communications to it.
        //
        // Thus, the check server ID command really just confirms what you
        // should already know...
        // Your wallet still wants to see that the server agrees with the server
        // ID, and that
        // the server is able to read messages that were encrypted to the public
        // key in the
        // contract, and that the server is able to sign all of its future
        // correspondence with
        // the same public key.
        //
        // It is the server operator's responsibility to secure the domain name
        // and web host
        // that users will connect to when they import the contract, as well as
        // the private
        // key that matches the public key from the contract.
        registerNym,      // register user account on a specific server, with
                          // public key. Nym ID will be hash of said public
                          // key.
        unregisterNym,    // Delete user account from a specific server.
        getRequestNumber, // Get the next request number from the server (for
                          // this
                          // user). Most requests must be
        // accompanied by a request number, which increments for each Nym with
        // each request.
        getTransactionNumbers, // Every transaction requires a transaction
                               // number.
                               // If your wallet doesn't have one,
        // then here it can request the server to send one over. (Or several.)
        processNymbox, // Used by AcceptEntireNymbox() as it's setting
                       // everything up.
        writeCheque, // Write a cheque. (Actually sends no message to the server
                     // -- returns false.)
        notarizePurse, // Same as the above, but sends an entire purse of tokens
                       // at once instead of sending individual tokens.
        notarizeCheque, // Deposit like the above, but deposits a cheque instead
                        // of cash tokens.
        paymentPlan, // Send a payment plan to the server (request to activate
                     // one onto yourself, basically.)
        // The test client will ask you to input the plan, which you must
        // already have (like a cheque).
        // The Payee must create it and sign it, then he sends it to the Payer,
        // who uses this command
        // to sign it and submit it to the server.
        badID
    };

public:
    explicit OTClient(OTWallet* theWallet);

    bool connect(OTServerContract* contract, Nym* nym);

    inline OTMessageBuffer& GetMessageBuffer()
    {
        return m_MessageBuffer;
    }

    inline OTMessageOutbuffer& GetMessageOutbuffer()
    {
        return m_MessageOutbuffer;
    }

    void ProcessMessageOut(OTServerContract* pServerContract, Nym* pNym,
                           const Message& theMessage);
    bool ProcessInBuffer(const Message& theServerReply) const;

    EXPORT int32_t ProcessUserCommand(OT_CLIENT_CMD_TYPE requestedCommand,
                                      Message& theMessage, Nym& theNym,
                                      const OTServerContract& theServer,
                                      const Account* pAccount = nullptr,
                                      int64_t lTransactionAmount = 0,
                                      AssetContract* pMyAssetContract = nullptr,
                                      const Identifier* pHisNymID = nullptr,
                                      const Identifier* pHisAcctID = nullptr);

    bool processServerReply(std::shared_ptr<Message> theReply,
                            Ledger* pNymbox = nullptr);

    bool AcceptEntireNymbox(Ledger& theNymbox, const Identifier& theNotaryID,
                            const OTServerContract& theServerContract,
                            Nym& theNym, Message& theMessage);

private:
    void ProcessIncomingTransactions(OTServerConnection& theConnection,
                                     const Message& theReply) const;
    void ProcessWithdrawalResponse(OTTransaction& theTransaction,
                                   const OTServerConnection& theConnection,
                                   const Message& theReply) const;
    void ProcessDepositResponse(OTTransaction& theTransaction,
                                const OTServerConnection& theConnection,
                                const Message& theReply) const;
    void ProcessPayDividendResponse(OTTransaction& theTransaction,
                                    const OTServerConnection& theConnection,
                                    const Message& theReply) const;

    void load_str_trans_add_to_ledger(const Identifier& the_nym_id,
                                      const String& str_trans,
                                      String str_box_type,
                                      const int64_t& lTransNum, Nym& the_nym,
                                      Ledger& ledger) const;

    struct ProcessServerReplyArgs;
    void setRecentHash(const Message& theReply, const String& strNotaryID,
                       Nym* pNym, bool setNymboxHash);
    bool processServerReplyTriggerClause(const Message& theReply,
                                         ProcessServerReplyArgs& args);
    bool processServerReplyGetRequestNumber(const Message& theReply,
                                            ProcessServerReplyArgs& args);
    bool processServerReplyCheckNym(const Message& theReply,
                                    ProcessServerReplyArgs& args);
    bool processServerReplyNotarizeTransaction(const Message& theReply,
                                               ProcessServerReplyArgs& args);
    bool processServerReplyGetTransactionNumbers(const Message& theReply,
                                                 ProcessServerReplyArgs& args);
    bool processServerReplyGetNymBox(const Message& theReply, Ledger* pNymbox,
                                     ProcessServerReplyArgs& args);
    bool processServerReplyGetBoxReceipt(const Message& theReply,
                                         Ledger* pNymbox,
                                         ProcessServerReplyArgs& args);
    bool processServerReplyProcessInbox(const Message& theReply,
                                        Ledger* pNymbox,
                                        ProcessServerReplyArgs& args);
    bool processServerReplyGetAccountData(const Message& theReply,
                                          Ledger* pNymbox,
                                          ProcessServerReplyArgs& args);
    bool processServerReplyGetInstrumentDefinition(
        const Message& theReply, ProcessServerReplyArgs& args);
    bool processServerReplyGetMint(const Message& theReply);
    bool processServerReplyGetMarketList(const Message& theReply);
    bool processServerReplyGetMarketOffers(const Message& theReply);
    bool processServerReplyGetMarketRecentTrades(const Message& theReply);
    bool processServerReplyGetNymMarketOffers(const Message& theReply);
    bool processServerReplyUnregisterNym(const Message& theReply,
                                         ProcessServerReplyArgs& args);
    bool processServerReplyUnregisterAccount(const Message& theReply,
                                             ProcessServerReplyArgs& args);
    bool processServerReplyRegisterInstrumentDefinition(
        const Message& theReply, ProcessServerReplyArgs& args);
    bool processServerReplyRegisterAccount(const Message& theReply,
                                           ProcessServerReplyArgs& args);

private:
    std::unique_ptr<OTServerConnection> m_pConnection;
    OTWallet* m_pWallet;
    OTMessageBuffer m_MessageBuffer;
    OTMessageOutbuffer m_MessageOutbuffer;
};

} // namespace opentxs

#endif // OPENTXS_CLIENT_OTCLIENT_HPP
