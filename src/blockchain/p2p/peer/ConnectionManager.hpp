// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <future>
#include <memory>
#include <tuple>
#include <utility>

#include "internal/blockchain/node/Node.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace p2p
{
namespace implementation
{
class Peer;
}  // namespace implementation

namespace peer
{
class Address;
class ConnectionManager;
}  // namespace peer
}  // namespace p2p
}  // namespace blockchain

namespace network
{
namespace asio
{
class Socket;
}  // namespace asio

namespace zeromq
{
class Frame;
class Message;
class Pipeline;
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace zmq = opentxs::network::zeromq;

namespace opentxs::blockchain::p2p::peer
{
using Task = node::internal::PeerManager::Task;

class ConnectionManager
{
public:
    using EndpointData = std::pair<UnallocatedCString, std::uint16_t>;
    using SendPromise = std::promise<bool>;

    static auto TCP(
        const api::Session& api,
        const int id,
        p2p::implementation::Peer& parent,
        network::zeromq::Pipeline& pipeline,
        const std::atomic<bool>& running,
        const Address& address,
        const std::size_t headerSize) noexcept
        -> std::unique_ptr<ConnectionManager>;
    static auto TCPIncoming(
        const api::Session& api,
        const int id,
        p2p::implementation::Peer& parent,
        network::zeromq::Pipeline& pipeline,
        const std::atomic<bool>& running,
        const Address& address,
        const std::size_t headerSize,
        network::asio::Socket&& socket) noexcept
        -> std::unique_ptr<ConnectionManager>;
    static auto ZMQ(
        const api::Session& api,
        const int id,
        p2p::implementation::Peer& parent,
        network::zeromq::Pipeline& pipeline,
        const std::atomic<bool>& running,
        const Address& address,
        const std::size_t headerSize) noexcept
        -> std::unique_ptr<ConnectionManager>;
    static auto ZMQIncoming(
        const api::Session& api,
        const int id,
        p2p::implementation::Peer& parent,
        network::zeromq::Pipeline& pipeline,
        const std::atomic<bool>& running,
        const Address& address,
        const std::size_t headerSize) noexcept
        -> std::unique_ptr<ConnectionManager>;

    virtual auto address() const noexcept -> UnallocatedCString = 0;
    virtual auto endpoint_data() const noexcept -> EndpointData = 0;
    virtual auto filter(const Task type) const noexcept -> bool = 0;
    virtual auto host() const noexcept -> UnallocatedCString = 0;
    virtual auto is_initialized() const noexcept -> bool = 0;
    virtual auto port() const noexcept -> std::uint16_t = 0;
    virtual auto style() const noexcept -> p2p::Network = 0;

    virtual auto connect() noexcept -> void = 0;
    virtual auto init() noexcept -> void = 0;
    virtual auto on_body(zmq::Message&&) noexcept -> void = 0;
    virtual auto on_connect(zmq::Message&&) noexcept -> void = 0;
    virtual auto on_header(zmq::Message&&) noexcept -> void = 0;
    virtual auto on_init(zmq::Message&&) noexcept -> void = 0;
    virtual auto on_register(zmq::Message&&) noexcept -> void = 0;
    virtual auto shutdown_external() noexcept -> void = 0;
    virtual auto stop_external() noexcept -> void = 0;
    virtual auto transmit(
        zmq::Frame&& header,
        zmq::Frame&& payload,
        std::unique_ptr<SendPromise> promise) noexcept -> void = 0;

    virtual ~ConnectionManager() = default;

protected:
    ConnectionManager() = default;
};
}  // namespace opentxs::blockchain::p2p::peer
