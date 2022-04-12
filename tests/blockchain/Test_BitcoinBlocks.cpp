// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <boost/container/flat_map.hpp>
#include <boost/container/vector.hpp>
#include <gtest/gtest.h>
#include <algorithm>
#include <array>
#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <utility>

#include "1_Internal.hpp"
#include "Helpers.hpp"
#include "bip158/Bip158.hpp"
#include "bip158/bch_filter_1307544.hpp"
#include "bip158/bch_filter_1307723.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/GCS.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Header.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Options.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace ottest
{
struct Test_BitcoinBlock : public ::testing::Test {
    const ot::api::session::Client& api_;

    auto CompareElements(
        const ot::Vector<ot::OTData>& input,
        ot::UnallocatedVector<ot::UnallocatedCString> expected) const -> bool
    {
        auto inputHex = ot::Vector<ot::UnallocatedCString>{};
        auto difference = ot::Vector<ot::UnallocatedCString>{};
        std::transform(
            std::begin(input), std::end(input), std::back_inserter(inputHex), [
            ](const auto& in) -> auto { return in->asHex(); });

        EXPECT_EQ(expected.size(), inputHex.size());

        if (expected.size() != inputHex.size()) { return false; }

        std::sort(std::begin(inputHex), std::end(inputHex));
        std::sort(std::begin(expected), std::end(expected));
        auto failureCount{0};

        for (auto i{0u}; i < expected.size(); ++i) {
            EXPECT_EQ(expected.at(i), inputHex.at(i));

            if (expected.at(i) != inputHex.at(i)) { ++failureCount; }
        }

        return 0 == failureCount;
    }

    auto CompareToOracle(
        const ot::blockchain::Type chain,
        const ot::blockchain::cfilter::Type filterType,
        const ot::Data& filter,
        const ot::blockchain::cfilter::Header& header) const -> bool
    {
        constexpr auto seednode{"do not init peers"};
        const auto started = api_.Network().Blockchain().Start(chain, seednode);

        EXPECT_TRUE(started);

        if (false == started) { return false; }

        const auto& network = api_.Network().Blockchain().GetChain(chain);
        const auto& hOracle = network.HeaderOracle();
        const auto& fOracle = network.FilterOracle();
        const auto genesisFilter =
            fOracle.LoadFilter(filterType, hOracle.GenesisBlockHash(chain), {});
        const auto genesisHeader = fOracle.LoadFilterHeader(
            filterType, hOracle.GenesisBlockHash(chain));

        EXPECT_TRUE(genesisFilter.IsValid());

        if (false == genesisFilter.IsValid()) { return false; }

        const auto encoded = [&] {
            auto out = api_.Factory().Data();
            genesisFilter.Encode(out->WriteInto());

            return out;
        }();

        EXPECT_EQ(filter.asHex(), encoded->asHex());
        EXPECT_EQ(header.asHex(), genesisHeader.asHex());

        return true;
    }

    auto ExtractElements(
        const Bip158Vector& vector,
        const ot::blockchain::block::Block& block,
        const std::size_t encodedElements) const noexcept
        -> ot::Vector<ot::OTData>
    {
        auto output = ot::Vector<ot::OTData>{};

        for (const auto& bytes : block.Internal().ExtractElements(
                 ot::blockchain::cfilter::Type::Basic_BIP158)) {
            output.emplace_back(
                api_.Factory().DataFromBytes(ot::reader(bytes)));
        }

        const auto& expectedElements = indexed_elements_.at(vector.height_);
        auto previousOutputs = vector.PreviousOutputs(api_);

        EXPECT_TRUE(CompareElements(output, expectedElements));

        for (auto& bytes : previousOutputs) {
            if ((nullptr != bytes->data()) && (0 != bytes->size())) {
                output.emplace_back(std::move(bytes));
            }
        }

        {
            auto copy{output};
            std::sort(copy.begin(), copy.end());
            copy.erase(std::unique(copy.begin(), copy.end()), copy.end());

            EXPECT_EQ(encodedElements, copy.size());
        }

        return output;
    }

    auto GenerateGenesisFilter(
        const ot::blockchain::Type chain,
        const ot::blockchain::cfilter::Type filterType) const noexcept -> bool
    {
        const auto& [genesisHex, filterMap] = genesis_block_data_.at(chain);
        const auto bytes = api_.Factory().DataFromHex(genesisHex);
        const auto block = api_.Factory().BitcoinBlock(chain, bytes->Bytes());

        EXPECT_TRUE(block);

        if (false == bool(block)) { return false; }

        constexpr auto masked{ot::blockchain::cfilter::Type::Basic_BIP158};
        constexpr auto replace{ot::blockchain::cfilter::Type::Basic_BCHVariant};

        const auto cfilter = ot::factory::GCS(
            api_, (filterType == masked) ? replace : filterType, *block, {});

        EXPECT_TRUE(cfilter.IsValid());

        if (false == cfilter.IsValid()) { return false; }

        {
            const auto proto = [&] {
                auto out = ot::Space{};
                cfilter.Serialize(ot::writer(out));

                return out;
            }();
            const auto cfilter2 = ot::factory::GCS(api_, ot::reader(proto), {});

            EXPECT_TRUE(cfilter2.IsValid());

            if (false == cfilter2.IsValid()) { return false; }

            const auto compressed1 = [&] {
                auto out = api_.Factory().Data();
                cfilter.Compressed(out->WriteInto());

                return out;
            }();
            const auto compressed2 = [&] {
                auto out = api_.Factory().Data();
                cfilter2.Compressed(out->WriteInto());

                return out;
            }();
            const auto encoded1 = [&] {
                auto out = api_.Factory().Data();
                cfilter.Encode(out->WriteInto());

                return out;
            }();
            const auto encoded2 = [&] {
                auto out = api_.Factory().Data();
                cfilter2.Encode(out->WriteInto());

                return out;
            }();

            EXPECT_EQ(compressed2, compressed1);
            EXPECT_EQ(encoded2, encoded1);
        }

        static const auto blank = ot::blockchain::cfilter::Header{};
        const auto filter = [&] {
            auto out = api_.Factory().Data();
            cfilter.Encode(out->WriteInto());

            return out;
        }();
        const auto header = cfilter.Header(blank);
        const auto& [expectedFilter, expectedHeader] = filterMap.at(filterType);

        EXPECT_EQ(filter->asHex(), expectedFilter);
        EXPECT_EQ(header.asHex(), expectedHeader);
        EXPECT_TRUE(CompareToOracle(chain, filterType, filter, header));

        return true;
    }

    Test_BitcoinBlock()
        : api_(ot::Context().StartClientSession(
              ot::Options{}.SetBlockchainWalletEnabled(false),
              0))
    {
    }
};

TEST_F(Test_BitcoinBlock, init) {}

TEST_F(Test_BitcoinBlock, regtest)
{
    EXPECT_TRUE(GenerateGenesisFilter(
        ot::blockchain::Type::UnitTest,
        ot::blockchain::cfilter::Type::Basic_BIP158));
    EXPECT_TRUE(GenerateGenesisFilter(
        ot::blockchain::Type::UnitTest,
        ot::blockchain::cfilter::Type::Basic_BCHVariant));
    EXPECT_TRUE(GenerateGenesisFilter(
        ot::blockchain::Type::UnitTest, ot::blockchain::cfilter::Type::ES));

    api_.Network().Blockchain().Stop(ot::blockchain::Type::UnitTest);
}

TEST_F(Test_BitcoinBlock, btc_genesis_mainnet)
{
    EXPECT_TRUE(GenerateGenesisFilter(
        ot::blockchain::Type::Bitcoin,
        ot::blockchain::cfilter::Type::Basic_BIP158));
    EXPECT_TRUE(GenerateGenesisFilter(
        ot::blockchain::Type::Bitcoin, ot::blockchain::cfilter::Type::ES));

    api_.Network().Blockchain().Stop(ot::blockchain::Type::Bitcoin);
}

TEST_F(Test_BitcoinBlock, btc_genesis_testnet)
{
    EXPECT_TRUE(GenerateGenesisFilter(
        ot::blockchain::Type::Bitcoin_testnet3,
        ot::blockchain::cfilter::Type::Basic_BIP158));
    EXPECT_TRUE(GenerateGenesisFilter(
        ot::blockchain::Type::Bitcoin_testnet3,
        ot::blockchain::cfilter::Type::ES));

    api_.Network().Blockchain().Stop(ot::blockchain::Type::Bitcoin_testnet3);
}

TEST_F(Test_BitcoinBlock, bch_genesis_mainnet)
{
    EXPECT_TRUE(GenerateGenesisFilter(
        ot::blockchain::Type::BitcoinCash,
        ot::blockchain::cfilter::Type::Basic_BCHVariant));
    EXPECT_TRUE(GenerateGenesisFilter(
        ot::blockchain::Type::BitcoinCash, ot::blockchain::cfilter::Type::ES));

    api_.Network().Blockchain().Stop(ot::blockchain::Type::BitcoinCash);
}

TEST_F(Test_BitcoinBlock, bch_genesis_testnet)
{
    EXPECT_TRUE(GenerateGenesisFilter(
        ot::blockchain::Type::BitcoinCash_testnet3,
        ot::blockchain::cfilter::Type::Basic_BCHVariant));
    EXPECT_TRUE(GenerateGenesisFilter(
        ot::blockchain::Type::BitcoinCash_testnet3,
        ot::blockchain::cfilter::Type::ES));

    api_.Network().Blockchain().Stop(
        ot::blockchain::Type::BitcoinCash_testnet3);
}

TEST_F(Test_BitcoinBlock, ltc_genesis_mainnet)
{
    EXPECT_TRUE(GenerateGenesisFilter(
        ot::blockchain::Type::Litecoin, ot::blockchain::cfilter::Type::ES));

    api_.Network().Blockchain().Stop(ot::blockchain::Type::Litecoin);
}

TEST_F(Test_BitcoinBlock, ltc_genesis_testnet)
{
    EXPECT_TRUE(GenerateGenesisFilter(
        ot::blockchain::Type::Litecoin_testnet4,
        ot::blockchain::cfilter::Type::ES));

    api_.Network().Blockchain().Stop(ot::blockchain::Type::Litecoin_testnet4);
}

TEST_F(Test_BitcoinBlock, pkt_mainnet)
{
    constexpr auto chain = ot::blockchain::Type::PKT;

    EXPECT_TRUE(GenerateGenesisFilter(
        chain, ot::blockchain::cfilter::Type::Basic_BIP158));
    EXPECT_TRUE(
        GenerateGenesisFilter(chain, ot::blockchain::cfilter::Type::ES));

    const auto& [genesisHex, filterMap] = genesis_block_data_.at(chain);
    const auto bytes = api_.Factory().DataFromHex(genesisHex);
    const auto pBlock = api_.Factory().BitcoinBlock(chain, bytes->Bytes());

    ASSERT_TRUE(pBlock);

    const auto& block = *pBlock;
    auto raw = api_.Factory().Data();

    EXPECT_TRUE(block.Serialize(raw->WriteInto()));
    EXPECT_EQ(raw.get(), bytes.get());

    api_.Network().Blockchain().Stop(chain);
}

TEST_F(Test_BitcoinBlock, bsv_genesis_mainnet)
{
    EXPECT_TRUE(GenerateGenesisFilter(
        ot::blockchain::Type::BitcoinSV,
        ot::blockchain::cfilter::Type::Basic_BCHVariant));
    EXPECT_TRUE(GenerateGenesisFilter(
        ot::blockchain::Type::BitcoinSV, ot::blockchain::cfilter::Type::ES));

    api_.Network().Blockchain().Stop(ot::blockchain::Type::BitcoinSV);
}

TEST_F(Test_BitcoinBlock, bsv_genesis_testnet)
{
    EXPECT_TRUE(GenerateGenesisFilter(
        ot::blockchain::Type::BitcoinSV_testnet3,
        ot::blockchain::cfilter::Type::Basic_BCHVariant));
    EXPECT_TRUE(GenerateGenesisFilter(
        ot::blockchain::Type::BitcoinSV_testnet3,
        ot::blockchain::cfilter::Type::ES));

    api_.Network().Blockchain().Stop(ot::blockchain::Type::BitcoinSV_testnet3);
}

TEST_F(Test_BitcoinBlock, bip158)
{
    for (const auto& vector : bip_158_vectors_) {
        const auto raw = vector.Block(api_);
        const auto pBlock = api_.Factory().BitcoinBlock(
            ot::blockchain::Type::Bitcoin_testnet3, raw->Bytes());

        ASSERT_TRUE(pBlock);

        const auto& block = *pBlock;

        EXPECT_EQ(block.ID(), vector.BlockHash(api_));

        const auto encodedFilter = vector.Filter(api_);
        auto encodedElements = std::size_t{};

        {
            namespace bb = ot::blockchain::bitcoin;
            auto expectedSize = std::size_t{1};
            auto it = static_cast<bb::ByteIterator>(encodedFilter->data());

            ASSERT_TRUE(opentxs::network::blockchain::bitcoin::DecodeSize(
                it, expectedSize, encodedFilter->size(), encodedElements));
        }

        static const auto params = ot::blockchain::internal::GetFilterParams(
            ot::blockchain::cfilter::Type::Basic_BIP158);
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
            cfilter.Encode(out->WriteInto());

            return out;
        }();

        EXPECT_EQ(filter.get(), encodedFilter.get());

        const auto header =
            cfilter.Header(vector.PreviousFilterHeader(api_)->Bytes());

        EXPECT_EQ(vector.FilterHeader(api_).get(), header);
    }
}

TEST_F(Test_BitcoinBlock, gcs_headers)
{
    for (const auto& vector : bip_158_vectors_) {
        const auto blockHash = vector.Block(api_);
        const auto encodedFilter = vector.Filter(api_);
        const auto previousHeader = vector.PreviousFilterHeader(api_);

        const auto cfilter = ot::factory::GCS(
            api_,
            ot::blockchain::cfilter::Type::Basic_BIP158,
            ot::blockchain::internal::BlockHashToFilterKey(blockHash->Bytes()),
            encodedFilter->Bytes(),
            {});

        ASSERT_TRUE(cfilter.IsValid());

        const auto header = cfilter.Header(previousHeader->Bytes());

        EXPECT_EQ(header, vector.FilterHeader(api_).get());
    }
}

TEST_F(Test_BitcoinBlock, serialization)
{
    for (const auto& vector : bip_158_vectors_) {
        const auto raw = vector.Block(api_);
        const auto pBlock = api_.Factory().BitcoinBlock(
            ot::blockchain::Type::Bitcoin_testnet3, raw->Bytes());

        ASSERT_TRUE(pBlock);

        const auto& block = *pBlock;
        auto serialized = api_.Factory().Data();

        EXPECT_TRUE(block.Serialize(serialized->WriteInto()));
        EXPECT_EQ(raw.get(), serialized);
    }
}

TEST_F(Test_BitcoinBlock, bch_filter_1307544)
{
    const auto& filter = bch_filter_1307544_;
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
        ot::blockchain::cfilter::Type::Basic_BCHVariant,
        ot::blockchain::internal::BlockHashToFilterKey(blockHash->Bytes()),
        encodedFilter,
        {});

    ASSERT_TRUE(cfilter.IsValid());

    const auto header = cfilter.Header(previousHeader->Bytes());

    EXPECT_EQ(header, expectedHeader.get());
}

TEST_F(Test_BitcoinBlock, bch_filter_1307723)
{
    const auto& filter = bch_filter_1307723_;
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
        ot::blockchain::cfilter::Type::Basic_BCHVariant,
        ot::blockchain::internal::BlockHashToFilterKey(blockHash->Bytes()),
        encodedFilter,
        {});

    ASSERT_TRUE(cfilter.IsValid());

    const auto header = cfilter.Header(previousHeader->Bytes());

    EXPECT_EQ(header, expectedHeader.get());
}
}  // namespace ottest
