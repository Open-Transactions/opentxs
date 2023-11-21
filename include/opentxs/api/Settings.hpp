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

class Settings;  // IWYU pragma: keep
}  // namespace api

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class OPENTXS_EXPORT opentxs::api::Settings
{
public:
    OPENTXS_NO_EXPORT auto Internal() const noexcept
        -> const internal::Settings&;
    [[nodiscard]] auto ReadBool(
        std::string_view section,
        std::string_view key,
        bool& out) const noexcept -> bool;
    [[nodiscard]] auto ReadNumber(
        std::string_view section,
        std::string_view key,
        std::int64_t& out) const noexcept -> bool;
    [[nodiscard]] auto ReadString(
        std::string_view section,
        std::string_view key,
        Writer&& out) const noexcept -> bool;
    [[nodiscard]] auto Save() const noexcept -> bool;
    [[nodiscard]] auto WriteBool(
        std::string_view section,
        std::string_view key,
        bool value) const noexcept -> bool;
    [[nodiscard]] auto WriteNumber(
        std::string_view section,
        std::string_view key,
        std::int64_t value) const noexcept -> bool;
    [[nodiscard]] auto WriteString(
        std::string_view section,
        std::string_view key,
        std::string_view value) const noexcept -> bool;

    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::Settings&;

    OPENTXS_NO_EXPORT Settings(internal::Settings* imp) noexcept;
    Settings() = delete;
    Settings(const Settings&) = delete;
    Settings(Settings&&) = delete;
    auto operator=(const Settings&) -> Settings& = delete;
    auto operator=(Settings&&) -> Settings& = delete;

    OPENTXS_NO_EXPORT ~Settings();

private:
    friend internal::Settings;

    internal::Settings* imp_;
};
