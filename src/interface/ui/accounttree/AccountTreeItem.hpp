// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/AccountType.hpp"

#pragma once

#include <iosfwd>

#include "interface/ui/base/Row.hpp"
#include "internal/interface/ui/AccountTreeItem.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"

class QVariant;

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api

namespace display
{
class Definition;
}  // namespace display

namespace ui
{
class AccountTreeItem;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
using AccountTreeItemRow =
    Row<AccountCurrencyRowInternal,
        AccountCurrencyInternalInterface,
        AccountCurrencyRowID>;

class AccountTreeItem : public AccountTreeItemRow
{
public:
    const api::session::Client& api_;

    auto AccountID() const noexcept -> UnallocatedCString final
    {
        return row_id_.asBase58(api_.Crypto());
    }
    auto Balance() const noexcept -> Amount final;
    auto ContractID() const noexcept -> UnallocatedCString final
    {
        return unit_id_.asBase58(api_.Crypto());
    }
    auto DisplayBalance() const noexcept -> UnallocatedCString final;
    auto DisplayUnit() const noexcept -> UnallocatedCString final
    {
        return unit_name_;
    }
    auto Name() const noexcept -> UnallocatedCString final;
    auto NotaryID() const noexcept -> UnallocatedCString final
    {
        return notary_id_.asBase58(api_.Crypto());
    }
    auto Type() const noexcept -> AccountType final { return type_; }
    auto Unit() const noexcept -> UnitType final { return unit_; }

    AccountTreeItem(
        const AccountCurrencyInternalInterface& parent,
        const api::session::Client& api,
        const AccountCurrencyRowID& rowID,
        const AccountCurrencySortKey& sortKey,
        CustomData& custom) noexcept;
    AccountTreeItem() = delete;
    AccountTreeItem(const AccountTreeItem&) = delete;
    AccountTreeItem(AccountTreeItem&&) = delete;
    auto operator=(const AccountTreeItem&) -> AccountTreeItem& = delete;
    auto operator=(AccountTreeItem&&) -> AccountTreeItem& = delete;

    ~AccountTreeItem() override;

protected:
    const AccountType type_;
    const UnitType unit_;
    const display::Definition& display_;
    const identifier::UnitDefinition unit_id_;
    const identifier::Notary notary_id_;
    const UnallocatedCString unit_name_;

private:
    Amount balance_;
    UnallocatedCString name_;

    auto qt_data(const int column, const int role, QVariant& out) const noexcept
        -> void final;

    auto reindex(const AccountCurrencySortKey&, CustomData&) noexcept
        -> bool final;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::AccountTreeItem>;
