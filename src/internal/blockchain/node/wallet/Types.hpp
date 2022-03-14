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
// WARNING update print function if new values are added or removed
enum class WalletJobs : OTZMQWorkType {
    shutdown = value(WorkType::Shutdown),
    init = OT_ZMQ_INIT_SIGNAL,
    shutdown_ready = OT_ZMQ_ACKNOWLEDGE_SHUTDOWN,
    statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};

// WARNING update print function if new values are added or removed
enum class AccountsJobs : OTZMQWorkType {
    shutdown = value(WorkType::Shutdown),
    nym = value(WorkType::NymCreated),
    header = value(WorkType::BlockchainNewHeader),
    reorg = value(WorkType::BlockchainReorg),
    reorg_begin_ack = OT_ZMQ_INTERNAL_SIGNAL + 2,
    reorg_end_ack = OT_ZMQ_INTERNAL_SIGNAL + 4,
    init = OT_ZMQ_INIT_SIGNAL,
    shutdown_begin = OT_ZMQ_PREPARE_SHUTDOWN,
    shutdown_ready = OT_ZMQ_ACKNOWLEDGE_SHUTDOWN,
    statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};

// WARNING update print function if new values are added or removed
enum class AccountJobs : OTZMQWorkType {
    shutdown = value(WorkType::Shutdown),
    subaccount = value(WorkType::BlockchainAccountCreated),
    reorg_begin = OT_ZMQ_INTERNAL_SIGNAL + 1,
    reorg_begin_ack = OT_ZMQ_INTERNAL_SIGNAL + 2,
    reorg_end = OT_ZMQ_INTERNAL_SIGNAL + 3,
    reorg_end_ack = OT_ZMQ_INTERNAL_SIGNAL + 4,
    init = OT_ZMQ_INIT_SIGNAL,
    key = OT_ZMQ_NEW_BLOCKCHAIN_WALLET_KEY_SIGNAL,
    shutdown_begin = OT_ZMQ_PREPARE_SHUTDOWN,
    shutdown_ready = OT_ZMQ_ACKNOWLEDGE_SHUTDOWN,
    statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};

// WARNING update print function if new values are added or removed
enum class SubchainJobs : OTZMQWorkType {
    shutdown = value(WorkType::Shutdown),
    filter = value(WorkType::BlockchainNewFilter),
    mempool = value(WorkType::BlockchainMempoolUpdated),
    block = value(WorkType::BlockchainBlockAvailable),
    reorg_begin = OT_ZMQ_INTERNAL_SIGNAL + 1,
    reorg_begin_ack = OT_ZMQ_INTERNAL_SIGNAL + 2,
    reorg_end = OT_ZMQ_INTERNAL_SIGNAL + 3,
    reorg_end_ack = OT_ZMQ_INTERNAL_SIGNAL + 4,
    startup = OT_ZMQ_INTERNAL_SIGNAL + 5,
    update = OT_ZMQ_INTERNAL_SIGNAL + 6,
    init = OT_ZMQ_INIT_SIGNAL,
    key = OT_ZMQ_NEW_BLOCKCHAIN_WALLET_KEY_SIGNAL,
    shutdown_begin = OT_ZMQ_PREPARE_SHUTDOWN,
    shutdown_ready = OT_ZMQ_ACKNOWLEDGE_SHUTDOWN,
    statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};

auto print(WalletJobs) noexcept -> std::string_view;
auto print(AccountsJobs) noexcept -> std::string_view;
auto print(AccountJobs) noexcept -> std::string_view;
auto print(SubchainJobs) noexcept -> std::string_view;
}  // namespace opentxs::blockchain::node::wallet
