// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <type_traits>

#include "opentxs/core/contract/peer/Types.hpp"  // IWYU pragma: keep

namespace opentxs::contract::peer
{
enum class RequestType : std::underlying_type_t<RequestType> {
    Error = 0,
    Bailment = 1,
    OutBailment = 2,
    PendingBailment = 3,
    ConnectionInfo = 4,
    StoreSecret = 5,
    VerifiedClaim = 6,
    Faucet = 7,
    Verification = 8,
};
}  // namespace opentxs::contract::peer
