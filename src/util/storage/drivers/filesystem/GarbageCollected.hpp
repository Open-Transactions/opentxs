// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <filesystem>
#include <string_view>

#include "opentxs/storage/Types.hpp"
#include "util/storage/drivers/filesystem/Common.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
}  // namespace api

namespace storage
{
class Config;
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::driver::filesystem
{
// Simple filesystem implementation of opentxs::storage
class GarbageCollected final : public Common
{
public:
    auto Description() const noexcept -> std::string_view final;

    GarbageCollected(
        const api::Crypto& crypto,
        const storage::Config& config) noexcept(false);
    GarbageCollected() = delete;
    GarbageCollected(const GarbageCollected&) = delete;
    GarbageCollected(GarbageCollected&&) = delete;
    auto operator=(const GarbageCollected&) -> GarbageCollected& = delete;
    auto operator=(GarbageCollected&&) -> GarbageCollected& = delete;

    ~GarbageCollected() final;

private:
    static auto purge(const fs::path& path) noexcept -> void;

    auto bucket_name(Bucket bucket) const noexcept -> fs::path;
    auto calculate_path(
        const Data& data,
        std::string_view key,
        Bucket bucket,
        fs::path& directory) const noexcept -> fs::path final;
    auto empty_bucket(const Data& data, Bucket bucket) const noexcept(false)
        -> bool final;
    auto root_filename(const Data& data) const noexcept -> fs::path final;

    using Common::init;
    auto init(Data& data) noexcept(false) -> void final;
};
}  // namespace opentxs::storage::driver::filesystem
