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
TEST_F(Test_HeaderOracle, add_checkpoint_already_in_best_chain)
{
    EXPECT_TRUE(create_blocks(create_6_));
    EXPECT_TRUE(apply_blocks(sequence_6_));
    EXPECT_TRUE(verify_post_state(post_state_1_));
    EXPECT_TRUE(verify_best_chain(best_chain_1_));
    EXPECT_TRUE(verify_siblings(siblings_1_));

    EXPECT_TRUE(header_oracle_.AddCheckpoint(6, get_block_hash(BLOCK_6)));

    const auto [height, hash] = header_oracle_.GetCheckpoint();

    EXPECT_EQ(height, 6);
    EXPECT_EQ(hash, get_block_hash(BLOCK_6));
    EXPECT_TRUE(verify_post_state(post_state_6_));
    EXPECT_TRUE(verify_best_chain(best_chain_6_));
    EXPECT_TRUE(verify_siblings(siblings_6_));
}
}  // namespace ottest
