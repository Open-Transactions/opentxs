// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/otdht/Node.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
#include <functional>  // IWYU pragma: keep
#include <string_view>
#include <utility>

#include "internal/network/otdht/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/alloc/Logging.hpp"
#include "network/otdht/node/Actor.hpp"
#include "network/otdht/node/Shared.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs::network::otdht
{
using namespace std::literals;

auto print(NodeJob in) noexcept -> std::string_view
{
    using enum NodeJob;
    static constexpr auto map =
        frozen::make_unordered_map<NodeJob, std::string_view>({
            {shutdown, "shutdown"sv},
            {chain_state, "chain_state"sv},
            {new_cfilter, "new_cfilter"sv},
            {new_peer, "new_peer"sv},
            {blockchain, "blockchain"sv},
            {add_listener, "add_listener"sv},
            {connect_peer_manager, "connect_peer_manager"sv},
            {disconnect_peer_manager, "disconnect_peer_manager"sv},
            {connect_peer, "connect_peer"sv},
            {disconnect_peer, "disconnect_peer"sv},
            {registration, "registration"sv},
            {init, "init"sv},
            {statemachine, "statemachine"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {
        LogAbort()(__FUNCTION__)(": invalid network::otdht::NodeJob: ")(
            static_cast<OTZMQWorkType>(in))
            .Abort();
    }
}
}  // namespace opentxs::network::otdht

namespace opentxs::network::otdht
{
Node::Node(
    const api::Session& api,
    const ReadView publicKey,
    const Secret& secretKey) noexcept
    : shared_([&] {
        const auto& zmq = api.Network().ZeroMQ().Internal();
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

auto Node::Init(std::shared_ptr<const api::Session> api) noexcept -> void
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
