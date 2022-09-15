// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <chrono>
#include <compare>

#include "internal/util/P0330.hpp"
#include "ottest/fixtures/blockchain/Common.hpp"
#include "ottest/fixtures/blockchain/regtest/Single.hpp"

namespace ottest
{
using namespace std::literals::chrono_literals;
using namespace opentxs::literals;

TEST_F(Regtest_fixture_single, init_opentxs) {}

TEST_F(Regtest_fixture_single, start_chains) { EXPECT_TRUE(Start()); }

TEST_F(Regtest_fixture_single, connect_peers) { EXPECT_TRUE(Connect()); }

TEST_F(Regtest_fixture_single, client_disconnection_timeout)
{
    EXPECT_TRUE(client_1_.Network().Blockchain().Stop(test_chain_));

    const auto start = ot::Clock::now();
    const auto limit = 90s;
    const auto handle = miner_.Network().Blockchain().GetChain(test_chain_);

    ASSERT_TRUE(handle);

    const auto& chain = handle.get();
    auto count = 1_uz;

    while ((ot::Clock::now() - start) < limit) {
        count = chain.GetPeerCount();

        if (0u == count) { break; }

        ot::Sleep(1s);
    }

    EXPECT_EQ(count, 1);
}

TEST_F(Regtest_fixture_single, shutdown) { Shutdown(); }
}  // namespace ottest
