// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <memory>

#include "Helpers.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/util/Container.hpp"

namespace ottest
{
TEST_F(Test_HeaderOracle, basic_sequence)
{
    const auto [heightBefore, hashBefore] = header_oracle_.BestChain();

    EXPECT_EQ(heightBefore, 0);
    EXPECT_EQ(hashBefore, bc::HeaderOracle::GenesisBlockHash(type_));

    EXPECT_TRUE(create_blocks(create_1_));
    EXPECT_TRUE(apply_blocks(sequence_1_));
    EXPECT_TRUE(verify_post_state(post_state_1_));
    EXPECT_TRUE(verify_siblings(siblings_1_));

    {
        const auto existing =
            header_oracle_.LoadHeader(get_block_hash(BLOCK_7));

        ASSERT_TRUE(existing);

        {
            const auto data = header_oracle_.BestChain(existing->Position());

            ASSERT_EQ(data.size(), 4);
            EXPECT_EQ(data.at(0).second, get_block_hash(BLOCK_7));
            EXPECT_EQ(data.at(1).second, get_block_hash(BLOCK_8));
            EXPECT_EQ(data.at(2).second, get_block_hash(BLOCK_9));
            EXPECT_EQ(data.at(3).second, get_block_hash(BLOCK_10));
        }
        {
            const auto target =
                header_oracle_.LoadHeader(get_block_hash(BLOCK_9));

            ASSERT_TRUE(target);

            const auto data = header_oracle_.Ancestors(
                existing->Position(), target->Position());

            ASSERT_EQ(data.size(), 3);
            EXPECT_EQ(data.at(0).second, get_block_hash(BLOCK_7));
            EXPECT_EQ(data.at(1).second, get_block_hash(BLOCK_8));
            EXPECT_EQ(data.at(2).second, get_block_hash(BLOCK_9));
        }
        {
            const auto target =
                header_oracle_.LoadHeader(get_block_hash(BLOCK_5));

            ASSERT_TRUE(target);

            const auto data = header_oracle_.Ancestors(
                existing->Position(), target->Position());

            ASSERT_EQ(data.size(), 1);
            EXPECT_EQ(data.at(0).second, get_block_hash(BLOCK_5));
        }
        {
            const auto data = header_oracle_.Ancestors(
                existing->Position(), existing->Position());

            ASSERT_EQ(data.size(), 1);
            EXPECT_EQ(data.at(0).second, get_block_hash(BLOCK_7));
        }
    }
}
}  // namespace ottest
