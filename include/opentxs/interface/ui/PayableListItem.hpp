// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "ContactListItem.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace ui
{
class PayableListItem;
}  // namespace ui

using OTUIPayableListItem = SharedPimpl<ui::PayableListItem>;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class OPENTXS_EXPORT PayableListItem : virtual public ContactListItem
{
public:
    virtual auto PaymentCode() const noexcept -> UnallocatedCString = 0;

    ~PayableListItem() override = default;

protected:
    PayableListItem() noexcept = default;

private:
    PayableListItem(const PayableListItem&) = delete;
    PayableListItem(PayableListItem&&) = delete;
    auto operator=(const PayableListItem&) -> PayableListItem& = delete;
    auto operator=(PayableListItem&&) -> PayableListItem& = delete;
};
}  // namespace opentxs::ui
