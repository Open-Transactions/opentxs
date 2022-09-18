// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "api/Legacy.hpp"  // IWYU pragma: associated

#include <cstdlib>
#include <filesystem>
#include <memory>
#include <string_view>

#include "internal/api/Factory.hpp"
#include "internal/api/Legacy.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Numbers.hpp"

#define CLIENT_CONFIG_KEY "client"
#define OPENTXS_CONFIG_KEY "opentxs"
#define SERVER_CONFIG_KEY "server"
#define DATA_FOLDER_EXT "_data"
#define CONFIG_FILE_EXT ".cfg"
#define PID_FILE "opentxs.lock"

namespace opentxs::factory
{
auto Legacy(const std::filesystem::path& home) noexcept
    -> std::unique_ptr<api::Legacy>
{
    using ReturnType = opentxs::api::imp::Legacy;

    return std::make_unique<ReturnType>(home);
}
}  // namespace opentxs::factory

namespace opentxs::api
{
auto Legacy::SuggestFolder(std::string_view appName) noexcept -> fs::path
{
    using ReturnType = opentxs::api::imp::Legacy;

    return ReturnType::get_home_directory() / ReturnType::get_suffix(appName);
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

Legacy::Legacy(const fs::path& home) noexcept
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

auto Legacy::AppendFile(
    fs::path& out,
    const fs::path& base,
    const fs::path& file) const noexcept -> bool
{
    try {
        out = remove_trailing_separator(base) / remove_trailing_separator(file);

        return true;
    } catch (...) {

        return false;
    }
}

auto Legacy::AppendFolder(
    fs::path& out,
    const fs::path& base,
    const fs::path& file) const noexcept -> bool
{
    try {
        out = remove_trailing_separator(base) /
              remove_trailing_separator(file) += fs::path::preferred_separator;

        return true;
    } catch (...) {

        return false;
    }
}

auto Legacy::BuildFolderPath(const fs::path& path) const noexcept -> bool
{
    return ConfirmCreateFolder(path);
}

auto Legacy::BuildFilePath(const fs::path& path) const noexcept -> bool
{
    try {
        if (false == path.has_parent_path()) { return false; }

        const auto parent = path.parent_path();
        fs::create_directories(parent);

        return fs::exists(parent);
    } catch (...) {

        return false;
    }
}

auto Legacy::ClientConfigFilePath(const int instance) const noexcept -> fs::path
{
    return get_file(client_config_file_, instance);
}

auto Legacy::ClientDataFolder(const int instance) const noexcept -> fs::path
{
    return get_path(client_data_folder_, instance);
}

auto Legacy::ConfirmCreateFolder(const fs::path& path) const noexcept -> bool
{
    try {
        fs::create_directories(path);

        return fs::exists(path);
    } catch (...) {

        return false;
    }
}

auto Legacy::FileExists(const fs::path& file, std::size_t& size) const noexcept
    -> bool
{
    size = 0_uz;

    try {
        if (fs::exists(file)) {
            size = convert_to_size(fs::file_size(file));

            return true;
        } else {

            return false;
        }
    } catch (...) {

        return false;
    }
}

auto Legacy::get_app_data_folder(const fs::path& home) noexcept -> fs::path
{
    if (false == home.empty()) { return home; }

    return get_home_directory() / get_suffix();
}

auto Legacy::get_home_directory() noexcept -> fs::path
{
    auto home = UnallocatedCString{};

    if (auto* env = ::getenv("HOME"); nullptr != env) {
        home = env;

        return home;
    }

    if (false == home.empty()) { return home; }

    home = get_home_platform();

    if (false == home.empty()) { return home; }

    LogConsole()("Unable to determine home directory.").Flush();

    OT_FAIL;
}

auto Legacy::get_suffix(std::string_view application) noexcept -> fs::path
{
    auto output = prepend();

    if (use_dot()) { output += '.'; }

    output += application;
    output += fs::path::preferred_separator;

    return output;
}

auto Legacy::get_file(const fs::path& fragment, const int instance)
    const noexcept -> fs::path
{
    const auto output = get_path(fragment, instance).string();

    return UnallocatedCString{output.c_str(), output.size() - 1};
}

auto Legacy::get_path(const fs::path& fragment, const int instance)
    const noexcept -> fs::path
{
    const auto name = [&] {
        auto out = fragment;

        if (0 != instance) {
            out += "-";
            out += std::to_string(instance);
        }

        return out;
    }();
    auto output = fs::path{};
    const auto success = AppendFolder(output, app_data_folder_, name);

    OT_ASSERT(success);

    return output;
}

auto Legacy::LedgerFileName(
    const identifier::Notary& server,
    const identifier::Generic& account) const noexcept -> fs::path
{
    return fs::path{server.asBase58(opentxs::Context().Crypto())} /
           fs::path{account.asBase58(opentxs::Context().Crypto())};
}

auto Legacy::MintFileName(
    const identifier::Notary& server,
    const identifier::UnitDefinition& unit,
    std::string_view extension) const noexcept -> fs::path
{
    auto out = fs::path{server.asBase58(opentxs::Context().Crypto())} /
               fs::path{unit.asBase58(opentxs::Context().Crypto())};

    if (valid(extension)) { out += extension; }

    return out;
}

auto Legacy::OpentxsConfigFilePath() const noexcept -> fs::path
{
    return get_file(opentxs_config_file_);
}

auto Legacy::PIDFilePath() const noexcept -> fs::path
{
    return get_file(pid_file_);
}

auto Legacy::remove_trailing_separator(const fs::path& in) noexcept -> fs::path
{
    const auto path = fs::path{in}.make_preferred();
    auto val = path.string();

    while ((!val.empty()) && (fs::path::preferred_separator == val.back())) {
        val.pop_back();
    }

    return val;
}

auto Legacy::ServerConfigFilePath(const int instance) const noexcept -> fs::path
{
    return get_file(server_config_file_, instance);
}

auto Legacy::ServerDataFolder(const int instance) const noexcept -> fs::path
{
    return get_path(server_data_folder_, instance);
}
}  // namespace opentxs::api::imp
