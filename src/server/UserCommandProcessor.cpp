/************************************************************
 *
 *  UserCommandProcessor.cpp
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

#include <opentxs/server/UserCommandProcessor.hpp>
#include <opentxs/server/OTServer.hpp>
#include <opentxs/server/ClientConnection.hpp>
#include <opentxs/server/Macros.hpp>
#include <opentxs/server/ServerSettings.hpp>
#include <opentxs/basket/BasketContract.hpp>
#include <opentxs/basket/Basket.hpp>
#include <opentxs/core/script/OTParty.hpp>
#include <opentxs/core/script/OTSmartContract.hpp>
#include <opentxs/core/OTAssetContract.hpp>
#include <opentxs/core/OTMessage.hpp>
#include <opentxs/core/OTPseudonym.hpp>
#include <opentxs/core/OTLog.hpp>
#include <opentxs/core/OTString.hpp>
#include <opentxs/core/crypto/OTAsymmetricKey.hpp>
#include <opentxs/core/crypto/OTASCIIArmor.hpp>
#include <opentxs/core/util/OTFolders.hpp>
#include <opentxs/core/OTStorage.hpp>
#include <opentxs/core/OTLedger.hpp>
#include <opentxs/cash/Mint.hpp>
#include <opentxs/core/trade/OTMarket.hpp>

namespace opentxs
{

UserCommandProcessor::UserCommandProcessor(OTServer* server)
    : server_(server)
{
}

// this function will create the Nym if it's not passed in. We pass it in so the
// caller has the option to query things about the Nym (like if it actually
// exists.)
bool UserCommandProcessor::ProcessUserCommand(OTMessage& theMessage,
                                              OTMessage& msgOut,
                                              ClientConnection* pConnection,
                                              OTPseudonym* pNym)
{
    msgOut.m_strRequestNum.Set(theMessage.m_strRequestNum);

    if (ServerSettings::__admin_server_locked &&
        ((ServerSettings::GetOverrideNymID().size() <=
          0) || // AND (there's no Override Nym ID listed --OR-- the Override
                // Nym ID doesn't
         (0 !=
          ServerSettings::GetOverrideNymID().compare(
              (theMessage.m_strNymID.Get()))))) // match the Nym's ID who
                                                // sent this message)
    {
        OTLog::vOutput(
            0, "UserCommandProcessor::ProcessUserCommand: Nym %s: failed "
               "attempt to message the server, while server is in "
               "**LOCK DOWN MODE**.\n",
            theMessage.m_strNymID.Get());
        return false;
    }

    // Validate the server ID, to keep users from intercepting a valid requst
    // and sending it to the wrong server.
    if (!(server_->m_strServerID == theMessage.m_strServerID)) {
        OTLog::Error("UserCommandProcessor::ProcessUserCommand: Invalid server "
                     "ID sent in "
                     "command request.\n");
        return false;
    }
    else {
        OTLog::Output(4, "Received valid Server ID with command request.\n");
    }

    // NYM WAS PASSED IN
    //
    // If one wasn't passed in, we'll use the one constructed here.
    OTPseudonym theNym(theMessage.m_strNymID);
    if (nullptr == pNym)
        pNym = &theNym;
    else if (!pNym->CompareID(theNym)) {
        OTString strTempNymID;
        pNym->GetIdentifier(strTempNymID);
        OTLog::vError(
            "UserCommandProcessor::ProcessUserCommand: NymID on the optional "
            "Nym passed in "
            "(%s) "
            "does NOT match the NymID on theMessage (%s). (Returning false.)\n",
            strTempNymID.Get(), theMessage.m_strNymID.Get());
        return false;
    }

    // NYM IS ACTUALLY SERVER
    //
    // For special cases where the Nym sending the transaction has the same
    // public key as
    // the server itself. (IE it IS the server Nym, then we'd want to use the
    // already-loaded
    // server nym object instead of loading a fresh one, so the two don't
    // overwrite each other.)
    //
    bool bNymIsServerNym =
        server_->m_strServerUserID.Compare(theMessage.m_strNymID);

    if (bNymIsServerNym) pNym = &server_->m_nymServer;

    OTString strMsgNymID;
    pNym->GetIdentifier(strMsgNymID);

    if (theMessage.m_strCommand.Compare("checkServerID")) {
        OTLog::vOutput(0,
                       "\n==> Received a checkServerID message. Nym: %s ...\n",
                       strMsgNymID.Get());
        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_check_server_id);
        std::unique_ptr<OTAsymmetricKey> pNymAuthentKey(
            OTAsymmetricKey::KeyFactory());
        std::unique_ptr<OTAsymmetricKey> pNymEncryptKey(
            OTAsymmetricKey::KeyFactory());
        OT_ASSERT(nullptr != pNymAuthentKey);
        OT_ASSERT(nullptr != pNymEncryptKey);
        OTAsymmetricKey& nymAuthentKey = *pNymAuthentKey;
        OTAsymmetricKey& nymEncryptionKey = *pNymEncryptKey;
        bool bIfNymPublicKey =
            (nymAuthentKey.SetPublicKey(theMessage.m_strNymPublicKey, true) &&
             nymEncryptionKey.SetPublicKey(theMessage.m_strNymID2, true));

        if (!bIfNymPublicKey) {
            OTLog::Error("Failure reading Nym's signing and/or encryption keys "
                         "from message.\n");
            return false;
        }
        // Not all contracts are signed with the authentication key, but
        // messages are.
        if (!theMessage.VerifyWithKey(nymAuthentKey)) {
            OTLog::Output(0, "checkServerID: Signature verification failed!\n");
            return false;
        }
        OTLog::Output(3,
                      "Signature verified! The message WAS signed by "
                      "the Private Authentication Key inside the message.\n");

        // This is only for verified Nyms, (and we're verified in here!)
        // We do this so that
        // we have the option later to encrypt the replies back to the
        // client...(using the
        // client's public key that we set here.)
        if (nullptr != pConnection)
            pConnection->SetPublicKey(
                theMessage.m_strNymID2); // theMessage.m_strNymID2
                                         // contains the public
                                         // encryption key for sending
                                         // an encrypted reply.

        UserCmdCheckServerID(*pNym, theMessage, msgOut);
        return true;
    }
    else if (theMessage.m_strCommand.Compare("createUserAccount")) {
        OTLog::vOutput(
            0, "\n==> Received a createUserAccount message. Nym: %s ...\n",
            strMsgNymID.Get());
        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_create_user_acct);
        if (bNymIsServerNym) {
            OTLog::Output(0,
                          "**** Sorry, the server Nym is forbidden from using "
                          "the createUserAccount message as a client. "
                          "PLEASE REMOVE THAT NYM FROM YOUR WALLET!! Create a "
                          "fresh Nym to use. ***\n");
            return false;
        }
        OTASCIIArmor& ascArmor = theMessage.m_ascPayload;
        OTASCIIArmor& ascArmor2 = theMessage.m_ascPayload2;
        // First try to get Credentials, if there are any.
        const bool bHasCredentials = (ascArmor.Exists() && ascArmor2.Exists());
        if (bHasCredentials) {
            OTString strCredentialList(ascArmor);

            if (strCredentialList.Exists()) {
                std::unique_ptr<OTDB::Storable> pStorable(OTDB::DecodeObject(
                    OTDB::STORED_OBJ_STRING_MAP, ascArmor2.Get()));
                OTDB::StringMap* pMap =
                    dynamic_cast<OTDB::StringMap*>(pStorable.get());
                if (nullptr == pMap)
                    OTLog::vOutput(0, "%s: @createUserAccount: Failed decoding "
                                      "StringMap object.\n",
                                   __FUNCTION__);
                else {
                    auto& theMap = pMap->the_map;
                    // NOTE: This action may very well be a malicious attacker
                    // saving a false
                    // credential list and a false set of credentials under a
                    // certain Nym ID!
                    // However, a Nym is always verified after loading, before
                    // being used for
                    // anything. (AND MUST BE.) And that includes before being
                    // saved to disk.
                    //
                    // DILEMMA at this point was, I don't want to save the
                    // credentials into the
                    // actual folder locations BEFORE they have been verified...
                    // SO I had to add "LoadFromString" functions to
                    // OTCredential (which I have done.)
                    // So now I should be able to continue here, load the
                    // credentials up from string,
                    // verify them, and if verified, THEN save them to disk...
                    // OTPseudonym::LoadFromString now allows you to load
                    // credentials from the map passed
                    // in (from the message) versus just reading them from local
                    // storage.
                    //
                    if (false ==
                        pNym->LoadFromString(strCredentialList, &theMap)) {
                        OTLog::vError(
                            "%s: @createUserAccount: Failure loading nym %s "
                            "from credential string.\n",
                            __FUNCTION__, theMessage.m_strNymID.Get());
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
                    else if (!pNym->VerifyPseudonym()) {
                        OTLog::vError(
                            "%s: @createUserAccount: Loaded nym %s "
                            "from credentials, but then it failed verifying.\n",
                            __FUNCTION__, theMessage.m_strNymID.Get());
                    }
                    else // Okay, we loaded the Nym up from the credentials in
                           // the message, AND
                    {      // verified the Nym (including the credentials.)
                        // So let's save it to local storage...
                        //

                        OTLog::Output(3, "Pseudonym verified!\n");
                        // Okay, now that the Nym is verified, let's verify the
                        // message itself...
                        //
                        if (false ==
                            theMessage.VerifySignature(*pNym)) // FYI, OTMessage
                                                               // overrides
                        // VerifySignature with
                        // VerifySigAuthent.
                        { // (Because we use authentication keys, not signing
                            // keys, for messages.)
                            OTLog::Output(0, "@createUserAccount: "
                                             "Authentication signature -- "
                                             "verification failed!\n");
                            return false;
                        }
                        OTLog::Output(
                            3, "Signature verified! The message WAS signed by "
                               "the Nym\'s private authentication key.\n");
                        // SAVE the credentials to local storage, now that
                        // things are verified.
                        //
                        std::string str_nym_id = theMessage.m_strNymID.Get();
                        OTString strFilename;
                        strFilename.Format("%s.cred", str_nym_id.c_str());

                        bool bStoredList = false;
                        OTString strOutput;

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
                            OTLog::vError("%s: @createUserAccount: Failed "
                                          "trying to armor or store: %s\n",
                                          __FUNCTION__, strFilename.Get());
                        else // IF the list saved, then we save the
                             // credentials themselves...
                        {
                            OTLog::vOutput(1, "@createUserAccount: Success "
                                              "saving public credential "
                                              "list for Nym: %s\n",
                                           str_nym_id.c_str());
                            for (auto& it : theMap) {
                                std::string str_cred_id = it.first;
                                OTString strCredential(it.second);
                                bool bStoredCredential = false;
                                strOutput.Release();
                                OTASCIIArmor ascLoopArmor(strCredential);
                                if (ascLoopArmor.Exists() &&
                                    ascLoopArmor.WriteArmoredString(
                                        strOutput,
                                        "CREDENTIAL") && // bEscaped=false
                                                         // by default.
                                    strOutput.Exists())
                                    bStoredCredential = OTDB::StorePlainString(
                                        strOutput.Get(),
                                        OTFolders::Pubcred().Get(), str_nym_id,
                                        str_cred_id);
                                if (!bStoredCredential)
                                    OTLog::vError(
                                        "%s: @createUserAccount: Failed "
                                        "trying to store credential %s for "
                                        "nym %s.\n",
                                        __FUNCTION__, str_cred_id.c_str(),
                                        str_nym_id.c_str());
                                else
                                    OTLog::vOutput(0, "@createUserAccount: "
                                                      "Success saving "
                                                      "public credential "
                                                      "ID: %s\n",
                                                   str_cred_id.c_str());
                            }
                        }
                        // Make sure we are encrypting the message we send
                        // back, if possible.
                        OTString strPublicEncrKey, strPublicSignKey;
                        OTAsymmetricKey& thePublicEncrKey =
                            const_cast<OTAsymmetricKey&>(
                                pNym->GetPublicEncrKey());
                        OTAsymmetricKey& thePublicSignKey =
                            const_cast<OTAsymmetricKey&>(
                                pNym->GetPublicSignKey());

                        thePublicEncrKey.GetPublicKey(strPublicEncrKey, false);
                        thePublicSignKey.GetPublicKey(strPublicSignKey, false);

                        // We also (for now) set the Nym's keypair (which is
                        // being phased out entirely,
                        // and being replaced with credentials.)
                        //
                        if (strPublicSignKey.Exists())
                            pNym->SetPublicKey(strPublicSignKey, false);

                        // This is only for verified Nyms, (and we're
                        // verified in here!) We do this so that
                        // we have the option later to encrypt the replies
                        // back to the client...(using the
                        // client's public key that we set here.)
                        if (strPublicEncrKey.Exists() &&
                            (nullptr != pConnection))
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
                        OTLog::Output(
                            0, "Verifying account doesn't already exist... "
                               "(IGNORE ANY ERRORS, IMMEDIATELY BELOW, "
                               "ABOUT FAILURE OPENING FILES)\n");

                        // Prepare to send success or failure back to user.
                        // (1) set up member variables

                        // reply to createUserAccount
                        msgOut.m_strCommand = "@createUserAccount";
                        msgOut.m_strNymID = theMessage.m_strNymID; // UserID
                        msgOut.m_strServerID =
                            server_->m_strServerID; // ServerID, a hash of
                                                    // the server
                                                    // contract.
                        msgOut.m_bSuccess = false;

                        // We send the user's message back to him,
                        // ascii-armored,
                        // as part of our response.
                        OTString tempInMessage;
                        theMessage.SaveContractRaw(tempInMessage);
                        msgOut.m_ascInReferenceTo.SetString(tempInMessage);

                        bool bLoadedSignedNymfile =
                            pNym->LoadSignedNymfile(server_->m_nymServer);

                        // He ALREADY exists. We'll set success to true, and
                        // send him a copy of his own nymfile.
                        // (Signature is verified already anyway, by this
                        // point.)
                        //
                        if (bLoadedSignedNymfile &&
                            (false == pNym->IsMarkedForDeletion())) {
                            OTLog::vOutput(
                                0, "(Allowed in order to prevent sync issues) "
                                   "==> User is registering nym that already "
                                   "exists: %s\n",
                                theMessage.m_strNymID.Get());

                            OTString strNymContents;
                            pNym->SavePseudonym(strNymContents);
                            OTIdentifier theNewNymID,
                                SERVER_ID(server_->m_strServerID);
                            pNym->GetIdentifier(theNewNymID);
                            OTLedger theNymbox(theNewNymID, theNewNymID,
                                               SERVER_ID);
                            bool bSuccessLoadingNymbox = theNymbox.LoadNymbox();

                            if (true == bSuccessLoadingNymbox)
                                bSuccessLoadingNymbox =
                                    (theNymbox.VerifyContractID() &&
                                     theNymbox.VerifyAccount(
                                         server_->m_nymServer));
                            // (No need here to load all the Box Receipts)
                            else {
                                bSuccessLoadingNymbox =
                                    theNymbox.GenerateLedger(
                                        theNewNymID, SERVER_ID,
                                        OTLedger::nymbox, true);

                                if (bSuccessLoadingNymbox) {
                                    bSuccessLoadingNymbox =
                                        theNymbox.SignContract(
                                            server_->m_nymServer);

                                    if (bSuccessLoadingNymbox) {
                                        bSuccessLoadingNymbox =
                                            theNymbox.SaveContract();

                                        if (bSuccessLoadingNymbox)
                                            bSuccessLoadingNymbox =
                                                theNymbox.SaveNymbox();
                                    }
                                }
                            }
                            // by this point, the nymbox DEFINITELY exists
                            // -- or not. (generation might have failed, or
                            // verification.)
                            //
                            if (!bSuccessLoadingNymbox) {
                                OTLog::vError("Error during user account "
                                              "re-registration. Failed "
                                              "verifying or generating "
                                              "nymbox for user: %s\n",
                                              theMessage.m_strNymID.Get());
                            }
                            msgOut.m_ascPayload.SetString(strNymContents);
                            msgOut.m_bSuccess = bSuccessLoadingNymbox;
                            msgOut.SignContract(server_->m_nymServer);
                            msgOut.SaveContract();
                            return true;
                        }
                        if (pNym->IsMarkedForDeletion())
                            pNym->MarkAsUndeleted();

                        // Good -- this means the account doesn't
                        // already exist.
                        // Let's create it.

                        msgOut.m_bSuccess = theMessage.SaveContract(
                            OTFolders::UserAcct().Get(),
                            theMessage.m_strNymID.Get());

                        // First we save the createUserAccount message
                        // in the accounts folder...
                        if (!msgOut.m_bSuccess) {
                            OTLog::Error("Error saving new user "
                                         "account verification "
                                         "file.\n");
                            msgOut.SignContract(server_->m_nymServer);
                            msgOut.SaveContract();
                            return true;
                        }

                        OTLog::Output(0, "Success saving new user "
                                         "account verification "
                                         "file.\n");

                        OTIdentifier theNewNymID,
                            SERVER_ID(server_->m_strServerID);
                        pNym->GetIdentifier(theNewNymID);
                        OTLedger theNymbox(theNewNymID, theNewNymID, SERVER_ID);
                        bool bSuccessLoadingNymbox = theNymbox.LoadNymbox();

                        if (true == bSuccessLoadingNymbox) // that's
                            // strange, this
                            // user didn't
                            // exist... but
                            // maybe I allow
                            // people to drop
                            // notes anyway,
                            // so then the
                            // nymbox might
                            // already exist,
                            // with usage
                            // tokens and
                            // messages
                            // inside....
                            bSuccessLoadingNymbox =
                                (theNymbox.VerifyContractID() &&
                                 theNymbox.VerifyAccount(server_->m_nymServer));
                        // (No need here to load all the Box
                        // Receipts)
                        else {
                            bSuccessLoadingNymbox =
                                theNymbox.GenerateLedger(theNewNymID, SERVER_ID,
                                                         OTLedger::nymbox,
                                                         true) &&
                                theNymbox.SignContract(server_->m_nymServer) &&
                                theNymbox.SaveContract() &&
                                theNymbox.SaveNymbox();
                        }
                        // by this point, the nymbox DEFINITELY
                        // exists -- or not. (generation might have
                        // failed, or verification.)

                        if (!bSuccessLoadingNymbox) {
                            OTLog::vError("Error during user account "
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
                            pNym->SaveSignedNymfile(server_->m_nymServer)) {
                            OTLog::vOutput(0, "Success creating "
                                              "new Nym. (User "
                                              "account fully "
                                              "created.)\n");

                            OTString strNymContents;
                            pNym->SavePseudonym(strNymContents);
                            msgOut.m_ascPayload.SetString(strNymContents);
                            msgOut.m_bSuccess = true;
                        }
                        msgOut.SignContract(server_->m_nymServer);
                        msgOut.SaveContract();
                        return true;
                    } // Success loading and verifying the Nym based on his
                      // credentials.
                } // Success decoding the map of credentials from the message.
            }     // credential list exists, after base64-decoding.
        }         // Has Credentials.
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
    if (!bNymIsServerNym &&
        (false == pNym->LoadPublicKey()) // && // Old style. (Deprecated, but
                                         // fine for now since it calls
                                         // LoadCredentials.)
        ) {
        OTLog::vError("Failure loading public credentials for Nym: %s\n",
                      theMessage.m_strNymID.Get());
        return false;
    }
    if (!bNymIsServerNym && pNym->IsMarkedForDeletion()) {
        OTLog::vOutput(0,
                       "(Failed) attempt by client to use a deleted Nym: %s\n",
                       theMessage.m_strNymID.Get());
        return false;
    }
    // Okay, the file was read into memory and Public Key was successfully
    // extracted!
    // Next, let's use that public key to verify (1) the NymID and (2) the
    // signature
    // on the message that we're processing.

    if (!pNym->VerifyPseudonym()) {
        OTLog::Output(
            0, "Pseudonym failed to verify. Hash of public key doesn't match "
               "Nym ID that was sent.\n");
        return false;
    }
    OTLog::Output(3, "Pseudonym verified!\n");

    // So far so good. Now let's see if the signature matches...
    if (!theMessage.VerifySignature(*pNym)) {
        OTLog::Output(0, "Signature verification failed!\n");
        return false;
    }
    OTLog::Output(3, "Signature verified! The message WAS signed by "
                     "the Nym\'s private key.\n");

    // Get the public key from pNym, and set it into the connection.
    // This is only for verified Nyms, (and we're verified in here!) We
    // do this so that
    // we have the option later to encrypt the replies back to the
    // client...(using the
    // client's public key that we set here.)
    if (nullptr != pConnection)
        pConnection->SetPublicKey(pNym->GetPublicEncrKey());

    // Now we might as well load up the rest of the Nym.
    // Notice I use the && to only load the nymfile if it's NOT the
    // server Nym.
    if (!bNymIsServerNym && !pNym->LoadSignedNymfile(server_->m_nymServer)) {
        OTLog::vError("Error loading Nymfile: %s\n",
                      theMessage.m_strNymID.Get());
        return false;
    }
    OTLog::Output(2, "Successfully loaded Nymfile into memory.\n");

    // ENTERING THE INNER SANCTUM OF SECURITY. If the user got all
    // the way to here,
    // Then he has passed multiple levels of security, and all
    // commands below will
    // assume the Nym is secure, validated, and loaded into memory
    // for use.
    //
    // But still need to verify the Request Number for all other
    // commands except
    // Get Request Number itself...

    // Request numbers start at 100 (currently). (Since certain
    // special messages USE 1 already...
    // Such as messages that occur before requestnumbers are
    // possible, like CreateUserAccount.)
    //
    int64_t lRequestNumber = 0;

    if (false ==
        pNym->GetCurrentRequestNum(server_->m_strServerID, lRequestNumber)) {
        OTLog::Output(0, "Nym file request number doesn't exist. "
                         "Apparently first-ever request to "
                         "server--but everything checks out. "
                         "(Shouldn't this request number have been "
                         "created already when the NymFile was "
                         "first created???????\n");
        // FIRST TIME!  This account has never before made a single
        // request to this server.
        // The above call always succeeds unless the number just
        // isn't there for that server.
        // Therefore, since it's the first time, we'll create it
        // now:
        pNym->IncrementRequestNum(server_->m_nymServer, server_->m_strServerID);

        // Call it again so that lRequestNumber is set to 1 also
        if (pNym->GetCurrentRequestNum(server_->m_strServerID,
                                       lRequestNumber)) {
            OTLog::Output(0, "Created first request number in Nym "
                             "file, apparently first-ever request. "
                             "(Shouldn't this have been created "
                             "already when the NymFile was first "
                             "created???????\n");
        }
        else {
            OTLog::Error("ERROR creating first request number in "
                         "Nym file.\n");
            return false;
        }
    }

    // At this point, I now have the current request number for this
    // nym in lRequestNumber
    // Let's compare it to the one that was sent in the message...
    // (This prevents attackers
    // from repeat-sending intercepted messages to the server.)

    // IF it's NOT a getRequest CMD, (therefore requires a request number)
    if (false == theMessage.m_strCommand.Compare("getRequest")) {
        // AND the request number attached does not match what we just
        // read out of the file...
        if (lRequestNumber != atol(theMessage.m_strRequestNum.Get())) {
            OTLog::vOutput(0, "Request number sent in this message "
                              "%ld does not match the one in the "
                              "file! (%lld)\n",
                           atol(theMessage.m_strRequestNum.Get()),
                           lRequestNumber);
            return false;
        }
        // it's not a getRequest CMD, and the request number
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
            pNym->GetUsageCredits() >= 0 &&
            (ServerSettings::GetOverrideNymID().size() <= 0 ||
             (0 !=
              ServerSettings::GetOverrideNymID().compare(
                  (theMessage.m_strNymID.Get()))))) {
            const int64_t& lUsageCredits = pNym->GetUsageCredits();

            if (0 == lUsageCredits) // If the User has ZERO
                                    // USAGE CREDITS LEFT. (Too
                                    // bad, even 1 would have
                                    // squeezed him by here.)
            {
                OTLog::vOutput(0, "Nym %s: ALL OUT of Usage "
                                  "Credits, while server is in "
                                  "**REQUIRE USAGE CREDITS "
                                  "MODE**!\n",
                               theMessage.m_strNymID.Get());
                return false;
            }

            const int64_t lUsageFinal = (lUsageCredits - 1);
            pNym->SetUsageCredits(lUsageFinal);
        }

        OTLog::vOutput(3, "Request number sent in this message "
                          "%lld DOES match the one in the "
                          "file!\n",
                       lRequestNumber);

        // At this point, it is some OTHER command (besides
        // getRequest)
        // AND the request number verifies, so we're going to
        // increment
        // the number, and let the command process.
        pNym->IncrementRequestNum(server_->m_nymServer, server_->m_strServerID);

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
    }
    else // If you entered this else, that means it IS a
           // getRequest command
    // So we allow it to go through without verifying this step,
    // and without incrementing the counter.
    {
        // pNym->IncrementRequestNum(server_->m_strServerID); //
        // commented
        // out cause this is the one case where we DON'T increment
        // this number.
        // We allow the user to get the number, we DON'T increment
        // it, and now the user
        // can send it on his next request for some other command,
        // and it will verify
        // properly. This prevents repeat messages.

        // NO RETURN HERE!!!! ON PURPOSE!!!!
    }

    // At this point, we KNOW that it is EITHER a GetRequest
    // command, which doesn't require a request number,
    // OR it was some other command, but the request number they
    // sent in the command MATCHES the one that we
    // just read out of the file.

    // Therefore, we can process ALL messages below this
    // point KNOWING that the Nym is properly verified in all ways.
    // No messages need to worry about verifying the Nym, or about
    // dealing with the Request Number. It's all handled in here.

    // If you made it down this far, that means the Pseudonym verifies! The
    // message is legit.
    //
    // (Server ID was good, NymID is a valid hash of User's public key, and the
    // Signature on
    // the message was signed by the user's private key.)
    //
    // Now we can process the message.
    //
    // All the commands below here, it is assumed that the user account exists
    // and is
    // referenceable via pNym. (An OTPseudonym object.)
    //
    // ALL commands below can assume the Nym is real, and that the NymID and
    // Public Key are
    // available for use -- and that they verify -- without having to check
    // again and again.

    // ACKNOWLEDGMENTS OF REPLIES ALREADY RECEIVED (FOR OPTIMIZATION.)

    // On the client side, whenever the client is DEFINITELY made aware of the
    // existence of a
    // server reply, he adds its request number to this list, which is sent
    // along with all client-side
    // requests to the server.
    // The server reads the list on the incoming client message (and it uses
    // these same functions
    // to store its own internal list.) If the # already appears on its internal
    // list, then it does
    // nothing. Otherwise, it loads up the Nymbox and removes the replyNotice,
    // and then adds the #
    // to its internal list.
    // For any numbers on the internal list but NOT on the client's list, the
    // server removes from
    // the internal list. (The client removed them when it saw the server's
    // internal list, which the
    // server sends with its replies.)
    //
    // This entire protocol, densely described, is unnecessary for OT to
    // function, but is great for
    // optimization, as it enables OT to avoid downloading all Box Receipts
    // containing replyNotices,
    // as long as the original reply was properly received when the request was
    // originally sent (which
    // is MOST of the time...)
    // Thus we can eliminate most replyNotice downloads, and likely a large % of
    // box receipt downloads
    // as well.
    //

    const OTIdentifier SERVER_ID(server_->m_strServerID);

    // The server reads the list of acknowledged replies from the incoming
    // client message...
    //
    bool bIsDirtyNym = false; // if we add any acknowledged replies to the
                              // server-side list, we will want to save (at the
                              // end.)
    std::set<int64_t> numlist_ack_reply;
    if (theMessage.m_AcknowledgedReplies.Output(
            numlist_ack_reply)) // returns false if the numlist was empty.
    {
        // Load Nymbox
        //
        OTLedger theNymbox(pNym->GetConstID(), pNym->GetConstID(), SERVER_ID);

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
                if (false ==
                    pNym->VerifyAcknowledgedNum(server_->m_strServerID,
                                                lRequestNum)) {
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
                            pReplyNotice->GetTransactionNum()); // deletes
                        pReplyNotice = nullptr;
                        // (pReplyNotice is deleted, below this point,
                        // automatically by the above Remove call.)

                        if (!bDeleted || !bRemoved)
                            OTLog::Error(
                                "UserCommandProcessor::ProcessUserCommand: "
                                "Failed trying "
                                "to delete a box receipt, or "
                                "while removing its stub from the Nymbox.\n");

                        // ...and add lRequestNum to server's acknowledgment
                        // list. (So this can't happen twice with same #.)
                        //
                        if (pNym->AddAcknowledgedNum(
                                server_->m_strServerID,
                                lRequestNum))   // doesn't save (here).
                            bIsDirtyNym = true; // So we don't have to save EACH
                                                // iteration, but instead just
                                                // once at the bottom.

                        if (bRemoved)
                            bIsDirtyNymbox = true; // So we don't have to save
                                                   // EACH iteration, but
                                                   // instead just once at the
                                                   // bottom.
                    }
                } // If server didn't already have a record of this acknowledged
                  // request #.
            }

            if (bIsDirtyNymbox) {
                theNymbox.ReleaseSignatures();
                theNymbox.SignContract(server_->m_nymServer);
                theNymbox.SaveContract();
                theNymbox.SaveNymbox();
            }
        } // If nymbox loaded and verified.
    }

    // For any numbers on the server's internal list but NOT on the client's
    // list, the server removes from
    // the internal list. (Because the client must have seen my acknowledgment
    // and thus removed the number
    // from its own list, so the server doesn't need to display it anymore.)
    //
    // Thus: iterate through the server's list of numbers, and see if each is on
    // the client's list. If not,
    // then remove it from my own (server's) internal list as well.
    //
    OTNumList numlist_to_remove; // a temp variable where we will put the
                                 // numbers "to be removed" (so we can remove
                                 // them all at once, after the loop.)
    const int32_t nAcknowledgedNumCount =
        pNym->GetAcknowledgedNumCount(SERVER_ID);

    if (nAcknowledgedNumCount > 0) {
        for (int32_t i = 0; i < nAcknowledgedNumCount; i++) {
            const int64_t lAcknowledgedNum =
                pNym->GetAcknowledgedNum(SERVER_ID, i); // index

            // For any numbers on the server's internal list but NOT on the
            // client's list (according
            // to the incoming message) the server removes them from its
            // internal list. (If the client
            // is done with them, then so is the server.)
            //
            if (false ==
                theMessage.m_AcknowledgedReplies.Verify(lAcknowledgedNum)) {
                numlist_to_remove.Add(lAcknowledgedNum);
            }
        }
        if (numlist_to_remove.Count() > 0) {
            std::set<int64_t> set_server_ack;
            if (numlist_to_remove.Output(set_server_ack)) {
                for (auto& it : set_server_ack) {
                    const int64_t lRequestNum = it;
                    if (pNym->RemoveAcknowledgedNum(server_->m_nymServer,
                                                    server_->m_strServerID,
                                                    lRequestNum, false))
                        bIsDirtyNym = true;
                }
            }
        }
    } // if there are server-side ack numbers that could potentially be removed,
      // if client's message doesn't list them.

    if (bIsDirtyNym) {
        pNym->SaveSignedNymfile(server_->m_nymServer); // we save here.
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

    msgOut.m_strServerID = server_->m_strServerID;
    msgOut.SetAcknowledgments(*pNym); // Must be called AFTER
                                      // msgOut.m_strServerID is already set.
                                      // (It uses it.)

    if (theMessage.m_strCommand.Compare("getRequest")) // This command is
                                                       // special because it's
                                                       // the only one that
                                                       // doesn't require a
                                                       // request number.
    // All of the other commands, below, will fail above if the proper
    // request number isn't included
    // in the message.  They will already have failed by this point if they
    // didn't verify.
    {
        OTLog::vOutput(0, "\n==> Received a getRequest message. Nym: %s ...\n",
                       strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_request);

        UserCmdGetRequest(*pNym, theMessage, msgOut);

        return true;
    }
    else if (theMessage.m_strCommand.Compare("getTransactionNum")) {
        OTLog::vOutput(
            0, "\n==> Received a getTransactionNum message. Nym: %s ...\n",
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_trans_num);

        UserCmdGetTransactionNum(*pNym, theMessage, msgOut);

        return true;
    }
    else if (theMessage.m_strCommand.Compare("checkUser")) {
        OTLog::vOutput(0, "\n==> Received a checkUser message. Nym: %s ...\n",
                       strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_check_user);

        UserCmdCheckUser(*pNym, theMessage, msgOut);

        return true;
    }
    else if (theMessage.m_strCommand.Compare("sendUserMessage")) {
        OTLog::vOutput(
            0, "\n==> Received a sendUserMessage message. Nym: %s ...\n",
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_send_message);

        UserCmdSendUserMessage(*pNym, theMessage, msgOut);

        return true;
    }
    else if (theMessage.m_strCommand.Compare("sendUserInstrument")) {
        OTLog::vOutput(
            0, "\n==> Received a sendUserInstrument message. Nym: %s ...\n",
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_send_message);

        UserCmdSendUserInstrument(*pNym, theMessage, msgOut);

        return true;
    }
    else if (theMessage.m_strCommand.Compare("deleteUserAccount")) {
        OTLog::vOutput(
            0, "\n==> Received a deleteUserAccount message. Nym: %s ...\n",
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_del_user_acct);

        UserCmdDeleteUser(*pNym, theMessage, msgOut);

        return true;
    }
    else if (theMessage.m_strCommand.Compare("deleteAssetAccount")) {
        OTLog::vOutput(
            0, "\n==> Received a deleteAssetAccount message. Nym: %s ...\n",
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_del_asset_acct);

        UserCmdDeleteAssetAcct(*pNym, theMessage, msgOut);

        return true;
    }
    else if (theMessage.m_strCommand.Compare("createAccount")) {
        OTLog::vOutput(0,
                       "\n==> Received a createAccount message. Nym: %s ...\n",
                       strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_create_asset_acct);

        UserCmdCreateAccount(*pNym, theMessage, msgOut);

        return true;
    }
    else if (theMessage.m_strCommand.Compare("issueAssetType")) {
        OTLog::vOutput(
            0, "\n==> Received an issueAssetType message. Nym: %s ...\n",
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_issue_asset);

        UserCmdIssueAssetType(*pNym, theMessage, msgOut);

        return true;
    }
    else if (theMessage.m_strCommand.Compare("issueBasket")) {
        OTLog::vOutput(0,
                       "\n==> Received an issueBasket message. Nym: %s ...\n",
                       strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_issue_basket);

        UserCmdIssueBasket(*pNym, theMessage, msgOut);

        return true;
    }
    else if (theMessage.m_strCommand.Compare("notarizeTransactions")) {
        OTLog::vOutput(0, "\n==> Received a notarizeTransactions message.  "
                          "Acct: %s Nym: %s  ...\n",
                       theMessage.m_strAcctID.Get(), strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_notarize_transaction);

        UserCmdNotarizeTransactions(*pNym, theMessage, msgOut);

        return true;
    }
    else if (theMessage.m_strCommand.Compare("getNymbox")) {
        OTLog::vOutput(0, "\n==> Received a getNymbox message. Nym: %s ...\n",
                       strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_nymbox);

        UserCmdGetNymbox(*pNym, theMessage, msgOut);

        return true;
    }
    else if (theMessage.m_strCommand.Compare("getBoxReceipt")) {
        OTLog::vOutput(0,
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

        if (bRunIt) UserCmdGetBoxReceipt(*pNym, theMessage, msgOut);

        return true;
    }
    else if (theMessage.m_strCommand.Compare("getInbox")) {
        OTLog::vOutput(
            0, "\n==> Received a getInbox message.  Acct: %s Nym: %s  ...\n",
            theMessage.m_strAcctID.Get(), strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_inbox);

        UserCmdGetInbox(*pNym, theMessage, msgOut);

        return true;
    }
    else if (theMessage.m_strCommand.Compare("getOutbox")) {
        OTLog::vOutput(
            0, "\n==> Received a getOutbox message.  Acct: %s Nym: %s  ...\n",
            theMessage.m_strAcctID.Get(), strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_outbox);

        UserCmdGetOutbox(*pNym, theMessage, msgOut);

        return true;
    }
    else if (theMessage.m_strCommand.Compare("getAccount")) {
        OTLog::vOutput(
            0, "\n==> Received a getAccount message.  Acct: %s Nym: %s  ...\n",
            theMessage.m_strAcctID.Get(), strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_acct);

        UserCmdGetAccount(*pNym, theMessage, msgOut);

        return true;
    }
    else if (theMessage.m_strCommand.Compare("getAccountFiles")) {
        OTLog::vOutput(0, "\n==> Received a getAccountFiles message.  Acct: %s "
                          "Nym: %s  ...\n",
                       theMessage.m_strAcctID.Get(), strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_inbox);
        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_outbox);
        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_acct);

        UserCmdGetAccountFiles(*pNym, theMessage, msgOut);

        return true;
    }
    else if (theMessage.m_strCommand.Compare("processNymbox")) {
        OTLog::vOutput(0,
                       "\n==> Received a processNymbox message. Nym: %s ...\n",
                       strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_process_nymbox);

        UserCmdProcessNymbox(*pNym, theMessage, msgOut);

        return true;
    }
    else if (theMessage.m_strCommand.Compare("processInbox")) {
        OTLog::vOutput(
            0, "\n==> Received a processInbox message. Acct: %s Nym: %s  ...\n",
            theMessage.m_strAcctID.Get(), strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_process_inbox);

        UserCmdProcessInbox(*pNym, theMessage, msgOut);

        return true;
    }
    else if (theMessage.m_strCommand.Compare("queryAssetTypes")) {
        OTLog::vOutput(
            0, "\n==> Received a queryAssetTypes message. Nym: %s ...\n",
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_contract);

        UserCmdQueryAssetTypes(*pNym, theMessage, msgOut);

        return true;
    }
    else if (theMessage.m_strCommand.Compare("getContract")) {
        OTLog::vOutput(0, "\n==> Received a getContract message. Nym: %s ...\n",
                       strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_contract);

        UserCmdGetContract(theMessage, msgOut);

        return true;
    }
    else if (theMessage.m_strCommand.Compare("getMint")) {
        OTLog::vOutput(0, "\n==> Received a getMint message. Nym: %s ...\n",
                       strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_mint);

        UserCmdGetMint(*pNym, theMessage, msgOut);

        return true;
    }
    else if (theMessage.m_strCommand.Compare("getMarketList")) {
        OTLog::vOutput(0,
                       "\n==> Received a getMarketList message. Nym: %s ...\n",
                       strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_market_list);

        UserCmdGetMarketList(*pNym, theMessage, msgOut);

        return true;
    }
    else if (theMessage.m_strCommand.Compare("getMarketOffers")) {
        OTLog::vOutput(
            0, "\n==> Received a getMarketOffers message. Nym: %s ...\n",
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_market_offers);

        UserCmdGetMarketOffers(*pNym, theMessage, msgOut);

        return true;
    }
    else if (theMessage.m_strCommand.Compare("getMarketRecentTrades")) {
        OTLog::vOutput(
            0, "\n==> Received a getMarketRecentTrades message. Nym: %s ...\n",
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(
            ServerSettings::__cmd_get_market_recent_trades);

        UserCmdGetMarketRecentTrades(*pNym, theMessage, msgOut);

        return true;
    }
    else if (theMessage.m_strCommand.Compare("getNym_MarketOffers")) {
        OTLog::vOutput(
            0, "\n==> Received a getNym_MarketOffers message. Nym: %s ...\n",
            strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_get_nym_market_offers);

        UserCmdGetNym_MarketOffers(*pNym, theMessage, msgOut);

        return true;
    }
    else if (theMessage.m_strCommand.Compare("triggerClause")) {
        OTLog::vOutput(0,
                       "\n==> Received a triggerClause message. Nym: %s ...\n",
                       strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_trigger_clause);

        UserCmdTriggerClause(*pNym, theMessage, msgOut);

        return true;
    }
    else if (theMessage.m_strCommand.Compare("usageCredits")) {
        OTLog::vOutput(0,
                       "\n==> Received a usageCredits message. Nym: %s ...\n",
                       strMsgNymID.Get());

        OT_ENFORCE_PERMISSION_MSG(ServerSettings::__cmd_usage_credits);

        UserCmdUsageCredits(*pNym, theMessage, msgOut);

        return true;
    }
    else {
        OTLog::vError("Unknown command type in the XML, or missing payload, in "
                      "ProcessMessage.\n");

        OTString strTemp;
        strTemp.Format("@%s", theMessage.m_strCommand.Get()); // Todo security.
                                                              // Review this.

        msgOut.m_strCommand = strTemp;
        msgOut.m_strAcctID = theMessage.m_strAcctID;
        msgOut.m_strServerID = theMessage.m_strServerID;
        msgOut.m_strNymID = theMessage.m_strNymID;

        msgOut.m_bSuccess = false;

        OTString strRef(theMessage);

        msgOut.m_ascInReferenceTo.SetString(strRef);

        msgOut.SignContract(server_->m_nymServer);
        msgOut.SaveContract();

        return false;
    }
}

// Get the list of markets on this server.
void UserCommandProcessor::UserCmdGetMarketList(OTPseudonym&, OTMessage& MsgIn,
                                                OTMessage& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "@getMarketList"; // reply to getMarketList
    msgOut.m_strNymID = MsgIn.m_strNymID;   // UserID
    //    msgOut.m_strServerID    = m_strServerID;    // This is already set in
    // ProcessUserCommand.

    OTASCIIArmor ascOutput;
    int32_t nMarketCount = 0;

    msgOut.m_bSuccess = server_->m_Cron.GetMarketList(ascOutput, nMarketCount);

    // If success,
    if ((true == msgOut.m_bSuccess) && (nMarketCount > 0)) {
        msgOut.m_ascPayload.Set(ascOutput);

        OTString strCount;
        strCount.Format("%d", nMarketCount);
        msgOut.m_lDepth = atol(strCount.Get());
    }
    // if Failed, we send the user's message back to him, ascii-armored as part
    // of response.
    else if (!msgOut.m_bSuccess) {
        OTString tempInMessage(MsgIn);
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
void UserCommandProcessor::UserCmdGetMarketOffers(OTPseudonym&,
                                                  OTMessage& MsgIn,
                                                  OTMessage& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "@getMarketOffers"; // reply to getMarketOffers
    msgOut.m_strNymID = MsgIn.m_strNymID;     // UserID
    msgOut.m_strNymID2 = MsgIn.m_strNymID2;   // Market ID.

    int64_t lDepth = MsgIn.m_lDepth;
    if (lDepth < 0) lDepth = 0;

    const OTIdentifier MARKET_ID(MsgIn.m_strNymID2);

    OTMarket* pMarket = server_->m_Cron.GetMarket(MARKET_ID);

    // If success,
    if ((msgOut.m_bSuccess =
             ((pMarket != nullptr) ? true : false))) // if assigned true
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
        OTString tempInMessage(MsgIn);
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
void UserCommandProcessor::UserCmdGetMarketRecentTrades(OTPseudonym&,
                                                        OTMessage& MsgIn,
                                                        OTMessage& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand =
        "@getMarketRecentTrades";           // reply to getMarketRecentTrades
    msgOut.m_strNymID = MsgIn.m_strNymID;   // UserID
    msgOut.m_strNymID2 = MsgIn.m_strNymID2; // Market ID.

    const OTIdentifier MARKET_ID(MsgIn.m_strNymID2);

    OTMarket* pMarket = server_->m_Cron.GetMarket(MARKET_ID);

    // If success,
    if ((msgOut.m_bSuccess =
             ((pMarket != nullptr) ? true : false))) // if assigned true
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
        OTString tempInMessage(MsgIn);
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
void UserCommandProcessor::UserCmdGetNym_MarketOffers(OTPseudonym& theNym,
                                                      OTMessage& MsgIn,
                                                      OTMessage& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "@getNym_MarketOffers"; // reply to getMarketOffers
    msgOut.m_strNymID = MsgIn.m_strNymID;         // UserID

    OTIdentifier NYM_ID;
    theNym.GetIdentifier(NYM_ID);

    OTASCIIArmor ascOutput;
    int32_t nOfferCount = 0;

    msgOut.m_bSuccess =
        server_->m_Cron.GetNym_OfferList(ascOutput, NYM_ID, nOfferCount);

    if ((true == msgOut.m_bSuccess) && (nOfferCount > 0)) {
        msgOut.m_ascPayload = ascOutput;
        msgOut.m_lDepth = nOfferCount;
    }
    // if Failed, we send the user's message back to him, ascii-armored as part
    // of response.
    if (!msgOut.m_bSuccess) {
        OTString tempInMessage(MsgIn);
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

void UserCommandProcessor::UserCmdCheckServerID(OTPseudonym&, OTMessage& MsgIn,
                                                OTMessage& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "@checkServerID";
    msgOut.m_strNymID = MsgIn.m_strNymID;

    if (MsgIn.m_strServerID == server_->m_strServerID)
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

void UserCommandProcessor::UserCmdGetTransactionNum(OTPseudonym& theNym,
                                                    OTMessage& MsgIn,
                                                    OTMessage& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "@getTransactionNum"; // reply to getTransactionNum
    msgOut.m_strNymID = MsgIn.m_strNymID;       // UserID

    const OTIdentifier SERVER_ID(server_->m_strServerID);

    // A few client requests, and a few server replies (not exactly matched up)
    // will include a copy of the NymboxHash.  The server may reject certain
    // client requests that have a bad value here (since it would be out of sync
    // anyway); the client is able to see the server's hash and realize to
    // re-download the nymbox and other intermediary files.
    //
    int32_t nCount = theNym.GetTransactionNumCount(SERVER_ID);
    const OTIdentifier theMsgNymboxHash(
        MsgIn.m_strNymboxHash); // theMsgNymboxHash is the hash sent by the
                                // client side
    OTIdentifier EXISTING_NYMBOX_HASH;

    const bool bGotNymboxHashServerSide =
        theNym.GetNymboxHashServerSide(SERVER_ID, EXISTING_NYMBOX_HASH);
    const bool bGotNymboxHashClientSide = MsgIn.m_strNymboxHash.Exists();

    if (bGotNymboxHashServerSide) // EXISTING_NYMBOX_HASH is the hash stored on
                                  // the server side
        EXISTING_NYMBOX_HASH.GetString(msgOut.m_strNymboxHash);
    if ((bGotNymboxHashServerSide && bGotNymboxHashClientSide &&
         (theMsgNymboxHash != EXISTING_NYMBOX_HASH)) ||
        (bGotNymboxHashServerSide && !bGotNymboxHashClientSide)) {
        OTLog::Output(
            0, "UserCommandProcessor::UserCmdGetTransactionNum: Rejecting "
               "message since nymbox hash "
               "doesn't match. (Send a getNymbox message to grab the "
               "newest one.)\n");

    }
    else if (nCount > 50) // todo no hardcoding. (max transaction nums allowed
                            // out at a single time.)
    {
        OTLog::vOutput(
            0,
            "UserCommandProcessor::UserCmdGetTransactionNum: Failure: Nym %s "
            "already has "
            "more than 50 unused transaction numbers signed out. (He needs to "
            "use those first. "
            "Tell him to download his latest Nymbox.)\n",
            MsgIn.m_strNymID.Get());
    }
    else {
        OTIdentifier USER_ID, NYMBOX_HASH;
        theNym.GetIdentifier(USER_ID);

        bool bSuccess = true;
        bool bSavedNymbox = false;
        OTLedger theLedger(USER_ID, USER_ID, SERVER_ID); // Nymbox

        // We'll store the transaction numbers here immediately after they're
        // issued,
        // before adding them to the Nymbox.
        //
        OTNumList theNumlist;

        int64_t lFirstTransNum =
            0; // While there may be 20 transaction numbers on this tranasction,
               // ONE of them (the first one) is the "official" number of this
               // transaction. The rest are just attached in an extra variable.

        // Update: Now we're going to grab 20 or 30 transaction numbers,
        // instead of just 1 like before!!!
        //
        for (int32_t i = 0; i < 100; i++) // todo, hardcoding!!!! (But notice we
                                          // grab 100 transaction numbers at a
                                          // time now.)
        {
            int64_t lTransNum = 0;
            // This call will save the new transaction number to the nym's file.
            // ??? Why did I say that, when clearly the last parameter is false?
            // AHHHH Because I drop it into the Nymbox instead, and make him
            // sign for it!

            if (false ==
                server_->transactor_.issueNextTransactionNumber(
                    theNym, lTransNum, false)) {
                lTransNum = 0;
                OTLog::Error("UserCommandProcessor::UserCmdGetTransactionNu: "
                             "Error issuing "
                             "next transaction number!\n");
                bSuccess = false;
                break;
            }

            theNumlist.Add(lTransNum); // <=========
            if (0 == i)                // First iteration
                lFirstTransNum = lTransNum;
        }

        if (!bSuccess) {
            // Apparently nothing. Also, plenty of logs just above already, if
            // this ever happens.
        }
        else if (!theLedger.LoadNymbox()) {
            OTLog::Error("Error loading Nymbox in "
                         "UserCommandProcessor::UserCmdGetTransactionNum\n");
        }
        // Drop in the Nymbox
        else if ((msgOut.m_bSuccess =
                      (theLedger.VerifyContractID() && // We don't need them
                       // right now, so we verify
                       theLedger.VerifySignature(
                           server_->m_nymServer) // everything else without
                                                 // loading them.
                       )                         // if loaded and verified.
                  ))                             // if success
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
                theLedger, OTTransaction::blank,
                lFirstTransNum); // Generate a new OTTransaction::blank

            if (nullptr !=
                pTransaction) // The above has an OT_ASSERT within, but
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
            }
            else
                theLedger.CalculateNymboxHash(NYMBOX_HASH);
        }
        else {
            OTLog::Error("Error verifying Nymbox in "
                         "UserCommandProcessor::UserCmdGetTransactionNum\n");
        }
        std::set<int64_t> theList;
        theNumlist.Output(theList);

        for (auto& it : theList) {
            const int64_t lTransNum = it;
            server_->transactor_.removeTransactionNumber(theNym, lTransNum,
                                                         false);
            server_->transactor_.removeIssuedNumber(
                theNym, lTransNum, false); // I'll drop it in his
                                           // Nymbox -- he can
                                           // SIGN for it.
            // Then why was it added in the first place? Because we originally
            // sent it back in the reply directly,
            // so IssueNext was designed to work that way originally.
        }

        if (bSavedNymbox) {
            theNym.SetNymboxHashServerSide(NYMBOX_HASH);
            theNym.SaveSignedNymfile(server_->m_nymServer);

            NYMBOX_HASH.GetString(msgOut.m_strNymboxHash);
        }
        else if (true == msgOut.m_bSuccess) {
            theLedger.CalculateNymboxHash(NYMBOX_HASH);

            theNym.SetNymboxHashServerSide(
                NYMBOX_HASH); // Save the hash onto the Nym
            theNym.SaveSignedNymfile(server_->m_nymServer);

            NYMBOX_HASH.GetString(
                msgOut.m_strNymboxHash); // Get the hash onto the message
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

void UserCommandProcessor::UserCmdGetRequest(OTPseudonym& theNym,
                                             OTMessage& MsgIn,
                                             OTMessage& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "@getRequest";  // reply to getRequest
    msgOut.m_strNymID = MsgIn.m_strNymID; // UserID

    msgOut.m_strRequestNum.Set(MsgIn.m_strRequestNum); // Outoing reply contains
                                                       // same request num
                                                       // coming in (1).

    int64_t lReqNum = 1; // The request number being REQUESTED (in this message)
                         // will be sent in msgOut.m_lNewRequestNum

    msgOut.m_bSuccess =
        theNym.GetCurrentRequestNum(server_->m_strServerID, lReqNum);

    // Server was unable to load ReqNum, which is unusual because the calling
    // function
    // should have already insured its existence.
    if (!msgOut.m_bSuccess) {
        OTLog::Error("Error loading request number in "
                     "UserCommandProcessor::UserCmdGetRequest\n");
    }
    else
        msgOut.m_lNewRequestNum = lReqNum;

    const OTIdentifier SERVER_ID(server_->m_strServerID);
    OTIdentifier EXISTING_NYMBOX_HASH;
    if (theNym.GetNymboxHashServerSide(SERVER_ID, EXISTING_NYMBOX_HASH))
        EXISTING_NYMBOX_HASH.GetString(msgOut.m_strNymboxHash);
    else {
        const OTIdentifier theNymID(theNym);
        OTLedger theLedger(theNymID, theNymID, SERVER_ID);

        if (theLedger.LoadNymbox() && theLedger.VerifyContractID() &&
            theLedger.VerifySignature(server_->m_nymServer)) {
            theLedger.CalculateNymboxHash(EXISTING_NYMBOX_HASH);

            theNym.SetNymboxHashServerSide(EXISTING_NYMBOX_HASH);
            theNym.SaveSignedNymfile(server_->m_nymServer);

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

void UserCommandProcessor::UserCmdSendUserMessage(OTPseudonym& theNym,
                                                  OTMessage& MsgIn,
                                                  OTMessage& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "@sendUserMessage"; // reply to sendUserMessage
    msgOut.m_strNymID = MsgIn.m_strNymID;     // UserID
    msgOut.m_strNymID2 = MsgIn.m_strNymID2;   // UserID of recipient pubkey

    const OTString strInMessage(MsgIn);
    const OTIdentifier SENDER_USER_ID(theNym),
        RECIPIENT_USER_ID(MsgIn.m_strNymID2), SERVER_ID(server_->m_strServerID);
    msgOut.m_ascInReferenceTo.SetString(strInMessage);
    const bool bSent =
        SendMessageToNym(SERVER_ID, SENDER_USER_ID, RECIPIENT_USER_ID,
                         &MsgIn); // pstrMessage=nullptr

    if (!bSent) {
        OTLog::vError("UserCommandProcessor::UserCmdSendUserMessage: Failed "
                      "while calling "
                      "SendMessageToNym.\n");
        msgOut.m_bSuccess = false;
    }
    else {
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

void UserCommandProcessor::UserCmdSendUserInstrument(OTPseudonym& theNym,
                                                     OTMessage& MsgIn,
                                                     OTMessage& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "@sendUserInstrument"; // reply to sendUserInstrument
    msgOut.m_strNymID = MsgIn.m_strNymID;        // UserID
    msgOut.m_strNymID2 = MsgIn.m_strNymID2;      // UserID of recipient pubkey

    const OTString strInMessage(MsgIn);
    const OTIdentifier SENDER_USER_ID(theNym),
        RECIPIENT_USER_ID(MsgIn.m_strNymID2), SERVER_ID(server_->m_strServerID);
    msgOut.m_ascInReferenceTo.SetString(strInMessage);
    const bool bSent = server_->SendInstrumentToNym(
        SERVER_ID, SENDER_USER_ID, RECIPIENT_USER_ID,
        &MsgIn); // pPayment=nullptr, szCommand=nullptr

    if (!bSent) {
        OTLog::vError(
            "UserCommandProcessor::UserCmdSendUserInstrument: Failed while "
            "calling SendInstrumentToNym.\n");
        msgOut.m_bSuccess = false;
    }
    else {
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

void UserCommandProcessor::UserCmdCheckUser(OTPseudonym&, OTMessage& MsgIn,
                                            OTMessage& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "@checkUser";   // reply to checkUser
    msgOut.m_strNymID = MsgIn.m_strNymID; // UserID
    msgOut.m_strNymID2 =
        MsgIn.m_strNymID2; // UserID of public key requested by user.

    OTPseudonym nym2;
    nym2.SetIdentifier(MsgIn.m_strNymID2);

    bool bLoaded = nym2.LoadPublicKey(); // This calls LoadCredentials inside.
    msgOut.m_bSuccess = (bLoaded && nym2.VerifyPseudonym());

    // If success, we send the Nym2's public key back to the user.
    if (msgOut.m_bSuccess) {
        nym2.GetPublicEncrKey().GetPublicKey(msgOut.m_strNymPublicKey, true);

        // NEW: Also attach the public credentials to the response
        //      (not just a public key.)
        //
        if (nym2.GetMasterCredentialCount() > 0) {
            // Create an OTDB::StringMap object.

            // this asserts already, on failure.
            std::unique_ptr<OTDB::Storable> pStorable(
                OTDB::CreateObject(OTDB::STORED_OBJ_STRING_MAP));
            OTDB::StringMap* pMap =
                dynamic_cast<OTDB::StringMap*>(pStorable.get());

            if (nullptr == pMap) {
                OTLog::vError("%s: Error: failed trying to create a "
                              "STORED_OBJ_STRING_MAP.\n",
                              __FUNCTION__);
                msgOut.m_bSuccess = false;
            }
            else {
                OTString strCredList;
                auto& theMap = pMap->the_map;

                nym2.GetPublicCredentials(strCredList, &theMap);
                // Serialize the StringMap to a string...
                //
                if (!theMap.empty()) // Won't bother if zero credentials
                {
                    std::string str_Encoded = OTDB::EncodeObject(*pMap);
                    const bool bSuccessEncoding = (str_Encoded.size() > 0);
                    if (bSuccessEncoding) {
                        msgOut.m_ascPayload.SetString(
                            strCredList); // <========== Success
                        msgOut.m_ascPayload2.Set(
                            str_Encoded.c_str()); // Payload contains
                                                  // credentials list, payload2
                                                  // contains actual
                                                  // credentials.
                    }
                }
            }
        } // bLoadedCredentials
    }     // msgOut.m_bSuccess
    // if Failed, we send the user's message back to him, ascii-armored as part
    // of response.
    else {
        OTString tempInMessage(MsgIn);
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
void UserCommandProcessor::UserCmdUsageCredits(OTPseudonym& theNym,
                                               OTMessage& MsgIn,
                                               OTMessage& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "@usageCredits";  // reply to usageCredits
    msgOut.m_strNymID = MsgIn.m_strNymID;   // UserID
    msgOut.m_strNymID2 = MsgIn.m_strNymID2; // UserID of user whose usage
                                            // credits are being examined /
                                            // adjusted.
    const bool bIsPrivilegedNym =
        ((ServerSettings::GetOverrideNymID().size() >
          0) && // And if there's an override Nym...
         (0 ==
          ServerSettings::GetOverrideNymID().compare(
              (MsgIn.m_strNymID.Get())))); // And if the acting Nym IS the
                                           // override Nym...
    // The amount the usage credits are being ADJUSTED by.
    const int64_t lAdjustment =
        (bIsPrivilegedNym && ServerSettings::__admin_usage_credits)
            ? MsgIn.m_lDepth
            : 0;

    msgOut.m_lDepth = 0; // Returns total Usage Credits on Nym at the end.
    OTPseudonym nym2;
    OTIdentifier nym2ID, SERVER_ID(server_->m_strServerID);
    nym2.SetIdentifier(MsgIn.m_strNymID2);
    nym2.GetIdentifier(nym2ID);

    const bool bIsSameNym = nym2.CompareID(theNym);
    OTPseudonym* pNym = nullptr;

    bool bErrorCondition = false;

    // If nym2 and theNym are already the same Nym, then pNym points to theNym
    // by now already.
    // (And we'll skip this block.) Otherwise we load up nym2, and point pNym to
    // nym2 instead.
    //
    if (bIsSameNym)
        pNym = &theNym;
    else // theNym and nym2 are different Nyms, so let's load it up.
    {
        bool bLoadedPublicKey =
            nym2.LoadPublicKey() &&
            nym2.VerifyPseudonym(); // Old style (deprecated.) For now, this
                                    // calls LoadCredentials inside (which is
                                    // the new style.) Eventually we'll just
                                    // call that here directly.
        bool bLoadSignedNymfile = nym2.LoadSignedNymfile(server_->m_nymServer);
        if (!bLoadSignedNymfile &&
            !bLoadedPublicKey) // Nym didn't already exist.
        {
            pNym = &nym2;
        }
        else if (bLoadedPublicKey && !bLoadSignedNymfile) // Error -- if key
                                                            // was there, then
                                                            // nymfile should
                                                            // have been also.
        {
            OTLog::vError(
                "%s: Nym public key (%s) exists, but nymfile doesn't! "
                "Could be error reading from storage. (Failure.)\n",
                __FUNCTION__, MsgIn.m_strNymID2.Get());
            bErrorCondition = true;
        }
        else {
            pNym = &nym2;
        }
    }
    if (!MsgIn.m_strNymID.Compare(MsgIn.m_strNymID2)) // If the Nym is
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
                   (MsgIn.m_strNymID.Get()))))) // ...And if he's
                                                // not the special
                                                // "override Nym"...
        {
            OTLog::vError("%s: Failed attempt by a normal Nym to view or "
                          "adjust usage credits on a different Nym (you're "
                          "only allowed to do this to yourself, unless your "
                          "nym is the specially-empowered 'override nym'.)\n",
                          __FUNCTION__);
            bErrorCondition = true;
        }
    }
    OTLedger theNymbox(nym2ID, nym2ID, SERVER_ID);
    bool bSuccessLoadingNymbox = theNymbox.LoadNymbox();

    if (bSuccessLoadingNymbox)
        bSuccessLoadingNymbox = (theNymbox.VerifyContractID() &&
                                 theNymbox.VerifyAccount(server_->m_nymServer));
    else {
        bSuccessLoadingNymbox =
            theNymbox.GenerateLedger(nym2ID, SERVER_ID, OTLedger::nymbox, true);

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
                : lTentativeNew; // It can never be less than -1.

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
                server_->m_nymServer); // We changed it, so let's save pNym...
        }
        else
            msgOut.m_bSuccess = true; // No adjustment -- we're just returning
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
        if (ServerSettings::__admin_usage_credits) // If the server has usage
                                                   // credits
                                                   // turned on...
            msgOut.m_lDepth = lNewCredits; // ...then adjustment or not, we send
                                           // the current usage credits balance
                                           // back in the server reply.
        else // Else if the server does NOT have usage credits turned on...
            msgOut.m_lDepth = -1; // ...then we always return -1, so the client
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
void UserCommandProcessor::UserCmdIssueAssetType(OTPseudonym& theNym,
                                                 OTMessage& MsgIn,
                                                 OTMessage& msgOut)
{
    const char* szFunc = "UserCommandProcessor::UserCmdIssueAssetType";

    // (1) set up member variables
    msgOut.m_strCommand = "@issueAssetType"; // reply to issueAssetType
    msgOut.m_strNymID = MsgIn.m_strNymID;    // UserID
    msgOut.m_strAssetID =
        MsgIn.m_strAssetID; // Asset Type ID, a hash of the asset contract.

    const OTIdentifier USER_ID(theNym), SERVER_ID(server_->m_strServerID),
        ASSET_TYPE_ID(MsgIn.m_strAssetID);

    OTAssetContract* pAssetContract =
        server_->transactor_.getAssetContract(ASSET_TYPE_ID);

    // Make sure the contract isn't already available on this server.
    //
    if (nullptr != pAssetContract) // it exists already.
    {
        OTLog::vError(
            "%s: Error: Attempt to issue asset type that already exists.\n",
            szFunc);
    }
    else {
        // Pull the contract out of the message and verify it.
        OTString strFoldername(OTFolders::Contract().Get()),
            strFilename(MsgIn.m_strAssetID.Get());

        OTString strContract(MsgIn.m_ascPayload);
        pAssetContract = new OTAssetContract(MsgIn.m_strAssetID, strFoldername,
                                             strFilename, MsgIn.m_strAssetID);

        OTIdentifier ASSET_USER_ID;
        bool bSuccessCalculateDigest = false;

        if (nullptr == pAssetContract) {
            OTLog::vOutput(0, "%s: Failed trying to instantiate asset "
                              "contract. Asset ID: %s\n",
                           szFunc, MsgIn.m_strAssetID.Get());
        }
        else // success instantiating contract.
        {
            bool bSuccessLoadingContract =
                pAssetContract->LoadContractFromString(strContract);

            if (!bSuccessLoadingContract) {
                OTLog::vOutput(0, "%s: Failed trying to load asset contract "
                                  "from string. Asset ID: %s\n",
                               szFunc, MsgIn.m_strAssetID.Get());
                OTLog::vOutput(1, "%s: Failed trying to load asset contract "
                                  "from string. Contract:\n\n%s\n\n",
                               szFunc, strContract.Get());
            }
            else if (pAssetContract->GetBasketInfo().Exists()) {
                OTLog::vOutput(0, "%s: Prevented attempt by user to issue a "
                                  "basket currency contract. (He needs to use "
                                  "the issueBasket message for that.)\n",
                               szFunc);
            }
            else // success loading contract from string.
            {
                OTPseudonym* pNym = const_cast<OTPseudonym*>(
                    pAssetContract->GetContractPublicNym());

                if (nullptr == pNym) {
                    OTLog::vOutput(0, "%s: Failed trying to retrieve Issuer's "
                                      "public key from asset "
                                      "contract. Asset ID: %s\n",
                                   szFunc, MsgIn.m_strAssetID.Get());
                }
                else // success retrieving issuer Nym's public key from asset
                       // contract.
                {
                    pNym->GetIdentifier(ASSET_USER_ID);

                    bSuccessCalculateDigest = true;
                }
            }
        }

        // Make sure the public key in the contract is the public key of the
        // Nym.
        // If we successfully loaded the contract from the string, and the
        // contract
        // internally verifies (the ID matches the hash of the contract, and the
        // signature verifies with the contract key that's inside the contract),
        // AND the Nym making this request has the same ID as the Nym in the
        // asset contract. (ONLY the issuer of that contract can connect to this
        // server and issue his currency.)
        // TODO make sure a receipt is issued that the issuer can post on his
        // website, to verify that he has indeed issued the currency at the
        // specified
        // transaction processor.  That way, users can double-check.
        if (bSuccessCalculateDigest) {
            if ((ASSET_USER_ID == USER_ID))
            // The ID of the user who signed the contract must be the ID of
            // the user
            // whose public key is associated with this user account. They
            // are one.
            {
                if (pAssetContract->VerifyContract()) {
                    // Create an ISSUER account (like a normal account, except
                    // it can go negative)

                    std::unique_ptr<OTAccount> pNewAccount(
                        OTAccount::GenerateNewAccount(
                            USER_ID, SERVER_ID, server_->m_nymServer, MsgIn,
                            OTAccount::issuer));

                    // If we successfully create the account, then bundle it in
                    // the message XML payload
                    if (nullptr !=
                        pNewAccount) // This last parameter generates an
                                     // ISSUER account
                    {                // instead of the default SIMPLE.
                        OTString tempPayload(*pNewAccount);
                        msgOut.m_ascPayload.SetString(tempPayload);

                        // Attach the new account number to the outgoing
                        // message.
                        pNewAccount->GetIdentifier(msgOut.m_strAcctID);

                        // Now that the account is actually created, let's add
                        // the new asset contract
                        // to the server's list.
                        server_->transactor_.addAssetContract(
                            *pAssetContract);              // Do NOT clean this
                                                           // up unless failure!
                                                           // Server will clean
                                                           // it up.
                        server_->mainFile_.SaveMainFile(); // So the main xml
                                                           // file knows
                                                           // to load
                        // this asset type next time we run.

                        // Make sure the contracts/%s file is created for next
                        // time.
                        pAssetContract->SaveContract(
                            OTFolders::Contract().Get(), strFilename.Get());
                        OTIdentifier theNewAccountID;
                        pNewAccount->GetIdentifier(theNewAccountID);
                        OTLog::Output(
                            0,
                            "Generating inbox/outbox for new issuer acct. \n");

                        OTLedger theOutbox(USER_ID, theNewAccountID, SERVER_ID),
                            theInbox(USER_ID, theNewAccountID, SERVER_ID);

                        bool bSuccessLoadingInbox = theInbox.LoadInbox();
                        bool bSuccessLoadingOutbox = theOutbox.LoadOutbox();
                        // ...or generate them otherwise...

                        if (true ==
                            bSuccessLoadingInbox) // WEIRD IF THIS HAPPENED...
                            bSuccessLoadingInbox = theInbox.VerifyAccount(
                                server_->m_nymServer); // todo -- this should
                                                       // NEVER
                        // happen, the ID was just
                        // RANDOMLY generated, so HOW did
                        // the inbox already exist???
                        else {
                            bSuccessLoadingInbox = theInbox.GenerateLedger(
                                theNewAccountID, SERVER_ID, OTLedger::inbox,
                                true);

                            if (bSuccessLoadingInbox) {
                                bSuccessLoadingInbox =
                                    theInbox.SignContract(server_->m_nymServer);

                                if (bSuccessLoadingInbox) {
                                    bSuccessLoadingInbox =
                                        theInbox.SaveContract();

                                    if (bSuccessLoadingInbox)
                                        bSuccessLoadingInbox =
                                            pNewAccount->SaveInbox(theInbox);
                                }
                            }
                        }
                        if (true ==
                            bSuccessLoadingOutbox) // WEIRD IF THIS HAPPENED....
                            bSuccessLoadingOutbox = theOutbox.VerifyAccount(
                                server_->m_nymServer); // todo -- this should
                                                       // NEVER
                        // happen, the ID was just
                        // RANDOMLY generated, so HOW did
                        // the outbox already exist???
                        else {
                            bSuccessLoadingOutbox = theOutbox.GenerateLedger(
                                theNewAccountID, SERVER_ID, OTLedger::outbox,
                                true);

                            if (bSuccessLoadingOutbox) {
                                bSuccessLoadingOutbox = theOutbox.SignContract(
                                    server_->m_nymServer);

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
                            OTString strNewAcctID(theNewAccountID);

                            OTLog::vError("ERROR generating inbox ledger in "
                                          "UserCommandProcessor::"
                                          "UserCmdIssueAssetType:\n%"
                                          "s\n",
                                          strNewAcctID.Get());
                        }
                        else if (!bSuccessLoadingOutbox) {
                            OTString strNewAcctID(theNewAccountID);

                            OTLog::vError("ERROR generating outbox ledger in "
                                          "UserCommandProcessor::"
                                          "UserCmdIssueAssetType:\n%"
                                          "s\n",
                                          strNewAcctID.Get());
                        }
                        else {
                            msgOut.m_bSuccess = true; // <==== SUCCESS!!

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
                    }
                    else
                        OTLog::Error(
                            "Failure generating new issuer account in "
                            "UserCommandProcessor::UserCmdIssueAssetType.\n");
                }
                else
                    OTLog::Error(
                        "Failure verifying asset contract in "
                        "UserCommandProcessor::UserCmdIssueAssetType.\n");
            }
            else {
                OTString strAssetUserID(ASSET_USER_ID), strUserID;
                theNym.GetIdentifier(strUserID);
                OTLog::vError(
                    "User ID on this user account (%s) does NOT match User ID "
                    "for public key used in asset contract: %s\n",
                    strUserID.Get(), strAssetUserID.Get());
            }
        }
        else
            OTLog::vError("%s: Failure loading asset contract from client.\n",
                          __FUNCTION__);
        if (pAssetContract && !msgOut.m_bSuccess) // We only clean it up here,
                                                  // if the Server didn't take
                                                  // ownership of it.
        {
            delete pAssetContract;
            pAssetContract = nullptr;
        }
    }

    // Either way, we need to send the user's command back to him as well.
    {
        OTString tempInMessage(MsgIn);
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

    // (You are in UserCmdIssueAssetType.)

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
        const OTString strReplyMessage(msgOut);
        const int64_t lReqNum = atol(MsgIn.m_strRequestNum.Get());
        // If it fails, it logs already.
        DropReplyNoticeToNymbox(SERVER_ID, USER_ID, strReplyMessage, lReqNum,
                                false, // trans success (not a transaction...)
                                &theNym);
    }
}

/// An existing user is creating an issuer account (that he will not control)
/// based on a basket of currencies.
void UserCommandProcessor::UserCmdIssueBasket(OTPseudonym& theNym,
                                              OTMessage& MsgIn,
                                              OTMessage& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "@issueBasket"; // reply to issueBasket
    msgOut.m_strNymID = MsgIn.m_strNymID; // UserID

    // Either way, we need to send the user's command back to him as well.
    {
        OTString tempInMessage(MsgIn);
        msgOut.m_ascInReferenceTo.SetString(tempInMessage);
    }

    const OTIdentifier USER_ID(theNym), SERVER_ID(server_->m_strServerID),
        SERVER_USER_ID(server_->m_nymServer);

    OTString strBasket(MsgIn.m_ascPayload);
    Basket theBasket;

    if (!theBasket.LoadContractFromString(strBasket)) {
        OTLog::vError("%s: Failed trying to load basket from string.\n",
                      __FUNCTION__);
    }
    else if (!theBasket.VerifySignature(theNym)) {
        OTLog::vError("%s: Failed verifying signature on basket.\n",
                      __FUNCTION__);
    }
    else {
        // The basket ID should be the same on all servers.
        // The basket contract ID will be unique on each server.
        //
        // The contract ID of the basket is calculated based on the UNSIGNED
        // portion of the contract
        // (so it is unique on every server) and for the same reason with the
        // AccountID removed before calculating.
        OTIdentifier BASKET_ID, BASKET_ACCOUNT_ID, BASKET_CONTRACT_ID;
        theBasket.CalculateContractID(BASKET_ID);

        // Use BASKET_ID to look up the Basket account and see if it already
        // exists (the server keeps a list.)
        bool bFoundBasket = server_->transactor_.lookupBasketAccountID(
            BASKET_ID, BASKET_ACCOUNT_ID);

        if (bFoundBasket) {
            OTLog::vError("%s: Rejected: user tried to create basket currency "
                          "that already exists.\n",
                          __FUNCTION__);
        }
        else // Basket doesn't already exist -- so perhaps we can create it
               // then.
        {
            // Let's make sure that all the sub-currencies for this basket are
            // available on this server.
            // NOTE: this also prevents someone from using another basket as a
            // sub-currency UNLESS it already
            // exists on this server. (For example, they couldn't use a basket
            // contract from some other
            // server, since it wouldn't be issued here...) Also note that
            // issueAssetType explicitly prevents
            // baskets from being issued -- you HAVE to use issueBasket for
            // creating any basket currency.
            // Taken in tandem, this insures that the only possible way to have
            // a basket currency as a sub-currency
            // is if it's already issued on this server.
            //
            bool bSubCurrenciesAllExist = true;

            for (int32_t i = 0; i < theBasket.Count(); i++) {
                BasketItem* pItem = theBasket.At(i);
                OT_ASSERT(nullptr != pItem);

                if (nullptr ==
                    server_->transactor_.getAssetContract(
                        pItem->SUB_CONTRACT_ID)) // Sub-currency
                                                 // not found.
                {
                    const OTString strSubID(pItem->SUB_CONTRACT_ID);
                    OTLog::vError("%s: Failed: Sub-currency for basket is not "
                                  "issued on this server: %s\n",
                                  __FUNCTION__, strSubID.Get());
                    bSubCurrenciesAllExist = false;
                    break;
                }
            }
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
                SERVER_USER_ID.GetString(MsgIn.m_strNymID);

                // We need to actually create all the sub-accounts.
                // This loop also sets the Account ID onto the basket items
                // (which formerly was blank, from the client.)
                // This loop also adds the BASKET_ID and the NEW ACCOUNT ID to a
                // map on the server for later reference.
                for (int32_t i = 0; i < theBasket.Count(); i++) {
                    BasketItem* pItem = theBasket.At(i);
                    OT_ASSERT(nullptr != pItem);

                    OTAccount* pNewAccount = nullptr;

                    // GenerateNewAccount expects the Asset ID to be in MsgIn.
                    // So we'll just put it there to make things easy...
                    //
                    pItem->SUB_CONTRACT_ID.GetString(MsgIn.m_strAssetID);

                    pNewAccount = OTAccount::GenerateNewAccount(
                        SERVER_USER_ID, SERVER_ID, server_->m_nymServer, MsgIn,
                        OTAccount::basketsub);

                    // If we successfully create the account, then bundle it
                    // in the message XML payload
                    //
                    if (nullptr != pNewAccount) {
                        msgOut.m_bSuccess = true;

                        // Now the item finally has its account ID. Let's grab
                        // it.
                        pNewAccount->GetIdentifier(pItem->SUB_ACCOUNT_ID);

                        delete pNewAccount;
                        pNewAccount = nullptr;
                    }
                    else {
                        OTLog::vError("%s: Failed while calling: "
                                      "OTAccount::GenerateNewAccount(SERVER_"
                                      "USER_ID, SERVER_ID, m_nymServer, "
                                      "MsgIn, OTAccount::basketsub)\n",
                                      __FUNCTION__);
                        msgOut.m_bSuccess = false;
                        break;
                    }
                } // for

                if (true == msgOut.m_bSuccess) {
                    // Generate a new OTAssetContract -- the ID will be a hash
                    // of THAT contract, which includes theBasket as well as
                    // the server's public key as part of its contents.
                    // Therefore, the actual Asset Type ID of the basket
                    // currency
                    // will be different from server to server.
                    //
                    // BUT!! Because we can also generate a hash of
                    // theBasket.m_xmlUnsigned (which is what
                    // Basket::CalculateContractID
                    // does) then we have a way of obtaining a number that will
                    // be the same from server to server, for cross-server
                    // transfers of basket assets.
                    //
                    // The way it will work is, when the cross-server transfer
                    // request is generated, the server will check the asset
                    // contract
                    // for the "from" account and see if it is for a basket
                    // currency. If it is, there will be a function on the
                    // contract
                    // that returns the Basket ID, which can be included in the
                    // message to the target server, which uses the ID to look
                    // for its own basket issuer account for the same basket
                    // asset type. This allows the target server to translate
                    // the
                    // Asset Type ID to its own corresponding ID for the same
                    // basket.
                    theBasket.ReleaseSignatures();
                    theBasket.SignContract(server_->m_nymServer);
                    theBasket.SaveContract();

                    // The basket does not yet exist on this server. Create a
                    // new Asset Contract to support it...

                    // Put the Server's Public Key into the "contract" key field
                    // of the new Asset Contract...
                    // This adds a "contract" key to the asset contract (the
                    // server's public key)
                    // Asset Contracts are verified by a key found internal to
                    // the contract, so it's
                    // necessary to put the key in there so it will verify
                    // later.
                    // This also updates the m_xmlUnsigned contents, signs the
                    // contract, saves it,
                    // and calculates the new ID.
                    OTAssetContract* pBasketContract =
                        new BasketContract(theBasket, server_->m_nymServer);

                    // Grab the new asset ID for the new basket currency
                    pBasketContract->GetIdentifier(BASKET_CONTRACT_ID);
                    OTString STR_BASKET_CONTRACT_ID(BASKET_CONTRACT_ID);

                    // set the new Asset Type ID, aka ContractID, onto the
                    // outgoing message.
                    msgOut.m_strAssetID = STR_BASKET_CONTRACT_ID;

                    // Save the new Asset Contract to disk
                    const OTString strFoldername(OTFolders::Contract().Get()),
                        strFilename(STR_BASKET_CONTRACT_ID.Get());

                    // Save the new basket contract to the contracts folder
                    // (So the users can use it the same as they would use any
                    // other contract.)
                    pBasketContract->SaveContract(strFoldername.Get(),
                                                  strFilename.Get());

                    server_->transactor_.addAssetContract(*pBasketContract);
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
                    // asset type ID and the contract file. Now let's create
                    // the issuer account the same as we would for any normal
                    // issuer account.)
                    //
                    // The issuer account is special compared to a normal issuer
                    // account, because within its walls, it must store an
                    // OTAccount for EACH sub-contract, in order to store the
                    // reserves. That's what makes the basket work.

                    OTAccount* pBasketAccount = nullptr;

                    // GenerateNewAccount expects the Asset ID to be in MsgIn.
                    // So we'll just put it there to make things easy...
                    MsgIn.m_strAssetID = STR_BASKET_CONTRACT_ID;

                    pBasketAccount = OTAccount::GenerateNewAccount(
                        SERVER_USER_ID, SERVER_ID, server_->m_nymServer, MsgIn,
                        OTAccount::basket);

                    if (nullptr != pBasketAccount) {
                        msgOut.m_bSuccess = true;

                        pBasketAccount->GetIdentifier(
                            msgOut.m_strAcctID); // string
                        pBasketAccount->GetAssetTypeID().GetString(
                            msgOut.m_strAssetID);

                        pBasketAccount->GetIdentifier(BASKET_ACCOUNT_ID); // id

                        // So the server can later use the BASKET_ID (which is
                        // universal)
                        // to lookup the account ID on this server corresponding
                        // to that basket.
                        // (The account ID will be different from server to
                        // server, thus the need
                        // to be able to look it up via the basket ID.)
                        server_->transactor_.addBasketAccountID(
                            BASKET_ID, BASKET_ACCOUNT_ID, BASKET_CONTRACT_ID);

                        server_->mainFile_.SaveMainFile(); // So the main xml
                                                           // file loads
                                                           // this
                        // basket info next time we run.

                        delete pBasketAccount;
                        pBasketAccount = nullptr;
                    }
                    else {
                        msgOut.m_bSuccess = false;
                    }

                } // if true == msgOut.m_bSuccess
            }     // Subcurrencies all do exist.
        }         // basket doesn't already exist (creating it)
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
void UserCommandProcessor::UserCmdCreateAccount(OTPseudonym& theNym,
                                                OTMessage& MsgIn,
                                                OTMessage& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "@createAccount"; // reply to createAccount
    msgOut.m_strNymID = MsgIn.m_strNymID;   // UserID

    // Either way, we need to send the user's command back to him as well.
    OTString tempInMessage(MsgIn);
    msgOut.m_ascInReferenceTo.SetString(tempInMessage);

    const OTIdentifier USER_ID(theNym), SERVER_ID(server_->m_strServerID);

    std::unique_ptr<OTAccount> pNewAccount(OTAccount::GenerateNewAccount(
        USER_ID, SERVER_ID, server_->m_nymServer, MsgIn));

    // If we successfully create the account, then bundle it in the message XML
    // payload
    if (nullptr != pNewAccount) {
        const char* szFunc = "UserCommandProcessor::UserCmdCreateAccount";
        OTAssetContract* pContract = server_->transactor_.getAssetContract(
            pNewAccount->GetAssetTypeID());

        if (nullptr == pContract) {
            const OTString strAssetID(pNewAccount->GetAssetTypeID());
            OTLog::vError(
                "%s: Error: Unable to get AssetContract for asset type: %s\n",
                szFunc, strAssetID.Get());
        }
        else if (pContract->IsShares()) {
            // The asset type keeps a list of all accounts for that type.
            // (For shares, not for currencies.)
            //
            const bool bAdded = pContract->AddAccountRecord(*pNewAccount);
            if (!bAdded) {
                const OTString strAssetID(pNewAccount->GetAssetTypeID());
                OTLog::vError(
                    "%s: ERROR Adding Account Record: %s ... Aborting.\n",
                    __FUNCTION__, strAssetID.Get());
                return; // error
            }
        }
        OTIdentifier theNewAccountID;
        pNewAccount->GetIdentifier(theNewAccountID);

        OTLedger theOutbox(USER_ID, theNewAccountID, SERVER_ID),
            theInbox(USER_ID, theNewAccountID, SERVER_ID);

        bool bSuccessLoadingInbox = theInbox.LoadInbox();
        bool bSuccessLoadingOutbox = theOutbox.LoadOutbox();

        // ...or generate them otherwise...

        if (true == bSuccessLoadingInbox) // WEIRD IF THIS HAPPENED...
            bSuccessLoadingInbox = theInbox.VerifyAccount(
                server_->m_nymServer); // todo -- this should NEVER happen, the
                                       // ID was
        // just RANDOMLY generated, so HOW did the inbox
        // already exist???
        else {
            bSuccessLoadingInbox = theInbox.GenerateLedger(
                theNewAccountID, SERVER_ID, OTLedger::inbox, true);

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

        if (true == bSuccessLoadingOutbox) // WEIRD IF THIS HAPPENED....
            bSuccessLoadingOutbox = theOutbox.VerifyAccount(
                server_->m_nymServer); // todo -- this should NEVER happen, the
                                       // ID was
        // just RANDOMLY generated, so HOW did the outbox
        // already exist???
        else {
            bSuccessLoadingOutbox = theOutbox.GenerateLedger(
                theNewAccountID, SERVER_ID, OTLedger::outbox, true);

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
            const OTString strNewAcctID(theNewAccountID);

            OTLog::vError("%s: ERROR generating inbox ledger: %s\n", szFunc,
                          strNewAcctID.Get());
        }
        else if (!bSuccessLoadingOutbox) {
            const OTString strNewAcctID(theNewAccountID);

            OTLog::vError("%s: ERROR generating outbox ledger: %s\n", szFunc,
                          strNewAcctID.Get());
        }
        else {
            msgOut.m_bSuccess = true; // <==== SUCCESS!!

            pNewAccount->GetIdentifier(msgOut.m_strAcctID);

            // On the server side, each nym stores a list of its asset accounts
            // (IDs).
            //
            std::set<std::string>& theAccountSet = theNym.GetSetAssetAccounts();
            theAccountSet.insert(msgOut.m_strAcctID.Get());

            theNym.SaveSignedNymfile(server_->m_nymServer);

            OTString tempPayload(*pNewAccount);
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
        const OTString strReplyMessage(msgOut);
        const int64_t lReqNum = atol(MsgIn.m_strRequestNum.Get());

        // If it fails, it logs already.
        DropReplyNoticeToNymbox(
            SERVER_ID, USER_ID, strReplyMessage,
            lReqNum, // No need to update the NymboxHash in this case.
            false);  // trans success (not a transaction)
                     //      DropReplyNoticeToNymbox(SERVER_ID, USER_ID,
                     // strReplyMessage, lReqNum, &theNym);
    }
}

// Deprecated (replaced by UserCmdGetAccountFiles)
void UserCommandProcessor::UserCmdGetAccount(OTPseudonym&, OTMessage& MsgIn,
                                             OTMessage& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "@getAccount";    // reply to getAccount
    msgOut.m_strNymID = MsgIn.m_strNymID;   // UserID
    msgOut.m_strAcctID = MsgIn.m_strAcctID; // The Account ID in question

    const OTIdentifier USER_ID(MsgIn.m_strNymID), ACCOUNT_ID(MsgIn.m_strAcctID),
        SERVER_ID(MsgIn.m_strServerID);

    OTAccount* pAccount = OTAccount::LoadExistingAccount(ACCOUNT_ID, SERVER_ID);
    bool bSuccessLoadingAccount = ((pAccount != nullptr) ? true : false);

    // Yup the account exists. Yup it has the same user ID.
    if (bSuccessLoadingAccount && (pAccount->GetUserID() == USER_ID)) {
        msgOut.m_bSuccess = true;
        // extract the account in ascii-armored form on the outgoing message
        OTString strPayload(
            *pAccount); // first grab it in plaintext string form
        msgOut.m_ascPayload.SetString(strPayload); // now the outgoing message
                                                   // has the account in its
                                                   // payload in base64 form.
    }
    // Send the user's command back to him if failure.
    else {
        msgOut.m_bSuccess = false;
        OTString tempInMessage(
            MsgIn); // Grab the incoming message in plaintext form
        msgOut.m_ascInReferenceTo.SetString(tempInMessage); // Set it into the
                                                            // base64-encoded
                                                            // object on the
                                                            // outgoing message
    }

    // (2) Sign the Message
    msgOut.SignContract(static_cast<const OTPseudonym&>(server_->m_nymServer));

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    //
    // FYI, SaveContract takes m_xmlUnsigned and wraps it with the signatures
    // and ------- BEGIN  bookends
    // If you don't pass a string in, then SaveContract saves the new version to
    // its member, m_strRawFile
    msgOut.SaveContract();
}

void UserCommandProcessor::UserCmdGetAccountFiles(OTPseudonym&,
                                                  OTMessage& MsgIn,
                                                  OTMessage& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "@getAccountFiles"; // reply to getAccountFiles
    msgOut.m_strNymID = MsgIn.m_strNymID;     // UserID
    msgOut.m_strAcctID = MsgIn.m_strAcctID;   // The Account ID in question

    const OTIdentifier USER_ID(MsgIn.m_strNymID), ACCOUNT_ID(MsgIn.m_strAcctID),
        SERVER_ID(MsgIn.m_strServerID);

    OTString strAccount, strInbox, strOutbox, strInboxHash, strOutboxHash;
    OTAccount* pAccount = OTAccount::LoadExistingAccount(ACCOUNT_ID, SERVER_ID);
    bool bSuccessLoadingAccount = ((pAccount != nullptr) ? true : false);
    bool bSuccessLoadingInbox = false;
    bool bSuccessLoadingOutbox = false;
    if (bSuccessLoadingAccount)
        bSuccessLoadingAccount = (pAccount->GetUserID() == USER_ID);
    // Yup the account exists. Yup it has the same user ID.
    if (bSuccessLoadingAccount) {
        // extract the account in ascii-armored form on the outgoing message
        pAccount->SaveContractRaw(
            strAccount); // first grab it in plaintext string form

        // Get the Inbox.
        //
        {
            OTLedger theInbox(USER_ID, ACCOUNT_ID, SERVER_ID);

            bSuccessLoadingInbox = theInbox.LoadInbox();

            if (!bSuccessLoadingInbox)
                OTLog::vError("%s: Failed trying to load Inbox from storage.\n",
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
                    theInbox.LoadedLegacyData()) // (which automatically saves
                                                 // the box receipt as the old
                                                 // data is loaded...)
                {
                    //                  bSuccessLoadingInbox =
                    // theInbox.VerifyAccount(server_->m_nymServer);    // Then
                    // Verify,
                    // which forces a LoadBoxReceipts... (

                    theInbox.ReleaseSignatures(); // UPDATE: We do NOT force the
                                                  // loading here, since they
                                                  // aren't needed.
                    theInbox.SignContract(
                        server_->m_nymServer); // Waste of resources.
                                               // Instead, we recognize
                                               // that it was old data,
                                               // and so
                    theInbox.SaveContract(); // we gracefully re-save in the new
                                             // format, so it won't repeatedly
                                             // be
                    theInbox.SaveInbox(); // loaded over and over again in the
                                          // large filesize.
                }

                if (!bSuccessLoadingInbox)
                    OTLog::vError(
                        "%s: Verification failed on Inbox after loading.\n",
                        __FUNCTION__);
            }
            if (bSuccessLoadingInbox) {
                theInbox.SaveContractRaw(strInbox);

                OTIdentifier theHash;
                if (theInbox.CalculateInboxHash(theHash))
                    theHash.GetString(strInboxHash);
            }
        }
        // Now get the OUTBOX.
        //
        if (bSuccessLoadingInbox) // (Which we don't bother to do unless the
                                  // inbox was already successful.)
        {
            OTLedger theOutbox(USER_ID, ACCOUNT_ID, SERVER_ID);

            bSuccessLoadingOutbox = theOutbox.LoadOutbox();

            if (!bSuccessLoadingOutbox)
                OTLog::vError(
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
                    theOutbox.LoadedLegacyData()) // (which automatically saves
                                                  // the box receipt as the old
                                                  // data is loaded...)
                {
                    //                  bSuccessLoadingOutbox =
                    // theOutbox.VerifyAccount(server_->m_nymServer);    // Then
                    // Verify,
                    // which forces a LoadBoxReceipts... (

                    theOutbox.ReleaseSignatures(); // UPDATE: We do NOT force
                                                   // the loading here, since
                                                   // they aren't needed.
                    theOutbox.SignContract(
                        server_->m_nymServer); // Waste of resources.
                                               // Instead, we
                                               // recognize that it
                                               // was old data, and so
                    theOutbox.SaveContract();  // we gracefully re-save in the
                                               // new format, so it won't
                                               // repeatedly be
                    theOutbox.SaveOutbox(); // loaded over and over again in the
                                            // large filesize.
                }

                if (!bSuccessLoadingOutbox)
                    OTLog::vError(
                        "%s: Verification Failed on Outbox after loading.\n",
                        __FUNCTION__);
            }
            if (bSuccessLoadingOutbox) {
                theOutbox.SaveContractRaw(strOutbox);

                OTIdentifier theHash;
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
        OTString tempInMessage(
            MsgIn); // Grab the incoming message in plaintext form
        msgOut.m_ascInReferenceTo.SetString(tempInMessage); // Set it into the
                                                            // base64-encoded
                                                            // object on the
                                                            // outgoing message
    }
    else                                                  // SUCCESS.
    {
        // Create an OTDB::StringMap object.
        // (To return the three files in.)

        // this asserts already, on failure.
        std::unique_ptr<OTDB::Storable> pStorable(
            OTDB::CreateObject(OTDB::STORED_OBJ_STRING_MAP));
        OTDB::StringMap* pMap = dynamic_cast<OTDB::StringMap*>(pStorable.get());
        // It exists.
        //
        if (nullptr == pMap)
            OTLog::vError(
                "%s: Error: failed trying to create a STORED_OBJ_STRING_MAP.\n",
                __FUNCTION__);
        else {
            auto& theMap = pMap->the_map;
            theMap.insert(std::pair<std::string, std::string>(
                "account", strAccount.Get()));
            theMap.insert(
                std::pair<std::string, std::string>("inbox", strInbox.Get()));
            theMap.insert(
                std::pair<std::string, std::string>("outbox", strOutbox.Get()));
            // Serialize the StringMap to a string...
            //
            std::string str_Encoded = OTDB::EncodeObject(*pMap);
            const bool bSuccessEncoding = (str_Encoded.size() > 0);

            if (!bSuccessEncoding)
                OTLog::vError("%s: Error: failed trying to encode a "
                              "STORED_OBJ_STRING_MAP.\n",
                              __FUNCTION__);
            else {
                msgOut.m_ascPayload.Set(str_Encoded.c_str()); // <============
                msgOut.m_strInboxHash = strInboxHash;
                msgOut.m_strOutboxHash = strOutboxHash;
                msgOut.m_bSuccess = true;
            }
        }
    }
    // (2) Sign the Message
    msgOut.SignContract(static_cast<const OTPseudonym&>(server_->m_nymServer));

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    //
    // FYI, SaveContract takes m_xmlUnsigned and wraps it with the signatures
    // and ------- BEGIN  bookends
    // If you don't pass a string in, then SaveContract saves the new version to
    // its member, m_strRawFile
    msgOut.SaveContract();
}

// Deprecated (replaced by UserCmdGetAccountFiles)
void UserCommandProcessor::UserCmdGetInbox(OTPseudonym&, OTMessage& MsgIn,
                                           OTMessage& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "@getInbox";      // reply to getInbox
    msgOut.m_strNymID = MsgIn.m_strNymID;   // UserID
    msgOut.m_strAcctID = MsgIn.m_strAcctID; // The Account ID in question

    const OTIdentifier USER_ID(MsgIn.m_strNymID), ACCOUNT_ID(MsgIn.m_strAcctID),
        SERVER_ID(MsgIn.m_strServerID);

    OTLedger theLedger(USER_ID, ACCOUNT_ID, SERVER_ID);

    msgOut.m_bSuccess = theLedger.LoadInbox();

    if (!msgOut.m_bSuccess)
        OTLog::vError("%s: Failed trying to load Inbox from storage.\n",
                      __FUNCTION__);
    else {
        // We do NOT call VerifyAccount in this function (because we don't need
        // to) and thus we do NOT
        // force the box receipts to be loaded here (which happens inside that
        // call.) But we DO verify
        // the IDs and the Signature, of course.
        //
        msgOut.m_bSuccess = (theLedger.VerifyContractID() &&
                             theLedger.VerifySignature(server_->m_nymServer));

        // If we loaded old data in this file... (when whole receipts were
        // stored in boxes.)
        //
        if (msgOut.m_bSuccess &&
            theLedger.LoadedLegacyData()) // (which automatically saves the box
                                          // receipt as the old data is
                                          // loaded...)
        {
            //            msgOut.m_bSuccess =
            // theLedger.VerifyAccount(server_->m_nymServer);    // Then Verify,
            // which
            // forces a LoadBoxReceipts... (

            theLedger.ReleaseSignatures(); // UPDATE: We do NOT force the
                                           // loading here, since they aren't
                                           // needed.
            theLedger.SignContract(
                server_->m_nymServer); // Waste of resources. Instead,
                                       // we recognize that it was old
                                       // data, and so
            theLedger.SaveContract();  // we gracefully re-save in the new
                                       // format, so it won't repeatedly be
            theLedger.SaveInbox(); // loaded over and over again in the large
                                   // filesize.
        }

        if (!msgOut.m_bSuccess)
            OTLog::vError("%s: Verification failed on Inbox after loading.\n",
                          __FUNCTION__);
    }

    if (msgOut.m_bSuccess) {
        // extract the ledger in ascii-armored form on the outgoing message
        OTString strPayload(
            theLedger); // first grab it in plaintext string form
        msgOut.m_ascPayload.SetString(strPayload); // now the outgoing message
                                                   // has the inbox ledger in
                                                   // its payload in base64
                                                   // form.

        OTIdentifier theHash;
        if (theLedger.CalculateInboxHash(theHash))
            theHash.GetString(msgOut.m_strInboxHash);
    }
    // Send the user's command back to him if failure.
    else {
        OTString tempInMessage(
            MsgIn); // Grab the incoming message in plaintext form
        msgOut.m_ascInReferenceTo.SetString(tempInMessage); // Set it into the
                                                            // base64-encoded
                                                            // object on the
                                                            // outgoing message
    }

    // (2) Sign the Message
    msgOut.SignContract(static_cast<const OTPseudonym&>(server_->m_nymServer));

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    //
    // FYI, SaveContract takes m_xmlUnsigned and wraps it with the signatures
    // and ------- BEGIN  bookends
    // If you don't pass a string in, then SaveContract saves the new version to
    // its member, m_strRawFile
    msgOut.SaveContract();
}

// Deprecated (replaced by UserCmdGetAccountFiles)
void UserCommandProcessor::UserCmdGetOutbox(OTPseudonym&, OTMessage& MsgIn,
                                            OTMessage& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "@getOutbox";     // reply to getOutbox
    msgOut.m_strNymID = MsgIn.m_strNymID;   // UserID
    msgOut.m_strAcctID = MsgIn.m_strAcctID; // The Account ID in question

    const OTIdentifier USER_ID(MsgIn.m_strNymID), ACCOUNT_ID(MsgIn.m_strAcctID),
        SERVER_ID(MsgIn.m_strServerID);

    OTLedger theLedger(USER_ID, ACCOUNT_ID, SERVER_ID);

    msgOut.m_bSuccess = theLedger.LoadOutbox();

    if (!msgOut.m_bSuccess)
        OTLog::vError("%s: Failed trying to load Outbox from storage.\n",
                      __FUNCTION__);
    else {
        // We do NOT call VerifyAccount in this function (because we don't need
        // to) and thus we do NOT
        // force the box receipts to be loaded here (which happens inside that
        // call.) But we DO verify
        // the IDs and the Signature, of course.
        //
        msgOut.m_bSuccess = (theLedger.VerifyContractID() &&
                             theLedger.VerifySignature(server_->m_nymServer));

        // If we loaded old data in this file... (when whole receipts were
        // stored in boxes.)
        //
        if (msgOut.m_bSuccess &&
            theLedger.LoadedLegacyData()) // (which automatically saves the box
                                          // receipt as the old data is
                                          // loaded...)
        {
            theLedger.ReleaseSignatures(); // UPDATE: We do NOT force the
                                           // loading here, since they aren't
                                           // needed.
            theLedger.SignContract(
                server_->m_nymServer); // Waste of resources. Instead,
                                       // we recognize that it was old
                                       // data, and so
            theLedger.SaveContract();  // we gracefully re-save in the new
                                       // format, so it won't repeatedly be
            theLedger.SaveOutbox(); // loaded over and over again in the large
                                    // filesize.
        }

        if (!msgOut.m_bSuccess)
            OTLog::vError("%s: Verification Failed on Outbox after loading.\n",
                          __FUNCTION__);
    }

    if (msgOut.m_bSuccess) {
        // extract the ledger in ascii-armored form on the outgoing message
        OTString strPayload(
            theLedger); // first grab it in plaintext string form
        msgOut.m_ascPayload.SetString(strPayload); // now the outgoing message
                                                   // has the outbox ledger in
                                                   // its payload in base64
                                                   // form.

        OTIdentifier theHash;
        if (theLedger.CalculateOutboxHash(theHash))
            theHash.GetString(msgOut.m_strOutboxHash);
    }
    // Send the user's command back to him if failure.
    else {
        OTString tempInMessage(
            MsgIn); // Grab the incoming message in plaintext form
        msgOut.m_ascInReferenceTo.SetString(tempInMessage); // Set it into the
                                                            // base64-encoded
                                                            // object on the
                                                            // outgoing message
    }

    // (2) Sign the Message
    msgOut.SignContract(static_cast<const OTPseudonym&>(server_->m_nymServer));

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    //
    // FYI, SaveContract takes m_xmlUnsigned and wraps it with the signatures
    // and ------- BEGIN  bookends
    // If you don't pass a string in, then SaveContract saves the new version to
    // its member, m_strRawFile
    msgOut.SaveContract();
}

void UserCommandProcessor::UserCmdQueryAssetTypes(OTPseudonym&,
                                                  OTMessage& MsgIn,
                                                  OTMessage& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "@queryAssetTypes"; // reply to queryAssetTypes
    msgOut.m_strNymID = MsgIn.m_strNymID;     // UserID
    msgOut.m_bSuccess = false;

    // Send the user's command back to him whether success or failure.
    OTString tempInMessage(
        MsgIn); // Grab the incoming message in plaintext form
    msgOut.m_ascInReferenceTo.SetString(tempInMessage); // Set it into the
                                                        // base64-encoded object
                                                        // on the outgoing
                                                        // message

    if (MsgIn.m_ascPayload.Exists()) // (which it should)
    {
        std::unique_ptr<OTDB::Storable> pStorable(OTDB::DecodeObject(
            OTDB::STORED_OBJ_STRING_MAP, MsgIn.m_ascPayload.Get()));
        OTDB::StringMap* pMap = dynamic_cast<OTDB::StringMap*>(pStorable.get());

        if (nullptr != pMap) // There was definitely a StringMap in the payload.
        {
            msgOut.m_bSuccess = true;

            std::map<std::string, std::string>& theMap = pMap->the_map;
            std::map<std::string, std::string> theNewMap;

            for (auto& it : theMap) {
                const std::string& str1 =
                    it.first; // Containing the asset type ID.
                const std::string& str2 =
                    it.second; // Containing the phrase "exists". (More are
                               // possible in the future.)

                // todo security: limit on length of this map? (sent through
                // user message...)

                // "exists" means, "Here's an asset ID. Please tell me
                // whether or not it's actually issued on this server."
                // Future options might include "issue", "audit", "contract",
                // etc.
                //
                if ((str1.size() > 0) &&
                    (str2.compare("exists") == 0)) // todo hardcoding
                {
                    const OTIdentifier theAssetID(str1.c_str());
                    OTAssetContract* pAssetContract =
                        server_->transactor_.getAssetContract(theAssetID);
                    if (nullptr != pAssetContract) // Yes, it exists.
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
                    str_Encoded.c_str(); // now the outgoing message has the
                                         // response map in its payload, in
                                         // base64 form.
            else
                msgOut.m_bSuccess = false; // Something went wrong.
        }                                  // if pMap exists.
    }
    else {
        msgOut.m_bSuccess = false;
    }

    // (2) Sign the Message
    msgOut.SignContract(static_cast<const OTPseudonym&>(server_->m_nymServer));

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    //
    // FYI, SaveContract takes m_xmlUnsigned and wraps it with the signatures
    // and ------- BEGIN  bookends
    // If you don't pass a string in, then SaveContract saves the new version to
    // its member, m_strRawFile
    msgOut.SaveContract();
}

void UserCommandProcessor::UserCmdGetContract(OTMessage& MsgIn,
                                              OTMessage& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "@getContract";     // reply to getContract
    msgOut.m_strNymID = MsgIn.m_strNymID;     // UserID
    msgOut.m_strAssetID = MsgIn.m_strAssetID; // The Asset Type ID in question

    const OTIdentifier ASSET_TYPE_ID(MsgIn.m_strAssetID);

    OTAssetContract* pContract =
        server_->transactor_.getAssetContract(ASSET_TYPE_ID);

    bool bSuccessLoadingContract = ((pContract != nullptr) ? true : false);

    // Yup the asset contract exists.
    if (bSuccessLoadingContract) {
        msgOut.m_bSuccess = true;
        // extract the account in ascii-armored form on the outgoing message
        OTString strPayload(
            *pContract); // first grab it in plaintext string form
        msgOut.m_ascPayload.SetString(strPayload); // now the outgoing message
                                                   // has the contract in its
                                                   // payload in base64 form.
    }
    // Send the user's command back to him if failure.
    else {
        msgOut.m_bSuccess = false;
        OTString tempInMessage(
            MsgIn); // Grab the incoming message in plaintext form
        msgOut.m_ascInReferenceTo.SetString(tempInMessage); // Set it into the
                                                            // base64-encoded
                                                            // object on the
                                                            // outgoing message
    }

    // (2) Sign the Message
    msgOut.SignContract(static_cast<const OTPseudonym&>(server_->m_nymServer));

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    //
    // FYI, SaveContract takes m_xmlUnsigned and wraps it with the signatures
    // and ------- BEGIN  bookends
    // If you don't pass a string in, then SaveContract saves the new version to
    // its member, m_strRawFile
    msgOut.SaveContract();
}

void UserCommandProcessor::UserCmdTriggerClause(OTPseudonym& theNym,
                                                OTMessage& MsgIn,
                                                OTMessage& msgOut)
{
    OTString strInReferenceTo(
        MsgIn); // Grab the incoming message in plaintext form
    msgOut.m_ascInReferenceTo.SetString(strInReferenceTo);
    // (1) set up member variables
    msgOut.m_strCommand = "@triggerClause"; // reply to triggerClause
    msgOut.m_strNymID = MsgIn.m_strNymID;   // UserID
    msgOut.m_bSuccess = false;              // Default value.
    const OTIdentifier SERVER_ID(server_->m_strServerID),
        theMsgNymboxHash(MsgIn.m_strNymboxHash); // theMsgNymboxHash is the hash
                                                 // sent by the client side
    OTIdentifier theSrvrNymboxHash;

    bool bGotNymboxHashServerSide =
        theNym.GetNymboxHashServerSide(SERVER_ID, theSrvrNymboxHash);
    const bool bGotNymboxHashClientSide = MsgIn.m_strNymboxHash.Exists();

    if (bGotNymboxHashServerSide) // theSrvrNymboxHash is the hash stored on the
                                  // server side
        theSrvrNymboxHash.GetString(msgOut.m_strNymboxHash);
    if ((bGotNymboxHashServerSide && bGotNymboxHashClientSide &&
         (theMsgNymboxHash != theSrvrNymboxHash)) ||
        (bGotNymboxHashServerSide && !bGotNymboxHashClientSide)) {
        OTLog::vOutput(0,
                       "%s: Rejecting message since nymbox hash doesn't match. "
                       "(Send a getNymbox message to grab the newest one.)\n",
                       __FUNCTION__);
    }
    else {
        OTSmartContract* pSmartContract = nullptr;
        OTCronItem* pCronItem =
            server_->m_Cron.GetItemByValidOpeningNum(MsgIn.m_lTransactionNum);

        if (nullptr == pCronItem) {
            OTLog::vOutput(0, "%s: Couldn't find smart contract based on "
                              "transaction #: %lld \n",
                           __FUNCTION__, MsgIn.m_lTransactionNum);
        }
        // Also: CAN this guy trigger it?
        else if (nullptr ==
                 (pSmartContract = dynamic_cast<OTSmartContract*>(pCronItem))) {
            OTLog::vOutput(
                0, "%s: Found cron item %lld based on %lld, but it wasn't a "
                   "smart contract. \n",
                __FUNCTION__, pCronItem->GetTransactionNum(),
                MsgIn.m_lTransactionNum);
        }
        else {
            // FIND THE PARTY / PARTY NAME
            OTAgent* pAgent = nullptr;
            OTParty* pParty =
                pSmartContract->FindPartyBasedOnNymAsAgent(theNym, &pAgent);

            if (nullptr == pParty) {
                OTLog::vOutput(0, "%s: Unable to find party to this contract "
                                  "(%lld based on %lld) "
                                  "based on Nym as agent: %s",
                               __FUNCTION__, pCronItem->GetTransactionNum(),
                               MsgIn.m_lTransactionNum, MsgIn.m_strNymID.Get());
            }
            else {
                bool bSuccess = false;
                const std::string str_clause_name = MsgIn.m_strNymID2.Get();

                if (pSmartContract->CanExecuteClause(pParty->GetPartyName(),
                                                     str_clause_name)) {
                    // Execute the clause.
                    //
                    mapOfClauses theMatchingClauses;
                    OTClause* pClause =
                        pSmartContract->GetClause(str_clause_name);

                    if (nullptr != pClause) {
                        OTLog::vOutput(0, "%s: At party request, processing "
                                          "smart contract clause: %s \n",
                                       __FUNCTION__, str_clause_name.c_str());

                        theMatchingClauses.insert(
                            std::pair<std::string, OTClause*>(str_clause_name,
                                                              pClause));

                        pSmartContract->ExecuteClauses(theMatchingClauses);

                        if (pSmartContract->IsFlaggedForRemoval()) {
                            OTLog::vOutput(0, "%s: Removing smart contract "
                                              "from cron processing: %lld\n",
                                           __FUNCTION__,
                                           pSmartContract->GetTransactionNum());
                        }
                        bSuccess = true;
                    }
                    else {
                        OTLog::vOutput(0, "%s: Failed attempt to process "
                                          "clause (%s) on smart contract: %lld "
                                          "\n",
                                       __FUNCTION__, str_clause_name.c_str(),
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
                    OTLog::vOutput(
                        0,
                        "%s: Party (%s) successfully triggered clause: %s.\n",
                        __FUNCTION__, pParty->GetPartyName().c_str(),
                        str_clause_name.c_str());

                    msgOut.m_bSuccess = true;
                }
                else
                    OTLog::vOutput(0, "%s:  Unable to trigger clause %s at "
                                      "request of party %s. "
                                      "(Either the permission wasn't there, or "
                                      "the clause wasn't found.)\n",
                                   __FUNCTION__, str_clause_name.c_str(),
                                   pParty->GetPartyName().c_str());
            }
        } // else found smart contract.
    }     // NymboxHash matches.
    bGotNymboxHashServerSide =
        theNym.GetNymboxHashServerSide(SERVER_ID, theSrvrNymboxHash);

    if (bGotNymboxHashServerSide) // theSrvrNymboxHash is the hash stored on the
                                  // server side
        theSrvrNymboxHash.GetString(msgOut.m_strNymboxHash);
    // (2) Sign the Message
    msgOut.SignContract(static_cast<const OTPseudonym&>(server_->m_nymServer));

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    //
    // FYI, SaveContract takes m_xmlUnsigned and wraps it with the signatures
    // and ------- BEGIN  bookends
    // If you don't pass a string in, then SaveContract saves the new version to
    // its member, m_strRawFile
    msgOut.SaveContract();
}

void UserCommandProcessor::UserCmdGetMint(OTPseudonym&, OTMessage& MsgIn,
                                          OTMessage& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "@getMint";         // reply to getMint
    msgOut.m_strNymID = MsgIn.m_strNymID;     // UserID
    msgOut.m_strAssetID = MsgIn.m_strAssetID; // The Asset Type ID in question

    const OTIdentifier ASSET_TYPE_ID(MsgIn.m_strAssetID);
    const OTString ASSET_ID_STR(ASSET_TYPE_ID);
    bool bSuccessLoadingMint = false;

    std::unique_ptr<Mint> pMint(
        Mint::MintFactory(server_->m_strServerID, ASSET_ID_STR));
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
            OTString strPayload(
                *pMint); // first grab it in plaintext string form
            msgOut.m_ascPayload.SetString(strPayload); // now the outgoing
                                                       // message has the inbox
                                                       // ledger in its payload
                                                       // in base64 form.
        }
        // Send the user's command back to him if failure.
    }

    if (!bSuccessLoadingMint) {
        msgOut.m_bSuccess = false;
        OTString tempInMessage(
            MsgIn); // Grab the incoming message in plaintext form
        msgOut.m_ascInReferenceTo.SetString(tempInMessage); // Set it into the
                                                            // base64-encoded
                                                            // object on the
                                                            // outgoing message
    }

    // (2) Sign the Message
    msgOut.SignContract(static_cast<const OTPseudonym&>(server_->m_nymServer));

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
void UserCommandProcessor::UserCmdDeleteUser(OTPseudonym& theNym,
                                             OTMessage& MsgIn,
                                             OTMessage& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "@deleteUserAccount"; // reply to deleteUserAccount
    msgOut.m_strNymID = MsgIn.m_strNymID;       // UserID

    const OTIdentifier USER_ID(MsgIn.m_strNymID),
        SERVER_ID(MsgIn.m_strServerID);

    OTLedger theLedger(USER_ID, USER_ID, SERVER_ID);

    std::set<int64_t>& theSetofCronItemIDs = theNym.GetSetOpenCronItems();

    // If success loading Nymbox, and there are transactions still inside, THEN
    // FAIL!!!
    // (Can't delete a Nym with open receipts...)
    //
    const bool bSuccessLoadNymbox =
        (theLedger.LoadNymbox() &&
         theLedger.VerifyAccount(server_->m_nymServer));
    if (!bSuccessLoadNymbox) {
        OTLog::Output(3, "Tried to delete Nym, but failed loading or verifying "
                         "the Nymbox.\n");
        msgOut.m_bSuccess = false;
    }
    else if (theLedger.GetTransactionCount() > 0) {
        OTLog::Output(3, "Tried to delete Nym, but there are still receipts in "
                         "the Nymbox. (Process them first.)\n");
        msgOut.m_bSuccess = false;
    }
    // This Nym still has items open on Cron!
    //
    else if (!theSetofCronItemIDs.empty()) {
        OTLog::Output(3, "Tried to delete Nym, but there are still open Cron "
                         "Items. (Close them first.)\n");
        msgOut.m_bSuccess = false;
    }
    else if (theNym.GetSetAssetAccounts().size() > 0) {
        OTLog::Output(3, "Tried to delete Nym, but there are still Asset "
                         "Accounts open for that Nym. (Close them first.)\n");
        msgOut.m_bSuccess = false;
    }
    // The Nym has used some of his transaction numbers, but hasn't closed them
    // out yet.
    // Close those transactions first.
    else if (theNym.GetTransactionNumCount(SERVER_ID) !=
             theNym.GetIssuedNumCount(SERVER_ID)) {
        OTLog::Output(3, "Tried to delete Nym, but there are still "
                         "transactions open for that Nym. (Close them "
                         "first.)\n");
        msgOut.m_bSuccess = false;
    }
    else // SUCCESS!
    {
        msgOut.m_bSuccess = true;

        // The Nym may have some numbers signed out, but none of them have come
        // through
        // and been "used but not closed" yet. (That is, removed from
        // transaction num list but still
        // on issued num list.) If they had (i.e. if the previous elseif just
        // above had discovered
        // mismatched counts) then we wouldn't be able to delete the Nym until
        // those transactions were
        // closed.
        // Since we know the counts match perfectly, here we remove all the
        // numbers.
        // The client side must know to remove all the numbers as well, when it
        // receives a successful
        // reply that the nym was "deleted."
        //
        while (theNym.GetTransactionNumCount(SERVER_ID) > 0) {
            int64_t lTemp = theNym.GetTransactionNum(SERVER_ID, 0); // index 0
            server_->transactor_.removeTransactionNumber(theNym, lTemp, false);
        }

        while (theNym.GetIssuedNumCount(SERVER_ID) > 0) {
            int64_t lTemp = theNym.GetIssuedNum(SERVER_ID, 0); // index 0
            server_->transactor_.removeIssuedNumber(theNym, lTemp, false);
        }

        theNym.MarkForDeletion(); // The nym isn't actually deleted yet, just
                                  // marked for deletion.
        //  It will get cleaned up later, during server maintenance.

        // SAVE the Nym... (now marked for deletion and with all of its
        // transaction numbers removed.)
        //
        theNym.SaveSignedNymfile(server_->m_nymServer);
    }

    // Send the user's command back to him (success or failure.)
    {
        OTString tempInMessage(
            MsgIn); // Grab the incoming message in plaintext form
        msgOut.m_ascInReferenceTo.SetString(tempInMessage); // Set it into the
                                                            // base64-encoded
                                                            // object on the
                                                            // outgoing message
    }

    // (2) Sign the Message
    msgOut.SignContract(static_cast<const OTPseudonym&>(server_->m_nymServer));

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    //
    // FYI, SaveContract takes m_xmlUnsigned and wraps it with the signatures
    // and ------- BEGIN  bookends
    // If you don't pass a string in, then SaveContract saves the new version to
    // its member, m_strRawFile
    msgOut.SaveContract();

    // (You are in UserCmdDeleteUser.)

    // TODO: We may also need to mark the Nymbox, as well as the credential
    // files, as "Marked For Deletion."

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
        const OTString strReplyMessage(msgOut);
        const int64_t lReqNum = atol(MsgIn.m_strRequestNum.Get());

        // If it fails, it logs already.
        DropReplyNoticeToNymbox(SERVER_ID, USER_ID, strReplyMessage, lReqNum,
                                false, // trans success
                                &theNym);
    }
}

// the "accountID" on this message will contain the NymID if retrieving a
// boxreceipt for
// the Nymbox. Otherwise it will contain an AcctID if retrieving a boxreceipt
// for an Asset Acct.
//
void UserCommandProcessor::UserCmdGetBoxReceipt(OTPseudonym&, OTMessage& MsgIn,
                                                OTMessage& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "@getBoxReceipt"; // reply to getBoxReceipt
    msgOut.m_strNymID = MsgIn.m_strNymID;   // UserID
    msgOut.m_strAcctID = MsgIn.m_strAcctID; // the asset account ID
                                            // (inbox/outbox), or Nym ID
                                            // (nymbox)
    msgOut.m_lTransactionNum =
        MsgIn.m_lTransactionNum; // TransactionNumber for the receipt in the box
                                 // (unique to the box.)
    msgOut.m_lDepth = MsgIn.m_lDepth;
    msgOut.m_bSuccess = false;

    const OTIdentifier USER_ID(MsgIn.m_strNymID),
        SERVER_ID(MsgIn.m_strServerID), ACCOUNT_ID(MsgIn.m_strAcctID);

    OTLedger* pLedger = nullptr;
    std::unique_ptr<OTLedger> theLedgerAngel;

    bool bErrorCondition = false;
    bool bSuccessLoading = false;

    switch (MsgIn.m_lDepth) {
    case 0: // Nymbox
        if (USER_ID == ACCOUNT_ID) {
            pLedger = new OTLedger(USER_ID, USER_ID, SERVER_ID);
            OT_ASSERT(nullptr != pLedger);
            theLedgerAngel.reset(pLedger);
            bSuccessLoading = pLedger->LoadNymbox(); // It's verified using
                                                     // VerifyAccount() below
                                                     // this switch block.
        }
        else                                       // Inbox / Outbox.
        {
            OTLog::vError(
                "UserCommandProcessor::UserCmdGetBoxReceipt: User requested "
                "Nymbox, but "
                "failed to provide the "
                "UserID (%s) in the AccountID (%s) field as expected.\n",
                MsgIn.m_strNymID.Get(), MsgIn.m_strAcctID.Get());
            bErrorCondition = true;
        }
        break;
    case 1: // Inbox
        if (USER_ID == ACCOUNT_ID) {
            OTLog::vError(
                "UserCommandProcessor::UserCmdGetBoxReceipt: User requested "
                "Inbox, but erroneously provided the "
                "UserID (%s) in the AccountID (%s) field.\n",
                MsgIn.m_strNymID.Get(), MsgIn.m_strAcctID.Get());
            bErrorCondition = true;
        }
        else {
            pLedger = new OTLedger(USER_ID, ACCOUNT_ID, SERVER_ID);
            OT_ASSERT(nullptr != pLedger);
            theLedgerAngel.reset(pLedger);
            bSuccessLoading = pLedger->LoadInbox(); // It's verified using
                                                    // VerifyAccount() below
                                                    // this switch block.
        }
        break;
    case 2: // Outbox
        if (USER_ID == ACCOUNT_ID) {
            OTLog::vError(
                "UserCommandProcessor::UserCmdGetBoxReceipt: User requested "
                "Outbox, but erroneously provided the "
                "UserID (%s) in the AccountID (%s) field.\n",
                MsgIn.m_strNymID.Get(), MsgIn.m_strAcctID.Get());
            bErrorCondition = true;
        }
        else {
            pLedger = new OTLedger(USER_ID, ACCOUNT_ID, SERVER_ID);
            OT_ASSERT(nullptr != pLedger);
            theLedgerAngel.reset(pLedger);
            bSuccessLoading = pLedger->LoadOutbox(); // It's verified using
                                                     // VerifyAccount() below
                                                     // this switch block.
        }
        break;
    default:
        OTLog::vError("UserCommandProcessor::UserCmdGetBoxReceipt: Unknown box "
                      "type: %lld\n",
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
        pLedger->VerifyContractID() && // for just one, so we're going to
                                       // VerifyContractID and Signature
                                       // instead. Then below, we'll
        pLedger->VerifySignature(
            server_->m_nymServer) // just load the one we actually need.
        ) {
        OTTransaction* pTransaction =
            pLedger->GetTransaction(MsgIn.m_lTransactionNum);
        if (nullptr == pTransaction) {
            OTLog::vError(
                "UserCommandProcessor::UserCmdGetBoxReceipt: User requested a "
                "transaction number "
                "(%lld) that's not in the %s. UserID (%s) and "
                "AccountID (%s) FYI.\n",
                MsgIn.m_lTransactionNum,
                (MsgIn.m_lDepth == 0)
                    ? "nymbox"
                    : ((MsgIn.m_lDepth == 1) ? "inbox"
                                             : "outbox"), // outbox is 2.
                MsgIn.m_strNymID.Get(),
                MsgIn.m_strAcctID.Get());
        }
        else {
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
                const OTString strBoxReceipt(*pTransaction);
                OT_ASSERT(strBoxReceipt.Exists());

                msgOut.m_ascPayload.SetString(strBoxReceipt);
                msgOut.m_bSuccess = true;

                OTLog::vOutput(
                    3, "UserCommandProcessor::UserCmdGetBoxReceipt: Success: "
                       "User is "
                       "retrieving the box receipt for transaction number "
                       "%lld in the %s for UserID (%s) AccountID (%s).\n",
                    MsgIn.m_lTransactionNum,
                    (MsgIn.m_lDepth == 0)
                        ? "nymbox"
                        : ((MsgIn.m_lDepth == 1) ? "inbox"
                                                 : "outbox"), // outbox is 2.
                    MsgIn.m_strNymID.Get(),
                    MsgIn.m_strAcctID.Get());
            }
            else {
                OTLog::vError(
                    "UserCommandProcessor::UserCmdGetBoxReceipt: User "
                    "requested a "
                    "transaction number (%lld) that's "
                    "failing to retrieve from the %s, AFTER calling "
                    "LoadBoxReceipt(). (Though it worked BEFORE calling it.) "
                    "UserID (%s) and AccountID (%s) FYI. IsAbbreviated == %s\n",
                    MsgIn.m_lTransactionNum,
                    (MsgIn.m_lDepth == 0)
                        ? "nymbox"
                        : ((MsgIn.m_lDepth == 1) ? "inbox"
                                                 : "outbox"), // outbox is 2.
                    MsgIn.m_strNymID.Get(),
                    MsgIn.m_strAcctID.Get(),
                    (nullptr == pTransaction)
                        ? "nullptr"
                        : (pTransaction->IsAbbreviated()) ? "true" : "false");
            }
        }
    }
    else {
        OTLog::vError(
            "UserCommandProcessor::UserCmdGetBoxReceipt: Failed loading or "
            "verifying %s. "
            "Transaction (%lld), UserID (%s) and AccountID (%s) FYI.\n",
            (MsgIn.m_lDepth == 0)
                ? "nymbox"
                : ((MsgIn.m_lDepth == 1) ? "inbox" : "outbox"), // outbox is 2.
            MsgIn.m_lTransactionNum,
            MsgIn.m_strNymID.Get(), MsgIn.m_strAcctID.Get());
    }

    // Send the user's command back to him (success or failure.)
    //  if (!msgOut.m_bSuccess)
    {
        const OTString tempInMessage(
            MsgIn); // Grab the incoming message in plaintext form
        msgOut.m_ascInReferenceTo.SetString(tempInMessage); // Set it into the
                                                            // base64-encoded
                                                            // object on the
                                                            // outgoing message
    }

    // (2) Sign the Message
    msgOut.SignContract(static_cast<const OTPseudonym&>(server_->m_nymServer));

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
void UserCommandProcessor::UserCmdDeleteAssetAcct(OTPseudonym& theNym,
                                                  OTMessage& MsgIn,
                                                  OTMessage& msgOut)
{
    const char* szFunc = "UserCommandProcessor::UserCmdDeleteAssetAcct";

    // (1) set up member variables
    msgOut.m_strCommand = "@deleteAssetAccount"; // reply to deleteAssetAccount
    msgOut.m_strNymID = MsgIn.m_strNymID;        // UserID
    msgOut.m_strAcctID = MsgIn.m_strAcctID; // the asset account being deleted.

    const OTIdentifier USER_ID(MsgIn.m_strNymID),
        SERVER_ID(MsgIn.m_strServerID), ACCOUNT_ID(MsgIn.m_strAcctID);

    std::unique_ptr<OTAccount> pAccount(
        OTAccount::LoadExistingAccount(ACCOUNT_ID, SERVER_ID));

    if (nullptr == pAccount || !pAccount->VerifyAccount(server_->m_nymServer)) {
        OTLog::vError("%s: Error loading or verifying account: %s\n", szFunc,
                      MsgIn.m_strAcctID.Get());
    }
    else if (pAccount->GetBalance() != 0) {
        OTLog::vOutput(1, "%s: Failed while trying to delete asset account %s: "
                          "Balance must be zero to do this!\n",
                       szFunc, MsgIn.m_strAcctID.Get());
    }
    else {
        std::unique_ptr<OTLedger> pInbox(
            pAccount->LoadInbox(server_->m_nymServer));
        std::unique_ptr<OTLedger> pOutbox(
            pAccount->LoadOutbox(server_->m_nymServer));

        if (nullptr == pInbox) {
            OTLog::vError("%s: Error loading or verifying inbox: %s\n", szFunc,
                          MsgIn.m_strAcctID.Get());
        }
        else if (nullptr == pOutbox) {
            OTLog::vError("%s: Error loading or verifying outbox: %s\n", szFunc,
                          MsgIn.m_strAcctID.Get());
        }
        else if (pInbox->GetTransactionCount() > 0) {
            OTLog::vOutput(
                3, "%s: Tried to delete asset account, but there are still "
                   "receipts in the Inbox. (Process them first.)\n",
                szFunc);
            msgOut.m_bSuccess = false;
        }
        else if (pOutbox->GetTransactionCount() > 0) {
            OTLog::vOutput(
                3, "%s: Tried to delete asset account, but there are still "
                   "receipts in the Outbox. (Process them first.)\n",
                szFunc);
            msgOut.m_bSuccess = false;
        }
        else // SUCCESS!
        {
            msgOut.m_bSuccess = true;

            std::set<std::string>& theAccountSet = theNym.GetSetAssetAccounts();
            theAccountSet.erase(MsgIn.m_strAcctID.Get());

            theNym.SaveSignedNymfile(server_->m_nymServer);
            OTAssetContract* pContract = server_->transactor_.getAssetContract(
                pAccount->GetAssetTypeID());

            if (nullptr == pContract) {
                const OTString strAssetID(pAccount->GetAssetTypeID());
                OTLog::vError("%s: Error: Unable to get AssetContract for "
                              "asset type: %s\n",
                              szFunc, strAssetID.Get());
            }
            else if (pContract->IsShares()) {
                // The asset type keeps a list of all accounts for that type.
                // (For shares, not for currencies.)
                //
                const bool bErased =
                    pContract->EraseAccountRecord(pAccount->GetAssetTypeID());
                if (!bErased) {
                    const OTString strAssetID(pAccount->GetAssetTypeID());
                    OTLog::vError(
                        "%s: ERROR Erasing Account Record: %s ... Aborting.\n",
                        __FUNCTION__, strAssetID.Get());
                    return; // error
                }
            }

            pAccount->MarkForDeletion(); // The account isn't actually deleted
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
        OTString tempInMessage(
            MsgIn); // Grab the incoming message in plaintext form
        msgOut.m_ascInReferenceTo.SetString(tempInMessage); // Set it into the
                                                            // base64-encoded
                                                            // object on the
                                                            // outgoing message
    }

    // (2) Sign the Message
    msgOut.SignContract(static_cast<const OTPseudonym&>(server_->m_nymServer));

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
        const OTString strReplyMessage(msgOut);
        const int64_t lReqNum = atol(MsgIn.m_strRequestNum.Get());

        // If it fails, it logs already.
        DropReplyNoticeToNymbox(SERVER_ID, USER_ID, strReplyMessage, lReqNum,
                                false, // trans success (not a transaction.)
                                &theNym);
    }
}

void UserCommandProcessor::UserCmdGetNymbox(OTPseudonym& theNym,
                                            OTMessage& MsgIn, OTMessage& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "@getNymbox";   // reply to getNymbox
    msgOut.m_strNymID = MsgIn.m_strNymID; // UserID

    const OTIdentifier USER_ID(MsgIn.m_strNymID),
        SERVER_ID(MsgIn.m_strServerID);
    OTIdentifier NYMBOX_HASH;
    bool bSavedNymbox = false;

    OTLedger theLedger(USER_ID, USER_ID, SERVER_ID);

    msgOut.m_bSuccess = theLedger.LoadNymbox();

    if (!msgOut.m_bSuccess)
        OTLog::Error("UserCommandProcessor::UserCmdGetNymbox: Failed trying to "
                     "load Nymbox "
                     "from storage.\n");
    else {
        // We do NOT call VerifyAccount in this function (because we don't need
        // to) and thus we do NOT
        // force the box receipts to be loaded here (which happens inside that
        // call.) But we DO verify
        // the IDs and the Signature, of course.
        //
        msgOut.m_bSuccess = (theLedger.VerifyContractID() &&
                             theLedger.VerifySignature(server_->m_nymServer));

        // If we loaded old data in this file... (when whole receipts were
        // stored in boxes.)
        //
        if (msgOut.m_bSuccess &&
            theLedger.LoadedLegacyData()) // (which automatically saves the box
                                          // receipt as the old data is
                                          // loaded...)
        {
            theLedger.ReleaseSignatures(); // UPDATE: We do NOT force the
                                           // loading here, since they aren't
                                           // needed.
            theLedger.SignContract(
                server_->m_nymServer); // Waste of resources. Instead,
                                       // we recognize that it was old
                                       // data, and so
            theLedger.SaveContract();  // we gracefully re-save in the new
                                       // format, so it won't repeatedly be
            theLedger.SaveNymbox(&NYMBOX_HASH); // loaded over and over again in
                                                // the large filesize.

            bSavedNymbox = true;
        }
        if (!msgOut.m_bSuccess)
            OTLog::Error("UserCommandProcessor::UserCmdGetNymbox: Verification "
                         "failed on "
                         "Nymbox after loading.\n");
    }

    if (true == msgOut.m_bSuccess) {
        // extract the ledger in ascii-armored form on the outgoing message
        OTString strPayload(
            theLedger); // first grab it in plaintext string form
        msgOut.m_ascPayload.SetString(strPayload); // now the outgoing message
                                                   // has the nymbox ledger in
                                                   // its payload in base64
                                                   // form.
    }
    // Send the user's command back to him if failure.
    else {
        OTString tempInMessage(
            MsgIn); // Grab the incoming message in plaintext form
        msgOut.m_ascInReferenceTo.SetString(tempInMessage); // Set it into the
                                                            // base64-encoded
                                                            // object on the
                                                            // outgoing message
    }

    if (bSavedNymbox) {
        // Todo: make objects (like nyms) "saveable", and ability to get
        // "dirty". (To prevent multiple
        // redundant saves.)
        theNym.SetNymboxHashServerSide(
            NYMBOX_HASH); // Save the hash onto the Nym
        theNym.SaveSignedNymfile(server_->m_nymServer);

        NYMBOX_HASH.GetString(
            msgOut.m_strNymboxHash); // Get the hash onto the message
    }
    else if (true == msgOut.m_bSuccess) {
        theLedger.CalculateNymboxHash(NYMBOX_HASH);

        theNym.SetNymboxHashServerSide(
            NYMBOX_HASH); // Save the hash onto the Nym
        theNym.SaveSignedNymfile(server_->m_nymServer);

        NYMBOX_HASH.GetString(
            msgOut.m_strNymboxHash); // Get the hash onto the message
    }
    else {
        OTIdentifier EXISTING_NYMBOX_HASH;
        if (theNym.GetNymboxHashServerSide(
                SERVER_ID,
                EXISTING_NYMBOX_HASH)) // if hash exists already for nym...
            EXISTING_NYMBOX_HASH.GetString(
                msgOut.m_strNymboxHash); // ...then set it onto the message.
    }

    // (2) Sign the Message
    msgOut.SignContract(static_cast<const OTPseudonym&>(server_->m_nymServer));

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    //
    // FYI, SaveContract takes m_xmlUnsigned and wraps it with the signatures
    // and ------- BEGIN  bookends
    // If you don't pass a string in, then SaveContract saves the new version to
    // its member, m_strRawFile
    msgOut.SaveContract();
}

void UserCommandProcessor::UserCmdProcessNymbox(OTPseudonym& theNym,
                                                OTMessage& MsgIn,
                                                OTMessage& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "@processNymbox"; // reply to processNymbox
    msgOut.m_strNymID = MsgIn.m_strNymID;   // UserID

    const OTIdentifier USER_ID(msgOut.m_strNymID),
        SERVER_ID(server_->m_strServerID), SERVER_USER_ID(server_->m_nymServer);

    OTLedger theLedger(USER_ID, USER_ID, SERVER_ID); // These are ledgers used
                                                     // as messages. The one we
                                                     // received
    // and the one we're sending back.
    std::unique_ptr<OTLedger> pResponseLedger(OTLedger::GenerateLedger(
        SERVER_USER_ID, USER_ID, SERVER_ID, OTLedger::message, false));

    // Grab the string (containing the request ledger) out of ascii-armored
    // form.
    OTString strLedger(MsgIn.m_ascPayload);

    bool bTransSuccess = false;

    const OTIdentifier theMsgNymboxHash(
        MsgIn.m_strNymboxHash); // theMsgNymboxHash is the hash sent by the
                                // client side
    OTIdentifier theSrvrNymboxHash;

    bool bGotNymboxHashServerSide =
        theNym.GetNymboxHashServerSide(SERVER_ID, theSrvrNymboxHash);

    if (bGotNymboxHashServerSide) // theSrvrNymboxHash is the hash stored on the
                                  // server side
        theSrvrNymboxHash.GetString(msgOut.m_strNymboxHash);
    else {
        OTLog::vOutput(
            1, "%s: We cannot obtain server side nymbox hash, will continue.\n",
            __FUNCTION__);
    }

    const bool bGotNymboxHashClientSide = MsgIn.m_strNymboxHash.Exists();

    if (!bGotNymboxHashClientSide) {
        OTLog::vOutput(
            1, "%s: We don't have a client side nymbox hash, will continue\n",
            __FUNCTION__);
    }

    if (bGotNymboxHashServerSide && bGotNymboxHashClientSide)
        if (theMsgNymboxHash != theSrvrNymboxHash) {
            OTLog::vOutput(0, "%s: The server and client nymbox hashes "
                              "missmatch! rejecting message.\n",
                           __FUNCTION__);
            goto send_message;
        }

    // theLedger contains a single transaction from the client, with an item
    // inside
    // for each inbox transaction the client wants to accept or reject.
    // Let's see if we can load it from the string that came in the message...
    //
    msgOut.m_bSuccess = theLedger.LoadContractFromString(strLedger);
    if (msgOut.m_bSuccess) // Yes, that is an assignment operator.
    {
        // In this case we need to process the transaction items from the ledger
        // and create a corresponding transaction where each of the new items
        // contains the answer to the transaction item sent.
        // Then we send that new "response ledger" back to the user in
        // MsgOut.Payload
        // as an @processNymbox message.

        if (theLedger.GetTransactionCount() == 0) {
            OTTransaction* pTranResponse = OTTransaction::GenerateTransaction(
                *pResponseLedger, OTTransaction::error_state, 0);
            pTranResponse->SignContract(server_->m_nymServer);
            pTranResponse->SaveContract();

            // Add the response transaction to the response ledger.
            // That will go into the response message and be sent back to the
            // client.
            pResponseLedger->AddTransaction(*pTranResponse);
        }
        for (auto& it : theLedger.GetTransactionMap()) {
            OTTransaction* pTransaction = it.second;
            OT_ASSERT_MSG(nullptr != pTransaction,
                          "nullptr transaction pointer in "
                          "UserCommandProcessor::UserCmdProcessNymbox\n");

            // for each transaction in the ledger, we create a transaction
            // response and add
            // that to the response ledger.
            OTTransaction* pTranResponse = OTTransaction::GenerateTransaction(
                *pResponseLedger, OTTransaction::error_state,
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
                theNym, *pTransaction, *pTranResponse, bTransSuccess);

            pTranResponse = nullptr; // at this point, the ledger now "owns" the
                                     // response, and will handle deleting it.
        }

        // DONE (Notices go to Nymbox now): should consider saving a copy of the
        // response
        // ledger here on the server.
        // Until the user signs off of the responses, maybe the user didn't
        // receive them.
        // The server should be able to re-send them until confirmation, then
        // delete them.
        // So might want to consider a SAVE TO FILE here of that ledger we're
        // sending out...
    }
    else {
        OTLog::Error("ERROR loading ledger from message in "
                     "UserCommandProcessor::UserCmdProcessNymbox\n");
    }

send_message:

    // sign the ledger
    pResponseLedger->SignContract(server_->m_nymServer);
    pResponseLedger->SaveContract();
    // extract the ledger in ascii-armored form
    OTString strPayload(*pResponseLedger);
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
        OTString tempInMessage(MsgIn);
        msgOut.m_ascInReferenceTo.SetString(tempInMessage);
    }

    // UPDATE NYMBOX HASH IN OUTGOING MESSAGE
    //
    // We already grabbed the server's NymboxHash near the top of this function,
    // and attached it to the outgoing reply message.
    //
    // However, the above block may have CHANGED this hash! Therefore, we grab
    // it
    // AGAIN, just in case. This way a failed message will receive the old hash,
    // and a successful message will receive the new hash.
    //
    bGotNymboxHashServerSide =
        theNym.GetNymboxHashServerSide(SERVER_ID, theSrvrNymboxHash);
    if (bGotNymboxHashServerSide) // theSrvrNymboxHash is the hash stored on the
                                  // server side
        theSrvrNymboxHash.GetString(msgOut.m_strNymboxHash);

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
        const OTString strReplyMessage(msgOut);
        const int64_t lReqNum = atol(MsgIn.m_strRequestNum.Get());

        // If it fails, it logs already.
        DropReplyNoticeToNymbox(
            SERVER_ID, USER_ID, strReplyMessage,
            lReqNum, // (We don't want to update the NymboxHash on the Nym, here
                     // in processNymbox, at least, not at this current point
                     // AFTER the reply message has already been signed.)
            bTransSuccess);
        //      DropReplyNoticeToNymbox(SERVER_ID, USER_ID,
        // strReplyMessage, lReqNum, bTransSuccess, &theNym); // Only pass
        // theNym if you want it to contain the LATEST hash. (Some messages
        // don't.)
    }
}

void UserCommandProcessor::UserCmdProcessInbox(OTPseudonym& theNym,
                                               OTMessage& MsgIn,
                                               OTMessage& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand = "@processInbox";  // reply to processInbox
    msgOut.m_strNymID = MsgIn.m_strNymID;   // UserID
    msgOut.m_strAcctID = MsgIn.m_strAcctID; // The Account ID in question

    const OTIdentifier USER_ID(msgOut.m_strNymID),
        ACCOUNT_ID(MsgIn.m_strAcctID), SERVER_ID(server_->m_strServerID),
        SERVER_USER_ID(server_->m_nymServer);

    OTLedger theLedger(USER_ID, ACCOUNT_ID, SERVER_ID); // These are ledgers
                                                        // used as messages. The
                                                        // one we received,
    // and the one we're sending back.
    std::unique_ptr<OTLedger> pResponseLedger(OTLedger::GenerateLedger(
        SERVER_USER_ID, ACCOUNT_ID, SERVER_ID, OTLedger::message, false));
    OT_ASSERT_MSG(nullptr != pResponseLedger, "UserCommandProcessor::"
                                              "UserCmdProcessInbox: ASSERT: "
                                              "nullptr != pResponseLedger");
    OTTransaction* pTranResponse = nullptr;

    // Grab the string (containing the request ledger) out of ascii-armored
    // form.
    OTString strLedger(MsgIn.m_ascPayload);

    bool bTransSuccess = false;
    const OTIdentifier theMsgNymboxHash(
        MsgIn.m_strNymboxHash); // theMsgNymboxHash is the hash sent by the
                                // client side
    OTIdentifier theSrvrNymboxHash;

    bool bGotNymboxHashServerSide =
        theNym.GetNymboxHashServerSide(SERVER_ID, theSrvrNymboxHash);

    if (bGotNymboxHashServerSide) // theSrvrNymboxHash is the hash stored on the
                                  // server side
        theSrvrNymboxHash.GetString(msgOut.m_strNymboxHash);
    else {
        OTLog::vOutput(
            1, "%s: We cannot obtain server side nymbox hash, will continue.\n",
            __FUNCTION__);
    }

    const bool bGotNymboxHashClientSide = MsgIn.m_strNymboxHash.Exists();

    if (!bGotNymboxHashClientSide) {
        OTLog::vOutput(
            1, "%s: We don't have a client side nymbox hash, will continue\n",
            __FUNCTION__);
    }

    if (bGotNymboxHashServerSide && bGotNymboxHashClientSide)
        if (theMsgNymboxHash != theSrvrNymboxHash) {
            OTLog::vOutput(0, "%s: The server and client nymbox hashes "
                              "missmatch! rejecting message.\n",
                           __FUNCTION__);
            goto send_message;
        }

    // theLedger contains a single transaction from the client, with an item
    // inside
    // for each inbox transaction the client wants to accept or reject.
    // Let's see if we can load it from the string that came in the message...
    msgOut.m_bSuccess = theLedger.LoadContractFromString(strLedger);
    if (msgOut.m_bSuccess) {
        OTAccount theAccount(USER_ID, ACCOUNT_ID, SERVER_ID);

        // Make sure the "from" account even exists...
        if (!theAccount.LoadContract()) {
            const OTString strAcctID(ACCOUNT_ID);
            OTLog::vOutput(0, "UserCommandProcessor::UserCmdProcessInbox: "
                              "Failed loading account: %s\n",
                           strAcctID.Get());
        }
        // Make sure the account isn't marked for deletion.
        else if (theAccount.IsMarkedForDeletion()) {
            OTLog::vOutput(
                0, "UserCommandProcessor::UserCmdProcessInbox: Failed attempt "
                   "to use an Asset account that was marked for "
                   "deletion.\n");
        }
        // Make sure the Account ID loaded from the file matches the one we just
        // set and used as the filename.
        else if (!theAccount.VerifyContractID()) {
            // this should never happen. How did the wrong ID get into the
            // account file, if the right
            // ID is on the filename itself? and vice versa.
            OTLog::Error("Error verifying account ID in "
                         "UserCommandProcessor::UserCmdProcessInbox\n");
        }
        // Make sure the nymID loaded up in the account as its actual owner
        // matches the nym who was
        // passed in to this function requesting a transaction on this
        // account... otherwise any asshole
        // could do transactions on your account, no?
        else if (!theAccount.VerifyOwner(theNym)) {
            OTLog::vOutput(0, "Failed verifying account ownership in "
                              "UserCommandProcessor::UserCmdProcessInbox\n");
        }
        // Make sure I, the server, have signed this file.
        else if (!theAccount.VerifySignature(server_->m_nymServer)) {
            OTLog::Error("Error verifying server signature on account in "
                         "UserCommandProcessor::UserCmdProcessInbox\n");
        }
        // No need to call VerifyAccount() here since the above calls go above
        // and beyond that method.
        else {
            // In this case we need to process the transaction items from the
            // ledger
            // and create a corresponding transaction where each of the new
            // items
            // contains the answer to the transaction item sent.
            // Then we send that new "response ledger" back to the user in
            // MsgOut.Payload
            // as an @processInbox message.

            OTTransaction* pTransaction =
                theLedger.GetTransaction(OTTransaction::processInbox);

            if (nullptr == pTransaction) // I'm assuming there's only one in the
                                         // ledger (for now anyways..)
            {
                OTLog::Error("Expected processInbox transaction in "
                             "UserCommandProcessor::UserCmdProcessInbox\n");
            }
            else {
                const int64_t lTransactionNumber =
                    pTransaction->GetTransactionNum();

                // We create a transaction response and add that to the response
                // ledger...
                //
                pTranResponse = OTTransaction::GenerateTransaction(
                    *pResponseLedger, OTTransaction::error_state,
                    lTransactionNumber);
                OT_ASSERT_MSG(nullptr != pTranResponse,
                              "UserCommandProcessor::UserCmdProcessInbox: "
                              "nullptr != pTranResponse");

                // Add the response transaction to the response ledger.
                // That will go into the response message and be sent back to
                // the client.
                pResponseLedger->AddTransaction(*pTranResponse);

                if (!server_->transactor_.verifyTransactionNumber(
                        theNym, lTransactionNumber)) {
                    // The user may not submit a transaction using a number he's
                    // already used before.
                    OTLog::vOutput(
                        0, "UserCommandProcessor::UserCmdProcessInbox: Error "
                           "verifying transaction num %lld for Nym "
                           "%s\n",
                        lTransactionNumber, msgOut.m_strNymID.Get());
                }

                // The items' acct and server ID were already checked in
                // VerifyContractID() when they were loaded.
                // Now this checks a little deeper, to verify ownership,
                // signatures, and transaction number
                // on each item.  That way those things don't have to be checked
                // for security over and over
                // again in the subsequent calls.
                //
                else if (!pTransaction->VerifyItems(theNym)) {
                    OTLog::Output(
                        0, "Error verifying transaction items "
                           "UserCommandProcessor::UserCmdProcessInbox\n");
                }

                // any other security stuff?
                // Todo do I need to verify the server ID here as well?
                else {
                    // We don't want any transaction number being used twice.
                    // (The number, at this point, is STILL issued to the user,
                    // who is still responsible
                    // for that number and must continue signing for it. All
                    // this means here is that the
                    // user no longer has the number on his AVAILABLE list.
                    // Removal from issued list happens separately.)
                    //
                    if (false ==
                        server_->transactor_.removeTransactionNumber(
                            theNym, lTransactionNumber, true)) {
                        OTLog::Error(
                            "Error removing transaction number (as "
                            "available) from user nym in "
                            "UserCommandProcessor::UserCmdProcessInbox\n");
                    }
                    else {
                        OTLog::Output(
                            2, "UserCmdProcessInbox type: Process Inbox\n");

                        server_->notary_.NotarizeProcessInbox(
                            theNym, theAccount, *pTransaction, *pTranResponse,
                            bTransSuccess);
                        // Where appropriate, remove a transaction number from
                        // my issued list
                        // (the list of numbers I must sign for in every balance
                        // agreement.)

                        if (!server_->transactor_.removeIssuedNumber(
                                theNym, lTransactionNumber, true)) {
                            OTLog::vError("%s: Error removing issued number "
                                          "from user nym.\n",
                                          __FUNCTION__);
                        }
                    }
                }
            }

            // DONE: should consider saving a copy of the response ledger here
            // on the server.
            // Until the user signs off of the responses, maybe the user didn't
            // receive them.
            // The server should be able to re-send them until confirmation,
            // then delete them.
            // So might want to consider a SAVE TO FILE here of that ledger
            // we're sending out...
            // UPDATE this is done now: notices go to the Nymbox.
        }
    }
    else {
        OTLog::Error("ERROR loading ledger from message in "
                     "UserCommandProcessor::UserCmdProcessInbox\n");
    }

send_message:

    if (nullptr == pTranResponse) {
        pTranResponse = OTTransaction::GenerateTransaction(
            *pResponseLedger, OTTransaction::error_state, 0);
        OT_ASSERT_MSG(nullptr != pTranResponse, "UserCommandProcessor::"
                                                "UserCmdProcessInbox 2: "
                                                "nullptr != pTranResponse");

        // Add the response transaction to the response ledger.
        // That will go into the response message and be sent back to the
        // client.
        pResponseLedger->AddTransaction(*pTranResponse);
    }

    // sign the outoing transaction
    OT_ASSERT_MSG(nullptr != pTranResponse, "UserCommandProcessor::"
                                            "UserCmdProcessInbox 3: nullptr != "
                                            "pTranResponse");

    pTranResponse->ReleaseSignatures();
    pTranResponse->SignContract(server_->m_nymServer);
    pTranResponse->SaveContract(); // don't forget to save (to internal raw file
                                   // member)

    // sign the ledger
    pResponseLedger->SignContract(server_->m_nymServer);
    pResponseLedger->SaveContract();
    // extract the ledger in ascii-armored form
    OTString strPayload(*pResponseLedger);
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
        OTString tempInMessage(MsgIn);
        msgOut.m_ascInReferenceTo.SetString(tempInMessage);
    }

    // UPDATE NYMBOX HASH IN OUTGOING MESSAGE
    //
    // We already grabbed the server's NymboxHash near the top of this function,
    // and attached it to the outgoing reply message.
    //
    // However, the above block may have CHANGED this hash! Therefore, we grab
    // it
    // AGAIN, just in case. This way a failed message will receive the old hash,
    // and a successful message will receive the new hash.
    //
    bGotNymboxHashServerSide =
        theNym.GetNymboxHashServerSide(SERVER_ID, theSrvrNymboxHash);
    if (bGotNymboxHashServerSide) // theSrvrNymboxHash is the hash stored on the
                                  // server side
        theSrvrNymboxHash.GetString(msgOut.m_strNymboxHash);

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
    // reply
    // into the Nymbox.  This way we are GUARANTEED that the Nym will receive
    // and process
    // it. (And thus never get out of sync.)
    //
    if (msgOut.m_bSuccess) {
        const OTString strReplyMessage(msgOut);
        const int64_t lReqNum = atol(MsgIn.m_strRequestNum.Get());

        // If it fails, it logs already.
        DropReplyNoticeToNymbox(SERVER_ID, USER_ID, strReplyMessage,
                                lReqNum, // We don't want to update the
                                         // Nym's copy here in
                                // processInbox (I don't think.)
                                bTransSuccess);
    }
}

/// There will be more code here to handle all that. In the meantime, I just
/// send
/// a test response back to make sure the communication works.
///
/// An existing user is sending a list of transactions to be notarized.
void UserCommandProcessor::UserCmdNotarizeTransactions(OTPseudonym& theNym,
                                                       OTMessage& MsgIn,
                                                       OTMessage& msgOut)
{
    // (1) set up member variables
    msgOut.m_strCommand =
        "@notarizeTransactions";            // reply to notarizeTransactions
    msgOut.m_strNymID = MsgIn.m_strNymID;   // UserID
    msgOut.m_strAcctID = MsgIn.m_strAcctID; // The Account ID in question

    const OTIdentifier USER_ID(MsgIn.m_strNymID), ACCOUNT_ID(MsgIn.m_strAcctID),
        SERVER_ID(server_->m_strServerID), SERVER_USER_ID(server_->m_nymServer);

    OTLedger theLedger(USER_ID, ACCOUNT_ID, SERVER_ID); // These are ledgers
                                                        // used as messages. The
                                                        // one we received and
                                                        // the one
    // that we're sending back in response.
    std::unique_ptr<OTLedger> pResponseLedger(OTLedger::GenerateLedger(
        SERVER_USER_ID, ACCOUNT_ID, SERVER_ID, OTLedger::message, false));

    bool bTransSuccess = false; // for the Nymbox notice.
    bool bCancelled = false;    // for "failed" transactions that were actually
                                // successful cancellations.

    int64_t lTransactionNumber = 0, lResponseNumber = 0;
    // Since the one going back (above) is a new ledger, we have to call
    // GenerateLedger.
    // Whereas the ledger we received from the server was generated there, so we
    // don't
    // have to generate it again. We just load it.

    OTString strLedger(MsgIn.m_ascPayload);

    const OTIdentifier theMsgNymboxHash(
        MsgIn.m_strNymboxHash); // theMsgNymboxHash is the hash sent by the
                                // client side
    OTIdentifier theSrvrNymboxHash;

    bool bGotNymboxHashServerSide =
        theNym.GetNymboxHashServerSide(SERVER_ID, theSrvrNymboxHash);

    if (bGotNymboxHashServerSide) // theSrvrNymboxHash is the hash stored on the
                                  // server side
        theSrvrNymboxHash.GetString(msgOut.m_strNymboxHash);
    else {
        OTLog::vOutput(
            1, "%s: We cannot obtain server side nymbox hash, will continue.\n",
            __FUNCTION__);
    }

    const bool bGotNymboxHashClientSide = MsgIn.m_strNymboxHash.Exists();

    if (!bGotNymboxHashClientSide) {
        OTLog::vOutput(
            1, "%s: We don't have a client side nymbox hash, will continue\n",
            __FUNCTION__);
    }

    if (bGotNymboxHashServerSide && bGotNymboxHashClientSide)
        if (theMsgNymboxHash != theSrvrNymboxHash) {
            OTLog::vOutput(0, "%s: The server and client nymbox hashes "
                              "missmatch! rejecting message.\n",
                           __FUNCTION__);
            goto send_message;
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
                OTLog::vError("WARNING: multiple transactions in a single "
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
            // serverID, and Transaction Number.
            OTTransaction* pTranResponse = OTTransaction::GenerateTransaction(
                *pResponseLedger, OTTransaction::error_state,
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
            // Therefore, for the "notarizeTransactions" message, I have decided
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
            server_->notary_.NotarizeTransaction(theNym, *pTransaction,
                                                 *pTranResponse, bTransSuccess);

            if (pTranResponse->IsCancelled()) bCancelled = true;

            lTransactionNumber = pTransaction->GetTransactionNum();
            lResponseNumber = pTranResponse->GetTransactionNum();

            OT_ASSERT_MSG(lTransactionNumber == lResponseNumber,
                          "Transaction number and response number should "
                          "always be the same. (But this time, they weren't.)");

            pTranResponse = nullptr; // at this point, the ledger now "owns" the
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
        OTString strPayload(*pResponseLedger);

        msgOut.m_ascPayload.SetString(strPayload); // now the outgoing message
                                                   // has the response ledger in
                                                   // its payload.
    }
    else {
        OTLog::Error("ERROR loading ledger from message in "
                     "UserCommandProcessor::UserCmdNotarizeTransactions\n");
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
        OTString tempInMessage(MsgIn);
        msgOut.m_ascInReferenceTo.SetString(tempInMessage);
    }

    // UPDATE NYMBOX HASH IN OUTGOING MESSAGE
    //
    // We already grabbed the server's NymboxHash near the top of this function,
    // and attached it to the outgoing reply message.
    //
    // However, the above block may have CHANGED this hash! Therefore, we grab
    // it
    // AGAIN, just in case. This way a failed message will receive the old hash,
    // and a successful message will receive the new hash.
    //
    bGotNymboxHashServerSide =
        theNym.GetNymboxHashServerSide(SERVER_ID, theSrvrNymboxHash);
    if (bGotNymboxHashServerSide) // theSrvrNymboxHash is the hash stored on the
                                  // server side
        theSrvrNymboxHash.GetString(msgOut.m_strNymboxHash);

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

    // (You are in UserCmdNotarizeTransactions.)

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
        const OTString strReplyMessage(msgOut);
        const int64_t lReqNum = atol(MsgIn.m_strRequestNum.Get());

        // If it fails, it logs already.
        //      DropReplyNoticeToNymbox(SERVER_ID, USER_ID,
        // strReplyMessage, lReqNum, bTransSuccess, &theNym); // We don't want
        // to update the Nym in this case (I don't think.)
        DropReplyNoticeToNymbox(SERVER_ID, USER_ID, strReplyMessage, lReqNum,
                                bTransSuccess); // trans success
    }
    if (bCancelled) {
        OTLog::vOutput(0, "Success: canceling transaction %lld for nym: %s \n",
                       lTransactionNumber, msgOut.m_strNymID.Get());
    }
    else if (bTransSuccess) {
        OTLog::vOutput(0, "Success: processing transaction %lld for nym: %s \n",
                       lTransactionNumber, msgOut.m_strNymID.Get());
    }
    else {
        OTLog::vOutput(0, "Failure: processing transaction %lld for nym: %s \n",
                       lTransactionNumber, msgOut.m_strNymID.Get());
    }
}

// msg, the request msg from payer, which is attached WHOLE to the Nymbox
// receipt. contains payment already.
// or pass pPayment instead: we will create our own msg here (with payment
// inside) to be attached to the receipt.
bool UserCommandProcessor::SendMessageToNym(
    const OTIdentifier& SERVER_ID, const OTIdentifier& SENDER_USER_ID,
    const OTIdentifier& RECIPIENT_USER_ID,
    OTMessage* pMsg, // the request msg from payer, which is attached
                     // WHOLE to the Nymbox receipt. contains message
                     // already.
    const OTString* pstrMessage) // or pass this instead: we will
                                 // create our own msg here (with
                                 // message inside) to be attached to
                                 // the receipt.
{
    return server_->DropMessageToNymbox(
        SERVER_ID, SENDER_USER_ID, RECIPIENT_USER_ID, OTTransaction::message,
        pMsg, pstrMessage); //, szCommand=nullptr
}

// After EVERY / ANY transaction, plus certain messages, we drop a copy of the
// server's reply into
// the Nymbox.  This way we are GUARANTEED that the Nym will receive and process
// it. (And thus
// never get out of sync.)  This is the function used for doing that.
//
void UserCommandProcessor::DropReplyNoticeToNymbox(
    const OTIdentifier& SERVER_ID, const OTIdentifier& USER_ID,
    const OTString& strMessage, const int64_t& lRequestNum,
    const bool bReplyTransSuccess, OTPseudonym* pActualNym)
{
    OTLedger theNymbox(USER_ID, USER_ID, SERVER_ID);

    bool bSuccessLoadingNymbox = theNymbox.LoadNymbox();

    if (true == bSuccessLoadingNymbox)
        bSuccessLoadingNymbox =
            (theNymbox.VerifyContractID() &&
             theNymbox.VerifySignature(server_->m_nymServer));

    if (!bSuccessLoadingNymbox) {
        const OTString strNymID(USER_ID);
        OTLog::vOutput(0, "OTServer::DropReplyNoticeToNymbox: Failed loading "
                          "or verifying Nymbox for user: %s\n",
                       strNymID.Get());
    }
    else {
        int64_t lReplyNoticeTransNum = 0;
        bool bGotNextTransNum = server_->transactor_.issueNextTransactionNumber(
            server_->m_nymServer, lReplyNoticeTransNum, false);

        if (!bGotNextTransNum) {
            lReplyNoticeTransNum = 0;
            OTLog::Error("OTServer::DropReplyNoticeToNymbox: Error getting "
                         "next transaction number for an "
                         "OTTransaction::replyNotice.\n");
        }
        else { // Drop in the Nymbox
            OTTransaction* pReplyNotice = OTTransaction::GenerateTransaction(
                theNymbox, OTTransaction::replyNotice, lReplyNoticeTransNum);
            OT_ASSERT(nullptr != pReplyNotice);
            OTItem* pReplyNoticeItem = OTItem::CreateItemFromTransaction(
                *pReplyNotice, OTItem::replyNotice);
            OT_ASSERT(nullptr != pReplyNoticeItem);
            pReplyNoticeItem->SetStatus(
                OTItem::acknowledgement); // Nymbox notice is always a success.
                                          // It's just a notice. (The message
                                          // inside it will have success/failure
                                          // also, and any transaction inside
                                          // that will also.)
            pReplyNoticeItem->SetAttachment(
                strMessage); // Purpose of this notice is to carry a copy of
                             // server's reply message (to certain requests,
                             // including all transactions.)
            pReplyNoticeItem->SignContract(server_->m_nymServer);
            pReplyNoticeItem->SaveContract();
            pReplyNotice->AddItem(*pReplyNoticeItem); // the Transaction's
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
            theNymbox.AddTransaction(*pReplyNotice); // Add the replyNotice to
                                                     // the nymbox. It takes
                                                     // ownership.
            theNymbox.ReleaseSignatures();
            theNymbox.SignContract(server_->m_nymServer);
            theNymbox.SaveContract();

            OTIdentifier NYMBOX_HASH;
            theNymbox.SaveNymbox(&NYMBOX_HASH);

            pReplyNotice->SaveBoxReceipt(theNymbox);

            if ((nullptr != pActualNym) && pActualNym->CompareID(USER_ID)) {
                pActualNym->SetNymboxHashServerSide(NYMBOX_HASH);
                pActualNym->SaveSignedNymfile(server_->m_nymServer);
            }
            else if (nullptr != pActualNym)
                OTLog::Error("OTServer::DropReplyNoticeToNymbox: ERROR: "
                             "pActualNym was not nullptr, but it didn't match "
                             "USER_ID.\n");
        }
    }
}

} // namespace opentxs
