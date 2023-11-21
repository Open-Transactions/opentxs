// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/Settings.internal.hpp"  // IWYU pragma: associated

namespace opentxs::api::internal
{
auto Settings::CheckSetSection(const String&, const String&, bool&)
    const noexcept -> bool
{
    return {};
}

auto Settings::CheckSet_bool(const String&, const String&, bool, bool&, bool&)
    const noexcept -> bool
{
    return {};
}

auto Settings::CheckSet_bool(
    const String&,
    const String&,
    bool,
    bool&,
    bool&,
    const String&) const noexcept -> bool
{
    return {};
}

auto Settings::CheckSet_long(
    const String&,
    const String&,
    std::int64_t,
    std::int64_t&,
    bool&) const noexcept -> bool
{
    return {};
}

auto Settings::CheckSet_long(
    const String&,
    const String&,
    std::int64_t,
    std::int64_t&,
    bool&,
    const String&) const noexcept -> bool
{
    return {};
}

auto Settings::CheckSet_str(
    const String&,
    const String&,
    const String&,
    String&,
    bool&) const noexcept -> bool
{
    return {};
}

auto Settings::CheckSet_str(
    const String&,
    const String&,
    const String&,
    String&,
    bool&,
    const String&) const noexcept -> bool
{
    return {};
}

auto Settings::CheckSet_str(
    const String&,
    const String&,
    const String&,
    UnallocatedCString&,
    bool&) const noexcept -> bool
{
    return {};
}

auto Settings::CheckSet_str(
    const String&,
    const String&,
    const String&,
    UnallocatedCString&,
    bool&,
    const String&) const noexcept -> bool
{
    return {};
}

auto Settings::Check_bool(const String&, const String&, bool&, bool&)
    const noexcept -> bool
{
    return {};
}

auto Settings::Check_long(const String&, const String&, std::int64_t&, bool&)
    const noexcept -> bool
{
    return {};
}

auto Settings::Check_str(const String&, const String&, String&, bool&)
    const noexcept -> bool
{
    return {};
}

auto Settings::HasConfigFilePath() const noexcept -> bool { return {}; }

auto Settings::IsEmpty() const noexcept -> bool { return {}; }

auto Settings::IsLoaded() const noexcept -> bool { return {}; }

auto Settings::Load() const noexcept -> bool { return {}; }

auto Settings::ReadBool(std::string_view, std::string_view, bool&)
    const noexcept -> bool
{
    return {};
}

auto Settings::ReadNumber(std::string_view, std::string_view, std::int64_t&)
    const noexcept -> bool
{
    return {};
}

auto Settings::ReadString(std::string_view, std::string_view, Writer&&)
    const noexcept -> bool
{
    return {};
}

auto Settings::Reset() noexcept -> bool { return {}; }

auto Settings::Save() const noexcept -> bool { return {}; }

auto Settings::SetConfigFilePath(const String&) const -> void {}

auto Settings::SetOption_bool(const String&, const String&, bool&)
    const noexcept -> bool
{
    return {};
}

auto Settings::Set_bool(const String&, const String&, bool, bool&)
    const noexcept -> bool
{
    return {};
}

auto Settings::Set_bool(
    const String&,
    const String&,
    bool,
    bool&,
    const String&) const noexcept -> bool
{
    return {};
}

auto Settings::Set_long(
    const String&,
    const String&,
    std::int64_t lValue,
    bool&) const noexcept -> bool
{
    return {};
}

auto Settings::Set_long(
    const String&,
    const String&,
    std::int64_t lValue,
    bool&,
    const String&) const noexcept -> bool
{
    return {};
}

auto Settings::Set_str(const String&, const String&, const String&, bool&)
    const noexcept -> bool
{
    return {};
}

auto Settings::Set_str(
    const String&,
    const String&,
    const String&,
    bool&,
    const String&) const noexcept -> bool
{
    return {};
}

auto Settings::WriteBool(std::string_view, std::string_view, const bool)
    const noexcept -> bool
{
    return {};
}

auto Settings::WriteNumber(
    std::string_view,
    std::string_view,
    const std::int64_t) const noexcept -> bool
{
    return {};
}

auto Settings::WriteString(std::string_view, std::string_view, std::string_view)
    const noexcept -> bool
{
    return {};
}
}  // namespace opentxs::api::internal
