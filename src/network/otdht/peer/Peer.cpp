// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/otdht/Peer.hpp"  // IWYU pragma: associated

#include <atomic>
#include <cstddef>
#include <string_view>

#include "internal/network/otdht/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/alloc/Logging.hpp"
#include "network/otdht/peer/Actor.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs::network::otdht
{
using namespace std::literals;

auto print(PeerJob job) noexcept -> std::string_view
{
    try {
        using enum PeerJob;
        static const auto map = Map<PeerJob, std::string_view>{
            {shutdown, "shutdown"sv},
            {chain_state, "chain_state"sv},
            {sync_request, "sync_request"sv},
            {sync_ack, "sync_ack"sv},
            {sync_reply, "sync_reply"sv},
            {sync_push, "sync_push"sv},
            {response, "response"sv},
            {push_tx, "push_tx"sv},
            {registration, "registration"sv},
            {init, "init"sv},
            {statemachine, "statemachine"sv},
        };

        return map.at(job);
    } catch (...) {
        LogAbort()(__FUNCTION__)("invalid PeerJob: ")(
            static_cast<OTZMQWorkType>(job))
            .Abort();
    }
}
}  // namespace opentxs::network::otdht

namespace opentxs::network::otdht
{
Peer::Peer(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<Node::Shared> shared,
    std::string_view routingID,
    std::string_view toRemote,
    std::string_view fromNode) noexcept
    : actor_([&] {
        assert_false(nullptr == api);
        assert_false(nullptr == shared);

        const auto& zmq = api->Network().ZeroMQ().Internal();
        const auto batchID = zmq.PreallocateBatch();

        return std::allocate_shared<Actor>(
            alloc::PMR<Actor>{zmq.Alloc(batchID)},
            api,
            shared,
            routingID,
            toRemote,
            fromNode,
            batchID);
    }())
{
    assert_false(nullptr == actor_);
}

auto Peer::NextID(alloc::Default alloc) noexcept -> CString
{
    static auto counter = std::atomic<std::size_t>{};
    auto out = CString{"OTDHT peer #", alloc};
    out.append(std::to_string(++counter));

    return out;
}

auto Peer::Init() noexcept -> void { actor_->Init(actor_); }

Peer::~Peer() = default;
}  // namespace opentxs::network::otdht
