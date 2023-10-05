// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/accounttree/AccountCurrency.hpp"  // IWYU pragma: associated

#include <memory>
#include <sstream>
#include <string_view>

#include "internal/interface/ui/AccountTreeItem.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"

namespace opentxs::factory
{
auto AccountCurrencyWidget(
    const ui::implementation::AccountTreeInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::AccountTreeRowID& rowID,
    const ui::implementation::AccountTreeSortKey& key,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::AccountTreeRowInternal>
{
    using ReturnType = ui::implementation::AccountCurrency;

    return std::make_unique<ReturnType>(parent, api, rowID, key, custom);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
AccountCurrency::AccountCurrency(
    const AccountTreeInternalInterface& parent,
    const api::session::Client& api,
    const AccountTreeRowID& rowID,
    const AccountTreeSortKey& key,
    CustomData& custom) noexcept
    : Combined(
          api,
          parent.Owner(),
          parent.WidgetID(),
          parent,
          rowID,
          key,
          false)
    , api_(api)
{
}

auto AccountCurrency::construct_row(
    const AccountCurrencyRowID& id,
    const AccountCurrencySortKey& index,
    CustomData& custom) const noexcept -> RowPointer
{
    return factory::AccountTreeItem(*this, api_, id, index, custom);
}

auto AccountCurrency::Debug() const noexcept -> UnallocatedCString
{
    auto out = std::stringstream{};
    auto counter{-1};
    out << "    * " << Name() << ":\n";
    auto item = First();
    const auto PrintRow = [&counter, &out](const auto& row) {
        out << "      * row " << std::to_string(++counter) << ":\n";
        out << "            account ID: " << row.AccountID() << '\n';
        out << "          account name: " << row.Name() << '\n';
        out << "          account type: " << opentxs::print(row.Type()) << '\n';
        out << "             notary ID: " << row.NotaryID() << '\n';
        out << "           notary name: " << row.NotaryName() << '\n';
        out << "               unit ID: " << row.ContractID() << '\n';
        out << "             unit type: " << row.DisplayUnit() << '\n';
        out << "               Balance: " << row.DisplayBalance() << '\n';
    };

    if (item->Valid()) {
        PrintRow(item.get());

        while (false == item->Last()) {
            item = Next();
            PrintRow(item.get());
        }
    } else {
        out << "      * no accounts\n";
    }

    return out.str();
}

auto AccountCurrency::reindex(
    const implementation::AccountTreeSortKey& key,
    implementation::CustomData& custom) noexcept -> bool
{
    return false;
}

AccountCurrency::~AccountCurrency() = default;
}  // namespace opentxs::ui::implementation
