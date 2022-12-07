// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/Export.hpp"
#include "opentxs/core/Types.hpp"

namespace opentxs
{
enum class AccountType : std::int8_t {
    Error = 0,
    Blockchain = 1,
    Custodial = 2,
};
}  // namespace opentxs
