// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#if OT_STORAGE_FS
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/Proto.hpp"

#include "storage/Plugin.hpp"
#include "storage/StorageConfig.hpp"
#include "StorageFS.hpp"

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>

#include <cstdio>
#include <cstdint>
#include <fstream>
#include <ios>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#if defined(__APPLE__)
extern "C" {
#include <fcntl.h>
}
#endif

extern "C" {
#include <unistd.h>
}

#include "StorageFSArchive.hpp"

#define ROOT_FILE_EXTENSION ".hash"

#define OT_METHOD "opentxs::StorageFSArchive::"

namespace opentxs
{
opentxs::api::storage::Plugin* Factory::StorageFSArchive(
    const api::storage::Storage& storage,
    const StorageConfig& config,
    const Digest& hash,
    const Random& random,
    const Flag& bucket,
    const std::string& folder,
    crypto::key::Symmetric& key)
{
    return new opentxs::storage::implementation::StorageFSArchive(
        storage, config, hash, random, bucket, folder, key);
}
}  // namespace opentxs

namespace opentxs::storage::implementation
{
StorageFSArchive::StorageFSArchive(
    const api::storage::Storage& storage,
    const StorageConfig& config,
    const Digest& hash,
    const Random& random,
    const Flag& bucket,
    const std::string& folder,
    crypto::key::Symmetric& key)
    : ot_super(storage, config, hash, random, folder, bucket)
    , encryption_key_(key)
    , encrypted_(bool(encryption_key_))
{
    Init_StorageFSArchive();
}

std::string StorageFSArchive::calculate_path(
    const std::string& key,
    const bool,
    std::string& directory) const
{
    directory = folder_;
    auto& level1 = folder_;
    std::string level2{};

    if (4 < key.size()) {
        directory += path_seperator_;
        directory += key.substr(0, 4);
        level2 = directory;
    }

    if (8 < key.size()) {
        directory += path_seperator_;
        directory += key.substr(4, 4);
    }

    boost::system::error_code ec{};
    boost::filesystem::create_directories(directory, ec);

    if (8 < key.size()) {
        if (false == sync(level2)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to sync directory ")(
                level2)(".")
                .Flush();
        }
    }

    if (false == sync(level1)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to sync directory ")(
            level1)(".")
            .Flush();
    }

    return {directory + path_seperator_ + key};
}

void StorageFSArchive::Cleanup()
{
    Cleanup_StorageFSArchive();
    ot_super::Cleanup();
}

void StorageFSArchive::Cleanup_StorageFSArchive()
{
    // future cleanup actions go here
}

bool StorageFSArchive::EmptyBucket(const bool) const { return true; }

void StorageFSArchive::Init_StorageFSArchive()
{
    OT_ASSERT(false == folder_.empty());

    boost::system::error_code ec{};

    if (boost::filesystem::create_directory(folder_, ec)) { ready_->On(); }
}

std::string StorageFSArchive::prepare_read(const std::string& input) const
{
    if (false == encrypted_) { return input; }

    const auto ciphertext = proto::TextToProto<proto::Ciphertext>(input);

    OT_ASSERT(encryption_key_);

    std::string output{};
    OTPasswordData reason("");

    if (false == encryption_key_.Decrypt(ciphertext, reason, output)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decrypt value.")
            .Flush();
    }

    return output;
}

std::string StorageFSArchive::prepare_write(const std::string& plaintext) const
{
    if (false == encrypted_) { return plaintext; }

    OT_ASSERT(encryption_key_);

    proto::Ciphertext ciphertext{};
    auto iv = Data::Factory();
    OTPasswordData reason("");
    const bool encrypted =
        encryption_key_.Encrypt(plaintext, iv, reason, ciphertext, false);

    if (false == encrypted) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to encrypt value.")
            .Flush();
    }

    return proto::ProtoAsString(ciphertext);
}

std::string StorageFSArchive::root_filename() const
{
    return folder_ + path_seperator_ + config_.fs_root_file_ +
           ROOT_FILE_EXTENSION;
}

StorageFSArchive::~StorageFSArchive() { Cleanup_StorageFSArchive(); }
}  // namespace opentxs::storage::implementation
#endif
