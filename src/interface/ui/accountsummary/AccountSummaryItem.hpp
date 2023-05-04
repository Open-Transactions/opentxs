// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "interface/ui/base/Row.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace ui
{
class AccountSummaryItem;
}  // namespace ui
}  // namespace opentxs

class QVariant;
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
using AccountSummaryItemRow =
    Row<IssuerItemRowInternal, IssuerItemInternalInterface, IssuerItemRowID>;

class AccountSummaryItem final : public AccountSummaryItemRow
{
public:
    const api::session::Client& api_;

    auto AccountID() const noexcept -> UnallocatedCString final
    {
        return account_id_.asBase58(api_.Crypto());
    }
    auto Balance() const noexcept -> Amount final
    {
        sLock lock(shared_lock_);
        return balance_;
    }
    auto DisplayBalance() const noexcept -> UnallocatedCString final;
    auto Name() const noexcept -> UnallocatedCString final;

    AccountSummaryItem(
        const IssuerItemInternalInterface& parent,
        const api::session::Client& api,
        const IssuerItemRowID& rowID,
        const IssuerItemSortKey& sortKey,
        CustomData& custom) noexcept;
    AccountSummaryItem() = delete;
    AccountSummaryItem(const AccountSummaryItem&) = delete;
    AccountSummaryItem(AccountSummaryItem&&) = delete;
    auto operator=(const AccountSummaryItem&) -> AccountSummaryItem& = delete;
    auto operator=(AccountSummaryItem&&) -> AccountSummaryItem& = delete;

    ~AccountSummaryItem() final = default;

private:
    const identifier::Account& account_id_;
    const UnitType& currency_;
    mutable Amount balance_;
    IssuerItemSortKey name_;
    mutable OTUnitDefinition contract_;

    static auto load_unit(
        const api::Session& api,
        const identifier::Account& id) -> OTUnitDefinition;

    auto qt_data(const int column, const int role, QVariant& out) const noexcept
        -> void final;

    auto reindex(const IssuerItemSortKey& key, CustomData& custom) noexcept
        -> bool final;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::AccountSummaryItem>;
