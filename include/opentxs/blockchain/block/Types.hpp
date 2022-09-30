// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/Types.hpp"

namespace opentxs::blockchain::block
{
using Version = std::int32_t;
using Height = std::int64_t;
using Txid = blockchain::Hash;
using pTxid = blockchain::pHash;

OPENTXS_EXPORT auto BlankHash() noexcept -> pHash;
}  // namespace opentxs::blockchain::block
