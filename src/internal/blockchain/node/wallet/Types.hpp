// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <memory>
#include <string_view>

#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::wallet
{
using StateSequence = std::size_t;

// WARNING update print function if new values are added or removed
enum class WalletJobs : OTZMQWorkType {
    shutdown = value(WorkType::Shutdown),
    start_wallet = OT_ZMQ_INTERNAL_SIGNAL + 0,
    rescan = OT_ZMQ_BLOCKCHAIN_WALLET_RESCAN,
    init = OT_ZMQ_INIT_SIGNAL,
    statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};  // IWYU pragma: export

// WARNING update print function if new values are added or removed
enum class AccountsJobs : OTZMQWorkType {
    shutdown = value(WorkType::Shutdown),
    nym = value(WorkType::NymCreated),
    header = value(WorkType::BlockchainNewHeader),
    reorg = value(WorkType::BlockchainReorg),
    rescan = OT_ZMQ_BLOCKCHAIN_WALLET_RESCAN,
    reorg_ready = OT_ZMQ_BLOCKCHAIN_WALLET_REORG_READY,
    shutdown_ready = OT_ZMQ_BLOCKCHAIN_WALLET_SHUTDOWN_READY,
    init = OT_ZMQ_INIT_SIGNAL,
    prepare_shutdown = OT_ZMQ_PREPARE_SHUTDOWN,
    statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};  // IWYU pragma: export

// WARNING update print function if new values are added or removed
enum class AccountJobs : OTZMQWorkType {
    shutdown = value(WorkType::Shutdown),
    subaccount = value(WorkType::BlockchainAccountCreated),
    prepare_reorg = OT_ZMQ_BLOCKCHAIN_WALLET_PREPARE_REORG,
    rescan = OT_ZMQ_BLOCKCHAIN_WALLET_RESCAN,
    finish_reorg = OT_ZMQ_BLOCKCHAIN_WALLET_FINISH_REORG,
    init = OT_ZMQ_INIT_SIGNAL,
    key = OT_ZMQ_NEW_BLOCKCHAIN_WALLET_KEY_SIGNAL,
    prepare_shutdown = OT_ZMQ_PREPARE_SHUTDOWN,
    statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};  // IWYU pragma: export

// WARNING update print function if new values are added or removed
enum class SubchainJobs : OTZMQWorkType {
    shutdown = value(WorkType::Shutdown),
    filter = OT_ZMQ_NEW_FILTER_SIGNAL,
    mempool = value(WorkType::BlockchainMempoolUpdated),
    start_scan = OT_ZMQ_INTERNAL_SIGNAL + 0,
    prepare_reorg = OT_ZMQ_BLOCKCHAIN_WALLET_PREPARE_REORG,
    update = OT_ZMQ_BLOCKCHAIN_WALLET_UPDATE,
    process = OT_ZMQ_BLOCKCHAIN_WALLET_PROCESS,
    watchdog = OT_ZMQ_BLOCKCHAIN_WALLET_WATCHDOG,
    watchdog_ack = OT_ZMQ_BLOCKCHAIN_WALLET_WATCHDOG_ACK,
    reprocess = OT_ZMQ_BLOCKCHAIN_WALLET_REPROCESS,
    rescan = OT_ZMQ_BLOCKCHAIN_WALLET_RESCAN,
    do_rescan = OT_ZMQ_BLOCKCHAIN_WALLET_DO_RESCAN,
    finish_reorg = OT_ZMQ_BLOCKCHAIN_WALLET_FINISH_REORG,
    block = OT_ZMQ_BLOCK_ORACLE_BLOCK_READY,
    init = OT_ZMQ_INIT_SIGNAL,
    key = OT_ZMQ_NEW_BLOCKCHAIN_WALLET_KEY_SIGNAL,
    prepare_shutdown = OT_ZMQ_PREPARE_SHUTDOWN,
    statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};  // IWYU pragma: export

// WARNING update print function if new values are added or removed
enum class JobType : unsigned int {
    scan,
    process,
    index,
    rescan,
    progress,
};  // IWYU pragma: export

enum class FeeOracleJobs : OTZMQWorkType {
    shutdown = value(WorkType::Shutdown),
    update_estimate = OT_ZMQ_INTERNAL_SIGNAL + 0,
    init = OT_ZMQ_INIT_SIGNAL,
    statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};  // IWYU pragma: export

enum class FeeSourceJobs : OTZMQWorkType {
    shutdown = value(WorkType::Shutdown),
    query = OT_ZMQ_INTERNAL_SIGNAL + 0,
    init = OT_ZMQ_INIT_SIGNAL,
    statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};  // IWYU pragma: export

auto print(WalletJobs) noexcept -> std::string_view;
auto print(AccountsJobs) noexcept -> std::string_view;
auto print(AccountJobs) noexcept -> std::string_view;
auto print(SubchainJobs) noexcept -> std::string_view;
auto print(JobType) noexcept -> std::string_view;
auto print(FeeOracleJobs) noexcept -> std::string_view;
auto print(FeeSourceJobs) noexcept -> std::string_view;
}  // namespace opentxs::blockchain::node::wallet
