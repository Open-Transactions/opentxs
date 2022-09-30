// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "interface/ui/contact/ContactSubsection.hpp"  // IWYU pragma: associated

#include <memory>
#include <string_view>
#include <thread>
#include <type_traits>

#include "interface/ui/base/Combined.hpp"
#include "interface/ui/base/Widget.hpp"
#include "internal/identity/wot/claim/Types.hpp"
#include "internal/serialization/protobuf/verify/VerifyContacts.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/identity/wot/claim/Group.hpp"
#include "opentxs/identity/wot/claim/Item.hpp"
#include "opentxs/util/Container.hpp"

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
          api.Factory().IdentifierFromBase58(parent.ContactID()),
          parent.WidgetID(),
          parent,
          rowID,
          key)
    , api_(api)
    , sequence_(-1)
{
    startup_ = std::make_unique<std::thread>(
        &ContactSubsection::startup,
        this,
        extract_custom<identity::wot::claim::Group>(custom));

    OT_ASSERT(startup_);
}

auto ContactSubsection::construct_row(
    const ContactSubsectionRowID& id,
    const ContactSubsectionSortKey& index,
    CustomData& custom) const noexcept -> RowPointer
{
    return factory::ContactItemWidget(*this, api_, id, index, custom);
}

auto ContactSubsection::Name(const UnallocatedCString& lang) const noexcept
    -> UnallocatedCString
{
    return UnallocatedCString{
        proto::TranslateItemType(translate(row_id_.second), lang)};
}

auto ContactSubsection::process_group(
    const identity::wot::claim::Group& group) noexcept
    -> UnallocatedSet<ContactSubsectionRowID>
{
    OT_ASSERT(row_id_.second == group.Type());

    UnallocatedSet<ContactSubsectionRowID> active{};

    for (const auto& [id, claim] : group) {
        OT_ASSERT(claim);

        CustomData custom{new identity::wot::claim::Item(*claim)};
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
        process_group(extract_custom<identity::wot::claim::Group>(custom)));

    return true;
}

void ContactSubsection::startup(
    const identity::wot::claim::Group group) noexcept
{
    process_group(group);
    finish_startup();
}
}  // namespace opentxs::ui::implementation
