// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/Paths.internal.hpp"  // IWYU pragma: associated

#include <utility>

#include "opentxs/api/PathsPrivate.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Options.hpp"

namespace opentxs::api::internal
{
static auto internal_concatenate(
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
            LogError()()("received negative number ")(_name);
        }
    } else {
        LogError()()("received nullptr");
    }

    return tmp;
}
}  // namespace opentxs::api::internal

namespace opentxs::api::internal
{
Paths::Paths(std::unique_ptr<PathsPrivate> imp) noexcept
    : imp_(std::move(imp))
{
    assert_false(nullptr == imp_);
}

auto Paths::Account() const noexcept -> const char* { return imp_->Account(); }

auto Paths::AppendFile(
    std::filesystem::path& out,
    const std::filesystem::path& base,
    const std::filesystem::path& file) const noexcept -> bool
{
    return imp_->AppendFile(out, base, file);
}

auto Paths::AppendFolder(
    std::filesystem::path& out,
    const std::filesystem::path& base,
    const std::filesystem::path& folder) const noexcept -> bool
{
    return imp_->AppendFolder(out, base, folder);
}

auto Paths::BlockchainCheckpoints() const noexcept -> std::filesystem::path
{
    return imp_->BlockchainCheckpoints();
}

auto Paths::BuildFilePath(const std::filesystem::path& path) const noexcept
    -> bool
{
    return imp_->BuildFilePath(path);
}

auto Paths::BuildFolderPath(const std::filesystem::path& path) const noexcept
    -> bool
{
    return imp_->BuildFolderPath(path);
}

auto Paths::ClientConfigFilePath(const int instance) const noexcept
    -> std::filesystem::path
{
    return imp_->ClientConfigFilePath(instance);
}

auto Paths::ClientDataFolder(const int instance) const noexcept
    -> std::filesystem::path
{
    return imp_->ClientDataFolder(instance);
}

auto Paths::Common() const noexcept -> const char* { return imp_->Common(); }

auto Paths::Concatenate(
    const UnallocatedCString& notary_id,
    const UnallocatedCString& path_separator,
    const UnallocatedCString& unit_id,
    const char* append) -> UnallocatedCString
{
    const UnallocatedCString app(append);
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

auto Paths::Concatenate(const UnallocatedCString& unit_id, const char* append)
    -> UnallocatedCString
{
    const UnallocatedCString app(append);
    UnallocatedCString tmp;
    tmp.reserve(unit_id.length() + app.length());
    tmp.append(unit_id);
    tmp.append(app);

    return tmp;
}

auto Paths::ConfirmCreateFolder(
    const std::filesystem::path& path) const noexcept -> bool
{
    return imp_->ConfirmCreateFolder(path);
}

auto Paths::Contract() const noexcept -> const char*
{
    return imp_->Contract();
}

auto Paths::Cron() const noexcept -> const char* { return imp_->Cron(); }

auto Paths::ExpiredBox() const noexcept -> const char*
{
    return imp_->ExpiredBox();
}

auto Paths::FileExists(const std::filesystem::path& path, std::size_t& size)
    const noexcept -> bool
{
    return imp_->FileExists(path, size);
}

auto Paths::GetFilenameA(const char* filename) noexcept -> UnallocatedCString
{
    static const UnallocatedCString ext{".a"};

    return internal_concatenate(filename, ext);
}

auto Paths::GetFilenameBin(const char* filename) noexcept -> UnallocatedCString
{
    static const UnallocatedCString ext{".bin"};

    return internal_concatenate(filename, ext);
}

auto Paths::GetFilenameCrn(TransactionNumber number) noexcept
    -> UnallocatedCString
{
    static const UnallocatedCString ext{".crn"};

    return internal_concatenate(std::to_string(number).c_str(), ext);
}

auto Paths::GetFilenameError(const char* filename) noexcept
    -> UnallocatedCString
{
    static const UnallocatedCString ext{".error"};

    return internal_concatenate(filename, ext);
}

auto Paths::GetFilenameFail(const char* filename) noexcept -> UnallocatedCString
{
    static const UnallocatedCString ext{".fail"};

    return internal_concatenate(filename, ext);
}

auto Paths::GetFilenameLst(const UnallocatedCString& filename) noexcept
    -> UnallocatedCString
{
    static const UnallocatedCString ext{".lst"};

    return internal_concatenate(filename.c_str(), ext);
}

auto Paths::GetFilenameR(const char* foldername) noexcept -> UnallocatedCString
{
    static const UnallocatedCString ext{".r"};

    return internal_concatenate(foldername, ext);
}

auto Paths::GetFilenameRct(TransactionNumber number) noexcept
    -> UnallocatedCString
{
    static const UnallocatedCString ext{".rct"};

    return internal_concatenate(std::to_string(number).c_str(), ext);
}

auto Paths::GetFilenameSuccess(const char* filename) noexcept
    -> UnallocatedCString
{
    static const UnallocatedCString ext{".success"};

    return internal_concatenate(filename, ext);
}

auto Paths::Home(const Options& args) noexcept -> std::filesystem::path
{
    return PathsPrivate::get_app_data_folder(args.Home());
}

auto Paths::Inbox() const noexcept -> const char* { return imp_->Inbox(); }

auto Paths::Init(const std::shared_ptr<const api::Crypto>& crypto) noexcept
    -> void
{
    return imp_->Init(crypto);
}

auto Paths::LedgerFileName(
    const identifier::Notary& server,
    const identifier::Account& account) const noexcept -> std::filesystem::path
{
    return imp_->LedgerFileName(server, account);
}

auto Paths::Market() const noexcept -> const char* { return imp_->Market(); }

auto Paths::Mint() const noexcept -> const char* { return imp_->Mint(); }

auto Paths::MintFileName(
    const identifier::Notary& server,
    const identifier::UnitDefinition& unit,
    std::string_view extension) const noexcept -> std::filesystem::path
{
    return imp_->MintFileName(server, unit, extension);
}

auto Paths::Nym() const noexcept -> const char* { return imp_->Nym(); }

auto Paths::Nymbox() const noexcept -> const char* { return imp_->Nymbox(); }

auto Paths::OpentxsConfigFilePath() const noexcept -> std::filesystem::path
{
    return imp_->OpentxsConfigFilePath();
}

auto Paths::Outbox() const noexcept -> const char* { return imp_->Outbox(); }

auto Paths::PIDFilePath() const noexcept -> std::filesystem::path
{
    return imp_->PIDFilePath();
}

auto Paths::PaymentInbox() const noexcept -> const char*
{
    return imp_->PaymentInbox();
}

auto Paths::Receipt() const noexcept -> const char* { return imp_->Receipt(); }

auto Paths::RecordBox() const noexcept -> const char*
{
    return imp_->RecordBox();
}

auto Paths::ServerConfigFilePath(const int instance) const noexcept
    -> std::filesystem::path
{
    return imp_->ServerConfigFilePath(instance);
}

auto Paths::ServerDataFolder(const int instance) const noexcept
    -> std::filesystem::path
{
    return imp_->ServerDataFolder(instance);
}

auto Paths::SuggestFolder(std::string_view appName) noexcept
    -> std::filesystem::path
{
    return PathsPrivate::get_home_directory() /
           PathsPrivate::get_suffix(appName);
}

Paths::~Paths() = default;
}  // namespace opentxs::api::internal
