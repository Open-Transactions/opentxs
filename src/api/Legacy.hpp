// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <filesystem>
#include <string_view>

#include "internal/api/Legacy.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Account;
class Notary;
class UnitDefinition;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::imp
{
class Legacy final : public api::Legacy
{
public:
    static auto get_app_data_folder(const fs::path& home) noexcept -> fs::path;
    static auto get_home_directory() noexcept -> fs::path;
    static auto get_suffix(std::string_view application) noexcept -> fs::path;

    auto Account() const noexcept -> const char* final { return account_; }
    auto AppendFile(fs::path& out, const fs::path& base, const fs::path& file)
        const noexcept -> bool final;
    auto AppendFolder(
        fs::path& out,
        const fs::path& base,
        const fs::path& folder) const noexcept -> bool final;
    auto BlockchainCheckpoints() const noexcept -> fs::path final;
    auto BuildFolderPath(const fs::path& path) const noexcept -> bool final;
    auto BuildFilePath(const fs::path& path) const noexcept -> bool final;
    auto ClientConfigFilePath(const int instance) const noexcept
        -> fs::path final;
    auto ClientDataFolder(const int instance) const noexcept -> fs::path final;
    auto Common() const noexcept -> const char* final { return common_; }
    auto ConfirmCreateFolder(const fs::path& path) const noexcept -> bool final;
    auto Contract() const noexcept -> const char* final { return contract_; }
    auto Cron() const noexcept -> const char* final { return cron_; }
    auto ExpiredBox() const noexcept -> const char* final
    {
        return expired_box_;
    }
    auto FileExists(const fs::path& path, std::size_t& size) const noexcept
        -> bool final;
    auto Inbox() const noexcept -> const char* final { return inbox_; }
    auto LedgerFileName(
        const identifier::Notary& server,
        const identifier::Account& account) const noexcept -> fs::path final;
    auto Market() const noexcept -> const char* final { return market_; }
    auto Mint() const noexcept -> const char* final { return mint_; }
    auto MintFileName(
        const identifier::Notary& server,
        const identifier::UnitDefinition& unit,
        std::string_view extension) const noexcept -> fs::path final;
    auto Nym() const noexcept -> const char* final { return nym_; }
    auto Nymbox() const noexcept -> const char* final { return nymbox_; }
    auto OpentxsConfigFilePath() const noexcept -> fs::path final;
    auto Outbox() const noexcept -> const char* final { return outbox_; }
    auto PIDFilePath() const noexcept -> fs::path final;
    auto PaymentInbox() const noexcept -> const char* final
    {
        return payment_inbox_;
    }
    auto Receipt() const noexcept -> const char* final { return receipt_; }
    auto RecordBox() const noexcept -> const char* final { return record_box_; }
    auto ServerConfigFilePath(const int instance) const noexcept
        -> fs::path final;
    auto ServerDataFolder(const int instance) const noexcept -> fs::path final;

    Legacy(const fs::path& home) noexcept;
    Legacy() = delete;
    Legacy(const Legacy&) = delete;
    Legacy(Legacy&&) = delete;
    auto operator=(const Legacy&) -> Legacy& = delete;
    auto operator=(Legacy&&) -> Legacy& = delete;

    ~Legacy() final = default;

private:
    static const char* account_;
    static const char* common_;
    static const char* contract_;
    static const char* cron_;
    static const char* expired_box_;
    static const char* inbox_;
    static const char* market_;
    static const char* mint_;
    static const char* nym_;
    static const char* nymbox_;
    static const char* outbox_;
    static const char* payment_inbox_;
    static const char* receipt_;
    static const char* record_box_;

    const fs::path app_data_folder_;
    const UnallocatedCString client_data_folder_;
    const UnallocatedCString server_data_folder_;
    const UnallocatedCString client_config_file_;
    const UnallocatedCString opentxs_config_file_;
    const UnallocatedCString server_config_file_;
    const UnallocatedCString pid_file_;

    static auto get_home_platform() noexcept -> UnallocatedCString;
    static auto get_suffix() noexcept -> fs::path;
    static auto prepend() noexcept -> UnallocatedCString;
    static auto remove_trailing_separator(const fs::path& in) noexcept
        -> fs::path;
    static auto use_dot() noexcept -> bool;

    auto get_path(const fs::path& fragment, const int instance = 0)
        const noexcept -> fs::path;
    auto get_file(const fs::path& fragment, const int instance = 0)
        const noexcept -> fs::path;
};
}  // namespace opentxs::api::imp
