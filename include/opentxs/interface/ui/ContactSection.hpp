// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/interface/ui/List.hpp"
#include "opentxs/interface/ui/ListRow.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
class ContactSection;
class ContactSubsection;
}  // namespace ui

using OTUIContactSection = SharedPimpl<ui::ContactSection>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
/**
  This model represents a section of meta-data for a specific contact.
  Each row is a ContactSubsection containing metadata about this contact.
*/
class OPENTXS_EXPORT ContactSection : virtual public List,
                                      virtual public ListRow
{
public:
    /// Returns the section name.
    virtual auto Name(const UnallocatedCString& lang) const noexcept
        -> UnallocatedCString = 0;
    /// Returns the first contact subsection.
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ContactSubsection> = 0;
    /// Returns the next contact subsection.
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ContactSubsection> = 0;
    /// Returns the section type as an enum.
    virtual auto Type() const noexcept -> identity::wot::claim::SectionType = 0;

    ContactSection(const ContactSection&) = delete;
    ContactSection(ContactSection&&) = delete;
    auto operator=(const ContactSection&) -> ContactSection& = delete;
    auto operator=(ContactSection&&) -> ContactSection& = delete;

    ~ContactSection() override = default;

protected:
    ContactSection() noexcept = default;
};
}  // namespace opentxs::ui
