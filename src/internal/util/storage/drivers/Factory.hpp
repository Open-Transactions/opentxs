// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <filesystem>
#include <memory>

#include "opentxs/storage/Types.hpp"
#include "opentxs/util/Container.hpp"  // IWYU pragma: keep

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Factory;
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
namespace driver
{
class Plugin;
}  // namespace driver

class Config;
class Driver;
}  // namespace storage

class String;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
auto StorageFSArchive(
    const api::Crypto& crypto,
    const storage::Config& config,
    const std::filesystem::path& folder,
    crypto::symmetric::Key& key) noexcept -> std::unique_ptr<storage::Driver>;
auto StorageFSGC(
    const api::Crypto& crypto,
    const storage::Config& config) noexcept -> std::unique_ptr<storage::Driver>;
auto StorageMemDB(
    const api::Crypto& crypto,
    const storage::Config& config) noexcept -> std::unique_ptr<storage::Driver>;
auto StorageLMDB(
    const api::Crypto& crypto,
    const storage::Config& config) noexcept -> std::unique_ptr<storage::Driver>;
auto StoragePlugin(
    const api::Crypto& crypto,
    const api::session::Factory& factory,
    const std::atomic<storage::Bucket>& primaryBucket,
    const storage::Config& config) noexcept
    -> std::shared_ptr<storage::driver::Plugin>;
auto StorageSqlite3(
    const api::Crypto& crypto,
    const storage::Config& config) noexcept -> std::unique_ptr<storage::Driver>;
}  // namespace opentxs::factory
