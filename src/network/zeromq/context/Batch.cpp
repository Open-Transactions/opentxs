// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                       // IWYU pragma: associated
#include "1_Internal.hpp"                     // IWYU pragma: associated
#include "internal/network/zeromq/Batch.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iterator>

#include "internal/network/zeromq/socket/Factory.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/ReplyCallback.hpp"

namespace opentxs::network::zeromq::internal
{
Batch::Batch(
    const BatchID id,
    const zeromq::Context& context,
    std::pmr::vector<socket::Type>&& types) noexcept
    : id_(id)
    , listen_callbacks_()
    , reply_callbacks_()
    , sockets_()
{
    sockets_.reserve(types.size());
    std::transform(
        types.begin(),
        types.end(),
        std::back_inserter(sockets_),
        [&](auto type) { return factory::ZMQSocket(context, type); });
}

auto Batch::ClearCallbacks() noexcept -> void
{
    for (auto& cb : listen_callbacks_) { cb->Deactivate(); }

    for (auto& cb : reply_callbacks_) { cb->Deactivate(); }
}

Batch::~Batch() = default;
}  // namespace opentxs::network::zeromq::internal
