// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/rpc/Helpers.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <utility>

#include "ottest/fixtures/blockchain/Common.hpp"
#include "ottest/fixtures/blockchain/ScanListener.hpp"
#include "ottest/fixtures/rpc/SendPayment_blockchain.hpp"

namespace ot = opentxs;

namespace ottest
{
TEST_F(RPC_BC, preconditions)
{
    ASSERT_TRUE(Start());
    ASSERT_TRUE(Connect());

    {
        auto future1 = listener_.get_future(account_, Subchain::External, 11);
        auto future2 = listener_.get_future(account_, Subchain::Internal, 11);

        EXPECT_TRUE(Mine(0, 1, mine_to_alex_));
        EXPECT_TRUE(Mine(1, 10));
        EXPECT_TRUE(listener_.wait(future1));
        EXPECT_TRUE(listener_.wait(future2));
    }
}

TEST_F(RPC_BC, blockchain_payment)
{
    const auto index{client_1_.Instance()};
    constexpr auto address{"n4VQ5YdHf7hLQ2gWQYYrcxoE5B7nWuDFNF"};
    const auto amount = ot::Amount{140000};
    const auto account =
        account_.Parent().AccountID().asBase58(client_1_.Crypto());
    const auto command =
        ot::rpc::request::SendPayment{index, account, address, amount};
    const auto& send = command.asSendPayment();
    const auto base = RPC_fixture::ot_.RPC(command);
    const auto& response = base->asSendPayment();
    const auto& codes = response.ResponseCodes();
    const auto& pending = response.Pending();

    EXPECT_EQ(command.AssociatedNyms().size(), 0);
    EXPECT_NE(command.Cookie().size(), 0);
    EXPECT_EQ(command.Session(), index);
    EXPECT_EQ(command.Type(), ot::rpc::CommandType::send_payment);
    EXPECT_NE(command.Version(), 0);
    EXPECT_EQ(send.Amount(), amount);
    EXPECT_EQ(send.AssociatedNyms().size(), 0);
    EXPECT_EQ(send.Cookie(), command.Cookie());
    EXPECT_EQ(send.DestinationAccount(), address);
    EXPECT_EQ(send.Session(), command.Session());
    EXPECT_EQ(send.SourceAccount(), account);
    EXPECT_EQ(send.Type(), command.Type());
    EXPECT_EQ(send.Version(), command.Version());
    EXPECT_EQ(base->Cookie(), command.Cookie());
    EXPECT_EQ(base->Session(), command.Session());
    EXPECT_EQ(base->Type(), command.Type());
    EXPECT_EQ(base->Version(), command.Version());
    EXPECT_EQ(response.Cookie(), base->Cookie());
    EXPECT_EQ(response.ResponseCodes(), base->ResponseCodes());
    EXPECT_EQ(response.Session(), base->Session());
    EXPECT_EQ(response.Type(), base->Type());
    EXPECT_EQ(response.Version(), base->Version());
    EXPECT_EQ(response.Version(), command.Version());
    EXPECT_EQ(response.Cookie(), command.Cookie());
    EXPECT_EQ(response.Session(), command.Session());
    EXPECT_EQ(response.Type(), command.Type());
    ASSERT_EQ(codes.size(), 1);
    EXPECT_EQ(codes.at(0).first, 0);
    EXPECT_EQ(codes.at(0).second, ot::rpc::ResponseCode::txid);
    ASSERT_EQ(pending.size(), 1);
    ASSERT_EQ(codes.size(), 1);
    EXPECT_EQ(pending.at(0).first, 0);
    EXPECT_NE(pending.at(0).second.size(), 0);

    transactions_.emplace_back(ot::IsHex, pending.at(0).second);
}

TEST_F(RPC_BC, postconditions)
{
    const auto handle = client_1_.Network().Blockchain().GetChain(test_chain_);

    ASSERT_TRUE(handle);

    const auto& network = handle.get();
    const auto [confirmed, unconfirmed] = network.GetBalance(alex_.ID());

    EXPECT_EQ(confirmed, 10000000000);
    EXPECT_EQ(unconfirmed, 9999859774);
}

TEST_F(RPC_BC, cleanup) { Cleanup(); }
}  // namespace ottest
