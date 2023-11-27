// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <filesystem>
#include <memory>

#include "opentxs/api/Network.hpp"
#include "opentxs/api/Network.internal.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/OTDHT.hpp"
#include "opentxs/api/network/ZeroMQ.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto

namespace internal
{
class Paths;
class Session;
}  // namespace internal

namespace network
{
class Asio;
class ZAP;
}  // namespace network

namespace session
{
namespace internal
{
class Client;
class Notary;
}  // namespace internal

class Endpoints;
}  // namespace session

class NetworkPrivate;  // IWYU pragma: keep
}  // namespace api

namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network

class Options;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api
{
class NetworkPrivate final : public api::internal::Network
{
public:
    auto Asio() const noexcept -> const network::Asio& final { return asio_; }
    auto Blockchain() const noexcept -> const network::Blockchain& final
    {
        return *blockchain_;
    }
    auto OTDHT() const noexcept -> const network::OTDHT& final
    {
        return *otdht_;
    }
    auto ZeroMQ() const noexcept -> const network::ZeroMQ& final
    {
        return zmq_;
    }

    auto Shutdown() noexcept -> void final;
    auto Start(
        std::shared_ptr<const api::session::internal::Client> api,
        const api::crypto::Blockchain& crypto,
        const api::internal::Paths& legacy,
        const std::filesystem::path& dataFolder,
        const Options& args) noexcept -> void final;
    auto Start(
        std::shared_ptr<const api::session::internal::Notary> api,
        const api::crypto::Blockchain& crypto,
        const api::internal::Paths& legacy,
        const std::filesystem::path& dataFolder,
        const Options& args) noexcept -> void final;

    NetworkPrivate(
        const api::internal::Session& api,
        const network::Asio& asio,
        const opentxs::network::zeromq::Context& zmq,
        const network::ZAP& zap,
        const api::session::Endpoints& endpoints,
        std::unique_ptr<api::network::Blockchain> blockchain) noexcept;
    NetworkPrivate() = delete;
    NetworkPrivate(const NetworkPrivate&) = delete;
    NetworkPrivate(NetworkPrivate&&) = delete;
    auto operator=(const NetworkPrivate&) -> NetworkPrivate& = delete;
    auto operator=(NetworkPrivate&&) -> NetworkPrivate& = delete;

    ~NetworkPrivate() final;

private:
    const network::Asio& asio_;
    network::ZeroMQ zmq_;
    std::unique_ptr<network::Blockchain> blockchain_;
    std::unique_ptr<network::OTDHT> otdht_;
};
}  // namespace opentxs::api
