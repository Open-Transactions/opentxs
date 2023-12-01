// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/system/error_code.hpp>

#pragma once

#include <cs_shared_guarded.h>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <future>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <string_view>

#include "BoostAsio.hpp"
#include "api/network/asio/Data.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/network/asio/Types.hpp"
#include "opentxs/network/asio/Endpoint.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace boost
{
namespace json
{
class value;
}  // namespace json

namespace system
{
class error_code;
}  // namespace system
}  // namespace boost

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Context;
class Envelope;
}  // namespace zeromq
}  // namespace network

class ByteArray;
class Timer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace beast = boost::beast;
namespace http = boost::beast::http;
namespace ip = boost::asio::ip;

namespace opentxs::api::network::asio
{
class Shared
{
public:
    using GuardedData =
        libguarded::shared_guarded<Data, std::shared_timed_mutex>;

    const opentxs::network::zeromq::Context& zmq_;
    const opentxs::network::zeromq::BatchID batch_id_;
    const UnallocatedCString endpoint_;
    std::atomic_bool running_;
    mutable GuardedData data_;

    static auto Connect(
        std::shared_ptr<const Shared> me,
        const opentxs::network::zeromq::Envelope& id,
        internal::Asio::SocketImp socket) noexcept -> bool;
    static auto FetchJson(
        std::shared_ptr<const Shared> me,
        const ReadView host,
        const ReadView path,
        const bool https,
        const ReadView notify) noexcept -> std::future<boost::json::value>;
    static auto Receive(
        std::shared_ptr<const Shared> me,
        const opentxs::network::zeromq::Envelope& id,
        const OTZMQWorkType type,
        const std::size_t bytes,
        internal::Asio::SocketImp socket) noexcept -> bool;
    static auto Resolve(
        std::shared_ptr<const Shared> me,
        const opentxs::network::zeromq::Envelope& id,
        std::string_view server,
        std::uint16_t port) noexcept -> void;
    static auto Transmit(
        std::shared_ptr<const Shared> me,
        const opentxs::network::zeromq::Envelope& id,
        const ReadView bytes,
        internal::Asio::SocketImp socket) noexcept -> bool;

    auto GetPublicAddress4() const noexcept -> std::shared_future<ByteArray>;
    auto GetPublicAddress6() const noexcept -> std::shared_future<ByteArray>;
    auto GetTimer() const noexcept -> Timer;
    auto IOContext() const noexcept -> boost::asio::io_context&;

    auto Init() noexcept -> void;
    auto StateMachine() noexcept -> bool;

    Shared(const opentxs::network::zeromq::Context& zmq, bool test) noexcept;
    Shared() = delete;
    Shared(const Shared&) = delete;
    Shared(Shared&&) = delete;
    auto operator=(const Shared&) -> Shared& = delete;
    auto operator=(Shared&&) -> Shared& = delete;

    ~Shared();

private:
    enum class ResponseType { IPvonly, AddressOnly };
    enum class IPversion { IPV4, IPV6 };

    using Resolver = Data::Resolver;
    using Response = http::response<http::string_body>;
    using Type = opentxs::network::asio::Endpoint::Type;

    struct Site {
        const std::optional<opentxs::network::asio::TLS> tls_{};
        const CString host_{};
        const CString target_{};
        const ResponseType response_type_{};
        const IPversion protocol_{};
    };

    static auto retrieve_json_http(
        std::shared_ptr<const Shared> me,
        opentxs::network::asio::TLS tls,
        const Data& data,
        const ReadView host,
        const ReadView path,
        const ReadView notify,
        std::shared_ptr<std::promise<boost::json::value>> promise) noexcept
        -> void;
    static auto retrieve_json_https(
        std::shared_ptr<const Shared> me,
        opentxs::network::asio::TLS tls,
        const Data& data,
        const ReadView host,
        const ReadView path,
        const ReadView notify,
        std::shared_ptr<std::promise<boost::json::value>> promise) noexcept
        -> void;
    static auto sites() -> const Vector<Site>&;

    auto post(const Data& data, internal::Asio::Callback cb) const noexcept
        -> bool;
    auto process_address_query(
        const ResponseType type,
        std::shared_ptr<std::promise<ByteArray>> promise,
        std::future<Response> future) const noexcept -> void;
    auto process_connect(
        internal::Asio::SocketImp socket,
        const boost::system::error_code& e,
        ReadView address,
        opentxs::network::zeromq::Envelope&& connection) const noexcept -> void;
    auto process_json(
        const Data& data,
        const ReadView notify,
        std::shared_ptr<std::promise<boost::json::value>> promise,
        std::future<Response> future) const noexcept -> void;
    auto process_receive(
        internal::Asio::SocketImp socket,
        const boost::system::error_code& e,
        ReadView address,
        opentxs::network::zeromq::Envelope&& connection,
        OTZMQWorkType type,
        std::size_t index,
        ReadView data) const noexcept -> void;
    auto process_resolve(
        const std::shared_ptr<Resolver>& resolver,
        const boost::system::error_code& e,
        const Resolver::results_type& results,
        std::string_view server,
        std::uint16_t port,
        opentxs::network::zeromq::Envelope&& connection) const noexcept -> void;
    auto process_transmit(
        internal::Asio::SocketImp socket,
        const boost::system::error_code& e,
        std::size_t bytes,
        opentxs::network::zeromq::Envelope&& connection,
        std::size_t index) const noexcept -> void;
    auto retrieve_address_async(
        const Data& data,
        const Site& site,
        std::shared_ptr<std::promise<ByteArray>> promise) const noexcept
        -> void;
    auto retrieve_address_async_ssl(
        opentxs::network::asio::TLS tls,
        const Data& data,
        const Site& site,
        std::shared_ptr<std::promise<ByteArray>> promise) const noexcept
        -> void;
    auto send_notification(const Data& data, const ReadView notify)
        const noexcept -> void;
};
}  // namespace opentxs::api::network::asio
