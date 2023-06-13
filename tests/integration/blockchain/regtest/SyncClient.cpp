// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <memory>

#include "ottest/fixtures/blockchain/Common.hpp"
#include "ottest/fixtures/blockchain/MinedBlocks.hpp"
#include "ottest/fixtures/blockchain/regtest/SyncClient.hpp"

namespace ottest
{
TEST_F(Regtest_fixture_sync_client, init_opentxs) {}

TEST_F(Regtest_fixture_sync_client, start_chains) { EXPECT_TRUE(Start()); }

TEST_F(Regtest_fixture_sync_client, connect_peers) { EXPECT_TRUE(Connect()); }

TEST_F(Regtest_fixture_sync_client, mine)
{
    constexpr auto count{10};

    EXPECT_TRUE(Mine(0, count));

    const auto handle = client_1_.Network().Blockchain().GetChain(test_chain_);

    ASSERT_TRUE(handle);

    const auto& chain = handle.get();
    const auto best = chain.HeaderOracle().BestChain();

    EXPECT_EQ(best.height_, 10);
    EXPECT_EQ(best.hash_, mined_blocks_.get(9).get());
}

TEST_F(Regtest_fixture_sync_client, shutdown) { Shutdown(); }
}  // namespace ottest
