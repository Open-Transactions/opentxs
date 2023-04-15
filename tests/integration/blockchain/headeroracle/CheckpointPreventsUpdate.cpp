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
TEST_F(HeaderOracle, checkpoint_prevents_update)
{
    EXPECT_TRUE(CreateBlocks(block_sequence_3_));
    EXPECT_TRUE(AddCheckpoint(block_8_, 4));
    EXPECT_TRUE(ApplyBlockSequence(block_sequence_3_));
    EXPECT_TRUE(VerifyBestChain(block_sequence_3_, 0));
}
}  // namespace ottest
