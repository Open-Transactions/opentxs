// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <filesystem>
#include <memory>
#include <string_view>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace internal
{
class Context;
class Factory;
class Log;
class Network;
class Paths;
class Session;
class Settings;
}  // namespace internal

namespace network
{
class Asio;
class Blockchain;
class ZAP;
}  // namespace network

namespace session
{
class Endpoints;
}  // namespace session

class Crypto;
}  // namespace api

namespace internal
{
class ShutdownSender;
}  // namespace internal

namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network

class Options;
class PasswordCaller;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
auto Context(
    const network::zeromq::Context& zmq,
    const api::network::Asio& asio,
    const internal::ShutdownSender& sender,
    const Options& args,
    PasswordCaller* externalPasswordCallback) noexcept
    -> std::shared_ptr<api::internal::Context>;
auto FactoryAPI(const api::Crypto& crypto) noexcept
    -> std::shared_ptr<api::internal::Factory>;
auto Log(
    const network::zeromq::Context& zmq,
    std::string_view endpoint) noexcept -> api::internal::Log;
auto NetworkAPI(
    const api::internal::Session& api,
    const api::network::Asio& asio,
    const network::zeromq::Context& zmq,
    const api::network::ZAP& zap,
    const api::session::Endpoints& endpoints,
    std::unique_ptr<api::network::Blockchain> blockchain) noexcept
    -> api::internal::Network*;
auto Paths(const std::filesystem::path& home) noexcept -> api::internal::Paths;
auto Settings(
    const api::internal::Paths& paths,
    const std::filesystem::path& path) noexcept -> api::internal::Settings*;
}  // namespace opentxs::factory
