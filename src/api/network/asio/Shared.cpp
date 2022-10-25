// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                 // IWYU pragma: associated
#include "api/network/asio/Shared.hpp"  // IWYU pragma: associated

#include <boost/algorithm/string.hpp>
#include <boost/intrusive/list.hpp>
#include <boost/json.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/system/error_code.hpp>
#include <cs_plain_guarded.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <functional>
#include <future>
#include <memory>
#include <new>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>

#include "BoostAsio.hpp"
#include "api/network/asio/Buffers.hpp"
#include "api/network/asio/Context.hpp"
#include "api/network/asio/Data.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/network/asio/HTTP.hpp"
#include "internal/network/asio/HTTPS.hpp"
#include "internal/network/zeromq/socket/Factory.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/network/zeromq/socket/SocketType.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Thread.hpp"
#include "internal/util/Timer.hpp"
#include "network/asio/Endpoint.hpp"
#include "network/asio/Socket.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/asio/Endpoint.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

namespace algo = boost::algorithm;
namespace ssl = boost::asio::ssl;

namespace opentxs::api::network::asio
{
using namespace std::literals;

Shared::Shared(
    const opentxs::network::zeromq::Context& zmq,
    opentxs::network::zeromq::BatchID batchID,
    allocator_type alloc) noexcept
    : zmq_(zmq)
    , batch_id_(std::move(batchID))
    , endpoint_(opentxs::network::zeromq::MakeArbitraryInproc(alloc))
    , running_(false)
    , data_(zmq_, endpoint_, alloc)
{
}

auto Shared::Connect(
    boost::shared_ptr<const Shared> me,
    const ReadView id,
    internal::Asio::SocketImp socket) const noexcept -> bool
{
    try {
        if (false == socket.operator bool()) {
            throw std::runtime_error{"invalid socket"};
        }

        if (0 == id.size()) { throw std::runtime_error{"invalid id"}; }

        const auto handle = data_.lock_shared();
        [[maybe_unused]] const auto& data = *handle;

        if (false == running_) { throw std::runtime_error{"shutting down"}; }

        const auto& endpoint = socket->endpoint_;
        const auto& internal = endpoint.GetInternal().data_;
        socket->socket_.async_connect(
            internal,
            [me,
             asio{socket},
             connection{space(id)},
             address{CString(endpoint.str(), get_allocator())}](const auto& e) {
                me->process_connect(asio, e, address, reader(connection));
            });

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Shared::get_allocator() const noexcept -> allocator_type
{
    return data_.lock_shared()->get_allocator();
}

auto Shared::FetchJson(
    boost::shared_ptr<const Shared> me,
    const ReadView host,
    const ReadView path,
    const bool https,
    const ReadView notify) const noexcept -> std::future<boost::json::value>
{
    auto promise = std::make_shared<std::promise<boost::json::value>>();
    auto future = promise->get_future();
    auto f =
        (https) ? &Shared::retrieve_json_https : &Shared::retrieve_json_http;
    const auto handle = data_.lock_shared();
    const auto& data = *handle;
    std::invoke(f, *this, me, data, host, path, notify, std::move(promise));

    return future;
}

auto Shared::GetPublicAddress4() const noexcept -> std::shared_future<ByteArray>
{
    return data_.lock_shared()->ipv4_future_;
}

auto Shared::GetPublicAddress6() const noexcept -> std::shared_future<ByteArray>
{
    return data_.lock_shared()->ipv6_future_;
}

auto Shared::GetTimer() const noexcept -> Timer
{
    return opentxs::factory::Timer(data_.lock_shared()->io_context_);
}

auto Shared::Init() noexcept -> void
{
    auto handle = data_.lock();
    auto& data = *handle;
    const auto threads = MaxJobs();
    data.io_context_->Init(std::max(threads / 8u, 1u), ThreadPriority::Normal);
    data.thread_pools_.at(ThreadPool::General)
        .Init(std::max(threads - 1u, 1u), ThreadPriority::AboveNormal);
    data.thread_pools_.at(ThreadPool::Storage)
        .Init(std::max(threads / 4u, 2u), ThreadPriority::Highest);
    data.thread_pools_.at(ThreadPool::Blockchain)
        .Init(std::max(threads, 1u), ThreadPriority::Lowest);
    running_ = true;
}

auto Shared::IOContext() const noexcept -> boost::asio::io_context&
{
    return *(data_.lock()->io_context_);
}

auto Shared::Post(
    ThreadPool type,
    internal::Asio::Callback cb,
    std::string_view threadName) const noexcept -> bool
{
    if (false == cb.operator bool()) { return false; }

    const auto handle = data_.lock_shared();

    return post(*handle, type, cb, threadName);
}

auto Shared::post(
    const Data& data,
    ThreadPool type,
    internal::Asio::Callback cb,
    std::string_view threadName) const noexcept -> bool
{
    OT_ASSERT(cb);

    if (false == running_) { return false; }

    auto& pool = [&]() -> auto&
    {
        if (ThreadPool::Network == type) {

            return *data.io_context_;
        } else {

            return data.thread_pools_.at(type);
        }
    }
    ();
    boost::asio::post(
        pool.get(),
        [action = std::move(cb),
         name = CString{"asio "}
                    .append(print(type))
                    .append(": ")
                    .append(threadName),
         type] {
            SetThisThreadsName(name);
            action();
            SetThisThreadsName(
                CString{"asio "}.append(print(type)).append(": idle"));
        });

    return true;
}

auto Shared::process_address_query(
    const ResponseType type,
    std::shared_ptr<std::promise<ByteArray>> promise,
    std::future<Response> future) const noexcept -> void
{
    if (!promise) { return; }

    try {
        const auto string = [&] {
            auto output = CString{};
            const auto body = future.get().body();

            switch (type) {
                case ResponseType::IPvonly: {
                    auto parts = Vector<CString>{};
                    algo::split(parts, body, algo::is_any_of(","));

                    if (parts.size() > 1) { output = parts[1]; }
                } break;
                case ResponseType::AddressOnly: {
                    output = body;
                } break;
                default: {
                    throw std::runtime_error{"Unknown response type"};
                }
            }

            return output;
        }();

        if (string.empty()) { throw std::runtime_error{"Empty response"}; }

        auto ec = beast::error_code{};
        const auto address = ip::make_address(string, ec);

        if (ec) {
            const auto error =
                CString{} + "error parsing ip address: " + ec.message().c_str();

            throw std::runtime_error{error.c_str()};
        }

        LogVerbose()(OT_PRETTY_CLASS())("GET response: IP address: ")(string)
            .Flush();

        if (address.is_v4()) {
            const auto bytes = address.to_v4().to_bytes();
            promise->set_value(ByteArray{bytes.data(), bytes.size()});
        } else if (address.is_v6()) {
            const auto bytes = address.to_v6().to_bytes();
            promise->set_value(ByteArray{bytes.data(), bytes.size()});
        }
    } catch (...) {
        promise->set_exception(std::current_exception());
    }
}

auto Shared::process_connect(
    const internal::Asio::SocketImp&,
    const boost::system::error_code& e,
    ReadView address,
    ReadView connection) const noexcept -> void
{
    auto handle = data_.try_lock_for(10ms);

    while (false == handle.operator bool()) {
        if (false == running_) { return; }

        handle = data_.try_lock_for(10ms);
    }

    handle->to_actor_.SendDeferred(
        [&] {
            if (e) {
                LogVerbose()(OT_PRETTY_STATIC(Shared))("asio connect error: ")(
                    e.message())
                    .Flush();
                auto work =
                    opentxs::network::zeromq::tagged_reply_to_connection(
                        connection, WorkType::AsioDisconnect);
                work.AddFrame(address.data(), address.size());
                work.AddFrame(e.message());

                return work;
            } else {
                auto work =
                    opentxs::network::zeromq::tagged_reply_to_connection(
                        connection, WorkType::AsioConnect);
                work.AddFrame(address.data(), address.size());

                return work;
            }
        }(),
        __FILE__,
        __LINE__);
}

auto Shared::process_json(
    const Data& data,
    const ReadView notify,
    std::shared_ptr<std::promise<boost::json::value>> promise,
    std::future<Response> future) const noexcept -> void
{
    if (!promise) { return; }

    try {
        const auto body = future.get().body();
        auto parser = boost::json::parser{};
        parser.write_some(body);
        promise->set_value(parser.release());
    } catch (...) {
        promise->set_exception(std::current_exception());
    }

    send_notification(data, notify);
}

auto Shared::process_receive(
    const internal::Asio::SocketImp&,
    const boost::system::error_code& e,
    std::size_t,
    ReadView address,
    ReadView connection,
    OTZMQWorkType type,
    Buffers::Handle buf) const noexcept -> void
{
    // TODO c++20 const auto& [index, buffer] = buf;
    const auto& index = buf.first;
    const auto& buffer = buf.second;
    auto handle = data_.try_lock_for(10ms);

    while (false == handle.operator bool()) {
        if (false == running_) { return; }

        handle = data_.try_lock_for(10ms);
    }

    auto& data = *handle;
    data.to_actor_.SendDeferred(
        [&]() {
            auto work = opentxs::network::zeromq::tagged_reply_to_connection(
                connection, e ? value(WorkType::AsioDisconnect) : type);

            if (e) {
                work.AddFrame(address.data(), address.size());
                work.AddFrame(e.message());
            } else {
                work.AddFrame(buffer.data(), buffer.size());
            }

            OT_ASSERT(1 < work.Body().size());

            return work;
        }(),
        __FILE__,
        __LINE__);
    data.buffers_.clear(index);
}

auto Shared::process_resolve(
    const std::shared_ptr<Resolver>&,
    const boost::system::error_code& e,
    const Resolver::results_type& results,
    std::uint16_t port,
    ReadView connection) const noexcept -> void
{
    auto handle = data_.try_lock_for(10ms);

    while (false == handle.operator bool()) {
        if (false == running_) { return; }

        handle = data_.try_lock_for(10ms);
    }

    handle->to_actor_.SendDeferred(
        [&] {
            static constexpr auto trueValue = std::byte{0x01};
            static constexpr auto falseValue = std::byte{0x00};
            auto work = opentxs::network::zeromq::tagged_reply_to_connection(
                connection, value(WorkType::AsioResolve));

            if (e) {
                work.AddFrame(falseValue);
                work.AddFrame(e.message());
            } else {
                work.AddFrame(trueValue);
                work.AddFrame(port);

                for (const auto& result : results) {
                    const auto address = result.endpoint().address();

                    if (address.is_v4()) {
                        const auto bytes = address.to_v4().to_bytes();
                        work.AddFrame(bytes.data(), bytes.size());
                    } else {
                        const auto bytes = address.to_v6().to_bytes();
                        work.AddFrame(bytes.data(), bytes.size());
                    }
                }
            }

            return work;
        }(),
        __FILE__,
        __LINE__);
}

auto Shared::process_transmit(
    const internal::Asio::SocketImp&,
    const boost::system::error_code& e,
    std::size_t bytes,
    ReadView connection) const noexcept -> void
{
    auto handle = data_.try_lock_for(10ms);

    while (false == handle.operator bool()) {
        if (false == running_) { return; }

        handle = data_.try_lock_for(10ms);
    }

    handle->to_actor_.SendDeferred(
        [&] {
            auto work = opentxs::network::zeromq::tagged_reply_to_connection(
                connection, value(WorkType::AsioSendResult));
            work.AddFrame(bytes);
            static constexpr auto trueValue = std::byte{0x01};
            static constexpr auto falseValue = std::byte{0x00};

            if (e) {
                work.AddFrame(falseValue);
                work.AddFrame(e.message());
            } else {
                work.AddFrame(trueValue);
            }

            return work;
        }(),
        __FILE__,
        __LINE__);
}

auto Shared::Receive(
    boost::shared_ptr<const Shared> me,
    const ReadView id,
    const OTZMQWorkType type,
    const std::size_t bytes,
    internal::Asio::SocketImp socket) const noexcept -> bool
{
    try {
        if (false == socket.operator bool()) {
            throw std::runtime_error{"invalid socket"};
        }

        if (0 == id.size()) { throw std::runtime_error{"invalid id"}; }

        const auto handle = data_.lock_shared();
        const auto& data = *handle;

        if (false == running_) { throw std::runtime_error{"shutting down"}; }

        auto bufData = data.buffers_.get(bytes);
        const auto& endpoint = socket->endpoint_;
        boost::asio::async_read(
            socket->socket_,
            bufData.second,
            [me,
             bufData,
             connection{space(id)},
             address = CString{endpoint.str(), get_allocator()},
             type,
             asio{socket}](const auto& e, auto size) {
                me->process_receive(
                    asio, e, size, address, reader(connection), type, bufData);
            });

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Shared::Resolve(
    boost::shared_ptr<const Shared> me,
    std::string_view connection,
    std::string_view server,
    std::uint16_t port) const noexcept -> void
{
    auto handle = data_.lock();
    auto& data = *handle;

    if (false == running_) { return; }

    try {
        auto alloc = data.get_allocator();
        auto& resolver = *data.resolver_;
        resolver.async_resolve(
            server,
            std::to_string(port),
            [me, port, id = CString{connection, alloc}, p = data.resolver_](
                const auto& e, const auto& results) {
                me->process_resolve(p, e, results, port, id);
            });
    } catch (const std::exception& e) {
        LogVerbose()(OT_PRETTY_CLASS())(e.what()).Flush();
    }
}

auto Shared::retrieve_address_async(
    const Data& data,
    const Site& site,
    std::shared_ptr<std::promise<ByteArray>> pPromise) const noexcept -> void
{
    using HTTP = opentxs::network::asio::HTTP;
    auto alloc = get_allocator();
    post(
        data,
        ThreadPool::Network,
        [job = std::allocate_shared<HTTP>(
             alloc,
             site.host_,
             site.target_,
             *data.io_context_,
             [this, promise = std::move(pPromise), type = site.response_type_](
                 auto&& future) mutable {
                 process_address_query(
                     type, std::move(promise), std::move(future));
             })] { job->Start(); },
        __FUNCTION__);
}

auto Shared::retrieve_address_async_ssl(
    const Data& data,
    const Site& site,
    std::shared_ptr<std::promise<ByteArray>> pPromise) const noexcept -> void
{
    using HTTPS = opentxs::network::asio::HTTPS;
    auto alloc = get_allocator();
    post(
        data,
        ThreadPool::Network,
        [job = std::allocate_shared<HTTPS>(
             alloc,
             site.host_,
             site.target_,
             *data.io_context_,
             [this, promise = std::move(pPromise), type = site.response_type_](
                 auto&& future) mutable {
                 process_address_query(
                     type, std::move(promise), std::move(future));
             })] { job->Start(); },
        __FUNCTION__);
}

auto Shared::retrieve_json_http(
    boost::shared_ptr<const Shared> me,
    const Data& data,
    const ReadView host,
    const ReadView path,
    const ReadView notify,
    std::shared_ptr<std::promise<boost::json::value>> pPromise) const noexcept
    -> void
{
    using HTTP = opentxs::network::asio::HTTP;
    auto alloc = get_allocator();
    post(
        data,
        ThreadPool::Network,
        [job = std::allocate_shared<HTTP>(
             alloc,
             host,
             path,
             *data.io_context_,
             [me,
              promise = std::move(pPromise),
              socket = CString{notify, alloc}](auto&& future) mutable {
                 auto handle = me->data_.try_lock_shared_for(10ms);

                 while (false == handle.operator bool()) {
                     if (false == me->running_) { return; }

                     handle = me->data_.try_lock_shared_for(10ms);
                 }

                 me->process_json(
                     *handle, socket, std::move(promise), std::move(future));
             })] { job->Start(); },
        __FUNCTION__);
}

auto Shared::retrieve_json_https(
    boost::shared_ptr<const Shared> me,
    const Data& data,
    const ReadView host,
    const ReadView path,
    const ReadView notify,
    std::shared_ptr<std::promise<boost::json::value>> pPromise) const noexcept
    -> void
{
    using HTTPS = opentxs::network::asio::HTTPS;
    auto alloc = get_allocator();
    post(
        data,
        ThreadPool::Network,
        [job = std::allocate_shared<HTTPS>(
             alloc,
             host,
             path,
             *data.io_context_,
             [me,
              promise = std::move(pPromise),
              socket = CString{notify, alloc}](auto&& future) mutable {
                 auto handle = me->data_.try_lock_shared_for(10ms);

                 while (false == handle.operator bool()) {
                     if (false == me->running_) { return; }

                     handle = me->data_.try_lock_shared_for(10ms);
                 }
                 me->process_json(
                     *handle, socket, std::move(promise), std::move(future));
             })] { job->Start(); },
        __FUNCTION__);
}

auto Shared::send_notification(const Data& data, const ReadView notify)
    const noexcept -> void
{
    if (false == valid(notify)) { return; }

    try {
        const auto endpoint = CString{notify};
        auto& socket = [&]() -> auto&
        {
            auto handle = data.notify_.lock();
            auto& map = *handle;

            if (auto it = map.find(endpoint); map.end() != it) {

                return it->second;
            }

            auto [it, added] = map.try_emplace(endpoint, [&] {
                auto out = factory::ZMQSocket(
                    zmq_, opentxs::network::zeromq::socket::Type::Publish);
                const auto rc = out.Connect(endpoint.data());

                if (false == rc) {
                    throw std::runtime_error{
                        "Failed to connect to notification endpoint"};
                }

                return out;
            }());

            return it->second;
        }
        ();
        LogTrace()(OT_PRETTY_CLASS())("notifying ")(endpoint).Flush();
        const auto rc = socket.lock()->Send(
            MakeWork(OT_ZMQ_STATE_MACHINE_SIGNAL), __FILE__, __LINE__);

        if (false == rc) {
            throw std::runtime_error{"Failed to send notification"};
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return;
    }
}

auto Shared::Shutdown() noexcept -> void
{
    running_ = false;
    auto handle = data_.lock();
    auto& data = *handle;
    data.resolver_.reset();
    data.io_context_->Stop();

    for (auto& [type, pool] : data.thread_pools_) { pool.Stop(); }

    data.thread_pools_.clear();
}

auto Shared::StateMachine() noexcept -> bool
{
    auto again{false};

    {
        auto handle = data_.lock();
        auto& data = *handle;
        data.ipv4_promise_ = {};
        data.ipv6_promise_ = {};
        data.ipv4_future_ = data.ipv4_promise_.get_future();
        data.ipv6_future_ = data.ipv6_promise_.get_future();
    }

    auto alloc = get_allocator();
    auto futures4 = Vector<std::future<ByteArray>>{alloc};
    auto futures6 = Vector<std::future<ByteArray>>{alloc};

    {
        auto handle = data_.try_lock_shared_for(10ms);

        while (false == handle.operator bool()) {
            if (false == running_) { return false; }

            handle = data_.try_lock_shared_for(10ms);
        }

        const auto& data = *handle;

        for (const auto& site : sites()) {
            auto promise = std::make_shared<std::promise<ByteArray>>();

            if (IPversion::IPV4 == site.protocol_) {
                futures4.emplace_back(promise->get_future());

                if ("https" == site.service_) {
                    retrieve_address_async_ssl(data, site, std::move(promise));
                } else {
                    retrieve_address_async(data, site, std::move(promise));
                }
            } else {
                futures6.emplace_back(promise->get_future());

                if ("https" == site.service_) {
                    retrieve_address_async_ssl(data, site, std::move(promise));
                } else {
                    retrieve_address_async(data, site, std::move(promise));
                }
            }
        }
    }

    auto result4 = ByteArray{};
    auto result6 = ByteArray{};
    static constexpr auto limit = 15s;
    static constexpr auto ready = std::future_status::ready;

    for (auto& future : futures4) {
        try {
            if (const auto status = future.wait_for(limit); ready == status) {
                auto result = future.get();

                if (result.empty()) { continue; }

                result4 = std::move(result);
                break;
            }
        } catch (...) {
            try {
                auto eptr = std::current_exception();

                if (eptr) { std::rethrow_exception(eptr); }
            } catch (const std::exception& e) {
                LogVerbose()(OT_PRETTY_CLASS())(e.what()).Flush();
            }
        }
    }

    for (auto& future : futures6) {
        try {
            if (const auto status = future.wait_for(limit); ready == status) {
                auto result = future.get();

                if (result.empty()) { continue; }

                result6 = std::move(result);
                break;
            }
        } catch (...) {
            try {
                auto eptr = std::current_exception();

                if (eptr) { std::rethrow_exception(eptr); }
            } catch (const std::exception& e) {
                LogVerbose()(OT_PRETTY_CLASS())(e.what()).Flush();
            }
        }
    }

    if (result4.empty() && result6.empty()) { again = true; }

    {
        auto handle = data_.lock();
        auto& data = *handle;
        data.ipv4_promise_.set_value(std::move(result4));
        data.ipv6_promise_.set_value(std::move(result6));
    }

    LogTrace()(OT_PRETTY_CLASS())("Finished checking ip addresses").Flush();

    return again;
}

auto Shared::Transmit(
    boost::shared_ptr<const Shared> me,
    const ReadView id,
    const ReadView bytes,
    internal::Asio::SocketImp socket) const noexcept -> bool
{
    try {
        if (false == socket.operator bool()) {
            throw std::runtime_error{"invalid socket"};
        }

        if (0 == id.size()) { throw std::runtime_error{"invalid id"}; }

        const auto handle = data_.lock_shared();
        const auto& data = *handle;

        if (false == running_) { return false; }

        const auto connection = std::make_shared<Space>(space(id));
        const auto buf = std::make_shared<Space>(space(bytes));

        return post(
            data,
            ThreadPool::Network,
            [me, socket, connection, buf] {
                boost::asio::async_write(
                    socket->socket_,
                    boost::asio::buffer(buf->data(), buf->size()),
                    [me, connection, asio{socket}, buffer{buf}](
                        const boost::system::error_code& e, std::size_t bytes) {
                        me->process_transmit(
                            asio, e, bytes, reader(*connection));
                    });
            },
            "asio transmit");
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

Shared::~Shared() = default;
}  // namespace opentxs::api::network::asio
