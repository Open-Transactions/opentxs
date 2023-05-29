// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "api/network/asio/Asio.hpp"         // IWYU pragma: associated
#include "internal/api/network/Factory.hpp"  // IWYU pragma: associated

#include <boost/json.hpp>  // IWYU pragma: keep
#include <boost/smart_ptr/make_shared.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
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
#include "internal/api/session/Endpoints.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Timer.hpp"
#include "internal/util/alloc/Logging.hpp"
#include "network/asio/Socket.hpp"
#include "opentxs/network/asio/Socket.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs::factory
{
auto AsioAPI(const network::zeromq::Context& zmq, bool test) noexcept
    -> std::unique_ptr<api::network::Asio>
{
    using ReturnType = api::network::implementation::Asio;

    return std::make_unique<ReturnType>(zmq, test);
}
}  // namespace opentxs::factory

namespace opentxs::api::network::implementation
{
Asio::Asio(const opentxs::network::zeromq::Context& zmq, bool test) noexcept
    : Asio(boost::make_shared<asio::Shared>(zmq, test), test)
{
}

Asio::Asio(boost::shared_ptr<asio::Shared> shared, const bool test) noexcept
    : test_(test)
    , main_(shared)
    , weak_(main_)
    , acceptors_(*this, *(shared->data_.lock_shared()->io_context_))
{
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

auto Asio::Connect(
    const opentxs::network::zeromq::Envelope& id,
    SocketImp socket) const noexcept -> bool
{
    if (auto p = weak_.lock(); p) {

        return p->Connect(p, id, socket);
    } else {

        return false;
    }
}

auto Asio::IOContext() const noexcept -> boost::asio::io_context&
{
    if (auto p = weak_.lock(); p) {

        return p->IOContext();
    } else {

        OT_FAIL;
    }
}

auto Asio::FetchJson(
    const ReadView host,
    const ReadView path,
    const bool https,
    const ReadView notify) const noexcept -> std::future<boost::json::value>
{
    if (test_) { OT_FAIL; }

    if (auto p = weak_.lock(); p) {

        return p->FetchJson(p, host, path, https, notify);
    } else {

        return {};
    }
}

auto Asio::GetPublicAddress4() const noexcept -> std::shared_future<ByteArray>
{
    if (auto p = weak_.lock(); p) {

        return p->GetPublicAddress4();
    } else {

        return {};
    }
}

auto Asio::GetPublicAddress6() const noexcept -> std::shared_future<ByteArray>
{
    if (auto p = weak_.lock(); p) {

        return p->GetPublicAddress6();
    } else {

        return {};
    }
}

auto Asio::GetTimer() const noexcept -> Timer
{
    if (auto p = weak_.lock(); p) {

        return p->GetTimer();
    } else {

        return {};
    }
}

auto Asio::Init(std::shared_ptr<const api::Context> context) noexcept -> void
{
    auto shared = weak_.lock();

    OT_ASSERT(shared);

    shared->Init();

    OT_ASSERT(context);

    // TODO the version of libc++ present in android ndk 23.0.7599858 has a
    // broken std::allocate_shared function so we're using boost::shared_ptr
    // instead of std::shared_ptr
    auto alloc = alloc::PMR<asio::Shared>{
        shared->zmq_.Internal().Alloc(shared->batch_id_)};
    auto actor = boost::allocate_shared<asio::Actor>(alloc, context, shared);

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

auto Asio::Receive(
    const opentxs::network::zeromq::Envelope& id,
    const OTZMQWorkType type,
    const std::size_t bytes,
    SocketImp socket) const noexcept -> bool
{
    if (auto p = weak_.lock(); p) {

        return p->Receive(p, id, type, bytes, socket);
    } else {

        return {};
    }
}

auto Asio::Shutdown() noexcept -> void
{
    acceptors_.Stop();
    main_.reset();
}

auto Asio::Transmit(
    const opentxs::network::zeromq::Envelope& id,
    const ReadView bytes,
    SocketImp socket) const noexcept -> bool
{
    if (auto p = weak_.lock(); p) {

        return p->Transmit(p, id, bytes, socket);
    } else {

        return {};
    }
}

Asio::~Asio() = default;
}  // namespace opentxs::api::network::implementation
