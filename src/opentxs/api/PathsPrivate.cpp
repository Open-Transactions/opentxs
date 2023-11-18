// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/PathsPrivate.hpp"  // IWYU pragma: associated

#include <cstdlib>
#include <filesystem>
#include <memory>
#include <string_view>

#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"

#define CLIENT_CONFIG_KEY "client"
#define OPENTXS_CONFIG_KEY "opentxs"
#define SERVER_CONFIG_KEY "server"
#define DATA_FOLDER_EXT "_data"
#define CONFIG_FILE_EXT ".cfg"
#define PID_FILE "opentxs.lock"

namespace opentxs::api::internal
{
PathsPrivate::PathsPrivate(const std::filesystem::path& home) noexcept
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
    , crypto_()
{
}

auto PathsPrivate::AppendFile(
    std::filesystem::path& out,
    const std::filesystem::path& base,
    const std::filesystem::path& file) const noexcept -> bool
{
    try {
        out = remove_trailing_separator(base) / remove_trailing_separator(file);

        return true;
    } catch (...) {

        return false;
    }
}

auto PathsPrivate::AppendFolder(
    std::filesystem::path& out,
    const std::filesystem::path& base,
    const std::filesystem::path& file) const noexcept -> bool
{
    try {
        out =
            remove_trailing_separator(base) / remove_trailing_separator(file) +=
            std::filesystem::path::preferred_separator;

        return true;
    } catch (...) {

        return false;
    }
}

auto PathsPrivate::BlockchainCheckpoints() const noexcept
    -> std::filesystem::path
{
    return std::filesystem::path{"blockchain"} /
           std::filesystem::path{"checkpoints"};
}

auto PathsPrivate::BuildFolderPath(
    const std::filesystem::path& path) const noexcept -> bool
{
    return ConfirmCreateFolder(path);
}

auto PathsPrivate::BuildFilePath(
    const std::filesystem::path& path) const noexcept -> bool
{
    try {
        if (false == path.has_parent_path()) { return false; }

        const auto parent = path.parent_path();
        std::filesystem::create_directories(parent);

        return std::filesystem::exists(parent);
    } catch (...) {

        return false;
    }
}

auto PathsPrivate::ClientConfigFilePath(const int instance) const noexcept
    -> std::filesystem::path
{
    return get_file(client_config_file_, instance);
}

auto PathsPrivate::ClientDataFolder(const int instance) const noexcept
    -> std::filesystem::path
{
    return get_path(client_data_folder_, instance);
}

auto PathsPrivate::ConfirmCreateFolder(
    const std::filesystem::path& path) const noexcept -> bool
{
    try {
        std::filesystem::create_directories(path);

        return std::filesystem::exists(path);
    } catch (...) {

        return false;
    }
}

auto PathsPrivate::FileExists(
    const std::filesystem::path& file,
    std::size_t& size) const noexcept -> bool
{
    size = 0_uz;

    try {
        if (std::filesystem::exists(file)) {
            size = convert_to_size(std::filesystem::file_size(file));

            return true;
        } else {

            return false;
        }
    } catch (...) {

        return false;
    }
}

auto PathsPrivate::get_app_data_folder(
    const std::filesystem::path& home) noexcept -> std::filesystem::path
{
    if (false == home.empty()) { return home; }

    return get_home_directory() / get_suffix();
}

auto PathsPrivate::get_home_directory() noexcept -> std::filesystem::path
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

    LogAbort()().Abort();
}

auto PathsPrivate::get_suffix(std::string_view application) noexcept
    -> std::filesystem::path
{
    auto output = prepend();

    if (use_dot()) { output += '.'; }

    output += application;
    output += std::filesystem::path::preferred_separator;

    return output;
}

auto PathsPrivate::get_file(
    const std::filesystem::path& fragment,
    const int instance) const noexcept -> std::filesystem::path
{
    const auto output = get_path(fragment, instance).string();

    return UnallocatedCString{output.c_str(), output.size() - 1};
}

auto PathsPrivate::get_path(
    const std::filesystem::path& fragment,
    const int instance) const noexcept -> std::filesystem::path
{
    const auto name = [&] {
        auto out = fragment;

        if (0 != instance) {
            out += "-";
            out += std::to_string(instance);
        }

        return out;
    }();
    auto output = std::filesystem::path{};
    const auto success = AppendFolder(output, app_data_folder_, name);

    assert_true(success);

    return output;
}

auto PathsPrivate::Init(
    const std::shared_ptr<const api::Crypto>& crypto) noexcept -> void
{
    crypto_ = crypto;
}

auto PathsPrivate::LedgerFileName(
    const identifier::Notary& server,
    const identifier::Account& account) const noexcept -> std::filesystem::path
{
    if (auto crypto = crypto_.lock(); crypto) {

        return std::filesystem::path{server.asBase58(*crypto)} /
               std::filesystem::path{account.asBase58(*crypto)};
    } else {

        return {};
    }
}

auto PathsPrivate::MintFileName(
    const identifier::Notary& server,
    const identifier::UnitDefinition& unit,
    std::string_view extension) const noexcept -> std::filesystem::path
{
    if (auto crypto = crypto_.lock(); crypto) {
        auto out = std::filesystem::path{server.asBase58(*crypto)} /
                   std::filesystem::path{unit.asBase58(*crypto)};

        if (valid(extension)) { out += extension; }

        return out;
    } else {

        return {};
    }
}

auto PathsPrivate::OpentxsConfigFilePath() const noexcept
    -> std::filesystem::path
{
    return get_file(opentxs_config_file_);
}

auto PathsPrivate::PIDFilePath() const noexcept -> std::filesystem::path
{
    return get_file(pid_file_);
}

auto PathsPrivate::remove_trailing_separator(
    const std::filesystem::path& in) noexcept -> std::filesystem::path
{
    const auto path = std::filesystem::path{in}.make_preferred();
    auto val = path.string();

    while ((!val.empty()) &&
           (std::filesystem::path::preferred_separator == val.back())) {
        val.pop_back();
    }

    return val;
}

auto PathsPrivate::ServerConfigFilePath(const int instance) const noexcept
    -> std::filesystem::path
{
    return get_file(server_config_file_, instance);
}

auto PathsPrivate::ServerDataFolder(const int instance) const noexcept
    -> std::filesystem::path
{
    return get_path(server_data_folder_, instance);
}
}  // namespace opentxs::api::internal

#undef PID_FILE
#undef CONFIG_FILE_EXT
#undef DATA_FOLDER_EXT
#undef SERVER_CONFIG_KEY
#undef OPENTXS_CONFIG_KEY
#undef CLIENT_CONFIG_KEY
