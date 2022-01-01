// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "1_Internal.hpp"                    // IWYU pragma: associated
#include "ui/contact/ContactSubsection.hpp"  // IWYU pragma: associated

#include <memory>
#include <set>
#include <thread>
#include <type_traits>

#include "internal/contact/Contact.hpp"
#include "internal/core/identifier/Identifier.hpp"  // IWYU pragma: keep
#include "internal/protobuf/verify/VerifyContacts.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/contact/ContactGroup.hpp"
#include "opentxs/contact/ContactItem.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "ui/base/Combined.hpp"
#include "ui/base/Widget.hpp"

namespace opentxs::factory
{
auto ContactSubsectionWidget(
    const ui::implementation::ContactSectionInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::ContactSectionRowID& rowID,
    const ui::implementation::ContactSectionSortKey& key,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::ContactSectionRowInternal>
{
    using ReturnType = ui::implementation::ContactSubsection;

    return std::make_shared<ReturnType>(parent, api, rowID, key, custom);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
ContactSubsection::ContactSubsection(
    const ContactSectionInternalInterface& parent,
    const api::session::Client& api,
    const ContactSectionRowID& rowID,
    const ContactSectionSortKey& key,
    CustomData& custom) noexcept
    : Combined(
          api,
          Identifier::Factory(parent.ContactID()),
          parent.WidgetID(),
          parent,
          rowID,
          key)
    , sequence_(-1)
{
    startup_ = std::make_unique<std::thread>(
        &ContactSubsection::startup,
        this,
        extract_custom<contact::ContactGroup>(custom));

    OT_ASSERT(startup_)
}

auto ContactSubsection::construct_row(
    const ContactSubsectionRowID& id,
    const ContactSubsectionSortKey& index,
    CustomData& custom) const noexcept -> RowPointer
{
    return factory::ContactItemWidget(*this, api_, id, index, custom);
}

auto ContactSubsection::Name(const std::string& lang) const noexcept
    -> std::string
{
    return proto::TranslateItemType(translate(row_id_.second), lang);
}

auto ContactSubsection::process_group(
    const contact::ContactGroup& group) noexcept
    -> std::pmr::set<ContactSubsectionRowID>
{
    OT_ASSERT(row_id_.second == group.Type())

    std::pmr::set<ContactSubsectionRowID> active{};

    for (const auto& [id, claim] : group) {
        OT_ASSERT(claim)

        CustomData custom{new contact::ContactItem(*claim)};
        add_item(id, ++sequence_, custom);
        active.emplace(id);
    }

    return active;
}

auto ContactSubsection::reindex(
    const ContactSectionSortKey&,
    CustomData& custom) noexcept -> bool
{
    delete_inactive(
        process_group(extract_custom<contact::ContactGroup>(custom)));

    return true;
}

void ContactSubsection::startup(const contact::ContactGroup group) noexcept
{
    process_group(group);
    finish_startup();
}
}  // namespace opentxs::ui::implementation
