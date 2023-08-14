// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <optional>
#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/display/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace display
{
class DefinitionPrivate;
class Scale;
}  // namespace display

class Amount;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::display
{
class OPENTXS_EXPORT Definition
{
public:
    auto AtomicScale() const noexcept -> ScaleIndex;
    auto DefaultScale() const noexcept -> ScaleIndex;
    auto Format(
        const Amount& amount,
        SpecifiedScale scale = std::nullopt,
        DecimalPlaces minDecimals = std::nullopt,
        DecimalPlaces maxDecimals = std::nullopt) const noexcept
        -> UnallocatedCString;
    auto Format(
        const Amount& amount,
        alloc::Strategy alloc,
        SpecifiedScale scale = std::nullopt,
        DecimalPlaces minDecimals = std::nullopt,
        DecimalPlaces maxDecimals = std::nullopt) const noexcept -> CString;
    auto Import(std::string_view formatted, SpecifiedScale scale = std::nullopt)
        const noexcept -> std::optional<Amount>;
    auto Scale(ScaleIndex scale) const noexcept -> const display::Scale&;
    auto ScaleCount() const noexcept -> ScaleIndex;
    auto ScaleName(ScaleIndex scale) const noexcept -> std::string_view;
    auto ShortName() const noexcept -> std::string_view;

    OPENTXS_NO_EXPORT Definition(DefinitionPrivate* imp) noexcept;
    Definition() noexcept;
    Definition(const Definition&) noexcept;
    Definition(Definition&&) noexcept;

    auto operator=(const Definition&) noexcept -> Definition&;
    auto operator=(Definition&&) noexcept -> Definition&;

    ~Definition();

private:
    DefinitionPrivate* imp_;

    auto swap(Definition& rhs) noexcept -> void;
};

OPENTXS_EXPORT auto GetDefinition(UnitType) noexcept -> const Definition&;
}  // namespace opentxs::display
