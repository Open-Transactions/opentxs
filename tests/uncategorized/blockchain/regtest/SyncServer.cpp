// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <atomic>
#include <memory>
#include <optional>

#include "internal/api/session/Wallet.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/network/otdht/Factory.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "ottest/fixtures/blockchain/Common.hpp"
#include "ottest/fixtures/blockchain/MinedBlocks.hpp"
#include "ottest/fixtures/blockchain/SyncRequestor.hpp"
#include "ottest/fixtures/blockchain/SyncSubscriber.hpp"
#include "ottest/fixtures/blockchain/regtest/SyncServer.hpp"
#include "ottest/fixtures/common/User.hpp"

namespace ottest
{
TEST_F(Regtest_fixture_sync_server, init_opentxs) {}

TEST_F(Regtest_fixture_sync_server, start_chains) { EXPECT_TRUE(Start()); }

TEST_F(Regtest_fixture_sync_server, connect_peers) { EXPECT_TRUE(Connect()); }

TEST_F(Regtest_fixture_sync_server, sync_genesis)
{
    Requestor().expected_ += 2;
    const auto handle = miner_.Network().Blockchain().GetChain(test_chain_);

    ASSERT_TRUE(handle);

    const auto& chain = handle.get();
    const auto pos = chain.HeaderOracle().BestChain();

    EXPECT_TRUE(Requestor().request(pos));
    ASSERT_TRUE(Requestor().wait());

    {
        const auto& msg = Requestor().get(++Requestor().checked_);
        const auto base = miner_.Factory().BlockchainSyncMessage(msg);

        ASSERT_TRUE(base);
        ASSERT_EQ(base->Type(), MessageType::sync_ack);

        const auto& ack = base->asAcknowledgement();
        const auto& states = ack.State();

        ASSERT_EQ(states.size(), 1);
        EXPECT_TRUE(Requestor().check(states.front(), pos));
        EXPECT_EQ(ack.Endpoint(), sync_server_push_endpoint_);
    }
    {
        const auto& msg = Requestor().get(++Requestor().checked_);
        const auto base = miner_.Factory().BlockchainSyncMessage(msg);

        ASSERT_TRUE(base);
        ASSERT_EQ(base->Type(), MessageType::sync_reply);

        const auto& reply = base->asData();
        const auto& state = reply.State();
        const auto& blocks = reply.Blocks();

        EXPECT_TRUE(Requestor().check(state, pos));
        EXPECT_EQ(blocks.size(), 0);
    }
}

TEST_F(Regtest_fixture_sync_server, mine)
{
    constexpr auto count{10};
    Subscriber().expected_ += count;

    EXPECT_TRUE(Mine(0, count));
    EXPECT_TRUE(Subscriber().wait());

    const auto handle =
        sync_server_.Network().Blockchain().GetChain(test_chain_);

    ASSERT_TRUE(handle);

    const auto& chain = handle.get();
    const auto best = chain.HeaderOracle().BestChain();

    EXPECT_EQ(best.height_, 10);
    EXPECT_EQ(best.hash_, mined_blocks_.get(9).get());
}

TEST_F(Regtest_fixture_sync_server, sync_full)
{
    Requestor().expected_ += 2;
    const auto handle = miner_.Network().Blockchain().GetChain(test_chain_);

    ASSERT_TRUE(handle);

    const auto& chain = handle.get();
    const auto genesis = Position{0, chain.HeaderOracle().BestHash(0)};

    EXPECT_TRUE(Requestor().request(genesis));
    ASSERT_TRUE(Requestor().wait());

    {
        const auto& msg = Requestor().get(++Requestor().checked_);
        const auto base = miner_.Factory().BlockchainSyncMessage(msg);

        ASSERT_TRUE(base);
        ASSERT_EQ(base->Type(), MessageType::sync_ack);

        const auto& ack = base->asAcknowledgement();
        const auto& states = ack.State();

        ASSERT_EQ(states.size(), 1);
        EXPECT_TRUE(Requestor().check(states.front(), 9));
        EXPECT_EQ(ack.Endpoint(), sync_server_push_endpoint_);
    }
    {
        const auto& msg = Requestor().get(++Requestor().checked_);
        const auto base = miner_.Factory().BlockchainSyncMessage(msg);

        ASSERT_TRUE(base);
        ASSERT_EQ(base->Type(), MessageType::sync_reply);

        const auto& reply = base->asData();
        const auto& state = reply.State();
        const auto& blocks = reply.Blocks();

        EXPECT_TRUE(Requestor().check(state, 9));
        ASSERT_EQ(blocks.size(), 10);
        EXPECT_TRUE(Requestor().check(blocks.at(0), 0));
        EXPECT_TRUE(Requestor().check(blocks.at(1), 1));
        EXPECT_TRUE(Requestor().check(blocks.at(2), 2));
        EXPECT_TRUE(Requestor().check(blocks.at(3), 3));
        EXPECT_TRUE(Requestor().check(blocks.at(4), 4));
        EXPECT_TRUE(Requestor().check(blocks.at(5), 5));
        EXPECT_TRUE(Requestor().check(blocks.at(6), 6));
        EXPECT_TRUE(Requestor().check(blocks.at(7), 7));
        EXPECT_TRUE(Requestor().check(blocks.at(8), 8));
        EXPECT_TRUE(Requestor().check(blocks.at(9), 9));
    }
}

TEST_F(Regtest_fixture_sync_server, sync_partial)
{
    Requestor().expected_ += 2;
    const auto start = Position{6, mined_blocks_.get(6).get()};

    EXPECT_TRUE(Requestor().request(start));
    ASSERT_TRUE(Requestor().wait());

    {
        const auto& msg = Requestor().get(++Requestor().checked_);
        const auto base = miner_.Factory().BlockchainSyncMessage(msg);

        ASSERT_TRUE(base);
        ASSERT_EQ(base->Type(), MessageType::sync_ack);

        const auto& ack = base->asAcknowledgement();
        const auto& states = ack.State();

        ASSERT_EQ(states.size(), 1);
        EXPECT_TRUE(Requestor().check(states.front(), 9));
        EXPECT_EQ(ack.Endpoint(), sync_server_push_endpoint_);
    }
    {
        const auto& msg = Requestor().get(++Requestor().checked_);
        const auto base = miner_.Factory().BlockchainSyncMessage(msg);

        ASSERT_TRUE(base);
        ASSERT_EQ(base->Type(), MessageType::sync_reply);

        const auto& reply = base->asData();
        const auto& state = reply.State();
        const auto& blocks = reply.Blocks();

        EXPECT_TRUE(Requestor().check(state, 9));
        ASSERT_EQ(blocks.size(), 4);
        EXPECT_TRUE(Requestor().check(blocks.at(0), 6));
        EXPECT_TRUE(Requestor().check(blocks.at(1), 7));
        EXPECT_TRUE(Requestor().check(blocks.at(2), 8));
        EXPECT_TRUE(Requestor().check(blocks.at(3), 9));
    }
}

TEST_F(Regtest_fixture_sync_server, reorg)
{
    constexpr auto count{4};
    Subscriber().expected_ += count;

    EXPECT_TRUE(Mine(8, count));
    EXPECT_TRUE(Subscriber().wait());

    const auto handle =
        sync_server_.Network().Blockchain().GetChain(test_chain_);

    ASSERT_TRUE(handle);

    const auto& chain = handle.get();
    const auto best = chain.HeaderOracle().BestChain();

    EXPECT_EQ(best.height_, 12);
    EXPECT_EQ(best.hash_, mined_blocks_.get(13).get());
}

TEST_F(Regtest_fixture_sync_server, sync_reorg)
{
    Requestor().expected_ += 2;
    const auto start = Position{10, mined_blocks_.get(9).get()};

    EXPECT_TRUE(Requestor().request(start));
    ASSERT_TRUE(Requestor().wait());

    {
        const auto& msg = Requestor().get(++Requestor().checked_);
        const auto base = miner_.Factory().BlockchainSyncMessage(msg);

        ASSERT_TRUE(base);
        ASSERT_EQ(base->Type(), MessageType::sync_ack);

        const auto& ack = base->asAcknowledgement();
        const auto& states = ack.State();

        ASSERT_EQ(states.size(), 1);
        EXPECT_TRUE(Requestor().check(states.front(), 13));
        EXPECT_EQ(ack.Endpoint(), sync_server_push_endpoint_);
    }
    {
        const auto& msg = Requestor().get(++Requestor().checked_);
        const auto base = miner_.Factory().BlockchainSyncMessage(msg);

        ASSERT_TRUE(base);
        ASSERT_EQ(base->Type(), MessageType::sync_reply);

        const auto& reply = base->asData();
        const auto& state = reply.State();
        const auto& blocks = reply.Blocks();

        EXPECT_TRUE(Requestor().check(state, 13));
        ASSERT_EQ(blocks.size(), 4);
        EXPECT_TRUE(Requestor().check(blocks.at(0), 10));
        EXPECT_TRUE(Requestor().check(blocks.at(1), 11));
        EXPECT_TRUE(Requestor().check(blocks.at(2), 12));
        EXPECT_TRUE(Requestor().check(blocks.at(3), 13));
    }
}

TEST_F(Regtest_fixture_sync_server, query)
{
    const auto original = opentxs::factory::BlockchainSyncQuery(0);

    EXPECT_EQ(original.Type(), MessageType::query);
    EXPECT_NE(original.Version(), 0);

    {
        const auto serialized = [&] {
            auto out = opentxs::network::zeromq::Message{};

            EXPECT_TRUE(original.Serialize(out));

            return out;
        }();

        EXPECT_EQ(serialized.size(), 2);

        const auto header = serialized.Header();
        const auto body = serialized.Body();

        EXPECT_EQ(header.size(), 0);
        EXPECT_EQ(body.size(), 1);

        auto recovered = miner_.Factory().BlockchainSyncMessage(serialized);

        ASSERT_TRUE(recovered);

        EXPECT_EQ(recovered->Type(), MessageType::query);
        EXPECT_NE(recovered->Version(), 0);

        const auto& query = recovered->asQuery();

        EXPECT_EQ(query.Type(), MessageType::query);
        EXPECT_NE(query.Version(), 0);
    }

    {
        const auto serialized = [&] {
            auto out = opentxs::network::zeromq::Message{};
            out.AddFrame("Header frame 1");
            out.AddFrame("Header frame 2");
            out.StartBody();

            EXPECT_TRUE(original.Serialize(out));

            return out;
        }();

        EXPECT_EQ(serialized.size(), 4);

        const auto header = serialized.Header();
        const auto body = serialized.Body();

        EXPECT_EQ(header.size(), 2);
        EXPECT_EQ(body.size(), 1);

        auto recovered = miner_.Factory().BlockchainSyncMessage(serialized);

        ASSERT_TRUE(recovered);

        EXPECT_EQ(recovered->Type(), MessageType::query);
        EXPECT_NE(recovered->Version(), 0);

        const auto& query = recovered->asQuery();

        EXPECT_EQ(query.Type(), MessageType::query);
        EXPECT_NE(query.Version(), 0);
    }
}

TEST_F(Regtest_fixture_sync_server, make_contracts)
{
    ASSERT_TRUE(alex_.nym_);
    ASSERT_FALSE(alex_.nym_id_.empty());
    ASSERT_FALSE(notary_.has_value());
    ASSERT_FALSE(unit_.has_value());

    const auto reason = miner_.Factory().PasswordPrompt(__func__);
    notary_.emplace(miner_.Wallet().Internal().Server(
        alex_.nym_id_.asBase58(ot_.Crypto()),
        "Example notary",
        "Don't use",
        {{ot::AddressType::Inproc,
          ot::contract::ProtocolVersion::Legacy,
          "inproc://lol_nope",
          80,
          2}},
        reason,
        2));

    ASSERT_TRUE(notary_.has_value());
    ASSERT_FALSE(notary_.value()->ID().empty());

    unit_.emplace(miner_.Wallet().Internal().CurrencyContract(
        alex_.nym_id_.asBase58(ot_.Crypto()),
        "My Dollars",
        "Example only",
        ot::UnitType::Usd,
        ot::unsigned_amount(0, 1, 100),
        reason));

    ASSERT_TRUE(unit_.has_value());
    ASSERT_FALSE(unit_.value()->ID().empty());
}

TEST_F(Regtest_fixture_sync_server, query_nonexistent_nym)
{
    ASSERT_TRUE(alex_.nym_);

    const auto& id = alex_.nym_id_;

    EXPECT_EQ(id.Type(), ot::identifier::Type::nym);

    const auto original = opentxs::factory::BlockchainSyncQueryContract(id);

    EXPECT_EQ(original.Type(), MessageType::contract_query);
    EXPECT_NE(original.Version(), 0);
    EXPECT_EQ(original.ID(), id);

    {
        const auto serialized = [&] {
            auto out = opentxs::network::zeromq::Message{};

            EXPECT_TRUE(original.Serialize(out));

            return out;
        }();

        EXPECT_EQ(serialized.size(), 3);

        const auto header = serialized.Header();
        const auto body = serialized.Body();

        EXPECT_EQ(header.size(), 0);
        EXPECT_EQ(body.size(), 2);

        auto recovered = miner_.Factory().BlockchainSyncMessage(serialized);

        ASSERT_TRUE(recovered);

        EXPECT_EQ(recovered->Type(), MessageType::contract_query);
        EXPECT_NE(recovered->Version(), 0);

        const auto& query = recovered->asQueryContract();

        EXPECT_EQ(query.Type(), MessageType::contract_query);
        EXPECT_NE(query.Version(), 0);
        EXPECT_EQ(query.ID(), id);
    }

    Requestor().expected_ += 1;

    EXPECT_TRUE(Requestor().request(original));
    ASSERT_TRUE(Requestor().wait());

    {
        const auto& msg = Requestor().get(++Requestor().checked_);
        const auto base = miner_.Factory().BlockchainSyncMessage(msg);

        ASSERT_TRUE(base);
        ASSERT_EQ(base->Type(), MessageType::contract);

        const auto& reply = base->asQueryContractReply();

        EXPECT_EQ(reply.ID(), id);
        EXPECT_EQ(reply.ContractType(), ot::contract::Type::nym);
        EXPECT_FALSE(opentxs::valid(reply.Payload()));
    }
}

TEST_F(Regtest_fixture_sync_server, query_nonexistent_notary)
{
    ASSERT_TRUE(notary_.has_value());

    const auto id = notary_.value()->ID();

    EXPECT_EQ(id.Type(), ot::identifier::Type::notary);

    const auto original = opentxs::factory::BlockchainSyncQueryContract(id);

    EXPECT_EQ(original.Type(), MessageType::contract_query);
    EXPECT_NE(original.Version(), 0);
    EXPECT_EQ(original.ID(), id);

    {
        const auto serialized = [&] {
            auto out = opentxs::network::zeromq::Message{};

            EXPECT_TRUE(original.Serialize(out));

            return out;
        }();

        EXPECT_EQ(serialized.size(), 3);

        const auto header = serialized.Header();
        const auto body = serialized.Body();

        EXPECT_EQ(header.size(), 0);
        EXPECT_EQ(body.size(), 2);

        auto recovered = miner_.Factory().BlockchainSyncMessage(serialized);

        ASSERT_TRUE(recovered);

        EXPECT_EQ(recovered->Type(), MessageType::contract_query);
        EXPECT_NE(recovered->Version(), 0);

        const auto& query = recovered->asQueryContract();

        EXPECT_EQ(query.Type(), MessageType::contract_query);
        EXPECT_NE(query.Version(), 0);
        EXPECT_EQ(query.ID(), id);
    }

    Requestor().expected_ += 1;

    EXPECT_TRUE(Requestor().request(original));
    ASSERT_TRUE(Requestor().wait());

    {
        const auto& msg = Requestor().get(++Requestor().checked_);
        const auto base = miner_.Factory().BlockchainSyncMessage(msg);

        ASSERT_TRUE(base);
        ASSERT_EQ(base->Type(), MessageType::contract);

        const auto& reply = base->asQueryContractReply();

        EXPECT_EQ(reply.ID(), id);
        EXPECT_EQ(reply.ContractType(), ot::contract::Type::notary);
        EXPECT_FALSE(opentxs::valid(reply.Payload()));
    }
}

TEST_F(Regtest_fixture_sync_server, query_nonexistent_unit)
{
    ASSERT_TRUE(unit_.has_value());

    const auto id = unit_.value()->ID();

    EXPECT_EQ(id.Type(), ot::identifier::Type::unitdefinition);

    const auto original = opentxs::factory::BlockchainSyncQueryContract(id);

    EXPECT_EQ(original.Type(), MessageType::contract_query);
    EXPECT_NE(original.Version(), 0);
    EXPECT_EQ(original.ID(), id);

    {
        const auto serialized = [&] {
            auto out = opentxs::network::zeromq::Message{};

            EXPECT_TRUE(original.Serialize(out));

            return out;
        }();

        EXPECT_EQ(serialized.size(), 3);

        const auto header = serialized.Header();
        const auto body = serialized.Body();

        EXPECT_EQ(header.size(), 0);
        EXPECT_EQ(body.size(), 2);

        auto recovered = miner_.Factory().BlockchainSyncMessage(serialized);

        ASSERT_TRUE(recovered);

        EXPECT_EQ(recovered->Type(), MessageType::contract_query);
        EXPECT_NE(recovered->Version(), 0);

        const auto& query = recovered->asQueryContract();

        EXPECT_EQ(query.Type(), MessageType::contract_query);
        EXPECT_NE(query.Version(), 0);
        EXPECT_EQ(query.ID(), id);
    }

    Requestor().expected_ += 1;

    EXPECT_TRUE(Requestor().request(original));
    ASSERT_TRUE(Requestor().wait());

    {
        const auto& msg = Requestor().get(++Requestor().checked_);
        const auto base = miner_.Factory().BlockchainSyncMessage(msg);

        ASSERT_TRUE(base);
        ASSERT_EQ(base->Type(), MessageType::contract);

        const auto& reply = base->asQueryContractReply();

        EXPECT_EQ(reply.ID(), id);
        EXPECT_EQ(reply.ContractType(), ot::contract::Type::unit);
        EXPECT_FALSE(opentxs::valid(reply.Payload()));
    }
}

TEST_F(Regtest_fixture_sync_server, publish_nym)
{
    ASSERT_TRUE(alex_.nym_);

    const auto& id = alex_.nym_id_;
    const auto original =
        opentxs::factory::BlockchainSyncPublishContract(*alex_.nym_);

    EXPECT_EQ(original.Type(), MessageType::publish_contract);
    EXPECT_NE(original.Version(), 0);
    EXPECT_EQ(original.ID(), id);
    EXPECT_EQ(original.ContractType(), ot::contract::Type::nym);

    {
        const auto serialized = [&] {
            auto out = opentxs::network::zeromq::Message{};

            EXPECT_TRUE(original.Serialize(out));

            return out;
        }();

        EXPECT_EQ(serialized.size(), 5);

        const auto header = serialized.Header();
        const auto body = serialized.Body();

        EXPECT_EQ(header.size(), 0);
        EXPECT_EQ(body.size(), 4);

        auto recovered = miner_.Factory().BlockchainSyncMessage(serialized);

        ASSERT_TRUE(recovered);

        EXPECT_EQ(recovered->Type(), MessageType::publish_contract);
        EXPECT_NE(recovered->Version(), 0);

        const auto& query = recovered->asPublishContract();

        EXPECT_EQ(query.Type(), MessageType::publish_contract);
        EXPECT_NE(query.Version(), 0);
        EXPECT_EQ(query.ID(), id);
        EXPECT_EQ(query.ContractType(), ot::contract::Type::nym);
    }

    Requestor().expected_ += 1;

    EXPECT_TRUE(Requestor().request(original));
    ASSERT_TRUE(Requestor().wait());

    {
        const auto& msg = Requestor().get(++Requestor().checked_);
        const auto base = miner_.Factory().BlockchainSyncMessage(msg);

        ASSERT_TRUE(base);
        ASSERT_EQ(base->Type(), MessageType::publish_ack);

        const auto& reply = base->asPublishContractReply();

        EXPECT_EQ(reply.ID(), id);
        EXPECT_TRUE(reply.Success());
    }
}

TEST_F(Regtest_fixture_sync_server, publish_notary)
{
    ASSERT_TRUE(notary_.has_value());

    const auto id = notary_.value()->ID();
    const auto original =
        opentxs::factory::BlockchainSyncPublishContract(notary_.value());

    EXPECT_EQ(original.Type(), MessageType::publish_contract);
    EXPECT_NE(original.Version(), 0);
    EXPECT_EQ(original.ID(), id);
    EXPECT_EQ(original.ContractType(), ot::contract::Type::notary);

    {
        const auto serialized = [&] {
            auto out = opentxs::network::zeromq::Message{};

            EXPECT_TRUE(original.Serialize(out));

            return out;
        }();

        EXPECT_EQ(serialized.size(), 5);

        const auto header = serialized.Header();
        const auto body = serialized.Body();

        EXPECT_EQ(header.size(), 0);
        EXPECT_EQ(body.size(), 4);

        auto recovered = miner_.Factory().BlockchainSyncMessage(serialized);

        ASSERT_TRUE(recovered);

        EXPECT_EQ(recovered->Type(), MessageType::publish_contract);
        EXPECT_NE(recovered->Version(), 0);

        const auto& query = recovered->asPublishContract();

        EXPECT_EQ(query.Type(), MessageType::publish_contract);
        EXPECT_NE(query.Version(), 0);
        EXPECT_EQ(query.ID(), id);
        EXPECT_EQ(query.ContractType(), ot::contract::Type::notary);
    }

    Requestor().expected_ += 1;

    EXPECT_TRUE(Requestor().request(original));
    ASSERT_TRUE(Requestor().wait());

    {
        const auto& msg = Requestor().get(++Requestor().checked_);
        const auto base = miner_.Factory().BlockchainSyncMessage(msg);

        ASSERT_TRUE(base);
        ASSERT_EQ(base->Type(), MessageType::publish_ack);

        const auto& reply = base->asPublishContractReply();

        EXPECT_EQ(reply.ID(), id);
        EXPECT_TRUE(reply.Success());
    }
}

TEST_F(Regtest_fixture_sync_server, publish_unit)
{
    ASSERT_TRUE(unit_.has_value());

    const auto id = unit_.value()->ID();
    const auto original =
        opentxs::factory::BlockchainSyncPublishContract(unit_.value());

    EXPECT_EQ(original.Type(), MessageType::publish_contract);
    EXPECT_NE(original.Version(), 0);
    EXPECT_EQ(original.ID(), id);
    EXPECT_EQ(original.ContractType(), ot::contract::Type::unit);

    {
        const auto serialized = [&] {
            auto out = opentxs::network::zeromq::Message{};

            EXPECT_TRUE(original.Serialize(out));

            return out;
        }();

        EXPECT_EQ(serialized.size(), 5);

        const auto header = serialized.Header();
        const auto body = serialized.Body();

        EXPECT_EQ(header.size(), 0);
        EXPECT_EQ(body.size(), 4);

        auto recovered = miner_.Factory().BlockchainSyncMessage(serialized);

        ASSERT_TRUE(recovered);

        EXPECT_EQ(recovered->Type(), MessageType::publish_contract);
        EXPECT_NE(recovered->Version(), 0);

        const auto& query = recovered->asPublishContract();

        EXPECT_EQ(query.Type(), MessageType::publish_contract);
        EXPECT_NE(query.Version(), 0);
        EXPECT_EQ(query.ID(), id);
        EXPECT_EQ(query.ContractType(), ot::contract::Type::unit);
    }

    Requestor().expected_ += 1;

    EXPECT_TRUE(Requestor().request(original));
    ASSERT_TRUE(Requestor().wait());

    {
        const auto& msg = Requestor().get(++Requestor().checked_);
        const auto base = miner_.Factory().BlockchainSyncMessage(msg);

        ASSERT_TRUE(base);
        ASSERT_EQ(base->Type(), MessageType::publish_ack);

        const auto& reply = base->asPublishContractReply();

        EXPECT_EQ(reply.ID(), id);
        EXPECT_TRUE(reply.Success());
    }
}

TEST_F(Regtest_fixture_sync_server, query_nym)
{
    ASSERT_TRUE(alex_.nym_);

    const auto& id = alex_.nym_id_;
    const auto expected = [&] {
        auto out = miner_.Factory().Data();
        alex_.nym_->Serialize(out.WriteInto());

        return out;
    }();

    EXPECT_EQ(id.Type(), ot::identifier::Type::nym);

    const auto original = opentxs::factory::BlockchainSyncQueryContract(id);

    EXPECT_EQ(original.Type(), MessageType::contract_query);
    EXPECT_NE(original.Version(), 0);
    EXPECT_EQ(original.ID(), id);

    Requestor().expected_ += 1;

    EXPECT_TRUE(Requestor().request(original));
    ASSERT_TRUE(Requestor().wait());

    {
        const auto& msg = Requestor().get(++Requestor().checked_);
        const auto base = miner_.Factory().BlockchainSyncMessage(msg);

        ASSERT_TRUE(base);
        ASSERT_EQ(base->Type(), MessageType::contract);

        const auto& reply = base->asQueryContractReply();

        EXPECT_EQ(reply.ID(), id);
        EXPECT_EQ(reply.ContractType(), ot::contract::Type::nym);
        ASSERT_TRUE(opentxs::valid(reply.Payload()));
        EXPECT_EQ(expected.Bytes(), reply.Payload());
    }
}

TEST_F(Regtest_fixture_sync_server, query_notary)
{
    ASSERT_TRUE(notary_.has_value());

    const auto id = notary_.value()->ID();
    const auto expected = [&] {
        auto out = miner_.Factory().Data();
        notary_.value()->Serialize(out.WriteInto());

        return out;
    }();

    EXPECT_EQ(id.Type(), ot::identifier::Type::notary);

    const auto original = opentxs::factory::BlockchainSyncQueryContract(id);

    EXPECT_EQ(original.Type(), MessageType::contract_query);
    EXPECT_NE(original.Version(), 0);
    EXPECT_EQ(original.ID(), id);

    Requestor().expected_ += 1;

    EXPECT_TRUE(Requestor().request(original));
    ASSERT_TRUE(Requestor().wait());

    {
        const auto& msg = Requestor().get(++Requestor().checked_);
        const auto base = miner_.Factory().BlockchainSyncMessage(msg);

        ASSERT_TRUE(base);
        ASSERT_EQ(base->Type(), MessageType::contract);

        const auto& reply = base->asQueryContractReply();

        EXPECT_EQ(reply.ID(), id);
        EXPECT_EQ(reply.ContractType(), ot::contract::Type::notary);
        ASSERT_TRUE(opentxs::valid(reply.Payload()));
        EXPECT_EQ(expected.Bytes(), reply.Payload());
    }
}

TEST_F(Regtest_fixture_sync_server, query_unit)
{
    ASSERT_TRUE(unit_.has_value());

    const auto id = unit_.value()->ID();
    const auto expected = [&] {
        auto out = miner_.Factory().Data();
        unit_.value()->Serialize(out.WriteInto());

        return out;
    }();

    EXPECT_EQ(id.Type(), ot::identifier::Type::unitdefinition);

    const auto original = opentxs::factory::BlockchainSyncQueryContract(id);

    EXPECT_EQ(original.Type(), MessageType::contract_query);
    EXPECT_NE(original.Version(), 0);
    EXPECT_EQ(original.ID(), id);

    {
        const auto serialized = [&] {
            auto out = opentxs::network::zeromq::Message{};

            EXPECT_TRUE(original.Serialize(out));

            return out;
        }();

        EXPECT_EQ(serialized.size(), 3);

        const auto header = serialized.Header();
        const auto body = serialized.Body();

        EXPECT_EQ(header.size(), 0);
        EXPECT_EQ(body.size(), 2);

        auto recovered = miner_.Factory().BlockchainSyncMessage(serialized);

        ASSERT_TRUE(recovered);

        EXPECT_EQ(recovered->Type(), MessageType::contract_query);
        EXPECT_NE(recovered->Version(), 0);

        const auto& query = recovered->asQueryContract();

        EXPECT_EQ(query.Type(), MessageType::contract_query);
        EXPECT_NE(query.Version(), 0);
        EXPECT_EQ(query.ID(), id);
    }

    Requestor().expected_ += 1;

    EXPECT_TRUE(Requestor().request(original));
    ASSERT_TRUE(Requestor().wait());

    {
        const auto& msg = Requestor().get(++Requestor().checked_);
        const auto base = miner_.Factory().BlockchainSyncMessage(msg);

        ASSERT_TRUE(base);
        ASSERT_EQ(base->Type(), MessageType::contract);

        const auto& reply = base->asQueryContractReply();

        EXPECT_EQ(reply.ID(), id);
        EXPECT_EQ(reply.ContractType(), ot::contract::Type::unit);
        ASSERT_TRUE(opentxs::valid(reply.Payload()));
        EXPECT_EQ(expected.Bytes(), reply.Payload());
    }
}

TEST_F(Regtest_fixture_sync_server, pushtx)
{
    static const auto hex = ot::UnallocatedCString{
        "01000000000102fff7f7881a8099afa6940d42d1e7f6362bec38171ea3edf43354"
        "1db4e4ad969f00000000494830450221008b9d1dc26ba6a9cb62127b02742fa9d7"
        "54cd3bebf337f7a55d114c8e5cdd30be022040529b194ba3f9281a99f2b1c0a19c"
        "0489bc22ede944ccf4ecbab4cc618ef3ed01eeffffffef51e1b804cc89d182d279"
        "655c3aa89e815b1b309fe287d9b2b55d57b90ec68a0100000000ffffffff02202c"
        "b206000000001976a9148280b37df378db99f66f85c95a783a76ac7a6d5988ac90"
        "93510d000000001976a9143bde42dbee7e4dbe6a21b2d50ce2f0167faa815988ac"
        "000247304402203609e17b84f6a7d30c80bfa610b5b4542f32a8a0d5447a12fb13"
        "66d7f01cc44a0220573a954c4518331561406f90300e8f3358f51928d43c212a8c"
        "aed02de67eebee0121025476c2e83188368da1ff3e292e7acafcdb3566bb0ad253"
        "f62fc70f07aeee635711000000"};
    const auto data = miner_.Factory().DataFromHex(hex);
    const auto tx =
        miner_.Factory().BitcoinTransaction(test_chain_, data.Bytes(), false);

    ASSERT_TRUE(tx);

    const auto original =
        opentxs::factory::BlockchainSyncPushTransaction(test_chain_, *tx);

    EXPECT_EQ(original.Type(), MessageType::pushtx);
    EXPECT_NE(original.Version(), 0);
    EXPECT_EQ(original.Chain(), test_chain_);
    EXPECT_EQ(original.ID(), tx->ID());
    EXPECT_EQ(original.Payload(), data.Bytes());

    {
        const auto serialized = [&] {
            auto out = opentxs::network::zeromq::Message{};

            EXPECT_TRUE(original.Serialize(out));

            return out;
        }();

        EXPECT_EQ(serialized.size(), 5);

        const auto header = serialized.Header();
        const auto body = serialized.Body();

        EXPECT_EQ(header.size(), 0);
        EXPECT_EQ(body.size(), 4);

        auto recovered = miner_.Factory().BlockchainSyncMessage(serialized);

        ASSERT_TRUE(recovered);

        EXPECT_EQ(recovered->Type(), original.Type());
        EXPECT_EQ(recovered->Version(), original.Version());

        const auto& query = recovered->asPushTransaction();

        EXPECT_EQ(query.Type(), original.Type());
        EXPECT_EQ(query.Version(), original.Version());
        EXPECT_EQ(query.Chain(), original.Chain());
        EXPECT_EQ(query.ID(), original.ID());
        EXPECT_EQ(query.Payload(), original.Payload());
    }

    Requestor().expected_ += 1;

    EXPECT_TRUE(Requestor().request(original));
    ASSERT_TRUE(Requestor().wait());

    {
        const auto& msg = Requestor().get(++Requestor().checked_);
        const auto base = miner_.Factory().BlockchainSyncMessage(msg);

        ASSERT_TRUE(base);
        ASSERT_EQ(base->Type(), MessageType::pushtx_reply);

        const auto& reply = base->asPushTransactionReply();

        EXPECT_EQ(reply.Chain(), original.Chain());
        EXPECT_EQ(reply.ID(), original.ID());
        EXPECT_TRUE(reply.Success());
    }
}

TEST_F(Regtest_fixture_sync_server, shutdown) { Shutdown(); }
}  // namespace ottest
