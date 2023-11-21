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
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace internal
{
class Paths;  // IWYU pragma: keep
class PathsPrivate;
}  // namespace internal

class Crypto;
}  // namespace api

namespace identifier
{
class Account;
class Notary;
class UnitDefinition;
}  // namespace identifier

class Options;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class opentxs::api::internal::Paths
{
public:
    static auto Concatenate(
        const UnallocatedCString& notary_id,
        const UnallocatedCString& path_separator,
        const UnallocatedCString& unit_id,
        const char* append = "") -> UnallocatedCString;
    static auto Concatenate(
        const UnallocatedCString& unit_id,
        const char* append) -> UnallocatedCString;
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
    static auto Home(const Options& args) noexcept -> std::filesystem::path;
    static auto SuggestFolder(std::string_view appName) noexcept
        -> std::filesystem::path;

    auto Account() const noexcept -> const char*;
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
    auto Common() const noexcept -> const char*;
    auto ConfirmCreateFolder(const std::filesystem::path& path) const noexcept
        -> bool;
    auto Contract() const noexcept -> const char*;
    auto Cron() const noexcept -> const char*;
    auto ExpiredBox() const noexcept -> const char*;
    auto FileExists(const std::filesystem::path& path, std::size_t& size)
        const noexcept -> bool;
    auto Inbox() const noexcept -> const char*;
    auto LedgerFileName(
        const identifier::Notary& server,
        const identifier::Account& account) const noexcept
        -> std::filesystem::path;
    auto Market() const noexcept -> const char*;
    auto Mint() const noexcept -> const char*;
    auto MintFileName(
        const identifier::Notary& server,
        const identifier::UnitDefinition& unit,
        std::string_view extension = {}) const noexcept
        -> std::filesystem::path;
    auto Nym() const noexcept -> const char*;
    auto Nymbox() const noexcept -> const char*;
    auto OpentxsConfigFilePath() const noexcept -> std::filesystem::path;
    auto Outbox() const noexcept -> const char*;
    auto PIDFilePath() const noexcept -> std::filesystem::path;
    auto PaymentInbox() const noexcept -> const char*;
    auto Receipt() const noexcept -> const char*;
    auto RecordBox() const noexcept -> const char*;
    auto ServerConfigFilePath(const int instance) const noexcept
        -> std::filesystem::path;
    auto ServerDataFolder(const int instance) const noexcept
        -> std::filesystem::path;

    auto Init(const std::shared_ptr<const api::Crypto>& crypto) noexcept
        -> void;

    Paths(std::unique_ptr<PathsPrivate> imp) noexcept;
    Paths() = delete;
    Paths(const Paths&) = delete;
    Paths(Paths&&) = delete;
    auto operator=(const Paths&) -> Paths& = delete;
    auto operator=(Paths&&) -> Paths& = delete;

    ~Paths();

private:
    std::unique_ptr<PathsPrivate> imp_;
};
