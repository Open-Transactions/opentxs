// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <compare>
#include <functional>
#include <future>
#include <span>
#include <tuple>
#include <utility>

#include "opentxs/Types.hpp"
#include "opentxs/WorkType.hpp"  // IWYU pragma: keep
#include "opentxs/WorkType.internal.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class Raw;
}  // namespace socket

class Frame;
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq
{
using ReceiveCallback = std::function<void(Message&&)>;
using ModifyCallback = std::function<void(socket::Raw&)>;
using ThreadStartArgs = Vector<std::pair<socket::Raw*, ReceiveCallback>>;
using StartArgs = Vector<std::tuple<SocketID, socket::Raw*, ReceiveCallback>>;
using AsyncResult = std::pair<bool, std::future<bool>>;

enum class Operation : OTZMQWorkType {
    add_socket = OT_ZMQ_INTERNAL_SIGNAL + 0,
    remove_socket = OT_ZMQ_INTERNAL_SIGNAL + 1,
    change_socket = OT_ZMQ_INTERNAL_SIGNAL + 2,
    shutdown = OT_ZMQ_INTERNAL_SIGNAL + 3,
};  // IWYU pragma: export

auto GetBatchID() noexcept -> BatchID;
auto GetSocketID() noexcept -> SocketID;
auto operator==(std::span<const Frame> lhs, std::span<const Frame> rhs) noexcept
    -> bool;
auto operator<=>(
    std::span<const Frame> lhs,
    std::span<const Frame> rhs) noexcept -> std::strong_ordering;
}  // namespace opentxs::network::zeromq
