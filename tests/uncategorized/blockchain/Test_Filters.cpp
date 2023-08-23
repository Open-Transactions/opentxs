// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iterator>
#include <string_view>
#include <utility>

#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/bloom/Filter.hpp"
#include "internal/blockchain/bloom/UpdateFlag.hpp"
#include "internal/blockchain/cfilter/GCS.hpp"
#include "internal/blockchain/params/ChainData.hpp"
#include "internal/util/P0330.hpp"
#include "ottest/fixtures/blockchain/Filters.hpp"

namespace ot = opentxs;

namespace ottest
{
using namespace opentxs::literals;
using namespace std::literals;

using Hash = ot::ByteArray;
auto stress_test_ = ot::Vector<Hash>{};

TEST_F(Filters, bloom_filter)
{
    ot::UnallocatedCString s1("blah");
    ot::UnallocatedCString s2("foo");
    ot::UnallocatedCString s3("justus");
    ot::UnallocatedCString s4("fellowtraveler");

    const auto object1(ot::ByteArray{s1.data(), s1.length()});
    const auto object2(ot::ByteArray{s2.data(), s2.length()});
    const auto object3(ot::ByteArray{s3.data(), s3.length()});
    const auto object4(ot::ByteArray{s4.data(), s4.length()});

    ot::OTBloomFilter pFilter{ot::factory::BloomFilter(
        api_, 9873485, ot::blockchain::bloom::UpdateFlag::None, 5, 0.001)};

    pFilter->AddElement(object1);
    pFilter->AddElement(object4);

    EXPECT_TRUE(pFilter->Test(object1));
    EXPECT_FALSE(pFilter->Test(object2));
    EXPECT_FALSE(pFilter->Test(object3));
    EXPECT_TRUE(pFilter->Test(object4));
}

TEST_F(Filters, bitstreams)
{
    {
        auto result = ot::Vector<std::byte>{};
        auto writer = ot::blockchain::internal::BitWriter{result};
        writer.write(1, 1);
        writer.write(1, 1);
        writer.write(1, 1);
        writer.write(1, 0);
        writer.write(1, 0);
        writer.write(1, 1);
        writer.write(1, 0);
        writer.write(1, 1);
        writer.flush();

        ASSERT_EQ(1, result.size());
        EXPECT_EQ(229, std::to_integer<std::uint8_t>(result.at(0)));
    }

    {
        auto result = ot::Vector<std::byte>{};
        auto writer = ot::blockchain::internal::BitWriter{result};
        writer.write(1, 1);
        writer.write(1, 0);
        writer.write(1, 1);
        writer.write(1, 1);
        writer.write(1, 1);
        writer.write(1, 0);
        writer.write(1, 0);
        writer.write(1, 1);
        writer.write(1, 0);
        writer.write(1, 1);
        writer.flush();

        ASSERT_EQ(2, result.size());
        EXPECT_EQ(185, std::to_integer<std::uint8_t>(result.at(0)));
        EXPECT_EQ(64, std::to_integer<std::uint8_t>(result.at(1)));

        auto reader = ot::blockchain::internal::BitReader{result};

        EXPECT_EQ(reader.read(1), 1);
        EXPECT_EQ(reader.read(1), 0);
        EXPECT_EQ(reader.read(1), 1);
        EXPECT_EQ(reader.read(1), 1);
        EXPECT_EQ(reader.read(1), 1);
        EXPECT_EQ(reader.read(1), 0);
        EXPECT_EQ(reader.read(1), 0);
        EXPECT_EQ(reader.read(1), 1);
        EXPECT_EQ(reader.read(1), 0);
        EXPECT_EQ(reader.read(1), 1);
    }

    {
        auto result = ot::Vector<std::byte>{};
        auto writer = ot::blockchain::internal::BitWriter{result};
        writer.write(1, 1);
        writer.write(1, 1);
        writer.write(1, 1);
        writer.write(1, 0);
        writer.write(19, 498577u);
        writer.flush();

        ASSERT_EQ(3, result.size());
        EXPECT_EQ(239, std::to_integer<std::uint8_t>(result.at(0)));
        EXPECT_EQ(55, std::to_integer<std::uint8_t>(result.at(1)));
        EXPECT_EQ(34, std::to_integer<std::uint8_t>(result.at(2)));
    }

    {
        auto result = ot::Vector<std::byte>{};
        auto writer = ot::blockchain::internal::BitWriter{result};
        writer.write(1, 1);
        writer.write(1, 1);
        writer.write(1, 1);
        writer.write(1, 0);
        writer.write(19, 17533928416540072849u);
        writer.flush();
        auto intReader = ot::blockchain::internal::BitReader{result};

        EXPECT_EQ(intReader.read(1), 1);
        EXPECT_EQ(intReader.read(1), 1);
        EXPECT_EQ(intReader.read(1), 1);
        EXPECT_EQ(intReader.read(1), 0);
        EXPECT_EQ(intReader.read(19u), 498577u);
    }
}

TEST_F(Filters, golomb_coding)
{
    const auto elements = ot::Vector<std::uint64_t>{2, 3, 5, 8, 13};
    const auto N = static_cast<std::uint32_t>(elements.size());
    const auto P = std::uint8_t{19};
    const auto encoded = ot::gcs::GolombEncode(P, elements, {});

    ASSERT_GT(encoded.size(), 0);

    const auto decoded = ot::gcs::GolombDecode(N, P, encoded, {});

    ASSERT_EQ(elements.size(), decoded.size());

    for (auto i = 0_uz; i < decoded.size(); ++i) {
        EXPECT_EQ(elements.at(i), decoded.at(i));
    }
}

TEST_F(Filters, gcs)
{
    const auto s1 = ot::UnallocatedCString{"blah"};
    const auto s2 = ot::UnallocatedCString{"foo"};
    const auto s3 = ot::UnallocatedCString{"justus"};
    const auto s4 = ot::UnallocatedCString{"fellowtraveler"};
    const auto s5 = ot::UnallocatedCString{"islajames"};
    const auto s6 = ot::UnallocatedCString{"timewaitsfornoman"};
    const auto object1(ot::ByteArray{s1.data(), s1.length()});
    const auto object2(ot::ByteArray{s2.data(), s2.length()});
    const auto object3(ot::ByteArray{s3.data(), s3.length()});
    const auto object4(ot::ByteArray{s4.data(), s4.length()});
    const auto object5(ot::ByteArray{s5.data(), s5.length()});
    const auto object6(ot::ByteArray{s6.data(), s6.length()});

    auto includedElements =
        ot::Vector<ot::ByteArray>{object1, object2, object3, object4};
    auto excludedElements = ot::Vector<ot::ByteArray>{object5, object6};
    auto key = ot::UnallocatedCString{"0123456789abcdef"};
    const auto gcs = ot::factory::GCS(
        api_, params_.first, params_.second, key, includedElements, {});

    ASSERT_TRUE(gcs.IsValid());
    EXPECT_TRUE(gcs.Test(object1, {}));
    EXPECT_TRUE(gcs.Test(object2, {}));
    EXPECT_TRUE(gcs.Test(object3, {}));
    EXPECT_TRUE(gcs.Test(object4, {}));
    EXPECT_FALSE(gcs.Test(object5, {}));
    EXPECT_FALSE(gcs.Test(object6, {}));
    EXPECT_TRUE(gcs.Test(includedElements, {}));
    EXPECT_FALSE(gcs.Test(excludedElements, {}));

    const auto partial = ot::Vector<ot::ReadView>{
        object1.Bytes(), object4.Bytes(), object5.Bytes(), object6.Bytes()};
    const auto matches = gcs.Match(partial, {}, {});

    EXPECT_TRUE(2 == matches.size());

    const auto good1 = object1.Bytes();
    const auto good2 = object4.Bytes();

    for (const auto& match : matches) {
        EXPECT_TRUE((good1 == *match) || (good2 == *match));
    }
}

TEST_F(Filters, bip158_case_0) { EXPECT_TRUE(TestGCSBlock(0)); }

TEST_F(Filters, bip158_case_49291) { EXPECT_TRUE(TestGCSBlock(49291)); }

// filter for block
// 0486d358f515ee448ee77f8dadb24e3c49e17e8208bbbcff2241c89900000000 should match
// output 0 of:
// https://live.blockcypher.com/btc-testnet/tx/507449133487c4c288a007b12ae6489204cf7d2316028433e289ef089a66fb8a/
TEST_F(Filters, bip158_case_1665877)
{
    const auto block = api_.Factory().DataFromHex(
        "0486d358f515ee448ee77f8dadb24e3c49e17e8208bbbcff2241c89900000000");
    const auto script = api_.Factory().DataFromHex(
        "76a914abcfcc53dcae0b49d45a4830a667dfb3aadf185b88ac");
    const auto encodedGCS = api_.Factory().DataFromHex(
        "8f1ee69134da43687f3cc23fcba34efcce9b25c6ba1b296a1c456a96d2d0f362800795"
        "dd0746a5264514ad1d50e190590c69a312ff1f3d66426292978c5f14751c1691705f17"
        "a260a23a92dedce351ce983ef4c09a17591e212c7cf34836da468b23f462511311dd53"
        "f7c0f6708305851aa96c6568d9a20e35871771e959790b88cdb70a3152ab500837199a"
        "b528f78ee97ecee538657b18a76c3abae2efb9dcfd013d63c4fe79faebe407bfbdbe47"
        "4ad42809a9493cc5b3d3f4668e2f5e2a2e0c631901dd8a8d2472a325349faa05fbf5b2"
        "642044d481712a30588471898fc5cb74e82f8a023449b5ff171f4393863e7dcf9c95da"
        "e8fd381036ab429c457005b2585da398b5a69fda0e8cfe94276423a55b57a9c5aebbfc"
        "18f6a9870e3ca8ca238a450c7c6aaa7c80f7eac71dfc30a57e4dd7a7abd682bb41e32f"
        "c47db5c3fb445394143671552051462c148e0ada789356733fb38d2e76b90a40e8ffd7"
        "c0bc6bbd8159136b4d48fb4e1709c2dc175cc2172d27110d750648ff2a43d2b8133550"
        "b263e994f9a0f4351e852968fba21f41a27628e900");

    auto key = ot::blockchain::internal::BlockHashToFilterKey(block.Bytes());
    const auto gcs = ot::factory::GCS(
        api_, params_.first, params_.second, key, 154, encodedGCS.Bytes(), {});

    ASSERT_TRUE(gcs.IsValid());
    EXPECT_TRUE(gcs.Test(script, {}));
}

TEST_F(Filters, bip158_headers)
{
    namespace bc = ot::blockchain::internal;

    const auto filter_0 = api_.Factory().DataFromHex("0x019dfca8");
    const auto filter_1 = api_.Factory().DataFromHex("0x015d5000");
    const auto filter_2 = api_.Factory().DataFromHex("0x0174a170");
    const auto filter_3 = api_.Factory().DataFromHex("0x016cf7a0");
    const auto blank = api_.Factory().DataFromHex(
        "0x0000000000000000000000000000000000000000000000000000000000000000");
    const auto expected_0 = api_.Factory().DataFromHex(
        "0x50b781aed7b7129012a6d20e2d040027937f3affaee573779908ebb779455821");
    const auto expected_1 = api_.Factory().DataFromHex(
        "0xe14fc288fdbf3c8d84f31bfc45892e44a0f152e82c0ddd1a5b749da513acbdd7");
    const auto expected_2 = api_.Factory().DataFromHex(
        "0xf06c381b7d46b1f8df603de51f25fda128dff8cbe8f204357e5e2bef11fd6a18");
    const auto expected_3 = api_.Factory().DataFromHex(
        "0x2a9d721212af044cec24f188631cff7b516fb1576a31d2b67c25b75adfaa638d");
    auto previous = blank.Bytes();
    auto hash = bc::FilterToHash(api_, filter_0.Bytes());
    auto calculated_a = bc::FilterHashToHeader(api_, hash.Bytes(), previous);
    auto calculated_b = bc::FilterToHeader(api_, filter_0.Bytes(), previous);

    EXPECT_EQ(calculated_a, calculated_b);
    EXPECT_EQ(calculated_a, expected_0);

    previous = expected_0.Bytes();
    hash = bc::FilterToHash(api_, filter_1.Bytes());
    calculated_a = bc::FilterHashToHeader(api_, hash.Bytes(), previous);
    calculated_b = bc::FilterToHeader(api_, filter_1.Bytes(), previous);

    EXPECT_EQ(calculated_a, calculated_b);
    EXPECT_EQ(calculated_a, expected_1);

    previous = expected_1.Bytes();
    hash = bc::FilterToHash(api_, filter_2.Bytes());
    calculated_a = bc::FilterHashToHeader(api_, hash.Bytes(), previous);
    calculated_b = bc::FilterToHeader(api_, filter_2.Bytes(), previous);

    EXPECT_EQ(calculated_a, calculated_b);
    EXPECT_EQ(calculated_a, expected_2);

    previous = expected_2.Bytes();
    hash = bc::FilterToHash(api_, filter_3.Bytes());
    calculated_a = bc::FilterHashToHeader(api_, hash.Bytes(), previous);
    calculated_b = bc::FilterToHeader(api_, filter_3.Bytes(), previous);

    EXPECT_EQ(calculated_a, calculated_b);
    EXPECT_EQ(calculated_a, expected_3);
}

TEST_F(Filters, hash)
{
    namespace bc = ot::blockchain::internal;

    const auto& block_0 =
        ot::blockchain::params::get(ot::blockchain::Type::Bitcoin_testnet3)
            .GenesisHash();
    const auto preimage = api_.Factory().DataFromHex("0x019dfca8");
    const auto filter_0 = api_.Factory().DataFromHex("0x9dfca8");
    const auto gcs = ot::factory::GCS(
        api_,
        params_.first,
        params_.second,
        bc::BlockHashToFilterKey(block_0.Bytes()),
        1,
        filter_0.Bytes(),
        {});

    ASSERT_TRUE(gcs.IsValid());

    const auto hash_a = gcs.Hash();
    const auto hash_b = bc::FilterToHash(api_, preimage.Bytes());

    EXPECT_EQ(hash_a, hash_b);
}

TEST_F(Filters, init_array)
{
    constexpr auto count{1000000u};
    stress_test_.reserve(count);
    stress_test_.clear();

    {
        ASSERT_TRUE(stress_test_.empty());

        auto& first = stress_test_.emplace_back();
        first.resize(32);

        ASSERT_TRUE(
            api_.Crypto().Util().RandomizeMemory(first.data(), first.size()));
    }

    while (stress_test_.size() < count) {
        const auto& previous = stress_test_.back();
        auto& next = stress_test_.emplace_back();

        EXPECT_TRUE(api_.Crypto().Hash().Digest(
            ot::crypto::HashType::Sha256, previous.Bytes(), next.WriteInto()));
    }
}

TEST_F(Filters, test_set_intersection)
{
    namespace bc = ot::blockchain::internal;

    const auto params = ot::blockchain::internal::GetFilterParams(
        ot::blockchain::cfilter::Type::ES);
    const auto hash = [&] {
        auto out = api_.Factory().Data();
        out.resize(32);
        api_.Crypto().Util().RandomizeMemory(out.data(), out.size());

        return out;
    }();
    const auto gcs = ot::factory::GCS(
        api_,
        params.first,
        params.second,
        bc::BlockHashToFilterKey(hash.Bytes()),
        stress_test_,
        {});

    ASSERT_TRUE(gcs.IsValid());

    const auto subset = [&] {
        constexpr auto count{1000u};
        auto copy{stress_test_};
        auto stop{count};
        auto begin{copy.begin()};
        auto end{copy.end()};
        std::size_t left = std::distance(begin, end);

        while (stop--) {
            auto r = begin;
            // NOLINTNEXTLINE(cert-msc30-c,cert-msc50-cpp)
            std::advance(r, rand() % left);
            std::swap(*begin, *r);
            ++begin;
            --left;
        }

        auto out = decltype(copy){copy.begin(), std::next(copy.begin(), count)};

        return out;
    }();

    const auto targets = [&] {
        auto out = ot::blockchain::cfilter::Targets{};
        out.reserve(subset.size());
        std::transform(
            subset.begin(),
            subset.end(),
            std::back_inserter(out),
            [](const auto& i) { return i.Bytes(); });

        return out;
    }();
    const auto matches = gcs.Match(targets, {}, {});

    EXPECT_EQ(matches.size(), targets.size());
}
}  // namespace ottest
