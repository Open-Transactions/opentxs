// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string_view>

#include "internal/core/String.hpp"
#include "internal/util/Flag.hpp"
#include "opentxs/api/Settings.internal.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace internal
{
class Paths;
}  // namespace internal

class SettingsPrivate;  // IWYU pragma: keep
}  // namespace api

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class opentxs::api::SettingsPrivate final : public internal::Settings
{
public:
    auto CheckSetSection(
        const String& strSection,
        const String& strComment,
        bool& out_bIsNewSection) const noexcept -> bool final;
    auto CheckSet_bool(
        const String& strSection,
        const String& strKey,
        bool bDefault,
        bool& out_bResult,
        bool& out_bIsNew) const noexcept -> bool final;
    auto CheckSet_bool(
        const String& strSection,
        const String& strKey,
        bool bDefault,
        bool& out_bResult,
        bool& out_bIsNew,
        const String& strComment) const noexcept -> bool final;
    auto CheckSet_long(
        const String& strSection,
        const String& strKey,
        std::int64_t lDefault,
        std::int64_t& out_lResult,
        bool& out_bIsNew) const noexcept -> bool final;
    auto CheckSet_long(
        const String& strSection,
        const String& strKey,
        std::int64_t lDefault,
        std::int64_t& out_lResult,
        bool& out_bIsNew,
        const String& strComment) const noexcept -> bool final;
    auto CheckSet_str(
        const String& strSection,
        const String& strKey,
        const String& strDefault,
        String& out_strResult,
        bool& out_bIsNew) const noexcept -> bool final;
    auto CheckSet_str(
        const String& strSection,
        const String& strKey,
        const String& strDefault,
        String& out_strResult,
        bool& out_bIsNew,
        const String& strComment) const noexcept -> bool final;
    auto CheckSet_str(
        const String& strSection,
        const String& strKey,
        const String& strDefault,
        UnallocatedCString& out_strResult,
        bool& out_bIsNew) const noexcept -> bool final;
    auto CheckSet_str(
        const String& strSection,
        const String& strKey,
        const String& strDefault,
        UnallocatedCString& out_strResult,
        bool& out_bIsNew,
        const String& strComment) const noexcept -> bool final;
    auto Check_bool(
        const String& strSection,
        const String& strKey,
        bool& out_bResult,
        bool& out_bKeyExist) const noexcept -> bool final;
    auto Check_long(
        const String& strSection,
        const String& strKey,
        std::int64_t& out_lResult,
        bool& out_bKeyExist) const noexcept -> bool final;
    auto Check_str(
        const String& strSection,
        const String& strKey,
        String& out_strResult,
        bool& out_bKeyExist) const noexcept -> bool final;
    auto HasConfigFilePath() const noexcept -> bool final;
    auto IsEmpty() const noexcept -> bool final;
    auto IsLoaded() const noexcept -> bool final;
    auto Load() const noexcept -> bool final;
    [[nodiscard]] auto ReadBool(
        std::string_view section,
        std::string_view key,
        bool& out) const noexcept -> bool final;
    [[nodiscard]] auto ReadNumber(
        std::string_view section,
        std::string_view key,
        std::int64_t& out) const noexcept -> bool final;
    [[nodiscard]] auto ReadString(
        std::string_view section,
        std::string_view key,
        Writer&& out) const noexcept -> bool final;
    [[nodiscard]] auto Save() const noexcept -> bool final;
    auto SetConfigFilePath(const String& strConfigFilePath) const noexcept
        -> void final;
    auto SetOption_bool(
        const String& strSection,
        const String& strKey,
        bool& bVariableName) const noexcept -> bool final;
    auto Set_bool(
        const String& strSection,
        const String& strKey,
        bool bValue,
        bool& out_bNewOrUpdate) const noexcept -> bool final;
    auto Set_bool(
        const String& strSection,
        const String& strKey,
        bool bValue,
        bool& out_bNewOrUpdate,
        const String& strComment) const noexcept -> bool final;
    auto Set_long(
        const String& strSection,
        const String& strKey,
        std::int64_t lValue,
        bool& out_bNewOrUpdate) const noexcept -> bool final;
    auto Set_long(
        const String& strSection,
        const String& strKey,
        std::int64_t lValue,
        bool& out_bNewOrUpdate,
        const String& strComment) const noexcept -> bool final;
    auto Set_str(
        const String& strSection,
        const String& strKey,
        const String& strValue,
        bool& out_bNewOrUpdate) const noexcept -> bool final;
    auto Set_str(
        const String& strSection,
        const String& strKey,
        const String& strValue,
        bool& out_bNewOrUpdate,
        const String& strComment) const noexcept -> bool final;
    [[nodiscard]] auto WriteBool(
        std::string_view section,
        std::string_view key,
        bool value) const noexcept -> bool final;
    [[nodiscard]] auto WriteNumber(
        std::string_view section,
        std::string_view key,
        std::int64_t value) const noexcept -> bool final;
    [[nodiscard]] auto WriteString(
        std::string_view section,
        std::string_view key,
        std::string_view value) const noexcept -> bool final;

    auto Reset() noexcept -> bool final;

    SettingsPrivate(
        const api::internal::Paths& legacy,
        const std::filesystem::path& path);
    SettingsPrivate(const SettingsPrivate&) = delete;
    auto operator=(const SettingsPrivate&) -> SettingsPrivate& = delete;

    ~SettingsPrivate() final;

private:
    class SettingsPvt;

    static const OTString blank_;

    const api::internal::Paths& legacy_;
    std::unique_ptr<SettingsPvt> pvt_;
    mutable OTFlag loaded_;
    mutable std::recursive_mutex lock_;
    mutable std::filesystem::path configuration_file_exact_path_;

    // Core (Load and Save)
    auto Load(const std::filesystem::path& path) const noexcept -> bool;
    auto Save(const std::filesystem::path& path) const noexcept -> bool;

    // Log (log to Output in a well-formated way).
    auto LogChange_str(
        const String& strSection,
        const String& strKey,
        const String& strValue) const noexcept -> bool;

    auto Init() -> bool;
};
