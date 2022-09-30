// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/blockchain/crypto/Types.hpp"  // IWYU pragma: associated

#include "opentxs/Export.hpp"

namespace opentxs::blockchain::crypto
{
enum class Subchain : std::uint8_t {
    Error = 0,
    Internal = 1,
    External = 2,
    Incoming = 3,
    Outgoing = 4,
    NotificationV3 = 5,
    NotificationV1 = 6,
    NotificationV2 = 7,
    NotificationV4 = 8,
    None = 255,
};
}  // namespace opentxs::blockchain::crypto
