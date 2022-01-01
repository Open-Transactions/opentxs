// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <mutex>
#include <vector>

#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/ReplyCallback.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Pimpl.hpp"

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

class Context;
class ListenCallback;
class ReplyCallback;
}  // namespace zeromq
}  // namespace network

using OTZMQListenCallback = Pimpl<network::zeromq::ListenCallback>;
using OTZMQReplyCallback = Pimpl<network::zeromq::ReplyCallback>;
}  // namespace opentxs

namespace opentxs::network::zeromq::internal
{
class Batch
{
public:
    const BatchID id_;
    std::pmr::vector<OTZMQListenCallback> listen_callbacks_;
    std::pmr::vector<OTZMQReplyCallback> reply_callbacks_;
    std::pmr::vector<socket::Raw> sockets_;

    auto ClearCallbacks() noexcept -> void;

    Batch(
        const BatchID id,
        const zeromq::Context& context,
        std::pmr::vector<socket::Type>&& types) noexcept;

    ~Batch();

private:
    Batch() = delete;
    Batch(const Batch&) = delete;
    Batch(Batch&&) = delete;
    auto operator=(const Batch&) -> Batch& = delete;
    auto operator=(Batch&&) -> Batch& = delete;
};
}  // namespace opentxs::network::zeromq::internal
