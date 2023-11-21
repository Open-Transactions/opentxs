// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/SettingsPrivate.hpp"  // IWYU pragma: associated

#include <simpleini/SimpleIni.h>
#include <cstdint>
#include <cstdlib>  // IWYU pragma: keep
#include <memory>
#include <utility>

#include "internal/util/Mutex.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Pimpl.hpp"
#include "internal/util/Size.hpp"
#include "opentxs/api/Paths.internal.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::api
{
auto StringFill(
    opentxs::String& out_strString,
    const char* szString,
    std::int32_t iLength,
    const char* szAppend = nullptr) -> bool;
auto StringFill(
    opentxs::String& out_strString,
    const char* szString,
    std::int32_t iLength,
    const char* szAppend) -> bool
{
    UnallocatedCString strString(szString);

    if (nullptr != szAppend) { strString.append(szAppend); }

    for (; (static_cast<std::int32_t>(strString.length()) < iLength);
         strString.append(" ")) {}

    out_strString.Set(strString.c_str());

    return true;
}
}  // namespace opentxs::api

namespace opentxs::api
{
const OTString SettingsPrivate::blank_{String::Factory()};

class SettingsPrivate::SettingsPvt
{
public:
    CSimpleIniA ini_simple_;

    SettingsPvt()
        : ini_simple_()
    {
    }
    SettingsPvt(const SettingsPvt&) = delete;
    auto operator=(const SettingsPvt&) -> SettingsPvt& = delete;
};

SettingsPrivate::SettingsPrivate(
    const api::internal::Paths& legacy,
    const std::filesystem::path& path)
    : legacy_(legacy)
    , pvt_(new SettingsPvt())
    , loaded_(Flag::Factory(false))
    , lock_()
    , configuration_file_exact_path_(path)
{
    if (configuration_file_exact_path_.empty()) {
        LogAbort()()("configuration_file_exact_path_ is empty!").Abort();
    }

    if (!Init()) { LogAbort()().Abort(); }
}

auto SettingsPrivate::Init() -> bool
{
    // First Load, Create new fresh config file if failed loading.
    if (!Load()) {
        LogConsole()()("No existing configuration. Creating a new file.")
            .Flush();
        if (!Reset()) { return false; }
        if (!Save()) { return false; }
    }

    if (!Reset()) { return false; }

    // Second Load, Throw Assert if Failed loading.
    if (!Load()) {
        LogError()()(
            "Unable to load config file. It should exist, as we just saved it!")
            .Flush();
        LogAbort()().Abort();
    }

    return true;
}

auto SettingsPrivate::Load(const std::filesystem::path& path) const noexcept
    -> bool
{
    const auto strConfigurationFileExactPath = String::Factory(path.string());

    if (!strConfigurationFileExactPath->Exists()) {
        LogError()()("strConfigurationFileExactPath is empty!").Flush();
        return false;
    }

    if (!legacy_.BuildFilePath(strConfigurationFileExactPath->Get())) {
        LogError()()("Failed to construct path ")(
            strConfigurationFileExactPath.get())
            .Flush();

        LogAbort()().Abort();
    }

    if (!IsEmpty()) {
        LogError()()("p_Settings is not empty!").Flush();
        LogAbort()().Abort();
    }

    auto lFilelength = 0_uz;

    if (!legacy_.FileExists(
            strConfigurationFileExactPath->Get(),
            lFilelength))  // we don't have a config file, lets
                           // create a blank one first.
    {
        pvt_->ini_simple_.Reset();  // clean the config.

        SI_Error rc = pvt_->ini_simple_.SaveFile(
            strConfigurationFileExactPath->Get());  // save a new file.
        if (0 > rc) {
            return false;  // error!
        }

        pvt_->ini_simple_.Reset();  // clean the config (again).
    }

    SI_Error rc =
        pvt_->ini_simple_.LoadFile(strConfigurationFileExactPath->Get());
    if (0 > rc) {
        return false;
    } else {
        return true;
    }
}

auto SettingsPrivate::Save(const std::filesystem::path& path) const noexcept
    -> bool
{
    const auto strConfigurationFileExactPath = String::Factory(path.string());

    if (!strConfigurationFileExactPath->Exists()) {
        LogError()()("Error: strConfigurationFileExactPath is empty!").Flush();
        return false;
    }

    SI_Error rc =
        pvt_->ini_simple_.SaveFile(strConfigurationFileExactPath->Get());
    if (0 > rc) {
        return false;
    } else {
        return true;
    }
}

auto SettingsPrivate::LogChange_str(
    const String& strSection,
    const String& strKey,
    const String& strValue) const noexcept -> bool
{
    if (!strSection.Exists()) {
        LogError()()("strSection  is empty!").Flush();
        LogAbort()().Abort();
    }
    if (!strKey.Exists()) {
        LogError()()("strKey is empty!").Flush();
        LogAbort()().Abort();
    }

    const char* const szValue = (strValue.Exists() && !strValue.Compare(""))
                                    ? strValue.Get()
                                    : "nullptr";

    auto strCategory = String::Factory(), strOption = String::Factory();
    if (!StringFill(strCategory, strSection.Get(), 12)) { return false; }
    if (!StringFill(strOption, strKey.Get(), 30, " to:")) { return false; }

    LogDetail()()("Setting ")(strCategory.get())(" ")(strOption.get())(" ")(
        szValue)
        .Flush();
    return true;
}

void SettingsPrivate::SetConfigFilePath(
    const String& strConfigFilePath) const noexcept
{
    rLock lock(lock_);
    configuration_file_exact_path_ =
        UnallocatedCString{strConfigFilePath.Bytes()};
}

auto SettingsPrivate::HasConfigFilePath() const noexcept -> bool
{
    rLock lock(lock_);

    return false == configuration_file_exact_path_.empty();
}

auto SettingsPrivate::Load() const noexcept -> bool
{
    rLock lock(lock_);
    loaded_->Off();

    if (Load(configuration_file_exact_path_)) {
        loaded_->On();

        return true;
    } else {

        return false;
    }
}

auto SettingsPrivate::Save() const noexcept -> bool
{
    rLock lock(lock_);

    return Save(configuration_file_exact_path_);
}

auto SettingsPrivate::IsLoaded() const noexcept -> bool
{
    return loaded_.get();
}

auto SettingsPrivate::Reset() noexcept -> bool
{
    loaded_->Off();
    pvt_->ini_simple_.Reset();

    return true;
}

auto SettingsPrivate::IsEmpty() const noexcept -> bool
{
    rLock lock(lock_);

    return pvt_->ini_simple_.IsEmpty();
}

auto SettingsPrivate::Check_str(
    const String& strSection,
    const String& strKey,
    String& out_strResult,
    bool& out_bKeyExist) const noexcept -> bool
{
    rLock lock(lock_);

    if (!strSection.Exists()) {
        LogError()()("strSection is empty!").Flush();
        LogAbort()().Abort();
    }
    if (strSection.Compare("")) {
        LogError()()("strSection is blank!").Flush();
        LogAbort()().Abort();
    }

    if (!strKey.Exists()) {
        LogError()()("strKey is empty!").Flush();
        LogAbort()().Abort();
    }
    if (strKey.Compare("")) {
        LogError()()("strKey is blank!").Flush();
        LogAbort()().Abort();
    }

    const char* szVar =
        pvt_->ini_simple_.GetValue(strSection.Get(), strKey.Get(), nullptr);
    auto strVar = String::Factory(szVar);

    if (strVar->Exists() && !strVar->Compare("")) {
        out_bKeyExist = true;
        out_strResult.Set(strVar);
    } else {
        out_bKeyExist = false;
        out_strResult.Set("");
    }

    return true;
}

auto SettingsPrivate::Check_long(
    const String& strSection,
    const String& strKey,
    std::int64_t& out_lResult,
    bool& out_bKeyExist) const noexcept -> bool
{
    rLock lock(lock_);

    if (!strSection.Exists()) {
        LogError()()("strSection is empty!").Flush();
        LogAbort()().Abort();
    }
    if (strSection.Compare("")) {
        LogError()()("strSection is blank!").Flush();
        LogAbort()().Abort();
    }

    if (!strKey.Exists()) {
        LogError()()("strKey is empty!").Flush();
        LogAbort()().Abort();
    }
    if (strKey.Compare("")) {
        LogError()()("strKey is Blank!").Flush();
        LogAbort()().Abort();
    }

    const char* szVar =
        pvt_->ini_simple_.GetValue(strSection.Get(), strKey.Get(), nullptr);
    auto strVar = String::Factory(szVar);

    if (strVar->Exists() && !strVar->Compare("")) {
        out_bKeyExist = true;
        out_lResult =
            pvt_->ini_simple_.GetLongValue(strSection.Get(), strKey.Get(), 0);
    } else {
        out_bKeyExist = false;
        out_lResult = 0;
    }

    return true;
}

auto SettingsPrivate::Check_bool(
    const String& strSection,
    const String& strKey,
    bool& out_bResult,
    bool& out_bKeyExist) const noexcept -> bool
{
    rLock lock(lock_);

    if (!strSection.Exists()) {
        LogError()()("strSection is empty!").Flush();
        LogAbort()().Abort();
    }
    if (strSection.Compare("")) {
        LogError()()("strSection is blank!").Flush();
        LogAbort()().Abort();
    }

    if (!strKey.Exists()) {
        LogError()()("strKey is empty!").Flush();
        LogAbort()().Abort();
    }
    if (strKey.Compare("")) {
        LogError()()("strKey is blank!").Flush();
        LogAbort()().Abort();
    }

    const char* szVar =
        pvt_->ini_simple_.GetValue(strSection.Get(), strKey.Get(), nullptr);
    auto strVar = String::Factory(szVar);

    if (strVar->Exists() &&
        (strVar->Compare("false") || strVar->Compare("true"))) {
        out_bKeyExist = true;
        if (strVar->Compare("true")) {
            out_bResult = true;
        } else {
            out_bResult = false;
        }
    } else {
        out_bKeyExist = false;
        out_bResult = false;
    }

    return true;
}

auto SettingsPrivate::Set_str(
    const String& strSection,
    const String& strKey,
    const String& strValue,
    bool& out_bNewOrUpdate) const noexcept -> bool
{
    return Set_str(strSection, strKey, strValue, out_bNewOrUpdate, blank_);
}

auto SettingsPrivate::Set_str(
    const String& strSection,
    const String& strKey,
    const String& strValue,
    bool& out_bNewOrUpdate,
    const String& strComment) const noexcept -> bool
{
    rLock lock(lock_);

    if (!strSection.Exists()) {
        LogError()()("strSection  is empty!").Flush();
        LogAbort()().Abort();
    }
    if (strSection.Compare("")) {
        LogError()()("strSection is blank!").Flush();
        LogAbort()().Abort();
    }

    if (!strKey.Exists()) {
        LogError()()("strKey is empty!").Flush();
        LogAbort()().Abort();
    }
    if (strKey.Compare("")) {
        LogError()()("strKey is blank!").Flush();
        LogAbort()().Abort();
    }

    const char* const szValue =
        (strValue.Exists() && !strValue.Compare("")) ? strValue.Get() : nullptr;
    const char* const szComment =
        (strComment.Exists() && !strComment.Compare("")) ? strComment.Get()
                                                         : nullptr;

    auto strOldValue = String::Factory(), strNewValue = String::Factory();
    bool bOldKeyExist = false, bNewKeyExist = false;

    // Check if Old Key exists.
    if (!Check_str(strSection, strKey, strOldValue, bOldKeyExist)) {
        return false;
    }

    if (bOldKeyExist) {
        if (strValue.Compare(strOldValue)) {
            out_bNewOrUpdate = false;
            return true;
        }
    }

    // Log to Output Setting Change
    if (!LogChange_str(strSection, strKey, strValue)) { return false; }

    // Set New Value
    SI_Error rc = pvt_->ini_simple_.SetValue(
        strSection.Get(), strKey.Get(), szValue, szComment, true);
    if (0 > rc) { return false; }

    if (nullptr == szValue)  // We set the key's value to null, thus removing
                             // it.
    {
        if (bOldKeyExist) {
            out_bNewOrUpdate = true;
        } else {
            out_bNewOrUpdate = false;
        }

        return true;
    }

    // Check if the new value is the same as intended.
    if (!Check_str(strSection, strKey, strNewValue, bNewKeyExist)) {
        return false;
    }

    if (bNewKeyExist) {
        if (strValue.Compare(strNewValue)) {
            // Success
            out_bNewOrUpdate = true;
            return true;
        }
    }

    // If we get here, error!
    LogAbort()().Abort();
}

auto SettingsPrivate::Set_long(
    const String& strSection,
    const String& strKey,
    std::int64_t lValue,
    bool& out_bNewOrUpdate) const noexcept -> bool
{
    return Set_long(strSection, strKey, lValue, out_bNewOrUpdate, blank_);
}

auto SettingsPrivate::Set_long(
    const String& strSection,
    const String& strKey,
    std::int64_t lValue,
    bool& out_bNewOrUpdate,
    const String& strComment) const noexcept -> bool
{
    rLock lock(lock_);

    if (!strSection.Exists()) {
        LogError()()("strSection is empty!").Flush();
        LogAbort()().Abort();
    }
    if (strSection.Compare("")) {
        LogError()()("strSection is blank!").Flush();
        LogAbort()().Abort();
    }

    if (!strKey.Exists()) {
        LogError()()("strKey is empty!").Flush();
        LogAbort()().Abort();
    }
    if (strKey.Compare("")) {
        LogError()()("strKey is blank!").Flush();
        LogAbort()().Abort();
    }

    auto strValue = String::Factory();
    strValue->Set(std::to_string(lValue).c_str());

    const char* const szComment =
        (strComment.Exists() && !strComment.Compare("")) ? strComment.Get()
                                                         : nullptr;

    auto strOldValue = String::Factory(), strNewValue = String::Factory();
    bool bOldKeyExist = false, bNewKeyExist = false;

    // Check if Old Key exists.
    if (!Check_str(strSection, strKey, strOldValue, bOldKeyExist)) {
        return false;
    }

    if (bOldKeyExist) {
        if (strValue->Compare(strOldValue)) {
            out_bNewOrUpdate = false;
            return true;
        }
    }

    // Log to Output Setting Change
    if (!LogChange_str(strSection, strKey, strValue)) { return false; }

    // Set New Value
    SI_Error rc = pvt_->ini_simple_.SetLongValue(
        strSection.Get(),
        strKey.Get(),
        convert_to_size(lValue),
        szComment,
        false,
        true);
    if (0 > rc) { return false; }

    // Check if the new value is the same as intended.
    if (!Check_str(strSection, strKey, strNewValue, bNewKeyExist)) {
        return false;
    }

    if (bNewKeyExist) {
        if (strValue->Compare(strNewValue)) {
            // Success
            out_bNewOrUpdate = true;
            return true;
        }
    }

    // If we get here, error!
    LogAbort()().Abort();
}

auto SettingsPrivate::Set_bool(
    const String& strSection,
    const String& strKey,
    bool bValue,
    bool& out_bNewOrUpdate) const noexcept -> bool
{
    return Set_bool(strSection, strKey, bValue, out_bNewOrUpdate, blank_);
}

auto SettingsPrivate::Set_bool(
    const String& strSection,
    const String& strKey,
    bool bValue,
    bool& out_bNewOrUpdate,
    const String& strComment) const noexcept -> bool
{
    rLock lock(lock_);

    if (!strSection.Exists()) {
        LogError()()("strSection is empty!").Flush();
        LogAbort()().Abort();
    }
    if (!strKey.Exists()) {
        LogError()()("strKey is empty!").Flush();
        LogAbort()().Abort();
    }
    const auto strValue = String::Factory(bValue ? "true" : "false");

    return Set_str(strSection, strKey, strValue, out_bNewOrUpdate, strComment);
}

auto SettingsPrivate::CheckSetSection(
    const String& strSection,
    const String& strComment,
    bool& out_bIsNewSection) const noexcept -> bool
{
    rLock lock(lock_);

    if (!strSection.Exists()) {
        LogError()()("strSection is empty!").Flush();
        LogAbort()().Abort();
    }
    if (!strComment.Exists()) {
        LogError()()("strComment is empty!").Flush();
        LogAbort()().Abort();
    }

    const char* const szComment =
        (strComment.Exists() && !strComment.Compare("")) ? strComment.Get()
                                                         : nullptr;

    std::int64_t lSectionSize =
        pvt_->ini_simple_.GetSectionSize(strSection.Get());

    if (1 > lSectionSize) {
        out_bIsNewSection = true;
        SI_Error rc = pvt_->ini_simple_.SetValue(
            strSection.Get(), nullptr, nullptr, szComment, false);
        if (0 > rc) { return false; }
    } else {
        out_bIsNewSection = false;
    }
    return true;
}

auto SettingsPrivate::CheckSet_str(
    const String& strSection,
    const String& strKey,
    const String& strDefault,
    String& out_strResult,
    bool& out_bIsNew) const noexcept -> bool
{
    return CheckSet_str(
        strSection, strKey, strDefault, out_strResult, out_bIsNew, blank_);
}

auto SettingsPrivate::CheckSet_str(
    const String& strSection,
    const String& strKey,
    const String& strDefault,
    String& out_strResult,
    bool& out_bIsNew,
    const String& strComment) const noexcept -> bool
{
    rLock lock(lock_);
    UnallocatedCString temp = out_strResult.Get();
    bool success = CheckSet_str(
        strSection, strKey, strDefault, temp, out_bIsNew, strComment);
    out_strResult.Set(String::Factory(temp));

    return success;
}

auto SettingsPrivate::CheckSet_str(
    const String& strSection,
    const String& strKey,
    const String& strDefault,
    UnallocatedCString& out_strResult,
    bool& out_bIsNew) const noexcept -> bool
{
    return CheckSet_str(
        strSection, strKey, strDefault, out_strResult, out_bIsNew, blank_);
}

auto SettingsPrivate::CheckSet_str(
    const String& strSection,
    const String& strKey,
    const String& strDefault,
    UnallocatedCString& out_strResult,
    bool& out_bIsNew,
    const String& strComment) const noexcept -> bool
{
    rLock lock(lock_);

    if (!strSection.Exists()) {
        LogError()()("strSection  is empty!").Flush();
        LogAbort()().Abort();
    }
    if (!strKey.Exists()) {
        LogError()()("strKey is empty!").Flush();
        LogAbort()().Abort();
    }

    const char* const szDefault =
        (strDefault.Exists() && !strDefault.Compare("")) ? strDefault.Get()
                                                         : nullptr;

    auto strTempResult = String::Factory();
    bool bKeyExist = false;
    if (!Check_str(strSection, strKey, strTempResult, bKeyExist)) {
        return false;
    }

    if (bKeyExist) {
        // Already have a key, lets use it's value.
        out_bIsNew = false;
        out_strResult = strTempResult->Get();
        return true;
    } else {
        bool bNewKeyCheck;
        if (!Set_str(
                strSection, strKey, strDefault, bNewKeyCheck, strComment)) {
            return false;
        }

        if (nullptr == szDefault)  // The Default is to have no key.
        {
            // Success
            out_bIsNew = false;
            out_strResult = "";
            return true;
        }

        if (bNewKeyCheck) {
            // Success
            out_bIsNew = true;
            out_strResult = strDefault.Get();
            return true;
        }
    }

    // If we get here, error!
    LogAbort()().Abort();
}

auto SettingsPrivate::CheckSet_long(
    const String& strSection,
    const String& strKey,
    std::int64_t lDefault,
    std::int64_t& out_lResult,
    bool& out_bIsNew) const noexcept -> bool
{
    return CheckSet_long(
        strSection, strKey, lDefault, out_lResult, out_bIsNew, blank_);
}

auto SettingsPrivate::CheckSet_long(
    const String& strSection,
    const String& strKey,
    std::int64_t lDefault,
    std::int64_t& out_lResult,
    bool& out_bIsNew,
    const String& strComment) const noexcept -> bool
{
    rLock lock(lock_);

    if (!strSection.Exists()) {
        LogError()()("strSection  is empty!").Flush();
        LogAbort()().Abort();
    }
    if (!strKey.Exists()) {
        LogError()()("strKey is empty!").Flush();
        LogAbort()().Abort();
    }

    std::int64_t lTempResult = 0;
    bool bKeyExist = false;
    if (!Check_long(strSection, strKey, lTempResult, bKeyExist)) {
        return false;
    }

    if (bKeyExist) {
        // Already have a key, lets use it's value.
        out_bIsNew = false;
        out_lResult = lTempResult;
        return true;
    } else {
        bool bNewKeyCheck;
        if (!Set_long(strSection, strKey, lDefault, bNewKeyCheck, strComment)) {
            return false;
        }
        if (bNewKeyCheck) {
            // Success
            out_bIsNew = true;
            out_lResult = lDefault;
            return true;
        }
    }

    // If we get here, error!
    LogAbort()().Abort();
}

auto SettingsPrivate::CheckSet_bool(
    const String& strSection,
    const String& strKey,
    bool bDefault,
    bool& out_bResult,
    bool& out_bIsNew) const noexcept -> bool
{
    return CheckSet_bool(
        strSection, strKey, bDefault, out_bResult, out_bIsNew, blank_);
}

auto SettingsPrivate::CheckSet_bool(
    const String& strSection,
    const String& strKey,
    bool bDefault,
    bool& out_bResult,
    bool& out_bIsNew,
    const String& strComment) const noexcept -> bool
{
    rLock lock(lock_);

    if (!strSection.Exists()) {
        LogError()()("strSection is empty!").Flush();
        LogAbort()().Abort();
    }
    if (!strKey.Exists()) {
        LogError()()("strKey is empty!").Flush();
        LogAbort()().Abort();
    }

    bool bKeyExist = false, bTempResult = false;
    if (!Check_bool(strSection, strKey, bTempResult, bKeyExist)) {
        return false;
    }

    if (bKeyExist) {
        // Already have a key, lets use it's value.
        out_bIsNew = false;
        out_bResult = bTempResult;
        return true;
    } else {
        bool bNewKeyCheck;
        if (!Set_bool(strSection, strKey, bDefault, bNewKeyCheck, strComment)) {
            return false;
        }
        if (bNewKeyCheck) {
            // Success
            out_bIsNew = true;
            out_bResult = bDefault;
            return true;
        }
    }

    // If we get here, error!
    LogAbort()().Abort();
}

auto SettingsPrivate::SetOption_bool(
    const String& strSection,
    const String& strKey,
    bool& bVariableName) const noexcept -> bool
{
    rLock lock(lock_);

    bool bNewOrUpdate;
    return CheckSet_bool(
        strSection, strKey, bVariableName, bVariableName, bNewOrUpdate);
}

auto SettingsPrivate::ReadBool(
    std::string_view section,
    std::string_view key,
    bool& out) const noexcept -> bool
{
    auto notUsed{false};

    return Check_bool(
        String::Factory(section.data(), section.size()),
        String::Factory(key.data(), key.size()),
        out,
        notUsed);
}

auto SettingsPrivate::ReadNumber(
    std::string_view section,
    std::string_view key,
    std::int64_t& out) const noexcept -> bool
{
    auto notUsed{false};

    return Check_long(
        String::Factory(section.data(), section.size()),
        String::Factory(key.data(), key.size()),
        out,
        notUsed);
}

auto SettingsPrivate::ReadString(
    std::string_view section,
    std::string_view key,
    Writer&& out) const noexcept -> bool
{
    auto notUsed{false};
    auto value = String::Factory();
    const auto rc = Check_str(
        String::Factory(section.data(), section.size()),
        String::Factory(key.data(), key.size()),
        value,
        notUsed);

    if (rc) {

        return copy(value->Bytes(), std::move(out));
    } else {

        return false;
    }
}

auto SettingsPrivate::WriteBool(
    std::string_view section,
    std::string_view key,
    bool value) const noexcept -> bool
{
    auto notUsed{false};

    return Set_bool(
        String::Factory(section.data(), section.size()),
        String::Factory(key.data(), key.size()),
        value,
        notUsed);
}

auto SettingsPrivate::WriteNumber(
    std::string_view section,
    std::string_view key,
    std::int64_t value) const noexcept -> bool
{
    auto notUsed{false};

    return Set_long(
        String::Factory(section.data(), section.size()),
        String::Factory(key.data(), key.size()),
        value,
        notUsed);
}

auto SettingsPrivate::WriteString(
    std::string_view section,
    std::string_view key,
    std::string_view value) const noexcept -> bool
{
    auto notUsed{false};

    return Set_str(
        String::Factory(section.data(), section.size()),
        String::Factory(key.data(), key.size()),
        String::Factory(value.data(), value.size()),
        notUsed);
}

SettingsPrivate::~SettingsPrivate()
{
    rLock lock(lock_);

    if (false == Save()) { LogAbort()()("failed to save config file").Abort(); }

    Reset();
}
}  // namespace opentxs::api
