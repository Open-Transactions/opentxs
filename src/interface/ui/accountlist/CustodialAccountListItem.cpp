// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "interface/ui/accountlist/CustodialAccountListItem.hpp"  // IWYU pragma: associated

#include <memory>

#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/util/SharedPimpl.hpp"

namespace opentxs::factory
{
auto AccountListItemCustodial(
    const ui::implementation::AccountListInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::AccountListRowID& rowID,
    const ui::implementation::AccountListSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::AccountListRowInternal>
{
    using ReturnType = ui::implementation::CustodialAccountListItem;

    return std::make_shared<ReturnType>(parent, api, rowID, sortKey, custom);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
CustodialAccountListItem::CustodialAccountListItem(
    const AccountListInternalInterface& parent,
    const api::session::Client& api,
    const AccountListRowID& rowID,
    const AccountListSortKey& sortKey,
    CustomData& custom) noexcept
    : AccountListItem(parent, api, rowID, sortKey, custom)
{
}

auto CustodialAccountListItem::NotaryName() const noexcept -> UnallocatedCString
{
    try {

        return api_.Wallet().Server(notary_id_)->EffectiveName();
    } catch (...) {

        return NotaryID();
    }
}

CustodialAccountListItem::~CustodialAccountListItem() = default;
}  // namespace opentxs::ui::implementation
