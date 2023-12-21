// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/unordered/detail/foa.hpp>
// IWYU pragma: no_include <boost/unordered/detail/foa/flat_map_types.hpp>
// IWYU pragma: no_include <boost/unordered/detail/foa/table.hpp>

#include "opentxs/core/display/Definition.hpp"  // IWYU pragma: associated

#include <boost/unordered/unordered_flat_map.hpp>
#include <array>  // IWYU pragma: keep
#include <memory>
#include <optional>
#include <string_view>
#include <utility>

#include "core/display/DefinitionPrivate.hpp"
#include "internal/core/display/Types.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/UnitType.hpp"       // IWYU pragma: keep
#include "opentxs/core/display/Scale.hpp"  // IWYU pragma: keep

namespace opentxs::display
{
using namespace std::literals;

auto to_scale(int value) noexcept -> SpecifiedScale
{
    if (0 > value) {

        return std::nullopt;
    } else {

        return static_cast<ScaleIndex>(value);
    }
}

auto GetDefinition(UnitType in) noexcept -> const Definition&
{
#include "core/display/data/ratios"        // IWYU pragma: keep
#include "core/display/data/scale_params"  // IWYU pragma: keep
#include "core/display/data/scales"        // IWYU pragma: keep
#include "core/display/data/z_def_params"  // IWYU pragma: keep

#define MAKE_SCALE(K, V) {K, std::make_unique<DefinitionPrivate>(V).release()},

    using enum UnitType;
    // TODO it should be possible to use a constexpr map here however the
    // compiler support just isn't there yet. GCC-13 can handle constructing
    // constexpr ScalePrivate and DefinitionPrivate objects, but Clang and even
    // MSVC have trouble with that, let alone XCode 14 and NDK-25.
    //
    // Revisit this after XCode 15 and NDK-26 are officially released.
    // const auto map = frozen::make_unordered_map<UnitType, Definition>({
    static const auto map = boost::unordered_flat_map<UnitType, Definition>{
#include "core/display/data/defs"  // IWYU pragma: keep
    };

    if (const auto i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return UnityDefinition();
    }
#undef MAKE_SCALE
}

auto UnityDefinition() noexcept -> const Definition&
{
    static constexpr auto ratio = std::array<Ratio, 1>{Ratio{10, 0}};
    static constexpr auto params = std::array<ScaleDef, 1_uz>{
        ScaleDef{""sv, ScaleInit{""sv, ""sv, ratio, 0, 0}}};
    static constexpr auto scale = std::array<ScaleRef, 1_uz>{ScaleRef{params}};
    static constexpr auto def =
        std::array<DefInit, 1_uz>{DefInit{""sv, 0, 0, scale}};
    static const auto definition =
        Definition{std::make_unique<DefinitionPrivate>(def).release()};

    return definition;
}
}  // namespace opentxs::display
