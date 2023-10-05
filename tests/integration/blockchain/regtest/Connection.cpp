// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <chrono>

#include "ottest/fixtures/blockchain/regtest/Single.hpp"

namespace ottest
{
using namespace std::literals::chrono_literals;
using namespace opentxs::literals;
using namespace std::literals;

TEST_F(Regtest_fixture_single, init_opentxs) {}

TEST_F(Regtest_fixture_single, start_chains) { EXPECT_TRUE(Start()); }

TEST_F(Regtest_fixture_single, connect_peers) { EXPECT_TRUE(Connect()); }

TEST_F(Regtest_fixture_single, mine_blocks) { EXPECT_TRUE(Mine(0, 10)); }

TEST_F(Regtest_fixture_single, shutdown) { Shutdown(); }
}  // namespace ottest
