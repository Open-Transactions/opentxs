// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated

#include <mutex>
#include <optional>

#include "internal/util/LogMacros.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::display
{
struct Definition::Imp {
    using Scales = std::pmr::vector<NamedScale>;

    const std::string short_name_;
    const Scales scales_;
    mutable std::mutex lock_;
    mutable std::optional<Map> cached_;

    auto Import(const std::string& in, const Index index) const noexcept(false)
        -> Amount
    {
        try {
            const auto& scale = scales_.at(static_cast<std::size_t>(index));

            return scale.second.Import(in);
        } catch (...) {
            throw std::out_of_range("Invalid scale index");
        }
    }

    auto Populate() const noexcept -> void
    {
        auto lock = Lock{lock_};

        if (false == cached_.has_value()) {
            auto map = Map{};
            auto index = Index{0};

            for (const auto& [name, scale] : scales_) {
                map.emplace(index++, name);
            }

            cached_.emplace(std::move(map));

            OT_ASSERT(cached_.has_value());
        }
    }

    Imp(std::string&& shortname, Scales&& scales) noexcept
        : short_name_(std::move(shortname))
        , scales_(std::move(scales))
        , lock_()
        , cached_()
    {
    }

    Imp(const Imp& rhs) noexcept
        : short_name_(rhs.short_name_)
        , scales_(rhs.scales_)
        , lock_()
        , cached_()
    {
    }
};
}  // namespace opentxs::display
