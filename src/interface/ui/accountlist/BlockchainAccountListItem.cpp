// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "interface/ui/accountlist/BlockchainAccountListItem.hpp"  // IWYU pragma: associated

#include <memory>

#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"

namespace opentxs::factory
{
auto AccountListItemBlockchain(
    const ui::implementation::AccountListInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::AccountListRowID& rowID,
    const ui::implementation::AccountListSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::AccountListRowInternal>
{
    using ReturnType = ui::implementation::BlockchainAccountListItem;

    return std::make_shared<ReturnType>(parent, api, rowID, sortKey, custom);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
BlockchainAccountListItem::BlockchainAccountListItem(
    const AccountListInternalInterface& parent,
    const api::session::Client& api,
    const AccountListRowID& rowID,
    const AccountListSortKey& sortKey,
    CustomData& custom) noexcept
    : AccountListItem(parent, api, rowID, sortKey, custom)
    , chain_(UnitToBlockchain(unit_))
    , notary_name_(blockchain::DisplayString(chain_))
{
}

BlockchainAccountListItem::~BlockchainAccountListItem() = default;
}  // namespace opentxs::ui::implementation
