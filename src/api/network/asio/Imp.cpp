// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"              // IWYU pragma: associated
#include "1_Internal.hpp"            // IWYU pragma: associated
#include "api/network/asio/Imp.hpp"  // IWYU pragma: associated

#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/json.hpp>
#include <boost/json/src.hpp>  // IWYU pragma: keep
#include <boost/system/error_code.hpp>
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <functional>
#include <future>
#include <stdexcept>
#include <string_view>
#include <thread>
#include <type_traits>
#include <utility>

#include "api/network/asio/Acceptors.hpp"
#include "api/network/asio/Context.hpp"
#include "core/StateMachine.hpp"
#include "internal/network/asio/HTTP.hpp"
#include "internal/network/asio/HTTPS.hpp"
#include "internal/network/zeromq/socket/Factory.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "network/asio/Endpoint.hpp"
#include "network/asio/Socket.hpp"  // IWYU pragma: keep
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/asio/Endpoint.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/Router.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Thread.hpp"
#include "util/Work.hpp"

namespace opentxs::api::network
{
Asio::Imp::Imp(const zmq::Context& zmq) noexcept
    : StateMachine([&] { return state_machine(); })
    , zmq_(zmq)
    , notification_endpoint_(opentxs::network::zeromq::MakeDeterministicInproc(
          "asio/register",
          -1,
          1))
    , data_cb_(zmq::ListenCallback::Factory(
          [this](auto&& in) { data_callback(std::move(in)); }))
    , data_socket_(zmq_.RouterSocket(data_cb_, zmq::socket::Direction::Bind))
    , buffers_()
    , lock_()
    , io_context_(std::make_shared<asio::Context>())
    , thread_pools_([] {
        auto out = UnallocatedMap<ThreadPool, asio::Context>{};
        out[ThreadPool::General];
        out[ThreadPool::Storage];
        out[ThreadPool::Blockchain];

        return out;
    }())
    , acceptors_(*this, *io_context_)
    , notify_()
    , ipv4_promise_()
    , ipv6_promise_()
    , ipv4_future_(ipv4_promise_.get_future())
    , ipv6_future_(ipv6_promise_.get_future())
{
    OT_ASSERT(io_context_);

    Trigger();
}

auto Asio::Imp::Accept(
    const opentxs::network::asio::Endpoint& endpoint,
    AcceptCallback cb) noexcept -> bool
{
    return acceptors_.Start(endpoint, std::move(cb));
}

auto Asio::Imp::Close(
    const opentxs::network::asio::Endpoint& endpoint) const noexcept -> bool
{
    return acceptors_.Close(endpoint);
}

auto Asio::Imp::Connect(
    const ReadView id,
    internal::Asio::Socket& socket) noexcept -> bool
{
    auto lock = sLock{lock_};

    if (shutdown()) { return false; }

    if (0 == id.size()) { return false; }

    const auto& endpoint = socket.endpoint_;
    const auto& internal = endpoint.GetInternal().data_;
    socket.socket_.async_connect(
        internal,
        [this, connection{space(id)}, address{endpoint.str()}](const auto& e) {
            if (e) {
                LogVerbose()(OT_PRETTY_CLASS())("asio connect error: ")(
                    e.message())
                    .Flush();
            }

            data_socket_->Send([&] {
                auto work =
                    opentxs::network::zeromq::tagged_reply_to_connection(
                        reader(connection),
                        e ? WorkType::AsioDisconnect : WorkType::AsioConnect);
                work.AddFrame(address);

                return work;
            }());
        });

    return true;
}

auto Asio::Imp::IOContext() noexcept -> boost::asio::io_context&
{
    return *io_context_;
}

auto Asio::Imp::data_callback(zmq::Message&& in) noexcept -> void
{
    const auto header = in.Header();

    if (0 == header.size()) { return; }

    const auto& connectionID = header.at(header.size() - 1u);

    if (0 == connectionID.size()) { return; }

    const auto body = in.Body();

    if (0 == body.size()) { return; }

    try {
        const auto work = [&] {
            try {

                return body.at(0).as<OTZMQWorkType>();
            } catch (...) {

                throw std::runtime_error{"Wrong size for work frame"};
            }
        }();

        switch (work) {
            case value(WorkType::AsioRegister): {
                data_socket_->Send([&] {
                    auto work =
                        opentxs::network::zeromq::tagged_reply_to_message(
                            in, WorkType::AsioRegister);
                    work.AddFrame(connectionID);

                    return work;
                }());
            } break;
            default: {
                throw std::runtime_error{
                    "Unknown work type " + std::to_string(work)};
            }
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
    }
}

auto Asio::Imp::FetchJson(
    const ReadView host,
    const ReadView path,
    const bool https,
    const ReadView notify) const noexcept -> std::future<boost::json::value>
{
    auto promise = std::make_shared<std::promise<boost::json::value>>();
    auto future = promise->get_future();
    auto f = (https) ? &Imp::retrieve_json_https : &Imp::retrieve_json_http;
    std::invoke(f, *this, host, path, notify, std::move(promise));

    return future;
}

auto Asio::Imp::GetPublicAddress4() const noexcept -> std::shared_future<OTData>
{
    auto lock = sLock{lock_};

    return ipv4_future_;
}

auto Asio::Imp::GetPublicAddress6() const noexcept -> std::shared_future<OTData>
{
    auto lock = sLock{lock_};

    return ipv6_future_;
}

auto Asio::Imp::GetTimer() noexcept -> Timer
{
    return opentxs::factory::Timer(io_context_);
}

auto Asio::Imp::Init() noexcept -> void
{
    auto lock = eLock{lock_};

    if (shutdown()) { return; }

    {
        const auto listen = data_socket_->Start(notification_endpoint_);

        OT_ASSERT(listen);
    }

    const auto threads =
        std::max<unsigned int>(std::thread::hardware_concurrency(), 1u);
    io_context_->Init(
        std::max<unsigned int>(threads / 8u, 1u), ThreadPriority::Normal);
    thread_pools_.at(ThreadPool::General)
        .Init(
            std::max<unsigned int>(threads - 1u, 1u),
            ThreadPriority::AboveNormal);
    thread_pools_.at(ThreadPool::Storage)
        .Init(
            std::max<unsigned int>(threads / 4u, 2u), ThreadPriority::Highest);
    thread_pools_.at(ThreadPool::Blockchain)
        .Init(std::max<unsigned int>(threads, 1u), ThreadPriority::Lowest);
}

auto Asio::Imp::NotificationEndpoint() const noexcept -> const char*
{
    return notification_endpoint_.c_str();
}

auto Asio::Imp::Post(ThreadPool type, Asio::Callback cb) noexcept -> bool
{
    auto lock = sLock{lock_};

    if (shutdown()) { return false; }

    auto& pool = [&]() -> auto&
    {
        if (ThreadPool::Network == type) {

            return *io_context_;
        } else {

            return thread_pools_.at(type);
        }
    }
    ();
    boost::asio::post(pool.get(), std::move(cb));

    return true;
}

auto Asio::Imp::process_address_query(
    const ResponseType type,
    std::shared_ptr<std::promise<OTData>> promise,
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
            promise->set_value(Data::Factory(bytes.data(), bytes.size()));
        } else if (address.is_v6()) {
            const auto bytes = address.to_v6().to_bytes();
            promise->set_value(Data::Factory(bytes.data(), bytes.size()));
        }
    } catch (...) {
        promise->set_exception(std::current_exception());
    }
}

auto Asio::Imp::process_json(
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

    send_notification(notify);
}

auto Asio::Imp::Receive(
    const ReadView id,
    const OTZMQWorkType type,
    const std::size_t bytes,
    internal::Asio::Socket& socket) noexcept -> bool
{
    auto lock = sLock{lock_};

    if (shutdown()) { return false; }

    if (0 == id.size()) { return false; }

    auto bufData = buffers_.get(bytes);
    const auto& endpoint = socket.endpoint_;
    boost::asio::async_read(
        socket.socket_,
        bufData.second,
        [this, connection{space(id)}, type, bufData, address{endpoint.str()}](
            const auto& e, auto size) {
            data_socket_->Send([&] {
                const auto& [index, buffer] = bufData;
                auto work =
                    opentxs::network::zeromq::tagged_reply_to_connection(
                        reader(connection),
                        e ? value(WorkType::AsioDisconnect) : type);

                if (e) {
                    LogVerbose()(OT_PRETTY_CLASS())("asio receive error: ")(
                        e.message())
                        .Flush();
                    work.AddFrame(address);
                } else {
                    work.AddFrame(buffer.data(), buffer.size());
                }

                OT_ASSERT(1 < work.Body().size());

                return work;
            }());
            buffers_.clear(bufData.first);
        });

    return true;
}

auto Asio::Imp::Resolve(std::string_view server, std::uint16_t port)
    const noexcept -> Resolved
{
    auto output = Resolved{};
    auto lock = sLock{lock_};

    if (shutdown()) { return output; }

    try {
        auto resolver = Resolver{io_context_->get()};
        const auto results = resolver.resolve(
            server, std::to_string(port), Resolver::query::numeric_service);
        output.reserve(results.size());

        for (const auto& result : results) {
            const auto address = result.endpoint().address();

            if (address.is_v4()) {
                const auto bytes = address.to_v4().to_bytes();
                output.emplace_back(
                    Type::ipv4,
                    ReadView{
                        reinterpret_cast<const char*>(bytes.data()),
                        bytes.size()},
                    port);
            } else {
                const auto bytes = address.to_v6().to_bytes();
                output.emplace_back(
                    Type::ipv6,
                    ReadView{
                        reinterpret_cast<const char*>(bytes.data()),
                        bytes.size()},
                    port);
            }
        }
    } catch (const std::exception& e) {
        LogVerbose()(OT_PRETTY_CLASS())(e.what()).Flush();
    }

    return output;
}

auto Asio::Imp::retrieve_address_async(
    const struct Site& site,
    std::shared_ptr<std::promise<OTData>> pPromise) -> void
{
    using HTTP = opentxs::network::asio::HTTP;
    auto alloc = alloc::Default{};
    boost::asio::post(
        io_context_->get(),
        [job = std::allocate_shared<HTTP>(
             alloc,
             site.host,
             site.target,
             *io_context_,
             [this, promise = std::move(pPromise), type = site.response_type](
                 auto&& future) mutable {
                 process_address_query(
                     type, std::move(promise), std::move(future));
             })] { job->Start(); });
}

auto Asio::Imp::retrieve_address_async_ssl(
    const struct Site& site,
    std::shared_ptr<std::promise<OTData>> pPromise) -> void
{
    using HTTPS = opentxs::network::asio::HTTPS;
    auto alloc = alloc::Default{};
    boost::asio::post(
        io_context_->get(),
        [job = std::allocate_shared<HTTPS>(
             alloc,
             site.host,
             site.target,
             *io_context_,
             [this, promise = std::move(pPromise), type = site.response_type](
                 auto&& future) mutable {
                 process_address_query(
                     type, std::move(promise), std::move(future));
             })] { job->Start(); });
}

auto Asio::Imp::retrieve_json_http(
    const ReadView host,
    const ReadView path,
    const ReadView notify,
    std::shared_ptr<std::promise<boost::json::value>> pPromise) const noexcept
    -> void
{
    using HTTP = opentxs::network::asio::HTTP;
    auto alloc = alloc::Default{};
    boost::asio::post(
        io_context_->get(),
        [job = std::allocate_shared<HTTP>(
             alloc,
             host,
             path,
             *io_context_,
             [this,
              promise = std::move(pPromise),
              socket = CString{notify, alloc}](auto&& future) mutable {
                 process_json(socket, std::move(promise), std::move(future));
             })] { job->Start(); });
}

auto Asio::Imp::retrieve_json_https(
    const ReadView host,
    const ReadView path,
    const ReadView notify,
    std::shared_ptr<std::promise<boost::json::value>> pPromise) const noexcept
    -> void
{
    using HTTPS = opentxs::network::asio::HTTPS;
    auto alloc = alloc::Default{};
    boost::asio::post(
        io_context_->get(),
        [job = std::allocate_shared<HTTPS>(
             alloc,
             host,
             path,
             *io_context_,
             [this,
              promise = std::move(pPromise),
              socket = CString{notify, alloc}](auto&& future) mutable {
                 process_json(socket, std::move(promise), std::move(future));
             })] { job->Start(); });
}

auto Asio::Imp::send_notification(const ReadView notify) const noexcept -> void
{
    if (false == valid(notify)) { return; }

    try {
        const auto endpoint = CString{notify};
        auto& socket = [&]() -> auto&
        {
            auto handle = notify_.lock();
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
        const auto rc =
            socket.lock()->Send(MakeWork(OT_ZMQ_STATE_MACHINE_SIGNAL));

        if (false == rc) {
            throw std::runtime_error{"Failed to send notification"};
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return;
    }
}

auto Asio::Imp::Shutdown() noexcept -> void
{
    Stop().get();

    {
        auto lock = eLock{lock_};
        acceptors_.Stop();
        io_context_->Stop();

        for (auto& [type, pool] : thread_pools_) { pool.Stop(); }

        thread_pools_.clear();
        data_socket_->Close();
    }
}

auto Asio::Imp::state_machine() noexcept -> bool
{
    auto again{false};

    {
        auto lock = eLock{lock_};
        ipv4_promise_ = {};
        ipv6_promise_ = {};
        ipv4_future_ = ipv4_promise_.get_future();
        ipv6_future_ = ipv6_promise_.get_future();
    }

    auto futures4 = UnallocatedVector<std::future<OTData>>{};
    auto futures6 = UnallocatedVector<std::future<OTData>>{};

    for (const auto& site : sites) {
        auto promise = std::make_shared<std::promise<OTData>>();

        if (IPversion::IPV4 == site.protocol) {
            futures4.emplace_back(promise->get_future());

            if ("https" == site.service) {
                retrieve_address_async_ssl(site, std::move(promise));
            } else {
                retrieve_address_async(site, std::move(promise));
            }
        } else {
            futures6.emplace_back(promise->get_future());

            if ("https" == site.service) {
                retrieve_address_async_ssl(site, std::move(promise));
            } else {
                retrieve_address_async(site, std::move(promise));
            }
        }
    }

    auto result4 = Data::Factory();
    auto result6 = Data::Factory();
    static constexpr auto limit = 15s;
    static constexpr auto ready = std::future_status::ready;

    for (auto& future : futures4) {
        try {
            if (const auto status = future.wait_for(limit); ready == status) {
                auto result = future.get();

                if (result->empty()) { continue; }

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

                if (result->empty()) { continue; }

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

    if (result4->empty() && result6->empty()) { again = true; }

    {
        auto lock = eLock{lock_};
        ipv4_promise_.set_value(std::move(result4));
        ipv6_promise_.set_value(std::move(result6));
    }

    LogTrace()(OT_PRETTY_CLASS())("Finished checking ip addresses").Flush();

    return again;
}

Asio::Imp::~Imp() { Shutdown(); }
}  // namespace opentxs::api::network
