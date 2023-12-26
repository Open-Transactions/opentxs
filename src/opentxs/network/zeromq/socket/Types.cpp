// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/network/zeromq/socket/Types.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
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
}  // namespace opentxs::network::zeromq::socket
