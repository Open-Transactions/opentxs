// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::storage::Driver

#include "util/storage/drivers/filesystem/GarbageCollected.hpp"  // IWYU pragma: associated

#include <cstdio>
#include <exception>
#include <memory>

#include "internal/api/crypto/Encode.hpp"
#include "internal/util/storage/drivers/Factory.hpp"
#include "opentxs/Context.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/storage/Types.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/storage/Driver.hpp"
#include "util/storage/Config.hpp"

namespace opentxs::factory
{
auto StorageFSGC(
    const api::Crypto& crypto,
    const storage::Config& config) noexcept -> std::unique_ptr<storage::Driver>
{
    using ReturnType = storage::driver::filesystem::GarbageCollected;

    try {

        return std::make_unique<ReturnType>(crypto, config);
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory

namespace opentxs::storage::driver::filesystem
{
using namespace std::literals;

GarbageCollected::GarbageCollected(
    const api::Crypto& crypto,
    const storage::Config& config) noexcept(false)
    : Common(crypto, config, config.path_)
{
    init();
}

auto GarbageCollected::bucket_name(Bucket bucket) const noexcept -> fs::path
{
    using enum Bucket;

    switch (bucket) {
        case left: {

            return config_.fs_primary_bucket_;
        }
        case right: {

            return config_.fs_secondary_bucket_;
        }
        default: {

            LogAbort()().Abort();
        }
    }
}

auto GarbageCollected::calculate_path(
    const Data& data,
    std::string_view key,
    Bucket bucket,
    fs::path& directory) const noexcept -> fs::path
{
    directory = data.folder_ / bucket_name(bucket);

    return directory / key;
}

auto GarbageCollected::Description() const noexcept -> std::string_view
{
    return "flat file"sv;
}

auto GarbageCollected::empty_bucket(const Data& data, Bucket bucket) const
    noexcept(false) -> bool
{
    const auto oldDirectory = [&] {
        auto out = fs::path{};
        calculate_path(data, "", bucket, out);

        return out;
    }();
    const auto random = crypto_.Encode().InternalEncode().RandomFilename();
    const auto newName = data.folder_ / random;

    if (0 !=
        std::rename(oldDirectory.string().c_str(), newName.string().c_str())) {
        return false;
    }

    RunJob([path = newName] { purge(path); });

    return fs::create_directory(oldDirectory);
}

auto GarbageCollected::init(Data& data) noexcept(false) -> void
{
    Common::init(data);
    fs::create_directory(data.folder_ / config_.fs_primary_bucket_);
    fs::create_directory(data.folder_ / config_.fs_secondary_bucket_);
}

auto GarbageCollected::purge(const fs::path& path) noexcept -> void
{
    if (path.empty()) { return; }

    fs::remove_all(path);
}

auto GarbageCollected::root_filename(const Data& data) const noexcept
    -> fs::path
{
    return data.folder_ / config_.fs_root_file_;
}

GarbageCollected::~GarbageCollected() = default;
}  // namespace opentxs::storage::driver::filesystem
