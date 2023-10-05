// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "ottest/fixtures/blockchain/SyncListener.hpp"
// IWYU pragma: no_include "ottest/fixtures/blockchain/TXOState.hpp"

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>

#include "ottest/fixtures/blockchain/regtest/Single.hpp"

namespace ottest
{
TEST_F(Regtest_fixture_single, init_opentxs) {}

TEST_F(Regtest_fixture_single, start_chains) { EXPECT_TRUE(Start()); }

TEST_F(Regtest_fixture_single, connect_peers) { EXPECT_TRUE(Connect()); }

TEST_F(Regtest_fixture_single, mine) { EXPECT_TRUE(Mine(0, 10)); }

TEST_F(Regtest_fixture_single, extend) { EXPECT_TRUE(Mine(10, 5)); }

TEST_F(Regtest_fixture_single, reorg) { EXPECT_TRUE(Mine(12, 5)); }

TEST_F(Regtest_fixture_single, shutdown) { Shutdown(); }
}  // namespace ottest
