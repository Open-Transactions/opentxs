// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <memory>

#include "ottest/data/blockchain/Headers.hpp"
#include "ottest/fixtures/blockchain/HeaderOracle.hpp"

namespace ottest
{
TEST_F(HeaderOracle, add_checkpoint_disconnected)
{
    EXPECT_TRUE(CreateBlocks(block_sequence_9_));
    EXPECT_TRUE(ApplyBlockSequence(block_sequence_9_));
    EXPECT_TRUE(VerifyBestChain(block_sequence_9_, 0));
    EXPECT_TRUE(AddCheckpoint(block_9_, 2));
    EXPECT_TRUE(VerifyBestChain(block_sequence_9_, 1));
}
}  // namespace ottest
