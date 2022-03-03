// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "1_Internal.hpp"
#include "interface/ui/base/Row.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/identity/wot/claim/Item.hpp"
#include "opentxs/interface/ui/ContactItem.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

namespace ui
{
class ContactItem;
}  // namespace ui
// }  // namespace v1
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
    auto ClaimID() const noexcept -> UnallocatedCString final
    {
        sLock lock(shared_lock_);

        return row_id_->str();
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
    ~ContactItem() final = default;

private:
    std::unique_ptr<identity::wot::claim::Item> item_;

    auto reindex(
        const ContactSubsectionSortKey& key,
        CustomData& custom) noexcept -> bool final;

    ContactItem() = delete;
    ContactItem(const ContactItem&) = delete;
    ContactItem(ContactItem&&) = delete;
    auto operator=(const ContactItem&) -> ContactItem& = delete;
    auto operator=(ContactItem&&) -> ContactItem& = delete;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::ContactItem>;
