// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>
#include <string_view>

#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::filteroracle
{
using NotifyCallback =
    std::function<void(const cfilter::Type, const block::Position&)>;

enum class DownloadJob : OTZMQWorkType {
    shutdown = value(WorkType::Shutdown),
    block = value(WorkType::BlockchainNewHeader),
    reorg = value(WorkType::BlockchainReorg),
    reset_filter_tip = OT_ZMQ_INTERNAL_SIGNAL + 0,
    heartbeat = OT_ZMQ_HEARTBEAT_SIGNAL,
    full_block = OT_ZMQ_NEW_FULL_BLOCK_SIGNAL,
    statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};  // IWYU pragma: export

// WARNING update print function if new values are added or removed
enum class BlockIndexerJob : OTZMQWorkType {
    shutdown = value(WorkType::Shutdown),
    reindex = OT_ZMQ_INTERNAL_SIGNAL + 0,
    job_finished = OT_ZMQ_INTERNAL_SIGNAL + 1,
    report = OT_ZMQ_BLOCKCHAIN_REPORT_STATUS,
    reorg = OT_ZMQ_REORG_SIGNAL,
    header = OT_ZMQ_NEW_BLOCK_HEADER_SIGNAL,
    block_ready = OT_ZMQ_BLOCK_ORACLE_BLOCK_READY,
    init = OT_ZMQ_INIT_SIGNAL,
    statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};  // IWYU pragma: export

auto print(BlockIndexerJob) noexcept -> std::string_view;
}  // namespace opentxs::blockchain::node::filteroracle
