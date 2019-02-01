// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/client/OTClient.hpp"

#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/OTX.hpp"
#include "opentxs/api/client/Workflow.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/api/Wallet.hpp"
#if OT_CASH
#include "opentxs/blind/Mint.hpp"
#include "opentxs/blind/Purse.hpp"
#endif  // OT_CASH
#include "opentxs/client/Helpers.hpp"
#include "opentxs/client/OTMessageOutbuffer.hpp"
#include "opentxs/client/OTWallet.hpp"
#include "opentxs/consensus/ManagedNumber.hpp"
#include "opentxs/consensus/ServerContext.hpp"
#include "opentxs/consensus/TransactionStatement.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/contract/basket/Basket.hpp"
#include "opentxs/core/contract/peer/PeerObject.hpp"
#include "opentxs/core/crypto/OTEnvelope.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/core/recurring/OTPaymentPlan.hpp"
#include "opentxs/core/trade/OTOffer.hpp"
#include "opentxs/core/trade/OTTrade.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/OTFolders.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Item.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/OTTransactionType.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/ext/OTPayment.hpp"
#include "opentxs/otx/Reply.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/OT.hpp"

#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>

#define OT_METHOD "opentxs::OTClient::"

namespace opentxs
{
OTClient::OTClient(const api::Core& core)
    : api_(core)
    , m_MessageOutbuffer(api_)
{
    // WARNING: do not access api_.Wallet() during construction
}

/// This function sets up "theMessage" so that it is ready to be sent out to
/// the server. If you want to set up a pingNotary command and send it to
/// the server, then you just call this to get the OTMessage object all set
/// up and ready to be sent.
//
/// returns -1 if error, don't send message.
/// returns  0 if NO error, but still, don't send message.
/// returns 1 if message is sent but there's not request number
/// returns >0 for processInbox, containing the number that was there before
/// processing.
/// returns >0 for nearly everything else, containing the request number
/// itself.
std::int32_t OTClient::ProcessUserCommand(
    const MessageType requestedCommand,
    ServerContext& context,
    Message& theMessage,
    const identifier::Nym& pHisNymID,
    const Identifier& pHisAcctID,
    const Amount lTransactionAmount,
    const Account* pAccount,
    const UnitDefinition* pMyUnitDefinition)
{
    // This is all preparatory work to get the various pieces of data
    // together
    // -- only then can we put those pieces into a message.
    RequestNumber lRequestNumber{0};
    const auto& nym = *context.Nym();

    if (nullptr != pAccount) {
        if (pAccount->GetPurportedNotaryID() != context.Server()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": pAccount->GetPurportedNotaryID() doesn't match "
                "NOTARY_ID. (Try adding: --server NOTARY_ID).")
                .Flush();

            return -1;
        }

        pAccount->GetIdentifier(theMessage.m_strAcctID);
    }

    theMessage.m_strNymID = String::Factory(nym.ID());
    theMessage.m_strNotaryID = String::Factory(context.Server());
    std::int64_t lReturnValue = 0;

    switch (requestedCommand) {
        // EVERY COMMAND BELOW THIS POINT (THEY ARE ALL OUTGOING TO THE
        // SERVER) MUST INCLUDE THE CORRECT REQUEST NUMBER, OR BE REJECTED
        // BY THE SERVER.
        //
        // The same commands must also increment the local counter of the
        // request number by calling theNym.IncrementRequestNum Otherwise it
        // will get out of sync, and future commands will start failing
        // (until it is resynchronized with a getRequestNumber message to
        // the server, which replies with the latest number. The code on
        // this side that processes that server reply is already smart
        // enough to update the local nym's copy of the request number when
        // it is received. In this way, the client becomes resynchronized
        // and the next command will work again. But it's better to
        // increment the counter properly. PROPERLY == every time you
        // actually get the request number from a nym and use it to make a
        // server request, then you should therefore also increment that
        // counter. If you call GetCurrentRequestNum AND USE IT WITH THE
        // SERVER, then make sure you call IncrementRequestNum immediately
        // after. Otherwise future commands will start failing.
        //
        // This is all because the server requres a new request number (last
        // one +1) with each request. This is in order to thwart would-be
        // attackers who cannot break the crypto, but try to capture
        // encrypted messages and send them to the server twice. Better that
        // new requests requre new request numbers :-)
        case MessageType::unregisterNym: {
            // (0) Set up the REQUEST NUMBER and then INCREMENT IT
            lRequestNumber = context.Request();
            theMessage.m_strRequestNum->Format("%" PRId64 "", lRequestNumber);
            context.IncrementRequest();

            // (1) set up member variables
            theMessage.m_strCommand->Set("unregisterNym");
            theMessage.SetAcknowledgments(context);

            // (2) Sign the Message
            theMessage.SignContract(nym);

            // (3) Save the Message (with signatures and all, back to its
            // internal
            // member m_strRawFile.)
            theMessage.SaveContract();

            lReturnValue = lRequestNumber;
        } break;
        case MessageType::processNymbox:  // PROCESS NYMBOX
        {
            // (0) Set up the REQUEST NUMBER and then INCREMENT IT
            lRequestNumber = context.Request();
            theMessage.m_strRequestNum->Format("%" PRId64 "", lRequestNumber);
            context.IncrementRequest();

            // (1) Set up member variables
            theMessage.m_strCommand = String::Factory("processNymbox");
            theMessage.SetAcknowledgments(context);
            auto NYMBOX_HASH = Identifier::Factory(context.LocalNymboxHash());
            NYMBOX_HASH->GetString(theMessage.m_strNymboxHash);

            if (!String::Factory(NYMBOX_HASH)->Exists()) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Failed getting NymboxHash from Nym for server: ")(
                    context.Server())(".")
                    .Flush();
            }

            // (2) Sign the Message
            theMessage.SignContract(nym);

            // (3) Save the Message (with signatures and all, back to its
            // internal
            // member m_strRawFile.)
            theMessage.SaveContract();

            lReturnValue = lRequestNumber;
        }

        // This is called by the user of the command line utility.
        //
        break;
        case MessageType::getTransactionNumbers:  // GET TRANSACTION NUM
        {
            // (0) Set up the REQUEST NUMBER and then INCREMENT IT
            lRequestNumber = context.Request();
            theMessage.m_strRequestNum->Format("%" PRId64 "", lRequestNumber);
            context.IncrementRequest();

            // (1) Set up member variables
            theMessage.m_strCommand = String::Factory("getTransactionNumbers");
            theMessage.SetAcknowledgments(context);
            auto NYMBOX_HASH = Identifier::Factory(context.LocalNymboxHash());
            NYMBOX_HASH->GetString(theMessage.m_strNymboxHash);

            if (NYMBOX_HASH->IsEmpty()) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Failed getting NymboxHash from Nym for server: ")(
                    context.Server())(".")
                    .Flush();
            }

            // (2) Sign the Message
            theMessage.SignContract(nym);

            // (3) Save the Message (with signatures and all, back to its
            // internal member m_strRawFile.)
            theMessage.SaveContract();

            lReturnValue = lRequestNumber;
        } break;
        default: {
            LogNormal(OT_METHOD)(__FUNCTION__).Flush();
        }
    }

    return static_cast<std::int32_t>(lReturnValue);
}

void OTClient::QueueOutgoingMessage(const Message& theMessage)
{
    auto serialized = String::Factory();
    const bool saved = theMessage.SaveContractRaw(serialized);

    OT_ASSERT(saved)

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
    // later in the Nymbox, and then worst case, look it up in the Outbuffer
    // and get my fucking transaction numbers back again!
    auto pMsg = api_.Factory().Message();

    OT_ASSERT((pMsg));

    if (pMsg->LoadContractFromString(serialized)) {
        std::shared_ptr<Message> msg{pMsg.release()};
        m_MessageOutbuffer.AddSentMessage(msg);
    } else {
        OT_FAIL
    }
}

}  // namespace opentxs
