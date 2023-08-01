// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <filesystem>
#include <memory>

#include "internal/api/network/Network.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/network/OTDHT.hpp"

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
class Client;
class Endpoints;
class Notary;
}  // namespace session

class Legacy;
class Session;
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

namespace opentxs::api::network::implementation
{
class Network final : public internal::Network
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
    auto ZeroMQ() const noexcept
        -> const opentxs::network::zeromq::Context& final
    {
        return zmq_;
    }

    auto Shutdown() noexcept -> void final;
    auto Start(
        std::shared_ptr<const api::session::Client> api,
        const api::crypto::Blockchain& crypto,
        const api::Legacy& legacy,
        const std::filesystem::path& dataFolder,
        const Options& args) noexcept -> void final;
    auto Start(
        std::shared_ptr<const api::session::Notary> api,
        const api::crypto::Blockchain& crypto,
        const api::Legacy& legacy,
        const std::filesystem::path& dataFolder,
        const Options& args) noexcept -> void final;

    Network(
        const api::Session& api,
        const network::Asio& asio,
        const opentxs::network::zeromq::Context& zmq,
        const api::session::Endpoints& endpoints,
        std::unique_ptr<api::network::Blockchain> blockchain) noexcept;
    Network() = delete;
    Network(const Network&) = delete;
    Network(Network&&) = delete;
    auto operator=(const Network&) -> Network& = delete;
    auto operator=(Network&&) -> Network& = delete;

    ~Network() final;

private:
    const network::Asio& asio_;
    const opentxs::network::zeromq::Context& zmq_;
    std::unique_ptr<network::Blockchain> blockchain_;
    std::unique_ptr<network::OTDHT> otdht_;
};
}  // namespace opentxs::api::network::implementation
