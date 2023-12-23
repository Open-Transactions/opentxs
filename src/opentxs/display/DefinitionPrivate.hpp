// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <optional>
#include <string_view>
#include <variant>

#include "internal/core/display/Definition.hpp"
#include "internal/core/display/Types.hpp"
#include "opentxs/display/Scale.hpp"
#include "opentxs/display/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Amount;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::display
{
class DefinitionPrivate final : public internal::Definition
{
public:
    struct Runtime {
        CString short_name_;
        Scales scales_;
        ScaleIndex atomic_;
        ScaleIndex default_;

        Runtime(
            std::string_view shortname = {},
            ScaleIndex defaultScale = {},
            ScaleIndex atomicScale = {},
            Scales&& scales = {}) noexcept;
        Runtime(const Runtime& rhs) noexcept;
    };

    auto AtomicScale() const noexcept -> ScaleIndex;
    auto DefaultScale() const noexcept -> ScaleIndex;
    auto Import(const std::string_view in, const ScaleIndex index)
        const noexcept -> std::optional<Amount>;
    // TODO constexpr
    auto RuntimeAllocated() const noexcept -> bool
    {
        return true;  // TODO return std::holds_alternative<Runtime>(data_);
    }
    auto Scale(ScaleIndex scale) const noexcept -> display::Scale;
    auto ScaleCount() const noexcept -> ScaleIndex;
    auto ScaleName(ScaleIndex scale) const noexcept -> std::string_view;
    auto ShortName() const noexcept -> std::string_view;

    DefinitionPrivate() noexcept;
    DefinitionPrivate(
        std::string_view shortname,
        ScaleIndex defaultScale,
        ScaleIndex atomicScale,
        Scales&& scales) noexcept;
    // TODO constexpr
    DefinitionPrivate(DefRef data) noexcept;
    // TODO constexpr
    DefinitionPrivate(const DefinitionPrivate& rhs) noexcept = default;

    // TODO constexpr
    ~DefinitionPrivate() = default;

private:
    const std::variant<Runtime, DefRef> data_;
};
}  // namespace opentxs::display
