// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/zeromq/socket/Types.hpp"  // IWYU pragma: associated
#include "opentxs/network/zeromq/socket/Types.hpp"   // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
#include <zmq.h>
#include <atomic>
#include <cstddef>
#include <string_view>

#include "opentxs/network/zeromq/socket/Direction.hpp"   // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Policy.hpp"      // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep

namespace opentxs::network::zeromq::socket
{
using namespace std::literals;

auto print(Direction in) noexcept -> std::string_view
{
    using enum Direction;
    static constexpr auto map =
        frozen::make_unordered_map<Direction, std::string_view>({
            {Bind, "Bind"sv},
            {Connect, "Connect"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown network::zeromq::socket::Direction"sv;
    }
}

auto print(Policy in) noexcept -> std::string_view
{
    using enum Policy;
    static constexpr auto map =
        frozen::make_unordered_map<Policy, std::string_view>({
            {Internal, "Internal"sv},
            {External, "External"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown network::zeromq::socket::Policy"sv;
    }
}

auto print(Type in) noexcept -> std::string_view
{
    using enum Type;
    static constexpr auto map =
        frozen::make_unordered_map<Type, std::string_view>({
            {Error, "Error"sv},
            {Request, "ZMQ_REQ"},
            {Reply, "ZMQ_REP"},
            {Publish, "ZMQ_PUB"},
            {Subscribe, "ZMQ_SUB"},
            {Pull, "ZMQ_PULL"},
            {Push, "ZMQ_PUSH"},
            {Pair, "ZMQ_PAIR"},
            {Dealer, "ZMQ_DEALER"},
            {Router, "ZMQ_ROUTER"},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown network::zeromq::socket::Type"sv;
    }
}

auto to_native(const Type type) noexcept -> int
{
    using enum Type;

    static const auto map = frozen::make_unordered_map<Type, int>({
        {Request, ZMQ_REQ},
        {Reply, ZMQ_REP},
        {Publish, ZMQ_PUB},
        {Subscribe, ZMQ_SUB},
        {Pull, ZMQ_PULL},
        {Push, ZMQ_PUSH},
        {Pair, ZMQ_PAIR},
        {Dealer, ZMQ_DEALER},
        {Router, ZMQ_ROUTER},
    });

    try {

        return map.at(type);
    } catch (...) {

        return 0;
    }
}

static auto zmq_socket_counter() noexcept -> std::atomic<std::ptrdiff_t>&
{
    static auto counter = std::atomic<std::ptrdiff_t>{0};

    return counter;
}

auto zmq_close_wrapper(void* socket) noexcept -> int
{
    const auto out = ::zmq_close(socket);
    --zmq_socket_counter();
    // TODO add optional logging

    return out;
}

auto zmq_socket_wrapper(void* context, int type) noexcept -> void*
{
    auto* const out = ::zmq_socket(context, type);
    ++zmq_socket_counter();
    // TODO add optional logging

    return out;
}
}  // namespace opentxs::network::zeromq::socket
