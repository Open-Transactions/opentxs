// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/UnitType.hpp"

#pragma once

#include <atomic>

#include "1_Internal.hpp"
#include "Proto.hpp"
#include "interface/ui/base/Row.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/contract/Unit.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/identity/wot/claim/ClaimType.hpp"
#include "opentxs/interface/ui/AccountSummaryItem.hpp"
#include "opentxs/util/Container.hpp"
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

class Session;
}  // namespace api

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

namespace ui
{
class AccountSummaryItem;
}  // namespace ui
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
using AccountSummaryItemRow =
    Row<IssuerItemRowInternal, IssuerItemInternalInterface, IssuerItemRowID>;

class AccountSummaryItem final : public AccountSummaryItemRow
{
public:
    auto AccountID() const noexcept -> UnallocatedCString final
    {
        return account_id_.str();
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

    ~AccountSummaryItem() final = default;

private:
    const Identifier& account_id_;
    const UnitType& currency_;
    mutable Amount balance_;
    IssuerItemSortKey name_;
    mutable OTUnitDefinition contract_;

    static auto load_unit(const api::Session& api, const Identifier& id)
        -> OTUnitDefinition;

    auto qt_data(const int column, const int role, QVariant& out) const noexcept
        -> void final;

    auto reindex(const IssuerItemSortKey& key, CustomData& custom) noexcept
        -> bool final;

    AccountSummaryItem() = delete;
    AccountSummaryItem(const AccountSummaryItem&) = delete;
    AccountSummaryItem(AccountSummaryItem&&) = delete;
    auto operator=(const AccountSummaryItem&) -> AccountSummaryItem& = delete;
    auto operator=(AccountSummaryItem&&) -> AccountSummaryItem& = delete;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::AccountSummaryItem>;
