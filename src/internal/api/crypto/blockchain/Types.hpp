// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "opentxs/util/WorkType.hpp"
#include "opentxs/util/WorkType.internal.hpp"

namespace opentxs::api::crypto::blockchain
{
// WARNING update print function if new values are added or removed
enum class BalanceOracleJobs : OTZMQWorkType {
    shutdown = value(WorkType::Shutdown),
    update_balance = OT_ZMQ_BALANCE_ORACLE_SUBMIT,
    registration = OT_ZMQ_REGISTER_SIGNAL,
    init = OT_ZMQ_INIT_SIGNAL,
    statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};  // IWYU pragma: export

auto print(BalanceOracleJobs) noexcept -> std::string_view;
}  // namespace opentxs::api::crypto::blockchain
