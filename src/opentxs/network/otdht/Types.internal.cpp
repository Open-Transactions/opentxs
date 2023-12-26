// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/network/otdht/Types.internal.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
#include <string_view>
#include <utility>

#include "opentxs/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::network::otdht
{
using namespace std::literals;

auto print(Job in) noexcept -> std::string_view
{
    using namespace std::literals;
    using enum Job;
    static constexpr auto map =
        frozen::make_unordered_map<Job, std::string_view>({
            {Shutdown, "Shutdown"sv},
            {BlockHeader, "BlockHeader"sv},
            {Reorg, "Reorg"sv},
            {SyncServerUpdated, "SyncServerUpdated"sv},
            {SyncAck, "SyncAck"sv},
            {SyncReply, "SyncReply"sv},
            {SyncPush, "SyncPush"sv},
            {Response, "Response"sv},
            {PublishContract, "PublishContract"sv},
            {QueryContract, "QueryContract"sv},
            {PushTransaction, "PushTransaction"sv},
            {Register, "Register"sv},
            {Request, "Request"sv},
            {Processed, "Processed"sv},
            {ReorgInternal, "ReorgInternal"sv},
            {NewHeaderTip, "NewHeaderTip"sv},
            {Init, "Init"sv},
            {NewCFilterTip, "NewCFilterTip"sv},
            {StateMachine, "StateMachine"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {
        LogAbort()(__FUNCTION__)("invalid Job: ")(
            static_cast<OTZMQWorkType>(in))
            .Abort();
    }
}

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
