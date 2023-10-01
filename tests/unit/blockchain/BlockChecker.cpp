// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>

#include "ottest/data/blockchain/Blocks.hpp"
#include "ottest/fixtures/blockchain/Blocks.hpp"

namespace ottest
{
using enum opentxs::blockchain::Type;

TEST_F(BlockchainBlocks, check_genesis)
{
    EXPECT_TRUE(CheckGenesisBlock(Bitcoin));
    EXPECT_TRUE(CheckGenesisBlock(Bitcoin_testnet3));
    EXPECT_TRUE(CheckGenesisBlock(BitcoinCash));
    EXPECT_TRUE(CheckGenesisBlock(BitcoinCash_testnet3));
    EXPECT_TRUE(CheckGenesisBlock(Litecoin));
    EXPECT_TRUE(CheckGenesisBlock(Litecoin_testnet4));
    EXPECT_TRUE(CheckGenesisBlock(PKT));
    EXPECT_TRUE(CheckGenesisBlock(BitcoinSV));
    EXPECT_TRUE(CheckGenesisBlock(BitcoinSV_testnet3));
    EXPECT_TRUE(CheckGenesisBlock(eCash));
    EXPECT_TRUE(CheckGenesisBlock(eCash_testnet3));
    EXPECT_TRUE(CheckGenesisBlock(Dash));
    EXPECT_TRUE(CheckGenesisBlock(Dash_testnet3));
    EXPECT_TRUE(CheckGenesisBlock(UnitTest));
}

TEST_F(BlockchainBlocks, btc_block_762580)
{
    const auto& [id, good] = GetBtcBlock762580();
    const auto badHeader = GetBtcBlock762580_bad_header();
    const auto badTxid = GetBtcBlock762580_bad_txid();
    const auto badWtxid = GetBtcBlock762580_bad_wtxid();
    const auto& api = ot_.StartClientSession(0);
    
    EXPECT_TRUE(CheckBlock(Bitcoin, id, good));
    EXPECT_TRUE(CheckTxids(api, Bitcoin, good));
    EXPECT_FALSE(CheckBlock(Ethereum, id, good));
    EXPECT_FALSE(CheckBlock(PKT, id, good));
    EXPECT_FALSE(CheckBlock(Bitcoin, id, badHeader));
    EXPECT_FALSE(CheckBlock(Bitcoin, id, badTxid));
    EXPECT_FALSE(CheckBlock(Bitcoin, id, badWtxid));
}

TEST_F(BlockchainBlocks, tn_btc_block_1489260)
{
    const auto& [id, good] = GetTnBtcBlock1489260();
    const auto& api = ot_.StartClientSession(0);
    
    EXPECT_TRUE(CheckBlock(Bitcoin_testnet3, id, good));
    EXPECT_TRUE(CheckTxids(api, Bitcoin_testnet3, good));
}

TEST_F(BlockchainBlocks, tn_dash_block_7000)
{
    const auto& [id, good] = GetTnDashBlock7000();
    const auto& api = ot_.StartClientSession(0);

    EXPECT_TRUE(CheckBlock(Dash_testnet3, id, good));
    EXPECT_TRUE(CheckTxids(api, Dash_testnet3, good));
}
}  // namespace ottest
