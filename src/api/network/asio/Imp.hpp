// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/system/error_code.hpp>
#include <boost/thread/thread.hpp>
#include <boost/utility/string_view.hpp>
#include <cs_plain_guarded.h>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <future>
#include <memory>
#include <shared_mutex>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#include "api/network/asio/Acceptors.hpp"
#include "api/network/asio/Buffers.hpp"
#include "api/network/asio/Context.hpp"
#include "core/StateMachine.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/api/network/Blockchain.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/asio/Endpoint.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/socket/Router.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/WorkType.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace boost
{
namespace asio
{
namespace ssl
{
class context;
}  // namespace ssl
}  // namespace asio

namespace json
{
class value;
}  // namespace json

namespace system
{
class error_code;
}  // namespace system
}  // namespace boost

namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace network
{
namespace asio
{
class Endpoint;
}  // namespace asio

namespace zeromq
{
namespace socket
{
class Raw;
}  // namespace socket

class Context;
class Message;
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace algo = boost::algorithm;
namespace beast = boost::beast;
namespace http = boost::beast::http;
namespace ip = boost::asio::ip;
namespace ssl = boost::asio::ssl;
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
    auto FetchJson(
        const ReadView host,
        const ReadView path,
        const bool https,
        const ReadView notify) const noexcept
        -> std::future<boost::json::value> final;
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

    using Resolver = ip::tcp::resolver;
    using Response = http::response<http::string_body>;
    using Type = opentxs::network::asio::Endpoint::Type;
    using GuardedSocket =
        libguarded::plain_guarded<opentxs::network::zeromq::socket::Raw>;
    using NotificationSockets = Map<CString, GuardedSocket>;
    using NotificationMap = libguarded::plain_guarded<NotificationSockets>;

    struct Site {
        const UnallocatedCString host{};
        const UnallocatedCString service{};
        const UnallocatedCString target{};
        const ResponseType response_type{};
        const IPversion protocol{};
        const unsigned http_version{};
    };

    static const Vector<Site> sites;

    const zmq::Context& zmq_;
    const UnallocatedCString notification_endpoint_;
    const OTZMQListenCallback data_cb_;
    OTZMQRouterSocket data_socket_;
    asio::Buffers buffers_;
    mutable std::shared_mutex lock_;
    mutable asio::Context io_context_;
    mutable UnallocatedMap<ThreadPool, asio::Context> thread_pools_;
    mutable asio::Acceptors acceptors_;
    mutable NotificationMap notify_;
    std::promise<OTData> ipv4_promise_;
    std::promise<OTData> ipv6_promise_;
    std::shared_future<OTData> ipv4_future_;
    std::shared_future<OTData> ipv6_future_;

    auto process_address_query(
        const ResponseType type,
        std::shared_ptr<std::promise<OTData>> promise,
        std::future<Response> future) const noexcept -> void;
    auto process_json(
        const ReadView notify,
        std::shared_ptr<std::promise<boost::json::value>> promise,
        std::future<Response> future) const noexcept -> void;
    auto retrieve_json_http(
        const ReadView host,
        const ReadView path,
        const ReadView notify,
        std::shared_ptr<std::promise<boost::json::value>> promise)
        const noexcept -> void;
    auto retrieve_json_https(
        const ReadView host,
        const ReadView path,
        const ReadView notify,
        std::shared_ptr<std::promise<boost::json::value>> promise)
        const noexcept -> void;
    auto send_notification(const ReadView notify) const noexcept -> void;

    auto data_callback(zmq::Message&& in) noexcept -> void;
    auto retrieve_address_async(
        const struct Site& site,
        std::shared_ptr<std::promise<OTData>> promise) -> void;
    auto retrieve_address_async_ssl(
        const struct Site& site,
        std::shared_ptr<std::promise<OTData>> promise) -> void;

    auto state_machine() noexcept -> bool;

    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    Imp& operator=(const Imp&) = delete;
    Imp& operator=(Imp&&) = delete;
};
}  // namespace opentxs::api::network
