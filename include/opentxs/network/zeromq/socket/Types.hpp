// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/network/zeromq/socket/SocketType.hpp"

#pragma once

#include <cstdint>
#include <string_view>
#include <tuple>
#include <utility>

#include "opentxs/Export.hpp"  // IWYU pragma: keep
#include "opentxs/util/Multiple.hpp"
#include "opentxs/util/Types.hpp"

namespace opentxs::network::zeromq::socket
{
enum class Direction : std::uint8_t;  // IWYU pragma: export
enum class Policy : std::uint8_t;     // IWYU pragma: export
enum class Type : std::uint8_t;       // IWYU pragma: export

using EndpointRequest = std::pair<std::string_view, Direction>;
using EndpointRequests = Multiple<EndpointRequest>;
using SocketRequest = std::tuple<Type, Policy, EndpointRequests>;
using SocketRequests = Multiple<SocketRequest>;
using LocalSeckey = ReadView;
using LocalPubkey = ReadView;
using RemotePubkey = ReadView;
using Domain = std::string_view;
using CurveClientRequest = std::tuple<
    Type,
    LocalSeckey,
    LocalPubkey,
    RemotePubkey,
    Domain,
    EndpointRequests>;
using CurveClientRequests = Multiple<CurveClientRequest>;
using CurveServerRequest =
    std::tuple<Type, LocalSeckey, Domain, EndpointRequests>;
using CurveServerRequests = Multiple<CurveServerRequest>;

OPENTXS_EXPORT auto print(Direction) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(Policy) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(Type) noexcept -> std::string_view;
}  // namespace opentxs::network::zeromq::socket
