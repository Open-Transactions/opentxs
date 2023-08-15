// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>

namespace ottest
{
using namespace opentxs::blockchain;
using enum opentxs::blockchain::Type;
using enum opentxs::blockchain::Category;

TEST(BlockchainData, Bitcoin)
{
    static constexpr auto chain = Bitcoin;

    EXPECT_EQ(associated_mainnet(chain), Bitcoin);
    EXPECT_EQ(category(chain), output_based);
    EXPECT_FALSE(is_descended_from(chain, UnknownBlockchain));
    EXPECT_TRUE(is_descended_from(chain, Bitcoin));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin_testnet3));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_ropsten));
    EXPECT_FALSE(is_descended_from(chain, Litecoin));
    EXPECT_FALSE(is_descended_from(chain, Litecoin_testnet4));
    EXPECT_FALSE(is_descended_from(chain, PKT));
    EXPECT_FALSE(is_descended_from(chain, PKT_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV_testnet3));
    EXPECT_FALSE(is_descended_from(chain, eCash));
    EXPECT_FALSE(is_descended_from(chain, eCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Casper));
    EXPECT_FALSE(is_descended_from(chain, Casper_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet4));
    EXPECT_FALSE(is_descended_from(chain, Dash));
    EXPECT_FALSE(is_descended_from(chain, Dash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_goerli));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_sepolia));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_holesovice));
    EXPECT_FALSE(is_descended_from(chain, UnitTest));
}

TEST(BlockchainData, Bitcoin_testnet3)
{
    static constexpr auto chain = Bitcoin_testnet3;

    EXPECT_EQ(associated_mainnet(chain), Bitcoin);
    EXPECT_EQ(category(chain), output_based);
    EXPECT_FALSE(is_descended_from(chain, UnknownBlockchain));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin));
    EXPECT_TRUE(is_descended_from(chain, Bitcoin_testnet3));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_ropsten));
    EXPECT_FALSE(is_descended_from(chain, Litecoin));
    EXPECT_FALSE(is_descended_from(chain, Litecoin_testnet4));
    EXPECT_FALSE(is_descended_from(chain, PKT));
    EXPECT_FALSE(is_descended_from(chain, PKT_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV_testnet3));
    EXPECT_FALSE(is_descended_from(chain, eCash));
    EXPECT_FALSE(is_descended_from(chain, eCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Casper));
    EXPECT_FALSE(is_descended_from(chain, Casper_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet4));
    EXPECT_FALSE(is_descended_from(chain, Dash));
    EXPECT_FALSE(is_descended_from(chain, Dash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_goerli));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_sepolia));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_holesovice));
    EXPECT_FALSE(is_descended_from(chain, UnitTest));
}

TEST(BlockchainData, BitcoinCash)
{
    static constexpr auto chain = BitcoinCash;

    EXPECT_EQ(associated_mainnet(chain), BitcoinCash);
    EXPECT_EQ(category(chain), output_based);
    EXPECT_FALSE(is_descended_from(chain, UnknownBlockchain));
    EXPECT_TRUE(is_descended_from(chain, Bitcoin));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin_testnet3));
    EXPECT_TRUE(is_descended_from(chain, BitcoinCash));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_ropsten));
    EXPECT_FALSE(is_descended_from(chain, Litecoin));
    EXPECT_FALSE(is_descended_from(chain, Litecoin_testnet4));
    EXPECT_FALSE(is_descended_from(chain, PKT));
    EXPECT_FALSE(is_descended_from(chain, PKT_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV_testnet3));
    EXPECT_FALSE(is_descended_from(chain, eCash));
    EXPECT_FALSE(is_descended_from(chain, eCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Casper));
    EXPECT_FALSE(is_descended_from(chain, Casper_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet4));
    EXPECT_FALSE(is_descended_from(chain, Dash));
    EXPECT_FALSE(is_descended_from(chain, Dash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_goerli));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_sepolia));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_holesovice));
    EXPECT_FALSE(is_descended_from(chain, UnitTest));
}

TEST(BlockchainData, BitcoinCash_testnet3)
{
    static constexpr auto chain = BitcoinCash_testnet3;

    EXPECT_EQ(associated_mainnet(chain), BitcoinCash);
    EXPECT_EQ(category(chain), output_based);
    EXPECT_FALSE(is_descended_from(chain, UnknownBlockchain));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin));
    EXPECT_TRUE(is_descended_from(chain, Bitcoin_testnet3));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash));
    EXPECT_TRUE(is_descended_from(chain, BitcoinCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_ropsten));
    EXPECT_FALSE(is_descended_from(chain, Litecoin));
    EXPECT_FALSE(is_descended_from(chain, Litecoin_testnet4));
    EXPECT_FALSE(is_descended_from(chain, PKT));
    EXPECT_FALSE(is_descended_from(chain, PKT_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV_testnet3));
    EXPECT_FALSE(is_descended_from(chain, eCash));
    EXPECT_FALSE(is_descended_from(chain, eCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Casper));
    EXPECT_FALSE(is_descended_from(chain, Casper_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet4));
    EXPECT_FALSE(is_descended_from(chain, Dash));
    EXPECT_FALSE(is_descended_from(chain, Dash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_goerli));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_sepolia));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_holesovice));
    EXPECT_FALSE(is_descended_from(chain, UnitTest));
}

TEST(BlockchainData, BitcoinCash_testnet4)
{
    static constexpr auto chain = BitcoinCash_testnet4;

    EXPECT_EQ(associated_mainnet(chain), BitcoinCash);
    EXPECT_EQ(category(chain), output_based);
    EXPECT_FALSE(is_descended_from(chain, UnknownBlockchain));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin_testnet3));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_ropsten));
    EXPECT_FALSE(is_descended_from(chain, Litecoin));
    EXPECT_FALSE(is_descended_from(chain, Litecoin_testnet4));
    EXPECT_FALSE(is_descended_from(chain, PKT));
    EXPECT_FALSE(is_descended_from(chain, PKT_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV_testnet3));
    EXPECT_FALSE(is_descended_from(chain, eCash));
    EXPECT_FALSE(is_descended_from(chain, eCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Casper));
    EXPECT_FALSE(is_descended_from(chain, Casper_testnet));
    EXPECT_TRUE(is_descended_from(chain, BitcoinCash_testnet4));
    EXPECT_FALSE(is_descended_from(chain, Dash));
    EXPECT_FALSE(is_descended_from(chain, Dash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_goerli));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_sepolia));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_holesovice));
    EXPECT_FALSE(is_descended_from(chain, UnitTest));
}

TEST(BlockchainData, BitcoinSV)
{
    static constexpr auto chain = BitcoinSV;

    EXPECT_EQ(associated_mainnet(chain), BitcoinSV);
    EXPECT_EQ(category(chain), output_based);
    EXPECT_FALSE(is_descended_from(chain, UnknownBlockchain));
    EXPECT_TRUE(is_descended_from(chain, Bitcoin));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin_testnet3));
    EXPECT_TRUE(is_descended_from(chain, BitcoinCash));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_ropsten));
    EXPECT_FALSE(is_descended_from(chain, Litecoin));
    EXPECT_FALSE(is_descended_from(chain, Litecoin_testnet4));
    EXPECT_FALSE(is_descended_from(chain, PKT));
    EXPECT_FALSE(is_descended_from(chain, PKT_testnet));
    EXPECT_TRUE(is_descended_from(chain, BitcoinSV));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV_testnet3));
    EXPECT_FALSE(is_descended_from(chain, eCash));
    EXPECT_FALSE(is_descended_from(chain, eCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Casper));
    EXPECT_FALSE(is_descended_from(chain, Casper_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet4));
    EXPECT_FALSE(is_descended_from(chain, Dash));
    EXPECT_FALSE(is_descended_from(chain, Dash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_goerli));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_sepolia));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_holesovice));
    EXPECT_FALSE(is_descended_from(chain, UnitTest));
}

TEST(BlockchainData, BitcoinSV_testnet3)
{
    static constexpr auto chain = BitcoinSV_testnet3;

    EXPECT_EQ(associated_mainnet(chain), BitcoinSV);
    EXPECT_EQ(category(chain), output_based);
    EXPECT_FALSE(is_descended_from(chain, UnknownBlockchain));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin));
    EXPECT_TRUE(is_descended_from(chain, Bitcoin_testnet3));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash));
    EXPECT_TRUE(is_descended_from(chain, BitcoinCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_ropsten));
    EXPECT_FALSE(is_descended_from(chain, Litecoin));
    EXPECT_FALSE(is_descended_from(chain, Litecoin_testnet4));
    EXPECT_FALSE(is_descended_from(chain, PKT));
    EXPECT_FALSE(is_descended_from(chain, PKT_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV));
    EXPECT_TRUE(is_descended_from(chain, BitcoinSV_testnet3));
    EXPECT_FALSE(is_descended_from(chain, eCash));
    EXPECT_FALSE(is_descended_from(chain, eCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Casper));
    EXPECT_FALSE(is_descended_from(chain, Casper_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet4));
    EXPECT_FALSE(is_descended_from(chain, Dash));
    EXPECT_FALSE(is_descended_from(chain, Dash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_goerli));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_sepolia));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_holesovice));
    EXPECT_FALSE(is_descended_from(chain, UnitTest));
}

TEST(BlockchainData, eCash)
{
    static constexpr auto chain = eCash;

    EXPECT_EQ(associated_mainnet(chain), eCash);
    EXPECT_EQ(category(chain), output_based);
    EXPECT_FALSE(is_descended_from(chain, UnknownBlockchain));
    EXPECT_TRUE(is_descended_from(chain, Bitcoin));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin_testnet3));
    EXPECT_TRUE(is_descended_from(chain, BitcoinCash));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_ropsten));
    EXPECT_FALSE(is_descended_from(chain, Litecoin));
    EXPECT_FALSE(is_descended_from(chain, Litecoin_testnet4));
    EXPECT_FALSE(is_descended_from(chain, PKT));
    EXPECT_FALSE(is_descended_from(chain, PKT_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV_testnet3));
    EXPECT_TRUE(is_descended_from(chain, eCash));
    EXPECT_FALSE(is_descended_from(chain, eCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Casper));
    EXPECT_FALSE(is_descended_from(chain, Casper_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet4));
    EXPECT_FALSE(is_descended_from(chain, Dash));
    EXPECT_FALSE(is_descended_from(chain, Dash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_goerli));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_sepolia));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_holesovice));
    EXPECT_FALSE(is_descended_from(chain, UnitTest));
}

TEST(BlockchainData, eCash_testnet3)
{
    static constexpr auto chain = eCash_testnet3;

    EXPECT_EQ(associated_mainnet(chain), eCash);
    EXPECT_EQ(category(chain), output_based);
    EXPECT_FALSE(is_descended_from(chain, UnknownBlockchain));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin));
    EXPECT_TRUE(is_descended_from(chain, Bitcoin_testnet3));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash));
    EXPECT_TRUE(is_descended_from(chain, BitcoinCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_ropsten));
    EXPECT_FALSE(is_descended_from(chain, Litecoin));
    EXPECT_FALSE(is_descended_from(chain, Litecoin_testnet4));
    EXPECT_FALSE(is_descended_from(chain, PKT));
    EXPECT_FALSE(is_descended_from(chain, PKT_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV_testnet3));
    EXPECT_FALSE(is_descended_from(chain, eCash));
    EXPECT_TRUE(is_descended_from(chain, eCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Casper));
    EXPECT_FALSE(is_descended_from(chain, Casper_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet4));
    EXPECT_FALSE(is_descended_from(chain, Dash));
    EXPECT_FALSE(is_descended_from(chain, Dash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_goerli));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_sepolia));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_holesovice));
    EXPECT_FALSE(is_descended_from(chain, UnitTest));
}

TEST(BlockchainData, Litecoin)
{
    static constexpr auto chain = Litecoin;

    EXPECT_EQ(associated_mainnet(chain), Litecoin);
    EXPECT_EQ(category(chain), output_based);
    EXPECT_FALSE(is_descended_from(chain, UnknownBlockchain));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin_testnet3));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_ropsten));
    EXPECT_TRUE(is_descended_from(chain, Litecoin));
    EXPECT_FALSE(is_descended_from(chain, Litecoin_testnet4));
    EXPECT_FALSE(is_descended_from(chain, PKT));
    EXPECT_FALSE(is_descended_from(chain, PKT_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV_testnet3));
    EXPECT_FALSE(is_descended_from(chain, eCash));
    EXPECT_FALSE(is_descended_from(chain, eCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Casper));
    EXPECT_FALSE(is_descended_from(chain, Casper_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet4));
    EXPECT_FALSE(is_descended_from(chain, Dash));
    EXPECT_FALSE(is_descended_from(chain, Dash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_goerli));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_sepolia));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_holesovice));
    EXPECT_FALSE(is_descended_from(chain, UnitTest));
}

TEST(BlockchainData, Litecoin_testnet4)
{
    static constexpr auto chain = Litecoin_testnet4;

    EXPECT_EQ(associated_mainnet(chain), Litecoin);
    EXPECT_EQ(category(chain), output_based);
    EXPECT_FALSE(is_descended_from(chain, UnknownBlockchain));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin_testnet3));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_ropsten));
    EXPECT_FALSE(is_descended_from(chain, Litecoin));
    EXPECT_TRUE(is_descended_from(chain, Litecoin_testnet4));
    EXPECT_FALSE(is_descended_from(chain, PKT));
    EXPECT_FALSE(is_descended_from(chain, PKT_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV_testnet3));
    EXPECT_FALSE(is_descended_from(chain, eCash));
    EXPECT_FALSE(is_descended_from(chain, eCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Casper));
    EXPECT_FALSE(is_descended_from(chain, Casper_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet4));
    EXPECT_FALSE(is_descended_from(chain, Dash));
    EXPECT_FALSE(is_descended_from(chain, Dash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_goerli));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_sepolia));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_holesovice));
    EXPECT_FALSE(is_descended_from(chain, UnitTest));
}

TEST(BlockchainData, PKT)
{
    static constexpr auto chain = PKT;

    EXPECT_EQ(associated_mainnet(chain), PKT);
    EXPECT_EQ(category(chain), output_based);
    EXPECT_FALSE(is_descended_from(chain, UnknownBlockchain));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin_testnet3));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_ropsten));
    EXPECT_FALSE(is_descended_from(chain, Litecoin));
    EXPECT_FALSE(is_descended_from(chain, Litecoin_testnet4));
    EXPECT_TRUE(is_descended_from(chain, PKT));
    EXPECT_FALSE(is_descended_from(chain, PKT_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV_testnet3));
    EXPECT_FALSE(is_descended_from(chain, eCash));
    EXPECT_FALSE(is_descended_from(chain, eCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Casper));
    EXPECT_FALSE(is_descended_from(chain, Casper_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet4));
    EXPECT_FALSE(is_descended_from(chain, Dash));
    EXPECT_FALSE(is_descended_from(chain, Dash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_goerli));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_sepolia));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_holesovice));
    EXPECT_FALSE(is_descended_from(chain, UnitTest));
}

TEST(BlockchainData, PKT_testnet)
{
    static constexpr auto chain = PKT_testnet;

    EXPECT_EQ(associated_mainnet(chain), PKT);
    EXPECT_EQ(category(chain), output_based);
    EXPECT_FALSE(is_descended_from(chain, UnknownBlockchain));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin_testnet3));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_ropsten));
    EXPECT_FALSE(is_descended_from(chain, Litecoin));
    EXPECT_FALSE(is_descended_from(chain, Litecoin_testnet4));
    EXPECT_FALSE(is_descended_from(chain, PKT));
    EXPECT_TRUE(is_descended_from(chain, PKT_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV_testnet3));
    EXPECT_FALSE(is_descended_from(chain, eCash));
    EXPECT_FALSE(is_descended_from(chain, eCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Casper));
    EXPECT_FALSE(is_descended_from(chain, Casper_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet4));
    EXPECT_FALSE(is_descended_from(chain, Dash));
    EXPECT_FALSE(is_descended_from(chain, Dash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_goerli));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_sepolia));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_holesovice));
    EXPECT_FALSE(is_descended_from(chain, UnitTest));
}

TEST(BlockchainData, Dash)
{
    static constexpr auto chain = Dash;

    EXPECT_EQ(associated_mainnet(chain), Dash);
    EXPECT_EQ(category(chain), output_based);
    EXPECT_FALSE(is_descended_from(chain, UnknownBlockchain));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin_testnet3));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_ropsten));
    EXPECT_FALSE(is_descended_from(chain, Litecoin));
    EXPECT_FALSE(is_descended_from(chain, Litecoin_testnet4));
    EXPECT_FALSE(is_descended_from(chain, PKT));
    EXPECT_FALSE(is_descended_from(chain, PKT_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV_testnet3));
    EXPECT_FALSE(is_descended_from(chain, eCash));
    EXPECT_FALSE(is_descended_from(chain, eCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Casper));
    EXPECT_FALSE(is_descended_from(chain, Casper_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet4));
    EXPECT_TRUE(is_descended_from(chain, Dash));
    EXPECT_FALSE(is_descended_from(chain, Dash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_goerli));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_sepolia));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_holesovice));
    EXPECT_FALSE(is_descended_from(chain, UnitTest));
}

TEST(BlockchainData, Dash_testnet3)
{
    static constexpr auto chain = Dash_testnet3;

    EXPECT_EQ(associated_mainnet(chain), Dash);
    EXPECT_EQ(category(chain), output_based);
    EXPECT_FALSE(is_descended_from(chain, UnknownBlockchain));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin_testnet3));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_ropsten));
    EXPECT_FALSE(is_descended_from(chain, Litecoin));
    EXPECT_FALSE(is_descended_from(chain, Litecoin_testnet4));
    EXPECT_FALSE(is_descended_from(chain, PKT));
    EXPECT_FALSE(is_descended_from(chain, PKT_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV_testnet3));
    EXPECT_FALSE(is_descended_from(chain, eCash));
    EXPECT_FALSE(is_descended_from(chain, eCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Casper));
    EXPECT_FALSE(is_descended_from(chain, Casper_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet4));
    EXPECT_FALSE(is_descended_from(chain, Dash));
    EXPECT_TRUE(is_descended_from(chain, Dash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_goerli));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_sepolia));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_holesovice));
    EXPECT_FALSE(is_descended_from(chain, UnitTest));
}

TEST(BlockchainData, UnitTest)
{
    static constexpr auto chain = UnitTest;

    EXPECT_EQ(associated_mainnet(chain), UnitTest);
    EXPECT_EQ(category(chain), output_based);
    EXPECT_FALSE(is_descended_from(chain, UnknownBlockchain));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin_testnet3));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_ropsten));
    EXPECT_FALSE(is_descended_from(chain, Litecoin));
    EXPECT_FALSE(is_descended_from(chain, Litecoin_testnet4));
    EXPECT_FALSE(is_descended_from(chain, PKT));
    EXPECT_FALSE(is_descended_from(chain, PKT_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV_testnet3));
    EXPECT_FALSE(is_descended_from(chain, eCash));
    EXPECT_FALSE(is_descended_from(chain, eCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Casper));
    EXPECT_FALSE(is_descended_from(chain, Casper_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet4));
    EXPECT_FALSE(is_descended_from(chain, Dash));
    EXPECT_FALSE(is_descended_from(chain, Dash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_goerli));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_sepolia));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_holesovice));
    EXPECT_TRUE(is_descended_from(chain, UnitTest));
}

TEST(BlockchainData, Ethereum)
{
    static constexpr auto chain = Ethereum;

    EXPECT_EQ(associated_mainnet(chain), Ethereum);
    EXPECT_EQ(category(chain), balance_based);
    EXPECT_FALSE(is_descended_from(chain, UnknownBlockchain));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin_testnet3));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet3));
    EXPECT_TRUE(is_descended_from(chain, Ethereum));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_ropsten));
    EXPECT_FALSE(is_descended_from(chain, Litecoin));
    EXPECT_FALSE(is_descended_from(chain, Litecoin_testnet4));
    EXPECT_FALSE(is_descended_from(chain, PKT));
    EXPECT_FALSE(is_descended_from(chain, PKT_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV_testnet3));
    EXPECT_FALSE(is_descended_from(chain, eCash));
    EXPECT_FALSE(is_descended_from(chain, eCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Casper));
    EXPECT_FALSE(is_descended_from(chain, Casper_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet4));
    EXPECT_FALSE(is_descended_from(chain, Dash));
    EXPECT_FALSE(is_descended_from(chain, Dash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_goerli));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_sepolia));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_holesovice));
    EXPECT_FALSE(is_descended_from(chain, UnitTest));
}

TEST(BlockchainData, Ethereum_ropsten)
{
    static constexpr auto chain = Ethereum_ropsten;

    EXPECT_EQ(associated_mainnet(chain), Ethereum);
    EXPECT_EQ(category(chain), balance_based);
    EXPECT_FALSE(is_descended_from(chain, UnknownBlockchain));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin_testnet3));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum));
    EXPECT_TRUE(is_descended_from(chain, Ethereum_ropsten));
    EXPECT_FALSE(is_descended_from(chain, Litecoin));
    EXPECT_FALSE(is_descended_from(chain, Litecoin_testnet4));
    EXPECT_FALSE(is_descended_from(chain, PKT));
    EXPECT_FALSE(is_descended_from(chain, PKT_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV_testnet3));
    EXPECT_FALSE(is_descended_from(chain, eCash));
    EXPECT_FALSE(is_descended_from(chain, eCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Casper));
    EXPECT_FALSE(is_descended_from(chain, Casper_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet4));
    EXPECT_FALSE(is_descended_from(chain, Dash));
    EXPECT_FALSE(is_descended_from(chain, Dash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_goerli));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_sepolia));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_holesovice));
    EXPECT_FALSE(is_descended_from(chain, UnitTest));
}

TEST(BlockchainData, Ethereum_goerli)
{
    static constexpr auto chain = Ethereum_goerli;

    EXPECT_EQ(associated_mainnet(chain), Ethereum);
    EXPECT_EQ(category(chain), balance_based);
    EXPECT_FALSE(is_descended_from(chain, UnknownBlockchain));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin_testnet3));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_ropsten));
    EXPECT_FALSE(is_descended_from(chain, Litecoin));
    EXPECT_FALSE(is_descended_from(chain, Litecoin_testnet4));
    EXPECT_FALSE(is_descended_from(chain, PKT));
    EXPECT_FALSE(is_descended_from(chain, PKT_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV_testnet3));
    EXPECT_FALSE(is_descended_from(chain, eCash));
    EXPECT_FALSE(is_descended_from(chain, eCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Casper));
    EXPECT_FALSE(is_descended_from(chain, Casper_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet4));
    EXPECT_FALSE(is_descended_from(chain, Dash));
    EXPECT_FALSE(is_descended_from(chain, Dash_testnet3));
    EXPECT_TRUE(is_descended_from(chain, Ethereum_goerli));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_sepolia));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_holesovice));
    EXPECT_FALSE(is_descended_from(chain, UnitTest));
}

TEST(BlockchainData, Ethereum_sepolia)
{
    static constexpr auto chain = Ethereum_sepolia;

    EXPECT_EQ(associated_mainnet(chain), Ethereum);
    EXPECT_EQ(category(chain), balance_based);
    EXPECT_FALSE(is_descended_from(chain, UnknownBlockchain));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin_testnet3));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_ropsten));
    EXPECT_FALSE(is_descended_from(chain, Litecoin));
    EXPECT_FALSE(is_descended_from(chain, Litecoin_testnet4));
    EXPECT_FALSE(is_descended_from(chain, PKT));
    EXPECT_FALSE(is_descended_from(chain, PKT_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV_testnet3));
    EXPECT_FALSE(is_descended_from(chain, eCash));
    EXPECT_FALSE(is_descended_from(chain, eCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Casper));
    EXPECT_FALSE(is_descended_from(chain, Casper_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet4));
    EXPECT_FALSE(is_descended_from(chain, Dash));
    EXPECT_FALSE(is_descended_from(chain, Dash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_goerli));
    EXPECT_TRUE(is_descended_from(chain, Ethereum_sepolia));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_holesovice));
    EXPECT_FALSE(is_descended_from(chain, UnitTest));
}

TEST(BlockchainData, Ethereum_holesovice)
{
    static constexpr auto chain = Ethereum_holesovice;

    EXPECT_EQ(associated_mainnet(chain), Ethereum);
    EXPECT_EQ(category(chain), balance_based);
    EXPECT_FALSE(is_descended_from(chain, UnknownBlockchain));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin_testnet3));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_ropsten));
    EXPECT_FALSE(is_descended_from(chain, Litecoin));
    EXPECT_FALSE(is_descended_from(chain, Litecoin_testnet4));
    EXPECT_FALSE(is_descended_from(chain, PKT));
    EXPECT_FALSE(is_descended_from(chain, PKT_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV_testnet3));
    EXPECT_FALSE(is_descended_from(chain, eCash));
    EXPECT_FALSE(is_descended_from(chain, eCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Casper));
    EXPECT_FALSE(is_descended_from(chain, Casper_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet4));
    EXPECT_FALSE(is_descended_from(chain, Dash));
    EXPECT_FALSE(is_descended_from(chain, Dash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_goerli));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_sepolia));
    EXPECT_TRUE(is_descended_from(chain, Ethereum_holesovice));
    EXPECT_FALSE(is_descended_from(chain, UnitTest));
}

TEST(BlockchainData, Casper)
{
    static constexpr auto chain = Casper;

    EXPECT_EQ(associated_mainnet(chain), Casper);
    EXPECT_EQ(category(chain), balance_based);
    EXPECT_FALSE(is_descended_from(chain, UnknownBlockchain));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin_testnet3));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_ropsten));
    EXPECT_FALSE(is_descended_from(chain, Litecoin));
    EXPECT_FALSE(is_descended_from(chain, Litecoin_testnet4));
    EXPECT_FALSE(is_descended_from(chain, PKT));
    EXPECT_FALSE(is_descended_from(chain, PKT_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV_testnet3));
    EXPECT_FALSE(is_descended_from(chain, eCash));
    EXPECT_FALSE(is_descended_from(chain, eCash_testnet3));
    EXPECT_TRUE(is_descended_from(chain, Casper));
    EXPECT_FALSE(is_descended_from(chain, Casper_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet4));
    EXPECT_FALSE(is_descended_from(chain, Dash));
    EXPECT_FALSE(is_descended_from(chain, Dash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_goerli));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_sepolia));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_holesovice));
    EXPECT_FALSE(is_descended_from(chain, UnitTest));
}

TEST(BlockchainData, Casper_testnet)
{
    static constexpr auto chain = Casper_testnet;

    EXPECT_EQ(associated_mainnet(chain), Casper);
    EXPECT_EQ(category(chain), balance_based);
    EXPECT_FALSE(is_descended_from(chain, UnknownBlockchain));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin));
    EXPECT_FALSE(is_descended_from(chain, Bitcoin_testnet3));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_ropsten));
    EXPECT_FALSE(is_descended_from(chain, Litecoin));
    EXPECT_FALSE(is_descended_from(chain, Litecoin_testnet4));
    EXPECT_FALSE(is_descended_from(chain, PKT));
    EXPECT_FALSE(is_descended_from(chain, PKT_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV));
    EXPECT_FALSE(is_descended_from(chain, BitcoinSV_testnet3));
    EXPECT_FALSE(is_descended_from(chain, eCash));
    EXPECT_FALSE(is_descended_from(chain, eCash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Casper));
    EXPECT_TRUE(is_descended_from(chain, Casper_testnet));
    EXPECT_FALSE(is_descended_from(chain, BitcoinCash_testnet4));
    EXPECT_FALSE(is_descended_from(chain, Dash));
    EXPECT_FALSE(is_descended_from(chain, Dash_testnet3));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_goerli));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_sepolia));
    EXPECT_FALSE(is_descended_from(chain, Ethereum_holesovice));
    EXPECT_FALSE(is_descended_from(chain, UnitTest));
}
}  // namespace ottest
