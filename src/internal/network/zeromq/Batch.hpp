// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <string_view>

#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/ReplyCallback.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Container.hpp"

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

using OTZMQListenCallback = Pimpl<network::zeromq::ListenCallback>;
using OTZMQReplyCallback = Pimpl<network::zeromq::ReplyCallback>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::internal
{
class Batch
{
public:
    const BatchID id_;
    const CString thread_name_;
    Vector<OTZMQListenCallback> listen_callbacks_;
    Vector<OTZMQReplyCallback> reply_callbacks_;
    Vector<socket::Raw> sockets_;
    std::atomic_bool toggle_;

    auto ClearCallbacks() noexcept -> void;

    Batch(
        const BatchID id,
        const zeromq::Context& context,
        Vector<socket::Type>&& types,
        const std::string_view threadname) noexcept;
    Batch() = delete;
    Batch(const Batch&) = delete;
    Batch(Batch&&) = delete;
    auto operator=(const Batch&) -> Batch& = delete;
    auto operator=(Batch&&) -> Batch& = delete;

    ~Batch();
};
}  // namespace opentxs::network::zeromq::internal
