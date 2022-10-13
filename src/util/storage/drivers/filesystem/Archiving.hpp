// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <filesystem>
#include <string_view>

#include "opentxs/util/Bytes.hpp"
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
class Plugin;
}  // namespace storage

class Flag;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::driver::filesystem
{
using namespace std::literals;

class Archiving final : public Common, public virtual storage::Driver
{

private:
    using ot_super = Common;

public:
    auto EmptyBucket(const bool bucket) const -> bool final;

    void Cleanup() final;

    Archiving(
        const api::Crypto& crypto,
        const api::network::Asio& asio,
        const api::session::Storage& storage,
        const storage::Config& config,
        const Flag& bucket,
        const UnallocatedCString& folder,
        crypto::symmetric::Key& key);
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

    auto calculate_path(std::string_view key, bool bucket, fs::path& directory)
        const noexcept -> fs::path final;
    auto prepare_read(const UnallocatedCString& ciphertext) const
        -> UnallocatedCString final;
    auto prepare_write(const UnallocatedCString& plaintext) const
        -> UnallocatedCString final;
    auto root_filename() const -> fs::path final;

    void Init_Archiving();
    void Cleanup_Archiving();
};
}  // namespace opentxs::storage::driver::filesystem
