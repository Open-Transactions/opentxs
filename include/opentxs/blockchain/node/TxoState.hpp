// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <limits>

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/node/Types.hpp"

namespace opentxs::blockchain::node
{
enum class TxoState : std::uint16_t {
    Error = 0,
    UnconfirmedNew = 1,
    UnconfirmedSpend = 2,
    ConfirmedNew = 3,
    ConfirmedSpend = 4,
    OrphanedNew = 5,
    OrphanedSpend = 6,
    Immature = 7,
    All = std::numeric_limits<std::uint16_t>::max(),
};
}  // namespace opentxs::blockchain::node
