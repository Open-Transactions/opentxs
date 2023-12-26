// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <filesystem>
#include <string_view>

#include "opentxs/Types.hpp"
#include "opentxs/storage/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "util/storage/drivers/filesystem/Common.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
}  // namespace api

namespace crypto
{
namespace symmetric
{
class Key;
}  // namespace symmetric
}  // namespace crypto

namespace storage
{
class Config;
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::driver::filesystem
{
using namespace std::literals;

class Archiving final : public Common
{
public:
    auto Description() const noexcept -> std::string_view final;

    Archiving(
        const api::Crypto& crypto,
        const storage::Config& config,
        const std::filesystem::path& folder,
        crypto::symmetric::Key& key) noexcept(false);
    Archiving() = delete;
    Archiving(const Archiving&) = delete;
    Archiving(Archiving&&) = delete;
    auto operator=(const Archiving&) -> Archiving& = delete;
    auto operator=(Archiving&&) -> Archiving& = delete;

    ~Archiving() final;

private:
    static constexpr auto root_file_extension_ = "hash"sv;

    crypto::symmetric::Key& encryption_key_;
    const bool encrypted_;

    auto calculate_path(
        const Data& data,
        std::string_view key,
        Bucket bucket,
        fs::path& directory) const noexcept -> fs::path final;
    auto do_write(
        const fs::path& directory,
        const fs::path& filename,
        File& file,
        ReadView data) const noexcept(false) -> void final;
    auto empty_bucket(const Data& data, Bucket bucket) const noexcept(false)
        -> bool final;
    auto finalize_read(UnallocatedCString&& ciphertext) const noexcept(false)
        -> UnallocatedCString final;
    auto root_filename(const Data& data) const noexcept -> fs::path final;

    using Common::init;
    auto init(Data& data) noexcept(false) -> void final;
};
}  // namespace opentxs::storage::driver::filesystem
