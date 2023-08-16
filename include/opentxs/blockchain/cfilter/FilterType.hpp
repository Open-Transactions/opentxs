// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <limits>

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/cfilter/Types.hpp"

namespace opentxs::blockchain::cfilter
{
enum class Type : TypeEnum {
    Basic_BIP158 = 0,
    Basic_BCHVariant = 1,
    ES = 88,
    UnknownCfilter = std::numeric_limits<TypeEnum>::max(),
};
}  // namespace opentxs::blockchain::cfilter