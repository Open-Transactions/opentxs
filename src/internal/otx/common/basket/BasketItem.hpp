// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

namespace opentxs
{
class BasketItem;

using dequeOfBasketItems = UnallocatedDeque<BasketItem*>;

class BasketItem
{
public:
    identifier::Generic sub_contract_id_;
    identifier::Generic sub_account_id_;
    TransactionNumber minimum_transfer_amount_{0};
    // lClosingTransactionNo:
    // Used when EXCHANGING a basket (NOT USED when first creating one.)
    // A basketReceipt must be dropped into each asset account during
    // an exchange, to account for the change in balance. Until that
    // receipt is accepted, lClosingTransactionNo will remain open as
    // an issued transaction number (an open transaction) on that Nym.
    // (One must be supplied for EACH asset account during an exchange.)
    TransactionNumber closing_transaction_no_{0};

    BasketItem();
    ~BasketItem() = default;
};
}  // namespace opentxs
