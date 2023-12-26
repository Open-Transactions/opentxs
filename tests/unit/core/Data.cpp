// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <compare>
#include <cstdint>
#include <span>
#include <string_view>

#include "ottest/data/core/Data.hpp"

namespace ot = opentxs;

namespace ottest
{
TEST(Data, default_accessors)
{
    const auto data = ot::ByteArray{};

    ASSERT_EQ(data.data(), nullptr);
    ASSERT_EQ(data.size(), 0);
}

TEST(Data, hex_without_prefix)
{
    for (const auto& input : HexWithoutPrefix()) {
        auto value = ot::ByteArray{};

        EXPECT_TRUE(value.DecodeHex(input));

        const auto output = value.asHex();

        EXPECT_EQ(output, input);
    }
}

TEST(Data, hex_with_prefix)
{
    for (const auto& input : HexWithPrefix()) {
        auto value = ot::ByteArray{};

        EXPECT_TRUE(value.DecodeHex(input));

        const auto output = value.asHex();

        EXPECT_EQ(output, input.substr(2));
    }
}

TEST(Data, comparison_equal_size)
{
    const auto one = ot::ByteArray{ot::IsHex, HexWithoutPrefix()[2]};
    const auto two = ot::ByteArray{ot::IsHex, HexWithoutPrefix()[3]};

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

TEST(Data, comparison_lhs_short)
{
    const auto one = ot::ByteArray{ot::IsHex, HexWithoutPrefix()[3]};
    const auto two = ot::ByteArray{ot::IsHex, HexWithoutPrefix()[4]};

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

TEST(Data, comparison_rhs_short)
{
    const auto one = ot::ByteArray{ot::IsHex, HexWithoutPrefix()[5]};
    const auto two = ot::ByteArray{ot::IsHex, HexWithoutPrefix()[6]};

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
    const ot::UnallocatedCString value(
        static_cast<const char*>(other.data()), other.size());
    ASSERT_EQ(value, "abcd");
}

TEST(Data, copy_from_interface)
{
    auto one = ot::ByteArray{"abcd", 4};
    auto other = ot::ByteArray{one};
    const ot::UnallocatedCString value(
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
