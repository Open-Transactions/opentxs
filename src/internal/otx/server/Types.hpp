// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "opentxs/WorkType.hpp"  // IWYU pragma: keep
#include "opentxs/WorkType.internal.hpp"

namespace opentxs::server
{
// WARNING update print function if new values are added or removed
enum class Jobs : OTZMQWorkType {
    shutdown = value(WorkType::Shutdown),
    init = OT_ZMQ_INIT_SIGNAL,
    statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};  // IWYU pragma: export

auto print(Jobs) noexcept -> std::string_view;
}  // namespace opentxs::server
