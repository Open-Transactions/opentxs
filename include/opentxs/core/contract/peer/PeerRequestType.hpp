// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"                   // IWYU pragma: associated
#include "opentxs/core/contract/peer/Types.hpp"  // IWYU pragma: associated

#include <cstdint>

namespace opentxs::contract::peer
{
enum class PeerRequestType : std::uint8_t {
    Error = 0,
    Bailment = 1,
    OutBailment = 2,
    PendingBailment = 3,
    ConnectionInfo = 4,
    StoreSecret = 5,
    VerificationOffer = 6,
    Faucet = 7,
};
}  // namespace opentxs::contract::peer
