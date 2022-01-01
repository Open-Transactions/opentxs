// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                 // IWYU pragma: associated
#include "1_Internal.hpp"               // IWYU pragma: associated
#include "otx/server/ReplyMessage.hpp"  // IWYU pragma: associated

#include <memory>
#include <string>

#include "internal/api/session/Wallet.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/NumList.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/otx/consensus/Client.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "otx/server/UserCommandProcessor.hpp"

namespace opentxs::server
{
ReplyMessage::ReplyMessage(
    const UserCommandProcessor& parent,
    const opentxs::api::session::Wallet& wallet,
    const identifier::Server& notaryID,
    const identity::Nym& signer,
    const Message& input,
    Server& server,
    const MessageType& type,
    Message& output,
    const PasswordPrompt& reason)
    : parent_(parent)
    , wallet_(wallet)
    , signer_(signer)
    , original_(input)
    , reason_(reason)
    , notary_id_(notaryID)
    , message_(output)
    , server_(server)
    , init_(false)
    , drop_(false)
    , drop_status_(false)
    , sender_nym_(nullptr)
    , context_(nullptr)
{
    message_.m_strRequestNum->Set(original_.m_strRequestNum);
    message_.m_strNotaryID->Set(original_.m_strNotaryID);
    message_.m_strNymID = original_.m_strNymID;
    message_.m_strCommand->Set(Message::ReplyCommand(type).c_str());
    message_.m_bSuccess = false;
    attach_request();
    init_ = init();
}

auto ReplyMessage::Acknowledged() const -> std::pmr::set<RequestNumber>
{
    std::pmr::set<RequestNumber> output{};
    original_.m_AcknowledgedReplies.Output(output);

    return output;
}

void ReplyMessage::attach_request()
{
    const std::string command = original_.m_strCommand->Get();
    const auto type = Message::Type(command);

    switch (type) {
        case MessageType::getMarketOffers:
        case MessageType::getMarketRecentTrades:
        case MessageType::getNymMarketOffers:
        case MessageType::registerContract:
        case MessageType::registerNym:
        case MessageType::unregisterNym:
        case MessageType::checkNym:
        case MessageType::registerInstrumentDefinition:
        case MessageType::queryInstrumentDefinitions:
        case MessageType::issueBasket:
        case MessageType::registerAccount:
        case MessageType::getBoxReceipt:
        case MessageType::getAccountData:
        case MessageType::unregisterAccount:
        case MessageType::notarizeTransaction:
        case MessageType::getNymbox:
        case MessageType::getInstrumentDefinition:
        case MessageType::getMint:
        case MessageType::processInbox:
        case MessageType::processNymbox:
        case MessageType::triggerClause:
        case MessageType::getMarketList:
        case MessageType::requestAdmin:
        case MessageType::addClaim: {
            LogVerbose()(OT_PRETTY_CLASS())("Attaching original ")(
                command)(" message.")
                .Flush();
            message_.m_ascInReferenceTo->SetString(String::Factory(original_));
        } break;
        case MessageType::pingNotary:
        case MessageType::usageCredits:
        case MessageType::sendNymMessage:
        case MessageType::getRequestNumber:
        case MessageType::getTransactionNumbers:
        default: {
        }
    }
}

void ReplyMessage::clear_request()
{
    const std::string command = original_.m_strCommand->Get();
    const auto type = Message::Type(command);

    switch (type) {
        case MessageType::checkNym:
        case MessageType::getNymbox:
        case MessageType::getAccountData:
        case MessageType::getInstrumentDefinition:
        case MessageType::getMint: {
            LogVerbose()(OT_PRETTY_CLASS())("Clearing original ")(
                command)(" message.")
                .Flush();
            message_.m_ascInReferenceTo->Release();
        } break;
        case MessageType::getMarketOffers:
        case MessageType::getMarketRecentTrades:
        case MessageType::getNymMarketOffers:
        case MessageType::registerContract:
        case MessageType::registerNym:
        case MessageType::unregisterNym:
        case MessageType::registerInstrumentDefinition:
        case MessageType::queryInstrumentDefinitions:
        case MessageType::issueBasket:
        case MessageType::registerAccount:
        case MessageType::getBoxReceipt:
        case MessageType::unregisterAccount:
        case MessageType::notarizeTransaction:
        case MessageType::processInbox:
        case MessageType::processNymbox:
        case MessageType::triggerClause:
        case MessageType::getMarketList:
        case MessageType::requestAdmin:
        case MessageType::addClaim:
        case MessageType::pingNotary:
        case MessageType::usageCredits:
        case MessageType::sendNymMessage:
        case MessageType::getRequestNumber:
        case MessageType::getTransactionNumbers:
        default: {
        }
    }
}

void ReplyMessage::ClearRequest() { message_.m_ascInReferenceTo->Release(); }

auto ReplyMessage::Context() -> otx::context::Client&
{
    OT_ASSERT(context_);

    return context_->get();
}

// REPLY NOTICE TO NYMBOX
//
// After specific messages, we drop a notice with a copy of the server's
// reply into the Nymbox. This way we are GUARANTEED that the Nym will
// receive and process it. (And thus never get out of sync.)
void ReplyMessage::DropToNymbox(const bool success)
{
    drop_ = true;
    drop_status_ = success;
}

auto ReplyMessage::HaveContext() const -> bool { return bool(context_); }

auto ReplyMessage::init() -> bool
{
    const auto senderNymID = identifier::Nym::Factory(original_.m_strNymID);
    const auto purportedServerID =
        identifier::Server::Factory(original_.m_strNotaryID);

    bool out = UserCommandProcessor::check_server_lock(senderNymID);

    if (out) {
        out &= UserCommandProcessor::check_message_notary(
            purportedServerID, notary_id_);
    }

    if (out) {
        out &= UserCommandProcessor::check_client_isnt_server(
            senderNymID, signer_);
    }

    return out;
}

auto ReplyMessage::Init() const -> const bool& { return init_; }

auto ReplyMessage::init_nym() -> bool
{
    sender_nym_ = wallet_.Nym(identifier::Nym::Factory(original_.m_strNymID));

    return bool(sender_nym_);
}

auto ReplyMessage::LoadContext(const PasswordPrompt& reason) -> bool
{
    if (false == init_nym()) {
        LogError()(OT_PRETTY_CLASS())("Nym (")(original_.m_strNymID)(
            ") does not exist")
            .Flush();

        return false;
    }

    context_ = std::make_unique<Editor<otx::context::Client>>(
        wallet_.Internal().mutable_ClientContext(sender_nym_->ID(), reason));

    return bool(context_);
}

auto ReplyMessage::Original() const -> const Message& { return original_; }

void ReplyMessage::OverrideType(const String& replyCommand)
{
    message_.m_strCommand = replyCommand;
}

void ReplyMessage::SetAccount(const String& accountID)
{
    OT_ASSERT(accountID.Exists());

    message_.m_strAcctID = accountID;
}

void ReplyMessage::SetBool(const bool value) { message_.m_bBool = value; }

void ReplyMessage::SetAcknowledgments(const otx::context::Client& context)
{
    message_.SetAcknowledgments(context);
}

void ReplyMessage::SetDepth(const std::int64_t depth)
{
    message_.m_lDepth = depth;
}

void ReplyMessage::SetEnum(const std::uint8_t value) { message_.enum_ = value; }

void ReplyMessage::SetInboxHash(const Identifier& hash)
{
    message_.m_strInboxHash = String::Factory(hash);
}

void ReplyMessage::SetInstrumentDefinitionID(const String& id)
{
    message_.m_strInstrumentDefinitionID = id;
}

void ReplyMessage::SetNymboxHash(const Identifier& hash)
{
    hash.GetString(message_.m_strNymboxHash);
}

void ReplyMessage::SetOutboxHash(const Identifier& hash)
{
    message_.m_strOutboxHash = String::Factory(hash);
}

auto ReplyMessage::SetPayload(const String& payload) -> bool
{
    return message_.m_ascPayload->SetString(payload);
}

auto ReplyMessage::SetPayload(const Data& payload) -> bool
{
    return message_.m_ascPayload->SetData(payload);
}

void ReplyMessage::SetPayload(const Armored& payload)
{
    message_.m_ascPayload = payload;
}

auto ReplyMessage::SetPayload2(const String& payload) -> bool
{
    return message_.m_ascPayload2->SetString(payload);
}

auto ReplyMessage::SetPayload3(const String& payload) -> bool
{
    return message_.m_ascPayload3->SetString(payload);
}

void ReplyMessage::SetRequestNumber(const RequestNumber number)
{
    message_.m_lNewRequestNum = number;
}

void ReplyMessage::SetSuccess(const bool success)
{
    message_.m_bSuccess = success;

    if (success) { clear_request(); }
}

void ReplyMessage::SetTransactionNumber(const TransactionNumber& number)
{
    message_.m_lTransactionNum = number;
}

auto ReplyMessage::Success() const -> const bool&
{
    return message_.m_bSuccess;
}

void ReplyMessage::SetTargetNym(const String& nymID)
{
    message_.m_strNymID2 = nymID;
}

ReplyMessage::~ReplyMessage()
{
    if (drop_ && context_) {
        parent_.drop_reply_notice_to_nymbox(
            wallet_,
            message_,
            original_.m_strRequestNum->ToLong(),
            drop_status_,
            context_->get(),
            server_);
    }

    if (context_) { SetNymboxHash(context_->get().LocalNymboxHash()); }

    message_.SignContract(signer_, reason_);
    message_.SaveContract();
}
}  // namespace opentxs::server
