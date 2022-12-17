// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <functional>
#include <future>
#include <tuple>
#include <utility>

#include "internal/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

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

class FrameSection;
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq
{
using BatchID = std::size_t;
using SocketID = std::size_t;
using ReceiveCallback = std::function<void(Message&&)>;
using ModifyCallback = std::function<void(socket::Raw&)>;
using ThreadStartArgs = Vector<std::pair<socket::Raw*, ReceiveCallback>>;
using StartArgs = Vector<std::tuple<SocketID, socket::Raw*, ReceiveCallback>>;
using AsyncResult = std::pair<bool, std::future<bool>>;
using EndpointArg = std::pair<CString, socket::Direction>;
using EndpointArgs = Vector<EndpointArg>;
using SocketData = std::pair<socket::Type, EndpointArgs>;

enum class Operation : OTZMQWorkType {
    add_socket = OT_ZMQ_INTERNAL_SIGNAL + 0,
    remove_socket = OT_ZMQ_INTERNAL_SIGNAL + 1,
    change_socket = OT_ZMQ_INTERNAL_SIGNAL + 2,
    shutdown = OT_ZMQ_INTERNAL_SIGNAL + 3,
};  // IWYU pragma: export

auto check_frame_count(
    const FrameSection& body,
    std::size_t required,
    alloc::Default alloc) noexcept(false) -> void;
[[nodiscard]] auto check_frame_count(
    const FrameSection& body,
    std::size_t required) noexcept -> bool;
auto GetBatchID() noexcept -> BatchID;
auto GetSocketID() noexcept -> SocketID;
}  // namespace opentxs::network::zeromq
