// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <type_traits>

#include "opentxs/otx/client/Types.hpp"  // IWYU pragma: keep

namespace opentxs::otx::client
{
enum class SendResult : std::underlying_type_t<SendResult> {
    TRANSACTION_NUMBERS = -3,
    INVALID_REPLY = -2,
    TIMEOUT = -1,
    Error = 0,
    UNNECESSARY = 1,
    VALID_REPLY = 2,
    SHUTDOWN = 3,
};
}  // namespace opentxs::otx::client
