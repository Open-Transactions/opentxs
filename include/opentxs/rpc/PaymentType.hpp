// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <type_traits>

#include "opentxs/rpc/Types.hpp"  // IWYU pragma: keep

namespace opentxs::rpc
{
enum class PaymentType : std::underlying_type_t<PaymentType> {
    error = 0,
    cheque = 1,
    transfer = 2,
    voucher = 3,
    invoice = 4,
    blinded = 5,
    blockchain = 6,
};
}  // namespace opentxs::rpc
