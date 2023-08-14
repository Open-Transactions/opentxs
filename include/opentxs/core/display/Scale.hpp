// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <optional>
#include <span>
#include <string_view>

#include "opentxs/Export.hpp"  // IWYU pragma: keep
#include "opentxs/core/display/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace display
{
class ScalePrivate;
}  // namespace display

class Amount;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::display
{
class OPENTXS_EXPORT Scale
{
public:
    auto DefaultMinDecimals() const noexcept -> DecimalPlaces;
    auto DefaultMaxDecimals() const noexcept -> DecimalPlaces;
    auto Format(
        const Amount& amount,
        DecimalPlaces minDecimals = std::nullopt,
        DecimalPlaces maxDecimals = std::nullopt) const noexcept
        -> UnallocatedCString;
    auto Format(
        const Amount& amount,
        alloc::Strategy alloc,
        DecimalPlaces minDecimals = std::nullopt,
        DecimalPlaces maxDecimals = std::nullopt) const noexcept -> CString;
    auto Import(std::string_view formatted) const noexcept
        -> std::optional<Amount>;
    auto MaximumDecimals() const noexcept -> DecimalCount;
    auto Prefix() const noexcept -> std::string_view;
    auto Ratios() const noexcept -> std::span<const Ratio>;
    auto Suffix() const noexcept -> std::string_view;

    OPENTXS_NO_EXPORT Scale(ScalePrivate* imp) noexcept;
    Scale() noexcept;
    Scale(const Scale&) noexcept;
    Scale(Scale&&) noexcept;
    auto operator=(const Scale&) -> Scale& = delete;
    auto operator=(Scale&&) -> Scale& = delete;

    ~Scale();

private:
    ScalePrivate* imp_;

    auto swap(Scale& rhs) noexcept -> void;
};
}  // namespace opentxs::display
