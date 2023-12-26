// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/payablelist/PayableListItem.hpp"  // IWYU pragma: associated

#include <memory>

#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/core/Contact.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto PayableListItem(
    const ui::implementation::PayableInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::PayableListRowID& rowID,
    const ui::implementation::PayableListSortKey& key,
    const UnallocatedCString& paymentcode,
    const UnitType& currency) noexcept
    -> std::shared_ptr<ui::implementation::PayableListRowInternal>
{
    using ReturnType = ui::implementation::PayableListItem;

    return std::make_shared<ReturnType>(
        parent, api, rowID, key, paymentcode, currency);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
PayableListItem::PayableListItem(
    const PayableInternalInterface& parent,
    const api::session::Client& api,
    const PayableListRowID& rowID,
    const PayableListSortKey& key,
    const UnallocatedCString& paymentcode,
    const UnitType& currency) noexcept
    : ot_super(parent, api, rowID, key)
    , payment_code_(paymentcode)
    , currency_(currency)
{
    init_contact_list();
}

auto PayableListItem::PaymentCode() const noexcept -> UnallocatedCString
{
    const auto lock = Lock{lock_};

    return payment_code_;
}

auto PayableListItem::reindex(
    const ContactListSortKey& key,
    CustomData& custom) noexcept -> bool
{
    const auto lock = Lock{lock_};

    return reindex(lock, key, custom);
}

auto PayableListItem::reindex(
    const Lock& lock,
    const ContactListSortKey& key,
    CustomData& custom) noexcept -> bool
{
    auto output = ot_super::reindex(lock, key, custom);

    const auto contact = api_.Contacts().Contact(row_id_);

    assert_false(nullptr == contact);

    auto paymentCode = contact->PaymentCode(currency_);

    if (payment_code_ != paymentCode) {
        payment_code_ = paymentCode;
        output |= true;
    }

    return output;
}
}  // namespace opentxs::ui::implementation
