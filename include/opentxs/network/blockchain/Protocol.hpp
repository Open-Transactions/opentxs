// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <limits>
#include <type_traits>

#include "opentxs/network/blockchain/Types.hpp"  // IWYU pragma: keep

namespace opentxs::network::blockchain
{
enum class Protocol : std::underlying_type_t<Protocol> {
    opentxs = 0,
    bitcoin = 1,
    ethereum = 2,
    unknown_protocol =
        std::numeric_limits<std::underlying_type_t<Protocol>>::max(),
};
}  // namespace opentxs::network::blockchain
