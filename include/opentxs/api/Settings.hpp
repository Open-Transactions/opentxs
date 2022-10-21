// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <string_view>

#include "opentxs/Export.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace internal
{
class Settings;
}  // namespace internal
}  // namespace api

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api
{
class OPENTXS_EXPORT Settings
{
public:
    virtual auto Internal() const noexcept -> const internal::Settings& = 0;
    [[nodiscard]] virtual auto ReadBool(
        const std::string_view section,
        const std::string_view key,
        bool& out) const noexcept -> bool = 0;
    [[nodiscard]] virtual auto ReadNumber(
        const std::string_view section,
        const std::string_view key,
        std::int64_t& out) const noexcept -> bool = 0;
    [[nodiscard]] virtual auto ReadString(
        const std::string_view section,
        const std::string_view key,
        Writer&& out) const noexcept -> bool = 0;
    [[nodiscard]] virtual auto Save() const noexcept -> bool = 0;
    [[nodiscard]] virtual auto WriteBool(
        const std::string_view section,
        const std::string_view key,
        const bool value) const noexcept -> bool = 0;
    [[nodiscard]] virtual auto WriteNumber(
        const std::string_view section,
        const std::string_view key,
        const std::int64_t value) const noexcept -> bool = 0;
    [[nodiscard]] virtual auto WriteString(
        const std::string_view section,
        const std::string_view key,
        const std::string_view value) const noexcept -> bool = 0;

    virtual auto Internal() noexcept -> internal::Settings& = 0;

    Settings(const Settings&) = delete;
    Settings(Settings&&) = delete;
    auto operator=(const Settings&) -> Settings& = delete;
    auto operator=(Settings&&) -> Settings& = delete;

    OPENTXS_NO_EXPORT virtual ~Settings() = default;

protected:
    Settings() = default;
};
}  // namespace opentxs::api
