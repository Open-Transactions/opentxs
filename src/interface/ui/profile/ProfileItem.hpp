// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "interface/ui/base/Row.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/identity/wot/claim/Attribute.hpp"  // IWYU pragma: keep
#include "opentxs/identity/wot/claim/Item.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identity
{
namespace wot
{
class Claim;
}  // namespace wot
}  // namespace identity

namespace ui
{
class ProfileItem;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

template class opentxs::SharedPimpl<opentxs::ui::ProfileItem>;

namespace opentxs::ui::implementation
{
using ProfileItemRow =
    Row<ProfileSubsectionRowInternal,
        ProfileSubsectionInternalInterface,
        ProfileSubsectionRowID>;

class ProfileItem final : public ProfileItemRow
{
public:
    const api::session::Client& api_;

    auto ClaimID() const noexcept -> UnallocatedCString final
    {
        const auto lock = sLock{shared_lock_};

        return row_id_.asBase58(api_.Crypto());
    }
    auto Delete() const noexcept -> bool final;
    auto IsActive() const noexcept -> bool final
    {
        const auto lock = sLock{shared_lock_};

        return item_->HasAttribute(identity::wot::claim::Attribute::Active);
    }
    auto IsPrimary() const noexcept -> bool final
    {
        const auto lock = sLock{shared_lock_};

        return item_->HasAttribute(identity::wot::claim::Attribute::Primary);
    }
    auto SetActive(const bool& active) const noexcept -> bool final;
    auto SetPrimary(const bool& primary) const noexcept -> bool final;
    auto SetValue(const UnallocatedCString& value) const noexcept -> bool final;
    auto Value() const noexcept -> UnallocatedCString final
    {
        const auto lock = sLock{shared_lock_};

        return UnallocatedCString{item_->Value()};
    }

    ProfileItem(
        const ProfileSubsectionInternalInterface& parent,
        const api::session::Client& api,
        const ProfileSubsectionRowID& rowID,
        const ProfileSubsectionSortKey& sortKey,
        CustomData& custom) noexcept;
    ProfileItem() = delete;
    ProfileItem(const ProfileItem&) = delete;
    ProfileItem(ProfileItem&&) = delete;
    auto operator=(const ProfileItem&) -> ProfileItem& = delete;
    auto operator=(ProfileItem&&) -> ProfileItem& = delete;

    ~ProfileItem() final = default;

private:
    std::unique_ptr<identity::wot::claim::Item> item_;

    auto add_claim(const identity::wot::Claim& claim) const noexcept -> bool;
    auto as_claim() const noexcept -> identity::wot::Claim;

    auto reindex(
        const ProfileSubsectionSortKey& key,
        CustomData& custom) noexcept -> bool final;
};
}  // namespace opentxs::ui::implementation
