// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "interface/ui/contactlist/ContactListItem.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/util/Container.hpp"

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

namespace ui
{
class PayableListItem;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
class PayableListItem final : public PayableListRowInternal,
                              public implementation::ContactListItem
{
public:
    auto PaymentCode() const noexcept -> UnallocatedCString final;

    PayableListItem(
        const PayableInternalInterface& parent,
        const api::session::Client& api,
        const PayableListRowID& rowID,
        const PayableListSortKey& key,
        const UnallocatedCString& paymentcode,
        const UnitType& currency) noexcept;
    PayableListItem() = delete;
    PayableListItem(const PayableListItem&) = delete;
    PayableListItem(PayableListItem&&) = delete;
    auto operator=(const PayableListItem&) -> PayableListItem& = delete;
    auto operator=(PayableListItem&&) -> PayableListItem& = delete;

    ~PayableListItem() final = default;

private:
    using ot_super = implementation::ContactListItem;

    UnallocatedCString payment_code_;
    const UnitType currency_;

    auto calculate_section(const Lock& lock) const noexcept
        -> UnallocatedCString final
    {
        return translate_section(lock);
    }
    auto qt_data(const int column, const int role, QVariant& out) const noexcept
        -> void final;

    auto reindex(const PayableListSortKey&, CustomData&) noexcept -> bool final;
    auto reindex(
        const Lock&,
        const PayableListSortKey& key,
        CustomData& custom) noexcept -> bool final;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::PayableListItem>;
