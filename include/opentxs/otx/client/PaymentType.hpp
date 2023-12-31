// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <type_traits>

#include "opentxs/otx/client/Types.hpp"  // IWYU pragma: keep

namespace opentxs::otx::client
{
enum class PaymentType : std::underlying_type_t<PaymentType> {
    Error = 0,
    Cheque = 1,
    Voucher = 2,
    Transfer = 3,
    Blinded = 4,
};
}  // namespace opentxs::otx::client
