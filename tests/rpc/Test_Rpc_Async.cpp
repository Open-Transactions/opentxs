// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"

#include <gtest/gtest.h>

#define COMMAND_VERSION 2
#define RESPONSE_VERSION 2
#define ACCOUNTEVENT_VERSION 2
#define TEST_NYM_4 "testNym4"
#define TEST_NYM_5 "testNym5"
#define TEST_NYM_6 "testNym6"
#define ISSUER_ACCOUNT_LABEL "issuer account"
#define USER_ACCOUNT_LABEL "user account"

using namespace opentxs;

namespace
{

class Test_Rpc_Async : public ::testing::Test
{
public:
    Test_Rpc_Async()
        : ot_{opentxs::OT::App()}
    {
        if (false == bool(notification_callback_)) {
            notification_callback_.reset(new OTZMQListenCallback(
                network::zeromq::ListenCallback::Factory(
                    [](const network::zeromq::Message& incoming) -> void {
                        process_notification(incoming);
                    })));
        }
        if (false == bool(notification_socket_)) {
            notification_socket_.reset(new OTZMQSubscribeSocket(
                ot_.ZMQ().SubscribeSocket(*notification_callback_)));
        }
    }

protected:
    const opentxs::api::Native& ot_;

    static int client_a_;
    static int client_b_;
    static bool create_account_complete_;
    static std::string create_account_task_id_;
    static std::string destination_account_id_;
    static int intro_server_;
    static bool issue_unitdef_complete_;
    static std::string issue_unitdef_task_id_;
    static std::unique_ptr<OTZMQListenCallback> notification_callback_;
    static std::unique_ptr<OTZMQSubscribeSocket> notification_socket_;
    static std::string pending_payment_task_id_;
    static std::string receiver_nym_id_;
    static std::set<std::string> register_nym_task_ids_;
    static std::int32_t register_nyms_count_;
    static bool send_payment_cheque_complete_;
    static std::string send_payment_cheque_task_id_;
    static std::string sender_nym_id_;
    static int server_;
    static std::mutex task_lock_;
    static std::string unit_definition_id_;
    static std::string workflow_id_;

    static void cleanup();
    static std::size_t get_index(const std::int32_t instance);
    static const api::Core& get_session(const std::int32_t instance);
    static const std::set<OTIdentifier> get_accounts(
        const proto::RPCCommand& command);
    static void process_notification(const network::zeromq::Message& incoming);
    static void setup();
    static void verify_nym_and_unitdef();

    proto::RPCCommand init(proto::RPCCommandType commandtype)
    {
        auto cookie = opentxs::Identifier::Random()->str();

        proto::RPCCommand command;
        command.set_version(COMMAND_VERSION);
        command.set_cookie(cookie);
        command.set_type(commandtype);

        return command;
    }
};

int Test_Rpc_Async::client_a_{0};
int Test_Rpc_Async::client_b_{0};
bool Test_Rpc_Async::create_account_complete_{false};
std::string Test_Rpc_Async::create_account_task_id_;
std::string Test_Rpc_Async::destination_account_id_;
int Test_Rpc_Async::intro_server_{0};
bool Test_Rpc_Async::issue_unitdef_complete_{false};
std::string Test_Rpc_Async::issue_unitdef_task_id_;
std::unique_ptr<OTZMQListenCallback> Test_Rpc_Async::notification_callback_{
    nullptr};
std::unique_ptr<OTZMQSubscribeSocket> Test_Rpc_Async::notification_socket_{
    nullptr};
std::string Test_Rpc_Async::pending_payment_task_id_;
std::string Test_Rpc_Async::receiver_nym_id_;
std::int32_t Test_Rpc_Async::register_nyms_count_{0};
std::set<std::string> Test_Rpc_Async::register_nym_task_ids_;
bool Test_Rpc_Async::send_payment_cheque_complete_{false};
std::string Test_Rpc_Async::send_payment_cheque_task_id_;
std::string Test_Rpc_Async::sender_nym_id_;
int Test_Rpc_Async::server_{0};
std::mutex Test_Rpc_Async::task_lock_{};
std::string Test_Rpc_Async::unit_definition_id_;
std::string Test_Rpc_Async::workflow_id_;

void Test_Rpc_Async::cleanup()
{
    notification_socket_.reset();
    notification_callback_.reset();

    Log::Sleep(std::chrono::seconds(2));
}

std::size_t Test_Rpc_Async::get_index(const std::int32_t instance)
{
    return (instance - (instance % 2)) / 2;
};

const api::Core& Test_Rpc_Async::get_session(const std::int32_t instance)
{
    auto is_server = instance % 2;

    if (is_server) {
        return opentxs::OT::App().Server(get_index(instance));
    } else {
        return opentxs::OT::App().Client(get_index(instance));
    }
};

void Test_Rpc_Async::process_notification(
    const network::zeromq::Message& incoming)
{
    ASSERT_EQ(2, incoming.Body().size());

    const std::string taskid = incoming.Body().at(0);
    const auto& frame = incoming.Body().at(1);
    const auto data = Data::Factory(frame.data(), frame.size());
    const auto completed = static_cast<bool>(data->at(0));
    Lock lock(task_lock_);

    if (0 != register_nym_task_ids_.count(taskid)) {
        ++register_nyms_count_;
    } else if (taskid == issue_unitdef_task_id_) {
        issue_unitdef_complete_ = true;
    } else if (taskid == send_payment_cheque_task_id_) {
        send_payment_cheque_complete_ = true;
    } else if (taskid == create_account_task_id_) {
        create_account_complete_ = true;
    }
};

void Test_Rpc_Async::setup()
{
    const api::Native& ot = opentxs::OT::App();

    auto& intro_server = ot.StartServer(ArgList(), ot.Servers(), true);

    auto& server = ot.StartServer(ArgList(), ot.Servers(), true);

    auto server_contract = server.Wallet().Server(server.ID());
    intro_server.Wallet().Server(server_contract->PublicContract());

    auto intro_server_contract =
        intro_server.Wallet().Server(intro_server.ID());

    ArgList empty_args;
    auto& client_a = ot.StartClient(empty_args, ot.Clients());
    auto& client_b = ot.StartClient(empty_args, ot.Clients());

    auto client_a_server_contract =
        client_a.Wallet().Server(intro_server_contract->PublicContract());
    client_a.Sync().SetIntroductionServer(*client_a_server_contract);

    auto client_b_server_contract =
        client_b.Wallet().Server(intro_server_contract->PublicContract());
    client_b.Sync().SetIntroductionServer(*client_b_server_contract);

    auto started =
        notification_socket_->get().Start(client_a.Endpoints().TaskComplete());

    ASSERT_TRUE(started);

    started =
        notification_socket_->get().Start(client_b.Endpoints().TaskComplete());

    ASSERT_TRUE(started);

    sender_nym_id_ =
        client_a.Exec().CreateNymHD(proto::CITEMTYPE_INDIVIDUAL, TEST_NYM_4);

    receiver_nym_id_ =
        client_b.Exec().CreateNymHD(proto::CITEMTYPE_INDIVIDUAL, TEST_NYM_5);

    auto unit_definition = client_a.Wallet().UnitDefinition(
        sender_nym_id_,
        "gdollar",
        "GoogleTestDollar",
        "G",
        "Google Test Dollars",
        "GTD",
        2,
        "gcent");

    ASSERT_TRUE(bool(unit_definition));

    unit_definition_id_ = unit_definition->ID()->str();

    intro_server_ = intro_server.Instance();
    server_ = server.Instance();
    client_a_ = client_a.Instance();
    client_b_ = client_b.Instance();
}

void Test_Rpc_Async::verify_nym_and_unitdef()
{
    auto end = std::time(nullptr) + 20;
    while (std::time(nullptr) < end) {
        if (2 == register_nyms_count_ && issue_unitdef_complete_) { break; }

        Log::Sleep(std::chrono::milliseconds(100));
    }

    ASSERT_EQ(2, register_nyms_count_);
    ASSERT_TRUE(issue_unitdef_complete_);
};

TEST_F(Test_Rpc_Async, RegisterNym)
{
    setup();

    // Register the sender nym.
    auto command = init(proto::RPCCOMMAND_REGISTERNYM);
    command.set_session(client_a_);
    command.set_owner(sender_nym_id_);
    auto& server = ot_.Server(get_index(server_));
    command.set_notary(server.ID().str());

    Lock lock(task_lock_);
    auto response = ot_.RPC(command);

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_QUEUED, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());
    ASSERT_TRUE(0 == response.identifier_size());

    ASSERT_EQ(1, response.task_size());
    register_nym_task_ids_.emplace(response.task(0).id());

    // Register the receiver nym.
    command = init(proto::RPCCOMMAND_REGISTERNYM);
    command.set_session(client_b_);

    command.set_owner(receiver_nym_id_);

    command.set_notary(server.ID().str());

    response = ot_.RPC(command);

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_QUEUED, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    ASSERT_TRUE(0 == response.identifier_size());

    ASSERT_EQ(1, response.task_size());
    register_nym_task_ids_.emplace(response.task(0).id());
}

TEST_F(Test_Rpc_Async, Create_Issuer_Account)
{
    auto command = init(proto::RPCCOMMAND_ISSUEUNITDEFINITION);
    command.set_session(client_a_);
    command.set_owner(sender_nym_id_);
    auto& server = ot_.Server(get_index(server_));
    command.set_notary(server.ID().str());
    command.set_unit(unit_definition_id_);
    command.add_identifier(ISSUER_ACCOUNT_LABEL);

    Lock lock(task_lock_);
    auto response = ot_.RPC(command);

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_QUEUED, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    ASSERT_EQ(1, response.task_size());
    issue_unitdef_task_id_ = response.task(0).id();
}

TEST_F(Test_Rpc_Async, Verify_Responses) { verify_nym_and_unitdef(); }

TEST_F(Test_Rpc_Async, Send_Payment_Cheque_No_Path)
{
    auto& client_a = ot_.Client(get_index(client_a_));
    auto command = init(proto::RPCCOMMAND_SENDPAYMENT);
    command.set_session(client_a_);

    const auto issueraccounts = client_a.Storage().AccountsByIssuer(
        Identifier::Factory(sender_nym_id_));

    ASSERT_TRUE(false == issueraccounts.empty());

    auto issueraccountid = *issueraccounts.cbegin();

    const auto contact = client_a.Contacts().NewContact(
        "label_only_contact",
        Identifier::Factory(),
        client_a.Factory().PaymentCode(""));

    auto sendpayment = command.mutable_sendpayment();

    ASSERT_NE(nullptr, sendpayment);

    sendpayment->set_version(1);
    sendpayment->set_type(proto::RPCPAYMENTTYPE_CHEQUE);
    sendpayment->set_contact(contact->ID().str());
    sendpayment->set_sourceaccount(issueraccountid->str());
    sendpayment->set_memo("Send_Payment_Cheque test");
    sendpayment->set_amount(100);

    auto response = ot_.RPC(command);

    EXPECT_EQ(RESPONSE_VERSION, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(
        proto::RPCRESPONSE_NO_PATH_TO_RECIPIENT, response.status(0).code());
}

TEST_F(Test_Rpc_Async, Send_Payment_Cheque)
{
    auto& client_a = ot_.Client(get_index(client_a_));
    auto command = init(proto::RPCCOMMAND_SENDPAYMENT);
    command.set_session(client_a_);
    auto& client_b = ot_.Client(get_index(client_b_));
    auto nym5 = client_b.Wallet().Nym(Identifier::Factory(receiver_nym_id_));

    ASSERT_TRUE(bool(nym5));

    auto& contacts = client_a.Contacts();
    const auto contact = contacts.NewContact(
        std::string(TEST_NYM_5),
        nym5->ID(),
        client_a.Factory().PaymentCode(nym5->PaymentCode()));

    ASSERT_TRUE(contact);

    const auto issueraccounts = client_a.Storage().AccountsByIssuer(
        Identifier::Factory(sender_nym_id_));

    ASSERT_TRUE(false == issueraccounts.empty());

    auto issueraccountid = *issueraccounts.cbegin();
    Lock lock(task_lock_);

    auto sendpayment = command.mutable_sendpayment();

    ASSERT_NE(nullptr, sendpayment);

    sendpayment->set_version(1);
    sendpayment->set_type(proto::RPCPAYMENTTYPE_CHEQUE);
    sendpayment->set_contact(contact->ID().str());
    sendpayment->set_sourceaccount(issueraccountid->str());
    sendpayment->set_memo("Send_Payment_Cheque test");
    sendpayment->set_amount(100);

    proto::RPCResponse response;

    do {
        response = ot_.RPC(command);

        ASSERT_EQ(1, response.status_size());
        auto responseCode = response.status(0).code();
        auto responseIsValid = responseCode == proto::RPCRESPONSE_RETRY ||
                               responseCode == proto::RPCRESPONSE_QUEUED;
        ASSERT_TRUE(responseIsValid);
        EXPECT_EQ(RESPONSE_VERSION, response.version());
        ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
        ASSERT_EQ(command.type(), response.type());

        command.set_cookie(opentxs::Identifier::Random()->str());
    } while (proto::RPCRESPONSE_RETRY == response.status(0).code());

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_QUEUED, response.status(0).code());

    ASSERT_EQ(1, response.task_size());
    send_payment_cheque_task_id_ = response.task(0).id();
    lock.unlock();

    auto end = std::time(nullptr) + 60;

    while (std::time(nullptr) < end) {
        if (send_payment_cheque_complete_) { break; }

        Log::Sleep(std::chrono::milliseconds(100));
    }

    ASSERT_TRUE(send_payment_cheque_complete_);
}

TEST_F(Test_Rpc_Async, Get_Pending_Payments)
{
    auto& client_b = ot_.Client(get_index(client_b_));

    // Make sure the workflows on the client are up-to-date.
    client_b.Sync().Refresh();

    const auto nym5 = Identifier::Factory(receiver_nym_id_);
    const auto& workflow = client_b.Workflow();
    std::set<OTIdentifier> workflows;
    auto end = std::time(nullptr) + 60;
    do {
        workflows = workflow.List(
            nym5,
            proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE,
            proto::PAYMENTWORKFLOWSTATE_CONVEYED);
    } while (workflows.empty() && std::time(nullptr) < end);

    ASSERT_TRUE(!workflows.empty());

    auto command = init(proto::RPCCOMMAND_GETPENDINGPAYMENTS);

    command.set_session(client_b_);
    command.set_owner(receiver_nym_id_);

    auto response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));
    EXPECT_EQ(RESPONSE_VERSION, response.version());

    ASSERT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    EXPECT_EQ(command.type(), response.type());

    EXPECT_EQ(1, response.accountevent_size());

    const auto& accountevent = response.accountevent(0);
    workflow_id_ = accountevent.workflow();

    ASSERT_TRUE(!workflow_id_.empty());
}

TEST_F(Test_Rpc_Async, Create_Compatible_Account)
{
    auto command = init(proto::RPCCOMMAND_CREATECOMPATIBLEACCOUNT);

    command.set_session(client_b_);
    command.set_owner(receiver_nym_id_);
    command.add_identifier(workflow_id_);

    auto response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));
    EXPECT_EQ(RESPONSE_VERSION, response.version());

    ASSERT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    EXPECT_EQ(command.type(), response.type());

    EXPECT_EQ(1, response.identifier_size());

    destination_account_id_ = response.identifier(0);

    ASSERT_TRUE(!destination_account_id_.empty());
}

TEST_F(Test_Rpc_Async, Get_Compatible_Account)
{
    auto command = init(proto::RPCCOMMAND_GETCOMPATIBLEACCOUNTS);

    command.set_session(client_b_);
    command.set_owner(receiver_nym_id_);
    command.add_identifier(workflow_id_);

    auto response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));
    EXPECT_EQ(RESPONSE_VERSION, response.version());

    ASSERT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    EXPECT_EQ(command.type(), response.type());

    EXPECT_EQ(1, response.identifier_size());

    ASSERT_STREQ(
        destination_account_id_.c_str(), response.identifier(0).c_str());
}

TEST_F(Test_Rpc_Async, Accept_Pending_Payments)
{
    auto command = init(proto::RPCCOMMAND_ACCEPTPENDINGPAYMENTS);

    command.set_session(client_b_);
    auto& acceptpendingpayment = *command.add_acceptpendingpayment();
    acceptpendingpayment.set_version(1);
    acceptpendingpayment.set_destinationaccount(destination_account_id_);
    acceptpendingpayment.set_workflow(workflow_id_);

    auto response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));
    EXPECT_EQ(RESPONSE_VERSION, response.version());

    ASSERT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_QUEUED, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    EXPECT_EQ(command.type(), response.type());
    EXPECT_EQ(1, response.task_size());

    auto pending_payment_task_id = response.task(0).id();

    ASSERT_TRUE(!pending_payment_task_id.empty());
}

TEST_F(Test_Rpc_Async, Get_Account_Activity)
{
    auto& client_a = ot_.Client(get_index(client_a_));

    const auto nym4 = Identifier::Factory(sender_nym_id_);
    const auto& workflow = client_a.Workflow();
    std::set<OTIdentifier> workflows;
    auto end = std::time(nullptr) + 60;
    do {
        workflows = workflow.List(
            nym4,
            proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE,
            proto::PAYMENTWORKFLOWSTATE_CONVEYED);

        if (workflows.empty()) { Log::Sleep(std::chrono::milliseconds(100)); }
    } while (workflows.empty() && std::time(nullptr) < end);

    ASSERT_TRUE(!workflows.empty());

    const auto issueraccounts = client_a.Storage().AccountsByIssuer(
        Identifier::Factory(sender_nym_id_));

    ASSERT_TRUE(!issueraccounts.empty());
    auto issuer_account_id = *issueraccounts.cbegin();

    auto command = init(proto::RPCCOMMAND_GETACCOUNTACTIVITY);
    command.set_session(client_a_);
    command.add_identifier(issuer_account_id->str());
    auto response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));
    EXPECT_EQ(RESPONSE_VERSION, response.version());

    ASSERT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_NONE, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    EXPECT_EQ(command.type(), response.type());
    EXPECT_EQ(0, response.accountevent_size());

    // TODO properly count the number of updates on the appropriate ui widget
    Log::Sleep(std::chrono::seconds(1));

    command = init(proto::RPCCOMMAND_GETACCOUNTACTIVITY);
    command.set_session(client_a_);
    command.add_identifier(issuer_account_id->str());
    response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));

    ASSERT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    EXPECT_EQ(command.type(), response.type());
    EXPECT_EQ(1, response.accountevent_size());

    const auto& accountevent = response.accountevent(0);
    EXPECT_EQ(ACCOUNTEVENT_VERSION, accountevent.version());
    EXPECT_STREQ(issuer_account_id->str().c_str(), accountevent.id().c_str());
    EXPECT_EQ(proto::ACCOUNTEVENT_OUTGOINGCHEQUE, accountevent.type());
    EXPECT_EQ(-100, accountevent.amount());

    // Destination account.

    auto& client_b = ot_.Client(get_index(client_b_));

    const auto nym5 = Identifier::Factory(receiver_nym_id_);
    const auto& receiverworkflow = client_b.Workflow();
    std::set<OTIdentifier> receiverworkflows;
    end = std::time(nullptr) + 60;
    do {
        receiverworkflows = receiverworkflow.List(
            nym5,
            proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE,
            proto::PAYMENTWORKFLOWSTATE_COMPLETED);

        if (receiverworkflows.empty()) {
            Log::Sleep(std::chrono::milliseconds(100));
        }
    } while (receiverworkflows.empty() && std::time(nullptr) < end);

    ASSERT_TRUE(!receiverworkflows.empty());

    command = init(proto::RPCCOMMAND_GETACCOUNTACTIVITY);
    command.set_session(client_b_);
    command.add_identifier(destination_account_id_);
    response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));
    EXPECT_EQ(RESPONSE_VERSION, response.version());

    ASSERT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_NONE, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    EXPECT_EQ(command.type(), response.type());
    EXPECT_EQ(0, response.accountevent_size());

    // TODO properly count the number of updates on the appropriate ui widget
    Log::Sleep(std::chrono::seconds(1));

    command = init(proto::RPCCOMMAND_GETACCOUNTACTIVITY);
    command.set_session(client_b_);
    command.add_identifier(destination_account_id_);
    response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));

    ASSERT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    EXPECT_EQ(command.type(), response.type());
    EXPECT_EQ(1, response.accountevent_size());

    const auto& accountevent2 = response.accountevent(0);
    EXPECT_EQ(ACCOUNTEVENT_VERSION, accountevent2.version());
    EXPECT_STREQ(destination_account_id_.c_str(), accountevent2.id().c_str());
    EXPECT_EQ(proto::ACCOUNTEVENT_INCOMINGCHEQUE, accountevent2.type());
    EXPECT_EQ(100, accountevent2.amount());
}

TEST_F(Test_Rpc_Async, Accept_2_Pending_Payments)
{
    // Send 1 payment
    send_payment_cheque_complete_ = false;

    auto& client_a = ot_.Client(get_index(client_a_));
    auto command = init(proto::RPCCOMMAND_SENDPAYMENT);
    command.set_session(client_a_);
    auto& client_b = ot_.Client(get_index(client_b_));
    auto nym5 = client_b.Wallet().Nym(Identifier::Factory(receiver_nym_id_));

    ASSERT_TRUE(bool(nym5));

    auto& contacts = client_a.Contacts();
    const auto contact = contacts.NewContact(
        std::string(TEST_NYM_5),
        nym5->ID(),
        client_a.Factory().PaymentCode(nym5->PaymentCode()));

    ASSERT_TRUE(contact);

    const auto issueraccounts = client_a.Storage().AccountsByIssuer(
        Identifier::Factory(sender_nym_id_));

    ASSERT_TRUE(false == issueraccounts.empty());

    auto issueraccountid = *issueraccounts.cbegin();
    Lock lock(task_lock_);

    auto sendpayment = command.mutable_sendpayment();

    ASSERT_NE(nullptr, sendpayment);

    sendpayment->set_version(1);
    sendpayment->set_type(proto::RPCPAYMENTTYPE_CHEQUE);
    sendpayment->set_contact(contact->ID().str());
    sendpayment->set_sourceaccount(issueraccountid->str());
    sendpayment->set_memo("Send_Payment_Cheque test");
    sendpayment->set_amount(100);

    proto::RPCResponse response;

    do {
        response = ot_.RPC(command);

        ASSERT_EQ(1, response.status_size());
        auto responseCode = response.status(0).code();
        auto responseIsValid = responseCode == proto::RPCRESPONSE_RETRY ||
                               responseCode == proto::RPCRESPONSE_QUEUED;
        ASSERT_TRUE(responseIsValid);
        EXPECT_EQ(RESPONSE_VERSION, response.version());
        ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
        ASSERT_EQ(command.type(), response.type());

        command.set_cookie(opentxs::Identifier::Random()->str());
    } while (proto::RPCRESPONSE_RETRY == response.status(0).code());

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_QUEUED, response.status(0).code());

    ASSERT_EQ(1, response.task_size());
    send_payment_cheque_task_id_ = response.task(0).id();
    lock.unlock();

    auto end = std::time(nullptr) + 60;

    while (std::time(nullptr) < end) {
        if (send_payment_cheque_complete_) { break; }

        Log::Sleep(std::chrono::milliseconds(100));
    }

    ASSERT_TRUE(send_payment_cheque_complete_);

    // Send a second payment.
    send_payment_cheque_complete_ = false;

    command = init(proto::RPCCOMMAND_SENDPAYMENT);
    command.set_session(client_a_);
    lock.lock();

    sendpayment = command.mutable_sendpayment();

    ASSERT_NE(nullptr, sendpayment);

    sendpayment->set_version(1);
    sendpayment->set_type(proto::RPCPAYMENTTYPE_CHEQUE);
    sendpayment->set_contact(contact->ID().str());
    sendpayment->set_sourceaccount(issueraccountid->str());
    sendpayment->set_memo("Send_Payment_Cheque test");
    sendpayment->set_amount(100);

    do {
        response = ot_.RPC(command);

        ASSERT_EQ(1, response.status_size());
        auto responseCode = response.status(0).code();
        auto responseIsValid = responseCode == proto::RPCRESPONSE_RETRY ||
                               responseCode == proto::RPCRESPONSE_QUEUED;
        ASSERT_TRUE(responseIsValid);
        EXPECT_EQ(RESPONSE_VERSION, response.version());
        ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
        ASSERT_EQ(command.type(), response.type());

        command.set_cookie(opentxs::Identifier::Random()->str());
    } while (proto::RPCRESPONSE_RETRY == response.status(0).code());

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_QUEUED, response.status(0).code());

    ASSERT_EQ(1, response.task_size());
    send_payment_cheque_task_id_ = response.task(0).id();
    lock.unlock();

    end = std::time(nullptr) + 60;

    while (std::time(nullptr) < end) {
        if (send_payment_cheque_complete_) { break; }

        Log::Sleep(std::chrono::milliseconds(100));
    }

    ASSERT_TRUE(send_payment_cheque_complete_);

    // Execute RPCCOMMAND_GETPENDINGPAYMENTS.

    // Make sure the workflows on the client are up-to-date.
    client_b.Sync().Refresh();

    const auto& workflow = client_b.Workflow();
    std::set<OTIdentifier> workflows;
    end = std::time(nullptr) + 60;
    do {
        workflows = workflow.List(
            nym5->ID(),
            proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE,
            proto::PAYMENTWORKFLOWSTATE_CONVEYED);
    } while (workflows.empty() && std::time(nullptr) < end);

    ASSERT_TRUE(!workflows.empty());

    command = init(proto::RPCCOMMAND_GETPENDINGPAYMENTS);

    command.set_session(client_b_);
    command.set_owner(receiver_nym_id_);

    response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));
    EXPECT_EQ(RESPONSE_VERSION, response.version());

    ASSERT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    EXPECT_EQ(command.type(), response.type());

    EXPECT_EQ(2, response.accountevent_size());

    const auto& accountevent1 = response.accountevent(0);
    const auto workflow_id_1 = accountevent1.workflow();

    ASSERT_TRUE(!workflow_id_1.empty());

    const auto& accountevent2 = response.accountevent(1);
    const auto workflow_id_2 = accountevent2.workflow();

    ASSERT_TRUE(!workflow_id_2.empty());

    // Execute RPCCOMMAND_ACCEPTPENDINGPAYMENTS
    command = init(proto::RPCCOMMAND_ACCEPTPENDINGPAYMENTS);

    command.set_session(client_b_);
    auto& acceptpendingpayment = *command.add_acceptpendingpayment();
    acceptpendingpayment.set_version(1);
    acceptpendingpayment.set_destinationaccount(destination_account_id_);
    acceptpendingpayment.set_workflow(workflow_id_1);
    auto& acceptpendingpayment2 = *command.add_acceptpendingpayment();
    acceptpendingpayment2.set_version(1);
    acceptpendingpayment2.set_destinationaccount(destination_account_id_);
    acceptpendingpayment2.set_workflow(workflow_id_2);

    response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));
    EXPECT_EQ(RESPONSE_VERSION, response.version());

    ASSERT_EQ(2, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_QUEUED, response.status(0).code());
    EXPECT_EQ(proto::RPCRESPONSE_QUEUED, response.status(1).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    EXPECT_EQ(command.type(), response.type());
    EXPECT_EQ(2, response.task_size());

    auto pending_payment_task_id1 = response.task(0).id();
    auto pending_payment_task_id2 = response.task(1).id();

    ASSERT_TRUE(!pending_payment_task_id1.empty());
    ASSERT_TRUE(!pending_payment_task_id2.empty());
}

TEST_F(Test_Rpc_Async, Create_Account)
{
    auto command = init(proto::RPCCOMMAND_CREATEACCOUNT);
    command.set_session(client_a_);

    auto& client_a = ot_.Client(get_index(client_a_));
    auto nym_id =
        client_a.Exec().CreateNymHD(proto::CITEMTYPE_INDIVIDUAL, TEST_NYM_6);

    ASSERT_FALSE(nym_id.empty());

    command.set_owner(nym_id);
    auto& server = ot_.Server(get_index(server_));
    command.set_notary(server.ID().str());
    command.set_unit(unit_definition_id_);
    command.add_identifier(USER_ACCOUNT_LABEL);

    Lock lock(task_lock_);
    auto response = ot_.RPC(command);

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_QUEUED, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    ASSERT_EQ(1, response.task_size());
    create_account_task_id_ = response.task(0).id();
    lock.unlock();

    auto end = std::time(nullptr) + 30;
    while (std::time(nullptr) < end) {
        if (true == create_account_complete_) { break; }

        Log::Sleep(std::chrono::milliseconds(100));
    }

    ASSERT_TRUE(create_account_complete_);

    cleanup();
}

}  // namespace
