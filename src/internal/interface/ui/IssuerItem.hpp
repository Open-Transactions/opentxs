// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/interface/ui/List.hpp"
#include "internal/interface/ui/ListRow.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
class AccountSummaryItem;
class IssuerItem;
}  // namespace ui

using OTUIIssuerItem = SharedPimpl<ui::IssuerItem>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class IssuerItem : virtual public List, virtual public ListRow
{
public:
    virtual auto ConnectionState() const noexcept -> bool = 0;
    virtual auto Debug() const noexcept -> UnallocatedCString = 0;
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::AccountSummaryItem> = 0;
    virtual auto Name() const noexcept -> UnallocatedCString = 0;
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::AccountSummaryItem> = 0;
    virtual auto Trusted() const noexcept -> bool = 0;

    IssuerItem(const IssuerItem&) = delete;
    IssuerItem(IssuerItem&&) = delete;
    auto operator=(const IssuerItem&) -> IssuerItem& = delete;
    auto operator=(IssuerItem&&) -> IssuerItem& = delete;

    ~IssuerItem() override = default;

protected:
    IssuerItem() noexcept = default;
};
}  // namespace opentxs::ui
