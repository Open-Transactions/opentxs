// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <chrono>
#include <compare>
#include <ctime>
#include <future>
#include <iostream>
#include <memory>
#include <span>
#include <string_view>
#include <utility>

#include "internal/api/session/Storage.hpp"
#include "internal/core/Armored.hpp"
#include "internal/core/String.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/otx/client/Client.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/otx/common/Cheque.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/otx/consensus/Client.hpp"
#include "internal/otx/consensus/Consensus.hpp"
#include "internal/otx/consensus/Server.hpp"
#include "internal/util/Editor.hpp"
#include "internal/util/Pimpl.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/api/session/Wallet.internal.hpp"
#include "ottest/fixtures/common/Base.hpp"
#include "ottest/fixtures/otx/Basic.hpp"

namespace ottest
{
using namespace std::literals;

constexpr auto test_seed_ =
    "one two three four five six seven eight nine ten eleven twelve"sv;
constexpr auto test_seed_passphrase_ = "seed passphrase"sv;

TEST_F(Basic, zmq_disconnected)
{
    EXPECT_EQ(
        ot::network::ConnectionState::NOT_ESTABLISHED,
        client_1_.ZMQ().Status(server_1_id_));
}

TEST_F(Basic, getRequestNumber_nclient_1_registered)
{
    auto serverContext = Base::InternalWallet(client_1_).mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(alice_nym_id_);

    EXPECT_EQ(serverContext.get().Request(), 0);
    EXPECT_FALSE(clientContext);

    const auto number = serverContext.get().UpdateRequestNumber(reason_c1_);

    EXPECT_EQ(number, 0);
    EXPECT_EQ(serverContext.get().Request(), 0);

    clientContext =
        Base::InternalWallet(server_1_).ClientContext(alice_nym_id_);

    EXPECT_FALSE(clientContext);
}

TEST_F(Basic, zmq_connected)
{
    EXPECT_EQ(
        ot::network::ConnectionState::ACTIVE,
        client_1_.ZMQ().Status(server_1_id_));
}

TEST_F(Basic, registerNym_first_time)
{
    const ot::RequestNumber sequence = alice_counter_;
    const ot::RequestNumber messages{2};
    alice_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_1_).mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(alice_nym_id_);

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
    clientContext =
        Base::InternalWallet(server_1_).ClientContext(alice_nym_id_);
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

TEST_F(Basic, getTransactionNumbers_Alice)
{
    const ot::RequestNumber sequence = alice_counter_;
    const ot::RequestNumber messages{5};
    alice_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_1_).mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(alice_nym_id_);

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

TEST_F(Basic, Reregister)
{
    const ot::RequestNumber sequence = alice_counter_;
    const ot::RequestNumber messages{5};
    alice_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_1_).mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(alice_nym_id_);

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
    clientContext =
        Base::InternalWallet(server_1_).ClientContext(alice_nym_id_);
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

TEST_F(Basic, issueAsset)
{
    const ot::RequestNumber sequence = alice_counter_;
    const ot::RequestNumber messages{3};
    alice_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_1_).mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(alice_nym_id_);

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
        client_1_.Factory().AccountIDFromBase58(message->acct_id_->Bytes());

    ASSERT_FALSE(accountID.empty());

    const auto clientAccount =
        Base::InternalWallet(client_1_).Account(accountID);
    const auto serverAccount =
        Base::InternalWallet(server_1_).Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);

    verify_account(
        *serverContext.get().Signer(),
        *clientContext->Signer(),
        clientAccount,
        serverAccount,
        reason_c1_,
        reason_s1_);
}

TEST_F(Basic, checkNym_missing)
{
    const ot::RequestNumber sequence = alice_counter_;
    const ot::RequestNumber messages{1};
    alice_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_1_).mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(alice_nym_id_);

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

TEST_F(Basic, publishNym)
{
    const ot::RequestNumber sequence = alice_counter_;
    const ot::RequestNumber messages{1};
    alice_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_1_).mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(alice_nym_id_);

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

TEST_F(Basic, checkNym)
{
    const ot::RequestNumber sequence = alice_counter_;
    const ot::RequestNumber messages{1};
    alice_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_1_).mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(alice_nym_id_);

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

TEST_F(Basic, downloadServerContract_missing)
{
    const ot::RequestNumber sequence = alice_counter_;
    const ot::RequestNumber messages{1};
    alice_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_1_).mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(alice_nym_id_);

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

TEST_F(Basic, publishServer)
{
    const ot::RequestNumber sequence = alice_counter_;
    const ot::RequestNumber messages{1};
    alice_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_1_).mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.get(), sequence);

    ot::otx::context::Server::DeliveryResult finished{};
    auto& stateMachine = *alice_state_machine_;
    auto server = Base::InternalWallet(server_2_).Server(server_2_id_);
    auto bytes = ot::Space{};
    EXPECT_TRUE(server->Serialize(ot::writer(bytes), true));
    Base::InternalWallet(client_1_).Server(ot::reader(bytes));
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

TEST_F(Basic, downloadServerContract)
{
    const ot::RequestNumber sequence = alice_counter_;
    const ot::RequestNumber messages{1};
    alice_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_1_).mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(alice_nym_id_);

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

TEST_F(Basic, registerNym_Bob)
{
    const ot::RequestNumber sequence = bob_counter_;
    const ot::RequestNumber messages{2};
    bob_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_2_).mutable_ServerContext(
        bob_nym_id_, server_1_id_, reason_c2_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(bob_nym_id_);

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
    clientContext = Base::InternalWallet(server_1_).ClientContext(bob_nym_id_);
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

TEST_F(Basic, getInstrumentDefinition_missing)
{
    const auto sequence = ot::RequestNumber{bob_counter_};
    const auto messages = ot::RequestNumber{1};
    bob_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_2_).mutable_ServerContext(
        bob_nym_id_, server_1_id_, reason_c2_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(bob_nym_id_);

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

TEST_F(Basic, publishUnitDefinition)
{
    const ot::RequestNumber sequence = bob_counter_;
    const ot::RequestNumber messages{1};
    bob_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_2_).mutable_ServerContext(
        bob_nym_id_, server_1_id_, reason_c2_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(bob_nym_id_);

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

TEST_F(Basic, getInstrumentDefinition_Alice)
{
    const ot::RequestNumber sequence = alice_counter_;
    const ot::RequestNumber messages{1};
    alice_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_1_).mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(alice_nym_id_);

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

TEST_F(Basic, getInstrumentDefinition_Bob)
{
    const ot::RequestNumber sequence = bob_counter_;
    const ot::RequestNumber messages{1};
    bob_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_2_).mutable_ServerContext(
        bob_nym_id_, server_1_id_, reason_c2_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(bob_nym_id_);

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

TEST_F(Basic, registerAccount)
{
    const ot::RequestNumber sequence = bob_counter_;
    const ot::RequestNumber messages{3};
    bob_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_2_).mutable_ServerContext(
        bob_nym_id_, server_1_id_, reason_c2_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(bob_nym_id_);

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
        client_1_.Factory().AccountIDFromBase58(message->acct_id_->Bytes());
    const auto clientAccount =
        Base::InternalWallet(client_2_).Account(accountID);
    const auto serverAccount =
        Base::InternalWallet(server_1_).Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);

    verify_account(
        *serverContext.get().Signer(),
        *clientContext->Signer(),
        clientAccount,
        serverAccount,
        reason_c2_,
        reason_s1_);

    bob_account_1_id_ = accountID.asBase58(client_1_.Crypto());
}

TEST_F(Basic, send_cheque)
{
    const ot::RequestNumber sequence = alice_counter_;
    const ot::RequestNumber messages{1};
    alice_counter_ += messages;
    auto cheque = write_cheque(
        client_1_,
        server_1_id_,
        CHEQUE_AMOUNT,
        {},
        {},
        find_issuer_account(),
        alice_nym_id_,
        ot::String::Factory(CHEQUE_MEMO),
        bob_nym_id_);

    ASSERT_TRUE(cheque);

    cheque_transaction_number_ = cheque->GetTransactionNum();

    EXPECT_NE(0, cheque_transaction_number_);

    auto payment = payment_from_cheque(client_1_, ot::String::Factory(*cheque));

    ASSERT_TRUE(payment);

    auto serverContext = Base::InternalWallet(client_1_).mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(alice_nym_id_);

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
    const auto workflowList = client_1_.Storage()
                                  .Internal()
                                  .PaymentWorkflowList(alice_nym_id_)
                                  .size();

    EXPECT_EQ(1, workflowList);

    const auto workflows =
        client_1_.Storage().Internal().PaymentWorkflowsByState(
            alice_nym_id_,
            ot::otx::client::PaymentWorkflowType::OutgoingCheque,
            ot::otx::client::PaymentWorkflowState::Conveyed);

    ASSERT_EQ(1, workflows.size());

    outgoing_cheque_workflow_id_ = *workflows.begin();
}

TEST_F(Basic, getNymbox_receive_cheque)
{
    const ot::RequestNumber sequence = bob_counter_;
    const ot::RequestNumber messages{4};
    bob_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_2_).mutable_ServerContext(
        bob_nym_id_, server_1_id_, reason_c2_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(bob_nym_id_);

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
        client_2_.Storage().Internal().PaymentWorkflowList(bob_nym_id_);

    EXPECT_EQ(1, workflowList.size());

    const auto workflows =
        client_2_.Storage().Internal().PaymentWorkflowsByState(
            bob_nym_id_,
            ot::otx::client::PaymentWorkflowType::IncomingCheque,
            ot::otx::client::PaymentWorkflowState::Conveyed);

    EXPECT_EQ(1, workflows.size());

    incoming_cheque_workflow_id_ = *workflows.begin();
}

TEST_F(Basic, getNymbox_after_clearing_nymbox_2_Bob)
{
    const ot::RequestNumber sequence = bob_counter_;
    const ot::RequestNumber messages{1};
    bob_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_2_).mutable_ServerContext(
        bob_nym_id_, server_1_id_, reason_c2_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(bob_nym_id_);

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

TEST_F(Basic, depositCheque)
{
    const auto accountID = find_user_account();
    const auto& workflowID = incoming_cheque_workflow_id_;
    auto [state, pCheque] =
        client_2_.Workflow().InstantiateCheque(bob_nym_id_, workflowID);

    ASSERT_EQ(ot::otx::client::PaymentWorkflowState::Conveyed, state);
    ASSERT_TRUE(pCheque);

    const std::shared_ptr<ot::Cheque> cheque{std::move(pCheque)};
    const ot::RequestNumber sequence = bob_counter_;
    const ot::RequestNumber messages{12};
    bob_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_2_).mutable_ServerContext(
        bob_nym_id_, server_1_id_, reason_c2_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(bob_nym_id_);

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
    const auto clientAccount =
        Base::InternalWallet(client_2_).Account(accountID);
    const auto serverAccount =
        Base::InternalWallet(server_1_).Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);
    EXPECT_EQ(CHEQUE_AMOUNT, serverAccount.get().GetBalance());
    EXPECT_EQ(CHEQUE_AMOUNT, clientAccount.get().GetBalance());

    const auto workflowList =
        client_2_.Storage().Internal().PaymentWorkflowList(bob_nym_id_).size();

    EXPECT_EQ(1, workflowList);

    const auto [wType, wState] =
        client_2_.Storage().Internal().PaymentWorkflowState(
            bob_nym_id_, incoming_cheque_workflow_id_);

    EXPECT_EQ(ot::otx::client::PaymentWorkflowType::IncomingCheque, wType);
    EXPECT_EQ(ot::otx::client::PaymentWorkflowState::Completed, wState);
}

TEST_F(Basic, getAccountData_after_cheque_deposited)
{
    const ot::RequestNumber sequence = alice_counter_;
    const ot::RequestNumber messages{5};
    alice_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_1_).mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(alice_nym_id_);

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
    const auto clientAccount =
        Base::InternalWallet(client_1_).Account(accountID);
    const auto serverAccount =
        Base::InternalWallet(server_1_).Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);

    verify_account(
        *serverContext.get().Signer(),
        *clientContext->Signer(),
        clientAccount,
        serverAccount,
        reason_c1_,
        reason_s1_);
    const auto workflowList = client_1_.Storage()
                                  .Internal()
                                  .PaymentWorkflowList(alice_nym_id_)
                                  .size();

    EXPECT_EQ(1, workflowList);

    const auto [wType, wState] =
        client_1_.Storage().Internal().PaymentWorkflowState(
            alice_nym_id_, outgoing_cheque_workflow_id_);

    EXPECT_EQ(wType, ot::otx::client::PaymentWorkflowType::OutgoingCheque);
    // TODO should be completed?
    EXPECT_EQ(wState, ot::otx::client::PaymentWorkflowState::Accepted);
}

TEST_F(Basic, resync)
{
    break_consensus();
    const ot::RequestNumber sequence = alice_counter_;
    const ot::RequestNumber messages{2};
    alice_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_1_).mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(alice_nym_id_);

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
    clientContext =
        Base::InternalWallet(server_1_).ClientContext(alice_nym_id_);
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

TEST_F(Basic, sendTransfer)
{
    const ot::RequestNumber sequence = alice_counter_;
    const ot::RequestNumber messages{5};
    alice_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_1_).mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(alice_nym_id_);

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
        Base::InternalWallet(server_1_).Account(senderAccountID);

    ASSERT_TRUE(serverAccount);

    // A successful sent transfer has an immediate effect on the
    // sender's account balance.
    EXPECT_EQ(
        (-1 * (CHEQUE_AMOUNT + TRANSFER_AMOUNT)),
        serverAccount.get().GetBalance());

    const auto workflowList = client_1_.Storage()
                                  .Internal()
                                  .PaymentWorkflowList(alice_nym_id_)
                                  .size();

    EXPECT_EQ(2, workflowList);

    const auto workflows =
        client_1_.Storage().Internal().PaymentWorkflowsByState(
            alice_nym_id_,
            ot::otx::client::PaymentWorkflowType::OutgoingTransfer,
            ot::otx::client::PaymentWorkflowState::Acknowledged);

    ASSERT_EQ(1, workflows.size());

    outgoing_transfer_workflow_id_ = *workflows.cbegin();

    EXPECT_NE(outgoing_transfer_workflow_id_.size(), 0);

    auto partysize = int{-1};
    EXPECT_TRUE(client_1_.Workflow().WorkflowPartySize(
        alice_nym_id_, outgoing_transfer_workflow_id_, partysize));
    EXPECT_EQ(partysize, 0);
}

TEST_F(Basic, getAccountData_after_incomingTransfer)
{
    const ot::RequestNumber sequence = bob_counter_;
    const ot::RequestNumber messages{5};
    bob_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_2_).mutable_ServerContext(
        bob_nym_id_, server_1_id_, reason_c2_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(bob_nym_id_);

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
    const auto clientAccount =
        Base::InternalWallet(client_2_).Account(accountID);
    const auto serverAccount =
        Base::InternalWallet(server_1_).Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);

    verify_account(
        *serverContext.get().Signer(),
        *clientContext->Signer(),
        clientAccount,
        serverAccount,
        reason_c2_,
        reason_s1_);

    EXPECT_EQ(
        (CHEQUE_AMOUNT + TRANSFER_AMOUNT), serverAccount.get().GetBalance());

    const auto workflowList =
        client_2_.Storage().Internal().PaymentWorkflowList(bob_nym_id_).size();

    EXPECT_EQ(2, workflowList);

    const auto workflows =
        client_2_.Storage().Internal().PaymentWorkflowsByState(
            bob_nym_id_,
            ot::otx::client::PaymentWorkflowType::IncomingTransfer,
            ot::otx::client::PaymentWorkflowState::Completed);

    ASSERT_EQ(1, workflows.size());

    incoming_transfer_workflow_id_ = *workflows.cbegin();

    EXPECT_NE(incoming_transfer_workflow_id_.size(), 0);

    auto partysize = int{-1};
    EXPECT_TRUE(client_2_.Workflow().WorkflowPartySize(
        bob_nym_id_, incoming_transfer_workflow_id_, partysize));
    EXPECT_EQ(1, partysize);

    EXPECT_STREQ(
        alice_nym_id_.asBase58(client_1_.Crypto()).c_str(),
        client_2_.Workflow()
            .WorkflowParty(bob_nym_id_, incoming_transfer_workflow_id_, 0)
            .c_str());

    EXPECT_EQ(
        client_2_.Workflow().WorkflowType(
            bob_nym_id_, incoming_transfer_workflow_id_),
        ot::otx::client::PaymentWorkflowType::IncomingTransfer);
    EXPECT_EQ(
        client_2_.Workflow().WorkflowState(
            bob_nym_id_, incoming_transfer_workflow_id_),
        ot::otx::client::PaymentWorkflowState::Completed);
}

TEST_F(Basic, getAccountData_after_transfer_accepted)
{
    const ot::RequestNumber sequence = alice_counter_;
    const ot::RequestNumber messages{5};
    alice_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_1_).mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(alice_nym_id_);

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
    const auto clientAccount =
        Base::InternalWallet(client_1_).Account(accountID);
    const auto serverAccount =
        Base::InternalWallet(server_1_).Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);

    verify_account(
        *serverContext.get().Signer(),
        *clientContext->Signer(),
        clientAccount,
        serverAccount,
        reason_c1_,
        reason_s1_);

    EXPECT_EQ(
        -1 * (CHEQUE_AMOUNT + TRANSFER_AMOUNT),
        serverAccount.get().GetBalance());

    const auto workflowList =
        client_2_.Storage().Internal().PaymentWorkflowList(bob_nym_id_).size();

    EXPECT_EQ(2, workflowList);

    const auto [type, state] =
        client_1_.Storage().Internal().PaymentWorkflowState(
            alice_nym_id_, outgoing_transfer_workflow_id_);

    EXPECT_EQ(type, ot::otx::client::PaymentWorkflowType::OutgoingTransfer);
    EXPECT_EQ(state, ot::otx::client::PaymentWorkflowState::Completed);
}

TEST_F(Basic, register_second_account)
{
    const ot::RequestNumber sequence = bob_counter_;
    const ot::RequestNumber messages{3};
    bob_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_2_).mutable_ServerContext(
        bob_nym_id_, server_1_id_, reason_c2_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(bob_nym_id_);

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
        client_2_.Factory().AccountIDFromBase58(message->acct_id_->Bytes());
    const auto clientAccount =
        Base::InternalWallet(client_2_).Account(accountID);
    const auto serverAccount =
        Base::InternalWallet(server_1_).Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);

    verify_account(
        *serverContext.get().Signer(),
        *clientContext->Signer(),
        clientAccount,
        serverAccount,
        reason_c2_,
        reason_s1_);

    bob_account_2_id_ = accountID.asBase58(client_1_.Crypto());
}

TEST_F(Basic, send_internal_transfer)
{
    const ot::RequestNumber sequence = bob_counter_;
    const ot::RequestNumber messages{5};
    bob_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_2_).mutable_ServerContext(
        bob_nym_id_, server_1_id_, reason_c2_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(bob_nym_id_);

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
        Base::InternalWallet(server_1_).Account(senderAccountID);

    ASSERT_TRUE(serverAccount);

    // A successful sent transfer has an immediate effect on the sender's
    // account balance.
    EXPECT_EQ(
        (CHEQUE_AMOUNT + TRANSFER_AMOUNT - SECOND_TRANSFER_AMOUNT),
        serverAccount.get().GetBalance());

    std::size_t count{0}, tries{100};
    ot::UnallocatedSet<ot::identifier::Generic> workflows{};
    while (0 == count) {
        // The state change from ACKNOWLEDGED to CONVEYED occurs
        // asynchronously due to server push notifications so the order in
        // which these states are observed by the sender is undefined.
        ot::Sleep(100ms);
        workflows = client_2_.Storage().Internal().PaymentWorkflowsByState(
            bob_nym_id_,
            ot::otx::client::PaymentWorkflowType::InternalTransfer,
            ot::otx::client::PaymentWorkflowState::Acknowledged);
        count = workflows.size();

        if (0 == count) {
            workflows = client_2_.Storage().Internal().PaymentWorkflowsByState(
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
        client_2_.Storage().Internal().PaymentWorkflowList(bob_nym_id_).size();

    EXPECT_EQ(3, workflowList);
}

TEST_F(Basic, getAccountData_after_incoming_internal_Transfer)
{
    const ot::RequestNumber sequence = bob_counter_;
    const ot::RequestNumber messages{5};
    bob_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_2_).mutable_ServerContext(
        bob_nym_id_, server_1_id_, reason_c2_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(bob_nym_id_);

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
    const auto clientAccount =
        Base::InternalWallet(client_2_).Account(accountID);
    const auto serverAccount =
        Base::InternalWallet(server_1_).Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);

    verify_account(
        *serverContext.get().Signer(),
        *clientContext->Signer(),
        clientAccount,
        serverAccount,
        reason_c2_,
        reason_s1_);
    const auto workflowList =
        client_2_.Storage().Internal().PaymentWorkflowList(bob_nym_id_).size();

    EXPECT_EQ(3, workflowList);

    const auto [type, state] =
        client_2_.Storage().Internal().PaymentWorkflowState(
            bob_nym_id_, internal_transfer_workflow_id_);

    EXPECT_EQ(type, ot::otx::client::PaymentWorkflowType::InternalTransfer);
    EXPECT_EQ(state, ot::otx::client::PaymentWorkflowState::Conveyed);
    EXPECT_EQ(SECOND_TRANSFER_AMOUNT, serverAccount.get().GetBalance());
}

TEST_F(Basic, getAccountData_after_internal_transfer_accepted)
{
    const ot::RequestNumber sequence = bob_counter_;
    const ot::RequestNumber messages{5};
    bob_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_2_).mutable_ServerContext(
        bob_nym_id_, server_1_id_, reason_c2_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(bob_nym_id_);

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
    const auto clientAccount =
        Base::InternalWallet(client_2_).Account(accountID);
    const auto serverAccount =
        Base::InternalWallet(server_1_).Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);

    verify_account(
        *serverContext.get().Signer(),
        *clientContext->Signer(),
        clientAccount,
        serverAccount,
        reason_c2_,
        reason_s1_);
    const auto workflowList =
        client_2_.Storage().Internal().PaymentWorkflowList(bob_nym_id_).size();

    EXPECT_EQ(3, workflowList);

    const auto [type, state] =
        client_2_.Storage().Internal().PaymentWorkflowState(
            bob_nym_id_, internal_transfer_workflow_id_);

    EXPECT_EQ(type, ot::otx::client::PaymentWorkflowType::InternalTransfer);
    EXPECT_EQ(state, ot::otx::client::PaymentWorkflowState::Completed);
    EXPECT_EQ(
        CHEQUE_AMOUNT + TRANSFER_AMOUNT - SECOND_TRANSFER_AMOUNT,
        serverAccount.get().GetBalance());
}

TEST_F(Basic, send_message)
{
    const ot::RequestNumber sequence = alice_counter_;
    const ot::RequestNumber messages{1};
    alice_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_1_).mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(alice_nym_id_);

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

TEST_F(Basic, receive_message)
{
    const ot::RequestNumber sequence = bob_counter_;
    const ot::RequestNumber messages{4};
    bob_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_2_).mutable_ServerContext(
        bob_nym_id_, server_1_id_, reason_c2_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(bob_nym_id_);

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

TEST_F(Basic, request_admin_wrong_password)
{
    const ot::RequestNumber sequence = alice_counter_;
    const ot::RequestNumber messages{1};
    alice_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_1_).mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(alice_nym_id_);

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

TEST_F(Basic, request_admin)
{
    const ot::RequestNumber sequence = alice_counter_;
    const ot::RequestNumber messages{1};
    alice_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_1_).mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(alice_nym_id_);

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

TEST_F(Basic, request_admin_already_admin)
{
    const ot::RequestNumber sequence = alice_counter_;
    const ot::RequestNumber messages{1};
    alice_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_1_).mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(alice_nym_id_);

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

TEST_F(Basic, request_admin_second_nym)
{
    const ot::RequestNumber sequence = bob_counter_;
    const ot::RequestNumber messages{1};
    bob_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_2_).mutable_ServerContext(
        bob_nym_id_, server_1_id_, reason_c2_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(bob_nym_id_);

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

TEST_F(Basic, addClaim)
{
    const ot::RequestNumber sequence = alice_counter_;
    const ot::RequestNumber messages{1};
    alice_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_1_).mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(alice_nym_id_);

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

TEST_F(Basic, renameServer)
{
    const ot::RequestNumber sequence = alice_counter_;
    const ot::RequestNumber messages{1};
    alice_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_1_).mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(alice_nym_id_);

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

    const auto server = Base::InternalWallet(client_1_).Server(server_1_id_);

    EXPECT_STREQ(NEW_SERVER_NAME, server->EffectiveName().c_str());
}

TEST_F(Basic, addClaim_nclient_1_admin)
{
    const ot::RequestNumber sequence = bob_counter_;
    const ot::RequestNumber messages{1};
    bob_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_2_).mutable_ServerContext(
        bob_nym_id_, server_1_id_, reason_c2_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(bob_nym_id_);

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

TEST_F(Basic, initiate_and_acknowledge_bailment)
{
    const auto aliceNym = client_1_.Wallet().Nym(alice_nym_id_);

    ASSERT_TRUE(aliceNym);

    auto peerrequest = client_1_.Factory().BailmentRequest(
        aliceNym,
        bob_nym_id_,
        find_unit_definition_id_2(),
        server_1_id_,
        reason_c1_);
    send_peer_request(
        aliceNym, peerrequest, ot::contract::peer::RequestType::Bailment);
    receive_request(
        aliceNym, peerrequest, ot::contract::peer::RequestType::Bailment);
    const auto bobNym = client_2_.Wallet().Nym(bob_nym_id_);

    ASSERT_TRUE(bobNym);

    auto peerreply = client_2_.Factory().BailmentReply(
        bobNym, alice_nym_id_, peerrequest.ID(), "instructions", reason_c2_);
    send_peer_reply(
        bobNym,
        peerreply,
        peerrequest,
        ot::contract::peer::RequestType::Bailment);
    receive_reply(
        bobNym,
        aliceNym,
        peerreply,
        peerrequest,
        ot::contract::peer::RequestType::Bailment);
}

TEST_F(Basic, initiate_and_acknowledge_outbailment)
{

    const auto aliceNym = client_1_.Wallet().Nym(alice_nym_id_);

    ASSERT_TRUE(aliceNym);

    auto peerrequest = client_1_.Factory().OutbailmentRequest(
        aliceNym,
        bob_nym_id_,
        find_unit_definition_id_2(),
        server_1_id_,
        1000,
        "message",
        reason_c1_);
    send_peer_request(
        aliceNym, peerrequest, ot::contract::peer::RequestType::OutBailment);
    receive_request(
        aliceNym, peerrequest, ot::contract::peer::RequestType::OutBailment);
    const auto bobNym = client_2_.Wallet().Nym(bob_nym_id_);

    ASSERT_TRUE(bobNym);

    auto peerreply = client_2_.Factory().OutbailmentReply(
        bobNym, alice_nym_id_, peerrequest.ID(), "details", reason_c2_);
    send_peer_reply(
        bobNym,
        peerreply,
        peerrequest,
        ot::contract::peer::RequestType::OutBailment);
    receive_reply(
        bobNym,
        aliceNym,
        peerreply,
        peerrequest,
        ot::contract::peer::RequestType::OutBailment);
}

TEST_F(Basic, notify_bailment_and_acknowledge_notice)
{
    const auto aliceNym = client_1_.Wallet().Nym(alice_nym_id_);

    ASSERT_TRUE(aliceNym);

    auto peerrequest = client_1_.Factory().BailmentNoticeRequest(
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
        peerrequest,
        ot::contract::peer::RequestType::PendingBailment);
    receive_request(
        aliceNym,
        peerrequest,
        ot::contract::peer::RequestType::PendingBailment);
    const auto bobNym = client_2_.Wallet().Nym(bob_nym_id_);

    ASSERT_TRUE(bobNym);

    auto peerreply = client_2_.Factory().BailmentNoticeReply(
        bobNym, alice_nym_id_, peerrequest.ID(), true, reason_c2_);
    send_peer_reply(
        bobNym,
        peerreply,
        peerrequest,
        ot::contract::peer::RequestType::PendingBailment);
    receive_reply(
        bobNym,
        aliceNym,
        peerreply,
        peerrequest,
        ot::contract::peer::RequestType::PendingBailment);
}

TEST_F(Basic, initiate_request_connection_and_acknowledge_connection)
{
    const auto aliceNym = client_1_.Wallet().Nym(alice_nym_id_);

    ASSERT_TRUE(aliceNym);

    auto peerrequest = client_1_.Factory().ConnectionRequest(
        aliceNym,
        bob_nym_id_,
        ot::contract::peer::ConnectionInfoType::Bitcoin,
        reason_c1_);
    send_peer_request(
        aliceNym, peerrequest, ot::contract::peer::RequestType::ConnectionInfo);
    receive_request(
        aliceNym, peerrequest, ot::contract::peer::RequestType::ConnectionInfo);
    const auto bobNym = client_2_.Wallet().Nym(bob_nym_id_);

    ASSERT_TRUE(bobNym);

    auto peerreply = client_2_.Factory().ConnectionReply(
        bobNym,
        alice_nym_id_,
        peerrequest.ID(),
        true,
        "localhost",
        "user",
        "password",
        "key",
        reason_c2_);
    send_peer_reply(
        bobNym,
        peerreply,
        peerrequest,
        ot::contract::peer::RequestType::ConnectionInfo);
    receive_reply(
        bobNym,
        aliceNym,
        peerreply,
        peerrequest,
        ot::contract::peer::RequestType::ConnectionInfo);
}

TEST_F(Basic, initiate_store_secret_and_acknowledge_notice)
{
    const auto aliceNym = client_1_.Wallet().Nym(alice_nym_id_);

    ASSERT_TRUE(aliceNym);

    static constexpr auto data = {test_seed_, test_seed_passphrase_};
    auto peerrequest = client_1_.Factory().StoreSecretRequest(
        aliceNym,
        bob_nym_id_,
        ot::contract::peer::SecretType::Bip39,
        data,
        reason_c1_);
    send_peer_request(
        aliceNym, peerrequest, ot::contract::peer::RequestType::StoreSecret);
    receive_request(
        aliceNym, peerrequest, ot::contract::peer::RequestType::StoreSecret);
    const auto bobNym = client_2_.Wallet().Nym(bob_nym_id_);

    ASSERT_TRUE(bobNym);

    auto peerreply = client_2_.Factory().StoreSecretReply(
        bobNym, alice_nym_id_, peerrequest.ID(), true, reason_c2_);
    send_peer_reply(
        bobNym,
        peerreply,
        peerrequest,
        ot::contract::peer::RequestType::StoreSecret);
    receive_reply(
        bobNym,
        aliceNym,
        peerreply,
        peerrequest,
        ot::contract::peer::RequestType::StoreSecret);
}

TEST_F(Basic, waitForCash_Alice)
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

TEST_F(Basic, downloadMint)
{
    const ot::RequestNumber sequence = bob_counter_;
    const ot::RequestNumber messages{1};
    bob_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_2_).mutable_ServerContext(
        bob_nym_id_, server_1_id_, reason_c2_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(bob_nym_id_);

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

TEST_F(Basic, withdrawCash)
{
    const ot::RequestNumber sequence = bob_counter_;
    const ot::RequestNumber messages{4};
    bob_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_2_).mutable_ServerContext(
        bob_nym_id_, server_1_id_, reason_c2_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(bob_nym_id_);

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

    const auto clientAccount =
        Base::InternalWallet(client_2_).Account(accountID);
    const auto serverAccount =
        Base::InternalWallet(server_1_).Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);
    EXPECT_EQ(
        CHEQUE_AMOUNT + TRANSFER_AMOUNT - SECOND_TRANSFER_AMOUNT - CASH_AMOUNT,
        serverAccount.get().GetBalance());
    EXPECT_EQ(
        serverAccount.get().GetBalance(), clientAccount.get().GetBalance());

    auto purseEditor = context.InternalServer().mutable_Purse(
        asset_contract_1_->ID(), reason_c2_);
    auto& purse = purseEditor.get();

    EXPECT_EQ(purse.Value(), CASH_AMOUNT);
}

TEST_F(Basic, send_cash)
{
    const ot::RequestNumber sequence = bob_counter_;
    const ot::RequestNumber messages{1};
    bob_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_2_).mutable_ServerContext(
        bob_nym_id_, server_1_id_, reason_c2_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(bob_nym_id_);

    ASSERT_TRUE(clientContext);

    const auto& bob = *context.Signer();
    const auto& unitID = asset_contract_1_->ID();
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

TEST_F(Basic, receive_cash)
{
    const ot::RequestNumber sequence = alice_counter_;
    const ot::RequestNumber messages{4};
    alice_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_1_).mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(alice_nym_id_);

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
    const auto& unitID = asset_contract_1_->ID();
    const auto workflows =
        client_1_.Storage().Internal().PaymentWorkflowsByState(
            alice_nym_id_,
            ot::otx::client::PaymentWorkflowType::IncomingCash,
            ot::otx::client::PaymentWorkflowState::Conveyed);

    ASSERT_EQ(1, workflows.size());

    const auto& workflowID = *workflows.begin();
    auto [state, incomingPurse] =
        client_1_.Workflow().InstantiatePurse(alice_nym_id_, workflowID);

    ASSERT_TRUE(incomingPurse);

    auto purseEditor =
        context.InternalServer().mutable_Purse(unitID, reason_c1_);
    auto& walletPurse = purseEditor.get();
    const auto& alice = *context.Signer();

    ASSERT_TRUE(incomingPurse.Unlock(alice, reason_c1_));
    ASSERT_TRUE(walletPurse.Unlock(alice, reason_c1_));

    auto token = incomingPurse.Pop();

    while (token) {
        EXPECT_TRUE(walletPurse.Push(std::move(token), reason_c1_));

        token = incomingPurse.Pop();
    }

    EXPECT_EQ(walletPurse.Value(), CASH_AMOUNT);
}

TEST_F(Basic, depositCash)
{
    const ot::RequestNumber sequence = alice_counter_;
    const ot::RequestNumber messages{4};
    alice_counter_ += messages;
    auto serverContext = Base::InternalWallet(client_1_).mutable_ServerContext(
        alice_nym_id_, server_1_id_, reason_c1_);
    auto& context = serverContext.get();
    auto clientContext =
        Base::InternalWallet(server_1_).ClientContext(alice_nym_id_);

    ASSERT_TRUE(clientContext);
    const auto& alice = *context.Signer();
    const auto accountID = find_issuer_account();
    const auto& unitID = asset_contract_1_->ID();
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
    const auto clientAccount =
        Base::InternalWallet(client_1_).Account(accountID);
    const auto serverAccount =
        Base::InternalWallet(server_1_).Account(accountID);

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

TEST_F(Basic, cleanup)
{
    alice_state_machine_.reset();
    bob_state_machine_.reset();
}
}  // namespace ottest
