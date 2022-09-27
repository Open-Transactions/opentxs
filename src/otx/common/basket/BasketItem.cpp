// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                               // IWYU pragma: associated
#include "1_Internal.hpp"                             // IWYU pragma: associated
#include "internal/otx/common/basket/BasketItem.hpp"  // IWYU pragma: associated

namespace opentxs
{
BasketItem::BasketItem()
    : sub_contract_id_()
    , sub_account_id_()
    , minimum_transfer_amount_(0)
    , closing_transaction_no_(0)
{
}
}  // namespace opentxs
