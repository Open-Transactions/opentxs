// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <memory>
#include <mutex>
#include <string_view>
#include <tuple>

#include "internal/api/Settings.hpp"
#include "internal/core/String.hpp"
#include "internal/util/Flag.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Legacy;
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::imp
{
class Settings final : public internal::Settings
{
public:
    auto CheckSetSection(
        const String& strSection,
        const String& strComment,
        bool& out_bIsNewSection) const -> bool final;
    auto CheckSet_bool(
        const String& strSection,
        const String& strKey,
        const bool& bDefault,
        bool& out_bResult,
        bool& out_bIsNew) const -> bool final;
    auto CheckSet_bool(
        const String& strSection,
        const String& strKey,
        const bool& bDefault,
        bool& out_bResult,
        bool& out_bIsNew,
        const String& strComment) const -> bool final;
    auto CheckSet_long(
        const String& strSection,
        const String& strKey,
        const std::int64_t& lDefault,
        std::int64_t& out_lResult,
        bool& out_bIsNew) const -> bool final;
    auto CheckSet_long(
        const String& strSection,
        const String& strKey,
        const std::int64_t& lDefault,
        std::int64_t& out_lResult,
        bool& out_bIsNew,
        const String& strComment) const -> bool final;
    auto CheckSet_str(
        const String& strSection,
        const String& strKey,
        const String& strDefault,
        String& out_strResult,
        bool& out_bIsNew) const -> bool final;
    auto CheckSet_str(
        const String& strSection,
        const String& strKey,
        const String& strDefault,
        String& out_strResult,
        bool& out_bIsNew,
        const String& strComment) const -> bool final;
    auto CheckSet_str(
        const String& strSection,
        const String& strKey,
        const String& strDefault,
        UnallocatedCString& out_strResult,
        bool& out_bIsNew) const -> bool final;
    auto CheckSet_str(
        const String& strSection,
        const String& strKey,
        const String& strDefault,
        UnallocatedCString& out_strResult,
        bool& out_bIsNew,
        const String& strComment) const -> bool final;
    auto Check_bool(
        const String& strSection,
        const String& strKey,
        bool& out_bResult,
        bool& out_bKeyExist) const -> bool final;
    auto Check_long(
        const String& strSection,
        const String& strKey,
        std::int64_t& out_lResult,
        bool& out_bKeyExist) const -> bool final;
    auto Check_str(
        const String& strSection,
        const String& strKey,
        String& out_strResult,
        bool& out_bKeyExist) const -> bool final;
    auto HasConfigFilePath() const -> bool final;
    auto IsEmpty() const -> bool final;
    auto IsLoaded() const -> const Flag& final;
    auto Load() const -> bool final;
    [[nodiscard]] auto ReadBool(
        const std::string_view section,
        const std::string_view key,
        bool& out) const noexcept -> bool final;
    [[nodiscard]] auto ReadNumber(
        const std::string_view section,
        const std::string_view key,
        std::int64_t& out) const noexcept -> bool final;
    [[nodiscard]] auto ReadString(
        const std::string_view section,
        const std::string_view key,
        const AllocateOutput out) const noexcept -> bool final;
    [[nodiscard]] auto Save() const noexcept -> bool final;
    auto SetConfigFilePath(const String& strConfigFilePath) const -> void final;
    auto SetOption_bool(
        const String& strSection,
        const String& strKey,
        bool& bVariableName) const -> bool final;
    auto Set_bool(
        const String& strSection,
        const String& strKey,
        const bool& bValue,
        bool& out_bNewOrUpdate) const -> bool final;
    auto Set_bool(
        const String& strSection,
        const String& strKey,
        const bool& bValue,
        bool& out_bNewOrUpdate,
        const String& strComment) const -> bool final;
    auto Set_long(
        const String& strSection,
        const String& strKey,
        const std::int64_t& lValue,
        bool& out_bNewOrUpdate) const -> bool final;
    auto Set_long(
        const String& strSection,
        const String& strKey,
        const std::int64_t& lValue,
        bool& out_bNewOrUpdate,
        const String& strComment) const -> bool final;
    auto Set_str(
        const String& strSection,
        const String& strKey,
        const String& strValue,
        bool& out_bNewOrUpdate) const -> bool final;
    auto Set_str(
        const String& strSection,
        const String& strKey,
        const String& strValue,
        bool& out_bNewOrUpdate,
        const String& strComment) const -> bool final;
    [[nodiscard]] auto WriteBool(
        const std::string_view section,
        const std::string_view key,
        const bool value) const noexcept -> bool final;
    [[nodiscard]] auto WriteNumber(
        const std::string_view section,
        const std::string_view key,
        const std::int64_t value) const noexcept -> bool final;
    [[nodiscard]] auto WriteString(
        const std::string_view section,
        const std::string_view key,
        const std::string_view value) const noexcept -> bool final;

    auto Reset() -> bool final;

    Settings(const api::Legacy& legacy, const String& strConfigFilePath);
    Settings(const Settings&) = delete;
    auto operator=(const Settings&) -> Settings& = delete;

    ~Settings() final;

private:
    class SettingsPvt;

    static const OTString blank_;

    const api::Legacy& legacy_;
    std::unique_ptr<SettingsPvt> pvt_;
    mutable OTFlag loaded_;
    mutable std::recursive_mutex lock_;
    mutable OTString configuration_file_exact_path_;

    // Core (Load and Save)
    auto Load(const String& strConfigurationFileExactPath) const -> bool;
    auto Save(const String& strConfigurationFileExactPath) const -> bool;

    // Log (log to Output in a well-formated way).
    auto LogChange_str(
        const String& strSection,
        const String& strKey,
        const String& strValue) const -> bool;

    auto Init() -> bool;
};
}  // namespace opentxs::api::imp
