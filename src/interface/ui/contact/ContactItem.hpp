// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <iosfwd>
#include <memory>

#include "interface/ui/base/Row.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/identity/wot/claim/Item.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{

namespace ui
{
class ContactItem;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
using ContactItemRow =
    Row<ContactSubsectionRowInternal,
        ContactSubsectionInternalInterface,
        ContactSubsectionRowID>;

class ContactItem final : public ContactItemRow
{
public:
    const api::session::Client& api_;

    auto ClaimID() const noexcept -> UnallocatedCString final
    {
        sLock lock(shared_lock_);

        return row_id_.asBase58(api_.Crypto());
    }
    auto IsActive() const noexcept -> bool final
    {
        sLock lock(shared_lock_);

        return item_->isActive();
    }
    auto IsPrimary() const noexcept -> bool final
    {
        sLock lock(shared_lock_);

        return item_->isPrimary();
    }
    auto Value() const noexcept -> UnallocatedCString final
    {
        sLock lock(shared_lock_);

        return item_->Value();
    }

    ContactItem(
        const ContactSubsectionInternalInterface& parent,
        const api::session::Client& api,
        const ContactSubsectionRowID& rowID,
        const ContactSubsectionSortKey& sortKey,
        CustomData& custom) noexcept;
    ContactItem() = delete;
    ContactItem(const ContactItem&) = delete;
    ContactItem(ContactItem&&) = delete;
    auto operator=(const ContactItem&) -> ContactItem& = delete;
    auto operator=(ContactItem&&) -> ContactItem& = delete;

    ~ContactItem() final = default;

private:
    std::unique_ptr<identity::wot::claim::Item> item_;

    auto reindex(
        const ContactSubsectionSortKey& key,
        CustomData& custom) noexcept -> bool final;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::ContactItem>;
