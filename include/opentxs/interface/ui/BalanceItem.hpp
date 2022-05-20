// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <chrono>
#include <cstdint>

#include "ListRow.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace ui
{
class BalanceItem;
}  // namespace ui

using OTUIBalanceItem = SharedPimpl<ui::BalanceItem>;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class OPENTXS_EXPORT BalanceItem : virtual public ListRow
{
public:
    virtual auto Amount() const noexcept -> opentxs::Amount = 0;
    virtual auto Confirmations() const noexcept -> int = 0;
    virtual auto Contacts() const noexcept
        -> UnallocatedVector<UnallocatedCString> = 0;
    virtual auto DisplayAmount() const noexcept -> UnallocatedCString = 0;
    virtual auto Memo() const noexcept -> UnallocatedCString = 0;
    virtual auto Text() const noexcept -> UnallocatedCString = 0;
    virtual auto Timestamp() const noexcept -> Time = 0;
    virtual auto Type() const noexcept -> otx::client::StorageBox = 0;
    virtual auto UUID() const noexcept -> UnallocatedCString = 0;
    virtual auto Workflow() const noexcept -> UnallocatedCString = 0;

    BalanceItem(const BalanceItem&) = delete;
    BalanceItem(BalanceItem&&) = delete;
    auto operator=(const BalanceItem&) -> BalanceItem& = delete;
    auto operator=(BalanceItem&&) -> BalanceItem& = delete;

    ~BalanceItem() override = default;

protected:
    BalanceItem() noexcept = default;
};
}  // namespace opentxs::ui
