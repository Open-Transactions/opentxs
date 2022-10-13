// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/api/Settings.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Flag;
class String;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::internal
{
class Settings : virtual public api::Settings
{
public:
    virtual auto CheckSetSection(
        const String& strSection,
        const String& strComment,
        bool& out_bIsNewSection) const -> bool = 0;
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
    virtual auto Check_bool(
        const String& strSection,
        const String& strKey,
        bool& out_bResult,
        bool& out_bKeyExist) const -> bool = 0;
    virtual auto Check_long(
        const String& strSection,
        const String& strKey,
        std::int64_t& out_lResult,
        bool& out_bKeyExist) const -> bool = 0;
    virtual auto Check_str(
        const String& strSection,
        const String& strKey,
        String& out_strResult,
        bool& out_bKeyExist) const -> bool = 0;
    virtual auto HasConfigFilePath() const -> bool = 0;
    auto Internal() const noexcept -> const internal::Settings& final
    {
        return *this;
    }
    virtual auto IsEmpty() const -> bool = 0;
    virtual auto IsLoaded() const -> const Flag& = 0;
    virtual auto Load() const -> bool = 0;
    virtual auto SetConfigFilePath(const String& strConfigFilePath) const
        -> void = 0;
    virtual auto SetOption_bool(
        const String& strSection,
        const String& strKey,
        bool& bVariableName) const -> bool = 0;
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

    auto Internal() noexcept -> internal::Settings& final { return *this; }
    virtual auto Reset() -> bool = 0;

    ~Settings() override = default;

protected:
    Settings() = default;
};
}  // namespace opentxs::api::internal
