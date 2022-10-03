// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                               // IWYU pragma: associated
#include "internal/otx/client/obsolete/OTClient.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <memory>

#include "internal/otx/Types.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/otx/consensus/Server.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace opentxs
{
OTClient::OTClient(const api::Session& api)
    : api_(api)
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
auto OTClient::ProcessUserCommand(
    const MessageType requestedCommand,
    otx::context::Server& context,
    Message& theMessage,
    const identifier::Generic& pHisNymID,
    const identifier::Generic& pHisAcctID,
    const PasswordPrompt& reason,
    const Amount& lTransactionAmount,
    const Account* pAccount,
    const contract::Unit* pMyUnitDefinition

    ) -> std::int32_t
{
    // This is all preparatory work to get the various pieces of data
    // together
    // -- only then can we put those pieces into a message.
    RequestNumber lRequestNumber{0};
    const auto& nym = *context.Nym();

    if (nullptr != pAccount) {
        if (pAccount->GetPurportedNotaryID() != context.Notary()) {
            LogError()(OT_PRETTY_CLASS())(
                "pAccount->GetPurportedNotaryID() doesn't match "
                "NOTARY_ID. (Try adding: --server NOTARY_ID).")
                .Flush();

            return -1;
        }

        pAccount->GetIdentifier(theMessage.acct_id_);
    }

    theMessage.nym_id_ = String::Factory(nym.ID());
    theMessage.notary_id_ = String::Factory(context.Notary());
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
            theMessage.request_num_->Set(
                std::to_string(lRequestNumber).c_str());
            context.IncrementRequest();

            // (1) set up member variables
            theMessage.command_->Set("unregisterNym");
            theMessage.SetAcknowledgments(context);

            // (2) Sign the Message
            theMessage.SignContract(nym, reason);

            // (3) Save the Message (with signatures and all, back to its
            // internal
            // member raw_file_.)
            theMessage.SaveContract();

            lReturnValue = lRequestNumber;
        } break;
        case MessageType::processNymbox:  // PROCESS NYMBOX
        {
            // (0) Set up the REQUEST NUMBER and then INCREMENT IT
            lRequestNumber = context.Request();
            theMessage.request_num_->Set(
                std::to_string(lRequestNumber).c_str());
            context.IncrementRequest();

            // (1) Set up member variables
            theMessage.command_ = String::Factory("processNymbox");
            theMessage.SetAcknowledgments(context);
            const auto& NYMBOX_HASH = context.LocalNymboxHash();
            NYMBOX_HASH.GetString(api_.Crypto(), theMessage.nymbox_hash_);

            if (!String::Factory(NYMBOX_HASH)->Exists()) {
                LogError()(OT_PRETTY_CLASS())(
                    "Failed getting NymboxHash from Nym for server: ")(
                    context.Notary())(".")
                    .Flush();
            }

            // (2) Sign the Message
            theMessage.SignContract(nym, reason);

            // (3) Save the Message (with signatures and all, back to its
            // internal
            // member raw_file_.)
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
            theMessage.request_num_->Set(
                std::to_string(lRequestNumber).c_str());
            context.IncrementRequest();

            // (1) Set up member variables
            theMessage.command_ = String::Factory("getTransactionNumbers");
            theMessage.SetAcknowledgments(context);
            const auto& NYMBOX_HASH = context.LocalNymboxHash();
            NYMBOX_HASH.GetString(api_.Crypto(), theMessage.nymbox_hash_);

            if (NYMBOX_HASH.empty()) {
                LogError()(OT_PRETTY_CLASS())(
                    "Failed getting NymboxHash from Nym for server: ")(
                    context.Notary())(".")
                    .Flush();
            }

            // (2) Sign the Message
            theMessage.SignContract(nym, reason);

            // (3) Save the Message (with signatures and all, back to its
            // internal member raw_file_.)
            theMessage.SaveContract();

            lReturnValue = lRequestNumber;
        } break;
        default: {
            LogConsole()(OT_PRETTY_CLASS()).Flush();
        }
    }

    return static_cast<std::int32_t>(lReturnValue);
}
}  // namespace opentxs
