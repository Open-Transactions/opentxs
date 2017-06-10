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

#include "opentxs/core/stdafx.hpp"

#include "opentxs/server/UserCommandProcessor.hpp"

#include "opentxs/api/Identity.hpp"
#include "opentxs/api/OT.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/cash/Mint.hpp"
#include "opentxs/consensus/ClientContext.hpp"
#include "opentxs/core/contract/basket/BasketContract.hpp"
#include "opentxs/core/cron/OTCron.hpp"
#include "opentxs/core/cron/OTCronItem.hpp"
#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/crypto/OTAsymmetricKey.hpp"
#include "opentxs/core/script/OTParty.hpp"
#include "opentxs/core/script/OTScriptable.hpp"
#include "opentxs/core/script/OTSmartContract.hpp"
#include "opentxs/core/trade/OTMarket.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/OTFolders.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Item.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/NumList.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/server/ClientConnection.hpp"
#include "opentxs/server/Macros.hpp"
#include "opentxs/server/MainFile.hpp"
#include "opentxs/server/Notary.hpp"
#include "opentxs/server/OTServer.hpp"
#include "opentxs/server/ServerSettings.hpp"
#include "opentxs/server/Transactor.hpp"

#include <inttypes.h>
#include <stdint.h>
#include <memory>
#include <set>
#include <string>

#define OT_METHOD "opentxs::UserCommandProcessor::"

namespace opentxs
{

UserCommandProcessor::UserCommandProcessor(OTServer* server)
    : server_(server)
{
}

bool UserCommandProcessor::ProcessUserCommand(
    Message& theMessage,
    Message& msgOut,
    ClientConnection* pConnection)
{
    msgOut.m_strRequestNum.Set(theMessage.m_strRequestNum);

    if (ServerSettings::__admin_server_locked &&
        ((ServerSettings::GetOverrideNymID().size() <=
          0) ||  // AND (there's no Override Nym ID listed --OR-- the Override
                 // Nym ID doesn't
         (0 !=
          ServerSettings::GetOverrideNymID().compare(
              (theMessage.m_strNymID.Get())))))  // match the Nym's ID who
                                                 // sent this message)
    {
        Log::vOutput(
            0,
            "UserCommandProcessor::ProcessUserCommand: Nym %s: failed "
            "attempt to message the server, while server is in "
            "**LOCK DOWN MODE**.\n",
            theMessage.m_strNymID.Get());
        return false;
    }

    // Validate the server ID, to keep users from intercepting a valid requst
    // and sending it to the wrong server.
    if (!(server_->m_strNotaryID == theMessage.m_strNotaryID)) {
        Log::Error("UserCommandProcessor::ProcessUserCommand: Invalid server "
                   "ID sent in "
                   "command request.\n");
        return false;
    } else {
        Log::Output(4, "Received valid Notary ID with command request.\n");
    }

    Nym theNym(theMessage.m_strNymID);

    // NYM IS ACTUALLY SERVER
    //
    // The server nym is not allowed to act as a client.
    const bool bNymIsServerNym =
        server_->m_strServerNymID.Compare(theMessage.m_strNymID);

    if (bNymIsServerNym) {
        otErr << __FUNCTION__ << ": Server nym is not allowed to act as a "
              << "client." << std::endl;

        return false;
    }

    String strMsgNymID;
    theNym.GetIdentifier(strMsgNymID);

    if (theMessage.m_strCommand.Compare("pingNotary")) {
        Log::vOutput(
            0,
            "\n==> Received a pingNotary message. Nym: %s ...\n",
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_check_notary_id);

        proto::AsymmetricKeyType keytypeAuthent =
            static_cast<proto::AsymmetricKeyType>(theMessage.keytypeAuthent_);

        proto::AsymmetricKeyType keytypeEncrypt =
            static_cast<proto::AsymmetricKeyType>(theMessage.keytypeEncrypt_);

        OTAsymmetricKey* nymAuthentKey = OTAsymmetricKey::KeyFactory(
            keytypeAuthent, theMessage.m_strNymPublicKey);

        if (nullptr == nymAuthentKey) {
            return false;
        }

        OTAsymmetricKey* nymEncryptKey =
            OTAsymmetricKey::KeyFactory(keytypeEncrypt, theMessage.m_strNymID2);

        if (nullptr == nymEncryptKey) {
            return false;
        }

        // Not all contracts are signed with the authentication key, but
        // messages are.
        if (!theMessage.VerifyWithKey(*nymAuthentKey)) {
            Log::Output(0, "pingNotary: Signature verification failed!\n");
            return false;
        }
        Log::Output(
            3,
            "Signature verified! The message WAS signed by "
            "the Private Authentication Key inside the message.\n");

        // This is only for verified Nyms, (and we're verified in here!)
        // We do this so that
        // we have the option later to encrypt the replies back to the
        // client...(using the
        // client's public key that we set here.)

        if (nullptr != pConnection)
            pConnection->SetPublicKey(
                *nymEncryptKey);  // theMessage.m_strNymID2
                                  // contains the public
                                  // encryption key for sending
                                  // an encrypted reply.

        UserCmdPingNotary(theNym, theMessage, msgOut);
        return true;
    } else if (theMessage.m_strCommand.Compare("registerNym")) {
        Log::vOutput(
            0,
            "\n==> Received a registerNym message. Nym: %s ...\n",
            strMsgNymID.Get());
        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_create_user_acct);
        auto serialized = proto::DataToProto<proto::CredentialIndex>(
            Data(theMessage.m_ascPayload));
        auto nym = OT::App().Contract().Nym(serialized);

        if (!nym) {
            Log::vError(
                "%s: registerNymResponse: Invalid nym %s \n",
                __FUNCTION__,
                serialized.nymid().c_str());
        } else {
            theNym.LoadCredentialIndex(nym->asPublicNym());
            Log::Output(3, "Pseudonym verified!\n");
            // Okay, now that the Nym is verified, let's verify the
            // message itself...
            //
            if (false == theMessage.VerifySignature(theNym))  // FYI, OTMessage
                                                              // overrides
            // VerifySignature with
            // VerifySigAuthent.
            {  // (Because we use authentication keys, not signing
                // keys, for messages.)
                Log::Output(
                    0,
                    "registerNymResponse: "
                    "Authentication signature -- "
                    "verification failed!\n");
                return false;
            }
            Log::Output(
                3,
                "Signature verified! The message WAS signed by "
                "the Nym\'s private authentication key.\n");

            // Make sure we are encrypting the message we send
            // back, if possible.
            String strPublicEncrKey, strPublicSignKey;
            OTAsymmetricKey& thePublicEncrKey =
                const_cast<OTAsymmetricKey&>(theNym.GetPublicEncrKey());
            OTAsymmetricKey& thePublicSignKey =
                const_cast<OTAsymmetricKey&>(theNym.GetPublicSignKey());

            thePublicEncrKey.GetPublicKey(strPublicEncrKey);
            thePublicSignKey.GetPublicKey(strPublicSignKey);

            // This is only for verified Nyms, (and we're
            // verified in here!) We do this so that
            // we have the option later to encrypt the replies
            // back to the client...(using the
            // client's public key that we set here.)
            if (strPublicEncrKey.Exists() && (nullptr != pConnection))
                pConnection->SetPublicKey(thePublicEncrKey);
            // Look up the NymID and see if it's already a valid
            // user account.
            //
            // If it is, then we can't very well create it
            // twice, can we?
            //
            // UPDATE: Actually we should, in such cases, just
            // return true with
            // a copy of the Nymfile. Helps prevent sync errors,
            // and gives people
            // a way to grab the server's copy of their nymfile,
            // if they need it.
            //
            Log::Output(
                0,
                "Verifying account doesn't already exist... "
                "(IGNORE ANY ERRORS, IMMEDIATELY BELOW, "
                "ABOUT FAILURE OPENING FILES)\n");

            // Prepare to send success or failure back to user.
            // (1) set up member variables

            // reply to registerNym
            msgOut.m_strCommand = "registerNymResponse";
            msgOut.m_strNymID = theMessage.m_strNymID;  // NymID
            msgOut.m_strNotaryID =
                server_->m_strNotaryID;  // NotaryID, a hash of
                                         // the server
                                         // contract.
            msgOut.m_bSuccess = false;

            // We send the user's message back to him, ascii-armored, as part of
            // our response.
            String tempInMessage;
            theMessage.SaveContractRaw(tempInMessage);
            msgOut.m_ascInReferenceTo.SetString(tempInMessage);

            auto context = OT::App().Contract().mutable_ClientContext(
                server_->GetServerNym().ID(), theNym.ID());

            bool bLoadedSignedNymfile =
                theNym.LoadSignedNymfile(server_->m_nymServer);

            // The below block is for the case where the Nym is re-registering,
            // even though he's already registered on this Notary.
            //
            // He ALREADY exists. We'll set success to true, and
            // send him a copy of his own nymfile.
            // (Signature is verified already anyway, by this
            // point.)
            //
            if (bLoadedSignedNymfile &&
                (false == theNym.IsMarkedForDeletion())) {
                Log::vOutput(
                    0,
                    "(Allowed in order to prevent sync issues) "
                    "==> User is registering nym that already "
                    "exists: %s\n",
                    theMessage.m_strNymID.Get());

                String strNymContents;
                theNym.SavePseudonym(strNymContents);
                Identifier theNewNymID, NOTARY_ID(server_->m_strNotaryID);
                theNym.GetIdentifier(theNewNymID);
                Ledger theNymbox(theNewNymID, theNewNymID, NOTARY_ID);
                bool bSuccessLoadingNymbox = theNymbox.LoadNymbox();

                if (true == bSuccessLoadingNymbox)
                    bSuccessLoadingNymbox =
                        (theNymbox.VerifyContractID() &&
                         theNymbox.VerifyAccount(server_->m_nymServer));
                // (No need here to load all the Box Receipts)
                else {
                    bSuccessLoadingNymbox = theNymbox.GenerateLedger(
                        theNewNymID, NOTARY_ID, Ledger::nymbox, true);

                    if (bSuccessLoadingNymbox) {
                        bSuccessLoadingNymbox =
                            theNymbox.SignContract(server_->m_nymServer);

                        if (bSuccessLoadingNymbox) {
                            bSuccessLoadingNymbox = theNymbox.SaveContract();

                            if (bSuccessLoadingNymbox)
                                bSuccessLoadingNymbox = theNymbox.SaveNymbox();
                        }
                    }
                }

                // by this point, the nymbox DEFINITELY "exists or not"
                // (Generation might have failed, or verification.)
                //
                if (!bSuccessLoadingNymbox) {
                    Log::vError(
                        "Error during user account "
                        "re-registration. Failed "
                        "verifying or generating "
                        "nymbox for user: %s\n",
                        theMessage.m_strNymID.Get());
                } else {
                    Identifier nymboxHash;
                    theNymbox.CalculateNymboxHash(nymboxHash);
                    auto context = OT::App().Contract().mutable_ClientContext(
                        server_->GetServerNym().ID(), theNym.ID());
                    context.It().SetLocalNymboxHash(nymboxHash);
                }

                msgOut.m_ascPayload.SetString(strNymContents);
                msgOut.m_bSuccess = bSuccessLoadingNymbox;
                msgOut.SignContract(server_->m_nymServer);
                msgOut.SaveContract();

                return true;
            }
            // The above block is for the case where the Nym is re-registering,
            // even though he's already registered on this Notary.

            if (theNym.IsMarkedForDeletion()) theNym.MarkAsUndeleted();

            // Good -- this means the account doesn't already exist.
            // Let's create it.

            msgOut.m_bSuccess = theMessage.SaveContract(
                OTFolders::UserAcct().Get(), theMessage.m_strNymID.Get());

            // First we save the registerNym message
            // in the accounts folder...
            if (!msgOut.m_bSuccess) {
                Log::Error("Error saving new user "
                           "account verification file.\n");
                msgOut.SignContract(server_->m_nymServer);
                msgOut.SaveContract();
                return true;
            }

            Log::Output(
                0,
                "Success saving new user "
                "account verification file.\n");

            Identifier theNewNymID, NOTARY_ID(server_->m_strNotaryID);
            theNym.GetIdentifier(theNewNymID);
            Ledger theNymbox(theNewNymID, theNewNymID, NOTARY_ID);
            bool bSuccessLoadingNymbox = theNymbox.LoadNymbox();

            if (true == bSuccessLoadingNymbox)
                // That's strange, this user didn't exist... but maybe I allow
                // people to drop notes anyway, so then the nymbox might already
                // exist, with usage tokens and messages inside....
                bSuccessLoadingNymbox =
                    (theNymbox.VerifyContractID() &&
                     theNymbox.VerifyAccount(server_->m_nymServer));
            // (No need here to load all the Box
            // Receipts)
            else {
                bSuccessLoadingNymbox =
                    theNymbox.GenerateLedger(
                        theNewNymID, NOTARY_ID, Ledger::nymbox, true) &&
                    theNymbox.SignContract(server_->m_nymServer) &&
                    theNymbox.SaveContract() && theNymbox.SaveNymbox();
            }
            // by this point, the nymbox DEFINITELY
            // exists -- or not. (generation might have
            // failed, or verification.)

            if (!bSuccessLoadingNymbox) {
                Log::vError(
                    "Error during user account "
                    "registration. Failed verifying or "
                    "generating nymbox for user: %s\n",
                    theMessage.m_strNymID.Get());
                return true;
            }
            // Either we loaded it up (it already
            // existed) or we didn't, in which case we
            // should
            // save it now (to create it.)
            //
            if (bLoadedSignedNymfile ||
                theNym.SaveSignedNymfile(server_->m_nymServer)) {
                Log::vOutput(0, "Success registering Nym credentials.\n");

                String strNymContents;
                theNym.SavePseudonym(strNymContents);
                msgOut.m_ascPayload.SetString(strNymContents);
                msgOut.m_bSuccess = true;
            }
            msgOut.SignContract(server_->m_nymServer);
            msgOut.SaveContract();
            return true;
        }  // Success loading and verifying the Nym based on his credentials.
    }
    // Look up the NymID and see if it's a valid user account.
    //
    // If we didn't receive a public key (above)
    // Or read one from our files (below)
    // ... then we'd have no way of validating the requests.
    //
    // If it is, then we read the public key from that Pseudonym and use it to
    // verify any
    // requests bearing that NymID.
    if (false == theNym.LoadPublicKey()) {
        Log::vError(
            "Failure loading public credentials for Nym: %s\n",
            theMessage.m_strNymID.Get());
        return false;
    }
    if (theNym.IsMarkedForDeletion()) {
        Log::vOutput(
            0,
            "(Failed) attempt by client to use a deleted Nym: %s\n",
            theMessage.m_strNymID.Get());
        return false;
    }
    // Okay, the file was read into memory and Public Key was successfully
    // extracted!
    // Next, let's use that public key to verify (1) the NymID and (2) the
    // signature
    // on the message that we're processing.

    if (!theNym.VerifyPseudonym()) {
        Log::Output(
            0,
            "Pseudonym failed to verify. Hash of public key doesn't match "
            "Nym ID that was sent.\n");
        return false;
    }
    Log::Output(3, "Pseudonym verified!\n");

    // So far so good. Now let's see if the signature matches...
    if (!theMessage.VerifySignature(theNym)) {
        Log::Output(0, "Signature verification failed!\n");
        return false;
    }
    Log::Output(
        3,
        "Signature verified! The message WAS signed by "
        "the Nym\'s private key.\n");

    // Get the public key from theNym, and set it into the connection.
    // This is only for verified Nyms, (and we're verified in here!) We
    // do this so that
    // we have the option later to encrypt the replies back to the
    // client...(using the
    // client's public key that we set here.)
    if (nullptr != pConnection)
        pConnection->SetPublicKey(theNym.GetPublicEncrKey());

    // Now we might as well load up the rest of the Nym.
    // Notice I use the && to only load the nymfile if it's NOT the
    // server Nym.
    if (!theNym.LoadSignedNymfile(server_->m_nymServer)) {
        Log::vError("Error loading Nymfile: %s\n", theMessage.m_strNymID.Get());
        return false;
    }
    Log::Output(2, "Successfully loaded Nymfile into memory.\n");

    auto context = OT::App().Contract().mutable_ClientContext(
        server_->GetServerNym().ID(), theNym.ID());
    context.It().SetRemoteNymboxHash(Identifier(theMessage.m_strNymboxHash));

    // ENTERING THE INNER SANCTUM OF SECURITY. If the user got all the way to
    // here, Then he has passed multiple levels of security, and all commands
    // below will assume the Nym is secure, validated, and loaded into memory
    // for use.
    // But still need to verify the Request Number for all other
    // commands except Get Request Number itself...
    // Request numbers start at 100 (currently). (Since certain special messages
    // USE 1 already... Such as messages that occur before requestnumbers are
    // possible, like RegisterNym.)
    auto requestNumber = context.It().Request();

    if (0 == requestNumber) {
        Log::Output(
            0,
            "Nym file request number doesn't exist. Apparently first-ever "
            "request to server--but everything checks out. (Shouldn't this "
            "request number have been created already when the NymFile was "
            "first created???????\n");
        // FIRST TIME!  This account has never before made a single request to
        // this server. The above call always succeeds unless the number just
        // isn't there for that server. Therefore, since it's the first time,
        // we'll create it now:
        requestNumber = context.It().IncrementRequest();

        OT_ASSERT(1 == requestNumber);
    }

    // At this point, I now have the current request number for this nym in
    // requestNumber Let's compare it to the one that was sent in the message...
    // (This prevents attackers from repeat-sending intercepted messages to the
    // server.)

    // IF it's NOT a getRequestNumber CMD, (therefore requires a request number)
    if (false == theMessage.m_strCommand.Compare("getRequestNumber")) {
        // AND the request number attached does not match what we just
        // read out of the file...
        if (requestNumber != theMessage.m_strRequestNum.ToLong()) {
            Log::vOutput(
                0,
                "Request number sent in this message "
                "%" PRId64 " does not match the one in the "
                "file! (%" PRId64 ")\n",
                theMessage.m_strRequestNum.ToLong(),
                requestNumber);
            return false;
        }
        // it's not a getRequestNumber CMD, and the request number
        // sent DOES match what we read out of the file!!

        // USAGE CREDITS...
        // Since (just below) we IncrementRequestNum() and
        // therefore save the Nym,
        // I figured it's a good spot to do our Usage Credits
        // code, so we don't have
        // to save twice.
        // IF (the OT Server is in "require usage credits" mode)
        // AND the User isn't magically FREE from
        // having to use usage credits (-1 is a
        // get-out-of-jail-free-card.)
        // AND (there's no Override Nym ID listed
        // --OR-- the Override Nym ID doesn't
        // match the Nym's ID who sent this message
        if (ServerSettings::__admin_usage_credits &&
            theNym.GetUsageCredits() >= 0 &&
            (ServerSettings::GetOverrideNymID().size() <= 0 ||
             (0 !=
              ServerSettings::GetOverrideNymID().compare(
                  (theMessage.m_strNymID.Get()))))) {
            const int64_t& lUsageCredits = theNym.GetUsageCredits();

            if (0 == lUsageCredits)  // If the User has ZERO
                                     // USAGE CREDITS LEFT. (Too
                                     // bad, even 1 would have
                                     // squeezed him by here.)
            {
                Log::vOutput(
                    0,
                    "Nym %s: ALL OUT of Usage "
                    "Credits, while server is in "
                    "**REQUIRE USAGE CREDITS "
                    "MODE**!\n",
                    theMessage.m_strNymID.Get());
                return false;
            }

            const int64_t lUsageFinal = (lUsageCredits - 1);
            theNym.SetUsageCredits(lUsageFinal);
        }

        Log::vOutput(
            3,
            "Request number sent in this message "
            "%" PRId64 " DOES match the one in the "
            "file!\n",
            requestNumber);

        // At this point, it is some OTHER command (besides getRequestNumber)
        // AND the request number verifies, so we're going to increment the
        // number, and let the command process.
        context.It().IncrementRequest();

        // **INSIDE** THE INNER SANCTUM OF SECURITY. If the user
        // got all the way to here,
        // Then he has passed multiple levels of security, and
        // all commands below will
        // assume the Nym is secure, validated, and loaded into
        // memory for use. They can
        // also assume that the request number has been verified
        // on this message.
        // EVERYTHING checks out.

        // NO RETURN HERE!!!! ON PURPOSE!!!!
    } else {
        // If you entered this else, that means it IS a getRequestNumber command
        // So we allow it to go through without verifying this step, and without
        // incrementing the counter.
        // theNym.IncrementRequestNum(server_->m_strNotaryID); //
        // commented out cause this is the one case where we DON'T increment
        // this number. We allow the user to get the number, we DON'T increment
        // it, and now the user can send it on his next request for some other
        // command, and it will verify properly. This prevents repeat messages.

        // NO RETURN HERE!!!! ON PURPOSE!!!!
    }

    // At this point, we KNOW that it is EITHER a GetRequestNumber command,
    // which doesn't require a request number, OR it was some other command, but
    // the request number they sent in the command MATCHES the one that we just
    // read out of the file.

    // Therefore, we can process ALL messages below this point KNOWING that the
    // Nym is properly verified in all ways. No messages need to worry about
    // verifying the Nym, or about dealing with the Request Number. It's all
    // handled in here.

    // If you made it down this far, that means the Pseudonym verifies! The
    // message is legit.
    //
    // (Notary ID was good, NymID is a valid hash of User's public key, and the
    // Signature on the message was signed by the user's private key.)
    //
    // Now we can process the message.
    //
    // All the commands below here, it is assumed that the user account exists
    // and is referenceable via theNym. (An OTPseudonym object.)
    //
    // ALL commands below can assume the Nym is real, and that the NymID and
    // Public Key are available for use -- and that they verify -- without
    // having to check again and again.

    // ACKNOWLEDGMENTS OF REPLIES ALREADY RECEIVED (FOR OPTIMIZATION.)

    // On the client side, whenever the client is DEFINITELY made aware of the
    // existence of a server reply, he adds its request number to this list,
    // which is sent along with all client-side requests to the server. The
    // server reads the list on the incoming client message (and it uses these
    // same functions to store its own internal list.) If the # already appears
    // on its internal list, then it does nothing. Otherwise, it loads up the
    // Nymbox and removes the replyNotice, and then adds the # to its internal
    // list. For any numbers on the internal list but NOT on the client's list,
    // the server removes from the internal list. (The client removed them when
    // it saw the server's internal list, which the server sends with its
    // replies.)
    //
    // This entire protocol, densely described, is unnecessary for OT to
    // function, but is great for optimization, as it enables OT to avoid
    // downloading all Box Receipts containing replyNotices, as long as the
    // original reply was properly received when the request was originally sent
    // (which is MOST of the time...) Thus we can eliminate most replyNotice
    // downloads, and likely a large % of box receipt downloads as well.

    const Identifier NOTARY_ID(server_->m_strNotaryID);

    // The server reads the list of acknowledged replies from the incoming
    // client message...

    // if we add any acknowledged replies to the server-side list, we will want
    // to save (at the end.)
    bool bIsDirtyNym = false;
    std::set<RequestNumber> numlist_ack_reply;

    if (theMessage.m_AcknowledgedReplies.Output(numlist_ack_reply)) {
        // Load Nymbox
        //
        Ledger theNymbox(theNym.GetConstID(), theNym.GetConstID(), NOTARY_ID);

        if (theNymbox.LoadNymbox() &&
            theNymbox.VerifySignature(server_->m_nymServer)) {
            // if we remove any replyNotices from the Nymbox, then we will want
            // to save the Nymbox (at the end.)
            bool bIsDirtyNymbox = false;

            for (auto& it : numlist_ack_reply) {
                const int64_t lRequestNum = it;
                // If the # already appears on its internal list, then it does
                // nothing. (It must have already done
                // whatever it needed to do, since it already has the number
                // recorded as acknowledged.)
                //
                // Otherwise, if the # does NOT already appear on server's
                // internal list, then it loads up the
                // Nymbox and removes the replyNotice, and then adds the # to
                // its internal list for safe-keeping.
                //
                if (!context.It().VerifyAcknowledgedNumber(lRequestNum)) {
                    // Verify whether a replyNotice exists in the Nymbox, with
                    // that lRequestNum
                    //
                    OTTransaction* pReplyNotice =
                        theNymbox.GetReplyNotice(lRequestNum);

                    if (nullptr != pReplyNotice) {
                        // If so, remove it...
                        //
                        const bool bDeleted =
                            pReplyNotice->DeleteBoxReceipt(theNymbox);
                        const bool bRemoved = theNymbox.RemoveTransaction(
                            pReplyNotice->GetTransactionNum());  // deletes
                        pReplyNotice = nullptr;
                        // (pReplyNotice is deleted, below this point,
                        // automatically by the above Remove call.)

                        if (!bDeleted || !bRemoved)
                            Log::Error(
                                "UserCommandProcessor::ProcessUserCommand: "
                                "Failed trying "
                                "to delete a box receipt, or "
                                "while removing its stub from the Nymbox.\n");

                        if (bRemoved)
                            bIsDirtyNymbox = true;  // So we don't have to save
                                                    // EACH iteration, but
                                                    // instead just once at the
                                                    // bottom.
                    }
                    // ...and add lRequestNum to server's acknowledgment
                    // list. (So this can't happen twice with same #.)
                    //
                    if (context.It().AddAcknowledgedNumber(lRequestNum)) {
                        // TODO remove this
                        bIsDirtyNym = true;
                    }

                }  // If server didn't already have a record of this
                   // acknowledged
                   // request #.
            }

            if (bIsDirtyNymbox) {
                theNymbox.ReleaseSignatures();
                theNymbox.SignContract(server_->m_nymServer);
                theNymbox.SaveContract();
                theNymbox.SaveNymbox();
            }
        }  // If nymbox loaded and verified.
    }

    // For any numbers on the server's internal list but NOT on the client's
    // list, the server removes from the internal list. (Because the client must
    // have seen my acknowledgment and thus removed the number from its own
    // list, so the server doesn't need to display it anymore.)

    std::set<RequestNumber> set_server_ack;

    if (theMessage.m_AcknowledgedReplies.Output(set_server_ack)) {
        context.It().FinishAcknowledgements(set_server_ack);
        bIsDirtyNym = true;
    }

    if (bIsDirtyNym) {
        theNym.SaveSignedNymfile(server_->m_nymServer);  // we save here.
    }

    // Note: in the ultimate future, we wouldn't even save the Nym down here,
    // but we'll let the entire message process
    // and then save the Nym at the end.
    // Then again -- you'd still want to know if the Nym was locked, at each
    // "save attempt" along the way. Because even
    // though the Nym might not actually save at each of those signposts, it
    // should still CANCEL OUT IF IT WOULD FAIL TRYING.
    // Of course we still only want to save the Nym once, but we still want each
    // step along the way -- each vital step that
    // would normally have saved each time -- to know whether or not it will
    // actually work, and if not, to fail the message
    // AT THAT POINT and not somewhere much later, at the bottom, after all
    // kinds of other processing has been done.
    //
    // Therefore in the new version we will probably still "Save" the Nym at
    // each critical point, but INTERNALLY, it won't
    // actually save until the bottom. BUT, even though it won't actually save,
    // it will still know if the TIMESTAMP IS WITHIN
    // VALID RANGE (each time), and it will still know that it has definitely
    // locked the resource (which happens the first time)
    // and it will still want to set the resource as dirty, internally, even
    // when it doesn't save it right away, because otherwise
    // it wouldn't know to save it later, either.

    msgOut.m_strNotaryID = server_->m_strNotaryID;
    msgOut.SetAcknowledgments(context.It());

    // This command is special because it's the only one that doesn't require a
    // request number. All of the other commands, below, will fail above if the
    // proper request number isn't included in the message.  They will already
    // have failed by this point if they // didn't verify.
    if (theMessage.m_strCommand.Compare("getRequestNumber")) {
        Log::vOutput(
            0,
            "\n==> Received a getRequestNumber message. Nym: %s ...\n",
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_requestnumber);

        UserCmdGetRequestNumber(theNym, context.It(), theMessage, msgOut);

        return true;
    } else if (theMessage.m_strCommand.Compare("getTransactionNumbers")) {
        Log::vOutput(
            0,
            "\n==> Received a getTransactionNumbers message. Nym: %s ...\n",
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_trans_nums);

        UserCmdGetTransactionNumbers(theNym, context.It(), theMessage, msgOut);

        return true;
    } else if (theMessage.m_strCommand.Compare("checkNym")) {
        Log::vOutput(
            0,
            "\n==> Received a checkNym message. Nym: %s ...\n",
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_check_nym);

        UserCmdCheckNym(theNym, theMessage, msgOut);

        return true;
    } else if (theMessage.m_strCommand.Compare("sendNymMessage")) {
        Log::vOutput(
            0,
            "\n==> Received a sendNymMessage message. Nym: %s ...\n",
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_send_message);

        UserCmdSendNymMessage(theNym, theMessage, msgOut);

        return true;
    } else if (theMessage.m_strCommand.Compare("sendNymInstrument")) {
        Log::vOutput(
            0,
            "\n==> Received a sendNymInstrument message. Nym: %s ...\n",
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_send_message);

        UserCmdSendNymInstrument(theNym, theMessage, msgOut);

        return true;
    } else if (theMessage.m_strCommand.Compare("unregisterNym")) {
        Log::vOutput(
            0,
            "\n==> Received a unregisterNym message. Nym: %s ...\n",
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_del_user_acct);

        UserCmdDeleteUser(theNym, context.It(), theMessage, msgOut);

        return true;
    } else if (theMessage.m_strCommand.Compare("unregisterAccount")) {
        Log::vOutput(
            0,
            "\n==> Received a unregisterAccount message. Nym: %s ...\n",
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_del_asset_acct);

        UserCmdDeleteAssetAcct(theNym, context.It(), theMessage, msgOut);

        return true;
    } else if (theMessage.m_strCommand.Compare("registerAccount")) {
        Log::vOutput(
            0,
            "\n==> Received a registerAccount message. Nym: %s ...\n",
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_create_asset_acct);

        UserCmdRegisterAccount(theNym, context.It(), theMessage, msgOut);

        return true;
    } else if (theMessage.m_strCommand.Compare(
                   "registerInstrumentDefinition")) {
        Log::vOutput(
            0,
            "\n==> Received an registerInstrumentDefinition "
            "message. Nym: %s ...\n",
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_issue_asset);

        UserCmdRegisterInstrumentDefinition(
            theNym, context.It(), theMessage, msgOut);

        return true;
    } else if (theMessage.m_strCommand.Compare("issueBasket")) {
        Log::vOutput(
            0,
            "\n==> Received an issueBasket message. Nym: %s ...\n",
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_issue_basket);

        UserCmdIssueBasket(theNym, theMessage, msgOut);

        return true;
    } else if (theMessage.m_strCommand.Compare("notarizeTransaction")) {
        Log::vOutput(
            0,
            "\n==> Received a notarizeTransaction message.  "
            "Acct: %s Nym: %s  ...\n",
            theMessage.m_strAcctID.Get(),
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_notarize_transaction);

        UserCmdNotarizeTransaction(theNym, context.It(), theMessage, msgOut);

        return true;
    } else if (theMessage.m_strCommand.Compare("getNymbox")) {
        Log::vOutput(
            0,
            "\n==> Received a getNymbox message. Nym: %s ...\n",
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_nymbox);

        UserCmdGetNymbox(theNym, context.It(), theMessage, msgOut);

        return true;
    } else if (theMessage.m_strCommand.Compare("getBoxReceipt")) {
        Log::vOutput(
            0,
            "\n==> Received a getBoxReceipt message. Nym: %s ...\n",
            strMsgNymID.Get());

        bool bRunIt = true;
        if (0 == theMessage.m_lDepth)
            OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_nymbox)
        else if (1 == theMessage.m_lDepth)
            OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_inbox)
        else if (2 == theMessage.m_lDepth)
            OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_outbox)
        else
            bRunIt = false;

        if (bRunIt) UserCmdGetBoxReceipt(theMessage, msgOut);

        return true;
    } else if (theMessage.m_strCommand.Compare("getAccountData")) {
        Log::vOutput(
            0,
            "\n==> Received a getAccountData message.  Acct: %s "
            "Nym: %s  ...\n",
            theMessage.m_strAcctID.Get(),
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_inbox);
        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_outbox);
        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_acct);

        UserCmdGetAccountData(theNym, theMessage, msgOut);

        return true;
    } else if (theMessage.m_strCommand.Compare("processNymbox")) {
        Log::vOutput(
            0,
            "\n==> Received a processNymbox message. Nym: %s ...\n",
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_process_nymbox);

        UserCmdProcessNymbox(theNym, context.It(), theMessage, msgOut);

        return true;
    } else if (theMessage.m_strCommand.Compare("processInbox")) {
        Log::vOutput(
            0,
            "\n==> Received a processInbox message. Acct: %s Nym: %s  ...\n",
            theMessage.m_strAcctID.Get(),
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_process_inbox);

        UserCmdProcessInbox(theNym, context.It(), theMessage, msgOut);

        return true;
    } else if (theMessage.m_strCommand.Compare("queryInstrumentDefinitions")) {
        Log::vOutput(
            0,
            "\n==> Received a queryInstrumentDefinitions "
            "message. Nym: %s ...\n",
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_contract);

        UserCmdQueryInstrumentDefinitions(theNym, theMessage, msgOut);

        return true;
    } else if (theMessage.m_strCommand.Compare("getInstrumentDefinition")) {
        Log::vOutput(
            0,
            "\n==> Received a getInstrumentDefinition message. Nym: %s ...\n",
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_contract);

        UserCmdGetInstrumentDefinition(theMessage, msgOut);

        return true;
    } else if (theMessage.m_strCommand.Compare("getMint")) {
        Log::vOutput(
            0,
            "\n==> Received a getMint message. Nym: %s ...\n",
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_mint);

        UserCmdGetMint(theNym, theMessage, msgOut);

        return true;
    } else if (theMessage.m_strCommand.Compare("getMarketList")) {
        Log::vOutput(
            0,
            "\n==> Received a getMarketList message. Nym: %s ...\n",
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_market_list);

        UserCmdGetMarketList(theNym, theMessage, msgOut);

        return true;
    } else if (theMessage.m_strCommand.Compare("getMarketOffers")) {
        Log::vOutput(
            0,
            "\n==> Received a getMarketOffers message. Nym: %s ...\n",
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_market_offers);

        UserCmdGetMarketOffers(theNym, theMessage, msgOut);

        return true;
    } else if (theMessage.m_strCommand.Compare("getMarketRecentTrades")) {
        Log::vOutput(
            0,
            "\n==> Received a getMarketRecentTrades message. Nym: %s ...\n",
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(
            ServerSettings::__cmd_get_market_recent_trades);

        UserCmdGetMarketRecentTrades(theNym, theMessage, msgOut);

        return true;
    } else if (theMessage.m_strCommand.Compare("getNymMarketOffers")) {
        Log::vOutput(
            0,
            "\n==> Received a getNymMarketOffers message. Nym: %s ...\n",
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_nym_market_offers);

        UserCmdGetNymMarketOffers(theNym, theMessage, msgOut);

        return true;
    } else if (theMessage.m_strCommand.Compare("triggerClause")) {
        Log::vOutput(
            0,
            "\n==> Received a triggerClause message. Nym: %s ...\n",
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_trigger_clause);

        UserCmdTriggerClause(theNym, context.It(), theMessage, msgOut);

        return true;
    } else if (theMessage.m_strCommand.Compare("usageCredits")) {
        Log::vOutput(
            0,
            "\n==> Received a usageCredits message. Nym: %s ...\n",
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_usage_credits);

        UserCmdUsageCredits(theNym, theMessage, msgOut);

        return true;
    } else if (theMessage.m_strCommand.Compare("registerContract")) {
        Log::vOutput(
            0,
            "\n==> Received a registerContract message. Nym: %s ...\n",
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_register_contract);

        UserCmdRegisterContract(theNym, theMessage, msgOut);

        return true;
    } else if (theMessage.m_strCommand.Compare("requestAdmin")) {
        Log::vOutput(
            0,
            "\n==> Received a requestAdmin message. Nym: %s ...\n",
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_request_admin);

        UserCmdRequestAdmin(theNym, theMessage, msgOut);

        return true;
    } else if (theMessage.m_strCommand.Compare("addClaim")) {
        Log::vOutput(
            0,
            "\n==> Received a addClaim message. Nym: %s ...\n",
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_request_admin);

        UserCmdAddClaim(theNym, theMessage, msgOut);

        return true;
    } else {
        Log::vError("Unknown command type in the XML, or missing payload, in "
                    "ProcessMessage.\n");

        String strTemp;
        strTemp.Format(
            "%sResponse",
            theMessage.m_strCommand.Get());  // Todo security.
                                             // Review this.

        msgOut.m_strCommand = strTemp;
        msgOut.m_strAcctID = theMessage.m_strAcctID;
        msgOut.m_strNotaryID = theMessage.m_strNotaryID;
        msgOut.m_strNymID = theMessage.m_strNymID;

        msgOut.m_bSuccess = false;

        String strRef(theMessage);

        msgOut.m_ascInReferenceTo.SetString(strRef);

        msgOut.SignContract(server_->m_nymServer);
        msgOut.SaveContract();

        return false;
    }
}

// Get the list of markets on this server.
void UserCommandProcessor::UserCmdGetMarketList(
    Nym&,
    Message& MsgIn,
    Message& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "getMarketListResponse";  // reply to getMarketList
    msgOut.m_strNymID = MsgIn.m_strNymID;           // NymID
    //    msgOut.m_strNotaryID    = m_strNotaryID;    // This is already set in
    // ProcessUserCommand.

    OTASCIIArmor ascOutput;
    int32_t nMarketCount = 0;

    msgOut.m_bSuccess = server_->m_Cron.GetMarketList(ascOutput, nMarketCount);

    // If success,
    if ((true == msgOut.m_bSuccess) && (nMarketCount > 0)) {
        msgOut.m_ascPayload.Set(ascOutput);

        String strCount;
        strCount.Format("%d", nMarketCount);
        msgOut.m_lDepth = strCount.ToLong();
    }
    // if Failed, we send the user's message back to him, ascii-armored as part
    // of response.
    else if (!msgOut.m_bSuccess) {
        String tempInMessage(MsgIn);
        msgOut.m_ascInReferenceTo.SetString(tempInMessage);
    }

    // (2) Sign the Message
    msgOut.SignContract(server_->m_nymServer);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    //
    // FYI, SaveContract takes m_xmlUnsigned and wraps it with the signatures
    // and ------- BEGIN  bookends
    // If you don't pass a string in, then SaveContract saves the new version to
    // its member, m_strRawFile
    msgOut.SaveContract();
}

// Get the publicly-available list of offers on a specific market.
void UserCommandProcessor::UserCmdGetMarketOffers(
    Nym&,
    Message& MsgIn,
    Message& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand =
        "getMarketOffersResponse";           // reply to getMarketOffers
    msgOut.m_strNymID = MsgIn.m_strNymID;    // NymID
    msgOut.m_strNymID2 = MsgIn.m_strNymID2;  // Market ID.

    int64_t lDepth = MsgIn.m_lDepth;
    if (lDepth < 0) lDepth = 0;

    const Identifier MARKET_ID(MsgIn.m_strNymID2);

    OTMarket* pMarket = server_->m_Cron.GetMarket(MARKET_ID);

    // If success,
    if ((msgOut.m_bSuccess =
             ((pMarket != nullptr) ? true : false)))  // if assigned true
    {
        OTASCIIArmor ascOutput;
        int32_t nOfferCount = 0;

        msgOut.m_bSuccess =
            pMarket->GetOfferList(ascOutput, lDepth, nOfferCount);

        if ((true == msgOut.m_bSuccess) && (nOfferCount > 0)) {
            msgOut.m_ascPayload = ascOutput;
            msgOut.m_lDepth = nOfferCount;
        }
    }

    // if Failed, we send the user's message back to him, ascii-armored as part
    // of response.
    if (!msgOut.m_bSuccess) {
        String tempInMessage(MsgIn);
        msgOut.m_ascInReferenceTo.SetString(tempInMessage);
    }

    // (2) Sign the Message
    msgOut.SignContract(server_->m_nymServer);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    //
    // FYI, SaveContract takes m_xmlUnsigned and wraps it with the signatures
    // and ------- BEGIN  bookends
    // If you don't pass a string in, then SaveContract saves the new version to
    // its member, m_strRawFile
    msgOut.SaveContract();
}

// Get a report of recent trades that have occurred on a specific market.
void UserCommandProcessor::UserCmdGetMarketRecentTrades(
    Nym&,
    Message& MsgIn,
    Message& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand =
        "getMarketRecentTradesResponse";     // reply to getMarketRecentTrades
    msgOut.m_strNymID = MsgIn.m_strNymID;    // NymID
    msgOut.m_strNymID2 = MsgIn.m_strNymID2;  // Market ID.

    const Identifier MARKET_ID(MsgIn.m_strNymID2);

    OTMarket* pMarket = server_->m_Cron.GetMarket(MARKET_ID);

    // If success,
    if ((msgOut.m_bSuccess =
             ((pMarket != nullptr) ? true : false)))  // if assigned true
    {
        OTASCIIArmor ascOutput;
        int32_t nTradeCount = 0;

        msgOut.m_bSuccess = pMarket->GetRecentTradeList(ascOutput, nTradeCount);

        if (true == msgOut.m_bSuccess) {
            msgOut.m_lDepth = nTradeCount;

            if (nTradeCount > 0) msgOut.m_ascPayload = ascOutput;
        }
    }

    // if Failed, we send the user's message back to him, ascii-armored as part
    // of response.
    if (!msgOut.m_bSuccess) {
        String tempInMessage(MsgIn);
        msgOut.m_ascInReferenceTo.SetString(tempInMessage);
    }

    // (2) Sign the Message
    msgOut.SignContract(server_->m_nymServer);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    //
    // FYI, SaveContract takes m_xmlUnsigned and wraps it with the signatures
    // and ------- BEGIN  bookends
    // If you don't pass a string in, then SaveContract saves the new version to
    // its member, m_strRawFile
    msgOut.SaveContract();
}

// Get the offers that a specific Nym has placed on a specific market.
//
void UserCommandProcessor::UserCmdGetNymMarketOffers(
    Nym& theNym,
    Message& MsgIn,
    Message& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand =
        "getNymMarketOffersResponse";      // reply to getMarketOffers
    msgOut.m_strNymID = MsgIn.m_strNymID;  // NymID

    Identifier NYM_ID;
    theNym.GetIdentifier(NYM_ID);

    OTASCIIArmor ascOutput;
    int32_t nOfferCount = 0;

    msgOut.m_bSuccess =
        server_->m_Cron.GetNym_OfferList(ascOutput, NYM_ID, nOfferCount);

    if ((msgOut.m_bSuccess) && (nOfferCount > 0)) {

        msgOut.m_ascPayload = ascOutput;
        msgOut.m_lDepth = nOfferCount;
    }
    // if Failed, we send the user's message back to him, ascii-armored as part
    // of response.
    if (!msgOut.m_bSuccess) {
        String tempInMessage(MsgIn);
        msgOut.m_ascInReferenceTo.SetString(tempInMessage);
    }

    // (2) Sign the Message
    msgOut.SignContract(server_->m_nymServer);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    //
    // FYI, SaveContract takes m_xmlUnsigned and wraps it with the signatures
    // and ------- BEGIN  bookends
    // If you don't pass a string in, then SaveContract saves the new version to
    // its member, m_strRawFile
    msgOut.SaveContract();
}

void UserCommandProcessor::UserCmdPingNotary(
    Nym&,
    Message& MsgIn,
    Message& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "pingNotaryResponse";
    msgOut.m_strNymID = MsgIn.m_strNymID;

    if (MsgIn.m_strNotaryID == server_->m_strNotaryID)
        msgOut.m_bSuccess = true;
    else
        msgOut.m_bSuccess = false;

    // (2) Sign the Message
    msgOut.SignContract(server_->m_nymServer);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    //
    // FYI, SaveContract takes m_xmlUnsigned and wraps it with the signatures
    // and ------- BEGIN  bookends
    // If you don't pass a string in, then SaveContract saves the new version to
    // its member, m_strRawFile
    msgOut.SaveContract();
}

void UserCommandProcessor::UserCmdGetTransactionNumbers(
    Nym& theNym,
    ClientContext& context,
    Message& MsgIn,
    Message& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand =
        "getTransactionNumbersResponse";   // reply to getTransactionNumbers
    msgOut.m_strNymID = MsgIn.m_strNymID;  // NymID

    const Identifier NOTARY_ID(server_->m_strNotaryID);

    // A few client requests, and a few server replies (not exactly matched up)
    // will include a copy of the NymboxHash.  The server may reject certain
    // client requests that have a bad value here (since it would be out of sync
    // anyway); the client is able to see the server's hash and realize to
    // re-download the nymbox and other intermediary files.
    //
    const auto nCount = context.AvailableNumbers();
    ;
    const bool hashMatch = context.NymboxHashMatch();

    if (context.HaveLocalNymboxHash()) {
        context.LocalNymboxHash().GetString(msgOut.m_strNymboxHash);
    }

    if (!hashMatch) {
        Log::Output(
            0,
            "UserCommandProcessor::UserCmdGetTransactionNumbers: Rejecting "
            "message since nymbox hash "
            "doesn't match. (Send a getNymbox message to grab the "
            "newest one.)\n");

    } else if (nCount > 50)  // todo no hardcoding. (max transaction nums
                             // allowed out at a single time.)
    {
        Log::vOutput(
            0,
            "UserCommandProcessor::UserCmdGetTransactionNumbers: Failure: Nym "
            "%s "
            "already has "
            "more than 50 unused transaction numbers signed out. (He needs to "
            "use those first. "
            "Tell him to download his latest Nymbox.)\n",
            MsgIn.m_strNymID.Get());
    } else {
        Identifier NYM_ID, NYMBOX_HASH;
        theNym.GetIdentifier(NYM_ID);

        bool bSuccess = true;
        bool bSavedNymbox = false;
        Ledger theLedger(NYM_ID, NYM_ID, NOTARY_ID);  // Nymbox

        // We'll store the transaction numbers here immediately after they're
        // issued,
        // before adding them to the Nymbox.
        //
        NumList theNumlist;

        // Update: Now we're going to grab 20 or 30 transaction numbers,
        // instead of just 1 like before!!!
        //
        for (int32_t i = 0; i < 100;
             i++)  // todo, hardcoding!!!! (But notice we
                   // grab 100 transaction numbers at a
                   // time now.)
        {
            int64_t lTransNum = 0;
            // This call will save the new transaction number to the nym's file.
            // ??? Why did I say that, when clearly the last parameter is false?
            // AHHHH Because I drop it into the Nymbox instead, and make him
            // sign for it!

            if (!server_->transactor_.issueNextTransactionNumber(lTransNum)) {
                lTransNum = 0;
                Log::Error(
                    "UserCommandProcessor::UserCmdGetTransactionNumbers: "
                    "Error issuing "
                    "next transaction number!\n");
                bSuccess = false;
                break;
            }

            theNumlist.Add(lTransNum);  // <=========
        }

        int64_t transactionNumber;
        if (bSuccess &&
            !server_->transactor_.issueNextTransactionNumber(
                transactionNumber)) {
            Log::Error("UserCommandProcessor::UserCmdGetTransactionNumbers: "
                       "Error issuing transaction number!\n");
            bSuccess = false;
        }

        if (!bSuccess) {
            // Apparently nothing. Also, plenty of logs just above already, if
            // this ever happens.
        } else if (!theLedger.LoadNymbox()) {
            Log::Error("Error loading Nymbox in "
                       "UserCommandProcessor::UserCmdGetTransactionNumbers\n");
        }
        // Drop in the Nymbox
        else if ((msgOut.m_bSuccess =
                      (theLedger.VerifyContractID() &&  // We don't need them
                       // right now, so we verify
                       theLedger.VerifySignature(
                           server_->m_nymServer)  // everything else without
                                                  // loading them.
                       )                          // if loaded and verified.
                  ))                              // if success
        {
            // Note: I decided against adding newly-requested transaction
            // numbers to existing OTTransaction::blanks in the Nymbox.
            // Why not? Because once the user downloads the Box Receipt, he will
            // think he has it already, when the Server meanwhile
            // has a new version of that same Box Receipt. But the user will
            // never re-download it if he believes that he already has
            // it.
            // Since the transaction can contain 10, 20, or 50 transaction
            // numbers now, we don't NEED to be able to combine them
            // anyway, since the problem is still effectively solved.

            OTTransaction* pTransaction = OTTransaction::GenerateTransaction(
                theLedger,
                OTTransaction::blank,
                originType::not_applicable,
                transactionNumber);  // Generate a new OTTransaction::blank

            if (nullptr !=
                pTransaction)  // The above has an OT_ASSERT within, but
                               // I just like to check my pointers.
            {
                // ADD the contents of theNumlist (the 20 new transaction
                // numbers we're giving the user)
                // to this OTTransaction::blank.
                //
                pTransaction->AddNumbersToTransaction(theNumlist);

                pTransaction->SignContract(server_->m_nymServer);
                pTransaction->SaveContract();

                theLedger.AddTransaction(*pTransaction);

                theLedger.ReleaseSignatures();
                theLedger.SignContract(server_->m_nymServer);
                theLedger.SaveContract();

                bSavedNymbox = true;
                theLedger.SaveNymbox(&NYMBOX_HASH);

                // Any inbox/nymbox/outbox ledger will only itself contain
                // abbreviated versions of the receipts, including their hashes.
                //
                // The rest is stored separately, in the box receipt, which is
                // created
                // whenever a receipt is added to a box, and deleted after a
                // receipt
                // is removed from a box.
                //
                pTransaction->SaveBoxReceipt(theLedger);
            } else
                theLedger.CalculateNymboxHash(NYMBOX_HASH);
        } else {
            Log::Error("Error verifying Nymbox in "
                       "UserCommandProcessor::UserCmdGetTransactionNumbers\n");
        }
        std::set<int64_t> theList;
        theNumlist.Output(theList);

        if (bSavedNymbox) {
            context.SetLocalNymboxHash(NYMBOX_HASH);
            theNym.SaveSignedNymfile(server_->m_nymServer);  // TODO remove this
            NYMBOX_HASH.GetString(msgOut.m_strNymboxHash);
        } else if (true == msgOut.m_bSuccess) {
            theLedger.CalculateNymboxHash(NYMBOX_HASH);
            context.SetLocalNymboxHash(NYMBOX_HASH);
            theNym.SaveSignedNymfile(server_->m_nymServer);  // TODO remove this
            NYMBOX_HASH.GetString(msgOut.m_strNymboxHash);
        }
        // else EXISTING_NYMBOX_HASH.GetString(msgOut.m_strNymboxHash); (above)
    }

    // (2) Sign the Message
    msgOut.SignContract(server_->m_nymServer);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    //
    // FYI, SaveContract takes m_xmlUnsigned and wraps it with the signatures
    // and ------- BEGIN  bookends
    // If you don't pass a string in, then SaveContract saves the new version to
    // its member, m_strRawFile
    msgOut.SaveContract();
}

void UserCommandProcessor::UserCmdGetRequestNumber(
    Nym& theNym,
    ClientContext& context,
    Message& MsgIn,
    Message& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "getRequestNumberResponse";
    msgOut.m_strNymID = MsgIn.m_strNymID;
    // Outoing reply contains same request num coming in (1).
    msgOut.m_strRequestNum.Set(MsgIn.m_strRequestNum);

    auto lReqNum = context.Request();
    msgOut.m_bSuccess = (0 != lReqNum);

    // Server was unable to load ReqNum, which is unusual because the calling
    // function should have already insured its existence.
    if (!msgOut.m_bSuccess) {
        Log::Error("Error loading request number in "
                   "UserCommandProcessor::UserCmdGetRequestNumber\n");
        lReqNum = 1;
        context.SetRequest(lReqNum);
        msgOut.m_bSuccess = true;
    } else {
        msgOut.m_lNewRequestNum = lReqNum;
    }

    const Identifier NOTARY_ID(server_->m_strNotaryID);
    Identifier EXISTING_NYMBOX_HASH = context.LocalNymboxHash();

    if (String(EXISTING_NYMBOX_HASH).Exists()) {
        EXISTING_NYMBOX_HASH.GetString(msgOut.m_strNymboxHash);
    } else {
        const Identifier theNymID(theNym);
        Ledger theLedger(theNymID, theNymID, NOTARY_ID);

        if (theLedger.LoadNymbox() && theLedger.VerifyContractID() &&
            theLedger.VerifySignature(server_->m_nymServer)) {
            theLedger.CalculateNymboxHash(EXISTING_NYMBOX_HASH);
            context.SetLocalNymboxHash(EXISTING_NYMBOX_HASH);
            theNym.SaveSignedNymfile(server_->m_nymServer);  // TODO remove this
            EXISTING_NYMBOX_HASH.GetString(msgOut.m_strNymboxHash);
        }
    }

    // (2) Sign the Message
    msgOut.SignContract(server_->m_nymServer);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    //
    // FYI, SaveContract takes m_xmlUnsigned and wraps it with the signatures
    // and ------- BEGIN  bookends
    // If you don't pass a string in, then SaveContract saves the new version to
    // its member, m_strRawFile
    msgOut.SaveContract();
}

void UserCommandProcessor::UserCmdSendNymMessage(
    Nym& theNym,
    Message& MsgIn,
    Message& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "sendNymMessageResponse";  // reply to sendNymMessage
    msgOut.m_strNymID = MsgIn.m_strNymID;            // NymID
    msgOut.m_strNymID2 = MsgIn.m_strNymID2;  // NymID of recipient pubkey

    const String strInMessage(MsgIn);
    const Identifier SENDER_NYM_ID(theNym), RECIPIENT_NYM_ID(MsgIn.m_strNymID2),
        NOTARY_ID(server_->m_strNotaryID);
    msgOut.m_ascInReferenceTo.SetString(strInMessage);
    const bool bSent = SendMessageToNym(
        NOTARY_ID,
        SENDER_NYM_ID,
        RECIPIENT_NYM_ID,
        &MsgIn);  // pstrMessage=nullptr

    if (!bSent) {
        Log::vError("UserCommandProcessor::UserCmdSendNymMessage: Failed "
                    "while calling "
                    "SendMessageToNym.\n");
        msgOut.m_bSuccess = false;
    } else {
        msgOut.m_bSuccess = true;
    }
    // (2) Sign the Message
    msgOut.SignContract(server_->m_nymServer);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    //
    // FYI, SaveContract takes m_xmlUnsigned and wraps it with the signatures
    // and ------- BEGIN  bookends
    // If you don't pass a string in, then SaveContract saves the new version to
    // its member, m_strRawFile
    msgOut.SaveContract();
}

void UserCommandProcessor::UserCmdSendNymInstrument(
    Nym& theNym,
    Message& MsgIn,
    Message& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand =
        "sendNymInstrumentResponse";         // reply to sendNymInstrument
    msgOut.m_strNymID = MsgIn.m_strNymID;    // NymID
    msgOut.m_strNymID2 = MsgIn.m_strNymID2;  // NymID of recipient pubkey

    const String strInMessage(MsgIn);
    const Identifier SENDER_NYM_ID(theNym), RECIPIENT_NYM_ID(MsgIn.m_strNymID2),
        NOTARY_ID(server_->m_strNotaryID);
    msgOut.m_ascInReferenceTo.SetString(strInMessage);
    const bool bSent = server_->SendInstrumentToNym(
        NOTARY_ID,
        SENDER_NYM_ID,
        RECIPIENT_NYM_ID,
        &MsgIn);  // pPayment=nullptr, szCommand=nullptr

    if (!bSent) {
        Log::vError(
            "UserCommandProcessor::UserCmdSendNymInstrument: Failed while "
            "calling SendInstrumentToNym.\n");
        msgOut.m_bSuccess = false;
    } else {
        msgOut.m_bSuccess = true;
    }
    // (2) Sign the Message
    msgOut.SignContract(server_->m_nymServer);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    //
    // FYI, SaveContract takes m_xmlUnsigned and wraps it with the signatures
    // and ------- BEGIN  bookends
    // If you don't pass a string in, then SaveContract saves the new version to
    // its member, m_strRawFile
    msgOut.SaveContract();
}

void UserCommandProcessor::UserCmdCheckNym(
    Nym&,
    Message& MsgIn,
    Message& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "checkNymResponse";  // reply to checkNym
    msgOut.m_strNymID = MsgIn.m_strNymID;      // NymID
    msgOut.m_strNymID2 =
        MsgIn.m_strNymID2;  // NymID of public key requested by user.

    msgOut.m_bSuccess = false;

    auto nym2 = OT::App().Contract().Nym(Identifier(MsgIn.m_strNymID2));

    // If success, return nym2 in serialized form
    if (nym2) {
        msgOut.m_ascPayload.SetData(proto::ProtoAsData(nym2->asPublicNym()));
        msgOut.m_bSuccess = true;
    }
    // --------------------------------------------------
    // if Failed, we send the user's message back to him, ascii-armored as part
    // of response.
    if (!msgOut.m_bSuccess) {
        String tempInMessage(MsgIn);
        msgOut.m_ascInReferenceTo.SetString(tempInMessage);
    }
    // --------------------------------------------------
    // (2) Sign the Message
    msgOut.SignContract(server_->m_nymServer);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    //
    // FYI, SaveContract takes m_xmlUnsigned and wraps it with the signatures
    // and ------- BEGIN  bookends
    // If you don't pass a string in, then SaveContract saves the new version to
    // its member, m_strRawFile
    msgOut.SaveContract();
}

void UserCommandProcessor::UserCmdRegisterContract(
    Nym&,
    Message& MsgIn,
    Message& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "registerContractResponse";
    msgOut.m_strNymID = MsgIn.m_strNymID;
    msgOut.m_bSuccess = false;
    const auto type = static_cast<ContractType>(MsgIn.enum_);

    switch (type) {
        case (ContractType::NYM): {
            const auto nym = proto::DataToProto<proto::CredentialIndex>(
                Data(MsgIn.m_ascPayload));
            msgOut.m_bSuccess = bool(OT::App().Contract().Nym(nym));

            break;
        }
        case (ContractType::SERVER): {
            const auto server = proto::DataToProto<proto::ServerContract>(
                Data(MsgIn.m_ascPayload));
            msgOut.m_bSuccess = bool(OT::App().Contract().Server(server));

            break;
        }
        case (ContractType::UNIT): {
            const auto unit = proto::DataToProto<proto::UnitDefinition>(
                Data(MsgIn.m_ascPayload));
            msgOut.m_bSuccess = bool(OT::App().Contract().UnitDefinition(unit));

            break;
        }
        default: {
            otErr << __FUNCTION__ << ": Invalid contract type: " << MsgIn.enum_
                  << std::endl;
        }
    }

    msgOut.m_ascInReferenceTo.SetString(String(MsgIn));
    msgOut.SignContract(server_->m_nymServer);
    msgOut.SaveContract();
}

/*
  Allows ANY Nym to GET AND SET the Usage Credits for ANY other Nym!
  UPDATE: Only the override Nym can change the credits,
  You might ask, "But what if I don't want users to be able to set the Usage
  Credits?"
  That makes sense: Go to ~/.ot/server.cfg and set cmd_usage_credits=false
  (which is its default BTW.)
  That way, NO ONE can set credits, or view them for other people. (People can
  still view their own.)
  But you might ask, "But what if I want the ADMIN to still be able to set and
  view credits?"
  That makes sense: Just make sure the override_nym_id in server.cfg is set to
  your admin Nym, and
  that Nym will STILL be able to use this message:
*/
void UserCommandProcessor::UserCmdUsageCredits(
    Nym& theNym,
    Message& MsgIn,
    Message& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "usageCreditsResponse";  // reply to usageCredits
    msgOut.m_strNymID = MsgIn.m_strNymID;          // NymID
    msgOut.m_strNymID2 = MsgIn.m_strNymID2;        // NymID of user whose usage
    // credits are being examined /
    // adjusted.
    const bool bIsPrivilegedNym =
        ((ServerSettings::GetOverrideNymID().size() >
          0) &&  // And if there's an override Nym...
         (0 ==
          ServerSettings::GetOverrideNymID().compare(
              (MsgIn.m_strNymID.Get()))));  // And if the acting Nym IS the
                                            // override Nym...
    // The amount the usage credits are being ADJUSTED by.
    const int64_t lAdjustment =
        (bIsPrivilegedNym && ServerSettings::__admin_usage_credits)
            ? MsgIn.m_lDepth
            : 0;

    msgOut.m_lDepth = 0;  // Returns total Usage Credits on Nym at the end.
    Nym nym2;
    Identifier nym2ID, NOTARY_ID(server_->m_strNotaryID);
    nym2.SetIdentifier(MsgIn.m_strNymID2);
    nym2.GetIdentifier(nym2ID);

    const bool bIsSameNym = nym2.CompareID(theNym);
    Nym* pNym = nullptr;

    bool bErrorCondition = false;

    // If nym2 and theNym are already the same Nym, then pNym points to theNym
    // by now already.
    // (And we'll skip this block.) Otherwise we load up nym2, and point pNym to
    // nym2 instead.
    //
    if (bIsSameNym)
        pNym = &theNym;
    else  // theNym and nym2 are different Nyms, so let's load it up.
    {
        bool bLoadedPublicKey =
            nym2.LoadPublicKey() &&
            nym2.VerifyPseudonym();  // Old style (deprecated.) For now, this
                                     // calls LoadCredentials inside (which is
                                     // the new style.) Eventually we'll just
                                     // call that here directly.
        bool bLoadSignedNymfile = nym2.LoadSignedNymfile(server_->m_nymServer);
        if (!bLoadSignedNymfile &&
            !bLoadedPublicKey)  // Nym didn't already exist.
        {
            pNym = &nym2;
        } else if (bLoadedPublicKey && !bLoadSignedNymfile)  // Error -- if key
                                                             // was there, then
                                                             // nymfile should
                                                             // have been also.
        {
            Log::vError(
                "%s: Nym public key (%s) exists, but nymfile doesn't! "
                "Could be error reading from storage. (Failure.)\n",
                __FUNCTION__,
                MsgIn.m_strNymID2.Get());
            bErrorCondition = true;
        } else {
            pNym = &nym2;
        }
    }
    if (!MsgIn.m_strNymID.Compare(MsgIn.m_strNymID2))  // If the Nym is
                                                       // not performing
                                                       // this on
                                                       // himself...
    {
        // Either this is a Nym performing the action on himself (which is
        // read-only.)
        // Or he is the "override Nym" (special powers) and he is allowed to do
        // it on anybody (adjusting the credits up or down.)
        // But if he's NOT the override Nym, then he can NOT do it, even
        // read-only, on anyone OTHER than himself.
        //
        // Inside this block, we've said, "If the Nym is NOT doing this on
        // himself... (But on someone else)"
        // ...Then we KNOW, if that's true, that the Nym had BETTER have special
        // powers, or there's an error.
        //
        if (!((ServerSettings::GetOverrideNymID().size() > 0) &&
              (0 ==
               ServerSettings::GetOverrideNymID().compare(
                   (MsgIn.m_strNymID.Get())))))  // ...And if he's
                                                 // not the special
                                                 // "override Nym"...
        {
            Log::vError(
                "%s: Failed attempt by a normal Nym to view or "
                "adjust usage credits on a different Nym (you're "
                "only allowed to do this to yourself, unless your "
                "nym is the specially-empowered 'override nym'.)\n",
                __FUNCTION__);
            bErrorCondition = true;
        }
    }
    Ledger theNymbox(nym2ID, nym2ID, NOTARY_ID);
    bool bSuccessLoadingNymbox = theNymbox.LoadNymbox();

    if (bSuccessLoadingNymbox)
        bSuccessLoadingNymbox =
            (theNymbox.VerifyContractID() &&
             theNymbox.VerifyAccount(server_->m_nymServer));
    else {
        bSuccessLoadingNymbox =
            theNymbox.GenerateLedger(nym2ID, NOTARY_ID, Ledger::nymbox, true);

        if (bSuccessLoadingNymbox) {
            bSuccessLoadingNymbox =
                theNymbox.SignContract(server_->m_nymServer);

            if (bSuccessLoadingNymbox) {
                bSuccessLoadingNymbox = theNymbox.SaveContract();

                if (bSuccessLoadingNymbox)
                    bSuccessLoadingNymbox = theNymbox.SaveNymbox();
            }
        }
    }
    if (!bSuccessLoadingNymbox) bErrorCondition = true;
    // By this point, pNym points to the correct Nym, if bErrorCondition=false;
    //
    if (!bErrorCondition) {
        // Get the current usage credits, which will be sent in the reply.
        //
        const int64_t& lOldCredits = pNym->GetUsageCredits();
        const int64_t lTentativeNew = lOldCredits + lAdjustment;
        const int64_t lNewCredits =
            (lTentativeNew < 0)
                ? (-1)
                : lTentativeNew;  // It can never be less than -1.

        // if adjustment is non-zero, and the acting Nym has authority to make
        // adjustments...
        //         if ((0 != lAdjustment) && bIsPrivilegedNym)
        //
        // Note: if the adjustment is non-zero, then we ALREADY know the acting
        // Nym has the authority.
        // How do we know?
        //
        // int64_t lAdjustment  = (bIsPrivilegedNym &&
        // ServerSettings::__admin_usage_credits) ? MsgIn.m_lDepth : 0;
        //
        // (Therefore we also know that the server is in usage credits mode, as
        // well.)
        //
        if (0 != lAdjustment) {
            // Only the override Nym can get inside this block and set the
            // credits.
            // And ONLY in the case where usage credits are turned on, on the
            // server side.
            // Any other Nym can use UsageCredits message but as read-only, to
            // retrieve
            // the value, not to actually set it. In that case, the adjustment
            // is always
            // interpreted as "0", and the return value usage credits is always
            // set to -1.
            //
            pNym->SetUsageCredits(lNewCredits);
            msgOut.m_bSuccess = pNym->SaveSignedNymfile(
                server_->m_nymServer);  // We changed it, so let's save pNym...
        } else
            msgOut.m_bSuccess = true;  // No adjustment -- we're just returning
                                       // the current usage credits or -1,
        // depending on whether server is in usage
        // mode.
        //
        // This is because we always return credits of -1 in this case, so we
        // don't want to be secretly
        // continuing to adjust the credits farther and farther while
        // simultaneously returning -1.
        // Either way (even if adjustment is zero) then lNewCredits contains the
        // value being sent back...
        //
        if (ServerSettings::__admin_usage_credits)  // If the server has usage
                                                    // credits
                                                    // turned on...
            msgOut.m_lDepth =
                lNewCredits;  // ...then adjustment or not, we send
                              // the current usage credits balance
                              // back in the server reply.
        else  // Else if the server does NOT have usage credits turned on...
            msgOut.m_lDepth = -1;  // ...then we always return -1, so the client
                                   // doesn't pop up any error messages related
                                   // to usage credits.
    }
    // (2) Sign the Message
    msgOut.SignContract(server_->m_nymServer);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    //
    // FYI, SaveContract takes m_xmlUnsigned and wraps it with the signatures
    // and ------- BEGIN  bookends
    // If you don't pass a string in, then SaveContract saves the new version to
    // its member, m_strRawFile
    msgOut.SaveContract();
}

/// An existing user is issuing a new currency.
void UserCommandProcessor::UserCmdRegisterInstrumentDefinition(
    Nym& theNym,
    ClientContext& context,
    Message& MsgIn,
    Message& msgOut)
{
    const char* szFunc =
        "UserCommandProcessor::UserCmdRegisterInstrumentDefinition";

    // (1) set up member variables
    msgOut.m_strCommand = "registerInstrumentDefinitionResponse";  // reply to
    // registerInstrumentDefinition
    msgOut.m_strNymID = MsgIn.m_strNymID;  // NymID
    msgOut.m_strInstrumentDefinitionID =
        MsgIn.m_strInstrumentDefinitionID;  // Instrument Definition ID, a hash
                                            // of the asset
                                            // contract.

    const Identifier NYM_ID(theNym), NOTARY_ID(server_->m_strNotaryID),
        INSTRUMENT_DEFINITION_ID(MsgIn.m_strInstrumentDefinitionID);

    auto pUnitDefinition =
        OT::App().Contract().UnitDefinition(INSTRUMENT_DEFINITION_ID);

    // Make sure the contract isn't already available on this server.
    //
    if (pUnitDefinition) {
        Log::vError(
            "%s: Error: Attempt to issue instrument definition that "
            "already exists.\n",
            szFunc);
    } else {
        auto serialized = proto::DataToProto<proto::UnitDefinition>(
            Data(MsgIn.m_ascPayload));
        if (proto::UNITTYPE_BASKET == serialized.type()) {
            Log::vOutput(
                0,
                "%s: Prevented attempt by user to issue a "
                "basket currency contract. (He needs to use "
                "the issueBasket message for that.)\n",
                szFunc);
        } else {
            pUnitDefinition = OT::App().Contract().UnitDefinition(serialized);

            if (!pUnitDefinition) {
                Log::vOutput(
                    0,
                    "%s: Failed trying to instantiate asset "
                    "contract. Instrument Definition Id: %s\n",
                    szFunc,
                    MsgIn.m_strInstrumentDefinitionID.Get());
            } else {
                // Create an ISSUER account (like a normal account, except
                // it can go negative)
                std::unique_ptr<Account> pNewAccount(
                    Account::GenerateNewAccount(
                        NYM_ID,
                        NOTARY_ID,
                        server_->m_nymServer,
                        MsgIn,
                        Account::issuer));

                // If we successfully create the account, then bundle it in
                // the message XML payload
                if (nullptr != pNewAccount)  // This last parameter generates an
                                             // ISSUER account
                {                            // instead of the default SIMPLE.
                    // Make sure the contracts/%s file is created for next
                    // time.
                    String tempPayload(*pNewAccount);
                    msgOut.m_ascPayload.SetString(tempPayload);

                    // Attach the new account number to the outgoing
                    // message.
                    pNewAccount->GetIdentifier(msgOut.m_strAcctID);

                    server_->mainFile_.SaveMainFile();

                    Identifier theNewAccountID;
                    pNewAccount->GetIdentifier(theNewAccountID);
                    Log::Output(
                        0, "Generating inbox/outbox for new issuer acct. \n");

                    Ledger theOutbox(NYM_ID, theNewAccountID, NOTARY_ID),
                        theInbox(NYM_ID, theNewAccountID, NOTARY_ID);

                    bool bSuccessLoadingInbox = theInbox.LoadInbox();
                    bool bSuccessLoadingOutbox = theOutbox.LoadOutbox();
                    // ...or generate them otherwise...

                    if (true ==
                        bSuccessLoadingInbox)  // WEIRD IF THIS HAPPENED...
                        bSuccessLoadingInbox = theInbox.VerifyAccount(
                            server_->m_nymServer);  // todo -- this should
                                                    // NEVER
                    // happen, the ID was just
                    // RANDOMLY generated, so HOW did
                    // the inbox already exist???
                    else {
                        bSuccessLoadingInbox = theInbox.GenerateLedger(
                            theNewAccountID, NOTARY_ID, Ledger::inbox, true);

                        if (bSuccessLoadingInbox) {
                            bSuccessLoadingInbox =
                                theInbox.SignContract(server_->m_nymServer);

                            if (bSuccessLoadingInbox) {
                                bSuccessLoadingInbox = theInbox.SaveContract();

                                if (bSuccessLoadingInbox)
                                    bSuccessLoadingInbox =
                                        pNewAccount->SaveInbox(theInbox);
                            }
                        }
                    }
                    if (true ==
                        bSuccessLoadingOutbox) {  // WEIRD IF THIS HAPPENED
                        bSuccessLoadingOutbox = theOutbox.VerifyAccount(
                            server_->m_nymServer);  // todo -- this should NEVER
                                                    // happen, the ID was just
                        // RANDOMLY generated, so HOW did
                        // the outbox already exist???
                    } else {
                        bSuccessLoadingOutbox = theOutbox.GenerateLedger(
                            theNewAccountID, NOTARY_ID, Ledger::outbox, true);

                        if (bSuccessLoadingOutbox) {
                            bSuccessLoadingOutbox =
                                theOutbox.SignContract(server_->m_nymServer);

                            if (bSuccessLoadingOutbox) {
                                bSuccessLoadingOutbox =
                                    theOutbox.SaveContract();

                                if (bSuccessLoadingOutbox)
                                    bSuccessLoadingOutbox =
                                        pNewAccount->SaveOutbox(theOutbox);
                            }
                        }
                    }
                    if (!bSuccessLoadingInbox) {
                        String strNewAcctID(theNewAccountID);

                        Log::vError(
                            "ERROR generating inbox ledger in "
                            "UserCommandProcessor::"
                            "UserCmdRegisterInstrumentDefinition:\n%"
                            "s\n",
                            strNewAcctID.Get());
                    } else if (!bSuccessLoadingOutbox) {
                        String strNewAcctID(theNewAccountID);

                        Log::vError(
                            "ERROR generating outbox ledger in "
                            "UserCommandProcessor::"
                            "UserCmdRegisterInstrumentDefinition:\n%"
                            "s\n",
                            strNewAcctID.Get());
                    } else {
                        msgOut.m_bSuccess = true;  // <==== SUCCESS!!

                        // On the server side, each nym stores a list of its
                        // asset accounts (IDs).
                        //
                        std::set<std::string>& theAccountSet =
                            theNym.GetSetAssetAccounts();
                        theAccountSet.insert(msgOut.m_strAcctID.Get());

                        theNym.SaveSignedNymfile(server_->m_nymServer);

                        // TODO fire off a separate process here to create
                        // the mint.
                        //
                        // THE PROGRAM ALREADY EXISTS (CreateMint) and you
                        // can RUN IT BY HAND FOR NOW.
                        // But in actual production environment, we'll
                        // trigger that executable here,
                        // and within a few minutes, users will be able to
                        // getMint successfully (and
                        // thus withdraw cash.)
                    }
                } else {
                    Log::Error("Failure generating new issuer account in "
                               "UserCommandProcessor::"
                               "UserCmdRegisterInstrumentDefinition.\n");
                }
            }
        }
    }

    // Either way, we need to send the user's command back to him as well.
    String tempInMessage(MsgIn);
    msgOut.m_ascInReferenceTo.SetString(tempInMessage);

    // (2) Sign the Message
    msgOut.SignContract(server_->m_nymServer);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    //
    // FYI, SaveContract takes m_xmlUnsigned and wraps it with the signatures
    // and ------- BEGIN  bookends
    // If you don't pass a string in, then SaveContract saves the new version to
    // its member, m_strRawFile
    msgOut.SaveContract();

    // (You are in UserCmdRegisterInstrumentDefinition.)

    // *************************************************************
    // REPLY NOTICE TO NYMBOX
    //
    // Now that we signed / saved the reply message...
    //
    // After specific messages, we drop a notice with a copy of the server's
    // reply
    // into the Nymbox.  This way we are GUARANTEED that the Nym will receive
    // and process
    // it. (And thus never get out of sync.)
    //
    if (msgOut.m_bSuccess) {
        const String strReplyMessage(msgOut);
        const int64_t lReqNum = MsgIn.m_strRequestNum.ToLong();
        // If it fails, it logs already.
        DropReplyNoticeToNymbox(
            NOTARY_ID,
            NYM_ID,
            strReplyMessage,
            lReqNum,
            false,  // trans success (not a transaction...)
            context,
            &theNym);
    }
}

/// An existing user is creating an issuer account (that he will not control)
/// based on a basket of currencies.
void UserCommandProcessor::UserCmdIssueBasket(
    Nym& theNym,
    Message& MsgIn,
    Message& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "issueBasketResponse";  // reply to issueBasket
    msgOut.m_strNymID = MsgIn.m_strNymID;         // NymID

    // Either way, we need to send the user's command back to him as well.
    {
        String tempInMessage(MsgIn);
        msgOut.m_ascInReferenceTo.SetString(tempInMessage);
    }

    const Identifier NYM_ID(theNym), NOTARY_ID(server_->m_strNotaryID),
        NOTARY_NYM_ID(server_->m_nymServer);

    auto serialized =
        proto::DataToProto<proto::UnitDefinition>(Data(MsgIn.m_ascPayload));

    if (!serialized.has_type()) {
        Log::vError("%s: Invalid unit definition.\n", __FUNCTION__);
    } else if (proto::UNITTYPE_BASKET != serialized.type()) {
        Log::vError("%s: Not a basket contract.\n", __FUNCTION__);
    } else {
        // The basket ID should be the same on all servers.
        // The basket contract ID will be unique on each server.
        //
        // The contract ID of the basket is calculated based on the UNSIGNED
        // portion of the contract
        // (so it is unique on every server) and for the same reason with the
        // AccountID removed before calculating.
        Identifier BASKET_ACCOUNT_ID, BASKET_CONTRACT_ID;
        Identifier BASKET_ID = BasketContract::CalculateBasketID(serialized);

        // Use BASKET_ID to look up the Basket account and see if it already
        // exists (the server keeps a list.)
        bool bFoundBasket = server_->transactor_.lookupBasketAccountID(
            BASKET_ID, BASKET_ACCOUNT_ID);

        if (bFoundBasket) {
            Log::vError(
                "%s: Rejected: user tried to create basket currency "
                "that already exists.\n",
                __FUNCTION__);
        } else  // Basket doesn't already exist -- so perhaps we can create it
                // then.
        {
            // Let's make sure that all the sub-currencies for this basket are
            // available on this server.
            // NOTE: this also prevents someone from using another basket as a
            // sub-currency UNLESS it already
            // exists on this server. (For example, they couldn't use a basket
            // contract from some other
            // server, since it wouldn't be issued here...) Also note that
            // registerInstrumentDefinition explicitly prevents
            // baskets from being issued -- you HAVE to use issueBasket for
            // creating any basket currency.
            // Taken in tandem, this insures that the only possible way to have
            // a basket currency as a sub-currency
            // is if it's already issued on this server.
            //
            bool bSubCurrenciesAllExist = true;

            for (auto& it : serialized.basket().item()) {
                std::string subcontractID = it.unit();
                auto pContract = OT::App().Contract().UnitDefinition(
                    Identifier(subcontractID));
                if (!pContract) {
                    Log::vError(
                        "%s: Failed: Sub-currency for basket is not "
                        "issued on this server: %s\n",
                        __FUNCTION__,
                        subcontractID.c_str());
                    bSubCurrenciesAllExist = false;
                    break;
                }
            }
            bool accountsReady = false;
            // By this point we know that the basket currency itself does NOT
            // already exist (good.)
            // We also know that all the subcurrencies DO already exist (good.)
            //
            if (bSubCurrenciesAllExist) {
                // GenerateNewAccount also expects the NymID to be stored in
                // m_strNymID.
                // Since we want the SERVER to be the user for basket accounts,
                // I'm setting it that
                // way in MsgIn so that GenerateNewAccount will create the
                // sub-account with the server
                // as the owner, instead of the user.
                NOTARY_NYM_ID.GetString(MsgIn.m_strNymID);

                // We need to actually create all the sub-accounts.
                // This loop also sets the Account ID onto the basket items
                // (which formerly was blank, from the client.)
                // This loop also adds the BASKET_ID and the NEW ACCOUNT ID to a
                // map on the server for later reference.

                for (auto& it : *serialized.mutable_basket()->mutable_item()) {
                    std::unique_ptr<Account> pNewAccount;

                    // GenerateNewAccount expects the Instrument Definition Id
                    // to be in MsgIn.
                    // So we'll just put it there to make things easy...
                    //
                    MsgIn.m_strInstrumentDefinitionID = String(it.unit());

                    pNewAccount.reset(Account::GenerateNewAccount(
                        NOTARY_NYM_ID,
                        NOTARY_ID,
                        server_->m_nymServer,
                        MsgIn,
                        Account::basketsub));

                    // If we successfully create the account, then bundle it
                    // in the message XML payload
                    //
                    if (nullptr != pNewAccount) {
                        String newAccountID;
                        pNewAccount->GetIdentifier(newAccountID);
                        it.set_account(newAccountID.Get());
                        accountsReady = true;
                    } else {
                        Log::vError(
                            "%s: Failed while calling: "
                            "OTAccount::GenerateNewAccount(SERVER_"
                            "NYM_ID, NOTARY_ID, m_nymServer, "
                            "MsgIn, OTAccount::basketsub)\n",
                            __FUNCTION__);
                        accountsReady = false;
                        break;
                    }
                }  // for

                std::shared_ptr<const UnitDefinition> contract;

                if (accountsReady) {
                    bool finalized = false;
                    auto nym = OT::App().Contract().Nym(NOTARY_NYM_ID);

                    if (nym) {
                        finalized =
                            BasketContract::FinalizeTemplate(nym, serialized);
                    }

                    if (finalized) {
                        if (proto::UNITTYPE_BASKET == serialized.type()) {
                            contract =
                                OT::App().Contract().UnitDefinition(serialized);

                            if (contract) {
                                BASKET_CONTRACT_ID = contract->ID();
                                msgOut.m_bSuccess = true;
                            } else {
                                otOut << __FUNCTION__ << ": Failed to construct"
                                      << " basket contract object."
                                      << std::endl;

                                msgOut.m_bSuccess = false;
                            }
                        } else {
                            otOut << __FUNCTION__ << ": Not a"
                                  << " basket contract object." << std::endl;

                            msgOut.m_bSuccess = false;
                        }
                    } else {
                        otOut << __FUNCTION__ << ": Failed to finalize"
                              << " basket contract object." << std::endl;

                        msgOut.m_bSuccess = false;
                    }
                } else {
                    otOut << __FUNCTION__ << ": Failed to create"
                          << " basket contract sub-accounts." << std::endl;

                    msgOut.m_bSuccess = false;
                }

                if (true == msgOut.m_bSuccess) {

                    // Grab the new instrument definition id for the new basket
                    // currency
                    const String STR_BASKET_CONTRACT_ID(contract->ID());

                    // set the new Instrument Definition ID, aka ContractID,
                    // onto the
                    // outgoing message.
                    msgOut.m_strInstrumentDefinitionID = STR_BASKET_CONTRACT_ID;

                    // I don't save this here. Instead, I wait for
                    // AddBasketAccountID and then I call SaveMainFile after
                    // that. See below.
                    // TODO need better code for reverting when something screws
                    // up halfway through a change.
                    // If I add this contract, it's not enough to just "not
                    // save" the file. I actually need to re-load the file
                    // in order to TRULY "make sure" this won't screw something
                    // up on down the line.

                    // Once the new Asset Type is generated, from which the
                    // BasketID can be requested at will, next we need to create
                    // the issuer account for that new Asset Type.  (We have the
                    // instrument definition ID and the contract file. Now let's
                    // create
                    // the issuer account the same as we would for any normal
                    // issuer account.)
                    //
                    // The issuer account is special compared to a normal issuer
                    // account, because within its walls, it must store an
                    // OTAccount for EACH sub-contract, in order to store the
                    // reserves. That's what makes the basket work.

                    Account* pBasketAccount = nullptr;

                    // GenerateNewAccount expects the Instrument Definition Id
                    // to be in MsgIn.
                    // So we'll just put it there to make things easy...
                    MsgIn.m_strInstrumentDefinitionID = STR_BASKET_CONTRACT_ID;

                    pBasketAccount = Account::GenerateNewAccount(
                        NOTARY_NYM_ID,
                        NOTARY_ID,
                        server_->m_nymServer,
                        MsgIn,
                        Account::basket);

                    if (nullptr != pBasketAccount) {
                        msgOut.m_bSuccess = true;

                        pBasketAccount->GetIdentifier(
                            msgOut.m_strAcctID);  // string
                        pBasketAccount->GetInstrumentDefinitionID().GetString(
                            msgOut.m_strInstrumentDefinitionID);

                        pBasketAccount->GetIdentifier(BASKET_ACCOUNT_ID);  // id

                        // So the server can later use the BASKET_ID (which is
                        // universal)
                        // to lookup the account ID on this server corresponding
                        // to that basket.
                        // (The account ID will be different from server to
                        // server, thus the need
                        // to be able to look it up via the basket ID.)
                        server_->transactor_.addBasketAccountID(
                            BASKET_ID, BASKET_ACCOUNT_ID, BASKET_CONTRACT_ID);

                        server_->mainFile_.SaveMainFile();  // So the main xml
                                                            // file loads
                                                            // this
                        // basket info next time we run.

                        delete pBasketAccount;
                        pBasketAccount = nullptr;
                    } else {
                        otOut << __FUNCTION__ << ": Failed to instantiate"
                              << " basket account." << std::endl;

                        msgOut.m_bSuccess = false;
                    }

                }  // if true == msgOut.m_bSuccess
            } else {
                otOut << __FUNCTION__ << ": missing sub-currencies."
                      << std::endl;
            }
        }
    }

    // (2) Sign the Message
    msgOut.SignContract(server_->m_nymServer);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    //
    // FYI, SaveContract takes m_xmlUnsigned and wraps it with the signatures
    // and ------- BEGIN  bookends
    // If you don't pass a string in, then SaveContract saves the new version to
    // its member, m_strRawFile
    msgOut.SaveContract();
}

/// An existing user is creating an asset account.
void UserCommandProcessor::UserCmdRegisterAccount(
    Nym& theNym,
    ClientContext& context,
    Message& MsgIn,
    Message& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand =
        "registerAccountResponse";         // reply to registerAccount
    msgOut.m_strNymID = MsgIn.m_strNymID;  // NymID

    // Either way, we need to send the user's command back to him as well.
    String tempInMessage(MsgIn);
    msgOut.m_ascInReferenceTo.SetString(tempInMessage);

    const Identifier NYM_ID(theNym), NOTARY_ID(server_->m_strNotaryID);

    std::unique_ptr<Account> pNewAccount(Account::GenerateNewAccount(
        NYM_ID, NOTARY_ID, server_->m_nymServer, MsgIn));

    // If we successfully create the account, then bundle it in the message XML
    // payload
    if (nullptr != pNewAccount) {
        const char* szFunc = "UserCommandProcessor::UserCmdRegisterAccount";
        auto pContract = OT::App().Contract().UnitDefinition(
            pNewAccount->GetInstrumentDefinitionID());

        if (!pContract) {
            const String strInstrumentDefinitionID(
                pNewAccount->GetInstrumentDefinitionID());
            Log::vError(
                "%s: Error: Unable to get UnitDefinition for "
                "instrument definition: %s\n",
                szFunc,
                strInstrumentDefinitionID.Get());
        } else if (pContract->Type() == proto::UNITTYPE_SECURITY) {
            // The instrument definition keeps a list of all accounts for that
            // type.
            // (For shares, not for currencies.)
            //
            const bool bAdded = pContract->AddAccountRecord(*pNewAccount);
            if (!bAdded) {
                const String strInstrumentDefinitionID(
                    pNewAccount->GetInstrumentDefinitionID());
                Log::vError(
                    "%s: ERROR Adding Account Record: %s ... Aborting.\n",
                    __FUNCTION__,
                    strInstrumentDefinitionID.Get());
                return;  // error
            }
        }
        Identifier theNewAccountID;
        pNewAccount->GetIdentifier(theNewAccountID);

        Ledger theOutbox(NYM_ID, theNewAccountID, NOTARY_ID),
            theInbox(NYM_ID, theNewAccountID, NOTARY_ID);

        bool bSuccessLoadingInbox = theInbox.LoadInbox();
        bool bSuccessLoadingOutbox = theOutbox.LoadOutbox();

        // ...or generate them otherwise...

        if (true == bSuccessLoadingInbox)  // WEIRD IF THIS HAPPENED...
            bSuccessLoadingInbox = theInbox.VerifyAccount(
                server_->m_nymServer);  // todo -- this should NEVER happen, the
                                        // ID was
        // just RANDOMLY generated, so HOW did the inbox
        // already exist???
        else {
            bSuccessLoadingInbox = theInbox.GenerateLedger(
                theNewAccountID, NOTARY_ID, Ledger::inbox, true);

            if (bSuccessLoadingInbox) {
                bSuccessLoadingInbox =
                    theInbox.SignContract(server_->m_nymServer);

                if (bSuccessLoadingInbox) {
                    bSuccessLoadingInbox = theInbox.SaveContract();

                    if (bSuccessLoadingInbox)
                        bSuccessLoadingInbox = pNewAccount->SaveInbox(theInbox);
                }
            }
        }

        if (true == bSuccessLoadingOutbox)  // WEIRD IF THIS HAPPENED....
            bSuccessLoadingOutbox = theOutbox.VerifyAccount(
                server_->m_nymServer);  // todo -- this should NEVER happen, the
                                        // ID was
        // just RANDOMLY generated, so HOW did the outbox
        // already exist???
        else {
            bSuccessLoadingOutbox = theOutbox.GenerateLedger(
                theNewAccountID, NOTARY_ID, Ledger::outbox, true);

            if (bSuccessLoadingOutbox) {
                bSuccessLoadingOutbox =
                    theOutbox.SignContract(server_->m_nymServer);

                if (bSuccessLoadingOutbox) {
                    bSuccessLoadingOutbox = theOutbox.SaveContract();

                    if (bSuccessLoadingOutbox)
                        bSuccessLoadingOutbox =
                            pNewAccount->SaveOutbox(theOutbox);
                }
            }
        }

        if (!bSuccessLoadingInbox) {
            const String strNewAcctID(theNewAccountID);

            Log::vError(
                "%s: ERROR generating inbox ledger: %s\n",
                szFunc,
                strNewAcctID.Get());
        } else if (!bSuccessLoadingOutbox) {
            const String strNewAcctID(theNewAccountID);

            Log::vError(
                "%s: ERROR generating outbox ledger: %s\n",
                szFunc,
                strNewAcctID.Get());
        } else {
            msgOut.m_bSuccess = true;  // <==== SUCCESS!!

            pNewAccount->GetIdentifier(msgOut.m_strAcctID);

            // On the server side, each nym stores a list of its asset accounts
            // (IDs).
            //
            std::set<std::string>& theAccountSet = theNym.GetSetAssetAccounts();
            theAccountSet.insert(msgOut.m_strAcctID.Get());

            theNym.SaveSignedNymfile(server_->m_nymServer);

            String tempPayload(*pNewAccount);
            msgOut.m_ascPayload.SetString(tempPayload);
        }
    }

    // (2) Sign the Message
    msgOut.SignContract(server_->m_nymServer);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    //
    // FYI, SaveContract takes m_xmlUnsigned and wraps it with the signatures
    // and ------- BEGIN  bookends
    // If you don't pass a string in, then SaveContract saves the new version to
    // its member, m_strRawFile
    msgOut.SaveContract();

    // (You are in UserCmdCreateAcct.)

    // *************************************************************
    // REPLY NOTICE TO NYMBOX
    //
    // Now that we signed / saved the reply message...
    //
    // After specific messages, we drop a notice with a copy of the server's
    // reply
    // into the Nymbox.  This way we are GUARANTEED that the Nym will receive
    // and process
    // it. (And thus never get out of sync.)
    //
    if (msgOut.m_bSuccess) {
        const String strReplyMessage(msgOut);
        const int64_t lReqNum = MsgIn.m_strRequestNum.ToLong();

        // If it fails, it logs already.
        DropReplyNoticeToNymbox(
            NOTARY_ID,
            NYM_ID,
            strReplyMessage,
            lReqNum,  // No need to update the NymboxHash in this case.
            false,    // trans success (not a transaction)
            context);
        //      DropReplyNoticeToNymbox(NOTARY_ID, NYM_ID,
        // strReplyMessage, lReqNum, &theNym);
    }
}

void UserCommandProcessor::UserCmdGetAccountData(
    Nym&,
    Message& MsgIn,
    Message& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "getAccountDataResponse";  // reply to getAccountData
    msgOut.m_strNymID = MsgIn.m_strNymID;            // NymID
    msgOut.m_strAcctID = MsgIn.m_strAcctID;  // The Account ID in question

    const Identifier NYM_ID(MsgIn.m_strNymID), ACCOUNT_ID(MsgIn.m_strAcctID),
        NOTARY_ID(MsgIn.m_strNotaryID);

    String strAccount, strInbox, strOutbox, strInboxHash, strOutboxHash;
    Account* pAccount = Account::LoadExistingAccount(ACCOUNT_ID, NOTARY_ID);
    bool bSuccessLoadingAccount = ((pAccount != nullptr) ? true : false);
    bool bSuccessLoadingInbox = false;
    bool bSuccessLoadingOutbox = false;
    if (bSuccessLoadingAccount)
        bSuccessLoadingAccount = (pAccount->GetNymID() == NYM_ID);
    // Yup the account exists. Yup it has the same user ID.
    if (bSuccessLoadingAccount) {
        // extract the account in ascii-armored form on the outgoing message
        pAccount->SaveContractRaw(
            strAccount);  // first grab it in plaintext string form

        // Get the Inbox.
        //
        {
            Ledger theInbox(NYM_ID, ACCOUNT_ID, NOTARY_ID);

            bSuccessLoadingInbox = theInbox.LoadInbox();

            if (!bSuccessLoadingInbox)
                Log::vError(
                    "%s: Failed trying to load Inbox from storage.\n",
                    __FUNCTION__);
            else {
                // We do NOT call VerifyAccount in this function (because we
                // don't need to) and thus we do NOT
                // force the box receipts to be loaded here (which happens
                // inside that call.) But we DO verify
                // the IDs and the Signature, of course.
                //
                bSuccessLoadingInbox =
                    (theInbox.VerifyContractID() &&
                     theInbox.VerifySignature(server_->m_nymServer));

                // If we loaded old data in this file... (when whole receipts
                // used to be stored in boxes.)
                //
                if (bSuccessLoadingInbox &&
                    theInbox.LoadedLegacyData())  // (which automatically saves
                                                  // the box receipt as the old
                                                  // data is loaded...)
                {
                    //                  bSuccessLoadingInbox =
                    // theInbox.VerifyAccount(server_->m_nymServer);    // Then
                    // Verify,
                    // which forces a LoadBoxReceipts... (

                    theInbox.ReleaseSignatures();  // UPDATE: We do NOT force
                                                   // the
                                                   // loading here, since they
                                                   // aren't needed.
                    theInbox.SignContract(
                        server_->m_nymServer);  // Waste of resources.
                                                // Instead, we recognize
                                                // that it was old data,
                                                // and so
                    theInbox.SaveContract();    // we gracefully re-save in the
                                                // new
                    // format, so it won't repeatedly
                    // be
                    theInbox.SaveInbox();  // loaded over and over again in the
                                           // large filesize.
                }

                if (!bSuccessLoadingInbox)
                    Log::vError(
                        "%s: Verification failed on Inbox after loading.\n",
                        __FUNCTION__);
            }
            if (bSuccessLoadingInbox) {
                theInbox.SaveContractRaw(strInbox);

                Identifier theHash;
                if (theInbox.CalculateInboxHash(theHash))
                    theHash.GetString(strInboxHash);
            }
        }
        // Now get the OUTBOX.
        //
        if (bSuccessLoadingInbox)  // (Which we don't bother to do unless the
                                   // inbox was already successful.)
        {
            Ledger theOutbox(NYM_ID, ACCOUNT_ID, NOTARY_ID);

            bSuccessLoadingOutbox = theOutbox.LoadOutbox();

            if (!bSuccessLoadingOutbox)
                Log::vError(
                    "%s: Failed trying to load Outbox from storage.\n",
                    __FUNCTION__);
            else {
                // We do NOT call VerifyAccount in this function (because we
                // don't need to) and thus we do NOT
                // force the box receipts to be loaded here (which happens
                // inside that call.) But we DO verify
                // the IDs and the Signature, of course.
                //
                bSuccessLoadingOutbox =
                    (theOutbox.VerifyContractID() &&
                     theOutbox.VerifySignature(server_->m_nymServer));

                // If we loaded old data in this file... (when whole receipts
                // used to be stored in boxes.)
                //
                if (bSuccessLoadingOutbox &&
                    theOutbox.LoadedLegacyData())  // (which automatically saves
                                                   // the box receipt as the old
                                                   // data is loaded...)
                {
                    //                  bSuccessLoadingOutbox =
                    // theOutbox.VerifyAccount(server_->m_nymServer);    // Then
                    // Verify,
                    // which forces a LoadBoxReceipts... (

                    theOutbox.ReleaseSignatures();  // UPDATE: We do NOT force
                                                    // the loading here, since
                                                    // they aren't needed.
                    theOutbox.SignContract(
                        server_->m_nymServer);  // Waste of resources.
                                                // Instead, we
                                                // recognize that it
                                                // was old data, and so
                    theOutbox.SaveContract();   // we gracefully re-save in the
                                                // new format, so it won't
                                                // repeatedly be
                    theOutbox.SaveOutbox();     // loaded over and over again in
                                                // the
                                                // large filesize.
                }

                if (!bSuccessLoadingOutbox)
                    Log::vError(
                        "%s: Verification Failed on Outbox after loading.\n",
                        __FUNCTION__);
            }
            if (bSuccessLoadingOutbox) {
                theOutbox.SaveContractRaw(strOutbox);

                Identifier theHash;
                if (theOutbox.CalculateOutboxHash(theHash))
                    theHash.GetString(strOutboxHash);
            }
        }
    }
    // TODO optimize: Really only !SuccessLoadingOutbox is needed here.
    // If it is false, then the others are definitely false as well.
    //
    if (!bSuccessLoadingOutbox || !bSuccessLoadingInbox ||
        !bSuccessLoadingAccount) {
        // FAILURE: (Send the user's command back to him.)
        //
        msgOut.m_bSuccess = false;
        String tempInMessage(
            MsgIn);  // Grab the incoming message in plaintext form
        msgOut.m_ascInReferenceTo.SetString(tempInMessage);  // Set it into the
                                                             // base64-encoded
                                                             // object on the
                                                             // outgoing message
    } else                                                   // SUCCESS.
    {
        msgOut.m_ascPayload.SetString(strAccount);
        msgOut.m_ascPayload2.SetString(strInbox);
        msgOut.m_ascPayload3.SetString(strOutbox);
        msgOut.m_strInboxHash = strInboxHash;
        msgOut.m_strOutboxHash = strOutboxHash;
        msgOut.m_bSuccess = true;
    }
    // (2) Sign the Message
    msgOut.SignContract(static_cast<const Nym&>(server_->m_nymServer));

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    //
    // FYI, SaveContract takes m_xmlUnsigned and wraps it with the signatures
    // and ------- BEGIN  bookends
    // If you don't pass a string in, then SaveContract saves the new version to
    // its member, m_strRawFile
    msgOut.SaveContract();
}

void UserCommandProcessor::UserCmdQueryInstrumentDefinitions(
    Nym&,
    Message& MsgIn,
    Message& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "queryInstrumentDefinitionsResponse";  // reply to
    // queryInstrumentDefinitions
    msgOut.m_strNymID = MsgIn.m_strNymID;  // NymID
    msgOut.m_bSuccess = false;

    // Send the user's command back to him whether success or failure.
    String tempInMessage(MsgIn);  // Grab the incoming message in plaintext form
    msgOut.m_ascInReferenceTo.SetString(tempInMessage);  // Set it into the
    // base64-encoded object
    // on the outgoing
    // message

    if (MsgIn.m_ascPayload.Exists())  // (which it should)
    {
        std::unique_ptr<OTDB::Storable> pStorable(OTDB::DecodeObject(
            OTDB::STORED_OBJ_STRING_MAP, MsgIn.m_ascPayload.Get()));
        OTDB::StringMap* pMap = dynamic_cast<OTDB::StringMap*>(pStorable.get());

        if (nullptr !=
            pMap)  // There was definitely a StringMap in the payload.
        {
            msgOut.m_bSuccess = true;

            std::map<std::string, std::string>& theMap = pMap->the_map;
            std::map<std::string, std::string> theNewMap;

            for (auto& it : theMap) {
                const std::string& str1 =
                    it.first;  // Containing the instrument definition ID.
                const std::string& str2 =
                    it.second;  // Containing the phrase "exists". (More are
                                // possible in the future.)

                // todo security: limit on length of this map? (sent through
                // user message...)

                // "exists" means, "Here's an instrument definition id. Please
                // tell me
                // whether or not it's actually issued on this server."
                // Future options might include "issue", "audit", "contract",
                // etc.
                //
                if ((str1.size() > 0) &&
                    (str2.compare("exists") == 0))  // todo hardcoding
                {
                    auto pContract =
                        OT::App().Contract().UnitDefinition(Identifier(str1));
                    if (pContract)  // Yes, it exists.
                        theNewMap[str1] = "true";
                    else
                        theNewMap[str1] = "false";
                }
            }

            // Replace contents of old map with contents of new map.
            //
            theMap.clear();
            theMap = theNewMap;
            // Serialize the StringMap back to a string...

            std::string str_Encoded = OTDB::EncodeObject(*pMap);

            if (str_Encoded.size() > 0)
                msgOut.m_ascPayload =
                    str_Encoded.c_str();  // now the outgoing message has the
                                          // response map in its payload, in
                                          // base64 form.
            else
                msgOut.m_bSuccess = false;  // Something went wrong.
        }                                   // if pMap exists.
    } else {
        msgOut.m_bSuccess = false;
    }

    // (2) Sign the Message
    msgOut.SignContract(static_cast<const Nym&>(server_->m_nymServer));

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    //
    // FYI, SaveContract takes m_xmlUnsigned and wraps it with the signatures
    // and ------- BEGIN  bookends
    // If you don't pass a string in, then SaveContract saves the new version to
    // its member, m_strRawFile
    msgOut.SaveContract();
}

void UserCommandProcessor::UserCmdGetInstrumentDefinition(
    Message& MsgIn,
    Message& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand =
        "getInstrumentDefinitionResponse";  // reply to getInstrumentDefinition
    msgOut.m_strNymID = MsgIn.m_strNymID;   // NymID
    msgOut.m_strInstrumentDefinitionID =
        MsgIn.m_strInstrumentDefinitionID;  // The Instrument Definition ID in
                                            // question

    const Identifier INSTRUMENT_DEFINITION_ID(
        MsgIn.m_strInstrumentDefinitionID);

    Data serialized;
    auto unitDefiniton =
        OT::App().Contract().UnitDefinition(INSTRUMENT_DEFINITION_ID);
    // Perhaps the provided ID is actually a server contract, not an
    // instrument definition?
    auto server = OT::App().Contract().Server(INSTRUMENT_DEFINITION_ID);

    // Yup the asset contract exists.
    if (unitDefiniton) {
        msgOut.m_bSuccess = true;
        serialized = proto::ProtoAsData<proto::UnitDefinition>(
            unitDefiniton->PublicContract());
        msgOut.m_ascPayload.SetData(serialized);  // now the outgoing message
                                                  // has the contract in its
                                                  // payload in base64 form.
    } else if (server) {
        msgOut.m_bSuccess = true;
        serialized =
            proto::ProtoAsData<proto::ServerContract>(server->PublicContract());
        msgOut.m_ascPayload.SetData(serialized);
    }
    // Send the user's command back to him if failure.
    else {
        msgOut.m_bSuccess = false;
        String tempInMessage(
            MsgIn);  // Grab the incoming message in plaintext form
        msgOut.m_ascInReferenceTo.SetString(tempInMessage);  // Set it into the
                                                             // base64-encoded
                                                             // object on the
                                                             // outgoing message
    }

    // (2) Sign the Message
    msgOut.SignContract(static_cast<const Nym&>(server_->m_nymServer));

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    //
    // FYI, SaveContract takes m_xmlUnsigned and wraps it with the signatures
    // and ------- BEGIN  bookends
    // If you don't pass a string in, then SaveContract saves the new version to
    // its member, m_strRawFile
    msgOut.SaveContract();
}

void UserCommandProcessor::UserCmdTriggerClause(
    Nym& theNym,
    ClientContext& context,
    Message& MsgIn,
    Message& msgOut)
{
    String strInReferenceTo(
        MsgIn);  // Grab the incoming message in plaintext form
    msgOut.m_ascInReferenceTo.SetString(strInReferenceTo);
    // (1) set up member variables
    msgOut.m_strCommand = "triggerClauseResponse";  // reply to triggerClause
    msgOut.m_strNymID = MsgIn.m_strNymID;           // NymID
    msgOut.m_bSuccess = false;                      // Default value.
    const Identifier NOTARY_ID(server_->m_strNotaryID);
    const bool hashMatch = context.NymboxHashMatch();

    if (context.HaveLocalNymboxHash()) {
        context.LocalNymboxHash().GetString(msgOut.m_strNymboxHash);
    }

    if (!hashMatch) {
        Log::vOutput(
            0,
            "%s: Rejecting message since nymbox hash doesn't match. "
            "(Send a getNymbox message to grab the newest one.)\n",
            __FUNCTION__);
    } else {
        OTSmartContract* pSmartContract = nullptr;
        OTCronItem* pCronItem =
            server_->m_Cron.GetItemByValidOpeningNum(MsgIn.m_lTransactionNum);

        if (nullptr == pCronItem) {
            Log::vOutput(
                0,
                "%s: Couldn't find smart contract based on "
                "transaction #: %" PRId64 " \n",
                __FUNCTION__,
                MsgIn.m_lTransactionNum);
        }
        // Also: CAN this guy trigger it?
        else if (
            nullptr ==
            (pSmartContract = dynamic_cast<OTSmartContract*>(pCronItem))) {
            Log::vOutput(
                0,
                "%s: Found cron item %" PRId64 " based on %" PRId64
                ", but it wasn't a "
                "smart contract. \n",
                __FUNCTION__,
                pCronItem->GetTransactionNum(),
                MsgIn.m_lTransactionNum);
        } else {
            // FIND THE PARTY / PARTY NAME
            OTAgent* pAgent = nullptr;
            OTParty* pParty =
                pSmartContract->FindPartyBasedOnNymAsAgent(theNym, &pAgent);

            if (nullptr == pParty) {
                Log::vOutput(
                    0,
                    "%s: Unable to find party to this contract "
                    "(%" PRId64 " based on %" PRId64 ") "
                    "based on Nym as agent: %s",
                    __FUNCTION__,
                    pCronItem->GetTransactionNum(),
                    MsgIn.m_lTransactionNum,
                    MsgIn.m_strNymID.Get());
            } else {
                bool bSuccess = false;
                const std::string str_clause_name = MsgIn.m_strNymID2.Get();

                if (pSmartContract->CanExecuteClause(
                        pParty->GetPartyName(), str_clause_name)) {
                    // Execute the clause.
                    //
                    mapOfClauses theMatchingClauses;
                    OTClause* pClause =
                        pSmartContract->GetClause(str_clause_name);

                    if (nullptr != pClause) {
                        Log::vOutput(
                            0,
                            "%s: At party request, processing "
                            "smart contract clause: %s \n",
                            __FUNCTION__,
                            str_clause_name.c_str());

                        theMatchingClauses.insert(
                            std::pair<std::string, OTClause*>(
                                str_clause_name, pClause));

                        pSmartContract->ExecuteClauses(theMatchingClauses);

                        if (pSmartContract->IsFlaggedForRemoval()) {
                            Log::vOutput(
                                0,
                                "%s: Removing smart contract "
                                "from cron processing: %" PRId64 "\n",
                                __FUNCTION__,
                                pSmartContract->GetTransactionNum());
                        }
                        bSuccess = true;
                    } else {
                        Log::vOutput(
                            0,
                            "%s: Failed attempt to process "
                            "clause (%s) on smart contract: %" PRId64 " "
                            "\n",
                            __FUNCTION__,
                            str_clause_name.c_str(),
                            pSmartContract->GetTransactionNum());
                    }
                }

                // If we just removed the smart contract from cron, that means a
                // finalReceipt was just dropped
                // into the inboxes for the relevant asset accounts. Once I
                // process that receipt out of my
                // inbox, (which will require my processing out all related
                // marketReceipts) then the closing
                // number will be removed from my list of responsibility.

                if (bSuccess) {
                    // Now we can set the response item as an acknowledgement
                    // instead of the default (rejection)
                    Log::vOutput(
                        0,
                        "%s: Party (%s) successfully triggered clause: %s.\n",
                        __FUNCTION__,
                        pParty->GetPartyName().c_str(),
                        str_clause_name.c_str());

                    msgOut.m_bSuccess = true;
                } else
                    Log::vOutput(
                        0,
                        "%s:  Unable to trigger clause %s at "
                        "request of party %s. "
                        "(Either the permission wasn't there, or "
                        "the clause wasn't found.)\n",
                        __FUNCTION__,
                        str_clause_name.c_str(),
                        pParty->GetPartyName().c_str());
            }
        }  // else found smart contract.
    }      // NymboxHash matches.

    if (context.HaveLocalNymboxHash()) {
        context.LocalNymboxHash().GetString(msgOut.m_strNymboxHash);
    }

    // (2) Sign the Message
    msgOut.SignContract(static_cast<const Nym&>(server_->m_nymServer));

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    //
    // FYI, SaveContract takes m_xmlUnsigned and wraps it with the signatures
    // and ------- BEGIN  bookends
    // If you don't pass a string in, then SaveContract saves the new version to
    // its member, m_strRawFile
    msgOut.SaveContract();
}

void UserCommandProcessor::UserCmdGetMint(Nym&, Message& MsgIn, Message& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "getMintResponse";  // reply to getMint
    msgOut.m_strNymID = MsgIn.m_strNymID;     // NymID
    msgOut.m_strInstrumentDefinitionID =
        MsgIn.m_strInstrumentDefinitionID;  // The Instrument Definition ID in
                                            // question

    const Identifier INSTRUMENT_DEFINITION_ID(
        MsgIn.m_strInstrumentDefinitionID);
    const String INSTRUMENT_DEFINITION_ID_STR(INSTRUMENT_DEFINITION_ID);
    bool bSuccessLoadingMint = false;

    std::unique_ptr<Mint> pMint(Mint::MintFactory(
        server_->m_strNotaryID, INSTRUMENT_DEFINITION_ID_STR));
    OT_ASSERT(nullptr != pMint);
    if (true == (bSuccessLoadingMint = pMint->LoadMint(".PUBLIC"))) {
        // You cannot hash the Mint to get its ID.
        // (The ID is a hash of the asset contract, not the mint contract.)
        // Instead, you must READ the ID from the Mint file, and then compare it
        // to the one expected
        // to see if they match (similar to how Account IDs are verified.)

        bSuccessLoadingMint = pMint->VerifyMint(server_->m_nymServer);

        // Yup the asset contract exists.
        if (bSuccessLoadingMint) {
            msgOut.m_bSuccess = true;

            // extract the account in ascii-armored form on the outgoing message
            String strPayload(
                *pMint);  // first grab it in plaintext string form
            msgOut.m_ascPayload.SetString(strPayload);  // now the outgoing
                                                        // message has the inbox
                                                        // ledger in its payload
                                                        // in base64 form.
        }
        // Send the user's command back to him if failure.
    }

    if (!bSuccessLoadingMint) {
        msgOut.m_bSuccess = false;
        String tempInMessage(
            MsgIn);  // Grab the incoming message in plaintext form
        msgOut.m_ascInReferenceTo.SetString(tempInMessage);  // Set it into the
                                                             // base64-encoded
                                                             // object on the
                                                             // outgoing message
    }

    // (2) Sign the Message
    msgOut.SignContract(static_cast<const Nym&>(server_->m_nymServer));

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    //
    // FYI, SaveContract takes m_xmlUnsigned and wraps it with the signatures
    // and ------- BEGIN  bookends
    // If you don't pass a string in, then SaveContract saves the new version to
    // its member, m_strRawFile
    msgOut.SaveContract();
}

// If a user requests to delete his own Nym, the server will allow it.
// IF: If the transaction numbers are all closable (available on both lists).
// AND if the Nymbox is empty. AND if there are no cron items open, AND if
// there are no asset accounts! (Delete them / Close them all FIRST! Or this
// fails.)
//
void UserCommandProcessor::UserCmdDeleteUser(
    Nym& theNym,
    ClientContext& context,
    Message& MsgIn,
    Message& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "unregisterNymResponse";  // reply to unregisterNym
    msgOut.m_strNymID = MsgIn.m_strNymID;           // NymID

    const Identifier NYM_ID(MsgIn.m_strNymID), NOTARY_ID(MsgIn.m_strNotaryID);

    Ledger theLedger(NYM_ID, NYM_ID, NOTARY_ID);

    // If success loading Nymbox, and there are transactions still inside, THEN
    // FAIL!!! (Can't delete a Nym with open receipts...)
    const bool bSuccessLoadNymbox =
        (theLedger.LoadNymbox() &&
         theLedger.VerifyAccount(server_->m_nymServer));

    if (!bSuccessLoadNymbox) {
        Log::Output(
            3,
            "Tried to delete Nym, but failed loading or verifying "
            "the Nymbox.\n");
        msgOut.m_bSuccess = false;
    } else if (theLedger.GetTransactionCount() > 0) {
        Log::Output(
            3,
            "Tried to delete Nym, but there are still receipts in "
            "the Nymbox. (Process them first.)\n");
        msgOut.m_bSuccess = false;
    } else if (0 < context.OpenCronItems()) {
        Log::Output(
            3,
            "Tried to delete Nym, but there are still open Cron "
            "Items. (Close them first.)\n");
        msgOut.m_bSuccess = false;
    } else if (theNym.GetSetAssetAccounts().size() > 0) {
        Log::Output(
            3,
            "Tried to delete Nym, but there are still Asset "
            "Accounts open for that Nym. (Close them first.)\n");
        msgOut.m_bSuccess = false;
    }
    // The Nym has used some of his transaction numbers, but hasn't closed them
    // out yet. Close those transactions first.
    else if (context.hasOpenTransactions()) {
        Log::Output(
            3,
            "Tried to delete Nym, but there are still "
            "transactions open for that Nym. (Close them "
            "first.)\n");
        msgOut.m_bSuccess = false;
    } else {
        msgOut.m_bSuccess = true;

        // The Nym may have some numbers signed out, but none of them have come
        // through and been "used but not closed" yet. (That is, removed from
        // transaction num list but still on issued num list.) If they had (i.e.
        // if the previous elseif just above had discovered mismatched counts)
        // then we wouldn't be able to delete the Nym until those transactions
        // were closed. Since we know the counts match perfectly, here we remove
        // all the numbers. The client side must know to remove all the numbers
        // as well, when it receives a successful reply that the nym was
        // "deleted."
        context.Reset();
        // The nym isn't actually deleted yet, just marked for deletion. It will
        // get cleaned up later, during server maintenance.
        theNym.MarkForDeletion();

        // SAVE the Nym... (now marked for deletion and with all of its
        // transaction numbers removed.)
        theNym.SaveSignedNymfile(server_->m_nymServer);
    }

    String tempInMessage(MsgIn);
    msgOut.m_ascInReferenceTo.SetString(tempInMessage);

    // (2) Sign the Message
    msgOut.SignContract(static_cast<const Nym&>(server_->m_nymServer));

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    msgOut.SaveContract();

    // (You are in UserCmdDeleteUser.)

    // TODO: We may also need to mark the Nymbox, as well as the credential
    // files, as "Marked For Deletion."

    // REPLY NOTICE TO NYMBOX
    //
    // Now that we signed / saved the reply message...
    //
    // After specific messages, we drop a notice with a copy of the server's
    // reply into the Nymbox.  This way we are GUARANTEED that the Nym will
    // receive and process it. (And thus never get out of sync.)
    if (msgOut.m_bSuccess) {
        const String strReplyMessage(msgOut);
        const int64_t lReqNum = MsgIn.m_strRequestNum.ToLong();

        // If it fails, it logs already.
        DropReplyNoticeToNymbox(
            NOTARY_ID,
            NYM_ID,
            strReplyMessage,
            lReqNum,
            false,  // trans success
            context,
            &theNym);
    }
}

// the "accountID" on this message will contain the NymID if retrieving a
// boxreceipt for
// the Nymbox. Otherwise it will contain an AcctID if retrieving a boxreceipt
// for an Asset Acct.
//
void UserCommandProcessor::UserCmdGetBoxReceipt(Message& MsgIn, Message& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "getBoxReceiptResponse";  // reply to getBoxReceipt
    msgOut.m_strNymID = MsgIn.m_strNymID;           // NymID
    msgOut.m_strAcctID = MsgIn.m_strAcctID;         // the asset account ID
                                                    // (inbox/outbox), or Nym ID
                                                    // (nymbox)
    msgOut.m_lTransactionNum = MsgIn.m_lTransactionNum;  // TransactionNumber
                                                         // for the receipt in
                                                         // the box
                                                         // (unique to the box.)
    msgOut.m_lDepth = MsgIn.m_lDepth;
    msgOut.m_bSuccess = false;

    const Identifier NYM_ID(MsgIn.m_strNymID), NOTARY_ID(MsgIn.m_strNotaryID),
        ACCOUNT_ID(MsgIn.m_strAcctID);

    std::unique_ptr<Ledger> pLedger(new Ledger(NYM_ID, ACCOUNT_ID, NOTARY_ID));

    bool bErrorCondition = false;
    bool bSuccessLoading = false;

    switch (MsgIn.m_lDepth) {
        case 0:  // Nymbox
            if (NYM_ID == ACCOUNT_ID) {
                // It's verified using VerifyAccount() below this switch block.
                bSuccessLoading = pLedger->LoadNymbox();
            } else  // Inbox / Outbox.
            {
                Log::vError(
                    "UserCommandProcessor::UserCmdGetBoxReceipt: User "
                    "requested "
                    "Nymbox, but "
                    "failed to provide the "
                    "NymID (%s) in the AccountID (%s) field as expected.\n",
                    MsgIn.m_strNymID.Get(),
                    MsgIn.m_strAcctID.Get());
                bErrorCondition = true;
            }
            break;
        case 1:  // Inbox
            if (NYM_ID == ACCOUNT_ID) {
                Log::vError(
                    "UserCommandProcessor::UserCmdGetBoxReceipt: User "
                    "requested "
                    "Inbox, but erroneously provided the "
                    "NymID (%s) in the AccountID (%s) field.\n",
                    MsgIn.m_strNymID.Get(),
                    MsgIn.m_strAcctID.Get());
                bErrorCondition = true;
            } else {
                // It's verified using VerifyAccount() below this switch block.
                bSuccessLoading = pLedger->LoadInbox();
            }
            break;
        case 2:  // Outbox
            if (NYM_ID == ACCOUNT_ID) {
                Log::vError(
                    "UserCommandProcessor::UserCmdGetBoxReceipt: User "
                    "requested "
                    "Outbox, but erroneously provided the "
                    "NymID (%s) in the AccountID (%s) field.\n",
                    MsgIn.m_strNymID.Get(),
                    MsgIn.m_strAcctID.Get());
                bErrorCondition = true;
            } else {
                // It's verified using VerifyAccount() below this switch block.
                bSuccessLoading = pLedger->LoadOutbox();
            }
            break;
        default:
            Log::vError(
                "UserCommandProcessor::UserCmdGetBoxReceipt: Unknown box "
                "type: %" PRId64 "\n",
                MsgIn.m_lDepth);
            bErrorCondition = true;
            break;
    }
    // At this point, we have the box loaded. Now let's use it to
    // load the appropriate box receipt...

    if (bSuccessLoading && !bErrorCondition &&
        // This call
        // causes all the Box Receipts to be loaded up and we don't need them
        // here, except
        pLedger->VerifyContractID() &&  // for just one, so we're going to
                                        // VerifyContractID and Signature
                                        // instead. Then below, we'll
        pLedger->VerifySignature(
            server_->m_nymServer)  // just load the one we actually need.
        ) {
        OTTransaction* pTransaction =
            pLedger->GetTransaction(MsgIn.m_lTransactionNum);
        if (nullptr == pTransaction) {
            Log::vError(
                "UserCommandProcessor::UserCmdGetBoxReceipt: User requested a "
                "transaction number "
                "(%" PRId64 ") that's not in the %s. NymID (%s) and "
                "AccountID (%s) FYI.\n",
                MsgIn.m_lTransactionNum,
                (MsgIn.m_lDepth == 0)
                    ? "nymbox"
                    : ((MsgIn.m_lDepth == 1) ? "inbox"
                                             : "outbox"),  // outbox is 2.
                MsgIn.m_strNymID.Get(),
                MsgIn.m_strAcctID.Get());
        } else {
            pLedger->LoadBoxReceipt(MsgIn.m_lTransactionNum);

            // The above call will replace pTransaction, inside pLedger, with
            // the full version
            // (instead of the abbreviated version) of that transaction, meaning
            // that the pTransaction
            // pointer is now a bad pointer, if that call was successful.
            // Therefore we just call GetTransaction() AGAIN. This way, whether
            // LoadBoxReceipt()
            // failed or not (perhaps it's legacy data and is already not
            // abbreviated, and thus the
            // LoadBoxReceipt call failed, but that's doesn't mean we're going
            // to fail HERE, now does it?)
            //
            pTransaction = pLedger->GetTransaction(MsgIn.m_lTransactionNum);

            if ((nullptr != pTransaction) && !pTransaction->IsAbbreviated() &&
                pTransaction->VerifyContractID() &&
                pTransaction->VerifySignature(server_->m_nymServer)) {
                // Okay so after finding it, then calling LoadBoxReceipt(), then
                // finding it again,
                // it's definitely not abbreviated by this point. Success!
                // LoadBoxReceipt() already calls VerifyBoxReceipt(), FYI.
                //
                const String strBoxReceipt(*pTransaction);
                OT_ASSERT(strBoxReceipt.Exists());

                msgOut.m_ascPayload.SetString(strBoxReceipt);
                msgOut.m_bSuccess = true;

                Log::vOutput(
                    3,
                    "UserCommandProcessor::UserCmdGetBoxReceipt: Success: "
                    "User is "
                    "retrieving the box receipt for transaction number "
                    "%" PRId64 " in the %s for NymID (%s) AccountID (%s).\n",
                    MsgIn.m_lTransactionNum,
                    (MsgIn.m_lDepth == 0)
                        ? "nymbox"
                        : ((MsgIn.m_lDepth == 1) ? "inbox"
                                                 : "outbox"),  // outbox is 2.
                    MsgIn.m_strNymID.Get(),
                    MsgIn.m_strAcctID.Get());
            } else {
                Log::vError(
                    "UserCommandProcessor::UserCmdGetBoxReceipt: User "
                    "requested a "
                    "transaction number (%" PRId64 ") that's "
                    "failing to retrieve from the %s, AFTER calling "
                    "LoadBoxReceipt(). (Though it worked BEFORE calling it.) "
                    "NymID (%s) and AccountID (%s) FYI. IsAbbreviated == %s\n",
                    MsgIn.m_lTransactionNum,
                    (MsgIn.m_lDepth == 0)
                        ? "nymbox"
                        : ((MsgIn.m_lDepth == 1) ? "inbox"
                                                 : "outbox"),  // outbox is 2.
                    MsgIn.m_strNymID.Get(),
                    MsgIn.m_strAcctID.Get(),
                    (nullptr == pTransaction)
                        ? "nullptr"
                        : (pTransaction->IsAbbreviated()) ? "true" : "false");
            }
        }
    } else {
        Log::vError(
            "UserCommandProcessor::UserCmdGetBoxReceipt: Failed loading or "
            "verifying %s. "
            "Transaction (%" PRId64 "), NymID (%s) and AccountID (%s) FYI.\n",
            (MsgIn.m_lDepth == 0)
                ? "nymbox"
                : ((MsgIn.m_lDepth == 1) ? "inbox" : "outbox"),  // outbox is 2.
            MsgIn.m_lTransactionNum,
            MsgIn.m_strNymID.Get(),
            MsgIn.m_strAcctID.Get());
    }

    // Grab the incoming message in plaintext form
    const String tempInMessage(MsgIn);
    // Set it into the base64-encoded object on the outgoing message
    msgOut.m_ascInReferenceTo.SetString(tempInMessage);

    // (2) Sign the Message
    msgOut.SignContract(static_cast<const Nym&>(server_->m_nymServer));

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    //
    // FYI, SaveContract takes m_xmlUnsigned and wraps it with the signatures
    // and ------- BEGIN  bookends
    // If you don't pass a string in, then SaveContract saves the new version to
    // its member, m_strRawFile
    msgOut.SaveContract();
}

// If the client wants to delete an asset account, the server will allow it...
// ...IF: the Inbox and Outbox are both EMPTY. AND the Balance must be empty as
// well!
//
void UserCommandProcessor::UserCmdDeleteAssetAcct(
    Nym& theNym,
    ClientContext& context,
    Message& MsgIn,
    Message& msgOut)
{
    const char* szFunc = "UserCommandProcessor::UserCmdDeleteAssetAcct";

    // (1) set up member variables
    msgOut.m_strCommand =
        "unregisterAccountResponse";         // reply to unregisterAccount
    msgOut.m_strNymID = MsgIn.m_strNymID;    // NymID
    msgOut.m_strAcctID = MsgIn.m_strAcctID;  // the asset account being deleted.

    const Identifier NYM_ID(MsgIn.m_strNymID), NOTARY_ID(MsgIn.m_strNotaryID),
        ACCOUNT_ID(MsgIn.m_strAcctID);

    std::unique_ptr<Account> pAccount(
        Account::LoadExistingAccount(ACCOUNT_ID, NOTARY_ID));

    if (nullptr == pAccount || !pAccount->VerifyAccount(server_->m_nymServer)) {
        Log::vError(
            "%s: Error loading or verifying account: %s\n",
            szFunc,
            MsgIn.m_strAcctID.Get());
    } else if (pAccount->GetBalance() != 0) {
        Log::vOutput(
            1,
            "%s: Failed while trying to delete asset account %s: "
            "Balance must be zero to do this!\n",
            szFunc,
            MsgIn.m_strAcctID.Get());
    } else {
        std::unique_ptr<Ledger> pInbox(
            pAccount->LoadInbox(server_->m_nymServer));
        std::unique_ptr<Ledger> pOutbox(
            pAccount->LoadOutbox(server_->m_nymServer));

        if (nullptr == pInbox) {
            Log::vError(
                "%s: Error loading or verifying inbox: %s\n",
                szFunc,
                MsgIn.m_strAcctID.Get());
        } else if (nullptr == pOutbox) {
            Log::vError(
                "%s: Error loading or verifying outbox: %s\n",
                szFunc,
                MsgIn.m_strAcctID.Get());
        } else if (pInbox->GetTransactionCount() > 0) {
            Log::vOutput(
                3,
                "%s: Tried to delete asset account, but there are still "
                "receipts in the Inbox. (Process them first.)\n",
                szFunc);
            msgOut.m_bSuccess = false;
        } else if (pOutbox->GetTransactionCount() > 0) {
            Log::vOutput(
                3,
                "%s: Tried to delete asset account, but there are still "
                "receipts in the Outbox. (Process them first.)\n",
                szFunc);
            msgOut.m_bSuccess = false;
        } else  // SUCCESS!
        {
            msgOut.m_bSuccess = true;

            std::set<std::string>& theAccountSet = theNym.GetSetAssetAccounts();
            theAccountSet.erase(MsgIn.m_strAcctID.Get());

            theNym.SaveSignedNymfile(server_->m_nymServer);
            auto pContract = OT::App().Contract().UnitDefinition(
                pAccount->GetInstrumentDefinitionID());

            if (!pContract) {
                const String strInstrumentDefinitionID(
                    pAccount->GetInstrumentDefinitionID());
                Log::vError(
                    "%s: Error: Unable to get UnitDefinition for "
                    "instrument definition: %s\n",
                    szFunc,
                    strInstrumentDefinitionID.Get());
            } else if (pContract->Type() == proto::UNITTYPE_SECURITY) {
                // The instrument definition keeps a list of all accounts for
                // that type.
                // (For shares, not for currencies.)
                //
                const bool bErased = pContract->EraseAccountRecord(
                    pAccount->GetInstrumentDefinitionID());
                if (!bErased) {
                    const String strInstrumentDefinitionID(
                        pAccount->GetInstrumentDefinitionID());
                    Log::vError(
                        "%s: ERROR Erasing Account Record: %s ... Aborting.\n",
                        __FUNCTION__,
                        strInstrumentDefinitionID.Get());
                    return;  // error
                }
            }

            pAccount->MarkForDeletion();  // The account isn't actually deleted
                                          // yet, just marked for deletion.
            //  It will get cleaned up later, during server maintenance.

            // SAVE the Account... (NOW THAT IT IS MARKED FOR DELETION.)
            //
            pAccount->ReleaseSignatures();
            pAccount->SignContract(server_->m_nymServer);
            pAccount->SaveContract();
            pAccount->SaveAccount();
        }
    }

    // Send the user's command back to him (success or failure.)
    {
        String tempInMessage(
            MsgIn);  // Grab the incoming message in plaintext form
        msgOut.m_ascInReferenceTo.SetString(tempInMessage);  // Set it into the
                                                             // base64-encoded
                                                             // object on the
                                                             // outgoing message
    }

    // (2) Sign the Message
    msgOut.SignContract(static_cast<const Nym&>(server_->m_nymServer));

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    //
    // FYI, SaveContract takes m_xmlUnsigned and wraps it with the signatures
    // and ------- BEGIN  bookends
    // If you don't pass a string in, then SaveContract saves the new version to
    // its member, m_strRawFile
    msgOut.SaveContract();

    // (You are in UserCmdDeleteAssetAcct.)

    // REPLY NOTICE TO NYMBOX
    //
    // Now that we signed / saved the reply message...
    //
    // After specific messages, we drop a notice with a copy of the server's
    // reply
    // into the Nymbox.  This way we are GUARANTEED that the Nym will receive
    // and process
    // it. (And thus never get out of sync.)
    //
    if (msgOut.m_bSuccess) {
        const String strReplyMessage(msgOut);
        const int64_t lReqNum = MsgIn.m_strRequestNum.ToLong();

        // If it fails, it logs already.
        DropReplyNoticeToNymbox(
            NOTARY_ID,
            NYM_ID,
            strReplyMessage,
            lReqNum,
            false,  // trans success (not a transaction.)
            context,
            &theNym);
    }
}

void UserCommandProcessor::UserCmdGetNymbox(
    Nym& theNym,
    ClientContext& context,
    Message& MsgIn,
    Message& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "getNymboxResponse";  // reply to getNymbox
    msgOut.m_strNymID = MsgIn.m_strNymID;       // NymID

    const Identifier NYM_ID(MsgIn.m_strNymID), NOTARY_ID(MsgIn.m_strNotaryID);
    Identifier NYMBOX_HASH;
    bool bSavedNymbox = false;

    Ledger theLedger(NYM_ID, NYM_ID, NOTARY_ID);

    msgOut.m_bSuccess = theLedger.LoadNymbox();

    if (!msgOut.m_bSuccess)
        Log::Error("UserCommandProcessor::UserCmdGetNymbox: Failed trying to "
                   "load Nymbox "
                   "from storage.\n");
    else {
        // We do NOT call VerifyAccount in this function (because we don't need
        // to) and thus we do NOT
        // force the box receipts to be loaded here (which happens inside that
        // call.) But we DO verify
        // the IDs and the Signature, of course.
        //
        msgOut.m_bSuccess =
            (theLedger.VerifyContractID() &&
             theLedger.VerifySignature(server_->m_nymServer));

        // If we loaded old data in this file... (when whole receipts were
        // stored in boxes.)
        //
        if (msgOut.m_bSuccess &&
            theLedger.LoadedLegacyData())  // (which automatically saves the box
                                           // receipt as the old data is
                                           // loaded...)
        {
            theLedger.ReleaseSignatures();  // UPDATE: We do NOT force the
                                            // loading here, since they aren't
                                            // needed.
            theLedger.SignContract(
                server_->m_nymServer);  // Waste of resources. Instead,
                                        // we recognize that it was old
                                        // data, and so
            theLedger.SaveContract();   // we gracefully re-save in the new
                                        // format, so it won't repeatedly be
            theLedger.SaveNymbox(
                &NYMBOX_HASH);  // loaded over and over again in
                                // the large filesize.

            bSavedNymbox = true;
        }
        if (!msgOut.m_bSuccess)
            Log::Error("UserCommandProcessor::UserCmdGetNymbox: Verification "
                       "failed on "
                       "Nymbox after loading.\n");
    }

    if (true == msgOut.m_bSuccess) {
        // extract the ledger in ascii-armored form on the outgoing message
        String strPayload(theLedger);  // first grab it in plaintext string form
        msgOut.m_ascPayload.SetString(strPayload);  // now the outgoing message
                                                    // has the nymbox ledger in
                                                    // its payload in base64
                                                    // form.
    }
    // Send the user's command back to him if failure.
    else {
        String tempInMessage(
            MsgIn);  // Grab the incoming message in plaintext form
        msgOut.m_ascInReferenceTo.SetString(tempInMessage);  // Set it into the
                                                             // base64-encoded
                                                             // object on the
                                                             // outgoing message
    }

    if (bSavedNymbox) {
        context.SetLocalNymboxHash(NYMBOX_HASH);
        theNym.SaveSignedNymfile(server_->m_nymServer);  // TODO remove this
        NYMBOX_HASH.GetString(
            msgOut.m_strNymboxHash);  // Get the hash onto the message
    } else if (true == msgOut.m_bSuccess) {
        theLedger.CalculateNymboxHash(NYMBOX_HASH);
        context.SetLocalNymboxHash(NYMBOX_HASH);
        theNym.SaveSignedNymfile(server_->m_nymServer);  // TODO remove this
        NYMBOX_HASH.GetString(
            msgOut.m_strNymboxHash);  // Get the hash onto the message
    } else {
        Identifier EXISTING_NYMBOX_HASH = context.LocalNymboxHash();

        if (String(EXISTING_NYMBOX_HASH).Exists()) {
            EXISTING_NYMBOX_HASH.GetString(msgOut.m_strNymboxHash);
        }
    }

    // (2) Sign the Message
    msgOut.SignContract(static_cast<const Nym&>(server_->m_nymServer));

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    //
    // FYI, SaveContract takes m_xmlUnsigned and wraps it with the signatures
    // and ------- BEGIN  bookends
    // If you don't pass a string in, then SaveContract saves the new version to
    // its member, m_strRawFile
    msgOut.SaveContract();
}

void UserCommandProcessor::UserCmdProcessNymbox(
    Nym& theNym,
    ClientContext& context,
    Message& MsgIn,
    Message& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "processNymboxResponse";  // reply to processNymbox
    msgOut.m_strNymID = MsgIn.m_strNymID;           // NymID

    const Identifier NYM_ID(msgOut.m_strNymID),
        NOTARY_ID(server_->m_strNotaryID), NOTARY_NYM_ID(server_->m_nymServer);

    Ledger theLedger(NYM_ID, NYM_ID, NOTARY_ID);  // These are ledgers used
                                                  // as messages. The one we
                                                  // received
    // and the one we're sending back.
    std::unique_ptr<Ledger> pResponseLedger(Ledger::GenerateLedger(
        NOTARY_NYM_ID, NYM_ID, NOTARY_ID, Ledger::message, false));

    // Grab the string (containing the request ledger) out of ascii-armored
    // form.
    String strLedger(MsgIn.m_ascPayload);
    bool bTransSuccess = false;

    if (context.HaveLocalNymboxHash()) {
        context.LocalNymboxHash().GetString(msgOut.m_strNymboxHash);
    } else {
        Log::vOutput(
            1,
            "%s: We cannot obtain server side nymbox hash, will continue.\n",
            __FUNCTION__);
    }

    if (!context.HaveRemoteNymboxHash()) {
        Log::vOutput(
            1,
            "%s: We don't have a client side nymbox hash, will continue\n",
            __FUNCTION__);
    }

    if (context.HaveLocalNymboxHash() && context.HaveRemoteNymboxHash()) {
        if (!context.NymboxHashMatch()) {
            Log::vOutput(
                0,
                "%s: The server and client nymbox hashes "
                "missmatch! rejecting message.\n",
                __FUNCTION__);
            goto send_message;
        }
    }

    // theLedger contains a single transaction from the client, with an item
    // inside
    // for each inbox transaction the client wants to accept or reject.
    // Let's see if we can load it from the string that came in the message...
    //
    msgOut.m_bSuccess = theLedger.LoadContractFromString(strLedger);
    if (msgOut.m_bSuccess)  // Yes, that is an assignment operator.
    {
        // In this case we need to process the transaction items from the ledger
        // and create a corresponding transaction where each of the new items
        // contains the answer to the transaction item sent.
        // Then we send that new "response ledger" back to the user in
        // MsgOut.Payload
        // as an processNymboxResponse message.

        if (theLedger.GetTransactionCount() == 0) {
            OTTransaction* pTranResponse = OTTransaction::GenerateTransaction(
                *pResponseLedger,
                OTTransaction::error_state,
                originType::not_applicable,
                0);
            pTranResponse->SignContract(server_->m_nymServer);
            pTranResponse->SaveContract();

            // Add the response transaction to the response ledger.
            // That will go into the response message and be sent back to the
            // client.
            pResponseLedger->AddTransaction(*pTranResponse);
        }
        for (auto& it : theLedger.GetTransactionMap()) {
            OTTransaction* pTransaction = it.second;
            OT_ASSERT_MSG(
                nullptr != pTransaction,
                "nullptr transaction pointer in "
                "UserCommandProcessor::UserCmdProcessNymbox\n");

            // for each transaction in the ledger, we create a transaction
            // response and add
            // that to the response ledger.
            OTTransaction* pTranResponse = OTTransaction::GenerateTransaction(
                *pResponseLedger,
                OTTransaction::error_state,
                originType::not_applicable,
                pTransaction->GetTransactionNum());

            // Add the response transaction to the response ledger.
            // That will go into the response message and be sent back to the
            // client.
            pResponseLedger->AddTransaction(*pTranResponse);

            // Now let's make sure the response transaction has a copy of the
            // transaction
            // it is responding to.
            //
            //            OTString strResponseTo;
            //            pTransaction->SaveContract(strResponseTo);
            //            pTranResponse->m_ascInReferenceTo.SetString(strResponseTo);
            //
            // I commented out the above because we are keeping too many copies.
            // Message contains a copy of the message it's responding to.
            // Then each transaction contains a copy of the transaction
            // responding to...
            // Then each ITEM in each transaction contains a copy of each item
            // it's responding to.
            //
            // Therefore, for the "processNymbox" message, I have decided (for
            // now) to have
            // the extra copy in the items themselves, and in the overall
            // message, but not in the
            // transactions. Thus, the above is commented out.

            // It should always return something. Success, or failure, that goes
            // into pTranResponse.
            // I don't think there's need for more return value than that. The
            // user has gotten deep
            // enough that they deserve SOME sort of response.
            //
            // This function also SIGNS the transaction, so there is no need to
            // sign it after this.
            // There's also no point to change it after this, unless you plan to
            // sign it twice.
            server_->notary_.NotarizeProcessNymbox(
                theNym, context, *pTransaction, *pTranResponse, bTransSuccess);
            // at this point, the ledger now "owns" the response, and will
            // handle deleting it.
            pTranResponse = nullptr;
        }

        // DONE (Notices go to Nymbox now): should consider saving a copy of the
        // response ledger here on the server. Until the user signs off of the
        // responses, maybe the user didn't receive them. The server should be
        // able to re-send them until confirmation, then delete them. So might
        // want to consider a SAVE TO FILE here of that ledger we're sending
        // out...
    } else {
        Log::Error("ERROR loading ledger from message in "
                   "UserCommandProcessor::UserCmdProcessNymbox\n");
    }

send_message:

    // sign the ledger
    pResponseLedger->SignContract(server_->m_nymServer);
    pResponseLedger->SaveContract();
    // extract the ledger in ascii-armored form
    String strPayload(*pResponseLedger);
    // now the outgoing message has the response ledger in its payload.
    msgOut.m_ascPayload.SetString(strPayload);

    // todo: consider commenting this out since the transaction reply items
    // already include a copy
    // of the original client communication that the server is responding to. No
    // point beating a
    // dead horse.
    //
    // Send the user's command back to him as well.
    {
        String tempInMessage(MsgIn);
        msgOut.m_ascInReferenceTo.SetString(tempInMessage);
    }

    // UPDATE NYMBOX HASH IN OUTGOING MESSAGE
    //
    // We already grabbed the server's NymboxHash near the top of this function,
    // and attached it to the outgoing reply message.
    //
    // However, the above block may have CHANGED this hash! Therefore, we grab
    // it AGAIN, just in case. This way a failed message will receive the old
    // hash, and a successful message will receive the new hash.

    if (context.HaveLocalNymboxHash()) {
        context.LocalNymboxHash().GetString(msgOut.m_strNymboxHash);
    }

    // (2) Sign the Message
    msgOut.SignContract(server_->m_nymServer);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    //
    // FYI, SaveContract takes m_xmlUnsigned and wraps it with the signatures
    // and ------- BEGIN  bookends
    // If you don't pass a string in, then SaveContract saves the new version to
    // its member, m_strRawFile
    msgOut.SaveContract();

    // (You are in UserCmdProcessNymbox.)

    // REPLY NOTICE TO NYMBOX
    //
    // Now that we signed / saved the reply message...
    //
    // After specific messages, we drop a notice with a copy of the server's
    // reply
    // into the Nymbox.  This way we are GUARANTEED that the Nym will receive
    // and process
    // it. (And thus never get out of sync.)
    //
    if (msgOut.m_bSuccess) {
        const String strReplyMessage(msgOut);
        const int64_t lReqNum = MsgIn.m_strRequestNum.ToLong();

        // If it fails, it logs already.
        DropReplyNoticeToNymbox(
            NOTARY_ID,
            NYM_ID,
            strReplyMessage,
            lReqNum,  // (We don't want to update the NymboxHash on the Nym,
                      // here
                      // in processNymbox, at least, not at this current point
                      // AFTER the reply message has already been signed.)
            bTransSuccess,
            context);
        //      DropReplyNoticeToNymbox(NOTARY_ID, NYM_ID,
        // strReplyMessage, lReqNum, bTransSuccess, &theNym); // Only pass
        // theNym if you want it to contain the LATEST hash. (Some messages
        // don't.)
    }
}

void UserCommandProcessor::UserCmdProcessInbox(
    Nym& signedNymfile,
    ClientContext& context,
    Message& MsgIn,
    Message& msgOut)
{
    const auto& clientNym = context.RemoteNym();
    msgOut.m_strCommand = "processInboxResponse";
    msgOut.m_strNymID = MsgIn.m_strNymID;
    msgOut.m_strAcctID = MsgIn.m_strAcctID;

    const Identifier NYM_ID(msgOut.m_strNymID);
    const Identifier ACCOUNT_ID(MsgIn.m_strAcctID);
    const Identifier NOTARY_ID(server_->m_strNotaryID);
    const Identifier NOTARY_NYM_ID(server_->m_nymServer);

    std::unique_ptr<Ledger> requestLedger(
        new Ledger(NYM_ID, ACCOUNT_ID, NOTARY_ID));
    std::unique_ptr<Ledger> responseLedger(Ledger::GenerateLedger(
        NOTARY_NYM_ID, ACCOUNT_ID, NOTARY_ID, Ledger::message, false));

    OT_ASSERT(requestLedger);
    OT_ASSERT(responseLedger);

    if (context.HaveLocalNymboxHash()) {
        context.LocalNymboxHash().GetString(msgOut.m_strNymboxHash);
    } else {
        otWarn << OT_METHOD << __FUNCTION__
               << ": Can not obtain server side nymbox hash, will continue."
               << std::endl;
    }

    if (!context.HaveRemoteNymboxHash()) {
        otWarn << OT_METHOD << __FUNCTION__
               << ": Can not obtain client side nymbox hash, will continue."
               << std::endl;
    }

    OTTransaction* processInboxResponse{nullptr};
    OTTransaction* processInbox{nullptr};
    bool bTransSuccess{false};
    Account theAccount(NYM_ID, ACCOUNT_ID, NOTARY_ID);
    TransactionNumber transactionNumber{0};

    if (context.HaveLocalNymboxHash() && context.HaveRemoteNymboxHash()) {
        if (!context.NymboxHashMatch()) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": The server and client nymbox hashes do not match. "
                  << "Rejecting message." << std::endl;

            goto send_message;
        }
    }

    // theLedger contains a single transaction from the client, with an item
    // inside for each inbox transaction the client wants to accept or reject.
    // Let's see if we can load it from the string that came in the message...
    msgOut.m_bSuccess =
        requestLedger->LoadContractFromString(String(MsgIn.m_ascPayload));

    if (false == msgOut.m_bSuccess) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed loading processInbox ledger." << std::endl;

        goto send_message;
    }

    if (false == theAccount.LoadContract()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed loading account: " << MsgIn.m_strAcctID << std::endl;

        goto send_message;
    }

    if (theAccount.IsMarkedForDeletion()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed attempt to use an Asset account that was marked "
              << "for deletion: " << MsgIn.m_strAcctID << std::endl;

        goto send_message;
    }

    if (false == theAccount.VerifyContractID()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed verify contract ID."
              << std::endl;

        goto send_message;
    }

    // Make sure the nymID loaded up in the account as its actual owner
    // matches the nym who was passed in to this function requesting a
    // transaction on this account... otherwise any asshole could do
    // transactions on your account, no?
    if (false == theAccount.VerifyOwner(clientNym)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to verify account ownership: " << MsgIn.m_strAcctID
              << std::endl;

        goto send_message;
    }

    if (false == theAccount.VerifySignature(server_->m_nymServer)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to verify server signature on account: "
              << MsgIn.m_strAcctID << std::endl;

        goto send_message;
    }

    // In this case we need to process the transaction items from the
    // ledger and create a corresponding transaction where each of the new
    // items contains the answer to the transaction item sent. Then we send that
    // new "response ledger" back to the user in MsgOut.Payload as an
    // processInboxResponse message.

    processInbox = requestLedger->GetTransaction(OTTransaction::processInbox);

    if (nullptr == processInbox) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Expected processInbox transaction." << std::endl;

        goto send_message;
    }

    transactionNumber = processInbox->GetTransactionNum();

    // We create a transaction response and add that to the response
    // ledger...
    processInboxResponse = OTTransaction::GenerateTransaction(
        *responseLedger,
        OTTransaction::error_state,
        originType::not_applicable,
        transactionNumber);

    OT_ASSERT(nullptr != processInboxResponse);

    // Add the response transaction to the response ledger. That will go into
    // the response message and be sent back to the client.
    responseLedger->AddTransaction(*processInboxResponse);

    if (false == context.VerifyIssuedNumber(transactionNumber)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Transaction number "
              << transactionNumber << " is not issued to " << msgOut.m_strNymID
              << std::endl;

        goto send_message;
    }

    // The items' acct and server ID were already checked in VerifyContractID()
    // when they were loaded. Now this checks a little deeper, to verify
    // ownership, signatures, and transaction number on each item. That way
    // those things don't have to be checked for security over and over again in
    // the subsequent calls.
    if (false == processInbox->VerifyItems(signedNymfile)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to verify transaction items. " << std::endl;

        goto send_message;
    }

    // We don't want any transaction number being used twice. (The number, at
    // this point, is STILL issued to the user, who is still responsible for
    // that number and must continue signing for it. All this means here is that
    // the user no longer has the number on his AVAILABLE list. Removal from
    // issued list happens separately.)
    if (false == context.ConsumeAvailable(transactionNumber)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Error removing available number " << transactionNumber
              << std::endl;

        goto send_message;
    }

    otWarn << OT_METHOD << __FUNCTION__ << ": Type: Process Inbox" << std::endl;

    server_->notary_.NotarizeProcessInbox(
        signedNymfile,
        context,
        theAccount,
        *processInbox,
        *processInboxResponse,
        bTransSuccess);

    if (false == context.ConsumeIssued(transactionNumber)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Error removing issued number "
              << transactionNumber << std::endl;
    }

send_message:
    if (nullptr == processInboxResponse) {
        processInboxResponse = OTTransaction::GenerateTransaction(
            *responseLedger,
            OTTransaction::error_state,
            originType::not_applicable,
            0);

        OT_ASSERT(nullptr != processInboxResponse);

        // Add the response transaction to the response ledger.
        // That will go into the response message and be sent back to the
        // client.
        responseLedger->AddTransaction(*processInboxResponse);
    }

    // sign the outoing transaction
    OT_ASSERT(nullptr != processInboxResponse);

    processInboxResponse->ReleaseSignatures();
    processInboxResponse->SignContract(server_->m_nymServer);
    processInboxResponse->SaveContract();

    // sign the ledger
    responseLedger->SignContract(server_->m_nymServer);
    responseLedger->SaveContract();

    msgOut.m_ascPayload.SetString(String(*responseLedger));
    msgOut.m_ascInReferenceTo.SetString(String(MsgIn));

    // UPDATE NYMBOX HASH IN OUTGOING MESSAGE
    //
    // We already grabbed the server's NymboxHash near the top of this function,
    // and attached it to the outgoing reply message.
    //
    // However, the above block may have CHANGED this hash! Therefore, we grab
    // it AGAIN, just in case. This way a failed message will receive the old
    // hash, and a successful message will receive the new hash.

    if (context.HaveLocalNymboxHash()) {
        context.LocalNymboxHash().GetString(msgOut.m_strNymboxHash);
    }

    // (2) Sign the Message
    msgOut.SignContract(server_->m_nymServer);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    //
    // FYI, SaveContract takes m_xmlUnsigned and wraps it with the signatures
    // and ------- BEGIN  bookends
    // If you don't pass a string in, then SaveContract saves the new version to
    // its member, m_strRawFile
    msgOut.SaveContract();

    // (You are in UserCmdProcessInbox.)

    // REPLY NOTICE TO NYMBOX
    //
    // Now that we signed / saved the reply message...
    //
    // After specific messages, we drop a notice with a copy of the server's
    // reply into the Nymbox.  This way we are GUARANTEED that the Nym will
    // receive and process it. (And thus never get out of sync.)
    if (msgOut.m_bSuccess) {
        DropReplyNoticeToNymbox(
            NOTARY_ID,
            NYM_ID,
            String(msgOut),
            MsgIn.m_strRequestNum.ToLong(),
            bTransSuccess,
            context);
    }
}

/// There will be more code here to handle all that. In the meantime, I just
/// send
/// a test response back to make sure the communication works.
///
/// An existing user is sending a list of transactions to be notarized.
void UserCommandProcessor::UserCmdNotarizeTransaction(
    Nym& theNym,
    ClientContext& context,
    Message& MsgIn,
    Message& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand =
        "notarizeTransactionResponse";       // reply to notarizeTransaction
    msgOut.m_strNymID = MsgIn.m_strNymID;    // NymID
    msgOut.m_strAcctID = MsgIn.m_strAcctID;  // The Account ID in question

    const Identifier NYM_ID(MsgIn.m_strNymID), ACCOUNT_ID(MsgIn.m_strAcctID),
        NOTARY_ID(server_->m_strNotaryID), NOTARY_NYM_ID(server_->m_nymServer);

    Ledger theLedger(NYM_ID, ACCOUNT_ID, NOTARY_ID);  // These are ledgers
                                                      // used as messages. The
                                                      // one we received and
                                                      // the one
    // that we're sending back in response.
    std::unique_ptr<Ledger> pResponseLedger(Ledger::GenerateLedger(
        NOTARY_NYM_ID, ACCOUNT_ID, NOTARY_ID, Ledger::message, false));

    bool bTransSuccess = false;  // for the Nymbox notice.
    bool bCancelled = false;     // for "failed" transactions that were actually
                                 // successful cancellations.

    int64_t lTransactionNumber = 0, lResponseNumber = 0;
    // Since the one going back (above) is a new ledger, we have to call
    // GenerateLedger.
    // Whereas the ledger we received from the server was generated there, so we
    // don't
    // have to generate it again. We just load it.

    String strLedger(MsgIn.m_ascPayload);

    if (context.HaveLocalNymboxHash()) {
        context.LocalNymboxHash().GetString(msgOut.m_strNymboxHash);
    } else {
        Log::vOutput(
            1,
            "%s: We cannot obtain server side nymbox hash, will continue.\n",
            __FUNCTION__);
    }

    if (!context.HaveRemoteNymboxHash()) {
        Log::vOutput(
            1,
            "%s: We don't have a client side nymbox hash, will continue\n",
            __FUNCTION__);
    }

    if (context.HaveLocalNymboxHash() && context.HaveRemoteNymboxHash()) {
        if (!context.NymboxHashMatch()) {
            Log::vOutput(
                0,
                "%s: The server and client nymbox hashes "
                "missmatch! rejecting message.\n",
                __FUNCTION__);
            goto send_message;
        }
    }

    // as long as the request ledger loads from the message into memory, success
    // is true
    // from there, the success or failure of the transactions within will be
    // carried in
    // their own status variables and those of the items inside those
    // transactions.
    msgOut.m_bSuccess = theLedger.LoadLedgerFromString(strLedger);

    if (msgOut.m_bSuccess) {
        // In this case we need to process the ledger items
        // and create a corresponding ledger where each of the new items
        // contains the answer to the ledger item sent.
        // Then we send that new "response ledger" back to the user in
        // MsgOut.Payload.
        // That is all done here. Until I write that, in the meantime,
        // let's just fprintf it out and see what it looks like.
        //        OTLog::Error("Loaded ledger out of message payload:\n%s\n",
        // strLedger.Get());
        //        OTLog::Error("Loaded ledger out of message payload.\n");

        // Loop through ledger transactions, and do a "NotarizeTransaction" call
        // for each one.
        // Inside that function it will do the various necessary authentication
        // and processing, not this one.
        // NOTE: In practice there is only ONE transaction, but in potential
        // there are many.
        // But so far, the code only actually has one, ever being sent.
        // Otherwise the messages
        // get too big IMO.
        //
        int32_t nCounter = 0;
        for (auto& it : theLedger.GetTransactionMap()) {
            OTTransaction* pTransaction = it.second;
            OT_ASSERT(nullptr != pTransaction);
            ++nCounter;

            if (1 != nCounter)
                Log::vError("WARNING: multiple transactions in a single "
                            "message ledger.\n");

            // for each transaction in the ledger, we create a transaction
            // response and add
            // that to the response ledger.

            // I don't call transactor_.issueNextTransactionNumber here because
            // I'm not
            // creating a new transaction
            // in someone's inbox or outbox. Instead, I'm making a transaction
            // response to a transaction
            // request, with a MATCHING transaction number (so don't need to
            // issue a new one) to be sent
            // back to the client in a message.
            //
            // On this new "response transaction", I set the ACCT ID, the
            // notaryID, and Transaction Number.
            OTTransaction* pTranResponse = OTTransaction::GenerateTransaction(
                *pResponseLedger,
                OTTransaction::error_state,
                originType::not_applicable,
                pTransaction->GetTransactionNum());
            // Add the response transaction to the response ledger.
            // That will go into the response message and be sent back to the
            // client.
            pResponseLedger->AddTransaction(*pTranResponse);

            // Now let's make sure the response transaction has a copy of the
            // transaction
            // it is responding to.
            //                OTString strResponseTo;
            //                pTransaction->SaveContract(strResponseTo);
            //                pTranResponse->m_ascInReferenceTo.SetString(strResponseTo);
            // I commented out the above because we are keeping too many copies.
            // Message contains a copy of the message it's responding to.
            // Then each transaction contains a copy of the transaction
            // responding to...
            // Then each ITEM in each transaction contains a copy of each item
            // it's responding to.
            //
            // Therefore, for the "notarizeTransaction" message, I have decided
            // (for now) to have
            // the extra copy in the items themselves, and in the overall
            // message, but not in the
            // transactions. Thus, the above is commented out.

            // It should always return something. Success, or failure, that goes
            // into pTranResponse.
            // I don't think there's need for more return value than that. The
            // user has gotten deep
            // enough that they deserve SOME sort of response.
            //
            // This function also SIGNS the transaction, so there is no need to
            // sign it after this.
            // There's also no point to change it after this, unless you plan to
            // sign it twice.
            //
            server_->notary_.NotarizeTransaction(
                theNym, context, *pTransaction, *pTranResponse, bTransSuccess);

            if (pTranResponse->IsCancelled()) bCancelled = true;

            lTransactionNumber = pTransaction->GetTransactionNum();
            lResponseNumber = pTranResponse->GetTransactionNum();

            OT_ASSERT_MSG(
                lTransactionNumber == lResponseNumber,
                "Transaction number and response number should "
                "always be the same. (But this time, they weren't.)");

            pTranResponse =
                nullptr;  // at this point, the ledger now "owns" the
                          // response, and will handle deleting it.
        }

        // TODO: should consider saving a copy of the response ledger here on
        // the server.
        // Until the user signs off of the responses, maybe the user didn't
        // receive them.
        // The server should be able to re-send them until confirmation, then
        // delete them.
        // So might want to consider a SAVE TO FILE here of that ledger we're
        // sending out...

        // sign the ledger
        pResponseLedger->SignContract(server_->m_nymServer);
        pResponseLedger->SaveContract();

        // extract the ledger in ascii-armored form
        String strPayload(*pResponseLedger);

        msgOut.m_ascPayload.SetString(strPayload);  // now the outgoing message
        // has the response ledger in
        // its payload.
    } else {
        Log::Error("ERROR loading ledger from message in "
                   "UserCommandProcessor::UserCmdNotarizeTransaction\n");
    }

send_message:
    // todo: consider commenting this out since the transaction reply items
    // already include a copy
    // of the original client communication that the server is responding to. No
    // point beating a
    // dead horse.
    //
    // Send the user's command back to him as well.
    {
        String tempInMessage(MsgIn);
        msgOut.m_ascInReferenceTo.SetString(tempInMessage);
    }

    // UPDATE NYMBOX HASH IN OUTGOING MESSAGE
    //
    // We already grabbed the server's NymboxHash near the top of this function,
    // and attached it to the outgoing reply message.
    //
    // However, the above block may have CHANGED this hash! Therefore, we grab
    // it AGAIN, just in case. This way a failed message will receive the old
    // hash, and a successful message will receive the new hash.

    if (context.HaveLocalNymboxHash()) {
        context.LocalNymboxHash().GetString(msgOut.m_strNymboxHash);
    }

    // (2) Sign the Message
    msgOut.SignContract(server_->m_nymServer);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    //
    // FYI, SaveContract takes m_xmlUnsigned and wraps it with the signatures
    // and ------- BEGIN  bookends
    // If you don't pass a string in, then SaveContract saves the new version to
    // its member, m_strRawFile
    msgOut.SaveContract();

    // (You are in UserCmdNotarizeTransaction.)

    // REPLY NOTICE TO NYMBOX
    //
    // Now that we signed / saved the reply message...
    //
    // After EVERY / ANY transaction, we drop a notice with a copy of the
    // server's reply
    // into the Nymbox.  This way we are GUARANTEED that the Nym will receive
    // and process
    // it. (And thus never get out of sync.)
    //
    if (msgOut.m_bSuccess) {
        const String strReplyMessage(msgOut);
        const int64_t lReqNum = MsgIn.m_strRequestNum.ToLong();

        // If it fails, it logs already.
        //      DropReplyNoticeToNymbox(NOTARY_ID, NYM_ID,
        // strReplyMessage, lReqNum, bTransSuccess, &theNym); // We don't want
        // to update the Nym in this case (I don't think.)
        DropReplyNoticeToNymbox(
            NOTARY_ID,
            NYM_ID,
            strReplyMessage,
            lReqNum,
            bTransSuccess,
            context);  // trans success
    }
    if (bCancelled) {
        Log::vOutput(
            0,
            "Success: canceling transaction %" PRId64 " for nym: %s \n",
            lTransactionNumber,
            msgOut.m_strNymID.Get());
    } else if (bTransSuccess) {
        Log::vOutput(
            0,
            "Success: processing transaction %" PRId64 " for nym: %s \n",
            lTransactionNumber,
            msgOut.m_strNymID.Get());
    } else {
        Log::vOutput(
            0,
            "Failure: processing transaction %" PRId64 " for nym: %s \n",
            lTransactionNumber,
            msgOut.m_strNymID.Get());
    }
}

// msg, the request msg from payer, which is attached WHOLE to the Nymbox
// receipt. contains payment already.
// or pass pPayment instead: we will create our own msg here (with payment
// inside) to be attached to the receipt.
bool UserCommandProcessor::SendMessageToNym(
    const Identifier& NOTARY_ID,
    const Identifier& SENDER_NYM_ID,
    const Identifier& RECIPIENT_NYM_ID,
    Message* pMsg,              // the request msg from payer, which is attached
                                // WHOLE to the Nymbox receipt. contains message
                                // already.
    const String* pstrMessage)  // or pass this instead: we will
                                // create our own msg here (with
                                // message inside) to be attached to
                                // the receipt.
{
    return server_->DropMessageToNymbox(
        NOTARY_ID,
        SENDER_NYM_ID,
        RECIPIENT_NYM_ID,
        OTTransaction::message,
        pMsg,
        pstrMessage);  //, szCommand=nullptr
}

// After EVERY / ANY transaction, plus certain messages, we drop a copy of the
// server's reply into
// the Nymbox.  This way we are GUARANTEED that the Nym will receive and process
// it. (And thus
// never get out of sync.)  This is the function used for doing that.
//
void UserCommandProcessor::DropReplyNoticeToNymbox(
    const Identifier& NOTARY_ID,
    const Identifier& NYM_ID,
    const String& strMessage,
    const int64_t& lRequestNum,
    const bool bReplyTransSuccess,
    ClientContext& context,
    Nym* pActualNym)
{
    Ledger theNymbox(NYM_ID, NYM_ID, NOTARY_ID);

    bool bSuccessLoadingNymbox = theNymbox.LoadNymbox();

    if (true == bSuccessLoadingNymbox)
        bSuccessLoadingNymbox =
            (theNymbox.VerifyContractID() &&
             theNymbox.VerifySignature(server_->m_nymServer));

    if (!bSuccessLoadingNymbox) {
        const String strNymID(NYM_ID);
        Log::vOutput(
            0,
            "UserCommandProcessor::DropReplyNoticeToNymbox: Failed loading "
            "or verifying Nymbox for user: %s\n",
            strNymID.Get());
    } else {
        int64_t lReplyNoticeTransNum = 0;
        bool bGotNextTransNum = server_->transactor_.issueNextTransactionNumber(
            lReplyNoticeTransNum);

        if (!bGotNextTransNum) {
            lReplyNoticeTransNum = 0;
            Log::Error(
                "UserCommandProcessor::DropReplyNoticeToNymbox: Error getting "
                "next transaction number for an "
                "OTTransaction::replyNotice.\n");
        } else {  // Drop in the Nymbox
            OTTransaction* pReplyNotice = OTTransaction::GenerateTransaction(
                theNymbox,
                OTTransaction::replyNotice,
                originType::not_applicable,
                lReplyNoticeTransNum);
            OT_ASSERT(nullptr != pReplyNotice);
            Item* pReplyNoticeItem = Item::CreateItemFromTransaction(
                *pReplyNotice, Item::replyNotice);
            OT_ASSERT(nullptr != pReplyNoticeItem);
            pReplyNoticeItem->SetStatus(
                Item::acknowledgement);  // Nymbox notice is always a success.
                                         // It's just a notice. (The message
                                         // inside it will have success/failure
                                         // also, and any transaction inside
                                         // that will also.)
            pReplyNoticeItem->SetAttachment(
                strMessage);  // Purpose of this notice is to carry a copy of
                              // server's reply message (to certain requests,
                              // including all transactions.)
            pReplyNoticeItem->SignContract(server_->m_nymServer);
            pReplyNoticeItem->SaveContract();
            pReplyNotice->AddItem(*pReplyNoticeItem);  // the Transaction's
            // destructor will cleanup
            // the item. It "owns" it
            // now.
            // So the client-side can quickly/easily match up the replyNotices
            // in
            // the Nymbox with the request numbers of the messages that were
            // sent.
            // I think this is actually WHY the server message low-level
            // functions
            // now RETURN the request number.
            // FYI: replyNotices will ONLY be in my Nymbox if the MESSAGE was
            // successful.
            // (Meaning, the balance agreement might have failed, and the
            // transaction
            // might have failed, but the MESSAGE ITSELF must be a success, in
            // order for
            // the replyNotice to appear in the Nymbox.)
            //
            pReplyNotice->SetRequestNum(lRequestNum);
            pReplyNotice->SetReplyTransSuccess(bReplyTransSuccess);
            pReplyNotice->SignContract(server_->m_nymServer);
            pReplyNotice->SaveContract();
            theNymbox.AddTransaction(*pReplyNotice);  // Add the replyNotice to
                                                      // the nymbox. It takes
                                                      // ownership.
            theNymbox.ReleaseSignatures();
            theNymbox.SignContract(server_->m_nymServer);
            theNymbox.SaveContract();

            Identifier NYMBOX_HASH;
            theNymbox.SaveNymbox(&NYMBOX_HASH);

            pReplyNotice->SaveBoxReceipt(theNymbox);

            if ((nullptr != pActualNym) && pActualNym->CompareID(NYM_ID)) {
                context.SetLocalNymboxHash(NYMBOX_HASH);
                // TODO remove this
                pActualNym->SaveSignedNymfile(server_->m_nymServer);
            } else if (nullptr != pActualNym) {
                Log::Error(
                    "UserCommandProcessor::DropReplyNoticeToNymbox: ERROR: "
                    "pActualNym was not nullptr, but it didn't match "
                    "NYM_ID.\n");
            }
        }
    }
}

void UserCommandProcessor::UserCmdRequestAdmin(
    Nym&,
    Message& MsgIn,
    Message& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "requestAdminResponse";
    msgOut.m_strNymID = MsgIn.m_strNymID;
    msgOut.m_bSuccess = false;

    const String& requestingNym = MsgIn.m_strNymID;
    const std::string candidate = requestingNym.Get();
    const std::string providedPassword = MsgIn.m_strAcctID.Get();

    std::string overrideNym, password;
    bool notUsed = false;
    OT::App().Config().CheckSet_str(
        "permissions", "override_nym_id", "", overrideNym, notUsed);
    OT::App().Config().CheckSet_str(
        "permissions", "admin_password", "", password, notUsed);
    const bool noAdminYet = overrideNym.empty();
    const bool passwordSet = !password.empty();
    const bool readyForAdmin = (noAdminYet && passwordSet);
    const bool correctPassword = (providedPassword == password);
    const bool returningAdmin = (candidate == overrideNym);
    const bool duplicateRequest = (!noAdminYet && returningAdmin);

    if (correctPassword) {
        if (readyForAdmin) {
            msgOut.m_bSuccess = OT::App().Config().Set_str(
                "permissions", "override_nym_id", requestingNym, notUsed);

            if (msgOut.m_bSuccess) {
                otErr << __FUNCTION__ << ": override nym set." << std::endl;
                OT::App().Config().Save();
            }
        } else {
            if (duplicateRequest) {
                msgOut.m_bSuccess = true;
            } else {
                otErr << __FUNCTION__ << ": Admin password empty or admin nym "
                      << "already set." << std::endl;
            }
        }
    } else {
        otErr << __FUNCTION__ << ": Incorrect password" << std::endl;
    }

    msgOut.m_ascInReferenceTo.SetString(String(MsgIn));
    msgOut.SignContract(server_->m_nymServer);
    msgOut.SaveContract();
}

void UserCommandProcessor::UserCmdAddClaim(
    Nym&,
    Message& MsgIn,
    Message& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "addClaimResponse";
    msgOut.m_strNymID = MsgIn.m_strNymID;
    msgOut.m_bSuccess = false;

    const String requestingNym = MsgIn.m_strNymID;
    const std::uint32_t section = MsgIn.m_strNymID2.ToUint();
    const std::uint32_t type = MsgIn.m_strInstrumentDefinitionID.ToUint();
    const std::string value = MsgIn.m_strAcctID.Get();
    const bool primary = MsgIn.m_bBool;
    std::set<std::uint32_t> attributes;

    if (primary) {
        attributes.insert(proto::CITEMATTR_PRIMARY);
    }

    attributes.insert(proto::CITEMATTR_ACTIVE);

    Claim claim{"", section, type, value, 0, 0, attributes};

    String overrideNym;
    bool keyExists = false;
    OT::App().Config().Check_str(
        "permissions", "override_nym_id", overrideNym, keyExists);
    const bool haveAdmin = keyExists && overrideNym.Exists();
    const bool isAdmin = haveAdmin && (overrideNym == requestingNym);

    if (isAdmin) {
        msgOut.m_bSuccess = OT::App().Identity().AddClaim(
            const_cast<Nym&>(server_->GetServerNym()), claim);
    }

    msgOut.m_ascInReferenceTo.SetString(String(MsgIn));
    msgOut.SignContract(server_->m_nymServer);
    msgOut.SaveContract();
}
}  // namespace opentxs
