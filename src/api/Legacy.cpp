// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "api/Legacy.hpp"  // IWYU pragma: associated

#include <boost/filesystem.hpp>
#include <cstdlib>
#include <memory>
#include <utility>

#include "internal/api/Factory.hpp"
#include "internal/api/Legacy.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Pimpl.hpp"

#define CLIENT_CONFIG_KEY "client"
#define OPENTXS_CONFIG_KEY "opentxs"
#define SERVER_CONFIG_KEY "server"
#define DATA_FOLDER_EXT "_data"
#define CONFIG_FILE_EXT ".cfg"
#define PID_FILE "opentxs.lock"

namespace opentxs::factory
{
auto Legacy(const UnallocatedCString& home) noexcept
    -> std::unique_ptr<api::Legacy>
{
    using ReturnType = opentxs::api::imp::Legacy;

    return std::make_unique<ReturnType>(home);
}
}  // namespace opentxs::factory

namespace opentxs::api
{
auto Legacy::PathSeparator() noexcept -> const char* { return "/"; }

auto Legacy::SuggestFolder(const UnallocatedCString& app) noexcept
    -> UnallocatedCString
{
    using ReturnType = opentxs::api::imp::Legacy;
    const auto path =
        ReturnType::get_home_directory() / ReturnType::get_suffix(app.c_str());

    return path.string();
}

auto Legacy::Concatenate(
    const UnallocatedCString& notary_id,
    const UnallocatedCString& path_separator,
    const UnallocatedCString& unit_id,
    const char* append) -> UnallocatedCString
{
    UnallocatedCString app(append);

    UnallocatedCString tmp;
    tmp.reserve(
        notary_id.length() + path_separator.length() + unit_id.length() +
        app.length());

    tmp.append(notary_id);
    tmp.append(path_separator);
    tmp.append(unit_id);
    tmp.append(app);

    return tmp;
}

auto Legacy::Concatenate(const UnallocatedCString& unit_id, const char* append)
    -> UnallocatedCString
{
    UnallocatedCString app(append);
    UnallocatedCString tmp;
    tmp.reserve(unit_id.length() + app.length());

    tmp.append(unit_id);
    tmp.append(app);

    return tmp;
}

auto Legacy::internal_concatenate(
    const char* _name,
    const UnallocatedCString& ext) noexcept -> UnallocatedCString
{
    UnallocatedCString name{_name ? _name : ""};
    UnallocatedCString tmp;
    if (!name.empty()) {       // if not empty
        if (name[0] != '-') {  // if not negative
            tmp.reserve(name.length() + ext.length());
            tmp.append(name);
            tmp.append(ext);
        } else {
            LogError()(__FILE__ ":")(__LINE__)(":")(__FUNCTION__)(
                "::received negative number ")(_name);
        }
    } else {
        LogError()(__FILE__
                   ":")(__LINE__)(":")(__FUNCTION__)("::received nullptr");
    }

    return tmp;
}

auto Legacy::GetFilenameBin(const char* filename) noexcept -> UnallocatedCString
{
    static UnallocatedCString ext{".bin"};
    return internal_concatenate(filename, ext);
}

auto Legacy::GetFilenameA(const char* filename) noexcept -> UnallocatedCString
{
    static UnallocatedCString ext{".a"};
    return internal_concatenate(filename, ext);
}

auto Legacy::GetFilenameR(const char* foldername) noexcept -> UnallocatedCString
{
    static UnallocatedCString ext{".r"};
    return internal_concatenate(foldername, ext);
}

auto Legacy::GetFilenameRct(TransactionNumber number) noexcept
    -> UnallocatedCString
{
    static UnallocatedCString ext{".rct"};
    return internal_concatenate(std::to_string(number).c_str(), ext);
}

auto Legacy::GetFilenameCrn(TransactionNumber number) noexcept
    -> UnallocatedCString
{
    static UnallocatedCString ext{".crn"};
    return internal_concatenate(std::to_string(number).c_str(), ext);
}

auto Legacy::GetFilenameSuccess(const char* filename) noexcept
    -> UnallocatedCString
{
    static UnallocatedCString ext{".success"};
    return internal_concatenate(filename, ext);
}

auto Legacy::GetFilenameFail(const char* filename) noexcept
    -> UnallocatedCString
{
    static UnallocatedCString ext{".fail"};
    return internal_concatenate(filename, ext);
}

auto Legacy::GetFilenameError(const char* filename) noexcept
    -> UnallocatedCString
{
    static UnallocatedCString ext{".error"};
    return Legacy::internal_concatenate(filename, ext);
}

auto Legacy::GetFilenameLst(const UnallocatedCString& filename) noexcept
    -> UnallocatedCString
{
    static UnallocatedCString ext{".lst"};
    return Legacy::internal_concatenate(filename.c_str(), ext);
}

}  // namespace opentxs::api

namespace opentxs::api::imp
{
const char* Legacy::account_{"account"};
const char* Legacy::common_{"storage"};
const char* Legacy::contract_{"contract"};
const char* Legacy::cron_{"cron"};
const char* Legacy::expired_box_{"expiredbox"};
const char* Legacy::inbox_{"inbox"};
const char* Legacy::market_{"market"};
const char* Legacy::mint_{"mint"};
const char* Legacy::nym_{"nyms"};
const char* Legacy::nymbox_{"nymbox"};
const char* Legacy::outbox_{"outbox"};
const char* Legacy::payment_inbox_{"paymentinbox"};
const char* Legacy::receipt_{"receipt"};
const char* Legacy::record_box_{"recordbox"};

Legacy::Legacy(const UnallocatedCString& home) noexcept
    : app_data_folder_(get_app_data_folder(home))
    , client_data_folder_(
          UnallocatedCString(CLIENT_CONFIG_KEY) + DATA_FOLDER_EXT)
    , server_data_folder_(
          UnallocatedCString(SERVER_CONFIG_KEY) + DATA_FOLDER_EXT)
    , client_config_file_(
          UnallocatedCString(CLIENT_CONFIG_KEY) + CONFIG_FILE_EXT)
    , opentxs_config_file_(
          UnallocatedCString(OPENTXS_CONFIG_KEY) + CONFIG_FILE_EXT)
    , server_config_file_(
          UnallocatedCString(SERVER_CONFIG_KEY) + CONFIG_FILE_EXT)
    , pid_file_(PID_FILE)
{
}

auto Legacy::AppendFile(String& out, const String& base, const String& file)
    const noexcept -> bool
{
    try {
        const auto path = fs::path{base.Get()}.remove_trailing_separator() /
                          fs::path{file.Get()}.remove_trailing_separator();
        out.Set(path.string().c_str());

        return true;
    } catch (...) {

        return false;
    }
}

auto Legacy::AppendFolder(String& out, const String& base, const String& file)
    const noexcept -> bool
{
    try {
        const auto path = fs::path{base.Get()}.remove_trailing_separator() /
                          fs::path{file.Get()}.remove_trailing_separator() /
                          fs::path{"/"};
        out.Set(path.string().c_str());

        return true;
    } catch (...) {

        return false;
    }
}

auto Legacy::BuildFolderPath(const String& path) const noexcept -> bool
{
    return ConfirmCreateFolder(path);
}

auto Legacy::BuildFilePath(const String& path) const noexcept -> bool
{
    try {
        const auto incoming = fs::path{path.Get()};

        if (false == incoming.has_parent_path()) { return false; }

        const auto parent = incoming.parent_path();
        fs::create_directories(parent);

        return fs::exists(parent);
    } catch (...) {

        return false;
    }
}

auto Legacy::ClientConfigFilePath(const int instance) const noexcept
    -> UnallocatedCString
{
    return get_file(client_config_file_, instance);
}

auto Legacy::ClientDataFolder(const int instance) const noexcept
    -> UnallocatedCString
{
    return get_path(client_data_folder_, instance);
}

auto Legacy::ConfirmCreateFolder(const String& path) const noexcept -> bool
{
    try {
        const auto folder = fs::path{path.Get()};
        fs::create_directories(folder);

        return fs::exists(folder);
    } catch (...) {

        return false;
    }
}

auto Legacy::FileExists(const String& path, std::size_t& size) const noexcept
    -> bool
{
    size = 0;

    try {
        const auto file = fs::path{path.Get()};

        if (fs::exists(file)) {
            size = fs::file_size(file);

            return true;
        } else {

            return false;
        }
    } catch (...) {

        return false;
    }
}

auto Legacy::get_app_data_folder(const UnallocatedCString& home) noexcept
    -> fs::path
{
    if (false == home.empty()) { return home; }

    return get_home_directory() / get_suffix();
}

auto Legacy::get_home_directory() noexcept -> fs::path
{
    auto home = UnallocatedCString{getenv("HOME")};

    if (false == home.empty()) { return std::move(home); }

    home = get_home_platform();

    if (false == home.empty()) { return std::move(home); }

    LogConsole()("Unable to determine home directory.").Flush();

    OT_FAIL;
}

auto Legacy::get_suffix(const char* application) noexcept -> fs::path
{
    auto output = prepend();

    if (use_dot()) { output += '.'; }

    output += application;
    output += '/';

    return std::move(output);
}

auto Legacy::get_file(const UnallocatedCString& fragment, const int instance)
    const noexcept -> UnallocatedCString
{
    const auto output = get_path(fragment, instance);

    return {output.c_str(), output.size() - 1};
}

auto Legacy::get_path(const UnallocatedCString& fragment, const int instance)
    const noexcept -> UnallocatedCString
{
    const auto name =
        (0 == instance) ? fragment : fragment + "-" + std::to_string(instance);
    auto output = String::Factory();
    const auto success = AppendFolder(
        output,
        String::Factory(app_data_folder_.string()),
        String::Factory(name.c_str()));

    OT_ASSERT(success)

    return output->Get();
}

auto Legacy::OpentxsConfigFilePath() const noexcept -> UnallocatedCString
{
    return get_file(opentxs_config_file_);
}

auto Legacy::PathExists(const String& path) const noexcept -> bool
{
    try {

        return fs::exists(fs::path{path.Get()});
    } catch (...) {

        return false;
    }
}

auto Legacy::PIDFilePath() const noexcept -> UnallocatedCString
{
    return get_file(pid_file_);
}

auto Legacy::ServerConfigFilePath(const int instance) const noexcept
    -> UnallocatedCString
{
    return get_file(server_config_file_, instance);
}

auto Legacy::ServerDataFolder(const int instance) const noexcept
    -> UnallocatedCString
{
    return get_path(server_data_folder_, instance);
}
}  // namespace opentxs::api::imp
