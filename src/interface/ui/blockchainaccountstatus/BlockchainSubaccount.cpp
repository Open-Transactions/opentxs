// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "interface/ui/blockchainaccountstatus/BlockchainSubaccount.hpp"  // IWYU pragma: associated

#include <memory>

#include "interface/ui/base/List.hpp"
#include "opentxs/core/identifier/Generic.hpp"

namespace opentxs::factory
{
auto BlockchainSubaccountWidget(
    const ui::implementation::BlockchainSubaccountSourceInternalInterface&
        parent,
    const api::session::Client& api,
    const ui::implementation::BlockchainSubaccountSourceRowID& rowID,
    const ui::implementation::BlockchainSubaccountSourceSortKey& key,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<
        ui::implementation::BlockchainSubaccountSourceRowInternal>
{
    using ReturnType = ui::implementation::BlockchainSubaccount;

    return std::make_unique<ReturnType>(parent, api, rowID, key, custom);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
BlockchainSubaccount::BlockchainSubaccount(
    const BlockchainSubaccountSourceInternalInterface& parent,
    const api::session::Client& api,
    const BlockchainSubaccountSourceRowID& rowID,
    const BlockchainSubaccountSourceSortKey& key,
    CustomData& custom) noexcept
    : Combined(
          api,
          parent.NymID(),
          parent.WidgetID(),
          parent,
          rowID,
          key,
          false)
{
}

auto BlockchainSubaccount::construct_row(
    const BlockchainSubaccountRowID& id,
    const BlockchainSubaccountSortKey& index,
    CustomData& custom) const noexcept -> RowPointer
{
    return factory::BlockchainSubchainWidget(*this, api_, id, index, custom);
}

auto BlockchainSubaccount::reindex(
    const implementation::BlockchainSubaccountSourceSortKey& key,
    implementation::CustomData& custom) noexcept -> bool
{
    return false;
}

BlockchainSubaccount::~BlockchainSubaccount() = default;
}  // namespace opentxs::ui::implementation
