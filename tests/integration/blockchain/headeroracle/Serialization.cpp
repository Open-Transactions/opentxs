// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <memory>

#include "ottest/data/blockchain/Headers.hpp"
#include "ottest/fixtures/blockchain/HeaderOracle.hpp"

namespace ottest
{
TEST_F(HeaderOracle, test_block_serialization)
{
    const auto empty = ot::blockchain::block::Hash{};

    ASSERT_TRUE(MakeTestBlock(block_1_, empty));

    const auto hash1 = GetBlockHash(block_1_);

    EXPECT_FALSE(hash1.IsNull());

    auto header = GetTestBlock(block_1_);

    ASSERT_TRUE(header);
    EXPECT_EQ(header->Hash(), hash1);
    EXPECT_EQ(header->ParentHash(), empty);

    auto space = ot::Space{};
    auto const bitcoinformat{false};
    ASSERT_TRUE(header->Serialize(ot::writer(space), bitcoinformat));
    header = api_.Factory().BlockHeader(ot::reader(space));

    ASSERT_TRUE(header);
    EXPECT_EQ(header->Hash(), hash1);
    EXPECT_EQ(header->ParentHash(), empty);
    ASSERT_TRUE(MakeTestBlock(block_2_, hash1));

    const auto hash2 = GetBlockHash(block_2_);

    EXPECT_FALSE(hash2.IsNull());

    header = GetTestBlock(block_2_);

    ASSERT_TRUE(header);
    EXPECT_EQ(header->Hash(), hash2);
    EXPECT_EQ(header->ParentHash(), hash1);

    ASSERT_TRUE(header->Serialize(ot::writer(space), bitcoinformat));
    header = api_.Factory().BlockHeader(ot::reader(space));

    ASSERT_TRUE(header);
    EXPECT_EQ(header->Hash(), hash2);
    EXPECT_EQ(header->ParentHash(), hash1);
}
}  // namespace ottest
