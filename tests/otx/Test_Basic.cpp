// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"
#include "opentxs/core/contract/peer/PeerReply.hpp"
#include "opentxs/core/contract/peer/PeerRequest.hpp"
#include "api/client/InternalClient.hpp"
#include "server/Server.hpp"
#include "server/Transactor.hpp"

#include <gtest/gtest.h>

using namespace opentxs;

#define CHEQUE_AMOUNT 144488
#define TRANSFER_AMOUNT 1144888
#define SECOND_TRANSFER_AMOUNT 500000
#define CASH_AMOUNT 100
#define CHEQUE_MEMO "cheque memo"
#define TRANSFER_MEMO "transfer memo"
#define FAILURE false
#define INBOX_TYPE 1
#define NYMBOX_TYPE 0
#define NO_TRANSACTION false
#define SUCCESS true
#define TRANSACTION true
#define UNIT_DEFINITION_CONTRACT_NAME "Mt Gox USD"
#define UNIT_DEFINITION_TERMS "YOLO"
#define UNIT_DEFINITION_PRIMARY_UNIT_NAME "dollars"
#define UNIT_DEFINITION_SYMBOL "$"
#define UNIT_DEFINITION_TLA "USA"
#define UNIT_DEFINITION_POWER 2
#define UNIT_DEFINITION_FRACTIONAL_UNIT_NAME "cents"
#define UNIT_DEFINITION_CONTRACT_NAME_2 "Mt Gox BTC"
#define UNIT_DEFINITION_TERMS_2 "YOLO"
#define UNIT_DEFINITION_PRIMARY_UNIT_NAME_2 "bitcoins"
#define UNIT_DEFINITION_SYMBOL_2 "B"
#define UNIT_DEFINITION_TLA_2 "BTC"
#define UNIT_DEFINITION_POWER_2 8
#define UNIT_DEFINITION_FRACTIONAL_UNIT_NAME_2 "satoshis"
#define MESSAGE_TEXT "example message text"
#define NEW_SERVER_NAME "Awesome McCoolName"
#define MINT_TIME_LIMIT_MINUTES 5
#define TEST_SEED                                                              \
    "one two three four five six seven eight nine ten eleven twelve"
#define TEST_SEED_PASSPHRASE "seed passphrase"

namespace
{
bool init_{false};

class Test_Basic : public ::testing::Test
{
public:
    using ProtoHasReply = std::function<bool(const proto::PeerReply&)>;
    using ProtoHasRequest = std::function<bool(const proto::PeerRequest&)>;

    struct matchID {
        matchID(const std::string& id)
            : id_{id}
        {
        }

        bool operator()(const std::pair<std::string, std::string>& id)
        {
            return id.first == id_;
        };

        const std::string id_;
    };

    static opentxs::RequestNumber alice_counter_;
    static opentxs::RequestNumber bob_counter_;
    static const opentxs::ArgList args_;
    static const std::string SeedA_;
    static const std::string SeedB_;
    static const std::string Alice_;
    static const std::string Bob_;
    static const OTNymID alice_nym_id_;
    static const OTNymID bob_nym_id_;
    static const std::shared_ptr<const ServerContract> server_contract_;
    static const std::shared_ptr<const UnitDefinition> asset_contract_1_;
    static const std::shared_ptr<const UnitDefinition> asset_contract_2_;
    static TransactionNumber cheque_transaction_number_;
    static std::string bob_account_1_id_;
    static std::string bob_account_2_id_;
    static std::string outgoing_cheque_workflow_id_;
    static std::string incoming_cheque_workflow_id_;
    static std::string outgoing_transfer_workflow_id_;
    static std::string incoming_transfer_workflow_id_;
    static std::string internal_transfer_workflow_id_;
    static std::unique_ptr<opentxs::api::client::internal::Operation>
        alice_state_machine_;
    static std::unique_ptr<opentxs::api::client::internal::Operation>
        bob_state_machine_;
#if OT_CASH
    static std::shared_ptr<blind::Purse> untrusted_purse_;
#endif
    const opentxs::api::client::Manager& client_1_;
    const opentxs::api::client::Manager& client_2_;
    const opentxs::api::server::Manager& server_1_;
    const opentxs::api::server::Manager& server_2_;
    const opentxs::identifier::Server& server_1_id_;
    const opentxs::identifier::Server& server_2_id_;
    const opentxs::ServerContext::ExtraArgs extra_args_;

    Test_Basic()
        : client_1_(OT::App().StartClient(args_, 0))
        , client_2_(OT::App().StartClient(args_, 1))
        , server_1_(OT::App().StartServer(args_, 0, true))
        , server_2_(OT::App().StartServer(args_, 1, true))
        , server_1_id_(
              dynamic_cast<const opentxs::identifier::Server&>(server_1_.ID()))
        , server_2_id_(
              dynamic_cast<const opentxs::identifier::Server&>(server_2_.ID()))
        , extra_args_()
    {
        if (false == init_) { init(); }
    }

    static bool find_id(const std::string& id, const ObjectList& list)
    {
        matchID matchid(id);

        return std::find_if(list.begin(), list.end(), matchid) != list.end();
    }

    static SendResult translate_result(const proto::LastReplyStatus status)
    {
        switch (status) {
            case proto::LASTREPLYSTATUS_MESSAGESUCCESS:
            case proto::LASTREPLYSTATUS_MESSAGEFAILED: {

                return SendResult::VALID_REPLY;
            }
            case proto::LASTREPLYSTATUS_UNKNOWN: {

                return SendResult::TIMEOUT;
            }
            case proto::LASTREPLYSTATUS_NOTSENT: {

                return SendResult::UNNECESSARY;
            }
            case proto::LASTREPLYSTATUS_INVALID:
            case proto::LASTREPLYSTATUS_NONE:
            default: {

                return SendResult::ERROR;
            }
        }
    }

    void break_consensus()
    {
        TransactionNumber newNumber{0};
        server_1_.Server().GetTransactor().issueNextTransactionNumber(
            newNumber);

        auto context = server_1_.Wallet().mutable_ClientContext(
            server_1_.NymID(), alice_nym_id_);
        context.It().IssueNumber(newNumber);
    }

    void import_server_contract(
        const ServerContract& contract,
        const opentxs::api::client::Manager& client)
    {
        auto clientVersion =
            client.Wallet().Server(server_contract_->PublicContract());

        OT_ASSERT(clientVersion)

        client.OTX().SetIntroductionServer(*clientVersion);
    }

    void init()
    {
        client_1_.OTX().DisableAutoaccept();
        client_2_.OTX().DisableAutoaccept();
        const_cast<std::string&>(SeedA_) = client_1_.Exec().Wallet_ImportSeed(
            "spike nominee miss inquiry fee nothing belt list other "
            "daughter leave valley twelve gossip paper",
            "");
        const_cast<std::string&>(SeedB_) = client_2_.Exec().Wallet_ImportSeed(
            "trim thunder unveil reduce crop cradle zone inquiry "
            "anchor skate property fringe obey butter text tank drama "
            "palm guilt pudding laundry stay axis prosper",
            "");
        const_cast<std::string&>(Alice_) = client_1_.Exec().CreateNymHD(
            proto::CITEMTYPE_INDIVIDUAL, "Alice", SeedA_, 0);
        const_cast<std::string&>(Bob_) = client_2_.Exec().CreateNymHD(
            proto::CITEMTYPE_INDIVIDUAL, "Bob", SeedB_, 0);
        const_cast<OTNymID&>(alice_nym_id_) = identifier::Nym::Factory(Alice_);
        const_cast<OTNymID&>(bob_nym_id_) = identifier::Nym::Factory(Bob_);
        const_cast<std::shared_ptr<const ServerContract>&>(server_contract_) =
            server_1_.Wallet().Server(server_1_id_);

        OT_ASSERT(server_contract_);
        OT_ASSERT(false == server_1_id_.empty());

        import_server_contract(*server_contract_, client_1_);
        import_server_contract(*server_contract_, client_2_);

#if OT_CASH
        server_1_.SetMintKeySize(OT_MINT_KEY_SIZE_TEST);
        server_2_.SetMintKeySize(OT_MINT_KEY_SIZE_TEST);
#endif
        alice_state_machine_.reset(opentxs::Factory::Operation(
            client_1_, alice_nym_id_, server_1_id_));

        OT_ASSERT(alice_state_machine_);

        alice_state_machine_->SetPush(false);
        bob_state_machine_.reset(
            opentxs::Factory::Operation(client_2_, bob_nym_id_, server_1_id_));

        OT_ASSERT(bob_state_machine_);

        bob_state_machine_->SetPush(false);
        init_ = true;
    }

    void create_unit_definition_1()
    {
        const_cast<std::shared_ptr<const UnitDefinition>&>(asset_contract_1_) =
            client_1_.Wallet().UnitDefinition(
                alice_nym_id_->str(),
                UNIT_DEFINITION_CONTRACT_NAME,
                UNIT_DEFINITION_TERMS,
                UNIT_DEFINITION_PRIMARY_UNIT_NAME,
                UNIT_DEFINITION_SYMBOL,
                UNIT_DEFINITION_TLA,
                UNIT_DEFINITION_POWER,
                UNIT_DEFINITION_FRACTIONAL_UNIT_NAME);

        ASSERT_TRUE(asset_contract_1_);
        EXPECT_EQ(proto::UNITTYPE_CURRENCY, asset_contract_1_->Type());
    }

    void create_unit_definition_2()
    {
        const_cast<std::shared_ptr<const UnitDefinition>&>(asset_contract_2_) =
            client_2_.Wallet().UnitDefinition(
                bob_nym_id_->str(),
                UNIT_DEFINITION_CONTRACT_NAME_2,
                UNIT_DEFINITION_TERMS_2,
                UNIT_DEFINITION_PRIMARY_UNIT_NAME_2,
                UNIT_DEFINITION_SYMBOL_2,
                UNIT_DEFINITION_TLA_2,
                UNIT_DEFINITION_POWER_2,
                UNIT_DEFINITION_FRACTIONAL_UNIT_NAME_2);

        ASSERT_TRUE(asset_contract_2_);
        EXPECT_EQ(proto::UNITTYPE_CURRENCY, asset_contract_2_->Type());
    }

    OTIdentifier find_issuer_account()
    {
        const auto accounts =
            client_1_.Storage().AccountsByOwner(alice_nym_id_);

        OT_ASSERT(1 == accounts.size());

        return *accounts.begin();
    }

    OTUnitID find_unit_definition_id_1()
    {
        const auto accountID = find_issuer_account();

        OT_ASSERT(false == accountID->empty());

        const auto output = client_1_.Storage().AccountContract(accountID);

        OT_ASSERT(false == output->empty());

        // TODO unit id type
        return identifier::UnitDefinition::Factory(output->str());
    }

    OTUnitID find_unit_definition_id_2()
    {
        OT_ASSERT(asset_contract_2_);

        // TODO unit id type
        return identifier::UnitDefinition::Factory(
            asset_contract_2_->ID()->str());
    }

    OTIdentifier find_user_account()
    {
        return Identifier::Factory(bob_account_1_id_);
    }

    OTIdentifier find_second_user_account()
    {
        return Identifier::Factory(bob_account_2_id_);
    }

    void receive_reply(
        const std::shared_ptr<const PeerReply>& peerreply,
        const std::shared_ptr<const PeerRequest>& peerrequest,
        ProtoHasReply protohasreply,
        ProtoHasRequest protohasrequest,
        proto::PeerRequestType prototype)
    {
        const RequestNumber sequence = alice_counter_;
        const RequestNumber messages{4};
        alice_counter_ += messages;
        auto serverContext = client_1_.Wallet().mutable_ServerContext(
            alice_nym_id_, server_1_id_);
        auto& context = serverContext.It();
        auto clientContext =
            server_1_.Wallet().ClientContext(server_1_.NymID(), alice_nym_id_);

        ASSERT_TRUE(clientContext);

        verify_state_pre(*clientContext, context, sequence);
        auto queue = context.RefreshNymbox(client_1_);

        ASSERT_TRUE(queue);

        const auto finished = queue->get();
        context.Join();
        context.ResetThread();
        const auto& [status, message] = finished;

        EXPECT_EQ(proto::LASTREPLYSTATUS_MESSAGESUCCESS, status);
        ASSERT_TRUE(message);

        const RequestNumber requestNumber =
            String::StringToUlong(message->m_strRequestNum->Get());
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

        const auto incomingID = peerreply->ID()->str();
        const auto foundincomingreply = find_id(incomingID, incomingreplies);

        ASSERT_TRUE(foundincomingreply);

        const auto incomingreply = client_1_.Wallet().PeerReply(
            alice_nym_id_, peerreply->ID(), StorageBox::INCOMINGPEERREPLY);

        ASSERT_TRUE(incomingreply);
        EXPECT_STREQ(
            incomingreply->initiator().c_str(), alice_nym_id_->str().c_str());
        EXPECT_STREQ(
            incomingreply->recipient().c_str(), bob_nym_id_->str().c_str());
        EXPECT_EQ(incomingreply->type(), prototype);
        EXPECT_STREQ(
            incomingreply->server().c_str(), server_1_id_.str().c_str());
        EXPECT_TRUE(protohasreply(*incomingreply));

        // Verify request is finished. 7
        const auto finishedrequests =
            client_1_.Wallet().PeerRequestFinished(alice_nym_id_);

        ASSERT_FALSE(finishedrequests.empty());

        const auto finishedrequestID = peerrequest->ID()->str();
        const auto foundfinishedrequest =
            find_id(finishedrequestID, finishedrequests);

        ASSERT_TRUE(foundfinishedrequest);

        std::time_t notused;
        const auto finishedrequest = client_1_.Wallet().PeerRequest(
            alice_nym_id_,
            peerrequest->ID(),
            StorageBox::FINISHEDPEERREQUEST,
            notused);

        ASSERT_TRUE(finishedrequest);
        EXPECT_STREQ(
            finishedrequest->initiator().c_str(), alice_nym_id_->str().c_str());
        EXPECT_STREQ(
            finishedrequest->recipient().c_str(), bob_nym_id_->str().c_str());
        EXPECT_EQ(finishedrequest->type(), prototype);
        EXPECT_TRUE(protohasrequest(*finishedrequest));

        auto complete = client_1_.Wallet().PeerRequestComplete(
            alice_nym_id_, peerreply->ID());

        ASSERT_TRUE(complete);

        // Verify reply was processed. 8
        const auto processedreplies =
            client_1_.Wallet().PeerReplyProcessed(alice_nym_id_);

        ASSERT_FALSE(processedreplies.empty());

        const auto processedreplyID = peerreply->ID()->str();
        const auto foundprocessedreply =
            find_id(processedreplyID, processedreplies);

        ASSERT_TRUE(foundprocessedreply);

        const auto processedreply = client_1_.Wallet().PeerReply(
            alice_nym_id_, peerreply->ID(), StorageBox::PROCESSEDPEERREPLY);

        ASSERT_TRUE(processedreply);
    }

    void receive_request(
        const std::shared_ptr<PeerRequest>& peerrequest,
        ProtoHasRequest protohasrequest,
        proto::PeerRequestType prototype)
    {
        const RequestNumber sequence = bob_counter_;
        const RequestNumber messages{4};
        bob_counter_ += messages;
        auto serverContext =
            client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_1_id_);
        auto& context = serverContext.It();
        auto clientContext =
            server_1_.Wallet().ClientContext(server_1_.NymID(), bob_nym_id_);

        ASSERT_TRUE(clientContext);

        verify_state_pre(*clientContext, context, sequence);
        auto queue = context.RefreshNymbox(client_2_);

        ASSERT_TRUE(queue);

        const auto finished = queue->get();
        context.Join();
        context.ResetThread();
        const auto& [status, message] = finished;

        EXPECT_EQ(proto::LASTREPLYSTATUS_MESSAGESUCCESS, status);
        ASSERT_TRUE(message);

        const RequestNumber requestNumber =
            String::StringToUlong(message->m_strRequestNum->Get());
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

        const auto incomingID = peerrequest->ID()->str();
        const auto found = find_id(incomingID, incomingrequests);

        ASSERT_TRUE(found);

        std::time_t notused;
        const auto incomingrequest = client_2_.Wallet().PeerRequest(
            bob_nym_id_,
            peerrequest->ID(),
            StorageBox::INCOMINGPEERREQUEST,
            notused);

        ASSERT_TRUE(incomingrequest);
        EXPECT_STREQ(
            incomingrequest->initiator().c_str(), alice_nym_id_->str().c_str());
        EXPECT_STREQ(
            incomingrequest->recipient().c_str(), bob_nym_id_->str().c_str());
        EXPECT_EQ(incomingrequest->type(), prototype);
        EXPECT_TRUE(protohasrequest(*incomingrequest));
    }

    void send_peer_reply(
        const std::shared_ptr<const PeerReply>& peerreply,
        const std::shared_ptr<const PeerRequest>& peerrequest,
        ProtoHasReply protohasreply,
        proto::PeerRequestType prototype)
    {
        const RequestNumber sequence = bob_counter_;
        const RequestNumber messages{1};
        bob_counter_ += messages;

        auto serverContext =
            client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_1_id_);
        auto& context = serverContext.It();
        auto clientContext =
            server_1_.Wallet().ClientContext(server_1_.NymID(), bob_nym_id_);

        ASSERT_TRUE(clientContext);

        verify_state_pre(*clientContext, serverContext.It(), sequence);
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

        const RequestNumber requestNumber =
            String::StringToUlong(message->m_strRequestNum->Get());
        const auto result = translate_result(std::get<0>(finished));
        verify_state_post(
            client_2_,
            *clientContext,
            serverContext.It(),
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

        const auto sentID = peerreply->ID()->str();
        const auto foundsentreply = find_id(sentID, sentreplies);

        ASSERT_TRUE(foundsentreply);

        const auto sentreply = client_2_.Wallet().PeerReply(
            bob_nym_id_, peerreply->ID(), StorageBox::SENTPEERREPLY);

        ASSERT_TRUE(sentreply);
        EXPECT_STREQ(
            sentreply->initiator().c_str(), alice_nym_id_->str().c_str());
        EXPECT_STREQ(
            sentreply->recipient().c_str(), bob_nym_id_->str().c_str());
        EXPECT_EQ(sentreply->type(), prototype);
        EXPECT_STREQ(sentreply->server().c_str(), server_1_id_.str().c_str());
        EXPECT_TRUE(protohasreply(*sentreply));

        // Verify request was processed. 4
        const auto processedrequests =
            client_2_.Wallet().PeerRequestProcessed(bob_nym_id_);

        ASSERT_FALSE(processedrequests.empty());

        const auto processedID = peerrequest->ID()->str();
        const auto foundrequest = find_id(processedID, processedrequests);

        ASSERT_TRUE(foundrequest);

        std::time_t notused;
        const auto processedrequest = client_2_.Wallet().PeerRequest(
            bob_nym_id_,
            peerrequest->ID(),
            StorageBox::PROCESSEDPEERREQUEST,
            notused);

        ASSERT_TRUE(processedrequest);

        auto complete =
            client_2_.Wallet().PeerReplyComplete(bob_nym_id_, peerreply->ID());

        ASSERT_TRUE(complete);

        // Verify reply is finished. 5
        const auto finishedreplies =
            client_2_.Wallet().PeerReplyFinished(bob_nym_id_);

        ASSERT_FALSE(finishedreplies.empty());

        const auto finishedID = peerreply->ID()->str();
        const auto foundfinishedreply = find_id(finishedID, finishedreplies);

        ASSERT_TRUE(foundfinishedreply);

        const auto finishedreply = client_2_.Wallet().PeerReply(
            bob_nym_id_, peerreply->ID(), StorageBox::FINISHEDPEERREPLY);

        ASSERT_TRUE(finishedreply);
    }

    void send_peer_request(
        const std::shared_ptr<PeerRequest>& peerrequest,
        ProtoHasRequest protohasrequest,
        proto::PeerRequestType prototype)
    {
        const RequestNumber sequence = alice_counter_;
        const RequestNumber messages{1};
        alice_counter_ += messages;

        auto serverContext = client_1_.Wallet().mutable_ServerContext(
            alice_nym_id_, server_1_id_);
        auto& context = serverContext.It();
        auto clientContext =
            server_1_.Wallet().ClientContext(server_1_.NymID(), alice_nym_id_);

        ASSERT_TRUE(clientContext);

        verify_state_pre(*clientContext, serverContext.It(), sequence);
        auto& stateMachine = *alice_state_machine_;
        auto started = stateMachine.SendPeerRequest(bob_nym_id_, peerrequest);

        ASSERT_TRUE(started);

        const auto finished = stateMachine.GetFuture().get();
        stateMachine.join();
        context.Join();
        context.ResetThread();
        const auto& message = std::get<1>(finished);

        ASSERT_TRUE(message);

        const RequestNumber requestNumber =
            String::StringToUlong(message->m_strRequestNum->Get());
        const auto result = translate_result(std::get<0>(finished));
        verify_state_post(
            client_1_,
            *clientContext,
            serverContext.It(),
            sequence,
            alice_counter_,
            requestNumber,
            result,
            message,
            SUCCESS,
            0,
            alice_counter_);

        // Verify request was sent. 1
        const auto sentrequests =
            client_1_.Wallet().PeerRequestSent(alice_nym_id_);

        ASSERT_FALSE(sentrequests.empty());

        const auto sentID = peerrequest->ID()->str();
        const auto found = find_id(sentID, sentrequests);

        ASSERT_TRUE(found);

        std::time_t notused;
        const auto sentrequest = client_1_.Wallet().PeerRequest(
            alice_nym_id_,
            peerrequest->ID(),
            StorageBox::SENTPEERREQUEST,
            notused);

        ASSERT_TRUE(sentrequest);
        EXPECT_STREQ(
            sentrequest->initiator().c_str(), alice_nym_id_->str().c_str());
        EXPECT_STREQ(
            sentrequest->recipient().c_str(), bob_nym_id_->str().c_str());
        EXPECT_EQ(sentrequest->type(), prototype);
        EXPECT_TRUE(protohasrequest(*sentrequest));
    }

    void verify_account(
        const Nym& clientNym,
        const Nym& serverNym,
        const Account& client,
        const Account& server)
    {
        EXPECT_EQ(client.GetBalance(), server.GetBalance());
        EXPECT_EQ(
            client.GetInstrumentDefinitionID(),
            server.GetInstrumentDefinitionID());

        std::unique_ptr<Ledger> clientInbox(client.LoadInbox(clientNym));
        std::unique_ptr<Ledger> serverInbox(server.LoadInbox(serverNym));
        std::unique_ptr<Ledger> clientOutbox(client.LoadOutbox(clientNym));
        std::unique_ptr<Ledger> serverOutbox(server.LoadOutbox(serverNym));

        ASSERT_TRUE(clientInbox);
        ASSERT_TRUE(serverInbox);
        ASSERT_TRUE(clientOutbox);
        ASSERT_TRUE(serverOutbox);

        auto clientInboxHash = Identifier::Factory();
        auto serverInboxHash = Identifier::Factory();
        auto clientOutboxHash = Identifier::Factory();
        auto serverOutboxHash = Identifier::Factory();

        EXPECT_TRUE(clientInbox->CalculateInboxHash(clientInboxHash));
        EXPECT_TRUE(serverInbox->CalculateInboxHash(serverInboxHash));
        EXPECT_TRUE(clientOutbox->CalculateOutboxHash(clientOutboxHash));
        EXPECT_TRUE(serverOutbox->CalculateOutboxHash(serverOutboxHash));
    }

    void verify_state_pre(
        const ClientContext& clientContext,
        const ServerContext& serverContext,
        const RequestNumber initialRequestNumber)
    {
        EXPECT_EQ(serverContext.Request(), initialRequestNumber);
        EXPECT_EQ(clientContext.Request(), initialRequestNumber);
    }

    void verify_state_post(
        const opentxs::api::client::Manager& client,
        const ClientContext& clientContext,
        const ServerContext& serverContext,
        const RequestNumber initialRequestNumber,
        const RequestNumber finalRequestNumber,
        const RequestNumber messageRequestNumber,
        const SendResult messageResult,
        const std::shared_ptr<Message>& message,
        const bool messageSuccess,
        const std::size_t expectedNymboxItems,
        opentxs::RequestNumber& counter)
    {
        EXPECT_EQ(messageRequestNumber, initialRequestNumber);
        EXPECT_EQ(serverContext.Request(), finalRequestNumber);
        EXPECT_EQ(clientContext.Request(), finalRequestNumber);
        EXPECT_EQ(
            serverContext.RemoteNymboxHash(), clientContext.LocalNymboxHash());
        EXPECT_EQ(
            serverContext.LocalNymboxHash(), serverContext.RemoteNymboxHash());
        EXPECT_EQ(SendResult::VALID_REPLY, messageResult);
        ASSERT_TRUE(message);
        EXPECT_EQ(messageSuccess, message->m_bSuccess);

        std::unique_ptr<Ledger> nymbox{
            client.OTAPI().LoadNymbox(server_1_id_, serverContext.Nym()->ID())};

        ASSERT_TRUE(nymbox);
        EXPECT_TRUE(nymbox->VerifyAccount(*serverContext.Nym()));

        const auto& transactionMap = nymbox->GetTransactionMap();

        EXPECT_EQ(expectedNymboxItems, transactionMap.size());

        counter = serverContext.Request();

        EXPECT_FALSE(serverContext.StaleNym());
    }
};

opentxs::RequestNumber Test_Basic::alice_counter_{0};
opentxs::RequestNumber Test_Basic::bob_counter_{0};
const opentxs::ArgList Test_Basic::args_{
    {{OPENTXS_ARG_STORAGE_PLUGIN, {"mem"}}}};
const std::string Test_Basic::SeedA_{""};
const std::string Test_Basic::SeedB_{""};
const std::string Test_Basic::Alice_{""};
const std::string Test_Basic::Bob_{""};
const OTNymID Test_Basic::alice_nym_id_{identifier::Nym::Factory()};
const OTNymID Test_Basic::bob_nym_id_{identifier::Nym::Factory()};
const std::shared_ptr<const ServerContract> Test_Basic::server_contract_{
    nullptr};
const std::shared_ptr<const UnitDefinition> Test_Basic::asset_contract_1_{
    nullptr};
const std::shared_ptr<const UnitDefinition> Test_Basic::asset_contract_2_{
    nullptr};
TransactionNumber Test_Basic::cheque_transaction_number_{0};
std::string Test_Basic::bob_account_1_id_{""};
std::string Test_Basic::bob_account_2_id_{""};
std::string Test_Basic::outgoing_cheque_workflow_id_{};
std::string Test_Basic::incoming_cheque_workflow_id_{};
std::string Test_Basic::outgoing_transfer_workflow_id_{};
std::string Test_Basic::incoming_transfer_workflow_id_{};
std::string Test_Basic::internal_transfer_workflow_id_{};
std::unique_ptr<opentxs::api::client::internal::Operation>
    Test_Basic::alice_state_machine_{nullptr};
std::unique_ptr<opentxs::api::client::internal::Operation>
    Test_Basic::bob_state_machine_{nullptr};
#if OT_CASH
std::shared_ptr<blind::Purse> Test_Basic::untrusted_purse_{};
#endif

TEST_F(Test_Basic, getRequestNumber_not_registered)
{
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_1_id_);
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), alice_nym_id_);

    EXPECT_EQ(serverContext.It().Request(), 0);
    EXPECT_FALSE(clientContext);

    const auto number = serverContext.It().UpdateRequestNumber();

    EXPECT_EQ(0, number);
    EXPECT_EQ(serverContext.It().Request(), 0);

    clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), alice_nym_id_);

    EXPECT_FALSE(clientContext);
}

TEST_F(Test_Basic, registerNym_first_time)
{
    const RequestNumber sequence = alice_counter_;
    const RequestNumber messages{2};
    alice_counter_ += messages;
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), alice_nym_id_);

    EXPECT_EQ(serverContext.It().Request(), sequence);
    EXPECT_FALSE(clientContext);

    ServerContext::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;
    auto started = stateMachine.Start(
        api::client::internal::Operation::Type::RegisterNym, extra_args_);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
    const auto result = translate_result(std::get<0>(finished));
    clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), alice_nym_id_);
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence,
        alice_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        alice_counter_);
}

TEST_F(Test_Basic, getTransactionNumbers_Alice)
{
    const RequestNumber sequence = alice_counter_;
    const RequestNumber messages{5};
    alice_counter_ += messages;
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);
    EXPECT_EQ(context.IssuedNumbers().size(), 0);
    EXPECT_EQ(clientContext->IssuedNumbers().size(), 0);

    verify_state_pre(*clientContext, context, sequence);
    ServerContext::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;
    auto started = stateMachine.Start(
        api::client::internal::Operation::Type::GetTransactionNumbers,
        extra_args_);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
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
        1,
        alice_counter_);
    const auto numbers = context.IssuedNumbers();

    EXPECT_EQ(numbers.size(), 100);

    for (const auto& number : numbers) {
        EXPECT_TRUE(clientContext->VerifyIssuedNumber(number));
    }
}

TEST_F(Test_Basic, Reregister)
{
    const RequestNumber sequence = alice_counter_;
    const RequestNumber messages{5};
    alice_counter_ += messages;
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    ServerContext::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;
    auto started = stateMachine.Start(
        api::client::internal::Operation::Type::RegisterNym, extra_args_);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
    const auto result = translate_result(std::get<0>(finished));
    clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), alice_nym_id_);
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence,
        alice_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        alice_counter_);
}

TEST_F(Test_Basic, issueAsset)
{
    const RequestNumber sequence = alice_counter_;
    const RequestNumber messages{3};
    alice_counter_ += messages;
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    create_unit_definition_1();

    ASSERT_TRUE(asset_contract_1_);

    verify_state_pre(*clientContext, context, sequence);
    ServerContext::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;
    auto serialized = std::make_shared<proto::UnitDefinition>();

    ASSERT_TRUE(serialized);

    *serialized = asset_contract_1_->PublicContract();
    auto started = stateMachine.IssueUnitDefinition(serialized, extra_args_);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
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
    const auto accountID = Identifier::Factory(message->m_strAcctID);

    ASSERT_FALSE(accountID->empty());

    const auto clientAccount = client_1_.Wallet().Account(accountID);
    const auto serverAccount = server_1_.Wallet().Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);

    verify_account(
        *serverContext.It().Nym(),
        *clientContext->Nym(),
        clientAccount,
        serverAccount);
}

TEST_F(Test_Basic, checkNym_missing)
{
    const RequestNumber sequence = alice_counter_;
    const RequestNumber messages{1};
    alice_counter_ += messages;
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);

    ServerContext::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;
    auto started = stateMachine.Start(
        api::client::internal::Operation::Type::CheckNym,
        bob_nym_id_,
        extra_args_);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
    const auto result = translate_result(std::get<0>(finished));
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence,
        alice_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        alice_counter_);

    EXPECT_FALSE(message->m_bBool);
    EXPECT_TRUE(message->m_ascPayload->empty());
}

TEST_F(Test_Basic, publishNym)
{
    const RequestNumber sequence = alice_counter_;
    const RequestNumber messages{1};
    alice_counter_ += messages;
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);

    ServerContext::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;
    auto nym = client_2_.Wallet().Nym(bob_nym_id_);

    ASSERT_TRUE(nym);

    client_1_.Wallet().Nym(nym->asPublicNym());
    auto started = stateMachine.PublishContract(bob_nym_id_);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
    const auto result = translate_result(std::get<0>(finished));
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence,
        alice_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        alice_counter_);
}

TEST_F(Test_Basic, checkNym)
{
    const RequestNumber sequence = alice_counter_;
    const RequestNumber messages{1};
    alice_counter_ += messages;
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);

    ServerContext::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;
    auto started = stateMachine.Start(
        api::client::internal::Operation::Type::CheckNym,
        bob_nym_id_,
        extra_args_);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
    const auto result = translate_result(std::get<0>(finished));
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence,
        alice_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        alice_counter_);

    EXPECT_TRUE(message->m_bBool);
    EXPECT_FALSE(message->m_ascPayload->empty());
}

TEST_F(Test_Basic, downloadServerContract_missing)
{
    const RequestNumber sequence = alice_counter_;
    const RequestNumber messages{1};
    alice_counter_ += messages;
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);

    ServerContext::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;
    auto started =
        stateMachine.DownloadContract(server_2_id_, ContractType::SERVER);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
    const auto result = translate_result(std::get<0>(finished));
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence,
        alice_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        alice_counter_);

    EXPECT_FALSE(message->m_bBool);
    EXPECT_TRUE(message->m_ascPayload->empty());
}

TEST_F(Test_Basic, publishServer)
{
    const RequestNumber sequence = alice_counter_;
    const RequestNumber messages{1};
    alice_counter_ += messages;
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);

    ServerContext::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;
    auto server = server_2_.Wallet().Server(server_2_id_);

    ASSERT_TRUE(server);

    client_1_.Wallet().Server(server->PublicContract());
    auto started = stateMachine.PublishContract(server_2_id_);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
    const auto result = translate_result(std::get<0>(finished));
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence,
        alice_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        alice_counter_);
}

TEST_F(Test_Basic, downloadServerContract)
{
    const RequestNumber sequence = alice_counter_;
    const RequestNumber messages{1};
    alice_counter_ += messages;
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);

    ServerContext::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;
    auto started =
        stateMachine.DownloadContract(server_2_id_, ContractType::SERVER);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
    const auto result = translate_result(std::get<0>(finished));
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence,
        alice_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        alice_counter_);

    EXPECT_TRUE(message->m_bBool);
    EXPECT_FALSE(message->m_ascPayload->empty());
}

TEST_F(Test_Basic, registerNym_Bob)
{
    const RequestNumber sequence = bob_counter_;
    const RequestNumber messages{2};
    bob_counter_ += messages;
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), bob_nym_id_);

    EXPECT_EQ(serverContext.It().Request(), sequence);
    EXPECT_FALSE(clientContext);

    ServerContext::DeliveryResult finished{};
    auto& stateMachine = *bob_state_machine_;
    auto started = stateMachine.Start(
        api::client::internal::Operation::Type::RegisterNym, extra_args_);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
    const auto result = translate_result(std::get<0>(finished));
    clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), bob_nym_id_);
    verify_state_post(
        client_2_,
        *clientContext,
        serverContext.It(),
        sequence,
        bob_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        bob_counter_);
}

TEST_F(Test_Basic, getInstrumentDefinition_missing)
{
    const RequestNumber sequence = bob_counter_;
    const RequestNumber messages{1};
    bob_counter_ += messages;
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), bob_nym_id_);

    ASSERT_TRUE(clientContext);

    create_unit_definition_2();

    ASSERT_TRUE(asset_contract_2_);

    verify_state_pre(*clientContext, context, sequence);
    ServerContext::DeliveryResult finished{};
    auto& stateMachine = *bob_state_machine_;
    auto started = stateMachine.DownloadContract(
        find_unit_definition_id_2(), ContractType::UNIT);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
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

    EXPECT_FALSE(message->m_bBool);
    EXPECT_TRUE(message->m_ascPayload->empty());
}

TEST_F(Test_Basic, publishUnitDefinition)
{
    const RequestNumber sequence = bob_counter_;
    const RequestNumber messages{1};
    bob_counter_ += messages;
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), bob_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);

    ServerContext::DeliveryResult finished{};
    auto& stateMachine = *bob_state_machine_;
    auto started = stateMachine.PublishContract(find_unit_definition_id_2());

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
    const auto result = translate_result(std::get<0>(finished));
    verify_state_post(
        client_2_,
        *clientContext,
        serverContext.It(),
        sequence,
        bob_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        bob_counter_);
}

TEST_F(Test_Basic, getInstrumentDefinition_Alice)
{
    const RequestNumber sequence = alice_counter_;
    const RequestNumber messages{1};
    alice_counter_ += messages;
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, context, sequence);
    ServerContext::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;
    auto started = stateMachine.DownloadContract(
        find_unit_definition_id_2(), ContractType::UNIT);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
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

    EXPECT_TRUE(message->m_bBool);
    EXPECT_FALSE(message->m_ascPayload->empty());
}

TEST_F(Test_Basic, getInstrumentDefinition_Bob)
{
    const RequestNumber sequence = bob_counter_;
    const RequestNumber messages{1};
    bob_counter_ += messages;
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), bob_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, context, sequence);
    ServerContext::DeliveryResult finished{};
    auto& stateMachine = *bob_state_machine_;
    auto started = stateMachine.DownloadContract(
        find_unit_definition_id_1(), ContractType::UNIT);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
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

    EXPECT_TRUE(message->m_bBool);
    EXPECT_FALSE(message->m_ascPayload->empty());
}

TEST_F(Test_Basic, registerAccount)
{
    const RequestNumber sequence = bob_counter_;
    const RequestNumber messages{3};
    bob_counter_ += messages;
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), bob_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, context, sequence);
    ServerContext::DeliveryResult finished{};
    auto& stateMachine = *bob_state_machine_;
    auto started = stateMachine.Start(
        api::client::internal::Operation::Type::RegisterAccount,
        find_unit_definition_id_1(),
        extra_args_);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
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

    const auto accountID = Identifier::Factory(message->m_strAcctID);
    const auto clientAccount = client_2_.Wallet().Account(accountID);
    const auto serverAccount = server_1_.Wallet().Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);

    verify_account(
        *serverContext.It().Nym(),
        *clientContext->Nym(),
        clientAccount,
        serverAccount);

    bob_account_1_id_ = accountID->str();
}

TEST_F(Test_Basic, send_cheque)
{
    const RequestNumber sequence = alice_counter_;
    const RequestNumber messages{1};
    alice_counter_ += messages;
    std::unique_ptr<Cheque> cheque{client_1_.OTAPI().WriteCheque(
        server_1_id_,
        CHEQUE_AMOUNT,
        0,
        0,
        find_issuer_account(),
        alice_nym_id_,
        String::Factory(CHEQUE_MEMO),
        bob_nym_id_)};

    ASSERT_TRUE(cheque);

    cheque_transaction_number_ = cheque->GetTransactionNum();

    EXPECT_NE(0, cheque_transaction_number_);

    std::shared_ptr<OTPayment> payment{
        client_1_.Factory().Payment(String::Factory(*cheque))};

    ASSERT_TRUE(payment);

    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    ServerContext::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;
    auto started = stateMachine.ConveyPayment(bob_nym_id_, payment);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
    const auto result = translate_result(std::get<0>(finished));
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence,
        alice_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        alice_counter_);
    const auto workflowList =
        client_1_.Storage().PaymentWorkflowList(alice_nym_id_->str()).size();

    EXPECT_EQ(1, workflowList);

    const auto workflows = client_1_.Storage().PaymentWorkflowsByState(
        alice_nym_id_->str(),
        proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE,
        proto::PAYMENTWORKFLOWSTATE_CONVEYED);

    ASSERT_EQ(1, workflows.size());

    outgoing_cheque_workflow_id_ = *workflows.begin();
}

TEST_F(Test_Basic, getNymbox_receive_cheque)
{
    const RequestNumber sequence = bob_counter_;
    const RequestNumber messages{4};
    bob_counter_ += messages;
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), bob_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, context, sequence);
    auto queue = context.RefreshNymbox(client_2_);

    ASSERT_TRUE(queue);

    const auto finished = queue->get();
    context.Join();
    context.ResetThread();
    const auto& [status, message] = finished;

    EXPECT_EQ(proto::LASTREPLYSTATUS_MESSAGESUCCESS, status);
    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
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
    const auto workflowList =
        client_2_.Storage().PaymentWorkflowList(bob_nym_id_->str());

    EXPECT_EQ(1, workflowList.size());

    const auto workflows = client_2_.Storage().PaymentWorkflowsByState(
        bob_nym_id_->str(),
        proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE,
        proto::PAYMENTWORKFLOWSTATE_CONVEYED);

    EXPECT_EQ(1, workflows.size());

    incoming_cheque_workflow_id_ = *workflows.begin();
}

TEST_F(Test_Basic, getNymbox_after_clearing_nymbox_2_Bob)
{
    const RequestNumber sequence = bob_counter_;
    const RequestNumber messages{1};
    bob_counter_ += messages;
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), bob_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, context, sequence);
    auto queue = context.RefreshNymbox(client_2_);

    ASSERT_TRUE(queue);

    const auto finished = queue->get();
    context.Join();
    context.ResetThread();
    const auto& [status, message] = finished;

    EXPECT_EQ(proto::LASTREPLYSTATUS_MESSAGESUCCESS, status);
    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
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
}

TEST_F(Test_Basic, depositCheque)
{
    const auto accountID = find_user_account();
    const auto workflowID = Identifier::Factory(incoming_cheque_workflow_id_);
    const auto workflow =
        client_2_.Workflow().LoadWorkflow(bob_nym_id_, workflowID);

    ASSERT_TRUE(workflow);
    ASSERT_TRUE(api::client::Workflow::ContainsCheque(*workflow));

    auto [state, pCheque] =
        api::client::Workflow::InstantiateCheque(client_2_, *workflow);

    ASSERT_EQ(proto::PAYMENTWORKFLOWSTATE_CONVEYED, state);
    ASSERT_TRUE(pCheque);

    std::shared_ptr<Cheque> cheque{std::move(pCheque)};
    const RequestNumber sequence = bob_counter_;
    const RequestNumber messages{12};
    bob_counter_ += messages;
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), bob_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    ServerContext::DeliveryResult finished{};
    auto& stateMachine = *bob_state_machine_;
    auto started = stateMachine.DepositCheque(accountID, cheque);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
    const auto result = translate_result(std::get<0>(finished));
    verify_state_post(
        client_2_,
        *clientContext,
        serverContext.It(),
        sequence + 6,
        bob_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        bob_counter_);
    const auto clientAccount = client_2_.Wallet().Account(accountID);
    const auto serverAccount = server_1_.Wallet().Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);
    EXPECT_EQ(CHEQUE_AMOUNT, serverAccount.get().GetBalance());
    EXPECT_EQ(CHEQUE_AMOUNT, clientAccount.get().GetBalance());

    const auto workflowList =
        client_2_.Storage().PaymentWorkflowList(bob_nym_id_->str()).size();

    EXPECT_EQ(1, workflowList);

    const auto [wType, wState] = client_2_.Storage().PaymentWorkflowState(
        bob_nym_id_->str(), incoming_cheque_workflow_id_);

    EXPECT_EQ(proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE, wType);
    EXPECT_EQ(proto::PAYMENTWORKFLOWSTATE_COMPLETED, wState);
}

TEST_F(Test_Basic, getAccountData_after_cheque_deposited)
{
    const RequestNumber sequence = alice_counter_;
    const RequestNumber messages{5};
    alice_counter_ += messages;
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    const auto accountID = find_issuer_account();

    ASSERT_FALSE(accountID->empty());

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    ServerContext::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;
    auto started = stateMachine.UpdateAccount(accountID);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
    const auto result = translate_result(std::get<0>(finished));
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence + 2,
        alice_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        alice_counter_);
    const auto clientAccount = client_1_.Wallet().Account(accountID);
    const auto serverAccount = server_1_.Wallet().Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);

    verify_account(
        *serverContext.It().Nym(),
        *clientContext->Nym(),
        clientAccount,
        serverAccount);
    const auto workflowList =
        client_1_.Storage().PaymentWorkflowList(alice_nym_id_->str()).size();

    EXPECT_EQ(1, workflowList);

    const auto [wType, wState] = client_1_.Storage().PaymentWorkflowState(
        alice_nym_id_->str(), outgoing_cheque_workflow_id_);

    EXPECT_EQ(wType, proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE);
    // TODO should be completed?
    EXPECT_EQ(wState, proto::PAYMENTWORKFLOWSTATE_ACCEPTED);
}

TEST_F(Test_Basic, resync)
{
    break_consensus();
    const RequestNumber sequence = alice_counter_;
    const RequestNumber messages{2};
    alice_counter_ += messages;
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    ServerContext::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;
    auto started = stateMachine.Start(
        api::client::internal::Operation::Type::RegisterNym, {"", true});

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
    const auto result = translate_result(std::get<0>(finished));
    clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), alice_nym_id_);
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence,
        alice_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        alice_counter_);
}

TEST_F(Test_Basic, sendTransfer)
{
    const RequestNumber sequence = alice_counter_;
    const RequestNumber messages{5};
    alice_counter_ += messages;
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    const auto senderAccountID = find_issuer_account();

    ASSERT_FALSE(senderAccountID->empty());

    const auto recipientAccountID = find_user_account();

    ASSERT_FALSE(recipientAccountID->empty());

    verify_state_pre(*clientContext, context, sequence);
    ServerContext::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;
    auto started = stateMachine.SendTransfer(
        senderAccountID,
        recipientAccountID,
        TRANSFER_AMOUNT,
        String::Factory(TRANSFER_MEMO));

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
    const auto result = translate_result(std::get<0>(finished));
    verify_state_post(
        client_1_,
        *clientContext,
        context,
        sequence + 1,
        alice_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        alice_counter_);
    const auto serverAccount = server_1_.Wallet().Account(senderAccountID);

    ASSERT_TRUE(serverAccount);

    // A successful sent transfer has an immediate effect on the
    // sender's account balance.
    EXPECT_EQ(
        (-1 * (CHEQUE_AMOUNT + TRANSFER_AMOUNT)),
        serverAccount.get().GetBalance());

    const auto workflowList =
        client_1_.Storage().PaymentWorkflowList(alice_nym_id_->str()).size();

    EXPECT_EQ(2, workflowList);

    const auto workflows = client_1_.Storage().PaymentWorkflowsByState(
        alice_nym_id_->str(),
        proto::PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER,
        proto::PAYMENTWORKFLOWSTATE_ACKNOWLEDGED);

    ASSERT_EQ(1, workflows.size());

    outgoing_transfer_workflow_id_ = *workflows.cbegin();

    EXPECT_NE(outgoing_transfer_workflow_id_.size(), 0);

    auto workflow = client_1_.Workflow().LoadWorkflow(
        alice_nym_id_, Identifier::Factory(outgoing_transfer_workflow_id_));

    ASSERT_TRUE(workflow);

    EXPECT_EQ(0, workflow->party_size());
}

TEST_F(Test_Basic, getAccountData_after_incomingTransfer)
{
    const RequestNumber sequence = bob_counter_;
    const RequestNumber messages{5};
    bob_counter_ += messages;
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), bob_nym_id_);

    ASSERT_TRUE(clientContext);

    const auto accountID = find_user_account();

    ASSERT_FALSE(accountID->empty());

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    ServerContext::DeliveryResult finished{};
    auto& stateMachine = *bob_state_machine_;
    auto started = stateMachine.UpdateAccount(accountID);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
    const auto result = translate_result(std::get<0>(finished));
    verify_state_post(
        client_2_,
        *clientContext,
        serverContext.It(),
        sequence + 2,
        bob_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        bob_counter_);
    const auto clientAccount = client_2_.Wallet().Account(accountID);
    const auto serverAccount = server_1_.Wallet().Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);

    verify_account(
        *serverContext.It().Nym(),
        *clientContext->Nym(),
        clientAccount,
        serverAccount);

    EXPECT_EQ(
        (CHEQUE_AMOUNT + TRANSFER_AMOUNT), serverAccount.get().GetBalance());

    const auto workflowList =
        client_2_.Storage().PaymentWorkflowList(bob_nym_id_->str()).size();

    EXPECT_EQ(2, workflowList);

    const auto workflows = client_2_.Storage().PaymentWorkflowsByState(
        bob_nym_id_->str(),
        proto::PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER,
        proto::PAYMENTWORKFLOWSTATE_COMPLETED);

    ASSERT_EQ(1, workflows.size());

    incoming_transfer_workflow_id_ = *workflows.cbegin();

    EXPECT_NE(incoming_transfer_workflow_id_.size(), 0);

    auto workflow = client_2_.Workflow().LoadWorkflow(
        bob_nym_id_, Identifier::Factory(incoming_transfer_workflow_id_));

    ASSERT_TRUE(workflow);

    EXPECT_EQ(1, workflow->party_size());
    EXPECT_STREQ(alice_nym_id_->str().c_str(), workflow->party(0).c_str());
    EXPECT_EQ(workflow->type(), proto::PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER);
    EXPECT_EQ(workflow->state(), proto::PAYMENTWORKFLOWSTATE_COMPLETED);
}

TEST_F(Test_Basic, getAccountData_after_transfer_accepted)
{
    const RequestNumber sequence = alice_counter_;
    const RequestNumber messages{5};
    alice_counter_ += messages;
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    const auto accountID = find_issuer_account();

    ASSERT_FALSE(accountID->empty());

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    ServerContext::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;
    auto started = stateMachine.UpdateAccount(accountID);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
    const auto result = translate_result(std::get<0>(finished));
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence + 2,
        alice_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        alice_counter_);
    const auto clientAccount = client_1_.Wallet().Account(accountID);
    const auto serverAccount = server_1_.Wallet().Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);

    verify_account(
        *serverContext.It().Nym(),
        *clientContext->Nym(),
        clientAccount,
        serverAccount);

    EXPECT_EQ(
        -1 * (CHEQUE_AMOUNT + TRANSFER_AMOUNT),
        serverAccount.get().GetBalance());

    const auto workflowList =
        client_2_.Storage().PaymentWorkflowList(bob_nym_id_->str()).size();

    EXPECT_EQ(2, workflowList);

    const auto [type, state] = client_1_.Storage().PaymentWorkflowState(
        alice_nym_id_->str(), outgoing_transfer_workflow_id_);

    EXPECT_EQ(type, proto::PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER);
    EXPECT_EQ(state, proto::PAYMENTWORKFLOWSTATE_COMPLETED);
}

TEST_F(Test_Basic, register_second_account)
{
    const RequestNumber sequence = bob_counter_;
    const RequestNumber messages{3};
    bob_counter_ += messages;
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), bob_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, context, sequence);
    ServerContext::DeliveryResult finished{};
    auto& stateMachine = *bob_state_machine_;
    auto started = stateMachine.Start(
        api::client::internal::Operation::Type::RegisterAccount,
        find_unit_definition_id_1(),
        extra_args_);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
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
    const auto accountID = Identifier::Factory(message->m_strAcctID);
    const auto clientAccount = client_2_.Wallet().Account(accountID);
    const auto serverAccount = server_1_.Wallet().Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);

    verify_account(
        *serverContext.It().Nym(),
        *clientContext->Nym(),
        clientAccount,
        serverAccount);

    bob_account_2_id_ = accountID->str();
}

TEST_F(Test_Basic, send_internal_transfer)
{
    const RequestNumber sequence = bob_counter_;
    const RequestNumber messages{5};
    bob_counter_ += messages;
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), bob_nym_id_);

    ASSERT_TRUE(clientContext);

    const auto senderAccountID = find_user_account();

    ASSERT_FALSE(senderAccountID->empty());

    const auto recipientAccountID = find_second_user_account();

    ASSERT_FALSE(recipientAccountID->empty());

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    ServerContext::DeliveryResult finished{};
    auto& stateMachine = *bob_state_machine_;
    auto started = stateMachine.SendTransfer(
        senderAccountID,
        recipientAccountID,
        SECOND_TRANSFER_AMOUNT,
        String::Factory(TRANSFER_MEMO));

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
    const auto result = translate_result(std::get<0>(finished));
    verify_state_post(
        client_2_,
        *clientContext,
        serverContext.It(),
        sequence + 1,
        bob_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        bob_counter_);
    const auto serverAccount = server_1_.Wallet().Account(senderAccountID);

    ASSERT_TRUE(serverAccount);

    // A successful sent transfer has an immediate effect on the sender's
    // account balance.
    EXPECT_EQ(
        (CHEQUE_AMOUNT + TRANSFER_AMOUNT - SECOND_TRANSFER_AMOUNT),
        serverAccount.get().GetBalance());

    std::size_t count{0}, tries{100};
    std::set<std::string> workflows{};
    while (0 == count) {
        // The state change from ACKNOWLEDGED to CONVEYED occurs asynchronously
        // due to server push notifications so the order in which these states
        // are observed by the sender is undefined.
        Log::Sleep(std::chrono::milliseconds(100));
        workflows = client_2_.Storage().PaymentWorkflowsByState(
            bob_nym_id_->str(),
            proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER,
            proto::PAYMENTWORKFLOWSTATE_ACKNOWLEDGED);
        count = workflows.size();

        if (0 == count) {
            workflows = client_2_.Storage().PaymentWorkflowsByState(
                bob_nym_id_->str(),
                proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER,
                proto::PAYMENTWORKFLOWSTATE_CONVEYED);
            count = workflows.size();
        }

        if (0 == --tries) { break; }
    }

    ASSERT_EQ(count, 1);

    internal_transfer_workflow_id_ = *workflows.cbegin();

    EXPECT_NE(internal_transfer_workflow_id_.size(), 0);

    const auto workflowList =
        client_2_.Storage().PaymentWorkflowList(bob_nym_id_->str()).size();

    EXPECT_EQ(3, workflowList);
}

TEST_F(Test_Basic, getAccountData_after_incoming_internal_Transfer)
{
    const RequestNumber sequence = bob_counter_;
    const RequestNumber messages{5};
    bob_counter_ += messages;
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), bob_nym_id_);

    ASSERT_TRUE(clientContext);

    const auto accountID = find_second_user_account();

    ASSERT_FALSE(accountID->empty());

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    ServerContext::DeliveryResult finished{};
    auto& stateMachine = *bob_state_machine_;
    auto started = stateMachine.UpdateAccount(accountID);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
    const auto result = translate_result(std::get<0>(finished));
    verify_state_post(
        client_2_,
        *clientContext,
        serverContext.It(),
        sequence + 2,
        bob_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        bob_counter_);
    const auto clientAccount = client_2_.Wallet().Account(accountID);
    const auto serverAccount = server_1_.Wallet().Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);

    verify_account(
        *serverContext.It().Nym(),
        *clientContext->Nym(),
        clientAccount,
        serverAccount);
    const auto workflowList =
        client_2_.Storage().PaymentWorkflowList(bob_nym_id_->str()).size();

    EXPECT_EQ(3, workflowList);

    const auto [type, state] = client_2_.Storage().PaymentWorkflowState(
        bob_nym_id_->str(), internal_transfer_workflow_id_);

    EXPECT_EQ(type, proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER);
    EXPECT_EQ(state, proto::PAYMENTWORKFLOWSTATE_CONVEYED);
    EXPECT_EQ(SECOND_TRANSFER_AMOUNT, serverAccount.get().GetBalance());
}

TEST_F(Test_Basic, getAccountData_after_internal_transfer_accepted)
{
    const RequestNumber sequence = bob_counter_;
    const RequestNumber messages{5};
    bob_counter_ += messages;
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), bob_nym_id_);

    ASSERT_TRUE(clientContext);

    const auto accountID = find_user_account();

    ASSERT_FALSE(accountID->empty());

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    ServerContext::DeliveryResult finished{};
    auto& stateMachine = *bob_state_machine_;
    auto started = stateMachine.UpdateAccount(accountID);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
    const auto result = translate_result(std::get<0>(finished));
    verify_state_post(
        client_2_,
        *clientContext,
        serverContext.It(),
        sequence + 2,
        bob_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        bob_counter_);
    const auto clientAccount = client_2_.Wallet().Account(accountID);
    const auto serverAccount = server_1_.Wallet().Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);

    verify_account(
        *serverContext.It().Nym(),
        *clientContext->Nym(),
        clientAccount,
        serverAccount);
    const auto workflowList =
        client_2_.Storage().PaymentWorkflowList(bob_nym_id_->str()).size();

    EXPECT_EQ(3, workflowList);

    const auto [type, state] = client_2_.Storage().PaymentWorkflowState(
        bob_nym_id_->str(), internal_transfer_workflow_id_);

    EXPECT_EQ(type, proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER);
    EXPECT_EQ(state, proto::PAYMENTWORKFLOWSTATE_COMPLETED);
    EXPECT_EQ(
        CHEQUE_AMOUNT + TRANSFER_AMOUNT - SECOND_TRANSFER_AMOUNT,
        serverAccount.get().GetBalance());
}

TEST_F(Test_Basic, send_message)
{
    const RequestNumber sequence = alice_counter_;
    const RequestNumber messages{1};
    alice_counter_ += messages;
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    auto& stateMachine = *alice_state_machine_;
    auto messageID = Identifier::Factory();
    auto setID = [&](const Identifier& in) -> void { messageID = in; };
    auto started = stateMachine.SendMessage(
        bob_nym_id_, String::Factory(MESSAGE_TEXT), setID);

    ASSERT_TRUE(started);

    const auto finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
    const auto result = translate_result(std::get<0>(finished));
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence,
        alice_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        alice_counter_);

    EXPECT_FALSE(messageID->empty());
}

TEST_F(Test_Basic, receive_message)
{
    const RequestNumber sequence = bob_counter_;
    const RequestNumber messages{4};
    bob_counter_ += messages;
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), bob_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, context, sequence);
    auto queue = context.RefreshNymbox(client_2_);

    ASSERT_TRUE(queue);

    const auto finished = queue->get();
    context.Join();
    context.ResetThread();
    const auto& [status, message] = finished;

    EXPECT_EQ(proto::LASTREPLYSTATUS_MESSAGESUCCESS, status);
    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
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
    const auto mailList =
        client_2_.Activity().Mail(bob_nym_id_, StorageBox::MAILINBOX);

    ASSERT_EQ(1, mailList.size());

    const auto mailID = Identifier::Factory(std::get<0>(*mailList.begin()));
    const auto text = client_2_.Activity().MailText(
        bob_nym_id_, mailID, StorageBox::MAILINBOX);

    ASSERT_TRUE(text);
    EXPECT_STREQ(MESSAGE_TEXT, text->c_str());
}

TEST_F(Test_Basic, request_admin_wrong_password)
{
    const RequestNumber sequence = alice_counter_;
    const RequestNumber messages{1};
    alice_counter_ += messages;
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    auto& stateMachine = *alice_state_machine_;
    auto started = stateMachine.RequestAdmin(String::Factory("WRONG PASSWORD"));

    ASSERT_TRUE(started);

    const auto finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
    const auto result = translate_result(std::get<0>(finished));
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence,
        alice_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        alice_counter_);

    EXPECT_FALSE(message->m_bBool);
    EXPECT_TRUE(context.AdminAttempted());
    EXPECT_FALSE(context.isAdmin());
}

TEST_F(Test_Basic, request_admin)
{
    const RequestNumber sequence = alice_counter_;
    const RequestNumber messages{1};
    alice_counter_ += messages;
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    auto& stateMachine = *alice_state_machine_;
    auto started = stateMachine.RequestAdmin(
        String::Factory(server_1_.GetAdminPassword()));

    ASSERT_TRUE(started);

    const auto finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
    const auto result = translate_result(std::get<0>(finished));
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence,
        alice_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        alice_counter_);

    EXPECT_TRUE(message->m_bBool);
    EXPECT_TRUE(context.AdminAttempted());
    EXPECT_TRUE(context.isAdmin());
}

TEST_F(Test_Basic, request_admin_already_admin)
{
    const RequestNumber sequence = alice_counter_;
    const RequestNumber messages{1};
    alice_counter_ += messages;
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    auto& stateMachine = *alice_state_machine_;
    auto started = stateMachine.RequestAdmin(
        String::Factory(server_1_.GetAdminPassword()));

    ASSERT_TRUE(started);

    const auto finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
    const auto result = translate_result(std::get<0>(finished));
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence,
        alice_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        alice_counter_);

    EXPECT_TRUE(message->m_bBool);
    EXPECT_TRUE(context.AdminAttempted());
    EXPECT_TRUE(context.isAdmin());
}

TEST_F(Test_Basic, request_admin_second_nym)
{
    const RequestNumber sequence = bob_counter_;
    const RequestNumber messages{1};
    bob_counter_ += messages;
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), bob_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    auto& stateMachine = *bob_state_machine_;
    auto started = stateMachine.RequestAdmin(
        String::Factory(server_1_.GetAdminPassword()));

    ASSERT_TRUE(started);

    const auto finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
    const auto result = translate_result(std::get<0>(finished));
    verify_state_post(
        client_2_,
        *clientContext,
        serverContext.It(),
        sequence,
        bob_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        bob_counter_);

    EXPECT_FALSE(message->m_bBool);
    EXPECT_TRUE(context.AdminAttempted());
    EXPECT_FALSE(context.isAdmin());
}

TEST_F(Test_Basic, addClaim)
{
    const RequestNumber sequence = alice_counter_;
    const RequestNumber messages{1};
    alice_counter_ += messages;
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    auto& stateMachine = *alice_state_machine_;
    auto started = stateMachine.AddClaim(
        proto::CONTACTSECTION_SCOPE,
        proto::CITEMTYPE_SERVER,
        String::Factory(NEW_SERVER_NAME),
        true);

    ASSERT_TRUE(started);

    const auto finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
    const auto result = translate_result(std::get<0>(finished));
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence,
        alice_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        alice_counter_);

    EXPECT_TRUE(message->m_bBool);
}

TEST_F(Test_Basic, renameServer)
{
    const RequestNumber sequence = alice_counter_;
    const RequestNumber messages{1};
    alice_counter_ += messages;
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);

    ServerContext::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;
    // TODO nym id type
    auto started = stateMachine.Start(
        api::client::internal::Operation::Type::CheckNym,
        identifier::Nym::Factory(context.RemoteNym().ID().str()),
        extra_args_);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
    const auto result = translate_result(std::get<0>(finished));
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence,
        alice_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        alice_counter_);

    EXPECT_TRUE(message->m_bBool);
    EXPECT_FALSE(message->m_ascPayload->empty());

    const auto server = client_1_.Wallet().Server(server_1_id_);

    ASSERT_TRUE(server);
    EXPECT_STREQ(NEW_SERVER_NAME, server->EffectiveName().c_str());
}

TEST_F(Test_Basic, addClaim_not_admin)
{
    const RequestNumber sequence = bob_counter_;
    const RequestNumber messages{1};
    bob_counter_ += messages;
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), bob_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    auto& stateMachine = *bob_state_machine_;
    auto started = stateMachine.AddClaim(
        proto::CONTACTSECTION_SCOPE,
        proto::CITEMTYPE_SERVER,
        String::Factory(NEW_SERVER_NAME),
        true);

    ASSERT_TRUE(started);

    const auto finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
    const auto result = translate_result(std::get<0>(finished));
    verify_state_post(
        client_2_,
        *clientContext,
        serverContext.It(),
        sequence,
        bob_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        bob_counter_);

    EXPECT_TRUE(message->m_bSuccess);
    EXPECT_FALSE(message->m_bBool);
}

TEST_F(Test_Basic, initiate_and_acknowledge_bailment)
{
    const auto aliceNym = client_1_.Wallet().Nym(alice_nym_id_);

    ASSERT_TRUE(aliceNym);

    std::shared_ptr<PeerRequest> peerrequest{PeerRequest::Create(
        client_1_.Wallet(),
        aliceNym,
        proto::PEERREQUEST_BAILMENT,
        find_unit_definition_id_2(),
        server_1_id_)};

    ASSERT_TRUE(peerrequest);

    ProtoHasRequest protohasrequest = &proto::PeerRequest::has_bailment;
    send_peer_request(
        peerrequest, protohasrequest, proto::PEERREQUEST_BAILMENT);
    receive_request(peerrequest, protohasrequest, proto::PEERREQUEST_BAILMENT);

    const auto bobNym = client_2_.Wallet().Nym(bob_nym_id_);

    ASSERT_TRUE(bobNym);

    std::shared_ptr<const PeerReply> peerreply{PeerReply::Create(
        client_2_.Wallet(),
        bobNym,
        proto::PEERREQUEST_BAILMENT,
        peerrequest->ID(),
        server_1_id_,
        "instructions")};

    ASSERT_TRUE(peerreply);

    ProtoHasReply protohasreply = &proto::PeerReply::has_bailment;
    send_peer_reply(
        peerreply, peerrequest, protohasreply, proto::PEERREQUEST_BAILMENT);
    receive_reply(
        peerreply,
        peerrequest,
        protohasreply,
        protohasrequest,
        proto::PEERREQUEST_BAILMENT);
}

TEST_F(Test_Basic, initiate_and_acknowledge_outbailment)
{
    const auto aliceNym = client_1_.Wallet().Nym(alice_nym_id_);

    ASSERT_TRUE(aliceNym);

    std::shared_ptr<PeerRequest> peerrequest{PeerRequest::Create(
        client_1_.Wallet(),
        aliceNym,
        proto::PEERREQUEST_OUTBAILMENT,
        find_unit_definition_id_2(),
        server_1_id_,
        1000,
        "message")};

    ASSERT_TRUE(peerrequest);

    ProtoHasRequest protohasrequest = &proto::PeerRequest::has_outbailment;
    send_peer_request(
        peerrequest, protohasrequest, proto::PEERREQUEST_OUTBAILMENT);
    receive_request(
        peerrequest, protohasrequest, proto::PEERREQUEST_OUTBAILMENT);

    const auto bobNym = client_2_.Wallet().Nym(bob_nym_id_);

    ASSERT_TRUE(bobNym);

    std::shared_ptr<const PeerReply> peerreply{PeerReply::Create(
        client_2_.Wallet(),
        bobNym,
        proto::PEERREQUEST_OUTBAILMENT,
        peerrequest->ID(),
        server_1_id_,
        "details")};

    ASSERT_TRUE(peerreply);

    ProtoHasReply protohasreply = &proto::PeerReply::has_outbailment;
    send_peer_reply(
        peerreply, peerrequest, protohasreply, proto::PEERREQUEST_OUTBAILMENT);
    receive_reply(
        peerreply,
        peerrequest,
        protohasreply,
        protohasrequest,
        proto::PEERREQUEST_OUTBAILMENT);
}

TEST_F(Test_Basic, notify_bailment_and_acknowledge_notice)
{
    const auto aliceNym = client_1_.Wallet().Nym(alice_nym_id_);

    ASSERT_TRUE(aliceNym);

    std::shared_ptr<PeerRequest> peerrequest{PeerRequest::Create(
        client_1_.Wallet(),
        aliceNym,
        proto::PEERREQUEST_PENDINGBAILMENT,
        find_unit_definition_id_2(),
        server_1_id_,
        bob_nym_id_,
        Identifier::Random(),
        Identifier::Random()->str(),
        1000)};

    ASSERT_TRUE(peerrequest);

    ProtoHasRequest protohasrequest = &proto::PeerRequest::has_pendingbailment;
    send_peer_request(
        peerrequest, protohasrequest, proto::PEERREQUEST_PENDINGBAILMENT);
    receive_request(
        peerrequest, protohasrequest, proto::PEERREQUEST_PENDINGBAILMENT);

    const auto bobNym = client_2_.Wallet().Nym(bob_nym_id_);

    ASSERT_TRUE(bobNym);

    std::shared_ptr<const PeerReply> peerreply{PeerReply::Create(
        client_2_.Wallet(), bobNym, peerrequest->ID(), server_1_id_, true)};

    ASSERT_TRUE(peerreply);

    ProtoHasReply protohasreply = &proto::PeerReply::has_notice;
    send_peer_reply(
        peerreply,
        peerrequest,
        protohasreply,
        proto::PEERREQUEST_PENDINGBAILMENT);
    receive_reply(
        peerreply,
        peerrequest,
        protohasreply,
        protohasrequest,
        proto::PEERREQUEST_PENDINGBAILMENT);
}

TEST_F(Test_Basic, initiate_request_connection_and_acknowledge_connection)
{
    const auto aliceNym = client_1_.Wallet().Nym(alice_nym_id_);

    ASSERT_TRUE(aliceNym);

    std::shared_ptr<PeerRequest> peerrequest{PeerRequest::Create(
        client_1_.Wallet(),
        aliceNym,
        proto::PEERREQUEST_CONNECTIONINFO,
        proto::CONNECTIONINFO_BITCOIN,
        bob_nym_id_,
        server_1_id_)};

    ASSERT_TRUE(peerrequest);

    ProtoHasRequest protohasrequest = &proto::PeerRequest::has_connectioninfo;
    send_peer_request(
        peerrequest, protohasrequest, proto::PEERREQUEST_CONNECTIONINFO);
    receive_request(
        peerrequest, protohasrequest, proto::PEERREQUEST_CONNECTIONINFO);

    const auto bobNym = client_2_.Wallet().Nym(bob_nym_id_);

    ASSERT_TRUE(bobNym);

    std::shared_ptr<const PeerReply> peerreply{PeerReply::Create(
        client_2_.Wallet(),
        bobNym,
        peerrequest->ID(),
        server_1_id_,
        true,
        "localhost",
        "user",
        "password",
        "key")};

    ASSERT_TRUE(peerreply);

    ProtoHasReply protohasreply = &proto::PeerReply::has_connectioninfo;
    send_peer_reply(
        peerreply,
        peerrequest,
        protohasreply,
        proto::PEERREQUEST_CONNECTIONINFO);
    receive_reply(
        peerreply,
        peerrequest,
        protohasreply,
        protohasrequest,
        proto::PEERREQUEST_CONNECTIONINFO);
}

TEST_F(Test_Basic, initiate_store_secret_and_acknowledge_notice)
{
    const auto aliceNym = client_1_.Wallet().Nym(alice_nym_id_);

    ASSERT_TRUE(aliceNym);

    std::shared_ptr<PeerRequest> peerrequest{PeerRequest::Create(
        client_1_.Wallet(),
        aliceNym,
        proto::PEERREQUEST_STORESECRET,
        proto::SECRETTYPE_BIP39,
        bob_nym_id_,
        TEST_SEED,
        TEST_SEED_PASSPHRASE,
        server_1_id_)};

    ASSERT_TRUE(peerrequest);

    ProtoHasRequest protohasrequest = &proto::PeerRequest::has_storesecret;
    send_peer_request(
        peerrequest, protohasrequest, proto::PEERREQUEST_STORESECRET);
    receive_request(
        peerrequest, protohasrequest, proto::PEERREQUEST_STORESECRET);

    const auto bobNym = client_2_.Wallet().Nym(bob_nym_id_);

    ASSERT_TRUE(bobNym);

    std::shared_ptr<const PeerReply> peerreply{PeerReply::Create(
        client_2_.Wallet(), bobNym, peerrequest->ID(), server_1_id_, true)};

    ASSERT_TRUE(peerreply);

    ProtoHasReply protohasreply = &proto::PeerReply::has_notice;
    send_peer_reply(
        peerreply, peerrequest, protohasreply, proto::PEERREQUEST_STORESECRET);
    receive_reply(
        peerreply,
        peerrequest,
        protohasreply,
        protohasrequest,
        proto::PEERREQUEST_STORESECRET);
}

#if OT_CASH
TEST_F(Test_Basic, waitForCash_Alice)
{
    auto mint = server_1_.GetPublicMint(find_unit_definition_id_1());
    const auto start = std::chrono::system_clock::now();
    std::cout << "Pausing for up to " << MINT_TIME_LIMIT_MINUTES
              << " minutes until mint generation is finished." << std::endl;

    while (false == bool(mint)) {
        std::cout << "* Waiting for mint..." << std::endl;
        Log::Sleep(std::chrono::seconds(10));
        mint = server_1_.GetPublicMint(find_unit_definition_id_1());
        const auto wait = std::chrono::system_clock::now() - start;
        const auto limit = std::chrono::minutes(MINT_TIME_LIMIT_MINUTES);

        if (wait > limit) { break; }
    }

    ASSERT_TRUE(mint);
}

TEST_F(Test_Basic, downloadMint)
{
    const RequestNumber sequence = bob_counter_;
    const RequestNumber messages{1};
    bob_counter_ += messages;
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), bob_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    auto& stateMachine = *bob_state_machine_;
    auto started = stateMachine.Start(
        api::client::internal::Operation::Type::DownloadMint,
        find_unit_definition_id_1(),
        {});

    ASSERT_TRUE(started);

    const auto finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
    const auto result = translate_result(std::get<0>(finished));
    verify_state_post(
        client_2_,
        *clientContext,
        serverContext.It(),
        sequence,
        bob_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        bob_counter_);

    EXPECT_TRUE(message->m_bSuccess);
    EXPECT_TRUE(message->m_bBool);
}

TEST_F(Test_Basic, withdrawCash)
{
    const RequestNumber sequence = bob_counter_;
    const RequestNumber messages{4};
    bob_counter_ += messages;
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), bob_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    ServerContext::DeliveryResult finished{};
    auto& stateMachine = *bob_state_machine_;
    const auto accountID = find_user_account();
    auto started = stateMachine.WithdrawCash(accountID, CASH_AMOUNT);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
    const auto result = translate_result(std::get<0>(finished));
    verify_state_post(
        client_2_,
        *clientContext,
        serverContext.It(),
        sequence + 1,
        bob_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        bob_counter_);

    const auto clientAccount = client_2_.Wallet().Account(accountID);
    const auto serverAccount = server_1_.Wallet().Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);
    EXPECT_EQ(
        CHEQUE_AMOUNT + TRANSFER_AMOUNT - SECOND_TRANSFER_AMOUNT - CASH_AMOUNT,
        serverAccount.get().GetBalance());
    EXPECT_EQ(
        serverAccount.get().GetBalance(), clientAccount.get().GetBalance());
    ASSERT_TRUE(asset_contract_1_);

    // TODO unit id type
    auto purseEditor = context.mutable_Purse(
        identifier::UnitDefinition::Factory(asset_contract_1_->ID()->str()));
    auto& purse = purseEditor.It();

    EXPECT_EQ(purse.Value(), CASH_AMOUNT);
}

TEST_F(Test_Basic, send_cash)
{
    const RequestNumber sequence = bob_counter_;
    const RequestNumber messages{1};
    bob_counter_ += messages;
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), bob_nym_id_);

    ASSERT_TRUE(clientContext);

    const auto& bob = *context.Nym();

    // TODO unit id type
    const auto unitID =
        identifier::UnitDefinition::Factory(asset_contract_1_->ID()->str());
    auto localPurseEditor = context.mutable_Purse(unitID);
    auto& localPurse = localPurseEditor.It();

    ASSERT_TRUE(localPurse.Unlock(bob));

    for (const auto& token : localPurse) { EXPECT_FALSE(token.ID().empty()); }

    std::shared_ptr<blind::Purse> pSendPurse{
        client_2_.Factory().Purse(bob, server_1_id_, unitID)};

    ASSERT_TRUE(pSendPurse);

    auto& sendPurse = *pSendPurse;

    ASSERT_TRUE(localPurse.IsUnlocked());

    auto token = localPurse.Pop();

    while (token) {
        const auto added = sendPurse.Push(token);

        EXPECT_TRUE(added);

        token = localPurse.Pop();
    }

    EXPECT_EQ(sendPurse.Value(), CASH_AMOUNT);

    const auto workflowID =
        client_2_.Workflow().AllocateCash(bob_nym_id_, sendPurse);

    EXPECT_FALSE(workflowID->empty());

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    ServerContext::DeliveryResult finished{};
    auto& stateMachine = *bob_state_machine_;
    auto started = stateMachine.SendCash(alice_nym_id_, workflowID);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
    const auto result = translate_result(std::get<0>(finished));
    verify_state_post(
        client_2_,
        *clientContext,
        serverContext.It(),
        sequence,
        bob_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        bob_counter_);

    EXPECT_EQ(localPurse.Value(), 0);

    auto workflow = client_2_.Workflow().LoadWorkflow(bob_nym_id_, workflowID);

    ASSERT_TRUE(workflow);
    EXPECT_EQ(proto::PAYMENTWORKFLOWSTATE_CONVEYED, workflow->state());
}

TEST_F(Test_Basic, receive_cash)
{
    const RequestNumber sequence = alice_counter_;
    const RequestNumber messages{4};
    alice_counter_ += messages;
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, context, sequence);
    auto queue = context.RefreshNymbox(client_1_);

    ASSERT_TRUE(queue);

    const auto finished = queue->get();
    context.Join();
    context.ResetThread();
    const auto& [status, message] = finished;

    EXPECT_EQ(proto::LASTREPLYSTATUS_MESSAGESUCCESS, status);
    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
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
    // TODO unit id type
    const auto unitID =
        identifier::UnitDefinition::Factory(asset_contract_1_->ID()->str());

    const auto workflows = client_1_.Storage().PaymentWorkflowsByState(
        alice_nym_id_->str(),
        proto::PAYMENTWORKFLOWTYPE_INCOMINGCASH,
        proto::PAYMENTWORKFLOWSTATE_CONVEYED);

    ASSERT_EQ(1, workflows.size());

    const auto& workflowID = Identifier::Factory(*workflows.begin());
    const auto pWorkflow =
        client_1_.Workflow().LoadWorkflow(alice_nym_id_, workflowID);

    ASSERT_TRUE(pWorkflow);

    const auto& workflow = *pWorkflow;
    auto [state, pPurse] =
        client_1_.Workflow().InstantiatePurse(client_1_, workflow);

    ASSERT_TRUE(pPurse);

    auto& incomingPurse = *pPurse;
    auto purseEditor = context.mutable_Purse(unitID);
    auto& walletPurse = purseEditor.It();
    const auto& alice = *context.Nym();

    ASSERT_TRUE(incomingPurse.Unlock(alice));
    ASSERT_TRUE(walletPurse.Unlock(alice));

    auto token = incomingPurse.Pop();

    while (token) {
        EXPECT_TRUE(walletPurse.Push(token));

        token = incomingPurse.Pop();
    }

    EXPECT_EQ(walletPurse.Value(), CASH_AMOUNT);
}

TEST_F(Test_Basic, depositCash)
{
    const RequestNumber sequence = alice_counter_;
    const RequestNumber messages{4};
    alice_counter_ += messages;
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_1_id_);
    auto& context = serverContext.It();
    auto clientContext =
        server_1_.Wallet().ClientContext(server_1_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);
    ASSERT_TRUE(asset_contract_1_);

    const auto& alice = *context.Nym();
    const auto accountID = find_issuer_account();
    // TODO unit ID type
    const auto unitID =
        identifier::UnitDefinition::Factory(asset_contract_1_->ID()->str());
    auto pPurse = context.Purse(unitID);

    ASSERT_TRUE(pPurse);

    auto& walletPurse = *pPurse;

    OTPurse copy{walletPurse};
    std::shared_ptr<blind::Purse> pSendPurse{
        client_2_.Factory().Purse(alice, server_1_id_, unitID)};

    ASSERT_TRUE(pSendPurse);

    auto& sendPurse = *pSendPurse;

    ASSERT_TRUE(walletPurse.Unlock(alice));
    ASSERT_TRUE(copy->Unlock(alice));
    ASSERT_TRUE(sendPurse.Unlock(alice));

    auto token = copy->Pop();

    while (token) {
        ASSERT_TRUE(sendPurse.Push(token));

        token = copy->Pop();
    }

    EXPECT_EQ(copy->Value(), 0);
    EXPECT_EQ(sendPurse.Value(), CASH_AMOUNT);
    EXPECT_EQ(walletPurse.Value(), CASH_AMOUNT);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    ServerContext::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;
    auto started = stateMachine.DepositCash(accountID, pSendPurse);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const RequestNumber requestNumber =
        String::StringToUlong(message->m_strRequestNum->Get());
    const auto result = translate_result(std::get<0>(finished));
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence + 1,
        alice_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        alice_counter_);
    const auto clientAccount = client_1_.Wallet().Account(accountID);
    const auto serverAccount = server_1_.Wallet().Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);

    EXPECT_EQ(
        -1 * (CHEQUE_AMOUNT + TRANSFER_AMOUNT - CASH_AMOUNT),
        serverAccount.get().GetBalance());
    EXPECT_EQ(
        serverAccount.get().GetBalance(), clientAccount.get().GetBalance());

    auto pWalletPurse = context.Purse(unitID);

    ASSERT_TRUE(pWalletPurse);
    EXPECT_EQ(pWalletPurse->Value(), 0);
}
#endif

TEST_F(Test_Basic, cleanup)
{
    alice_state_machine_.reset();
    bob_state_machine_.reset();
}
}  // namespace
