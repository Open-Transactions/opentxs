// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <chrono>
#include <cstdint>
#include <iosfwd>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "Basic.hpp"
#include "internal/protobuf/Check.hpp"
#include "internal/protobuf/verify/RPCResponse.hpp"
#include "internal/util/Shared.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/crypto/Config.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Notary.hpp"
#include "opentxs/api/session/OTX.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/api/session/Workflow.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/contact/ContactGroup.hpp"
#include "opentxs/contact/ContactItem.hpp"
#include "opentxs/contact/SectionType.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/otx/client/PaymentWorkflowState.hpp"
#include "opentxs/otx/client/PaymentWorkflowType.hpp"
#include "opentxs/rpc/AccountData.hpp"
#include "opentxs/rpc/AccountEvent.hpp"
#include "opentxs/rpc/AccountEventType.hpp"
#include "opentxs/rpc/AccountType.hpp"
#include "opentxs/rpc/ResponseCode.hpp"
#include "opentxs/rpc/request/SendPayment.hpp"
#include "opentxs/rpc/response/Base.hpp"
#include "opentxs/rpc/response/SendPayment.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/SharedPimpl.hpp"
#include "serialization/protobuf/APIArgument.pb.h"
#include "serialization/protobuf/AddClaim.pb.h"
#include "serialization/protobuf/AddContact.pb.h"
#include "serialization/protobuf/ContactItem.pb.h"
#include "serialization/protobuf/CreateInstrumentDefinition.pb.h"
#include "serialization/protobuf/CreateNym.pb.h"
#include "serialization/protobuf/Enums.pb.h"
#include "serialization/protobuf/GetWorkflow.pb.h"
#include "serialization/protobuf/HDSeed.pb.h"
#include "serialization/protobuf/ModifyAccount.pb.h"
#include "serialization/protobuf/MoveFunds.pb.h"
#include "serialization/protobuf/Nym.pb.h"
#include "serialization/protobuf/PaymentWorkflow.pb.h"
#include "serialization/protobuf/PaymentWorkflowEnums.pb.h"
#include "serialization/protobuf/RPCCommand.pb.h"
#include "serialization/protobuf/RPCEnums.pb.h"
#include "serialization/protobuf/RPCResponse.pb.h"
#include "serialization/protobuf/RPCStatus.pb.h"
#include "serialization/protobuf/ServerContract.pb.h"
#include "serialization/protobuf/SessionData.pb.h"

#define TEST_SEED                                                              \
    "one two three four five six seven eight nine ten eleven twelve"
#define TEST_SEED_PASSPHRASE "seed passphrase"
#define ISSUER_ACCOUNT_LABEL "issuer account"
#define USER_ACCOUNT_LABEL "user account"
#define RENAMED_ACCOUNT_LABEL "renamed"

#define COMMAND_VERSION 3
#define RESPONSE_VERSION 3
#define STATUS_VERSION 2
#define APIARG_VERSION 1
#define CREATENYM_VERSION 2
#define ADDCONTACT_VERSION 1
#define CREATEINSTRUMENTDEFINITION_VERSION 1
#define MOVEFUNDS_VERSION 1
#define GETWORKFLOW_VERSION 1
#define SESSIONDATA_VERSION 1
#define MODIFYACCOUNT_VERSION 1
#define ADDCLAIM_VERSION 2
#define ADDCLAIM_SECTION_VERSION 6
#define CONTACTITEM_VERSION 6

using namespace opentxs;

namespace
{
class Test_Rpc : public ::testing::Test
{
public:
    Test_Rpc()
        : ot_{ot::Context()}
    {
    }

protected:
    const ot::api::Context& ot_;

    static OTUnitID unit_definition_id_;
    static std::string issuer_account_id_;
    static proto::ServerContract server_contract_;
    static proto::ServerContract server2_contract_;
    static proto::ServerContract server3_contract_;
    static std::string server_id_;
    static std::string server2_id_;
    static std::string server3_id_;
    static std::string nym1_id_;
    static std::string nym2_account_id_;
    static std::string nym2_id_;
    static std::string nym3_account1_id_;
    static std::string nym3_id_;
    static std::string nym3_account2_id_;
    static std::string seed_id_;
    static std::string seed2_id_;
    static std::pmr::map<std::string, int> widget_update_counters_;
    static std::string workflow_id_;
    static std::string claim_id_;

    static std::size_t get_index(const std::int32_t instance);
    static const api::Session& get_session(const std::int32_t instance);

    proto::RPCCommand init(proto::RPCCommandType commandtype)
    {
        auto cookie = ot::Identifier::Random()->str();

        proto::RPCCommand command;
        command.set_version(COMMAND_VERSION);
        command.set_cookie(cookie);
        command.set_type(commandtype);

        return command;
    }

    bool add_session(proto::RPCCommandType commandtype, ArgList& args)
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

        EXPECT_TRUE(proto::Validate(response, VERBOSE));

        EXPECT_EQ(1, response.status_size());

        if (proto::RPCCOMMAND_ADDSERVERSESSION == commandtype) {
            if (server2_id_.empty()) {
                auto& manager = Test_Rpc::get_session(response.session());
                auto& servermanager =
                    dynamic_cast<const api::session::Notary&>(manager);
                servermanager.SetMintKeySize(OT_MINT_KEY_SIZE_TEST);
                server2_id_ = servermanager.ID().str();
                auto servercontract =
                    servermanager.Wallet().Server(servermanager.ID());

                // Import the server contract
                auto& client = get_session(0);
                auto& clientmanager =
                    dynamic_cast<const api::session::Client&>(client);
                auto clientservercontract = clientmanager.Wallet().Server(
                    servercontract->PublicContract());
            } else if (server3_id_.empty()) {
                auto& manager = Test_Rpc::get_session(response.session());
                auto& servermanager =
                    dynamic_cast<const api::session::Notary&>(manager);
                servermanager.SetMintKeySize(OT_MINT_KEY_SIZE_TEST);
                server3_id_ = servermanager.ID().str();
                auto servercontract =
                    servermanager.Wallet().Server(servermanager.ID());

                // Import the server contract
                auto& client = get_session(0);
                auto& clientmanager =
                    dynamic_cast<const api::session::Client&>(client);
                auto clientservercontract = clientmanager.Wallet().Server(
                    servercontract->PublicContract());
            }
        }

        return proto::RPCRESPONSE_SUCCESS == response.status(0).code();
    }

    void list(proto::RPCCommandType commandtype, std::int32_t session = -1)
    {
        auto command = init(commandtype);
        command.set_session(session);

        auto response = ot_.RPC(command);

        EXPECT_TRUE(proto::Validate(response, VERBOSE));

        EXPECT_EQ(RESPONSE_VERSION, response.version());
        EXPECT_EQ(command.cookie(), response.cookie());
        EXPECT_EQ(command.type(), response.type());

        EXPECT_EQ(1, response.status_size());
        EXPECT_EQ(proto::RPCRESPONSE_NONE, response.status(0).code());
    }

    void wait_for_state_machine(
        const ot::api::session::Client& api,
        const identifier::Nym& nymID,
        const identifier::Server& serverID)
    {
        auto task = api.OTX().DownloadNym(nymID, serverID, nymID);
        ThreadStatus status{ThreadStatus::RUNNING};

        while (0 == std::get<0>(task)) {
            Sleep(std::chrono::milliseconds(100));
            task = api.OTX().DownloadNym(nymID, serverID, nymID);
        }

        while (ThreadStatus::RUNNING == status) {
            Sleep(std::chrono::milliseconds(10));
            status = api.OTX().Status(std::get<0>(task));
        }

        EXPECT_TRUE(ThreadStatus::FINISHED_SUCCESS == status);
    }

    void receive_payment(
        const ot::api::session::Client& api,
        const identifier::Nym& nymID,
        const identifier::Server& serverID,
        const Identifier& accountID)
    {
        auto task = api.OTX().ProcessInbox(nymID, serverID, accountID);
        ThreadStatus status{ThreadStatus::RUNNING};

        while (0 == std::get<0>(task)) {
            Sleep(std::chrono::milliseconds(100));
            task = api.OTX().ProcessInbox(nymID, serverID, accountID);
        }

        while (ThreadStatus::RUNNING == status) {
            Sleep(std::chrono::milliseconds(10));
            status = api.OTX().Status(std::get<0>(task));
        }

        EXPECT_TRUE(ThreadStatus::FINISHED_SUCCESS == status);
    }
};

OTUnitID Test_Rpc::unit_definition_id_{identifier::UnitDefinition::Factory()};
std::string Test_Rpc::issuer_account_id_{};
proto::ServerContract Test_Rpc::server_contract_;
proto::ServerContract Test_Rpc::server2_contract_;
proto::ServerContract Test_Rpc::server3_contract_;
std::string Test_Rpc::server_id_{};
std::string Test_Rpc::server2_id_{};
std::string Test_Rpc::server3_id_{};
std::string Test_Rpc::nym1_id_{};
std::string Test_Rpc::nym2_account_id_{};
std::string Test_Rpc::nym2_id_{};
std::string Test_Rpc::nym3_account1_id_{};
std::string Test_Rpc::nym3_id_{};
std::string Test_Rpc::nym3_account2_id_{};
std::string Test_Rpc::seed_id_{};
std::string Test_Rpc::seed2_id_{};
std::pmr::map<std::string, int> Test_Rpc::widget_update_counters_{};
std::string Test_Rpc::workflow_id_{};
std::string Test_Rpc::claim_id_{};

std::size_t Test_Rpc::get_index(const std::int32_t instance)
{
    return (instance - (instance % 2)) / 2;
}

const api::Session& Test_Rpc::get_session(const std::int32_t instance)
{
    auto is_server = instance % 2;

    if (is_server) {
        return ot::Context().Server(static_cast<int>(get_index(instance)));
    } else {
        return ot::Context().ClientSession(
            static_cast<int>(get_index(instance)));
    }
}

TEST_F(Test_Rpc, List_Client_Sessions_None)
{
    list(proto::RPCCOMMAND_LISTCLIENTSESSIONS);
}

TEST_F(Test_Rpc, List_Server_Sessions_None)
{
    list(proto::RPCCOMMAND_LISTSERVERSESSIONS);
}

// The client created in this test gets used in subsequent tests.
TEST_F(Test_Rpc, Add_Client_Session)
{
    auto command = init(proto::RPCCOMMAND_ADDCLIENTSESSION);
    command.set_session(-1);

    auto response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));

    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());

    EXPECT_EQ(response.session(), 0);
}

TEST_F(Test_Rpc, List_Server_Contracts_None)
{
    list(proto::RPCCOMMAND_LISTSERVERCONTRACTS, 0);
}

TEST_F(Test_Rpc, List_Seeds_None) { list(proto::RPCCOMMAND_LISTHDSEEDS, 0); }

// The server created in this test gets used in subsequent tests.
TEST_F(Test_Rpc, Add_Server_Session)
{
    ArgList args{{OPENTXS_ARG_INPROC, {std::to_string(ot_.Servers() * 2 + 1)}}};

    auto command = init(proto::RPCCOMMAND_ADDSERVERSESSION);

    command.set_session(-1);
    for (auto& arg : args) {
        auto apiarg = command.add_arg();
        apiarg->set_version(APIARG_VERSION);
        apiarg->set_key(arg.first);
        apiarg->add_value(*arg.second.begin());
    }

    auto response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));

    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());

    EXPECT_EQ(1, response.session());

    auto& manager = get_session(response.session());
    auto reason = manager.Factory().PasswordPrompt(__func__);

    // Register the server on the client.
    auto& servermanager = dynamic_cast<const api::session::Notary&>(manager);
    servermanager.SetMintKeySize(OT_MINT_KEY_SIZE_TEST);
    server_id_ = servermanager.ID().str();
    auto servercontract = servermanager.Wallet().Server(servermanager.ID());

    auto& client = get_session(0);
    auto& clientmanager = dynamic_cast<const api::session::Client&>(client);
    auto clientservercontract =
        clientmanager.Wallet().Server(servercontract->PublicContract());

    // Make the server the introduction server.
    clientmanager.OTX().SetIntroductionServer(clientservercontract);
}

TEST_F(Test_Rpc, Get_Server_Password)
{
    auto command = init(proto::RPCCOMMAND_GETSERVERPASSWORD);
    command.set_session(1);
    auto response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());
    EXPECT_EQ(1, response.status_size());

    const auto& status = response.status(0);

    EXPECT_EQ(STATUS_VERSION, status.version());
    EXPECT_EQ(status.index(), 0);
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, status.code());
    EXPECT_EQ(1, response.identifier_size());

    const auto& password = response.identifier(0);

    EXPECT_FALSE(password.empty());
}

TEST_F(Test_Rpc, Get_Admin_Nym_None)
{
    auto command = init(proto::RPCCOMMAND_GETADMINNYM);
    command.set_session(1);
    auto response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());
    EXPECT_EQ(1, response.status_size());

    const auto& status = response.status(0);

    EXPECT_EQ(STATUS_VERSION, status.version());
    EXPECT_EQ(status.index(), 0);
    EXPECT_EQ(proto::RPCRESPONSE_NONE, status.code());
    EXPECT_EQ(response.identifier_size(), 0);
}

TEST_F(Test_Rpc, List_Client_Sessions)
{
    ArgList args;
    auto added = add_session(proto::RPCCOMMAND_ADDCLIENTSESSION, args);
    EXPECT_TRUE(added);

    added = add_session(proto::RPCCOMMAND_ADDCLIENTSESSION, args);
    EXPECT_TRUE(added);

    auto command = init(proto::RPCCOMMAND_LISTCLIENTSESSIONS);
    command.set_session(-1);

    auto response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));

    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());

    EXPECT_EQ(3, response.sessions_size());

    for (auto& session : response.sessions()) {
        EXPECT_EQ(SESSIONDATA_VERSION, session.version());
        EXPECT_TRUE(
            0 == session.instance() || 2 == session.instance() ||
            4 == session.instance());
    }
}

TEST_F(Test_Rpc, List_Server_Sessions)
{
    ArgList args{{OPENTXS_ARG_INPROC, {std::to_string(ot_.Servers() * 2 + 1)}}};

    auto added = add_session(proto::RPCCOMMAND_ADDSERVERSESSION, args);
    EXPECT_TRUE(added);

    args[OPENTXS_ARG_INPROC] = {std::to_string(ot_.Servers() * 2 + 1)};

    added = add_session(proto::RPCCOMMAND_ADDSERVERSESSION, args);
    EXPECT_TRUE(added);

    auto command = init(proto::RPCCOMMAND_LISTSERVERSESSIONS);
    command.set_session(-1);

    auto response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));

    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());

    EXPECT_EQ(3, response.sessions_size());

    for (auto& session : response.sessions()) {
        EXPECT_EQ(SESSIONDATA_VERSION, session.version());
        EXPECT_TRUE(
            1 == session.instance() || 3 == session.instance() ||
            5 == session.instance());
    }
}

TEST_F(Test_Rpc, List_Server_Contracts)
{
    auto command = init(proto::RPCCOMMAND_LISTSERVERCONTRACTS);
    command.set_session(1);

    auto response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));

    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());

    EXPECT_EQ(1, response.identifier_size());
}

TEST_F(Test_Rpc, Get_Notary_Contract)
{
    auto command = init(proto::RPCCOMMAND_GETSERVERCONTRACT);
    command.set_session(0);
    command.add_identifier(server_id_);

    auto response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));

    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());

    EXPECT_EQ(1, response.notary_size());

    server_contract_ = response.notary(0);
}

TEST_F(Test_Rpc, Get_Notary_Contracts)
{
    auto command = init(proto::RPCCOMMAND_GETSERVERCONTRACT);
    command.set_session(0);
    command.add_identifier(server2_id_);
    command.add_identifier(server3_id_);

    auto response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));

    EXPECT_EQ(2, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(1).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());

    EXPECT_EQ(2, response.notary_size());

    server2_contract_ = response.notary(0);
    server3_contract_ = response.notary(1);
}

TEST_F(Test_Rpc, Import_Server_Contract)
{
    auto command = init(proto::RPCCOMMAND_IMPORTSERVERCONTRACT);
    command.set_session(2);
    auto& server = *command.add_server();
    server = server_contract_;

    auto response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));

    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());
}

TEST_F(Test_Rpc, Import_Server_Contracts)
{
    auto command = init(proto::RPCCOMMAND_IMPORTSERVERCONTRACT);
    command.set_session(2);
    auto& server = *command.add_server();
    server = server2_contract_;
    auto& server2 = *command.add_server();
    server2 = server3_contract_;
    auto response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));
    EXPECT_EQ(2, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(1).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());
}

TEST_F(Test_Rpc, Import_Server_Contract_Partial)
{
    auto command = init(proto::RPCCOMMAND_IMPORTSERVERCONTRACT);
    command.set_session(3);
    auto& server = *command.add_server();
    server = server_contract_;
    auto& invalid_server = *command.add_server();
    invalid_server = server_contract_;
    invalid_server.set_nymid("invalid nym identifier");
    auto response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));
    EXPECT_EQ(2, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(proto::RPCRESPONSE_NONE, response.status(1).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());
}

TEST_F(Test_Rpc, List_Contacts_None)
{
    list(proto::RPCCOMMAND_LISTCONTACTS, 0);
}

// The nym created in this test is used in subsequent tests.
TEST_F(Test_Rpc, Create_Nym)
{
    // Add tests for specifying the seedid and index (not -1).
    // Add tests for adding claims.

    auto command = init(proto::RPCCOMMAND_CREATENYM);
    command.set_session(0);

    auto createnym = command.mutable_createnym();

    EXPECT_NE(nullptr, createnym);

    createnym->set_version(CREATENYM_VERSION);
    createnym->set_type(translate(contact::ClaimType::Individual));
    createnym->set_name("testNym1");
    createnym->set_index(-1);

    auto response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));

    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());

    EXPECT_TRUE(0 != response.identifier_size());

    nym1_id_ = response.identifier(0);

    // Now create more nyms for later tests.
    command = init(proto::RPCCOMMAND_CREATENYM);
    command.set_session(0);

    createnym = command.mutable_createnym();

    EXPECT_NE(nullptr, createnym);

    createnym->set_version(CREATENYM_VERSION);
    createnym->set_type(translate(contact::ClaimType::Individual));
    createnym->set_name("testNym2");
    createnym->set_index(-1);

    response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));

    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());

    EXPECT_TRUE(0 != response.identifier_size());

    nym2_id_ = response.identifier(0);

    command = init(proto::RPCCOMMAND_CREATENYM);
    command.set_session(0);

    createnym = command.mutable_createnym();

    EXPECT_NE(nullptr, createnym);

    createnym->set_version(CREATENYM_VERSION);
    createnym->set_type(translate(contact::ClaimType::Individual));
    createnym->set_name("testNym3");
    createnym->set_index(-1);

    response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));

    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());

    EXPECT_TRUE(0 != response.identifier_size());

    nym3_id_ = response.identifier(0);
}

TEST_F(Test_Rpc, List_Contacts)
{
    auto command = init(proto::RPCCOMMAND_LISTCONTACTS);
    command.set_session(0);

    auto response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));

    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());

    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());

    EXPECT_TRUE(3 == response.identifier_size());
}

TEST_F(Test_Rpc, Add_Contact)
{
    // Add a contact using a label.
    auto command = init(proto::RPCCOMMAND_ADDCONTACT);
    command.set_session(0);

    auto& addcontact = *command.add_addcontact();
    addcontact.set_version(ADDCONTACT_VERSION);
    addcontact.set_label("TestContact1");

    auto response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));

    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());

    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());

    EXPECT_EQ(1, response.identifier_size());

    // Add a contact using a nym id.
    auto& client = ot_.ClientSession(0);
    EXPECT_EQ(4, client.Contacts().ContactList().size());

    ot_.ClientSession(2);

    command = init(proto::RPCCOMMAND_ADDCONTACT);

    command.set_session(2);

    auto& addcontact2 = *command.add_addcontact();
    addcontact2.set_version(ADDCONTACT_VERSION);
    addcontact2.set_nymid(nym2_id_);

    response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));

    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());

    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());

    EXPECT_EQ(1, response.identifier_size());

    // Add a contact using a payment code.
    command = init(proto::RPCCOMMAND_ADDCONTACT);

    command.set_session(2);

    auto& addcontact3 = *command.add_addcontact();
    addcontact3.set_version(ADDCONTACT_VERSION);
    addcontact3.set_paymentcode(
        client.Wallet().Nym(identifier::Nym::Factory(nym3_id_))->PaymentCode());

    response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));

    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());

    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());

    EXPECT_EQ(1, response.identifier_size());
}

TEST_F(Test_Rpc, List_Unit_Definitions_None)
{
    list(proto::RPCCOMMAND_LISTUNITDEFINITIONS, 0);
}

TEST_F(Test_Rpc, Create_Unit_Definition)
{
    auto command = init(proto::RPCCOMMAND_CREATEUNITDEFINITION);
    command.set_session(0);
    command.set_owner(nym1_id_);
    auto def = command.mutable_createunit();

    EXPECT_NE(nullptr, def);

    def->set_version(CREATEINSTRUMENTDEFINITION_VERSION);
    def->set_name("GoogleTestDollar");
    def->set_symbol("G");
    def->set_primaryunitname("gdollar");
    def->set_fractionalunitname("gcent");
    def->set_tla("GTD");
    def->set_power(2);
    def->set_terms("Google Test Dollars");
    def->set_unitofaccount(translate(contact::ClaimType::USD));
    auto response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));
    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());
    EXPECT_EQ(1, response.identifier_size());

    const auto& unitID = response.identifier(0);

    EXPECT_TRUE(Identifier::Validate(unitID));

    unit_definition_id_->SetString(unitID);

    EXPECT_FALSE(unit_definition_id_->empty());
}

TEST_F(Test_Rpc, List_Unit_Definitions)
{
    auto command = init(proto::RPCCOMMAND_LISTUNITDEFINITIONS);
    command.set_session(0);
    auto response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));
    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());
    EXPECT_EQ(1, response.identifier_size());

    const auto unitID =
        identifier::UnitDefinition::Factory(response.identifier(0));

    EXPECT_EQ(unit_definition_id_, unitID);
}

TEST_F(Test_Rpc, Add_Claim)
{
    auto command = init(proto::RPCCOMMAND_ADDCLAIM);
    command.set_session(0);

    command.set_owner(nym1_id_);

    auto& addclaim = *command.add_claim();
    addclaim.set_version(ADDCLAIM_VERSION);
    addclaim.set_sectionversion(ADDCLAIM_SECTION_VERSION);
    addclaim.set_sectiontype(proto::CONTACTSECTION_RELATIONSHIP);

    auto& additem = *addclaim.mutable_item();
    additem.set_version(CONTACTITEM_VERSION);
    additem.set_type(proto::CITEMTYPE_ALIAS);
    additem.set_value("RPCCOMMAND_ADDCLAIM");

    auto response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());
    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
}

TEST_F(Test_Rpc, Add_Claim_No_Nym)
{
    auto command = init(proto::RPCCOMMAND_ADDCLAIM);
    command.set_session(0);

    // Use an id that isn't a nym.
    command.set_owner(unit_definition_id_->str());

    auto& addclaim = *command.add_claim();
    addclaim.set_version(ADDCLAIM_VERSION);
    addclaim.set_sectionversion(ADDCLAIM_SECTION_VERSION);
    addclaim.set_sectiontype(proto::CONTACTSECTION_RELATIONSHIP);

    auto& additem = *addclaim.mutable_item();
    additem.set_version(CONTACTITEM_VERSION);
    additem.set_type(proto::CITEMTYPE_ALIAS);
    additem.set_value("RPCCOMMAND_ADDCLAIM");

    auto response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));

    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());

    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_NYM_NOT_FOUND, response.status(0).code());
}

TEST_F(Test_Rpc, Delete_Claim_No_Nym)
{
    auto command = init(proto::RPCCOMMAND_DELETECLAIM);
    command.set_session(0);

    command.set_owner(unit_definition_id_->str());

    auto& client = ot_.ClientSession(0);
    auto nym = client.Wallet().Nym(ot::identifier::Nym::Factory(nym1_id_));
    auto& claims = nym->Claims();
    auto group = claims.Group(
        opentxs::contact::SectionType::Relationship,
        opentxs::contact::ClaimType::Alias);
    const auto claim = group->Best();
    claim_id_ = claim->ID().str();
    command.add_identifier(claim_id_);
    auto response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));

    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());

    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_NYM_NOT_FOUND, response.status(0).code());
}

TEST_F(Test_Rpc, Delete_Claim)
{
    auto command = init(proto::RPCCOMMAND_DELETECLAIM);
    command.set_session(0);
    command.set_owner(nym1_id_);
    command.add_identifier(claim_id_);
    auto response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));

    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());

    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
}

TEST_F(Test_Rpc, RegisterNym)
{
    auto command = init(proto::RPCCOMMAND_REGISTERNYM);
    command.set_session(0);
    command.set_owner(nym1_id_);
    auto& server = ot_.Server(0);
    command.set_notary(server.ID().str());
    auto response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));

    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());

    // Register the other nyms.
    command = init(proto::RPCCOMMAND_REGISTERNYM);
    command.set_session(0);
    command.set_owner(nym2_id_);
    command.set_notary(server.ID().str());
    response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));

    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());

    command = init(proto::RPCCOMMAND_REGISTERNYM);
    command.set_session(0);
    command.set_owner(nym3_id_);
    command.set_notary(server.ID().str());
    response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));

    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());
}

TEST_F(Test_Rpc, RegisterNym_No_Nym)
{
    auto command = init(proto::RPCCOMMAND_REGISTERNYM);
    command.set_session(0);
    // Use an id that isn't a nym.
    command.set_owner(unit_definition_id_->str());
    auto& server = ot_.Server(0);
    command.set_notary(server.ID().str());
    auto response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));
    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_NYM_NOT_FOUND, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());
}

TEST_F(Test_Rpc, Create_Issuer_Account)
{
    auto command = init(proto::RPCCOMMAND_ISSUEUNITDEFINITION);
    command.set_session(0);
    command.set_owner(nym1_id_);
    auto& server = ot_.Server(0);
    command.set_notary(server.ID().str());

    EXPECT_FALSE(unit_definition_id_->empty());

    command.set_unit(unit_definition_id_->str());
    command.add_identifier(ISSUER_ACCOUNT_LABEL);
    auto response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));
    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());
    EXPECT_EQ(1, response.identifier_size());

    issuer_account_id_ = response.identifier(0);

    EXPECT_TRUE(Identifier::Validate(issuer_account_id_));
}

TEST_F(Test_Rpc, Lookup_Account_ID)
{
    auto command = init(proto::RPCCOMMAND_LOOKUPACCOUNTID);
    command.set_session(0);
    command.set_param(ISSUER_ACCOUNT_LABEL);
    auto response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));
    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());
    EXPECT_EQ(1, response.identifier_size());
    EXPECT_STREQ(response.identifier(0).c_str(), issuer_account_id_.c_str());
}

TEST_F(Test_Rpc, Get_Unit_Definition)
{
    auto command = init(proto::RPCCOMMAND_GETUNITDEFINITION);
    command.set_session(0);
    command.add_identifier(unit_definition_id_->str());

    auto response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));

    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());
    EXPECT_EQ(1, response.unit_size());
}

TEST_F(Test_Rpc, Create_Issuer_Account_Unnecessary)
{
    auto command = init(proto::RPCCOMMAND_ISSUEUNITDEFINITION);
    command.set_session(0);
    command.set_owner(nym1_id_);
    auto& server = ot_.Server(0);
    command.set_notary(server.ID().str());

    EXPECT_FALSE(unit_definition_id_->empty());

    command.set_unit(unit_definition_id_->str());
    command.add_identifier(ISSUER_ACCOUNT_LABEL);
    auto response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));

    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_UNNECESSARY, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());
    EXPECT_EQ(response.identifier_size(), 0);
}

TEST_F(Test_Rpc, Create_Account)
{
    auto command = init(proto::RPCCOMMAND_CREATEACCOUNT);
    command.set_session(0);
    command.set_owner(nym2_id_);
    auto& server = ot_.Server(0);
    command.set_notary(server.ID().str());

    EXPECT_FALSE(unit_definition_id_->empty());

    command.set_unit(unit_definition_id_->str());
    command.add_identifier(USER_ACCOUNT_LABEL);
    auto response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));
    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());
    EXPECT_EQ(1, response.identifier_size());

    {
        const auto& accountID = response.identifier(0);

        EXPECT_TRUE(Identifier::Validate(accountID));

        nym2_account_id_ = accountID;
    }

    // Create two accounts for nym 3.
    command = init(proto::RPCCOMMAND_CREATEACCOUNT);
    command.set_session(0);
    command.set_owner(nym3_id_);
    command.set_notary(server.ID().str());
    command.set_unit(unit_definition_id_->str());
    command.add_identifier(USER_ACCOUNT_LABEL);
    response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));

    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());

    EXPECT_EQ(1, response.identifier_size());

    {
        const auto& accountID = response.identifier(0);

        EXPECT_TRUE(Identifier::Validate(accountID));

        nym3_account1_id_ = response.identifier(0);
    }

    command = init(proto::RPCCOMMAND_CREATEACCOUNT);
    command.set_session(0);
    command.set_owner(nym3_id_);
    command.set_notary(server.ID().str());
    command.set_unit(unit_definition_id_->str());
    command.add_identifier(USER_ACCOUNT_LABEL);
    response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));

    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());

    EXPECT_EQ(1, response.identifier_size());

    {
        const auto& accountID = response.identifier(0);

        EXPECT_TRUE(Identifier::Validate(accountID));

        nym3_account2_id_ = response.identifier(0);
    }
}

TEST_F(Test_Rpc, Send_Payment_Transfer)
{
    auto& client = ot_.ClientSession(0);
    auto& contacts = client.Contacts();
    const auto contactid =
        contacts.ContactID(identifier::Nym::Factory(nym3_id_));
    const auto command = ot::rpc::request::SendPayment{
        0,
        issuer_account_id_,
        contactid->str(),
        nym3_account1_id_,
        75,
        "Send_Payment_Transfer test"};
    const auto base = ot_.RPC(command);
    const auto& response = base.asSendPayment();
    const auto& codes = response.ResponseCodes();
    const auto& pending = response.Pending();

    EXPECT_EQ(response.Version(), command.Version());
    EXPECT_EQ(response.Cookie(), command.Cookie());
    EXPECT_EQ(response.Type(), command.Type());
    ASSERT_EQ(codes.size(), 1);
    EXPECT_EQ(codes.at(0).first, 0);
    EXPECT_EQ(codes.at(0).second, ot::rpc::ResponseCode::success);
    ASSERT_EQ(pending.size(), 0);

    const auto nym1id = identifier::Nym::Factory(nym1_id_);
    const auto nym3id = identifier::Nym::Factory(nym3_id_);
    wait_for_state_machine(
        client, nym1id, identifier::Server::Factory(server_id_));

    {
        const auto account =
            client.Wallet().Account(Identifier::Factory(issuer_account_id_));

        EXPECT_TRUE(account);

        EXPECT_EQ(-75, account.get().GetBalance());
    }

    receive_payment(
        client,
        nym3id,
        identifier::Server::Factory(server_id_),
        Identifier::Factory(nym3_account1_id_));

    {
        const auto account =
            client.Wallet().Account(Identifier::Factory(nym3_account1_id_));

        EXPECT_TRUE(account);

        EXPECT_EQ(75, account.get().GetBalance());
    }
}

// TODO: tests for RPCPAYMENTTYPE_VOUCHER, RPCPAYMENTTYPE_INVOICE,
// RPCPAYMENTTYPE_BLIND
TEST_F(Test_Rpc, Move_Funds)
{
    auto command = init(proto::RPCCOMMAND_MOVEFUNDS);
    command.set_session(0);
    const auto& manager = ot_.ClientSession(0);
    auto nym3id = identifier::Nym::Factory(nym3_id_);
    auto movefunds = command.mutable_movefunds();

    EXPECT_NE(nullptr, movefunds);

    movefunds->set_version(MOVEFUNDS_VERSION);
    movefunds->set_type(proto::RPCPAYMENTTYPE_TRANSFER);
    movefunds->set_sourceaccount(nym3_account1_id_);
    movefunds->set_destinationaccount(nym3_account2_id_);
    movefunds->set_memo("Move_Funds test");
    movefunds->set_amount(25);
    auto response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));
    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());

    wait_for_state_machine(
        manager, nym3id, identifier::Server::Factory(server_id_));

    {
        const auto account =
            manager.Wallet().Account(Identifier::Factory(nym3_account1_id_));

        EXPECT_TRUE(account);
        EXPECT_EQ(50, account.get().GetBalance());
    }

    receive_payment(
        manager,
        nym3id,
        identifier::Server::Factory(server_id_),
        Identifier::Factory(nym3_account2_id_));

    {
        const auto account =
            manager.Wallet().Account(Identifier::Factory(nym3_account2_id_));

        EXPECT_TRUE(account);
        EXPECT_EQ(25, account.get().GetBalance());
    }
}

TEST_F(Test_Rpc, Get_Workflow)
{
    auto& client = ot_.ClientSession(0);

    // Make sure the workflows on the client are up-to-date.
    client.OTX().Refresh();

    auto nym3id = identifier::Nym::Factory(nym3_id_);

    const auto& workflow = client.Workflow();
    auto workflows = workflow.List(
        nym3id,
        otx::client::PaymentWorkflowType::InternalTransfer,
        otx::client::PaymentWorkflowState::Completed);

    EXPECT_TRUE(!workflows.empty());

    auto workflowid = *workflows.begin();

    auto command = init(proto::RPCCOMMAND_GETWORKFLOW);

    command.set_session(0);

    auto& getworkflow = *command.add_getworkflow();
    getworkflow.set_version(GETWORKFLOW_VERSION);
    getworkflow.set_nymid(nym3_id_);
    getworkflow.set_workflowid(workflowid->str());

    auto response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));
    EXPECT_EQ(RESPONSE_VERSION, response.version());

    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());

    EXPECT_EQ(1, response.workflow_size());

    const auto& paymentworkflow = response.workflow(0);
    EXPECT_STREQ(workflowid->str().c_str(), paymentworkflow.id().c_str());
    EXPECT_EQ(
        otx::client::PaymentWorkflowType::InternalTransfer,
        translate(paymentworkflow.type()));
    EXPECT_EQ(
        otx::client::PaymentWorkflowState::Completed,
        translate(paymentworkflow.state()));

    workflow_id_ = workflowid->str();
}

TEST_F(Test_Rpc, Get_Compatible_Account_No_Cheque)
{
    auto command = init(proto::RPCCOMMAND_GETCOMPATIBLEACCOUNTS);

    command.set_session(0);
    command.set_owner(nym3_id_);
    // Use a transfer workflow (no cheque).
    command.add_identifier(workflow_id_);

    auto response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));
    EXPECT_EQ(RESPONSE_VERSION, response.version());

    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_CHEQUE_NOT_FOUND, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());

    EXPECT_EQ(response.identifier_size(), 0);
}

TEST_F(Test_Rpc, Rename_Account_Not_Found)
{
    auto command = init(proto::RPCCOMMAND_RENAMEACCOUNT);
    command.set_session(0);
    ot_.ClientSession(0);

    auto& modify = *command.add_modifyaccount();
    modify.set_version(MODIFYACCOUNT_VERSION);
    // Use an id that isn't an account.
    modify.set_accountid(unit_definition_id_->str());
    modify.set_label(RENAMED_ACCOUNT_LABEL);

    auto response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));
    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());

    EXPECT_EQ(proto::RPCRESPONSE_ACCOUNT_NOT_FOUND, response.status(0).code());
}

TEST_F(Test_Rpc, Rename_Accounts)
{
    auto command = init(proto::RPCCOMMAND_RENAMEACCOUNT);
    command.set_session(0);
    ot_.ClientSession(0);
    const std::pmr::vector<std::string> accounts{
        issuer_account_id_,
        nym2_account_id_,
        nym3_account1_id_,
        nym3_account2_id_};

    for (const auto& id : accounts) {
        auto& modify = *command.add_modifyaccount();
        modify.set_version(MODIFYACCOUNT_VERSION);
        modify.set_accountid(id);
        modify.set_label(RENAMED_ACCOUNT_LABEL);
    }

    auto response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));
    EXPECT_EQ(4, response.status_size());

    for (const auto& status : response.status()) {
        EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, status.code());
    }
}

TEST_F(Test_Rpc, Get_Nym)
{
    auto command = init(proto::RPCCOMMAND_GETNYM);
    command.set_session(0);
    command.add_identifier(nym1_id_);

    auto response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));

    EXPECT_EQ(1, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());

    EXPECT_EQ(1, response.nym_size());

    const auto& credentialindex = response.nym(0);
    EXPECT_EQ(identity::Nym::DefaultVersion, credentialindex.version());
    EXPECT_STREQ(nym1_id_.c_str(), credentialindex.nymid().c_str());
    EXPECT_EQ(proto::NYM_PUBLIC, credentialindex.mode());
    EXPECT_EQ(4, credentialindex.revision());
    EXPECT_EQ(1, credentialindex.activecredentials_size());
    EXPECT_EQ(credentialindex.revokedcredentials_size(), 0);
}

TEST_F(Test_Rpc, Get_Nyms)
{
    auto command = init(proto::RPCCOMMAND_GETNYM);
    command.set_session(0);
    command.add_identifier(nym1_id_);
    command.add_identifier(nym2_id_);
    command.add_identifier(nym3_id_);
    // Use an id that isn't a nym.
    command.add_identifier(issuer_account_id_);

    auto response = ot_.RPC(command);

    EXPECT_TRUE(proto::Validate(response, VERBOSE));

    EXPECT_EQ(4, response.status_size());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(1).code());
    EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(2).code());
    EXPECT_EQ(proto::RPCRESPONSE_NYM_NOT_FOUND, response.status(3).code());
    EXPECT_EQ(RESPONSE_VERSION, response.version());
    EXPECT_EQ(command.cookie(), response.cookie());
    EXPECT_EQ(command.type(), response.type());

    EXPECT_EQ(3, response.nym_size());

    auto& credentialindex = response.nym(0);
    EXPECT_EQ(identity::Nym::DefaultVersion, credentialindex.version());
    EXPECT_TRUE(
        nym1_id_ == credentialindex.nymid() ||
        nym2_id_ == credentialindex.nymid() ||
        nym3_id_ == credentialindex.nymid());
    EXPECT_EQ(proto::NYM_PUBLIC, credentialindex.mode());
    EXPECT_EQ(4, credentialindex.revision());
    EXPECT_EQ(1, credentialindex.activecredentials_size());
    EXPECT_EQ(credentialindex.revokedcredentials_size(), 0);
}

TEST_F(Test_Rpc, Import_Seed_Invalid)
{
    if (ot::api::crypto::HaveHDKeys()) {
        auto command = init(proto::RPCCOMMAND_IMPORTHDSEED);
        command.set_session(0);
        auto& seed = *command.mutable_hdseed();
        seed.set_version(1);
        seed.set_words("bad seed words");
        seed.set_passphrase(TEST_SEED_PASSPHRASE);

        auto response = ot_.RPC(command);

        EXPECT_TRUE(proto::Validate(response, VERBOSE));

        EXPECT_EQ(1, response.status_size());
        EXPECT_EQ(proto::RPCRESPONSE_INVALID, response.status(0).code());
        EXPECT_EQ(RESPONSE_VERSION, response.version());
        EXPECT_EQ(command.cookie(), response.cookie());
        EXPECT_EQ(command.type(), response.type());

        EXPECT_EQ(response.identifier_size(), 0);
    } else {
        // TODO
    }
}

TEST_F(Test_Rpc, Import_Seed)
{
    if (ot::api::crypto::HaveHDKeys()) {
        auto command = init(proto::RPCCOMMAND_IMPORTHDSEED);
        command.set_session(0);
        auto& seed = *command.mutable_hdseed();
        seed.set_version(1);
        seed.set_words(TEST_SEED);
        seed.set_passphrase(TEST_SEED_PASSPHRASE);

        auto response = ot_.RPC(command);

        EXPECT_TRUE(proto::Validate(response, VERBOSE));

        EXPECT_EQ(1, response.status_size());
        EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
        EXPECT_EQ(RESPONSE_VERSION, response.version());
        EXPECT_EQ(command.cookie(), response.cookie());
        EXPECT_EQ(command.type(), response.type());

        EXPECT_EQ(1, response.identifier_size());

        seed_id_ = response.identifier(0);
    } else {
        // TODO
    }
}

TEST_F(Test_Rpc, List_Seeds)
{
    if (ot::api::crypto::HaveHDKeys()) {
        auto command = init(proto::RPCCOMMAND_LISTHDSEEDS);
        command.set_session(0);

        auto response = ot_.RPC(command);

        EXPECT_TRUE(proto::Validate(response, VERBOSE));

        EXPECT_EQ(1, response.status_size());
        EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
        EXPECT_EQ(RESPONSE_VERSION, response.version());
        EXPECT_EQ(command.cookie(), response.cookie());
        EXPECT_EQ(command.type(), response.type());

        EXPECT_EQ(2, response.identifier_size());

        if (seed_id_ == response.identifier(0)) {
            seed2_id_ = response.identifier(1);
        } else if (seed_id_ == response.identifier(1)) {
            seed2_id_ = response.identifier(0);
        } else {
            FAIL();
        }
    } else {
        // TODO
    }
}

TEST_F(Test_Rpc, Get_Seed)
{
    if (ot::api::crypto::HaveHDKeys()) {
        auto command = init(proto::RPCCOMMAND_GETHDSEED);
        command.set_session(0);
        command.add_identifier(seed_id_);
        auto response = ot_.RPC(command);

        EXPECT_TRUE(proto::Validate(response, VERBOSE));
        EXPECT_EQ(1, response.status_size());
        EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
        EXPECT_EQ(RESPONSE_VERSION, response.version());
        EXPECT_EQ(command.cookie(), response.cookie());
        EXPECT_EQ(command.type(), response.type());
        EXPECT_EQ(1, response.seed_size());

        auto seed = response.seed(0);

        EXPECT_STREQ(seed_id_.c_str(), seed.id().c_str());
        EXPECT_STREQ(TEST_SEED, seed.words().c_str());
        EXPECT_STREQ(TEST_SEED_PASSPHRASE, seed.passphrase().c_str());
    } else {
        // TODO
    }
}

TEST_F(Test_Rpc, Get_Seeds)
{
    if (ot::api::crypto::HaveHDKeys()) {
        auto command = init(proto::RPCCOMMAND_GETHDSEED);
        command.set_session(0);
        command.add_identifier(seed_id_);
        command.add_identifier(seed2_id_);
        auto response = ot_.RPC(command);

        EXPECT_TRUE(proto::Validate(response, VERBOSE));
        EXPECT_EQ(2, response.status_size());
        EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
        EXPECT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(1).code());
        EXPECT_EQ(RESPONSE_VERSION, response.version());
        EXPECT_EQ(command.cookie(), response.cookie());
        EXPECT_EQ(command.type(), response.type());
        EXPECT_EQ(2, response.seed_size());

        auto seed = response.seed(0);

        if (seed.id() != seed_id_) { seed = response.seed(1); }

        EXPECT_STREQ(seed_id_.c_str(), seed.id().c_str());
        EXPECT_STREQ(TEST_SEED, seed.words().c_str());
        EXPECT_STREQ(TEST_SEED_PASSPHRASE, seed.passphrase().c_str());
    } else {
        // TODO
    }
}

TEST_F(Test_Rpc, Get_Transaction_Data)
{
    if (ot::api::crypto::HaveHDKeys()) {
        auto command = init(proto::RPCCOMMAND_GETTRANSACTIONDATA);
        command.set_session(0);
        command.add_identifier(seed_id_);  // Not a real uuid
        auto response = ot_.RPC(command);

        EXPECT_TRUE(proto::Validate(response, VERBOSE));
        EXPECT_EQ(1, response.status_size());
        EXPECT_EQ(proto::RPCRESPONSE_UNIMPLEMENTED, response.status(0).code());
    } else {
        // TODO
    }
}
}  // namespace
