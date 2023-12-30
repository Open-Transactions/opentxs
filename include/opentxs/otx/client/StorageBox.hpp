// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <type_traits>

#include "opentxs/otx/client/Types.hpp"  // IWYU pragma: keep

namespace opentxs::otx::client
{
enum class StorageBox : std::underlying_type_t<StorageBox> {
    SENTPEERREQUEST = 0,
    INCOMINGPEERREQUEST = 1,
    SENTPEERREPLY = 2,
    INCOMINGPEERREPLY = 3,
    FINISHEDPEERREQUEST = 4,
    FINISHEDPEERREPLY = 5,
    PROCESSEDPEERREQUEST = 6,
    PROCESSEDPEERREPLY = 7,
    MAILINBOX = 8,
    MAILOUTBOX = 9,
    BLOCKCHAIN = 10,
    RESERVED_1 = 11,
    INCOMINGCHEQUE = 12,
    OUTGOINGCHEQUE = 13,
    OUTGOINGTRANSFER = 14,
    INCOMINGTRANSFER = 15,
    INTERNALTRANSFER = 16,
    PENDING_SEND = 253,
    DRAFT = 254,
    UNKNOWN = 255,
};
}  // namespace opentxs::otx::client
