// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <type_traits>

#include "opentxs/blockchain/node/Types.hpp"  // IWYU pragma: keep

namespace opentxs::blockchain::node
{
enum class SendResult : std::underlying_type_t<SendResult> {
    Sent = 0,
    UnspecifiedError = 1,
    InvalidSenderNym = 2,
    MissingRecipients = 3,
    SerializationError = 4,
    InsufficientConfirmedFunds = 5,
    DatabaseError = 6,
    DuplicateProposal = 7,
    OutputCreationError = 8,
    ChangeError = 9,
    InsufficientFunds = 10,
    InputCreationError = 11,
    SignatureError = 12,
    SendFailed = 13,
    InvalidSweep = 14,
};
}  // namespace opentxs::blockchain::node
