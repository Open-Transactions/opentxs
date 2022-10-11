// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"               // IWYU pragma: associated
#include "api/network/asio/Data.hpp"  // IWYU pragma: associated

#include <utility>

#include "api/network/asio/Context.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/SocketType.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/network/zeromq/Context.hpp"

namespace opentxs::api::network::asio
{
Data::Data(
    const opentxs::network::zeromq::Context& zmq,
    std::string_view endpoint,
    allocator_type alloc) noexcept
    : Allocated(std::move(alloc))
    , to_actor_([&] {
        using SocketType = opentxs::network::zeromq::socket::Type;
        auto out = zmq.Internal().RawSocket(SocketType::Push);
        const auto rc = out.Connect(endpoint.data());

        OT_ASSERT(rc);

        return out;
    }())
    , ipv4_promise_()
    , ipv6_promise_()
    , ipv4_future_(ipv4_promise_.get_future())
    , ipv6_future_(ipv6_promise_.get_future())
    , io_context_(std::make_shared<asio::Context>())
    , thread_pools_([&] {
        auto out = Map<ThreadPool, asio::Context>{get_allocator()};
        out[ThreadPool::General];
        out[ThreadPool::Storage];
        out[ThreadPool::Blockchain];

        return out;
    }())
    , buffers_()
    , notify_(get_allocator())
    , resolver_(std::make_shared<Resolver>(io_context_->get()))
{
    OT_ASSERT(io_context_);
    OT_ASSERT(resolver_);
}

Data::~Data() = default;
}  // namespace opentxs::api::network::asio
