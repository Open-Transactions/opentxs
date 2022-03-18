// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <mutex>
#include <string_view>

#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::wallet
{
// WARNING update print function if new values are added or removed
enum class WalletJobs : OTZMQWorkType {
    shutdown = value(WorkType::Shutdown),
    init = OT_ZMQ_INIT_SIGNAL,
    statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};

// WARNING update print function if new values are added or removed
enum class AccountsJobs : OTZMQWorkType {
    shutdown = value(WorkType::Shutdown),
    nym = value(WorkType::NymCreated),
    header = value(WorkType::BlockchainNewHeader),
    reorg = value(WorkType::BlockchainReorg),
    init = OT_ZMQ_INIT_SIGNAL,
    statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};

// WARNING update print function if new values are added or removed
enum class AccountJobs : OTZMQWorkType {
    shutdown = value(WorkType::Shutdown),
    subaccount = value(WorkType::BlockchainAccountCreated),
    prepare_shutdown = OT_ZMQ_PREPARE_SHUTDOWN + 0,
    init = OT_ZMQ_INIT_SIGNAL,
    key = OT_ZMQ_NEW_BLOCKCHAIN_WALLET_KEY_SIGNAL,
    statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};

// WARNING update print function if new values are added or removed
enum class SubchainJobs : OTZMQWorkType {
    shutdown = value(WorkType::Shutdown),
    filter = value(WorkType::BlockchainNewFilter),
    mempool = value(WorkType::BlockchainMempoolUpdated),
    block = value(WorkType::BlockchainBlockAvailable),
    prepare_shutdown = OT_ZMQ_PREPARE_SHUTDOWN + 0,
    update = OT_ZMQ_INTERNAL_SIGNAL + 1,
    init = OT_ZMQ_INIT_SIGNAL,
    key = OT_ZMQ_NEW_BLOCKCHAIN_WALLET_KEY_SIGNAL,
    statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};

auto lock_for_reorg(std::recursive_timed_mutex& mutex) noexcept
    -> std::unique_lock<std::recursive_timed_mutex>;
auto print(WalletJobs) noexcept -> std::string_view;
auto print(AccountsJobs) noexcept -> std::string_view;
auto print(AccountJobs) noexcept -> std::string_view;
auto print(SubchainJobs) noexcept -> std::string_view;
}  // namespace opentxs::blockchain::node::wallet
