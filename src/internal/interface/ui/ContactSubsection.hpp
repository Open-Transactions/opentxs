// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/interface/ui/List.hpp"
#include "internal/interface/ui/ListRow.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
class ContactItem;
class ContactSubsection;
}  // namespace ui

using OTUIContactSubsection = SharedPimpl<ui::ContactSubsection>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
/**
  This model represents a subsection of meta-data for a ContactSection of a
  specific Contact. Each row is a ContactItem containing metadata about this
  contact.
*/
class ContactSubsection : virtual public List, virtual public ListRow
{
public:
    /// Returns the name of the subsection.
    virtual auto Name(const UnallocatedCString& lang) const noexcept
        -> UnallocatedCString = 0;
    /// Returns the first ContactItem row.
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ContactItem> = 0;
    /// Returns the next ContactItem row.
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ContactItem> = 0;
    /// Returns the claim type as a ClaimType enum.
    virtual auto Type() const noexcept -> identity::wot::claim::ClaimType = 0;

    ContactSubsection(const ContactSubsection&) = delete;
    ContactSubsection(ContactSubsection&&) = delete;
    auto operator=(const ContactSubsection&) -> ContactSubsection& = delete;
    auto operator=(ContactSubsection&&) -> ContactSubsection& = delete;

    ~ContactSubsection() override = default;

protected:
    ContactSubsection() noexcept = default;
};
}  // namespace opentxs::ui
