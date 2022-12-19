// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/system/error_code.hpp>

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <cs_shared_guarded.h>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <future>
#include <memory>
#include <shared_mutex>
#include <string_view>

#include "BoostAsio.hpp"
#include "api/network/asio/Buffers.hpp"
#include "api/network/asio/Data.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "opentxs/network/asio/Endpoint.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WorkType.hpp"

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

template <class T>
class shared_ptr;
}  // namespace boost

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Context;
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
class Shared final : public Allocated
{
public:
    using GuardedData =
        libguarded::shared_guarded<Data, std::shared_timed_mutex>;

    const opentxs::network::zeromq::Context& zmq_;
    const opentxs::network::zeromq::BatchID batch_id_;
    const CString endpoint_;
    std::atomic_bool running_;
    mutable GuardedData data_;

    auto Connect(
        boost::shared_ptr<const Shared> me,
        const ReadView id,
        internal::Asio::SocketImp socket) const noexcept -> bool;
    auto FetchJson(
        boost::shared_ptr<const Shared> me,
        const ReadView host,
        const ReadView path,
        const bool https,
        const ReadView notify) const noexcept
        -> std::future<boost::json::value>;
    auto get_allocator() const noexcept -> allocator_type final;
    auto GetPublicAddress4() const noexcept -> std::shared_future<ByteArray>;
    auto GetPublicAddress6() const noexcept -> std::shared_future<ByteArray>;
    auto GetTimer() const noexcept -> Timer;
    auto IOContext() const noexcept -> boost::asio::io_context&;
    auto Receive(
        boost::shared_ptr<const Shared> me,
        const ReadView id,
        const OTZMQWorkType type,
        const std::size_t bytes,
        internal::Asio::SocketImp socket) const noexcept -> bool;
    auto Resolve(
        boost::shared_ptr<const Shared> me,
        std::string_view connection,
        std::string_view server,
        std::uint16_t port) const noexcept -> void;
    auto Transmit(
        boost::shared_ptr<const Shared> me,
        const ReadView id,
        const ReadView bytes,
        internal::Asio::SocketImp socket) const noexcept -> bool;

    auto Init() noexcept -> void;
    auto Shutdown() noexcept -> void;
    auto StateMachine() noexcept -> bool;

    Shared(
        const opentxs::network::zeromq::Context& zmq,
        opentxs::network::zeromq::BatchID batchID,
        allocator_type alloc) noexcept;
    Shared() = delete;
    Shared(const Shared&) = delete;
    Shared(Shared&&) = delete;
    auto operator=(const Shared&) -> Shared& = delete;
    auto operator=(Shared&&) -> Shared& = delete;

    ~Shared() final;

private:
    enum class ResponseType { IPvonly, AddressOnly };
    enum class IPversion { IPV4, IPV6 };

    using Resolver = Data::Resolver;
    using Response = http::response<http::string_body>;
    using Type = opentxs::network::asio::Endpoint::Type;

    struct Site {
        const CString host_{};
        const CString service_{};
        const CString target_{};
        const ResponseType response_type_{};
        const IPversion protocol_{};
        const unsigned http_version_{};
    };

    static auto sites() -> const Vector<Site>&;

    auto post(const Data& data, internal::Asio::Callback cb) const noexcept
        -> bool;
    auto process_address_query(
        const ResponseType type,
        std::shared_ptr<std::promise<ByteArray>> promise,
        std::future<Response> future) const noexcept -> void;
    auto process_connect(
        const internal::Asio::SocketImp& socket,
        const boost::system::error_code& e,
        ReadView address,
        ReadView connection) const noexcept -> void;
    auto process_json(
        const Data& data,
        const ReadView notify,
        std::shared_ptr<std::promise<boost::json::value>> promise,
        std::future<Response> future) const noexcept -> void;
    auto process_receive(
        const internal::Asio::SocketImp& socket,
        const boost::system::error_code& e,
        std::size_t bytes,
        ReadView address,
        ReadView connection,
        OTZMQWorkType type,
        Buffers::Handle buf) const noexcept -> void;
    auto process_resolve(
        const std::shared_ptr<Resolver>& resolver,
        const boost::system::error_code& e,
        const Resolver::results_type& results,
        std::uint16_t port,
        ReadView connection) const noexcept -> void;
    auto process_transmit(
        const internal::Asio::SocketImp& socket,
        const boost::system::error_code& e,
        std::size_t bytes,
        ReadView connection) const noexcept -> void;
    auto retrieve_address_async(
        const Data& data,
        const Site& site,
        std::shared_ptr<std::promise<ByteArray>> promise) const noexcept
        -> void;
    auto retrieve_address_async_ssl(
        const Data& data,
        const Site& site,
        std::shared_ptr<std::promise<ByteArray>> promise) const noexcept
        -> void;
    auto retrieve_json_http(
        boost::shared_ptr<const Shared> me,
        const Data& data,
        const ReadView host,
        const ReadView path,
        const ReadView notify,
        std::shared_ptr<std::promise<boost::json::value>> promise)
        const noexcept -> void;
    auto retrieve_json_https(
        boost::shared_ptr<const Shared> me,
        const Data& data,
        const ReadView host,
        const ReadView path,
        const ReadView notify,
        std::shared_ptr<std::promise<boost::json::value>> promise)
        const noexcept -> void;
    auto send_notification(const Data& data, const ReadView notify)
        const noexcept -> void;
};
}  // namespace opentxs::api::network::asio
