// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <utility>

#include "interface/ui/accounttree/AccountTreeItem.hpp"
#include "interface/ui/base/Combined.hpp"
#include "interface/ui/base/List.hpp"
#include "interface/ui/base/RowType.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/util/Container.hpp"

class QVariant;

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
class AccountCurrency;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
using AccountCurrencyList = List<
    AccountCurrencyExternalInterface,
    AccountCurrencyInternalInterface,
    AccountCurrencyRowID,
    AccountCurrencyRowInterface,
    AccountCurrencyRowInternal,
    AccountCurrencyRowBlank,
    AccountCurrencySortKey,
    AccountCurrencyPrimaryID>;
using AccountCurrencyRow = RowType<
    AccountTreeRowInternal,
    AccountTreeInternalInterface,
    AccountTreeRowID>;

class AccountCurrency final : public Combined<
                                  AccountCurrencyList,
                                  AccountCurrencyRow,
                                  AccountTreeSortKey>
{
public:
    const api::session::Client& api_;

    auto API() const noexcept -> const api::Session& final { return api_; }
    auto Currency() const noexcept -> UnitType final { return row_id_; }
    auto Debug() const noexcept -> UnallocatedCString final;
    auto Name() const noexcept -> UnallocatedCString final
    {
        return key_.second;
    }

    AccountCurrency(
        const AccountTreeInternalInterface& parent,
        const api::session::Client& api,
        const AccountTreeRowID& rowID,
        const AccountTreeSortKey& key,
        CustomData& custom) noexcept;
    AccountCurrency() = delete;
    AccountCurrency(const AccountCurrency&) = delete;
    AccountCurrency(AccountCurrency&&) = delete;
    auto operator=(const AccountCurrency&) -> AccountCurrency& = delete;
    auto operator=(AccountCurrency&&) -> AccountCurrency& = delete;

    ~AccountCurrency() final;

private:
    auto construct_row(
        const AccountCurrencyRowID& id,
        const AccountCurrencySortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;

    auto last(const AccountCurrencyRowID& id) const noexcept -> bool final
    {
        return AccountCurrencyList::last(id);
    }
    auto qt_data(const int column, const int role, QVariant& out) const noexcept
        -> void final;

    auto reindex(
        const implementation::AccountTreeSortKey& key,
        implementation::CustomData& custom) noexcept -> bool final;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::AccountCurrency>;
