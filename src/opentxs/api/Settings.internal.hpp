// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <string_view>

#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace internal
{
class Settings;  // IWYU pragma: keep
}  // namespace internal
}  // namespace api

class String;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class opentxs::api::internal::Settings
{
public:
    virtual auto CheckSetSection(
        const String& strSection,
        const String& strComment,
        bool& out_bIsNewSection) const noexcept -> bool;
    virtual auto CheckSet_bool(
        const String& strSection,
        const String& strKey,
        bool bDefault,
        bool& out_bResult,
        bool& out_bIsNew) const noexcept -> bool;
    virtual auto CheckSet_bool(
        const String& strSection,
        const String& strKey,
        bool bDefault,
        bool& out_bResult,
        bool& out_bIsNew,
        const String& strComment) const noexcept -> bool;
    virtual auto CheckSet_long(
        const String& strSection,
        const String& strKey,
        std::int64_t lDefault,
        std::int64_t& out_lResult,
        bool& out_bIsNew) const noexcept -> bool;
    virtual auto CheckSet_long(
        const String& strSection,
        const String& strKey,
        std::int64_t lDefault,
        std::int64_t& out_lResult,
        bool& out_bIsNew,
        const String& strComment) const noexcept -> bool;
    virtual auto CheckSet_str(
        const String& strSection,
        const String& strKey,
        const String& strDefault,
        String& out_strResult,
        bool& out_bIsNew) const noexcept -> bool;
    virtual auto CheckSet_str(
        const String& strSection,
        const String& strKey,
        const String& strDefault,
        String& out_strResult,
        bool& out_bIsNew,
        const String& strComment) const noexcept -> bool;
    virtual auto CheckSet_str(
        const String& strSection,
        const String& strKey,
        const String& strDefault,
        UnallocatedCString& out_strResult,
        bool& out_bIsNew) const noexcept -> bool;
    virtual auto CheckSet_str(
        const String& strSection,
        const String& strKey,
        const String& strDefault,
        UnallocatedCString& out_strResult,
        bool& out_bIsNew,
        const String& strComment) const noexcept -> bool;
    virtual auto Check_bool(
        const String& strSection,
        const String& strKey,
        bool& out_bResult,
        bool& out_bKeyExist) const noexcept -> bool;
    virtual auto Check_long(
        const String& strSection,
        const String& strKey,
        std::int64_t& out_lResult,
        bool& out_bKeyExist) const noexcept -> bool;
    virtual auto Check_str(
        const String& strSection,
        const String& strKey,
        String& out_strResult,
        bool& out_bKeyExist) const noexcept -> bool;
    virtual auto HasConfigFilePath() const noexcept -> bool;
    virtual auto IsEmpty() const noexcept -> bool;
    virtual auto IsLoaded() const noexcept -> bool;
    virtual auto Load() const noexcept -> bool;
    [[nodiscard]] virtual auto ReadBool(
        std::string_view section,
        std::string_view key,
        bool& out) const noexcept -> bool;
    [[nodiscard]] virtual auto ReadNumber(
        std::string_view section,
        std::string_view key,
        std::int64_t& out) const noexcept -> bool;
    [[nodiscard]] virtual auto ReadString(
        std::string_view section,
        std::string_view key,
        Writer&& out) const noexcept -> bool;
    [[nodiscard]] virtual auto Save() const noexcept -> bool;
    virtual auto SetConfigFilePath(const String& strConfigFilePath) const
        -> void;
    virtual auto SetOption_bool(
        const String& strSection,
        const String& strKey,
        bool& bVariableName) const noexcept -> bool;
    virtual auto Set_bool(
        const String& strSection,
        const String& strKey,
        bool bValue,
        bool& out_bNewOrUpdate) const noexcept -> bool;
    virtual auto Set_bool(
        const String& strSection,
        const String& strKey,
        bool bValue,
        bool& out_bNewOrUpdate,
        const String& strComment) const noexcept -> bool;
    virtual auto Set_long(
        const String& strSection,
        const String& strKey,
        std::int64_t lValue,
        bool& out_bNewOrUpdate) const noexcept -> bool;
    virtual auto Set_long(
        const String& strSection,
        const String& strKey,
        std::int64_t lValue,
        bool& out_bNewOrUpdate,
        const String& strComment) const noexcept -> bool;
    virtual auto Set_str(
        const String& strSection,
        const String& strKey,
        const String& strValue,
        bool& out_bNewOrUpdate) const noexcept -> bool;
    virtual auto Set_str(
        const String& strSection,
        const String& strKey,
        const String& strValue,
        bool& out_bNewOrUpdate,
        const String& strComment) const noexcept -> bool;
    [[nodiscard]] virtual auto WriteBool(
        std::string_view section,
        std::string_view key,
        bool value) const noexcept -> bool;
    [[nodiscard]] virtual auto WriteNumber(
        std::string_view section,
        std::string_view key,
        std::int64_t value) const noexcept -> bool;
    [[nodiscard]] virtual auto WriteString(
        std::string_view section,
        std::string_view key,
        std::string_view value) const noexcept -> bool;

    virtual auto Reset() noexcept -> bool;

    virtual ~Settings() = default;

protected:
    Settings() = default;
};
