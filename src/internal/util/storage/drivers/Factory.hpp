// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "opentxs/util/Container.hpp"  // IWYU pragma: keep

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace network
{
class Asio;
}  // namespace network

namespace session
{
class Factory;
class Storage;
}  // namespace session

class Crypto;
}  // namespace api

namespace crypto
{
namespace key
{
class Symmetric;
}  // namespace key
}  // namespace crypto

namespace storage
{
namespace driver
{
namespace internal
{
class Multiplex;
}  // namespace internal
}  // namespace driver

class Config;
class Plugin;
}  // namespace storage

class Flag;
class String;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
auto StorageFSArchive(
    const api::Crypto& crypto,
    const api::network::Asio& asio,
    const api::session::Storage& parent,
    const storage::Config& config,
    const Flag& bucket,
    const UnallocatedCString& folder,
    crypto::key::Symmetric& key) noexcept -> std::unique_ptr<storage::Plugin>;
auto StorageFSGC(
    const api::Crypto& crypto,
    const api::network::Asio& asio,
    const api::session::Storage& parent,
    const storage::Config& config,
    const Flag& bucket) noexcept -> std::unique_ptr<storage::Plugin>;
auto StorageMemDB(
    const api::Crypto& crypto,
    const api::network::Asio& asio,
    const api::session::Storage& parent,
    const storage::Config& config,
    const Flag& bucket) noexcept -> std::unique_ptr<storage::Plugin>;
auto StorageLMDB(
    const api::Crypto& crypto,
    const api::network::Asio& asio,
    const api::session::Storage& parent,
    const storage::Config& config,
    const Flag& bucket) noexcept -> std::unique_ptr<storage::Plugin>;
auto StorageMultiplex(
    const api::Crypto& crypto,
    const api::network::Asio& asio,
    const api::session::Factory& factory,
    const api::session::Storage& parent,
    const Flag& primaryBucket,
    const storage::Config& config) noexcept
    -> std::unique_ptr<storage::driver::internal::Multiplex>;
auto StorageSqlite3(
    const api::Crypto& crypto,
    const api::network::Asio& asio,
    const api::session::Storage& parent,
    const storage::Config& config,
    const Flag& bucket) noexcept -> std::unique_ptr<storage::Plugin>;
}  // namespace opentxs::factory
