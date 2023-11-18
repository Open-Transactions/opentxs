// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace internal
{
class Session;
}  // namespace internal

namespace network
{
class Asio;
class Blockchain;
class Network;
class OTDHT;
}  // namespace network

namespace session
{
class Client;
class Endpoints;
}  // namespace session
}  // namespace api

namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
auto AsioAPI(const network::zeromq::Context& zmq, bool test) noexcept
    -> std::unique_ptr<api::network::Asio>;
auto BlockchainNetworkAPI(
    const api::session::Client& api,
    const api::session::Endpoints& endpoints,
    const opentxs::network::zeromq::Context& zmq) noexcept
    -> std::unique_ptr<api::network::Blockchain>;
auto BlockchainNetworkAPINull() noexcept
    -> std::unique_ptr<api::network::Blockchain>;
auto NetworkAPI(
    const api::internal::Session& api,
    const api::network::Asio& asio,
    const network::zeromq::Context& zmq,
    const api::session::Endpoints& endpoints,
    std::unique_ptr<api::network::Blockchain> blockchain) noexcept
    -> std::unique_ptr<api::network::Network>;
auto OTDHT(
    const api::internal::Session& api,
    const network::zeromq::Context& zmq,
    const api::session::Endpoints& endpoints,
    const api::network::Blockchain& blockchain) noexcept
    -> std::unique_ptr<api::network::OTDHT>;
}  // namespace opentxs::factory
