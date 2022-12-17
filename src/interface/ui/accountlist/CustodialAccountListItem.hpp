// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "interface/ui/accountlist/AccountListItem.hpp"
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
class CustodialAccountListItem final : public AccountListItem
{
public:
    auto NotaryName() const noexcept -> UnallocatedCString final;

    CustodialAccountListItem(
        const AccountListInternalInterface& parent,
        const api::session::Client& api,
        const AccountListRowID& rowID,
        const AccountListSortKey& sortKey,
        CustomData& custom) noexcept;
    CustodialAccountListItem() = delete;
    CustodialAccountListItem(const CustodialAccountListItem&) = delete;
    CustodialAccountListItem(CustodialAccountListItem&&) = delete;
    auto operator=(const CustodialAccountListItem&)
        -> CustodialAccountListItem& = delete;
    auto operator=(CustodialAccountListItem&&)
        -> CustodialAccountListItem& = delete;

    ~CustodialAccountListItem() final;
};
}  // namespace opentxs::ui::implementation
