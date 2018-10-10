// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"
#include "server/Server.hpp"
#include "server/Transactor.hpp"

#include <gtest/gtest.h>

using namespace opentxs;

#define CHEQUE_AMOUNT 144488
#define CHEQUE_MEMO "memo"
#define FAILURE false
#define INBOX_TYPE 1
#define NYMBOX_TYPE 0
#define NYMBOX_UPDATED true
#define NYMBOX_SAME false
#define NO_TRANSACTION false
#define SUCCESS true
#define TRANSACTION true
#define UNIT_DEFINITION_CONTRACT_NAME "Mt Gox USD"
#define UNIT_DEFINITION_TERMS "YOLO"
#define UNIT_DEFINITION_PRIMARY_UNIT_NAME "dollars"
#define UNIT_DEFINITION_SYMBOL "$"
#define UNIT_DEFINITION_TLA "USA"
#define UNIT_DEFINITION_POWER 2
#define UNIT_DEFINITION_FRACTIONAL_UNIT_NAME "cents"

namespace
{
bool init_{false};

class Test_Basic : public ::testing::Test
{
public:
    static const opentxs::ArgList args_;
    static const std::string SeedA_;
    static const std::string SeedB_;
    static const std::string Alice_;
    static const std::string Bob_;
    static const OTIdentifier alice_nym_id_;
    static const OTIdentifier bob_nym_id_;
    static const std::shared_ptr<const ServerContract> server_contract_;
    static const std::shared_ptr<const UnitDefinition> asset_contract_1_;
    static TransactionNumber cheque_transaction_number_;

    const opentxs::api::client::Manager& client_1_;
    const opentxs::api::client::Manager& client_2_;
    const opentxs::api::server::Manager& server_;
    const Identifier& server_id_;

    Test_Basic()
        : client_1_(OT::App().StartClient(args_, 0))
        , client_2_(OT::App().StartClient(args_, 1))
        , server_(OT::App().StartServer(args_, 0, true))
        , server_id_(server_.ID())
    {
        if (false == init_) { init(); }
    }

    void break_consensus()
    {
        TransactionNumber newNumber{0};
        server_.Server().GetTransactor().issueNextTransactionNumber(newNumber);

        auto context = server_.Wallet().mutable_ClientContext(
            server_.NymID(), alice_nym_id_);
        context.It().IssueNumber(newNumber);
    }

    void import_server_contract(
        const ServerContract& contract,
        const opentxs::api::client::Manager& client)
    {
        auto clientVersion =
            client.Wallet().Server(server_contract_->PublicContract());

        OT_ASSERT(clientVersion)

        client.Sync().SetIntroductionServer(*clientVersion);
    }

    void init()
    {
        client_1_.Sync().DisableAutoaccept();
        client_2_.Sync().DisableAutoaccept();
        const_cast<std::string&>(SeedA_) = client_1_.Exec().Wallet_ImportSeed(
            "spike nominee miss inquiry fee nothing belt list other "
            "daughter leave valley twelve gossip paper",
            "");
        const_cast<std::string&>(SeedB_) = client_2_.Exec().Wallet_ImportSeed(
            "trim thunder unveil reduce crop cradle zone inquiry "
            "anchor skate property fringe obey butter text tank drama "
            "palm guilt pudding laundry stay axis prosper",
            "");
        const_cast<std::string&>(Alice_) = client_1_.Exec().CreateNymHD(
            proto::CITEMTYPE_INDIVIDUAL, "Alice", SeedA_, 0);
        const_cast<std::string&>(Bob_) = client_2_.Exec().CreateNymHD(
            proto::CITEMTYPE_INDIVIDUAL, "Bob", SeedB_, 0);
        const_cast<OTIdentifier&>(alice_nym_id_) = Identifier::Factory(Alice_);
        const_cast<OTIdentifier&>(bob_nym_id_) = Identifier::Factory(Bob_);
        const_cast<std::shared_ptr<const ServerContract>&>(server_contract_) =
            server_.Wallet().Server(server_id_);

        OT_ASSERT(server_contract_);
        OT_ASSERT(false == server_id_.empty());

        import_server_contract(*server_contract_, client_1_);
        import_server_contract(*server_contract_, client_2_);

        init_ = true;
    }

    void create_unit_definition()
    {
        const_cast<std::shared_ptr<const UnitDefinition>&>(asset_contract_1_) =
            client_1_.Wallet().UnitDefinition(
                alice_nym_id_->str(),
                UNIT_DEFINITION_CONTRACT_NAME,
                UNIT_DEFINITION_TERMS,
                UNIT_DEFINITION_PRIMARY_UNIT_NAME,
                UNIT_DEFINITION_SYMBOL,
                UNIT_DEFINITION_TLA,
                UNIT_DEFINITION_POWER,
                UNIT_DEFINITION_FRACTIONAL_UNIT_NAME);

        ASSERT_TRUE(asset_contract_1_);
        EXPECT_EQ(proto::UNITTYPE_CURRENCY, asset_contract_1_->Type());
    }

    OTIdentifier find_issuer_account()
    {
        const auto accounts =
            client_1_.Storage().AccountsByOwner(alice_nym_id_);

        OT_ASSERT(1 == accounts.size());

        return *accounts.begin();
    }

    OTIdentifier find_unit_definition_id()
    {
        const auto accountID = find_issuer_account();

        OT_ASSERT(false == accountID->empty());

        const auto output = client_1_.Storage().AccountContract(accountID);

        OT_ASSERT(false == output->empty());

        return output;
    }

    OTIdentifier find_user_account()
    {
        const auto accounts = client_2_.Storage().AccountsByOwner(bob_nym_id_);

        OT_ASSERT(1 == accounts.size());

        return *accounts.begin();
    }

    void verify_account(
        const Nym& clientNym,
        const Nym& serverNym,
        const Account& client,
        const Account& server)
    {
        EXPECT_EQ(client.GetBalance(), server.GetBalance());
        EXPECT_EQ(
            client.GetInstrumentDefinitionID(),
            server.GetInstrumentDefinitionID());

        std::unique_ptr<Ledger> clientInbox(client.LoadInbox(clientNym));
        std::unique_ptr<Ledger> serverInbox(server.LoadInbox(serverNym));
        std::unique_ptr<Ledger> clientOutbox(client.LoadOutbox(clientNym));
        std::unique_ptr<Ledger> serverOutbox(server.LoadOutbox(serverNym));

        ASSERT_TRUE(clientInbox);
        ASSERT_TRUE(serverInbox);
        ASSERT_TRUE(clientOutbox);
        ASSERT_TRUE(serverOutbox);

        auto clientInboxHash = Identifier::Factory();
        auto serverInboxHash = Identifier::Factory();
        auto clientOutboxHash = Identifier::Factory();
        auto serverOutboxHash = Identifier::Factory();

        EXPECT_TRUE(clientInbox->CalculateInboxHash(clientInboxHash));
        EXPECT_TRUE(serverInbox->CalculateInboxHash(serverInboxHash));
        EXPECT_TRUE(clientOutbox->CalculateOutboxHash(clientOutboxHash));
        EXPECT_TRUE(serverOutbox->CalculateOutboxHash(serverOutboxHash));
    }

    void verify_state_pre(
        const ClientContext& clientContext,
        const ServerContext& serverContext,
        const RequestNumber initialRequestNumber)
    {
        EXPECT_EQ(serverContext.Request(), initialRequestNumber);
        EXPECT_EQ(clientContext.Request(), initialRequestNumber);
    }

    void verify_state_post(
        const opentxs::api::client::Manager& client,
        const ClientContext& clientContext,
        const ServerContext& serverContext,
        const RequestNumber initialRequestNumber,
        const RequestNumber messageRequestNumber,
        const TransactionNumber messageTransactionNumber,
        const SendResult messageResult,
        const std::shared_ptr<Message>& message,
        const bool messageSuccess,
        const bool nymboxHashUpdated,
        const bool usesTransaction,
        const std::size_t expectedNymboxItems)
    {
        const auto nextRequestNumber = initialRequestNumber + 1;

        EXPECT_EQ(messageRequestNumber, initialRequestNumber);
        EXPECT_EQ(serverContext.Request(), nextRequestNumber);
        EXPECT_EQ(clientContext.Request(), nextRequestNumber);
        EXPECT_EQ(
            serverContext.RemoteNymboxHash(), clientContext.LocalNymboxHash());

        if (nymboxHashUpdated) {
            EXPECT_NE(
                serverContext.LocalNymboxHash(),
                serverContext.RemoteNymboxHash());
        } else {
            EXPECT_EQ(
                serverContext.LocalNymboxHash(),
                serverContext.RemoteNymboxHash());
        }

        if (usesTransaction) {
            // TODO
        } else {
            EXPECT_EQ(0, messageTransactionNumber);
        }

        EXPECT_EQ(SendResult::VALID_REPLY, messageResult);
        ASSERT_TRUE(message);

        EXPECT_EQ(messageSuccess, message->m_bSuccess);

        std::unique_ptr<Ledger> nymbox{
            client.OTAPI().LoadNymbox(server_id_, serverContext.Nym()->ID())};

        ASSERT_TRUE(nymbox);
        EXPECT_TRUE(nymbox->VerifyAccount(*serverContext.Nym()));

        const auto& transactionMap = nymbox->GetTransactionMap();

        EXPECT_EQ(expectedNymboxItems, transactionMap.size());
    }
};

const opentxs::ArgList Test_Basic::args_{
    {{OPENTXS_ARG_STORAGE_PLUGIN, {"mem"}}}};
const std::string Test_Basic::SeedA_{""};
const std::string Test_Basic::SeedB_{""};
const std::string Test_Basic::Alice_{""};
const std::string Test_Basic::Bob_{""};
const OTIdentifier Test_Basic::alice_nym_id_{Identifier::Factory()};
const OTIdentifier Test_Basic::bob_nym_id_{Identifier::Factory()};
const std::shared_ptr<const ServerContract> Test_Basic::server_contract_{
    nullptr};
const std::shared_ptr<const UnitDefinition> Test_Basic::asset_contract_1_{
    nullptr};
TransactionNumber Test_Basic::cheque_transaction_number_{0};

TEST_F(Test_Basic, getRequestNumber_not_registered)
{
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);

    EXPECT_EQ(serverContext.It().Request(), 0);
    EXPECT_FALSE(clientContext);

    const auto number = serverContext.It().UpdateRequestNumber();

    EXPECT_EQ(0, number);
    EXPECT_EQ(serverContext.It().Request(), 0);

    clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);

    EXPECT_FALSE(clientContext);
}

TEST_F(Test_Basic, registerNym_first_time)
{
    const RequestNumber sequence{0};
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);

    EXPECT_EQ(serverContext.It().Request(), sequence);
    EXPECT_FALSE(clientContext);

    const auto [requestNumber, transactionNumber, reply] =
        client_1_.OTAPI().registerNym(serverContext.It());
    const auto& [result, message] = reply;
    clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_SAME,
        NO_TRANSACTION,
        0);
}

TEST_F(Test_Basic, getTransactionNumbers_first_time)
{
    const RequestNumber sequence{1};
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_1_.OTAPI().getTransactionNumbers(serverContext.It());
    const auto& [result, message] = reply;
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_UPDATED,
        NO_TRANSACTION,
        0);
}

TEST_F(Test_Basic, Reregister)
{
    const RequestNumber sequence{2};
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_1_.OTAPI().registerNym(serverContext.It());
    const auto& [result, message] = reply;
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_UPDATED,  // TODO Maybe OTClient should have updated local nymbox
                         // hash
        NO_TRANSACTION,
        0);
}

TEST_F(Test_Basic, getNymbox_after_transaction_numbers)
{
    const RequestNumber sequence{3};
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_1_.OTAPI().getNymbox(serverContext.It());
    const auto& [result, message] = reply;
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_SAME,
        NO_TRANSACTION,
        1);
    std::unique_ptr<Ledger> nymbox{client_1_.OTAPI().LoadNymbox(
        serverContext.It().Server(), serverContext.It().Nym()->ID())};

    ASSERT_TRUE(nymbox);

    const auto& transactionMap = nymbox->GetTransactionMap();

    ASSERT_EQ(1, transactionMap.size());

    const TransactionNumber number{transactionMap.begin()->first};
    const auto& transaction = *transactionMap.begin()->second;

    EXPECT_TRUE(transaction.IsAbbreviated());
    EXPECT_EQ(transactionType::blank, transaction.GetType());
    EXPECT_FALSE(nymbox->LoadBoxReceipt(number));
}

TEST_F(Test_Basic, getBoxReceipt_transaction_numbers)
{
    const RequestNumber sequence{4};
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);
    const auto& nymID = serverContext.It().Nym()->ID();
    const auto& serverID = serverContext.It().Server();

    ASSERT_TRUE(clientContext);

    std::unique_ptr<Ledger> nymbox{
        client_1_.OTAPI().LoadNymbox(serverID, nymID)};

    ASSERT_TRUE(nymbox);

    TransactionNumber number{0};

    {
        const auto& transactionMap = nymbox->GetTransactionMap();

        ASSERT_EQ(1, transactionMap.size());

        number = {transactionMap.begin()->first};
        nymbox.reset();
    }

    ASSERT_NE(0, number);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_1_.OTAPI().getBoxReceipt(
            serverContext.It(), nymID, NYMBOX_TYPE, number);
    const auto& [result, message] = reply;
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_SAME,
        NO_TRANSACTION,
        1);
    nymbox.reset(client_1_.OTAPI().LoadNymbox(serverID, nymID).release());

    ASSERT_TRUE(nymbox);

    const auto& transactionMap = nymbox->GetTransactionMap();

    ASSERT_EQ(1, transactionMap.size());

    const auto& transaction = *transactionMap.begin()->second;

    EXPECT_FALSE(transaction.IsAbbreviated());
}

TEST_F(Test_Basic, processNymbox)
{
    const RequestNumber sequence{5};
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_1_.OTAPI().processNymbox(serverContext.It());
    const auto& [result, message] = reply;
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_UPDATED,
        NO_TRANSACTION,
        0);
}

TEST_F(Test_Basic, getNymbox_after_processNymbox)
{
    const RequestNumber sequence{6};
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_1_.OTAPI().getNymbox(serverContext.It());
    const auto& [result, message] = reply;
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_SAME,
        NO_TRANSACTION,
        1);

    std::unique_ptr<Ledger> nymbox{client_1_.OTAPI().LoadNymbox(
        serverContext.It().Server(), serverContext.It().Nym()->ID())};

    ASSERT_TRUE(nymbox);

    const auto& transactionMap = nymbox->GetTransactionMap();

    ASSERT_EQ(1, transactionMap.size());

    const TransactionNumber number{transactionMap.begin()->first};
    const auto& transaction = *transactionMap.begin()->second;

    EXPECT_TRUE(transaction.IsAbbreviated());
    EXPECT_EQ(transactionType::successNotice, transaction.GetType());
    EXPECT_FALSE(nymbox->LoadBoxReceipt(number));
}

TEST_F(Test_Basic, getBoxReceipt_success_notice)
{
    const RequestNumber sequence{7};
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);
    const auto& nymID = serverContext.It().Nym()->ID();
    const auto& serverID = serverContext.It().Server();

    ASSERT_TRUE(clientContext);

    std::unique_ptr<Ledger> nymbox{
        client_1_.OTAPI().LoadNymbox(serverID, nymID)};

    ASSERT_TRUE(nymbox);

    TransactionNumber number{0};

    {
        const auto& transactionMap = nymbox->GetTransactionMap();

        ASSERT_EQ(1, transactionMap.size());

        number = {transactionMap.begin()->first};
        nymbox.reset();
    }

    ASSERT_NE(0, number);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_1_.OTAPI().getBoxReceipt(
            serverContext.It(), nymID, NYMBOX_TYPE, number);
    const auto& [result, message] = reply;
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_SAME,
        NO_TRANSACTION,
        1);

    nymbox.reset(client_1_.OTAPI().LoadNymbox(serverID, nymID).release());

    ASSERT_TRUE(nymbox);

    const auto& transactionMap = nymbox->GetTransactionMap();

    ASSERT_EQ(1, transactionMap.size());

    const auto& transaction = *transactionMap.begin()->second;

    EXPECT_FALSE(transaction.IsAbbreviated());
}

TEST_F(Test_Basic, issueAsset)
{
    const RequestNumber sequence{8};
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    create_unit_definition();
    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_1_.OTAPI().registerInstrumentDefinition(
            serverContext.It(), asset_contract_1_->PublicContract());
    const auto& [result, message] = reply;
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_UPDATED,
        NO_TRANSACTION,
        1);
    const auto accountID = Identifier::Factory(message->m_strAcctID);

    ASSERT_FALSE(accountID->empty());

    const auto clientAccount = client_1_.Wallet().Account(accountID);
    const auto serverAccount = server_.Wallet().Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);

    verify_account(
        *serverContext.It().Nym(),
        *clientContext->Nym(),
        clientAccount,
        serverAccount);
}

TEST_F(Test_Basic, getAccountData_after_issuer_account)
{
    const RequestNumber sequence{9};
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    const auto accountID = find_issuer_account();

    ASSERT_FALSE(accountID->empty());

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_1_.OTAPI().getAccountData(serverContext.It(), accountID);
    const auto& [result, message] = reply;
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_UPDATED,
        NO_TRANSACTION,
        1);

    const auto clientAccount = client_1_.Wallet().Account(accountID);
    const auto serverAccount = server_.Wallet().Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);

    verify_account(
        *serverContext.It().Nym(),
        *clientContext->Nym(),
        clientAccount,
        serverAccount);
}

TEST_F(Test_Basic, getNymbox_after_registerInstrumentDefinition)
{
    const RequestNumber sequence{10};
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_1_.OTAPI().getNymbox(serverContext.It());
    const auto& [result, message] = reply;
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_SAME,
        NO_TRANSACTION,
        1);
}

TEST_F(Test_Basic, processNymbox_after_registerInstrumentDefinition)
{
    const RequestNumber sequence{11};
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_1_.OTAPI().processNymbox(serverContext.It());
    const auto& [result, message] = reply;
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_UPDATED,
        NO_TRANSACTION,
        0);
}

TEST_F(Test_Basic, registerNym_Bob)
{
    const RequestNumber sequence{0};
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), bob_nym_id_);

    EXPECT_EQ(serverContext.It().Request(), sequence);
    EXPECT_FALSE(clientContext);

    const auto [requestNumber, transactionNumber, reply] =
        client_2_.OTAPI().registerNym(serverContext.It());
    const auto& [result, message] = reply;
    clientContext =
        server_.Wallet().ClientContext(server_.NymID(), bob_nym_id_);
    verify_state_post(
        client_2_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_SAME,
        NO_TRANSACTION,
        0);
}

TEST_F(Test_Basic, getInstrumentDefinition)
{
    const RequestNumber sequence{1};
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), bob_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_2_.OTAPI().getInstrumentDefinition(
            serverContext.It(), find_unit_definition_id());
    const auto& [result, message] = reply;
    clientContext =
        server_.Wallet().ClientContext(server_.NymID(), bob_nym_id_);
    verify_state_post(
        client_2_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_SAME,
        NO_TRANSACTION,
        0);
}

TEST_F(Test_Basic, registerAccount)
{
    const RequestNumber sequence{2};
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), bob_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_2_.OTAPI().registerAccount(
            serverContext.It(), find_unit_definition_id());
    const auto& [result, message] = reply;
    clientContext =
        server_.Wallet().ClientContext(server_.NymID(), bob_nym_id_);
    verify_state_post(
        client_2_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_UPDATED,
        NO_TRANSACTION,
        0);
    const auto accountID = Identifier::Factory(message->m_strAcctID);

    ASSERT_FALSE(accountID->empty());

    const auto clientAccount = client_2_.Wallet().Account(accountID);
    const auto serverAccount = server_.Wallet().Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);

    verify_account(
        *serverContext.It().Nym(),
        *clientContext->Nym(),
        clientAccount,
        serverAccount);
}

TEST_F(Test_Basic, getNymbox_after_registerAccount)
{
    const RequestNumber sequence{3};
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), bob_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_2_.OTAPI().getNymbox(serverContext.It());
    const auto& [result, message] = reply;
    verify_state_post(
        client_2_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_SAME,
        NO_TRANSACTION,
        0);
}

TEST_F(Test_Basic, getTransactionNumbers_Bob)
{
    const RequestNumber sequence{4};
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), bob_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_2_.OTAPI().getTransactionNumbers(serverContext.It());
    const auto& [result, message] = reply;
    verify_state_post(
        client_2_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_UPDATED,
        NO_TRANSACTION,
        0);
}

TEST_F(Test_Basic, getNymbox_after_transaction_numbers_Bob)
{
    const RequestNumber sequence{5};
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), bob_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_2_.OTAPI().getNymbox(serverContext.It());
    const auto& [result, message] = reply;
    verify_state_post(
        client_2_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_SAME,
        NO_TRANSACTION,
        1);
    std::unique_ptr<Ledger> nymbox{client_2_.OTAPI().LoadNymbox(
        serverContext.It().Server(), serverContext.It().Nym()->ID())};

    ASSERT_TRUE(nymbox);

    const auto& transactionMap = nymbox->GetTransactionMap();

    ASSERT_EQ(1, transactionMap.size());

    const TransactionNumber number{transactionMap.begin()->first};
    const auto& transaction = *transactionMap.begin()->second;

    EXPECT_TRUE(transaction.IsAbbreviated());
    EXPECT_EQ(transactionType::blank, transaction.GetType());
    EXPECT_FALSE(nymbox->LoadBoxReceipt(number));
}

TEST_F(Test_Basic, getBoxReceipt_transaction_numbers_bob)
{
    const RequestNumber sequence{6};
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), bob_nym_id_);
    const auto& nymID = serverContext.It().Nym()->ID();
    const auto& serverID = serverContext.It().Server();

    ASSERT_TRUE(clientContext);

    std::unique_ptr<Ledger> nymbox{
        client_2_.OTAPI().LoadNymbox(serverID, nymID)};

    ASSERT_TRUE(nymbox);

    TransactionNumber number{0};

    {
        const auto& transactionMap = nymbox->GetTransactionMap();

        ASSERT_EQ(1, transactionMap.size());

        number = {transactionMap.begin()->first};
        nymbox.reset();
    }

    ASSERT_NE(0, number);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_2_.OTAPI().getBoxReceipt(
            serverContext.It(), nymID, NYMBOX_TYPE, number);
    const auto& [result, message] = reply;
    verify_state_post(
        client_2_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_SAME,
        NO_TRANSACTION,
        1);
    nymbox.reset(client_2_.OTAPI().LoadNymbox(serverID, nymID).release());

    ASSERT_TRUE(nymbox);

    const auto& transactionMap = nymbox->GetTransactionMap();

    ASSERT_EQ(1, transactionMap.size());

    const auto& transaction = *transactionMap.begin()->second;

    EXPECT_FALSE(transaction.IsAbbreviated());
}

TEST_F(Test_Basic, processNymbox_bob)
{
    const RequestNumber sequence{7};
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), bob_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_2_.OTAPI().processNymbox(serverContext.It());
    const auto& [result, message] = reply;
    verify_state_post(
        client_2_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_UPDATED,
        NO_TRANSACTION,
        0);
}

TEST_F(Test_Basic, getAccountData_after_user_account)
{
    const RequestNumber sequence{8};
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), bob_nym_id_);

    ASSERT_TRUE(clientContext);

    const auto accountID = find_user_account();

    ASSERT_FALSE(accountID->empty());

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_2_.OTAPI().getAccountData(serverContext.It(), accountID);
    const auto& [result, message] = reply;
    verify_state_post(
        client_2_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_UPDATED,
        NO_TRANSACTION,
        0);

    const auto clientAccount = client_2_.Wallet().Account(accountID);
    const auto serverAccount = server_.Wallet().Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);

    verify_account(
        *serverContext.It().Nym(),
        *clientContext->Nym(),
        clientAccount,
        serverAccount);
}

TEST_F(Test_Basic, getNymbox_after_processNymbox_Bob)
{
    const RequestNumber sequence{9};
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), bob_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_2_.OTAPI().getNymbox(serverContext.It());
    const auto& [result, message] = reply;
    verify_state_post(
        client_2_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_SAME,
        NO_TRANSACTION,
        1);

    std::unique_ptr<Ledger> nymbox{client_2_.OTAPI().LoadNymbox(
        serverContext.It().Server(), serverContext.It().Nym()->ID())};

    ASSERT_TRUE(nymbox);

    const auto& transactionMap = nymbox->GetTransactionMap();

    ASSERT_EQ(1, transactionMap.size());

    const TransactionNumber number{transactionMap.begin()->first};
    const auto& transaction = *transactionMap.begin()->second;

    EXPECT_TRUE(transaction.IsAbbreviated());
    EXPECT_EQ(transactionType::successNotice, transaction.GetType());
    EXPECT_FALSE(nymbox->LoadBoxReceipt(number));
}

TEST_F(Test_Basic, getBoxReceipt_success_notice_Bob)
{
    const RequestNumber sequence{10};
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), bob_nym_id_);
    const auto& nymID = serverContext.It().Nym()->ID();
    const auto& serverID = serverContext.It().Server();

    ASSERT_TRUE(clientContext);

    std::unique_ptr<Ledger> nymbox{
        client_2_.OTAPI().LoadNymbox(serverID, nymID)};

    ASSERT_TRUE(nymbox);

    TransactionNumber number{0};

    {
        const auto& transactionMap = nymbox->GetTransactionMap();

        ASSERT_EQ(1, transactionMap.size());

        number = {transactionMap.begin()->first};
        nymbox.reset();
    }

    ASSERT_NE(0, number);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_2_.OTAPI().getBoxReceipt(
            serverContext.It(), nymID, NYMBOX_TYPE, number);
    const auto& [result, message] = reply;
    verify_state_post(
        client_2_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_SAME,
        NO_TRANSACTION,
        1);

    nymbox.reset(client_2_.OTAPI().LoadNymbox(serverID, nymID).release());

    ASSERT_TRUE(nymbox);

    const auto& transactionMap = nymbox->GetTransactionMap();

    ASSERT_EQ(1, transactionMap.size());

    const auto& transaction = *transactionMap.begin()->second;

    EXPECT_FALSE(transaction.IsAbbreviated());
}

TEST_F(Test_Basic, processNymbox_after_accept_transaction_numbers_bob)
{
    const RequestNumber sequence{11};
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), bob_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_2_.OTAPI().processNymbox(serverContext.It());
    const auto& [result, message] = reply;
    verify_state_post(
        client_2_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_UPDATED,
        NO_TRANSACTION,
        0);
}

TEST_F(Test_Basic, getNymbox_after_clearing_nymbox_1_Bob)
{
    const RequestNumber sequence{12};
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), bob_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_2_.OTAPI().getNymbox(serverContext.It());
    const auto& [result, message] = reply;
    verify_state_post(
        client_2_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_SAME,
        NO_TRANSACTION,
        0);
}

TEST_F(Test_Basic, checkNym)
{
    const RequestNumber sequence{12};
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_1_.OTAPI().checkNym(serverContext.It(), bob_nym_id_);
    const auto& [result, message] = reply;
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_UPDATED,
        NO_TRANSACTION,
        0);
}

TEST_F(Test_Basic, send_cheque)
{
    std::unique_ptr<Cheque> cheque{client_1_.OTAPI().WriteCheque(
        server_id_,
        CHEQUE_AMOUNT,
        0,
        0,
        find_issuer_account(),
        alice_nym_id_,
        String::Factory(CHEQUE_MEMO),
        bob_nym_id_)};
    std::unique_ptr<Message> request{};

    ASSERT_TRUE(cheque);

    cheque_transaction_number_ = cheque->GetTransactionNum();

    EXPECT_NE(0, cheque_transaction_number_);

    std::unique_ptr<OTPayment> payment{
        client_1_.Factory().Payment(String::Factory(*cheque))};

    ASSERT_TRUE(payment);

    const RequestNumber sequence{13};
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_1_.OTAPI().sendNymInstrument(
            serverContext.It(), request, bob_nym_id_, *payment, false);
    const auto& [result, message] = reply;
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_UPDATED,
        TRANSACTION,
        0);

    ASSERT_TRUE(request);

    auto workflow =
        client_1_.Workflow().SendCheque(*cheque, *request, message.get());

    EXPECT_TRUE(workflow);
}

TEST_F(Test_Basic, getNymbox_receive_cheque)
{
    const RequestNumber sequence{13};
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), bob_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_2_.OTAPI().getNymbox(serverContext.It());
    const auto& [result, message] = reply;
    verify_state_post(
        client_2_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_SAME,
        NO_TRANSACTION,
        1);

    std::unique_ptr<Ledger> nymbox{client_2_.OTAPI().LoadNymbox(
        serverContext.It().Server(), serverContext.It().Nym()->ID())};

    ASSERT_TRUE(nymbox);

    const auto& transactionMap = nymbox->GetTransactionMap();

    ASSERT_EQ(1, transactionMap.size());

    const TransactionNumber number{transactionMap.begin()->first};
    const auto& transaction = *transactionMap.begin()->second;

    EXPECT_EQ(transactionType::message, transaction.GetType());

    // If the push notification was received, then the box receipt already
    // exists. But since we can't guarantee timing it's possible the
    // notification did not arrive.
    if (transaction.IsAbbreviated()) {
        EXPECT_FALSE(nymbox->LoadBoxReceipt(number));
    }
}

TEST_F(Test_Basic, getBoxReceipt_incoming_cheque)
{
    const RequestNumber sequence{14};
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), bob_nym_id_);
    const auto& nymID = serverContext.It().Nym()->ID();
    const auto& serverID = serverContext.It().Server();

    ASSERT_TRUE(clientContext);

    std::unique_ptr<Ledger> nymbox{
        client_2_.OTAPI().LoadNymbox(serverID, nymID)};

    ASSERT_TRUE(nymbox);

    TransactionNumber number{0};

    {
        const auto& transactionMap = nymbox->GetTransactionMap();

        ASSERT_EQ(1, transactionMap.size());

        number = {transactionMap.begin()->first};
        nymbox.reset();
    }

    ASSERT_NE(0, number);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_2_.OTAPI().getBoxReceipt(
            serverContext.It(), nymID, NYMBOX_TYPE, number);
    const auto& [result, message] = reply;
    verify_state_post(
        client_2_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_SAME,
        NO_TRANSACTION,
        1);
    nymbox.reset(client_2_.OTAPI().LoadNymbox(serverID, nymID).release());

    ASSERT_TRUE(nymbox);

    const auto& transactionMap = nymbox->GetTransactionMap();

    ASSERT_EQ(1, transactionMap.size());

    const auto& transaction = *transactionMap.begin()->second;

    EXPECT_FALSE(transaction.IsAbbreviated());
}

TEST_F(Test_Basic, processNymbox_incoming_cheque)
{
    const RequestNumber sequence{15};
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), bob_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_2_.OTAPI().processNymbox(serverContext.It());
    const auto& [result, message] = reply;
    verify_state_post(
        client_2_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_UPDATED,
        NO_TRANSACTION,
        0);
}

TEST_F(Test_Basic, getNymbox_after_clearing_nymbox_2_Bob)
{
    const RequestNumber sequence{16};
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), bob_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_2_.OTAPI().getNymbox(serverContext.It());
    const auto& [result, message] = reply;
    verify_state_post(
        client_2_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_SAME,
        NO_TRANSACTION,
        0);
}

TEST_F(Test_Basic, depositCheque)
{
    const auto accountID = find_user_account();
    const auto workflows =
        client_2_.Storage().PaymentWorkflowList(bob_nym_id_->str());

    ASSERT_EQ(1, workflows.size());

    const auto& workflowID = Identifier::Factory(workflows.begin()->first);
    const auto workflow =
        client_2_.Workflow().LoadWorkflow(bob_nym_id_, workflowID);

    ASSERT_TRUE(workflow);
    ASSERT_TRUE(api::client::Workflow::ContainsCheque(*workflow));

    const auto [state, cheque] =
        api::client::Workflow::InstantiateCheque(client_2_, *workflow);

    ASSERT_EQ(proto::PAYMENTWORKFLOWSTATE_CONVEYED, state);
    ASSERT_TRUE(cheque);

    const RequestNumber sequence{17};
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), bob_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_2_.OTAPI().depositCheque(serverContext.It(), accountID, *cheque);
    const auto& [result, message] = reply;
    verify_state_post(
        client_2_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_UPDATED,
        TRANSACTION,
        0);
    const auto serverAccount = server_.Wallet().Account(accountID);

    ASSERT_TRUE(serverAccount);
    EXPECT_EQ(CHEQUE_AMOUNT, serverAccount.get().GetBalance());
}

TEST_F(Test_Basic, getAccountData_after_deposit_cheque)
{
    const RequestNumber sequence{18};
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), bob_nym_id_);

    ASSERT_TRUE(clientContext);

    const auto accountID = find_user_account();

    ASSERT_FALSE(accountID->empty());

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_2_.OTAPI().getAccountData(serverContext.It(), accountID);
    const auto& [result, message] = reply;
    verify_state_post(
        client_2_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_UPDATED,
        NO_TRANSACTION,
        0);

    const auto clientAccount = client_2_.Wallet().Account(accountID);
    const auto serverAccount = server_.Wallet().Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);

    verify_account(
        *serverContext.It().Nym(),
        *clientContext->Nym(),
        clientAccount,
        serverAccount);

    EXPECT_EQ(CHEQUE_AMOUNT, clientAccount.get().GetBalance());
}

TEST_F(Test_Basic, getNymbox_after_deposit_cheque)
{
    const RequestNumber sequence{19};
    auto serverContext =
        client_2_.Wallet().mutable_ServerContext(bob_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), bob_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_2_.OTAPI().getNymbox(serverContext.It());
    const auto& [result, message] = reply;
    verify_state_post(
        client_2_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_SAME,
        NO_TRANSACTION,
        0);
}

TEST_F(Test_Basic, getAccountData_after_cheque_deposited)
{
    const RequestNumber sequence{14};
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    const auto accountID = find_issuer_account();

    ASSERT_FALSE(accountID->empty());

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_1_.OTAPI().getAccountData(serverContext.It(), accountID);
    const auto& [result, message] = reply;
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_UPDATED,
        NO_TRANSACTION,
        0);
    const auto clientAccount = client_1_.Wallet().Account(accountID);
    const auto serverAccount = server_.Wallet().Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);

    verify_account(
        *serverContext.It().Nym(),
        *clientContext->Nym(),
        clientAccount,
        serverAccount);

    EXPECT_EQ(-1 * CHEQUE_AMOUNT, serverAccount.get().GetBalance());

    std::unique_ptr<Ledger> inbox{
        clientAccount.get().LoadInbox(*serverContext.It().Nym())};

    ASSERT_TRUE(inbox);

    const auto& transactionMap = inbox->GetTransactionMap();

    ASSERT_EQ(1, transactionMap.size());

    const TransactionNumber number{transactionMap.begin()->first};
    const auto& transaction = *transactionMap.begin()->second;

    EXPECT_EQ(transactionType::chequeReceipt, transaction.GetType());
}

TEST_F(Test_Basic, getBoxReceipt_cheque_receipt)
{
    const auto accountID = find_issuer_account();
    const RequestNumber sequence{15};
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    TransactionNumber number{0};

    {
        const auto clientAccount = client_1_.Wallet().Account(accountID);

        std::unique_ptr<Ledger> inbox{
            clientAccount.get().LoadInbox(*serverContext.It().Nym())};

        ASSERT_TRUE(inbox);

        const auto& transactionMap = inbox->GetTransactionMap();

        ASSERT_EQ(1, transactionMap.size());

        number = {transactionMap.begin()->first};
    }

    ASSERT_NE(0, number);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_1_.OTAPI().getBoxReceipt(
            serverContext.It(), accountID, INBOX_TYPE, number);
    const auto& [result, message] = reply;
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_UPDATED,
        NO_TRANSACTION,
        0);
    const auto clientAccount = client_1_.Wallet().Account(accountID);
    std::unique_ptr<Ledger> inbox{
        clientAccount.get().LoadInbox(*serverContext.It().Nym())};

    ASSERT_TRUE(inbox);

    const auto& transactionMap = inbox->GetTransactionMap();

    ASSERT_EQ(1, transactionMap.size());

    const auto& transaction = *transactionMap.begin()->second;

    EXPECT_FALSE(transaction.IsAbbreviated());
}

TEST_F(Test_Basic, getNymbox_after_cheque_deposited)
{
    const RequestNumber sequence{16};
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_1_.OTAPI().getNymbox(serverContext.It());
    const auto& [result, message] = reply;
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_SAME,
        NO_TRANSACTION,
        0);
}

TEST_F(Test_Basic, processInbox)
{
    const RequestNumber sequence{17};
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    const auto accountID = find_issuer_account();

    ASSERT_FALSE(accountID->empty());

    auto [response, inbox, number] =
        client_1_.OTAPI().CreateProcessInbox(accountID, serverContext.It());

    ASSERT_TRUE(response);
    ASSERT_TRUE(inbox);
    ASSERT_EQ(1, inbox->GetTransactionCount());
    EXPECT_NE(0, number);

    auto transaction = inbox->GetTransactionByIndex(0);

    ASSERT_NE(nullptr, transaction);
    ASSERT_EQ(transactionType::chequeReceipt, transaction->GetType());

    const auto workflow =
        client_1_.Workflow().ClearCheque(alice_nym_id_, *transaction);

    EXPECT_TRUE(workflow);

    const auto accepted = client_1_.OTAPI().IncludeResponse(
        accountID, true, serverContext.It(), *transaction, *response);

    ASSERT_TRUE(accepted);

    const auto finalized = client_1_.OTAPI().FinalizeProcessInbox(
        accountID, serverContext.It(), *response, *inbox);

    ASSERT_TRUE(finalized);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_1_.OTAPI().processInbox(
            serverContext.It(), accountID, String::Factory(*response));
    const auto& [result, message] = reply;
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_UPDATED,
        TRANSACTION,
        0);

    const auto clientAccount = client_1_.Wallet().Account(accountID);
    const auto serverAccount = server_.Wallet().Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);

    verify_account(
        *serverContext.It().Nym(),
        *clientContext->Nym(),
        clientAccount,
        serverAccount);

    EXPECT_EQ(-1 * CHEQUE_AMOUNT, serverAccount.get().GetBalance());
}

TEST_F(Test_Basic, getNymbox_after_processInbox)
{
    const RequestNumber sequence{18};
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_1_.OTAPI().getNymbox(serverContext.It());
    const auto& [result, message] = reply;
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_SAME,
        NO_TRANSACTION,
        0);
}

TEST_F(Test_Basic, resync)
{
    break_consensus();
    const RequestNumber sequence{19};
    auto serverContext =
        client_1_.Wallet().mutable_ServerContext(alice_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_1_.OTAPI().registerNym(serverContext.It(), true);
    const auto& [result, message] = reply;
    clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);
    verify_state_post(
        client_1_,
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        SUCCESS,
        NYMBOX_SAME,
        NO_TRANSACTION,
        0);
}
}  // namespace
