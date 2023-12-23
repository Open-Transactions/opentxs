// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <limits>
#include <type_traits>

#include "opentxs/blockchain/crypto/Types.hpp"  // IWYU pragma: keep

namespace opentxs::blockchain::crypto
{
enum class SubaccountType : std::underlying_type_t<SubaccountType> {
    Error = 0,
    HD = 1,
    PaymentCode = 2,
    Imported = 3,
    Notification =
        std::numeric_limits<std::underlying_type_t<SubaccountType>>::max(),
};
}  // namespace opentxs::blockchain::crypto
