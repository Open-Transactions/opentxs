// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/network/zeromq/socket/SocketType.hpp"

#pragma once

#include "opentxs/network/zeromq/socket/Types.hpp"  // IWYU pragma: keep

namespace opentxs::network::zeromq::socket
{
auto to_native(const Type) noexcept -> int;
auto zmq_close_wrapper(void* socket) noexcept -> int;
auto zmq_socket_wrapper(void* context, int type) noexcept -> void*;
}  // namespace opentxs::network::zeromq::socket
