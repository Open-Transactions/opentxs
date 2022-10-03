// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <filesystem>
#include <string_view>

#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Generic;
class Notary;
class UnitDefinition;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace fs = std::filesystem;

namespace opentxs::api
{
class Legacy
{
public:
    static auto SuggestFolder(std::string_view appName) noexcept -> fs::path;
    static auto GetFilenameBin(const char* filename) noexcept
        -> UnallocatedCString;
    static auto GetFilenameA(const char* filename) noexcept
        -> UnallocatedCString;
    static auto GetFilenameR(const char* filename) noexcept
        -> UnallocatedCString;
    static auto GetFilenameRct(TransactionNumber number) noexcept
        -> UnallocatedCString;
    static auto GetFilenameCrn(TransactionNumber number) noexcept
        -> UnallocatedCString;
    static auto GetFilenameSuccess(const char* filename) noexcept
        -> UnallocatedCString;
    static auto GetFilenameFail(const char* filename) noexcept
        -> UnallocatedCString;
    static auto GetFilenameError(const char* filename) noexcept
        -> UnallocatedCString;
    static auto GetFilenameLst(const UnallocatedCString& filename) noexcept
        -> UnallocatedCString;
    static auto Concatenate(
        const UnallocatedCString& notary_id,
        const UnallocatedCString& path_separator,
        const UnallocatedCString& unit_id,
        const char* append = "") -> UnallocatedCString;
    static auto Concatenate(
        const UnallocatedCString& unit_id,
        const char* append) -> UnallocatedCString;
    virtual auto Account() const noexcept -> const char* = 0;
    virtual auto AppendFile(
        fs::path& out,
        const fs::path& base,
        const fs::path& file) const noexcept -> bool = 0;
    virtual auto AppendFolder(
        fs::path& out,
        const fs::path& base,
        const fs::path& folder) const noexcept -> bool = 0;
    virtual auto BuildFolderPath(const fs::path& path) const noexcept
        -> bool = 0;
    virtual auto BuildFilePath(const fs::path& path) const noexcept -> bool = 0;
    virtual auto ClientConfigFilePath(const int instance) const noexcept
        -> fs::path = 0;
    virtual auto ClientDataFolder(const int instance) const noexcept
        -> fs::path = 0;
    virtual auto Common() const noexcept -> const char* = 0;
    virtual auto ConfirmCreateFolder(const fs::path& path) const noexcept
        -> bool = 0;
    virtual auto Contract() const noexcept -> const char* = 0;
    virtual auto Cron() const noexcept -> const char* = 0;
    virtual auto ExpiredBox() const noexcept -> const char* = 0;
    virtual auto FileExists(const fs::path& path, std::size_t& size)
        const noexcept -> bool = 0;
    virtual auto Inbox() const noexcept -> const char* = 0;
    virtual auto LedgerFileName(
        const identifier::Notary& server,
        const identifier::Generic& account) const noexcept -> fs::path = 0;
    virtual auto Market() const noexcept -> const char* = 0;
    virtual auto Mint() const noexcept -> const char* = 0;
    virtual auto MintFileName(
        const identifier::Notary& server,
        const identifier::UnitDefinition& unit,
        std::string_view extension = {}) const noexcept -> fs::path = 0;
    virtual auto Nym() const noexcept -> const char* = 0;
    virtual auto Nymbox() const noexcept -> const char* = 0;
    virtual auto OpentxsConfigFilePath() const noexcept -> fs::path = 0;
    virtual auto Outbox() const noexcept -> const char* = 0;
    virtual auto PIDFilePath() const noexcept -> fs::path = 0;
    virtual auto PaymentInbox() const noexcept -> const char* = 0;
    virtual auto Receipt() const noexcept -> const char* = 0;
    virtual auto RecordBox() const noexcept -> const char* = 0;
    virtual auto ServerConfigFilePath(const int instance) const noexcept
        -> fs::path = 0;
    virtual auto ServerDataFolder(const int instance) const noexcept
        -> fs::path = 0;

    Legacy(const Legacy&) = delete;
    Legacy(Legacy&&) = delete;
    auto operator=(const Legacy&) -> Legacy& = delete;
    auto operator=(Legacy&&) -> Legacy& = delete;

    virtual ~Legacy() = default;

protected:
    Legacy() noexcept = default;

private:
    static auto internal_concatenate(
        const char* name,
        const UnallocatedCString& ext) noexcept -> UnallocatedCString;
};
}  // namespace opentxs::api
