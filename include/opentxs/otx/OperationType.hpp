// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/otx/Types.hpp"  // IWYU pragma: associated

#include <cstdint>

#include "opentxs/Export.hpp"

namespace opentxs::otx
{
enum class OperationType : std::uint16_t {
    Invalid = 0,
    AddClaim = 1,
    CheckNym = 2,
    ConveyPayment = 3,
    DepositCash = 4,
    DepositCheque = 5,
    DownloadContract = 6,
    DownloadMint = 7,
    GetTransactionNumbers = 8,
    IssueUnitDefinition = 9,
    PublishNym = 10,
    PublishServer = 11,
    PublishUnit = 12,
    RefreshAccount = 13,
    RegisterAccount = 14,
    RegisterNym = 15,
    RequestAdmin = 16,
    SendCash = 17,
    SendMessage = 18,
    SendPeerReply = 19,
    SendPeerRequest = 20,
    SendTransfer = 21,
    WithdrawCash = 22,
};
}  // namespace opentxs::otx
