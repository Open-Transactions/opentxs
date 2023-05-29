// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "api/network/asio/Data.hpp"  // IWYU pragma: associated

#include <functional>

#include "api/network/asio/Context.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/SocketType.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/network/zeromq/Context.hpp"

namespace opentxs::api::network::asio
{
using namespace std::literals;

Data::Data(
    const opentxs::network::zeromq::Context& zmq,
    std::string_view endpoint,
    bool test) noexcept
    : to_actor_([&] {
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
    , notify_()
    , resolver_(std::make_shared<Resolver>(io_context_->get()))
{
    OT_ASSERT(io_context_);
    OT_ASSERT(resolver_);

    if (test) {
        ipv4_promise_.set_value({IsHex, "0x7f000001"sv});
        ipv6_promise_.set_value(
            {IsHex, "0x00000000000000000000000000000001"sv});
    }
}

Data::~Data() = default;
}  // namespace opentxs::api::network::asio
