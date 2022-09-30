// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "opentxs/core/display/Scale.hpp"  // IWYU pragma: associated

#include <boost/lexical_cast/bad_lexical_cast.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <exception>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <type_traits>
#include <utility>

#include "internal/core/Amount.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/util/Literals.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::display
{
class Scale::Imp
{
public:
    const CString prefix_;
    const CString suffix_;
    const OptionalInt default_min_;
    const OptionalInt default_max_;
    const Vector<Ratio> ratios_;

    auto format(
        const Amount& amount,
        const OptionalInt minDecimals,
        const OptionalInt maxDecimals) const noexcept(false)
        -> UnallocatedCString
    {
        auto output = std::stringstream{};

        if (0 < prefix_.size()) { output << prefix_; }

        const auto decimalSymbol = locale_.decimal_point();
        const auto seperator = locale_.thousands_sep();
        const auto scaled = amount.Internal().ToFloat() * outgoing_;
        const auto [min, max] = effective_limits(minDecimals, maxDecimals);
        auto fractionalDigits = std::max<unsigned>(max, 1_uz);
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

        const auto wholeDigits = string.size() - fractionalDigits -
                                 (haveDecimal ? 1_uz : 0_uz) -
                                 (string.front() == '-' ? 1_uz : 0_uz);
        auto counter =
            (4_uz > wholeDigits) ? 1_uz : 4_uz - ((wholeDigits - 1_uz) % 3_uz);
        auto pushed = 0_uz;
        auto formatDecimals{false};

        for (const auto c : string) {
            output << c;
            ++pushed;

            if (pushed < wholeDigits && (c != '-')) {
                if (0_uz == (counter % 4_uz)) {
                    output << seperator;
                    ++counter;
                }

                ++counter;
            } else if (c == decimalSymbol) {
                counter = 1_uz;
                formatDecimals = true;
            }

            if (formatDecimals) {
                if (0_uz == (counter % 4_uz) && pushed < string.size()) {
                    static const auto narrowNonBreakingSpace = u8"\u202F"_sv;
                    output << narrowNonBreakingSpace;
                    ++counter;
                }

                ++counter;
            }
        }

        if (0 < suffix_.size()) { output << ' ' << suffix_; }

        return output.str();
    }
    auto Import(const std::string_view formatted) const noexcept(false)
        -> Amount
    {
        try {
            const auto scaled =
                incoming_ * amount::Float{Imp::strip(formatted)};

            return internal::FloatToAmount(scaled);
        } catch (const std::exception& e) {
            LogTrace()(OT_PRETTY_CLASS())(e.what()).Flush();

            throw std::current_exception();
        }
    }
    auto MaximumDecimals() const noexcept -> std::uint8_t
    {
        return static_cast<std::uint8_t>(absolute_max_);
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
        , absolute_max_{}
    {
    }
    Imp(std::string_view prefix,
        std::string_view suffix,
        Vector<Ratio>&& ratios,
        const OptionalInt defaultMinDecimals,
        const OptionalInt defaultMaxDecimals) noexcept
        : prefix_(prefix)
        , suffix_(suffix)
        , default_min_(defaultMinDecimals)
        , default_max_(defaultMaxDecimals)
        , ratios_(std::move(ratios))
        , incoming_(calculate_incoming_ratio(ratios_))
        , outgoing_(calculate_outgoing_ratio(ratios_))
        , locale_()
        // NOLINTNEXTLINE(clang-analyzer-core.StackAddressEscape)
        , absolute_max_(20 + bmp::log10(incoming_))
    {
        OT_ASSERT(default_max_.value_or(0) >= default_min_.value_or(0));
        OT_ASSERT(0 <= absolute_max_);
        OT_ASSERT(absolute_max_ <= std::numeric_limits<std::uint8_t>::max());
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
        , absolute_max_(rhs.absolute_max_)
    {
    }

private:
    struct Locale : std::numpunct<char> {
    };

    const amount::Float incoming_;
    const amount::Float outgoing_;
    const Locale locale_;
    const int absolute_max_;

    // ratio for converting display string to Amount
    static auto calculate_incoming_ratio(const Vector<Ratio>& ratios) noexcept
        -> amount::Float
    {
        auto output = amount::Float{1};

        for (const auto& [base, exponent] : ratios) {
            output *= bmp::pow(amount::Float{base}, exponent);
        }

        return output;
    }
    // ratio for converting Amount to display string
    static auto calculate_outgoing_ratio(const Vector<Ratio>& ratios) noexcept
        -> amount::Float
    {
        auto output = amount::Float{1};

        for (const auto& [base, exponent] : ratios) {
            output *= bmp::pow(amount::Float{base}, -1 * exponent);
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
            effMin = default_min_.value_or(0_uz);
        }

        if (max.has_value()) {
            effMax = max.value();
        } else {
            effMax = default_max_.value_or(0_uz);
        }

        effMax = std::min(effMax, static_cast<uint8_t>(absolute_max_));

        return output;
    }

    auto strip(const std::string_view in) const noexcept -> UnallocatedCString
    {
        const auto decimal = locale_.decimal_point();
        auto output = UnallocatedCString{};

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
