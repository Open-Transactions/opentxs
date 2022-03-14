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
TEST_F(Test_HeaderOracle, checkpoint_prevents_update)
{
    EXPECT_TRUE(create_blocks(create_3_));

    const auto [height1, hash1] = header_oracle_.GetCheckpoint();

    EXPECT_EQ(height1, -1);
    EXPECT_EQ(hash1, ot::blockchain::block::BlankHash());
    EXPECT_TRUE(header_oracle_.AddCheckpoint(4, get_block_hash(BLOCK_8)));

    const auto [height2, hash2] = header_oracle_.GetCheckpoint();

    EXPECT_EQ(height2, 4);
    EXPECT_EQ(hash2, get_block_hash(BLOCK_8));

    EXPECT_TRUE(apply_blocks(sequence_3_));
    EXPECT_TRUE(verify_post_state(post_state_3_));
    EXPECT_TRUE(verify_siblings(siblings_3_));
}
}  // namespace ottest
