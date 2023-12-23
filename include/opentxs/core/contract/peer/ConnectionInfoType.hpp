// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <type_traits>

#include "opentxs/core/contract/peer/Types.hpp"  // IWYU pragma: keep

namespace opentxs::contract::peer
{
enum class ConnectionInfoType : std::underlying_type_t<ConnectionInfoType> {
    Error = 0,
    Bitcoin = 1,
    BtcRpc = 2,
    BitMessage = 3,
    BitMessageRPC = 4,
    SSH = 5,
    CJDNS = 6,
};
}  // namespace opentxs::contract::peer
