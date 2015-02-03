/************************************************************
 *
 *  MessageProcessor.cpp
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

#include <opentxs/server/ServerSettings.hpp>
#include <opentxs/server/ServerLoader.hpp>
#include <opentxs/server/MessageProcessor.hpp>
#include <opentxs/server/OTServer.hpp>
#include <opentxs/server/ClientConnection.hpp>
#include <opentxs/core/Log.hpp>
#include <opentxs/core/Message.hpp>
#include <opentxs/core/String.hpp>
#include <opentxs/core/OTSettings.hpp>
#include <opentxs/core/util/OTDataFolder.hpp>
#include <opentxs/core/crypto/OTEnvelope.hpp>
#include <opentxs/core/util/Timer.hpp>

#include <czmq.h>

namespace opentxs
{

MessageProcessor::MessageProcessor(ServerLoader& loader)
    : server_(loader.getServer())
    , zmqSocket_(zsock_new_rep(NULL))
    , zmqAuth_(zactor_new(zauth, NULL))
    , zmqPoller_(zpoller_new(zmqSocket_, NULL))
{
    init(loader.getPort());
}

MessageProcessor::~MessageProcessor()
{
    zpoller_remove(zmqPoller_, zmqSocket_);
    zpoller_destroy(&zmqPoller_);
    zactor_destroy(&zmqAuth_);
    zsock_destroy(&zmqSocket_);
}

void MessageProcessor::init(int port)
{
    if (port == 0) {
        OT_FAIL;
    }
    if (!zsys_has_curve()) {
        Log::vError("Error: libzmq has no libsodium support");
        OT_FAIL;
    }
    zstr_sendx(zmqAuth_, "CURVE", CURVE_ALLOW_ANY, NULL);
    zsock_wait(zmqAuth_);
    zsock_set_zap_domain(zmqSocket_, "global");
    zsock_set_curve_server(zmqSocket_, 1);
    // test key values taken from `man zmq_curve`
    zsock_set_curve_publickey(zmqSocket_,
                              "rq:rM>}U?@Lns47E1%kR.o@n%FcmmsL/@{H8]yf7");
    zsock_set_curve_secretkey(zmqSocket_,
                              "JTKVSB%%)wK0E.X)V>+}o?pNmC{O&4W4b!Ni{Lh6");
    zsock_bind(zmqSocket_, "tcp://*:%d", port);
}

void MessageProcessor::run()
{
    for (;;) {
        // timeout is the time left until the next cron should execute.
        int64_t timeout = server_->computeTimeout();
        if (timeout <= 0) {
            server_->ProcessCron();
            continue;
        }

        // wait for incoming message or up to timeout,
        // i.e. stop polling in time for the next cron execution.
        if (zpoller_wait(zmqPoller_, timeout)) {
            processSocket();
            continue;
        }
        if (zpoller_terminated(zmqPoller_)) {
            otErr << __FUNCTION__
                  << ": zpoller_terminated - process interrupted or"
                  << " parent context destroyed\n";
            break;
        }

        if (!zpoller_expired(zmqPoller_)) {
            otErr << __FUNCTION__ << ": zpoller_wait error\n";
            // we do not want busy loop if something goes wrong
            Log::SleepMilliseconds(100);
        }
    }
}

void MessageProcessor::processSocket()
{
    char* msg = zstr_recv(zmqSocket_);
    if (msg == nullptr) {
        Log::Error("zeromq recv() failed\n");
        return;
    }
    std::string requestString(msg);
    zstr_free(&msg);

    std::string responseString;

    bool error = processMessage(requestString, responseString);

    if (error) {
        responseString = "";
    }

    int rc = zstr_send(zmqSocket_, responseString.c_str());

    if (rc != 0) {
        Log::vError("MessageProcessor: failed to send response\n"
                    "request:\n%s\n\n"
                    "response:\n%s\n\n",
                    requestString.c_str(), responseString.c_str());
    }
}

bool MessageProcessor::processMessage(const std::string& messageString,
                                      std::string& reply)
{
    if (messageString.size() < 1) return false;

    // First we grab the client's message
    OTASCIIArmor ascMessage;
    ascMessage.MemSet(messageString.data(), messageString.size());

    OTEnvelope envelope;
    if (!envelope.SetAsciiArmoredData(ascMessage)) {
        Log::vError("Error retrieving envelope.\n");
        return true;
    }

    // Now the base64 is decoded and the envelope is in binary form again.
    Log::vOutput(2, "Successfully retrieved envelope from message.\n");

    // Decrypt the Envelope.
    String envelopeContents;
    if (!envelope.Open(server_->GetServerNym(), envelopeContents)) {
        Log::vError("Unable to open envelope.\n");
        return true;
    }

    // All decrypted--now let's load the results into an OTMessage.
    // No need to call message.ParseRawFile() after, since
    // LoadContractFromString handles it.
    Message message;
    if (!envelopeContents.Exists() ||
        !message.LoadContractFromString(envelopeContents)) {
        Log::vError("Error loading message from envelope "
                    "contents:\n\n%s\n\n",
                    envelopeContents.Get());
        return true;
    }

    Message replyMessage;
    replyMessage.m_strCommand.Format("%sResponse", message.m_strCommand.Get());
    // NymID
    replyMessage.m_strNymID = message.m_strNymID;
    // NotaryID, a hash of the server contract
    replyMessage.m_strNotaryID = message.m_strNotaryID;
    // The default reply. In fact this is probably superfluous
    replyMessage.m_bSuccess = false;

    ClientConnection client;
    Nym nym(message.m_strNymID);

    bool processedUserCmd = server_->userCommandProcessor_.ProcessUserCommand(
        message, replyMessage, &client, &nym);

    // By optionally passing in &client, the client Nym's public
    // key will be set on it whenever verification is complete. (So
    // for the reply, I'll  have the key and thus I'll be able to
    // encrypt reply to the recipient.)
    if (!processedUserCmd) {
        String s1(message);

        Log::vOutput(0, "Unable to process user command: %s\n ********** "
                        "REQUEST:\n\n%s\n\n",
                     message.m_strCommand.Get(), s1.Get());

        // NOTE: normally you would even HAVE a true or false if
        // we're in this block. ProcessUserCommand()
        // is what tries to process a command and then sets false
        // if/when it fails. Until that point, you
        // wouldn't get any server reply.  I'm now changing this
        // slightly, so you still get a reply (defaulted
        // to success==false.) That way if a client needs to re-sync
        // his request number, he will get the false
        // and therefore know to resync the # as his next move, vs
        // being stuck with no server reply (and thus
        // stuck with a bad socket.)
        // We sign the reply here, but not in the else block, since
        // it's already signed in cases where
        // ProcessUserCommand() is a success, by the time that call
        // returns.

        // Since the process call definitely failed, I'm
        replyMessage.m_bSuccess = false;
        // making sure this here is definitely set to
        // false (even though it probably was already.)
        replyMessage.SignContract(server_->GetServerNym());
        replyMessage.SaveContract();

        String s2(replyMessage);

        Log::vOutput(0, " ********** RESPONSE:\n\n%s\n\n", s2.Get());
    }
    else {
        // At this point the reply is ready to go, and client
        // has the public key of the recipient...
        Log::vOutput(1, "Successfully processed user command: %s.\n",
                     message.m_strCommand.Get());
    }

    // IF ProcessUserCommand returned true, THEN we process the
    // message for the recipient.
    // ELSE IF ProcessUserCommand returned false, YET the PubKey
    // DOES exist, THEN in this case also, we process the message
    // for the recipient.
    //
    // if success + Nym's pub key exists here on server.
    if (processedUserCmd || nym.Server_PubKeyExists()) {
        // The transaction is now processed (or not), and the
        // server's reply message is in replyMessage.
        // Let's seal it up to the recipient's nym (in an envelope)
        // and send back to the user...
        OTEnvelope recipientEnvelope;

        bool sealed =
            client.SealMessageForRecipient(replyMessage, recipientEnvelope);

        if (!sealed) {
            Log::vOutput(0, "Unable to seal envelope. (No reply will be "
                            "sent.)\n");
            return true;
        }

        OTASCIIArmor ascReply;
        if (!recipientEnvelope.GetAsciiArmoredData(ascReply)) {
            Log::vOutput(
                0,
                "Unable to GetAsciiArmoredData from sealed "
                "envelope int oOTASCIIArmor object. (No reply envelope will be "
                "sent.)\n");
            return true;
        }

        String output;
        bool val = ascReply.Exists() &&
                   ascReply.WriteArmoredString(output, "ENVELOPE");

        if (!val || !output.Exists()) {
            Log::vOutput(0,
                         "Unable to WriteArmoredString from "
                         "OTASCIIArmor object into OTString object. (No reply "
                         "envelope will be sent.)\n");
            return true;
        }

        reply.assign(output.Get(), output.GetLength());
    }
    // ELSE we send the message in the CLEAR. (As an armored
    // message, instead of as an armored envelope.)
    else {
        String replyString(replyMessage);

        if (!replyString.Exists()) {
            Log::vOutput(0, "Failed trying to grab the reply "
                            "in OTString form. "
                            "(No reply message will be sent.)\n");
            return true;
        }

        OTASCIIArmor ascReply(replyString);
        String output;
        bool val =
            ascReply.Exists() && ascReply.WriteArmoredString(output, "MESSAGE");

        if (!val || !output.Exists()) {
            Log::vOutput(0,
                         "Unable to WriteArmoredString from "
                         "OTASCIIArmor object into OTString object. (No reply "
                         "message will be sent.)\n");
            return true;
        }

        reply.assign(output.Get(), output.GetLength());
    }

    return false;
}

} // namespace opentxs
