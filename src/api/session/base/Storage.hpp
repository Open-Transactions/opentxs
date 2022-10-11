// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <filesystem>
#include <memory>

#include "internal/core/String.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Options.hpp"
#include "util/storage/Config.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace crypto
{
class Seed;
}  // namespace crypto

namespace network
{
class Asio;
}  // namespace network

namespace session
{
class Crypto;
class Endpoints;
class Factory;
class Storage;
}  // namespace session

class Crypto;
class Legacy;
class Session;
class Settings;
}  // namespace api

namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network

namespace storage
{
class Config;
}  // namespace storage

class Flag;
class Options;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::base
{
class Storage
{
public:
    Storage() = delete;
    Storage(const Storage&) = delete;
    Storage(Storage&&) = delete;
    auto operator=(const Storage&) -> Storage& = delete;
    auto operator=(Storage&&) -> Storage& = delete;

    virtual ~Storage();

protected:
    const api::Settings& config_;
    const Options args_;
    const std::filesystem::path data_folder_;
    const opentxs::storage::Config storage_config_;

private:
    std::unique_ptr<api::session::Factory> factory_p_;

protected:
    std::unique_ptr<api::session::Storage> storage_;

private:
    std::unique_ptr<Crypto> crypto_p_;

protected:
    const api::session::Factory& factory_;
    session::Crypto& crypto_;
    OTSymmetricKey storage_encryption_key_;

    virtual auto cleanup() noexcept -> void;
    auto init(
        const api::session::Factory& factory,
        const api::crypto::Seed& seeds) noexcept -> void;
    auto start() noexcept -> void;

    Storage(
        const Flag& running,
        Options&& args,
        const api::Session& session,
        const api::session::Endpoints& endpoints,
        const api::Crypto& crypto,
        const api::Settings& config,
        const api::Legacy& legacy,
        const api::network::Asio& asio,
        const opentxs::network::zeromq::Context& zmq,
        const std::filesystem::path& dataFolder,
        std::unique_ptr<api::session::Factory> factory);
};
}  // namespace opentxs::api::session::base
