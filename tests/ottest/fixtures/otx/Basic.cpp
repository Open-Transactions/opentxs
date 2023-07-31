// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/otx/Basic.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <algorithm>
#include <ctime>
#include <future>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "2_Factory.hpp"
#include "internal/api/session/Client.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/String.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/otx/client/Client.hpp"
#include "internal/otx/client/Pair.hpp"
#include "internal/otx/client/obsolete/OT_API.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/otx/common/Ledger.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/otx/consensus/Client.hpp"
#include "internal/otx/consensus/Server.hpp"
#include "internal/util/Editor.hpp"
#include "internal/util/LogMacros.hpp"
#include "ottest/env/OTTestEnvironment.hpp"
#include "otx/server/Server.hpp"
#include "otx/server/Transactor.hpp"

namespace ottest
{
namespace ot = opentxs;

using namespace std::literals;

ot::RequestNumber Basic::alice_counter_{0};
ot::RequestNumber Basic::bob_counter_{0};
const ot::crypto::SeedID Basic::SeedA_{};
const ot::crypto::SeedID Basic::SeedB_{};
const ot::identifier::Nym Basic::alice_nym_id_{};
const ot::identifier::Nym Basic::bob_nym_id_{};
ot::TransactionNumber Basic::cheque_transaction_number_{0};
ot::UnallocatedCString Basic::bob_account_1_id_{""};
ot::UnallocatedCString Basic::bob_account_2_id_{""};
ot::UnallocatedCString Basic::outgoing_cheque_workflow_id_{};
ot::UnallocatedCString Basic::incoming_cheque_workflow_id_{};
ot::UnallocatedCString Basic::outgoing_transfer_workflow_id_{};
ot::UnallocatedCString Basic::incoming_transfer_workflow_id_{};
ot::UnallocatedCString Basic::internal_transfer_workflow_id_{};
const ot::UnallocatedCString Basic::unit_id_1_{};
const ot::UnallocatedCString Basic::unit_id_2_{};
std::unique_ptr<ot::otx::client::internal::Operation>
    Basic::alice_state_machine_{nullptr};
std::unique_ptr<ot::otx::client::internal::Operation> Basic::bob_state_machine_{
    nullptr};
std::shared_ptr<ot::otx::blind::Purse> Basic::untrusted_purse_{};
bool Basic::init_{false};

Basic::matchID::matchID(const ot::UnallocatedCString& id)
    : id_{id}
{
}

auto Basic::matchID::operator()(
    const std::pair<ot::UnallocatedCString, ot::UnallocatedCString>& id) -> bool
{
    return id.first == id_;
}

Basic::Basic()
    : client_1_(dynamic_cast<const ot::api::session::Client&>(
          OTTestEnvironment::GetOT().StartClientSession(0)))
    , client_2_(dynamic_cast<const ot::api::session::Client&>(
          OTTestEnvironment::GetOT().StartClientSession(1)))
    , server_1_(dynamic_cast<const ot::api::session::Notary&>(
          OTTestEnvironment::GetOT().StartNotarySession(0)))
    , server_2_(dynamic_cast<const ot::api::session::Notary&>(
          OTTestEnvironment::GetOT().StartNotarySession(1)))
    , reason_c1_(client_1_.Factory().PasswordPrompt(__func__))
    , reason_c2_(client_2_.Factory().PasswordPrompt(__func__))
    , reason_s1_(server_1_.Factory().PasswordPrompt(__func__))
    , reason_s2_(server_2_.Factory().PasswordPrompt(__func__))
    , asset_contract_1_(load_unit(client_1_, unit_id_1_))
    , asset_contract_2_(load_unit(client_2_, unit_id_2_))
    , server_1_id_(dynamic_cast<const ot::identifier::Notary&>(server_1_.ID()))
    , server_2_id_(dynamic_cast<const ot::identifier::Notary&>(server_2_.ID()))
    , server_contract_(server_1_.Wallet().Internal().Server(server_1_id_))
    , extra_args_()
{
    if (false == init_) { init(); }
}

auto Basic::find_id(
    const ot::UnallocatedCString& id,
    const ot::ObjectList& list) -> bool
{
    matchID matchid(id);

    return std::find_if(list.begin(), list.end(), matchid) != list.end();
}

auto Basic::load_unit(
    const ot::api::Session& api,
    const ot::UnallocatedCString& id) noexcept -> ot::OTUnitDefinition
{
    try {
        return api.Wallet().Internal().UnitDefinition(
            api.Factory().UnitIDFromBase58(id));
    } catch (...) {
        return api.Factory().InternalSession().UnitDefinition();
    }
}

auto Basic::translate_result(const ot::otx::LastReplyStatus status)
    -> ot::otx::client::SendResult
{
    switch (status) {
        case ot::otx::LastReplyStatus::MessageSuccess:
        case ot::otx::LastReplyStatus::MessageFailed: {

            return ot::otx::client::SendResult::VALID_REPLY;
        }
        case ot::otx::LastReplyStatus::Unknown: {

            return ot::otx::client::SendResult::TIMEOUT;
        }
        case ot::otx::LastReplyStatus::NotSent: {

            return ot::otx::client::SendResult::UNNECESSARY;
        }
        case ot::otx::LastReplyStatus::Invalid:
        case ot::otx::LastReplyStatus::None:
        default: {

            return ot::otx::client::SendResult::Error;
        }
    }
}

void Basic::break_consensus()
{
    ot::TransactionNumber newNumber{0};
    server_1_.Server().GetTransactor().issueNextTransactionNumber(newNumber);

    auto context = server_1_.Wallet().Internal().mutable_ClientContext(
        alice_nym_id_, reason_s1_);
    context.get().IssueNumber(newNumber);
}

void Basic::import_server_contract(
    const ot::contract::Server& contract,
    const ot::api::session::Client& client)
{
    auto bytes = ot::Space{};
    EXPECT_TRUE(server_contract_->Serialize(ot::writer(bytes), true));

    auto clientVersion = client.Wallet().Internal().Server(ot::reader(bytes));
    client.OTX().SetIntroductionServer(clientVersion);
}

void Basic::init()
{
    client_1_.OTX().DisableAutoaccept();
    client_1_.InternalClient().Pair().Stop().get();
    client_2_.OTX().DisableAutoaccept();
    client_2_.InternalClient().Pair().Stop().get();
    const_cast<ot::crypto::SeedID&>(SeedA_) = client_1_.Crypto().Seed().ImportSeed(
        client_1_.Factory().SecretFromText(
            "spike nominee miss inquiry fee nothing belt list other daughter leave valley twelve gossip paper"sv),
        client_1_.Factory().SecretFromText(""sv),
        opentxs::crypto::SeedStyle::BIP39,
        opentxs::crypto::Language::en,
        client_1_.Factory().PasswordPrompt("Importing a BIP-39 seed"));
    const_cast<ot::crypto::SeedID&>(SeedB_) = client_2_.Crypto().Seed().ImportSeed(
        client_2_.Factory().SecretFromText(
            "trim thunder unveil reduce crop cradle zone inquiry anchor skate property fringe obey butter text tank drama palm guilt pudding laundry stay axis prosper"sv),
        client_2_.Factory().SecretFromText(""sv),
        opentxs::crypto::SeedStyle::BIP39,
        opentxs::crypto::Language::en,
        client_2_.Factory().PasswordPrompt("Importing a BIP-39 seed"));
    const_cast<ot::identifier::Nym&>(alice_nym_id_) =
        client_1_.Wallet()
            .Nym({client_1_.Factory(), SeedA_, 0}, reason_c1_, "Alice")
            ->ID();
    const_cast<ot::identifier::Nym&>(bob_nym_id_) =
        client_2_.Wallet()
            .Nym({client_2_.Factory(), SeedB_, 0}, reason_c2_, "Bob")
            ->ID();

    OT_ASSERT(false == server_1_id_.empty());

    import_server_contract(server_contract_, client_1_);
    import_server_contract(server_contract_, client_2_);

    alice_state_machine_.reset(ot::Factory::Operation(
        client_1_, alice_nym_id_, server_1_id_, reason_c1_));

    OT_ASSERT(alice_state_machine_);

    alice_state_machine_->SetPush(false);
    bob_state_machine_.reset(ot::Factory::Operation(
        client_2_, bob_nym_id_, server_1_id_, reason_c2_));

    OT_ASSERT(bob_state_machine_);

    bob_state_machine_->SetPush(false);
    init_ = true;
}

void Basic::create_unit_definition_1()
{
    const_cast<ot::OTUnitDefinition&>(asset_contract_1_) =
        client_1_.Wallet().Internal().CurrencyContract(
            alice_nym_id_.asBase58(client_1_.Crypto()),
            UNIT_DEFINITION_CONTRACT_NAME,
            UNIT_DEFINITION_TERMS,
            UNIT_DEFINITION_UNIT_OF_ACCOUNT,
            1,
            reason_c1_);
    EXPECT_EQ(ot::contract::UnitType::Currency, asset_contract_1_->Type());

    if (asset_contract_1_->ID().empty()) {
        throw std::runtime_error("Failed to create unit definition 1");
    }

    const_cast<ot::UnallocatedCString&>(unit_id_1_) =
        asset_contract_1_->ID().asBase58(client_1_.Crypto());
}

void Basic::create_unit_definition_2()
{
    const_cast<ot::OTUnitDefinition&>(asset_contract_2_) =
        client_2_.Wallet().Internal().CurrencyContract(
            bob_nym_id_.asBase58(client_1_.Crypto()),
            UNIT_DEFINITION_CONTRACT_NAME_2,
            UNIT_DEFINITION_TERMS_2,
            UNIT_DEFINITION_UNIT_OF_ACCOUNT_2,
            1,
            reason_c2_);
    EXPECT_EQ(ot::contract::UnitType::Currency, asset_contract_2_->Type());

    if (asset_contract_2_->ID().empty()) {
        throw std::runtime_error("Failed to create unit definition 2");
    }

    const_cast<ot::UnallocatedCString&>(unit_id_2_) =
        asset_contract_2_->ID().asBase58(client_1_.Crypto());
}

auto Basic::find_issuer_account() -> ot::identifier::Account
{
    const auto accounts = client_1_.Storage().AccountsByOwner(alice_nym_id_);

    OT_ASSERT(1 == accounts.size());

    return *accounts.begin();
}

auto Basic::find_unit_definition_id_1() -> ot::identifier::UnitDefinition
{
    const auto accountID = find_issuer_account();

    OT_ASSERT(false == accountID.empty());

    const auto output = client_1_.Storage().AccountContract(accountID);

    OT_ASSERT(false == output.empty());

    return output;
}

auto Basic::find_unit_definition_id_2() -> ot::identifier::UnitDefinition
{
    return asset_contract_2_->ID();
}

auto Basic::find_user_account() -> ot::identifier::Account
{
    return client_2_.Factory().AccountIDFromBase58(bob_account_1_id_);
}

auto Basic::find_second_user_account() -> ot::identifier::Account
{
    return client_2_.Factory().AccountIDFromBase58(bob_account_2_id_);
}

void Basic::receive_reply(
    const std::shared_ptr<const ot::identity::Nym>& recipient,
    const std::shared_ptr<const ot::identity::Nym>& sender,
    const ot::contract::peer::Reply& peerreply,
    const ot::contract::peer::Request& peerrequest,
    ot::contract::peer::RequestType requesttype)
{
    const ot::RequestNumber sequence = alice_counter_;
    const ot::RequestNumber messages{4};
    alice_counter_ += messages;
    auto serverContext = client_1_.Wallet().Internal().mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto& context = serverContext.get();
    auto clientContext =
        server_1_.Wallet().Internal().ClientContext(alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, context, sequence);
    auto queue = context.RefreshNymbox(client_1_, reason_c1_);

    ASSERT_TRUE(queue);

    const auto finished = queue->get();
    context.Join();
    context.ResetThread();
    const auto& [status, message] = finished;

    EXPECT_EQ(ot::otx::LastReplyStatus::MessageSuccess, status);
    ASSERT_TRUE(message);

    const ot::RequestNumber requestNumber =
        ot::String::StringToUlong(message->request_num_->Get());
    const auto result = translate_result(std::get<0>(finished));
    verify_state_post(
        client_1_,
        *clientContext,
        context,
        sequence,
        alice_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        alice_counter_);

    // Verify reply was received. 6
    const auto incomingreplies =
        client_1_.Wallet().PeerReplyIncoming(alice_nym_id_);

    ASSERT_FALSE(incomingreplies.empty());

    const auto incomingID = peerreply.ID().asBase58(client_1_.Crypto());
    const auto foundincomingreply = find_id(incomingID, incomingreplies);

    ASSERT_TRUE(foundincomingreply);

    const auto incomingreply = client_1_.Wallet().PeerReply(
        alice_nym_id_,
        peerreply.ID(),
        ot::otx::client::StorageBox::INCOMINGPEERREPLY);

    EXPECT_TRUE(incomingreply.IsValid());
    EXPECT_EQ(incomingreply.Type(), requesttype);

    verify_reply(requesttype, peerreply, incomingreply);

    // Verify request is finished. 7
    const auto finishedrequests =
        client_1_.Wallet().PeerRequestFinished(alice_nym_id_);

    ASSERT_FALSE(finishedrequests.empty());

    const auto finishedrequestID =
        peerrequest.ID().asBase58(client_1_.Crypto());
    const auto foundfinishedrequest =
        find_id(finishedrequestID, finishedrequests);

    ASSERT_TRUE(foundfinishedrequest);

    const auto finishedrequest = client_1_.Wallet().PeerRequest(
        alice_nym_id_,
        peerrequest.ID(),
        ot::otx::client::StorageBox::FINISHEDPEERREQUEST);

    EXPECT_TRUE(finishedrequest.IsValid());
    EXPECT_EQ(finishedrequest.Initiator(), alice_nym_id_);
    EXPECT_EQ(finishedrequest.Responder(), bob_nym_id_);
    EXPECT_EQ(finishedrequest.Type(), requesttype);

    verify_request(requesttype, peerrequest, finishedrequest);
    auto complete =
        client_1_.Wallet().PeerRequestComplete(alice_nym_id_, peerreply.ID());

    ASSERT_TRUE(complete);

    // Verify reply was processed. 8
    const auto processedreplies =
        client_1_.Wallet().PeerReplyProcessed(alice_nym_id_);

    ASSERT_FALSE(processedreplies.empty());

    const auto processedreplyID = peerreply.ID().asBase58(client_1_.Crypto());
    const auto foundprocessedreply =
        find_id(processedreplyID, processedreplies);

    ASSERT_TRUE(foundprocessedreply);

    const auto processedreply = client_1_.Wallet().PeerReply(
        alice_nym_id_,
        peerreply.ID(),
        ot::otx::client::StorageBox::PROCESSEDPEERREPLY);

    EXPECT_TRUE(processedreply.IsValid());

    verify_reply(requesttype, peerreply, processedreply);
}

void Basic::receive_request(
    const std::shared_ptr<const ot::identity::Nym>& nym,
    const ot::contract::peer::Request& peerrequest,
    ot::contract::peer::RequestType requesttype)
{
    const ot::RequestNumber sequence = bob_counter_;
    const ot::RequestNumber messages{4};
    bob_counter_ += messages;
    auto serverContext = client_2_.Wallet().Internal().mutable_ServerContext(
        bob_nym_id_, server_1_id_, reason_c2_);
    auto& context = serverContext.get();
    auto clientContext =
        server_1_.Wallet().Internal().ClientContext(bob_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, context, sequence);
    auto queue = context.RefreshNymbox(client_2_, reason_c2_);

    ASSERT_TRUE(queue);

    const auto finished = queue->get();
    context.Join();
    context.ResetThread();
    const auto& [status, message] = finished;

    EXPECT_EQ(ot::otx::LastReplyStatus::MessageSuccess, status);
    ASSERT_TRUE(message);

    const ot::RequestNumber requestNumber =
        ot::String::StringToUlong(message->request_num_->Get());
    const auto result = translate_result(std::get<0>(finished));
    verify_state_post(
        client_2_,
        *clientContext,
        context,
        sequence,
        bob_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        bob_counter_);

    // Verify request was received. 2
    const auto incomingrequests =
        client_2_.Wallet().PeerRequestIncoming(bob_nym_id_);

    ASSERT_FALSE(incomingrequests.empty());

    const auto incomingID = peerrequest.ID().asBase58(client_1_.Crypto());
    const auto found = find_id(incomingID, incomingrequests);

    ASSERT_TRUE(found);

    const auto incomingrequest = client_2_.Wallet().PeerRequest(
        bob_nym_id_,
        peerrequest.ID(),
        ot::otx::client::StorageBox::INCOMINGPEERREQUEST);

    EXPECT_TRUE(incomingrequest.IsValid());
    EXPECT_EQ(incomingrequest.Initiator(), alice_nym_id_);
    EXPECT_EQ(incomingrequest.Responder(), bob_nym_id_);
    EXPECT_EQ(incomingrequest.Type(), requesttype);

    verify_request(requesttype, peerrequest, incomingrequest);
}

void Basic::send_peer_reply(
    const std::shared_ptr<const ot::identity::Nym>& nym,
    const ot::contract::peer::Reply& peerreply,
    const ot::contract::peer::Request& peerrequest,
    ot::contract::peer::RequestType requesttype)
{
    const ot::RequestNumber sequence = bob_counter_;
    const ot::RequestNumber messages{1};
    bob_counter_ += messages;

    auto serverContext = client_2_.Wallet().Internal().mutable_ServerContext(
        bob_nym_id_, server_1_id_, reason_c2_);
    auto& context = serverContext.get();
    auto clientContext =
        server_1_.Wallet().Internal().ClientContext(bob_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.get(), sequence);
    auto& stateMachine = *bob_state_machine_;
    auto started =
        stateMachine.SendPeerReply(alice_nym_id_, peerreply, peerrequest);

    ASSERT_TRUE(started);

    const auto finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const ot::RequestNumber requestNumber =
        ot::String::StringToUlong(message->request_num_->Get());
    const auto result = translate_result(std::get<0>(finished));
    verify_state_post(
        client_2_,
        *clientContext,
        serverContext.get(),
        sequence,
        bob_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        bob_counter_);

    // Verify reply was sent. 3
    const auto sentreplies = client_2_.Wallet().PeerReplySent(bob_nym_id_);

    ASSERT_FALSE(sentreplies.empty());

    const auto sentID = peerreply.ID().asBase58(client_1_.Crypto());
    const auto foundsentreply = find_id(sentID, sentreplies);

    ASSERT_TRUE(foundsentreply);

    const auto sentreply = client_2_.Wallet().PeerReply(
        bob_nym_id_,
        peerreply.ID(),
        ot::otx::client::StorageBox::SENTPEERREPLY);

    EXPECT_TRUE(sentreply.IsValid());
    EXPECT_EQ(sentreply.Type(), requesttype);

    verify_reply(requesttype, peerreply, sentreply);
    // Verify request was processed. 4
    const auto processedrequests =
        client_2_.Wallet().PeerRequestProcessed(bob_nym_id_);

    ASSERT_FALSE(processedrequests.empty());

    const auto processedID = peerrequest.ID().asBase58(client_1_.Crypto());
    const auto foundrequest = find_id(processedID, processedrequests);

    ASSERT_TRUE(foundrequest);

    const auto processedrequest = client_2_.Wallet().PeerRequest(
        bob_nym_id_,
        peerrequest.ID(),
        ot::otx::client::StorageBox::PROCESSEDPEERREQUEST);

    ASSERT_TRUE(processedrequest.IsValid());

    auto complete =
        client_2_.Wallet().PeerReplyComplete(bob_nym_id_, peerreply.ID());

    ASSERT_TRUE(complete);

    // Verify reply is finished. 5
    const auto finishedreplies =
        client_2_.Wallet().PeerReplyFinished(bob_nym_id_);

    ASSERT_FALSE(finishedreplies.empty());

    const auto finishedID = peerreply.ID().asBase58(client_1_.Crypto());
    const auto foundfinishedreply = find_id(finishedID, finishedreplies);

    ASSERT_TRUE(foundfinishedreply);

    const auto finishedreply = client_2_.Wallet().PeerReply(
        bob_nym_id_,
        peerreply.ID(),
        ot::otx::client::StorageBox::FINISHEDPEERREPLY);

    ASSERT_TRUE(finishedreply.IsValid());
}

void Basic::send_peer_request(
    const std::shared_ptr<const ot::identity::Nym>& nym,
    const ot::contract::peer::Request& peerrequest,
    ot::contract::peer::RequestType requesttype)
{
    const ot::RequestNumber sequence = alice_counter_;
    const ot::RequestNumber messages{1};
    alice_counter_ += messages;

    auto serverContext = client_1_.Wallet().Internal().mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto& context = serverContext.get();
    auto clientContext =
        server_1_.Wallet().Internal().ClientContext(alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.get(), sequence);
    auto& stateMachine = *alice_state_machine_;
    auto started = stateMachine.SendPeerRequest(bob_nym_id_, peerrequest);

    ASSERT_TRUE(started);

    const auto finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const ot::RequestNumber requestNumber =
        ot::String::StringToUlong(message->request_num_->Get());
    const auto result = translate_result(std::get<0>(finished));
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.get(),
        sequence,
        alice_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        alice_counter_);

    // Verify request was sent. 1
    const auto sentrequests = client_1_.Wallet().PeerRequestSent(alice_nym_id_);

    ASSERT_FALSE(sentrequests.empty());

    const auto sentID = peerrequest.ID().asBase58(client_1_.Crypto());
    const auto found = find_id(sentID, sentrequests);

    ASSERT_TRUE(found);

    const auto sentrequest = client_1_.Wallet().PeerRequest(
        alice_nym_id_,
        peerrequest.ID(),
        ot::otx::client::StorageBox::SENTPEERREQUEST);

    EXPECT_TRUE(sentrequest.IsValid());
    EXPECT_EQ(sentrequest.Initiator(), alice_nym_id_);
    EXPECT_EQ(sentrequest.Responder(), bob_nym_id_);
    EXPECT_EQ(sentrequest.Type(), requesttype);

    verify_request(requesttype, peerrequest, sentrequest);
}

void Basic::verify_account(
    const ot::identity::Nym& clientNym,
    const ot::identity::Nym& serverNym,
    const ot::Account& client,
    const ot::Account& server,
    const ot::PasswordPrompt& reasonC,
    const ot::PasswordPrompt& reasonS)
{
    EXPECT_EQ(client.GetBalance(), server.GetBalance());
    EXPECT_EQ(
        client.GetInstrumentDefinitionID(), server.GetInstrumentDefinitionID());

    std::unique_ptr<ot::Ledger> clientInbox(client.LoadInbox(clientNym));
    std::unique_ptr<ot::Ledger> serverInbox(server.LoadInbox(serverNym));
    std::unique_ptr<ot::Ledger> clientOutbox(client.LoadOutbox(clientNym));
    std::unique_ptr<ot::Ledger> serverOutbox(server.LoadOutbox(serverNym));

    ASSERT_TRUE(clientInbox);
    ASSERT_TRUE(serverInbox);
    ASSERT_TRUE(clientOutbox);
    ASSERT_TRUE(serverOutbox);

    auto clientInboxHash = ot::identifier::Generic{};
    auto serverInboxHash = ot::identifier::Generic{};
    auto clientOutboxHash = ot::identifier::Generic{};
    auto serverOutboxHash = ot::identifier::Generic{};

    EXPECT_TRUE(clientInbox->CalculateInboxHash(clientInboxHash));
    EXPECT_TRUE(serverInbox->CalculateInboxHash(serverInboxHash));
    EXPECT_TRUE(clientOutbox->CalculateOutboxHash(clientOutboxHash));
    EXPECT_TRUE(serverOutbox->CalculateOutboxHash(serverOutboxHash));
}

auto Basic::verify_bailment(
    const ot::contract::peer::Reply& originalreply,
    const ot::contract::peer::Reply& restoredreply) noexcept -> void
{
    verify_reply_properties(originalreply, restoredreply.asBailment());

    EXPECT_FALSE(restoredreply.asBailmentNotice().IsValid());
    EXPECT_FALSE(restoredreply.asConnection().IsValid());
    EXPECT_FALSE(restoredreply.asFaucet().IsValid());
    EXPECT_FALSE(restoredreply.asOutbailment().IsValid());
    EXPECT_FALSE(restoredreply.asStoreSecret().IsValid());
}

auto Basic::verify_bailment(
    const ot::contract::peer::Request& originalrequest,
    const ot::contract::peer::Request& restoredrequest) noexcept -> void
{
    const auto& original = originalrequest.asBailment();
    const auto& bailment = restoredrequest.asBailment();

    EXPECT_EQ(bailment.Notary(), original.Notary());
    EXPECT_EQ(bailment.Unit(), original.Unit());

    verify_request_properties(originalrequest, bailment);

    EXPECT_FALSE(restoredrequest.asBailmentNotice().IsValid());
    EXPECT_FALSE(restoredrequest.asConnection().IsValid());
    EXPECT_FALSE(restoredrequest.asFaucet().IsValid());
    EXPECT_FALSE(restoredrequest.asOutbailment().IsValid());
    EXPECT_FALSE(restoredrequest.asStoreSecret().IsValid());
}

auto Basic::verify_bailment_notice(
    const ot::contract::peer::Reply& originalreply,
    const ot::contract::peer::Reply& restoredreply) noexcept -> void
{
    verify_reply_properties(originalreply, restoredreply.asBailmentNotice());

    EXPECT_FALSE(restoredreply.asBailment().IsValid());
    EXPECT_FALSE(restoredreply.asConnection().IsValid());
    EXPECT_FALSE(restoredreply.asFaucet().IsValid());
    EXPECT_FALSE(restoredreply.asOutbailment().IsValid());
    EXPECT_FALSE(restoredreply.asStoreSecret().IsValid());
}

auto Basic::verify_bailment_notice(
    const ot::contract::peer::Request& originalrequest,
    const ot::contract::peer::Request& restoredrequest) noexcept -> void
{
    verify_request_properties(
        originalrequest, restoredrequest.asBailmentNotice());

    EXPECT_FALSE(restoredrequest.asBailment().IsValid());
    EXPECT_FALSE(restoredrequest.asConnection().IsValid());
    EXPECT_FALSE(restoredrequest.asFaucet().IsValid());
    EXPECT_FALSE(restoredrequest.asOutbailment().IsValid());
    EXPECT_FALSE(restoredrequest.asStoreSecret().IsValid());
}

auto Basic::verify_connection(
    const ot::contract::peer::Reply& originalreply,
    const ot::contract::peer::Reply& restoredreply) noexcept -> void
{
    verify_reply_properties(originalreply, restoredreply.asConnection());

    EXPECT_FALSE(restoredreply.asBailment().IsValid());
    EXPECT_FALSE(restoredreply.asBailmentNotice().IsValid());
    EXPECT_FALSE(restoredreply.asFaucet().IsValid());
    EXPECT_FALSE(restoredreply.asOutbailment().IsValid());
    EXPECT_FALSE(restoredreply.asStoreSecret().IsValid());
}

auto Basic::verify_connection(
    const ot::contract::peer::Request& originalrequest,
    const ot::contract::peer::Request& restoredrequest) noexcept -> void
{
    verify_request_properties(originalrequest, restoredrequest.asConnection());

    EXPECT_FALSE(restoredrequest.asBailment().IsValid());
    EXPECT_FALSE(restoredrequest.asBailmentNotice().IsValid());
    EXPECT_FALSE(restoredrequest.asFaucet().IsValid());
    EXPECT_FALSE(restoredrequest.asOutbailment().IsValid());
    EXPECT_FALSE(restoredrequest.asStoreSecret().IsValid());
}

auto Basic::verify_outbailment(
    const ot::contract::peer::Reply& originalreply,
    const ot::contract::peer::Reply& restoredreply) noexcept -> void
{
    verify_reply_properties(originalreply, restoredreply.asOutbailment());

    EXPECT_FALSE(restoredreply.asBailment().IsValid());
    EXPECT_FALSE(restoredreply.asBailmentNotice().IsValid());
    EXPECT_FALSE(restoredreply.asConnection().IsValid());
    EXPECT_FALSE(restoredreply.asFaucet().IsValid());
    EXPECT_FALSE(restoredreply.asStoreSecret().IsValid());
}

auto Basic::verify_outbailment(
    const ot::contract::peer::Request& originalrequest,
    const ot::contract::peer::Request& restoredrequest) noexcept -> void
{
    verify_request_properties(originalrequest, restoredrequest.asOutbailment());

    EXPECT_FALSE(restoredrequest.asBailment().IsValid());
    EXPECT_FALSE(restoredrequest.asBailmentNotice().IsValid());
    EXPECT_FALSE(restoredrequest.asConnection().IsValid());
    EXPECT_FALSE(restoredrequest.asFaucet().IsValid());
    EXPECT_FALSE(restoredrequest.asStoreSecret().IsValid());
}

auto Basic::verify_reply(
    ot::contract::peer::RequestType requesttype,
    const ot::contract::peer::Reply& original,
    const ot::contract::peer::Reply& restored) noexcept -> void
{
    switch (requesttype) {
        case ot::contract::peer::RequestType::Bailment: {
            verify_bailment(original, restored);
        } break;
        case ot::contract::peer::RequestType::PendingBailment: {
            verify_bailment_notice(original, restored);
        } break;
        case ot::contract::peer::RequestType::ConnectionInfo: {
            verify_connection(original, restored);
        } break;
        case ot::contract::peer::RequestType::OutBailment: {
            verify_outbailment(original, restored);
        } break;
        case ot::contract::peer::RequestType::StoreSecret: {
            // TODO
        } break;
        case ot::contract::peer::RequestType::VerifiedClaim:
        case ot::contract::peer::RequestType::Faucet:
        case ot::contract::peer::RequestType::Error:
        default: {
        }
    }
}

auto Basic::verify_request(
    ot::contract::peer::RequestType requesttype,
    const ot::contract::peer::Request& original,
    const ot::contract::peer::Request& restored) noexcept -> void
{
    switch (requesttype) {
        case ot::contract::peer::RequestType::Bailment: {
            verify_bailment(original, restored);
        } break;
        case ot::contract::peer::RequestType::PendingBailment: {
            verify_bailment_notice(original, restored);
        } break;
        case ot::contract::peer::RequestType::ConnectionInfo: {
            verify_connection(original, restored);
        } break;
        case ot::contract::peer::RequestType::OutBailment: {
            verify_outbailment(original, restored);
        } break;
        case ot::contract::peer::RequestType::StoreSecret: {
            verify_storesecret(original, restored);
        } break;
        case ot::contract::peer::RequestType::VerifiedClaim:
        case ot::contract::peer::RequestType::Faucet:
        case ot::contract::peer::RequestType::Error:
        default: {
        }
    }
}

template <typename T>
void Basic::verify_reply_properties(
    const ot::contract::peer::Reply& original,
    const T& restored)
{
    auto lhs = opentxs::ByteArray{};
    auto rhs = opentxs::ByteArray{};

    EXPECT_NE(restored.Version(), 0);
    EXPECT_EQ(restored.Alias(), original.Alias());
    EXPECT_EQ(restored.Name(), original.Name());
    EXPECT_TRUE(restored.Serialize(lhs.WriteInto()));
    EXPECT_TRUE(original.Serialize(rhs.WriteInto()));
    EXPECT_EQ(lhs, rhs);
    EXPECT_EQ(restored.Type(), original.Type());
    EXPECT_EQ(restored.Version(), original.Version());
}

template <typename T>
void Basic::verify_request_properties(
    const ot::contract::peer::Request& original,
    const T& restored)
{
    auto lhs = opentxs::ByteArray{};
    auto rhs = opentxs::ByteArray{};

    EXPECT_NE(restored.Version(), 0);
    EXPECT_EQ(restored.Alias(), original.Alias());
    EXPECT_EQ(restored.Initiator(), original.Initiator());
    EXPECT_EQ(restored.Name(), original.Name());
    EXPECT_EQ(restored.Responder(), original.Responder());
    EXPECT_TRUE(restored.Serialize(lhs.WriteInto()));
    EXPECT_TRUE(original.Serialize(rhs.WriteInto()));
    EXPECT_EQ(lhs, rhs);
    EXPECT_EQ(restored.Type(), original.Type());
    EXPECT_EQ(restored.Version(), original.Version());
}

void Basic::verify_storesecret(
    const ot::contract::peer::Request& originalrequest,
    const ot::contract::peer::Request& restoredrequest)
{
    const auto& storesecret = restoredrequest.asStoreSecret();
    verify_request_properties(originalrequest, storesecret);
}

void Basic::verify_state_pre(
    const ot::otx::context::Client& clientContext,
    const ot::otx::context::Server& serverContext,
    const ot::RequestNumber initialRequestNumber)
{
    EXPECT_EQ(serverContext.Request(), initialRequestNumber);
    EXPECT_EQ(clientContext.Request(), initialRequestNumber);
}

void Basic::verify_state_post(
    const ot::api::session::Client& client,
    const ot::otx::context::Client& clientContext,
    const ot::otx::context::Server& serverContext,
    const ot::RequestNumber initialRequestNumber,
    const ot::RequestNumber finalRequestNumber,
    const ot::RequestNumber messageRequestNumber,
    const ot::otx::client::SendResult messageResult,
    const std::shared_ptr<ot::Message>& message,
    const bool messageSuccess,
    const std::size_t expectedNymboxItems,
    ot::RequestNumber& counter)
{
    EXPECT_EQ(messageRequestNumber, initialRequestNumber);
    EXPECT_EQ(serverContext.Request(), finalRequestNumber);
    EXPECT_EQ(clientContext.Request(), finalRequestNumber);
    EXPECT_EQ(
        serverContext.RemoteNymboxHash(), clientContext.LocalNymboxHash());
    EXPECT_EQ(
        serverContext.LocalNymboxHash(), serverContext.RemoteNymboxHash());
    EXPECT_EQ(ot::otx::client::SendResult::VALID_REPLY, messageResult);
    ASSERT_TRUE(message);
    EXPECT_EQ(messageSuccess, message->success_);

    std::unique_ptr<ot::Ledger> nymbox{
        client.InternalClient().OTAPI().LoadNymbox(
            server_1_id_, serverContext.Signer()->ID())};

    ASSERT_TRUE(nymbox);
    EXPECT_TRUE(nymbox->VerifyAccount(*serverContext.Signer()));

    const auto& transactionMap = nymbox->GetTransactionMap();

    EXPECT_EQ(expectedNymboxItems, transactionMap.size());

    counter = serverContext.Request();

    EXPECT_FALSE(serverContext.StaleNym());
}

}  // namespace ottest
