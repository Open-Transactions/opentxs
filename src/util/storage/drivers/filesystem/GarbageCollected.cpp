// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::storage::Plugin

#include "util/storage/drivers/filesystem/GarbageCollected.hpp"  // IWYU pragma: associated

#include <cstdio>
#include <memory>

#include "internal/api/crypto/Encode.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/storage/drivers/Factory.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/util/storage/Plugin.hpp"
#include "util/storage/Config.hpp"

namespace opentxs::factory
{
auto StorageFSGC(
    const api::Crypto& crypto,
    const api::network::Asio& asio,
    const api::session::Storage& parent,
    const storage::Config& config,
    const Flag& bucket) noexcept -> std::unique_ptr<storage::Plugin>
{
    using ReturnType = storage::driver::filesystem::GarbageCollected;

    return std::make_unique<ReturnType>(crypto, asio, parent, config, bucket);
}
}  // namespace opentxs::factory

namespace opentxs::storage::driver::filesystem
{
GarbageCollected::GarbageCollected(
    const api::Crypto& crypto,
    const api::network::Asio& asio,
    const api::session::Storage& storage,
    const storage::Config& config,
    const Flag& bucket)
    : ot_super(crypto, asio, storage, config, config.path_.string(), bucket)
{
    Init_GarbageCollected();
}

auto GarbageCollected::bucket_name(const bool bucket) const noexcept -> fs::path
{
    return bucket ? config_.fs_secondary_bucket_ : config_.fs_primary_bucket_;
}

auto GarbageCollected::calculate_path(
    std::string_view key,
    bool bucket,
    fs::path& directory) const noexcept -> fs::path
{
    directory = folder_ / bucket_name(bucket);

    return directory / key;
}

void GarbageCollected::Cleanup()
{
    Cleanup_GarbageCollected();
    ot_super::Cleanup();
}

void GarbageCollected::Cleanup_GarbageCollected()
{
    // future cleanup actions go here
}

auto GarbageCollected::EmptyBucket(const bool bucket) const -> bool
{
    const auto oldDirectory = [&] {
        auto out = fs::path{};
        calculate_path("", bucket, out);

        return out;
    }();
    const auto random = crypto_.Encode().InternalEncode().RandomFilename();
    const auto newName = folder_ / random;

    if (0 !=
        std::rename(oldDirectory.string().c_str(), newName.string().c_str())) {
        return false;
    }

    asio_.Internal().Post(
        ThreadPool::General,
        [=, this] { purge(newName.string()); },
        "GarbageCollected");

    return fs::create_directory(oldDirectory);
}

void GarbageCollected::Init_GarbageCollected()
{
    fs::create_directory(folder_ / config_.fs_primary_bucket_);
    fs::create_directory(folder_ / config_.fs_secondary_bucket_);
    ready_->On();
}

void GarbageCollected::purge(const UnallocatedCString& path) const
{
    if (path.empty()) { return; }

    fs::remove_all(path);
}

auto GarbageCollected::root_filename() const -> fs::path
{
    OT_ASSERT(false == folder_.empty());
    OT_ASSERT(false == config_.fs_root_file_.empty());

    return folder_ / config_.fs_root_file_;
}

GarbageCollected::~GarbageCollected() { Cleanup_GarbageCollected(); }
}  // namespace opentxs::storage::driver::filesystem
