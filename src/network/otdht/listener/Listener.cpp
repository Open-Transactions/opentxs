// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/otdht/Listener.hpp"  // IWYU pragma: associated

#include <atomic>
#include <cstddef>
#include <string_view>

#include "internal/network/zeromq/Context.hpp"
#include "internal/util/alloc/Logging.hpp"
#include "network/otdht/listener/Actor.hpp"
#include "opentxs/api/Network.hpp"
#include "opentxs/api/Session.internal.hpp"
#include "opentxs/api/network/ZeroMQ.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::network::otdht
{
Listener::Listener(
    std::shared_ptr<const api::internal::Session> api,
    std::shared_ptr<Node::Shared> shared,
    std::string_view routerBind,
    std::string_view routerAdvertise,
    std::string_view publishBind,
    std::string_view publishAdvertise,
    std::string_view routingID,
    std::string_view fromNode) noexcept
    : actor_([&] {
        assert_false(nullptr == api);
        assert_false(nullptr == shared);

        const auto& zmq = api->Network().ZeroMQ().Context().Internal();
        const auto batchID = zmq.PreallocateBatch();

        return std::allocate_shared<Actor>(
            alloc::PMR<Actor>{zmq.Alloc(batchID)},
            api,
            shared,
            routerBind,
            routerAdvertise,
            publishBind,
            publishAdvertise,
            routingID,
            fromNode,
            batchID);
    }())
{
    assert_false(nullptr == actor_);
}

auto Listener::NextID(alloc::Default alloc) noexcept -> CString
{
    static auto counter = std::atomic<std::size_t>{};
    auto out = CString{"OTDHT listener #", alloc};
    out.append(std::to_string(++counter));

    return out;
}

auto Listener::Init() noexcept -> void { actor_->Init(actor_); }

Listener::~Listener() = default;
}  // namespace opentxs::network::otdht
