// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <QChar>
#include <QString>
#include <atomic>
#include <iterator>
#include <locale>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <tuple>

#include "internal/interface/ui/AccountActivity.hpp"
#include "internal/util/Literals.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/core/display/Scale.hpp"
#include "opentxs/core/display/Types.hpp"
#include "opentxs/interface/qt/AmountValidator.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::ui
{
using namespace std::literals;

struct AmountValidator::Imp {
    using Parent = ui::AccountActivity;

    std::atomic<int> scale_;

    auto fixup(QString& input) const -> void
    {
        const auto index = [this]() -> display::SpecifiedScale {
            if (auto scale = scale_.load(); 0 > scale) {

                return std::nullopt;
            } else {

                return static_cast<display::ScaleIndex>(scale);
            }
        }();
        input = fix(input, index).c_str();
    }
    auto getMaxDecimals() const -> int { return max_; }
    auto getMinDecimals() const -> int { return min_; }
    auto getScale() const -> int { return scale_.load(); }
    auto revise(QString& input, int previous) const -> QString
    {
        try {

            return fix(input, display::to_scale(previous)).c_str();
        } catch (const std::exception& e) {
            LogTrace()()(e.what()).Flush();

            return {};
        }
    }
    auto setMaxDecimals(int max) -> bool
    {
        const auto old = max_.exchange(max);

        return old != max;
    }
    auto setMinDecimals(int min) -> bool
    {
        const auto old = min_.exchange(min);

        return old != min;
    }
    auto setScale(const int scale, int& old) -> bool
    {
        old = scale_.exchange(scale);
        const auto& definition = display::GetDefinition(unittype());
        const auto index = effective_scale(definition, scale);
        const auto scaledef = definition.Scale(index);
        const auto& minDecimals = scaledef.DefaultMinDecimals();

        if (minDecimals.has_value()) { setMinDecimals(minDecimals.value()); }

        const auto& maxDecimals = scaledef.DefaultMaxDecimals();

        if (maxDecimals.has_value()) { setMaxDecimals(maxDecimals.value()); }

        return old != scale;
    }

    auto validate(QString& input, int& pos) const -> State
    {
        try {
            const auto& definition = display::GetDefinition(unittype());
            const auto index = effective_scale(definition);
            const auto scale = definition.Scale(index);
            auto [whole, fractional, isNegative] = strip(input, pos);
            add_leading_zero(input, pos, whole);
            add_seperators(whole, fractional, isNegative, input, pos);
            add_prefix(scale, input, pos);
            add_suffix(scale, input, pos);
            const auto& maxDecimals{getMaxDecimals()};
            const auto& minDecimals{getMinDecimals()};

            if ((0 < maxDecimals && maxDecimals < fractional) ||
                (0 < minDecimals && minDecimals > fractional)) {

                return State::Invalid;
            }

            return State::Acceptable;
        } catch (const std::exception& e) {
            LogTrace()()(e.what()).Flush();

            return State::Invalid;
        }
    }
    Imp(Parent& parent) noexcept
        : scale_(-1)
        , parent_(parent)
        , unittype_(std::nullopt)
        , min_(-1)
        , max_(-1)
        , locale_()
    {
    }

    ~Imp() = default;

private:
    struct Locale : std::numpunct<char> {
    };

    const Parent& parent_;
    mutable std::optional<UnitType> unittype_;
    std::atomic_int min_;
    std::atomic_int max_;
    const Locale locale_;

    static auto effective_scale(
        const display::Definition& definition,
        int scale) noexcept -> display::ScaleIndex
    {
        if (0 > scale) {

            return definition.DefaultScale();
        } else {

            return static_cast<display::ScaleIndex>(scale);
        }
    }

    auto add_fractional(const int fractional, QString& input, int& pos)
        const noexcept -> void
    {
        const auto decimalSymbol = locale_.decimal_point();
        const auto& space = narrow_non_breaking_space_;
        auto distance{0};
        auto counter{-1};
        auto buf = input.toStdString();
        auto foundDecimal{false};

        for (auto i = buf.begin(); i != buf.end(); ++i, ++distance) {
            if (foundDecimal) {
                ++counter;

                if ((0 < counter) && (0 == (counter % 3))) {
                    if (distance < pos) { ++pos; }

                    i = std::next(
                        buf.insert(i, space.begin(), space.end()),
                        space.size());
                    ++distance;
                }
            } else if (decimalSymbol == *i) {
                foundDecimal = true;
            }
        }

        input = QString::fromStdString(buf);
    }
    auto add_leading_zero(QString& input, int& pos, int& whole) const noexcept
        -> void
    {
        const auto decimalSymbol = locale_.decimal_point();

        if (0 == input.size()) { return; }

        if (decimalSymbol == input.at(0)) {
            assert_true(0 == whole);

            static constexpr auto zero = '0';
            input.prepend(zero);
            pos += 1;
            whole += 1;
        }
    }
    auto add_prefix(const display::Scale& scale, QString& input, int& pos)
        const noexcept -> void
    {
        const auto value = scale.Prefix();

        if (false == valid(value)) { return; }

        const auto prefix = QString::fromStdString(UnallocatedCString{value});
        input.prepend(prefix);
        pos += prefix.size();
    }
    auto add_seperators(
        const int whole,
        const int fractional,
        const bool isNegative,
        QString& input,
        int& pos) const noexcept -> void
    {
        if (whole > 3) { add_thousands(whole, isNegative, input, pos); }

        if (fractional > 3) { add_fractional(fractional, input, pos); }
    }
    auto add_suffix(const display::Scale& scale, QString& input, int& pos)
        const noexcept -> void
    {
        const auto value = scale.Suffix();

        if (false == valid(value)) { return; }

        static constexpr auto space = ' ';
        const auto suffix = QString::fromStdString(UnallocatedCString{value});
        input.append(space);
        input.append(suffix);
    }
    auto add_thousands(
        const int whole,
        const bool isNegative,
        QString& input,
        int& pos) const noexcept -> void
    {
        assert_true(3 < input.size());

        const auto target = ((whole + (isNegative ? 1 : 0)) % 3);
        const auto seperator = locale_.thousands_sep();
        const auto stop = whole - (isNegative ? 1 : 2);
        auto counter{-1};
        auto buf = input.toStdString();

        for (auto i = buf.begin(); i != buf.end(); ++i) {
            ++counter;
            const auto index = (counter % 3);

            if (counter == stop) { break; }

            if ((0 < counter) && (index == target)) {
                const auto distance = std::distance(buf.begin(), i);

                if (distance <= pos) { ++pos; }

                i = std::next(buf.insert(i, seperator));
            }
        }

        input = QString::fromStdString(buf);
    }
    auto effective_scale(const display::Definition& definition) const noexcept
        -> display::ScaleIndex
    {
        return effective_scale(definition, scale_.load());
    }
    auto fix(QString& input, display::SpecifiedScale oldScale) const noexcept
        -> UnallocatedCString
    {
        static const auto get = [](const auto& val) -> display::DecimalPlaces {
            auto out = val.load();

            if (0 > out) {

                return std::nullopt;
            } else {

                return out;
            }
        };

        const auto& definition = display::GetDefinition(unittype());
        const auto newScale = effective_scale(definition);
        const auto min = get(min_);
        const auto max = get(max_);
        const auto data = input.toStdString();

        if (const auto amount = definition.Import(data, oldScale); amount) {

            return definition.Format(*amount, newScale, min, max);
        } else {

            return {};
        }
    }
    auto strip(QString& input, int& pos) const noexcept
        -> std::tuple<int, int, bool>
    {
        const auto decimalSymbol = locale_.decimal_point();
        const auto negativeSign = '-';
        auto whole{0};
        auto fractional{0};
        auto isNegative{false};
        auto decimalCount{0};
        auto current{-1};
        auto revised = QString{};
        auto haveLeadingDigit{false};
        const auto& maxDecimals{getMaxDecimals()};

        for (const auto& c : input) {
            ++current;
            auto keep{true};

            if (c.isDigit()) {
                static const auto zero = QChar{'0'};
                const auto leadingZero = (0 < revised.size()) && (zero == c) &&
                                         (0 == decimalCount) &&
                                         (false == haveLeadingDigit);

                if (leadingZero) {
                    keep = false;
                } else {
                    if (fractional < maxDecimals || 0 == maxDecimals) {
                        keep = true;
                    } else {
                        keep = false;
                    }

                    if (0 < decimalCount) {
                        ++fractional;
                    } else {
                        if (zero != c) { haveLeadingDigit = true; }

                        ++whole;
                    }
                }
            } else if (decimalSymbol == c) {
                ++decimalCount;
                keep = (1 == decimalCount);
            } else if (negativeSign == c) {
                if (isNegative) {
                    keep = false;
                } else {
                    isNegative = true;
                    keep = true;
                }
            } else {
                keep = false;
            }

            if (keep) {
                revised.append(c);
            } else if (current < pos) {
                pos -= 1;
            }
        }
        input = revised;

        return std::make_tuple(whole, fractional, isNegative);
    }

    auto unittype() const noexcept -> UnitType;
};
}  // namespace opentxs::ui
