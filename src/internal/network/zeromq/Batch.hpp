// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <mutex>

#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/ReplyCallback.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
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
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::internal
{
class Batch
{
public:
    const BatchID id_;
    Vector<OTZMQListenCallback> listen_callbacks_;
    Vector<OTZMQReplyCallback> reply_callbacks_;
    Vector<socket::Raw> sockets_;
    std::atomic_bool toggle_;

    auto ClearCallbacks() noexcept -> void;

    Batch(
        const BatchID id,
        const zeromq::Context& context,
        Vector<socket::Type>&& types) noexcept;

    ~Batch();

private:
    Batch() = delete;
    Batch(const Batch&) = delete;
    Batch(Batch&&) = delete;
    auto operator=(const Batch&) -> Batch& = delete;
    auto operator=(Batch&&) -> Batch& = delete;
};
}  // namespace opentxs::network::zeromq::internal
