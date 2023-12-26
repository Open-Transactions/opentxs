// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/profile/ProfileItem.hpp"  // IWYU pragma: associated

#include <memory>
#include <optional>

#include "interface/ui/base/Widget.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/identity/wot/Claim.hpp"
#include "opentxs/identity/wot/claim/Attribute.hpp"  // IWYU pragma: keep
#include "opentxs/identity/wot/claim/Item.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
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

auto ProfileItem::add_claim(const identity::wot::Claim& claim) const noexcept
    -> bool
{
    auto reason = api_.Factory().PasswordPrompt(__func__);

    auto nym = api_.Wallet().mutable_Nym(parent_.NymID(), reason);

    return nym.AddClaim(claim, reason);
}

auto ProfileItem::as_claim() const noexcept -> identity::wot::Claim
{
    const auto lock = sLock{shared_lock_};

    return item_->asClaim();
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
    const auto lock = eLock{shared_lock_};
    item_ = std::make_unique<identity::wot::claim::Item>(
        extract_custom<identity::wot::claim::Item>(custom));

    assert_false(nullptr == item_);

    return true;
}

auto ProfileItem::SetActive(const bool& active) const noexcept -> bool
{
    auto claim = as_claim();
    using enum identity::wot::claim::Attribute;

    if (active) {
        claim.Add(Active);
    } else {
        claim.Remove(Active);
        claim.Remove(Primary);
    }

    return add_claim(claim);
}

auto ProfileItem::SetPrimary(const bool& primary) const noexcept -> bool
{
    auto claim = as_claim();
    using enum identity::wot::claim::Attribute;

    if (primary) {
        claim.Add(Active);
        claim.Add(Primary);
    } else {
        claim.Remove(Primary);
    }

    return add_claim(claim);
}

auto ProfileItem::SetValue(const UnallocatedCString& newValue) const noexcept
    -> bool
{
    return add_claim(as_claim().CreateModified(newValue));
}
}  // namespace opentxs::ui::implementation
