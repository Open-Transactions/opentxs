// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>

#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
class Flag;
class String;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api
{
class OPENTXS_EXPORT Settings
{
public:
    virtual void SetConfigFilePath(const String& strConfigFilePath) const = 0;
    virtual auto HasConfigFilePath() const -> bool = 0;

    // Core (Public Load and Save)
    virtual auto Load() const -> bool = 0;
    virtual auto Save() const -> bool = 0;

    virtual auto IsLoaded() const -> const Flag& = 0;

    // Configuration Helpers
    //

    // Core (Reset Config, and Check if Config is empty)
    virtual auto IsEmpty() const -> bool = 0;

    // Check Only (get value of key from configuration, if the key exists, then
    // out_bKeyExist will be true.)
    virtual auto Check_str(
        const String& strSection,
        const String& strKey,
        String& out_strResult,
        bool& out_bKeyExist) const -> bool = 0;
    virtual auto Check_long(
        const String& strSection,
        const String& strKey,
        std::int64_t& out_lResult,
        bool& out_bKeyExist) const -> bool = 0;
    virtual auto Check_bool(
        const String& strSection,
        const String& strKey,
        bool& out_bResult,
        bool& out_bKeyExist) const -> bool = 0;

    // Set Only (set new or update value, out_bNewOrUpdate will be true if the
    // value changes.)
    virtual auto Set_str(
        const String& strSection,
        const String& strKey,
        const String& strValue,
        bool& out_bNewOrUpdate) const -> bool = 0;
    virtual auto Set_str(
        const String& strSection,
        const String& strKey,
        const String& strValue,
        bool& out_bNewOrUpdate,
        const String& strComment) const -> bool = 0;
    virtual auto Set_long(
        const String& strSection,
        const String& strKey,
        const std::int64_t& lValue,
        bool& out_bNewOrUpdate) const -> bool = 0;
    virtual auto Set_long(
        const String& strSection,
        const String& strKey,
        const std::int64_t& lValue,
        bool& out_bNewOrUpdate,
        const String& strComment) const -> bool = 0;
    virtual auto Set_bool(
        const String& strSection,
        const String& strKey,
        const bool& bValue,
        bool& out_bNewOrUpdate) const -> bool = 0;
    virtual auto Set_bool(
        const String& strSection,
        const String& strKey,
        const bool& bValue,
        bool& out_bNewOrUpdate,
        const String& strComment) const -> bool = 0;

    // Check for a Section, if the section dosn't exist, it will be made and
    // out_bIsNewSection will be true.)
    virtual auto CheckSetSection(
        const String& strSection,
        const String& strComment,
        bool& out_bIsNewSection) const -> bool = 0;

    // Check for Key, and returns if the key exists, otherwise will set the
    // default key. If the default key is set, then out_bIsNew will be true.)
    virtual auto CheckSet_str(
        const String& strSection,
        const String& strKey,
        const String& strDefault,
        UnallocatedCString& out_strResult,
        bool& out_bIsNew) const -> bool = 0;
    virtual auto CheckSet_str(
        const String& strSection,
        const String& strKey,
        const String& strDefault,
        UnallocatedCString& out_strResult,
        bool& out_bIsNew,
        const String& strComment) const -> bool = 0;
    virtual auto CheckSet_str(
        const String& strSection,
        const String& strKey,
        const String& strDefault,
        String& out_strResult,
        bool& out_bIsNew) const -> bool = 0;
    virtual auto CheckSet_str(
        const String& strSection,
        const String& strKey,
        const String& strDefault,
        String& out_strResult,
        bool& out_bIsNew,
        const String& strComment) const -> bool = 0;
    virtual auto CheckSet_long(
        const String& strSection,
        const String& strKey,
        const std::int64_t& lDefault,
        std::int64_t& out_lResult,
        bool& out_bIsNew) const -> bool = 0;
    virtual auto CheckSet_long(
        const String& strSection,
        const String& strKey,
        const std::int64_t& lDefault,
        std::int64_t& out_lResult,
        bool& out_bIsNew,
        const String& strComment) const -> bool = 0;
    virtual auto CheckSet_bool(
        const String& strSection,
        const String& strKey,
        const bool& bDefault,
        bool& out_bResult,
        bool& out_bIsNew) const -> bool = 0;
    virtual auto CheckSet_bool(
        const String& strSection,
        const String& strKey,
        const bool& bDefault,
        bool& out_bResult,
        bool& out_bIsNew,
        const String& strComment) const -> bool = 0;

    // Set Option helper function for setting bool's
    virtual auto SetOption_bool(
        const String& strSection,
        const String& strKey,
        bool& bVariableName) const -> bool = 0;

    virtual auto Reset() -> bool = 0;

    OPENTXS_NO_EXPORT virtual ~Settings() = default;

protected:
    Settings() = default;

private:
    Settings(const Settings&) = delete;
    Settings(Settings&&) = delete;
    auto operator=(const Settings&) -> Settings& = delete;
    auto operator=(Settings&&) -> Settings& = delete;
};
}  // namespace opentxs::api
