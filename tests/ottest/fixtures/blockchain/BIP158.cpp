// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/blockchain/BIP158.hpp"  // IWYU pragma: associated

#include <boost/container/flat_map.hpp>
#include <boost/container/vector.hpp>
#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <utility>

#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "ottest/data/blockchain/Bip158.hpp"
#include "ottest/fixtures/blockchain/Basic.hpp"

namespace ottest
{
BIP158::BIP158()
    : api_(ot::Context().StartClientSession(
          ot::Options{}.SetBlockchainWalletEnabled(false),
          0))
{
}

auto BIP158::CompareElements(
    const ot::Vector<ot::ByteArray>& input,
    ot::UnallocatedVector<ot::UnallocatedCString> expected) const -> bool
{
    auto inputHex = ot::Vector<ot::UnallocatedCString>{};
    auto difference = ot::Vector<ot::UnallocatedCString>{};
    std::transform(
        std::begin(input), std::end(input), std::back_inserter(inputHex), [
        ](const auto& in) -> auto{ return in.asHex(); });

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

auto BIP158::CompareToOracle(
    const ot::blockchain::Type chain,
    const ot::blockchain::cfilter::Type filterType,
    const ot::Data& filter,
    const ot::blockchain::cfilter::Header& header) const -> bool
{
    constexpr auto seednode{"do not init peers"};
    const auto started = api_.Network().Blockchain().Start(chain, seednode);

    EXPECT_TRUE(started);

    if (false == started) { return false; }

    const auto handle = api_.Network().Blockchain().GetChain(chain);

    EXPECT_TRUE(handle);

    if (false == handle.IsValid()) { return false; }

    const auto& network = handle.get();
    const auto& fOracle = network.FilterOracle();
    const auto& genesis = ot::blockchain::params::get(chain).GenesisHash();
    const auto genesisFilter = fOracle.LoadFilter(filterType, genesis, {}, {});
    const auto genesisHeader = fOracle.LoadFilterHeader(filterType, genesis);

    EXPECT_TRUE(genesisFilter.IsValid());

    if (false == genesisFilter.IsValid()) { return false; }

    const auto encoded = [&] {
        auto out = api_.Factory().Data();
        genesisFilter.Encode(out.WriteInto());

        return out;
    }();

    EXPECT_EQ(filter.asHex(), encoded.asHex());
    EXPECT_EQ(header.asHex(), genesisHeader.asHex());

    return true;
}

auto BIP158::ExtractElements(
    const Bip158Vector& vector,
    const ot::blockchain::block::Block& block,
    const std::size_t encodedElements) const noexcept
    -> ot::Vector<ot::ByteArray>
{
    auto output = ot::Vector<ot::ByteArray>{};

    for (const auto& bytes : block.Internal().ExtractElements(
             ot::blockchain::cfilter::Type::Basic_BIP158, {})) {
        output.emplace_back(api_.Factory().DataFromBytes(ot::reader(bytes)));
    }

    const auto& expectedElements = GetBip158Elements().at(vector.height_);
    auto previousOutputs = vector.PreviousOutputs(api_);

    EXPECT_TRUE(CompareElements(output, expectedElements));

    for (auto& bytes : previousOutputs) {
        if ((nullptr != bytes.data()) && (0 != bytes.size())) {
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

auto BIP158::GenerateGenesisFilter(
    const ot::blockchain::Type chain,
    const ot::blockchain::cfilter::Type filterType) const noexcept -> bool
{
    const auto& [genesisHex, filterMap] = genesis_block_data_.at(chain);
    const auto bytes = api_.Factory().DataFromHex(genesisHex);
    const auto block = api_.Factory().BlockchainBlock(chain, bytes.Bytes(), {});

    EXPECT_TRUE(block.IsValid());

    using enum ot::blockchain::cfilter::Type;
    constexpr auto masked{Basic_BIP158};
    constexpr auto replace{Basic_BCHVariant};

    const auto cfilter = ot::factory::GCS(
        api_, (filterType == masked) ? replace : filterType, block, {}, {});

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
            cfilter.Compressed(out.WriteInto());

            return out;
        }();
        const auto compressed2 = [&] {
            auto out = api_.Factory().Data();
            cfilter2.Compressed(out.WriteInto());

            return out;
        }();
        const auto encoded1 = [&] {
            auto out = api_.Factory().Data();
            cfilter.Encode(out.WriteInto());

            return out;
        }();
        const auto encoded2 = [&] {
            auto out = api_.Factory().Data();
            cfilter2.Encode(out.WriteInto());

            return out;
        }();

        EXPECT_EQ(compressed2, compressed1);
        EXPECT_EQ(encoded2, encoded1);
    }

    static const auto blank = ot::blockchain::cfilter::Header{};
    const auto filter = [&] {
        auto out = api_.Factory().Data();
        cfilter.Encode(out.WriteInto());

        return out;
    }();
    const auto header = cfilter.Header(blank);
    const auto& [expectedFilter, expectedHeader] = filterMap.at(filterType);

    EXPECT_EQ(filter.asHex(), expectedFilter);
    EXPECT_EQ(header.asHex(), expectedHeader);
    EXPECT_TRUE(CompareToOracle(chain, filterType, filter, header));

    return true;
}
}  // namespace ottest
