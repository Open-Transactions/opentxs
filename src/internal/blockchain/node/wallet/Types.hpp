// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::wallet
{
// NOTE update print function if new values are added
enum class WalletJobs : OTZMQWorkType {
    shutdown = value(WorkType::Shutdown),
    nym = value(WorkType::NymCreated),
    header = value(WorkType::BlockchainNewHeader),
    reorg = value(WorkType::BlockchainReorg),
    filter = value(WorkType::BlockchainNewFilter),
    mempool = value(WorkType::BlockchainMempoolUpdated),
    block = value(WorkType::BlockchainBlockAvailable),
    job_finished = OT_ZMQ_INTERNAL_SIGNAL + 0,
    init = OT_ZMQ_INIT_SIGNAL,
    key = OT_ZMQ_NEW_BLOCKCHAIN_WALLET_KEY_SIGNAL,
    statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};

auto print(WalletJobs) noexcept -> std::string_view;
}  // namespace opentxs::blockchain::node::wallet