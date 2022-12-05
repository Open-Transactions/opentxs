// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/otdht/Listener.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <atomic>
#include <cstddef>
#include <string_view>

#include "internal/network/otdht/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/LogMacros.hpp"
#include "network/otdht/listener/Actor.hpp"
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

auto print(ListenerJob job) noexcept -> std::string_view
{
    try {
        using Job = ListenerJob;
        static const auto map = Map<Job, std::string_view>{
            {Job::shutdown, "shutdown"sv},
            {Job::chain_state, "chain_state"sv},
            {Job::sync_request, "sync_request"sv},
            {Job::sync_reply, "sync_reply"sv},
            {Job::sync_push, "sync_push"sv},
            {Job::sync_query, "sync_query"sv},
            {Job::response, "response"sv},
            {Job::publish_contract, "publish_contract"sv},
            {Job::query_contract, "query_contract"sv},
            {Job::pushtx, "pushtx"sv},
            {Job::registration, "registration"sv},
            {Job::init, "init"sv},
            {Job::statemachine, "statemachine"sv},
        };

        return map.at(job);
    } catch (...) {
        LogAbort()(__FUNCTION__)("invalid ListenerJob: ")(
            static_cast<OTZMQWorkType>(job))
            .Abort();
    }
}
}  // namespace opentxs::network::otdht

namespace opentxs::network::otdht
{
Listener::Listener(
    std::shared_ptr<const api::Session> api,
    boost::shared_ptr<Node::Shared> shared,
    std::string_view routerBind,
    std::string_view routerAdvertise,
    std::string_view publishBind,
    std::string_view publishAdvertise,
    std::string_view routingID,
    std::string_view fromNode) noexcept
    : actor_([&] {
        OT_ASSERT(api);
        OT_ASSERT(shared);

        const auto& zmq = api->Network().ZeroMQ().Internal();
        const auto batchID = zmq.PreallocateBatch();
        // TODO the version of libc++ present in android ndk 23.0.7599858 has a
        // broken std::allocate_shared function so we're using boost::shared_ptr
        // instead of std::shared_ptr

        return boost::allocate_shared<Actor>(
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
    OT_ASSERT(actor_);
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
