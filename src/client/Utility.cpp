// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/client/Utility.hpp"

#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/client/OT_API.hpp"
#include "opentxs/client/OTAPI_Exec.hpp"
#include "opentxs/consensus/ServerContext.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"

#include <ostream>

#define MIN_MESSAGE_LENGTH 10

#define OT_METHOD "opentxs::Utility::"

namespace opentxs
{

bool VerifyMessage(const std::string& message)
{
    if (MIN_MESSAGE_LENGTH > message.length()) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": too short: ")(message).Flush();

        return false;
    }

    return true;
}

NetworkOperationStatus VerifyMessageSuccess(
    const api::client::Manager& api,
    const std::string& message)
{
    if (!VerifyMessage(message)) { return REPLY_NOT_RECEIVED; }

    const auto status = api.Exec().Message_GetSuccess(message);

    switch (status) {
        case REPLY_NOT_RECEIVED: {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Unable to check success status for message: ")(message)
                .Flush();
        } break;
        case MESSAGE_SUCCESS_FALSE: {
            LogDetail(OT_METHOD)(__FUNCTION__)(
                ": Reply received: success == FALSE for message: ")(message)
                .Flush();
        } break;
        case MESSAGE_SUCCESS_TRUE: {
            LogDetail(OT_METHOD)(__FUNCTION__)(
                ": Reply received: success == TRUE. ")
                .Flush();
        } break;
        default: {
            LogNormal(OT_METHOD)(__FUNCTION__)(": Unknown message status: ")(
                status)(" for message: ")(message)
                .Flush();

            return REPLY_NOT_RECEIVED;
        } break;
    }

    return status;
}

std::int32_t VerifyMsgBalanceAgrmntSuccess(
    const api::client::Manager& api,
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& strMessage)  // For when an OTMessage is the input.
{
    if (!VerifyMessage(strMessage)) { return -1; }

    std::int32_t nSuccess = api.Exec().Message_GetBalanceAgreementSuccess(
        NOTARY_ID, NYM_ID, ACCOUNT_ID, strMessage);
    switch (nSuccess) {
        case -1:
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Error calling "
                "OT_API_Msg_GetBlnceAgrmntSuccess, for message: ")(strMessage)(
                ".")
                .Flush();
            break;
        case 0:
            LogDetail(OT_METHOD)(__FUNCTION__)(": Reply received: success == ")(
                "FALSE. Reply message: ")(strMessage)
                .Flush();
            break;
        case 1:
            LogDetail(OT_METHOD)(__FUNCTION__)(": Reply received: success == ")(
                "TRUE.")
                .Flush();
            break;
        default:
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Error. (This should never "
                "happen!) Input: ")(strMessage)(".")
                .Flush();
            nSuccess = -1;
            break;
    }

    return nSuccess;
}

std::int32_t VerifyMsgTrnxSuccess(
    const api::client::Manager& api,
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& strMessage)
{
    if (!VerifyMessage(strMessage)) { return -1; }

    std::int32_t nSuccess = api.Exec().Message_GetTransactionSuccess(
        NOTARY_ID, NYM_ID, ACCOUNT_ID, strMessage);
    switch (nSuccess) {
        case -1:
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Error calling "
                "OT_API_Message_GetSuccess, for message: ")(strMessage)(".")
                .Flush();
            break;
        case 0:
            LogDetail(OT_METHOD)(__FUNCTION__)(
                ": Reply received: success == FALSE. ")("Reply message: ")(
                strMessage)
                .Flush();
            break;
        case 1:
            LogDetail(OT_METHOD)(__FUNCTION__)(
                ": Reply received: success == TRUE.")
                .Flush();
            break;
        default:
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Error. (This should never happen!) "
                "Input: ")(strMessage)(".")
                .Flush();
            nSuccess = -1;
            break;
    }

    return nSuccess;
}

//
// This code was repeating a lot, so I just added a function for it.
//
std::int32_t InterpretTransactionMsgReply(
    const api::client::Manager& api,
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& strAttempt,
    const std::string& strResponse)
{
    std::int32_t nMessageSuccess = VerifyMessageSuccess(api, strResponse);
    if (-1 == nMessageSuccess) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Message error: ")(strAttempt)(".")
            .Flush();
        return -1;
    }
    if (0 == nMessageSuccess) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Server reply (")(strAttempt)(
            "): Message failure.")
            .Flush();

        return 0;
    }

    std::int32_t nBalanceSuccess = VerifyMsgBalanceAgrmntSuccess(
        api, NOTARY_ID, NYM_ID, ACCOUNT_ID, strResponse);
    if (-1 == nBalanceSuccess) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Balance agreement error: ")(
            strAttempt)(".")
            .Flush();
        return -1;
    }
    if (0 == nBalanceSuccess) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Server reply (")(strAttempt)(
            "): Balance agreement failure.")
            .Flush();
        return 0;
    }

    std::int32_t nTransSuccess =
        VerifyMsgTrnxSuccess(api, NOTARY_ID, NYM_ID, ACCOUNT_ID, strResponse);
    if (-1 == nTransSuccess) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Transaction error: ")(strAttempt)(
            ".")
            .Flush();
        return -1;
    }
    if (0 == nTransSuccess) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Server reply (")(strAttempt)(
            "): Transaction failure.")
            .Flush();
        return 0;
    }

    return 1;
}

Utility::Utility(ServerContext& context, const api::client::Manager& api)
    : strLastReplyReceived("")
    , delay_ms(50)
    , max_trans_dl(10)
    , context_(context)
    , api_(api)
{
}

void Utility::delay() const { Log::Sleep(std::chrono::milliseconds(delay_ms)); }

void Utility::longDelay() const
{
    Log::Sleep(std::chrono::milliseconds(delay_ms + 200));
}

std::int32_t Utility::getNbrTransactionCount() const { return max_trans_dl; }

void Utility::setNbrTransactionCount(std::int32_t new_trans_dl)
{
    max_trans_dl = new_trans_dl;
}

std::string Utility::getLastReplyReceived() const
{
    return strLastReplyReceived;
}

void Utility::setLastReplyReceived(const std::string& strReply)
{
    strLastReplyReceived = strReply;
}

std::int32_t Utility::getNymboxLowLevel()
{
    bool bWasSent = false;
    return getNymboxLowLevel(bWasSent);
}

// This returns -1 if error, or a positive request number if it was sent.
// (It cannot return 0;
// Called by getAndProcessNymbox.
std::int32_t Utility::getNymboxLowLevel(bool& bWasSent)
{
    bWasSent = false;
    auto [nRequestNum, transactionNum, result] =
        api_.OTAPI().getNymbox(context_);
    const auto& [status, reply] = result;
    [[maybe_unused]] const auto& notUsed1 = transactionNum;

    switch (status) {
        case SendResult::VALID_REPLY: {
            bWasSent = true;
            setLastReplyReceived(String::Factory(*reply)->Get());

            return nRequestNum;
        } break;
        case SendResult::TIMEOUT: {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to send getNymbox message due to error.")
                .Flush();
            setLastReplyReceived("");

            return -1;
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Error!").Flush();
            setLastReplyReceived("");

            return -1;
        }
    }
}

std::int32_t Utility::getNymbox(
    const std::string& notaryID,
    const std::string& nymID)
{
    bool bForceDownload = false;
    return getNymbox(notaryID, nymID, bForceDownload);
}

std::int32_t Utility::getNymbox(
    const std::string& notaryID,
    const std::string& nymID,
    bool bForceDownload)
{
    std::string strLocation = "Utility::getNymbox";

    std::string strRecentHash = api_.Exec().GetNym_RecentHash(notaryID, nymID);
    bool bRecentHash = VerifyStringVal(strRecentHash);
    if (!bRecentHash) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Warning: Unable to retrieve recent cached "
            "copy  of server-side NymboxHash from "
            "client-side nym (perhaps he's never "
            "downloaded it before?).")
            .Flush();
    }

    std::string strLocalHash = api_.Exec().GetNym_NymboxHash(notaryID, nymID);
    bool bLocalHash = VerifyStringVal(strLocalHash);
    if (!bLocalHash) {
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": Warning: Unable to retrieve client-side ")("NymboxHash for: ")(
            " notaryID: ")(notaryID)(" nymID: ")(nymID)
            .Flush();
    }

    if (!bForceDownload) {
        if (bLocalHash && bRecentHash &&
            (strRecentHash == strLocalHash))  // the hashes match -- no need to
                                              // download anything.
        {
            LogDetail(OT_METHOD)(__FUNCTION__)(
                ": The hashes already match (skipping Nymbox download).")
                .Flush();
            return 1;
        }
    }

    // -- SECTION 1: "GET NYMBOX"
    //
    bool bWasMsgSent = false;
    std::int32_t nGetNymbox = getNymboxLowLevel(bWasMsgSent);

    if (ConnectionState::ACTIVE != api_.ZMQ().Status(notaryID)) return -1;

    if (bWasMsgSent) {
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": FYI: we just sent a getNymboxLowLevel msg. RequestNum: ")(
            nGetNymbox)
            .Flush();
    }

    if (!(bWasMsgSent) || ((nGetNymbox <= 0) && (-1 != nGetNymbox))) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failure: this.getNymboxLowLevel returned unexpected value: ")(
            nGetNymbox)(".")
            .Flush();
        return -1;
    }  // NOTE: for getNymbox, there is no '0' return value;

    if (-1 == nGetNymbox)  // we'll try re-syncing the request number, then try
                           // again.
    {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": FYI: this.getNymboxLowLevel returned -1. (Re-trying...).")
            .Flush();

        const auto nGetRequestNumber = context_.UpdateRequestNumber();

        if (0 >= nGetRequestNumber) {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Failure: this.getNymboxLowLevel failed, "
                "then I tried to resync with "
                "this.getRequestNumber and then that "
                "failed too. (I give up.).")
                .Flush();
            return -1;
        }

        // todo: should the member variable strLastReplyReceived be set to this
        // one?
        // before, the member var was shadowed (string strLastReplyReceived =
        // getLastReplyReceived();).
        std::string lastReplyReceived = getLastReplyReceived();
        // I had to do this bit because getRequestNumber doesn't return the
        // reply itself. But in this case, I needed it.
        if (!VerifyStringVal(lastReplyReceived))  // THIS SHOULD NEVER HAPPEN.
        {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": ERROR in getLastReplyReceived(): Why "
                "was this std::string not set, when "
                "this.getRequestNumber was otherwise an "
                "apparent success?")
                .Flush();
            return -1;  // (SHOULD NEVER HAPPEN. This std::string is set in the
                        // getRequestNumber function.)
        }

        // BY THIS POINT, we have received a server reply:
        // getRequestNumberResponse
        // (Unless it is malformed.) It's definitely not null, nor empty.

        // Grab the NymboxHash on the getRequestNumberResponse reply, and also
        // the one
        // I
        // already had on my client-side Nym... (So we can compare them.)
        //
        //      If the hashes do NOT match, then I DO need to download nymbox
        // and box receipts.
        /*
         *      ===> If the NymboxHash is changed from what I expected, then I
         *need to re-download the
         *      nymbox (and any box receipts I don't already have.)
         *
         *      Then I need to process the Nymbox. But first, see if my missing
         *server reply is in there.
         *      If it is, then I have the server reply! (As if we had succeeded
         *in the first place!!)
         *      Next, process the Nymbox (which processes that reply) and then
         *return strReply;
         *
         *      (Clearly this is just going to be a normal part of the
         *getRequestNumber
         *syncronization.)
         *
         *      By the time that much is done, I will KNOW the request number,
         *the nymbox, the box receipts,
         *      etc are ALL syncronized properly, and that I THEN processed the
         *Nymbox successfully.
         *
         *
         *      NOTICE: In this example I do NOT want to pull out my sent
         *message from the outbuffer (using the request number) and try to
         *harvest all the transaction numbers. Why not? Because possibly
         *the server DID reply! And if I processed that reply properly, it would
         *sync my transaction numbers properly just from that! ===>
         *
         *      ===> Therefore, I need to see FIRST if the old message has a
         *reply WAITING in the Nymbox. THEN
         *      I need to process the Nymbox. ONLY if the reply wasn't there,
         *can I THEN pull out the message from my outbuffer and harvest it.
         *(Which I am reticent to do, until I am SURE the server really
         *never saw that message in the first place.)
         *
         *      However, as long as my NymboxHash hasn't changed, then I'm safe!
         *But if it HAS changed,
         *      then I HAVE to A. download it B. SEE if the reply is there for
         *the request number, then
         *      C. process it. ... If the reply wasn't there, THEN Harvest the
         *transaction #s (for transaction
         *      messages) and then re-try.
         */

        // Grabbing again in case it's changed.
        std::string strServerHash =
            api_.Exec().Message_GetNymboxHash(lastReplyReceived);
        bool bServerHash = VerifyStringVal(strServerHash);
        if (!bServerHash) {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Warning: Unable to retrieve server-side NymboxHash "
                "from server getRequestNumberResponse reply: ")(
                lastReplyReceived)(".")
                .Flush();
        }

        strLocalHash = api_.Exec().GetNym_NymboxHash(notaryID, nymID);
        bLocalHash = VerifyStringVal(strLocalHash);
        if (!bLocalHash) {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Warning(2): Unable to retrieve client-side NymboxHash "
                "for: notaryID: ")(notaryID)(" nymID: ")(nymID)(".")
                .Flush();
        }

        // The hashes don't match -- so let's definitely re-try to download the
        // latest nymbox.
        if (bForceDownload || !bLocalHash || !bServerHash ||
            (bServerHash && bLocalHash && !(strServerHash == strLocalHash))) {
            // the getRequestNumber worked, and the server hashes don't match,
            // so let's try the call again...

            nGetNymbox = getNymboxLowLevel(bWasMsgSent);

            if (!(bWasMsgSent) || ((nGetNymbox <= 0) && (-1 != nGetNymbox))) {
                LogNormal(OT_METHOD)(__FUNCTION__)(
                    ": Failure(2): this.getNymboxLowLevel returned "
                    "unexpected value: ")(nGetNymbox)(".")
                    .Flush();
                return -1;
            }

            if (-1 == nGetNymbox)  // we'll try re-syncing the request number,
                                   // then try again.
            {
                LogNormal(OT_METHOD)(__FUNCTION__)(
                    ": Failure: this.getNymboxLowLevel "
                    "returned -1, even after syncing the "
                    "request number successfully. (Giving "
                    "up).")
                    .Flush();
                return -1;
            }
        }
    }

    // By this point, we DEFINITELY know that the Nymbox was retrieved
    // successfully.
    // (With request number nGetNymbox.) This is because the getNymboxLowLevel()
    // call
    // also tries to receive the reply, so we already know by now whether the
    // reply
    // was successfully received.
    //

    return nGetNymbox;
}

// NEW ONES:

// public static std::int32_t getAndProcessNymbox(String notaryID, String nymID,
// OTBool bWasMsgSent, boolean bForceDownload,
//                                      var nRequestNumber, // nRequestNumber
// refers to a PREVIOUS msg (like a cash withdrawal) that had an error and then
// called this while trying to resync. (The caller will want to know whether it
// was found in the Nymbox.)
//                                      OTBool bFoundNymboxItem,  //
// bFoundNymboxItem is OUTPUT bool, telling caller whether it was found.
//                                      boolean bHarvestingForRetry, //
// bHarvestingForRetry is INPUT, in the case nRequestNumber needs to be
// harvested before a flush occurs.
//                                      boolean bMsgReplySuccess,    //
// bMsgReplySuccess is INPUT, and is in case nRequestNumber needs to be
// HARVESTED before a FLUSH happens.
//                                      boolean bMsgReplyFailure,    //
// bMsgReplyFailure is INPUT, and is in case nRequestNumber needs to be
// HARVESTED before a FLUSH happens.
//                                      boolean bMsgTransSuccess,    //
// bMsgTransSuccess is INPUT, and is in case nRequestNumber needs to be
// HARVESTED before a FLUSH happens.
//                                      boolean bMsgTransFailure)    //
// bMsgTransFailure is INPUT, and is in case nRequestNumber needs to be
// HARVESTED before a FLUSH happens.

// def Utility::getAndProcessNymbox_8(notaryID, nymID, bWasMsgSent, //
// bWasMsgSent is OUTPUT.
//                                   bForceDownload,       // INPUT
//                                   nRequestNumber,       // nRequestNumber
// refers to a PREVIOUS msg (like a cash withdrawal) that had an error and then
// called this while trying to resync. (The caller will want to know whether it
// was found in the Nymbox.)
//                                   bFoundNymboxItem,     // bFoundNymboxItem
// is OUTPUT bool, telling caller whether it was found.
//                                   bHarvestingForRetry,  //
// bHarvestingForRetry is INPUT, in the case nRequestNumber needs to be
// harvested before a flush occurs.
//                                   bMsgFoursome) // OTfourbool class, used to
// reduce the number of parameters here, which I am suspicious that ChaiScript
// cannot handle.

// Returns:
//   -1 ERROR.
//    0 Nymbox was empty -- nothing done. (bWasMsgSent = false)
//    0 Transactionstatus = = server reply received (bWasMsgSent = true), but;
//      the server reply hasstatus = = FAILED. (All harvesting was subsequently
// successful for processNymbox).;
//    1 If the ProcessNymbox Transaction status (from the server reply) is
// SUCCESS,
//      then this function returns 1.
//   >1 If the ProcessNymbox Transaction status (from the server reply) is >1,
// then this function returns the
//      REQUEST NUMBER from when it was originally sent. (Harvesting was NOT
// performed, which is why the request
//      number is being returned, so the caller can choose what to do next.)

// def Utility::getAndProcessNymbox_8(notaryID, nymID, bWasMsgSent,
// bForceDownload, nRequestNumber, bFoundNymboxItem, bHarvestingForRetry,
// bMsgFoursome)
//{
//    cout << "Here's the dead version!\n";
//}

// var nGetAndProcessNymbox = getAndProcessNymbox_8(notaryID, nymID,
// bWasMsgSent, bForceDownload, nRequestNumber, bFoundNymboxItem,
// bHarvestingForRetry, the_foursome);

std::int32_t Utility::getAndProcessNymbox_8(
    const std::string& notaryID,
    const std::string& nymID,
    bool& bWasMsgSent,
    bool bForceDownload,
    std::int32_t nRequestNumber,
    bool& bFoundNymboxItem,
    bool bHarvestingForRetry,
    const OTfourbool& bMsgFoursome)
{
    std::string strLocation = "Utility::getAndProcessNymbox";

    bool bMsgReplySuccess = bMsgFoursome[0];
    bool bMsgReplyFailure = bMsgFoursome[1];
    bool bMsgTransSuccess = bMsgFoursome[2];
    bool bMsgTransFailure = bMsgFoursome[3];

    if (0 > nRequestNumber) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed verifying nRequestNumber as an integer.")
            .Flush();
        return -1;
    }

    if (1 == nRequestNumber) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": WARNING: Request Num of '1' was just passed in here.")
            .Flush();
    }

    bWasMsgSent = false;

    // what is nRequestNumber?
    //
    // Let's say a message, say for a cash withdrawal with request number 5, has
    // FAILED.
    // Since the message failed, perhaps the request number was out of sync, or
    // Nymbox hash
    // was old? So, let's say that it then sent a getRequestNumber message, in
    // order
    // to resync,
    // and discovered that the Nymbox hash has changed. Therefore the Nymbox is
    // now being
    // re-downloaded and processed, so that the cash withdrawal can be attempted
    // again.
    //
    // HOWEVER, before we PROCESS the Nymbox, we need to see if the withdrawal
    // reply is already
    // sitting in it. Why, you ask, if the withdrawal failed, would I expect a
    // reply to be in
    // the Nymbox? In case 1, the message was dropped, so I don't know if the
    // reply is there
    // until I check the Nymbox. In case 2, the message may have failed OR
    // SUCCEEDED, with the
    // successful message containing a FAILED TRANSACTION.
    // Thus, we just want to check the Nymbox for nRequestNumber, and make sure
    // whether it's
    // there or not, before we PROCESS the nymbox, because once we do THAT, it
    // will be empty.
    //
    // We will return a;
    // was already in the Nymbox. We can also harvest the transaction numbers
    // from the reply
    // message, if it's a transaction, so that everything is set for the re-try.
    // (Possibly pass
    // a bool parameter dictating whether the harvesting is being done for a
    // re-try or not.)
    //

    // -- SECTION 1: "GET NYMBOX"
    //
    // This call is sufficiently high-level enough that it already has re-tries
    // built into it. That's why you don't see me re-trying the getNymbox if it
    // fails.
    //

    std::int32_t nGetNymbox = getNymbox(notaryID, nymID, bForceDownload);
    if (nGetNymbox < 1) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failure: this.getNymbox returned: ")(nGetNymbox)(".")
            .Flush();
        return -1;
    }

    // By this point, we DEFINITELY know that the Nymbox was retrieved
    // successfully.
    // (With request number nGetNymbox.) This is because the getNymboxLowLevel()
    // call
    // also tries to receive the reply, so we already know by now whether the
    // reply
    // was successfully received.
    //
    /*
     *  FYI: nRequestNumber is the request number, if >0, for whatever command
     * is causing this getAndProcessNymbox to occur (like a cash withdrawal, or
     * a cheque deposit, etc.) We pass it in here so we can verify whether it's
     *on
     * the Nymbox, before we process it out (so the caller knows whether to
     *clawback.)
     *
     * FYI: nGetNymbox is the request number from getNymboxLowLevel() (above.)
     *We know for a FACT, by this point, this number is >0.
     *
     * FYI: nProcess (just below) is the request number for the PROCESS NYMBOX
     *message.
     * Below the switch block just down there, we know for a fact this number is
     *>0.
     */

    // -- SECTION 2: "SEND PROCESS NYMBOX"
    //

    // Next, we have to make sure we have all the BOX RECEIPTS downloaded
    // for this Nymbox.

    // If the caller wanted to know whether a certain reply (by request number)
    // was in the Nymbox, then bFoundNymboxItem
    // will be set true in this call, if it was there. That way he can Harvest
    // his own msg if he needs to. (Just like I
    // harvest my own processNymbox call below, if necessary.)
    //
    // nBoxType = 0 aka nymbox;
    //

    std::int32_t nBoxType = 0;

    bool bInsured = insureHaveAllBoxReceipts(
        notaryID,
        nymID,
        nymID,
        nBoxType,
        nRequestNumber,
        bFoundNymboxItem);  // ***************************;
    if (bInsured) {
        // If the caller was on about a specific request number...
        //
        if (nRequestNumber > 0) {
            // ...And if we DID NOT find that request number in the Nymbox
            // (along with the server's reply), then harvest it!!
            // (If we HAD found it, then we'd know it didn't NEED harvesting,
            // since the server clearly REPLIED to it already.)

            // FOUND it in the nymbox! Therefore we can remove without
            // harvesting.
            // (Server definitely processed it, so nothing to harvest.)
            //
            if (bFoundNymboxItem) {
                // Notice, if the above call to insureHaveAllBoxReceipts had any
                // network hiccups, then
                // it may have had to get and processNymbox, meaning the below
                // Remove would fail, since
                // the sent message was already removed. Therefore, might want
                // to update this to call getSent
                // FIRST, before trying to remove.
                // (Might want different messages in either case.)
                //
                bool nRemovedMsg = api_.Exec().RemoveSentMessage(
                    std::int64_t(nRequestNumber), notaryID, nymID);
                LogVerbose(OT_METHOD)(__FUNCTION__)(
                    ": (Found server reply in ")(
                    "Nymbox. Removing local sent msg.) Request number: ")(
                    nRemovedMsg)
                    .Flush();
            } else  // We DIDN'T find it in the nymbox, so we can harvest it:
            {
                // NOTE: This may always fail,

                LogDebug(OT_METHOD)(__FUNCTION__)(
                    ": FYI: Calling OT_API_GetSentMessage...")
                    .Flush();

                std::string strSentMsg = api_.Exec().GetSentMessage(
                    std::int64_t(nRequestNumber), notaryID, nymID);

                if (!VerifyStringVal(strSentMsg)) {
                    LogVerbose(OT_METHOD)(__FUNCTION__)(
                        ": (1) OT_API_GetSentMessage returned nullptr ")(
                        "for clawback. (Must have already been cleared. ")(
                        "OT uses deliberate redundancy, though optimizes ")(
                        "this wherever possible.) Request number: ")(
                        nRequestNumber)
                        .Flush();
                } else  // SwigWrap::GetSentMessage success.
                {
                    LogNormal(OT_METHOD)(__FUNCTION__)(
                        ": FYI: Harvesting transaction "
                        "numbers from failed Msg "
                        "attempt...")
                        .Flush();

                    bool nHarvested = api_.Exec().Msg_HarvestTransactionNumbers(
                        strSentMsg,
                        nymID,
                        bHarvestingForRetry,  // bHarvestingForRetry.
                        bMsgReplySuccess,     // bReplyWasSuccess,       //
                                              // RECEIVED
                                              // server reply: explicit success.
                        bMsgReplyFailure,     // bReplyWasFailure,       //
                                              // RECEIVED
                                              // server reply: explicit failure.
                        bMsgTransSuccess,  // bTransactionWasSuccess, // MESSAGE
                                           // success, Transaction success.
                                           // (Explicit.)
                        bMsgTransFailure);  // bTransactionWasFailure  //
                                            // MESSAGE
                                            // success, Transaction failure.
                                            // (Explicit.)
                    LogNormal(OT_METHOD)(__FUNCTION__)(
                        ": OT_API_Msg_HarvestTransactionNumbers: ")(nHarvested)(
                        ".")
                        .Flush();

                    bool nRemovedMsg = api_.Exec().RemoveSentMessage(
                        std::int64_t(nRequestNumber), notaryID, nymID);
                    LogVerbose(OT_METHOD)(__FUNCTION__)(nRemovedMsg).Flush();
                }  // strSentMsg NOT null!
            }
        }

        // (flush): LOOP THROUGH ALL "SENT" messages, and see if ANY of them has
        // a reply
        // sitting in my Nymbox. If so, REMOVE IT from "Sent" queue, (since
        // clearly the server
        // DID respond already.) And if it's NOT in my nymbox, that means I
        // DEFINITELY need to
        // harvest it back since the server definitely rejected it or never
        // received it.
        //
        // The Nym actually SAVES the sent messages PER SERVER, so that this
        // will continue to work in every possible case!!
        // NOTE: Also now storing, on the client nym, a copy of the server's
        // latest nymbox hash
        // for that nym, in addition to the nym's copy (which only updates when
        // he gets his Nymbox.)
        // That way the Nym, even before syncing the nymboxes, and even before
        // sending a new message
        // to find out if they are out of sync, ALREADY KNOWS if they're in sync
        // or not. (That's why
        // all those other server messages send a copy of that hash back, not
        // just the getNymbox msg.)
        //
        //
        //            void api_.Exec().FlushSentMessages(
        //            std::int32_t //
        // bHarvestingForRetry, // bHarvestingForRetry is actually OT_BOOL
        //                              const char * NOTARY_ID,
        //                              const char * NYM_ID,
        //                              const char * THE_NYMBOX);
        // NoVerify means don't load up all the box receipts.
        // Especially in this case--we only care about whether a box receipt is
        // THERE, not
        // what it contains. FlushSentMessages will work just fine WITHOUT
        // loading those
        // box receipts (because the Nymbox contains enough of an abbreviated
        // record already
        // for each one, that we will have the info we need already.)
        //
        std::string strNymbox = api_.Exec().LoadNymboxNoVerify(
            notaryID, nymID);  // FLUSH SENT MESSAGES!!!!  (AND HARVEST.);

        if (VerifyStringVal(strNymbox)) {
            api_.Exec().FlushSentMessages(
                false,  // harvesting for retry = = OT_FALSE. None of the things
                // are being re-tried by the time they are being flushed.
                // They were already old news.;
                notaryID,
                nymID,
                strNymbox);
        }
        // Flushing removes all the messages from the "sent messages" queue,
        // and harvests any transaction numbers to be had. How do I know for
        // sure
        // that I can get away with this? How do I know whether the server has
        // processed those messages or not? How can I harvest them as though
        // they
        // were dropped on the network somewhere? The answer is because I JUST
        // called GetNymbox and downloaded the latest one. I can SEE which
        // replies
        // are in there -- and which ones aren't. I pass that Nymbox into the
        // flush
        // call, so that flush can be careful to remove all sent messages that
        // have
        // nymbox replies, and only harvest the others.
        else {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Error while trying to flush sent messages: Failed "
                "loading Nymbox for nym: ")(nymID)(".")
                .Flush();
        }

        std::int32_t nMsgSentRequestNumOut = -1;
        std::int32_t nReplySuccessOut = -1;
        std::int32_t nBalanceSuccessOut = -1;
        std::int32_t nTransSuccessOut = -1;

        // PROCESS NYMBOX
        //
        // Returns:
        // -1 Error.
        //  0 Nymbox was empty -- nothing done. (bWasMsgSent = false)
        //  0 Transactionstatus = = server reply received (bWasMsgSent = true),;
        //    but the server reply saysstatus = =failed.;
        // >0 If the Transaction status (from the server reply) is SUCCESS, then
        // this function
        //    returns the REQUEST NUMBER from when it was originally sent.

        std::int32_t nProcess = processNymbox(
            notaryID,
            nymID,
            bWasMsgSent,
            nMsgSentRequestNumOut,
            nReplySuccessOut,
            nBalanceSuccessOut,
            nTransSuccessOut);

        if (-1 == nProcess) {
            // Todo: might want to remove the sent message here, IF bMsgWasSent
            // is true.
            // (Just like case 0.)
            //
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Failure: processNymbox: error (-1). (It "
                "couldn't send. I give up.).")
                .Flush();

            return -1;  // (It didn't even send.)
        } else if (0 == nProcess) {
            // Nymbox was empty. (So we didn't send any process message because
            // there was nothing to process.)
            if (!bWasMsgSent) {
                return 0;  // success. done. (box was empty already.)
            }
            // else: the message WAS sent, (the Nymbox was NOT empty)
            //       and then the server replied "success==FALSE"
            //       in its REPLY to that message! Thus we continue and DROP
            // THROUGH...
        } else if (nProcess < 0) {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Failure: processNymbox: unexpected: ")(nProcess)(
                ". (I give up.).")
                .Flush();

            return -1;
        }
        // bWasMsgSent = true;  // unnecessary -- set already by processNymbox
        // call above.

        // By this point, we definitely have a >0 request number from the
        // sendProcessNymbox()
        // call, stored in nProcess (meaning the message WAS sent.) (Except in
        // case of 0, see next line which fixes this:)
        //

        nProcess = nMsgSentRequestNumOut;  // Sometimes this could be 0 still,
                                           // so we fix it here.;
        std::int32_t nReplySuccess = nReplySuccessOut;
        std::int32_t nTransSuccess = nTransSuccessOut;
        std::int32_t nBalanceSuccess = nBalanceSuccessOut;

        /*

        return const;

        char *    SwigWrap::GetSentMessage(const char* REQUEST_NUMBER)
        OT_BOOL   SwigWrap::RemoveSentMessage(const char* REQUEST_NUMBER)

        */
        // All of these booleans (except "error") represent RECEIVED ANSWERS
        // from the server.
        // In other words, "false" does not mean "failed to find message."
        // Rather, it means "DEFINITELY got a reply, and that reply sayssuccess
        // = =false.";

        // SHOULD NEVER HAPPEN (processNymbox call just above was successful,
        // therefore the sent message SHOULD be here in my cache.)
        //
        std::string strReplyProcess = getLastReplyReceived();
        // I had to do this bit because getRequestNumber doesn't return the;
        // reply itself. But in this case, I needed it.
        if (!VerifyStringVal(strReplyProcess))  // THIS SHOULD NEVER HAPPEN.
        {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": ERROR in getLastReplyReceived(): why "
                "was this std::string not set, when "
                "getRequestNumber was otherwise an "
                "apparent success?")
                .Flush();

            return -1;  // (SHOULD NEVER HAPPEN. This std::string is set in the
                        // getRequestNumber function.)
        }

        bool bProcessNymboxReplyError =
            (!VerifyStringVal(strReplyProcess) || (nReplySuccess < 0));
        bool bProcessNymboxBalanceError =
            (!VerifyStringVal(strReplyProcess) || (nBalanceSuccess < 0));
        bool bProcessNymboxTransError =
            (!VerifyStringVal(strReplyProcess) || (nTransSuccess < 0));

        bool bProcessNymboxReplySuccess =
            (!bProcessNymboxReplyError && (nReplySuccess > 0));
        bool bProcessNymboxReplyFailure =
            (!bProcessNymboxReplyError && (nReplySuccess == 0));

        bool bProcessNymboxBalanceSuccess =
            (!bProcessNymboxReplyError && !bProcessNymboxBalanceError &&
             (nBalanceSuccess > 0));
        bool bProcessNymboxBalanceFailure =
            (!bProcessNymboxReplyError && !bProcessNymboxBalanceError &&
             (nBalanceSuccess == 0));

        bool bProcessNymboxTransSuccess =
            (!bProcessNymboxReplyError && !bProcessNymboxBalanceError &&
             !bProcessNymboxTransError && (nTransSuccess > 0));
        bool bProcessNymboxTransFailure =
            (!bProcessNymboxReplyError && !bProcessNymboxBalanceError &&
             !bProcessNymboxTransError && (nTransSuccess == 0));

        bool bProcessAnyError =
            (bProcessNymboxReplyError || bProcessNymboxBalanceError ||
             bProcessNymboxTransError);
        bool bProcessAnyFailure =
            (bProcessNymboxReplyFailure || bProcessNymboxBalanceFailure ||
             bProcessNymboxTransFailure);
        bool bProcessAllSuccess =
            (bProcessNymboxReplySuccess && bProcessNymboxBalanceSuccess &&
             bProcessNymboxTransSuccess);

        // Note: we LEAVE the sent message in the "sent queue" until we are
        // certain that it processed.
        // If we are NOT certain that it processed, then we try to download the
        // Nymbox and see if there's
        // a reply there (for the sent message.) If we are able to confirm THAT,
        // AFTER SUCCESSFULLY downloading
        // the Nymbox, then then we don't have to do anything because we know
        // for sure it was processed.
        // Similarly, if we DEFINITELY download the Nymbox and do NOT find the
        // reply, then we know the server
        // DEFINITELY did not receive (or at least process) that message, which
        // is what allows us to HARVEST
        // the transaction numbers back from the sent message, and remove the
        // sent message from the sent queue.
        //
        // However, if we are NOT able to Verify any of these things, NOR are we
        // able to download the Nymbox to
        // see, then we DO leave the message in the sent queue. This is
        // deliberate, since it gives us the opportunity
        // in the future to clear those sent messages NEXT time we successfully
        // DO download the Nymbox, and in the
        // meantime, it allows us to store a record of EXACTLY which messages
        // were MISSED.
        //

        std::int32_t nHarvested = -1;

        if (bProcessAllSuccess) {
            // the processNymbox was a complete success, including the message
            // AND the transaction AND the transaction statement.
            // Therefore, there's DEFINITELY nothing to clawback.
            //
            // (Thus I RemoveSentMessage for the processNymbox message, since
            // I'm totally done with it now.)
            //
            //         std::int32_t nRemoved =
            // SwigWrap::RemoveSentMessage(Integer.toString(nProcess),
            // notaryID, nymID);

            // NOTE: The above call is unnecessary, since a successful process
            // means
            // we already received the successful server reply, and OT's
            // "ProcessServerReply"
            // already removed the sent message from the sent buffer (so no need
            // to do that here.)
            //
        } else if (bProcessAnyError || bProcessAnyFailure)  // let's resync, and
                                                            // clawback whatever
        // transaction numbers we might have used
        // on the processNymbox request...
        {
            nGetNymbox = getNymbox(
                notaryID,
                nymID,
                true);  // bForceDownload=true - NOTE: could
                        // maybe change this to false and have
                        // it still work.;

            if (nGetNymbox < 1) {
                LogNormal(OT_METHOD)(__FUNCTION__)(
                    ": Failure: this.getNymbox returned: ")(nGetNymbox)(".")
                    .Flush();

                return -1;
            }

            bool bWasFound = false;
            nBoxType =
                0;  // I think it was already zero, but since i'm using it here;
            // PURELY to be a '0', I feel safer setting it again. (Nymbox is 0.)
            //   bool bInsured = insureHaveAllBoxReceipts(notaryID, nymID,
            // nymID, nBoxType, nRequestNumber, bFoundNymboxItem);   //
            // ***************************;
            if (insureHaveAllBoxReceipts(
                    notaryID,
                    nymID,
                    nymID,
                    nBoxType,
                    nProcess,
                    bWasFound))  // This will tell
                                 // us whether the
                                 // processNymbox
                                 // reply was
                                 // found in the
                                 // Nymbox
            {
                // we FOUND the processNymbox reply in the Nymbox!
                //
                if (bWasFound) {
                    // Thus, no need to clawback any transaction numbers,
                    // since the server clearly already processed this
                    // processNymbox
                    // transaction, since I have a reply to it already sitting
                    // in my Nymbox.
                    //
                    //                       std::int32_t nRemoved =
                    // SwigWrap::RemoveSentMessage(Integer.toString(nProcess),
                    // notaryID, nymID);
                    //
                    // NOTE: The above call is unnecessary, since a successful
                    // process means
                    // we already received the successful server reply, and OT's
                    // "ProcessServerReply"
                    // already removed the sent message from the sent buffer (so
                    // no need to do that here.)

                    LogNormal(OT_METHOD)(__FUNCTION__)(
                        ": FYI: I *did* find the "
                        "processNymboxResponse reply in my "
                        "Nymbox, so NO NEED to clawback "
                        "any transaction numbers.")
                        .Flush();
                } else  // was NOT found... we need to clawback.
                {
                    // This means the server's reply was definitely NOT found in
                    // the
                    // Nymbox, even after successfully DOWNLOADING that Nymbox.
                    // Which
                    // means the server never got it in the first place, or
                    // rejected it
                    // at the message level before the transaction portion had a
                    // chance
                    // to run. Either way, we need to claw back any relevant
                    // transaction
                    // numbers...

                    // HARVEST the processNymbox message from outgoing messages.

                    LogDebug(OT_METHOD)(__FUNCTION__)(
                        ": FYI: Calling OT_API_GetSentMessage...")
                        .Flush();

                    std::string strSentProcessNymboxMsg =
                        api_.Exec().GetSentMessage(
                            std::int64_t(nProcess), notaryID, nymID);

                    if (!VerifyStringVal(strSentProcessNymboxMsg)) {
                        LogVerbose(OT_METHOD)(__FUNCTION__)(
                            ": (2) OT_API_GetSentMessage returned ")(
                            "nullptr for clawback. (Must have already ")(
                            "been cleared. OT uses deliberate ")(
                            "redundancy, though optimizes this wherever ")(
                            "possible.) Request number: ")(nProcess)
                            .Flush();
                    } else  // strSentProcessNymboxMsg NOT null!
                    {
                        LogNormal(OT_METHOD)(__FUNCTION__)(
                            ": FYI: Harvesting transaction "
                            "numbers from failed "
                            "processNymbox attempt...")
                            .Flush();

                        nHarvested = api_.Exec().Msg_HarvestTransactionNumbers(
                            strSentProcessNymboxMsg,
                            nymID,
                            false,  // bHarvestingForRetry = = false;
                            bProcessNymboxReplySuccess,  // bReplyWasSuccess,
                                                         // // RECEIVED server
                                                         // reply: explicit
                                                         // success.
                            bProcessNymboxReplyFailure,  // bReplyWasFailure,
                                                         // // RECEIVED server
                                                         // reply: explicit
                                                         // failure.
                            bProcessNymboxTransSuccess,  // bTransactionWasSuccess,
                                                         // // MESSAGE success,
                                                         // Transaction success.
                                                         // (Explicit.)
                            bProcessNymboxTransFailure);  // bTransactionWasFailure
                                                          // // MESSAGE success,
                        // Transaction failure.
                        // (Explicit.)

                        LogNormal(OT_METHOD)(__FUNCTION__)(
                            ": OT_API_Msg_HarvestTransactionNumbers: ")(
                            nHarvested)(".")
                            .Flush();

                        bool nRemovedProcessNymboxMsg =
                            api_.Exec().RemoveSentMessage(
                                std::int64_t(nProcess), notaryID, nymID);

                        LogVerbose(OT_METHOD)(__FUNCTION__)(
                            nRemovedProcessNymboxMsg)
                            .Flush();
                    }  // strSentProcessNymboxMsg NOT null!
                }  // a specific receipt was not found in the nymbox (need to
                   // clawback the transaction numbers on that receipt.)

                strNymbox = api_.Exec().LoadNymboxNoVerify(
                    notaryID,
                    nymID);  // FLUSH SENT MESSAGES!!!!  (AND HARVEST.);
                if (VerifyStringVal(strNymbox)) {
                    api_.Exec().FlushSentMessages(
                        false,  // harvesting for retry = = OT_FALSE
                        notaryID,
                        nymID,
                        strNymbox);
                }
                // Flushing removes all the messages from the "sent messages"
                // queue,
                // and harvests any transaction numbers to be had. How do I know
                // for sure
                // that I can get away with this? How do I know whether the
                // server has
                // processed those messages or not? How can I harvest them as
                // though they
                // were dropped on the network somewhere? The answer is because
                // I JUST
                // called getNymbox and downloaded the latest one. I can SEE
                // which replies
                // are in there -- and which ones aren't. I pass that Nymbox
                // into the flush
                // call, so that flush can be careful to remove all sent
                // messages that have
                // nymbox replies, and only harvest the others.
                else {
                    LogNormal(OT_METHOD)(__FUNCTION__)(
                        ": Error while trying to flush "
                        "sent messages: Failed loading "
                        "Nymbox for nym: ")(nymID)(".")
                        .Flush();
                }
            }     // if insureHaveAllBoxReceipts()
            else  // we do NOT have all the box receipts.
            {
                LogNormal(OT_METHOD)(__FUNCTION__)(
                    ": Error: insureHaveAllBoxReceipts "
                    "failed. (I give up).")
                    .Flush();
                return -1;
            }
        }  // else if (bProcessAnyError || bProcessAnyFailure)

        // Return the request number, if potentially needed by caller.
        // If explicit success, the request number is returned as a positive
        // number (though already removed from sent queue.) Whereas if explicit
        // failure (replystatus = failed) then we harvest the numbers;
        //
        if (bProcessAllSuccess)
        //          return getNymbox(notaryID, nymID, true);
        // //bForceDownload = true. Since we DID process it successfully,
        // then we grab it again.
        {
            return 1;
        }  // We don't need the sent message after this, and we've already
           // removed it from sent queue.

        if (bProcessAnyFailure || bProcessAnyError) {
            if (nHarvested < 1)  // If the message failed, and the harvesting
                                 // failed, then we return the
            {
                return nProcess;
            }  // number for the process nymbox, so the caller has a choice of
               // what to do next.

            if (bProcessAnyFailure) {
                return 0;
            }  // by this point, we've definitely harvested, AND removed sent
               // message from sent queue. So we just return 0;
        }

        return -1;  // must've been an error.
    }               // if insureAllBoxReceipts()
    else {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": insureHaveAllBoxReceipts failed. (I give up).")
            .Flush();
    }

    return -1;
}

//  def Utility::getAndProcessNymbox(String notaryID, String nymID, OTBool
// bWasMsgSent, boolean bForceDownload)
//
std::int32_t Utility::getAndProcessNymbox_4(
    const std::string& notaryID,
    const std::string& nymID,
    bool& bWasMsgSent,
    bool bForceDownload)
{
    std::string strLocation = "Utility::getAndProcessNymbox_4";

    if (!VerifyStringVal(notaryID) || !VerifyStringVal(nymID)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": SHOULD NEVER HAPPEN!!! ASSERT!! ERROR!! "
            "FAILURE!!! PROBLEM!!!!!")
            .Flush();
        return -1;
    }
    //   bool bMsgReplySuccess5 = false;
    //   bool bMsgReplyFailure5 = false;
    //   bool bMsgTransSuccess5 = false;
    //   bool bMsgTransFailure5 = false;

    std::int32_t nRequestNumber = 0;
    bool bHarvestingForRetry = false;
    OTfourbool the_foursome = {false, false, false, false};
    bool bFoundNymboxItem = false;  // bFoundNymboxItem is output bool, telling
                                    // caller whether it was found.;

    return getAndProcessNymbox_8(
        notaryID,
        nymID,
        bWasMsgSent,
        bForceDownload,
        nRequestNumber,
        bFoundNymboxItem,
        bHarvestingForRetry,
        the_foursome);
}

//  public static std::int32_t getNymboxLowLevel(String notaryID, String nymID)
//  public static std::int32_t receiveNymboxLowLevel(String notaryID, String
//  nymID,
// var nRequestNum)
//  public static std::int32_t processNymboxLowLevel(String notaryID, String
//  nymID) {

//  def Utility::getAndProcessNymbox(String notaryID, String nymID, OTBool
// bWasMsgSent)
std::int32_t Utility::getAndProcessNymbox_3(
    const std::string& notaryID,
    const std::string& nymID,
    bool& bWasMsgSent)
{
    bool bForceDownload = false;
    return getAndProcessNymbox_4(notaryID, nymID, bWasMsgSent, bForceDownload);
}

// NEW VERSION:

// PROCESS NYMBOX
//
// Returns:
// -1 Error.
//  0 Nymbox was empty -- nothing done. (bWasMsgSent = false)
//  0 server reply received, but it sayssuccess = =false on that msg.
// (bWasMsgSent = true);
// >0 If the Transaction status (from the server reply) is SUCCESS, then this
// function
//    returns the REQUEST NUMBER from when it was originally sent.
//
// public static std::int32_t processNymbox(String    notaryID, String nymID,
//                                OTBool    bWasMsgSent,
//
//                                OTInteger nMsgSentRequestNumOut,
//                                OTInteger nReplySuccessOut,
//                                OTInteger nBalanceSuccessOut,
//                                OTInteger nTransSuccessOut)

std::int32_t Utility::processNymbox(
    const std::string& notaryID,
    const std::string& nymID,
    bool& bWasMsgSent,
    std::int32_t& nMsgSentRequestNumOut,
    std::int32_t& nReplySuccessOut,
    std::int32_t& nBalanceSuccessOut,
    std::int32_t& nTransSuccessOut)
{
    bWasMsgSent = false;
    std::string strLocation = "Utility::processNymbox";

    nMsgSentRequestNumOut = -1;
    nReplySuccessOut = -1;
    nBalanceSuccessOut = -1;
    nTransSuccessOut = -1;

    // -- SECTION 2: "SEND PROCESS NYMBOX"

    // Next, we have to make sure we have all the BOX RECEIPTS downloaded
    // for this Nymbox.
    const auto [nProcess, trans, result] = api_.OTAPI().processNymbox(context_);
    const auto& [status, reply] = result;
    [[maybe_unused]] const auto& notUsed1 = trans;
    [[maybe_unused]] const auto& notUsed2 = status;

    if (-1 == nProcess) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": (2) error (-1), when calling "
            "sendProcessNymboxLowLevel. (It couldn't send. "
            "I give up).")
            .Flush();
        return -1;  // (It didn't even send.)
    }
    // Nymbox was empty. (So we didn't send any process message because there
    // was nothing to process.)
    if (0 == nProcess) {
        return 0;  // success. done.
    }
    if (nProcess < 0) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unexpected: ")(nProcess)(
            ", when calling sendProcessNymboxLowLevel. (I give up).")
            .Flush();
        return -1;
    }

    bWasMsgSent = true;
    nMsgSentRequestNumOut = nProcess;

    OT_ASSERT(reply)

    const std::string strReplyProcess = String::Factory(*reply)->Get();
    std::int32_t nReplySuccess = VerifyMessageSuccess(
        api_, strReplyProcess);  // sendProcessNymboxLowLevel;
    std::int32_t nTransSuccess = -1;
    std::int32_t nBalanceSuccess = -1;
    ;
    if (nReplySuccess > 0)  // If message was success, then let's see if the
                            // transaction was, too.
    {
        nBalanceSuccess = api_.Exec().Message_GetBalanceAgreementSuccess(
            notaryID,
            nymID,
            nymID,
            strReplyProcess);  // the processNymbox transaction.;
        if (nBalanceSuccess > 0) {
            nTransSuccess = api_.Exec().Message_GetTransactionSuccess(
                notaryID,
                nymID,
                nymID,
                strReplyProcess);  // the processNymbox transaction.;
        }
    }

    nReplySuccessOut = nReplySuccess;
    nBalanceSuccessOut = nBalanceSuccess;
    nTransSuccessOut = nTransSuccess;

    // NOTE: The caller MUST have a call to SwigWrap::RemoveSentMessage
    // to correspond to THIS function's call of sendProcessNymboxLowLevel().
    //
    if (nTransSuccess > 0) {
        setLastReplyReceived(strReplyProcess);

        return nProcess;
    }

    setLastReplyReceived("");

    return nTransSuccess;
}

// called by getBoxReceiptWithErrorCorrection   DONE
bool Utility::getBoxReceiptLowLevel(
    const std::string& accountID,
    std::int32_t nBoxType,
    std::int64_t strTransactionNum,
    bool& bWasSent)  // bWasSent is OTBool
{
    std::string strLocation = "Utility::getBoxReceiptLowLevel";
    bWasSent = false;

    auto [nRequestNum, transactionNum, result] = api_.OTAPI().getBoxReceipt(
        context_, Identifier::Factory(accountID), nBoxType, strTransactionNum);
    const auto& [status, reply] = result;
    [[maybe_unused]] const auto& notUsed1 = transactionNum;
    [[maybe_unused]] const auto& notUsed3 = nRequestNum;

    switch (status) {
        case SendResult::VALID_REPLY: {
            bWasSent = true;
            setLastReplyReceived(String::Factory(*reply)->Get());

            return true;
        } break;
        case SendResult::TIMEOUT: {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to send getNymbox message due to error.")
                .Flush();
            setLastReplyReceived("");

            return false;
        } break;
        default: {
        }
    }

    LogOutput(OT_METHOD)(__FUNCTION__)(": Error!").Flush();
    setLastReplyReceived("");

    return false;
}

// called by insureHaveAllBoxReceipts     DONE
bool Utility::getBoxReceiptWithErrorCorrection(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& accountID,
    std::int32_t nBoxType,
    std::int64_t strTransactionNum)  // nBoxType is std::int32_t
{
    std::string strLocation = "Utility::getBoxReceiptWithErrorCorrection";

    bool bWasSent = false;
    bool bWasRequestSent = false;

    if (getBoxReceiptLowLevel(
            accountID, nBoxType, strTransactionNum, bWasSent)) {
        return true;
    }

    if (bWasSent && (0 < context_.UpdateRequestNumber(bWasRequestSent))) {
        if (bWasRequestSent &&
            getBoxReceiptLowLevel(
                accountID, nBoxType, strTransactionNum, bWasSent)) {
            return true;
        }
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": getBoxReceiptLowLevel failed, then "
            "getRequestNumber succeeded, then "
            "getBoxReceiptLowLevel failed again. (I give "
            "up).")
            .Flush();
    } else {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": getBoxReceiptLowLevel failed, then "
            "getRequestNumber failed. (I give up.) Was "
            "getRequestNumber message sent: ")(bWasRequestSent)(".")
            .Flush();
    }

    return false;
}

// This function assumes you just downloaded the latest version of the box
// (inbox, outbox, or nymbox)
// and its job is to make sure all the related box receipts are downloaded as
// well and available, though
// not necessarily loaded into memory. (Yet.)
// NOTE: If nBoxType is 0 (nymbox) then pass nymID as the accountID as well!
//
bool Utility::insureHaveAllBoxReceipts(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& accountID,
    std::int32_t nBoxType)  // nBoxType is std::int32_t
{
    bool bFoundIt = false;
    std::int32_t nRequestSeeking = 0;
    return insureHaveAllBoxReceipts(
        notaryID, nymID, accountID, nBoxType, nRequestSeeking, bFoundIt);
}

bool Utility::insureHaveAllBoxReceipts(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& accountID,
    std::int32_t nBoxType,
    std::int32_t nRequestSeeking,
    bool& bFoundIt)
{
    std::string strLocation = "Utility::insureHaveAllBoxReceipts";

    const auto theNotaryID = Identifier::Factory(notaryID),
               theNymID = Identifier::Factory(nymID),
               theAccountID = Identifier::Factory(accountID);

    std::unique_ptr<Ledger> pLedger;

    if (0 == nBoxType) {
        pLedger.reset(
            api_.OTAPI().LoadNymboxNoVerify(theNotaryID, theNymID).release());
    } else if (1 == nBoxType) {
        pLedger.reset(
            api_.OTAPI()
                .LoadInboxNoVerify(theNotaryID, theNymID, theAccountID)
                .release());
    } else if (2 == nBoxType) {
        pLedger.reset(
            api_.OTAPI()
                .LoadOutboxNoVerify(theNotaryID, theNymID, theAccountID)
                .release());
    } else {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Error. Expected nBoxType of 0,1,2 (nymbox, "
            "inbox, or outbox).")
            .Flush();
        return false;
    }

    // Unable to load or verify inbox/outbox/nymbox
    // Notice I don't call VerifyAccount() here (not that the API even exposes
    // that method) but why not? Because that method tries to load up all the
    // box receipts, in addition to verifying the signature. Therefore I call
    // "Load XXXX NoVerify()", avoiding all that, then I verify the Signature
    // itself.
    //
    // That's because this function's whole point is to find out what the box
    // receipts are, and download them from the server. No point trying to load
    // them before that time, when I know it will fail.
    //
    if (!pLedger
        //      || !pLedger->VerifySignature(nymID, ledger)
    ) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Unable to load "  // or verify signature on "
            "ledger. (Failure).")
            .Flush();
        return false;
    }
    // ----------------------------------------------------------------
    bool bReturnValue = true;  // Assuming an empty box, we return success;

    // At this point, the box is definitely loaded.
    //
    // Next we'll iterate the receipts within, and for each, verify that the
    // Box Receipt already exists. If not, then we'll download it using
    // getBoxReceiptLowLevel(). If any download fails, then we break out of the
    // loop (WITHOUT continuing on to try the rest.)
    //
    auto& map_receipts = pLedger->GetTransactionMap();

    for (auto& receipt_entry : map_receipts) {
        const auto& lTransactionNum = receipt_entry.first;
        auto& pTransaction = receipt_entry.second;

        if (lTransactionNum <= 0 || nullptr == pTransaction) {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Error: Transaction null or has ID "
                "less-than-or-equal-to 0.")
                .Flush();
            continue;
        }

        const bool bIsReplyNotice =
            (transactionType::replyNotice == pTransaction->GetType());

        const RequestNumber lRequestNum =
            bIsReplyNotice ? pTransaction->GetRequestNum() : 0;

        const bool bShouldDownload =
            (!bIsReplyNotice || (bIsReplyNotice && (0 < lRequestNum) &&
                                 !api_.OTAPI().HaveAlreadySeenReply(
                                     theNotaryID, theNymID, lRequestNum)));

        // This block executes if we should download it (assuming we
        // haven't already, which it also checks for.)
        //
        if (bShouldDownload) {
            bool bHaveBoxReceipt = api_.OTAPI().DoesBoxReceiptExist(
                theNotaryID, theNymID, theAccountID, nBoxType, lTransactionNum);
            if (!bHaveBoxReceipt) {
                LogDetail(OT_METHOD)(__FUNCTION__)(
                    ": Downloading box receipt to add to my collection...")
                    .Flush();
                const bool bDownloaded = getBoxReceiptWithErrorCorrection(
                    notaryID, nymID, accountID, nBoxType, lTransactionNum);
                if (!bDownloaded) {
                    LogNormal(OT_METHOD)(__FUNCTION__)(
                        ": Failed downloading box receipt. "
                        "(Skipping any others.) Transaction "
                        "number: ")(lTransactionNum)(".")
                        .Flush();

                    bReturnValue = false;
                    break;
                    // No point continuing to loop and fail 500
                    // times, when
                    // getBoxReceiptWithErrorCorrection()
                    // already failed
                    // even doing the getRequestNumber() trick
                    // and everything, and whatever retries are
                    // inside OT, before it finally
                    // gave up.
                }
                // else (Download success.)
            }  // if (!bHaveBoxReceipt)
        }

        // else we already have the box receipt, no need to
        // download again.
    }  // for
    // ----------------------------------------------------------------
    //
    // if nRequestSeeking is >0, that means the caller wants to know if there is
    // a receipt present for that request number.
    // (which is only a valid option ifnBoxType = = 0 for Nymbox.);
    // IF the receipt is found, then bFoundIt is set to true.
    //
    if ((nRequestSeeking > 0) && (0 == nBoxType)) {

        // NOTE: the below call to SwigWrap::Nymbox_GetReplyNotice
        // will succeed even if only the abbreviated receipt is
        // available, because the caller mainly just wants to know
        // if it is there. Technically the full receipt SHOULD
        // always be there, with the above loop, but since the above
        // loop can break in case of error, it's still possible that
        // box receipts haven't been downloaded by the time you
        // reach this code. Nevertheless, we will see if the reply
        // is there for the appropriate request number, whether
        // abbreviated or not.
        // // //
        // UPDATE: I am now adding specific cases where the
        // replyNotice is NOT downloaded. You still use it, through
        // its abbreviated version -- and the actual version IS
        // still available for download through the server's API.
        // But with a replyNotice, just knowing that it exists is
        // usually enough for the client, who probably still has a
        // cached copy of the original sent message anyway. Only in
        // cases where he doesn't, would he need to download it.
        // (Why? So he can process the server's reply.) Therefore
        // the cached sent message is useless, since it doesn't
        // contain the server's reply! Hmm. So I need that reply BUT
        // ONLY IN CASES where I didn't already receive it as a
        // normal part of processing (and that is MOST of the time,
        // meaning most cases can thus entirely eliminate the
        // download.)
        // // //
        // PROTOCOL FOR NOT DOWNLOADING MOST OF THE BOX RECEIPTS
        // // //
        // Solution: User messages should contain a list of the last
        // X number of request numbers that they have DEFINITELY
        // seen the response to. The server, meanwhile, since the
        // user has DEFINITELY seen the response, can now safely
        // remove the replyNotice from the Nymbox. The only reason
        // it was there in the first place was to make sure the user
        // got the reply. Since the user is straight-up
        // acknowledging that he received it, the server no longer
        // needs to "make sure" and thus it can remove that reply
        // from the Nymbox, and mark the box receipt for deletion.
        // This will be MOST REPLIES! We'll eliminate the step of
        // having to download the box receipt. The above call to
        // getBoxReceiptWithErrorCorrection should also be smart
        // enough not to bother downloading any replyNotice Box
        // Receipts if their request number appears on that list.
        // Again: the list means I DEFINITELY already responded to
        // it--if the request # is on that list, then NO NEED
        // downloading the Box Receipt -- I DEFINITELY already got
        // that reply!
        // // //
        // Therefore, Something like
        // SwigWrap::HaveAlreadySeenReply( notaryID, nymID,
        // requestNum);
        // // //
        // Perhaps also, on the server side, send a list of request
        // numbers for that Nym that the server KNOWS the Nym saw
        // the reply to. This way, the Nym can remove the number
        // from his list, and thus won't be continually causing the
        // server to load up the Nymbox and try to remove the
        // replyNotice (since it's already been removed.)
        // // //
        // The Nym doesn't have to keep a list of ALL request
        // numbers he's seen the reply to. Rather, just the past X
        // number of them, and with the number explicitly removed
        // once he sees the server acknowledgment.
        // (ServerAckOfAlreadyGotReply.)
        // // //
        // The server, meanwhile, is free to remove the ACK for any
        // request # once he sees that the client has as well.
        // Server also only needs to store a list of the past X
        // request #s. Also: since both sides REMOVE the number,
        // there need not necessarily be a limit on the size of the
        // list, since it grows and shrinks as needed.
        // // //
        // Whenever Wallet processes a server reply, just see if
        // it's on that "replied to already" list already on client
        // side. If so, discard the reply.
        // OTClient::ProcessServerReply probably best place to do
        // (We replied to it already, so discard it.) Also, for any
        // server reply, look at the list of numbers on it. The
        // server is acknowledging to us that it KNOWS we got those
        // replies, and that it ALREADY has removed them from the
        // Nymbox as a result. Therefore we can remove ALL of those
        // numbers from our own list as well. No need for an API
        // call to do this, since it will happen internally to OT.
        // // //
        // On the server side, any numbers on its own list were only
        // there to acknowledge numbers that had been on the client
        // side list. Therefore, when those numbers disappear from
        // the client side list, the server simply removes them.
        // Again: ANY NUMBERS on the server list, which do NOT
        // appear on the client list, are REMOVED From the server
        // list. After all, the client has clearly now removed them,
        // so the server doesn't have to keep them around either.
        // // //
        // These are useful for synchronization but also there's a
        // std::int64_t term benefit, if we include them in the
        // signed receipt (which they will be already, if the
        // receipt contains the entire message and not just the
        // transaction.) That benefit is that we can prove receipt
        // of notice. At least, notice of server replies. But for
        // other notice types, such as notice of upcoming meeting.
        // Or notice of upcoming election. Or notice of election
        // outcome. Or notice of petition to put X issue on the next
        // ballot, or to nominate Y Nym for some corporate role.
        // Sometimes you want to be able to PROVE that notice was
        // received. Does this prove that? Hmm, not necessarily.
        // Currently I'm using this as an optimization scheme, which
        // is useful even if not provable. How to make it provable?
        // // //
        // Back from tangent: Wait a sec! If I notice the server
        // that I saw the reply, the server will remove that reply
        // from my Nymbox -- but it's still in my Nymbox on the
        // client side! Until I download the latest Nymbox. Thus if
        // I try to PROCESS MY NYMBOX, I will be attempting to
        // accept a receipt that's already gone! (And the
        // processNymbox will therefore FAIL!) Solution: be smart
        // enough, when processing Nymbox, to IGNORE any
        // replyNotices when the request Number appears on the
        // client's list! As the wallet processes the Nymbox it
        // should already know to skip the ones that were already
        // replied-to. Meanwhile the server side will deliberately
        // NOT update the Nymbox hash just because the receipt was
        // removed. Otherwise it could trigger an unnecessary
        // download of the Nymbox, when the whole point of this
        // exercise was to prevent unnecessary downloads. It only
        // updates the Nymbox hash when it WANTS me to download the
        // Nymbox, and that certainly does NOT apply to cases where
        // the only change involved the removal of some old receipt
        // I already acknowledged. (No need to force any downloads
        // based on THAT case, after all.)
        //
        std::string strReplyNotice = api_.Exec().Nymbox_GetReplyNotice(
            notaryID, nymID, std::int64_t(nRequestSeeking));

        if (VerifyStringVal(strReplyNotice)) { bFoundIt = true; }
    }

    return bReturnValue;
}

/*
bool Utility::insureHaveAllBoxReceipts(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& accountID,
    std::int32_t nBoxType,
    std::int32_t nRequestSeeking,
    bool& bFoundIt)
{
    std::string strLocation = "Utility::insureHaveAllBoxReceipts";

    std::string ledger = "";

    if (0 == nBoxType) {
        ledger = SwigWrap::LoadNymboxNoVerify(notaryID, nymID);
    } else if (1 == nBoxType) {
        ledger = SwigWrap::LoadInboxNoVerify(notaryID, nymID, accountID);
    } else if (2 == nBoxType) {
        ledger = SwigWrap::LoadOutboxNoVerify(notaryID, nymID, accountID);
    } else {
        otOut << strLocation << ": Error. Expected nBoxType of 0,1,2 (nymbox, "
                                "inbox, or outbox.)\n";
        return false;
    }

    // Unable to load or verify inbox/outbox/nymbox
    // Notice I don't call VerifyAccount() here (not that the API even exposes
    // that method) but why not? Because that method tries to load up all the
    // box receipts, in addition to verifying the signature. Therefore I call
    // "Load XXXX NoVerify()", avoiding all that, then I verify the Signature
    // itself. That's because this function's whole point is to find out what
    // the box receipts are, and download them from the server. No point trying
    // to load them before that time, when I know it will fail.
    //
    if (!VerifyStringVal(ledger) ||
        (!SwigWrap::VerifySignature(nymID, ledger))) {
        otOut << strLocation << ": Unable to load or verify signature on "
                                "ledger. (Failure.) Contents:\n"
              << ledger << "\n";
        return false;
    }

    // At this point, the box is definitely loaded.
    // Next we'll iterate the receipts
    // within, and for each, verify that the Box Receipt already exists. If not,
    // then we'll download it using getBoxReceiptLowLevel(). If any download
    // fails,
    // then we break out of the loop (without continuing on to try the rest.)
    //
    bool bReturnValue = true;  // Assuming an empty box, we return success;

    std::int32_t nReceiptCount =
        SwigWrap::Ledger_GetCount(notaryID, nymID, accountID, ledger);
    if (nReceiptCount > 0) {
        for (std::int32_t i_loop = 0; i_loop < nReceiptCount; ++i_loop) {
            std::int64_t lTransactionNum =
                SwigWrap::Ledger_GetTransactionIDByIndex(
                    notaryID, nymID, accountID, ledger, i_loop);
            if (lTransactionNum != -1) {
                if (lTransactionNum > 0) {
                    std::string strTransaction =
                        SwigWrap::Ledger_GetTransactionByID(
                            notaryID,
                            nymID,
                            accountID,
                            ledger,
                            lTransactionNum);

                    // Note: SwigWrap::Ledger_GetTransactionByID tries to get
                    // the full transaction from the ledger and
                    // return it;
                    // and pass the full version back to us. Failing that
                    // (perhaps the box receipt hasn't been downloaded
                    // yet) then it will try to re-sign / re-save the
                    // abbreviated version and pass it back. So if, by this
                    // point, it STILL has failed, that means it couldn't even
                    // give us the abbreviated version either!
                    //
                    if (!VerifyStringVal(strTransaction)) {
                        otOut << strLocation
                              << ": Error: Null transaction somehow returned, "
                                 "(not even an abbreviated one!) even though I "
                                 "had a good ID "
                              << lTransactionNum
                              << " which came originally as lTransactionNum: "
                              << lTransactionNum << " for index: " << i_loop
                              << " with the contents:\n\n"
                              << strTransaction << "\n\n"
                              << "HERE IS THE LEDGER ITSELF:\n\n" << ledger <<
"\n\n"; return false;
                    }

                    // This block might have a full version, OR an abbreviated
                    // version of the box receipt in
                    // question, in the strTransaction variable. It will attempt
                    // to download the full box
                    // receipt, if we don't already have it here on the client
                    // side.
                    {
                        std::string strTransType =
                            SwigWrap::Transaction_GetType(
                                notaryID, nymID, accountID, strTransaction);
                        bool bIsReplyNotice =
                            (VerifyStringVal(strTransType) &&
                             (strTransType == "replyNotice"));
                        std::int64_t lRequestNum = 0;
                        if (bIsReplyNotice) {
                            lRequestNum = SwigWrap::ReplyNotice_GetRequestNum(
                                notaryID, nymID, strTransaction);
                        }

                        bool bShouldDownload =
                            (!bIsReplyNotice ||
                             (bIsReplyNotice && (0 < lRequestNum) &&
                              !SwigWrap::HaveAlreadySeenReply(
                                  notaryID, nymID, lRequestNum)));

                        if (bShouldDownload)  // This block executes if we
                                              // should
                        // download it (assuming we haven't
                        // already, which it also checks
                        // for.)
                        {
                            bool bHaveBoxReceipt =
                                SwigWrap::DoesBoxReceiptExist(
                                    notaryID,
                                    nymID,
                                    accountID,
                                    nBoxType,
                                    lTransactionNum);
                            if (!bHaveBoxReceipt) {
                                otWarn << strLocation << ": Downloading box "
                                                         "receipt to add to my "
                                                         "collection...\n";

                                bool bDownloaded =
                                    getBoxReceiptWithErrorCorrection(
                                        notaryID,
                                        nymID,
                                        accountID,
                                        nBoxType,
                                        lTransactionNum);
                                if (!bDownloaded) {
                                    otOut
                                        << strLocation
                                        << ": Failed downloading box receipt. "
                                           "(Skipping any others.) Transaction "
                                           "number: "
                                        << lTransactionNum << "\n";

                                    bReturnValue = false;
                                    break;
                                    // No point continuing to loop and fail 500
                                    // times, when
                                    // getBoxReceiptWithErrorCorrection()
                                    // already failed
                                    // even doing the getRequestNumber() trick
                                    // and
                                    // everything, and whatever retries are
                                    // inside OT, before it finally
                                    // gave up.
                                }
                                // else (Download success.)
                            }  // if (!bHaveBoxReceipt)
                        }

                        // else we already have the box receipt, no need to
                        // download again.
                    }
                }  // if (lTransactionNum > 0)
                else {
                    otOut
                        << strLocation
                        << ": Error: TransactionNum less-than-or-equal-to 0.\n";
                }
            } else {
                otOut << strLocation << ": Error: TransactionNum was null, "
                                        "when trying to read it based on the "
                                        "index (within bounds, too!)\n";
            }
        }  // ************* FOR LOOP ******************
    }      // if (nReceiptCount > 0)

    //
    // if nRequestSeeking is >0, that means the caller wants to know if there is
    // a receipt present for that request number.
    // (which is only a valid option ifnBoxType = = 0 for Nymbox.);
    // IF the receipt is found, then bFoundIt is set to true.
    //
    if ((nRequestSeeking > 0) && (0 == nBoxType)) {
        // NOTE: the below call to SwigWrap::Nymbox_GetReplyNotice will
        // succeed even if
        // only the abbreviated receipt is available, because the caller mainly
        // just
        // wants to know if it is there.
        // Technically the full receipt SHOULD always be there, with the above
        // loop,
        // but since the above loop can break in case of error, it's still
        // possible that
        // box receipts haven't been downloaded by the time you reach this code.
        // Nevertheless, we will see if the reply is there for the appropriate
        // request
        // number, whether abbreviated or not.
        //
        // UPDATE: I am now adding specific cases where the replyNotice is NOT
        // downloaded.
        // You still use it, through its abbreviated version -- and the actual
        // version
        // IS still available for download through the server's API. But with a
        // replyNotice,
        // just knowing that it exists is usually enough for the client, who
        // probably still
        // has a cached copy of the original sent message anyway. Only in cases
        // where he
        // doesn't, would he need to download it. (Why? So he can process the
        // server's reply.)
        // Therefore the cached sent message is useless, since it doesn't
        // contain the server's
        // reply! Hmm. So I need that reply BUT ONLY IN CASES where I didn't
        // already receive it
        // as a normal part of processing (and that is MOST of the time, meaning
        // most cases can
        // thus entirely eliminate the download.)
        //
        // PROTOCOL FOR NOT DOWNLOADING MOST OF THE BOX RECEIPTS
        //
        // Solution: User messages should contain a list of the last X number of
        // request numbers
        // that they have DEFINITELY seen the response to. The server,
        // meanwhile, since the user
        // has DEFINITELY seen the response, can now safely remove the
        // replyNotice from the Nymbox.
        // The only reason it was there in the first place was to make sure the
        // user got the reply.
        // Since the user is straight-up acknowledging that he received it, the
        // server no longer
        // needs to "make sure" and thus it can remove that reply from the
        // Nymbox, and mark the
        // box receipt for deletion. This will be MOST REPLIES! We'll eliminate
        // the step of having
        // to download the box receipt.
        // The above call to getBoxReceiptWithErrorCorrection should also be
        // smart enough not to
        // bother downloading any replyNotice Box Receipts if their request
        // number appears on that
        // list. Again: the list means I DEFINITELY already responded to it--if
        // the request # is on
        // that list, then NO NEED downloading the Box Receipt -- I DEFINITELY
        // already got that reply!
        //
        // Therefore, Something like SwigWrap::HaveAlreadySeenReply(notaryID,
        // nymID, requestNum);
        //
        // Perhaps also, on the server side, send a list of request numbers for
        // that Nym that the
        // server KNOWS the Nym saw the reply to. This way, the Nym can remove
        // the number from his
        // list, and thus won't be continually causing the server to load up the
        // Nymbox and try
        // to remove the replyNotice (since it's already been removed.)
        //
        // The Nym doesn't have to keep a list of ALL request numbers he's seen
        // the reply to.
        // Rather, just the past X number of them, and with the number
        // explicitly removed once
        // he sees the server acknowledgment. (ServerAckOfAlreadyGotReply.)
        //
        // The server, meanwhile, is free to remove the ACK for any request #
        // once he sees that
        // the client has as well. Server also only needs to store a list of the
        // past X request #s.
        // Also: since both sides REMOVE the number, there need not necessarily
        // be a limit on the
        // size of the list, since it grows and shrinks as needed.
        //
        // Whenever Wallet processes a server reply, just see if it's on that
        // "replied to already"
        // list already on client side. If so, discard the reply.
        // OTClient::ProcessServerReply probably
        // best place to do  (We replied to it already, so discard it.)
        // Also, for any server reply, look at the list of numbers on it. The
        // server is acknowledging
        // to us that it KNOWS we got those replies, and that it ALREADY has
        // removed them from the
        // Nymbox as a result. Therefore we can remove ALL of those numbers from
        // our own list
        // as well. No need for an API call to do this, since it will happen
        // internally to OT.
        //
        // On the server side, any numbers on its own list were only there to
        // acknowledge numbers
        // that had been on the client side list. Therefore, when those numbers
        // disappear from the
        // client side list, the server simply removes them. Again: ANY NUMBERS
        // on the server list,
        // which do NOT appear on the client list, are REMOVED From the server
        // list. After all, the
        // client has clearly now removed them, so the server doesn't have to
        // keep them around either.
        //
        // These are useful for synchronization but also there's a std::int64_t
        // term
        // benefit, if we include
        // them in the signed receipt (which they will be already, if the
        // receipt contains the entire
        // message and not just the transaction.) That benefit is that we can
        // prove receipt of notice.
        // At least, notice of server replies. But for other notice types, such
        // as notice of upcoming
        // meeting. Or notice of upcoming election. Or notice of election
        // outcome. Or notice of petition
        // to put X issue on the next ballot, or to nominate Y Nym for some
        // corporate role. Sometimes
        // you want to be able to PROVE that notice was received. Does this
        // prove that?
        // Hmm, not necessarily. Currently I'm using this as an optimization
        // scheme, which is useful
        // even if not provable. How to make it provable?
        //
        // Back from tangent: Wait a sec! If I notice the server that I saw the
        // reply, the server will
        // remove that reply from my Nymbox -- but it's still in my Nymbox on
        // the client side! Until
        // I download the latest Nymbox. Thus if I try to PROCESS MY NYMBOX, I
        // will be attempting to
        // accept a receipt that's already gone! (And the processNymbox will
        // therefore FAIL!)
        // Solution: be smart enough, when processing Nymbox, to IGNORE any
        // replyNotices when the request
        // Number appears on the client's list! As the wallet processes the
        // Nymbox it should already
        // know to skip the ones that were already replied-to.
        // Meanwhile the server side will deliberately NOT update the Nymbox
        // hash just because the receipt
        // was removed. Otherwise it could trigger an unnecessary download of
        // the Nymbox, when the whole
        // point of this exercise was to prevent unnecessary downloads. It only
        // updates the Nymbox hash
        // when it WANTS me to download the Nymbox, and that certainly does NOT
        // apply to cases where the
        // only change involved the removal of some old receipt I already
        // acknowledged. (No need to force
        // any downloads based on THAT case, after all.)
        //
        std::string strReplyNotice = SwigWrap::Nymbox_GetReplyNotice(
            notaryID, nymID, std::int64_t(nRequestSeeking));

        if (VerifyStringVal(strReplyNotice)) {
            bFoundIt = true;
        }
    }

    return bReturnValue;
}
 */

/*
static void getBoxReceipt( std::string NOTARY_ID, std::string NYM_ID,
std::string ACCT_ID, //
If for Nymbox (vs inbox/outbox) then pass NYM_ID
in this field also.
std::int32_t  nBoxType, // 0/nymbox, 1/inbox, 2/outbox
const std::string TRANSACTION_NUMBER);

static bool DoesBoxReceiptExist( std::string NOTARY_ID, std::string NYM_ID,
std::string
ACCT_ID, // If for Nymbox (vs inbox/outbox) then pass NYM_ID
in this field also.
std::int32_t  nBoxType, // 0/nymbox, 1/inbox, 2/outbox
const std::string TRANSACTION_NUMBER);
*/

bool Utility::getIntermediaryFiles(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& accountID)
{
    bool bForceDownload = false;
    return getIntermediaryFiles(notaryID, nymID, accountID, bForceDownload);
}

// NOTE: This is a new version, that uses getInboxAccount new version, which
// uses getAccountData instead of getAccount, getInbox, and getOutbox.
bool Utility::getIntermediaryFiles(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& accountID,
    bool bForceDownload)  // booleanbForceDownload = false;
{
    std::string strLocation = "Utility::getIntermediaryFiles";

    if (!VerifyStringVal(notaryID) || notaryID.size() < 10) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": nullptr or invalid notaryID.")
            .Flush();
        return false;
    }
    if (!VerifyStringVal(nymID) || nymID.size() < 10) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Nullptr or invalid nymID.")
            .Flush();
        return false;
    }
    if (!VerifyStringVal(accountID) || accountID.size() < 10) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": nullptr or invalid accountID.")
            .Flush();
        return false;
    }

    bool bWasSentInbox = false;
    bool bWasSentAccount = false;

    // This is a new version of getInboxAccount that downloads ALL
    // THREE files (account/inbox/outbox) in a single server message.
    //
    std::int32_t nGetInboxAcct = getInboxAccount(
        accountID, bWasSentInbox, bWasSentAccount, bForceDownload);

    // if we received an error state, and the "getAccountData" message wasn't
    // even sent,
    // then no point doing a bunch of retries -- it failed.
    //
    if (-1 == nGetInboxAcct) {
        if (!bWasSentAccount) {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": this.getInboxAccount failed, without "
                "even sending getAccountData. (Returning "
                "false).")
                .Flush();
            return false;
        }
    }

    // If it wasn't sent, and 0 was returned, that means
    // no error: we already have the latest inbox. (Nothing done.)
    //
    else if (!bWasSentInbox && (0 == nGetInboxAcct)) {
        // we don't return true;
    } else if (1 != nGetInboxAcct) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": getInboxAccount failed. (Trying one more time...")(".")
            .Flush();
        const auto nGetRequestNumber = context_.UpdateRequestNumber();

        if (0 >= nGetRequestNumber) {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Failure: getInboxAccount failed, then I "
                "tried to resync with getRequestNumber and "
                "then that failed too. (I give up).")
                .Flush();
            return false;
        }

        ///////////////////////// WARNING: DEAD CODE
        ///?????????????????????????????????

        // We sync'd the request number, so now we try the function again...
        //
        std::int32_t nSecondtry = getInboxAccount(
            accountID, bWasSentInbox, bWasSentAccount, bForceDownload);

        if ((-1 == nSecondtry) && !bWasSentAccount) {
            // if we received an error state, and the "getAccountData" message
            // wasn't even sent,
            // then no point doing a bunch of retries -- it failed.
            //
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": getInboxAccount failed a second time, "
                "without even sending getAccountData. "
                "(Returning false).")
                .Flush();
            return false;
        }
        // If it wasn't sent, and 0 was returned, that means
        // no error: we already have the latest inbox. (Nothing done.)
        //
        if (!bWasSentInbox && (0 == nSecondtry)) {
            // we don't return true;
        } else if (1 != nSecondtry) {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": getInboxAccount re-try failed. (That's twice "
                "now--Returning false.) Value: ")(nSecondtry)(".")
                .Flush();
            return false;
        }
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": getInboxAccount second call succeeded. (Continuing...).")
            .Flush();
    }

    return true;
}

// NOTE: This is a new version that uses the new server message, getAccountData
// (Which combines getAccount, getInbox, and getOutbox into a single message.)
std::int32_t Utility::getInboxAccount(
    const std::string& accountID,
    bool& bWasSentInbox,
    bool& bWasSentAccount,
    const bool)
{
    std::string strLocation = "Utility::getInboxAccount";
    bWasSentAccount = false;
    bWasSentInbox = false;
    auto [nRequestNum, transactionNum, result] =
        api_.OTAPI().getAccountData(context_, Identifier::Factory(accountID));
    const auto& [status, reply] = result;
    [[maybe_unused]] const auto& notUsed1 = transactionNum;
    [[maybe_unused]] const auto& notUsed2 = nRequestNum;

    switch (status) {
        case SendResult::VALID_REPLY: {
            bWasSentAccount = true;
            bWasSentInbox = true;
            setLastReplyReceived(String::Factory(*reply)->Get());
        } break;
        case SendResult::TIMEOUT: {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to send getNymbox message due to error.")
                .Flush();
            setLastReplyReceived("");

            return -1;
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Error!").Flush();
            setLastReplyReceived("");

            return -1;
        }
    }

    const std::string notaryID = String::Factory(context_.Server())->Get();
    const std::string nymID = String::Factory(context_.Nym()->ID())->Get();

    // DOWNLOAD THE BOX RECEIPTS.
    if (!insureHaveAllBoxReceipts(
            notaryID,
            nymID,
            accountID,
            1))  // <===== nBoxType = 1 aka INBOX;
    {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": getAccountData succeeded, but then "
            "insureHaveAllBoxReceipts failed on the inbox. "
            "(I give up).")
            .Flush();
        return -1;
    }

    if (!insureHaveAllBoxReceipts(
            notaryID,
            nymID,
            accountID,
            2))  // <===== nBoxType = 2 aka OUTBOX;
    {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": getAccountData succeeded, but then "
            "insureHaveAllBoxReceipts failed on the "
            "outbox. (I give up).")
            .Flush();
        return -1;
    }

    return 1;
}

// Same as the above function, except you only have to pass the accountID.
// (instead of 3 IDs...)
//
bool Utility::getInboxOutboxAccount(const std::string& accountID)
{
    bool bForceDownload = false;
    return getInboxOutboxAccount(accountID, bForceDownload);
}

bool Utility::getInboxOutboxAccount(
    const std::string& accountID,
    bool bForceDownload)  // booleanbForceDownload = false;
{
    std::string strLocation = "Utility::getInboxOutboxAccount";

    if (!VerifyStringVal(accountID) || accountID.size() < 10) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": invalid accountID: ")(accountID)(
            ".")
            .Flush();
        return false;
    }

    std::string notaryID = api_.Exec().GetAccountWallet_NotaryID(accountID);
    std::string nymID = api_.Exec().GetAccountWallet_NymID(accountID);
    if (!getIntermediaryFiles(notaryID, nymID, accountID, bForceDownload)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": getIntermediaryFiles failed. (Returning).")
            .Flush();
        return false;
    }

    return true;
}

// getInboxAccount()
// Grabs the "Account", which is the intermediary file containing the current
// balance, verified against
// last signed receipt. Server must have your signature on the last balance
// agreement plus, if applicable,
// any inbox receipts (box receipts), also with your signature, in order to
// justify the current balance.
// Any inbox receipts, further, are only valid if they each contain a
// transaction number that was previously
// already signed out to you.
// (As you can see, the "account" is not a list of transactions, as per the
// classical understanding in
// double-entry accounting, but instead it's just a signed balance agreement,
// plus any as-yet-unclosed
// transactions that have cleared since that balance was last signed, and are
// still waiting in the inbox
// for the next balance agreement to be signed when they can be removed.)

// In addition to the "Account" there is also the Inbox itself, as well as all
// of its box receipts.
// The box receipts are stored in abbreviated form in the Inbox itself, with the
// actual full
// versions in separate files. These are retrieved individually from the server
// after the inbox itself
// is, and then each is verified against a hash kept inside its abbreviated
// version.)
// DONE
std::int32_t Utility::getInboxAccount(
    const std::string& accountID,
    bool& bWasSentInbox,
    bool& bWasSentAccount)
{
    bool bForceDownload = false;
    return getInboxAccount(
        accountID, bWasSentInbox, bWasSentAccount, bForceDownload);
}

}  // namespace opentxs
