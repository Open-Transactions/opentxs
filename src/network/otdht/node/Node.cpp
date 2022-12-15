// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/otdht/Node.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <string_view>
#include <utility>

#include "internal/network/otdht/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/LogMacros.hpp"
#include "network/otdht/node/Actor.hpp"
#include "network/otdht/node/Shared.hpp"
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

auto print(NodeJob job) noexcept -> std::string_view
{
    try {
        using enum NodeJob;
        static const auto map = Map<NodeJob, std::string_view>{
            {shutdown, "shutdown"sv},
            {chain_state, "chain_state"sv},
            {new_cfilter, "new_cfilter"sv},
            {new_peer, "new_peer"sv},
            {add_listener, "add_listener"sv},
            {registration, "registration"sv},
            {init, "init"sv},
            {statemachine, "statemachine"sv},
        };

        return map.at(job);
    } catch (...) {
        LogAbort()(__FUNCTION__)("invalid NodeJob: ")(
            static_cast<OTZMQWorkType>(job))
            .Abort();
    }
}
}  // namespace opentxs::network::otdht

namespace opentxs::network::otdht
{
Node::Node(const api::Session& api) noexcept
    : shared_([&] {
        const auto& zmq = api.Network().ZeroMQ().Internal();
        const auto batchID = zmq.PreallocateBatch();
        // TODO the version of libc++ present in android ndk 23.0.7599858 has a
        // broken std::allocate_shared function so we're using boost::shared_ptr
        // instead of std::shared_ptr

        return boost::allocate_shared<Shared>(
            alloc::PMR<Shared>{zmq.Alloc(batchID)}, api, batchID);
    }())
{
    OT_ASSERT(shared_);
}

auto Node::get_allocator() const noexcept -> allocator_type
{
    return shared_->get_allocator();
}

auto Node::Init(std::shared_ptr<const api::Session> api) noexcept -> void
{
    // TODO the version of libc++ present in android ndk 23.0.7599858 has a
    // broken std::allocate_shared function so we're using boost::shared_ptr
    // instead of std::shared_ptr
    auto actor = boost::allocate_shared<Actor>(
        alloc::PMR<Actor>{get_allocator()},
        std::move(api),
        shared_,
        shared_->batch_id_);

    OT_ASSERT(actor);

    actor->Init(actor);
}

Node::~Node() = default;
}  // namespace opentxs::network::otdht
