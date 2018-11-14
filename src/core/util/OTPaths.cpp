// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/util/OTPaths.hpp"

#include "opentxs/api/Settings.hpp"
#include "opentxs/core/util/Assert.hpp"
#ifdef _WIN32
#include "opentxs/core/util/OTWindowsRegistryTools.hpp"
#endif
#include "opentxs/core/util/StringUtils.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"

#include "Internal.hpp"
#include "Factory.hpp"

#include <sys/stat.h>

#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ostream>
#include <string>
#include <vector>

#define OT_METHOD "opentxs::OTPaths::"

#ifndef S_ISDIR
#define S_ISDIR(mode) (((mode)&S_IFMT) == S_IFDIR)
#endif

#ifndef S_ISREG
#define S_ISREG(mode) (((mode)&S_IFMT) == S_IFREG)
#endif

#if defined(_WIN32)
#define OT_APPDATA_DIR "OpenTransactions"

#elif defined(TARGET_OS_MAC)
#if TARGET_OS_IPHONE  // iOS
#define OT_APPDATA_DIR "Documents"
#else  // OSX
#define OT_APPDATA_DIR "OpenTransactions"
#endif

#elif defined(ANDROID)
#define OT_APPDATA_DIR "ot"

#else  // unix
#define OT_APPDATA_DIR ".ot"

#endif

#ifndef OT_PREFIX_PATH
#ifdef _WIN32
#define OT_PREFIX_PATH                                                         \
    OTPaths::AppDataFolder()  // windows, set to OT AppData Folder.
#elif TARGET_OS_IPHONE
#define OT_PREFIX_PATH                                                         \
    OTPaths::AppDataFolder()  // iphone,  set to OT AppData Folder.
#elif ANDROID
#define OT_PREFIX_PATH                                                         \
    "res/raw"  // android, set to res/raw folder for static files in android app
// sandbox.
#else
#define OT_PREFIX_PATH "/usr/local"  // default prefix_path unix
#endif
#endif

#define OT_INIT_CONFIG_FILENAME "ot_init.cfg"
#define OT_CONFIG_ISRELATIVE "_is_relative"

#ifdef _WIN32
#define OT_SCRIPTS_DIR "scripts/ot"
#elif defined(ANDROID)
#define OT_SCRIPTS_DIR ""
#else
#define OT_SCRIPTS_DIR "lib/opentxs"
#endif

namespace opentxs
{

api::Settings* OTPaths::s_settings{Factory::Settings()};

OTString OTPaths::s_strAppBinaryFolder(String::Factory());
OTString OTPaths::s_strHomeFolder(String::Factory());
OTString OTPaths::s_strAppDataFolder(String::Factory());
OTString OTPaths::s_strGlobalConfigFile(String::Factory());
OTString OTPaths::s_strPrefixFolder(String::Factory());
OTString OTPaths::s_strScriptsFolder(String::Factory());

OTPaths::~OTPaths() {}

const String& OTPaths::AppBinaryFolder()
{
    return OTPaths::s_strAppBinaryFolder;
}

void OTPaths::SetAppBinaryFolder(const String& strLocation)
{
    //  OTPaths::s_strAppBinaryFolder = String::Factory(strLocation.Get());
    OTPaths::s_strAppBinaryFolder->Set(strLocation);
}

const String& OTPaths::HomeFolder() { return OTPaths::s_strHomeFolder; }

void OTPaths::SetHomeFolder(const String& strLocation)
{
    OTPaths::s_strHomeFolder->Set(strLocation);

    s_strAppDataFolder->Release();  // So it will be regenerated.

#ifdef ANDROID
    OTPaths::s_settings->SetConfigFilePath(GlobalConfigFile());
#endif
}

const String& OTPaths::AppDataFolder()
{
    if (s_strAppDataFolder->Exists())
        return s_strAppDataFolder;  // already got it, just return it.

    auto strHomeDataFolder = String::Factory(OTPaths::HomeFolder().Get()),
         strAppDataFolder =
             String::Factory("");  // eg. /home/user/  (the folder that the OT
                                   // appdata folder will be in.)

    if (!strHomeDataFolder->Exists() && !GetHomeFromSystem(strHomeDataFolder)) {
        OT_FAIL;
    }

    // now lets change all the '\' into '/'
    // then check that the path /home/user indeed exists, and is a folder.

    FixPath(strHomeDataFolder, strHomeDataFolder, true);
    if (!PathExists(strHomeDataFolder)) OT_FAIL;

    // ok, we have the HomeData Folder, lets append our OT folder to it.

    if (!AppendFolder(
            strAppDataFolder,
            strHomeDataFolder,
            String::Factory(OT_APPDATA_DIR)))
        OT_FAIL;

    bool bFolderCreated;
    if (!BuildFolderPath(strAppDataFolder, bFolderCreated)) OT_FAIL;

    s_strAppDataFolder = strAppDataFolder;  // all good lets set it.

    return s_strAppDataFolder;
}

const String& OTPaths::GlobalConfigFile()
{
    if (s_strGlobalConfigFile->Exists())
        return s_strGlobalConfigFile;  // got it, lets return it.

    auto strGlobalConfigFile = String::Factory();

    if (!AppendFile(
            strGlobalConfigFile,
            AppDataFolder(),
            String::Factory(OT_INIT_CONFIG_FILENAME)))
        OT_FAIL;

    s_strGlobalConfigFile = strGlobalConfigFile;

    return s_strGlobalConfigFile;
}

const String& OTPaths::PrefixFolder()
{
    if (s_strPrefixFolder->Exists())
        return s_strPrefixFolder;  // got it, lets return it.

    // lets load from the statndard config, or create the entry.
    if (LoadSetPrefixFolder())
        return s_strPrefixFolder;
    else {
        OT_FAIL;
    }
}

const String& OTPaths::ScriptsFolder()
{
    if (s_strScriptsFolder->Exists())
        return s_strScriptsFolder;  // got it, lets return it.

    // load it from config (if we already have it set in the config).
    if (LoadSetScriptsFolder())
        return s_strScriptsFolder;
    else {
        OT_FAIL;
    }
}

// The LoadSet Functions will update the static values.
// static
bool OTPaths::LoadSetPrefixFolder   // eg. /usr/local/
    (api::Settings& config,         // optional
     const String& strPrefixFolder  // optional
     // const bool& bIsRelative (cannot be relative);
    )
{
    /*
    The prefix path is special.

    This path is tested if it is different to the
    one that would be automatically selected by this program
    (aka either compiled into, or from the registry, or the default user data
    directory).

    If the set path is different to what would be supplied and the override path
    value is set.
    Then we will use that path.

    Otherwise, we will update the path in the configuration to link against the
    updated path.

    Users will need to set the override path flag in the configuration,
    if they want to manually set the prefix path.
    */

    if (&config == s_settings) ConfigureDefaultSettings();

    const bool bPreLoaded(config.IsLoaded());

    if (!bPreLoaded) {
        config.Reset();
        if (!config.Load()) { OT_FAIL; }
    }

    {
        // get default path
        auto strDefaultPrefixPath = String::Factory(OT_PREFIX_PATH);
        {
            if (!strDefaultPrefixPath->Exists()) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Error: OT_PREFIX_PATH is not set!")
                    .Flush();
                OT_FAIL;
            }

#ifdef _WIN32
            auto strTemp = String::Factory();
            if (OTPaths::Win_GetInstallFolderFromRegistry(strTemp)) {
                strDefaultPrefixPath = strTemp;
            }
#endif

            if (!ToReal(strDefaultPrefixPath, strDefaultPrefixPath)) {
                OT_FAIL;
            }
            if (!FixPath(strDefaultPrefixPath, strDefaultPrefixPath, true)) {
                OT_FAIL;
            }
        }

        auto strLocalPrefixPath = String::Factory();
        bool bPrefixPathOverride = false;

        {
            // now check the configuration to see what values we have:
            auto strConfigPath = String::Factory();

            bool bIsNew = false;
            auto strPrefixPathOverride =
                String::Factory("prefix_path_override");

            if (!config.CheckSet_str(
                    String::Factory("paths"),
                    String::Factory("prefix_path"),
                    strDefaultPrefixPath,
                    strConfigPath,
                    bIsNew)) {
                return false;
            }
            if (!config.CheckSet_bool(
                    String::Factory("paths"),
                    strPrefixPathOverride,
                    false,
                    bPrefixPathOverride,
                    bIsNew,
                    String::Factory(
                        "; This will force the prefix not to change"))) {
                return false;
            }

            // if the config dosn't have a prefix path set. Lets set the
            // default.
            // if a prefix path was passed in, we will override with that later.
            if (!strConfigPath->Exists() || (3 > strConfigPath->GetLength())) {
                LogOutput(OT_METHOD)(__FUNCTION__)(": Error: Bad "
                                                   "prefix_path"
                                                   " in config, will reset!")
                    .Flush();

                strConfigPath = strDefaultPrefixPath;  // set
                bPrefixPathOverride = false;

                // lets set the default path, and reset override
                bool bNewOrUpdate = false;
                if (!config.Set_str(
                        String::Factory("paths"),
                        String::Factory("prefix_path"),
                        strDefaultPrefixPath,
                        bNewOrUpdate)) {
                    return false;
                }
                if (!config.Set_bool(
                        String::Factory("paths"),
                        strPrefixPathOverride,
                        false,
                        bNewOrUpdate)) {
                    return false;
                }
            }

            strLocalPrefixPath = strConfigPath;
        }

        {
            if (!bPrefixPathOverride) {
                bool bUpdate = false;

                // default
                if (!strLocalPrefixPath->Compare(strDefaultPrefixPath)) {
                    strLocalPrefixPath = strDefaultPrefixPath;
                    bUpdate = true;
                }

                // passed in
                if (strPrefixFolder.Exists() &&
                    (3 < strPrefixFolder.GetLength())) {
                    // a prefix folder was passed in... lets use it, and update
                    // the config if the override isn't set
                    auto strTmp = String::Factory(strPrefixFolder.Get());

                    if (!ToReal(strTmp, strTmp)) { OT_FAIL; }

                    if (!FixPath(strTmp, strTmp, true)) { OT_FAIL; }

                    if (!strLocalPrefixPath->Compare(strTmp)) {
                        strLocalPrefixPath = strTmp;
                        bUpdate = true;
                    }
                }

                // we need to update the path in the config
                if (bUpdate) {
                    bool bNewOrUpdate = false;
                    if (!config.Set_str(
                            String::Factory("paths"),
                            String::Factory("prefix_path"),
                            strLocalPrefixPath,
                            bNewOrUpdate)) {
                        return false;
                    }
                }
            }
        }

        {
            if (!strLocalPrefixPath->Exists()) { OT_FAIL; }

            if (!ToReal(strLocalPrefixPath, strLocalPrefixPath)) { OT_FAIL; }
            if (!FixPath(strLocalPrefixPath, strLocalPrefixPath, true)) {
                OT_FAIL;
            }
            s_strPrefixFolder = strLocalPrefixPath;
        }
    }

    if (!bPreLoaded) {
        if (!config.Save()) { OT_FAIL; }
        config.Reset();
    }

    return true;
}

// static
bool OTPaths::LoadSetScriptsFolder    // ie. PrefixFolder() + [ if (NOT Android)
                                      // "lib/opentxs/" ]
    (api::Settings& config,           // optional
     const String& strScriptsFolder,  // optional
     const bool& bIsRelative          // optional
    )
{
    if (&config == s_settings) ConfigureDefaultSettings();

    const bool bPreLoaded(config.IsLoaded());

    if (!bPreLoaded) {

        config.Reset();
        if (!config.Load()) { OT_FAIL; }
    }

    auto strRelativeKey = String::Factory();
    strRelativeKey->Format("%s%s", "scripts", OT_CONFIG_ISRELATIVE);

    // local vairables.
    bool bConfigIsRelative = false;
    auto strConfigFolder = String::Factory();

    // lets first check what we have in the configuration:
    {
        bool bKeyIsNew = false;

        if (!config.CheckSet_bool(
                String::Factory("paths"),
                strRelativeKey,
                true,
                bConfigIsRelative,
                bKeyIsNew)) {
            return false;
        }
        if (!config.CheckSet_str(
                String::Factory("paths"),
                String::Factory("scripts"),
                String::Factory(OT_SCRIPTS_DIR),
                strConfigFolder,
                bKeyIsNew)) {
            return false;
        }
    }

    // lets first test if there was a folder passed in

    if ((strScriptsFolder.Exists()) && (3 < strScriptsFolder.GetLength())) {

        // we have a folder passed in, lets now check if we need to update
        // anything:

        if (bConfigIsRelative != bIsRelative) {

            bConfigIsRelative = bIsRelative;
            bool bNewOrUpdated = false;

            if (!config.Set_bool(
                    String::Factory("paths"),
                    strRelativeKey,
                    bConfigIsRelative,
                    bNewOrUpdated)) {
                return false;
            }
        }

        if (!strConfigFolder->Compare(strScriptsFolder)) {

            strConfigFolder = strScriptsFolder;  // update folder
            bool bNewOrUpdated = false;

            if (!config.Set_str(
                    String::Factory("paths"),
                    String::Factory("scripts"),
                    strConfigFolder,
                    bNewOrUpdated)) {
                return false;
            }
        }
    }

    if (bConfigIsRelative) {
        if (!FixPath(strConfigFolder, strConfigFolder, true)) { OT_FAIL; }

        auto strPrefixScriptPath = String::Factory();
        AppendFolder(strPrefixScriptPath, PrefixFolder(), strConfigFolder);

        auto strAppBinaryScriptPath = String::Factory();

        // if the AppBinaryFolder is set, we will attempt to use this script
        // path instead.
        // however if the directory dosn't exist, we will default back to
        // appending to the prefix.

        // TODO:  Make the prefix path set to AppBinaryFolder. (da2ce7)

        if (AppBinaryFolder().Exists()) {
            AppendFolder(
                strAppBinaryScriptPath, AppBinaryFolder(), strConfigFolder);
            if (!OTPaths::FolderExists(strAppBinaryScriptPath)) {
                strAppBinaryScriptPath =
                    String::Factory();  // don't have anything here.
            }
        }

        s_strScriptsFolder = strAppBinaryScriptPath->Exists()
                                 ? strAppBinaryScriptPath
                                 : strPrefixScriptPath;

        if (!s_strScriptsFolder->Exists()) OT_FAIL;

    } else {
        if (!ToReal(strConfigFolder, strConfigFolder)) { OT_FAIL; }

        if (!FixPath(strConfigFolder, strConfigFolder, true)) { OT_FAIL; }
        s_strScriptsFolder = strConfigFolder;  // set
    }

    if (!bPreLoaded) {
        if (!config.Save()) { OT_FAIL; }
        config.Reset();
    }

    return true;  // success
}

// static
bool OTPaths::Get(
    api::Settings& config,
    const String& strSection,
    const String& strKey,
    String& out_strVar,
    bool& out_bIsRelative,
    bool& out_bKeyExist)
{
    if (!strSection.Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: "
                                           "strSection passed in!")
            .Flush();
        OT_FAIL;
    }
    if (!strKey.Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: "
                                           "strKey passed in!")
            .Flush();
        OT_FAIL;
    }

    out_strVar.Set("");
    out_bIsRelative = false;
    out_bKeyExist = false;

    const bool bPreLoaded(config.IsLoaded());

    if (!bPreLoaded) {
        config.Reset();
        if (!config.Load()) { OT_FAIL; }
    }

    bool bBoolExists(false), bIsRelative(false);
    auto strRelativeKey = String::Factory(), strOutFolder = String::Factory();

    strRelativeKey->Format("%s%s", strKey.Get(), OT_CONFIG_ISRELATIVE);

    if (config.Check_bool(
            strSection, strRelativeKey, bIsRelative, bBoolExists)) {
        bool bStringExists = false;
        if (config.Check_str(strSection, strKey, strOutFolder, bStringExists)) {
            if (bBoolExists && bStringExists) {
                if (!bIsRelative)  // lets fix the path, so it dosn't matter how
                                   // people write it in the config.
                {
                    if (!ToReal(strOutFolder, strOutFolder)) { OT_FAIL; }

                    if (!FixPath(strOutFolder, strOutFolder, true)) { OT_FAIL; }
                }

                out_strVar.Set(strOutFolder);
                out_bIsRelative = bIsRelative;
                out_bKeyExist = true;
            } else {
                out_strVar.Set("");
                out_bIsRelative = false;
                out_bKeyExist = false;
            }

            if (!bPreLoaded) { config.Reset(); }

            return true;
        }
    }
    // if we get here, there has been a error!
    OT_FAIL;
}

// static
bool OTPaths::Set(
    api::Settings& config,
    const String& strSection,
    const String& strKey,
    const String& strValue,
    const bool& bIsRelative,
    bool& out_bIsNewOrUpdated,
    const String& strComment)
{
    if (!strSection.Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: "
                                           "strSection passed in!")
            .Flush();
        OT_FAIL;
    }
    if (!strKey.Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: "
                                           "strKey passed in!")
            .Flush();
        OT_FAIL;
    }

    out_bIsNewOrUpdated = false;

    const bool bPreLoaded(config.IsLoaded());

    if (!bPreLoaded)  // we only need to load, if not already loaded.
    {
        config.Reset();
        if (!config.Load()) { OT_FAIL; }
    }

    bool bBoolIsNew(false);
    auto strRelativeKey = String::Factory();

    strRelativeKey->Format("%s%s", strKey.Get(), OT_CONFIG_ISRELATIVE);

    if (config.Set_bool(
            strSection, strRelativeKey, bIsRelative, bBoolIsNew, strComment)) {
        bool bStringIsNew = false;
        if (config.Set_str(strSection, strKey, strValue, bStringIsNew)) {
            if (bBoolIsNew && bStringIsNew)  // using existing key
            {
                out_bIsNewOrUpdated = true;
            }

            if (!bPreLoaded) {
                if (!config.Save()) { OT_FAIL; }
                config.Reset();
            }

            return true;
        }
    }

    // if we get here, there has been a error!
    OT_FAIL;
}

// static
bool OTPaths::FixPath(
    const String& strPath,
    String& out_strFixedPath,
    const bool& bIsFolder)
{
    if (!strPath.Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: "
                                           "strPath passed in!")
            .Flush();
        OT_FAIL;
    }

    std::string l_strPath(strPath.Get());
    // first change all back-slashes to forward slashes:
    std::string l_strPath_noBackslash(
        String::replace_chars(l_strPath, "\\", '/'));

    // now we make sure we have the correct trailing "/".

    if ('/' == *l_strPath_noBackslash.rbegin()) {
        if (bIsFolder) {
            out_strFixedPath.Set(l_strPath_noBackslash.c_str());
            return true;
        } else {
            out_strFixedPath.Set(
                l_strPath_noBackslash
                    .substr(0, l_strPath_noBackslash.size() - 1)
                    .c_str());
            return true;
        }
    } else {
        if (bIsFolder) {
            l_strPath_noBackslash += "/";
            out_strFixedPath.Set(l_strPath_noBackslash.c_str());
            return true;
        } else {
            out_strFixedPath.Set(l_strPath_noBackslash.c_str());
            return true;
        }
    }
}

// static
bool OTPaths::PathExists(const String& strPath)
{
    if (!strPath.Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: "
                                           "strPath passed in!")
            .Flush();
        OT_FAIL;
    }

    // remove trailing backslash for stat
    std::string l_strPath(strPath.Get());
    l_strPath = (String::replace_chars(l_strPath, "\\", '/'));  // all \ to /

    // std::string l_strPath_stat = l_strPath;
    std::string l_strPath_stat("");

    // remove last / if it exists (for l_strPath_stat)
    if ('/' == *l_strPath.rbegin())
        l_strPath_stat = l_strPath.substr(0, l_strPath.size() - 1);
    else
        l_strPath_stat = l_strPath;

    struct stat st;
    memset(&st, 0, sizeof(st));

    if (0 == stat(l_strPath_stat.c_str(), &st))  // good we have at-least on a
                                                 // node
    {
        if ('/' != *l_strPath.rbegin()) {
            std::int64_t temp_l = 0;
            return FileExists(strPath, temp_l);
        } else {
            return FolderExists(strPath);
        }
    }
    return false;
}

// static
bool OTPaths::FileExists(const String& strFilePath, std::int64_t& nFileLength)
{
    if (!strFilePath.Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: "
                                           "strFilePath passed in!")
            .Flush();
        OT_FAIL;
    }

    // remove trailing backslash for stat
    std::string l_strPath(strFilePath.Get());
    l_strPath = (String::replace_chars(l_strPath, "\\", '/'));  // all \ to /

    if ('/' != *l_strPath.rbegin()) {
#ifdef _WIN32
        std::string l_strPath_stat = l_strPath;
        struct _stat st_buf;
        memset(&st_buf, 0, sizeof(st_buf));
        char filename[4086];  // not sure about this buffer,
        // on windows paths cannot be longer than 4086,
        // so it should be fine... needs more research.
        strcpy_s(filename, l_strPath_stat.c_str());
        _stat(filename, &st_buf);
#else
        struct stat st_buf;
        memset(&st_buf, 0, sizeof(st_buf));
        stat(l_strPath.c_str(), &st_buf);
#endif

        // check for file
        if (S_ISREG(st_buf.st_mode)) {
            // good we have a file.
            size_t lFileLength = st_buf.st_size;
            nFileLength = static_cast<std::int64_t>(lFileLength);
            return true;
        }
    }
    return false;
}

// static
bool OTPaths::FolderExists(const String& strFolderPath)
{
    if (!strFolderPath.Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: "
                                           "strFolderPath passed in!")
            .Flush();
        OT_FAIL;
    }

    // remove trailing backslash for stat
    std::string l_strPath(strFolderPath.Get());
    l_strPath = (String::replace_chars(l_strPath, "\\", '/'));  // all \ to /

    if ('/' == *l_strPath.rbegin()) {
#ifdef _WIN32
        std::string l_strPath_stat = l_strPath.substr(0, l_strPath.size() - 1);
        struct _stat st_buf;
        memset(&st_buf, 0, sizeof(st_buf));
        char filename[4086] = "";  // not sure about this buffer,
        // on windows paths cannot be longer than 4086,
        // so it should be fine... needs more research.
        strcpy_s(filename, l_strPath_stat.c_str());
        _stat(filename, &st_buf);
#else
        struct stat st_buf;
        memset(&st_buf, 0, sizeof(st_buf));
        stat(l_strPath.c_str(), &st_buf);
#endif

        if (S_ISDIR(st_buf.st_mode)) {
            // good we have a directory.
            return true;
        }
    }
    return false;
}

// static
bool OTPaths::ConfirmCreateFolder(
    const String& strExactPath,
    bool& out_Exists,
    bool& out_IsNew)
{
    const bool bExists = (strExactPath.Exists() && !strExactPath.Compare(""));
    OT_ASSERT_MSG(
        bExists,
        "OTPaths::ConfirmCreateFolder: Assert failed: no strFolderName\n");

    std::string l_strExactPath(strExactPath.Get());

    if ('/' != *l_strExactPath.rbegin()) return false;  // not a directory.

    // Confirm If Directory Exists Already
    out_Exists = PathExists(strExactPath);

    if (out_Exists) {
        out_IsNew = false;
        return true;  // Already Have Folder, lets return true!
    } else {
        // It dosn't exist: lets create it.

#ifdef _WIN32
        bool bCreateDirSuccess = (_mkdir(strExactPath.Get()) == 0);
#else
        bool bCreateDirSuccess = (mkdir(strExactPath.Get(), 0700) == 0);
#endif

        if (!bCreateDirSuccess) {
            out_IsNew = false;
            out_Exists = false;
            return false;
        }

        // At this point if the folder still doesn't exist, nothing we can do.
        // We
        // already tried to create the folder, and SUCCEEDED, and then STILL
        // failed
        // to find it (if this is still false.)
        else {
            bool bCheckDirExist = PathExists(strExactPath);

            if (!bCheckDirExist) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Unable To Confirm Created Directory ")(strExactPath)(".")
                    .Flush();
                out_IsNew = false;
                out_Exists = false;
                return false;
            } else {
                out_IsNew = true;
                out_Exists = false;
                return true;  // We have created and checked the Folder
            }
        }
    }
}

// static
bool OTPaths::ToReal(const String& strExactPath, String& out_strCanonicalPath)
{
    if (!strExactPath.Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: "
                                           "strExactPath passed in!")
            .Flush();
        OT_FAIL;
    }

#ifdef _WIN32
#ifdef _UNICODE

    const char* szPath = strExactPath.Get();
    size_t newsize = strlen(szPath) + 1;
    wchar_t* wzPath = new wchar_t[newsize];

    size_t convertedChars = 0;
    mbstowcs_s(&convertedChars, wzPath, newsize, szPath, 4096);

    wchar_t szBuf[4096] = L"";

    if (GetFullPathName(wzPath, 4096, szBuf, nullptr)) {
        out_strCanonicalPath.Set(utf8util::UTF8FromUTF16(szBuf));
        return true;
    } else {
        out_strCanonicalPath.Set("");
        return false;
    }

#else
    char_t szBuf[4096] = "";
    char_t const* szPath = strRealPath.Get();

    if (GetFullPathName(szPath, 4096, szBuf, nullptr)) {
        out_strCanonicalPath.Set(szBuf);
        return true;
    } else {
        out_strCanonicalPath.Set("");
        return false;
    }

#endif
#else

    char* actualpath = realpath(strExactPath.Get(), NULL);
    if (actualpath == NULL) {

        if (errno == ENOTDIR) {
            out_strCanonicalPath.Set(strExactPath);
            return true;
        }

        if (errno == ENOENT) {
            out_strCanonicalPath.Set(strExactPath);
            return true;
        }

        OT_ASSERT_MSG(
            (errno != EACCES),
            "Error (Realpath: EACCES): Unable to "
            "build RealPath: access denied");
        OT_ASSERT_MSG(
            (errno != EINVAL),
            "Error (RealPath: EINVAL): Input value into RealPath was nullptr");
        OT_ASSERT_MSG(
            (errno != ELOOP),
            "Error (RealPath: ELOOP): Resloving links resulted in a loop.");
        OT_ASSERT_MSG(
            (errno != ENAMETOOLONG),
            "Error (RealPath: ENAMETOOLONG): Name too std::int64_t.");
        OT_ASSERT_MSG(
            (errno != ERANGE),
            "Error (RealPath: ERANGE): Resulting "
            "path is too std::int64_t for the buffer");
        OT_ASSERT_MSG(
            (errno != EIO), "Error (RealPath: EIO): Unable to access path.");

        OT_ASSERT_MSG(
            (false),
            "Error (RealPath: OTHER): Something bad Happend with 'realpath'.");
    }
    out_strCanonicalPath.Set(actualpath);
    free(actualpath);
    return true;
#endif
}

// static
bool OTPaths::GetHomeFromSystem(String& out_strHomeFolder)
{
#ifdef _WIN32
#ifdef _UNICODE
    TCHAR szPath[MAX_PATH] = L"";
#else
    TCHAR szPath[MAX_PATH] = "";
#endif

    if (SUCCEEDED(SHGetFolderPath(
            nullptr, CSIDL_APPDATA | CSIDL_FLAG_CREATE, nullptr, 0, szPath))) {
#ifdef UNICODE
        out_strHomeFolder.Set(utf8util::UTF8FromUTF16(szPath));
#else
        out_strHomeFolder.Set(szPath);
#endif
    } else {
        out_strHomeFolder.Set("");
        return false;
    }

#elif defined(__APPLE__)

    auto home = String::Factory(getenv("HOME"));
    auto library = String::Factory();
    AppendFolder(library, home, String::Factory("Library"));
    AppendFolder(
        out_strHomeFolder, library, String::Factory("Application Support"));

#else
    out_strHomeFolder.Set(getenv("HOME"));
#endif

    return out_strHomeFolder.Exists();
}

#ifdef _WIN32

// static
bool OTPaths::Win_GetInstallFolderFromRegistry(String& out_InstallFolderPath)
{
    WindowsRegistryTools windowsRegistryTools;

    HKEY hKey = 0;
    LONG lRes = RegOpenKeyExW(
        HKEY_LOCAL_MACHINE, L"SOFTWARE\\Open-Transactions", 0, KEY_READ, &hKey);
    bool bExistsAndSuccess(lRes == ERROR_SUCCESS);
    bool bDoesNotExistsSpecifically(lRes == ERROR_FILE_NOT_FOUND);

    std::wstring strValueOfBinDir;
    windowsRegistryTools.GetStringRegKey(
        hKey, L"Path", strValueOfBinDir, L"bad");

    if (bExistsAndSuccess && !bDoesNotExistsSpecifically) {
        std::string strInstallPath(String::ws2s(strValueOfBinDir));
        out_InstallFolderPath.Set(strInstallPath.c_str());

        return true;
    }

    return false;
}

#endif

// static
bool OTPaths::AppendFolder(
    String& out_strPath,
    const String& strBasePath,
    const String& strFolderName)
{
    if (!strBasePath.Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: "
                                           "strBasePath passed in!")
            .Flush();
        OT_FAIL;
    }
    if (!strFolderName.Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: "
                                           "strFolderName passed in!")
            .Flush();
        OT_FAIL;
    }

    auto l_strBasePath_fix = String::Factory(),
         l_strFolderName_fix = String::Factory();

    if (!FixPath(strBasePath, l_strBasePath_fix, true)) return false;
    if (!FixPath(strFolderName, l_strFolderName_fix, true)) return false;

    std::string l_strBasePath(l_strBasePath_fix->Get()),
        l_strFolderName(l_strFolderName_fix->Get());

    l_strBasePath.append(l_strFolderName);

    const auto l_strPath = String::Factory(l_strBasePath);

    out_strPath.Set(l_strPath);
    return true;
}

// static
bool OTPaths::AppendFile(
    String& out_strPath,
    const String& strBasePath,
    const String& strFileName)
{
    if (!strBasePath.Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: "
                                           "strBasePath passed in!")
            .Flush();
        OT_FAIL;
    }
    if (!strFileName.Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: "
                                           "strFileName passed in!")
            .Flush();
        OT_FAIL;
    }

    auto l_strBasePath_fix = String::Factory(),
         l_strFileName_fix = String::Factory();

    if (!FixPath(strBasePath, l_strBasePath_fix, true)) return false;
    if (!FixPath(strFileName, l_strFileName_fix, false)) return false;

    std::string l_strBasePath(l_strBasePath_fix->Get()),
        l_strFileName(l_strFileName_fix->Get());

    l_strBasePath.append(l_strFileName);

    const auto l_strPath = String::Factory(l_strBasePath);

    out_strPath.Set(l_strPath);
    return true;
}

// this function dosn't change the "strRelativePath" so.  It will only fix the
// strBasePath.
// static
bool OTPaths::RelativeToCanonical(
    String& out_strCanonicalPath,
    const String& strBasePath,
    const String& strRelativePath)
{
    if (!strBasePath.Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: "
                                           "strBasePath passed in!")
            .Flush();
        OT_FAIL;
    }
    if (!strRelativePath.Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null: "
                                           "strRelativePath passed in!")
            .Flush();
        OT_FAIL;
    }

    auto l_strBasePath_fix = String::Factory();
    if (!FixPath(strBasePath, l_strBasePath_fix, true)) return false;

    if (strRelativePath.Compare(".")) {
        out_strCanonicalPath.Set(strBasePath);
        return true;
    }  // if ".", return base path.

    std::string l_strBasePath(l_strBasePath_fix->Get()),
        l_strRelativePath(strRelativePath.Get());

    l_strBasePath.append(l_strRelativePath);

    auto l_strPath = String::Factory(l_strBasePath),
         l_strCanonicalPath = String::Factory();

    if (!ToReal(l_strPath, l_strCanonicalPath)) return false;

    out_strCanonicalPath.Set(l_strCanonicalPath);

    return true;
}

// static
bool OTPaths::BuildFolderPath(
    const String& strFolderPath,
    bool& out_bFolderCreated)
{
    out_bFolderCreated = false;

    auto l_strFolderPath_fix = String::Factory(),
         l_strFolderPath_real = String::Factory();

    if (!ToReal(strFolderPath, l_strFolderPath_real))
        return false;  // path to real

    if (!FixPath(l_strFolderPath_real, l_strFolderPath_fix, true))
        return false;  // real to fixed real

    std::string l_strFolderPath(l_strFolderPath_fix->Get());  // fixed real
                                                              // path.

    std::vector<std::string> vFolders;

    split_byChar(vFolders, l_strFolderPath, "/", split::no_empties);

    size_t nSize = vFolders.size();

    std::string l_strPathPart("");
    bool l_FolderExists(false), l_bBuiltFolder(false);

    for (size_t i = 0; i < nSize; i++) {
#ifndef _WIN32                             // aka UNIX
        if (0 == i) l_strPathPart += "/";  // add annother / for root.
#endif
        l_strPathPart += vFolders[i];
        l_strPathPart += "/";

        if (0 == i) continue;  // / or x:/ should be skiped.

        auto strPathPart = String::Factory(l_strPathPart);

        if (!ConfirmCreateFolder(strPathPart, l_FolderExists, l_bBuiltFolder))
            return false;

        if (!out_bFolderCreated && l_bBuiltFolder) out_bFolderCreated = true;
    }
    return true;
}

// static
bool OTPaths::BuildFilePath(
    const String& strFolderPath,
    bool& out_bFolderCreated)
{
    out_bFolderCreated = false;

    auto l_strFilePath_fix = String::Factory(),
         l_strFilePath_real = String::Factory();

    if (!ToReal(strFolderPath, l_strFilePath_real))
        return false;  // path to real

    if (!FixPath(l_strFilePath_real, l_strFilePath_fix, false))
        return false;  // real to fixed real

    std::string l_strFilePath(l_strFilePath_fix->Get());  // fixed real path.

    std::vector<std::string> vFolders;

    split_byChar(vFolders, l_strFilePath, "/", split::no_empties);

    size_t nSize = vFolders.size();

    std::string l_strPathPart("");
    bool l_FolderExists(false), l_bBuiltFolder(false);

    for (size_t i = 0; i < nSize; i++) {
#ifndef _WIN32                             // aka UNIX
        if (0 == i) l_strPathPart += "/";  // add annother / for root.
#endif

        l_strPathPart += vFolders[i];

        if ((i + 1) == nSize) continue;  // file should be skipped

        l_strPathPart += "/";  // is a folder, so should append /

        if (0 == i) continue;  // / or x:/ should be skiped.

        auto strPathPart = String::Factory(l_strPathPart);
        if (!ConfirmCreateFolder(strPathPart, l_FolderExists, l_bBuiltFolder))
            return false;

        if (!out_bFolderCreated && l_bBuiltFolder) out_bFolderCreated = true;
    }
    return true;
}

void OTPaths::ConfigureDefaultSettings()
{
    if (!s_settings->HasConfigFilePath())
        s_settings->SetConfigFilePath(GlobalConfigFile());
}

}  // namespace opentxs
