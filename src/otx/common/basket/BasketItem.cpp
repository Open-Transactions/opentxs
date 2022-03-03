// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                               // IWYU pragma: associated
#include "1_Internal.hpp"                             // IWYU pragma: associated
#include "internal/otx/common/basket/BasketItem.hpp"  // IWYU pragma: associated

#include "opentxs/core/identifier/Generic.hpp"

namespace opentxs
{
BasketItem::BasketItem()
    : SUB_CONTRACT_ID(Identifier::Factory())
    , SUB_ACCOUNT_ID(Identifier::Factory())
    , lMinimumTransferAmount(0)
    , lClosingTransactionNo(0)
{
}
}  // namespace opentxs
