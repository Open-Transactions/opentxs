// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/otdht/Node.hpp"  // IWYU pragma: associated

#include <functional>  // IWYU pragma: keep
#include <utility>

#include "internal/network/zeromq/Context.hpp"
#include "internal/util/alloc/Logging.hpp"
#include "network/otdht/node/Actor.hpp"
#include "network/otdht/node/Shared.hpp"
#include "opentxs/api/Network.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/network/ZeroMQ.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::network::otdht
{
Node::Node(
    const api::Session& api,
    const ReadView publicKey,
    const Secret& secretKey) noexcept
    : shared_([&] {
        const auto& zmq = api.Network().ZeroMQ().Context().Internal();
        const auto batchID = zmq.PreallocateBatch();

        return std::allocate_shared<Shared>(
            alloc::PMR<Shared>{zmq.Alloc(batchID)},
            batchID,
            publicKey,
            secretKey);
    }())
{
    assert_false(nullptr == shared_);
}

auto Node::get_allocator() const noexcept -> allocator_type
{
    return shared_->get_allocator();
}

auto Node::Init(std::shared_ptr<const api::internal::Session> api) noexcept
    -> void
{
    auto actor = std::allocate_shared<Actor>(
        alloc::PMR<Actor>{get_allocator()},
        std::move(api),
        shared_,
        shared_->batch_id_);

    assert_false(nullptr == actor);

    actor->Init(actor);
}

Node::~Node() = default;
}  // namespace opentxs::network::otdht
