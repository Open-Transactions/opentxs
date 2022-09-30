// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <filesystem>
#include <memory>

#include "internal/util/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto

namespace network
{
class Asio;
}  // namespace network

namespace session
{
class Activity;
class Client;
class Contacts;
class Crypto;
class Endpoints;
class Factory;
class Notary;
class OTX;
class Storage;
class UI;
class Wallet;
class Workflow;
}  // namespace session

class Context;
class Crypto;
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

namespace opentxs::factory
{
auto ActivityAPI(
    const api::session::Client& api,
    const api::session::Contacts& contact) noexcept
    -> std::unique_ptr<api::session::Activity>;
auto ClientSession(
    const api::Context& parent,
    Flag& running,
    Options&& args,
    const api::Settings& config,
    const api::Crypto& crypto,
    const network::zeromq::Context& context,
    const std::filesystem::path& dataFolder,
    const int instance) noexcept -> std::shared_ptr<api::session::Client>;
auto ContactAPI(const api::session::Client& api) noexcept
    -> std::unique_ptr<api::session::Contacts>;
auto EndpointsAPI(const int instance) noexcept
    -> std::unique_ptr<api::session::Endpoints>;
auto NotarySession(
    const api::Context& parent,
    Flag& running,
    Options&& args,
    const api::Crypto& crypto,
    const api::Settings& config,
    const network::zeromq::Context& context,
    const std::filesystem::path& dataFolder,
    const int instance) -> std::shared_ptr<api::session::Notary>;
auto OTX(
    const Flag& running,
    const api::session::Client& api,
    ContextLockCallback lockCallback) noexcept
    -> std::unique_ptr<api::session::OTX>;
auto SessionCryptoAPI(
    api::Crypto& parent,
    const api::Session& session,
    const api::session::Endpoints& endpoints,
    const api::session::Factory& factory,
    const api::session::Storage& storage,
    const network::zeromq::Context& zmq) noexcept
    -> std::unique_ptr<api::session::Crypto>;
auto SessionFactoryAPI(const api::session::Client& parent) noexcept
    -> std::unique_ptr<api::session::Factory>;
auto SessionFactoryAPI(const api::session::Notary& parent) noexcept
    -> std::unique_ptr<api::session::Factory>;
auto StorageAPI(
    const api::Crypto& crypto,
    const api::network::Asio& asio,
    const api::session::Factory& factory,
    const Flag& running,
    const opentxs::storage::Config& config) noexcept
    -> std::unique_ptr<api::session::Storage>;
auto UI(
    const api::session::Client& api,
    const api::crypto::Blockchain& blockchain,
    const Flag& running) noexcept -> std::unique_ptr<api::session::UI>;
auto WalletAPI(const api::session::Client& parent) noexcept
    -> std::unique_ptr<api::session::Wallet>;
auto WalletAPI(const api::session::Notary& parent) noexcept
    -> std::unique_ptr<api::session::Wallet>;
auto Workflow(
    const api::Session& api,
    const api::session::Activity& activity,
    const api::session::Contacts& contact) noexcept
    -> std::unique_ptr<api::session::Workflow>;
}  // namespace opentxs::factory
