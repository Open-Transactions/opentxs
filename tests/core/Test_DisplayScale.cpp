// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <iterator>
#include <limits>
#include <optional>
#include <stdexcept>
#include <utility>

#include "opentxs/Version.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/UnitType.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/util/Container.hpp"

namespace ot = opentxs;

namespace ottest
{
TEST(DisplayScale, usd_format)
{
    const auto usd = opentxs::display::GetDefinition(opentxs::UnitType::Usd);

    const auto amount1 = opentxs::Amount{14000000000};
    const auto amount2 = opentxs::Amount{14000000880};
    const auto amount3 = opentxs::Amount{14000000500};
    const auto amount4 = opentxs::Amount{14000015000};

    EXPECT_EQ(usd.Format(amount1), usd.Format(amount1, 0));
    EXPECT_EQ(
        usd.Format(amount1, 0), ot::UnallocatedCString{u8"$14,000,000,000.00"});
    EXPECT_EQ(usd.Import(usd.Format(amount1, 0), 0), amount1);
    EXPECT_EQ(
        usd.Format(amount1, 1),
        ot::UnallocatedCString{u8"1,400,000,000,000 ¢"});
    EXPECT_EQ(usd.Import(usd.Format(amount1, 1), 1), amount1);
    EXPECT_EQ(usd.Format(amount1, 2), ot::UnallocatedCString{u8"$14,000 MM"});
    EXPECT_EQ(usd.Import(usd.Format(amount1, 2), 2), amount1);
    EXPECT_EQ(
        usd.Format(amount1, 3),
        ot::UnallocatedCString{u8"14,000,000,000,000 ₥"});
    EXPECT_EQ(usd.Import(usd.Format(amount1, 3), 3), amount1);

    EXPECT_EQ(
        usd.Format(amount2, 0), ot::UnallocatedCString{u8"$14,000,000,880.00"});
    EXPECT_EQ(
        usd.Format(amount2, 1),
        ot::UnallocatedCString{u8"1,400,000,088,000 ¢"});
    EXPECT_EQ(
        usd.Format(amount2, 2),
        ot::UnallocatedCString{u8"$14,000.000\u202F88 MM"});
    EXPECT_EQ(
        usd.Format(amount2, 2, std::nullopt, 2),
        ot::UnallocatedCString{u8"$14,000 MM"});
    EXPECT_EQ(
        usd.Format(amount2, 2, 0, 2), ot::UnallocatedCString{u8"$14,000 MM"});
    EXPECT_EQ(
        usd.Format(amount2, 2, 1, 2), ot::UnallocatedCString{u8"$14,000.0 MM"});

    EXPECT_EQ(
        usd.Format(amount3, 2, 1, 2), ot::UnallocatedCString{u8"$14,000.0 MM"});

    EXPECT_EQ(
        usd.Format(amount4, 2, 1, 2),
        ot::UnallocatedCString{u8"$14,000.02 MM"});

    const auto amount5 = opentxs::Amount{-100};
    EXPECT_EQ(usd.Format(amount5, 0), ot::UnallocatedCString{u8"$-100.00"});

    const auto commaTest = ot::UnallocatedVector<
        std::pair<opentxs::Amount, ot::UnallocatedCString>>{
        {00, u8"$0"},
        {1, u8"$1"},
        {10, u8"$10"},
        {100, u8"$100"},
        {1000, u8"$1,000"},
        {10000, u8"$10,000"},
        {100000, u8"$100,000"},
        {1000000, u8"$1,000,000"},
        {10000000, u8"$10,000,000"},
        {100000000, u8"$100,000,000"},
        {1000000000, u8"$1,000,000,000"},
        {10000000000, u8"$10,000,000,000"},
        {100000000000, u8"$100,000,000,000"},
        {-1, u8"$-1"},
        {-10, u8"$-10"},
        {-100, u8"$-100"},
        {-1000, u8"$-1,000"},
        {-10000, u8"$-10,000"},
        {-100000, u8"$-100,000"},
        {-1000000, u8"$-1,000,000"},
        {-10000000, u8"$-10,000,000"},
        {-100000000, u8"$-100,000,000"},
        {-1000000000, u8"$-1,000,000,000"},
        {-10000000000, u8"$-10,000,000,000"},
        {-100000000000, u8"$-100,000,000,000"},
        {-1234567890, u8"$-1,234,567,890"},
        {-123456789, u8"$-123,456,789"},
        {-12345678, u8"$-12,345,678"},
        {-1234567, u8"$-1,234,567"},
        {-123456, u8"$-123,456"},
        {-12345, u8"$-12,345"},
        {-1234, u8"$-1,234"},
        {-123, u8"$-123"},
        {-12, u8"$-12"},
        {-1, u8"$-1"},
        {0, u8"$0"},
        {1, u8"$1"},
        {12, u8"$12"},
        {123, u8"$123"},
        {1234, u8"$1,234"},
        {12345, u8"$12,345"},
        {123456, u8"$123,456"},
        {1234567, u8"$1,234,567"},
        {12345678, u8"$12,345,678"},
        {123456789, u8"$123,456,789"},
        {1234567890, u8"$1,234,567,890"},
        {opentxs::signed_amount(-1234, 5, 10), u8"$-1,234.5"},
        {opentxs::signed_amount(-123, 4, 10), u8"$-123.4"},
        {opentxs::signed_amount(-12, 3, 10), u8"$-12.3"},
        {opentxs::signed_amount(-1, 2, 10), u8"$-1.2"},
        {-opentxs::signed_amount(0, 1, 10), u8"$-0.1"},
        {opentxs::signed_amount(0, 1, 10), u8"$0.1"},
        {opentxs::signed_amount(1, 2, 10), u8"$1.2"},
        {opentxs::signed_amount(12, 3, 10), u8"$12.3"},
        {opentxs::signed_amount(123, 4, 10), u8"$123.4"},
        {opentxs::signed_amount(1234, 5, 10), u8"$1,234.5"},
        {-opentxs::signed_amount(0, 12345, 100000), u8"$-0.123"},
        {-opentxs::signed_amount(0, 1234, 10000), u8"$-0.123"},
        {-opentxs::signed_amount(0, 123, 1000), u8"$-0.123"},
        {-opentxs::signed_amount(0, 12, 100), u8"$-0.12"},
        {opentxs::signed_amount(0, 12, 100), u8"$0.12"},
        {opentxs::signed_amount(0, 123, 1000), u8"$0.123"},
        {opentxs::signed_amount(0, 1234, 10000), u8"$0.123"},
        {opentxs::signed_amount(0, 12345, 100000), u8"$0.123"},
    };

    for (const auto& [amount, expected] : commaTest) {
        EXPECT_EQ(usd.Format(amount, 0, 0), expected);
    }
}

TEST(DisplayScale, usd_fractions)
{
    const auto usd = opentxs::display::GetDefinition(opentxs::UnitType::Usd);

    auto half = opentxs::signed_amount(0, 5, 10);
    EXPECT_EQ(usd.Format(half, 0), ot::UnallocatedCString{u8"$0.50"});

    half = opentxs::unsigned_amount(1, 5, 10);
    EXPECT_EQ(usd.Format(half, 0), ot::UnallocatedCString{u8"$1.50"});

    auto threequarter = opentxs::signed_amount(2, 75, 100);
    EXPECT_EQ(usd.Format(threequarter, 0), ot::UnallocatedCString{u8"$2.75"});

    threequarter = opentxs::unsigned_amount(3, 75, 100);
    EXPECT_EQ(usd.Format(threequarter, 0), ot::UnallocatedCString{u8"$3.75"});

    auto seveneighths = opentxs::signed_amount(4, 7, 8);
    EXPECT_EQ(
        usd.Format(seveneighths, 0, 0, 3), ot::UnallocatedCString{u8"$4.875"});

    seveneighths = opentxs::unsigned_amount(5, 7, 8);
    EXPECT_EQ(
        usd.Format(seveneighths, 0, 0, 3), ot::UnallocatedCString{u8"$5.875"});
}

TEST(DisplayScale, usd_limits)
{
    const auto usd = opentxs::display::GetDefinition(opentxs::UnitType::Usd);

    const auto largest_whole_number =
        usd.Import(u8"115792089237316195423570985008687907853"
                   u8"269984665640564039457584007913129639935");
    EXPECT_EQ(
        usd.Format(largest_whole_number, 0),
        ot::UnallocatedCString{
            u8"$115,792,089,237,316,195,423,570,985,008,687,907,853,"
            u8"269,984,665,640,564,039,457,584,007,913,129,639,935.00"});
    EXPECT_EQ(
        usd.Import(usd.Format(largest_whole_number, 0), 0),
        largest_whole_number);

    try {
        const auto largest_whole_number_plus_one =
            usd.Import(u8"115792089237316195423570985008687907853"
                       u8"269984665640564039457584007913129639936");
    } catch (std::out_of_range&) {
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }

    const auto smallest_fraction = opentxs::unsigned_amount(
        0, 1, std::numeric_limits<unsigned long long int>::max());

    EXPECT_EQ(
        usd.Import(u8"0."
                   u8"00000000000000000005421010862427"
                   u8"52217003726400434970855712890625"),
        smallest_fraction);

    EXPECT_EQ(
        usd.Format(smallest_fraction, 0, 0, 64),
        ot::UnallocatedCString{
            u8"$0.000\u202F000\u202F000\u202F000\u202F000\u202F000\u202F05"});
}

TEST(DisplayScale, usd_scales)
{
    const auto usd = opentxs::display::GetDefinition(opentxs::UnitType::Usd);

    const auto scales = usd.GetScales();
    auto it = scales.begin();

    {
        const auto& [index, name] = *it;

        EXPECT_EQ(index, 0);
        EXPECT_EQ(name, "dollars");
    }

    std::advance(it, 1);

    {
        const auto& [index, name] = *it;

        EXPECT_EQ(index, 1);
        EXPECT_EQ(name, "cents");
    }

    std::advance(it, 1);

    {
        const auto& [index, name] = *it;

        EXPECT_EQ(index, 2);
        EXPECT_EQ(name, "millions");
    }
}

TEST(DisplayScale, btc)
{
    const auto btc = opentxs::display::GetDefinition(opentxs::UnitType::Btc);

    const auto scales = btc.GetScales();
    auto it = scales.begin();

    {
        const auto& [index, name] = *it;

        EXPECT_EQ(index, 0);
        EXPECT_EQ(name, u8"BTC");
    }

    std::advance(it, 1);

    {
        const auto& [index, name] = *it;

        EXPECT_EQ(index, 1);
        EXPECT_EQ(name, u8"mBTC");
    }

    std::advance(it, 1);

    {
        const auto& [index, name] = *it;

        EXPECT_EQ(index, 2);
        EXPECT_EQ(name, u8"bits");
    }

    std::advance(it, 1);

    {
        const auto& [index, name] = *it;

        EXPECT_EQ(index, 3);
        EXPECT_EQ(name, u8"μBTC");
    }

    std::advance(it, 1);

    {
        const auto& [index, name] = *it;

        EXPECT_EQ(index, 4);
        EXPECT_EQ(name, u8"satoshi");
    }

    const auto amount1 = opentxs::Amount{100000000};
    const auto amount2 = opentxs::Amount{1};
    const auto amount3 = opentxs::Amount{2099999999999999};

    EXPECT_EQ(btc.Format(amount1, 0), ot::UnallocatedCString{u8"1 ₿"});
    EXPECT_EQ(btc.Import(btc.Format(amount1, 0), 0), amount1);
    EXPECT_EQ(btc.Format(amount1, 1), ot::UnallocatedCString{u8"1,000 mBTC"});
    EXPECT_EQ(btc.Import(btc.Format(amount1, 1), 1), amount1);
    EXPECT_EQ(
        btc.Format(amount1, 2), ot::UnallocatedCString{u8"1,000,000 bits"});
    EXPECT_EQ(btc.Import(btc.Format(amount1, 2), 2), amount1);
    EXPECT_EQ(
        btc.Format(amount1, 3), ot::UnallocatedCString{u8"1,000,000 μBTC"});
    EXPECT_EQ(btc.Import(btc.Format(amount1, 3), 3), amount1);
    EXPECT_EQ(
        btc.Format(amount1, 4),
        ot::UnallocatedCString{u8"100,000,000 satoshis"});
    EXPECT_EQ(btc.Import(btc.Format(amount1, 4), 4), amount1);

    EXPECT_EQ(
        btc.Format(amount2, 0),
        ot::UnallocatedCString{u8"0.000\u202F000\u202F01 ₿"});
    EXPECT_EQ(
        btc.Format(amount2, 1), ot::UnallocatedCString{u8"0.000\u202F01 mBTC"});
    EXPECT_EQ(btc.Format(amount2, 3), ot::UnallocatedCString{u8"0.01 μBTC"});

    EXPECT_EQ(
        btc.Format(amount3, 0),
        ot::UnallocatedCString{u8"20,999,999.999\u202F999\u202F99 ₿"});
    EXPECT_EQ(btc.Import(btc.Format(amount3, 0), 0), amount3);
}

TEST(DisplayScale, pkt)
{
    const auto pkt = opentxs::display::GetDefinition(opentxs::UnitType::Pkt);

    const auto scales = pkt.GetScales();
    auto it = scales.begin();

    {
        const auto& [index, name] = *it;

        EXPECT_EQ(index, 0);
        EXPECT_EQ(name, u8"PKT");
    }

    std::advance(it, 1);

    {
        const auto& [index, name] = *it;

        EXPECT_EQ(index, 1);
        EXPECT_EQ(name, u8"mPKT");
    }

    std::advance(it, 1);

    {
        const auto& [index, name] = *it;

        EXPECT_EQ(index, 2);
        EXPECT_EQ(name, u8"μPKT");
    }

    std::advance(it, 1);

    {
        const auto& [index, name] = *it;

        EXPECT_EQ(index, 3);
        EXPECT_EQ(name, u8"nPKT");
    }

    std::advance(it, 1);

    {
        const auto& [index, name] = *it;

        EXPECT_EQ(index, 4);
        EXPECT_EQ(name, u8"pack");
    }

    const auto amount1 = opentxs::Amount{1073741824};
    const auto amount2 = opentxs::Amount{1};

    EXPECT_EQ(pkt.Format(amount1, 0), ot::UnallocatedCString{u8"1 PKT"});
    EXPECT_EQ(pkt.Import(pkt.Format(amount1, 0), 0), amount1);
    EXPECT_EQ(pkt.Format(amount1, 1), ot::UnallocatedCString{u8"1,000 mPKT"});
    EXPECT_EQ(pkt.Import(pkt.Format(amount1, 1), 1), amount1);
    EXPECT_EQ(
        pkt.Format(amount1, 2), ot::UnallocatedCString{u8"1,000,000 μPKT"});
    EXPECT_EQ(pkt.Import(pkt.Format(amount1, 2), 2), amount1);
    EXPECT_EQ(
        pkt.Format(amount1, 3), ot::UnallocatedCString{u8"1,000,000,000 nPKT"});
    EXPECT_EQ(pkt.Import(pkt.Format(amount1, 3), 3), amount1);
    EXPECT_EQ(
        pkt.Format(amount1, 4), ot::UnallocatedCString{u8"1,073,741,824 pack"});
    EXPECT_EQ(pkt.Import(pkt.Format(amount1, 4), 4), amount1);

    EXPECT_EQ(
        pkt.Format(amount2, 0),
        ot::UnallocatedCString{u8"0.000\u202F000\u202F000\u202F93 PKT"});
    EXPECT_EQ(
        pkt.Format(amount2, 1),
        ot::UnallocatedCString{u8"0.000\u202F000\u202F93 mPKT"});
    EXPECT_EQ(
        pkt.Format(amount2, 2), ot::UnallocatedCString{u8"0.000\u202F93 μPKT"});
    EXPECT_EQ(pkt.Format(amount2, 3), ot::UnallocatedCString{u8"0.93 nPKT"});
    EXPECT_EQ(pkt.Format(amount2, 4), ot::UnallocatedCString{u8"1 pack"});
}
}  // namespace ottest
