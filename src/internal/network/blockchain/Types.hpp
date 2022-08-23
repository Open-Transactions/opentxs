// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "internal/blockchain/node/Types.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

namespace opentxs::network::blockchain
{
// WARNING update print function if new values are added or removed
enum class PeerJob : OTZMQWorkType {
    shutdown = value(WorkType::Shutdown),
    blockheader = value(WorkType::BlockchainNewHeader),
    reorg = value(WorkType::BlockchainReorg),
    blockbatch = value(WorkType::BlockchainBlockDownloadQueue),
    mempool = value(WorkType::BlockchainMempoolUpdated),
    registration = value(WorkType::AsioRegister),
    connect = value(WorkType::AsioConnect),
    disconnect = value(WorkType::AsioDisconnect),
    sendresult = value(WorkType::AsioSendResult),
    p2p = value(WorkType::BitcoinP2P),
    broadcasttx =
        value(opentxs::blockchain::node::PeerManagerJobs::BroadcastTransaction),
    jobavailablecfheaders = value(
        opentxs::blockchain::node::PeerManagerJobs::JobAvailableCfheaders),
    jobavailablecfilters =
        value(opentxs::blockchain::node::PeerManagerJobs::JobAvailableCfilters),
    dealerconnected = OT_ZMQ_INTERNAL_SIGNAL + 120,
    jobtimeout = OT_ZMQ_INTERNAL_SIGNAL + 121,
    needpeers = OT_ZMQ_INTERNAL_SIGNAL + 122,
    statetimeout = OT_ZMQ_INTERNAL_SIGNAL + 123,
    activitytimeout = OT_ZMQ_INTERNAL_SIGNAL + 124,
    needping = OT_ZMQ_INTERNAL_SIGNAL + 125,
    body = OT_ZMQ_INTERNAL_SIGNAL + 126,
    header = OT_ZMQ_INTERNAL_SIGNAL + 127,
    jobavailablegetheaders = OT_ZMQ_HEADER_ORACLE_JOB_READY,
    jobavailableblock = OT_ZMQ_BLOCK_BATCH_JOB_AVAILABLE,
    jobavailableblockbatch = OT_ZMQ_BLOCK_FETCH_JOB_AVAILABLE,
    heartbeat = OT_ZMQ_HEARTBEAT_SIGNAL,
    block = OT_ZMQ_NEW_FULL_BLOCK_SIGNAL,
    init = OT_ZMQ_INIT_SIGNAL,
    statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};

// WARNING update print function if new values are added or removed
enum class DHTJob : OTZMQWorkType {
    shutdown = value(WorkType::Shutdown),
    sync_ack = value(WorkType::P2PBlockchainSyncAck),
    sync_reply = value(WorkType::P2PBlockchainSyncReply),
    sync_push = value(WorkType::P2PBlockchainNewBlock),
    response = value(WorkType::P2PResponse),
    push_tx = value(WorkType::P2PPushTransaction),
    job_processed = OT_ZMQ_INTERNAL_SIGNAL + 0,
    registration = OT_ZMQ_REGISTER_SIGNAL,
    init = OT_ZMQ_INIT_SIGNAL,
    cfilter = OT_ZMQ_NEW_FILTER_SIGNAL,
    statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};

auto print(PeerJob) noexcept -> std::string_view;
auto print(DHTJob) noexcept -> std::string_view;
}  // namespace opentxs::network::blockchain
