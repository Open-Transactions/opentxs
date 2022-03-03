// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "ListRow.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace ui
{
class ContactListItem;
}  // namespace ui

using OTUIContactListItem = SharedPimpl<ui::ContactListItem>;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class OPENTXS_EXPORT ContactListItem : virtual public ListRow
{
public:
    virtual auto ContactID() const noexcept -> UnallocatedCString = 0;
    virtual auto DisplayName() const noexcept -> UnallocatedCString = 0;
    virtual auto ImageURI() const noexcept -> UnallocatedCString = 0;
    virtual auto Section() const noexcept -> UnallocatedCString = 0;

    ~ContactListItem() override = default;

protected:
    ContactListItem() noexcept = default;

private:
    ContactListItem(const ContactListItem&) = delete;
    ContactListItem(ContactListItem&&) = delete;
    auto operator=(const ContactListItem&) -> ContactListItem& = delete;
    auto operator=(ContactListItem&&) -> ContactListItem& = delete;
};
}  // namespace opentxs::ui
