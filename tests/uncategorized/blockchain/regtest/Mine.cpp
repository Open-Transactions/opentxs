// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <chrono>
#include <compare>
#include <memory>

#include "internal/blockchain/bitcoin/block/Transaction.hpp"
#include "ottest/fixtures/blockchain/Common.hpp"
#include "ottest/fixtures/blockchain/regtest/Single.hpp"

namespace ottest
{
TEST_F(Regtest_fixture_single, init_opentxs) {}

TEST_F(Regtest_fixture_single, start_chains) { EXPECT_TRUE(Start()); }

TEST_F(Regtest_fixture_single, generate_block)
{
    const auto handle = miner_.Network().Blockchain().GetChain(test_chain_);

    ASSERT_TRUE(handle);

    const auto& network = handle.get();
    const auto& headerOracle = network.HeaderOracle();
    auto previousHeader = [&] {
        const auto genesis = headerOracle.LoadHeader(
            ot::blockchain::node::HeaderOracle::GenesisBlockHash(test_chain_));

        return genesis->as_Bitcoin();
    }();
    using OutputBuilder = ot::api::session::Factory::OutputBuilder;
    auto tx = miner_.Factory().BitcoinGenerationTransaction(
        test_chain_,
        previousHeader.Height() + 1,
        [&] {
            auto output = ot::UnallocatedVector<OutputBuilder>{};
            const auto text = ot::UnallocatedCString{"null"};
            const auto keys = ot::UnallocatedSet<ot::blockchain::crypto::Key>{};
            output.emplace_back(
                5000000000,
                miner_.Factory().BitcoinScriptNullData(test_chain_, {text}),
                keys);

            return output;
        }(),
        coinbase_fun_);

    ASSERT_TRUE(tx);

    {
        const auto& inputs = tx->Inputs();

        ASSERT_EQ(inputs.size(), 1);

        const auto& input = inputs.at(0);

        EXPECT_EQ(input.Coinbase().size(), 89);
    }

    {
        const auto serialized = [&] {
            auto output = miner_.Factory().Data();
            tx->Internal().Serialize(output.WriteInto());

            return output;
        }();

        ASSERT_GT(serialized.size(), 0);

        const auto recovered = miner_.Factory().BitcoinTransaction(
            test_chain_, serialized.Bytes(), true);

        ASSERT_TRUE(recovered);

        const auto serialized2 = [&] {
            auto output = miner_.Factory().Data();
            recovered->Internal().Serialize(output.WriteInto());

            return output;
        }();

        EXPECT_EQ(recovered->ID(), tx->ID());
        EXPECT_EQ(serialized, serialized2);
    }

    auto block = miner_.Factory().BitcoinBlock(
        previousHeader,
        tx,
        previousHeader.nBits(),
        {},
        previousHeader.Version(),
        [start{ot::Clock::now()}] {
            return (ot::Clock::now() - start) > std::chrono::minutes(1);
        });

    ASSERT_TRUE(block);

    const auto serialized = [&] {
        auto output = miner_.Factory().Data();
        block->Serialize(output.WriteInto());

        return output;
    }();

    ASSERT_GT(serialized.size(), 0);

    const auto recovered =
        miner_.Factory().BitcoinBlock(test_chain_, serialized.Bytes());

    EXPECT_TRUE(recovered);
}

TEST_F(Regtest_fixture_single, shutdown) { Shutdown(); }
}  // namespace ottest
