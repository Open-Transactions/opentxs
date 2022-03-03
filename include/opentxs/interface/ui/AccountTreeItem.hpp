// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/UnitType.hpp"

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Types.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/interface/ui/ListRow.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
class Amount;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class OPENTXS_EXPORT AccountTreeItem : virtual public ListRow
{
public:
    virtual auto AccountID() const noexcept -> UnallocatedCString = 0;
    virtual auto Balance() const noexcept -> Amount = 0;
    virtual auto ContractID() const noexcept -> UnallocatedCString = 0;
    virtual auto DisplayBalance() const noexcept -> UnallocatedCString = 0;
    virtual auto DisplayUnit() const noexcept -> UnallocatedCString = 0;
    virtual auto Name() const noexcept -> UnallocatedCString = 0;
    virtual auto NotaryID() const noexcept -> UnallocatedCString = 0;
    virtual auto NotaryName() const noexcept -> UnallocatedCString = 0;
    virtual auto Type() const noexcept -> AccountType = 0;
    virtual auto Unit() const noexcept -> UnitType = 0;

    ~AccountTreeItem() override = default;

protected:
    AccountTreeItem() noexcept = default;

private:
    AccountTreeItem(const AccountTreeItem&) = delete;
    AccountTreeItem(AccountTreeItem&&) = delete;
    auto operator=(const AccountTreeItem&) -> AccountTreeItem& = delete;
    auto operator=(AccountTreeItem&&) -> AccountTreeItem& = delete;
};
}  // namespace opentxs::ui
