// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/blockchain/node/Types.hpp"  // IWYU pragma: associated

#include <limits>

#include "opentxs/Export.hpp"

namespace opentxs::blockchain::node
{
enum class SendResult : TypeEnum {
    UnspecifiedError = 0,
    InvalidSenderNym = 1,
    AddressNotValidforChain = 2,
    UnsupportedAddressFormat = 3,
    SenderMissingPaymentCode = 4,
    UnsupportedRecipientPaymentCode = 5,
    HDDerivationFailure = 6,
    DatabaseError = 7,
    DuplicateProposal = 8,
    OutputCreationError = 9,
    ChangeError = 10,
    InsufficientFunds = 11,
    InputCreationError = 12,
    SignatureError = 13,
    SendFailed = 14,
    Sent = std::numeric_limits<TypeEnum>::max(),
};
}  // namespace opentxs::blockchain::node
