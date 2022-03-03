// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
class String;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api
{
class Legacy
{
public:
    static auto PathSeparator() noexcept -> const char*;
    static auto SuggestFolder(const UnallocatedCString& app) noexcept
        -> UnallocatedCString;
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
    virtual auto AppendFile(String& out, const String& base, const String& file)
        const noexcept -> bool = 0;
    virtual auto AppendFolder(
        String& out,
        const String& base,
        const String& folder) const noexcept -> bool = 0;
    virtual auto BuildFolderPath(const String& path) const noexcept -> bool = 0;
    virtual auto BuildFilePath(const String& path) const noexcept -> bool = 0;
    virtual auto ClientConfigFilePath(const int instance) const noexcept
        -> UnallocatedCString = 0;
    virtual auto ClientDataFolder(const int instance) const noexcept
        -> UnallocatedCString = 0;
    virtual auto Common() const noexcept -> const char* = 0;
    virtual auto ConfirmCreateFolder(const String& path) const noexcept
        -> bool = 0;
    virtual auto Contract() const noexcept -> const char* = 0;
    virtual auto Cron() const noexcept -> const char* = 0;
    virtual auto ExpiredBox() const noexcept -> const char* = 0;
    virtual auto FileExists(const String& path, std::size_t& size)
        const noexcept -> bool = 0;
    virtual auto Inbox() const noexcept -> const char* = 0;
    virtual auto Market() const noexcept -> const char* = 0;
    virtual auto Mint() const noexcept -> const char* = 0;
    virtual auto Nym() const noexcept -> const char* = 0;
    virtual auto Nymbox() const noexcept -> const char* = 0;
    virtual auto OpentxsConfigFilePath() const noexcept
        -> UnallocatedCString = 0;
    virtual auto Outbox() const noexcept -> const char* = 0;
    virtual auto PathExists(const String& path) const noexcept -> bool = 0;
    virtual auto PIDFilePath() const noexcept -> UnallocatedCString = 0;
    virtual auto PaymentInbox() const noexcept -> const char* = 0;
    virtual auto Receipt() const noexcept -> const char* = 0;
    virtual auto RecordBox() const noexcept -> const char* = 0;
    virtual auto ServerConfigFilePath(const int instance) const noexcept
        -> UnallocatedCString = 0;
    virtual auto ServerDataFolder(const int instance) const noexcept
        -> UnallocatedCString = 0;

    virtual ~Legacy() = default;

protected:
    Legacy() noexcept = default;

private:
    Legacy(const Legacy&) = delete;
    Legacy(Legacy&&) = delete;
    auto operator=(const Legacy&) -> Legacy& = delete;
    auto operator=(Legacy&&) -> Legacy& = delete;

    static auto internal_concatenate(
        const char* name,
        const UnallocatedCString& ext) noexcept -> UnallocatedCString;
};
}  // namespace opentxs::api
