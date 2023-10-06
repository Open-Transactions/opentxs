// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/display/DefinitionPrivate.hpp"  // IWYU pragma: associated

#include <memory>
#include <optional>
#include <span>
#include <string_view>
#include <tuple>
#include <utility>

#include "core/display/ScalePrivate.hpp"
#include "internal/core/display/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/display/Scale.hpp"
#include "opentxs/core/display/Types.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::display
{
DefinitionPrivate::Runtime::Runtime(
    std::string_view shortname,
    ScaleIndex defaultScale,
    ScaleIndex atomicScale,
    Scales&& scales) noexcept
    : short_name_(shortname)
    , scales_(std::move(scales))
    , atomic_(atomicScale)
    , default_(defaultScale)
{
}

DefinitionPrivate::Runtime::Runtime(const Runtime&) noexcept = default;
}  // namespace opentxs::display

namespace opentxs::display
{
DefinitionPrivate::DefinitionPrivate() noexcept
    : data_(Runtime{})
{
}

DefinitionPrivate::DefinitionPrivate(
    std::string_view shortname,
    ScaleIndex defaultScale,
    ScaleIndex atomicScale,
    Scales&& scales) noexcept
    : data_(
          std::in_place_type_t<Runtime>{},
          shortname,
          defaultScale,
          atomicScale,
          std::move(scales))
{
}

DefinitionPrivate::DefinitionPrivate(DefRef data) noexcept
    : data_(data)
{
}

auto DefinitionPrivate::AtomicScale() const noexcept -> ScaleIndex
{
    struct Visitor {
        auto operator()(const Runtime& in) noexcept -> ScaleIndex
        {
            return in.atomic_;
        }
        auto operator()(const DefRef& in) noexcept -> ScaleIndex
        {
            return std::get<2>(in.front());
        }
    };

    return std::visit(Visitor{}, data_);
}

auto DefinitionPrivate::DefaultScale() const noexcept -> ScaleIndex
{
    struct Visitor {
        auto operator()(const Runtime& in) noexcept -> ScaleIndex
        {
            return in.default_;
        }
        auto operator()(const DefRef& in) noexcept -> ScaleIndex
        {
            return std::get<1>(in.front());
        }
    };

    return std::visit(Visitor{}, data_);
}

auto DefinitionPrivate::Import(
    const std::string_view in,
    const ScaleIndex index) const noexcept -> std::optional<Amount>
{
    if (index < ScaleCount()) {
        struct Visitor {
            std::string_view in_{};
            ScaleIndex scale_{};

            auto operator()(const Runtime& in) noexcept -> std::optional<Amount>
            {
                return in.scales_[scale_].second.Import(in_);
            }
            auto operator()(const DefRef& in) noexcept -> std::optional<Amount>
            {
                const auto& def = std::get<3>(in.front())[scale_];

                return ScalePrivate{def}.Import(in_);
            }
        };

        return std::visit(Visitor{in, index}, data_);
    } else {
        LogError()()("scale out of range").Flush();

        return std::nullopt;
    }
}

auto DefinitionPrivate::Scale(ScaleIndex scale) const noexcept -> display::Scale
{
    if (scale < ScaleCount()) {
        struct Visitor {
            ScaleIndex scale_{};

            auto operator()(const Runtime& in) noexcept -> display::Scale
            {
                return in.scales_[scale_].second;
            }
            auto operator()(const DefRef& in) noexcept -> display::Scale
            {
                const auto& def = std::get<3>(in.front())[scale_];

                return std::make_unique<ScalePrivate>(def).release();
            }
        };

        return std::visit(Visitor{scale}, data_);
    } else {

        return {};
    }
}

auto DefinitionPrivate::ScaleCount() const noexcept -> ScaleIndex
{
    struct Visitor {
        auto operator()(const Runtime& in) noexcept -> ScaleIndex
        {
            return static_cast<ScaleIndex>(in.scales_.size());
        }
        auto operator()(const DefRef& in) noexcept -> ScaleIndex
        {
            return static_cast<ScaleIndex>(std::get<3>(in.front()).size());
        }
    };

    return std::visit(Visitor{}, data_);
}

auto DefinitionPrivate::ScaleName(ScaleIndex scale) const noexcept
    -> std::string_view
{
    if (scale < ScaleCount()) {
        struct Visitor {
            ScaleIndex scale_{};

            auto operator()(const Runtime& in) noexcept -> std::string_view
            {
                return in.scales_[scale_].first;
            }
            auto operator()(const DefRef& in) noexcept -> std::string_view
            {
                return std::get<3>(in.front())[scale_].front().first;
            }
        };

        return std::visit(Visitor{scale}, data_);
    } else {

        return {};
    }
}

auto DefinitionPrivate::ShortName() const noexcept -> std::string_view
{
    struct Visitor {
        auto operator()(const Runtime& in) noexcept -> std::string_view
        {
            return in.short_name_;
        }
        auto operator()(const DefRef& in) noexcept -> std::string_view
        {
            return std::get<0>(in.front());
        }
    };

    return std::visit(Visitor{}, data_);
}
}  // namespace opentxs::display
