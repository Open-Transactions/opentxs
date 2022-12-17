// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "interface/ui/accounttree/AccountTreeItem.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/util/Container.hpp"

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
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
class CustodialAccountTreeItem final : public AccountTreeItem
{
public:
    auto NotaryName() const noexcept -> UnallocatedCString final;

    CustodialAccountTreeItem(
        const AccountCurrencyInternalInterface& parent,
        const api::session::Client& api,
        const AccountCurrencyRowID& rowID,
        const AccountCurrencySortKey& sortKey,
        CustomData& custom) noexcept;
    CustodialAccountTreeItem() = delete;
    CustodialAccountTreeItem(const CustodialAccountTreeItem&) = delete;
    CustodialAccountTreeItem(CustodialAccountTreeItem&&) = delete;
    auto operator=(const CustodialAccountTreeItem&)
        -> CustodialAccountTreeItem& = delete;
    auto operator=(CustodialAccountTreeItem&&)
        -> CustodialAccountTreeItem& = delete;

    ~CustodialAccountTreeItem() final;
};
}  // namespace opentxs::ui::implementation
