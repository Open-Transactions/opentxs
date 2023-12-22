// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstdint>

#include "ottest/fixtures/blockchain/NumericHash.hpp"

namespace ottest
{
TEST_F(NumericHash, init_opentxs) {}

TEST_F(NumericHash, number_low_1)
{
    // Little endian
    const auto raw = ot::ByteArray{ot::IsHex, "0x01"};
    const ot::UnallocatedCString decimal{"1"};
    // Big endian
    const ot::UnallocatedCString hex{
        "0000000000000000000000000000000000000000000000000000000000000001"};
    const auto number = ot::blockchain::block::NumericHash{raw};

    EXPECT_EQ(decimal, number.Decimal());
    EXPECT_EQ(hex, number.asHex());
}

TEST_F(NumericHash, number_low_32)
{
    // Little endian
    const auto raw = ot::ByteArray{
        ot::IsHex,
        "0x0100000000000000000000000000000000000000000000000000000000000000"};
    const ot::UnallocatedCString decimal{"1"};
    // Big endian
    const ot::UnallocatedCString hex{
        "0000000000000000000000000000000000000000000000000000000000000001"};
    const auto number = ot::blockchain::block::NumericHash{raw};

    EXPECT_EQ(decimal, number.Decimal());
    EXPECT_EQ(hex, number.asHex());
}

TEST_F(NumericHash, number_high)
{
    // Little endian
    const auto raw = ot::ByteArray{
        ot::IsHex,
        "0xf1ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"};
    const ot::UnallocatedCString decimal{
        "1157920892373161954235709850086879078532699846656405640394575840079131"
        "29639921"};
    // Big endian
    const ot::UnallocatedCString hex{
        "fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1"};
    const auto number = ot::blockchain::block::NumericHash{raw};

    EXPECT_EQ(decimal, number.Decimal());
    EXPECT_EQ(hex, number.asHex());
}

TEST_F(NumericHash, nBits_1)
{
    const std::int32_t nBits{83923508};  // 0x05009234
    const ot::UnallocatedCString decimal{"2452881408"};
    const ot::UnallocatedCString hex{
        "0000000000000000000000000000000000000000000000000000000092340000"};
    const auto number = ot::blockchain::block::NumericHash{nBits};

    EXPECT_EQ(decimal, number.Decimal());
    EXPECT_EQ(hex, number.asHex());
}

TEST_F(NumericHash, nBits_2)
{
    const std::int32_t nBits{68301910};  // 0x04123456
    const ot::UnallocatedCString decimal{"305419776"};
    const ot::UnallocatedCString hex{
        "0000000000000000000000000000000000000000000000000000000012345600"};
    const auto number = ot::blockchain::block::NumericHash{nBits};

    EXPECT_EQ(decimal, number.Decimal());
    EXPECT_EQ(hex, number.asHex());
}

TEST_F(NumericHash, nBits_3)
{
    const std::int32_t nBits{404472624};  // 0x81bc330
    const ot::UnallocatedCString decimal{
        "680733321990486529407107157001552378184394215934016880640"};
    const ot::UnallocatedCString hex{
        "00000000000000001bc330000000000000000000000000000000000000000000"};
    const auto number = ot::blockchain::block::NumericHash{nBits};

    EXPECT_EQ(decimal, number.Decimal());
    EXPECT_EQ(hex, number.asHex());
}

TEST_F(NumericHash, nBits_4)
{
    const std::int32_t nBits{453248203};  // 0x1b0404cb
    const ot::UnallocatedCString decimal{
        "1653206561150525499452195696179626311675293455763937233695932416"};
    const ot::UnallocatedCString hex{
        "00000000000404cb000000000000000000000000000000000000000000000000"};
    const auto number = ot::blockchain::block::NumericHash{nBits};

    EXPECT_EQ(decimal, number.Decimal());
    EXPECT_EQ(hex, number.asHex());
}

TEST_F(NumericHash, nBits_5)
{
    const std::int32_t nBits{486604799};  // 0x1d00ffff
    const ot::UnallocatedCString decimal{
        "26959535291011309493156476344723991336010898738574164086137773096960"};
    const ot::UnallocatedCString hex{
        "00000000ffff0000000000000000000000000000000000000000000000000000"};
    const auto number = ot::blockchain::block::NumericHash{nBits};
    const auto work =
        ot::blockchain::Work{number, ot::blockchain::Type::Bitcoin};

    EXPECT_EQ(decimal, number.Decimal());
    EXPECT_EQ(hex, number.asHex());
    EXPECT_STREQ("1", work.Decimal().c_str());
}
}  // namespace ottest
