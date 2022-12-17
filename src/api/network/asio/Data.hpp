// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_plain_guarded.h>
#include <future>
#include <memory>
#include <string_view>

#include "BoostAsio.hpp"
#include "api/network/asio/Buffers.hpp"
#include "api/network/asio/Context.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Allocated.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{

namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::network::asio
{
class Data final : public opentxs::implementation::Allocated
{
public:
    using GuardedSocket =
        libguarded::plain_guarded<opentxs::network::zeromq::socket::Raw>;
    using NotificationMap = Map<CString, GuardedSocket>;
    using GuardedNotifications = libguarded::plain_guarded<NotificationMap>;
    using Resolver = boost::asio::ip::tcp::resolver;

    opentxs::network::zeromq::socket::Raw to_actor_;
    std::promise<ByteArray> ipv4_promise_;
    std::promise<ByteArray> ipv6_promise_;
    std::shared_future<ByteArray> ipv4_future_;
    std::shared_future<ByteArray> ipv6_future_;
    mutable std::shared_ptr<asio::Context> io_context_;
    mutable Map<ThreadPool, asio::Context> thread_pools_;
    mutable Buffers buffers_;
    mutable GuardedNotifications notify_;
    std::shared_ptr<Resolver> resolver_;

    Data(
        const opentxs::network::zeromq::Context& zmq,
        std::string_view endpoint,
        allocator_type alloc) noexcept;
    Data() = delete;
    Data(const Data&) = delete;
    Data(Data&&) = delete;
    auto operator=(const Data&) -> Data& = delete;
    auto operator=(Data&&) -> Data& = delete;

    ~Data() final;
};
}  // namespace opentxs::api::network::asio
