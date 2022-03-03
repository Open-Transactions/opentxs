// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

namespace opentxs::api::crypto::blockchain
{
// WARNING update print function if new values are added or removed
enum class BalanceOracleJobs : OTZMQWorkType {
    shutdown = value(WorkType::Shutdown),
    update_chain_balance = OT_ZMQ_INTERNAL_SIGNAL + 0,
    update_nym_balance = OT_ZMQ_INTERNAL_SIGNAL + 1,
    registration = OT_ZMQ_REGISTER_SIGNAL,
    init = OT_ZMQ_INIT_SIGNAL,
    statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};

auto print(BalanceOracleJobs) noexcept -> std::string_view;
}  // namespace opentxs::api::crypto::blockchain
