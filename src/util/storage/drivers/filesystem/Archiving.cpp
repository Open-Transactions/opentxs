// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "util/storage/drivers/filesystem/Archiving.hpp"  // IWYU pragma: associated

#include <Ciphertext.pb.h>
#include <memory>
#include <system_error>

#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/util/Flag.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/storage/drivers/Factory.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/PasswordPrompt.hpp"  // IWYU pragma: keep
#include "util/storage/Config.hpp"

namespace opentxs::factory
{
auto StorageFSArchive(
    const api::Crypto& crypto,
    const api::network::Asio& asio,
    const api::session::Storage& parent,
    const storage::Config& config,
    const Flag& bucket,
    const UnallocatedCString& folder,
    crypto::key::Symmetric& key) noexcept -> std::unique_ptr<storage::Plugin>
{
    using ReturnType = storage::driver::filesystem::Archiving;

    return std::make_unique<ReturnType>(
        crypto, asio, parent, config, bucket, folder, key);
}
}  // namespace opentxs::factory

namespace opentxs::storage::driver::filesystem
{
Archiving::Archiving(
    const api::Crypto& crypto,
    const api::network::Asio& asio,
    const api::session::Storage& storage,
    const storage::Config& config,
    const Flag& bucket,
    const UnallocatedCString& folder,
    crypto::key::Symmetric& key)
    : ot_super(crypto, asio, storage, config, folder, bucket)
    , encryption_key_(key)
    , encrypted_(bool(encryption_key_))
{
    Init_Archiving();
}

auto Archiving::calculate_path(
    std::string_view key,
    bool bucket,
    fs::path& directory) const noexcept -> fs::path
{
    directory = folder_;
    const auto& level1 = folder_;
    UnallocatedCString level2{};

    if (4 < key.size()) {
        directory / key.substr(0, 4);
        level2 = directory.string();
    }

    if (8 < key.size()) { directory / key.substr(4, 4); }

    auto ec = std::error_code{};
    fs::create_directories(directory, ec);

    if (8 < key.size()) {
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

void Archiving::Cleanup()
{
    Cleanup_Archiving();
    ot_super::Cleanup();
}

void Archiving::Cleanup_Archiving()
{
    // future cleanup actions go here
}

auto Archiving::EmptyBucket(const bool) const -> bool { return true; }

void Archiving::Init_Archiving()
{
    OT_ASSERT(false == folder_.empty());

    auto ec = std::error_code{};

    if (fs::create_directory(folder_, ec)) { ready_->On(); }
}

auto Archiving::prepare_read(const UnallocatedCString& input) const
    -> UnallocatedCString
{
    if (false == encrypted_) { return input; }

    const auto ciphertext = proto::Factory<proto::Ciphertext>(input);

    OT_ASSERT(encryption_key_);

    UnallocatedCString output{};
    auto reason =
        encryption_key_.api().Factory().PasswordPrompt("Storage read");

    if (false == encryption_key_.Decrypt(ciphertext, reason, writer(output))) {
        LogError()(OT_PRETTY_CLASS())("Failed to decrypt value.").Flush();
    }

    return output;
}

auto Archiving::prepare_write(const UnallocatedCString& plaintext) const
    -> UnallocatedCString
{
    if (false == encrypted_) { return plaintext; }

    OT_ASSERT(encryption_key_);

    proto::Ciphertext ciphertext{};
    auto reason =
        encryption_key_.api().Factory().PasswordPrompt("Storage write");
    const bool encrypted =
        encryption_key_.Encrypt(plaintext, reason, ciphertext, false);

    if (false == encrypted) {
        LogError()(OT_PRETTY_CLASS())("Failed to encrypt value.").Flush();
    }

    return proto::ToString(ciphertext);
}

auto Archiving::root_filename() const -> fs::path
{
    return (folder_ / config_.fs_root_file_)
        .replace_extension(root_file_extension_);
}

Archiving::~Archiving() { Cleanup_Archiving(); }
}  // namespace opentxs::storage::driver::filesystem
