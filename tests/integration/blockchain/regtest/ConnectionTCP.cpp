// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <memory>

#include "ottest/fixtures/blockchain/regtest/TCP.hpp"

namespace ottest
{
TEST_F(Regtest_fixture_tcp, init_opentxs) {}

TEST_F(Regtest_fixture_tcp, start_chains) { EXPECT_TRUE(Start()); }

TEST_F(Regtest_fixture_tcp, connect_peers) { EXPECT_TRUE(Connect()); }

TEST_F(Regtest_fixture_tcp, mine_blocks) { EXPECT_TRUE(Mine(0, 10)); }

TEST_F(Regtest_fixture_tcp, shutdown) { Shutdown(); }
}  // namespace ottest
