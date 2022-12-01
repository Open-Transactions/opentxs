// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <memory>

#include "ottest/fixtures/blockchain/StartStop.hpp"

namespace ottest
{
TEST_F(Test_StartStop, init_opentxs) {}

TEST_F(Test_StartStop, all)
{
    using enum opentxs::blockchain::Type;
    EXPECT_TRUE(api_.Network().Blockchain().Start(Bitcoin));
    EXPECT_TRUE(api_.Network().Blockchain().Start(Bitcoin_testnet3));
    EXPECT_TRUE(api_.Network().Blockchain().Start(BitcoinCash));
    EXPECT_TRUE(api_.Network().Blockchain().Start(BitcoinCash_testnet3));
    EXPECT_FALSE(api_.Network().Blockchain().Start(Ethereum_frontier));
    EXPECT_FALSE(api_.Network().Blockchain().Start(Ethereum_ropsten));
    EXPECT_TRUE(api_.Network().Blockchain().Start(Litecoin));
    EXPECT_TRUE(api_.Network().Blockchain().Start(Litecoin_testnet4));
    EXPECT_TRUE(api_.Network().Blockchain().Start(PKT));
    EXPECT_TRUE(api_.Network().Blockchain().Start(UnitTest));
    EXPECT_TRUE(api_.Network().Blockchain().Stop(UnitTest));
    EXPECT_TRUE(api_.Network().Blockchain().Stop(PKT));
    EXPECT_TRUE(api_.Network().Blockchain().Stop(Litecoin_testnet4));
    EXPECT_TRUE(api_.Network().Blockchain().Stop(Litecoin));
    EXPECT_TRUE(api_.Network().Blockchain().Stop(Ethereum_ropsten));
    EXPECT_TRUE(api_.Network().Blockchain().Stop(Ethereum_frontier));
    EXPECT_TRUE(api_.Network().Blockchain().Stop(BitcoinCash_testnet3));
    EXPECT_TRUE(api_.Network().Blockchain().Stop(BitcoinCash));
    EXPECT_TRUE(api_.Network().Blockchain().Stop(Bitcoin_testnet3));
    EXPECT_TRUE(api_.Network().Blockchain().Stop(Bitcoin));

    // TODO: Add BSV
}
}  // namespace ottest
