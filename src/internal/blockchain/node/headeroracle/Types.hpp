// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::headeroracle
{
// WARNING update print function if new values are added or removed
enum class Job : OTZMQWorkType {
    shutdown = value(WorkType::Shutdown),
    update_remote_height = OT_ZMQ_INTERNAL_SIGNAL + 0,
    job_finished = OT_ZMQ_INTERNAL_SIGNAL + 1,
    submit_block_header = OT_ZMQ_INTERNAL_SIGNAL + 2,
    submit_block_hash = OT_ZMQ_INTERNAL_SIGNAL + 3,
    report = OT_ZMQ_BLOCKCHAIN_REPORT_STATUS,
    init = OT_ZMQ_INIT_SIGNAL,
    statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};  // IWYU pragma: export

auto print(Job) noexcept -> std::string_view;
}  // namespace opentxs::blockchain::node::headeroracle
