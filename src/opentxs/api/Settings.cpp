// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/Settings.hpp"  // IWYU pragma: associated

#include <utility>

#include "opentxs/api/Settings.internal.hpp"

namespace opentxs::api
{
Settings::Settings(internal::Settings* imp) noexcept
    : imp_(imp)
{
}

auto Settings::Internal() const noexcept -> const internal::Settings&
{
    return *imp_;
}

auto Settings::Internal() noexcept -> internal::Settings& { return *imp_; }

auto Settings::ReadBool(
    std::string_view section,
    std::string_view key,
    bool& out) const noexcept -> bool
{
    return imp_->ReadBool(section, key, out);
}

auto Settings::ReadNumber(
    std::string_view section,
    std::string_view key,
    std::int64_t& out) const noexcept -> bool
{
    return imp_->ReadNumber(section, key, out);
}

auto Settings::ReadString(
    std::string_view section,
    std::string_view key,
    Writer&& out) const noexcept -> bool
{
    return imp_->ReadString(section, key, std::move(out));
}

auto Settings::Save() const noexcept -> bool { return imp_->Save(); }

auto Settings::WriteBool(
    std::string_view section,
    std::string_view key,
    bool value) const noexcept -> bool
{
    return imp_->WriteBool(section, key, value);
}

auto Settings::WriteNumber(
    std::string_view section,
    std::string_view key,
    std::int64_t value) const noexcept -> bool
{
    return imp_->WriteNumber(section, key, value);
}

auto Settings::WriteString(
    std::string_view section,
    std::string_view key,
    std::string_view value) const noexcept -> bool
{
    return imp_->WriteString(section, key, value);
}

Settings::~Settings()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::api
