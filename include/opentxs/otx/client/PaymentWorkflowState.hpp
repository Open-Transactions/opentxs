// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <type_traits>

#include "opentxs/otx/client/Types.hpp"  // IWYU pragma: keep

namespace opentxs::otx::client
{
enum class PaymentWorkflowState : std::underlying_type_t<PaymentWorkflowState> {
    Error = 0,
    Unsent = 1,
    Conveyed = 2,
    Cancelled = 3,
    Accepted = 4,
    Completed = 5,
    Expired = 6,
    Initiated = 7,
    Aborted = 8,
    Acknowledged = 9,
    Rejected = 10,
};
}  // namespace opentxs::otx::client
