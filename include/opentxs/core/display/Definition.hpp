// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <optional>
#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/display/Scale.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Amount;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::display
{
class OPENTXS_EXPORT Definition
{
public:
    using Index = unsigned int;
    using Name = CString;
    using NamedScale = std::pair<Name, Scale>;
    using Map = opentxs::Map<Index, Name>;
    using Scales = Vector<NamedScale>;
    using OptionalInt = Scale::OptionalInt;

    auto DisplayScales() const noexcept -> const Scales&;
    auto Format(
        const Amount amount,
        const Index scale = 0,
        const OptionalInt minDecimals = std::nullopt,
        const OptionalInt maxDecimals = std::nullopt) const noexcept(false)
        -> UnallocatedCString;
    auto GetScales() const noexcept -> const Map&;
    auto Import(const std::string_view formatted, const Index scale = 0) const
        noexcept(false) -> Amount;
    auto ShortName() const noexcept -> std::string_view;

    virtual auto swap(Definition& rhs) noexcept -> void;

    Definition(std::string_view shortname, Scales&& scales) noexcept;
    Definition() noexcept;
    Definition(const Definition&) noexcept;
    Definition(Definition&&) noexcept;

    auto operator=(const Definition&) noexcept -> Definition&;
    auto operator=(Definition&&) noexcept -> Definition&;

    virtual ~Definition();

private:
    struct Imp;

    Imp* imp_;
};

OPENTXS_EXPORT auto GetDefinition(UnitType) noexcept -> const Definition&;
}  // namespace opentxs::display
