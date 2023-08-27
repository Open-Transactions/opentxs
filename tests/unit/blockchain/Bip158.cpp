// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <boost/container/flat_map.hpp>
#include <boost/container/vector.hpp>
#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <functional>
#include <optional>
#include <span>
#include <string_view>

#include "internal/blockchain/Blockchain.hpp"
#include "internal/util/P0330.hpp"
#include "ottest/data/blockchain/Bip158.hpp"
#include "ottest/fixtures/blockchain/BIP158.hpp"
#include "ottest/fixtures/blockchain/Basic.hpp"

namespace ottest
{
using enum ot::blockchain::Type;
using enum ot::blockchain::cfilter::Type;

TEST_F(BIP158, init) {}

TEST_F(BIP158, regtest)
{
    EXPECT_TRUE(GenerateGenesisFilter(UnitTest, Basic_BIP158));
    EXPECT_TRUE(GenerateGenesisFilter(UnitTest, Basic_BCHVariant));
    EXPECT_TRUE(GenerateGenesisFilter(UnitTest, ES));

    api_.Network().Blockchain().Stop(UnitTest);
}

TEST_F(BIP158, btc_genesis_mainnet)
{
    EXPECT_TRUE(GenerateGenesisFilter(Bitcoin, Basic_BIP158));
    EXPECT_TRUE(GenerateGenesisFilter(Bitcoin, ES));

    api_.Network().Blockchain().Stop(Bitcoin);
}

TEST_F(BIP158, btc_genesis_testnet)
{
    EXPECT_TRUE(GenerateGenesisFilter(Bitcoin_testnet3, Basic_BIP158));
    EXPECT_TRUE(GenerateGenesisFilter(Bitcoin_testnet3, ES));

    api_.Network().Blockchain().Stop(Bitcoin_testnet3);
}

TEST_F(BIP158, bch_genesis_mainnet)
{
    EXPECT_TRUE(GenerateGenesisFilter(BitcoinCash, Basic_BCHVariant));
    EXPECT_TRUE(GenerateGenesisFilter(BitcoinCash, ES));

    api_.Network().Blockchain().Stop(BitcoinCash);
}

TEST_F(BIP158, bch_genesis_testnet3)
{
    EXPECT_TRUE(GenerateGenesisFilter(BitcoinCash_testnet3, Basic_BCHVariant));
    EXPECT_TRUE(GenerateGenesisFilter(BitcoinCash_testnet3, ES));

    api_.Network().Blockchain().Stop(BitcoinCash_testnet3);
}

TEST_F(BIP158, bch_genesis_testnet4)
{
    EXPECT_TRUE(GenerateGenesisFilter(BitcoinCash_testnet4, Basic_BCHVariant));
    EXPECT_TRUE(GenerateGenesisFilter(BitcoinCash_testnet4, ES));

    api_.Network().Blockchain().Stop(BitcoinCash_testnet4);
}

TEST_F(BIP158, ltc_genesis_mainnet)
{
    EXPECT_TRUE(GenerateGenesisFilter(Litecoin, ES));

    api_.Network().Blockchain().Stop(Litecoin);
}

TEST_F(BIP158, ltc_genesis_testnet)
{
    EXPECT_TRUE(GenerateGenesisFilter(Litecoin_testnet4, ES));

    api_.Network().Blockchain().Stop(Litecoin_testnet4);
}

TEST_F(BIP158, pkt_mainnet)
{
    constexpr auto chain = PKT;

    EXPECT_TRUE(GenerateGenesisFilter(chain, Basic_BIP158));
    EXPECT_TRUE(GenerateGenesisFilter(chain, ES));

    const auto& [genesisHex, filterMap] = genesis_block_data_.at(chain);
    const auto bytes = api_.Factory().DataFromHex(genesisHex);
    const auto block = api_.Factory().BlockchainBlock(chain, bytes.Bytes(), {});

    EXPECT_TRUE(block.IsValid());

    auto raw = api_.Factory().Data();

    EXPECT_TRUE(block.Serialize(raw.WriteInto()));
    EXPECT_EQ(raw, bytes);

    api_.Network().Blockchain().Stop(chain);
}

TEST_F(BIP158, bsv_genesis_mainnet)
{
    EXPECT_TRUE(GenerateGenesisFilter(BitcoinSV, Basic_BCHVariant));
    EXPECT_TRUE(GenerateGenesisFilter(BitcoinSV, ES));

    api_.Network().Blockchain().Stop(BitcoinSV);
}

TEST_F(BIP158, bsv_genesis_testnet)
{
    EXPECT_TRUE(GenerateGenesisFilter(BitcoinSV_testnet3, Basic_BCHVariant));
    EXPECT_TRUE(GenerateGenesisFilter(BitcoinSV_testnet3, ES));

    api_.Network().Blockchain().Stop(BitcoinSV_testnet3);
}

TEST_F(BIP158, xec_genesis_mainnet)
{
    EXPECT_TRUE(GenerateGenesisFilter(eCash, Basic_BCHVariant));
    EXPECT_TRUE(GenerateGenesisFilter(eCash, ES));

    api_.Network().Blockchain().Stop(eCash);
}

TEST_F(BIP158, xec_genesis_testnet)
{
    EXPECT_TRUE(GenerateGenesisFilter(eCash_testnet3, Basic_BCHVariant));
    EXPECT_TRUE(GenerateGenesisFilter(eCash_testnet3, ES));

    api_.Network().Blockchain().Stop(eCash_testnet3);
}

TEST_F(BIP158, dash_genesis_mainnet)
{
    EXPECT_TRUE(GenerateGenesisFilter(Dash, Basic_BIP158));
    EXPECT_TRUE(GenerateGenesisFilter(Dash, ES));

    api_.Network().Blockchain().Stop(Dash);
}

TEST_F(BIP158, dash_genesis_testnet)
{
    EXPECT_TRUE(GenerateGenesisFilter(Dash_testnet3, Basic_BIP158));
    EXPECT_TRUE(GenerateGenesisFilter(Dash_testnet3, ES));

    api_.Network().Blockchain().Stop(Dash_testnet3);
}

TEST_F(BIP158, bip158)
{
    for (const auto& vector : GetBip158Vectors()) {
        const auto raw = vector.Block(api_);
        const auto block =
            api_.Factory().BlockchainBlock(Bitcoin_testnet3, raw.Bytes(), {});

        EXPECT_TRUE(block.IsValid());

        EXPECT_EQ(block.ID(), vector.BlockHash(api_));

        const auto encodedFilter = vector.Filter(api_);
        using namespace opentxs::literals;
        using namespace std::literals;
        auto encodedElements = 0_uz;

        {
            auto bytes = encodedFilter.Bytes();
            const auto decoded =
                opentxs::network::blockchain::bitcoin::DecodeCompactSize(bytes);

            ASSERT_TRUE(decoded.has_value());

            encodedElements = *decoded;
        }

        static const auto params =
            ot::blockchain::internal::GetFilterParams(Basic_BIP158);
        const auto cfilter = ot::factory::GCS(
            api_,
            params.first,
            params.second,
            ot::blockchain::internal::BlockHashToFilterKey(block.ID().Bytes()),
            ExtractElements(vector, block, encodedElements),
            {});

        ASSERT_TRUE(cfilter.IsValid());

        const auto filter = [&] {
            auto out = api_.Factory().Data();
            cfilter.Encode(out.WriteInto());

            return out;
        }();

        EXPECT_EQ(filter, encodedFilter);

        const auto header =
            cfilter.Header(vector.PreviousFilterHeader(api_).Bytes());

        EXPECT_EQ(vector.FilterHeader(api_), header);
    }
}

TEST_F(BIP158, gcs_headers)
{
    for (const auto& vector : GetBip158Vectors()) {
        const auto blockHash = vector.Block(api_);
        const auto encodedFilter = vector.Filter(api_);
        const auto previousHeader = vector.PreviousFilterHeader(api_);

        const auto cfilter = ot::factory::GCS(
            api_,
            Basic_BIP158,
            ot::blockchain::internal::BlockHashToFilterKey(blockHash.Bytes()),
            encodedFilter.Bytes(),
            {});

        ASSERT_TRUE(cfilter.IsValid());

        const auto header = cfilter.Header(previousHeader.Bytes());

        EXPECT_EQ(header, vector.FilterHeader(api_));
    }
}

TEST_F(BIP158, serialization)
{
    for (const auto& vector : GetBip158Vectors()) {
        const auto raw = vector.Block(api_);
        const auto block =
            api_.Factory().BlockchainBlock(Bitcoin_testnet3, raw.Bytes(), {});

        EXPECT_TRUE(block.IsValid());

        auto serialized = api_.Factory().Data();

        EXPECT_TRUE(block.Serialize(serialized.WriteInto()));
        EXPECT_EQ(raw, serialized);
    }
}

TEST_F(BIP158, bch_filter_1307544)
{
    const auto filter = GetBchCfilter1307544();
    const auto blockHash = api_.Factory().DataFromHex(
        "a9df8e8b72336137aaf70ac0d390c2a57b2afc826201e9f78b00000000000000");
    const auto encodedFilter = ot::ReadView{
        reinterpret_cast<const char*>(filter.data()), filter.size()};
    const auto previousHeader = api_.Factory().DataFromHex(
        "258c5095df5d3d57d4a427add793df679615366ce8ac6e1803a6ea02fca44fc6");
    const auto expectedHeader = api_.Factory().DataFromHex(
        "1aa1093ac9289923d390f3bdb2218095dc2d2559f14b4a68b20fcf1656b612b4");

    const auto cfilter = ot::factory::GCS(
        api_,
        Basic_BCHVariant,
        ot::blockchain::internal::BlockHashToFilterKey(blockHash.Bytes()),
        encodedFilter,
        {});

    ASSERT_TRUE(cfilter.IsValid());

    const auto header = cfilter.Header(previousHeader.Bytes());

    EXPECT_EQ(header, expectedHeader);
}

TEST_F(BIP158, bch_filter_1307723)
{
    const auto filter = GetBchCfilter1307723();
    const auto blockHash = api_.Factory().DataFromHex(
        "c28ca17ec9727809b449447eac0ba416a0b347f3836843f31303000000000000");
    const auto encodedFilter = ot::ReadView{
        reinterpret_cast<const char*>(filter.data()), filter.size()};
    const auto previousHeader = api_.Factory().DataFromHex(
        "4417c11a1bfecdbd6948b225dfb92a86021bc2220e1b7d9749af04637b0c9e1f");
    const auto expectedHeader = api_.Factory().DataFromHex(
        "747d817e9a7b2130e000b197a08219fa2667c8dc8313591d00492bb9213293ae");

    const auto cfilter = ot::factory::GCS(
        api_,
        Basic_BCHVariant,
        ot::blockchain::internal::BlockHashToFilterKey(blockHash.Bytes()),
        encodedFilter,
        {});

    ASSERT_TRUE(cfilter.IsValid());

    const auto header = cfilter.Header(previousHeader.Bytes());

    EXPECT_EQ(header, expectedHeader);
}
}  // namespace ottest
