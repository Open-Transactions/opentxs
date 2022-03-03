// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

namespace opentxs::network::p2p
{
enum class Job : OTZMQWorkType {
    Shutdown = value(WorkType::Shutdown),
    BlockHeader = value(WorkType::BlockchainNewHeader),
    Reorg = value(WorkType::BlockchainReorg),
    SyncServerUpdated = value(WorkType::SyncServerUpdated),
    SyncAck = value(WorkType::P2PBlockchainSyncAck),
    SyncReply = value(WorkType::P2PBlockchainSyncReply),
    SyncPush = value(WorkType::P2PBlockchainNewBlock),
    Response = value(WorkType::P2PResponse),
    PublishContract = value(WorkType::P2PPublishContract),
    QueryContract = value(WorkType::P2PQueryContract),
    PushTransaction = value(WorkType::P2PPushTransaction),
    Register = OT_ZMQ_INTERNAL_SIGNAL + 0,
    Request = OT_ZMQ_INTERNAL_SIGNAL + 1,
    Processed = OT_ZMQ_INTERNAL_SIGNAL + 2,
    StateMachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};
}  // namespace opentxs::network::p2p
