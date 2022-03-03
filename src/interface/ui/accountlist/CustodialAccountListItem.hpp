// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "1_Internal.hpp"
#include "interface/ui/accountlist/AccountListItem.hpp"
#include "interface/ui/base/Row.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/interface/ui/AccountListItem.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"

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
// }  // namespace v1
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

    ~CustodialAccountListItem() final;

private:
    CustodialAccountListItem() = delete;
    CustodialAccountListItem(const CustodialAccountListItem&) = delete;
    CustodialAccountListItem(CustodialAccountListItem&&) = delete;
    auto operator=(const CustodialAccountListItem&)
        -> CustodialAccountListItem& = delete;
    auto operator=(CustodialAccountListItem&&)
        -> CustodialAccountListItem& = delete;
};
}  // namespace opentxs::ui::implementation
