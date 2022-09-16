// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <compare>
#include <cstdint>

namespace ot = opentxs;

namespace ottest
{
struct Default_Data : public ::testing::Test {
    ot::ByteArray data_;
    const ot::UnallocatedVector<ot::UnallocatedCString> hex_{
        "",
        "61",
        "626262",
        "636363",
        "73696d706c792061206c6f6e6720737472696e67",
        "00eb15231dfceb60925886b67d065299925915aeb172c06647",
        "516b6fcd0f"
        "bf4f89001e670274dd"
        "572e4794",
        "ecac89cad93923c02321",
        "10c8511e",
        "00000000000000000000",
        "000111d38e5fc9071ffcd20b4a763cc9ae4f252bb4e48fd66a835e252ada93ff480d6d"
        "d43dc62a641155a5",
        "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f202122"
        "232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f404142434445"
        "464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f606162636465666768"
        "696a6b6c6d6e6f707172737475767778797a7b7c7d7e7f808182838485868788898a8b"
        "8c8d8e8f909192939495969798999a9b9c9d9e9fa0a1a2a3a4a5a6a7a8a9aaabacadae"
        "afb0b1b2b3b4b5b6b7b8b9babbbcbdbebfc0c1c2c3c4c5c6c7c8c9cacbcccdcecfd0d1"
        "d2d3d4d5d6d7d8d9dadbdcdddedfe0e1e2e3e4e5e6e7e8e9eaebecedeeeff0f1f2f3f4"
        "f5f6f7f8f9fafbfcfdfeff"};
    const ot::UnallocatedVector<ot::UnallocatedCString> hex_2_{
        "0x000000000000000000",
        "0X000111d38e5fc9071ffcd20b4a763cc9ae4f252bb4e48fd66a835e252ada93ff480d"
        "6dd43dc62a641155a5"};

    Default_Data()
        : data_(ot::ByteArray{})
    {
    }
};

TEST_F(Default_Data, default_accessors)
{
    ASSERT_EQ(data_.data(), nullptr);
    ASSERT_EQ(data_.size(), 0);
}

TEST_F(Default_Data, hex)
{
    for (const auto& input : hex_) {
        auto value = ot::ByteArray{};

        EXPECT_TRUE(value.DecodeHex(input));

        const auto output = value.asHex();

        EXPECT_EQ(output, input);
    }
}

TEST_F(Default_Data, comparison_equal_size)
{
    const auto one = [](const auto& hex) {
        auto out = ot::ByteArray{};
        out.DecodeHex(hex);
        return out;
    }(hex_.at(2));
    const auto two = [](const auto& hex) {
        auto out = ot::ByteArray{};
        out.DecodeHex(hex);
        return out;
    }(hex_.at(3));

    EXPECT_FALSE(one == two);
    EXPECT_FALSE(two == one);
    EXPECT_TRUE(one != two);
    EXPECT_TRUE(two != one);
    EXPECT_TRUE(one < two);
    EXPECT_TRUE(one <= two);
    EXPECT_FALSE(two < one);
    EXPECT_FALSE(two <= one);
    EXPECT_TRUE(two > one);
    EXPECT_TRUE(two >= one);
    EXPECT_FALSE(one > two);
    EXPECT_FALSE(one >= two);
}

TEST_F(Default_Data, comparison_lhs_short)
{
    const auto one = [](const auto& hex) {
        auto out = ot::ByteArray{};
        out.DecodeHex(hex);
        return out;
    }(hex_.at(3));
    const auto two = [](const auto& hex) {
        auto out = ot::ByteArray{};
        out.DecodeHex(hex);
        return out;
    }(hex_.at(4));

    EXPECT_FALSE(one == two);
    EXPECT_FALSE(two == one);
    EXPECT_TRUE(one != two);
    EXPECT_TRUE(two != one);
    EXPECT_TRUE(one < two);
    EXPECT_TRUE(one <= two);
    EXPECT_FALSE(two < one);
    EXPECT_FALSE(two <= one);
    EXPECT_TRUE(two > one);
    EXPECT_TRUE(two >= one);
    EXPECT_FALSE(one > two);
    EXPECT_FALSE(one >= two);
}

TEST_F(Default_Data, comparison_rhs_short)
{
    const auto one = [](const auto& hex) {
        auto out = ot::ByteArray{};
        out.DecodeHex(hex);
        return out;
    }(hex_.at(5));
    const auto two = [](const auto& hex) {
        auto out = ot::ByteArray{};
        out.DecodeHex(hex);
        return out;
    }(hex_.at(6));

    EXPECT_FALSE(one == two);
    EXPECT_FALSE(two == one);
    EXPECT_TRUE(one != two);
    EXPECT_TRUE(two != one);
    EXPECT_FALSE(one < two);
    EXPECT_FALSE(one <= two);
    EXPECT_TRUE(two < one);
    EXPECT_TRUE(two <= one);
    EXPECT_FALSE(two > one);
    EXPECT_FALSE(two >= one);
    EXPECT_TRUE(one > two);
    EXPECT_TRUE(one >= two);
}

TEST(Data, compare_equal_to_self)
{
    auto one = ot::ByteArray{"abcd", 4};
    ASSERT_TRUE(one == one);
}

TEST(Data, compare_equal_to_other_same)
{
    auto one = ot::ByteArray{"abcd", 4};
    auto other = ot::ByteArray{"abcd", 4};
    ASSERT_TRUE(one == other);
}

TEST(Data, compare_equal_to_other_different)
{
    auto one = ot::ByteArray{"abcd", 4};
    auto other = ot::ByteArray{"zzzz", 4};
    ASSERT_FALSE(one == other);
}

TEST(Data, compare_not_equal_to_self)
{
    auto one = ot::ByteArray{"aaaa", 4};
    ASSERT_FALSE(one != one);
}

TEST(Data, compare_not_equal_to_other_same)
{
    auto one = ot::ByteArray{"abcd", 4};
    auto other = ot::ByteArray{"abcd", 4};
    ASSERT_FALSE(one != other);
}

TEST(Data, compare_not_equal_to_other_different)
{
    auto one = ot::ByteArray{"abcd", 4};
    auto other = ot::ByteArray{"zzzz", 4};
    ASSERT_TRUE(one != other);
}

TEST(Data, copy_from_pimpl)
{
    auto one = ot::ByteArray{"abcd", 4};
    auto other = ot::ByteArray{one};
    ot::UnallocatedCString value(
        static_cast<const char*>(other.data()), other.size());
    ASSERT_EQ(value, "abcd");
}

TEST(Data, copy_from_interface)
{
    auto one = ot::ByteArray{"abcd", 4};
    auto other = ot::ByteArray{one};
    ot::UnallocatedCString value(
        static_cast<const char*>(other.data()), other.size());
    ASSERT_EQ(value, "abcd");
}

TEST(Data, map_1)
{
    const auto one = [](const auto& hex) {
        auto out = ot::ByteArray{};
        out.DecodeHex(hex);
        return out;
    }("4860eb18bf1b1620e37e9490fc8a427514416fd75159ab86688e9a8300000000");
    const auto two = [](const auto& hex) {
        auto out = ot::ByteArray{};
        out.DecodeHex(hex);
        return out;
    }("bddd99ccfda39da1b108ce1a5d70038d0a967bacb68b6b63065f626a00000000");

    EXPECT_FALSE(one == two);
    EXPECT_TRUE(one != two);
    EXPECT_TRUE(one < two);
    EXPECT_TRUE(one <= two);
    EXPECT_FALSE(one > two);
    EXPECT_FALSE(one >= two);

    EXPECT_FALSE(two == one);
    EXPECT_TRUE(two != one);
    EXPECT_FALSE(two < one);
    EXPECT_FALSE(two <= one);
    EXPECT_TRUE(two > one);
    EXPECT_TRUE(two >= one);

    ot::UnallocatedMap<ot::ByteArray, ot::UnallocatedCString> map{};

    EXPECT_EQ(map.size(), 0);
    EXPECT_EQ(map.count(one), 0);
    EXPECT_EQ(map.count(two), 0);

    map.emplace(one, "foo");

    EXPECT_EQ(map.size(), 1);
    EXPECT_EQ(map.count(one), 1);
    EXPECT_EQ(map.count(two), 0);

    map.emplace(two, "bar");

    EXPECT_EQ(map.size(), 2);
    EXPECT_EQ(map.count(one), 1);
    EXPECT_EQ(map.count(two), 1);
}

TEST(Data, map_2)
{
    const auto one = [](const auto& hex) {
        auto out = ot::ByteArray{};
        out.DecodeHex(hex);
        return out;
    }("4860eb18bf1b1620e37e9490fc8a427514416fd75159ab86688e9a8300000000");
    const auto two = [](const auto& hex) {
        auto out = ot::ByteArray{};
        out.DecodeHex(hex);
        return out;
    }("4860eb18bf1b1620e37e9490fc8a427514416fd75159ab86688e9a8300000001");

    EXPECT_FALSE(one == two);
    EXPECT_TRUE(one != two);
    EXPECT_TRUE(one < two);
    EXPECT_TRUE(one <= two);
    EXPECT_FALSE(one > two);
    EXPECT_FALSE(one >= two);

    EXPECT_FALSE(two == one);
    EXPECT_TRUE(two != one);
    EXPECT_FALSE(two < one);
    EXPECT_FALSE(two <= one);
    EXPECT_TRUE(two > one);
    EXPECT_TRUE(two >= one);

    ot::UnallocatedMap<ot::ByteArray, ot::UnallocatedCString> map{};

    EXPECT_EQ(map.size(), 0);
    EXPECT_EQ(map.count(one), 0);
    EXPECT_EQ(map.count(two), 0);

    map.emplace(one, "foo");

    EXPECT_EQ(1, map.size());
    EXPECT_EQ(1, map.count(one));
    EXPECT_EQ(map.count(two), 0);

    map.emplace(two, "bar");

    EXPECT_EQ(2, map.size());
    EXPECT_EQ(1, map.count(one));
    EXPECT_EQ(1, map.count(two));
}

TEST(Data, is_null)
{
    const auto one = [](const auto& hex) {
        auto out = ot::ByteArray{};
        out.DecodeHex(hex);
        return out;
    }("00000000");
    const auto two = [](const auto& hex) {
        auto out = ot::ByteArray{};
        out.DecodeHex(hex);
        return out;
    }("4860eb18bf1b1620e37e9490fc8a427514416fd75159ab86688e9a8300000001");
    const auto three = [](const auto& hex) {
        auto out = ot::ByteArray{};
        out.DecodeHex(hex);
        return out;
    }("0000000000000000000000000000000000000000000000000000000000000001");
    const auto four = ot::ByteArray{};

    EXPECT_TRUE(one.IsNull());
    EXPECT_FALSE(two.IsNull());
    EXPECT_FALSE(three.IsNull());
    EXPECT_TRUE(four.IsNull());
}

TEST(Data, endian_16)
{
    const auto data1 = ot::ByteArray{std::uint16_t{4096u}};

    EXPECT_STREQ(data1.asHex().c_str(), "1000");

    auto data2 = [](const auto& hex) {
        auto out = ot::ByteArray{};
        out.DecodeHex(hex);
        return out;
    }("1000");
    auto recovered = std::uint16_t{};

    EXPECT_TRUE(data2.Extract(recovered));
    EXPECT_EQ(recovered, 4096u);

    data2 += std::uint16_t{4096u};

    EXPECT_STREQ(data2.asHex().c_str(), "10001000");
}

TEST(Data, endian_32)
{
    const auto data1 = ot::ByteArray{std::uint32_t{268435456u}};

    EXPECT_STREQ(data1.asHex().c_str(), "10000000");

    auto data2 = [](const auto& hex) {
        auto out = ot::ByteArray{};
        out.DecodeHex(hex);
        return out;
    }("10000000");
    auto recovered = std::uint32_t{};

    EXPECT_TRUE(data2.Extract(recovered));
    EXPECT_EQ(recovered, 268435456u);

    data2 += std::uint32_t{268435456u};

    EXPECT_STREQ(data2.asHex().c_str(), "1000000010000000");
}

TEST(Data, endian_64)
{
    const auto data1 = ot::ByteArray{std::uint64_t{1152921504606846976ull}};

    EXPECT_STREQ(data1.asHex().c_str(), "1000000000000000");

    auto data2 = [](const auto& hex) {
        auto out = ot::ByteArray{};
        out.DecodeHex(hex);
        return out;
    }("1000000000000000");
    auto recovered1 = std::uint64_t{};

    EXPECT_TRUE(data2.Extract(recovered1));
    EXPECT_EQ(recovered1, 1152921504606846976ull);

    data2 += std::uint64_t{1152921504606846976ull};

    EXPECT_STREQ(data2.asHex().c_str(), "10000000000000001000000000000000");

    auto recovered2 = std::uint64_t{};

    EXPECT_TRUE(data2.Extract(recovered2, 4));
    EXPECT_EQ(recovered2, 268435456u);
}

TEST(Data, extract)
{
    const auto vector = [](const auto& hex) {
        auto out = ot::ByteArray{};
        out.DecodeHex(hex);
        return out;
    }("00000000000000000000ffff178140ba");
    const auto prefix = [](const auto& hex) {
        auto out = ot::ByteArray{};
        out.DecodeHex(hex);
        return out;
    }("00000000000000000000ffff");
    const auto suffix = [](const auto& hex) {
        auto out = ot::ByteArray{};
        out.DecodeHex(hex);
        return out;
    }("178140ba");

    auto calculatedPrefix = ot::ByteArray{};
    auto calculatedSuffix = ot::ByteArray{};

    EXPECT_EQ(16, vector.size());
    EXPECT_EQ(12, prefix.size());
    EXPECT_EQ(4, suffix.size());
    EXPECT_TRUE(vector.Extract(prefix.size(), calculatedPrefix));
    EXPECT_TRUE(vector.Extract(suffix.size(), calculatedSuffix, prefix.size()));

    EXPECT_EQ(prefix, calculatedPrefix);
    EXPECT_EQ(suffix, calculatedSuffix);
}
}  // namespace ottest
