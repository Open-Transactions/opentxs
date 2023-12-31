// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/rpc/RpcAsync.hpp"  // IWYU pragma: associated

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

namespace ottest
{
namespace ot = opentxs;

int RpcAsync::sender_session_{0};
int RpcAsync::receiver_session_{0};
ot::identifier::Generic RpcAsync::destination_account_id_{
    ot::identifier::Generic::Factory()};
int RpcAsync::intro_server_{0};
std::unique_ptr<OTZMQListenCallback> RpcAsync::notification_callback_{nullptr};
std::unique_ptr<OTZMQSubscribeSocket> RpcAsync::notification_socket_{nullptr};
ot::identifier::Nym RpcAsync::receiver_nym_id_{ot::identifier::Nym::Factory()};
ot::identifier::Nym RpcAsync::sender_nym_id_{ot::identifier::Nym::Factory()};
int RpcAsync::server_{0};
ot::identifier::UnitDefinition RpcAsync::unit_definition_id_{
    ot::identifier::UnitDefinition::Factory()};
ot::identifier::Generic RpcAsync::workflow_id_{
    ot::identifier::Generic::Factory()};
ot::identifier::Notary RpcAsync::intro_server_id_{
    ot::identifier::Notary::Factory()};
ot::identifier::Notary RpcAsync::server_id_{ot::identifier::Notary::Factory()};
RpcAsync::PushChecker RpcAsync::push_checker_{};
std::promise<ot::UnallocatedVector<bool>> RpcAsync::push_received_{};
ot::UnallocatedVector<bool> RpcAsync::push_results_{};
std::size_t RpcAsync::push_results_count_{0};

RpcAsync::RpcAsync()
{
    if (false == bool(notification_callback_)) {
        notification_callback_.reset(new OTZMQListenCallback(
            ot::network::zeromq::ListenCallback::Factory(
                [](const ot::network::zeromq::Message&& incoming) -> void {
                    process_notification(incoming);
                })));
    }
    if (false == bool(notification_socket_)) {
        notification_socket_.reset(new OTZMQSubscribeSocket(
            ot_.ZMQ().SubscribeSocket(*notification_callback_)));
    }
}

bool RpcAsync::check_push_results(const ot::UnallocatedVector<bool>& results)
{
    return std::all_of(
        results.cbegin(), results.cend(), [](bool result) { return result; });
}

ot::protobuf::RPCCommand RpcAsync::init(
    ot::protobuf::RPCCommandType commandtype)
{
    auto cookie = ot::identifier::Generic::Random()->str();

    ot::protobuf::RPCCommand command;
    command.set_version(COMMAND_VERSION);
    command.set_cookie(cookie);
    command.set_type(commandtype);

    return command;
}

std::future<ot::UnallocatedVector<bool>> RpcAsync::set_push_checker(
    PushChecker func,
    std::size_t count = 1)
{
    push_checker_ = func;
    push_results_count_ = count;
    push_received_ = {};

    return push_received_.get_future();
}

void RpcAsync::cleanup()
{
    notification_socket_->get().Close();
    notification_socket_.reset();
    notification_callback_.reset();
}

std::size_t RpcAsync::get_index(const std::int32_t instance)
{
    return (instance - (instance % 2)) / 2;
}

const ot::api::Session& RpcAsync::get_session(const std::int32_t instance)
{
    auto is_server = instance % 2;

    if (is_server) {
        return OTTestEnvironment::GetOT().Server(
            static_cast<int>(get_index(instance)));
    } else {
        return OTTestEnvironment::GetOT().ClientSession(
            static_cast<int>(get_index(instance)));
    }
}

void RpcAsync::process_notification(
    const ot::network::zeromq::Message&& incoming)
{
    if (1 < incoming.Body().size()) { return; }

    const auto& frame = incoming.Body().at(0);
    const auto rpcpush = ot::protobuf::Factory<protobuf::RPCPush>(frame);

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
            push_received_.set_value(ot::UnallocatedVector<bool>{false});
        } catch (...) {
        }

        push_checker_ = {};
        push_received_ = {};
        push_results_ = {};
    }
}

bool RpcAsync::default_push_callback(const ot::protobuf::RPCPush& push)
{
    if (false == ot::protobuf::Validate(opentxs::LogError(), push)) {
        return false;
    }

    if (ot::protobuf::RPCPUSH_TASK != push.type()) { return false; }

    auto& task = push.taskcomplete();

    if (false == task.result()) { return false; }

    if (ot::protobuf::RPCRESPONSE_SUCCESS != task.code()) { return false; }

    return true;
}

void RpcAsync::setup()
{
    const ot::api::Context& ot = OTTestEnvironment::GetOT();

    auto& intro_server = ot.StartNotarySession(
        ArgList(), static_cast<int>(ot.NotarySessionCount()), true);
    auto& server = ot.StartNotarySession(
        ArgList(), static_cast<int>(ot.NotarySessionCount()), true);
    auto reasonServer = server.Factory().PasswordPrompt(__func__);
    intro_server.SetMintKeySize(OT_MINT_KEY_SIZE_TEST);
    server.SetMintKeySize(OT_MINT_KEY_SIZE_TEST);
    auto server_contract = server.Wallet().Server(server.ID());
    intro_server.Wallet().Server(server_contract->PublicContract());
    server_id_ = ot::identifier::Notary::Factory(server_contract->ID()->str());
    auto intro_server_contract =
        intro_server.Wallet().Server(intro_server.ID());
    intro_server_id_ =
        ot::identifier::Notary::Factory(intro_server_contract->ID()->str());
    auto cookie = ot::identifier::Generic::Random()->str();
    ot::protobuf::RPCCommand command;
    command.set_version(COMMAND_VERSION);
    command.set_cookie(cookie);
    command.set_type(ot::protobuf::RPCCOMMAND_ADDCLIENTSESSION);
    command.set_session(-1);
    auto response = ot.RPC(command);

    ASSERT_TRUE(ot::protobuf::Validate(opentxs::LogError(), response));
    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(ot::protobuf::RPCRESPONSE_SUCCESS, response.status(0).code());

    auto& senderClient =
        ot.Client(static_cast<int>(get_index(response.session())));
    auto reasonS = senderClient.Factory().PasswordPrompt(__func__);

    cookie = ot::identifier::Generic::Random()->str();
    command.set_cookie(cookie);
    command.set_type(ot::protobuf::RPCCOMMAND_ADDCLIENTSESSION);
    command.set_session(-1);
    response = ot.RPC(command);

    ASSERT_TRUE(ot::protobuf::Validate(opentxs::LogError(), response));
    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(ot::protobuf::RPCRESPONSE_SUCCESS, response.status(0).code());

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
        ot::UnitType::Usd,
        reasonS);
    unit_definition_id_ =
        ot::identifier::UnitDefinition::Factory(unit_definition->ID()->str());
    intro_server_ = intro_server.Instance();
    server_ = server.Instance();
    sender_session_ = senderClient.Instance();
    receiver_session_ = receiverClient.Instance();
}

}  // namespace ottest
