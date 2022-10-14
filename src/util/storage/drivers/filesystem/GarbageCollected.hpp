// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <filesystem>
#include <string_view>

#include "opentxs/util/Container.hpp"
#include "opentxs/util/storage/Driver.hpp"
#include "util/storage/drivers/filesystem/Common.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace network
{
class Asio;
}  // namespace network

namespace session
{
class Storage;
}  // namespace session

class Crypto;
}  // namespace api

namespace storage
{
class Config;
class Plugin;
}  // namespace storage

class Flag;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::driver::filesystem
{
// Simple filesystem implementation of opentxs::storage
class GarbageCollected final : public Common, public virtual storage::Driver
{
private:
    using ot_super = Common;

public:
    auto EmptyBucket(const bool bucket) const -> bool final;

    void Cleanup() final;

    GarbageCollected(
        const api::Crypto& crypto,
        const api::network::Asio& asio,
        const api::session::Storage& storage,
        const storage::Config& config,
        const Flag& bucket);
    GarbageCollected() = delete;
    GarbageCollected(const GarbageCollected&) = delete;
    GarbageCollected(GarbageCollected&&) = delete;
    auto operator=(const GarbageCollected&) -> GarbageCollected& = delete;
    auto operator=(GarbageCollected&&) -> GarbageCollected& = delete;

    ~GarbageCollected() final;

private:
    auto bucket_name(const bool bucket) const noexcept -> fs::path;
    auto calculate_path(std::string_view key, bool bucket, fs::path& directory)
        const noexcept -> fs::path final;
    void purge(const UnallocatedCString& path) const;
    auto root_filename() const -> fs::path final;

    void Cleanup_GarbageCollected();
    void Init_GarbageCollected();
};
}  // namespace opentxs::storage::driver::filesystem
