// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <functional>
#include <future>
#include <iosfwd>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "Basic.hpp"
#include "internal/protobuf/Check.hpp"
#include "internal/protobuf/verify/RPCPush.hpp"
#include "internal/protobuf/verify/RPCResponse.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Notary.hpp"
#include "opentxs/api/session/OTX.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/api/session/Workflow.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"
#include "opentxs/otx/client/PaymentWorkflowState.hpp"
#include "opentxs/otx/client/PaymentWorkflowType.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/SharedPimpl.hpp"
#include "serialization/protobuf/APIArgument.pb.h"
#include "serialization/protobuf/AcceptPendingPayment.pb.h"
#include "serialization/protobuf/AccountEvent.pb.h"
#include "serialization/protobuf/PaymentWorkflowEnums.pb.h"
#include "serialization/protobuf/RPCCommand.pb.h"
#include "serialization/protobuf/RPCEnums.pb.h"
#include "serialization/protobuf/RPCPush.pb.h"
#include "serialization/protobuf/RPCResponse.pb.h"
#include "serialization/protobuf/RPCStatus.pb.h"
#include "serialization/protobuf/RPCTask.pb.h"
#include "serialization/protobuf/SendPayment.pb.h"
#include "serialization/protobuf/TaskComplete.pb.h"

#define COMMAND_VERSION 3
#define RESPONSE_VERSION 3
#define ACCOUNTEVENT_VERSION 2
#define APIARG_VERSION 1
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
    using PushChecker = std::function<bool(const ot::proto::RPCPush&)>;

    Test_Rpc_Async()
        : ot_{ot::Context()}
    {
        if (false == bool(notification_callback_)) {
            notification_callback_.reset(new OTZMQListenCallback(
                network::zeromq::ListenCallback::Factory(
                    [](const network::zeromq::Message&& incoming) -> void {
                        process_notification(incoming);
                    })));
        }
        if (false == bool(notification_socket_)) {
            notification_socket_.reset(new OTZMQSubscribeSocket(
                ot_.ZMQ().SubscribeSocket(*notification_callback_)));
        }
    }

protected:
    const ot::api::Context& ot_;

    static int sender_session_;
    static int receiver_session_;
    static OTIdentifier destination_account_id_;
    static int intro_server_;
    static std::unique_ptr<OTZMQListenCallback> notification_callback_;
    static std::unique_ptr<OTZMQSubscribeSocket> notification_socket_;
    static OTNymID receiver_nym_id_;
    static OTNymID sender_nym_id_;
    static int server_;
    static OTUnitID unit_definition_id_;
    static OTIdentifier workflow_id_;
    static ot::OTServerID intro_server_id_;
    static ot::OTServerID server_id_;

    static bool check_push_results(const std::pmr::vector<bool>& results)
    {
        return std::all_of(results.cbegin(), results.cend(), [](bool result) {
            return result;
        });
    }
    static void cleanup();
    static std::size_t get_index(const std::int32_t instance);
    static const api::Session& get_session(const std::int32_t instance);
    static void process_notification(const network::zeromq::Message&& incoming);
    static bool default_push_callback(const ot::proto::RPCPush& push);
    static void setup();

    proto::RPCCommand init(proto::RPCCommandType commandtype)
    {
        auto cookie = ot::Identifier::Random()->str();

        proto::RPCCommand command;
        command.set_version(COMMAND_VERSION);
        command.set_cookie(cookie);
        command.set_type(commandtype);

        return command;
    }

    std::future<std::pmr::vector<bool>> set_push_checker(
        PushChecker func,
        std::size_t count = 1)
    {
        push_checker_ = func;
        push_results_count_ = count;
        push_received_ = {};

        return push_received_.get_future();
    }

private:
    static PushChecker push_checker_;
    static std::promise<std::pmr::vector<bool>> push_received_;
    static std::pmr::vector<bool> push_results_;
    static std::size_t push_results_count_;
};

int Test_Rpc_Async::sender_session_{0};
int Test_Rpc_Async::receiver_session_{0};
OTIdentifier Test_Rpc_Async::destination_account_id_{Identifier::Factory()};
int Test_Rpc_Async::intro_server_{0};
std::unique_ptr<OTZMQListenCallback> Test_Rpc_Async::notification_callback_{
    nullptr};
std::unique_ptr<OTZMQSubscribeSocket> Test_Rpc_Async::notification_socket_{
    nullptr};
OTNymID Test_Rpc_Async::receiver_nym_id_{identifier::Nym::Factory()};
OTNymID Test_Rpc_Async::sender_nym_id_{identifier::Nym::Factory()};
int Test_Rpc_Async::server_{0};
OTUnitID Test_Rpc_Async::unit_definition_id_{
    identifier::UnitDefinition::Factory()};
OTIdentifier Test_Rpc_Async::workflow_id_{Identifier::Factory()};
OTServerID Test_Rpc_Async::intro_server_id_{identifier::Server::Factory()};
OTServerID Test_Rpc_Async::server_id_{identifier::Server::Factory()};
Test_Rpc_Async::PushChecker Test_Rpc_Async::push_checker_{};
std::promise<std::pmr::vector<bool>> Test_Rpc_Async::push_received_{};
std::pmr::vector<bool> Test_Rpc_Async::push_results_{};
std::size_t Test_Rpc_Async::push_results_count_{0};

void Test_Rpc_Async::cleanup()
{
    notification_socket_->get().Close();
    notification_socket_.reset();
    notification_callback_.reset();
}

std::size_t Test_Rpc_Async::get_index(const std::int32_t instance)
{
    return (instance - (instance % 2)) / 2;
}

const api::Session& Test_Rpc_Async::get_session(const std::int32_t instance)
{
    auto is_server = instance % 2;

    if (is_server) {
        return ot::Context().Server(static_cast<int>(get_index(instance)));
    } else {
        return ot::Context().ClientSession(
            static_cast<int>(get_index(instance)));
    }
}

void Test_Rpc_Async::process_notification(
    const network::zeromq::Message&& incoming)
{
    if (1 < incoming.Body().size()) { return; }

    const auto& frame = incoming.Body().at(0);
    const auto rpcpush = proto::Factory<proto::RPCPush>(frame);

    if (push_checker_) {
        push_results_.emplace_back(push_checker_(rpcpush));
        if (push_results_.size() == push_results_count_) {
            push_received_.set_value(push_results_);
            push_checker_ = {};
            push_received_ = {};
            push_results_ = {};
        }
    } else {
        try {
            push_received_.set_value(std::pmr::vector<bool>{false});
        } catch (...) {
        }

        push_checker_ = {};
        push_received_ = {};
        push_results_ = {};
    }
}

bool Test_Rpc_Async::default_push_callback(const ot::proto::RPCPush& push)
{
    if (false == proto::Validate(push, VERBOSE)) { return false; }

    if (proto::RPCPUSH_TASK != push.type()) { return false; }

    auto& task = push.taskcomplete();

    if (false == task.result()) { return false; }

    if (proto::RPCRESPONSE_SUCCESS != task.code()) { return false; }

    return true;
}

void Test_Rpc_Async::setup()
{
    const api::Context& ot = ot::Context();

    auto& intro_server = ot.StartNotarySession(
        ArgList(), static_cast<int>(ot.NotarySessionCount()), true);
    auto& server = ot.StartNotarySession(
        ArgList(), static_cast<int>(ot.NotarySessionCount()), true);
    auto reasonServer = server.Factory().PasswordPrompt(__func__);
    intro_server.SetMintKeySize(OT_MINT_KEY_SIZE_TEST);
    server.SetMintKeySize(OT_MINT_KEY_SIZE_TEST);
    auto server_contract = server.Wallet().Server(server.ID());
    intro_server.Wallet().Server(server_contract->PublicContract());
    server_id_ = identifier::Server::Factory(server_contract->ID()->str());
    auto intro_server_contract =
        intro_server.Wallet().Server(intro_server.ID());
    intro_server_id_ =
        identifier::Server::Factory(intro_server_contract->ID()->str());
    auto cookie = ot::Identifier::Random()->str();
    proto::RPCCommand command;
    command.set_version(COMMAND_VERSION);
    command.set_cookie(cookie);
    command.set_type(proto::RPCCOMMAND_ADDCLIENTSESSION);
    command.set_session(-1);
    auto response = ot.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));
    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());

    auto& senderClient =
        ot.Client(static_cast<int>(get_index(response.session())));
    auto reasonS = senderClient.Factory().PasswordPrompt(__func__);

    cookie = ot::Identifier::Random()->str();
    command.set_cookie(cookie);
    command.set_type(proto::RPCCOMMAND_ADDCLIENTSESSION);
    command.set_session(-1);
    response = ot.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));
    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());

    auto& receiverClient =
        ot.Client(static_cast<int>(get_index(response.session())));
    auto reasonR = receiverClient.Factory().PasswordPrompt(__func__);

    auto client_a_server_contract =
        senderClient.Wallet().Server(intro_server_contract->PublicContract());
    senderClient.OTX().SetIntroductionServer(client_a_server_contract);

    auto client_b_server_contract =
        receiverClient.Wallet().Server(intro_server_contract->PublicContract());
    receiverClient.OTX().SetIntroductionServer(client_b_server_contract);

    auto started = notification_socket_->get().Start(
        ot.ZMQ().BuildEndpoint("rpc/push", -1, 1));

    ASSERT_TRUE(started);

    sender_nym_id_ = senderClient.Wallet().Nym(reasonS, TEST_NYM_4)->ID();

    receiver_nym_id_ = receiverClient.Wallet().Nym(reasonR, TEST_NYM_5)->ID();

    auto unit_definition = senderClient.Wallet().UnitDefinition(
        sender_nym_id_->str(),
        "gdollar",
        "GoogleTestDollar",
        "G",
        "Google Test Dollars",
        "GTD",
        2,
        "gcent",
        ot::core::UnitType::USD,
        reasonS);
    unit_definition_id_ =
        identifier::UnitDefinition::Factory(unit_definition->ID()->str());
    intro_server_ = intro_server.Instance();
    server_ = server.Instance();
    sender_session_ = senderClient.Instance();
    receiver_session_ = receiverClient.Instance();
}

TEST_F(Test_Rpc_Async, Setup)
{
    setup();
    ot::Context();
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

TEST_F(Test_Rpc_Async, RegisterNym_Receiver)
{
    // Register the receiver nym.
    auto command = init(proto::RPCCOMMAND_REGISTERNYM);
    command.set_session(receiver_session_);
    command.set_owner(receiver_nym_id_->str());
    command.set_notary(server_id_->str());
    auto future = set_push_checker(default_push_callback);
    auto response = ot_.RPC(command);

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_QUEUED, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());
    ASSERT_TRUE(0 == response.identifier_size());
    ASSERT_EQ(1, response.task_size());
    EXPECT_TRUE(check_push_results(future.get()));
}

TEST_F(Test_Rpc_Async, Create_Issuer_Account)
{
    auto command = init(proto::RPCCOMMAND_ISSUEUNITDEFINITION);
    command.set_session(sender_session_);
    command.set_owner(sender_nym_id_->str());
    auto& server = ot_.Server(static_cast<int>(get_index(server_)));
    command.set_notary(server.ID().str());
    command.set_unit(unit_definition_id_->str());
    command.add_identifier(ISSUER_ACCOUNT_LABEL);
    auto future = set_push_checker(default_push_callback);
    auto response = ot_.RPC(command);

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_QUEUED, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());
    ASSERT_EQ(1, response.task_size());
    EXPECT_TRUE(check_push_results(future.get()));
}

TEST_F(Test_Rpc_Async, Send_Payment_Cheque_No_Contact)
{
    auto& client_a = get_session(sender_session_);
    auto command = init(proto::RPCCOMMAND_SENDPAYMENT);
    command.set_session(sender_session_);

    const auto issueraccounts =
        client_a.Storage().AccountsByIssuer(sender_nym_id_);

    ASSERT_TRUE(false == issueraccounts.empty());

    auto issueraccountid = *issueraccounts.cbegin();

    auto sendpayment = command.mutable_sendpayment();

    ASSERT_NE(nullptr, sendpayment);

    sendpayment->set_version(1);
    sendpayment->set_type(proto::RPCPAYMENTTYPE_CHEQUE);
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
    ASSERT_EQ(proto::RPCRESPONSE_CONTACT_NOT_FOUND, response.status(0).code());
}

TEST_F(Test_Rpc_Async, Send_Payment_Cheque_No_Account_Owner)
{
    auto& client_a =
        ot_.ClientSession(static_cast<int>(get_index(sender_session_)));
    auto command = init(proto::RPCCOMMAND_SENDPAYMENT);
    command.set_session(sender_session_);

    const auto issueraccounts =
        client_a.Storage().AccountsByIssuer(sender_nym_id_);

    ASSERT_TRUE(false == issueraccounts.empty());

    auto issueraccountid = *issueraccounts.cbegin();

    const auto contact = client_a.Contacts().NewContact(
        "label_only_contact",
        identifier::Nym::Factory(),
        client_a.Factory().PaymentCode(std::string{}));

    auto sendpayment = command.mutable_sendpayment();

    ASSERT_NE(nullptr, sendpayment);

    sendpayment->set_version(1);
    sendpayment->set_type(proto::RPCPAYMENTTYPE_CHEQUE);
    sendpayment->set_contact(contact->ID().str());
    sendpayment->set_sourceaccount(receiver_nym_id_->str());
    sendpayment->set_memo("Send_Payment_Cheque test");
    sendpayment->set_amount(100);

    auto response = ot_.RPC(command);

    EXPECT_EQ(RESPONSE_VERSION, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(
        proto::RPCRESPONSE_ACCOUNT_OWNER_NOT_FOUND, response.status(0).code());
}

TEST_F(Test_Rpc_Async, Send_Payment_Cheque_No_Path)
{
    auto& client_a =
        ot_.ClientSession(static_cast<int>(get_index(sender_session_)));
    auto command = init(proto::RPCCOMMAND_SENDPAYMENT);
    command.set_session(sender_session_);

    const auto issueraccounts =
        client_a.Storage().AccountsByIssuer(sender_nym_id_);

    ASSERT_TRUE(false == issueraccounts.empty());

    auto issueraccountid = *issueraccounts.cbegin();

    const auto contact = client_a.Contacts().NewContact(
        "label_only_contact",
        identifier::Nym::Factory(),
        client_a.Factory().PaymentCode(std::string{}));

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
    auto& client_a =
        ot_.ClientSession(static_cast<int>(get_index(sender_session_)));
    auto command = init(proto::RPCCOMMAND_SENDPAYMENT);
    command.set_session(sender_session_);
    auto& client_b = get_session(receiver_session_);

    ASSERT_FALSE(receiver_nym_id_->empty());

    auto nym5 = client_b.Wallet().Nym(receiver_nym_id_);

    ASSERT_TRUE(bool(nym5));

    auto& contacts = client_a.Contacts();
    const auto contact = contacts.NewContact(
        std::string(TEST_NYM_5),
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
    sendpayment->set_type(proto::RPCPAYMENTTYPE_CHEQUE);
    sendpayment->set_contact(contact->ID().str());
    sendpayment->set_sourceaccount(issueraccountid->str());
    sendpayment->set_memo("Send_Payment_Cheque test");
    sendpayment->set_amount(100);

    proto::RPCResponse response;

    auto future = set_push_checker(default_push_callback);
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

        if (responseCode == proto::RPCRESPONSE_RETRY) {
            client_a.OTX().ContextIdle(sender_nym_id_, server_id_).get();
            command.set_cookie(ot::Identifier::Random()->str());
        }
    } while (proto::RPCRESPONSE_RETRY == response.status(0).code());

    client_a.OTX().Refresh();
    client_a.OTX().ContextIdle(sender_nym_id_, server_id_).get();
    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_QUEUED, response.status(0).code());
    EXPECT_EQ(1, response.task_size());
    EXPECT_TRUE(check_push_results(future.get()));
}

TEST_F(Test_Rpc_Async, Get_Pending_Payments)
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
    std::pmr::set<OTIdentifier> workflows;
    auto end = std::time(nullptr) + 60;
    do {
        workflows = workflow.List(
            receiver_nym_id_,
            otx::client::PaymentWorkflowType::IncomingCheque,
            otx::client::PaymentWorkflowState::Conveyed);
    } while (workflows.empty() && std::time(nullptr) < end);

    ASSERT_TRUE(!workflows.empty());

    auto command = init(proto::RPCCOMMAND_GETPENDINGPAYMENTS);

    command.set_session(receiver_session_);
    command.set_owner(receiver_nym_id_->str());

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
    workflow_id_ = Identifier::Factory(accountevent.workflow());

    ASSERT_TRUE(!workflow_id_->empty());
}

TEST_F(Test_Rpc_Async, Create_Compatible_Account)
{
    auto command = init(proto::RPCCOMMAND_CREATECOMPATIBLEACCOUNT);

    command.set_session(receiver_session_);
    command.set_owner(receiver_nym_id_->str());
    command.add_identifier(workflow_id_->str());

    auto response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));
    EXPECT_EQ(RESPONSE_VERSION, response.version());

    ASSERT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    EXPECT_EQ(command.type(), response.type());

    destination_account_id_ =
        Identifier::Factory(response.identifier(0).c_str());

    EXPECT_TRUE(!destination_account_id_->empty());
}

TEST_F(Test_Rpc_Async, Get_Compatible_Account_Bad_Workflow)
{
    auto command = init(proto::RPCCOMMAND_GETCOMPATIBLEACCOUNTS);

    command.set_session(receiver_session_);
    command.set_owner(receiver_nym_id_->str());
    // Use an id that isn't a workflow.
    command.add_identifier(receiver_nym_id_->str());

    auto response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));
    EXPECT_EQ(RESPONSE_VERSION, response.version());

    ASSERT_EQ(response.status_size(), 1);
    EXPECT_EQ(proto::RPCRESPONSE_WORKFLOW_NOT_FOUND, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    EXPECT_EQ(command.type(), response.type());

    EXPECT_EQ(response.identifier_size(), 0);
}

TEST_F(Test_Rpc_Async, Get_Compatible_Account)
{
    auto command = init(proto::RPCCOMMAND_GETCOMPATIBLEACCOUNTS);

    command.set_session(receiver_session_);
    command.set_owner(receiver_nym_id_->str());
    command.add_identifier(workflow_id_->str());

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
        destination_account_id_->str().c_str(), response.identifier(0).c_str());
}

TEST_F(Test_Rpc_Async, Accept_Pending_Payments_Bad_Workflow)
{
    auto command = init(proto::RPCCOMMAND_ACCEPTPENDINGPAYMENTS);

    command.set_session(receiver_session_);
    auto& acceptpendingpayment = *command.add_acceptpendingpayment();
    acceptpendingpayment.set_version(1);
    acceptpendingpayment.set_destinationaccount(destination_account_id_->str());
    // Use an id that isn't a workflow.
    acceptpendingpayment.set_workflow(destination_account_id_->str());

    auto response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));
    EXPECT_EQ(RESPONSE_VERSION, response.version());

    ASSERT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_WORKFLOW_NOT_FOUND, response.status(0).code());
    EXPECT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    EXPECT_EQ(command.type(), response.type());
    EXPECT_EQ(1, response.task_size());

    auto pending_payment_task_id = response.task(0).id();

    ASSERT_TRUE(pending_payment_task_id.empty());
}

TEST_F(Test_Rpc_Async, Accept_Pending_Payments)
{
    auto command = init(proto::RPCCOMMAND_ACCEPTPENDINGPAYMENTS);

    command.set_session(receiver_session_);
    auto& acceptpendingpayment = *command.add_acceptpendingpayment();
    acceptpendingpayment.set_version(1);
    acceptpendingpayment.set_destinationaccount(destination_account_id_->str());
    acceptpendingpayment.set_workflow(workflow_id_->str());

    auto future = set_push_checker(default_push_callback);
    auto response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));
    EXPECT_EQ(RESPONSE_VERSION, response.version());

    ASSERT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_QUEUED, response.status(0).code());
    EXPECT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    EXPECT_EQ(command.type(), response.type());
    EXPECT_EQ(1, response.task_size());
    EXPECT_TRUE(check_push_results(future.get()));
}

TEST_F(Test_Rpc_Async, Get_Account_Activity)
{
    const auto& client =
        ot_.ClientSession(static_cast<int>(get_index(receiver_session_)));
    client.OTX().Refresh();
    client.OTX().ContextIdle(receiver_nym_id_, server_id_).get();

    auto& client_a =
        ot_.ClientSession(static_cast<int>(get_index(sender_session_)));

    const auto& workflow = client_a.Workflow();
    std::pmr::set<OTIdentifier> workflows;
    auto end = std::time(nullptr) + 60;
    do {
        workflows = workflow.List(
            sender_nym_id_,
            otx::client::PaymentWorkflowType::OutgoingCheque,
            otx::client::PaymentWorkflowState::Conveyed);

        if (workflows.empty()) { Sleep(std::chrono::milliseconds(100)); }
    } while (workflows.empty() && std::time(nullptr) < end);

    ASSERT_TRUE(!workflows.empty());

    const auto issueraccounts =
        client_a.Storage().AccountsByIssuer(sender_nym_id_);

    ASSERT_TRUE(!issueraccounts.empty());
    ASSERT_EQ(1, issueraccounts.size());
    auto issuer_account_id = *issueraccounts.cbegin();

    auto command = init(proto::RPCCOMMAND_GETACCOUNTACTIVITY);
    command.set_session(sender_session_);
    command.add_identifier(issuer_account_id->str());
    proto::RPCResponse response;
    do {
        response = ot_.RPC(command);

        ASSERT_TRUE(proto::Validate(response, VERBOSE));
        EXPECT_EQ(RESPONSE_VERSION, response.version());

        ASSERT_EQ(1, response.status_size());
        auto responseCode = response.status(0).code();
        auto responseIsValid = responseCode == proto::RPCRESPONSE_NONE ||
                               responseCode == proto::RPCRESPONSE_SUCCESS;
        ASSERT_TRUE(responseIsValid);
        EXPECT_STREQ(command.cookie().c_str(), response.cookie().c_str());
        EXPECT_EQ(command.type(), response.type());
        if (response.accountevent_size() < 1) {
            client_a.OTX().Refresh();
            client_a.OTX().ContextIdle(sender_nym_id_, server_id_).get();
            command.set_cookie(ot::Identifier::Random()->str());
        }
    } while (proto::RPCRESPONSE_NONE == response.status(0).code() ||
             response.accountevent_size() < 1);

    // TODO properly count the number of updates on the appropriate ui widget

    auto foundevent = false;
    for (const auto& accountevent : response.accountevent()) {
        EXPECT_EQ(ACCOUNTEVENT_VERSION, accountevent.version());
        EXPECT_STREQ(
            issuer_account_id->str().c_str(), accountevent.id().c_str());
        if (proto::ACCOUNTEVENT_OUTGOINGCHEQUE == accountevent.type()) {
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
    std::pmr::set<OTIdentifier> receiverworkflows;
    end = std::time(nullptr) + 60;
    do {
        receiverworkflows = receiverworkflow.List(
            receiver_nym_id_,
            otx::client::PaymentWorkflowType::IncomingCheque,
            otx::client::PaymentWorkflowState::Completed);

        if (receiverworkflows.empty()) {
            Sleep(std::chrono::milliseconds(100));
        }
    } while (receiverworkflows.empty() && std::time(nullptr) < end);

    ASSERT_TRUE(!receiverworkflows.empty());

    command = init(proto::RPCCOMMAND_GETACCOUNTACTIVITY);
    command.set_session(receiver_session_);
    command.add_identifier(destination_account_id_->str());
    do {
        response = ot_.RPC(command);

        ASSERT_TRUE(proto::Validate(response, VERBOSE));
        EXPECT_EQ(RESPONSE_VERSION, response.version());

        ASSERT_EQ(1, response.status_size());
        auto responseCode = response.status(0).code();
        auto responseIsValid = responseCode == proto::RPCRESPONSE_NONE ||
                               responseCode == proto::RPCRESPONSE_SUCCESS;
        ASSERT_TRUE(responseIsValid);
        EXPECT_STREQ(command.cookie().c_str(), response.cookie().c_str());
        EXPECT_EQ(command.type(), response.type());
        if (response.accountevent_size() < 1) {
            client_b.OTX().Refresh();
            client_b.OTX().ContextIdle(receiver_nym_id_, server_id_).get();

            command.set_cookie(ot::Identifier::Random()->str());
        }

    } while (proto::RPCRESPONSE_NONE == response.status(0).code() ||
             response.accountevent_size() < 1);

    // TODO properly count the number of updates on the appropriate ui widget

    foundevent = false;
    for (const auto& accountevent : response.accountevent()) {
        EXPECT_EQ(ACCOUNTEVENT_VERSION, accountevent.version());
        EXPECT_STREQ(
            destination_account_id_->str().c_str(), accountevent.id().c_str());
        if (proto::ACCOUNTEVENT_INCOMINGCHEQUE == accountevent.type()) {
            EXPECT_EQ(100, accountevent.amount());
            foundevent = true;
        }
    }

    EXPECT_TRUE(foundevent);
}

TEST_F(Test_Rpc_Async, Accept_2_Pending_Payments)
{
    // Send 1 payment

    auto& client_a =
        ot_.ClientSession(static_cast<int>(get_index(sender_session_)));
    auto command = init(proto::RPCCOMMAND_SENDPAYMENT);
    command.set_session(sender_session_);
    auto& client_b =
        ot_.ClientSession(static_cast<int>(get_index(receiver_session_)));

    ASSERT_FALSE(receiver_nym_id_->empty());

    auto nym5 = client_b.Wallet().Nym(receiver_nym_id_);

    ASSERT_TRUE(bool(nym5));

    auto& contacts = client_a.Contacts();
    const auto contact = contacts.NewContact(
        std::string(TEST_NYM_5),
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
    sendpayment->set_type(proto::RPCPAYMENTTYPE_CHEQUE);
    sendpayment->set_contact(contact->ID().str());
    sendpayment->set_sourceaccount(issueraccountid->str());
    sendpayment->set_memo("Send_Payment_Cheque test");
    sendpayment->set_amount(100);

    proto::RPCResponse response;

    auto future = set_push_checker(default_push_callback);
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

        command.set_cookie(ot::Identifier::Random()->str());
    } while (proto::RPCRESPONSE_RETRY == response.status(0).code());

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_QUEUED, response.status(0).code());

    ASSERT_EQ(1, response.task_size());
    EXPECT_TRUE(check_push_results(future.get()));

    // Send a second payment.

    command = init(proto::RPCCOMMAND_SENDPAYMENT);
    command.set_session(sender_session_);

    sendpayment = command.mutable_sendpayment();

    ASSERT_NE(nullptr, sendpayment);

    sendpayment->set_version(1);
    sendpayment->set_type(proto::RPCPAYMENTTYPE_CHEQUE);
    sendpayment->set_contact(contact->ID().str());
    sendpayment->set_sourceaccount(issueraccountid->str());
    sendpayment->set_memo("Send_Payment_Cheque test");
    sendpayment->set_amount(100);

    future = set_push_checker(default_push_callback);
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

        command.set_cookie(ot::Identifier::Random()->str());
    } while (proto::RPCRESPONSE_RETRY == response.status(0).code());

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_QUEUED, response.status(0).code());

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
    std::pmr::set<OTIdentifier> workflows;
    auto end = std::time(nullptr) + 60;

    do {
        workflows = workflow.List(
            receiver_nym_id_,
            otx::client::PaymentWorkflowType::IncomingCheque,
            otx::client::PaymentWorkflowState::Conveyed);
    } while (workflows.empty() && std::time(nullptr) < end);

    ASSERT_TRUE(!workflows.empty());

    command = init(proto::RPCCOMMAND_GETPENDINGPAYMENTS);

    command.set_session(receiver_session_);
    command.set_owner(receiver_nym_id_->str());

    response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));
    EXPECT_EQ(RESPONSE_VERSION, response.version());

    ASSERT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
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
    command = init(proto::RPCCOMMAND_ACCEPTPENDINGPAYMENTS);

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

    ASSERT_TRUE(proto::Validate(response, VERBOSE));
    EXPECT_EQ(RESPONSE_VERSION, response.version());

    ASSERT_EQ(2, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_QUEUED, response.status(0).code());
    EXPECT_EQ(proto::RPCRESPONSE_QUEUED, response.status(1).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    EXPECT_EQ(command.type(), response.type());
    ASSERT_EQ(2, response.task_size());
    EXPECT_TRUE(check_push_results(future.get()));
}

TEST_F(Test_Rpc_Async, Create_Account)
{
    auto command = init(proto::RPCCOMMAND_CREATEACCOUNT);
    command.set_session(sender_session_);

    auto& client_a =
        ot_.ClientSession(static_cast<int>(get_index(sender_session_)));
    auto reason = client_a.Factory().PasswordPrompt(__func__);
    auto nym_id = client_a.Wallet().Nym(reason, TEST_NYM_6)->ID().str();

    ASSERT_FALSE(nym_id.empty());

    command.set_owner(nym_id);
    auto& server = ot_.Server(static_cast<int>(get_index(server_)));
    command.set_notary(server.ID().str());
    command.set_unit(unit_definition_id_->str());
    command.add_identifier(USER_ACCOUNT_LABEL);

    auto future = set_push_checker(default_push_callback);
    auto response = ot_.RPC(command);

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_QUEUED, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());
    ASSERT_EQ(1, response.task_size());
    EXPECT_TRUE(check_push_results(future.get()));
}

TEST_F(Test_Rpc_Async, Add_Server_Session_Bad_Argument)
{
    // Start a server on a specific port.
    ArgList args{{OPENTXS_ARG_COMMANDPORT, {"8922"}}};

    auto command = init(proto::RPCCOMMAND_ADDSERVERSESSION);
    command.set_session(-1);

    for (auto& arg : args) {
        auto apiarg = command.add_arg();
        apiarg->set_version(APIARG_VERSION);
        apiarg->set_key(arg.first);
        apiarg->add_value(*arg.second.begin());
    }

    auto response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));
    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    ASSERT_EQ(RESPONSE_VERSION, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    // Try to start a second server on the same port.
    command = init(proto::RPCCOMMAND_ADDSERVERSESSION);
    command.set_session(-1);

    for (auto& arg : args) {
        auto apiarg = command.add_arg();
        apiarg->set_version(APIARG_VERSION);
        apiarg->set_key(arg.first);
        apiarg->add_value(*arg.second.begin());
    }

    response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));
    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(
        proto::RPCRESPONSE_BAD_SERVER_ARGUMENT, response.status(0).code());
    ASSERT_EQ(RESPONSE_VERSION, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    cleanup();
}
}  // namespace
