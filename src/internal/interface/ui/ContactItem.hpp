// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "ListRow.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
class ContactItem;
}  // namespace ui

using OTUIContactItem = SharedPimpl<ui::ContactItem>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
/**
  Each row in the ContactSubsection model is a ContactItem.
  This ContactItem model represents a specific claim about a Contact.
*/
class ContactItem : virtual public ListRow
{
public:
    /// Returns the claim ID for this claim.
    virtual auto ClaimID() const noexcept -> UnallocatedCString = 0;
    /// Returns a boolean indicating whether or not this claim is active.
    virtual auto IsActive() const noexcept -> bool = 0;
    /// Returns a boolean indicating whether or not this is the primary claim.
    virtual auto IsPrimary() const noexcept -> bool = 0;
    /// Returns the actual contains of the claim as a string.
    virtual auto Value() const noexcept -> UnallocatedCString = 0;

    ContactItem(const ContactItem&) = delete;
    ContactItem(ContactItem&&) = delete;
    auto operator=(const ContactItem&) -> ContactItem& = delete;
    auto operator=(ContactItem&&) -> ContactItem& = delete;

    ~ContactItem() override = default;

protected:
    ContactItem() noexcept = default;
};
}  // namespace opentxs::ui
