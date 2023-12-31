// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/rpc/Rpc.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <opentxs/protobuf/APIArgument.pb.h>
#include <opentxs/protobuf/AddClaim.pb.h>
#include <opentxs/protobuf/AddContact.pb.h>
#include <opentxs/protobuf/ContactItem.pb.h>
#include <opentxs/protobuf/CreateInstrumentDefinition.pb.h>
#include <opentxs/protobuf/CreateNym.pb.h>
#include <opentxs/protobuf/Enums.pb.h>
#include <opentxs/protobuf/GetWorkflow.pb.h>
#include <opentxs/protobuf/HDSeed.pb.h>
#include <opentxs/protobuf/ModifyAccount.pb.h>
#include <opentxs/protobuf/MoveFunds.pb.h>
#include <opentxs/protobuf/Nym.pb.h>
#include <opentxs/protobuf/PaymentWorkflow.pb.h>
#include <opentxs/protobuf/PaymentWorkflowEnums.pb.h>
#include <opentxs/protobuf/RPCCommand.pb.h>
#include <opentxs/protobuf/RPCEnums.pb.h>
#include <opentxs/protobuf/RPCResponse.pb.h>
#include <opentxs/protobuf/RPCStatus.pb.h>
#include <opentxs/protobuf/ServerContract.pb.h>
#include <opentxs/protobuf/SessionData.pb.h>
#include <chrono>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <utility>

#include "internal/otx/common/Account.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/util/Shared.hpp"
#include "opentxs/protobuf/syntax/RPCResponse.hpp"
#include "ottest/Basic.hpp"
#include "ottest/fixtures/common/Base.hpp"

namespace ottest
{
namespace ot = opentxs;

ot::identifier::UnitDefinition Rpc::unit_definition_id_{
    ot::identifier::UnitDefinition::Factory()};
ot::UnallocatedCString Rpc::issuer_account_id_{};
ot::protobuf::ServerContract Rpc::server_contract_;
ot::protobuf::ServerContract Rpc::server2_contract_;
ot::protobuf::ServerContract Rpc::server3_contract_;
ot::UnallocatedCString Rpc::server_id_{};
ot::UnallocatedCString Rpc::server2_id_{};
ot::UnallocatedCString Rpc::server3_id_{};
ot::UnallocatedCString Rpc::nym1_id_{};
ot::UnallocatedCString Rpc::nym2_account_id_{};
ot::UnallocatedCString Rpc::nym2_id_{};
ot::UnallocatedCString Rpc::nym3_account1_id_{};
ot::UnallocatedCString Rpc::nym3_id_{};
ot::UnallocatedCString Rpc::nym3_account2_id_{};
ot::UnallocatedCString Rpc::seed_id_{};
ot::UnallocatedCString Rpc::seed2_id_{};
ot::UnallocatedMap<ot::UnallocatedCString, int> Rpc::widget_update_counters_{};
ot::UnallocatedCString Rpc::workflow_id_{};
ot::UnallocatedCString Rpc::claim_id_{};

ot::protobuf::RPCCommand Rpc::init(ot::protobuf::RPCCommandType commandtype)
{
    auto cookie = ot::identifier::Generic::Random()->str();

    ot::protobuf::RPCCommand command;
    command.set_version(COMMAND_VERSION);
    command.set_cookie(cookie);
    command.set_type(commandtype);

    return command;
}

bool Rpc::add_session(ot::protobuf::RPCCommandType commandtype, ArgList& args)
{
    auto command = init(commandtype);
    command.set_session(-1);
    for (auto& arg : args) {
        auto apiarg = command.add_arg();
        apiarg->set_version(APIARG_VERSION);
        apiarg->set_key(arg.first);
        apiarg->add_value(*arg.second.begin());
    }
    auto response = ot_.RPC(command);

    EXPECT_TRUE(ot::protobuf::Validate(opentxs::LogError(), response));

    EXPECT_EQ(1, response.status_size());

    if (ot::protobuf::RPCCOMMAND_ADDSERVERSESSION == commandtype) {
        if (server2_id_.empty()) {
            auto& manager = Rpc::get_session(response.session());
            auto& servermanager =
                dynamic_cast<const ot::api::session::Notary&>(manager);
            servermanager.SetMintKeySize(OT_MINT_KEY_SIZE_TEST);
            server2_id_ = servermanager.ID().asBase58(ot_.Crypto());
            auto servercontract =
                servermanager.Wallet().Server(servermanager.ID());

            // Import the server contract
            auto& client = get_session(0);
            auto& clientmanager =
                dynamic_cast<const ot::api::session::Client&>(client);
            auto clientservercontract =
                clientmanager.Wallet().Server(servercontract->PublicContract());
        } else if (server3_id_.empty()) {
            auto& manager = Rpc::get_session(response.session());
            auto& servermanager =
                dynamic_cast<const ot::api::session::Notary&>(manager);
            servermanager.SetMintKeySize(OT_MINT_KEY_SIZE_TEST);
            server3_id_ = servermanager.ID().asBase58(ot_.Crypto());
            auto servercontract =
                servermanager.Wallet().Server(servermanager.ID());

            // Import the server contract
            auto& client = get_session(0);
            auto& clientmanager =
                dynamic_cast<const ot::api::session::Client&>(client);
            auto clientservercontract =
                clientmanager.Wallet().Server(servercontract->PublicContract());
        }
    }

    return ot::protobuf::RPCRESPONSE_SUCCESS == response.status(0).code();
}

void Rpc::list(
    ot::protobuf::RPCCommandType commandtype,
    std::int32_t session = -1)
{
    auto command = init(commandtype);
    command.set_session(session);

    auto response = ot_.RPC(command);

    EXPECT_TRUE(ot::protobuf::Validate(opentxs::LogError(), response));

    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());

    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(ot::protobuf::RPCRESPONSE_NONE, response.status(0).code());
}

void Rpc::wait_for_state_machine(
    const ot::api::session::Client& api,
    const ot::identifier::Nym& nymID,
    const ot::identifier::Notary& serverID)
{
    auto task = api.OTX().DownloadNym(nymID, serverID, nymID);
    ThreadStatus status{ThreadStatus::RUNNING};

    while (0 == std::get<0>(task)) {
        ot::sleep(100ms);
        task = api.OTX().DownloadNym(nymID, serverID, nymID);
    }

    while (ThreadStatus::RUNNING == status) {
        ot::sleep(10ms);
        status = api.OTX().Status(std::get<0>(task));
    }

    EXPECT_TRUE(ThreadStatus::FINISHED_SUCCESS == status);
}

void Rpc::receive_payment(
    const ot::api::session::Client& api,
    const ot::identifier::Nym& nymID,
    const ot::identifier::Notary& serverID,
    const identifier::Account& accountID)
{
    auto task = api.OTX().ProcessInbox(nymID, serverID, accountID);
    ThreadStatus status{ThreadStatus::RUNNING};

    while (0 == std::get<0>(task)) {
        ot::sleep(100ms);
        task = api.OTX().ProcessInbox(nymID, serverID, accountID);
    }

    while (ThreadStatus::RUNNING == status) {
        ot::sleep(10ms);
        status = api.OTX().Status(std::get<0>(task));
    }

    EXPECT_TRUE(ThreadStatus::FINISHED_SUCCESS == status);
}

std::size_t Rpc::get_index(const std::int32_t instance)
{
    return (instance - (instance % 2)) / 2;
}

const ot::api::Session& Rpc::get_session(const std::int32_t instance)
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

}  // namespace ottest
