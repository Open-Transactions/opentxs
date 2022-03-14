// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <memory>

#include "Helpers.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"

namespace ottest
{
TEST_F(Test_HeaderOracle, checkpoint_prevents_reorg)
{
    EXPECT_TRUE(create_blocks(create_4_));
    EXPECT_TRUE(header_oracle_.AddCheckpoint(4, get_block_hash(BLOCK_4)));

    const auto [cpHeight, cpHash] = header_oracle_.GetCheckpoint();

    EXPECT_EQ(cpHeight, 4);
    EXPECT_EQ(cpHash, get_block_hash(BLOCK_4));

    EXPECT_TRUE(apply_blocks(sequence_4_));
    EXPECT_TRUE(verify_post_state(post_state_4_));
    EXPECT_TRUE(verify_siblings(siblings_4_));
}
}  // namespace ottest
