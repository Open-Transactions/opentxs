// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"

#include <gtest/gtest.h>

using namespace opentxs;

// TODO 1.14.0
#define REPLY_VERSION 1
#define REQUEST_VERSION 1

namespace
{
bool init_{false};

class Test_Messages : public ::testing::Test
{
public:
    static const opentxs::ArgList args_;
    static const std::string SeedA_;
    static const std::string Alice_;
    static const OTNymID alice_nym_id_;
    static const std::shared_ptr<const ServerContract> server_contract_;

    const opentxs::api::client::Manager& client_;
    const opentxs::api::server::Manager& server_;
    const Identifier& server_id_;

    Test_Messages()
        : client_(OT::App().StartClient(args_, 0))
        , server_(OT::App().StartServer(args_, 0, true))
        , server_id_(server_.ID())
    {
        if (false == init_) { init(); }
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
        const_cast<std::string&>(SeedA_) = client_.Exec().Wallet_ImportSeed(
            "spike nominee miss inquiry fee nothing belt list other "
            "daughter leave valley twelve gossip paper",
            "");
        const_cast<std::string&>(Alice_) = client_.Exec().CreateNymHD(
            proto::CITEMTYPE_INDIVIDUAL, "Alice", SeedA_, 0);
        const_cast<OTNymID&>(alice_nym_id_) = identifier::Nym::Factory(Alice_);
        const_cast<std::shared_ptr<const ServerContract>&>(server_contract_) =
            server_.Wallet().Server(server_id_);

        OT_ASSERT(server_contract_);
        OT_ASSERT(false == server_id_.empty());

        import_server_contract(*server_contract_, client_);

        init_ = true;
    }
};

const opentxs::ArgList Test_Messages::args_{
    {{OPENTXS_ARG_STORAGE_PLUGIN, {"mem"}}}};
const std::string Test_Messages::SeedA_{""};
const std::string Test_Messages::Alice_{""};
const OTNymID Test_Messages::alice_nym_id_{identifier::Nym::Factory()};
const std::shared_ptr<const ServerContract> Test_Messages::server_contract_{
    nullptr};

TEST_F(Test_Messages, activateRequest)
{
    const proto::ServerRequestType type{proto::SERVERREQUEST_ACTIVATE};
    auto requestID = Identifier::Factory();
    const auto alice = client_.Wallet().Nym(alice_nym_id_);

    ASSERT_TRUE(alice);

    auto request = opentxs::otx::Request::Factory(alice, server_id_, type);

    ASSERT_TRUE(request->Nym());
    EXPECT_EQ(alice_nym_id_.get(), request->Nym()->ID());
    EXPECT_EQ(alice_nym_id_.get(), request->Initiator());
    EXPECT_EQ(server_id_, request->Server());
    EXPECT_EQ(type, request->Type());
    EXPECT_EQ(0, request->Number());

    requestID = request->ID();

    EXPECT_FALSE(requestID->empty());
    EXPECT_TRUE(request->Validate());

    auto serialized = request->Contract();

    EXPECT_EQ(REQUEST_VERSION, serialized.version());
    EXPECT_EQ(requestID->str(), serialized.id());
    EXPECT_EQ(type, serialized.type());
    EXPECT_EQ(Alice_, serialized.nym());
    EXPECT_EQ(server_id_.str(), serialized.server());
    EXPECT_EQ(0, serialized.request());
    EXPECT_FALSE(serialized.has_credentials());
    EXPECT_TRUE(serialized.has_signature());
    EXPECT_EQ(proto::SIGROLE_SERVERREQUEST, serialized.signature().role());

    request->SetNumber(1);

    EXPECT_TRUE(request->Validate());
    EXPECT_EQ(1, request->Number());

    serialized = request->Contract();

    EXPECT_EQ(1, serialized.request());

    request->SetIncludeNym(true);

    EXPECT_TRUE(request->Validate());

    serialized = request->Contract();
    EXPECT_TRUE(serialized.has_credentials());

    const auto serverCopy = opentxs::otx::Request::Factory(server_, serialized);

    ASSERT_TRUE(serverCopy->Nym());
    EXPECT_EQ(alice_nym_id_.get(), serverCopy->Nym()->ID());
    EXPECT_EQ(alice_nym_id_.get(), serverCopy->Initiator());
    EXPECT_EQ(server_id_, serverCopy->Server());
    EXPECT_EQ(type, serverCopy->Type());
    EXPECT_EQ(1, serverCopy->Number());
    EXPECT_EQ(requestID.get(), serverCopy->ID());
    EXPECT_TRUE(serverCopy->Validate());
}

TEST_F(Test_Messages, pushReply)
{
    const std::string payload{"TEST PAYLOAD"};
    const proto::ServerReplyType type{proto::SERVERREPLY_PUSH};
    auto replyID = Identifier::Factory();
    const auto server = server_.Wallet().Nym(server_.NymID());

    ASSERT_TRUE(server);

    auto reply = opentxs::otx::Reply::Factory(
        server, alice_nym_id_, server_id_, type, true);

    ASSERT_TRUE(reply->Nym());
    EXPECT_EQ(server_.NymID(), reply->Nym()->ID());
    EXPECT_EQ(alice_nym_id_.get(), reply->Recipient());
    EXPECT_EQ(server_id_, reply->Server());
    EXPECT_EQ(type, reply->Type());
    EXPECT_EQ(0, reply->Number());
    EXPECT_FALSE(reply->Push());
    proto::OTXPush push;
    push.set_version(1);
    push.set_type(proto::OTXPUSH_NYMBOX);
    push.set_item(payload);
    reply->SetPush(push);

    replyID = reply->ID();

    EXPECT_FALSE(replyID->empty());
    EXPECT_TRUE(reply->Validate());

    auto serialized = reply->Contract();

    EXPECT_EQ(REPLY_VERSION, serialized.version());
    EXPECT_EQ(replyID->str(), serialized.id());
    EXPECT_EQ(type, serialized.type());
    EXPECT_EQ(Alice_, serialized.nym());
    EXPECT_EQ(server_id_.str(), serialized.server());
    EXPECT_EQ(0, serialized.request());
    EXPECT_TRUE(serialized.success());
    EXPECT_TRUE(serialized.has_signature());
    EXPECT_EQ(proto::SIGROLE_SERVERREPLY, serialized.signature().role());

    reply->SetNumber(1);

    EXPECT_TRUE(reply->Validate());
    EXPECT_EQ(1, reply->Number());

    serialized = reply->Contract();

    EXPECT_EQ(1, serialized.request());

    ASSERT_TRUE(reply->Push());
    EXPECT_EQ(payload, reply->Push()->item());
    EXPECT_TRUE(reply->Validate());

    serialized = reply->Contract();

    EXPECT_EQ(payload, serialized.push().item());

    const auto aliceCopy = opentxs::otx::Reply::Factory(client_, serialized);

    ASSERT_TRUE(aliceCopy->Nym());
    EXPECT_EQ(server_.NymID(), aliceCopy->Nym()->ID());
    EXPECT_EQ(alice_nym_id_.get(), aliceCopy->Recipient());
    EXPECT_EQ(server_id_, aliceCopy->Server());
    EXPECT_EQ(type, aliceCopy->Type());
    EXPECT_EQ(1, aliceCopy->Number());
    EXPECT_EQ(replyID.get(), aliceCopy->ID());
    ASSERT_TRUE(aliceCopy->Push());
    EXPECT_EQ(payload, aliceCopy->Push()->item());
    EXPECT_TRUE(aliceCopy->Validate());
}
}  // namespace
