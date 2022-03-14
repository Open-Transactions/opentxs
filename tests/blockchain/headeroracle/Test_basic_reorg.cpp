// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <iostream>
#include <stdexcept>
#include <utility>

#include "Helpers.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/util/Container.hpp"

namespace ottest
{
TEST_F(Test_HeaderOracle, basic_reorg)
{
    EXPECT_TRUE(create_blocks(create_2_));
    EXPECT_TRUE(apply_blocks(sequence_2_));
    EXPECT_TRUE(verify_post_state(post_state_2_));
    EXPECT_TRUE(verify_siblings(siblings_2_));
    {
        auto gotException{false};

        try {
            const auto tip = bb::Position{4, get_block_hash(BLOCK_6)};
            const auto reorg = header_oracle_.CalculateReorg(tip);

            ASSERT_EQ(reorg.size(), 2);

            auto it = reorg.cbegin();

            {
                const auto& position = *(it++);

                EXPECT_EQ(position.first, tip.first);
                EXPECT_EQ(position.second, tip.second);
            }

            {
                const auto& position = *(it++);
                const auto parent = bb::Position{3, get_block_hash(BLOCK_5)};

                EXPECT_EQ(position.first, parent.first);
                EXPECT_EQ(position.second, parent.second);
            }
        } catch (const std::exception& e) {
            std::cout << e.what() << '\n';
            gotException = true;
        }

        EXPECT_FALSE(gotException);
    }
    {
        const auto tip = bb::Position{3, get_block_hash(BLOCK_4)};
        auto gotException{false};

        try {
            const auto reorg = header_oracle_.CalculateReorg(tip);

            ASSERT_EQ(reorg.size(), 1);

            auto it = reorg.cbegin();

            {
                const auto& position = *(it++);

                EXPECT_EQ(position.first, tip.first);
                EXPECT_EQ(position.second, tip.second);
            }
        } catch (const std::exception& e) {
            std::cout << e.what() << '\n';
            gotException = true;
        }

        EXPECT_FALSE(gotException);
    }
    {
        const auto tip = bb::Position{5, get_block_hash(BLOCK_8)};
        auto gotException{false};

        try {
            const auto reorg = header_oracle_.CalculateReorg(tip);

            EXPECT_EQ(reorg.size(), 0);
        } catch (const std::exception& e) {
            std::cout << e.what() << '\n';
            gotException = true;
        }

        EXPECT_FALSE(gotException);
    }
    {
        const auto tip = make_position(6, BLOCK_9);
        auto gotException{false};

        try {
            header_oracle_.CalculateReorg(tip);
        } catch (...) {
            gotException = true;
        }

        EXPECT_TRUE(gotException);
    }
    {
        const auto tip = make_position(6, BLOCK_8);
        auto gotException{false};

        try {
            header_oracle_.CalculateReorg(tip);
        } catch (...) {
            gotException = true;
        }

        EXPECT_TRUE(gotException);
    }
}
}  // namespace ottest
