// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <algorithm>
#include <chrono>
#include <compare>
#include <ctime>
#include <future>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>

#include "2_Factory.hpp"
#include "internal/api/FactoryAPI.hpp"
#include "internal/api/session/Client.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/Armored.hpp"
#include "internal/core/String.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/core/contract/peer/BailmentNotice.hpp"
#include "internal/core/contract/peer/BailmentReply.hpp"
#include "internal/core/contract/peer/BailmentRequest.hpp"
#include "internal/core/contract/peer/ConnectionReply.hpp"
#include "internal/core/contract/peer/ConnectionRequest.hpp"
#include "internal/core/contract/peer/NoticeAcknowledgement.hpp"
#include "internal/core/contract/peer/OutBailmentReply.hpp"
#include "internal/core/contract/peer/OutBailmentRequest.hpp"
#include "internal/core/contract/peer/PeerReply.hpp"
#include "internal/core/contract/peer/PeerRequest.hpp"
#include "internal/core/contract/peer/StoreSecret.hpp"
#include "internal/otx/client/Client.hpp"
#include "internal/otx/client/OTPayment.hpp"
#include "internal/otx/client/Pair.hpp"
#include "internal/otx/client/obsolete/OTAPI_Exec.hpp"
#include "internal/otx/client/obsolete/OT_API.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/otx/common/Cheque.hpp"
#include "internal/otx/common/Ledger.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/otx/consensus/Base.hpp"
#include "internal/otx/consensus/Client.hpp"
#include "internal/otx/consensus/Consensus.hpp"
#include "internal/otx/consensus/Server.hpp"
#include "internal/util/Editor.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Shared.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "otx/server/Server.hpp"
#include "otx/server/Transactor.hpp"

#define CHEQUE_AMOUNT 144488
#define TRANSFER_AMOUNT 1144888
#define SECOND_TRANSFER_AMOUNT 500000
#define CHEQUE_MEMO "cheque memo"
#define TRANSFER_MEMO "transfer memo"
#define SUCCESS true
#define UNIT_DEFINITION_CONTRACT_NAME "Mt Gox USD"
#define UNIT_DEFINITION_TERMS "YOLO"
#define UNIT_DEFINITION_UNIT_OF_ACCOUNT ot::UnitType::Usd
#define UNIT_DEFINITION_CONTRACT_NAME_2 "Mt Gox BTC"
#define UNIT_DEFINITION_TERMS_2 "YOLO"
#define UNIT_DEFINITION_UNIT_OF_ACCOUNT_2 ot::UnitType::Btc
#define MESSAGE_TEXT "example message text"
#define NEW_SERVER_NAME "Awesome McCoolName"
#define TEST_SEED                                                              \
    "one two three four five six seven eight nine ten eleven twelve"
#define TEST_SEED_PASSPHRASE "seed passphrase"
#define CASH_AMOUNT 100
#define MINT_TIME_LIMIT_MINUTES 5

namespace ot = opentxs;

namespace ottest
{
using namespace std::literals::chrono_literals;

bool init_{false};

class Test_Basic : public ::testing::Test
{
public:
    struct matchID {
        matchID(const ot::UnallocatedCString& id)
            : id_{id}
        {
        }

        auto operator()(
            const std::pair<ot::UnallocatedCString, ot::UnallocatedCString>& id)
            -> bool
        {
            return id.first == id_;
        }

        const ot::UnallocatedCString id_;
    };

    static ot::RequestNumber alice_counter_;
    static ot::RequestNumber bob_counter_;
    static const ot::UnallocatedCString SeedA_;
    static const ot::UnallocatedCString SeedB_;
    static const ot::identifier::Nym alice_nym_id_;
    static const ot::identifier::Nym bob_nym_id_;
    static ot::TransactionNumber cheque_transaction_number_;
    static ot::UnallocatedCString bob_account_1_id_;
    static ot::UnallocatedCString bob_account_2_id_;
    static ot::UnallocatedCString outgoing_cheque_workflow_id_;
    static ot::UnallocatedCString incoming_cheque_workflow_id_;
    static ot::UnallocatedCString outgoing_transfer_workflow_id_;
    static ot::UnallocatedCString incoming_transfer_workflow_id_;
    static ot::UnallocatedCString internal_transfer_workflow_id_;
    static const ot::UnallocatedCString unit_id_1_;
    static const ot::UnallocatedCString unit_id_2_;
    static std::unique_ptr<ot::otx::client::internal::Operation>
        alice_state_machine_;
    static std::unique_ptr<ot::otx::client::internal::Operation>
        bob_state_machine_;
    static std::shared_ptr<ot::otx::blind::Purse> untrusted_purse_;

    const ot::api::session::Client& client_1_;
    const ot::api::session::Client& client_2_;
    const ot::api::session::Notary& server_1_;
    const ot::api::session::Notary& server_2_;
    ot::PasswordPrompt reason_c1_;
    ot::PasswordPrompt reason_c2_;
    ot::PasswordPrompt reason_s1_;
    ot::PasswordPrompt reason_s2_;
    const ot::OTUnitDefinition asset_contract_1_;
    const ot::OTUnitDefinition asset_contract_2_;
    const ot::identifier::Notary& server_1_id_;
    const ot::identifier::Notary& server_2_id_;
    const ot::OTServerContract server_contract_;
    const ot::otx::context::Server::ExtraArgs extra_args_;

    Test_Basic()
        : client_1_(dynamic_cast<const ot::api::session::Client&>(
              ot::Context().StartClientSession(0)))
        , client_2_(dynamic_cast<const ot::api::session::Client&>(
              ot::Context().StartClientSession(1)))
        , server_1_(dynamic_cast<const ot::api::session::Notary&>(
              ot::Context().StartNotarySession(0)))
        , server_2_(dynamic_cast<const ot::api::session::Notary&>(
              ot::Context().StartNotarySession(1)))
        , reason_c1_(client_1_.Factory().PasswordPrompt(__func__))
        , reason_c2_(client_2_.Factory().PasswordPrompt(__func__))
        , reason_s1_(server_1_.Factory().PasswordPrompt(__func__))
        , reason_s2_(server_2_.Factory().PasswordPrompt(__func__))
        , asset_contract_1_(load_unit(client_1_, unit_id_1_))
        , asset_contract_2_(load_unit(client_2_, unit_id_2_))
        , server_1_id_(
              dynamic_cast<const ot::identifier::Notary&>(server_1_.ID()))
        , server_2_id_(
              dynamic_cast<const ot::identifier::Notary&>(server_2_.ID()))
        , server_contract_(server_1_.Wallet().Internal().Server(server_1_id_))
        , extra_args_()
    {
        if (false == init_) { init(); }
    }

    static auto find_id(
        const ot::UnallocatedCString& id,
        const ot::ObjectList& list) -> bool
    {
        matchID matchid(id);

        return std::find_if(list.begin(), list.end(), matchid) != list.end();
    }

    static auto load_unit(
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

    static auto translate_result(const ot::otx::LastReplyStatus status)
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

    void break_consensus()
    {
        ot::TransactionNumber newNumber{0};
        server_1_.Server().GetTransactor().issueNextTransactionNumber(
            newNumber);

        auto context = server_1_.Wallet().Internal().mutable_ClientContext(
            alice_nym_id_, reason_s1_);
        context.get().IssueNumber(newNumber);
    }

    void import_server_contract(
        const ot::contract::Server& contract,
        const ot::api::session::Client& client)
    {
        auto bytes = ot::Space{};
        EXPECT_TRUE(server_contract_->Serialize(ot::writer(bytes), true));

        auto clientVersion =
            client.Wallet().Internal().Server(ot::reader(bytes));
        client.OTX().SetIntroductionServer(clientVersion);
    }

    void init()
    {
        client_1_.OTX().DisableAutoaccept();
        client_1_.InternalClient().Pair().Stop().get();
        client_2_.OTX().DisableAutoaccept();
        client_2_.InternalClient().Pair().Stop().get();
        const_cast<ot::UnallocatedCString&>(SeedA_) =
            client_1_.InternalClient().Exec().Wallet_ImportSeed(
                "spike nominee miss inquiry fee nothing belt list other "
                "daughter leave valley twelve gossip paper",
                "");
        const_cast<ot::UnallocatedCString&>(SeedB_) =
            client_2_.InternalClient().Exec().Wallet_ImportSeed(
                "trim thunder unveil reduce crop cradle zone inquiry "
                "anchor skate property fringe obey butter text tank drama "
                "palm guilt pudding laundry stay axis prosper",
                "");
        const_cast<ot::identifier::Nym&>(alice_nym_id_) =
            client_1_.Wallet().Nym({SeedA_, 0}, reason_c1_, "Alice")->ID();
        const_cast<ot::identifier::Nym&>(bob_nym_id_) =
            client_2_.Wallet().Nym({SeedB_, 0}, reason_c2_, "Bob")->ID();

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

    void create_unit_definition_1()
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

    void create_unit_definition_2()
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

    auto find_issuer_account() -> ot::identifier::Generic
    {
        const auto accounts =
            client_1_.Storage().AccountsByOwner(alice_nym_id_);

        OT_ASSERT(1 == accounts.size());

        return *accounts.begin();
    }

    auto find_unit_definition_id_1() -> ot::identifier::UnitDefinition
    {
        const auto accountID = find_issuer_account();

        OT_ASSERT(false == accountID.empty());

        const auto output = client_1_.Storage().AccountContract(accountID);

        OT_ASSERT(false == output.empty());

        return output;
    }

    auto find_unit_definition_id_2() -> ot::identifier::UnitDefinition
    {
        return client_1_.Factory().Internal().UnitIDConvertSafe(
            asset_contract_2_->ID());
    }

    auto find_user_account() -> ot::identifier::Generic
    {
        return client_2_.Factory().IdentifierFromBase58(bob_account_1_id_);
    }

    auto find_second_user_account() -> ot::identifier::Generic
    {
        return client_2_.Factory().IdentifierFromBase58(bob_account_2_id_);
    }

    void receive_reply(
        const std::shared_ptr<const ot::identity::Nym>& recipient,
        const std::shared_ptr<const ot::identity::Nym>& sender,
        const ot::OTPeerReply& peerreply,
        const ot::OTPeerRequest& peerrequest,
        ot::contract::peer::PeerRequestType requesttype)
    {
        const ot::RequestNumber sequence = alice_counter_;
        const ot::RequestNumber messages{4};
        alice_counter_ += messages;
        auto serverContext =
            client_1_.Wallet().Internal().mutable_ServerContext(
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

        const auto incomingID = peerreply->ID().asBase58(client_1_.Crypto());
        const auto foundincomingreply = find_id(incomingID, incomingreplies);

        ASSERT_TRUE(foundincomingreply);

        auto bytes = ot::Space{};
        EXPECT_TRUE(client_1_.Wallet().PeerReply(
            alice_nym_id_,
            peerreply->ID(),
            ot::otx::client::StorageBox::INCOMINGPEERREPLY,
            ot::writer(bytes)));
        auto incomingreply = client_1_.Factory().InternalSession().PeerReply(
            recipient, ot::reader(bytes));

        EXPECT_EQ(incomingreply->Type(), requesttype);
        EXPECT_EQ(incomingreply->Server(), server_1_id_);
        verify_reply(requesttype, peerreply, incomingreply);

        // Verify request is finished. 7
        const auto finishedrequests =
            client_1_.Wallet().PeerRequestFinished(alice_nym_id_);

        ASSERT_FALSE(finishedrequests.empty());

        const auto finishedrequestID =
            peerrequest->ID().asBase58(client_1_.Crypto());
        const auto foundfinishedrequest =
            find_id(finishedrequestID, finishedrequests);

        ASSERT_TRUE(foundfinishedrequest);

        std::time_t notused;
        EXPECT_TRUE(client_1_.Wallet().PeerRequest(
            alice_nym_id_,
            peerrequest->ID(),
            ot::otx::client::StorageBox::FINISHEDPEERREQUEST,
            notused,
            ot::writer(bytes)));
        auto finishedrequest =
            client_1_.Factory().InternalSession().PeerRequest(
                sender, ot::reader(bytes));

        EXPECT_EQ(finishedrequest->Initiator(), alice_nym_id_);
        EXPECT_EQ(finishedrequest->Recipient(), bob_nym_id_);
        EXPECT_EQ(finishedrequest->Type(), requesttype);
        verify_request(requesttype, peerrequest, finishedrequest);

        auto complete = client_1_.Wallet().PeerRequestComplete(
            alice_nym_id_, peerreply->ID());

        ASSERT_TRUE(complete);

        // Verify reply was processed. 8
        const auto processedreplies =
            client_1_.Wallet().PeerReplyProcessed(alice_nym_id_);

        ASSERT_FALSE(processedreplies.empty());

        const auto processedreplyID =
            peerreply->ID().asBase58(client_1_.Crypto());
        const auto foundprocessedreply =
            find_id(processedreplyID, processedreplies);

        ASSERT_TRUE(foundprocessedreply);

        EXPECT_TRUE(client_1_.Wallet().PeerReply(
            alice_nym_id_,
            peerreply->ID(),
            ot::otx::client::StorageBox::PROCESSEDPEERREPLY,
            ot::writer(bytes)));
        auto processedreply = client_1_.Factory().InternalSession().PeerReply(
            recipient, ot::reader(bytes));
        verify_reply(requesttype, peerreply, processedreply);
    }

    void receive_request(
        const std::shared_ptr<const ot::identity::Nym>& nym,
        const ot::OTPeerRequest& peerrequest,
        ot::contract::peer::PeerRequestType requesttype)
    {
        const ot::RequestNumber sequence = bob_counter_;
        const ot::RequestNumber messages{4};
        bob_counter_ += messages;
        auto serverContext =
            client_2_.Wallet().Internal().mutable_ServerContext(
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

        const auto incomingID = peerrequest->ID().asBase58(client_1_.Crypto());
        const auto found = find_id(incomingID, incomingrequests);

        ASSERT_TRUE(found);

        std::time_t notused;
        auto bytes = ot::Space{};
        EXPECT_TRUE(client_2_.Wallet().PeerRequest(
            bob_nym_id_,
            peerrequest->ID(),
            ot::otx::client::StorageBox::INCOMINGPEERREQUEST,
            notused,
            ot::writer(bytes)));
        auto incomingrequest =
            client_2_.Factory().InternalSession().PeerRequest(
                nym, ot::reader(bytes));

        EXPECT_EQ(incomingrequest->Initiator(), alice_nym_id_);
        EXPECT_EQ(incomingrequest->Recipient(), bob_nym_id_);
        EXPECT_EQ(incomingrequest->Type(), requesttype);
        EXPECT_EQ(incomingrequest->Server(), server_1_id_);
        verify_request(requesttype, peerrequest, incomingrequest);
    }

    void send_peer_reply(
        const std::shared_ptr<const ot::identity::Nym>& nym,
        const ot::OTPeerReply& peerreply,
        const ot::OTPeerRequest& peerrequest,
        ot::contract::peer::PeerRequestType requesttype)
    {
        const ot::RequestNumber sequence = bob_counter_;
        const ot::RequestNumber messages{1};
        bob_counter_ += messages;

        auto serverContext =
            client_2_.Wallet().Internal().mutable_ServerContext(
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

        const auto sentID = peerreply->ID().asBase58(client_1_.Crypto());
        const auto foundsentreply = find_id(sentID, sentreplies);

        ASSERT_TRUE(foundsentreply);

        auto bytes = ot::Space{};
        EXPECT_TRUE(client_2_.Wallet().PeerReply(
            bob_nym_id_,
            peerreply->ID(),
            ot::otx::client::StorageBox::SENTPEERREPLY,
            ot::writer(bytes)));
        auto sentreply = client_2_.Factory().InternalSession().PeerReply(
            nym, ot::reader(bytes));

        EXPECT_EQ(sentreply->Type(), requesttype);
        EXPECT_EQ(sentreply->Server(), server_1_id_);
        verify_reply(requesttype, peerreply, sentreply);

        // Verify request was processed. 4
        const auto processedrequests =
            client_2_.Wallet().PeerRequestProcessed(bob_nym_id_);

        ASSERT_FALSE(processedrequests.empty());

        const auto processedID = peerrequest->ID().asBase58(client_1_.Crypto());
        const auto foundrequest = find_id(processedID, processedrequests);

        ASSERT_TRUE(foundrequest);

        std::time_t notused;
        auto processedrequest = ot::Space{};
        auto loaded = client_2_.Wallet().PeerRequest(
            bob_nym_id_,
            peerrequest->ID(),
            ot::otx::client::StorageBox::PROCESSEDPEERREQUEST,
            notused,
            ot::writer(processedrequest));

        ASSERT_TRUE(loaded);

        auto complete =
            client_2_.Wallet().PeerReplyComplete(bob_nym_id_, peerreply->ID());

        ASSERT_TRUE(complete);

        // Verify reply is finished. 5
        const auto finishedreplies =
            client_2_.Wallet().PeerReplyFinished(bob_nym_id_);

        ASSERT_FALSE(finishedreplies.empty());

        const auto finishedID = peerreply->ID().asBase58(client_1_.Crypto());
        const auto foundfinishedreply = find_id(finishedID, finishedreplies);

        ASSERT_TRUE(foundfinishedreply);

        auto finishedreply = ot::Space{};
        loaded = client_2_.Wallet().PeerReply(
            bob_nym_id_,
            peerreply->ID(),
            ot::otx::client::StorageBox::FINISHEDPEERREPLY,
            ot::writer(finishedreply));

        ASSERT_TRUE(loaded);
    }

    void send_peer_request(
        const std::shared_ptr<const ot::identity::Nym>& nym,
        const ot::OTPeerRequest& peerrequest,
        ot::contract::peer::PeerRequestType requesttype)
    {
        const ot::RequestNumber sequence = alice_counter_;
        const ot::RequestNumber messages{1};
        alice_counter_ += messages;

        auto serverContext =
            client_1_.Wallet().Internal().mutable_ServerContext(
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
        const auto sentrequests =
            client_1_.Wallet().PeerRequestSent(alice_nym_id_);

        ASSERT_FALSE(sentrequests.empty());

        const auto sentID = peerrequest->ID().asBase58(client_1_.Crypto());
        const auto found = find_id(sentID, sentrequests);

        ASSERT_TRUE(found);

        std::time_t notused;
        auto bytes = ot::Space{};
        EXPECT_TRUE(client_1_.Wallet().PeerRequest(
            alice_nym_id_,
            peerrequest->ID(),
            ot::otx::client::StorageBox::SENTPEERREQUEST,
            notused,
            ot::writer(bytes)));
        auto sentrequest = client_1_.Factory().InternalSession().PeerRequest(
            nym, ot::reader(bytes));

        EXPECT_EQ(sentrequest->Initiator(), alice_nym_id_);
        EXPECT_EQ(sentrequest->Recipient(), bob_nym_id_);
        EXPECT_EQ(sentrequest->Type(), requesttype);
        EXPECT_EQ(sentrequest->Server(), server_1_id_);
        verify_request(requesttype, peerrequest, sentrequest);
    }

    void verify_account(
        const ot::identity::Nym& clientNym,
        const ot::identity::Nym& serverNym,
        const ot::Account& client,
        const ot::Account& server,
        const ot::PasswordPrompt& reasonC,
        const ot::PasswordPrompt& reasonS)
    {
        EXPECT_EQ(client.GetBalance(), server.GetBalance());
        EXPECT_EQ(
            client.GetInstrumentDefinitionID(),
            server.GetInstrumentDefinitionID());

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

    void verify_acknowledgement(
        const ot::contract::peer::Reply& originalreply,
        const ot::contract::peer::Reply& restoredreply)
    {
        const auto& acknowledgement = restoredreply.asAcknowledgement();
        verify_reply_properties(originalreply, acknowledgement);

        const auto& bailment = restoredreply.asBailment();
        EXPECT_EQ(bailment.Version(), 0);
        const auto& connection = restoredreply.asConnection();
        EXPECT_EQ(connection.Version(), 0);
        const auto& outbailment = restoredreply.asOutbailment();
        EXPECT_EQ(outbailment.Version(), 0);
    }

    void verify_bailment(
        const ot::contract::peer::Reply& originalreply,
        const ot::contract::peer::Reply& restoredreply)
    {
        const auto& bailment = restoredreply.asBailment();
        verify_reply_properties(originalreply, bailment);

        const auto& acknowledgement = restoredreply.asAcknowledgement();
        EXPECT_EQ(acknowledgement.Version(), 0);
        const auto& connection = restoredreply.asConnection();
        EXPECT_EQ(connection.Version(), 0);
        const auto& outbailment = restoredreply.asOutbailment();
        EXPECT_EQ(outbailment.Version(), 0);
    }

    void verify_bailment(
        const ot::contract::peer::Request& originalrequest,
        const ot::contract::peer::Request& restoredrequest)
    {
        const auto& original = originalrequest.asBailment();
        const auto& bailment = restoredrequest.asBailment();
        EXPECT_EQ(bailment.ServerID(), original.ServerID());
        EXPECT_EQ(bailment.UnitID(), original.UnitID());
        verify_request_properties(originalrequest, bailment);

        const auto& bailmentnotice = restoredrequest.asBailmentNotice();
        EXPECT_EQ(bailmentnotice.Version(), 0);
        const auto& connection = restoredrequest.asConnection();
        EXPECT_EQ(connection.Version(), 0);
        const auto& outbailment = restoredrequest.asOutbailment();
        EXPECT_EQ(outbailment.Version(), 0);
        const auto& storesecret = restoredrequest.asStoreSecret();
        EXPECT_EQ(storesecret.Version(), 0);
    }

    void verify_bailment_notice(
        const ot::contract::peer::Request& originalrequest,
        const ot::contract::peer::Request& restoredrequest)
    {
        const auto& bailmentnotice = restoredrequest.asBailmentNotice();
        verify_request_properties(originalrequest, bailmentnotice);

        const auto& bailment = restoredrequest.asBailment();
        EXPECT_EQ(bailment.Version(), 0);
        const auto& connection = restoredrequest.asConnection();
        EXPECT_EQ(connection.Version(), 0);
        const auto& outbailment = restoredrequest.asOutbailment();
        EXPECT_EQ(outbailment.Version(), 0);
        const auto& storesecret = restoredrequest.asStoreSecret();
        EXPECT_EQ(storesecret.Version(), 0);
    }

    void verify_connection(
        const ot::contract::peer::Reply& originalreply,
        const ot::contract::peer::Reply& restoredreply)
    {
        const auto& connection = restoredreply.asConnection();
        verify_reply_properties(originalreply, connection);

        const auto& acknowledgement = restoredreply.asAcknowledgement();
        EXPECT_EQ(acknowledgement.Version(), 0);
        const auto& bailment = restoredreply.asBailment();
        EXPECT_EQ(bailment.Version(), 0);
        const auto& outbailment = restoredreply.asOutbailment();
        EXPECT_EQ(outbailment.Version(), 0);
    }

    void verify_connection(
        const ot::contract::peer::Request& originalrequest,
        const ot::contract::peer::Request& restoredrequest)
    {
        const auto& connection = restoredrequest.asConnection();
        verify_request_properties(originalrequest, connection);

        const auto& bailment = restoredrequest.asBailment();
        EXPECT_EQ(bailment.Version(), 0);
        const auto& bailmentnotice = restoredrequest.asBailmentNotice();
        EXPECT_EQ(bailmentnotice.Version(), 0);
        const auto& outbailment = restoredrequest.asOutbailment();
        EXPECT_EQ(outbailment.Version(), 0);
        const auto& storesecret = restoredrequest.asStoreSecret();
        EXPECT_EQ(storesecret.Version(), 0);
    }

    void verify_outbailment(
        const ot::contract::peer::Reply& originalreply,
        const ot::contract::peer::Reply& restoredreply)
    {
        const auto& outbailment = restoredreply.asOutbailment();
        verify_reply_properties(originalreply, outbailment);

        const auto& acknowledgement = restoredreply.asAcknowledgement();
        EXPECT_EQ(acknowledgement.Version(), 0);
        const auto& bailment = restoredreply.asBailment();
        EXPECT_EQ(bailment.Version(), 0);
        const auto& connection = restoredreply.asConnection();
        EXPECT_EQ(connection.Version(), 0);
    }

    void verify_outbailment(
        const ot::contract::peer::Request& originalrequest,
        const ot::contract::peer::Request& restoredrequest)
    {
        const auto& outbailment = restoredrequest.asOutbailment();
        verify_request_properties(originalrequest, outbailment);

        const auto& bailment = restoredrequest.asBailment();
        EXPECT_EQ(bailment.Version(), 0);
        const auto& bailmentnotice = restoredrequest.asBailmentNotice();
        EXPECT_EQ(bailmentnotice.Version(), 0);
        const auto& connection = restoredrequest.asConnection();
        EXPECT_EQ(connection.Version(), 0);
        const auto& storesecret = restoredrequest.asStoreSecret();
        EXPECT_EQ(storesecret.Version(), 0);
    }

    void verify_reply(
        ot::contract::peer::PeerRequestType requesttype,
        const ot::contract::peer::Reply& original,
        const ot::contract::peer::Reply& restored)
    {
        switch (requesttype) {
            case ot::contract::peer::PeerRequestType::Bailment: {
                verify_bailment(original, restored);
                break;
            }
            case ot::contract::peer::PeerRequestType::PendingBailment: {
                verify_acknowledgement(original, restored);
                break;
            }
            case ot::contract::peer::PeerRequestType::ConnectionInfo: {
                verify_connection(original, restored);
                break;
            }
            case ot::contract::peer::PeerRequestType::OutBailment: {
                verify_outbailment(original, restored);
                break;
            }
            case ot::contract::peer::PeerRequestType::StoreSecret:
            case ot::contract::peer::PeerRequestType::VerificationOffer:
            case ot::contract::peer::PeerRequestType::Faucet:
            case ot::contract::peer::PeerRequestType::Error:
            default:
                break;
        }
    }

    void verify_request(
        ot::contract::peer::PeerRequestType requesttype,
        const ot::contract::peer::Request& original,
        const ot::contract::peer::Request& restored)
    {
        switch (requesttype) {
            case ot::contract::peer::PeerRequestType::Bailment: {
                verify_bailment(original, restored);
                break;
            }
            case ot::contract::peer::PeerRequestType::PendingBailment: {
                verify_bailment_notice(original, restored);
                break;
            }
            case ot::contract::peer::PeerRequestType::ConnectionInfo: {
                verify_connection(original, restored);
                break;
            }
            case ot::contract::peer::PeerRequestType::OutBailment: {
                verify_outbailment(original, restored);
                break;
            }
            case ot::contract::peer::PeerRequestType::StoreSecret: {
                verify_storesecret(original, restored);
                break;
            }
            case ot::contract::peer::PeerRequestType::VerificationOffer:
            case ot::contract::peer::PeerRequestType::Faucet:
            case ot::contract::peer::PeerRequestType::Error:
            default:
                break;
        }
    }

    template <typename T>
    void verify_reply_properties(
        const ot::contract::peer::Reply& original,
        const T& restored)
    {
        EXPECT_NE(restored.Version(), 0);

        EXPECT_EQ(restored.Alias(), original.Alias());
        EXPECT_EQ(restored.Name(), original.Name());
        EXPECT_EQ(restored.Serialize(), original.Serialize());
        EXPECT_EQ(restored.Type(), original.Type());
        EXPECT_EQ(restored.Version(), original.Version());
    }

    template <typename T>
    void verify_request_properties(
        const ot::contract::peer::Request& original,
        const T& restored)
    {
        EXPECT_NE(restored.Version(), 0);

        EXPECT_EQ(restored.Alias(), original.Alias());
        EXPECT_EQ(restored.Initiator(), original.Initiator());
        EXPECT_EQ(restored.Name(), original.Name());
        EXPECT_EQ(restored.Recipient(), original.Recipient());
        EXPECT_EQ(restored.Serialize(), original.Serialize());
        EXPECT_EQ(restored.Server(), original.Server());
        EXPECT_EQ(restored.Type(), original.Type());
        EXPECT_EQ(restored.Version(), original.Version());
    }

    void verify_storesecret(
        const ot::contract::peer::Request& originalrequest,
        const ot::contract::peer::Request& restoredrequest)
    {
        const auto& storesecret = restoredrequest.asStoreSecret();
        verify_request_properties(originalrequest, storesecret);
    }

    void verify_state_pre(
        const ot::otx::context::Client& clientContext,
        const ot::otx::context::Server& serverContext,
        const ot::RequestNumber initialRequestNumber)
    {
        EXPECT_EQ(serverContext.Request(), initialRequestNumber);
        EXPECT_EQ(clientContext.Request(), initialRequestNumber);
    }

    void verify_state_post(
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
                server_1_id_, serverContext.Nym()->ID())};

        ASSERT_TRUE(nymbox);
        EXPECT_TRUE(nymbox->VerifyAccount(*serverContext.Nym()));

        const auto& transactionMap = nymbox->GetTransactionMap();

        EXPECT_EQ(expectedNymboxItems, transactionMap.size());

        counter = serverContext.Request();

        EXPECT_FALSE(serverContext.StaleNym());
    }
};

ot::RequestNumber Test_Basic::alice_counter_{0};
ot::RequestNumber Test_Basic::bob_counter_{0};
const ot::UnallocatedCString Test_Basic::SeedA_{""};
const ot::UnallocatedCString Test_Basic::SeedB_{""};
const ot::identifier::Nym Test_Basic::alice_nym_id_{};
const ot::identifier::Nym Test_Basic::bob_nym_id_{};
ot::TransactionNumber Test_Basic::cheque_transaction_number_{0};
ot::UnallocatedCString Test_Basic::bob_account_1_id_{""};
ot::UnallocatedCString Test_Basic::bob_account_2_id_{""};
ot::UnallocatedCString Test_Basic::outgoing_cheque_workflow_id_{};
ot::UnallocatedCString Test_Basic::incoming_cheque_workflow_id_{};
ot::UnallocatedCString Test_Basic::outgoing_transfer_workflow_id_{};
ot::UnallocatedCString Test_Basic::incoming_transfer_workflow_id_{};
ot::UnallocatedCString Test_Basic::internal_transfer_workflow_id_{};
const ot::UnallocatedCString Test_Basic::unit_id_1_{};
const ot::UnallocatedCString Test_Basic::unit_id_2_{};
std::unique_ptr<ot::otx::client::internal::Operation>
    Test_Basic::alice_state_machine_{nullptr};
std::unique_ptr<ot::otx::client::internal::Operation>
    Test_Basic::bob_state_machine_{nullptr};
std::shared_ptr<ot::otx::blind::Purse> Test_Basic::untrusted_purse_{};

TEST_F(Test_Basic, zmq_disconnected)
{
    EXPECT_EQ(
        ot::network::ConnectionState::NOT_ESTABLISHED,
        client_1_.ZMQ().Status(server_1_id_.asBase58(client_1_.Crypto())));
}

TEST_F(Test_Basic, getRequestNumber_nclient_1_registered)
{
    auto serverContext = client_1_.Wallet().Internal().mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto clientContext =
        server_1_.Wallet().Internal().ClientContext(alice_nym_id_);

    EXPECT_EQ(serverContext.get().Request(), 0);
    EXPECT_FALSE(clientContext);

    const auto number = serverContext.get().UpdateRequestNumber(reason_c1_);

    EXPECT_EQ(number, 0);
    EXPECT_EQ(serverContext.get().Request(), 0);

    clientContext = server_1_.Wallet().Internal().ClientContext(alice_nym_id_);

    EXPECT_FALSE(clientContext);
}

TEST_F(Test_Basic, zmq_connected)
{
    EXPECT_EQ(
        ot::network::ConnectionState::ACTIVE,
        client_1_.ZMQ().Status(server_1_id_.asBase58(client_1_.Crypto())));
}

TEST_F(Test_Basic, registerNym_first_time)
{
    const ot::RequestNumber sequence = alice_counter_;
    const ot::RequestNumber messages{2};
    alice_counter_ += messages;
    auto serverContext = client_1_.Wallet().Internal().mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto& context = serverContext.get();
    auto clientContext =
        server_1_.Wallet().Internal().ClientContext(alice_nym_id_);

    EXPECT_EQ(serverContext.get().Request(), sequence);
    EXPECT_FALSE(clientContext);

    ot::otx::context::Server::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;
    auto started =
        stateMachine.Start(ot::otx::OperationType::RegisterNym, extra_args_);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const ot::RequestNumber requestNumber =
        ot::String::StringToUlong(message->request_num_->Get());
    const auto result = translate_result(std::get<0>(finished));
    clientContext = server_1_.Wallet().Internal().ClientContext(alice_nym_id_);
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
}

TEST_F(Test_Basic, getTransactionNumbers_Alice)
{
    const ot::RequestNumber sequence = alice_counter_;
    const ot::RequestNumber messages{5};
    alice_counter_ += messages;
    auto serverContext = client_1_.Wallet().Internal().mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto& context = serverContext.get();
    auto clientContext =
        server_1_.Wallet().Internal().ClientContext(alice_nym_id_);

    ASSERT_TRUE(clientContext);
    EXPECT_EQ(context.IssuedNumbers().size(), 0);
    EXPECT_EQ(clientContext->IssuedNumbers().size(), 0);

    verify_state_pre(*clientContext, context, sequence);
    ot::otx::context::Server::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;
    auto started = stateMachine.Start(
        ot::otx::OperationType::GetTransactionNumbers, extra_args_);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
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
    const ot::RequestNumber sequence = alice_counter_;
    const ot::RequestNumber messages{5};
    alice_counter_ += messages;
    auto serverContext = client_1_.Wallet().Internal().mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto& context = serverContext.get();
    auto clientContext =
        server_1_.Wallet().Internal().ClientContext(alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.get(), sequence);
    ot::otx::context::Server::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;
    auto started =
        stateMachine.Start(ot::otx::OperationType::RegisterNym, extra_args_);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const ot::RequestNumber requestNumber =
        ot::String::StringToUlong(message->request_num_->Get());
    const auto result = translate_result(std::get<0>(finished));
    clientContext = server_1_.Wallet().Internal().ClientContext(alice_nym_id_);
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
}

TEST_F(Test_Basic, issueAsset)
{
    const ot::RequestNumber sequence = alice_counter_;
    const ot::RequestNumber messages{3};
    alice_counter_ += messages;
    auto serverContext = client_1_.Wallet().Internal().mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto& context = serverContext.get();
    auto clientContext =
        server_1_.Wallet().Internal().ClientContext(alice_nym_id_);

    ASSERT_TRUE(clientContext);

    create_unit_definition_1();
    verify_state_pre(*clientContext, context, sequence);
    ot::otx::context::Server::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;

    auto bytes = ot::Space{};
    EXPECT_TRUE(asset_contract_1_->Serialize(ot::writer(bytes), true));
    auto started =
        stateMachine.IssueUnitDefinition(ot::reader(bytes), extra_args_);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
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
        context,
        sequence,
        alice_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        alice_counter_);
    const auto accountID =
        client_1_.Factory().IdentifierFromBase58(message->acct_id_->Bytes());

    ASSERT_FALSE(accountID.empty());

    const auto clientAccount = client_1_.Wallet().Internal().Account(accountID);
    const auto serverAccount = server_1_.Wallet().Internal().Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);

    verify_account(
        *serverContext.get().Nym(),
        *clientContext->Nym(),
        clientAccount,
        serverAccount,
        reason_c1_,
        reason_s1_);
}

TEST_F(Test_Basic, checkNym_missing)
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

    ot::otx::context::Server::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;
    auto started = stateMachine.Start(
        ot::otx::OperationType::CheckNym, bob_nym_id_, extra_args_);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
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

    EXPECT_FALSE(message->bool_);
    EXPECT_TRUE(message->payload_->empty());
}

TEST_F(Test_Basic, publishNym)
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

    ot::otx::context::Server::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;
    auto nym = client_2_.Wallet().Nym(bob_nym_id_);

    ASSERT_TRUE(nym);

    auto bytes = ot::Space{};
    EXPECT_TRUE(nym->Serialize(ot::writer(bytes)));
    client_1_.Wallet().Nym(ot::reader(bytes));
    auto started = stateMachine.PublishContract(bob_nym_id_);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
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
}

TEST_F(Test_Basic, checkNym)
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

    ot::otx::context::Server::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;
    auto started = stateMachine.Start(
        ot::otx::OperationType::CheckNym, bob_nym_id_, extra_args_);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
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

    EXPECT_TRUE(message->bool_);
    EXPECT_FALSE(message->payload_->empty());
}

TEST_F(Test_Basic, downloadServerContract_missing)
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

    ot::otx::context::Server::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;
    auto started =
        stateMachine.DownloadContract(server_2_id_, ot::contract::Type::notary);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
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

    EXPECT_FALSE(message->bool_);
    EXPECT_TRUE(message->payload_->empty());
}

TEST_F(Test_Basic, publishServer)
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

    ot::otx::context::Server::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;
    auto server = server_2_.Wallet().Internal().Server(server_2_id_);
    auto bytes = ot::Space{};
    EXPECT_TRUE(server->Serialize(ot::writer(bytes), true));
    client_1_.Wallet().Internal().Server(ot::reader(bytes));
    auto started = stateMachine.PublishContract(server_2_id_);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
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
}

TEST_F(Test_Basic, downloadServerContract)
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

    ot::otx::context::Server::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;
    auto started =
        stateMachine.DownloadContract(server_2_id_, ot::contract::Type::notary);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
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

    EXPECT_TRUE(message->bool_);
    EXPECT_FALSE(message->payload_->empty());
}

TEST_F(Test_Basic, registerNym_Bob)
{
    const ot::RequestNumber sequence = bob_counter_;
    const ot::RequestNumber messages{2};
    bob_counter_ += messages;
    auto serverContext = client_2_.Wallet().Internal().mutable_ServerContext(
        bob_nym_id_, server_1_id_, reason_c2_);
    auto& context = serverContext.get();
    auto clientContext =
        server_1_.Wallet().Internal().ClientContext(bob_nym_id_);

    EXPECT_EQ(serverContext.get().Request(), sequence);
    EXPECT_FALSE(clientContext);

    ot::otx::context::Server::DeliveryResult finished{};
    auto& stateMachine = *bob_state_machine_;
    auto started =
        stateMachine.Start(ot::otx::OperationType::RegisterNym, extra_args_);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const ot::RequestNumber requestNumber =
        ot::String::StringToUlong(message->request_num_->Get());
    const auto result = translate_result(std::get<0>(finished));
    clientContext = server_1_.Wallet().Internal().ClientContext(bob_nym_id_);
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
}

TEST_F(Test_Basic, getInstrumentDefinition_missing)
{
    const auto sequence = ot::RequestNumber{bob_counter_};
    const auto messages = ot::RequestNumber{1};
    bob_counter_ += messages;
    auto serverContext = client_2_.Wallet().Internal().mutable_ServerContext(
        bob_nym_id_, server_1_id_, reason_c2_);
    auto& context = serverContext.get();
    auto clientContext =
        server_1_.Wallet().Internal().ClientContext(bob_nym_id_);

    ASSERT_TRUE(clientContext);

    create_unit_definition_2();
    verify_state_pre(*clientContext, context, sequence);
    ot::otx::context::Server::DeliveryResult finished{};
    auto& stateMachine = *bob_state_machine_;
    auto started = stateMachine.DownloadContract(
        find_unit_definition_id_2(), ot::contract::Type::unit);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
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
        context,
        sequence,
        bob_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        bob_counter_);

    EXPECT_FALSE(message->bool_);
    EXPECT_TRUE(message->payload_->empty());
}

TEST_F(Test_Basic, publishUnitDefinition)
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

    ot::otx::context::Server::DeliveryResult finished{};
    auto& stateMachine = *bob_state_machine_;
    auto started = stateMachine.PublishContract(find_unit_definition_id_2());

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
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
}

TEST_F(Test_Basic, getInstrumentDefinition_Alice)
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

    verify_state_pre(*clientContext, context, sequence);
    ot::otx::context::Server::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;
    auto started = stateMachine.DownloadContract(
        find_unit_definition_id_2(), ot::contract::Type::unit);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
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
        context,
        sequence,
        alice_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        alice_counter_);

    EXPECT_TRUE(message->bool_);
    EXPECT_FALSE(message->payload_->empty());
}

TEST_F(Test_Basic, getInstrumentDefinition_Bob)
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

    verify_state_pre(*clientContext, context, sequence);
    ot::otx::context::Server::DeliveryResult finished{};
    auto& stateMachine = *bob_state_machine_;
    auto started = stateMachine.DownloadContract(
        find_unit_definition_id_1(), ot::contract::Type::unit);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
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
        context,
        sequence,
        bob_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        bob_counter_);

    EXPECT_TRUE(message->bool_);
    EXPECT_FALSE(message->payload_->empty());
}

TEST_F(Test_Basic, registerAccount)
{
    const ot::RequestNumber sequence = bob_counter_;
    const ot::RequestNumber messages{3};
    bob_counter_ += messages;
    auto serverContext = client_2_.Wallet().Internal().mutable_ServerContext(
        bob_nym_id_, server_1_id_, reason_c2_);
    auto& context = serverContext.get();
    auto clientContext =
        server_1_.Wallet().Internal().ClientContext(bob_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, context, sequence);
    ot::otx::context::Server::DeliveryResult finished{};
    auto& stateMachine = *bob_state_machine_;
    auto started = stateMachine.Start(
        ot::otx::OperationType::RegisterAccount,
        find_unit_definition_id_1(),
        extra_args_);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
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
        context,
        sequence,
        bob_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        bob_counter_);

    const auto accountID =
        client_1_.Factory().IdentifierFromBase58(message->acct_id_->Bytes());
    const auto clientAccount = client_2_.Wallet().Internal().Account(accountID);
    const auto serverAccount = server_1_.Wallet().Internal().Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);

    verify_account(
        *serverContext.get().Nym(),
        *clientContext->Nym(),
        clientAccount,
        serverAccount,
        reason_c2_,
        reason_s1_);

    bob_account_1_id_ = accountID.asBase58(client_1_.Crypto());
}

TEST_F(Test_Basic, send_cheque)
{
    const ot::RequestNumber sequence = alice_counter_;
    const ot::RequestNumber messages{1};
    alice_counter_ += messages;
    std::unique_ptr<ot::Cheque> cheque{
        client_1_.InternalClient().OTAPI().WriteCheque(
            server_1_id_,
            CHEQUE_AMOUNT,
            {},
            {},
            find_issuer_account(),
            alice_nym_id_,
            ot::String::Factory(CHEQUE_MEMO),
            bob_nym_id_)};

    ASSERT_TRUE(cheque);

    cheque_transaction_number_ = cheque->GetTransactionNum();

    EXPECT_NE(0, cheque_transaction_number_);

    std::shared_ptr<ot::OTPayment> payment{
        client_1_.Factory().InternalSession().Payment(
            ot::String::Factory(*cheque))};

    ASSERT_TRUE(payment);

    auto serverContext = client_1_.Wallet().Internal().mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto& context = serverContext.get();
    auto clientContext =
        server_1_.Wallet().Internal().ClientContext(alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.get(), sequence);
    ot::otx::context::Server::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;
    auto started = stateMachine.ConveyPayment(bob_nym_id_, payment);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
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
    const auto workflowList =
        client_1_.Storage().PaymentWorkflowList(alice_nym_id_).size();

    EXPECT_EQ(1, workflowList);

    const auto workflows = client_1_.Storage().PaymentWorkflowsByState(
        alice_nym_id_,
        ot::otx::client::PaymentWorkflowType::OutgoingCheque,
        ot::otx::client::PaymentWorkflowState::Conveyed);

    ASSERT_EQ(1, workflows.size());

    outgoing_cheque_workflow_id_ = *workflows.begin();
}

TEST_F(Test_Basic, getNymbox_receive_cheque)
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
    const auto workflowList =
        client_2_.Storage().PaymentWorkflowList(bob_nym_id_);

    EXPECT_EQ(1, workflowList.size());

    const auto workflows = client_2_.Storage().PaymentWorkflowsByState(
        bob_nym_id_,
        ot::otx::client::PaymentWorkflowType::IncomingCheque,
        ot::otx::client::PaymentWorkflowState::Conveyed);

    EXPECT_EQ(1, workflows.size());

    incoming_cheque_workflow_id_ = *workflows.begin();
}

TEST_F(Test_Basic, getNymbox_after_clearing_nymbox_2_Bob)
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
}

TEST_F(Test_Basic, depositCheque)
{
    const auto accountID = find_user_account();
    const auto workflowID =
        client_2_.Factory().IdentifierFromBase58(incoming_cheque_workflow_id_);
    auto [state, pCheque] =
        client_2_.Workflow().InstantiateCheque(bob_nym_id_, workflowID);

    ASSERT_EQ(ot::otx::client::PaymentWorkflowState::Conveyed, state);
    ASSERT_TRUE(pCheque);

    std::shared_ptr<ot::Cheque> cheque{std::move(pCheque)};
    const ot::RequestNumber sequence = bob_counter_;
    const ot::RequestNumber messages{12};
    bob_counter_ += messages;
    auto serverContext = client_2_.Wallet().Internal().mutable_ServerContext(
        bob_nym_id_, server_1_id_, reason_c2_);
    auto& context = serverContext.get();
    auto clientContext =
        server_1_.Wallet().Internal().ClientContext(bob_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.get(), sequence);
    ot::otx::context::Server::DeliveryResult finished{};
    auto& stateMachine = *bob_state_machine_;
    auto started = stateMachine.DepositCheque(accountID, cheque);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
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
        sequence + 6,
        bob_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        bob_counter_);
    const auto clientAccount = client_2_.Wallet().Internal().Account(accountID);
    const auto serverAccount = server_1_.Wallet().Internal().Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);
    EXPECT_EQ(CHEQUE_AMOUNT, serverAccount.get().GetBalance());
    EXPECT_EQ(CHEQUE_AMOUNT, clientAccount.get().GetBalance());

    const auto workflowList =
        client_2_.Storage().PaymentWorkflowList(bob_nym_id_).size();

    EXPECT_EQ(1, workflowList);

    const auto [wType, wState] = client_2_.Storage().PaymentWorkflowState(
        bob_nym_id_, incoming_cheque_workflow_id_);

    EXPECT_EQ(ot::otx::client::PaymentWorkflowType::IncomingCheque, wType);
    EXPECT_EQ(ot::otx::client::PaymentWorkflowState::Completed, wState);
}

TEST_F(Test_Basic, getAccountData_after_cheque_deposited)
{
    const ot::RequestNumber sequence = alice_counter_;
    const ot::RequestNumber messages{5};
    alice_counter_ += messages;
    auto serverContext = client_1_.Wallet().Internal().mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto& context = serverContext.get();
    auto clientContext =
        server_1_.Wallet().Internal().ClientContext(alice_nym_id_);

    ASSERT_TRUE(clientContext);

    const auto accountID = find_issuer_account();

    ASSERT_FALSE(accountID.empty());

    verify_state_pre(*clientContext, serverContext.get(), sequence);
    ot::otx::context::Server::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;
    auto started = stateMachine.UpdateAccount(accountID);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
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
        sequence + 2,
        alice_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        alice_counter_);
    const auto clientAccount = client_1_.Wallet().Internal().Account(accountID);
    const auto serverAccount = server_1_.Wallet().Internal().Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);

    verify_account(
        *serverContext.get().Nym(),
        *clientContext->Nym(),
        clientAccount,
        serverAccount,
        reason_c1_,
        reason_s1_);
    const auto workflowList =
        client_1_.Storage().PaymentWorkflowList(alice_nym_id_).size();

    EXPECT_EQ(1, workflowList);

    const auto [wType, wState] = client_1_.Storage().PaymentWorkflowState(
        alice_nym_id_, outgoing_cheque_workflow_id_);

    EXPECT_EQ(wType, ot::otx::client::PaymentWorkflowType::OutgoingCheque);
    // TODO should be completed?
    EXPECT_EQ(wState, ot::otx::client::PaymentWorkflowState::Accepted);
}

TEST_F(Test_Basic, resync)
{
    break_consensus();
    const ot::RequestNumber sequence = alice_counter_;
    const ot::RequestNumber messages{2};
    alice_counter_ += messages;
    auto serverContext = client_1_.Wallet().Internal().mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto& context = serverContext.get();
    auto clientContext =
        server_1_.Wallet().Internal().ClientContext(alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.get(), sequence);
    ot::otx::context::Server::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;
    auto started =
        stateMachine.Start(ot::otx::OperationType::RegisterNym, {"", true});

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
    stateMachine.join();
    context.Join();
    context.ResetThread();
    const auto& message = std::get<1>(finished);

    ASSERT_TRUE(message);

    const ot::RequestNumber requestNumber =
        ot::String::StringToUlong(message->request_num_->Get());
    const auto result = translate_result(std::get<0>(finished));
    clientContext = server_1_.Wallet().Internal().ClientContext(alice_nym_id_);
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
}

TEST_F(Test_Basic, sendTransfer)
{
    const ot::RequestNumber sequence = alice_counter_;
    const ot::RequestNumber messages{5};
    alice_counter_ += messages;
    auto serverContext = client_1_.Wallet().Internal().mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto& context = serverContext.get();
    auto clientContext =
        server_1_.Wallet().Internal().ClientContext(alice_nym_id_);

    ASSERT_TRUE(clientContext);

    const auto senderAccountID = find_issuer_account();

    ASSERT_FALSE(senderAccountID.empty());

    const auto recipientAccountID = find_user_account();

    ASSERT_FALSE(recipientAccountID.empty());

    verify_state_pre(*clientContext, context, sequence);
    ot::otx::context::Server::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;
    auto started = stateMachine.SendTransfer(
        senderAccountID,
        recipientAccountID,
        TRANSFER_AMOUNT,
        ot::String::Factory(TRANSFER_MEMO));

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
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
        context,
        sequence + 1,
        alice_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        alice_counter_);
    const auto serverAccount =
        server_1_.Wallet().Internal().Account(senderAccountID);

    ASSERT_TRUE(serverAccount);

    // A successful sent transfer has an immediate effect on the
    // sender's account balance.
    EXPECT_EQ(
        (-1 * (CHEQUE_AMOUNT + TRANSFER_AMOUNT)),
        serverAccount.get().GetBalance());

    const auto workflowList =
        client_1_.Storage().PaymentWorkflowList(alice_nym_id_).size();

    EXPECT_EQ(2, workflowList);

    const auto workflows = client_1_.Storage().PaymentWorkflowsByState(
        alice_nym_id_,
        ot::otx::client::PaymentWorkflowType::OutgoingTransfer,
        ot::otx::client::PaymentWorkflowState::Acknowledged);

    ASSERT_EQ(1, workflows.size());

    outgoing_transfer_workflow_id_ = *workflows.cbegin();

    EXPECT_NE(outgoing_transfer_workflow_id_.size(), 0);

    auto partysize = int{-1};
    EXPECT_TRUE(client_1_.Workflow().WorkflowPartySize(
        alice_nym_id_,
        client_1_.Factory().IdentifierFromBase58(
            outgoing_transfer_workflow_id_),
        partysize));
    EXPECT_EQ(partysize, 0);
}

TEST_F(Test_Basic, getAccountData_after_incomingTransfer)
{
    const ot::RequestNumber sequence = bob_counter_;
    const ot::RequestNumber messages{5};
    bob_counter_ += messages;
    auto serverContext = client_2_.Wallet().Internal().mutable_ServerContext(
        bob_nym_id_, server_1_id_, reason_c2_);
    auto& context = serverContext.get();
    auto clientContext =
        server_1_.Wallet().Internal().ClientContext(bob_nym_id_);

    ASSERT_TRUE(clientContext);

    const auto accountID = find_user_account();

    ASSERT_FALSE(accountID.empty());

    verify_state_pre(*clientContext, serverContext.get(), sequence);
    ot::otx::context::Server::DeliveryResult finished{};
    auto& stateMachine = *bob_state_machine_;
    auto started = stateMachine.UpdateAccount(accountID);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
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
        sequence + 2,
        bob_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        bob_counter_);
    const auto clientAccount = client_2_.Wallet().Internal().Account(accountID);
    const auto serverAccount = server_1_.Wallet().Internal().Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);

    verify_account(
        *serverContext.get().Nym(),
        *clientContext->Nym(),
        clientAccount,
        serverAccount,
        reason_c2_,
        reason_s1_);

    EXPECT_EQ(
        (CHEQUE_AMOUNT + TRANSFER_AMOUNT), serverAccount.get().GetBalance());

    const auto workflowList =
        client_2_.Storage().PaymentWorkflowList(bob_nym_id_).size();

    EXPECT_EQ(2, workflowList);

    const auto workflows = client_2_.Storage().PaymentWorkflowsByState(
        bob_nym_id_,
        ot::otx::client::PaymentWorkflowType::IncomingTransfer,
        ot::otx::client::PaymentWorkflowState::Completed);

    ASSERT_EQ(1, workflows.size());

    incoming_transfer_workflow_id_ = *workflows.cbegin();

    EXPECT_NE(incoming_transfer_workflow_id_.size(), 0);

    auto partysize = int{-1};
    EXPECT_TRUE(client_2_.Workflow().WorkflowPartySize(
        bob_nym_id_,
        client_2_.Factory().IdentifierFromBase58(
            incoming_transfer_workflow_id_),
        partysize));
    EXPECT_EQ(1, partysize);

    EXPECT_STREQ(
        alice_nym_id_.asBase58(client_1_.Crypto()).c_str(),
        client_2_.Workflow()
            .WorkflowParty(
                bob_nym_id_,
                client_2_.Factory().IdentifierFromBase58(
                    incoming_transfer_workflow_id_),
                0)
            .c_str());

    EXPECT_EQ(
        client_2_.Workflow().WorkflowType(
            bob_nym_id_,
            client_2_.Factory().IdentifierFromBase58(
                incoming_transfer_workflow_id_)),
        ot::otx::client::PaymentWorkflowType::IncomingTransfer);
    EXPECT_EQ(
        client_2_.Workflow().WorkflowState(
            bob_nym_id_,
            client_2_.Factory().IdentifierFromBase58(
                incoming_transfer_workflow_id_)),
        ot::otx::client::PaymentWorkflowState::Completed);
}

TEST_F(Test_Basic, getAccountData_after_transfer_accepted)
{
    const ot::RequestNumber sequence = alice_counter_;
    const ot::RequestNumber messages{5};
    alice_counter_ += messages;
    auto serverContext = client_1_.Wallet().Internal().mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto& context = serverContext.get();
    auto clientContext =
        server_1_.Wallet().Internal().ClientContext(alice_nym_id_);

    ASSERT_TRUE(clientContext);

    const auto accountID = find_issuer_account();

    ASSERT_FALSE(accountID.empty());

    verify_state_pre(*clientContext, serverContext.get(), sequence);
    ot::otx::context::Server::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;
    auto started = stateMachine.UpdateAccount(accountID);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
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
        sequence + 2,
        alice_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        alice_counter_);
    const auto clientAccount = client_1_.Wallet().Internal().Account(accountID);
    const auto serverAccount = server_1_.Wallet().Internal().Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);

    verify_account(
        *serverContext.get().Nym(),
        *clientContext->Nym(),
        clientAccount,
        serverAccount,
        reason_c1_,
        reason_s1_);

    EXPECT_EQ(
        -1 * (CHEQUE_AMOUNT + TRANSFER_AMOUNT),
        serverAccount.get().GetBalance());

    const auto workflowList =
        client_2_.Storage().PaymentWorkflowList(bob_nym_id_).size();

    EXPECT_EQ(2, workflowList);

    const auto [type, state] = client_1_.Storage().PaymentWorkflowState(
        alice_nym_id_, outgoing_transfer_workflow_id_);

    EXPECT_EQ(type, ot::otx::client::PaymentWorkflowType::OutgoingTransfer);
    EXPECT_EQ(state, ot::otx::client::PaymentWorkflowState::Completed);
}

TEST_F(Test_Basic, register_second_account)
{
    const ot::RequestNumber sequence = bob_counter_;
    const ot::RequestNumber messages{3};
    bob_counter_ += messages;
    auto serverContext = client_2_.Wallet().Internal().mutable_ServerContext(
        bob_nym_id_, server_1_id_, reason_c2_);
    auto& context = serverContext.get();
    auto clientContext =
        server_1_.Wallet().Internal().ClientContext(bob_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, context, sequence);
    ot::otx::context::Server::DeliveryResult finished{};
    auto& stateMachine = *bob_state_machine_;
    auto started = stateMachine.Start(
        ot::otx::OperationType::RegisterAccount,
        find_unit_definition_id_1(),
        extra_args_);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
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
        context,
        sequence,
        bob_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        bob_counter_);
    const auto accountID =
        client_2_.Factory().IdentifierFromBase58(message->acct_id_->Bytes());
    const auto clientAccount = client_2_.Wallet().Internal().Account(accountID);
    const auto serverAccount = server_1_.Wallet().Internal().Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);

    verify_account(
        *serverContext.get().Nym(),
        *clientContext->Nym(),
        clientAccount,
        serverAccount,
        reason_c2_,
        reason_s1_);

    bob_account_2_id_ = accountID.asBase58(client_1_.Crypto());
}

TEST_F(Test_Basic, send_internal_transfer)
{
    const ot::RequestNumber sequence = bob_counter_;
    const ot::RequestNumber messages{5};
    bob_counter_ += messages;
    auto serverContext = client_2_.Wallet().Internal().mutable_ServerContext(
        bob_nym_id_, server_1_id_, reason_c2_);
    auto& context = serverContext.get();
    auto clientContext =
        server_1_.Wallet().Internal().ClientContext(bob_nym_id_);

    ASSERT_TRUE(clientContext);

    const auto senderAccountID = find_user_account();

    ASSERT_FALSE(senderAccountID.empty());

    const auto recipientAccountID = find_second_user_account();

    ASSERT_FALSE(recipientAccountID.empty());

    verify_state_pre(*clientContext, serverContext.get(), sequence);
    ot::otx::context::Server::DeliveryResult finished{};
    auto& stateMachine = *bob_state_machine_;
    auto started = stateMachine.SendTransfer(
        senderAccountID,
        recipientAccountID,
        SECOND_TRANSFER_AMOUNT,
        ot::String::Factory(TRANSFER_MEMO));

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
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
        sequence + 1,
        bob_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        bob_counter_);
    const auto serverAccount =
        server_1_.Wallet().Internal().Account(senderAccountID);

    ASSERT_TRUE(serverAccount);

    // A successful sent transfer has an immediate effect on the sender's
    // account balance.
    EXPECT_EQ(
        (CHEQUE_AMOUNT + TRANSFER_AMOUNT - SECOND_TRANSFER_AMOUNT),
        serverAccount.get().GetBalance());

    std::size_t count{0}, tries{100};
    ot::UnallocatedSet<ot::UnallocatedCString> workflows{};
    while (0 == count) {
        // The state change from ACKNOWLEDGED to CONVEYED occurs
        // asynchronously due to server push notifications so the order in
        // which these states are observed by the sender is undefined.
        ot::Sleep(100ms);
        workflows = client_2_.Storage().PaymentWorkflowsByState(
            bob_nym_id_,
            ot::otx::client::PaymentWorkflowType::InternalTransfer,
            ot::otx::client::PaymentWorkflowState::Acknowledged);
        count = workflows.size();

        if (0 == count) {
            workflows = client_2_.Storage().PaymentWorkflowsByState(
                bob_nym_id_,
                ot::otx::client::PaymentWorkflowType::InternalTransfer,
                ot::otx::client::PaymentWorkflowState::Conveyed);
            count = workflows.size();
        }

        if (0 == --tries) { break; }
    }

    ASSERT_EQ(count, 1);

    internal_transfer_workflow_id_ = *workflows.cbegin();

    EXPECT_NE(internal_transfer_workflow_id_.size(), 0);

    const auto workflowList =
        client_2_.Storage().PaymentWorkflowList(bob_nym_id_).size();

    EXPECT_EQ(3, workflowList);
}

TEST_F(Test_Basic, getAccountData_after_incoming_internal_Transfer)
{
    const ot::RequestNumber sequence = bob_counter_;
    const ot::RequestNumber messages{5};
    bob_counter_ += messages;
    auto serverContext = client_2_.Wallet().Internal().mutable_ServerContext(
        bob_nym_id_, server_1_id_, reason_c2_);
    auto& context = serverContext.get();
    auto clientContext =
        server_1_.Wallet().Internal().ClientContext(bob_nym_id_);

    ASSERT_TRUE(clientContext);

    const auto accountID = find_second_user_account();

    ASSERT_FALSE(accountID.empty());

    verify_state_pre(*clientContext, serverContext.get(), sequence);
    ot::otx::context::Server::DeliveryResult finished{};
    auto& stateMachine = *bob_state_machine_;
    auto started = stateMachine.UpdateAccount(accountID);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
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
        sequence + 2,
        bob_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        bob_counter_);
    const auto clientAccount = client_2_.Wallet().Internal().Account(accountID);
    const auto serverAccount = server_1_.Wallet().Internal().Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);

    verify_account(
        *serverContext.get().Nym(),
        *clientContext->Nym(),
        clientAccount,
        serverAccount,
        reason_c2_,
        reason_s1_);
    const auto workflowList =
        client_2_.Storage().PaymentWorkflowList(bob_nym_id_).size();

    EXPECT_EQ(3, workflowList);

    const auto [type, state] = client_2_.Storage().PaymentWorkflowState(
        bob_nym_id_, internal_transfer_workflow_id_);

    EXPECT_EQ(type, ot::otx::client::PaymentWorkflowType::InternalTransfer);
    EXPECT_EQ(state, ot::otx::client::PaymentWorkflowState::Conveyed);
    EXPECT_EQ(SECOND_TRANSFER_AMOUNT, serverAccount.get().GetBalance());
}

TEST_F(Test_Basic, getAccountData_after_internal_transfer_accepted)
{
    const ot::RequestNumber sequence = bob_counter_;
    const ot::RequestNumber messages{5};
    bob_counter_ += messages;
    auto serverContext = client_2_.Wallet().Internal().mutable_ServerContext(
        bob_nym_id_, server_1_id_, reason_c2_);
    auto& context = serverContext.get();
    auto clientContext =
        server_1_.Wallet().Internal().ClientContext(bob_nym_id_);

    ASSERT_TRUE(clientContext);

    const auto accountID = find_user_account();

    ASSERT_FALSE(accountID.empty());

    verify_state_pre(*clientContext, serverContext.get(), sequence);
    ot::otx::context::Server::DeliveryResult finished{};
    auto& stateMachine = *bob_state_machine_;
    auto started = stateMachine.UpdateAccount(accountID);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
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
        sequence + 2,
        bob_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        bob_counter_);
    const auto clientAccount = client_2_.Wallet().Internal().Account(accountID);
    const auto serverAccount = server_1_.Wallet().Internal().Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);

    verify_account(
        *serverContext.get().Nym(),
        *clientContext->Nym(),
        clientAccount,
        serverAccount,
        reason_c2_,
        reason_s1_);
    const auto workflowList =
        client_2_.Storage().PaymentWorkflowList(bob_nym_id_).size();

    EXPECT_EQ(3, workflowList);

    const auto [type, state] = client_2_.Storage().PaymentWorkflowState(
        bob_nym_id_, internal_transfer_workflow_id_);

    EXPECT_EQ(type, ot::otx::client::PaymentWorkflowType::InternalTransfer);
    EXPECT_EQ(state, ot::otx::client::PaymentWorkflowState::Completed);
    EXPECT_EQ(
        CHEQUE_AMOUNT + TRANSFER_AMOUNT - SECOND_TRANSFER_AMOUNT,
        serverAccount.get().GetBalance());
}

TEST_F(Test_Basic, send_message)
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
    auto messageID = ot::identifier::Generic{};
    auto setID = [&](const ot::identifier::Generic& in) -> void {
        messageID = in;
    };
    auto started = stateMachine.SendMessage(
        bob_nym_id_, ot::String::Factory(MESSAGE_TEXT), setID);

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

    EXPECT_FALSE(messageID.empty());
}

TEST_F(Test_Basic, receive_message)
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
    const auto mailList = client_2_.Activity().Mail(
        bob_nym_id_, ot::otx::client::StorageBox::MAILINBOX);

    ASSERT_EQ(1, mailList.size());

    const auto mailID = client_2_.Factory().IdentifierFromBase58(
        std::get<0>(*mailList.begin()));
    const auto text = client_2_.Activity().MailText(
        bob_nym_id_,
        mailID,
        ot::otx::client::StorageBox::MAILINBOX,
        reason_c2_);

    EXPECT_STREQ(MESSAGE_TEXT, text.get().c_str());
}

TEST_F(Test_Basic, request_admin_wrong_password)
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
    auto started =
        stateMachine.RequestAdmin(ot::String::Factory("WRONG PASSWORD"));

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

    EXPECT_FALSE(message->bool_);
    EXPECT_TRUE(context.AdminAttempted());
    EXPECT_FALSE(context.isAdmin());
}

TEST_F(Test_Basic, request_admin)
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
    auto started = stateMachine.RequestAdmin(
        ot::String::Factory(server_1_.GetAdminPassword()));

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

    EXPECT_TRUE(message->bool_);
    EXPECT_TRUE(context.AdminAttempted());
    EXPECT_TRUE(context.isAdmin());
}

TEST_F(Test_Basic, request_admin_already_admin)
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
    auto started = stateMachine.RequestAdmin(
        ot::String::Factory(server_1_.GetAdminPassword()));

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

    EXPECT_TRUE(message->bool_);
    EXPECT_TRUE(context.AdminAttempted());
    EXPECT_TRUE(context.isAdmin());
}

TEST_F(Test_Basic, request_admin_second_nym)
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
    auto started = stateMachine.RequestAdmin(
        ot::String::Factory(server_1_.GetAdminPassword()));

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

    EXPECT_FALSE(message->bool_);
    EXPECT_TRUE(context.AdminAttempted());
    EXPECT_FALSE(context.isAdmin());
}

TEST_F(Test_Basic, addClaim)
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
    auto started = stateMachine.AddClaim(
        ot::identity::wot::claim::SectionType::Scope,
        ot::identity::wot::claim::ClaimType::Server,
        ot::String::Factory(NEW_SERVER_NAME),
        true);

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

    EXPECT_TRUE(message->bool_);
}

TEST_F(Test_Basic, renameServer)
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

    ot::otx::context::Server::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;
    auto started = stateMachine.Start(
        ot::otx::OperationType::CheckNym,
        context.RemoteNym().ID(),
        extra_args_);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
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

    EXPECT_TRUE(message->bool_);
    EXPECT_FALSE(message->payload_->empty());

    const auto server = client_1_.Wallet().Internal().Server(server_1_id_);

    EXPECT_STREQ(NEW_SERVER_NAME, server->EffectiveName().c_str());
}

TEST_F(Test_Basic, addClaim_nclient_1_admin)
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
    auto started = stateMachine.AddClaim(
        ot::identity::wot::claim::SectionType::Scope,
        ot::identity::wot::claim::ClaimType::Server,
        ot::String::Factory(NEW_SERVER_NAME),
        true);

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

    EXPECT_TRUE(message->success_);
    EXPECT_FALSE(message->bool_);
}

TEST_F(Test_Basic, initiate_and_acknowledge_bailment)
{
    const auto aliceNym = client_1_.Wallet().Nym(alice_nym_id_);

    ASSERT_TRUE(aliceNym);

    auto peerrequest = client_1_.Factory().InternalSession().BailmentRequest(
        aliceNym,
        bob_nym_id_,
        find_unit_definition_id_2(),
        server_1_id_,
        reason_c1_);
    send_peer_request(
        aliceNym,
        peerrequest.as<ot::contract::peer::Request>(),
        ot::contract::peer::PeerRequestType::Bailment);
    receive_request(
        aliceNym,
        peerrequest.as<ot::contract::peer::Request>(),
        ot::contract::peer::PeerRequestType::Bailment);
    const auto bobNym = client_2_.Wallet().Nym(bob_nym_id_);

    ASSERT_TRUE(bobNym);

    auto peerreply = client_2_.Factory().InternalSession().BailmentReply(
        bobNym,
        alice_nym_id_,
        peerrequest->ID(),
        server_1_id_,
        "instructions",
        reason_c2_);
    send_peer_reply(
        bobNym,
        peerreply.as<ot::contract::peer::Reply>(),
        peerrequest.as<ot::contract::peer::Request>(),
        ot::contract::peer::PeerRequestType::Bailment);
    receive_reply(
        bobNym,
        aliceNym,
        peerreply.as<ot::contract::peer::Reply>(),
        peerrequest.as<ot::contract::peer::Request>(),
        ot::contract::peer::PeerRequestType::Bailment);
}

TEST_F(Test_Basic, initiate_and_acknowledge_outbailment)
{

    const auto aliceNym = client_1_.Wallet().Nym(alice_nym_id_);

    ASSERT_TRUE(aliceNym);

    auto peerrequest = client_1_.Factory().InternalSession().OutbailmentRequest(
        aliceNym,
        bob_nym_id_,
        find_unit_definition_id_2(),
        server_1_id_,
        1000,
        "message",
        reason_c1_);
    send_peer_request(
        aliceNym,
        peerrequest.as<ot::contract::peer::Request>(),
        ot::contract::peer::PeerRequestType::OutBailment);
    receive_request(
        aliceNym,
        peerrequest.as<ot::contract::peer::Request>(),
        ot::contract::peer::PeerRequestType::OutBailment);
    const auto bobNym = client_2_.Wallet().Nym(bob_nym_id_);

    ASSERT_TRUE(bobNym);

    auto peerreply = client_2_.Factory().InternalSession().OutbailmentReply(
        bobNym,
        alice_nym_id_,
        peerrequest->ID(),
        server_1_id_,
        "details",
        reason_c2_);
    send_peer_reply(
        bobNym,
        peerreply.as<ot::contract::peer::Reply>(),
        peerrequest.as<ot::contract::peer::Request>(),
        ot::contract::peer::PeerRequestType::OutBailment);
    receive_reply(
        bobNym,
        aliceNym,
        peerreply.as<ot::contract::peer::Reply>(),
        peerrequest.as<ot::contract::peer::Request>(),
        ot::contract::peer::PeerRequestType::OutBailment);
}

TEST_F(Test_Basic, notify_bailment_and_acknowledge_notice)
{
    const auto aliceNym = client_1_.Wallet().Nym(alice_nym_id_);

    ASSERT_TRUE(aliceNym);

    auto peerrequest = client_1_.Factory().InternalSession().BailmentNotice(
        aliceNym,
        bob_nym_id_,
        find_unit_definition_id_2(),
        server_1_id_,
        client_1_.Factory().IdentifierFromRandom(),
        client_1_.Factory().IdentifierFromRandom().asBase58(client_1_.Crypto()),
        1000,
        reason_c1_);
    send_peer_request(
        aliceNym,
        peerrequest.as<ot::contract::peer::Request>(),
        ot::contract::peer::PeerRequestType::PendingBailment);
    receive_request(
        aliceNym,
        peerrequest.as<ot::contract::peer::Request>(),
        ot::contract::peer::PeerRequestType::PendingBailment);
    const auto bobNym = client_2_.Wallet().Nym(bob_nym_id_);

    ASSERT_TRUE(bobNym);

    auto peerreply = client_2_.Factory().InternalSession().ReplyAcknowledgement(
        bobNym,
        alice_nym_id_,
        peerrequest->ID(),
        server_1_id_,
        ot::contract::peer::PeerRequestType::PendingBailment,
        true,
        reason_c2_);
    send_peer_reply(
        bobNym,
        peerreply.as<ot::contract::peer::Reply>(),
        peerrequest.as<ot::contract::peer::Request>(),
        ot::contract::peer::PeerRequestType::PendingBailment);
    receive_reply(
        bobNym,
        aliceNym,
        peerreply.as<ot::contract::peer::Reply>(),
        peerrequest.as<ot::contract::peer::Request>(),
        ot::contract::peer::PeerRequestType::PendingBailment);
}

TEST_F(Test_Basic, initiate_request_connection_and_acknowledge_connection)
{
    const auto aliceNym = client_1_.Wallet().Nym(alice_nym_id_);

    ASSERT_TRUE(aliceNym);

    auto peerrequest = client_1_.Factory().InternalSession().ConnectionRequest(
        aliceNym,
        bob_nym_id_,
        ot::contract::peer::ConnectionInfoType::Bitcoin,
        server_1_id_,
        reason_c1_);
    send_peer_request(
        aliceNym,
        peerrequest.as<ot::contract::peer::Request>(),
        ot::contract::peer::PeerRequestType::ConnectionInfo);
    receive_request(
        aliceNym,
        peerrequest.as<ot::contract::peer::Request>(),
        ot::contract::peer::PeerRequestType::ConnectionInfo);
    const auto bobNym = client_2_.Wallet().Nym(bob_nym_id_);

    ASSERT_TRUE(bobNym);

    auto peerreply = client_2_.Factory().InternalSession().ConnectionReply(
        bobNym,
        alice_nym_id_,
        peerrequest->ID(),
        server_1_id_,
        true,
        "localhost",
        "user",
        "password",
        "key",
        reason_c2_);
    send_peer_reply(
        bobNym,
        peerreply.as<ot::contract::peer::Reply>(),
        peerrequest.as<ot::contract::peer::Request>(),
        ot::contract::peer::PeerRequestType::ConnectionInfo);
    receive_reply(
        bobNym,
        aliceNym,
        peerreply.as<ot::contract::peer::Reply>(),
        peerrequest.as<ot::contract::peer::Request>(),
        ot::contract::peer::PeerRequestType::ConnectionInfo);
}

TEST_F(Test_Basic, initiate_store_secret_and_acknowledge_notice)
{
    const auto aliceNym = client_1_.Wallet().Nym(alice_nym_id_);

    ASSERT_TRUE(aliceNym);

    auto peerrequest = client_1_.Factory().InternalSession().StoreSecret(
        aliceNym,
        bob_nym_id_,
        ot::contract::peer::SecretType::Bip39,
        TEST_SEED,
        TEST_SEED_PASSPHRASE,
        server_1_id_,
        reason_c1_);
    send_peer_request(
        aliceNym,
        peerrequest.as<ot::contract::peer::Request>(),
        ot::contract::peer::PeerRequestType::StoreSecret);
    receive_request(
        aliceNym,
        peerrequest.as<ot::contract::peer::Request>(),
        ot::contract::peer::PeerRequestType::StoreSecret);
    const auto bobNym = client_2_.Wallet().Nym(bob_nym_id_);

    ASSERT_TRUE(bobNym);

    auto peerreply = client_2_.Factory().InternalSession().ReplyAcknowledgement(
        bobNym,
        alice_nym_id_,
        peerrequest->ID(),
        server_1_id_,
        ot::contract::peer::PeerRequestType::StoreSecret,
        true,
        reason_c2_);
    send_peer_reply(
        bobNym,
        peerreply.as<ot::contract::peer::Reply>(),
        peerrequest.as<ot::contract::peer::Request>(),
        ot::contract::peer::PeerRequestType::StoreSecret);
    receive_reply(
        bobNym,
        aliceNym,
        peerreply.as<ot::contract::peer::Reply>(),
        peerrequest.as<ot::contract::peer::Request>(),
        ot::contract::peer::PeerRequestType::StoreSecret);
}

TEST_F(Test_Basic, waitForCash_Alice)
{
    auto CheckMint = [&]() -> bool {
        return server_1_.GetPublicMint(find_unit_definition_id_1());
    };
    auto mint = CheckMint();
    const auto start = ot::Clock::now();
    std::cout << "Pausing for up to " << MINT_TIME_LIMIT_MINUTES
              << " minutes until mint generation is finished." << std::endl;

    while (false == mint) {
        std::cout << "* Waiting for mint..." << std::endl;
        ot::Sleep(10s);
        mint = CheckMint();
        const auto wait = ot::Clock::now() - start;
        const auto limit = std::chrono::minutes(MINT_TIME_LIMIT_MINUTES);

        if (wait > limit) { break; }
    }

    ASSERT_TRUE(mint);
}

TEST_F(Test_Basic, downloadMint)
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
    auto started = stateMachine.Start(
        ot::otx::OperationType::DownloadMint, find_unit_definition_id_1(), {});

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

    EXPECT_TRUE(message->success_);
    EXPECT_TRUE(message->bool_);
}

TEST_F(Test_Basic, withdrawCash)
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

    verify_state_pre(*clientContext, serverContext.get(), sequence);
    ot::otx::context::Server::DeliveryResult finished{};
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

    const ot::RequestNumber requestNumber =
        ot::String::StringToUlong(message->request_num_->Get());
    const auto result = translate_result(std::get<0>(finished));
    verify_state_post(
        client_2_,
        *clientContext,
        serverContext.get(),
        sequence + 1,
        bob_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        bob_counter_);

    const auto clientAccount = client_2_.Wallet().Internal().Account(accountID);
    const auto serverAccount = server_1_.Wallet().Internal().Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);
    EXPECT_EQ(
        CHEQUE_AMOUNT + TRANSFER_AMOUNT - SECOND_TRANSFER_AMOUNT - CASH_AMOUNT,
        serverAccount.get().GetBalance());
    EXPECT_EQ(
        serverAccount.get().GetBalance(), clientAccount.get().GetBalance());

    auto purseEditor = context.InternalServer().mutable_Purse(
        client_2_.Factory().Internal().UnitIDConvertSafe(
            asset_contract_1_->ID()),
        reason_c2_);
    auto& purse = purseEditor.get();

    EXPECT_EQ(purse.Value(), CASH_AMOUNT);
}

TEST_F(Test_Basic, send_cash)
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

    const auto& bob = *context.Nym();

    const auto unitID = client_2_.Factory().Internal().UnitIDConvertSafe(
        asset_contract_1_->ID());
    auto localPurseEditor =
        context.InternalServer().mutable_Purse(unitID, reason_c2_);
    auto& localPurse = localPurseEditor.get();

    ASSERT_TRUE(localPurse.Unlock(bob, reason_c2_));

    for (const auto& token : localPurse) {
        EXPECT_FALSE(token.ID(reason_c2_).empty());
    }

    auto sendPurse =
        client_2_.Factory().Purse(bob, server_1_id_, unitID, reason_c2_);

    ASSERT_TRUE(sendPurse);
    ASSERT_TRUE(localPurse.IsUnlocked());

    auto token = localPurse.Pop();

    while (token) {
        const auto added = sendPurse.Push(std::move(token), reason_c2_);

        EXPECT_TRUE(added);

        token = localPurse.Pop();
    }

    EXPECT_EQ(sendPurse.Value(), CASH_AMOUNT);

    const auto workflowID =
        client_2_.Workflow().AllocateCash(bob_nym_id_, sendPurse);

    EXPECT_FALSE(workflowID.empty());

    verify_state_pre(*clientContext, serverContext.get(), sequence);
    ot::otx::context::Server::DeliveryResult finished{};
    auto& stateMachine = *bob_state_machine_;
    auto started = stateMachine.SendCash(alice_nym_id_, workflowID);

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
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

    EXPECT_EQ(localPurse.Value(), 0);

    EXPECT_EQ(
        ot::otx::client::PaymentWorkflowState::Conveyed,
        client_2_.Workflow().WorkflowState(bob_nym_id_, workflowID));
}

TEST_F(Test_Basic, receive_cash)
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
    const auto unitID = client_1_.Factory().Internal().UnitIDConvertSafe(
        asset_contract_1_->ID());

    const auto workflows = client_1_.Storage().PaymentWorkflowsByState(
        alice_nym_id_,
        ot::otx::client::PaymentWorkflowType::IncomingCash,
        ot::otx::client::PaymentWorkflowState::Conveyed);

    ASSERT_EQ(1, workflows.size());

    const auto& workflowID =
        client_1_.Factory().IdentifierFromBase58(*workflows.begin());
    auto [state, incomingPurse] =
        client_1_.Workflow().InstantiatePurse(alice_nym_id_, workflowID);

    ASSERT_TRUE(incomingPurse);

    auto purseEditor =
        context.InternalServer().mutable_Purse(unitID, reason_c1_);
    auto& walletPurse = purseEditor.get();
    const auto& alice = *context.Nym();

    ASSERT_TRUE(incomingPurse.Unlock(alice, reason_c1_));
    ASSERT_TRUE(walletPurse.Unlock(alice, reason_c1_));

    auto token = incomingPurse.Pop();

    while (token) {
        EXPECT_TRUE(walletPurse.Push(std::move(token), reason_c1_));

        token = incomingPurse.Pop();
    }

    EXPECT_EQ(walletPurse.Value(), CASH_AMOUNT);
}

TEST_F(Test_Basic, depositCash)
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
    const auto& alice = *context.Nym();
    const auto accountID = find_issuer_account();
    const auto unitID = client_1_.Factory().Internal().UnitIDConvertSafe(
        asset_contract_1_->ID());
    const auto& walletPurse = context.Purse(unitID);

    ASSERT_TRUE(walletPurse);

    auto copy{walletPurse};
    auto sendPurse =
        client_1_.Factory().Purse(alice, server_1_id_, unitID, reason_c1_);

    ASSERT_TRUE(sendPurse);
    ASSERT_TRUE(walletPurse.Unlock(alice, reason_c1_));
    ASSERT_TRUE(copy.Unlock(alice, reason_c1_));
    ASSERT_TRUE(sendPurse.Unlock(alice, reason_c1_));

    auto token = copy.Pop();

    while (token) {
        ASSERT_TRUE(sendPurse.Push(std::move(token), reason_c1_));

        token = copy.Pop();
    }

    EXPECT_EQ(copy.Value(), 0);
    EXPECT_EQ(sendPurse.Value(), CASH_AMOUNT);
    EXPECT_EQ(walletPurse.Value(), CASH_AMOUNT);

    verify_state_pre(*clientContext, serverContext.get(), sequence);
    ot::otx::context::Server::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;
    auto started = stateMachine.DepositCash(accountID, std::move(sendPurse));

    ASSERT_TRUE(started);

    finished = stateMachine.GetFuture().get();
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
        sequence + 1,
        alice_counter_,
        requestNumber,
        result,
        message,
        SUCCESS,
        0,
        alice_counter_);
    const auto clientAccount = client_1_.Wallet().Internal().Account(accountID);
    const auto serverAccount = server_1_.Wallet().Internal().Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);

    EXPECT_EQ(
        -1 * (CHEQUE_AMOUNT + TRANSFER_AMOUNT - CASH_AMOUNT),
        serverAccount.get().GetBalance());
    EXPECT_EQ(
        serverAccount.get().GetBalance(), clientAccount.get().GetBalance());

    const auto& pWalletPurse = context.Purse(unitID);

    ASSERT_TRUE(pWalletPurse);
    EXPECT_EQ(pWalletPurse.Value(), 0);
}

TEST_F(Test_Basic, cleanup)
{
    alice_state_machine_.reset();
    bob_state_machine_.reset();
}
}  // namespace ottest
