// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <limits>
#include <type_traits>

#include "opentxs/blockchain/token/Types.hpp"

namespace opentxs::blockchain::token
{
enum class Type : std::underlying_type_t<Type> {
    null = 0,
    slp = 1,
    cashtoken = 2,
    erc20 = 3,
    erc721 = 4,
    erc1155 = 5,
    erc4626 = 6,
    eos = 7,
    nep5 = 8,
    nep11 = 9,
    nep17 = 10,
    tzip7 = 11,
    tzip12 = 12,
    trc20 = 13,
    trc10 = 14,
    bep2 = 15,
    bsc20 = 16,
    unknown = std::numeric_limits<int>::max(),
};
}  // namespace opentxs::blockchain::token
