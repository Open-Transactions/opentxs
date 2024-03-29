// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "otx/server/ReplyMessage.hpp"  // IWYU pragma: associated

#include <memory>

#include "internal/core/Armored.hpp"
#include "internal/core/String.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/otx/common/NumList.hpp"
#include "internal/otx/consensus/Client.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Notary.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/api/session/Wallet.internal.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Notary.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/otx/Types.internal.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "otx/server/Server.hpp"
#include "otx/server/UserCommandProcessor.hpp"

namespace opentxs::server
{
ReplyMessage::ReplyMessage(
    const UserCommandProcessor& parent,
    const opentxs::api::session::Wallet& wallet,
    const identifier::Notary& notaryID,
    const identity::Nym& signer,
    const Message& input,
    Server& server,
    const otx::MessageType& type,
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
    message_.request_num_->Set(original_.request_num_);
    message_.notary_id_->Set(original_.notary_id_);
    message_.nym_id_ = original_.nym_id_;
    message_.command_->Set(Message::ReplyCommand(type).c_str());
    message_.success_ = false;
    attach_request();
    init_ = init();
}

auto ReplyMessage::Acknowledged() const -> UnallocatedSet<RequestNumber>
{
    UnallocatedSet<RequestNumber> output{};
    original_.acknowledged_replies_.Output(output);

    return output;
}

void ReplyMessage::attach_request()
{
    const UnallocatedCString command = original_.command_->Get();
    const auto type = Message::Type(command);

    switch (type) {
        case otx::MessageType::getMarketOffers:
        case otx::MessageType::getMarketRecentTrades:
        case otx::MessageType::getNymMarketOffers:
        case otx::MessageType::registerContract:
        case otx::MessageType::registerNym:
        case otx::MessageType::unregisterNym:
        case otx::MessageType::checkNym:
        case otx::MessageType::registerInstrumentDefinition:
        case otx::MessageType::queryInstrumentDefinitions:
        case otx::MessageType::issueBasket:
        case otx::MessageType::registerAccount:
        case otx::MessageType::getBoxReceipt:
        case otx::MessageType::getAccountData:
        case otx::MessageType::unregisterAccount:
        case otx::MessageType::notarizeTransaction:
        case otx::MessageType::getNymbox:
        case otx::MessageType::getInstrumentDefinition:
        case otx::MessageType::getMint:
        case otx::MessageType::processInbox:
        case otx::MessageType::processNymbox:
        case otx::MessageType::triggerClause:
        case otx::MessageType::getMarketList:
        case otx::MessageType::requestAdmin:
        case otx::MessageType::addClaim: {
            LogVerbose()()("Attaching original ")(command)(" message.").Flush();
            message_.in_reference_to_->SetString(String::Factory(original_));
        } break;
        case otx::MessageType::pingNotary:
        case otx::MessageType::usageCredits:
        case otx::MessageType::sendNymMessage:
        case otx::MessageType::getRequestNumber:
        case otx::MessageType::getTransactionNumbers:
        default: {
        }
    }
}

void ReplyMessage::clear_request()
{
    const UnallocatedCString command = original_.command_->Get();
    const auto type = Message::Type(command);

    switch (type) {
        case otx::MessageType::checkNym:
        case otx::MessageType::getNymbox:
        case otx::MessageType::getAccountData:
        case otx::MessageType::getInstrumentDefinition:
        case otx::MessageType::getMint: {
            LogVerbose()()("Clearing original ")(command)(" message.").Flush();
            message_.in_reference_to_->Release();
        } break;
        case otx::MessageType::getMarketOffers:
        case otx::MessageType::getMarketRecentTrades:
        case otx::MessageType::getNymMarketOffers:
        case otx::MessageType::registerContract:
        case otx::MessageType::registerNym:
        case otx::MessageType::unregisterNym:
        case otx::MessageType::registerInstrumentDefinition:
        case otx::MessageType::queryInstrumentDefinitions:
        case otx::MessageType::issueBasket:
        case otx::MessageType::registerAccount:
        case otx::MessageType::getBoxReceipt:
        case otx::MessageType::unregisterAccount:
        case otx::MessageType::notarizeTransaction:
        case otx::MessageType::processInbox:
        case otx::MessageType::processNymbox:
        case otx::MessageType::triggerClause:
        case otx::MessageType::getMarketList:
        case otx::MessageType::requestAdmin:
        case otx::MessageType::addClaim:
        case otx::MessageType::pingNotary:
        case otx::MessageType::usageCredits:
        case otx::MessageType::sendNymMessage:
        case otx::MessageType::getRequestNumber:
        case otx::MessageType::getTransactionNumbers:
        default: {
        }
    }
}

void ReplyMessage::ClearRequest() { message_.in_reference_to_->Release(); }

auto ReplyMessage::Context() -> otx::context::Client&
{
    assert_false(nullptr == context_);

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
    const auto senderNymID = parent_.server_.API().Factory().NymIDFromBase58(
        original_.nym_id_->Bytes());
    const auto purportedServerID =
        parent_.server_.API().Factory().NotaryIDFromBase58(
            original_.notary_id_->Bytes());

    bool out = UserCommandProcessor::check_server_lock(
        parent_.server_.API(), senderNymID);

    if (out) {
        out &= UserCommandProcessor::check_message_notary(
            parent_.server_.API().Crypto(), purportedServerID, notary_id_);
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
    sender_nym_ = wallet_.Nym(parent_.server_.API().Factory().NymIDFromBase58(
        original_.nym_id_->Bytes()));

    return bool(sender_nym_);
}

auto ReplyMessage::LoadContext(const PasswordPrompt& reason) -> bool
{
    if (false == init_nym()) {
        LogError()()("Nym (")(original_.nym_id_.get())(") does not exist")
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
    message_.command_ = replyCommand;
}

void ReplyMessage::SetAccount(const String& accountID)
{
    assert_true(accountID.Exists());

    message_.acct_id_ = accountID;
}

void ReplyMessage::SetBool(const bool value) { message_.bool_ = value; }

void ReplyMessage::SetAcknowledgments(const otx::context::Client& context)
{
    message_.SetAcknowledgments(context);
}

void ReplyMessage::SetDepth(const std::int64_t depth)
{
    message_.depth_ = depth;
}

void ReplyMessage::SetEnum(const std::uint8_t value) { message_.enum_ = value; }

void ReplyMessage::SetInboxHash(const identifier::Generic& hash)
{
    message_.inbox_hash_ =
        String::Factory(hash, parent_.server_.API().Crypto());
}

void ReplyMessage::SetInstrumentDefinitionID(const String& id)
{
    message_.instrument_definition_id_ = id;
}

void ReplyMessage::SetNymboxHash(const identifier::Generic& hash)
{
    hash.GetString(parent_.server_.API().Crypto(), message_.nymbox_hash_);
}

void ReplyMessage::SetOutboxHash(const identifier::Generic& hash)
{
    message_.outbox_hash_ =
        String::Factory(hash, parent_.server_.API().Crypto());
}

auto ReplyMessage::SetPayload(const String& payload) -> bool
{
    return message_.payload_->SetString(payload);
}

auto ReplyMessage::SetPayload(const Data& payload) -> bool
{
    return message_.payload_->SetData(payload);
}

void ReplyMessage::SetPayload(const Armored& payload)
{
    message_.payload_ = payload;
}

auto ReplyMessage::SetPayload2(const String& payload) -> bool
{
    return message_.payload2_->SetString(payload);
}

auto ReplyMessage::SetPayload3(const String& payload) -> bool
{
    return message_.payload3_->SetString(payload);
}

void ReplyMessage::SetRequestNumber(const RequestNumber number)
{
    message_.new_request_num_ = number;
}

void ReplyMessage::SetSuccess(const bool success)
{
    message_.success_ = success;

    if (success) { clear_request(); }
}

void ReplyMessage::SetTransactionNumber(const TransactionNumber& number)
{
    message_.transaction_num_ = number;
}

auto ReplyMessage::Success() const -> const bool& { return message_.success_; }

void ReplyMessage::SetTargetNym(const String& nymID)
{
    message_.nym_id2_ = nymID;
}

ReplyMessage::~ReplyMessage()
{
    if (drop_ && context_) {
        parent_.drop_reply_notice_to_nymbox(
            wallet_,
            message_,
            original_.request_num_->ToLong(),
            drop_status_,
            context_->get(),
            server_);
    }

    if (context_) { SetNymboxHash(context_->get().LocalNymboxHash()); }

    message_.SignContract(signer_, reason_);
    message_.SaveContract();
}
}  // namespace opentxs::server
