// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/AccountType.hpp"

#pragma once

#include "1_Internal.hpp"
#include "interface/ui/base/Row.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/interface/ui/AccountListItem.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/SharedPimpl.hpp"

class QVariant;

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
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
class AccountListItem;
}  // namespace ui
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
using AccountListItemRow =
    Row<AccountListRowInternal, AccountListInternalInterface, AccountListRowID>;

class AccountListItem : public AccountListItemRow
{
public:
    auto AccountID() const noexcept -> UnallocatedCString final
    {
        return row_id_->str();
    }
    auto Balance() const noexcept -> Amount final;
    auto ContractID() const noexcept -> UnallocatedCString final
    {
        return unit_id_->str();
    }
    auto DisplayBalance() const noexcept -> UnallocatedCString final;
    auto DisplayUnit() const noexcept -> UnallocatedCString final
    {
        return unit_name_;
    }
    auto Name() const noexcept -> UnallocatedCString final;
    auto NotaryID() const noexcept -> UnallocatedCString final
    {
        return notary_id_->str();
    }
    auto Type() const noexcept -> AccountType final { return type_; }
    auto Unit() const noexcept -> UnitType final { return unit_; }

    AccountListItem(
        const AccountListInternalInterface& parent,
        const api::session::Client& api,
        const AccountListRowID& rowID,
        const AccountListSortKey& sortKey,
        CustomData& custom) noexcept;
    ~AccountListItem() override;

protected:
    const AccountType type_;
    const UnitType unit_;
    const display::Definition& display_;
    const OTUnitID unit_id_;
    const OTNotaryID notary_id_;
    const UnallocatedCString unit_name_;

private:
    Amount balance_;
    UnallocatedCString name_;

    auto qt_data(const int column, const int role, QVariant& out) const noexcept
        -> void final;

    auto reindex(const AccountListSortKey&, CustomData&) noexcept -> bool final;

    AccountListItem() = delete;
    AccountListItem(const AccountListItem&) = delete;
    AccountListItem(AccountListItem&&) = delete;
    auto operator=(const AccountListItem&) -> AccountListItem& = delete;
    auto operator=(AccountListItem&&) -> AccountListItem& = delete;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::AccountListItem>;
