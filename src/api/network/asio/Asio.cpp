// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "api/network/asio/Asio.hpp"         // IWYU pragma: associated
#include "internal/api/network/Factory.hpp"  // IWYU pragma: associated

#include <boost/json.hpp>  // IWYU pragma: keep
#include <boost/smart_ptr/make_shared.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <frozen/bits/algorithms.h>
#include <frozen/bits/basic_types.h>
#include <frozen/unordered_map.h>
#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <string_view>
#include <utility>

#include "BoostAsio.hpp"
#include "api/network/asio/Acceptors.hpp"
#include "api/network/asio/Actor.hpp"
#include "api/network/asio/Context.hpp"
#include "api/network/asio/Data.hpp"
#include "api/network/asio/Shared.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Timer.hpp"
#include "network/asio/Socket.hpp"
#include "opentxs/network/asio/Socket.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs
{
auto print(ThreadPool value) noexcept -> std::string_view
{
    using namespace std::literals;
    using enum opentxs::ThreadPool;
    static constexpr auto map =
        frozen::make_unordered_map<ThreadPool, std::string_view>({
            {General, "General"sv},
            {Network, "Network"sv},
            {Storage, "Storage"sv},
            {Blockchain, "Blockchain"sv},
        });

    try {
        return map.at(value);
    } catch (...) {

        return "Unknown ThreadPool type"sv;
    }
}
}  // namespace opentxs

namespace opentxs::factory
{
auto AsioAPI(const network::zeromq::Context& zmq) noexcept
    -> std::unique_ptr<api::network::Asio>
{
    using ReturnType = api::network::implementation::Asio;

    return std::make_unique<ReturnType>(zmq);
}
}  // namespace opentxs::factory

namespace opentxs::api::network::implementation
{
Asio::Asio(const opentxs::network::zeromq::Context& zmq) noexcept
    : shared_p_([&] {
        const auto batchID = zmq.Internal().PreallocateBatch();
        auto* alloc = zmq.Internal().Alloc(batchID);
        // TODO the version of libc++ present in android ndk 23.0.7599858 has a
        // broken std::allocate_shared function so we're using boost::shared_ptr
        // instead of std::shared_ptr

        return boost::allocate_shared<asio::Shared>(
            alloc::PMR<asio::Shared>{alloc}, zmq, batchID);
    }())
    , shared_(*shared_p_)
    , acceptors_(*this, *(shared_.data_.lock_shared()->io_context_))
{
    OT_ASSERT(shared_p_);
}

auto Asio::Accept(const Endpoint& endpoint, AcceptCallback cb) const noexcept
    -> bool
{
    return acceptors_.Start(endpoint, std::move(cb));
}

auto Asio::Close(const Endpoint& endpoint) const noexcept -> bool
{
    return acceptors_.Close(endpoint);
}

auto Asio::Connect(const ReadView id, SocketImp socket) const noexcept -> bool
{
    return shared_.Connect(shared_p_, id, socket);
}

auto Asio::IOContext() const noexcept -> boost::asio::io_context&
{
    return shared_.IOContext();
}

auto Asio::FetchJson(
    const ReadView host,
    const ReadView path,
    const bool https,
    const ReadView notify) const noexcept -> std::future<boost::json::value>
{
    return shared_.FetchJson(shared_p_, host, path, https, notify);
}

auto Asio::GetPublicAddress4() const noexcept -> std::shared_future<ByteArray>
{
    return shared_.GetPublicAddress4();
}

auto Asio::GetPublicAddress6() const noexcept -> std::shared_future<ByteArray>
{
    return shared_.GetPublicAddress6();
}

auto Asio::GetTimer() const noexcept -> Timer { return shared_.GetTimer(); }

auto Asio::Init(std::shared_ptr<const api::Context> context) noexcept -> void
{
    shared_.Init();

    OT_ASSERT(context);

    // TODO the version of libc++ present in android ndk 23.0.7599858 has a
    // broken std::allocate_shared function so we're using boost::shared_ptr
    // instead of std::shared_ptr
    auto actor = boost::allocate_shared<asio::Actor>(
        alloc::PMR<asio::Actor>{shared_.get_allocator()}, context, shared_p_);

    OT_ASSERT(actor);

    actor->Init(actor);
}

auto Asio::MakeSocket(const Endpoint& endpoint) const noexcept
    -> opentxs::network::asio::Socket
{
    using Imp = opentxs::network::asio::Socket::Imp;
    using Shared = std::shared_ptr<Imp>;

    return {[&]() -> void* {
        return std::make_unique<Shared>(
                   std::make_shared<Imp>(endpoint, *const_cast<Asio*>(this)))
            .release();
    }};
}

auto Asio::NotificationEndpoint() const noexcept -> std::string_view
{
    return session::internal::Endpoints::Asio();
}

auto Asio::Post(
    ThreadPool type,
    internal::Asio::Callback cb,
    std::string_view threadName) const noexcept -> bool
{
    return shared_.Post(type, cb, threadName);
}

auto Asio::Receive(
    const ReadView id,
    const OTZMQWorkType type,
    const std::size_t bytes,
    SocketImp socket) const noexcept -> bool
{
    return shared_.Receive(shared_p_, id, type, bytes, socket);
}

auto Asio::Shutdown() noexcept -> void
{
    acceptors_.Stop();
    shared_.Shutdown();
}

auto Asio::Transmit(const ReadView id, const ReadView bytes, SocketImp socket)
    const noexcept -> bool
{
    return shared_.Transmit(shared_p_, id, bytes, socket);
}

Asio::~Asio() { Shutdown(); }
}  // namespace opentxs::api::network::implementation
