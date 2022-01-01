// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <future>
#include <map>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "api/network/asio/Acceptors.hpp"
#include "api/network/asio/Buffers.hpp"
#include "api/network/asio/Context.hpp"
#include "core/StateMachine.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/api/network/Blockchain.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/socket/Router.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/WorkType.hpp"

namespace boost
{
namespace asio
{
namespace ssl
{
class context;
}  // namespace ssl
}  // namespace asio

namespace system
{
class error_code;
}  // namespace system
}  // namespace boost

namespace opentxs
{
namespace network
{
namespace asio
{
class Endpoint;
}  // namespace asio

namespace zeromq
{
class Context;
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace zmq = opentxs::network::zeromq;

namespace opentxs::api::network
{
struct Asio::Imp final : public api::network::internal::Asio,
                         public opentxs::internal::StateMachine {
    auto NotificationEndpoint() const noexcept -> const char*;

    auto Accept(
        const opentxs::network::asio::Endpoint& endpoint,
        AcceptCallback cb) noexcept -> bool;
    auto Close(const opentxs::network::asio::Endpoint& endpoint) const noexcept
        -> bool;
    auto Connect(const ReadView id, internal::Asio::Socket& socket) noexcept
        -> bool final;
    auto GetPublicAddress4() const noexcept -> std::shared_future<OTData>;
    auto GetPublicAddress6() const noexcept -> std::shared_future<OTData>;
    auto GetTimer() noexcept -> Timer final;
    auto Init() noexcept -> void;
    auto IOContext() noexcept -> boost::asio::io_context& final;
    auto Post(ThreadPool type, Asio::Callback cb) noexcept -> bool final;
    auto Receive(
        const ReadView id,
        const OTZMQWorkType type,
        const std::size_t bytes,
        internal::Asio::Socket& socket) noexcept -> bool final;
    auto Resolve(std::string_view server, std::uint16_t port) const noexcept
        -> Resolved;
    auto Shutdown() noexcept -> void;

    Imp(const zmq::Context& zmq) noexcept;

    ~Imp() final;

private:
    enum class ResponseType { IPvonly, AddressOnly };
    enum class IPversion { IPV4, IPV6 };

    struct Site {
        const std::string host{};
        const std::string service{};
        const std::string target{};
        const ResponseType response_type{};
        const IPversion protocol{};
        const unsigned http_version{};
    };

    static const std::pmr::vector<Site> sites;

    const zmq::Context& zmq_;
    const std::string notification_endpoint_;
    const OTZMQListenCallback data_cb_;
    OTZMQRouterSocket data_socket_;
    asio::Buffers buffers_;
    mutable std::shared_mutex lock_;
    mutable asio::Context io_context_;
    mutable std::pmr::map<ThreadPool, asio::Context> thread_pools_;
    mutable asio::Acceptors acceptors_;
    std::promise<OTData> ipv4_promise_;
    std::promise<OTData> ipv6_promise_;
    std::shared_future<OTData> ipv4_future_;
    std::shared_future<OTData> ipv6_future_;

    auto data_callback(zmq::Message&& in) noexcept -> void;
    auto load_root_certificates(
        boost::asio::ssl::context& ctx,
        boost::system::error_code& ec) noexcept -> void;
    auto retrieve_address_async(
        const struct Site& site,
        std::promise<OTData>&& promise) -> void;
    auto retrieve_address_async_ssl(
        const struct Site& site,
        std::promise<OTData>&& promise) -> void;

    auto state_machine() noexcept -> bool;

    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    Imp& operator=(const Imp&) = delete;
    Imp& operator=(Imp&&) = delete;
};
}  // namespace opentxs::api::network
