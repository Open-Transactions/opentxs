// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <memory>

#include "ottest/fixtures/blockchain/regtest/ZMQ.hpp"

namespace ottest
{
TEST_F(Regtest_fixture_zmq, init_opentxs)
{
    using enum ot::network::blockchain::Transport;

    EXPECT_EQ(zmq_listen_address_.Type(), zmq);
    EXPECT_EQ(zmq_listen_address_.Subtype(), ipv4);
}

TEST_F(Regtest_fixture_zmq, start_chains) { EXPECT_TRUE(Start()); }

TEST_F(Regtest_fixture_zmq, connect_peers) { EXPECT_TRUE(Connect()); }

TEST_F(Regtest_fixture_zmq, mine_blocks) { EXPECT_TRUE(Mine(0, 10)); }

TEST_F(Regtest_fixture_zmq, shutdown) { Shutdown(); }
}  // namespace ottest
