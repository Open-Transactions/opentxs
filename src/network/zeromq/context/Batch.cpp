// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/zeromq/Batch.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <functional>
#include <iterator>

#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/ReplyCallback.hpp"
#include "internal/network/zeromq/socket/Factory.hpp"

namespace opentxs::network::zeromq::internal
{
Batch::Batch(
    const BatchID id,
    const zeromq::Context& context,
    Vector<socket::Type>&& types,
    const std::string_view threadname) noexcept
    : id_(id)
    , thread_name_(threadname)
    , listen_callbacks_()
    , reply_callbacks_()
    , sockets_()
    , toggle_(false)
{
    sockets_.reserve(types.size());
    std::ranges::transform(types, std::back_inserter(sockets_), [&](auto type) {
        return factory::ZMQSocket(context, type);
    });
}

auto Batch::ClearCallbacks() noexcept -> void
{
    for (auto& cb : listen_callbacks_) { cb->Deactivate(); }

    for (auto& cb : reply_callbacks_) { cb->Deactivate(); }
}

Batch::~Batch() = default;
}  // namespace opentxs::network::zeromq::internal
