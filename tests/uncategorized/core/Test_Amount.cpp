// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <boost/exception/exception.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <limits>
#include <memory>
#include <span>
#include <stdexcept>
#include <utility>

#include "internal/core/Amount.hpp"
#include "internal/core/Factory.hpp"

namespace ot = opentxs;
namespace bmp = boost::multiprecision;

namespace ottest
{
using namespace opentxs::literals;

constexpr auto int_max = std::numeric_limits<int>::max();
constexpr auto int_min = std::numeric_limits<int>::min();
constexpr auto long_max = std::numeric_limits<long int>::max();
constexpr auto long_min = std::numeric_limits<long int>::min();
constexpr auto longlong_max = std::numeric_limits<long long int>::max();
constexpr auto longlong_min = std::numeric_limits<long long int>::min();
constexpr auto uint_max = std::numeric_limits<unsigned int>::max();
constexpr auto uint_min = std::numeric_limits<unsigned int>::min();
constexpr auto ulong_max = std::numeric_limits<unsigned long int>::max();
constexpr auto ulong_min = std::numeric_limits<unsigned long int>::min();
constexpr auto ulonglong_max =
    std::numeric_limits<unsigned long long int>::max();
constexpr auto ulonglong_min =
    std::numeric_limits<unsigned long long int>::min();

TEST(Amount, limits)
{
    try {
        const auto max_backend =
            u8"115792089237316195423570985008687907853269984665640564039457584007913129639935"_sv;
        const auto backend_amount = ot::factory::Amount(max_backend, true);
        EXPECT_TRUE(true);
    } catch (std::overflow_error&) {
        EXPECT_TRUE(false);
    } catch (...) {
        EXPECT_TRUE(false);
    }

    try {
        const auto max_backend_plus_one =
            u8"115792089237316195423570985008687907853269984665640564039457584007913129639936"_sv;
        const auto backend_amount =
            ot::factory::Amount(max_backend_plus_one, true);
        EXPECT_TRUE(false);
    } catch (std::overflow_error&) {
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }
}

TEST(Amount, default_constructor)
{
    const auto amount = ot::Amount();

    ASSERT_TRUE(amount == 0LL);
}

TEST(Amount, int_constructor)
{
    const auto max_amount = ot::Amount(int_max);

    ASSERT_TRUE(max_amount == int_max);

    const auto min_amount = ot::Amount(int_min);

    ASSERT_TRUE(min_amount == int_min);
}

TEST(Amount, long_constructor)
{
    const auto max_amount = ot::Amount(long_max);

    ASSERT_TRUE(max_amount == long_max);

    const auto min_amount = ot::Amount(long_min);

    ASSERT_TRUE(min_amount == long_min);
}

TEST(Amount, longlong_constructor)
{
    const auto max_amount = ot::Amount(longlong_max);

    ASSERT_TRUE(max_amount == longlong_max);

    const auto min_amount = ot::Amount(longlong_min);

    ASSERT_TRUE(min_amount == longlong_min);
}

TEST(Amount, uint_constructor)
{
    const auto max_amount = ot::Amount(uint_max);

    ASSERT_TRUE(max_amount == uint_max);

    const auto min_amount = ot::Amount(uint_min);

    ASSERT_TRUE(min_amount == uint_min);
}

TEST(Amount, ulong_constructor)
{
    const auto max_amount = ot::Amount(ulong_max);

    ASSERT_TRUE(max_amount == ulong_max);

    const auto min_amount = ot::Amount(ulong_min);

    ASSERT_TRUE(min_amount == ulong_min);
}

TEST(Amount, ulonglong_constructor)
{
    const auto max_amount = ot::Amount(ulonglong_max);

    ASSERT_TRUE(max_amount == ulonglong_max);

    const auto min_amount = ot::Amount(ulonglong_min);

    ASSERT_TRUE(min_amount == ulonglong_min);
}

TEST(Amount, string_constructor)
{
    const auto max_amount = ulonglong_max;
    const auto amount = ot::factory::Amount(std::to_string(max_amount), true);

    ASSERT_TRUE(amount == max_amount);

    try {
        const auto overflow =
            std::numeric_limits<bmp::checked_int1024_t>::max();
        const auto overflow_amount = ot::factory::Amount(overflow.str());
        EXPECT_TRUE(false);
    } catch (std::overflow_error&) {
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }

    try {
        const auto underflow =
            std::numeric_limits<bmp::checked_int1024_t>::min();
        const auto underflow_amount = ot::factory::Amount(underflow.str());
        EXPECT_TRUE(false);
    } catch (std::overflow_error&) {
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }
}

TEST(Amount, zmqframe_constructor)
{
    auto message = opentxs::network::zeromq::Message{};

    const auto max = ot::Amount{ulonglong_max};
    max.Serialize(message.AppendBytes());

    const auto amount = ot::factory::Amount(message.get()[0]);
    ASSERT_TRUE(amount == ulonglong_max);
}

TEST(Amount, copy_constructor)
{
    const auto amount = ot::Amount(ulonglong_max);

    const auto copy = ot::Amount(amount);

    ASSERT_TRUE(copy == ulonglong_max);
}

TEST(Amount, move_constructor)
{
    const auto amount = std::make_unique<ot::Amount>(ulonglong_max);

    const auto moved = ot::Amount(std::move(*amount));

    ASSERT_TRUE(moved == ulonglong_max);
}

TEST(Amount, copy_assignment)
{
    const auto amount = ot::Amount(ulonglong_max);

    auto copy = ot::Amount();

    copy = amount;

    ASSERT_TRUE(copy == ulonglong_max);
}

TEST(Amount, move_assignment)
{
    const auto amount = std::make_unique<ot::Amount>(ulonglong_max);

    auto moved = ot::Amount();

    moved = std::move(*amount);

    ASSERT_TRUE(moved == ulonglong_max);
}

TEST(Amount, unary_minus_operator)
{
    auto amount = ot::Amount{1};

    ASSERT_TRUE(-amount == -1);

    amount = ot::Amount{-1};

    ASSERT_TRUE(-amount == 1);
}

TEST(Amount, less_than)
{
    const ot::Amount int_amount{int_max - 1};

    ASSERT_TRUE(int_amount < int_max);
    ASSERT_FALSE(int_amount < int_max - 2);

    ASSERT_TRUE(int_max - 2 < int_amount);
    ASSERT_FALSE(int_max < int_amount);

    const ot::Amount long_amount{long_max - 1};

    ASSERT_TRUE(long_amount < long_max);
    ASSERT_FALSE(long_amount < long_max - 2);

    ASSERT_TRUE(long_max - 2 < long_amount);
    ASSERT_FALSE(long_max < long_amount);

    const ot::Amount longlong_amount{longlong_max - 1};

    ASSERT_TRUE(longlong_amount < longlong_max);
    ASSERT_FALSE(longlong_amount < longlong_max - 2);

    ASSERT_TRUE(longlong_max - 2 < longlong_amount);
    ASSERT_FALSE(longlong_max < longlong_amount);

    const ot::Amount ulonglong_amount{ulonglong_max - 1};

    ASSERT_TRUE(ulonglong_amount < ulonglong_max);
    ASSERT_FALSE(ulonglong_amount < ulonglong_max - 2);

    ASSERT_TRUE(ulonglong_max - 2 < ulonglong_amount);
    ASSERT_FALSE(ulonglong_max < ulonglong_amount);

    ASSERT_TRUE(ot::Amount{0} < ulonglong_amount);
    ASSERT_FALSE(ulonglong_amount < ot::Amount{0});
}

TEST(Amount, greater_than)
{
    const ot::Amount int_amount{int_max - 1};

    ASSERT_TRUE(int_amount > int_max - 2);
    ASSERT_FALSE(int_amount > int_max);

    ASSERT_TRUE(int_max > int_amount);
    ASSERT_FALSE(int_max - 2 > int_amount);

    const ot::Amount long_amount{long_max - 1};

    ASSERT_TRUE(long_amount > long_max - 2);
    ASSERT_FALSE(long_amount > long_max);

    ASSERT_TRUE(long_max > long_amount);
    ASSERT_FALSE(long_max - 2 > long_amount);

    const ot::Amount longlong_amount{longlong_max - 1};

    ASSERT_TRUE(longlong_amount > longlong_max - 2);
    ASSERT_FALSE(longlong_amount > longlong_max);

    ASSERT_TRUE(longlong_max > longlong_amount);
    ASSERT_FALSE(longlong_max - 2 > longlong_amount);

    const ot::Amount ulonglong_amount{ulonglong_max - 1};

    ASSERT_TRUE(ulonglong_amount > ulonglong_max - 2);
    ASSERT_FALSE(ulonglong_amount > ulonglong_max);

    ASSERT_TRUE(ulonglong_max > ulonglong_amount);
    ASSERT_FALSE(ulonglong_max - 2 > ulonglong_amount);

    ASSERT_TRUE(ot::Amount{ulonglong_max} > ulonglong_amount);
    ASSERT_FALSE(ot::Amount{0} > ulonglong_amount);
}

TEST(Amount, equal_to)
{
    const ot::Amount int_amount{int_max};

    ASSERT_TRUE(int_amount == int_max);
    ASSERT_FALSE(int_amount == int_max - 1);

    ASSERT_TRUE(int_max == int_amount);
    ASSERT_FALSE(int_max - 1 == int_amount);

    const ot::Amount long_amount{long_max};

    ASSERT_TRUE(long_amount == long_max);
    ASSERT_FALSE(long_amount == long_max - 1);

    ASSERT_TRUE(long_max == long_amount);
    ASSERT_FALSE(long_max - 1 == long_amount);

    const ot::Amount longlong_amount{longlong_max};

    ASSERT_TRUE(longlong_amount == longlong_max);
    ASSERT_FALSE(longlong_amount == longlong_max - 1);

    ASSERT_TRUE(longlong_max == longlong_amount);
    ASSERT_FALSE(longlong_max - 1 == longlong_amount);

    const ot::Amount uint_amount{uint_max};

    ASSERT_TRUE(uint_amount == uint_max);
    ASSERT_FALSE(uint_amount == uint_max - 1);

    const ot::Amount ulong_amount{ulong_max};

    ASSERT_TRUE(ulong_amount == ulong_max);
    ASSERT_FALSE(ulong_amount == ulong_max - 1);

    const ot::Amount ulonglong_amount{ulonglong_max};

    ASSERT_TRUE(ulonglong_amount == ulonglong_max);
    ASSERT_FALSE(ulonglong_amount == ulonglong_max - 1);

    ASSERT_TRUE(ulonglong_max == ulonglong_amount);
    ASSERT_FALSE(ulonglong_max - 1 == ulonglong_amount);

    ASSERT_TRUE(ot::Amount{ulonglong_max} == ulonglong_amount);
    ASSERT_FALSE(ot::Amount{0} == ulonglong_amount);
}

TEST(Amount, not_equal_to)
{
    const ot::Amount int_amount{int_max};

    ASSERT_TRUE(int_amount != int_max - 1);
    ASSERT_FALSE(int_amount != int_max);

    ASSERT_TRUE(int_max - 1 != int_amount);
    ASSERT_FALSE(int_max != int_amount);

    const ot::Amount long_amount{long_max};

    ASSERT_TRUE(long_amount != long_max - 1);
    ASSERT_FALSE(long_amount != long_max);

    const ot::Amount longlong_amount{longlong_max};

    ASSERT_TRUE(longlong_amount != longlong_max - 1);
    ASSERT_FALSE(longlong_amount != longlong_max);

    ASSERT_TRUE(longlong_max - 1 != longlong_amount);
    ASSERT_FALSE(longlong_max != longlong_amount);

    const ot::Amount ulonglong_amount{ulonglong_max};

    ASSERT_TRUE(ulonglong_amount != ulonglong_max - 1);
    ASSERT_FALSE(ulonglong_amount != ulonglong_max);

    ASSERT_TRUE(ulonglong_max - 1 != ulonglong_amount);
    ASSERT_FALSE(ulonglong_max != ulonglong_amount);

    ASSERT_TRUE(ot::Amount{ulonglong_max - 1} != ulonglong_amount);
    ASSERT_FALSE(ot::Amount{ulonglong_max} != ulonglong_amount);
}

TEST(Amount, less_than_or_equal_to)
{
    const ot::Amount int_amount{int_max - 1};

    ASSERT_TRUE(int_amount <= int_max);
    ASSERT_TRUE(int_amount <= int_max - 1);
    ASSERT_FALSE(int_amount <= int_max - 2);

    const ot::Amount long_amount{long_max - 1};

    ASSERT_TRUE(long_max - 2 <= long_amount);
    ASSERT_TRUE(long_max - 1 <= long_amount);
    ASSERT_FALSE(long_max <= long_amount);

    const ot::Amount longlong_amount{longlong_max - 1};

    ASSERT_TRUE(longlong_amount <= longlong_max);
    ASSERT_TRUE(longlong_amount <= longlong_max - 1);
    ASSERT_FALSE(longlong_amount <= longlong_max - 2);

    ASSERT_TRUE(longlong_max - 2 <= longlong_amount);
    ASSERT_TRUE(longlong_max - 1 <= longlong_amount);
    ASSERT_FALSE(longlong_max <= longlong_amount);

    const ot::Amount uint_amount{uint_max - 1};

    ASSERT_TRUE(uint_amount <= uint_max);
    ASSERT_TRUE(uint_amount <= uint_max - 1);
    ASSERT_FALSE(uint_amount <= uint_max - 2);

    const ot::Amount ulong_amount{ulong_max - 1};

    ASSERT_TRUE(ulong_amount <= ulong_max);
    ASSERT_TRUE(ulong_amount <= ulong_max - 1);
    ASSERT_FALSE(ulong_amount <= ulong_max - 2);

    const ot::Amount ulonglong_amount{ulonglong_max - 1};

    ASSERT_TRUE(ulonglong_amount <= ulonglong_max);
    ASSERT_TRUE(ulonglong_amount <= ulonglong_max - 1);
    ASSERT_FALSE(ulonglong_amount <= ulonglong_max - 2);

    ASSERT_TRUE(ulonglong_max - 2 <= ulonglong_amount);
    ASSERT_TRUE(ulonglong_max - 1 <= ulonglong_amount);
    ASSERT_FALSE(ulonglong_max <= ulonglong_amount);

    ASSERT_TRUE(ot::Amount{ulonglong_max - 2} <= ulonglong_amount);
    ASSERT_TRUE(ot::Amount{ulonglong_max - 1} <= ulonglong_amount);
    ASSERT_FALSE(ot::Amount{ulonglong_max} <= ulonglong_amount);
}

TEST(Amount, greater_than_or_equal_to)
{
    const ot::Amount int_amount{int_max - 1};

    ASSERT_TRUE(int_max >= int_amount);
    ASSERT_TRUE(int_max - 1 >= int_amount);
    ASSERT_FALSE(int_max - 2 >= int_amount);

    const ot::Amount long_amount{long_max - 1};

    ASSERT_TRUE(long_amount >= long_max - 2);
    ASSERT_TRUE(long_amount >= long_max - 1);
    ASSERT_FALSE(long_amount >= long_max);

    const ot::Amount longlong_amount{longlong_max - 1};

    ASSERT_TRUE(longlong_amount >= longlong_max - 2);
    ASSERT_TRUE(longlong_amount >= longlong_max - 1);
    ASSERT_FALSE(longlong_amount >= longlong_max);

    ASSERT_TRUE(longlong_max >= longlong_amount);
    ASSERT_TRUE(longlong_max - 1 >= longlong_amount);
    ASSERT_FALSE(longlong_max - 2 >= longlong_amount);

    const ot::Amount ulonglong_amount{ulonglong_max - 1};

    ASSERT_TRUE(ulonglong_amount >= ulonglong_max - 2);
    ASSERT_TRUE(ulonglong_amount >= ulonglong_max - 1);
    ASSERT_FALSE(ulonglong_amount >= ulonglong_max);

    ASSERT_TRUE(ulonglong_max >= ulonglong_amount);
    ASSERT_TRUE(ulonglong_max - 1 >= ulonglong_amount);
    ASSERT_FALSE(ulonglong_max - 2 >= ulonglong_amount);

    ASSERT_TRUE(ot::Amount{ulonglong_max} >= ulonglong_amount);
    ASSERT_TRUE(ot::Amount{ulonglong_max - 1} >= ulonglong_amount);
    ASSERT_FALSE(ot::Amount{ulonglong_max - 2} >= ulonglong_amount);
}

TEST(Amount, plus)
{
    const ot::Amount amount;

    ASSERT_TRUE(ot::Amount{1} + amount == 1);

    try {
        const auto max = std::numeric_limits<ot::amount::Integer>::max();
        auto amount_max = ot::factory::Amount(max.str());
        auto overflow = amount_max + ot::Amount{1};
        EXPECT_TRUE(false);
    } catch (std::overflow_error&) {
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }

    try {
        const auto max = std::numeric_limits<ot::amount::Integer>::max();
        auto amount_max = ot::factory::Amount(max.str());
        amount_max += ot::Amount{1};
        EXPECT_TRUE(false);
    } catch (std::overflow_error&) {
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }
}

TEST(Amount, minus)
{
    const ot::Amount amount;

    ASSERT_TRUE(ot::Amount{1} - amount == 1);

    auto ulonglong_amount = ot::Amount{3};

    ulonglong_amount -= ot::Amount{1};

    ASSERT_TRUE(ulonglong_amount == 2);

    try {
        const auto min = std::numeric_limits<ot::amount::Integer>::min();
        auto amount_min = ot::factory::Amount(min.str());
        auto underflow = amount_min - ot::Amount{1};
        EXPECT_TRUE(false);
    } catch (std::overflow_error&) {
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }

    try {
        const auto min = std::numeric_limits<ot::amount::Integer>::min();
        auto amount_min = ot::factory::Amount(min.str());
        amount_min -= ot::Amount{1};
        EXPECT_TRUE(false);
    } catch (std::overflow_error&) {
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }
}

TEST(Amount, multiply)
{
    const ot::Amount int_amount{int_max / 2};

    auto int_result = int_amount * 2;

    ASSERT_TRUE(int_result == ((int_max / 2) * 2));

    int_result = 2 * int_amount;

    ASSERT_TRUE(int_result == ((int_max / 2) * 2));

    const ot::Amount long_amount{long_max / 2};

    auto long_result = long_amount * 2L;

    ASSERT_TRUE(long_result == ((long_max / 2) * 2));

    const ot::Amount longlong_amount{longlong_max / 2};

    auto longlong_result = longlong_amount * 2LL;

    ASSERT_TRUE(longlong_result == ((longlong_max / 2) * 2));

    longlong_result = 2LL * longlong_amount;

    ASSERT_TRUE(longlong_result == ((longlong_max / 2) * 2));

    const ot::Amount uint_amount{uint_max / 2};

    const auto uint_result = 2ul * uint_amount;

    ASSERT_TRUE(uint_result == ((uint_max / 2) * 2));

    const ot::Amount ulong_amount{ulong_max / 2};

    const auto ulong_result = 2ul * ulong_amount;

    ASSERT_TRUE(ulong_result == ((ulong_max / 2) * 2));

    const ot::Amount ulonglong_amount{ulonglong_max / 2};

    auto ulonglong_result = ulonglong_amount * 2ull;

    ASSERT_TRUE(ulonglong_result == ((ulonglong_max / 2) * 2));

    ulonglong_result = 2ull * ulonglong_amount;

    ASSERT_TRUE(ulonglong_result == ((ulonglong_max / 2) * 2));

    ASSERT_TRUE(ot::Amount{2} * ulonglong_amount == ((ulonglong_max / 2) * 2));

    ulonglong_result = ulonglong_amount;
    ulonglong_result *= ot::Amount{2};

    ASSERT_TRUE(ulonglong_result == ((ulonglong_max / 2) * 2));

    try {
        const auto max = std::numeric_limits<ot::amount::Integer>::max();
        const auto amount = ot::factory::Amount(max.str());
        const auto overflow = amount * 2;
        EXPECT_TRUE(false);
    } catch (std::overflow_error&) {
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }

    try {
        const auto max = std::numeric_limits<ot::amount::Integer>::max();
        const auto amount = ot::factory::Amount(max.str());
        const auto overflow = 2 * amount;
        EXPECT_TRUE(false);
    } catch (std::overflow_error&) {
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }

    try {
        const auto max = std::numeric_limits<ot::amount::Integer>::max();
        const auto amount = ot::factory::Amount(max.str());
        const auto overflow = amount * 2L;
        EXPECT_TRUE(false);
    } catch (std::overflow_error&) {
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }

    try {
        const auto max = std::numeric_limits<ot::amount::Integer>::max();
        const auto amount = ot::factory::Amount(max.str());
        const auto overflow = amount * 2LL;
        EXPECT_TRUE(false);
    } catch (std::overflow_error&) {
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }

    try {
        const auto max = std::numeric_limits<ot::amount::Integer>::max();
        const auto amount = ot::factory::Amount(max.str());
        const auto overflow = 2LL * amount;
        EXPECT_TRUE(false);
    } catch (std::overflow_error&) {
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }

    try {
        const auto max = std::numeric_limits<ot::amount::Integer>::max();
        const auto amount = ot::factory::Amount(max.str());
        const auto overflow = amount * 2ull;
        EXPECT_TRUE(false);
    } catch (std::overflow_error&) {
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }

    try {
        const auto max = std::numeric_limits<ot::amount::Integer>::max();
        const auto amount = ot::factory::Amount(max.str());
        const auto overflow = 2ull * amount;
        EXPECT_TRUE(false);
    } catch (std::overflow_error&) {
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }

    try {
        const auto max = std::numeric_limits<ot::amount::Integer>::max();
        const auto amount = ot::factory::Amount(max.str());
        const auto overflow = amount * amount;
        EXPECT_TRUE(false);
    } catch (std::overflow_error&) {
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }

    try {
        const auto max = std::numeric_limits<ot::amount::Integer>::max();
        auto amount = ot::factory::Amount(max.str());
        amount *= ot::Amount{2};
        EXPECT_TRUE(false);
    } catch (std::overflow_error&) {
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }
}

TEST(Amount, divide)
{
    const ot::Amount int_amount{int_max - 1};

    const auto int_result = int_amount / 2;

    ASSERT_TRUE(int_result == int_max / 2);

    const ot::Amount longlong_amount{longlong_max - 1};

    const auto longlong_result = longlong_amount / 2LL;

    ASSERT_TRUE(longlong_result == longlong_max / 2);

    const ot::Amount ulonglong_amount{ulonglong_max - 1};

    const auto ulonglong_result = ulonglong_amount / 2ull;

    ASSERT_TRUE(ulonglong_result == ulonglong_max / 2);

    ASSERT_TRUE(ulonglong_amount / ot::Amount{2} == ulonglong_max / 2);

    try {
        const auto result = int_amount / 0;
        EXPECT_TRUE(false);
    } catch (std::exception&) {
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }

    try {
        const auto result = longlong_amount / 0LL;
        EXPECT_TRUE(false);
    } catch (std::exception&) {
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }

    try {
        const auto result = ulonglong_amount / 0ull;
        EXPECT_TRUE(false);
    } catch (std::exception&) {
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }

    try {
        const auto result = ulonglong_amount / ot::Amount{};
        EXPECT_TRUE(false);
    } catch (std::exception&) {
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }
}

TEST(Amount, modulo)
{
    const ot::Amount int_amount{3};

    const auto int_result = int_amount % 2;

    ASSERT_TRUE(int_result == 1);

    const ot::Amount longlong_amount{3LL};

    const auto longlong_result = longlong_amount % 2LL;

    ASSERT_TRUE(longlong_result == 1LL);

    const ot::Amount ulonglong_amount{3ull};

    const auto ulonglong_result = ulonglong_amount % 2ull;

    ASSERT_TRUE(ulonglong_result == 1ull);

    ASSERT_TRUE(ulonglong_amount % ot::Amount{2} == 1);
}
}  // namespace ottest
