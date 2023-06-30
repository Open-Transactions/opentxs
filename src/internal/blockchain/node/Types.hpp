// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/block/Hash.hpp"

#pragma once

#include <string_view>

#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node
{
enum class ManagerJobs : OTZMQWorkType {
    shutdown = value(WorkType::Shutdown),
    sync_reply = value(WorkType::P2PBlockchainSyncReply),
    sync_new_block = value(WorkType::P2PBlockchainNewBlock),
    heartbeat = OT_ZMQ_INTERNAL_SIGNAL + 3,
    send_to_address = OT_ZMQ_INTERNAL_SIGNAL + 4,
    send_to_paymentcode = OT_ZMQ_INTERNAL_SIGNAL + 5,
    sweep = OT_ZMQ_INTERNAL_SIGNAL + 6,
    start_wallet = OT_ZMQ_INTERNAL_SIGNAL + 7,
    init = OT_ZMQ_INIT_SIGNAL,
    filter_update = OT_ZMQ_NEW_FILTER_SIGNAL,
    statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};  // IWYU pragma: export

enum class PeerManagerJobs : OTZMQWorkType {
    shutdown = value(WorkType::Shutdown),
    resolve = value(WorkType::AsioResolve),
    disconnect = OT_ZMQ_INTERNAL_SIGNAL + 0,
    addpeer = OT_ZMQ_INTERNAL_SIGNAL + 1,
    addlistener = OT_ZMQ_INTERNAL_SIGNAL + 2,
    verifypeer = OT_ZMQ_INTERNAL_SIGNAL + 3,
    spawn_peer = OT_ZMQ_INTERNAL_SIGNAL + 4,
    register_ack = OT_ZMQ_INTERNAL_SIGNAL + 5,
    gossip_address = OT_ZMQ_INTERNAL_SIGNAL + 6,
    broadcasttx = OT_ZMQ_BLOCKCHAIN_BROADCAST_TX,
    report = OT_ZMQ_BLOCKCHAIN_REPORT_STATUS,
    init = OT_ZMQ_INIT_SIGNAL,
    statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};  // IWYU pragma: export

enum class SyncServerJobs : OTZMQWorkType {
    shutdown = value(WorkType::Shutdown),
    heartbeat = OT_ZMQ_HEARTBEAT_SIGNAL,
    filter = OT_ZMQ_NEW_FILTER_SIGNAL,
    statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};  // IWYU pragma: export

enum class StatsJobs : OTZMQWorkType {
    shutdown = value(WorkType::Shutdown),
    block_header = value(WorkType::BlockchainNewHeader),
    reorg = value(WorkType::BlockchainReorg),
    cfilter = value(WorkType::BlockchainNewFilter),
    peer = value(WorkType::BlockchainPeerAdded),
    sync_server = value(WorkType::BlockchainSyncServerProgress),
    block = value(WorkType::BlockchainBlockOracleProgress),
    init = OT_ZMQ_INIT_SIGNAL,
    statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};  // IWYU pragma: export

auto print(ManagerJobs) noexcept -> std::string_view;
auto print(PeerManagerJobs) noexcept -> std::string_view;
auto print(StatsJobs) noexcept -> std::string_view;
}  // namespace opentxs::blockchain::node
