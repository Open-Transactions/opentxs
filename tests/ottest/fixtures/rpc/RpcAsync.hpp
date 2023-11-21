// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <APIArgument.pb.h>
#include <AcceptPendingPayment.pb.h>
#include <AccountEvent.pb.h>
#include <PaymentWorkflowEnums.pb.h>
#include <RPCCommand.pb.h>
#include <RPCEnums.pb.h>
#include <RPCPush.pb.h>
#include <RPCResponse.pb.h>
#include <RPCStatus.pb.h>
#include <RPCTask.pb.h>
#include <SendPayment.pb.h>
#include <TaskComplete.pb.h>
#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
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
#include "internal/serialization/protobuf/verify/RPCPush.hpp"
#include "internal/serialization/protobuf/verify/RPCResponse.hpp"
#include "ottest/Basic.hpp"
#include "ottest/fixtures/common/Base.hpp"

#define COMMAND_VERSION 3
#define RESPONSE_VERSION 3
#define ACCOUNTEVENT_VERSION 2
#define APIARG_VERSION 1
#define TEST_NYM_4 "testNym4"
#define TEST_NYM_5 "testNym5"
#define TEST_NYM_6 "testNym6"
#define ISSUER_ACCOUNT_LABEL "issuer account"
#define USER_ACCOUNT_LABEL "user account"

namespace ottest
{
namespace ot = opentxs;

class OPENTXS_EXPORT RpcAsync : virtual public Base
{
public:
    using PushChecker = std::function<bool(const ot::proto::RPCPush&)>;

    RpcAsync();

protected:
    static int sender_session_;
    static int receiver_session_;
    static ot::identifier::Generic destination_account_id_;
    static int intro_server_;
    static std::unique_ptr<OTZMQListenCallback> notification_callback_;
    static std::unique_ptr<OTZMQSubscribeSocket> notification_socket_;
    static ot::identifier::Nym receiver_nym_id_;
    static ot::identifier::Nym sender_nym_id_;
    static int server_;
    static ot::identifier::UnitDefinition unit_definition_id_;
    static ot::identifier::Generic workflow_id_;
    static ot::identifier::Notary intro_server_id_;
    static ot::identifier::Notary server_id_;

    static bool check_push_results(const ot::UnallocatedVector<bool>& results);
    static void cleanup();
    static std::size_t get_index(const std::int32_t instance);
    static const ot::api::Session& get_session(const std::int32_t instance);
    static void process_notification(
        const ot::network::zeromq::Message&& incoming);
    static bool default_push_callback(const ot::proto::RPCPush& push);
    static void setup();

    ot::proto::RPCCommand init(ot::proto::RPCCommandType commandtype);
    std::future<ot::UnallocatedVector<bool>> set_push_checker(
        PushChecker func,
        std::size_t count = 1);

private:
    static PushChecker push_checker_;
    static std::promise<ot::UnallocatedVector<bool>> push_received_;
    static ot::UnallocatedVector<bool> push_results_;
    static std::size_t push_results_count_;
};
}  // namespace ottest
