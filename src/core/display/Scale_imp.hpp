// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "1_Internal.hpp"                  // IWYU pragma: associated
#include "opentxs/core/display/Scale.hpp"  // IWYU pragma: associated

#include <boost/multiprecision/cpp_dec_float.hpp>
#include <cctype>
#include <iomanip>
#include <locale>
#include <sstream>
#include <utility>

#include "core/Amount.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/util/Log.hpp"

namespace bmp = boost::multiprecision;

namespace opentxs::display
{
struct Scale::Imp {
    const std::string prefix_;
    const std::string suffix_;
    const OptionalInt default_min_;
    const OptionalInt default_max_;
    const std::pmr::vector<Ratio> ratios_;

    auto format(
        const Amount& amount,
        const OptionalInt minDecimals,
        const OptionalInt maxDecimals) const noexcept(false) -> std::string
    {
        auto output = std::stringstream{};

        if (0 < prefix_.size()) { output << prefix_; }

        const auto decimalSymbol = locale_.decimal_point();
        const auto seperator = locale_.thousands_sep();
        auto raw_amount =
            amount.Internal().extract_float<Scale::Imp::Backend>();
        const auto scaled = outgoing_ * Scale::Imp::Backend{raw_amount};
        const auto [min, max] = effective_limits(minDecimals, maxDecimals);
        auto fractionalDigits = std::max<unsigned>(max, 1u);
        auto string = scaled.str(fractionalDigits, std::ios_base::fixed);
        auto haveDecimal{true};

        if (0 == max) {
            string.pop_back();
            string.pop_back();
            --fractionalDigits;
            haveDecimal = false;
        }

        static constexpr auto zero = '0';

        while ((fractionalDigits > min) && (string.back() == zero)) {
            string.pop_back();
            --fractionalDigits;
        }

        if (string.back() == decimalSymbol) {
            string.pop_back();
            haveDecimal = false;
        }

        const auto wholeDigits = std::size_t{
            string.size() - fractionalDigits - (haveDecimal ? 1u : 0u)};
        auto counter = std::size_t{
            (4u > wholeDigits) ? 1u : 4u - ((wholeDigits - 1u) % 3u)};
        auto pushed = std::size_t{0};
        auto formatDecimals{false};

        for (const auto c : string) {
            output << c;
            ++pushed;

            if (pushed < wholeDigits) {
                if (0u == (counter % 4u)) {
                    output << seperator;
                    ++counter;
                }

                ++counter;
            } else if (c == decimalSymbol) {
                counter = 1u;
                formatDecimals = true;
            }

            if (formatDecimals) {
                if (0u == (counter % 4u) && pushed < string.size()) {
                    static constexpr auto narrowNonBreakingSpace = u8"\u202F";
                    output << narrowNonBreakingSpace;
                    ++counter;
                }

                ++counter;
            }
        }

        if (0 < suffix_.size()) { output << ' ' << suffix_; }

        return output.str();
    }
    auto Import(const std::string& formatted) const noexcept(false) -> Amount
    {
        try {
            const auto output = incoming_ * Imp::Backend{Imp::strip(formatted)};

            auto amount = Amount{};
            amount.Internal().amount_ = Amount::Imp::shift_left(
                output.convert_to<Amount::Imp::Backend>());

            return amount;
        } catch (const std::exception& e) {
            LogTrace()(OT_PRETTY_CLASS())(e.what()).Flush();

            throw std::current_exception();
        }
    }

    Imp() noexcept
        : prefix_{}
        , suffix_{}
        , default_min_{}
        , default_max_{}
        , ratios_{}
        , incoming_{}
        , outgoing_{}
        , locale_{}
    {
    }
    Imp(const std::string& prefix,
        const std::string& suffix,
        const std::pmr::vector<Ratio>& ratios,
        const OptionalInt defaultMinDecimals,
        const OptionalInt defaultMaxDecimals) noexcept
        : prefix_(prefix)
        , suffix_(suffix)
        , default_min_(defaultMinDecimals)
        , default_max_(defaultMaxDecimals)
        , ratios_(ratios)
        , incoming_(calculate_incoming_ratio(ratios))
        , outgoing_(calculate_outgoing_ratio(ratios))
        , locale_()
    {
        OT_ASSERT(default_max_.value_or(0) >= default_min_.value_or(0));
    }
    Imp(const Imp& rhs) noexcept
        : prefix_(rhs.prefix_)
        , suffix_(rhs.suffix_)
        , default_min_(rhs.default_min_)
        , default_max_(rhs.default_max_)
        , ratios_(rhs.ratios_)
        , incoming_(rhs.incoming_)
        , outgoing_(rhs.outgoing_)
        , locale_()
    {
    }

private:
    using Backend = bmp::cpp_dec_float_100;

    struct Locale : std::numpunct<char> {
    };

    const Backend incoming_;
    const Backend outgoing_;
    const Locale locale_;

    // ratio for converting display string to Amount
    static auto calculate_incoming_ratio(
        const std::pmr::vector<Ratio>& ratios) noexcept -> Backend
    {
        auto output = Backend{1};

        for (const auto& [base, exponent] : ratios) {
            output *= bmp::pow(Backend{base}, exponent);
        }

        return output;
    }
    // ratio for converting Amount to display string
    static auto calculate_outgoing_ratio(
        const std::pmr::vector<Ratio>& ratios) noexcept -> Backend
    {
        auto output = Backend{1};

        for (const auto& [base, exponent] : ratios) {
            output *= bmp::pow(Backend{base}, -1 * exponent);
        }

        return output;
    }

    auto effective_limits(const OptionalInt min, const OptionalInt max)
        const noexcept -> std::pair<std::uint8_t, std::uint8_t>
    {
        auto output = std::pair<std::uint8_t, std::uint8_t>{};
        auto& [effMin, effMax] = output;

        if (min.has_value()) {
            effMin = min.value();
        } else {
            effMin = default_min_.value_or(0u);
        }

        if (max.has_value()) {
            effMax = max.value();
        } else {
            effMax = default_max_.value_or(0u);
        }

        return output;
    }

    auto strip(const std::string& in) const noexcept -> std::string
    {
        const auto decimal = locale_.decimal_point();
        auto output = std::string{};

        for (const auto& c : in) {
            if (0 != std::isdigit(c)) {
                output += c;
            } else if (c == decimal) {
                output += c;
            }
        }

        return output;
    }
};
}  // namespace opentxs::display
