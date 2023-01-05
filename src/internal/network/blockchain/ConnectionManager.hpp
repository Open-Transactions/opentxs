// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace node
{
class Manager;
}  // namespace node
}  // namespace blockchain

namespace network
{
namespace blockchain
{
class Address;
}  // namespace blockchain

namespace asio
{
class Socket;
}  // namespace asio

namespace zeromq
{
class Frame;
class Message;
}  // namespace zeromq
}  // namespace network

class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::blockchain
{
class ConnectionManager
{
public:
    using BodySize = std::function<std::size_t(const zeromq::Frame& header)>;

    static auto TCP(
        const api::Session& api,
        const Log& log,
        const int id,
        const Address& address,
        const std::size_t headerSize,
        BodySize&& gbs) noexcept -> std::unique_ptr<ConnectionManager>;
    static auto TCPIncoming(
        const api::Session& api,
        const Log& log,
        const int id,
        const Address& address,
        const std::size_t headerSize,
        BodySize&& gbs,
        network::asio::Socket&& socket) noexcept
        -> std::unique_ptr<ConnectionManager>;
    static auto ZMQ(
        const api::Session& api,
        const Log& log,
        const int id,
        const Address& address,
        const std::size_t headerSize) noexcept
        -> std::unique_ptr<ConnectionManager>;
    static auto ZMQIncoming(
        const api::Session& api,
        const opentxs::blockchain::node::Manager& node,
        const Log& log,
        const int id,
        const Address& address,
        const std::size_t headerSize) noexcept
        -> std::unique_ptr<ConnectionManager>;

    virtual auto is_initialized() const noexcept -> bool = 0;
    virtual auto send() const noexcept -> zeromq::Message = 0;

    virtual auto do_connect() noexcept
        -> std::pair<bool, std::optional<std::string_view>> = 0;
    virtual auto do_init() noexcept -> std::optional<std::string_view> = 0;
    virtual auto on_body(zeromq::Message&&) noexcept
        -> std::optional<zeromq::Message> = 0;
    virtual auto on_connect() noexcept -> void = 0;
    virtual auto on_header(zeromq::Message&&) noexcept
        -> std::optional<zeromq::Message> = 0;
    virtual auto on_init() noexcept -> zeromq::Message = 0;
    virtual auto on_register(zeromq::Message&&) noexcept -> void = 0;
    virtual auto shutdown_external() noexcept -> void = 0;
    virtual auto stop_external() noexcept -> std::optional<zeromq::Message> = 0;
    virtual auto transmit(zeromq::Message&& message) noexcept
        -> std::optional<zeromq::Message> = 0;

    virtual ~ConnectionManager() = default;

protected:
    ConnectionManager() = default;
};
}  // namespace opentxs::network::blockchain
