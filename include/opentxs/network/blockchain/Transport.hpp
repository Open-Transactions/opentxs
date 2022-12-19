// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/network/blockchain/Types.hpp"

namespace opentxs::network::blockchain
{
enum class Transport : std::uint8_t {
    invalid = 0,
    ipv4 = 1,
    ipv6 = 2,
    onion2 = 3,
    onion3 = 4,
    eep = 5,
    cjdns = 6,
    zmq = 7,
};
}  // namespace opentxs::network::blockchain
