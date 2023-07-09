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
#include "internal/util/alloc/Logging.hpp"
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
        using enum ListenerJob;
        static const auto map = Map<ListenerJob, std::string_view>{
            {shutdown, "shutdown"sv},
            {chain_state, "chain_state"sv},
            {sync_request, "sync_request"sv},
            {sync_reply, "sync_reply"sv},
            {sync_push, "sync_push"sv},
            {sync_query, "sync_query"sv},
            {response, "response"sv},
            {publish_contract, "publish_contract"sv},
            {query_contract, "query_contract"sv},
            {pushtx, "pushtx"sv},
            {registration, "registration"sv},
            {init, "init"sv},
            {statemachine, "statemachine"sv},
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

auto Listener::NextID(alloc::Strategy alloc) noexcept -> CString
{
    static auto counter = std::atomic<std::size_t>{};
    auto out = CString{"OTDHT listener #", alloc.result_};
    out.append(std::to_string(++counter));

    return out;
}

auto Listener::Init() noexcept -> void { actor_->Init(actor_); }

Listener::~Listener() = default;
}  // namespace opentxs::network::otdht
