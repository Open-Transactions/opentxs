// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/display/ScalePrivate.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <exception>
#include <limits>
#include <locale>
#include <span>
#include <sstream>
#include <utility>

#include "internal/core/Amount.hpp"
#include "internal/util/Literals.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Literals.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::display
{
ScalePrivate::Calculated::Calculated(std::span<const Ratio> ratios) noexcept
    : incoming_(calculate_incoming_ratio(ratios))
    , outgoing_(calculate_outgoing_ratio(ratios))
    , locale_()
    // NOLINTNEXTLINE(clang-analyzer-core.StackAddressEscape)
    , absolute_max_(20 + bmp::log10(incoming_))
{
    assert_true(0 <= absolute_max_);
    assert_true(absolute_max_ <= std::numeric_limits<std::uint8_t>::max());
}

auto ScalePrivate::Calculated::calculate_incoming_ratio(
    std::span<const Ratio> ratios) noexcept -> amount::Float
{
    auto output = amount::Float{1};

    for (const auto& [base, exponent] : ratios) {
        output *= bmp::pow(amount::Float{base}, exponent);
    }

    return output;
}

auto ScalePrivate::Calculated::calculate_outgoing_ratio(
    std::span<const Ratio> ratios) noexcept -> amount::Float
{
    auto output = amount::Float{1};

    for (const auto& [base, exponent] : ratios) {
        output *= bmp::pow(amount::Float{base}, -1 * exponent);
    }

    return output;
}
}  // namespace opentxs::display

namespace opentxs::display
{
ScalePrivate::Runtime::Runtime(
    std::string_view prefix,
    std::string_view suffix,
    Vector<Ratio>&& ratios,
    const DecimalPlaces defaultMinDecimals,
    const DecimalPlaces defaultMaxDecimals) noexcept
    : prefix_(prefix)
    , suffix_(suffix)
    , default_min_(defaultMinDecimals)
    , default_max_(defaultMaxDecimals)
    , ratios_(std::move(ratios))
{
    assert_true(default_max_.value_or(0) >= default_min_.value_or(0));
}

ScalePrivate::Runtime::Runtime(const Runtime&) noexcept = default;
}  // namespace opentxs::display

namespace opentxs::display
{
ScalePrivate::ScalePrivate() noexcept
    : data_(Runtime{})
    , calculated_(nullptr)
{
}

ScalePrivate::ScalePrivate(
    std::string_view prefix,
    std::string_view suffix,
    Vector<Ratio>&& ratios,
    const DecimalPlaces defaultMinDecimals,
    const DecimalPlaces defaultMaxDecimals) noexcept
    : data_(
          std::in_place_type_t<Runtime>{},
          prefix,
          suffix,
          std::move(ratios),
          defaultMinDecimals,
          defaultMaxDecimals)
    , calculated_(nullptr)
{
}

ScalePrivate::ScalePrivate(ScaleRef data) noexcept
    : data_(data)
    , calculated_(nullptr)
{
}

ScalePrivate::ScalePrivate(const ScalePrivate& rhs) noexcept
    : data_(rhs.data_)
    , calculated_(nullptr)
{
}

auto ScalePrivate::calculated() const noexcept -> const Calculated&
{
    const auto* p = calculated_.load();

    if (nullptr != p) { return *p; }

    const auto key = reinterpret_cast<std::uintptr_t>(this);
    const auto i = map().lock()->try_emplace(key, Ratios()).first;
    calculated_.store(std::addressof(i->second));

    return i->second;
}

auto ScalePrivate::DefaultMinDecimals() const noexcept -> DecimalPlaces
{
    struct Visitor {
        auto operator()(const Runtime& in) noexcept -> DecimalPlaces
        {
            return in.default_min_;
        }
        auto operator()(const ScaleRef& in) noexcept -> DecimalPlaces
        {
            return std::get<3>(in.front().second);
        }
    };

    return std::visit(Visitor{}, data_);
}

auto ScalePrivate::DefaultMaxDecimals() const noexcept -> DecimalPlaces
{
    struct Visitor {
        auto operator()(const Runtime& in) noexcept -> DecimalPlaces
        {
            return in.default_max_;
        }
        auto operator()(const ScaleRef& in) noexcept -> DecimalPlaces
        {
            return std::get<4>(in.front().second);
        }
    };

    return std::visit(Visitor{}, data_);
}

auto ScalePrivate::effective_limits(
    const DecimalPlaces min,
    const DecimalPlaces max) const noexcept -> Ratio
{
    auto output = Ratio{};
    auto& [effMin, effMax] = output;

    if (min.has_value()) {
        effMin = min.value();
    } else {
        effMin = DefaultMinDecimals().value_or(0_uz);
    }

    if (max.has_value()) {
        effMax = max.value();
    } else {
        effMax = DefaultMaxDecimals().value_or(0_uz);
    }

    effMax = std::min(
        effMax, static_cast<std::int_fast8_t>(calculated().absolute_max_));

    return output;
}

auto ScalePrivate::Format(
    const Amount& amount,
    const DecimalPlaces minDecimals,
    const DecimalPlaces maxDecimals,
    alloc::Strategy alloc) const noexcept -> std::stringstream
{
    auto output = std::stringstream{};  // TODO alloc

    if (const auto pre = Prefix(); false == pre.empty()) { output << pre; }

    const auto& calc = calculated();
    const auto decimalSymbol = calc.locale_.decimal_point();
    const auto seperator = calc.locale_.thousands_sep();
    const auto scaled = amount.Internal().ToFloat() * calc.outgoing_;
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
                output << narrow_non_breaking_space_;
                ++counter;
            }

            ++counter;
        }
    }

    if (const auto post = Suffix(); false == post.empty()) {
        output << ' ' << post;
    }

    return output;
}

auto ScalePrivate::Import(const std::string_view formatted) const noexcept
    -> std::optional<Amount>
{
    try {
        const auto scaled = calculated().incoming_ *
                            amount::Float{ScalePrivate::strip(formatted)};

        return opentxs::internal::FloatToAmount(scaled);
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return std::nullopt;
    }
}

auto ScalePrivate::map() noexcept -> Guarded&
{
    static auto map = Guarded{};

    return map;
}

auto ScalePrivate::MaximumDecimals() const noexcept -> DecimalCount
{
    return static_cast<DecimalCount>(calculated().absolute_max_);
}

auto ScalePrivate::Prefix() const noexcept -> std::string_view
{
    struct Visitor {
        auto operator()(const Runtime& in) noexcept -> std::string_view
        {
            return in.prefix_;
        }
        auto operator()(const ScaleRef& in) noexcept -> std::string_view
        {
            return std::get<0>(in.front().second);
        }
    };

    return std::visit(Visitor{}, data_);
}

auto ScalePrivate::Ratios() const noexcept -> std::span<const Ratio>
{
    struct Visitor {
        auto operator()(const Runtime& in) noexcept -> std::span<const Ratio>
        {
            return in.ratios_;
        }
        auto operator()(const ScaleRef& in) noexcept -> std::span<const Ratio>
        {
            return std::get<2>(in.front().second);
        }
    };

    return std::visit(Visitor{}, data_);
}

auto ScalePrivate::Suffix() const noexcept -> std::string_view
{
    struct Visitor {
        auto operator()(const Runtime& in) noexcept -> std::string_view
        {
            return in.suffix_;
        }
        auto operator()(const ScaleRef& in) noexcept -> std::string_view
        {
            return std::get<1>(in.front().second);
        }
    };

    return std::visit(Visitor{}, data_);
}

auto ScalePrivate::strip(const std::string_view in) const noexcept
    -> UnallocatedCString
{
    const auto decimal = calculated().locale_.decimal_point();
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

ScalePrivate::~ScalePrivate()
{
    if (nullptr != calculated_.load()) {
        map().lock()->erase(reinterpret_cast<std::uintptr_t>(this));
    }
}
}  // namespace opentxs::display
