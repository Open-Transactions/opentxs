// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "opentxs/Types.hpp"

namespace opentxs
{
class Amount;
}  // namespace opentxs

namespace opentxs::display
{
class Scale
{
public:
    struct Imp;

    /// A ratio should express the quantity of smallest values (from Amount)
    /// which represent a scale unit.
    using Ratio = std::pair<std::uint8_t, std::int8_t>;
    using OptionalInt = std::optional<std::uint8_t>;

    auto DefaultMinDecimals() const noexcept -> OptionalInt;
    auto DefaultMaxDecimals() const noexcept -> OptionalInt;
    auto Format(
        const Amount& amount,
        const OptionalInt minDecimals = std::nullopt,
        const OptionalInt maxDecimals = std::nullopt) const noexcept(false)
        -> std::string;
    auto Import(const std::string& formatted) const noexcept(false) -> Amount;
    auto Prefix() const noexcept -> std::string;
    auto Ratios() const noexcept -> const std::pmr::vector<Ratio>&;
    auto Suffix() const noexcept -> std::string;

    virtual auto swap(Scale& rhs) noexcept -> void;

    OPENTXS_NO_EXPORT Scale(Imp* imp) noexcept;
    Scale() noexcept;
    Scale(
        const std::string& prefix,
        const std::string& suffix,
        const std::pmr::vector<Ratio>& ratios,
        const OptionalInt defaultMinDecimals = std::nullopt,
        const OptionalInt defaultMaxDecimals = std::nullopt) noexcept;
    Scale(const Scale&) noexcept;
    Scale(Scale&&) noexcept;

    virtual ~Scale();

private:
    Imp* imp_;

    auto operator=(const Scale&) -> Scale& = delete;
    auto operator=(Scale&&) -> Scale& = delete;
};
}  // namespace opentxs::display
