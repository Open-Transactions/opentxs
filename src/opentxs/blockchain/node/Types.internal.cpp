// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/blockchain/node/Types.internal.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
#include <functional>  // IWYU pragma: keep
#include <utility>

#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::node
{
using namespace std::literals;

auto print(ManagerJobs in) noexcept -> std::string_view
{
    using enum ManagerJobs;
    static constexpr auto map =
        frozen::make_unordered_map<ManagerJobs, std::string_view>({
            {shutdown, "shutdown"sv},
            {sync_reply, "sync_reply"sv},
            {sync_new_block, "sync_new_block"sv},
            {heartbeat, "heartbeat"sv},
            {start_wallet, "start_wallet"sv},
            {init, "init"sv},
            {filter_update, "filter_update"sv},
            {statemachine, "statemachine"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {
        LogAbort()(__FUNCTION__)(": invalid node::ManagerJobs: ")(
            static_cast<OTZMQWorkType>(in))
            .Abort();
    }
}

auto print(PeerManagerJobs in) noexcept -> std::string_view
{
    using enum PeerManagerJobs;
    static constexpr auto map =
        frozen::make_unordered_map<PeerManagerJobs, std::string_view>({
            {shutdown, "shutdown"sv},
            {resolve, "resolve"sv},
            {disconnect, "disconnect"sv},
            {addpeer, "addpeer"sv},
            {addlistener, "addlistener"sv},
            {verifypeer, "verifypeer"sv},
            {spawn_peer, "spawn_peer"sv},
            {register_ack, "register_ack"sv},
            {gossip_address, "gossip_address"sv},
            {broadcasttx, "broadcasttx"sv},
            {report, "report"sv},
            {init, "init"sv},
            {statemachine, "statemachine"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {
        LogAbort()(__FUNCTION__)(": invalid PeerManagerJobs: ")(
            static_cast<OTZMQWorkType>(in))
            .Abort();
    }
}

auto print(StatsJobs in) noexcept -> std::string_view
{
    using enum StatsJobs;
    static constexpr auto map =
        frozen::make_unordered_map<StatsJobs, std::string_view>({
            {shutdown, "shutdown"sv},
            {block_header, "block_header"sv},
            {reorg, "reorg"sv},
            {cfilter, "cfilter"sv},
            {peer, "peer"sv},
            {sync_server, "sync_server"sv},
            {block, "block"sv},
            {init, "init"sv},
            {statemachine, "statemachine"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {
        LogAbort()(__FUNCTION__)(": invalid StatsJobs: ")(
            static_cast<OTZMQWorkType>(in))
            .Abort();
    }
}
}  // namespace opentxs::blockchain::node
