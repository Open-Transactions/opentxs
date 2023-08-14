// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <optional>
#include <string_view>
#include <utility>

#include "internal/core/display/Definition.hpp"
#include "internal/core/display/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/display/Scale.hpp"
#include "opentxs/core/display/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::display
{
class DefinitionPrivate final : public internal::Definition
{
public:
    const CString short_name_;
    const Scales scales_;
    const ScaleIndex atomic_;
    const ScaleIndex default_;

    auto Import(const std::string_view in, const ScaleIndex index)
        const noexcept -> std::optional<Amount>
    {
        if (index < scales_.size()) {

            return scales_[index].second.Import(in);
        } else {
            LogError()(OT_PRETTY_CLASS())("scale out of range").Flush();

            return std::nullopt;
        }
    }

    DefinitionPrivate() noexcept
        : short_name_()
        , scales_()
        , atomic_()
        , default_()
    {
    }
    DefinitionPrivate(
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
    DefinitionPrivate(const DefinitionPrivate& rhs) noexcept
        : short_name_(rhs.short_name_)
        , scales_(rhs.scales_)
        , atomic_(rhs.atomic_)
        , default_(rhs.default_)
    {
    }
};
}  // namespace opentxs::display
