// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/core/display/Scale.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::display
{
struct Definition::Imp {
    using Scales = Vector<NamedScale>;

    const CString short_name_;
    const Scales scales_;
    mutable std::mutex lock_;
    mutable std::optional<Map> cached_;

    auto Import(const std::string_view in, const Index index) const
        noexcept(false) -> Amount
    {
        try {
            const auto& scale = scales_.at(static_cast<std::size_t>(index));

            return scale.second.Import(in);
        } catch (...) {
            throw std::out_of_range("Invalid input or scale index");
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

    Imp(CString&& shortname, Scales&& scales) noexcept
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
