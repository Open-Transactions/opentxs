// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::storage::Driver

#include "util/storage/drivers/filesystem/Archiving.hpp"  // IWYU pragma: associated

#include <Ciphertext.pb.h>
#include <memory>
#include <stdexcept>
#include <system_error>
#include <utility>

#include "internal/crypto/symmetric/Key.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/storage/drivers/Factory.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/crypto/symmetric/Key.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/PasswordPrompt.hpp"  // IWYU pragma: keep
#include "opentxs/util/Writer.hpp"
#include "opentxs/util/storage/Driver.hpp"
#include "util/storage/Config.hpp"

namespace opentxs::factory
{
auto StorageFSArchive(
    const api::Crypto& crypto,
    const storage::Config& config,
    const std::filesystem::path& folder,
    crypto::symmetric::Key& key) noexcept -> std::unique_ptr<storage::Driver>
{
    using ReturnType = storage::driver::filesystem::Archiving;

    try {

        return std::make_unique<ReturnType>(crypto, config, folder, key);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory

namespace opentxs::storage::driver::filesystem
{
using namespace std::literals;

Archiving::Archiving(
    const api::Crypto& crypto,
    const storage::Config& config,
    const std::filesystem::path& folder,
    crypto::symmetric::Key& key) noexcept(false)
    : Common(crypto, config, folder)
    , encryption_key_(key)
    , encrypted_(bool(encryption_key_))
{
    init();
}

auto Archiving::calculate_path(
    const Data& data,
    std::string_view key,
    Bucket bucket,
    fs::path& directory) const noexcept -> fs::path
{
    directory = data.folder_;
    const auto& level1 = data.folder_;
    UnallocatedCString level2{};

    if (4_uz < key.size()) {
        directory.append(key.substr(0, 4));
        level2 = directory.string();
    }

    if (8_uz < key.size()) { directory.append(key.substr(4, 4)); }

    auto ec = std::error_code{};
    fs::create_directories(directory, ec);

    if (8_uz < key.size()) {
        if (false == sync(level2)) {
            LogError()(OT_PRETTY_CLASS())("Unable to sync directory ")(
                level2)(".")
                .Flush();
        }
    }

    if (false == sync(level1)) {
        LogError()(OT_PRETTY_CLASS())("Unable to sync directory ")(level1)
            .Flush();
    }

    return fs::path{directory} / key;
}

auto Archiving::Description() const noexcept -> std::string_view
{
    return "archiving flat file"sv;
}

auto Archiving::do_write(
    const fs::path& directory,
    const fs::path& filename,
    File& file,
    ReadView data) const noexcept(false) -> void
{
    if (encrypted_) {
        auto ciphertext = proto::Ciphertext{};
        auto reason = encryption_key_.Internal().API().Factory().PasswordPrompt(
            "Storage write");
        const auto encrypt =
            encryption_key_.Internal().Encrypt(data, ciphertext, reason, false);

        if (false == encrypt) {
            throw std::runtime_error{"encryption failure"};
        }

        finalize_write(directory, filename, file, proto::ToString(ciphertext));
    } else {
        finalize_write(directory, filename, file, data);
    }
}

auto Archiving::empty_bucket(const Data&, Bucket) const noexcept(false) -> bool
{
    return true;
}

auto Archiving::init(Data& data) noexcept(false) -> void
{
    Common::init(data);
    fs::create_directory(data.folder_);
}

auto Archiving::finalize_read(UnallocatedCString&& input) const noexcept(false)
    -> UnallocatedCString
{
    if (false == encrypted_) { return std::move(input); }

    const auto ciphertext = proto::Factory<proto::Ciphertext>(input);

    auto output = ""s;
    auto reason = encryption_key_.Internal().API().Factory().PasswordPrompt(
        "Storage read");
    const auto decrypt =
        encryption_key_.Internal().Decrypt(ciphertext, writer(output), reason);

    if (false == decrypt) { throw std::runtime_error{"decryption failure"}; }

    return output;
}

auto Archiving::root_filename(const Data& data) const noexcept -> fs::path
{
    return (data.folder_ / config_.fs_root_file_)
        .replace_extension(root_file_extension_);
}

Archiving::~Archiving() = default;
}  // namespace opentxs::storage::driver::filesystem
