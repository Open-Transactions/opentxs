// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/accounttree/BlockchainAccountTreeItem.hpp"  // IWYU pragma: associated

#include <memory>

#include "opentxs/blockchain/Types.hpp"

namespace opentxs::factory
{
auto AccountTreeItemBlockchain(
    const ui::implementation::AccountCurrencyInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::AccountCurrencyRowID& rowID,
    const ui::implementation::AccountCurrencySortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::AccountCurrencyRowInternal>
{
    using ReturnType = ui::implementation::BlockchainAccountTreeItem;

    return std::make_shared<ReturnType>(parent, api, rowID, sortKey, custom);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
BlockchainAccountTreeItem::BlockchainAccountTreeItem(
    const AccountCurrencyInternalInterface& parent,
    const api::session::Client& api,
    const AccountCurrencyRowID& rowID,
    const AccountCurrencySortKey& sortKey,
    CustomData& custom) noexcept
    : AccountTreeItem(parent, api, rowID, sortKey, custom)
    , chain_(unit_to_blockchain(unit_))
    , notary_name_(print(chain_))
{
}

BlockchainAccountTreeItem::~BlockchainAccountTreeItem() = default;
}  // namespace opentxs::ui::implementation
