// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/contact/ContactSection.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <memory>

#include "internal/identity/wot/claim/Types.hpp"
#include "ottest/env/OTTestEnvironment.hpp"

namespace ottest
{
namespace ot = opentxs;

ContactSection::ContactSection()
    : api_(OTTestEnvironment::GetOT().StartClientSession(0))
    , contact_section_(
          dynamic_cast<const ot::api::session::Client&>(api_),
          ot::UnallocatedCString("testContactSectionNym1"),
          opentxs::CONTACT_CONTACT_DATA_VERSION,
          opentxs::CONTACT_CONTACT_DATA_VERSION,
          ot::identity::wot::claim::SectionType::Identifier,
          ot::identity::wot::claim::Section::GroupMap{})
    , contact_group_(new ot::identity::wot::claim::Group(
          ot::UnallocatedCString("testContactGroupNym1"),
          ot::identity::wot::claim::SectionType::Identifier,
          ot::identity::wot::claim::ClaimType::Employee,
          {}))
    , active_contact_item_(new ot::identity::wot::claim::Item(
          dynamic_cast<const ot::api::session::Client&>(api_),
          ot::UnallocatedCString("activeContactItem"),
          opentxs::CONTACT_CONTACT_DATA_VERSION,
          opentxs::CONTACT_CONTACT_DATA_VERSION,
          ot::identity::wot::claim::SectionType::Identifier,
          ot::identity::wot::claim::ClaimType::Employee,
          ot::UnallocatedCString("activeContactItemValue"),
          {ot::identity::wot::claim::Attribute::Active},
          {},
          {},
          ""))
{
}
}  // namespace ottest
