// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/accountsummary/AccountSummaryItem.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "interface/ui/base/Widget.hpp"
#include "internal/api/session/Storage.hpp"
#include "internal/core/contract/Unit.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Factory.internal.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/api/session/Wallet.internal.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"  // IWYU pragma: keep
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto AccountSummaryItem(
    const ui::implementation::IssuerItemInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::IssuerItemRowID& rowID,
    const ui::implementation::IssuerItemSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::IssuerItemRowInternal>
{
    using ReturnType = ui::implementation::AccountSummaryItem;

    return std::make_shared<ReturnType>(parent, api, rowID, sortKey, custom);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
AccountSummaryItem::AccountSummaryItem(
    const IssuerItemInternalInterface& parent,
    const api::session::Client& api,
    const IssuerItemRowID& rowID,
    const IssuerItemSortKey& sortKey,
    CustomData& custom) noexcept
    : AccountSummaryItemRow(parent, api, rowID, true)
    , api_(api)
    , account_id_(std::get<0>(row_id_))
    , currency_(std::get<1>(row_id_))
    , balance_(extract_custom<Amount>(custom))
    , name_(sortKey)
    , contract_(load_unit(api_, account_id_))
{
}

auto AccountSummaryItem::DisplayBalance() const noexcept -> UnallocatedCString
{
    if (0 == contract_->Version()) {
        eLock lock(shared_lock_);

        try {
            contract_ = api_.Wallet().Internal().UnitDefinition(
                api_.Storage().Internal().AccountContract(account_id_));
        } catch (...) {
        }
    }

    sLock lock(shared_lock_);

    if (0 < contract_->Version()) {
        const auto& definition = display::GetDefinition(currency_);
        UnallocatedCString output = definition.Format(balance_);

        if (0 < output.size()) { return output; }

        balance_.Serialize(writer(output));
        return output;
    }

    return {};
}

auto AccountSummaryItem::load_unit(
    const api::Session& api,
    const identifier::Account& id) -> OTUnitDefinition
{
    try {
        return api.Wallet().Internal().UnitDefinition(
            api.Storage().Internal().AccountContract(id));
    } catch (...) {

        return api.Factory().Internal().Session().UnitDefinition();
    }
}

auto AccountSummaryItem::Name() const noexcept -> UnallocatedCString
{
    sLock lock(shared_lock_);

    return name_;
}

auto AccountSummaryItem::reindex(
    const IssuerItemSortKey& key,
    CustomData& custom) noexcept -> bool
{
    eLock lock(shared_lock_);
    balance_ = extract_custom<Amount>(custom);
    name_ = key;

    return true;
}
}  // namespace opentxs::ui::implementation
