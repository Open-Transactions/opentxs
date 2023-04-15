// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <string_view>

namespace ot = opentxs;

namespace ottest
{
using namespace std::literals;

static constexpr auto test_hex_{
    "6fe28c0ab6f1b372c1a6a246ae63f74f931e8365e15a089c68d6190000000000"sv};

TEST(FixedByteArray, default_construct)
{
    auto value = ot::FixedByteArray<32>{};

    EXPECT_EQ(value.size(), 32);
}

TEST(FixedByteArray, hex)
{
    auto value = ot::FixedByteArray<32>{};

    EXPECT_TRUE(value.DecodeHex(test_hex_));

    auto data = ot::ByteArray{};

    EXPECT_TRUE(data.DecodeHex(test_hex_));
    EXPECT_EQ(value, data);

    const auto fromFixed = value.asHex();
    const auto fromData = data.asHex();

    EXPECT_EQ(fromFixed, test_hex_);
    EXPECT_EQ(fromData, test_hex_);

    const auto value2 = ot::FixedByteArray<32>{value.Bytes()};

    EXPECT_EQ(value2, value);
}
}  // namespace ottest
