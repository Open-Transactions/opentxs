// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                          // IWYU pragma: associated
#include "interface/ui/profile/ProfileItem.hpp"  // IWYU pragma: associated

#include <memory>
#include <tuple>
#include <utility>

#include "interface/ui/base/Widget.hpp"
#include "internal/identity/wot/claim/Types.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/identity/wot/claim/Attribute.hpp"
#include "opentxs/identity/wot/claim/Item.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/NymEditor.hpp"
#include "opentxs/util/PasswordPrompt.hpp"  // IWYU pragma: keep

namespace opentxs::factory
{
auto ProfileItemWidget(
    const ui::implementation::ProfileSubsectionInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::ProfileSubsectionRowID& rowID,
    const ui::implementation::ProfileSubsectionSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::ProfileSubsectionRowInternal>
{
    using ReturnType = ui::implementation::ProfileItem;

    return std::make_shared<ReturnType>(parent, api, rowID, sortKey, custom);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
ProfileItem::ProfileItem(
    const ProfileSubsectionInternalInterface& parent,
    const api::session::Client& api,
    const ProfileSubsectionRowID& rowID,
    const ProfileSubsectionSortKey& sortKey,
    CustomData& custom) noexcept
    : ProfileItemRow(parent, api, rowID, true)
    , api_(api)
    , item_{new identity::wot::claim::Item(
          extract_custom<identity::wot::claim::Item>(custom))}
{
}

auto ProfileItem::add_claim(const Claim& claim) const noexcept -> bool
{
    auto reason = api_.Factory().PasswordPrompt(__func__);

    auto nym = api_.Wallet().mutable_Nym(parent_.NymID(), reason);

    return nym.AddClaim(claim, reason);
}

auto ProfileItem::as_claim() const noexcept -> Claim
{
    sLock lock(shared_lock_);
    Claim output{};
    auto& [id, section, type, value, start, end, attributes] = output;
    id = "";
    section = translate(parent_.Section());
    type = translate(parent_.Type());
    value = item_->Value();
    start = item_->Start();
    end = item_->End();

    if (item_->isPrimary()) {
        attributes.emplace(translate(identity::wot::claim::Attribute::Primary));
    }

    if (item_->isPrimary() || item_->isActive()) {
        attributes.emplace(translate(identity::wot::claim::Attribute::Active));
    }

    return output;
}

auto ProfileItem::Delete() const noexcept -> bool
{
    auto reason = api_.Factory().PasswordPrompt(__func__);

    auto nym = api_.Wallet().mutable_Nym(parent_.NymID(), reason);

    return nym.DeleteClaim(row_id_, reason);
}

auto ProfileItem::reindex(
    const ProfileSubsectionSortKey&,
    CustomData& custom) noexcept -> bool
{
    eLock lock(shared_lock_);
    item_ = std::make_unique<identity::wot::claim::Item>(
        extract_custom<identity::wot::claim::Item>(custom));

    OT_ASSERT(item_);

    return true;
}

auto ProfileItem::SetActive(const bool& active) const noexcept -> bool
{
    Claim claim{};
    auto& attributes = std::get<6>(claim);

    if (active) {
        attributes.emplace(translate(identity::wot::claim::Attribute::Active));
    } else {
        attributes.erase(translate(identity::wot::claim::Attribute::Active));
        attributes.erase(translate(identity::wot::claim::Attribute::Primary));
    }

    return add_claim(claim);
}

auto ProfileItem::SetPrimary(const bool& primary) const noexcept -> bool
{
    Claim claim{};
    auto& attributes = std::get<6>(claim);

    if (primary) {
        attributes.emplace(translate(identity::wot::claim::Attribute::Primary));
        attributes.emplace(translate(identity::wot::claim::Attribute::Active));
    } else {
        attributes.erase(translate(identity::wot::claim::Attribute::Primary));
    }

    return add_claim(claim);
}

auto ProfileItem::SetValue(const UnallocatedCString& newValue) const noexcept
    -> bool
{
    Claim claim{};
    std::get<3>(claim) = newValue;

    return add_claim(claim);
}
}  // namespace opentxs::ui::implementation
