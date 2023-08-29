// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace ot = opentxs;

namespace ottest
{
using namespace opentxs::literals;
using namespace std::literals;

TEST(DisplayScale, usd_format)
{
    const auto usd = opentxs::display::GetDefinition(opentxs::UnitType::Usd);

    const auto amount1 = opentxs::Amount{14000000000};
    const auto amount2 = opentxs::Amount{14000000880};
    const auto amount3 = opentxs::Amount{14000000500};
    const auto amount4 = opentxs::Amount{14000015000};

    EXPECT_EQ(usd.Format(amount1), usd.Format(amount1, 0));
    EXPECT_EQ(
        usd.Format(amount1, 0),
        ot::UnallocatedCString{u8"$14,000,000,000.00"sv});
    EXPECT_EQ(usd.Import(usd.Format(amount1, 0), 0), amount1);
    EXPECT_EQ(
        usd.Format(amount1, 1),
        ot::UnallocatedCString{u8"1,400,000,000,000 ¢"sv});
    EXPECT_EQ(usd.Import(usd.Format(amount1, 1), 1), amount1);
    EXPECT_EQ(usd.Format(amount1, 2), ot::UnallocatedCString{u8"$14,000 MM"sv});
    EXPECT_EQ(usd.Import(usd.Format(amount1, 2), 2), amount1);
    EXPECT_EQ(
        usd.Format(amount1, 3),
        ot::UnallocatedCString{u8"14,000,000,000,000 ₥"sv});
    EXPECT_EQ(usd.Import(usd.Format(amount1, 3), 3), amount1);

    EXPECT_EQ(
        usd.Format(amount2, 0),
        ot::UnallocatedCString{u8"$14,000,000,880.00"sv});
    EXPECT_EQ(
        usd.Format(amount2, 1),
        ot::UnallocatedCString{u8"1,400,000,088,000 ¢"sv});
    EXPECT_EQ(
        usd.Format(amount2, 2),
        ot::UnallocatedCString{u8"$14,000.000\u202F88 MM"sv});
    EXPECT_EQ(
        usd.Format(amount2, 2, std::nullopt, 2),
        ot::UnallocatedCString{u8"$14,000 MM"sv});
    EXPECT_EQ(
        usd.Format(amount2, 2, 0, 2), ot::UnallocatedCString{u8"$14,000 MM"sv});
    EXPECT_EQ(
        usd.Format(amount2, 2, 1, 2),
        ot::UnallocatedCString{u8"$14,000.0 MM"sv});

    EXPECT_EQ(
        usd.Format(amount3, 2, 1, 2),
        ot::UnallocatedCString{u8"$14,000.0 MM"sv});

    EXPECT_EQ(
        usd.Format(amount4, 2, 1, 2),
        ot::UnallocatedCString{u8"$14,000.02 MM"sv});

    const auto amount5 = opentxs::Amount{-100};
    EXPECT_EQ(usd.Format(amount5, 0), ot::UnallocatedCString{u8"$-100.00"sv});

    const auto commaTest =
        ot::UnallocatedVector<std::pair<opentxs::Amount, std::string_view>>{
            {00, u8"$0"sv},
            {1, u8"$1"sv},
            {10, u8"$10"sv},
            {100, u8"$100"sv},
            {1000, u8"$1,000"sv},
            {10000, u8"$10,000"sv},
            {100000, u8"$100,000"sv},
            {1000000, u8"$1,000,000"sv},
            {10000000, u8"$10,000,000"sv},
            {100000000, u8"$100,000,000"sv},
            {1000000000, u8"$1,000,000,000"sv},
            {10000000000, u8"$10,000,000,000"sv},
            {100000000000, u8"$100,000,000,000"sv},
            {-1, u8"$-1"sv},
            {-10, u8"$-10"sv},
            {-100, u8"$-100"sv},
            {-1000, u8"$-1,000"sv},
            {-10000, u8"$-10,000"sv},
            {-100000, u8"$-100,000"sv},
            {-1000000, u8"$-1,000,000"sv},
            {-10000000, u8"$-10,000,000"sv},
            {-100000000, u8"$-100,000,000"sv},
            {-1000000000, u8"$-1,000,000,000"sv},
            {-10000000000, u8"$-10,000,000,000"sv},
            {-100000000000, u8"$-100,000,000,000"sv},
            {-1234567890, u8"$-1,234,567,890"sv},
            {-123456789, u8"$-123,456,789"sv},
            {-12345678, u8"$-12,345,678"sv},
            {-1234567, u8"$-1,234,567"sv},
            {-123456, u8"$-123,456"sv},
            {-12345, u8"$-12,345"sv},
            {-1234, u8"$-1,234"sv},
            {-123, u8"$-123"sv},
            {-12, u8"$-12"sv},
            {-1, u8"$-1"sv},
            {0, u8"$0"sv},
            {1, u8"$1"sv},
            {12, u8"$12"sv},
            {123, u8"$123"sv},
            {1234, u8"$1,234"sv},
            {12345, u8"$12,345"sv},
            {123456, u8"$123,456"sv},
            {1234567, u8"$1,234,567"sv},
            {12345678, u8"$12,345,678"sv},
            {123456789, u8"$123,456,789"sv},
            {1234567890, u8"$1,234,567,890"sv},
            {opentxs::signed_amount(-1234, 5, 10), u8"$-1,234.5"sv},
            {opentxs::signed_amount(-123, 4, 10), u8"$-123.4"sv},
            {opentxs::signed_amount(-12, 3, 10), u8"$-12.3"sv},
            {opentxs::signed_amount(-1, 2, 10), u8"$-1.2"sv},
            {-opentxs::signed_amount(0, 1, 10), u8"$-0.1"sv},
            {opentxs::signed_amount(0, 1, 10), u8"$0.1"sv},
            {opentxs::signed_amount(1, 2, 10), u8"$1.2"sv},
            {opentxs::signed_amount(12, 3, 10), u8"$12.3"sv},
            {opentxs::signed_amount(123, 4, 10), u8"$123.4"sv},
            {opentxs::signed_amount(1234, 5, 10), u8"$1,234.5"sv},
            {-opentxs::signed_amount(0, 12345, 100000), u8"$-0.123"sv},
            {-opentxs::signed_amount(0, 1234, 10000), u8"$-0.123"sv},
            {-opentxs::signed_amount(0, 123, 1000), u8"$-0.123"sv},
            {-opentxs::signed_amount(0, 12, 100), u8"$-0.12"sv},
            {opentxs::signed_amount(0, 12, 100), u8"$0.12"sv},
            {opentxs::signed_amount(0, 123, 1000), u8"$0.123"sv},
            {opentxs::signed_amount(0, 1234, 10000), u8"$0.123"sv},
            {opentxs::signed_amount(0, 12345, 100000), u8"$0.123"sv},
        };

    for (const auto& [amount, expected] : commaTest) {
        EXPECT_EQ(usd.Format(amount, 0, 0), expected);
    }
}

TEST(DisplayScale, usd_fractions)
{
    const auto usd = opentxs::display::GetDefinition(opentxs::UnitType::Usd);

    auto half = opentxs::signed_amount(0, 5, 10);
    EXPECT_EQ(usd.Format(half, 0), ot::UnallocatedCString{u8"$0.50"sv});

    half = opentxs::unsigned_amount(1, 5, 10);
    EXPECT_EQ(usd.Format(half, 0), ot::UnallocatedCString{u8"$1.50"sv});

    auto threequarter = opentxs::signed_amount(2, 75, 100);
    EXPECT_EQ(usd.Format(threequarter, 0), ot::UnallocatedCString{u8"$2.75"sv});

    threequarter = opentxs::unsigned_amount(3, 75, 100);
    EXPECT_EQ(usd.Format(threequarter, 0), ot::UnallocatedCString{u8"$3.75"sv});

    auto seveneighths = opentxs::signed_amount(4, 7, 8);
    EXPECT_EQ(
        usd.Format(seveneighths, 0, 0, 3),
        ot::UnallocatedCString{u8"$4.875"sv});

    seveneighths = opentxs::unsigned_amount(5, 7, 8);
    EXPECT_EQ(
        usd.Format(seveneighths, 0, 0, 3),
        ot::UnallocatedCString{u8"$5.875"sv});
}

TEST(DisplayScale, usd_limits)
{
    const auto usd = opentxs::display::GetDefinition(opentxs::UnitType::Usd);

    const auto largest_whole_number =
        *usd.Import(u8"115792089237316195423570985008687907853"sv
                    u8"269984665640564039457584007913129639935"sv);
    EXPECT_EQ(
        usd.Format(largest_whole_number, 0),
        ot::UnallocatedCString{
            u8"$115,792,089,237,316,195,423,570,985,008,687,907,853,"sv
            u8"269,984,665,640,564,039,457,584,007,913,129,639,935.00"sv});
    EXPECT_EQ(
        *usd.Import(usd.Format(largest_whole_number, 0), 0),
        largest_whole_number);

    try {
        const auto largest_whole_number_plus_one =
            usd.Import(u8"115792089237316195423570985008687907853"sv
                       u8"269984665640564039457584007913129639936"sv);
    } catch (std::out_of_range&) {
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }

    const auto smallest_fraction = opentxs::unsigned_amount(
        0, 1, std::numeric_limits<unsigned long long int>::max());

    EXPECT_EQ(
        usd.Import(u8"0."sv
                   u8"00000000000000000005421010862427"sv
                   u8"52217003726400434970855712890625"sv),
        smallest_fraction);

    EXPECT_EQ(
        usd.Format(smallest_fraction, 0, 0, 64),
        ot::UnallocatedCString{
            u8"$0.000\u202F000\u202F000\u202F000\u202F000\u202F000\u202F05"sv});
}

TEST(DisplayScale, usd_scales)
{
    const auto usd = opentxs::display::GetDefinition(opentxs::UnitType::Usd);

    EXPECT_EQ(usd.ScaleName(0u), u8"dollars"sv);
    EXPECT_EQ(usd.ScaleName(1u), u8"cents"sv);
    EXPECT_EQ(usd.ScaleName(2u), u8"millions"sv);
}

TEST(DisplayScale, btc)
{
    const auto btc = opentxs::display::GetDefinition(opentxs::UnitType::Btc);

    EXPECT_EQ(btc.ScaleName(0u), u8"BTC"sv);
    EXPECT_EQ(btc.ScaleName(1u), u8"mBTC"sv);
    EXPECT_EQ(btc.ScaleName(2u), u8"bits"sv);
    EXPECT_EQ(btc.ScaleName(3u), u8"μBTC"sv);
    EXPECT_EQ(btc.ScaleName(4u), u8"satoshi"sv);

    const auto amount1 = opentxs::Amount{100000000};
    const auto amount2 = opentxs::Amount{1};
    const auto amount3 = opentxs::Amount{2099999999999999};

    EXPECT_EQ(btc.Format(amount1, 0), ot::UnallocatedCString{u8"1 ₿"sv});
    EXPECT_EQ(btc.Import(btc.Format(amount1, 0), 0), amount1);
    EXPECT_EQ(btc.Format(amount1, 1), ot::UnallocatedCString{u8"1,000 mBTC"sv});
    EXPECT_EQ(btc.Import(btc.Format(amount1, 1), 1), amount1);
    EXPECT_EQ(
        btc.Format(amount1, 2), ot::UnallocatedCString{u8"1,000,000 bits"sv});
    EXPECT_EQ(btc.Import(btc.Format(amount1, 2), 2), amount1);
    EXPECT_EQ(
        btc.Format(amount1, 3), ot::UnallocatedCString{u8"1,000,000 μBTC"sv});
    EXPECT_EQ(btc.Import(btc.Format(amount1, 3), 3), amount1);
    EXPECT_EQ(
        btc.Format(amount1, 4), ot::UnallocatedCString{u8"100,000,000 sats"sv});
    EXPECT_EQ(btc.Import(btc.Format(amount1, 4), 4), amount1);

    EXPECT_EQ(
        btc.Format(amount2, 0),
        ot::UnallocatedCString{u8"0.000\u202F000\u202F01 ₿"sv});
    EXPECT_EQ(
        btc.Format(amount2, 1),
        ot::UnallocatedCString{u8"0.000\u202F01 mBTC"sv});
    EXPECT_EQ(btc.Format(amount2, 3), ot::UnallocatedCString{u8"0.01 μBTC"sv});

    EXPECT_EQ(
        btc.Format(amount3, 0),
        ot::UnallocatedCString{u8"20,999,999.999\u202F999\u202F99 ₿"sv});
    EXPECT_EQ(btc.Import(btc.Format(amount3, 0), 0), amount3);
}

TEST(DisplayScale, pkt)
{
    const auto pkt = opentxs::display::GetDefinition(opentxs::UnitType::Pkt);

    EXPECT_EQ(pkt.ScaleName(0u), u8"PKT"sv);
    EXPECT_EQ(pkt.ScaleName(1u), u8"mPKT"sv);
    EXPECT_EQ(pkt.ScaleName(2u), u8"μPKT"sv);
    EXPECT_EQ(pkt.ScaleName(3u), u8"nPKT"sv);
    EXPECT_EQ(pkt.ScaleName(4u), u8"pack"sv);

    const auto amount1 = opentxs::Amount{1073741824};
    const auto amount2 = opentxs::Amount{1};

    EXPECT_EQ(pkt.Format(amount1, 0), ot::UnallocatedCString{u8"1 PKT"sv});
    EXPECT_EQ(pkt.Import(pkt.Format(amount1, 0), 0), amount1);
    EXPECT_EQ(pkt.Format(amount1, 1), ot::UnallocatedCString{u8"1,000 mPKT"sv});
    EXPECT_EQ(pkt.Import(pkt.Format(amount1, 1), 1), amount1);
    EXPECT_EQ(
        pkt.Format(amount1, 2), ot::UnallocatedCString{u8"1,000,000 μPKT"sv});
    EXPECT_EQ(pkt.Import(pkt.Format(amount1, 2), 2), amount1);
    EXPECT_EQ(
        pkt.Format(amount1, 3),
        ot::UnallocatedCString{u8"1,000,000,000 nPKT"sv});
    EXPECT_EQ(pkt.Import(pkt.Format(amount1, 3), 3), amount1);
    EXPECT_EQ(
        pkt.Format(amount1, 4),
        ot::UnallocatedCString{u8"1,073,741,824 pack"sv});
    EXPECT_EQ(pkt.Import(pkt.Format(amount1, 4), 4), amount1);

    EXPECT_EQ(
        pkt.Format(amount2, 0),
        ot::UnallocatedCString{u8"0.000\u202F000\u202F000\u202F93 PKT"sv});
    EXPECT_EQ(
        pkt.Format(amount2, 1),
        ot::UnallocatedCString{u8"0.000\u202F000\u202F93 mPKT"sv});
    EXPECT_EQ(
        pkt.Format(amount2, 2),
        ot::UnallocatedCString{u8"0.000\u202F93 μPKT"sv});
    EXPECT_EQ(pkt.Format(amount2, 3), ot::UnallocatedCString{u8"0.93 nPKT"sv});
    EXPECT_EQ(pkt.Format(amount2, 4), ot::UnallocatedCString{u8"1 pack"sv});
}
}  // namespace ottest
