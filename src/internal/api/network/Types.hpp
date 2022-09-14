// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

namespace opentxs::api::network
{
// WARNING update print function if new values are added or removed
enum class BlockchainJob : OTZMQWorkType {
    shutdown = value(WorkType::Shutdown),
    block_header = value(WorkType::BlockchainNewHeader),
    peer_active = value(WorkType::BlockchainPeerAdded),
    reorg = value(WorkType::BlockchainReorg),
    cfilter_progress = value(WorkType::BlockchainSyncProgress),
    cfilter = value(WorkType::BlockchainNewFilter),
    block_queue = value(WorkType::BlockchainBlockDownloadQueue),
    peer_connected = value(WorkType::BlockchainPeerConnected),
    mempool = value(WorkType::BlockchainMempoolUpdated),
    block_available = value(WorkType::BlockchainBlockAvailable),
    sync_server = value(WorkType::BlockchainSyncServerProgress),
    block = value(WorkType::BlockchainBlockOracleProgress),
    report_status = OT_ZMQ_BLOCKCHAIN_REPORT_STATUS,
    init = OT_ZMQ_INIT_SIGNAL,
    statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};

auto print(BlockchainJob) noexcept -> std::string_view;
}  // namespace opentxs::api::network
