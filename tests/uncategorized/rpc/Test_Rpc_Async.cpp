// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <opentxs/protobuf/APIArgument.pb.h>
#include <opentxs/protobuf/AcceptPendingPayment.pb.h>
#include <opentxs/protobuf/AccountEvent.pb.h>
#include <opentxs/protobuf/PaymentWorkflowEnums.pb.h>
#include <opentxs/protobuf/RPCCommand.pb.h>
#include <opentxs/protobuf/RPCEnums.pb.h>
#include <opentxs/protobuf/RPCPush.pb.h>
#include <opentxs/protobuf/RPCResponse.pb.h>
#include <opentxs/protobuf/RPCStatus.pb.h>
#include <opentxs/protobuf/RPCTask.pb.h>
#include <opentxs/protobuf/SendPayment.pb.h>
#include <opentxs/protobuf/TaskComplete.pb.h>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <functional>
#include <future>
#include <iosfwd>
#include <memory>
#include <utility>

#include "internal/serialization/protobuf/Check.hpp"
#include "opentxs/protobuf/syntax/RPCPush.hpp"
#include "opentxs/protobuf/syntax/RPCResponse.hpp"
#include "ottest/Basic.hpp"
#include "ottest/fixtures/common/Base.hpp"
#include "ottest/fixtures/rpc/RpcAsync.hpp"

namespace ottest
{
namespace ot = opentxs;

TEST_F(RpcAsync, Setup)
{
    setup();
    OTTestEnvironment::GetOT();
    auto& senderClient = get_session(sender_session_);
    auto& receiverClient = get_session(receiver_session_);
    auto reasonS = senderClient.Factory().PasswordPrompt(__func__);
    auto reasonR = receiverClient.Factory().PasswordPrompt(__func__);

    try {
        senderClient.Wallet().Server(server_id_);
        EXPECT_FALSE(true);
    } catch (...) {
        EXPECT_FALSE(false);
    }

    try {
        receiverClient.Wallet().Server(server_id_);
        EXPECT_FALSE(true);
    } catch (...) {
        EXPECT_FALSE(false);
    }

    EXPECT_NE(sender_session_, receiver_session_);
}

TEST_F(RpcAsync, RegisterNym_Receiver)
{
    // Register the receiver nym.
    auto command = init(protobuf::RPCCOMMAND_REGISTERNYM);
    command.set_session(receiver_session_);
    command.set_owner(receiver_nym_id_->str());
    command.set_notary(server_id_->str());
    auto future = set_push_checker(default_push_callback);
    auto response = ot_.RPC(command);

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(protobuf::RPCRESPONSE_QUEUED, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());
    ASSERT_TRUE(0 == response.identifier_size());
    ASSERT_EQ(1, response.task_size());
    EXPECT_TRUE(check_push_results(future.get()));
}

TEST_F(RpcAsync, Create_Issuer_Account)
{
    auto command = init(protobuf::RPCCOMMAND_ISSUEUNITDEFINITION);
    command.set_session(sender_session_);
    command.set_owner(sender_nym_id_->str());
    auto& server = ot_.Server(static_cast<int>(get_index(server_)));
    command.set_notary(server.ID().asBase58(ot_.Crypto()));
    command.set_unit(unit_definition_id_->str());
    command.add_identifier(ISSUER_ACCOUNT_LABEL);
    auto future = set_push_checker(default_push_callback);
    auto response = ot_.RPC(command);

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(protobuf::RPCRESPONSE_QUEUED, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());
    ASSERT_EQ(1, response.task_size());
    EXPECT_TRUE(check_push_results(future.get()));
}

TEST_F(RpcAsync, Send_Payment_Cheque_No_Contact)
{
    auto& client_a = get_session(sender_session_);
    auto command = init(protobuf::RPCCOMMAND_SENDPAYMENT);
    command.set_session(sender_session_);

    const auto issueraccounts =
        client_a.Storage().AccountsByIssuer(sender_nym_id_);

    ASSERT_TRUE(false == issueraccounts.empty());

    auto issueraccountid = *issueraccounts.cbegin();

    auto sendpayment = command.mutable_sendpayment();

    ASSERT_NE(nullptr, sendpayment);

    sendpayment->set_version(1);
    sendpayment->set_type(protobuf::RPCPAYMENTTYPE_CHEQUE);
    // Use an id that isn't a contact.
    sendpayment->set_contact(receiver_nym_id_->str());
    sendpayment->set_sourceaccount(issueraccountid->str());
    sendpayment->set_memo("Send_Payment_Cheque test");
    sendpayment->set_amount(100);

    auto response = ot_.RPC(command);

    EXPECT_EQ(RESPONSE_VERSION, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(
        protobuf::RPCRESPONSE_CONTACT_NOT_FOUND, response.status(0).code());
}

TEST_F(RpcAsync, Send_Payment_Cheque_No_Account_Owner)
{
    auto& client_a =
        ot_.ClientSession(static_cast<int>(get_index(sender_session_)));
    auto command = init(protobuf::RPCCOMMAND_SENDPAYMENT);
    command.set_session(sender_session_);

    const auto issueraccounts =
        client_a.Storage().AccountsByIssuer(sender_nym_id_);

    ASSERT_TRUE(false == issueraccounts.empty());

    auto issueraccountid = *issueraccounts.cbegin();

    const auto contact = client_a.Contacts().NewContact(
        "label_only_contact",
        ot::identifier::Nym::Factory(),
        client_a.Factory().PaymentCode(ot::UnallocatedCString{}));

    auto sendpayment = command.mutable_sendpayment();

    ASSERT_NE(nullptr, sendpayment);

    sendpayment->set_version(1);
    sendpayment->set_type(protobuf::RPCPAYMENTTYPE_CHEQUE);
    sendpayment->set_contact(contact->ID().asBase58(ot_.Crypto()));
    sendpayment->set_sourceaccount(receiver_nym_id_->str());
    sendpayment->set_memo("Send_Payment_Cheque test");
    sendpayment->set_amount(100);

    auto response = ot_.RPC(command);

    EXPECT_EQ(RESPONSE_VERSION, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(
        protobuf::RPCRESPONSE_ACCOUNT_OWNER_NOT_FOUND,
        response.status(0).code());
}

TEST_F(RpcAsync, Send_Payment_Cheque_No_Path)
{
    auto& client_a =
        ot_.ClientSession(static_cast<int>(get_index(sender_session_)));
    auto command = init(protobuf::RPCCOMMAND_SENDPAYMENT);
    command.set_session(sender_session_);

    const auto issueraccounts =
        client_a.Storage().AccountsByIssuer(sender_nym_id_);

    ASSERT_TRUE(false == issueraccounts.empty());

    auto issueraccountid = *issueraccounts.cbegin();

    const auto contact = client_a.Contacts().NewContact(
        "label_only_contact",
        ot::identifier::Nym::Factory(),
        client_a.Factory().PaymentCode(ot::UnallocatedCString{}));

    auto sendpayment = command.mutable_sendpayment();

    ASSERT_NE(nullptr, sendpayment);

    sendpayment->set_version(1);
    sendpayment->set_type(protobuf::RPCPAYMENTTYPE_CHEQUE);
    sendpayment->set_contact(contact->ID().asBase58(ot_.Crypto()));
    sendpayment->set_sourceaccount(issueraccountid->str());
    sendpayment->set_memo("Send_Payment_Cheque test");
    sendpayment->set_amount(100);

    auto response = ot_.RPC(command);

    EXPECT_EQ(RESPONSE_VERSION, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(
        protobuf::RPCRESPONSE_NO_PATH_TO_RECIPIENT, response.status(0).code());
}

TEST_F(RpcAsync, Send_Payment_Cheque)
{
    auto& client_a =
        ot_.ClientSession(static_cast<int>(get_index(sender_session_)));
    auto command = init(protobuf::RPCCOMMAND_SENDPAYMENT);
    command.set_session(sender_session_);
    auto& client_b = get_session(receiver_session_);

    ASSERT_FALSE(receiver_nym_id_->empty());

    auto nym5 = client_b.Wallet().Nym(receiver_nym_id_);

    ASSERT_TRUE(bool(nym5));

    auto& contacts = client_a.Contacts();
    const auto contact = contacts.NewContact(
        ot::UnallocatedCString(TEST_NYM_5),
        receiver_nym_id_,
        client_a.Factory().PaymentCode(nym5->PaymentCode()));

    ASSERT_TRUE(contact);

    const auto issueraccounts =
        client_a.Storage().AccountsByIssuer(sender_nym_id_);

    ASSERT_TRUE(false == issueraccounts.empty());

    auto issueraccountid = *issueraccounts.cbegin();

    auto sendpayment = command.mutable_sendpayment();

    ASSERT_NE(nullptr, sendpayment);

    sendpayment->set_version(1);
    sendpayment->set_type(protobuf::RPCPAYMENTTYPE_CHEQUE);
    sendpayment->set_contact(contact->ID().asBase58(ot_.Crypto()));
    sendpayment->set_sourceaccount(issueraccountid->str());
    sendpayment->set_memo("Send_Payment_Cheque test");
    sendpayment->set_amount(100);

    protobuf::RPCResponse response;

    auto future = set_push_checker(default_push_callback);
    do {
        response = ot_.RPC(command);

        ASSERT_EQ(1, response.status_size());
        auto responseCode = response.status(0).code();
        auto responseIsValid = responseCode == protobuf::RPCRESPONSE_RETRY ||
                               responseCode == protobuf::RPCRESPONSE_QUEUED;
        ASSERT_TRUE(responseIsValid);
        EXPECT_EQ(RESPONSE_VERSION, response.version());
        ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
        ASSERT_EQ(command.type(), response.type());

        if (responseCode == protobuf::RPCRESPONSE_RETRY) {
            client_a.OTX().ContextIdle(sender_nym_id_, server_id_).get();
            command.set_cookie(ot::identifier::Generic::Random()->str());
        }
    } while (protobuf::RPCRESPONSE_RETRY == response.status(0).code());

    client_a.OTX().Refresh();
    client_a.OTX().ContextIdle(sender_nym_id_, server_id_).get();
    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(protobuf::RPCRESPONSE_QUEUED, response.status(0).code());
    EXPECT_EQ(1, response.task_size());
    EXPECT_TRUE(check_push_results(future.get()));
}

TEST_F(RpcAsync, Get_Pending_Payments)
{
    auto& client_b =
        ot_.ClientSession(static_cast<int>(get_index(receiver_session_)));

    // Make sure the workflows on the client are up-to-date.
    client_b.OTX().Refresh();
    auto future1 =
        client_b.OTX().ContextIdle(receiver_nym_id_, intro_server_id_);
    auto future2 = client_b.OTX().ContextIdle(receiver_nym_id_, server_id_);
    future1.get();
    future2.get();
    const auto& workflow = client_b.Workflow();
    ot::UnallocatedSet<identifier::Generic> workflows;
    auto end = std::time(nullptr) + 60;
    do {
        workflows = workflow.List(
            receiver_nym_id_,
            ot::otx::client::PaymentWorkflowType::IncomingCheque,
            ot::otx::client::PaymentWorkflowState::Conveyed);
    } while (workflows.empty() && std::time(nullptr) < end);

    ASSERT_TRUE(!workflows.empty());

    auto command = init(protobuf::RPCCOMMAND_GETPENDINGPAYMENTS);

    command.set_session(receiver_session_);
    command.set_owner(receiver_nym_id_->str());

    auto response = ot_.RPC(command);

    ASSERT_TRUE(protobuf::Validate(opentxs::LogError(), response));
    EXPECT_EQ(RESPONSE_VERSION, response.version());

    ASSERT_EQ(1, response.status_size());
    EXPECT_EQ(protobuf::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    EXPECT_EQ(command.type(), response.type());

    EXPECT_EQ(1, response.accountevent_size());

    const auto& accountevent = response.accountevent(0);
    workflow_id_ = ot::identifier::Generic::Factory(accountevent.workflow());

    ASSERT_TRUE(!workflow_id_->empty());
}

TEST_F(RpcAsync, Create_Compatible_Account)
{
    auto command = init(protobuf::RPCCOMMAND_CREATECOMPATIBLEACCOUNT);

    command.set_session(receiver_session_);
    command.set_owner(receiver_nym_id_->str());
    command.add_identifier(workflow_id_->str());

    auto response = ot_.RPC(command);

    ASSERT_TRUE(protobuf::Validate(opentxs::LogError(), response));
    EXPECT_EQ(RESPONSE_VERSION, response.version());

    ASSERT_EQ(1, response.status_size());
    EXPECT_EQ(protobuf::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    EXPECT_EQ(command.type(), response.type());

    destination_account_id_ =
        ot::identifier::Generic::Factory(response.identifier(0).c_str());

    EXPECT_TRUE(!destination_account_id_->empty());
}

TEST_F(RpcAsync, Get_Compatible_Account_Bad_Workflow)
{
    auto command = init(protobuf::RPCCOMMAND_GETCOMPATIBLEACCOUNTS);

    command.set_session(receiver_session_);
    command.set_owner(receiver_nym_id_->str());
    // Use an id that isn't a workflow.
    command.add_identifier(receiver_nym_id_->str());

    auto response = ot_.RPC(command);

    ASSERT_TRUE(protobuf::Validate(opentxs::LogError(), response));
    EXPECT_EQ(RESPONSE_VERSION, response.version());

    ASSERT_EQ(response.status_size(), 1);
    EXPECT_EQ(
        protobuf::RPCRESPONSE_WORKFLOW_NOT_FOUND, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    EXPECT_EQ(command.type(), response.type());

    EXPECT_EQ(response.identifier_size(), 0);
}

TEST_F(RpcAsync, Get_Compatible_Account)
{
    auto command = init(protobuf::RPCCOMMAND_GETCOMPATIBLEACCOUNTS);

    command.set_session(receiver_session_);
    command.set_owner(receiver_nym_id_->str());
    command.add_identifier(workflow_id_->str());

    auto response = ot_.RPC(command);

    ASSERT_TRUE(protobuf::Validate(opentxs::LogError(), response));
    EXPECT_EQ(RESPONSE_VERSION, response.version());

    ASSERT_EQ(1, response.status_size());
    EXPECT_EQ(protobuf::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    EXPECT_EQ(command.type(), response.type());

    EXPECT_EQ(1, response.identifier_size());

    ASSERT_STREQ(
        destination_account_id_->str().c_str(), response.identifier(0).c_str());
}

TEST_F(RpcAsync, Accept_Pending_Payments_Bad_Workflow)
{
    auto command = init(protobuf::RPCCOMMAND_ACCEPTPENDINGPAYMENTS);

    command.set_session(receiver_session_);
    auto& acceptpendingpayment = *command.add_acceptpendingpayment();
    acceptpendingpayment.set_version(1);
    acceptpendingpayment.set_destinationaccount(destination_account_id_->str());
    // Use an id that isn't a workflow.
    acceptpendingpayment.set_workflow(destination_account_id_->str());

    auto response = ot_.RPC(command);

    ASSERT_TRUE(protobuf::Validate(opentxs::LogError(), response));
    EXPECT_EQ(RESPONSE_VERSION, response.version());

    ASSERT_EQ(1, response.status_size());
    EXPECT_EQ(
        protobuf::RPCRESPONSE_WORKFLOW_NOT_FOUND, response.status(0).code());
    EXPECT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    EXPECT_EQ(command.type(), response.type());
    EXPECT_EQ(1, response.task_size());

    auto pending_payment_task_id = response.task(0).id();

    ASSERT_TRUE(pending_payment_task_id.empty());
}

TEST_F(RpcAsync, Accept_Pending_Payments)
{
    auto command = init(protobuf::RPCCOMMAND_ACCEPTPENDINGPAYMENTS);

    command.set_session(receiver_session_);
    auto& acceptpendingpayment = *command.add_acceptpendingpayment();
    acceptpendingpayment.set_version(1);
    acceptpendingpayment.set_destinationaccount(destination_account_id_->str());
    acceptpendingpayment.set_workflow(workflow_id_->str());

    auto future = set_push_checker(default_push_callback);
    auto response = ot_.RPC(command);

    ASSERT_TRUE(protobuf::Validate(opentxs::LogError(), response));
    EXPECT_EQ(RESPONSE_VERSION, response.version());

    ASSERT_EQ(1, response.status_size());
    EXPECT_EQ(protobuf::RPCRESPONSE_QUEUED, response.status(0).code());
    EXPECT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    EXPECT_EQ(command.type(), response.type());
    EXPECT_EQ(1, response.task_size());
    EXPECT_TRUE(check_push_results(future.get()));
}

TEST_F(RpcAsync, Get_Account_Activity)
{
    const auto& client =
        ot_.ClientSession(static_cast<int>(get_index(receiver_session_)));
    client.OTX().Refresh();
    client.OTX().ContextIdle(receiver_nym_id_, server_id_).get();

    auto& client_a =
        ot_.ClientSession(static_cast<int>(get_index(sender_session_)));

    const auto& workflow = client_a.Workflow();
    ot::UnallocatedSet<identifier::Generic> workflows;
    auto end = std::time(nullptr) + 60;
    do {
        workflows = workflow.List(
            sender_nym_id_,
            ot::otx::client::PaymentWorkflowType::OutgoingCheque,
            ot::otx::client::PaymentWorkflowState::Conveyed);

        if (workflows.empty()) { ot::sleep(100ms); }
    } while (workflows.empty() && std::time(nullptr) < end);

    ASSERT_TRUE(!workflows.empty());

    const auto issueraccounts =
        client_a.Storage().AccountsByIssuer(sender_nym_id_);

    ASSERT_TRUE(!issueraccounts.empty());
    ASSERT_EQ(1, issueraccounts.size());
    auto issuer_account_id = *issueraccounts.cbegin();

    auto command = init(protobuf::RPCCOMMAND_GETACCOUNTACTIVITY);
    command.set_session(sender_session_);
    command.add_identifier(issuer_account_id->str());
    protobuf::RPCResponse response;
    do {
        response = ot_.RPC(command);

        ASSERT_TRUE(protobuf::Validate(opentxs::LogError(), response));
        EXPECT_EQ(RESPONSE_VERSION, response.version());

        ASSERT_EQ(1, response.status_size());
        auto responseCode = response.status(0).code();
        auto responseIsValid = responseCode == protobuf::RPCRESPONSE_NONE ||
                               responseCode == protobuf::RPCRESPONSE_SUCCESS;
        ASSERT_TRUE(responseIsValid);
        EXPECT_STREQ(command.cookie().c_str(), response.cookie().c_str());
        EXPECT_EQ(command.type(), response.type());
        if (response.accountevent_size() < 1) {
            client_a.OTX().Refresh();
            client_a.OTX().ContextIdle(sender_nym_id_, server_id_).get();
            command.set_cookie(ot::identifier::Generic::Random()->str());
        }
    } while (protobuf::RPCRESPONSE_NONE == response.status(0).code() ||
             response.accountevent_size() < 1);

    // TODO properly count the number of updates on the appropriate ui widget

    auto foundevent = false;
    for (const auto& accountevent : response.accountevent()) {
        EXPECT_EQ(ACCOUNTEVENT_VERSION, accountevent.version());
        EXPECT_STREQ(
            issuer_account_id->str().c_str(), accountevent.id().c_str());
        if (protobuf::ACCOUNTEVENT_OUTGOINGCHEQUE == accountevent.type()) {
            EXPECT_EQ(-100, accountevent.amount());
            foundevent = true;
        }
    }

    EXPECT_TRUE(foundevent);

    // Destination account.

    auto& client_b =
        ot_.ClientSession(static_cast<int>(get_index(receiver_session_)));
    client_b.OTX().ContextIdle(receiver_nym_id_, server_id_).get();

    const auto& receiverworkflow = client_b.Workflow();
    ot::UnallocatedSet<identifier::Generic> receiverworkflows;
    end = std::time(nullptr) + 60;
    do {
        receiverworkflows = receiverworkflow.List(
            receiver_nym_id_,
            ot::otx::client::PaymentWorkflowType::IncomingCheque,
            ot::otx::client::PaymentWorkflowState::Completed);

        if (receiverworkflows.empty()) { ot::sleep(100ms); }
    } while (receiverworkflows.empty() && std::time(nullptr) < end);

    ASSERT_TRUE(!receiverworkflows.empty());

    command = init(protobuf::RPCCOMMAND_GETACCOUNTACTIVITY);
    command.set_session(receiver_session_);
    command.add_identifier(destination_account_id_->str());
    do {
        response = ot_.RPC(command);

        ASSERT_TRUE(protobuf::Validate(opentxs::LogError(), response));
        EXPECT_EQ(RESPONSE_VERSION, response.version());

        ASSERT_EQ(1, response.status_size());
        auto responseCode = response.status(0).code();
        auto responseIsValid = responseCode == protobuf::RPCRESPONSE_NONE ||
                               responseCode == protobuf::RPCRESPONSE_SUCCESS;
        ASSERT_TRUE(responseIsValid);
        EXPECT_STREQ(command.cookie().c_str(), response.cookie().c_str());
        EXPECT_EQ(command.type(), response.type());
        if (response.accountevent_size() < 1) {
            client_b.OTX().Refresh();
            client_b.OTX().ContextIdle(receiver_nym_id_, server_id_).get();

            command.set_cookie(ot::identifier::Generic::Random()->str());
        }

    } while (protobuf::RPCRESPONSE_NONE == response.status(0).code() ||
             response.accountevent_size() < 1);

    // TODO properly count the number of updates on the appropriate ui widget

    foundevent = false;
    for (const auto& accountevent : response.accountevent()) {
        EXPECT_EQ(ACCOUNTEVENT_VERSION, accountevent.version());
        EXPECT_STREQ(
            destination_account_id_->str().c_str(), accountevent.id().c_str());
        if (protobuf::ACCOUNTEVENT_INCOMINGCHEQUE == accountevent.type()) {
            EXPECT_EQ(100, accountevent.amount());
            foundevent = true;
        }
    }

    EXPECT_TRUE(foundevent);
}

TEST_F(RpcAsync, Accept_2_Pending_Payments)
{
    // Send 1 payment

    auto& client_a =
        ot_.ClientSession(static_cast<int>(get_index(sender_session_)));
    auto command = init(protobuf::RPCCOMMAND_SENDPAYMENT);
    command.set_session(sender_session_);
    auto& client_b =
        ot_.ClientSession(static_cast<int>(get_index(receiver_session_)));

    ASSERT_FALSE(receiver_nym_id_->empty());

    auto nym5 = client_b.Wallet().Nym(receiver_nym_id_);

    ASSERT_TRUE(bool(nym5));

    auto& contacts = client_a.Contacts();
    const auto contact = contacts.NewContact(
        ot::UnallocatedCString(TEST_NYM_5),
        receiver_nym_id_,
        client_a.Factory().PaymentCode(nym5->PaymentCode()));

    ASSERT_TRUE(contact);

    const auto issueraccounts =
        client_a.Storage().AccountsByIssuer(sender_nym_id_);

    ASSERT_TRUE(false == issueraccounts.empty());

    auto issueraccountid = *issueraccounts.cbegin();

    auto sendpayment = command.mutable_sendpayment();

    ASSERT_NE(nullptr, sendpayment);

    sendpayment->set_version(1);
    sendpayment->set_type(protobuf::RPCPAYMENTTYPE_CHEQUE);
    sendpayment->set_contact(contact->ID().asBase58(ot_.Crypto()));
    sendpayment->set_sourceaccount(issueraccountid->str());
    sendpayment->set_memo("Send_Payment_Cheque test");
    sendpayment->set_amount(100);

    protobuf::RPCResponse response;

    auto future = set_push_checker(default_push_callback);
    do {
        response = ot_.RPC(command);

        ASSERT_EQ(1, response.status_size());
        auto responseCode = response.status(0).code();
        auto responseIsValid = responseCode == protobuf::RPCRESPONSE_RETRY ||
                               responseCode == protobuf::RPCRESPONSE_QUEUED;
        ASSERT_TRUE(responseIsValid);
        EXPECT_EQ(RESPONSE_VERSION, response.version());
        ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
        ASSERT_EQ(command.type(), response.type());

        command.set_cookie(ot::identifier::Generic::Random()->str());
    } while (protobuf::RPCRESPONSE_RETRY == response.status(0).code());

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(protobuf::RPCRESPONSE_QUEUED, response.status(0).code());

    ASSERT_EQ(1, response.task_size());
    EXPECT_TRUE(check_push_results(future.get()));

    // Send a second payment.

    command = init(protobuf::RPCCOMMAND_SENDPAYMENT);
    command.set_session(sender_session_);

    sendpayment = command.mutable_sendpayment();

    ASSERT_NE(nullptr, sendpayment);

    sendpayment->set_version(1);
    sendpayment->set_type(protobuf::RPCPAYMENTTYPE_CHEQUE);
    sendpayment->set_contact(contact->ID().asBase58(ot_.Crypto()));
    sendpayment->set_sourceaccount(issueraccountid->str());
    sendpayment->set_memo("Send_Payment_Cheque test");
    sendpayment->set_amount(100);

    future = set_push_checker(default_push_callback);
    do {
        response = ot_.RPC(command);

        ASSERT_EQ(1, response.status_size());
        auto responseCode = response.status(0).code();
        auto responseIsValid = responseCode == protobuf::RPCRESPONSE_RETRY ||
                               responseCode == protobuf::RPCRESPONSE_QUEUED;
        ASSERT_TRUE(responseIsValid);
        EXPECT_EQ(RESPONSE_VERSION, response.version());
        ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
        ASSERT_EQ(command.type(), response.type());

        command.set_cookie(ot::identifier::Generic::Random()->str());
    } while (protobuf::RPCRESPONSE_RETRY == response.status(0).code());

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(protobuf::RPCRESPONSE_QUEUED, response.status(0).code());

    ASSERT_EQ(1, response.task_size());
    EXPECT_TRUE(check_push_results(future.get()));

    // Execute RPCCOMMAND_GETPENDINGPAYMENTS.

    // Make sure the workflows on the client are up-to-date.
    client_b.OTX().Refresh();
    auto future1 =
        client_b.OTX().ContextIdle(receiver_nym_id_, intro_server_id_);
    auto future2 = client_b.OTX().ContextIdle(receiver_nym_id_, server_id_);
    future1.get();
    future2.get();
    const auto& workflow = client_b.Workflow();
    ot::UnallocatedSet<identifier::Generic> workflows;
    auto end = std::time(nullptr) + 60;

    do {
        workflows = workflow.List(
            receiver_nym_id_,
            ot::otx::client::PaymentWorkflowType::IncomingCheque,
            ot::otx::client::PaymentWorkflowState::Conveyed);
    } while (workflows.empty() && std::time(nullptr) < end);

    ASSERT_TRUE(!workflows.empty());

    command = init(protobuf::RPCCOMMAND_GETPENDINGPAYMENTS);

    command.set_session(receiver_session_);
    command.set_owner(receiver_nym_id_->str());

    response = ot_.RPC(command);

    ASSERT_TRUE(protobuf::Validate(opentxs::LogError(), response));
    EXPECT_EQ(RESPONSE_VERSION, response.version());

    ASSERT_EQ(1, response.status_size());
    EXPECT_EQ(protobuf::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    EXPECT_EQ(command.type(), response.type());

    ASSERT_EQ(2, response.accountevent_size());

    const auto& accountevent1 = response.accountevent(0);
    const auto workflow_id_1 = accountevent1.workflow();

    ASSERT_TRUE(!workflow_id_1.empty());

    const auto& accountevent2 = response.accountevent(1);
    const auto workflow_id_2 = accountevent2.workflow();

    ASSERT_TRUE(!workflow_id_2.empty());

    // Execute RPCCOMMAND_ACCEPTPENDINGPAYMENTS
    command = init(protobuf::RPCCOMMAND_ACCEPTPENDINGPAYMENTS);

    command.set_session(receiver_session_);
    auto& acceptpendingpayment = *command.add_acceptpendingpayment();
    acceptpendingpayment.set_version(1);
    acceptpendingpayment.set_destinationaccount(destination_account_id_->str());
    acceptpendingpayment.set_workflow(workflow_id_1);
    auto& acceptpendingpayment2 = *command.add_acceptpendingpayment();
    acceptpendingpayment2.set_version(1);
    acceptpendingpayment2.set_destinationaccount(
        destination_account_id_->str());
    acceptpendingpayment2.set_workflow(workflow_id_2);

    future = set_push_checker(default_push_callback, 2);

    response = ot_.RPC(command);

    ASSERT_TRUE(protobuf::Validate(opentxs::LogError(), response));
    EXPECT_EQ(RESPONSE_VERSION, response.version());

    ASSERT_EQ(2, response.status_size());
    EXPECT_EQ(protobuf::RPCRESPONSE_QUEUED, response.status(0).code());
    EXPECT_EQ(protobuf::RPCRESPONSE_QUEUED, response.status(1).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    EXPECT_EQ(command.type(), response.type());
    ASSERT_EQ(2, response.task_size());
    EXPECT_TRUE(check_push_results(future.get()));
}

TEST_F(RpcAsync, Create_Account)
{
    auto command = init(protobuf::RPCCOMMAND_CREATEACCOUNT);
    command.set_session(sender_session_);

    auto& client_a =
        ot_.ClientSession(static_cast<int>(get_index(sender_session_)));
    auto reason = client_a.Factory().PasswordPrompt(__func__);
    auto nym_id =
        client_a.Wallet().Nym(reason, TEST_NYM_6)->ID().asBase58(ot_.Crypto());

    ASSERT_FALSE(nym_id.empty());

    command.set_owner(nym_id);
    auto& server = ot_.Server(static_cast<int>(get_index(server_)));
    command.set_notary(server.ID().asBase58(ot_.Crypto()));
    command.set_unit(unit_definition_id_->str());
    command.add_identifier(USER_ACCOUNT_LABEL);

    auto future = set_push_checker(default_push_callback);
    auto response = ot_.RPC(command);

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(protobuf::RPCRESPONSE_QUEUED, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());
    ASSERT_EQ(1, response.task_size());
    EXPECT_TRUE(check_push_results(future.get()));
}

TEST_F(RpcAsync, Add_Server_Session_Bad_Argument)
{
    // Start a server on a specific port.
    ArgList args{{OPENTXS_ARG_COMMANDPORT, {"8922"}}};

    auto command = init(protobuf::RPCCOMMAND_ADDSERVERSESSION);
    command.set_session(-1);

    for (auto& arg : args) {
        auto apiarg = command.add_arg();
        apiarg->set_version(APIARG_VERSION);
        apiarg->set_key(arg.first);
        apiarg->add_value(*arg.second.begin());
    }

    auto response = ot_.RPC(command);

    ASSERT_TRUE(protobuf::Validate(opentxs::LogError(), response));
    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(protobuf::RPCRESPONSE_SUCCESS, response.status(0).code());
    ASSERT_EQ(RESPONSE_VERSION, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    // Try to start a second server on the same port.
    command = init(protobuf::RPCCOMMAND_ADDSERVERSESSION);
    command.set_session(-1);

    for (auto& arg : args) {
        auto apiarg = command.add_arg();
        apiarg->set_version(APIARG_VERSION);
        apiarg->set_key(arg.first);
        apiarg->add_value(*arg.second.begin());
    }

    response = ot_.RPC(command);

    ASSERT_TRUE(protobuf::Validate(opentxs::LogError(), response));
    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(
        protobuf::RPCRESPONSE_BAD_SERVER_ARGUMENT, response.status(0).code());
    ASSERT_EQ(RESPONSE_VERSION, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    cleanup();
}
}  // namespace ottest
