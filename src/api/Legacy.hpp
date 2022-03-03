// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/filesystem.hpp>
#include <cstddef>
#include <iosfwd>

#include "internal/api/Legacy.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
class String;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace fs = boost::filesystem;

namespace opentxs::api::imp
{
class Legacy final : public api::Legacy
{
public:
    static auto get_home_directory() noexcept -> fs::path;
    static auto get_suffix(const char* application) noexcept -> fs::path;

    auto Account() const noexcept -> const char* final { return account_; }
    auto AppendFile(String& out, const String& base, const String& file)
        const noexcept -> bool final;
    auto AppendFolder(String& out, const String& base, const String& folder)
        const noexcept -> bool final;
    auto BuildFolderPath(const String& path) const noexcept -> bool final;
    auto BuildFilePath(const String& path) const noexcept -> bool final;
    auto ClientConfigFilePath(const int instance) const noexcept
        -> UnallocatedCString final;
    auto ClientDataFolder(const int instance) const noexcept
        -> UnallocatedCString final;
    auto Common() const noexcept -> const char* final { return common_; }
    auto ConfirmCreateFolder(const String& path) const noexcept -> bool final;
    auto Contract() const noexcept -> const char* final { return contract_; }
    auto Cron() const noexcept -> const char* final { return cron_; }
    auto ExpiredBox() const noexcept -> const char* final
    {
        return expired_box_;
    }
    auto FileExists(const String& path, std::size_t& size) const noexcept
        -> bool final;
    auto Inbox() const noexcept -> const char* final { return inbox_; }
    auto Market() const noexcept -> const char* final { return market_; }
    auto Mint() const noexcept -> const char* final { return mint_; }
    auto Nym() const noexcept -> const char* final { return nym_; }
    auto Nymbox() const noexcept -> const char* final { return nymbox_; }
    auto OpentxsConfigFilePath() const noexcept -> UnallocatedCString final;
    auto Outbox() const noexcept -> const char* final { return outbox_; }
    auto PathExists(const String& path) const noexcept -> bool final;
    auto PIDFilePath() const noexcept -> UnallocatedCString final;
    auto PaymentInbox() const noexcept -> const char* final
    {
        return payment_inbox_;
    }
    auto Receipt() const noexcept -> const char* final { return receipt_; }
    auto RecordBox() const noexcept -> const char* final { return record_box_; }
    auto ServerConfigFilePath(const int instance) const noexcept
        -> UnallocatedCString final;
    auto ServerDataFolder(const int instance) const noexcept
        -> UnallocatedCString final;

    Legacy(const UnallocatedCString& home) noexcept;

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

    static auto get_app_data_folder(const UnallocatedCString& home) noexcept
        -> fs::path;
    static auto get_home_platform() noexcept -> UnallocatedCString;
    static auto get_suffix() noexcept -> fs::path;
    static auto prepend() noexcept -> UnallocatedCString;
    static auto use_dot() noexcept -> bool;

    auto get_path(const UnallocatedCString& fragment, const int instance = 0)
        const noexcept -> UnallocatedCString;
    auto get_file(const UnallocatedCString& fragment, const int instance = 0)
        const noexcept -> UnallocatedCString;

    Legacy() = delete;
    Legacy(const Legacy&) = delete;
    Legacy(Legacy&&) = delete;
    auto operator=(const Legacy&) -> Legacy& = delete;
    auto operator=(Legacy&&) -> Legacy& = delete;
};
}  // namespace opentxs::api::imp
