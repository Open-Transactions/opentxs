// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "interface/ui/contactlist/ContactListItem.hpp"  // IWYU pragma: associated

#include <locale>
#include <memory>
#include <utility>

#include "internal/interface/ui/UI.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"

namespace opentxs::factory
{
auto ContactListItem(
    const ui::implementation::ContactListInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::ContactListRowID& rowID,
    const ui::implementation::ContactListSortKey& key) noexcept
    -> std::shared_ptr<ui::implementation::ContactListRowInternal>
{
    using ReturnType = ui::implementation::ContactListItem;
    // TODO ContactListItem should have a common parent class and three final
    // child classes so that the construction process can work consistently no
    // matter which of the three child types is used.
    auto out = std::make_shared<ReturnType>(parent, api, rowID, key);
    out->init_contact_list();

    return out;
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
ContactListItem::ContactListItem(
    const ContactListInternalInterface& parent,
    const api::session::Client& api,
    const ContactListRowID& rowID,
    const ContactListSortKey& key) noexcept
    : ContactListItemRow(parent, api, rowID, true)
    , api_(api)
    , key_(key)
    , section_()
{
}

auto ContactListItem::calculate_section() const noexcept -> UnallocatedCString
{
    auto lock = Lock{lock_};

    return calculate_section(lock);
}

auto ContactListItem::calculate_section(const Lock& lock) const noexcept
    -> UnallocatedCString
{
    if (row_id_ == parent_.ID()) { return {"ME"}; }

    return translate_section(lock);
}

auto ContactListItem::ContactID() const noexcept -> UnallocatedCString
{
    return row_id_.asBase58(api_.Crypto());
}

auto ContactListItem::DisplayName() const noexcept -> UnallocatedCString
{
    auto lock = Lock{lock_};

    return key_.second;
}

auto ContactListItem::ImageURI() const noexcept -> UnallocatedCString
{
    // TODO

    return {};
}

auto ContactListItem::init_contact_list() noexcept -> void
{
    section_ = calculate_section();
}

auto ContactListItem::reindex(
    const ContactListSortKey& key,
    CustomData& custom) noexcept -> bool
{
    auto lock = Lock{lock_};

    return reindex(lock, key, custom);
}

auto ContactListItem::reindex(
    const Lock& lock,
    const ContactListSortKey& key,
    CustomData&) noexcept -> bool
{
    auto output = (key_ != key);

    if (output) { key_ = key; }

    if (auto section = calculate_section(lock); section != section_) {
        section_ = section;
        output |= true;
    }

    return output;
}

auto ContactListItem::Section() const noexcept -> UnallocatedCString
{
    auto lock = Lock{lock_};

    return section_;
}

auto ContactListItem::translate_section(const Lock&) const noexcept
    -> UnallocatedCString
{
    if (key_.second.empty()) { return {" "}; }

    std::locale locale;
    UnallocatedCString output{" "};
    output[0] = std::toupper(key_.second[0], locale);

    return output;
}
}  // namespace opentxs::ui::implementation
