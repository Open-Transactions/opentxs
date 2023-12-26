// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/display/Scale.hpp"  // IWYU pragma: associated

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds="  // NOLINT
#include <boost/multiprecision/cpp_int.hpp>

#pragma GCC diagnostic pop
#include <cs_plain_guarded.h>
#include <algorithm>
#include <atomic>
#include <cctype>
#include <cstdint>
#include <exception>
#include <limits>
#include <locale>
#include <span>
#include <sstream>
#include <utility>

#include "internal/core/Amount.hpp"
#include "internal/core/display/Types.hpp"
#include "internal/util/Literals.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Literals.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::display
{
class ScalePrivate
{
public:
    struct Runtime {
        const CString prefix_;
        const CString suffix_;
        const DecimalPlaces default_min_;
        const DecimalPlaces default_max_;
        const Vector<Ratio> ratios_;

        Runtime(
            std::string_view prefix = {},
            std::string_view suffix = {},
            Vector<Ratio>&& ratios = {},
            const DecimalPlaces defaultMinDecimals = {},
            const DecimalPlaces defaultMaxDecimals = {}) noexcept;
        Runtime(const Runtime& rhs) noexcept;
    };

    auto DefaultMinDecimals() const noexcept -> DecimalPlaces;
    auto DefaultMaxDecimals() const noexcept -> DecimalPlaces;
    auto Format(
        const Amount& amount,
        const DecimalPlaces minDecimals,
        const DecimalPlaces maxDecimals,
        alloc::Strategy alloc) const noexcept -> std::stringstream;
    auto Import(const std::string_view formatted) const noexcept
        -> std::optional<Amount>;
    auto MaximumDecimals() const noexcept -> DecimalCount;
    auto Prefix() const noexcept -> std::string_view;
    auto Ratios() const noexcept -> std::span<const Ratio>;
    // TODO constexpr
    auto RuntimeAllocated() const noexcept -> bool
    {
        return true;  // TODO return std::holds_alternative<Runtime>(data_);
    }
    auto Suffix() const noexcept -> std::string_view;

    ScalePrivate() noexcept;
    ScalePrivate(
        std::string_view prefix,
        std::string_view suffix,
        Vector<Ratio>&& ratios,
        const DecimalPlaces defaultMinDecimals,
        const DecimalPlaces defaultMaxDecimals) noexcept;
    // TODO constexpr
    ScalePrivate(ScaleRef data) noexcept;
    // TODO constexpr
    ScalePrivate(const ScalePrivate& rhs) noexcept;

    // TODO constexpr
    ~ScalePrivate();

private:
    struct Locale : std::numpunct<char> {
    };

    struct Calculated {
        amount::Float incoming_;
        amount::Float outgoing_;
        Locale locale_;
        int absolute_max_;

        Calculated(std::span<const Ratio> ratios) noexcept;

    private:
        // ratio for converting display string to Amount
        static auto calculate_incoming_ratio(
            std::span<const Ratio> ratios) noexcept -> amount::Float;
        // ratio for converting Amount to display string
        static auto calculate_outgoing_ratio(
            std::span<const Ratio> ratios) noexcept -> amount::Float;
    };

    using CalculatedMap = Map<std::uintptr_t, Calculated>;
    using Guarded = libguarded::plain_guarded<CalculatedMap>;

    const std::variant<Runtime, ScaleRef> data_;
    mutable std::atomic<const Calculated*> calculated_;

    static auto map() noexcept -> Guarded&;

    auto calculated() const noexcept -> const Calculated&;
    auto effective_limits(const DecimalPlaces min, const DecimalPlaces max)
        const noexcept -> Ratio;

    auto strip(const std::string_view in) const noexcept -> UnallocatedCString;
};
}  // namespace opentxs::display
