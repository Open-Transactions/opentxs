// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <filesystem>
#include <memory>
#include <string_view>

#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace internal
{
class PathsPrivate;  // IWYU pragma: keep
}  // namespace internal

class Crypto;
}  // namespace api

namespace identifier
{
class Account;
class Notary;
class UnitDefinition;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class opentxs::api::internal::PathsPrivate
{
public:
    static auto get_app_data_folder(const std::filesystem::path& home) noexcept
        -> std::filesystem::path;
    static auto get_home_directory() noexcept -> std::filesystem::path;
    static auto get_suffix(std::string_view application) noexcept
        -> std::filesystem::path;

    auto Account() const noexcept -> const char* { return account_; }
    auto AppendFile(
        std::filesystem::path& out,
        const std::filesystem::path& base,
        const std::filesystem::path& file) const noexcept -> bool;
    auto AppendFolder(
        std::filesystem::path& out,
        const std::filesystem::path& base,
        const std::filesystem::path& folder) const noexcept -> bool;
    auto BlockchainCheckpoints() const noexcept -> std::filesystem::path;
    auto BuildFolderPath(const std::filesystem::path& path) const noexcept
        -> bool;
    auto BuildFilePath(const std::filesystem::path& path) const noexcept
        -> bool;
    auto ClientConfigFilePath(const int instance) const noexcept
        -> std::filesystem::path;
    auto ClientDataFolder(const int instance) const noexcept
        -> std::filesystem::path;
    auto Common() const noexcept -> const char* { return common_; }
    auto ConfirmCreateFolder(const std::filesystem::path& path) const noexcept
        -> bool;
    auto Contract() const noexcept -> const char* { return contract_; }
    auto Cron() const noexcept -> const char* { return cron_; }
    auto ExpiredBox() const noexcept -> const char* { return expired_box_; }
    auto FileExists(const std::filesystem::path& path, std::size_t& size)
        const noexcept -> bool;
    auto Inbox() const noexcept -> const char* { return inbox_; }
    auto LedgerFileName(
        const identifier::Notary& server,
        const identifier::Account& account) const noexcept
        -> std::filesystem::path;
    auto Market() const noexcept -> const char* { return market_; }
    auto Mint() const noexcept -> const char* { return mint_; }
    auto MintFileName(
        const identifier::Notary& server,
        const identifier::UnitDefinition& unit,
        std::string_view extension) const noexcept -> std::filesystem::path;
    auto Nym() const noexcept -> const char* { return nym_; }
    auto Nymbox() const noexcept -> const char* { return nymbox_; }
    auto OpentxsConfigFilePath() const noexcept -> std::filesystem::path;
    auto Outbox() const noexcept -> const char* { return outbox_; }
    auto PIDFilePath() const noexcept -> std::filesystem::path;
    auto PaymentInbox() const noexcept -> const char* { return payment_inbox_; }
    auto Receipt() const noexcept -> const char* { return receipt_; }
    auto RecordBox() const noexcept -> const char* { return record_box_; }
    auto ServerConfigFilePath(const int instance) const noexcept
        -> std::filesystem::path;
    auto ServerDataFolder(const int instance) const noexcept
        -> std::filesystem::path;

    auto Init(const std::shared_ptr<const api::Crypto>& crypto) noexcept
        -> void;

    PathsPrivate(const std::filesystem::path& home) noexcept;
    PathsPrivate() = delete;
    PathsPrivate(const PathsPrivate&) = delete;
    PathsPrivate(PathsPrivate&&) = delete;
    auto operator=(const PathsPrivate&) -> PathsPrivate& = delete;
    auto operator=(PathsPrivate&&) -> PathsPrivate& = delete;

    ~PathsPrivate() = default;

private:
    static constexpr auto account_ = "account";
    static constexpr auto common_ = "storage";
    static constexpr auto contract_ = "contract";
    static constexpr auto cron_ = "cron";
    static constexpr auto expired_box_ = "expiredbox";
    static constexpr auto inbox_ = "inbox";
    static constexpr auto market_ = "market";
    static constexpr auto mint_ = "mint";
    static constexpr auto nym_ = "nyms";
    static constexpr auto nymbox_ = "nymbox";
    static constexpr auto outbox_ = "outbox";
    static constexpr auto payment_inbox_ = "paymentinbox";
    static constexpr auto receipt_ = "receipt";
    static constexpr auto record_box_ = "recordbox";

    const std::filesystem::path app_data_folder_;
    const UnallocatedCString client_data_folder_;
    const UnallocatedCString server_data_folder_;
    const UnallocatedCString client_config_file_;
    const UnallocatedCString opentxs_config_file_;
    const UnallocatedCString server_config_file_;
    const UnallocatedCString pid_file_;
    std::weak_ptr<const api::Crypto> crypto_;

    static auto get_home_platform() noexcept -> UnallocatedCString;
    static auto get_suffix() noexcept -> std::filesystem::path;
    static auto prepend() noexcept -> UnallocatedCString;
    static auto remove_trailing_separator(
        const std::filesystem::path& in) noexcept -> std::filesystem::path;
    static auto use_dot() noexcept -> bool;

    auto get_path(const std::filesystem::path& fragment, const int instance = 0)
        const noexcept -> std::filesystem::path;
    auto get_file(const std::filesystem::path& fragment, const int instance = 0)
        const noexcept -> std::filesystem::path;
};
