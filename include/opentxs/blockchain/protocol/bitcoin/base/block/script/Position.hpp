// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <type_traits>

#include "opentxs/blockchain/protocol/bitcoin/base/block/script/Types.hpp"  // IWYU pragma: keep

namespace opentxs::blockchain::protocol::bitcoin::base::block::script
{
enum class Position : std::underlying_type_t<Position> {
    Coinbase = 0,
    Input = 1,
    Output = 2,
    Redeem = 3,
};
}  // namespace opentxs::blockchain::protocol::bitcoin::base::block::script
