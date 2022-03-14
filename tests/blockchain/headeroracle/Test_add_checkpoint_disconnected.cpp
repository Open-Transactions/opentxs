// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <memory>
#include <utility>

#include "Helpers.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/core/Data.hpp"

namespace ottest
{
TEST_F(Test_HeaderOracle, add_checkpoint_disconnected)
{
    EXPECT_TRUE(create_blocks(create_9_));
    EXPECT_TRUE(apply_blocks(sequence_9_));
    EXPECT_TRUE(verify_post_state(post_state_2_));
    EXPECT_TRUE(verify_siblings(siblings_2_));

    EXPECT_TRUE(header_oracle_.AddCheckpoint(2, get_block_hash(BLOCK_9)));

    const auto [height, hash] = header_oracle_.GetCheckpoint();

    EXPECT_EQ(height, 2);
    EXPECT_EQ(hash->asHex(), get_block_hash(BLOCK_9)->asHex());
    EXPECT_TRUE(verify_post_state(post_state_9_));
    EXPECT_TRUE(verify_best_chain(best_chain_9_));
    EXPECT_TRUE(verify_siblings(siblings_9_));
}
}  // namespace ottest
