// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <memory>
#include <mutex>
#include <tuple>

#include "internal/util/Flag.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/core/String.hpp"
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
class Settings final : public api::Settings
{
public:
    void SetConfigFilePath(const String& strConfigFilePath) const final;
    auto HasConfigFilePath() const -> bool final;

    // Core (Public Load and Save)
    auto Load() const -> bool final;
    auto Save() const -> bool final;

    auto IsLoaded() const -> const Flag& final;

    // Configuration Helpers
    //

    // Core (Reset Config, and Check if Config is empty)
    auto IsEmpty() const -> bool final;

    // Check Only (get value of key from configuration, if the key exists, then
    // out_bKeyExist will be true.)
    auto Check_str(
        const String& strSection,
        const String& strKey,
        String& out_strResult,
        bool& out_bKeyExist) const -> bool final;
    auto Check_long(
        const String& strSection,
        const String& strKey,
        std::int64_t& out_lResult,
        bool& out_bKeyExist) const -> bool final;
    auto Check_bool(
        const String& strSection,
        const String& strKey,
        bool& out_bResult,
        bool& out_bKeyExist) const -> bool final;

    // Set Only (set new or update value, out_bNewOrUpdate will be true if the
    // value changes.)
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
    auto Set_bool(
        const String& strSection,
        const String& strKey,
        const bool& bValue,
        bool& out_bNewOrUpdate,
        const String& strComment = String::Factory()) const -> bool final;
    auto Set_bool(
        const String& strSection,
        const String& strKey,
        const bool& bValue,
        bool& out_bNewOrUpdate) const -> bool final;

    // Check for a Section, if the section dosn't exist, it will be made and
    // out_bIsNewSection will be true.)
    auto CheckSetSection(
        const String& strSection,
        const String& strComment,
        bool& out_bIsNewSection) const -> bool final;

    // Check for Key, and returns if the key exists, otherwise will set the
    // default key. If the default key is set, then out_bIsNew will be true.)
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

    // Set Option helper function for setting bool's
    auto SetOption_bool(
        const String& strSection,
        const String& strKey,
        bool& bVariableName) const -> bool final;

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
